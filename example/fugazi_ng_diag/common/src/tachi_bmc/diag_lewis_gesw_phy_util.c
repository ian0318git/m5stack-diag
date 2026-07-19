/* $Id: diag_lewis_gesw_phy_util.c,v 1.2 2016/04/20 11:25:26 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_lewis_gesw_phy_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_lewis_gesw_phy_utils.c - Marvell lewis_gesw (98DX4235) Switch tests
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

int diag_lewis_gesw_phy_util (void);
void lewis_gesw_get_port(char * port_s);
void lewis_gesw_get_reg(char *reg_s);
void lewis_gesw_get_data(char *hex_data_s);

static int diag_lewis_gesw_phyread_util (void);
static int diag_lewis_gesw_phywr_util (void);
static int diag_lewis_gesw_phy_enstub_util (void);
static int diag_lewis_gesw_phy_disenstub_util (void);
int diag_lewis_gesw_phy_led_green_util (void);
int diag_lewis_gesw_phy_led_amber_util (void);
int diag_lewis_gesw_phy_led_clear_util (void);

/* Sub Menu used for lewis_gesw tests.
 */
static submenu_xtable_t lewis_gesw_phy_util_submenu_table[] = {
    {"Phy 1680L register read", (type_t(*)())diag_lewis_gesw_phyread_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy 1680L register write", (type_t(*)())diag_lewis_gesw_phywr_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy 1680L enable stub test", (type_t(*)())diag_lewis_gesw_phy_enstub_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy 1680L disable stub test", (type_t(*)())diag_lewis_gesw_phy_disenstub_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy 1680L led green", (type_t(*)())diag_lewis_gesw_phy_led_green_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy 1680L led amber", (type_t(*)())diag_lewis_gesw_phy_led_amber_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Phy 1680L led clear", (type_t(*)())diag_lewis_gesw_phy_led_clear_util,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};


#define LEWIS_GESW_PHY_UTIL_SUBMENU_TABLE_SIZE (sizeof(lewis_gesw_phy_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lewis_gesw_phy_util_primary_items[LEWIS_GESW_PHY_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t lewis_gesw_phy_util_secondary_items[LEWIS_GESW_PHY_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t lewis_gesw_phy_util_subtest_menu = {
    "%s Util Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    lewis_gesw_phy_util_primary_items,
};
menuinfo_t *lewis_gesw_phy_util_submenup = &lewis_gesw_phy_util_subtest_menu;

int diag_lewis_gesw_phy_util (void)
{

    build_primary_submenu(lewis_gesw_phy_util_submenu_table,
			  LEWIS_GESW_PHY_UTIL_SUBMENU_TABLE_SIZE,
                          "Lewis GESW", &lewis_gesw_phy_util_submenup);
    build_secondary_submenu(lewis_gesw_phy_util_submenu_table,
                            LEWIS_GESW_PHY_UTIL_SUBMENU_TABLE_SIZE,
                            lewis_gesw_phy_util_secondary_items);    
	
    menu(lewis_gesw_phy_util_submenup, lewis_gesw_phy_util_secondary_items, '\0');
    return (PASSED);
}

