/* $Id: platform_eth.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/platform_eth.c,v $
 *------------------------------------------------------------------
 *
 * Platform specific code for Linux base ethernet port loopback test
 * 
 * May 2016, Xiaoying Zhang
 *
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
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  
#include "bcm54194_api.h"
#include "bcm82752_api.h"
#include "bcm82752_reg_def.h"
#include "bcm54194_test.h"
#include "bcm82752_test.h"
//#include "platform_ext_lpbk.h"

#define F_GRP	     (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E	     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL	     (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/*****************************************************************************
 ***************************  Globals Variables   ****************************
 *****************************************************************************/
/* PHY addr mapping of front panel port */
int eth_mapping_phy_addr[] = {0x0, 0x1, 0, 0xF, 0x10, 0x11, 0x12};
int ge_port_mapping_phy_addr[] = {0xF, 0x10, 0x11, 0x12};
int te_port_mapping_phy_addr[] = {0x0, 0x1};
int eth_mapping_cvmx_bgx_num[] = {CVMX_BGX0_INF_ID, CVMX_BGX0_INF_ID, CVMX_BGX1_INF_ID,
                                  CVMX_BGX2_INF_ID, CVMX_BGX2_INF_ID, CVMX_BGX2_INF_ID,
                                  CVMX_BGX2_INF_ID};
//int eth_mapping_cvmx_ipd_port[] = {2048, 2080, 2304, 2560, 2576, 2592, 2608};

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t eth_port_tests_submenu_table[] = {
    /* ------------ SGMII copper and SFP loopback tests -------------*/
    {"GE PHY BCM541xx Test", (type_t(*)()) ge_phy_bcm541xx_test,   0,
     F_ALL, (type_t(*)())0, 0, (type_t(*)())ge_phy_bcm541xx_test,   1},
    /* ------------ SGMII copper and SFP loopback tests -------------*/
    {"10G PHY BCM8275x Test", (type_t(*)())ten_g_bcm8275x_test,   0,
     F_ALL, (type_t(*)())0, 0, (type_t(*)())ten_g_bcm8275x_test,   1},
};


#define ETH_PORT_TESTS_SUBMENU_TABLE_SIZE (sizeof(eth_port_tests_submenu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t eth_port_tests_primary_items[ETH_PORT_TESTS_SUBMENU_TABLE_SIZE +
					  MAX_BASE_ITEMS];
static mitem_t eth_port_tests_secondary_items[ETH_PORT_TESTS_SUBMENU_TABLE_SIZE +
					    MAX_BASE_ITEMS];

menuinfo_t eth_port_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    eth_port_tests_primary_items,
};
menuinfo_t *eth_port_submenup = &eth_port_subtest_menu;

/* disable non support items */
int is_item_available()
{
    return 0;
}

/*
 * Function: get_sfp_config
 *
 * Description: get sfp configure register from FPGA,
 *              including status bits.
 *
 * Input: sfp_num - SFP number 0 to 3
 *
 * Return: result - sfp configuration register.
 */
unsigned int get_sfp_config (int sfp_num)
{
    unsigned int result;
    unsigned long addr = get_platform_sfp_stat_ctrl_addr();
    sfp_stat_ctrl_t *sfp_stat_ctrl = (sfp_stat_ctrl_t *)addr;

    switch(sfp_num){
        case SFP0:
            result = sfp_stat_ctrl->sfp0_conf;
            break;
        case SFP1:
            result = sfp_stat_ctrl->sfp1_conf;
            break;
        case SFP2: /* it is SFP+ 0 on neptune */
            result = sfp_stat_ctrl->sfp_p0_conf;
            break;
        case SFP3: /* it is SFP+ 1 on neptune */
            result = sfp_stat_ctrl->sfp_p1_conf;
            break;
        default:
            printf("error: not support this SFP port num %d\n", sfp_num);
            break;
    }

    return (result);
}

/*
 * Function: is_sfp_present
 *
 * Description: Check is SFP is plugged in
 *
 * Input: sfp_num - SFP number 0 to 3
 *
 * Return: true/false
 */
