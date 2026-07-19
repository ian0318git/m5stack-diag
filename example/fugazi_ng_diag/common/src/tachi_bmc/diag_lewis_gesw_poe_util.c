/* $Id: diag_lewis_gesw_poe_util.c,v 1.2 2016/04/20 11:25:26 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_lewis_gesw_poe_util.c,v $
*------------------------------------------------------------------
 *
 * diag_lewis_gesw_poe_util.c
 * Marvell lewis_gesw (98DX4235) Switch PoE Utilitis
 * 
 * December 2015, Jimmy Yang
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

int diag_lewis_gesw_poe_util (void);
void lewis_gesw_get_poe_port(char * port_s);
int diag_lewis_gesw_poe_port_endis_util(int endis);
int diag_lewis_gesw_poe_led_amber_util(void);
int diag_lewis_gesw_poe_led_green_util(void);
int diag_lewis_gesw_poe_led_off_util(void);

static int diag_lewis_gesw_poe_init_util (void);
static int diag_lewis_gesw_poe_show_power_util (void);
static int diag_lewis_gesw_poe_sys_stat_util (void);

/* Sub Menu used for lewis_gesw tests.
 */
static submenu_xtable_t lewis_gesw_poe_util_submenu_table[] = {
    {"PoE device initialize", (type_t(*)())diag_lewis_gesw_poe_init_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE port enable", (type_t(*)())diag_lewis_gesw_poe_port_endis_util,   1,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE port disable", (type_t(*)())diag_lewis_gesw_poe_port_endis_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show PoE port power", (type_t(*)())diag_lewis_gesw_poe_show_power_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show PoE system status", (type_t(*)())diag_lewis_gesw_poe_sys_stat_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE led amber", (type_t(*)())diag_lewis_gesw_poe_led_amber_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE led green", (type_t(*)())diag_lewis_gesw_poe_led_green_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PoE led off", (type_t(*)())diag_lewis_gesw_poe_led_off_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};


#define LEWIS_GESW_POE_UTIL_SUBMENU_TABLE_SIZE (sizeof(lewis_gesw_poe_util_submenu_table) / \
                                                sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lewis_gesw_poe_util_primary_items[LEWIS_GESW_POE_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t lewis_gesw_poe_util_secondary_items[LEWIS_GESW_POE_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t lewis_gesw_poe_util_subtest_menu = {
    "%s Util Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    lewis_gesw_poe_util_primary_items,
};
menuinfo_t *lewis_gesw_poe_util_submenup = &lewis_gesw_poe_util_subtest_menu;

int diag_lewis_gesw_poe_util (void)
{

    build_primary_submenu(lewis_gesw_poe_util_submenu_table,
			  LEWIS_GESW_POE_UTIL_SUBMENU_TABLE_SIZE,
                          "Lewis GESW", &lewis_gesw_poe_util_submenup);
    build_secondary_submenu(lewis_gesw_poe_util_submenu_table,
                            LEWIS_GESW_POE_UTIL_SUBMENU_TABLE_SIZE,
                            lewis_gesw_poe_util_secondary_items);    
	
    menu(lewis_gesw_poe_util_submenup, lewis_gesw_poe_util_secondary_items, '\0');
    return (PASSED);
}

void lewis_gesw_get_poe_port(char *port_s) 
{
    int port_num;
	
    port_num = getdec_answer("\n Enter port number (0-7 or 8: all)", 8, 0, 8);
    sprintf(port_s, "%d", port_num);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_init_util
 *
 * Description: Utility to initialize the PoE/dragonite device
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_poe_init_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_INIT_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with initializing the PoE device.\n");
        ret = FAILED;
    }

    return (ret);
}

int diag_lewis_gesw_poe_port_endis_util (int endis)
{
    int ret = PASSED;
    char test_comm_en[] = NC_MVL_POE_PORT_EN_UTILS;
    char test_comm_dis[] = NC_MVL_POE_PORT_DIS_UTILS;
    char port_num_s[15];
    int comm_lngth = sizeof(test_comm_dis) + sizeof(NC_MVL_POE_PORT) + sizeof(port_num_s) + 
                     sizeof("NC_MVL_BMC_CONN") + sizeof("\r");
    char comm[comm_lngth];

    /* Take in port and register numbers from user */
    lewis_gesw_get_poe_port(port_num_s);

    /* Prepare command to send to Switch */
    if (endis) {
        sprintf(comm, "%s %s %s %s\r", test_comm_en, NC_MVL_POE_PORT, port_num_s, NC_MVL_BMC_CONN);
    } else {
        sprintf(comm, "%s %s %s %s\r", test_comm_dis, NC_MVL_POE_PORT, port_num_s, NC_MVL_BMC_CONN);
    } 

    if (run_lewis_gesw_test(comm, 1)) {
        printf("\nERROR: Something went wrong with the Poe Port setting.\n");
        ret = FAILED;
    }
    return(ret);

}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_show_power_util
 *
 * Description: Utility to show the power from each PoE port
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_poe_show_power_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_SHOW_PWR_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with showing the PoE power.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_sys_stat_util
 *
 * Description: Utility to show PoE system status
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_poe_sys_stat_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_SHOW_SYS_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with showing the PoE system status.\n");
        ret = FAILED;
    }

    return (ret);
}


/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_led_amber_util
 *
 * Description: Utility to light poe led amber
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_poe_led_amber_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_LED_AMB_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with setting the PoE led.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_led_green_util
 *
 * Description: Utility to light poe led green
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_poe_led_green_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_LED_GRN_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with setting the PoE led.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_poe_led_off_util
 *
 * Description: Utility to light poe led off
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_poe_led_off_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_POE_LED_OFF_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with setting the PoE led.\n");
        ret = FAILED;
    }

    return (ret);
}
/*----------------------------------------------------------------
$Log: diag_lewis_gesw_poe_util.c,v $
Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/01/12 07:35:02  jimmyya
Add cvs logs

$Endlog$
*/
