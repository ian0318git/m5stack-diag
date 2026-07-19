/* $Id: platform_sgmii.c,v 1.2 2013/10/08 08:48:31 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_sgmii.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base SGMII port loopback test
 * 
 * Dec 2010 ptong
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
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

void sgmii_get_link_status(int, int *, int *, int*);
void smi_ctl_reg_dump(void);

/* Structures inheritted from Xformers to hold SGMII port
 * status data for displaying info.
 */
typedef struct {
    uint32_t    packets;
    uint32_t    controls;
    uint32_t    dmacs;
    uint32_t    drops;
    uint32_t    errors;
    uint32_t    rsvrd;        /* To preserve 64-bit alignment */
    uint64_t    octets;
    uint64_t    controlOctets;
    uint64_t    dmacOctets;
    uint64_t    dropOctets;
} cvmx_gmx_rx_status_t;

typedef struct {
    uint64_t    octets;
    uint32_t    packets;
    uint32_t    multicasts;
    uint32_t    broadcasts;
    uint32_t    underflows;
    uint32_t    controls;
    uint32_t    singleCollides;
    uint32_t    multiCollides;
    uint32_t    dropCollides;
    uint32_t    dropDefers;
    uint32_t    histSmall;
    uint32_t    hist64;
    uint32_t    hist127;
    uint32_t    hist255;
    uint32_t    hist511;
    uint32_t    hist1023;
    uint32_t    hist1518;
    uint32_t    histLarge;
} cvmx_gmx_tx_status_t;


/**********************************************************************
 *
 * Function: sgmii_get_link_status
 *
 * Description: Get SGMII Link Status from Cavium CPU
 *
 * Input: port_num - port number
 *
 * Return: NONE
 */
