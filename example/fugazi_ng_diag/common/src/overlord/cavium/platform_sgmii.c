/* $Id: platform_sgmii.c,v 1.6 2012/11/03 01:28:36 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_sgmii.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base SGMII port loopback test
 * 
 * Dec 2010 ptong
 * Copyright (c) 2011-2012 by Cisco Systems, Inc.
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

#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"

#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"

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
    int interface = CVMX_GMX0_INF_ID;
    int index = port_num;

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
 * Function: set_sgmii_int_lpbk
 *
 * Description:
 * Set or clear the internal loopback bit of the cavium sgmii ports
 *
 * Input: eth_num - ethernet port number
 *        onoff - true or false to tur on or off the internal loopback
 *
 * Return: void
 */
void
set_sgmii_int_lpbk (int eth_num, boolean onoff)
{
#define SGMII_DRIVER_DELAY    1 // Kernel need time to bring up link
    cvmx_pcsx_mrx_control_reg_t pcs_mr_ctrl;
    int interface = CVMX_GMX0_INF_ID;
    int index = eth_num;

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
    sleep(SGMII_DRIVER_DELAY);
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
    cvmx_pcsx_mrx_status_reg_t pcs_mr_sts;
    int interface = CVMX_GMX0_INF_ID;
    int index = eth_num;
    int repeat = 100;

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
    int interface = CVMX_GMX0_INF_ID;
    int index = port_num;
    int wait_cnt = 0;

    /* Set autoneg, clear the loopback just in case other test set it */
    pcs_mr_ctrl.u64 = cvmx_read_csr(CVMX_PCSX_MRX_CONTROL_REG(index, interface));
    pcs_mr_ctrl.s.loopbck1 = 0;
    pcs_mr_ctrl.s.an_en = (an_en == 0) ? 0 : 1;

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

    while (wait_cnt < 20) { /* wait till link is up */
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
        return(PASS);
    }
    else {
	printf("%s() sgmii link is not up\n", __FUNCTION__);
        return(FAIL);
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
    int interface   = 0;    // 52xx only has 1 interface.

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
    int interface   = 0;    // 52xx only has 1 interface.

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

    get_gmx_rx_status (port, &rxStats);
    get_gmx_tx_status (port, &txStats);

    printf("\n ----------------Tx Stats-------------------------------\n");
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

    printf("\n ----------------Rx Stats-------------------------------\n");
    printf (" Port%2d Rx: received Pkts %u (%lu octets)\n"
            "              control Pkts %u (%lu octets)\n"
            "        DMAC filtered Pkts %u (%lu octets)\n"
            "              dropped Pkts %u (%lu octets)\n"
            "                    errors %u\n",
            port, rxStats.packets, rxStats.octets,
            rxStats.controls, rxStats.controlOctets,
            rxStats.dmacs, rxStats.dmacOctets,
            rxStats.drops, rxStats.dropOctets, rxStats.errors);
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
    int interface = CVMX_GMX0_INF_ID;
    int index;

    index = gethex_answer("\nEnter port number", 0, 0, PLAT_SGMII_NUM_MAX);
    printf("\nPort %d\n", index);

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
}

/*
 * Function: sgmii_phy_reg_dump
 *
 * This function displays the PHY setting of the requested SGMII port.
 * Using phy_reg_show() to dump PHY page. 
 *
 * Input: none.
 *
 * Output: void
 */
void
sgmii_phy_reg_dump(void)
{
    int port, phy_sel, dump_type, page_sel = 0;

    port = getdec_answer("\nEnter SGMII port number", 0, 0, PLAT_SGMII_NUM_MAX);
    phy_sel = getdec_answer("\nSelect 0:Ext. 1:Int. PHY", 0, 0, 1);

    if (phy_sel)
        phy_sel = ADDR_BRIDGE_PHY;
    else 
        phy_sel = ADDR_MEDIA_PHY;


    if (getc_answer("Dump ALL Page?", "yn", 'y') == 'y') {
        dump_type = DUMP_ALL_PAGE;

    } else {
        /* select page. */
        dump_type = DUMP_ONE_PAGE;
        page_sel = getdec_answer("\nSelect PAGE", 0, 0, PLAT_PAGE_NUM_MAX);
    }


    phy_reg_show(port, phy_sel, page_sel, dump_type);
}

/*-------------------------------------------------
$Log: platform_sgmii.c,v $
Revision 1.6  2012/11/03 01:28:36  ptong
Document and clean up

Revision 1.5  2012/10/05 09:12:10  alpeng
support media/bridge PHY register dump

Revision 1.4  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
