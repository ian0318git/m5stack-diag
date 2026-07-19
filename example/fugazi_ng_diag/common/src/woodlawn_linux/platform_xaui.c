/* $Id: platform_xaui.c,v 1.2 2013/10/08 08:48:31 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_xaui.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base XAUI port loopback test
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
#include "diag_common_lib.h"

#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_xaui.h"
#include "platform_ext_lpbk.h"

/*****************************************************************************
 *  Functions Declaration
 *****************************************************************************/
void config_bp_xaui(void);
void config_xaui(void);
int xaui_lpbk_test(int);
int xaui_set_packet(int, int); 

extern int __cvmx_helper_xaui_link_init(int);
    
/* Packets to be used in xaui port loopback tests
 */
static pktdata_info_t pktdata[] = {
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 10},
  {0xa1, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 10},
  {0xa2, (ETH_UDP_DATA_MAX_LEN - 1), H_INCFILL, 10},
  {0xa3, ETH_UDP_DATA_MAX_LEN, H_INCFILL, 10},
};

/* Platform CPU xaui ports configurations for diagnostics
 */
static eth_lpbk_info_t eth_lpbk_tbl[] = {
    {XAUI1,  NAME_XAUI,  OCTEON_XAUI1_IP_ADDR,  OCTEON_XAUI1_DUMMY_IP_ADDR,  OCTEON_NETMASK,
     0,  0,  pktdata, sizeof(pktdata)/sizeof(pktdata_info_t) },
};

/* Platform CPU xaui ports configurations for diagnostics
 */
static eth_lpbk_info_t bp_xaui_lpbk_tbl[] = {
    {XAUI0,  NAME_XAUI,  OCTEON_XAUI0_IP_ADDR,  OCTEON_XAUI0_DUMMY_IP_ADDR,  OCTEON_NETMASK,
     0,  0,  pktdata, sizeof(pktdata)/sizeof(pktdata_info_t) },
};

/******************************************************************************
 *
 * Function: enable_xaui1_interface
 *
 * Description: This function enables the xaui0 interface.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int enable_xaui1_interface(void)
{
    char *ifconfig_all = "ifconfig -a";
    char *enable_xaui1= "ifconfig xaui0 19.19.19.19 netmask 255.255.0.0 promisc -arp -allmulti -multicast";
    char xaui_test[INFO_LEN];


    /* Clear the system command log buffer */
    memset(xaui_test, '\0', INFO_LEN);

    /* if xaui0 is not existed, enable the xaui0 */
    if (exec_cmd(ifconfig_all, xaui_test, INFO_LEN) == FAILED) {
        cterr('f', 0, "Execute ifconfig command failed");
        return (FAILED);
    } else if(strstr(xaui_test, "xaui1") == NULL) {
        if (exec_cmd(enable_xaui1, xaui_test, INFO_LEN) == FAILED) {
            cterr('f', 0, "Enable xaui1 command failed");
            return (FAILED);
        } else if(strstr(xaui_test, "xaui1") == NULL) {
            cterr('f', 0, "Failed to enalbe xaui1 interface");
            return (FAILED);
        }
    }

    return (PASSED);
}

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
    cvmx_pcsxx_status2_reg_t pcsxx_status2_reg;
    cvmx_pcsxx_status2_reg_t qlm2_pcsxx_status2_reg;
    cvmx_pcsxx_control1_reg_t pcsxx_control1_reg;
    boolean linkup = FAILED;

    if (__cvmx_helper_xaui_link_init(interface)  == -1) {
        cterr('f', 0, "xaui0 initialize fail");
        return (FAILED);
    }

    do {
        msleep(10);
        /* Link is up only if both RX and TX are happy */
        gmxx_tx_xaui_ctl.u64 = cvmx_read_csr(CVMX_GMXX_TX_XAUI_CTL(interface));
        gmxx_rx_xaui_ctl.u64 = cvmx_read_csr(CVMX_GMXX_RX_XAUI_CTL(interface));
        pcsxx_status1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_STATUS1_REG(interface));
        pcsxx_status2_reg.u64 = cvmx_read_csr(CVMX_PCSXX_STATUS2_REG(interface));
        qlm2_pcsxx_status2_reg.u64 = cvmx_read_csr(CVMX_PCSXX_STATUS2_REG(interface));
        
        pcsxx_control1_reg.u64 = cvmx_read_csr(CVMX_PCSXX_CONTROL1_REG(interface));
