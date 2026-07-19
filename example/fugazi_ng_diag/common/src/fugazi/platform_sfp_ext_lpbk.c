/* $Id: platform_sfp_ext_lpbk.c,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_sfp_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_sfp_ext_lpbk.c
 * SFP external loopback test.
 *
 * June 2016 Mecca Ho
 * Jan 2019, Letsai modified for Fugazi.
 *
 * Copyright (c) 2017-2021 by Cisco Systems, Inc.
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
#include "proto.h"

#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "diag_bcm54194_api.h"
#include "diag_bnxt.h"
#include "diag_miura_reg.h"
#include "diag_bcm_lib.h"
#include "diag_bcm57412_utils.h"

/* QLM 5 eth number from qLM SFP port 0 ~ 1*/
int eth_qlm5_sfp_list[] = {ETH4, ETH5, ETH6, ETH7, ETH8, ETH9, ETH10, ETH11};

int eth_mapping_sfp_num[] = {SFP_PORT0, SFP_PORT1};

/* Fiber only test 1000Mpbs, need to replace SFP module when testing 100Mbps */
int sfp_speed_list[] = {SPD_1000MBPS};

boolean fugazi_is_1g_phy_linkup(int);
extern void show_eth_counter (char *type, int port);
extern int devad;


/*------------------------------------------------------------------
 *
 * Function: switch_fiber
 *   ensure the copper is truned off before fiber test.
 *   turn the copper after the test is finished.
 *
 * Input: phy_num - PHY number
 *        port - port number
 *        onoff - turn on/off advertise reg.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int switch_fiber(int phy_num, int port, boolean onoff)
{
	int  rc = PASSED;

    if (onoff) {
        /*enable fiber, disable copper */
        if ((rc = bcm54194_sig_pwr_ctrl(phy_num, port, DISABLE_SIG, BCM54194_COPPER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: disable copper failed", port);
            return(FAILED);
        }        
        if ((rc = bcm54194_sig_pwr_ctrl(phy_num, port, ENABLE_SIG, BCM54194_FIBER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: enable fiber failed", port);
            return(FAILED);
        }
    } else {
        /*enable copper, disable fiber */
        if ((rc = bcm54194_sig_pwr_ctrl(phy_num, port, DISABLE_SIG, BCM54194_FIBER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: disable fiber failed", port);
            return(FAILED);
        }
        if ((rc = bcm54194_sig_pwr_ctrl(phy_num, port, ENABLE_SIG, BCM54194_COPPER_INTF)) != PASSED) {
            cterr('f',0, "GE port %d: enable copper failed", port);
            return (FAILED);
        }
    }

   return (rc);
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
    int phy_addr = ge_port_mapping_phy_addr_down[eth_num];
    int phy_num  = (int) (eth_num/2);
   
    sprintf(pname,"%s%d", type, eth_num);
    
    /* Stop linux driver polling link status when configuring PHY */
    bcm54194_suspend_lnx_link_polling (type, eth_num, TRUE);

    
    rc = bcm54194_config_loopback(phy_num, phy_addr, speed, BCM54194_FIBER_INTF, GE_PHY_SFP_EXT_LPBK, enable);
    if (rc != PASSED) {
        printf("GE PHY config loopback failed.\n");
        return rc;
    }
    
    if (enable) {
        /* Check GE PHY SERDES link status */
        if (!bcm54194_is_linkup(phy_num, phy_addr, BCM54194_FIBER_INTF)) {
            printf("%s(): GE PHY SGMII link up time out\n", __FUNCTION__);
            rc = FAILED;
            return rc;
        } else {
            bcm54194_suspend_lnx_link_polling (type, eth_num, FALSE);
            return rc;
        }
    }
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

    /* setup loopback information */
    rc = sfp_set_phy_lpbk(SEL_PORT_ETH, eth_num, speed, TRUE);
    if (rc == FAILED) {
        printf("sfp_set_phy_lpbk failed, port: %d\n", eth_num);
    return (rc);
    }
    if (fugazi_sfp_present(eth_num)) {
        cterr('f', 0, "Fugazi port %d sfp not present", eth_num);
        return (FAILED);
    }

    /* Check the link status */
    if (!fugazi_is_1g_phy_linkup(eth_num)){
        printf(" GE PHY link up time out, port: %d\n", eth_num);
        return (FAILED);
    } 
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = fugazi_set_packet(SEL_PORT_ETH, eth_num, speed);
    if (rc == FAILED) {
        printf("fugazi_set_packet failed, port: %d\n", eth_num);
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
    int rc = 0,  result = PASSED;
    int port_cnt, port_curr;
    int speed;
    int port;
    int *eth_port_list = eth_qlm5_sfp_list;

    if (check_ge_int_lpbk_flag() || !check_ext_lpbk_flag()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return (PASSED); /* external loopback is not set, skipped. */
    }

    port_cnt = sizeof(eth_qlm5_sfp_list) / sizeof(int);
    
    /* select port */
    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        /* get test environment variable */
        port = eth_port_list[port_curr];
        /* select speed*/
            speed = SPD_1000MBPS;
            testname("BCM54194 Phy External SFP loopback");
            prpass(testpass, "Test GE eth%d speed-%d ", port,
                              speed);
            printf("\n\nAt the beginning of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            show_eth_counter(SEL_PORT_ETH, port);
            /* setup loopback information */          
            rc = ge_phy_sfp_lpbk_test(port, speed);

            printf("\n\nIn the end of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            msleep(1000);
            show_eth_counter(SEL_PORT_ETH, port);
            if (rc != PASSED) {
                /* prevent keep "trying speed..." message */
                cterr('f',0,"Setup sfp port - %d loopback information failed", 
                      port);
                result = FAILED;
            }
    } /* port_curr */
    
    return (result);
}

/*------------------------------------------------------------------
 *
 * Function: sfp_ext_lpbk_test_util
 * this function is used to support utiles 
 *
 * Input:  port_curr - port number
 *         speed - 100M/1000M bps
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sfp_ext_lpbk_test_util(int port_curr, int speed)
{
    int rc = PASSED;
    int phy_num =FUGAZI_MAC_1G_PHY_0;

    rc = ge_phy_sfp_lpbk_test(port_curr, speed);
    if (rc != PASSED) {
        cterr('f',0,"Setup sfp port - %d loopback information failed", port_curr);
    }
    /* ensure fiber is off before leaving test.*/
    switch_fiber(phy_num, port_curr, DISABLE_SIG);
    return (rc);
}


/*------------------------------------------------------------------
 *
 * Function: fugazi_is_1g_phy_linkup
 * this function is used to check if link is up at both Network, and System side
 * consistency for 1 second to declare link up.
 *
 * Input:  eth_num - eth number in 1G PHY (4,...,11)
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
boolean fugazi_is_1g_phy_linkup (int eth_num)
{
    uint16_t rdb_rdval;
    uint16_t ieee_rdval, ieee_offset;
    int phy_num = (int) (eth_num/2);
    int phy_addr_down = ge_port_mapping_phy_addr_down[eth_num];
    int phy_addr_up = ge_port_mapping_phy_addr_up[eth_num];
    int rdb_offset;
    int retry = 0, max_try = 200 , link_up_count = 0;
    int max_link_up_consistency = 10;

    while (retry++ < max_try) { /* wait for 1 seconds */
        printf(".");
        fflush(stdout);
        /* Network Side */
        rdb_offset = BCM54194_MODE_CTRL_REG; /* 0x21 */
        bcm54194_rdb_read(phy_num, phy_addr_down, rdb_offset, &rdb_rdval);
        bcm54194_rdb_read(phy_num, phy_addr_down, rdb_offset, &rdb_rdval);

        /* System Side */
        ieee_offset = BCM54194_STAT_REG; /* 0x01 */
        fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr_up, devad, ieee_offset, &ieee_rdval);
        fugazi_bnxt_mdio_read(&fugazi_struct->bnxt[phy_num], phy_addr_up, devad, ieee_offset, &ieee_rdval);

        if ((rdb_rdval & BCM54194_SERDES_LINK_UP) && (ieee_rdval & BCM54194_LINK_STAT_BIT)) {
            link_up_count++; 
        } else {
            link_up_count = 0;
        }

        /* All up, return now */
        if (link_up_count >= max_link_up_consistency) {
            printf("\n!!! Link_up !!!\n");
            break;
        } else { /* Delay and try again */
            fugazi_mdelay(10);
        }
    }
    printf("\n");

    /* Link up : only if it is consistent up for 100ms */
    if (link_up_count < max_link_up_consistency) {
        printf("\n%s: eth%d link down (link_up_count=%d, retry=%d)\n",__FUNCTION__, eth_num, link_up_count, retry);
        if (!(rdb_rdval & BCM54194_SERDES_LINK_UP)){  /* check if 0x40 */
            printf("\nNo link at network side!");
        }else if (!(ieee_rdval & BCM54194_LINK_STAT_BIT)){  /* check if 0x04 */
            printf("\nNo link at system side!");
        }else{
            printf("\nNo link at both system side and network side!");
        }
        return (FALSE);
    }
    
    return (TRUE);

}





/*-------------------------------------------------
 * $Log: platform_sfp_ext_lpbk.c,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.4  2021/05/21 18:48:20  pdoong
 * Let BCM54195 external loopback test complete on every port even there is a failure happen
 *
 * Revision 1.1.8.3  2021/04/29 01:44:02  pdoong
 * Add checking if PHY Network side link is up in 'SyncE Recovered Clock Test'
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.20  2020/08/24 00:05:20  pdoong
 * Clean code for ER.
 *
 * Revision 1.1.6.19  2020/08/06 04:28:57  pdoong
 * clean code for BCM54194 1G PHY
 *
 * Revision 1.1.6.18  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.17  2020/01/15 07:30:09  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.16  2019/11/14 09:29:50  iachang
 * Add SFP present check before GE External Loopback test.
 *
 * Revision 1.1.6.15  2019/10/16 06:12:31  letsai
 * Modify file name
 *
 * Revision 1.1.6.14  2019/09/24 02:42:54  letsai
 * Enhance error message of check link status function.
 *
 * Revision 1.1.6.13  2019/09/23 07:38:25  letsai
 * Add packet counter utility of BCM54194 phy
 *
 * Revision 1.1.6.12  2019/08/28 05:58:14  letsai
 * Enhance check link status function.
 *
 * Revision 1.1.6.11  2019/08/02 07:16:50  letsai
 * 1.Add debug messgage. 2.Fix initial process for BCM 54194 phy
 *
 * Revision 1.1.6.10  2019/07/15 08:22:12  letsai
 * Add check link status function on network side for BCM 54194 phy
 *
 * Revision 1.1.6.9  2019/04/18 23:11:58  letsai
 * Add loopback mode config uyility and clean up code.
 *
 * Revision 1.1.6.8  2019/04/18 01:21:30  letsai
 * 1. Clean up code
 * 2. Modify 1G phy address mapping
 * 3. Modify print message of MCU FW opgrade
 *
 * Revision 1.1.6.7  2019/04/10 21:26:59  letsai
 * 1. Support BCM54194 PHY SGMII Internal Loopback test.
 * 2. Return FAILED when M.2 module not present.
 * 3. Clean up code.
 *
 * Revision 1.1.6.6  2019/04/10 16:29:30  letsai
 * 1. Fix ethernet mapping.
 * 2. Support all BCM54194 phy in utilities.
 * 3. Remove unused functions.
 *
 * Revision 1.1.6.5  2019/04/09 16:10:40  letsai
 * 1. Support all BCM54194 PHY (0~3) Register Test.
 * 2. Let utilities can dump each phy registers.
 * 3. Check link status for each phy and each port(upstream and downstream).
 *
 * Revision 1.1.6.4  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.3  2019/03/25 18:37:36  letsai
 * Modified eth and port number
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

