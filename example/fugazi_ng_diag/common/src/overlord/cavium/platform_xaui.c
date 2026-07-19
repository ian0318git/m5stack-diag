/* $Id: platform_xaui.c,v 1.6 2012/11/06 23:00:51 ptong Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_xaui.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base XAUI port loopback test
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
#include "monitor.h"
#include "cross_platform.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "proto.h"
#include "queryflags.h"


#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"

#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"

/* Packets to be used in xaui port loopback tests
 */
static pktdata_info_t pktdata[] = {
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 30},
  {0xa1, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 30},
  {0xa2, (ETH_UDP_DATA_MAX_LEN - 1), H_INCFILL, 30},
  {0xa3, ETH_UDP_DATA_MAX_LEN, H_INCFILL, 30},
};

/* Platform CPU xaui ports configurations for diagnostics
 */
static eth_lpbk_info_t eth_lpbk_tbl[] = {
    {XAUI0,  NAME_XAUI,  OCTEON_XAUI0_IP_ADDR,  OCTEON_DUMMY_IP_ADDR,  OCTEON_NETMASK,
     0,  0,  pktdata, sizeof(pktdata)/sizeof(pktdata_info_t) },
};

/**********************************************************************
 *
 * Function: is_xaui_linkup
 *
 * Description:
 * Check if the xaui port link status is up
 * 
 * Input: portnum - xaui port number
 *
 * Return: true/false
 */
boolean
is_xaui_linkup (int portnum)
{
    int interface = CVMX_XAUI_INF_ID;
    int repeat = 100;
    cvmx_gmxx_tx_xaui_ctl_t gmxx_tx_xaui_ctl;
    cvmx_gmxx_rx_xaui_ctl_t gmxx_rx_xaui_ctl;
    cvmx_pcsxx_status1_reg_t pcsxx_status1_reg;
    boolean linkup = FALSE;


    do {
        msleep(10);
	/* Link is up only if both RX and TX are happy */
	gmxx_tx_xaui_ctl.u64 = cvmx_read_csr(CVMX_GMXX_TX_XAUI_CTL(interface));
	gmxx_rx_xaui_ctl.u64 = cvmx_read_csr(CVMX_GMXX_RX_XAUI_CTL(interface));
	pcsxx_status1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_STATUS1_REG(interface));

	if ((gmxx_tx_xaui_ctl.s.ls == 0) && (gmxx_rx_xaui_ctl.s.status == 0) &&
	    (pcsxx_status1_reg.s.rcv_lnk == 1)) {
	    linkup = TRUE;
	}
    } while ((repeat-- > 0) && (linkup == FALSE));
    return(linkup);
}

/**********************************************************************
 *
 * Function: xaui_port_cfg
 *
 * Description: Configure the xaui port for proper operation
 * 
 * Input: portnum - Xaui port number (XAUI0, XAUI1, etc)
 *
 * Return: pass/fail
 */
int
xaui_port_cfg (int portnum)
{
    cvmx_pcsxx_misc_ctl_reg_t xauiMiscCtl;
    int interface = CVMX_XAUI_INF_ID;

    /* Make sure gmxno is clear for rx */
    xauiMiscCtl.u64 = cvmx_read_csr(CVMX_PCSXX_MISC_CTL_REG(interface));
    xauiMiscCtl.s.gmxeno = 0;
    cvmx_write_csr (CVMX_PCSXX_MISC_CTL_REG(interface), xauiMiscCtl.u64);
    return(PASS);
}

/**********************************************************************
 *
 * Function: set_xaui_int_lpbk
 *
 * Description:
 * Set or clear the internal loopback bit of the cavium xaui port
 * 
 * Input: portnum - ethernet port number
 *        onoff - true or false to tur on or off the internal loopback
 *
 * Return: void
 */
void
set_xaui_int_lpbk (int portnum, boolean onoff)
{
#define XAUI_DRIVER_DELAY    1 // Kernel need time to bring up link
    int interface = CVMX_XAUI_INF_ID;
    cvmx_pcsxx_control1_reg_t pcsxx_control1_reg;

    pcsxx_control1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_CONTROL1_REG(interface));

    if (onoff == TRUE) {
      pcsxx_control1_reg.s.loopbck1 = 1;
    }
    else {
      pcsxx_control1_reg.s.loopbck1 = 0;
    }
    cvmx_write_csr (CVMX_PCSXX_CONTROL1_REG(interface), pcsxx_control1_reg.u64);

    /* To ensure the write is completed */
    pcsxx_control1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_CONTROL1_REG(interface));

    /* Give time for HW to settle when loopback is set */
    sleep(XAUI_DRIVER_DELAY);
}

