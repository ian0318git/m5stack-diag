/* $Id: platform_sfp_ext_lpbk.c,v 1.2 2018/05/18 09:24:57 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_sfp_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_sfp_ext_lpbk.c
 * SFP external loopback test.
 *
 * June 2016 Mecca Ho
 * Copyright (c) 2017-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>  /* for bzero*/
#include<string.h>
#include<sys/socket.h>
#include<features.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<linux/ethtool.h> /*struct ethtool */
#include<linux/sockios.h> /* SIOCETHTOOL */
#include <sys/types.h> /* getpid */
#include <unistd.h>  /* getpid */
#include <netinet/in.h>  /* for including the linux_eth.h */
#include <stdint.h>

#include<errno.h>
#include<sys/ioctl.h>
#include<net/if.h>

#include<pthread.h>
#include "defs.h"
#include "types.h"
#include "common.h"
#include "error.h"
#include "monitor.h"

#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "bcm54194_api.h"

/* QLM 5 eth number from qLM SFP port 0 ~ 1*/
int eth_qlm5_sfp_list[] = {ETH3, ETH4};

int eth_mapping_sfp_num[] = {SFP_PORT0, SFP_PORT1};

/* Fiber only test 1000Mpbs, need to replace SFP module when testing 100Mbps */
int sfp_speed_list[] = {SPD_1000MBPS};

extern void show_eth_counter (char *type, int port);

