/* $Id: platform_eth.c,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_eth.c,v $
 *------------------------------------------------------------------
 *
 * Platform specific code for Linux base ethernet port loopback test
 * 
 * Feb 2019, Leschen
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
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
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "dash_fpga.h" /* for get SFP ctrl reg */  
#include "queryflags.h" /* for query user functions */  
#include "bcm82752_api.h"
#include "bcm82752_reg_def.h"
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
int te_port_mapping_phy_addr[] = {0x10, 0x11};
int eth_mapping_cvmx_bgx_num[] = {CVMX_BGX0_INF_ID, CVMX_BGX0_INF_ID, CVMX_BGX1_INF_ID,
                                  CVMX_BGX2_INF_ID, CVMX_BGX2_INF_ID, CVMX_BGX2_INF_ID,
                                  CVMX_BGX2_INF_ID};

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t eth_port_tests_submenu_table[] = {
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
Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2019/03/12 07:41:52  leschen
Initial check in to support BCM82752


$Endlog$
*/