/**********************************************************************
 *
 * Function: xaui_port_lpbk
 *
 * Description:
 * XAUI port loopback test
 * 
 * Input: portnum - port number
 *        lbmode - internal or external loopback mode
 *
 * Return: pass/fail
 */
static int
xaui_port_lpbk(int port, int lbmode)
{
    int pn, pcnt;
    int rv;

    pcnt = sizeof(eth_lpbk_tbl)/sizeof(eth_lpbk_info_t);

    for (pn=0; pn < pcnt; pn++) {
      if (port == eth_lpbk_tbl[pn].eth_port)
	break;
    }

    if (pn >= pcnt) {
        printf("%s() XAUI port %d not valid\n", __func__, port);
	return(FAIL);
    }

    if(is_xaui_linkup(port) == FALSE) {
        printf("XAUI port %d link is down\n", port);
	return(FAIL);
    }

    /* Configure the port for loopback */
    xaui_port_cfg(port);
    if (lbmode == LOOP_INT) {
        set_xaui_int_lpbk(port, TRUE);
    }
    else {
        set_xaui_int_lpbk(port, FALSE);
    }

    rv = eth_port_lpbk(&eth_lpbk_tbl[pn]);

    if (lbmode == LOOP_INT) {
        set_xaui_int_lpbk(port, FALSE);
    }
    
    return(rv);
}

/**********************************************************************
 *
 * Function: xaui_lpbk_test
 *
 * Description: XAUI loopback test for both internal and external
 * 
 * Input: lbmode - loopback mode
 *
 * Return: pass/fail
 */
static int
xaui_lpbk_test(int lbmode)
{
    int pp, pcnt, port;
    int rc;
    int evb_xaui_list[] = { XAUI0 };
    int *xaui_lpbk_ports;
    char *lbname;
 
    lbname = (lbmode == LOOP_INT) ? "internal" : "external";
    
    testname("Xaui %s loopback", lbname);
    prpass(testpass, "");

    xaui_lpbk_ports = evb_xaui_list;
    pcnt = sizeof(evb_xaui_list) / sizeof(int);

    for (pp=0; pp < pcnt; pp++) {
        port = xaui_lpbk_ports[pp];
	rc = xaui_port_lpbk(port, lbmode);

	if (rc == FAIL) {
	    cterr('f',0,"XAUI port %d %s loopback failed\n", port, lbname);
	    return(FAIL);
	}
	printf("XAUI port %d %s loopback passed.\n", port, lbname);
    }
    return(PASS);
}

/**********************************************************************
 *
 * Function: xaui_internal_lpbk_test
 *
 * Description: XAUI internal loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int
xaui_internal_lpbk_test(void)
{
    return(xaui_lpbk_test(LOOP_INT));
}

/*
 * Function: xaui_external_lpbk_test
 *
 * Description: XAUI external loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int
xaui_external_lpbk_test(void)
{
    return(xaui_lpbk_test(LOOP_EXT));
}

/*
 * Function: config_xaui0
 *
 * Description: Use system call to execute "ifconif" to setup xaui0.
 *
 * Input: void
 *
 * Return: void
 */
void config_xaui0(void)
{
    char cmdbuf[64];

    sprintf(cmdbuf, "ifconfig xaui0 %s netmask 255.255.255.0 -promisc arp up", OCTEON_XAUI0_IP_ADDR);
    system(cmdbuf);
}

/*
 * Function: xaui_ping_test
 *
 * Description: Cavium XAUI port ping the host SGMII port 1 via
 *              the GESW to verify the XAUI is functional.
 *
 * Input: void
 *
 * Return: void
 */