#if 0
        printf("pcsxx_status1_reg.s.lpable is %x\n", pcsxx_status1_reg.s.lpable);
        printf("pcsxx_status1_reg.s.rcv_lnk is %x\n", pcsxx_status1_reg.s.rcv_lnk);
        printf("pcsxx_status2reg.s.dev %x\n", pcsxx_status2_reg.s.dev);
        printf("qlm2_pcsxx_status2_reg.s.dev %x\n", qlm2_pcsxx_status2_reg.s.dev);

        printf("gmxx_tx_xaui_ctl.s.ls is %x\n", gmxx_tx_xaui_ctl.s.ls);
        printf("gmxx_rx_xaui_ctl.s.status is %x\n", gmxx_rx_xaui_ctl.s.status);
#endif
        printf("pcsxx_control1_reg.s.reset is %x\n", pcsxx_control1_reg.s.reset);

        if ((gmxx_tx_xaui_ctl.s.ls == 0) && (gmxx_rx_xaui_ctl.s.status == 0) &&
            (pcsxx_status1_reg.s.rcv_lnk == 1)) {
            linkup = PASSED;
        }
    } while ((repeat-- > 0) && (linkup == FAILED));

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
xaui_port_cfg (int portnum, int lbmode)
{
    cvmx_pcsxx_misc_ctl_reg_t xauiMiscCtl;
    int interface;
 
    if (lbmode == LOOP_XAUI_BP) {
        interface = CVMX_BP_XAUI_INF_ID;
    } else {
        interface = CVMX_XAUI_INF_ID;
    }

    /* Make sure gmxno is clear for rx */
    xauiMiscCtl.u64 = cvmx_read_csr(CVMX_PCSXX_MISC_CTL_REG(interface));
    xauiMiscCtl.s.gmxeno = 0;
    cvmx_write_csr (CVMX_PCSXX_MISC_CTL_REG(interface), xauiMiscCtl.u64);

    return (PASSED);
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
set_xaui_int_lpbk (int lbmode, boolean onoff)
{
#define XAUI_DRIVER_DELAY    1 // Kernel need time to bring up link

    int interface;
    if (lbmode == LOOP_XAUI_BP) {
        interface = CVMX_BP_XAUI_INF_ID;
    } else {
        interface = CVMX_XAUI_INF_ID;
    }
    
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
 * Function: set_xaui_ext_lpbk
 *
 * Description:
 * Set or clear the external loopback bit of the cavium xaui port
 *
 * Input: onoff - true or false to tur on or off the internal loopback
 *
 * Return: void
 */
int
set_xaui_ext_lpbk (boolean onoff)
{
    int interface = CVMX_GMX2_INF_ID;

    cvmx_gmxx_xaui_ext_loopback_t gmxx_xaui_ext_loopback;
    cvmx_gmxx_prtx_cfg_t          gmx_cfg;
    cvmx_pcsxx_control1_reg_t     xauiCtl;
    cvmx_pcsx_miscx_ctl_reg_t     xauiMiscCtl;
    cvmx_gmxx_tx_xaui_ctl_t       gmxXauiTxCtl;

    /* Set the external loop */
    gmxx_xaui_ext_loopback.u64 = cvmx_read_csr(CVMX_GMXX_XAUI_EXT_LOOPBACK(interface));
    gmxx_xaui_ext_loopback.s.en = onoff;
    cvmx_write_csr(CVMX_GMXX_XAUI_EXT_LOOPBACK(interface), gmxx_xaui_ext_loopback.u64);

    /* Take the link through a reset */
    /* To Do: Porting from Cavium SDK, but execute this will casue hang
     * in woodlawn, do not know the root cause */
#if NOT_HANG_HERE
    if (__cvmx_helper_xaui_link_init(interface)  == -1) {
        cterr('f', 0, "xaui0 initialize fail");
        return (FAILED);
    }
#endif

    /* (1) Interface has already been enabled. */

    /* (2) Disable GMX. */
    xauiMiscCtl.u64 = cvmx_read_csr(CVMX_PCSX_MISCX_CTL_REG(0, interface));
    xauiMiscCtl.s.gmxeno = 1;
    cvmx_write_csr (CVMX_PCSX_MISCX_CTL_REG(0, interface), xauiMiscCtl.u64);

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

    /* GMX configure */
    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(0, interface));
    gmx_cfg.s.speed = 1;
    gmx_cfg.s.speed_msb = 0;
    gmx_cfg.s.slottime = 1;
    cvmx_write_csr(CVMX_GMXX_TX_PRTS(interface), 1);
    cvmx_write_csr(CVMX_GMXX_TXX_SLOT(0, interface), 512);
    cvmx_write_csr(CVMX_GMXX_TXX_BURST(0, interface), 8192);
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

    /* (8) Enable packet reception */
    xauiMiscCtl.u64 = cvmx_read_csr(CVMX_PCSX_MISCX_CTL_REG(0, interface));
    xauiMiscCtl.s.gmxeno = 0;
    cvmx_write_csr (CVMX_PCSX_MISCX_CTL_REG(0, interface), xauiMiscCtl.u64);

    /* Clear all error interrupts before enabling the interface. */
    cvmx_write_csr(CVMX_GMXX_RXX_INT_REG(0,interface), ~0x0ull);
    cvmx_write_csr(CVMX_GMXX_TX_INT_REG(interface), ~0x0ull);
    cvmx_write_csr(CVMX_PCSXX_INT_REG(interface), ~0x0ull);

    /* Enable GMX */
    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(0, interface));
    gmx_cfg.s.en = 1;
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

    msleep(3000);

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: xaui_set_packet
 *  Set up packet info for tx and rx using.
 *
 *
 * Input:  port: current test port   
 *         speed: current test speed   
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int xaui_set_packet(int port, int speed) 
{    
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0;
    uchar orig_hkpflag = hkeepflags;
    
    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;
    
    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = pktdata[typ_curr].send_count;
        pkt_len = pktdata[typ_curr].len;
        pkt_val = pktdata[typ_curr].val;
        hkeepflags |= pktdata[typ_curr].hkpflags;

        prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                                    port, speed, pkt_cnt, pkt_len, pkt_val);
                  
        /* To do the tx/rx loopback test */
        rc = tx_rx_diag(SEL_PORT_XAUI, port, speed, pkt_cnt, pkt_len, pkt_val);
       
        if (rc == FAILED) {
            cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            show_status_info(port + ADDR_MEDIA_PHY);
            return (FAILED);
        }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", port, speed);
    fflush(stdout);
    
    hkeepflags = orig_hkpflag;

    return (PASSED);
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
    int rv = PASSED;

    if (lbmode == LOOP_XAUI_BP) {
        pcnt = sizeof(bp_xaui_lpbk_tbl)/sizeof(eth_lpbk_info_t);
    } else {
        pcnt = sizeof(eth_lpbk_tbl)/sizeof(eth_lpbk_info_t);
    }
    

    for (pn = 0; pn < pcnt; pn++) {
        /* Scan all the ports in the table */
        if ((port == eth_lpbk_tbl[pn].eth_port) || (port == bp_xaui_lpbk_tbl[pn].eth_port)) {
            break;
        }
    }

    if (pn >= pcnt) {
        cterr('f', 0, "%s() XAUI pn is %d pcnt is %d port %d not valid\n",
              __func__, pn, pcnt, port);
        return(FAILED);
    }

#ifdef WORKING_ON_O2
    /* To do: Woodlawn will hang inside this function.
     * Need to check the root cause
     */
    if(is_xaui_linkup(port) == FAILED) {
        cterr('f', 0, "XAUI port %d link is down\n", port);
        return (FAILED);
    }
#endif

    /* Configure the port for loopback */
    xaui_port_cfg(port, lbmode);
    if (lbmode == LOOP_INT) {
        set_xaui_int_lpbk(lbmode, TRUE);
    } else {
        set_xaui_int_lpbk(lbmode, FALSE);
    }

    if (lbmode == LOOP_EXT) {
        /* internal and external xaui loopback test */
        if (xaui_set_packet(XAUI1, SPD_10000MBPS) == FAILED) {
            cterr('f', 0, "%s(): xaui_set_packet failed", __FUNCTION__);
            return (FAILED);
        }
    } else {
        /* backplane xaui loopback test */
        if (xaui_set_packet(XAUI0, SPD_10000MBPS) == FAILED) {
            cterr('f', 0, "%s(): xaui_set_packet failed", __FUNCTION__);
            return (FAILED);
        }
    }

    if (lbmode == LOOP_INT) {
        set_xaui_int_lpbk(lbmode, FALSE);
    }
    
    return (rv);
}

