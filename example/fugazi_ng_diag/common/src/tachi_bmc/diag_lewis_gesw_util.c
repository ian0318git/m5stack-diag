/* $Id: diag_lewis_gesw_util.c,v 1.3 2016/06/21 03:03:44 jimmyya Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_lewis_gesw_util.c,v $ 
 *------------------------------------------------------------------
 *
 * diag_lewis_gesw_utils.c - Marvell lewis_gesw (98DX4235) Switch tests
 * 
 * November 2015, Josh Skow
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "slot.h"
#include "menu.h"
#include "ngio.h"
#include "defs.h"
#include "diag_lewis_gesw_test.h"


int diag_lewis_gesw_util (int run_all_tests);
void lewis_gesw_get_port(char * port_s);
void lewis_gesw_get_reg(char *reg_s);
void lewis_gesw_get_data(char *hex_data_s);
int diag_lewis_gesw_set_nim_vlan_util(int);
int diag_lewis_gesw_nim_iface_setting(int,int);
int diag_lewis_gesw_x710_endis_serdes_lpbk (int endis);

static int diag_lewis_gesw_serdes_set_lpbk_util (void);
static int diag_lewis_gesw_serdes_unset_lpbk_util (void);
static int diag_lewis_gesw_show_iface_status_util (void);
static int diag_lewis_gesw_show_iface_mac_counter_util (void);
static int diag_lewis_gesw_show_diag_ver_util (void);

/* Sub Menu used for lewis_gesw tests.
 */
static submenu_xtable_t lewis_gesw_util_submenu_table[] = {
    {"Phy 1680L Utilities", (type_t(*)())diag_lewis_gesw_phy_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE Utilities", (type_t(*)())diag_lewis_gesw_poe_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show Lewis diag image version", (type_t(*)())diag_lewis_gesw_show_diag_ver_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SERDES port set internal loopback", (type_t(*)())diag_lewis_gesw_serdes_set_lpbk_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SERDES port unset internal loopback", (type_t(*)())diag_lewis_gesw_serdes_unset_lpbk_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show port interface status", (type_t(*)())diag_lewis_gesw_show_iface_status_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show port packet counter", (type_t(*)())diag_lewis_gesw_show_iface_mac_counter_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Set NIM vlan setting", (type_t(*)())diag_lewis_gesw_set_nim_vlan_util,   1,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Unset NIM vlan setting", (type_t(*)())diag_lewis_gesw_set_nim_vlan_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};


#define lewis_gesw_UTIL_SUBMENU_TABLE_SIZE (sizeof(lewis_gesw_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lewis_gesw_util_primary_items[lewis_gesw_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t lewis_gesw_util_secondary_items[lewis_gesw_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t lewis_gesw_util_subtest_menu = {
    "%s Util Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    lewis_gesw_util_primary_items,
};
menuinfo_t *lewis_gesw_util_submenup = &lewis_gesw_util_subtest_menu;