int xaui_ping_test(void)
{
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline;
    char *interval_str, *result_file;
    FILE *fp;
    int tx_cnt, rx_cnt;
    int rv = FAIL;

    config_xaui0();

    testname("Xaui ping host CPU via GE switch");
    prpass(testpass, "");
    printf("\n");

    pktcnt = 400;
    interval_str = "0.01";
    deadline = 5;
    result_file = "xaui_ping_result";

    fp = fopen(result_file, "r");
    if (fp != NULL) {
        fclose(fp);
	sprintf(cmdbuf, "rm -f %s", result_file);
	system(cmdbuf);
    }

    sprintf(cmdbuf, "ping -c %d -i %s -w %d -I xaui0 %s > %s",
	    pktcnt, interval_str, deadline, HOST_ETH1_IP_ADDR,
	    result_file);

    system(cmdbuf);

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        printf("Error: %s was not created\n", result_file);
	goto  xaui_ping_test_exit;
    }
    
    /* Check the result
     */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);

	if (strstr(buf, "received") != NULL) {
	    printf("ping result: %s", buf);
	    break;
	}
    }
    fclose(fp);
    sprintf(cmdbuf, "rm -f %s", result_file);
    system(cmdbuf);

    /* Read the string 
     */
    sscanf(buf, "%d %s %s %d", &tx_cnt, dum_char, dum_char, &rx_cnt);

    if((rx_cnt != pktcnt) || (tx_cnt != pktcnt)) {
        printf("Error: ping test packet count mismatch. "
	       "Expected= %d, Actual: tx= %d rx= %d\n",
	       pktcnt, tx_cnt, rx_cnt);
	goto  xaui_ping_test_exit;
    }

    rv = PASS;

 xaui_ping_test_exit:

    if (rv == PASS) {
        printf("ping passed\n");
    }
    else {
        cterr('f',0,"ping failed\n");
    }

    return(rv);
}

/*
 * Function: dump_xaui_port_status
 *
 * Description: Util to display XAUI registers
 *
 * Input: void
 *
 * Return: void
 */
void
display_xaui_port_status(void)
{
    int interface = CVMX_XAUI_INF_ID;
    int ipd_port = 0;
    cvmx_gmxx_tx_xaui_ctl_t gmxx_tx_xaui_ctl;
    cvmx_gmxx_rx_xaui_ctl_t gmxx_rx_xaui_ctl;
    cvmx_pcsxx_status1_reg_t pcsxx_status1_reg;
    cvmx_pcsxx_control1_reg_t pcsxx_control1_reg;

    gmxx_tx_xaui_ctl.u64 = cvmx_read_csr(CVMX_GMXX_TX_XAUI_CTL(interface));
    gmxx_rx_xaui_ctl.u64 = cvmx_read_csr(CVMX_GMXX_RX_XAUI_CTL(interface));
    pcsxx_status1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_STATUS1_REG(interface));
    pcsxx_control1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_CONTROL1_REG(interface));

    /* GMX tx link status */
    printf("CVMX_GMXX_TX_XAUI_CTL= %#1lx\n", gmxx_tx_xaui_ctl.u64);
    printf("  gmx tx link status bypass= %d\n", gmxx_tx_xaui_ctl.s.ls_byp);
    printf("  gmx tx link status= %#x\n", gmxx_tx_xaui_ctl.s.ls);
    
    /* GMX rx link status */
    printf("CVMX_GMXX_RX_XAUI_CTL= %#1lx\n", gmxx_rx_xaui_ctl.u64);
    printf("  gmx rx link status= %#x\n", gmxx_rx_xaui_ctl.s.status);

    /* PCSX rx link status */
    printf("CVMX_PCSXX_STATUS1_REG= %#1lx\n", pcsxx_status1_reg.u64);
    printf("  pcs rcv link status= %#x\n", pcsxx_status1_reg.s.rcv_lnk);

    /* PCSX control1 */
    printf("CVMX_PCSXX_CONTROL1_REG= %#1lx\n", pcsxx_control1_reg.u64);
    printf("  pcs internal loopback= %#x\n", pcsxx_control1_reg.s.loopbck1);

    printf("Link Status: XAUI port %d is ", ipd_port);
    if ((gmxx_tx_xaui_ctl.s.ls == 0) &&
	(gmxx_rx_xaui_ctl.s.status == 0) &&
	(pcsxx_status1_reg.s.rcv_lnk == 1)) {
        printf("up\n");
    }
    else {
        printf("down\n");
    }
}

/*
 * Function: dump_xaui_gmx_regs
 *
 * Description: Util to display XAUI registers
 *
 * Input: void
 *
 * Return: void
 */