/**********************************************************************
 *
 * Function: xaui_lpbk_test
 *
 * Description: XAUI loopback test for internal and external and backplane
 * 
 * Input: lbmode - loopback mode
 *
 * Return: pass/fail
 */
int xaui_lpbk_test (int lbmode)
{
    int pp, pcnt, port;
    int rc;
    int *xaui_lpbk_ports;
    char *lbname;
    int evb_xaui_list;
    
    if (lbmode == LOOP_XAUI_BP) {
        /* XAUI backplane loopback test */
        evb_xaui_list = XAUI0;
    } else {
        evb_xaui_list = XAUI1;
    }

    if (lbmode == LOOP_INT) {
        lbname = "internal";
    } else if (lbmode == LOOP_EXT) {
        lbname = "external";
    } else {
        lbname = "backplane";
    }

    xaui_lpbk_ports = &evb_xaui_list;
    pcnt = sizeof(evb_xaui_list) / sizeof(int);

    for (pp=0; pp < pcnt; pp++) {
        port = xaui_lpbk_ports[pp];
        rc = xaui_port_lpbk(port, lbmode);

        if (rc == FAILED) {
            cterr('f',0,"XAUI port %d %s loopback failed\n", port, lbname);
            return(FAILED);
        } else {
            prpass(testpass, "XAUI port %d %s loopback passed", port, lbname);
            return(PASSED);
        }
    }

    return (rc);
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

/**********************************************************************
 *
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

void config_xaui (void)
{
    system("ifconfig xaui1 19.19.19.19 netmask 255.255.255.0 promisc -arp -allmulti -multicast up");
}

void config_bp_xaui(void)
{
    char sm_ip[16];
    char *sm_slot;
    char sm_cmd[64];

    /* Assign IP Address based on slot number */
    sm_slot = getenv("sm_slot");
    if (sm_slot == NULL) {
        printf("%s: NULL POINTER\n", sm_slot);
        system("ifconfig xaui0 192.123.123.100 netmask 255.255.255.0 up");
        return;
    }

    /* 192.123.123.101 for slot 1, 192.123.123.102 for slot 2 */
    sprintf(sm_ip, "%s.%d", WOODLAWN_DIAG_IP_ADDR_SUBNET,
            WOODLAWN_DIAG_IP_ADDR_BASE + atoi(sm_slot));
    sprintf(sm_cmd, "ifconfig xaui0 %s netmask 255.255.255.0 up", sm_ip);
    system(sm_cmd);
}