void lewis_gesw_get_port(char *port_s) 
{
    int port_num;
	
    port_num = getdec_answer("\n Enter port number (64-71)", 64, 64, 71);
    sprintf(port_s, "%d", port_num);
}
void lewis_gesw_get_reg(char *reg_s)
{
    int reg_num;
	
    reg_num = getdec_answer("\n Enter register number (0-100)", 0, 0, 100);
    sprintf(reg_s, "%d", reg_num);
}
void lewis_gesw_get_data(char *hex_data_s)
{
    unsigned long reg_data;
	
    reg_data = gethex_answer("\n Enter register data (0x0-0xFFFF)", 0, 0, 0xFFFF);
    sprintf(hex_data_s, "%lx\r", reg_data);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phyread_util
 *
 * Description: Utility to read from the PHY 1680L
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phyread_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_REG_RD_UTILS; 
    char port_num_s[15];
    char reg_num_s[15];
    int comm_lngth = sizeof(test_comm) + sizeof(NC_MVL_PORT) + sizeof(port_num_s) + sizeof(NC_MVL_REG)
                     + sizeof(reg_num_s) + sizeof("\r");
    char comm[comm_lngth];

    /* Take in port and register numbers from user */
    lewis_gesw_get_port(port_num_s);
    lewis_gesw_get_reg(reg_num_s);
	
    /* Prepare command to send to Switch */
    sprintf(comm, "%s %s %s %s %s\r", test_comm, NC_MVL_PORT, port_num_s, NC_MVL_REG, reg_num_s );
	
    if (run_lewis_gesw_test(comm, 1)) {
        printf("\nERROR: Something went wrong with the PHY read.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phywr_util
 *
 * Description: Utility to write to the PHY 1680L
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phywr_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_REG_WR_UTILS; 
    char port_num_s[15];
    char reg_num_s[15];
    char reg_data_s[31];
    int comm_lngth = sizeof(test_comm) + sizeof(NC_MVL_PORT) + sizeof(port_num_s) + sizeof(NC_MVL_REG)
                     + sizeof(reg_num_s) + sizeof(NC_MVL_DATA) + sizeof(reg_data_s) + sizeof("\r");
    char comm[comm_lngth];

    /* Take in port, register and data from user */
    lewis_gesw_get_port(port_num_s);
    lewis_gesw_get_reg(reg_num_s);
    lewis_gesw_get_data(reg_data_s);
	
    /* Prepare command to send to Switch */
    sprintf(comm, "%s %s %s %s %s %s 0x%s\r", test_comm, NC_MVL_PORT, port_num_s, NC_MVL_REG, reg_num_s, NC_MVL_DATA, reg_data_s );

    if (run_lewis_gesw_test(comm, 1)) {
        printf("ERROR: Something went wrong with the PHY write.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_enstub_util
 *
 * Description: Utility to perform a stub test on the PHY 1680L
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phy_enstub_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_STUB_EN_UTILS; 
    char port_num_s[15];
    int comm_lngth = sizeof(test_comm) + sizeof(NC_MVL_PORT) + sizeof(port_num_s) + 
                     + sizeof(NC_MVL_BMC_CONN) + sizeof("\r");
    char comm[comm_lngth];

    /* Take in port and register numbers from user */
    lewis_gesw_get_port(port_num_s);
	
    /* Prepare command to send to Switch */
    sprintf(comm, "%s %s %s %s\r", test_comm, NC_MVL_PORT, port_num_s, NC_MVL_BMC_CONN);
	
    if (run_lewis_gesw_test(comm, 1)) {
        printf("ERROR: Something went wrong with the PHY stub test.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_disenstub_util
 *
 * Description: Utility to disable stub test on the PHY 1680L
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
static int diag_lewis_gesw_phy_disenstub_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_STUB_DIS_UTILS; 
    char port_num_s[15];
    int comm_lngth = sizeof(test_comm) + sizeof(NC_MVL_PORT) + sizeof(port_num_s) + 
                     sizeof(NC_MVL_BMC_CONN) + sizeof("\r");
    char comm[comm_lngth];

    /* Take in port and register numbers from user */
    lewis_gesw_get_port(port_num_s);
	
    /* Prepare command to send to Switch */
    sprintf(comm, "%s %s %s %s\r", test_comm, NC_MVL_PORT, port_num_s, NC_MVL_BMC_CONN);
	
    if (run_lewis_gesw_test(comm, 1)) {
        printf("ERROR: Something went wrong with the PHY stub test.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_led_green_util
 *
 * Description: Utility to light phy 1680L led green
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_phy_led_green_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_LED_GRN_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with the setting the PHY led.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_led_amber_util
 *
 * Description: Utility to light phy 1680L led amber
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_phy_led_amber_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_LED_AMB_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with the setting the PHY led.\n");
        ret = FAILED;
    }

    return (ret);
}

/*------------------------------------------------------------------------------
 *
 * Function: diag_lewis_gesw_phy_led_clear_util
 *
 * Description: Utility to resoter led setting
 *
 * Input:  VOID
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int diag_lewis_gesw_phy_led_clear_util (void)
{
    int ret = PASSED;
    char test_comm[] = NC_MVL_PHY_LED_CLR_UTILS; 
	
    if (run_lewis_gesw_test(test_comm, 1)) {
        printf("ERROR: Something went wrong with the setting the PHY led\n");
        ret = FAILED;
    }

    return (ret);
}
/*----------------------------------------------------------------
$Log: diag_lewis_gesw_phy_util.c,v $
Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/01/12 07:35:02  jimmyya
Add cvs logs

$Endlog$
*/