void
dump_xaui_gmx_regs(void)
{
    int interface = CVMX_XAUI_INF_ID;
    int ipd_port = 0;
    ulong reg;

    reg = cvmx_read_csr(CVMX_GMXX_TX_XAUI_CTL(interface));
    printf("CVMX_GMXX_TX_XAUI_CTL= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_RX_XAUI_CTL(interface));
    printf("CVMX_GMXX_RX_XAUI_CTL= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_RX_XAUI_BAD_COL(interface));
    printf("CVMX_GMXX_RX_XAUI_BAD_COL= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_XAUI_EXT_LOOPBACK(interface));
    printf("CVMX_GMXX_XAUI_EXT_LOOPBACK= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_RX_HG2_STATUS(interface));
    printf("CVMX_GMXX_RX_HG2_STATUS= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_HG2_CONTROL(interface));
    printf("CVMX_GMXX_HG2_CONTROL= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_TX_HG2_REG1(interface));
    printf("CVMX_GMXX_TX_HG2_REG1= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_TX_HG2_REG2(interface));
    printf("CVMX_GMXX_TX_HG2_REG2= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_PRTX_CBFC_CTL(ipd_port, interface));
    printf("CVMX_GMXX_PRTX_CBFC_CTL= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_TXX_CBFC_XOFF(ipd_port, interface));
    printf("CVMX_GMXX_TXX_CBFC_XOFF= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_GMXX_TXX_CBFC_XON(ipd_port, interface));
    printf("CVMX_GMXX_TXX_CBFC_XON= %#1lx\n", reg);
}

/*
 * Function: dump_xaui_pcs_regs
 *
 * Description: Util to display the XAUI registers
 *
 * Input: void
 *
 * Return: void
 */
void
dump_xaui_pcs_regs(void)
{
    int interface = CVMX_XAUI_INF_ID;
    ulong reg;

    reg = cvmx_read_csr(CVMX_PCSXX_CONTROL1_REG(interface));
    printf("CVMX_PCSXX_CONTROL1_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_STATUS1_REG(interface));
    printf("CVMX_PCSXX_STATUS1_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_SPD_ABIL_REG(interface));
    printf("CVMX_PCSXX_SPD_ABIL_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_CONTROL2_REG(interface));
    printf("CVMX_PCSXX_CONTROL2_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_STATUS2_REG(interface));
    printf("CVMX_PCSXX_STATUS2_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_10GBX_STATUS_REG(interface));
    printf("CVMX_PCSXX_10GBX_STATUS_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_TX_RX_STATES_REG(interface));
    printf("CVMX_PCSXX_TX_RX_STATES_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_RX_SYNC_STATES_REG(interface));
    printf("CVMX_PCSXX_RX_SYNC_STATES_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_TX_RX_POLARITY_REG(interface));
    printf("CVMX_PCSXX_TX_RX_POLARITY_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_MISC_CTL_REG(interface));
    printf("CVMX_PCSXX_MISC_CTL_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_BIT_LOCK_STATUS_REG(interface));
    printf("CVMX_PCSXX_BIT_LOCK_STATUS_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_INT_REG(interface));
    printf("CVMX_PCSXX_INT_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_INT_EN_REG(interface));
    printf("CVMX_PCSXX_INT_EN_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_LOG_ANL_REG(interface));
    printf("CVMX_PCSXX_LOG_ANL_REG= %#1lx\n", reg);

    reg = cvmx_read_csr(CVMX_PCSXX_BIST_STATUS_REG(interface));
    printf("CVMX_PCSXX_BIST_STATUS_REG= %#1lx\n", reg);
}

/*
 * Function: xaui_int_lpbk_util
 *
 * Description: Utility to set or clear Cavium XAUI internal loopback
 *
 * Input: none
 *
 * Return: void
 */
void
xaui_int_lpbk_util(void)
{
    int port = 0;

    if(getc_answer("Set internal loopback? ", "yn", 'y') == 'y') {
        set_xaui_int_lpbk(port, TRUE);
    }
    else {
        set_xaui_int_lpbk(port, FALSE);
    }
}

#ifdef KEEP_FOR_REFERENCE
/* Note: This function does not work with loopback plug, but
 * it may work with a link partner. Will try it when we have
 * the Vindicator HW.
 */
/**
 * @INTERNAL
 * Bringup and enable a XAUI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 */
int force_xaui_enable(void)
{
    cvmx_gmxx_prtx_cfg_t          gmx_cfg;
    cvmx_pcsxx_control1_reg_t     xauiCtl;
    cvmx_pcsxx_status1_reg_t      xauiStatus1;
    cvmx_pcsxx_misc_ctl_reg_t     xauiMiscCtl;
    cvmx_gmxx_tx_xaui_ctl_t       gmxXauiTxCtl;
    cvmx_gmxx_rx_xaui_ctl_t       gmxXauiRxCtl;
    cvmx_pcsxx_10gbx_status_reg_t pcsxx_10gbx_status;
    int interface = CVMX_XAUI_INF_ID;
    int repeat = 100;

    /* (1) Interface has already been enabled. */

    /* (2) Disable GMX. */
    xauiMiscCtl.u64 = cvmx_read_csr(CVMX_PCSXX_MISC_CTL_REG(interface));
    xauiMiscCtl.s.gmxeno = 1;
    cvmx_write_csr (CVMX_PCSXX_MISC_CTL_REG(interface), xauiMiscCtl.u64);

    /* (3) Disable GMX and PCSX interrupts. */
    cvmx_write_csr(CVMX_GMXX_RXX_INT_EN(0,interface), 0x0);
    cvmx_write_csr(CVMX_GMXX_TX_INT_EN(interface), 0x0);
    cvmx_write_csr(CVMX_PCSXX_INT_EN_REG(interface), 0x0);

    /* (4) Bring up the PCSX and GMX reconciliation layer. */
    /* (4)a Set polarity and lane swapping. */
    /* (4)b */
    gmxXauiTxCtl.u64 = cvmx_read_csr (CVMX_GMXX_TX_XAUI_CTL(interface));
    gmxXauiTxCtl.s.dic_en = 1; /* Enable better IFG packing and improves performance */
    gmxXauiTxCtl.s.uni_en = 0;
    cvmx_write_csr (CVMX_GMXX_TX_XAUI_CTL(interface), gmxXauiTxCtl.u64);

    /* (4)c Aply reset sequence */
    xauiCtl.u64 = cvmx_read_csr (CVMX_PCSXX_CONTROL1_REG(interface));
    xauiCtl.s.lo_pwr = 0;
    xauiCtl.s.reset  = 1;
    cvmx_write_csr (CVMX_PCSXX_CONTROL1_REG(interface), xauiCtl.u64);

    /* Wait for PCS to come out of reset */
    repeat = 100;
    do {
        msleep(10);
        xauiCtl.u64 = cvmx_read_csr (CVMX_PCSXX_CONTROL1_REG(interface));
    } while ((repeat-- > 0) && (xauiCtl.s.reset != 0));
    if (repeat <= 0) {
        printf("%s() Xaui PCS did not come out of reset\n", __FUNCTION__);
	return(FAIL);
    }

    /* Wait for PCS to be aligned */
    repeat = 100;
    do {
        msleep(100);
	pcsxx_10gbx_status.u64 = cvmx_read_csr (CVMX_PCSXX_10GBX_STATUS_REG(interface));
    } while ((repeat-- > 0) && (pcsxx_10gbx_status.s.alignd != 1));
    if (repeat <= 0) {
        printf("%s() Xaui PCS 10GBX not aligned\n", __FUNCTION__);
	return(FAIL);
    }

    /* Wait for RX to be ready */
    repeat = 100;
    do {
        msleep(10);
	gmxXauiRxCtl.u64 = cvmx_read_csr (CVMX_GMXX_RX_XAUI_CTL(interface));
    } while ((repeat-- > 0) && (gmxXauiRxCtl.s.status != 0));
    if (repeat <= 0) {
        printf("%s() Xaui GMX RX link not ok\n", __FUNCTION__);
	return(FAIL);
    }

    /* (6) Configure GMX */
    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(0, interface));
    gmx_cfg.s.en = 0;
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

    /* GMX configure */
    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(0, interface));
    gmx_cfg.s.speed = 1;
    gmx_cfg.s.speed_msb = 0;
    gmx_cfg.s.slottime = 1;
    cvmx_write_csr(CVMX_GMXX_TX_PRTS(interface), 1);
    cvmx_write_csr(CVMX_GMXX_TXX_SLOT(0, interface), 512);
    cvmx_write_csr(CVMX_GMXX_TXX_BURST(0, interface), 8192);
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

    /* Wait for receive link */
    repeat = 100;
    do {
        msleep(10);
        xauiStatus1.u64 = cvmx_read_csr (CVMX_PCSXX_STATUS1_REG(interface));
    } while ((repeat-- > 0) && (xauiStatus1.s.rcv_lnk != 1));
    if (repeat <= 0) {
        printf("%s() Xaui PCS rcv link not up\n", __FUNCTION__);
	return(FAIL);
    }

    /* (8) Enable packet reception */
    xauiMiscCtl.s.gmxeno = 0;
    cvmx_write_csr (CVMX_PCSXX_MISC_CTL_REG(interface), xauiMiscCtl.u64);

    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(0, interface));
    gmx_cfg.s.en = 1;
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

    return (PASS);
}
#endif /* reference */

/*-------------------------------------------------
$Log: platform_xaui.c,v $
Revision 1.6  2012/11/06 23:00:51  ptong
Add header and clean up

Revision 1.5  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.4  2012/05/08 00:05:15  ptong
Improve test printing

Revision 1.3  2012/04/17 22:01:26  ptong
Added more utility to run DP test from host.

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