/* xaui port ping test
 */
int xaui_ping_test(void)
{
    char cmdbuf[128], buf[128], dum_char[32];
    uint  pktcnt, deadline;
    char *interval_str, *result_file;
    FILE *fp;
    int tx_cnt, rx_cnt;
    int rv = FAILED;

    config_xaui();

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

    rv = PASSED;

 xaui_ping_test_exit:

    if (rv == PASSED) {
        printf("ping passed\n");
    }
    else {
        cterr('f',0,"ping failed\n");
    }

    return(rv);
}


void
display_xaui_port_status(void)
{
    int interface = CVMX_XAUI_INF_ID;
    int ipd_port = 0;
    cvmx_gmxx_tx_xaui_ctl_t gmxx_tx_xaui_ctl;
    cvmx_gmxx_rx_xaui_ctl_t gmxx_rx_xaui_ctl;
    cvmx_pcsxx_status1_reg_t pcsxx_status1_reg;
    cvmx_pcsxx_control1_reg_t pcsxx_control1_reg;

    interface = getdec_answer("Enter QLM number (2/3)", CVMX_BP_XAUI_INF_ID,
                               CVMX_GMX2_INF_ID, CVMX_GMX3_INF_ID);
    fflush(stdout);

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

void
dump_xaui_gmx_regs(void)
{
    int interface = CVMX_XAUI_INF_ID;
    int ipd_port = 0;
    ulong reg;

    interface = getdec_answer("Enter QLM number (2/3)", CVMX_BP_XAUI_INF_ID,
                               CVMX_GMX2_INF_ID, CVMX_GMX3_INF_ID);
    fflush(stdout);

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

void
dump_xaui_pcs_regs(void)
{
    int interface = CVMX_XAUI_INF_ID;
    ulong reg;

    interface = getdec_answer("Enter QLM number (2/3)", CVMX_BP_XAUI_INF_ID,
                               CVMX_GMX2_INF_ID, CVMX_GMX3_INF_ID);
    fflush(stdout);

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

#ifdef PFIX_LATER
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
	return (FAILED);
    }

    /* Wait for PCS to be aligned */
    repeat = 100;
    do {
        msleep(100);
	pcsxx_10gbx_status.u64 = cvmx_read_csr (CVMX_PCSXX_10GBX_STATUS_REG(interface));
    } while ((repeat-- > 0) && (pcsxx_10gbx_status.s.alignd != 1));
    if (repeat <= 0) {
        printf("%s() Xaui PCS 10GBX not aligned\n", __FUNCTION__);
	return (FAILED);
    }

    /* Wait for RX to be ready */
    repeat = 100;
    do {
        msleep(10);
	gmxXauiRxCtl.u64 = cvmx_read_csr (CVMX_GMXX_RX_XAUI_CTL(interface));
    } while ((repeat-- > 0) && (gmxXauiRxCtl.s.status != 0));
    if (repeat <= 0) {
        printf("%s() Xaui GMX RX link not ok\n", __FUNCTION__);
	return (FAILED);
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
	return (FAILED);
    }

    /* (8) Enable packet reception */
    xauiMiscCtl.s.gmxeno = 0;
    cvmx_write_csr (CVMX_PCSXX_MISC_CTL_REG(interface), xauiMiscCtl.u64);

    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(0, interface));
    gmx_cfg.s.en = 1;
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(0, interface), gmx_cfg.u64);

    return (PASSED);
}
#endif //PFIX_LATER