void sgmii_get_link_status (int port_num, int *link_up, int *speed, int*dup)
{
    cvmx_pcsx_anx_results_reg_t pcsx_anx_results_reg;
    int interface;

    if ((port_num > ETH0) && (port_num < ETH4)) {
        interface = CVMX_GMX0_INF_ID;
    } else {
        interface = CVMX_GMX4_INF_ID;
    }

    port_num %= 4;

    /* Read the autoneg results */
    pcsx_anx_results_reg.u64 = cvmx_read_csr(CVMX_PCSX_ANX_RESULTS_REG(port_num, interface));

    *link_up = pcsx_anx_results_reg.s.link_ok;
    *dup     = pcsx_anx_results_reg.s.dup;

    switch (pcsx_anx_results_reg.s.spd) {
    case 0:
        *speed = 10;
        break;
    case 1:
        *speed = 100;
        break;
    case 2:
        *speed = 1000;
        break;
    default:
        *speed = 0;
        *link_up = 0;
    }
}


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
void
set_gmxeno (int port_num, boolean onoff)
{
    cvmx_pcsx_miscx_ctl_reg_t pcs_misc_ctrl;
    int interface;
    int index = port_num;

    interface = getdec_answer("\nEnter interface number", 0, 0, CVMX_GMX4_INF_ID);
    printf("\nInterface %d\n", interface);

    /* Make sure gmxno is clear for rx */
    pcs_misc_ctrl.u64 = cvmx_read_csr(CVMX_PCSX_MISCX_CTL_REG(index, interface));
    if (onoff)
       pcs_misc_ctrl.s.gmxeno = 0; /* zero for process MAC address*/
    else 
       pcs_misc_ctrl.s.gmxeno = 1;
    pcs_misc_ctrl.s.samp_pt = 0x5; /* keep lower 8 can work both on 10/100 */
    cvmx_write_csr(CVMX_PCSX_MISCX_CTL_REG(index, interface), pcs_misc_ctrl.u64);

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
void
set_sgmii_int_lpbk (int eth_num, boolean onoff)
{
    #define SGMII_DRIVER_DELAY    1 // Kernel need time to bring up link
    cvmx_pcsx_mrx_control_reg_t pcs_mr_ctrl;
    int interface;
    int index = eth_num % 4;

    if ((eth_num == ETH0) ||(eth_num == ETH1) || (eth_num == ETH3)) {
        interface = CVMX_GMX0_INF_ID;
    } else {
        interface = CVMX_GMX4_INF_ID;
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nInterface %d\n", interface);
    }
    
    pcs_mr_ctrl.u64 = cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface));

    if (onoff == TRUE) {
      pcs_mr_ctrl.s.loopbck1 = 1;
    }
    else {
      pcs_mr_ctrl.s.loopbck1 = 0;
    }
    cvmx_write_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface), pcs_mr_ctrl.u64);

    /* To ensure the write is completed */
    pcs_mr_ctrl.u64 = cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface));

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
boolean
cavium_is_sgmii_linkup (int eth_num)
{
  //pfix this linkup check is not correct. need to fix it.
    cvmx_pcsx_mrx_status_reg_t pcs_mr_sts;
    int interface;
    int index = eth_num;
    int repeat = 1000;

    interface = getdec_answer("\nEnter interface number", 0, 0, CVMX_GMX4_INF_ID);
    printf("\nInterface %d\n", interface);


    do {
        msleep(10);
        pcs_mr_sts.u64 = cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(index, interface));
    } while ((repeat-- > 0) && (pcs_mr_sts.s.lnk_st == 0));
    return(pcs_mr_sts.s.lnk_st);
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
int
sgmii_port_cfg (int port_num, int speed, int an_en)
{
    cvmx_gmxx_prtx_cfg_t prtx_cfg;
    cvmx_pcsx_mrx_control_reg_t pcs_mr_ctrl;
    cvmx_pcsx_miscx_ctl_reg_t pcs_misc_ctrl;
    cvmx_pcsx_mrx_status_reg_t sts_reg;
    int interface;
    int index = port_num;
    int wait_cnt = 0;

    interface = getdec_answer("\nEnter interface number", 0, 0, CVMX_GMX4_INF_ID);
    printf("\nInterface %d\n", interface);


    /* Set autoneg, clear the loopback just in case other test set it */
    pcs_mr_ctrl.u64 = cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
    pcs_mr_ctrl.s.loopbck1 = 0;
    pcs_mr_ctrl.s.an_en = (an_en == 0) ? 0 : 1;

#if 0 //pfix
    // pfix-b follow the cvmx-helper-sgmii.c
    pcs_mr_ctrl.s.rst_an = 1;
    pcs_mr_ctrl.s.an_en = 1;
    pcs_mr_ctrl.s.pwr_dn = 0;
    // pfix-e
#endif

    switch(speed) {
    case SPD_10MBPS:
        pcs_mr_ctrl.s.spdmsb = 0;
        pcs_mr_ctrl.s.spdlsb = 0;
        break;
    case SPD_100MBPS:
        pcs_mr_ctrl.s.spdmsb = 0;
        pcs_mr_ctrl.s.spdlsb = 1;
        break;
    case SPD_1000MBPS:
        pcs_mr_ctrl.s.spdmsb = 1;
        pcs_mr_ctrl.s.spdlsb = 0;
        break;
    }
    pcs_mr_ctrl.s.dup = 1; /* full duplex */
    cvmx_write_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface), pcs_mr_ctrl.u64);

    while (wait_cnt < 200) { /* wait till link is up */
        msleep(500); 
        sts_reg.u64 = cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(index, interface));

        if (sts_reg.s.lnk_st) {
            do {
            prtx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));
            } while (!(prtx_cfg.s.rx_idle && prtx_cfg.s.tx_idle));

                /* Set port config register */
            prtx_cfg.s.duplex = 1;    /* full dup */
            switch(speed) {
            case SPD_10MBPS:
              prtx_cfg.s.speed_msb = 1;
              prtx_cfg.s.speed = 0;
              break;
            case SPD_100MBPS:
              prtx_cfg.s.speed_msb = 0;
              prtx_cfg.s.speed = 0;
              break;
            case SPD_1000MBPS:
              prtx_cfg.s.speed_msb = 0;
              prtx_cfg.s.speed = 1;
              break;
            }
                prtx_cfg.s.slottime = 1;
            cvmx_write_csr(CVMX_GMXX_TXX_SLOT(index, interface), 512);
            cvmx_write_csr(CVMX_GMXX_TXX_BURST(index, interface), 0 /*8192*/);
            cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), prtx_cfg.u64);
            //printf("\n===== write %#.8x to prtx_cfg\n", prtx_cfg.u64);

            /* Make sure gmxno is clear for rx */
            pcs_misc_ctrl.u64 = cvmx_read_csr(CVMX_PCSX_MISCX_CTL_REG(index, interface));
            pcs_misc_ctrl.s.gmxeno = 0;
            pcs_misc_ctrl.s.samp_pt = 8;
            cvmx_write_csr(CVMX_PCSX_MISCX_CTL_REG(index, interface), pcs_misc_ctrl.u64);

            break;
        }
        wait_cnt++;
    } /* while */

    /* Check the link status */
    if(cavium_is_sgmii_linkup(port_num)) {
        //printf("pfix %s() sgmii link is up\n", __FUNCTION__);
        return (PASSED);
    } else {
        cterr('f', 0, "pfix %s() sgmii link is not up\n", __FUNCTION__);
        return (FAILED);
    }
}

