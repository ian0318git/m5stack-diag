/* $Id: platform_sgmii.c,v 1.2 2018/05/18 09:24:57 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_sgmii.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base SGMII port loopback test
 * 
 * July 2016 Mecca Ho
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>


#include "defs.h"
#include "types.h"
#include "common.h"
#include "proto.h"
#include "monitor.h"
#include "cross_platform.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "queryflags.h"
#include "nvsysvars.h"

#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"
#include "cvmx-smix-defs.h"

#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"

void smi_ctl_reg_dump(void);

/* SGMII */
int sgmii_mapping_qlm_num[] = {CVMX_GMX5_INF_ID, CVMX_GMX5_INF_ID,
                               CVMX_GMX6_INF_ID, CVMX_GMX6_INF_ID};

/* Structures inheritted from Xformers to hold SGMII port
 * status data for displaying info.
 */
typedef struct {
    uint64_t    packets;
    uint64_t    octets;
    uint64_t    pause;
    uint64_t    controlPause;
    uint64_t    dmacPackets;
    uint64_t    dmacOctets;
    uint64_t    drop;
    uint64_t    dropOctets;
    uint64_t    errors;
} cvmx_bgx_rx_status_t;

typedef struct {
    uint64_t    dropCollides;
    uint64_t    dropDefers;
    uint64_t    multiCollides;
    uint64_t    singleCollides;
    uint64_t    octets;
    uint64_t    packets;
    uint64_t    histSmall;
    uint64_t    hist64;
    uint64_t    hist127;
    uint64_t    hist255;
    uint64_t    hist511;
    uint64_t    hist1023;
    uint64_t    hist1518;
    uint64_t    histLarge;
    uint64_t    broadcastDMAC;
    uint64_t    multicastDMAC;
    uint64_t    underflows;
    uint64_t    controlPause;
} cvmx_bgx_tx_status_t;

/**********************************************************************
 *
 * Function: set_gmxeno
 *
 * Description: Configure the GMX register to prevent the GMX can 
 *              process the MAC address
 * 
 * Input: port_num - port number
 *        onoff - turn on/off this function
 *
 * Return: NONE
 */
void set_gmxeno (int eth_num, boolean onoff)
{
    int index = eth_mapping_sgmii_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];
    cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;

    /* Make sure gmxno is clear for rx */
    gmp_misc_ctl.u64 = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
    if (onoff) {
        gmp_misc_ctl.s.gmxeno = 0; /* zero for process MAC address*/
    }

    cvmx_write_csr(CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface), gmp_misc_ctl.u64);
}

/**********************************************************************
 *
 * Destination   Gateway    Genmask         Flags Metric Ref Use Iface
 * 196.1.1.1     *          255.255.255.255 UH    0      0     0 eth3
 * 
 * In order for the GE switch to accept and forward the packet from the
 * ingress port (e.g. port 0) to another port (e.g. port 1) at which
 * loopback is configured, a broadcast packet is sent out from the 
 * cpu port to the GE swtich. The router arp table must have the
 * following entry.
 *
 * Return: void
 */
void set_sgmii_int_lpbk (int eth_num, boolean onoff)
{
#define SGMII_DRIVER_DELAY    1 // Kernel need time to bring up link

    int index = eth_mapping_sgmii_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];
    cvmx_bgxx_gmp_pcs_mrx_control_t gmp_mrx_control;
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nInterface %d\n", interface);
    }
    
    gmp_mrx_control.u64 = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));

    if (onoff == TRUE) {
    	gmp_mrx_control.s.loopbck1 = 1;
    } else {
    	gmp_mrx_control.s.loopbck1 = 0;
    }

    cvmx_write_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface), gmp_mrx_control.u64);

    /* Soft-reset PCS.
     * A PCS reset sequence should always be used whenever
     * the operating mode of PCS logic changes.
     */
    gmp_mrx_control.s.reset = 1;
    cvmx_write_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface), gmp_mrx_control.u64);

    /* To ensure the write is completed */
    gmp_mrx_control.u64 = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));

    /* Give time for Linux drive and HW to settle when loopback is set */
    if (onoff) {
        sleep(SGMII_DRIVER_DELAY);
    }
}