/*-------------------------------------------------
 * $Log: platform_xaui.c,v $
 * Revision 1.2  2013/10/08 08:48:31  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:11  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/06/17 11:12:06  leschen
 * Add fflush
 *
 * Revision 1.1.2.1  2013/04/24 10:37:25  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.5  2013/04/01 04:09:55  kuangik
 * Assign XAUI BP IP address based on slot number
 *
 * Revision 1.12  2013/03/07 12:30:22  leslie
 * Use raw socket instead of using ip socket for 10G loopback test.
 *
 * Revision 1.11  2013/03/01 13:51:56  kuangik
 * Update Loopback Test, SFP Present, and SFP EEPROM display
 *
 * Revision 1.10  2013/02/18 09:08:41  leslie
 * Fix and clean up code
 *
 * Revision 1.9  2013/01/18 06:49:20  leslie
 * Fix and clean up code.
 *
 * Revision 1.8  2012/12/11 01:35:26  leslie
 * Fix XAUI loopback code for backplane XAUI loopback test.
 *
 * Revision 1.7  2012/11/08 02:50:45  kody
 * Add enable QLM2 XAUI ext-loopback for O2 backplane XAUI loopback test.
 *
 * Revision 1.6  2012/10/03 06:07:03  kody
 * Show the test mode log in test function.
 *
 * Revision 1.5  2012/09/21 11:54:46  kody
 * Woodlawn use xaui1 to do the lpbk test.
 *
 * Revision 1.4  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.2  2012/05/18 10:26:20  kody
 * Add enable xaui0 api
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.2  2011/11/03 21:42:41  ptong
 * Support xaui loopback test
 *
 * Revision 1.1.2.1  2011/04/05 19:59:39  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