/*
 * Function: get_gmx_rx_status
 *
 * This function will get info on the Rx interface of the Octeon PKO.
 *
 * Input: port number.
 *        ptr to rx gmx status struct.
 *
 * Output: none.
 */
static void
get_gmx_rx_status (int localPort, cvmx_gmx_rx_status_t *stats)
{
    int interface;    // 52xx only has 1 interface.

    interface = getdec_answer("\nEnter interface number", 0, 0, CVMX_GMX4_INF_ID);
    fflush(stdout);
    printf("\nInterface %d\n", interface);
    fflush(stdout);

    stats->packets  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_PKTS (localPort, interface));
    stats->controls  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_PKTS_CTL (localPort, interface));
    stats->dmacs  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_PKTS_DMAC (localPort, interface));
    stats->drops  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_PKTS_DRP (localPort, interface));
    stats->errors  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_PKTS_BAD (localPort, interface));

    stats->octets  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_OCTS (localPort, interface));
    stats->controlOctets  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_OCTS_CTL (localPort, interface));
    stats->dmacOctets  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_OCTS_DMAC (localPort, interface));
    stats->dropOctets  =
        cvmx_read_csr (CVMX_GMXX_RXX_STATS_OCTS_DRP (localPort, interface));
}


/*
 * Function: get_gmx_tx_status
 *
 * This function will get info on the Tx port of Octeon.
 *
 * Input: port number.
 *        ptr to tx gmx port status.
 *
 * Output: none.
 */