/**********************************************************************
 *
 * Function: cavium_is_sgmii_linkup
 *
 * Description:
 * Check if the SGMII port link status is up (for cavium internal lopback)
 *
 * Input: eth_num - ethernet port number
 *
 * Return: true/false
 */
boolean cavium_is_sgmii_linkup (int eth_num)
{
    cvmx_bgxx_gmp_pcs_mrx_status_t gmp_mrx_status;
    int index = eth_mapping_sgmii_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];
    int repeat = 1000;

    do {
        msleep(10);
        gmp_mrx_status.u64 = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_STATUS(index, interface));
    } while ((repeat-- > 0) && (gmp_mrx_status.s.lnk_st == 0));

    return(gmp_mrx_status.s.lnk_st);
}


/**********************************************************************
 *
 * Function: sgmii_port_cfg
 *
 * Description: Configure the sgmii port for loopback test
 * 
 * Input: port_num - SGMII port number (SGMII0, SGMII1, etc)
 *
 * Return: pass/fail
 */
int sgmii_port_cfg (int eth_num, int speed, int an_en)
{
    int index = eth_mapping_sgmii_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];
    cvmx_bgxx_gmp_pcs_mrx_control_t gmp_mrx_control;

    /* Set autoneg, clear the loopback just in case other test set it */
    gmp_mrx_control.u64 = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
    gmp_mrx_control.s.loopbck1 = 0;
    gmp_mrx_control.s.an_en = (an_en == 0) ? 0 : 1;

    switch(speed) {
        case SPD_10MBPS:
            gmp_mrx_control.s.spdmsb = 0;
            gmp_mrx_control.s.spdlsb = 0;
	    break;
        case SPD_100MBPS:
            gmp_mrx_control.s.spdmsb = 0;
            gmp_mrx_control.s.spdlsb = 1;
	    break;
        case SPD_1000MBPS:
            gmp_mrx_control.s.spdmsb = 1;
            gmp_mrx_control.s.spdlsb = 0;
	    break;
    }
    gmp_mrx_control.s.dup = 1; /* full duplex */
    cvmx_write_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface), gmp_mrx_control.u64);

    /* Check the link status */
    if (cavium_is_sgmii_linkup(eth_num)) {
        return(PASS);
    } else {
        printf("%s() sgmii link is not up\n", __FUNCTION__);
        return(FAIL);
    }
}

/*
 * Function: get_bgx_rx_status
 *
 * This function will get info on the Rx interface of the Octeon PKO.
 *
 * Input: port number.
 *        ptr to rx gmx status struct.
 *
 * Output: none.
 */
static void get_bgx_rx_status (int eth_num, cvmx_bgx_rx_status_t *stats)
{
    int index = eth_mapping_sgmii_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];

    printf("\nInterface %d\n", interface);
    fflush(stdout);

    stats->packets  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT0 (index, interface));
    stats->octets  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT1 (index, interface));
    stats->pause  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT2 (index, interface));
    stats->controlPause  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT3 (index, interface));
    stats->dmacPackets  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT4 (index, interface));
    stats->dmacOctets  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT5 (index, interface));
    stats->drop  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT6 (index, interface));
    stats->dropOctets  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT7 (index, interface));
    stats->errors  =
        cvmx_read_csr (CVMX_BGXX_CMRX_RX_STAT8 (index, interface));
}


/*
 * Function: get_bgx_tx_status
 *
 * This function will get info on the Tx port of Octeon.
 *
 * Input: port number.
 *        ptr to tx gmx port status.
 *
 * Output: none.
 */