int diag_lewis_gesw_util (int run_all_tests)
{

    build_primary_submenu(lewis_gesw_util_submenu_table,
			              lewis_gesw_UTIL_SUBMENU_TABLE_SIZE,
                          "Lewis GESW", &lewis_gesw_util_submenup);
    build_secondary_submenu(lewis_gesw_util_submenu_table,
                            lewis_gesw_UTIL_SUBMENU_TABLE_SIZE,
                            lewis_gesw_util_secondary_items);    
	
    if (run_all_tests) {
        exec_doall_menu_items(lewis_gesw_util_submenup);
    } else {
        menu(lewis_gesw_util_submenup, lewis_gesw_util_secondary_items, '\0');
    }
    return (PASSED);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_set_serdes_lpbk_util
 *
 * Description: Utility to enable internal loopback on a serdes port
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_serdes_set_lpbk_util (void)
{
	int ret = PASSED;
	char test_comm[] = NC_MVL_SW_LB_EN_UTILS; 
	char port_num_s[15];
	int comm_lngth = sizeof(test_comm) + sizeof("port") + sizeof(port_num_s) + sizeof("\r");
	char comm[comm_lngth];

	/* Take in port and register numbers from user */
	lewis_gesw_get_port(port_num_s);
	
	/* Prepare command to send to Switch */
	sprintf(comm, "%s %s %s\r", test_comm, "port", port_num_s);
	
 	if (run_lewis_gesw_test(comm, TACHI_SPECIFIC)) 
	{
		printf("ERROR: Something went wrong with the setting the SERDES internal loopback.\n");
        ret = FAILED;
	}

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_serdes_unset_lpbk_util
 *
 * Description: Utility to disable internal loopback on a serdes port
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_serdes_unset_lpbk_util (void)
{
	int ret = PASSED;
	char test_comm[] = NC_MVL_SW_LB_DIS_UTILS; 
	char port_num_s[15];
	int comm_lngth = sizeof(test_comm) + sizeof("port") + sizeof(port_num_s) + sizeof("\r");
	char comm[comm_lngth];

	/* Take in port and register numbers from user */
	lewis_gesw_get_port(port_num_s);
	
	/* Prepare command to send to Switch */
	sprintf(comm, "%s %s %s\r", test_comm, "port", port_num_s);
	
 	if (run_lewis_gesw_test(comm, TACHI_SPECIFIC)) 
	{
		printf("ERROR: Something went wrong with the unsetting the SERDES internal loopback.\n");
        ret = FAILED;
	}

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_x710_endis_serdes_lpbk
 *
 * Description: Function to enable/disable internal loopback on a serdes port
 *
 * Input:  TRUE or FALSE
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_x710_endis_serdes_lpbk (int endis)
{
    int ret = PASSED;
    char test_comm_en[] = NC_MVL_X710_SW_LB_EN; 
    char test_comm_dis[] = NC_MVL_X710_SW_LB_DIS; 

    
    if (endis) {
        if (run_lewis_gesw_test(test_comm_en, 1)) {
            printf("ERROR: Something went wrong with the setting the SERDES internal loopback.\n");
            ret = FAILED;
        }
    } else {
        if (run_lewis_gesw_test(test_comm_dis, 1)) {
            printf("ERROR: Something went wrong with the setting the SERDES internal loopback.\n");
            ret = FAILED;
        }
    }    
    
    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_show_iface_status_util
 *
 * Description: Utility to show interface status of a port
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_show_iface_status_util (void)
{
	int ret = PASSED;
	char test_comm[] = NC_MVL_SHOW_IF_STA_UTILS; 
	char port_num_s[15];
	int comm_lngth = sizeof(test_comm) + sizeof(port_num_s) + sizeof("\r");
	char comm[comm_lngth];

	/* Take in port and register numbers from user */
	lewis_gesw_get_port(port_num_s);
	
	/* Prepare command to send to Switch */
	sprintf(comm, "%s%s\r", test_comm, port_num_s);
	
 	if (run_lewis_gesw_test(comm, TACHI_SPECIFIC)) 
	{
		printf("ERROR: Something went wrong with showing the interface status.\n");
        ret = FAILED;
	}

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_show_iface_mac_counter_util
 *
 * Description: Utility to show packet counters of a port
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_show_iface_mac_counter_util (void)
{
	int ret = PASSED;
	char test_comm[] = NC_MVL_SHOW_MAC_CNT_UTILS; 
	char port_num_s[15];
	int comm_lngth = sizeof(test_comm) + sizeof(port_num_s) + sizeof("\r\r");
	char comm[comm_lngth];

	/* Take in port and register numbers from user */
	lewis_gesw_get_port(port_num_s);
	
	/* Prepare command to send to Switch */
	sprintf(comm, "%s%s\r\r", test_comm, port_num_s);
	
 	if (run_lewis_gesw_test(comm, TACHI_SPECIFIC)) 
	{
		printf("ERROR: Something went wrong with showing the packet counter.\n");
        ret = FAILED;
	}

    return (ret);
}


/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_show_diag_ver_util
 *
 * Description: This function show the lewis diag version
 *
 * Input: 
 *       void
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_show_diag_ver_util (void)
{   
    int ret = PASSED;
    char test_comm[] = NC_MVL_SHOW_DIAG_VER_UTILS;
    
    if (run_lewis_gesw_test(test_comm, 1)) {
        cterr('f', 0, "Show GESW diag image verions Failed");
        ret = FAILED;
    }
    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_nim_iface_setting
 *
 * Description: This function is used to change the Lewis NIM interface setting.
 *
 * Input:
 *       int is_set:  1: set  0: unset
 *       int iface_type: 0: 1000_Base_X 1:SGMII 2: 10G-KR (for future usage)
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
*/
int diag_lewis_gesw_nim_iface_setting (int is_set, int iface_type)
{
    int ret = PASSED;
    char test_comm[64];

    if (is_set) {
        switch (iface_type) {
            case NIM_1000BASE_X:
                sprintf(test_comm, "%s%s",NC_MVL_EXAMPLES, NC_MVL_NIM_IFACE_1000BASEX);  
                break;
            case NIM_SGMII:
                sprintf(test_comm, "%s%s",NC_MVL_EXAMPLES, NC_MVL_NIM_IFACE_SGMII);
                break;
            case NIM_KR:
                sprintf(test_comm, "%s%s",NC_MVL_EXAMPLES, NC_MVL_NIM_IFACE_KR);
                break;
            default:
                cterr('f', 0, "Wrong Lewis nim's interface setting");
                ret = FAILED;
                return (ret);
                
        }
    } else {
        /* Lewis default NIM interface setting.*/
        sprintf(test_comm, "%s%s",NC_MVL_EXAMPLES, NC_MVL_NIM_IFACE_1000BASEX);
    }
    if (run_lewis_gesw_test(test_comm, 1)) {
        cterr('f', 0, "Set Lewis nim's interface Failed");
        ret = FAILED;
       } 

     return (ret);
}


/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_set_nim_vlan_util
 *
 * Description: This function executes the lewis_gesw poe test
 *
 * Input: 
 *       int stting_flag:  1: set  0: unset
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_set_nim_vlan_util (int setting_flag)
{   
    int ret = PASSED;
    char test_comm_examples[] = NC_MVL_EXAMPLES;
    char test_comm_set[] = NC_MVL_NIM_VLAN_SET;
    char test_comm_unset[] = NC_MVL_NIM_VLAN_UNSET;
    int chr_lng = strlen(test_comm_examples) + strlen(test_comm_unset);
    char test_comm[chr_lng];
    
    if (setting_flag) {
        sprintf(test_comm, "%s%s",test_comm_examples, test_comm_set);
    } else {
        sprintf(test_comm, "%s%s",test_comm_examples, test_comm_unset);
    }
    if (run_lewis_gesw_test(test_comm, 1)) {
        cterr('f', 0, "Set nim vlan Failed");
        ret = FAILED;
	}

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: gesw_image_info
 *
 * Description: This function shows the version and compile information of the 
 *              GESW image.
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int gesw_image_info(void) 
{
	int ret = PASSED;
	if (run_lewis_gesw_test(NC_MVL_SHOW_INFO, TACHI_SPECIFIC)) 
	{
		printf("\nGESW is powered off or NC connection is down\n");
        ret = FAILED;
	} 
    return (ret);
}

