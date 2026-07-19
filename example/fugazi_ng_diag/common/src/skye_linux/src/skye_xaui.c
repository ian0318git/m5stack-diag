/* $Id: skye_xaui.c,v 1.2 2015/05/25 03:59:17 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_xaui.c,v $
 *------------------------------------------------------------------
 * Module specific code for Linux base XAUI port loopback test
 * 
 * Porting from woodlawn by steja
 * 
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
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
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "proto.h"
#include "queryflags.h"
#include "diag_common_lib.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "skye_eth.h"
#include "skye_xaui.h"
#include "skye_ext_lpbk.h"

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
    {XAUI1,  NAME_XAUI,  TILERA_XAUI1_IP_ADDR,  TILERA_XAUI1_DUMMY_IP_ADDR,  TILERA_NETMASK,
     0,  0,  pktdata, sizeof(pktdata)/sizeof(pktdata_info_t) },
};

/* Platform CPU xaui ports configurations for diagnostics
 */
static eth_lpbk_info_t bp_xaui_lpbk_tbl[] = {
    {XAUI0,  NAME_XAUI,  TILERA_XAUI0_IP_ADDR,  TILERA_XAUI0_DUMMY_IP_ADDR,  TILERA_NETMASK,
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
int
enable_xaui1_interface (void)
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
int
xaui_set_packet (int port, int speed) 
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
xaui_port_lpbk (int port, int lbmode)
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

    if (lbmode == LOOP_EXT) {
        /* internal and external xaui loopback test */
        if (xaui_set_packet(XAUI1, SPD_10000MBPS) == FAILED) {
            cterr('f', 0, "%s(): xaui_set_packet failed", __FUNCTION__);
            return (FAILED);
        }
    } else {
        /* backplane xaui loopback test */
    	if (xaui_set_packet(2, SPD_10000MBPS) == FAILED) {
            cterr('f', 0, "%s(): xaui_set_packet failed", __FUNCTION__);
            return (FAILED);
        }
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
int
xaui_lpbk_test (int lbmode)
{
    int pp, pcnt, port;
    int rc = 0;
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
xaui_internal_lpbk_test (void)
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
xaui_external_lpbk_test (void)
{
    return(xaui_lpbk_test(LOOP_EXT));
}

/**********************************************************************
 *
 * Function: config_xaui
 *
 * Description: XAUI config
 *
 * Input: void
 *
 * Return: none
 */
void
config_xaui (void)
{
    system("ifconfig xaui1 19.19.19.19 netmask 255.255.255.0 promisc -arp -allmulti -multicast up");
}

/**********************************************************************
 *
 * Function: config_bp_xaui
 *
 * Description: XAUI backplane config
 *
 * Input: void
 *
 * Return: none
 */
void
config_bp_xaui (void)
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
    sprintf(sm_ip, "%s.%d", SHRINKRAY_DIAG_IP_ADDR_SUBNET,
            SHRINKRAY_DIAG_IP_ADDR_BASE + atoi(sm_slot));
    sprintf(sm_cmd, "ifconfig xaui0 %s netmask 255.255.255.0 up", sm_ip);
    system(sm_cmd);
}

/**********************************************************************
 *
 * Function: xaui_ping_test
 *
 * Description: XAUI ping test
 *
 * Input: void
 *
 * Return: none
 */
int
xaui_ping_test (void)
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

/*-------------------------------------------------
 * $Log: skye_xaui.c,v $
 * Revision 1.2  2015/05/25 03:59:17  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/04/30 08:33:54  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:37  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:56  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * skye_xaui.c:
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.6  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.4.5  2013/11/05 09:17:54  steja
 * 1. Fix the MDIO not stable issue
 * 2. debug tlk log
 *
 * Revision 1.1.4.4  2013/10/05 06:20:24  steja
 * Update for debug
 *
 * Revision 1.1.4.3  2013/09/27 07:25:14  steja
 * update code for bringup
 *
 * Revision 1.1.4.2  2013/09/13 07:00:10  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 *-------------------------------------------------
 */