static void get_bgx_tx_status (int eth_num, cvmx_bgx_tx_status_t *stats)
{
    int index = eth_mapping_sgmii_num[eth_num];
    int interface = eth_mapping_cvmx_bgx_num[eth_num];

    printf("\nInterface %d\n", interface);
    fflush(stdout);

    stats->dropCollides  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT0 (index, interface));
    stats->dropDefers  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT1 (index, interface));
    stats->multiCollides  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT2 (index, interface));
    stats->singleCollides  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT3 (index, interface));
    stats->octets  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT4 (index, interface));
    stats->packets =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT5 (index, interface));
    stats->histSmall  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT6 (index, interface));
    stats->hist64  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT7 (index, interface));
    stats->hist127  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT8 (index, interface));
    stats->hist255  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT9 (index, interface));
    stats->hist511  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT10 (index, interface));
    stats->hist1023  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT11 (index, interface));
    stats->hist1518  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT12 (index, interface));
    stats->histLarge  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT13 (index, interface));
    stats->broadcastDMAC  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT14 (index, interface));
    stats->multicastDMAC  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT15 (index, interface));
    stats->underflows  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT16 (index, interface));
    stats->controlPause  =
        cvmx_read_csr (CVMX_BGXX_CMRX_TX_STAT17 (index, interface));
}

/*
 * Function: display_sgmii_port_stats
 *
 * This function displays stats for the requested SGMII port.
 *
 * Input: none.
 *
 * Output: void
 */
void display_sgmii_port_stats (void)
{
    int port;
    cvmx_bgx_rx_status_t    rxStats;
    cvmx_bgx_tx_status_t    txStats;

    port = gethex_answer("\nEnter port number", 0, 0, PLAT_SGMII_NUM_MAX);
    printf("\nPort %d\n", port);
    fflush(stdout);

    get_bgx_rx_status (port, &rxStats);
    get_bgx_tx_status (port, &txStats);

    printf("\n ----------------Tx Stats-------------------------------\n");
    fflush(stdout);
    printf (" Port%2d Tx:     packets sent: %lu (%lu octets)\n"
            "            multicast Pkts %lu\n"
            "            broadcast Pkts %lu\n"
            "              control/pause Pkts %lu\n"
            "            underflow Pkts %lu\n"
    		"        collisions: single %lu, multiple %lu\n"
            "         drops: collisions %lu, deferrals %lu\n",
            port, txStats.packets, txStats.octets, txStats.multicastDMAC,
            txStats.broadcastDMAC, txStats.controlPause, txStats.underflows,
            txStats.singleCollides, txStats.multiCollides, txStats.dropCollides,
            txStats.dropDefers);
    fflush(stdout);

    printf("\n ----------------Rx Stats-------------------------------\n");
    fflush(stdout);
    printf (" Port%2d Rx: received Pkts %lu (%lu octets)\n"
    		"             pause Pkts %lu\n"
            "             control/pause Pkts %lu\n"
            "        DMAC filtered Pkts %lu (%lu octets)\n"
            "              dropped Pkts %lu (%lu octets)\n"
            "                    errors %lu\n",
            port, rxStats.packets, rxStats.octets, rxStats.pause,
            rxStats.controlPause, rxStats.dmacPackets, rxStats.dmacOctets,
            rxStats.drop, rxStats.dropOctets, rxStats.errors);
    fflush(stdout);
}

/*
 * Function: display_sgmii_port_cfg
 *
 * This function displays the setting of the requested SGMII port.
 *
 * Input: none.
 *
 * Output: void
 */