/*------------------------------------------------------------------
 *
 * Function: adv_full_duplex
 *   make sure the test is on full duplex.
 *   we have encounter the condition that fiber is not in full duplex 
 *   mode after sgmii tests. Thus, we add this step to turn on full
 *   duplex.
 *
 * Input:  onoff - turn on/off advertise reg.
 *         port - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int adv_full_duplex(boolean onoff, int port)
{
    sleep(ETH_DRIVER_DELAY);

    return (PASSED);
}



/*------------------------------------------------------------------
 *
 * Function: switch_fiber
 *   ensure the copper is truned off before fiber test.
 *   turn the copper after the test is finished.
 *
 * Input:  type - port type
 *         onoff - turn on/off advertise reg.
 *         port - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int switch_fiber(int port, boolean onoff)
{
    int rc = PASSED;

    if (onoff) {
        /*enable fiber, disable copper */
        if ((rc = bcm54194_sig_pwr_ctrl(port, DISABLE_SIG, BCM54194_COPPER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: disable copper failed", port);
            return(FAILED);
        }        
        if ((rc = bcm54194_sig_pwr_ctrl(port, ENABLE_SIG, BCM54194_FIBER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: enable fiber failed", port);
            return(FAILED);
        }
    } else {
        /*enable copper, disable fiber */
        if ((rc = bcm54194_sig_pwr_ctrl(port, DISABLE_SIG, BCM54194_FIBER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: disable fiber failed", port);
            return(FAILED);
        }
        if ((rc = bcm54194_sig_pwr_ctrl(port, ENABLE_SIG, BCM54194_COPPER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: enable copper failed", port);
            return (FAILED);
        }
    }

   return (PASSED);
} 

/*------------------------------------------------------------------
 *
 * Function: sfp_set_phy_lpbk
 *  initial and setup loopback type on sgmii, both internal lpbk and 
 *  external lpbk can use this function. The initial function is based 
 *  on ethtool, because we can not let port link up via set the PHY
 *  directly.
 *
 * Input:  type - port type
 *         port - eth port number
 *         speed - 100M/1000M bps
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_set_phy_lpbk(char *type, int eth_num, int speed, boolean enable)
{
    char pname[10];
    int rc = 0;
    char cmd_str[32];
    int ge_port = eth_mapping_phy_addr[eth_num];
   
    sprintf(pname,"%s%d", type, eth_num);
    
    /* turn on fiber and off the copper */
    switch_fiber(ge_port, enable);

    /* Need to select Copper register space before bring eth up */
    bcm54194_reg_1000x_en(ge_port, FALSE);
    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);
    sleep(1);
    
    /* Stop linux driver polling link status when configuring PHY */
    bcm54194_suspend_lnx_link_polling (type, eth_num, TRUE);

    rc = bcm54194_config_loopback(ge_port, speed, BCM54194_FIBER_INTF, GE_PHY_SFP_EXT_LPBK, enable);
    if (rc != PASSED) {
        printf("GE PHY config loopback failed.\n");
        goto ge_phy_config_lpbk_exit;
    }

    if (enable) {
        /* Check GE PHY SERDES link status */
        if (!bcm54194_is_linkup(ge_port, BCM54194_FIBER_INTF)) {
            printf("%s(): GE PHY SGMII link up time out\n", __FUNCTION__);
            rc = FAILED;
            goto ge_phy_config_lpbk_exit;
        } else {
            bcm54194_suspend_lnx_link_polling (type, eth_num, FALSE);
            return rc;
        }
    }

ge_phy_config_lpbk_exit:
    bcm54194_suspend_lnx_link_polling (type, eth_num, FALSE);
    sleep(ETH_DRIVER_DELAY);
    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: ge_phy_sfp_lpbk_test
 *  This is the entry point for bridge PHY internal loopback test.
 *
 * Input:  port - port number
 *         speed - test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ge_phy_sfp_lpbk_test(int eth_num, int speed)
{
    int rc = PASSED;
    int sgmii_num = eth_mapping_sgmii_num[eth_num];

    /* setup loopback information */
    rc = sfp_set_phy_lpbk(SEL_PORT_ETH, eth_num, speed, TRUE);
    if (rc == FAILED) {
        printf("sfp_set_phy_lpbk failed, port: %d\n", eth_num);
        goto ge_phy_sfp_lpbk_exit;
    }

    /* ensure the cavium is not in loopback mode. */
    set_sgmii_int_lpbk(sgmii_num, FALSE);

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = neptune_set_packet(SEL_PORT_ETH, eth_num, speed);
    if (rc == FAILED) {
        printf("neptune_set_packet failed, port: %d\n", eth_num);
        goto ge_phy_sfp_lpbk_exit;
    }

ge_phy_sfp_lpbk_exit:
    /* restore the setting */
    if ((sfp_set_phy_lpbk(SEL_PORT_ETH, eth_num, speed, FALSE)) != PASSED) {
       printf("setup GE PHY internal loopback failed\n");
       return (rc);
    }

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: sfp_phy_ext_lpbk_test
 * This is the entry point for SFP external loopback test only.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_phy_ext_lpbk_test (void)
{
    int rc = 0;
    int port_cnt, port_curr;
    int speed, speed_curr, speed_cnt;
    int port;
    int *eth_mapping_front_panel_port = eth_mapping_ge_num;
    int *eth_port_list = eth_qlm5_sfp_list;
    int *eth_speed_list = sfp_speed_list;

    if (check_ge_int_lpbk_flag() || !check_ext_lpbk_flag()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return (PASSED); /* external loopback is not set, skipped. */
    }

    port_cnt = sizeof(eth_qlm5_sfp_list) / sizeof(int);
    speed_cnt = sizeof(sfp_speed_list) / sizeof(int);
    
    /* select port */
    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        /* get test environment variable */
        port = eth_port_list[port_curr];
#if 0
        /* check SFP is available. if not, return failed */
        if (is_sfp_present(port) == FALSE) {
            cterr('f', 0, "SFP module %d is not detected", port);
            /* ensure fiber is off before leaving test.*/
            switch_fiber(port, DISABLE_SIG);
            /* prevent keep "trying speed..." message */
            neptune_err_clean_up(port);
            return (FAILED);
        } else {
            prpass(testpass, "SFP module %d is detected", port);
        } 
#endif
        /* select speed*/
        for (speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            speed = eth_speed_list[speed_curr];
            testname("External SFP loopback");
            prpass(testpass, "Test GE%d speed-%d ", eth_mapping_front_panel_port[port],
                              speed);
            printf("\n\nAt the beginning of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            show_eth_counter(SEL_PORT_ETH, port);
            /* setup loopback information */          
            rc = ge_phy_sfp_lpbk_test(port, speed);
            printf("\n\nIn the end of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            show_eth_counter(SEL_PORT_ETH, port);
            if (rc != PASSED) {
                /* ensure fiber is off before leaving test.*/
                switch_fiber(port, DISABLE_SIG);
                /* prevent keep "trying speed..." message */
                neptune_err_clean_up(port);
                cterr('f',0,"Setup sfp port - %d loopback information failed", 
                      port);
                break; /* skip to next port */
            }

        } /* speed_curr */
    
        /* ensure fiber is off before leaving test.*/
        switch_fiber(port, DISABLE_SIG);
    } /* port_curr */
    
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: sfp_ext_lpbk_test_util
 * this function is used to support utiles 
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_ext_lpbk_test_util(int port_curr, int speed)
{
    int rc = 0; 

    rc = ge_phy_sfp_lpbk_test(port_curr, speed);
    if (rc != PASSED) {
        /* prevent keep "trying speed..." message */
        neptune_err_clean_up(port_curr);
        cterr('f',0,"Setup sfp port - %d loopback information failed", port_curr);
    }
    /* ensure fiber is off before leaving test.*/
    switch_fiber(port_curr, DISABLE_SIG);
    return (rc);
}

/*-------------------------------------------------
 * $Log: platform_sfp_ext_lpbk.c,v $
 * Revision 1.2  2018/05/18 09:24:57  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.14  2018/05/15 04:16:06  meho
 * Fixed show packet counter for 10G PHY loopback test.
 *
 * Revision 1.1.2.13  2017/04/18 08:17:16  meho
 * Corrected the SFP speed list.
 *
 * Revision 1.1.2.12  2017/04/10 05:27:25  meho
 * Integrated BCM82752/82757 API.
 *
 * Revision 1.1.2.11  2017/01/09 07:09:38  meho
 * No need move to test higher speed once it failed at lower speed.
 *
 * Revision 1.1.2.10  2016/12/28 09:07:23  meho
 * Changed the test name of loopback test.
 *
 * Revision 1.1.2.9  2016/12/27 02:01:42  meho
 * Added ge-Int loopback flag to control Cavium GE int/ext loopback test.
 *
 * Revision 1.1.2.8  2016/12/19 07:49:45  meho
 * Added VERBOSE flag in sending packet for loopback test.
 *
 * Revision 1.1.2.7  2016/11/28 03:43:55  meho
 * 1. Fixed GE phy Mac/Int/Ext loopback test bugs.
 * 2. Added 10G FW download.
 *
 * Revision 1.1.2.6  2016/08/18 06:57:49  meho
 * Code clean up.
 *
 * Revision 1.1.2.5  2016/08/12 10:12:19  meho
 * Clean up code.
 *
 * Revision 1.1.2.4  2016/07/21 07:27:37  meho
 * Added 100Mbps speed in GE PHY SFP loopback test.
 *
 * Revision 1.1.2.3  2016/07/20 08:09:50  meho
 * 1. Updated BCM82752 firmware array.
 * 2. Added 10G PHY loopback debug utilities.
 *
 * Revision 1.1.2.2  2016/07/20 01:44:59  meho
 * Added GE PHY loopback debug utilities.
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