static void
get_gmx_tx_status (int localPort, cvmx_gmx_tx_status_t *stats)
{
    int interface;    // 52xx only has 1 interface.

    interface = getdec_answer("\nEnter interface number", 0, 0, CVMX_GMX4_INF_ID);
    printf("\nInterface %d\n", interface);
    fflush(stdout);

    stats->dropCollides  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT0 (localPort, interface)) >> 32;
    stats->dropDefers  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT0 (localPort, interface)) &
        0x00000000FFFFFFFFLU;
    stats->singleCollides  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT1 (localPort, interface)) >> 32;
    stats->multiCollides  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT1 (localPort, interface)) &
        0x00000000FFFFFFFFLU;
    stats->packets =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT3 (localPort, interface));
    stats->octets  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT2 (localPort, interface));
    stats->multicasts  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT8 (localPort, interface)) >> 32;
    stats->broadcasts  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT8 (localPort, interface)) &
        0x00000000FFFFFFFFLU;
    stats->underflows  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT9 (localPort, interface)) >> 32;
    stats->controls  =
        cvmx_read_csr (CVMX_GMXX_TXX_STAT9 (localPort, interface)) &
        0x00000000FFFFFFFFLU;
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
void
display_sgmii_port_stats (void)
{
    int port;
    cvmx_gmx_rx_status_t    rxStats;
    cvmx_gmx_tx_status_t    txStats;

    port = gethex_answer("\nEnter port number", 0, 0, PLAT_SGMII_NUM_MAX);
    printf("\nPort %d\n", port);
    fflush(stdout);

    get_gmx_rx_status (port, &rxStats);
    get_gmx_tx_status (port, &txStats);

    printf("\n ----------------Tx Stats-------------------------------\n");
    fflush(stdout);
    printf (" Port%2d Tx:     sent Pkts %u (%lu octets)\n"
            "            multicast Pkts %u\n"
            "            broadcast Pkts %u\n"
            "              control Pkts %u\n"
            "            underflow Pkts %u\n"
            "        collisions: single %u, multiple %u\n"
            "         drops: collisions %u, deferrals %u\n",
            port, txStats.packets, txStats.octets, txStats.multicasts,
            txStats.broadcasts, txStats.controls, txStats.underflows,
            txStats.singleCollides, txStats.multiCollides,
            txStats.dropCollides, txStats.dropDefers);
    fflush(stdout);

    printf("\n ----------------Rx Stats-------------------------------\n");
    fflush(stdout);
    printf (" Port%2d Rx: received Pkts %u (%lu octets)\n"
            "              control Pkts %u (%lu octets)\n"
            "        DMAC filtered Pkts %u (%lu octets)\n"
            "              dropped Pkts %u (%lu octets)\n"
            "                    errors %u\n",
            port, rxStats.packets, rxStats.octets,
            rxStats.controls, rxStats.controlOctets,
            rxStats.dmacs, rxStats.dmacOctets,
            rxStats.drops, rxStats.dropOctets, rxStats.errors);
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
void
display_sgmii_port_cfg (void)
{
    ulong reg;
    int interface;
    int index;

    interface = getdec_answer("\nEnter interface number", 0, 0, CVMX_GMX4_INF_ID);
    fflush(stdout);
    printf("\nInterface %d\n", interface);
    fflush(stdout);

    index = gethex_answer("\nEnter port number", 0, 0, PLAT_SGMII_NUM_MAX);
    fflush(stdout);
    printf("\nPort %d\n", index);
    fflush(stdout);

    reg = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));
    printf("CVMX_GMXX_PRTX_CFG= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));
    printf("CVMX_GMXX_INF_MODE= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
    printf("CVMX_PCSX_MRX_CONTROL_REG= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_PCSX_MRX_STATUS_REG(index, interface));
    printf("CVMX_PCSX_MRX_STATUS_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_ANX_ADV_REG(index, interface));
    printf("CVMX_PCSX_ANX_ADV_REG= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_PCSX_ANX_LP_ABIL_REG(index, interface));
    printf("CVMX_PCSX_ANX_LP_ABIL_REG= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_PCSX_ANX_RESULTS_REG(index, interface));
    printf("CVMX_PCSX_ANX_RESULTS_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_SGMX_AN_ADV_REG(index, interface));
    printf("CVMX_PCSX_SGMX_AN_ADV_REG= %#.lx\n", reg);
    reg = cvmx_read_csr(CVMX_PCSX_SGMX_LP_ADV_REG(index, interface));
    printf("CVMX_PCSX_SGMX_LP_ADV_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_MISCX_CTL_REG(index, interface));
    printf("CVMX_PCSX_MISCX_CTL_REG= %#.lx\n", reg);

    printf("\nDump more register information\n");

    reg = cvmx_read_csr(CVMX_PCSX_TXX_STATES_REG(index, interface));
    printf("CVMX_PCSX_TXX_STATES_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_TX_RXX_POLARITY_REG(index, interface));
    printf("CVMX_PCSX_TX_RXX_POLARITY_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_RXX_STATES_REG(index, interface));
    printf("CVMX_PCSX_RXX_STATES_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_RXX_SYNC_REG(index, interface));
    printf("CVMX_PCSX_RXX_SYNC_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_LINKX_TIMER_COUNT_REG(index, interface));
    printf("CVMX_PCSX_LINKX_TIMER_COUNT_REG= %#.lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSX_ANX_EXT_ST_REG(index, interface));
    printf("CVMX_PCSX_ANX_EXT_ST_REG= %#.lx\n", reg);
}

/*
 * Function: sgmii_phy_reg_dump
 *
 * This function displays the PHY setting of the requested SGMII port.
 *
 * Input: none.
 *
 * Output: void
 */
void
sgmii_phy_reg_dump(void)
{
    int port;

    port = getdec_answer("\nEnter SGMII port number", 0, 0, PLAT_SGMII_NUM_MAX);
    fflush(stdout);
    phy_reg_dump(NAME_ETH, port);
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
 * Revision 1.2  2013/10/08 08:48:31  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/06/18 00:33:45  leschen
 * Add fflush
 *
 * Revision 1.1.2.2  2013/06/17 11:11:27  leschen
 * Add fflush
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.3  2013/03/27 04:49:37  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.8  2013/02/18 08:48:38  leslie
 * Modify set_sgmii_int_lpbk function to judge QLM0 or QLM4
 *
 * Revision 1.7  2013/01/18 06:46:58  leslie
 * Fix and clean up code.
 *
 * Revision 1.6  2012/10/24 10:45:40  leslie
 * Add auto judge QLM0 or QLM4.
 *
 * Revision 1.5  2012/09/19 10:06:03  leslie
 * Fix the set_sgmii_int_lpbk function for woodlawn internal loopback test.
 *
 * Revision 1.4  2012/09/05 22:57:00  kody
 * Modify for dumping different QLM interface register.
 *
 * Revision 1.3  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.6  2011/12/07 11:54:31  alpeng
 * using semaphore to handle pthreads, replacing cmpbyte to memcmp.
 * update menu item.
 *
 * Revision 1.1.2.4  2011/11/22 08:55:43  alpeng
 * clean up code and remove useless delay
 *
 * Revision 1.1.2.3  2011/11/02 00:55:24  alpeng
 * update loopback test, add util. packet number and length should be increased
 *
 * Revision 1.1.2.2  2011/09/09 22:22:12  ptong
 * Add phy_reg_access and cfg_phy functions
 *
 * Revision 1.1.2.1  2011/04/05 19:59:38  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