void display_sgmii_port_cfg (void)
{
    ulong reg;
    int index, interface = CVMX_BGX2_INF_ID;

    index = gethex_answer("\nEnter port number(GE0 ~ GE3)", 0, 0, PLAT_SGMII_NUM_MAX);

    fflush(stdout);
    printf("\nPort %d, Interface %d\n", index, interface);
    fflush(stdout);

    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface));
    printf("CVMX_BGXX_GMP_PCS_MRX_CONTROL= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_STATUS(index, interface));
    printf("CVMX_BGXX_GMP_PCS_MRX_STATUS= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_ANX_ADV(index, interface));
    printf("CVMX_BGXX_GMP_PCS_ANX_ADV= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_ANX_LP_ABIL(index, interface));
    printf("CVMX_BGXX_GMP_PCS_ANX_LP_ABIL= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_ANX_RESULTS(index, interface));
    printf("CVMX_BGXX_GMP_PCS_ANX_RESULTS= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_ANX_EXT_ST(index, interface));
    printf("CVMX_BGXX_GMP_PCS_ANX_EXT_ST= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, interface));
    printf("CVMX_BGXX_GMP_PCS_LINKX_TIMER= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_TX_RXX_POLARITY(index, interface));
    printf("CVMX_BGXX_GMP_PCS_TX_RXX_POLARITY= %#.lx\n", reg);


    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_RXX_SYNC(index, interface));
    printf("CVMX_BGXX_GMP_PCS_RXX_SYNC= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_RXX_STATES(index, interface));
    printf("CVMX_BGXX_GMP_PCS_RXX_STATES= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_TXX_STATES(index, interface));
    printf("CVMX_BGXX_GMP_PCS_TXX_STATES= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(index, interface));
    printf("CVMX_BGXX_GMP_PCS_SGMX_AN_ADV= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_SGMX_LP_ADV(index, interface));
    printf("CVMX_BGXX_GMP_PCS_SGMX_LP_ADV= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MISCX_CTL(index, interface));
    printf("CVMX_BGXX_GMP_PCS_MISCX_CTL= %#.lx\n", reg);
}


/*
 * Function: smi_ctl_reg_dump
 *
 * This function displays Cavium SMI register settings
 *
 * Input: none.
 *
 * Output: void
 */
void smi_ctl_reg_dump (void)
{
    int bus_id;
    cvmx_smix_rd_dat_t smi_rd;
    cvmx_smix_clk_t smi_clk;
    cvmx_smix_cmd_t smi_cmd;
    cvmx_smix_wr_dat_t smi_wr;
    cvmx_smix_en_t smi_en;

    printf("SMI Register Display:");

    for (bus_id = 0; bus_id < 4; bus_id++) {
        printf("\nSMI Bus-%d:\n", bus_id);
        smi_rd.u64 = cvmx_read_csr(CVMX_SMIX_RD_DAT(bus_id));
        smi_wr.u64 = cvmx_read_csr(CVMX_SMIX_WR_DAT(bus_id));
        smi_clk.u64 = cvmx_read_csr(CVMX_SMIX_CLK(bus_id));
        smi_en.u64 = cvmx_read_csr(CVMX_SMIX_EN(bus_id));
        smi_cmd.u64 = cvmx_read_csr(CVMX_SMIX_CMD(bus_id));

        printf(" Enable       : %#lx\n", smi_en.u64);
        printf(" Clock        : %#lx\n", smi_clk.u64);
        printf(" Command      : %#lx\n", smi_cmd.u64);
        printf(" Write        : %#lx\n", smi_wr.u64);
        printf(" Read         : %#lx\n", smi_rd.u64);
    }

}

/*-------------------------------------------------
 * $Log: platform_sgmii.c,v $
 * Revision 1.2  2018/05/18 09:24:57  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.7  2017/04/10 05:27:25  meho
 * Integrated BCM82752/82757 API.
 *
 * Revision 1.1.2.6  2016/11/28 03:43:55  meho
 * 1. Fixed GE phy Mac/Int/Ext loopback test bugs.
 * 2. Added 10G FW download.
 *
 * Revision 1.1.2.5  2016/08/24 06:55:53  meho
 * Added dump Cavium sgmii tx/rx statistic register utility.
 *
 * Revision 1.1.2.4  2016/08/18 06:57:49  meho
 * Code clean up.
 *
 * Revision 1.1.2.3  2016/08/12 10:12:19  meho
 * Clean up code.
 *
 * Revision 1.1.2.2  2016/07/13 08:28:09  meho
 * 1. Added Cavium PCS internal loopback.
 * 2. Added check link up function for bcm54194.
 *
 * Revision 1.1.2.1  2016/07/07 09:04:30  meho
 * 1. Added BCM54194 RDB register r/w utility.
 * 2. Added GE PHY internal/external loopback skeleton.
 * 3. Added 10GE PHY internal/external loopback skeleton.
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