unsigned int is_sfp_present(int sfp_num)
{
    unsigned int result;
    result = get_sfp_config(sfp_num);

    if (result & SFP_PRESENT) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*
 * Function: is_sfp_tx_fault
 *
 * Description: Check is SFP has tx fault
 *
 * Input: sfp_num - SFP number 0 to 3
 *
 * Return: true/false
 */
unsigned int is_sfp_tx_fault(int sfp_num)
{
    unsigned int result;
    result = get_sfp_config(sfp_num);

    if (result & SFP_TX_FAULT) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*------------------------------------------------------------------
 *
 * Function: eth_port_test_main
 *	This is the entry point for the ethernet port main
 *	test.
 *
 * Input:  exec_tests = 0 show submenu, !=0 perform all tests
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int eth_port_test_main (int exec_tests)
{
    build_primary_submenu(eth_port_tests_submenu_table,
			  ETH_PORT_TESTS_SUBMENU_TABLE_SIZE,
                          "Ethernet port", &eth_port_submenup);
    build_secondary_submenu(eth_port_tests_submenu_table,
                            ETH_PORT_TESTS_SUBMENU_TABLE_SIZE,
                            eth_port_tests_secondary_items);

    if (exec_tests) {
        exec_doall_menu_items(eth_port_submenup);
    } else {
        menu(eth_port_submenup, eth_port_tests_secondary_items, '\0' );
    }
 
    return(PASSED);
}

/*------------------------------------------------------------------
 * Function: check_ext_lpbk_flag
 *
 * Check if external loopback flag is set in diag
 *
 * Input:  NONE
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int check_ext_lpbk_flag(void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

/*------------------------------------------------------------------
 * Function: check_ge_int_lpbk_flag
 *
 * Check if ge internal loopback flag is set in diag
 * Please turn on this flag while no external stub plug in.
 *
 * Input:  NONE
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int check_ge_int_lpbk_flag(void)
{
    /* according to menu_show_dflags(), D_GE_INT_LOOPBACK is inverse flag */ 
    if (diagflag_xram & D_GE_INT_LOOPBACK) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*-------------------------------------------------
$Log: platform_eth.c,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.20  2018/05/03 10:06:13  alpeng
fix bug for sfp i2c scan test; if sfp module is not present, print warning and skip test

Revision 1.1.2.19  2017/04/10 05:27:24  meho
Integrated BCM82752/82757 API.

Revision 1.1.2.18  2016/12/27 08:55:48  meho
Fixed show error count bug.

Revision 1.1.2.17  2016/12/27 02:01:42  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.1.2.16  2016/12/15 02:00:18  meho
Added check external flag for GE loopback test.

Revision 1.1.2.15  2016/11/29 06:27:52  meho
Changed submenu name and code clean up.

Revision 1.1.2.14  2016/11/28 03:43:55  meho
1. Fixed GE phy Mac/Int/Ext loopback test bugs.
2. Added 10G FW download.

Revision 1.1.2.13  2016/09/30 09:03:16  meho
code clean up

Revision 1.1.2.12  2016/09/30 08:57:30  meho
Fixed menu bug.

Revision 1.1.2.11  2016/08/12 10:12:19  meho
Clean up code.

Revision 1.1.2.10  2016/07/20 01:44:59  meho
Added GE PHY loopback debug utilities.

Revision 1.1.2.9  2016/07/14 09:17:41  meho
Added internal/SFP-external loopback for BCM82752.

Revision 1.1.2.8  2016/07/13 08:28:09  meho
1. Added Cavium PCS internal loopback.
2. Added check link up function for bcm54194.

Revision 1.1.2.7  2016/07/07 09:04:30  meho
1. Added BCM54194 RDB register r/w utility.
2. Added GE PHY internal/external loopback skeleton.
3. Added 10GE PHY internal/external loopback skeleton.

Revision 1.1.2.6  2016/06/23 12:44:54  meho
Added bcm54194 soft-reset function.

Revision 1.1.2.5  2016/06/23 07:10:53  meho
Update previous comment:
Added switch to SGMII/Copper/Fiber register space steps in BCM54194 r/w utility.

Revision 1.1.2.4  2016/06/23 06:28:48  meho
Added switch to SGMII/Copper/Fiber register space steps in BCM54191 r/w utility.

Revision 1.1.2.3  2016/06/22 10:40:27  meho
Added GE/10GE PHY r/w utilities.

Revision 1.1.2.2  2016/06/12 10:31:07  bowang3
Add bcm82752 10G PHY code framework

Revision 1.1.2.1  2016/06/06 05:58:51  xiaoyizh
Initial Check-in for Neptune Data Plane diags.


$Endlog$
*/
