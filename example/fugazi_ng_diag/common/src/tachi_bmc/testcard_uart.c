/* $Id: testcard_uart.c,v 1.3 2018/12/18 13:29:11 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/testcard_uart.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_uart.c
 *
 * Description: Testcard UART related diag tests and utilities.
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "defs.h"
#include "error.h"
#include "types.h"
#include "i2c_api.h"
#include "linux_api.h"
#include "menu.h"
#include "nvmonvars.h"
#include "ngio.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "diag_fpga_uart.h"
#include "diag_console_util.h"
#include "platform_fru.h"

/********************************************************************** 
                     Function Prototypes                              
***********************************************************************
 */
void build_tc_uart_menu(int);
int  tc_uart_lpbk_test(int);

static void build_tc_uart_utils(int);
static int  tc_set_uart_lpbk(uint8_t);
static int  tc_get_uart_lpbk_stat(void);

extern int do_all_menu_items(struct menuinfo *);

/***********************************************************************
                     Global Variables                            
 ***********************************************************************
 */

/* Definition of UART Loopback mode Options */
#define UART_HOST_LPBK_DIS   0x00
#define UART_HOST_LPBK_EN    0x01
#define UART_LINE_LPBK_DIS   0x02
#define UART_LINE_LPBK_EN    0x03

static uart_baud_info tc_uart_test_baud[] = {
    {"9600",   B9600},
    {"115200", B115200}
};


/***********************************************************************
                           Menus                             
 ***********************************************************************
 */
/*
 * UART Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_uart_diag_tbl[] = {
    {"UART Utilities",              (PFT)build_tc_uart_utils,   TRUE,
     0,                             (PFT)0,                     0,
     (PFT)build_tc_uart_utils,      TRUE},
    {"UART Internal loopback test", (PFT)tc_uart_lpbk_test,     TC_INT_LPBK,
     (MF_CONTINUOUS | MF_DOALL),    (PFT)0,                     0,
     (PFT)0,                        0},
    {"UART External loopback test", (PFT)tc_uart_lpbk_test,     TC_EXT_LPBK,
     (MF_CONTINUOUS),               (PFT)0,                     0,
     (PFT)0,                        0},
};

#define TC_UART_DIAG_TBL_SIZE (sizeof(tc_uart_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_uart_diag_pri_items[TC_UART_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_uart_diag_sec_items[TC_UART_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_uart_diag = {
    "TestCard UART SubMenu",   /* title */
    0,                         /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,     /* shows major flags */
    0,                         /* generic prompt */
    0,                         /* size -- bumped by add_menu_item() */
    tc_uart_diag_pri_items,
};

static struct menuinfo *tc_uart_diag_p = &tc_uart_diag;


/*
 * UART Utilities SubMenu
 */
static submenu_xtable_t uart_utils_tbl[] = {
    {"Get UART Loopback status",          (PFT)tc_get_uart_lpbk_stat,
     FALSE,                               0, (PFT)0, 0, (PFT)0, 0},
    {"Set UART Host Loopback mode",       (PFT)tc_set_uart_lpbk,
     UART_HOST_LPBK_EN,                   0, (PFT)0, 0, (PFT)0, 0},
    {"Release UART Host Loopback mode",   (PFT)tc_set_uart_lpbk,
     UART_HOST_LPBK_DIS,                  0, (PFT)0, 0, (PFT)0, 0},
    {"Set UART Line Loopback mode",       (PFT)tc_set_uart_lpbk,
     UART_LINE_LPBK_EN,                   0, (PFT)0, 0, (PFT)0, 0},
    {"Release UART Line Loopback mode",   (PFT)tc_set_uart_lpbk,
     UART_LINE_LPBK_DIS,                  0, (PFT)0, 0, (PFT)0, 0},
};

#define UART_UTILS_TBL_SIZE (sizeof(uart_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_uart_utils_pri_items[UART_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_uart_utils_sec_items[UART_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_uart_utils = {
    "TestCard UART Utilities",   /* title */
    0,                           /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,       /* shows major flags */
    0,                           /* generic prompt */
    0,                           /* size -- bumped by add_menu_item() */
    tc_uart_utils_pri_items,
};

static struct menuinfo *tc_uart_utils_p = &tc_uart_utils;


/*******************************************************************************
 *
 * Function   :	build_tc_uart_menu
 * Description:	Build TestCard UART tests and utilities submenu.
 * Inputs     :	submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_uart_menu (int submenu)
{
    build_primary_submenu(tc_uart_diag_tbl, TC_UART_DIAG_TBL_SIZE,
                          "TestCard UART SubMenu", &tc_uart_diag_p);
    build_secondary_submenu(tc_uart_diag_tbl, TC_UART_DIAG_TBL_SIZE,
                            tc_uart_diag_sec_items);
    if (submenu) {
        /* Entered with submenu */
        menu(&tc_uart_diag, tc_uart_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_uart_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   : build_tc_uart_utils
 * Description: Build TestCard UART related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_uart_utils (int submenu)
{
    build_primary_submenu(uart_utils_tbl, UART_UTILS_TBL_SIZE,
                          "TestCard UART Utilities", &tc_uart_utils_p);
    build_secondary_submenu(uart_utils_tbl, UART_UTILS_TBL_SIZE,
                            tc_uart_utils_sec_items);

    menu(&tc_uart_utils, tc_uart_utils_sec_items, 0);
}

/*******************************************************************************
 *
 * Function   : tc_uart_lpbk_test
 * Description:	Test TestCard UART interface by loopback test.
 * Inputs     : test_type - Loopback Type, external/internal loopback test 
 * Outputs    : Passed or Failed 
 *
 *******************************************************************************
 */
int tc_uart_lpbk_test (int test_type)
{
    char    test_if[TC_BUF_SIZE], test_name[TC_BUF_SIZE];
    int     result = FAILED, ctr = 0, test_total = 0;

    fru_table_offset = tc_fru_table_offset;
    platform_fru_table[fru_table_offset].pid_string = testcard_10gkr_pid;
    platform_fru_table[fru_table_offset].location_string = nim_10gkr_loc;
    cterr_add_component("Loopback test", "Testcard NIM UART interface");
    cterr_add_debug("UART string path: ",
                    "Platform FPGA UART interface->NIM UART interface",
                    "Check platform UART with console switch utility",
                    "Check platform FPGA reg test",
                    "Check NIM FPGA reg test for verify I2C to NIM");

    test_total = (sizeof(tc_uart_test_baud) / sizeof(uart_baud_info));

    /* Reset test buffer */
    memset(test_if, 0, TC_BUF_SIZE);
    memset(test_name, 0, TC_BUF_SIZE);


    /* Setup FPGA BMC UART connection */
    if ((diag_uart_to_nim_cnnt(testcard_if_p->slot)) == FAILED) {
        printf("%s: Fails in NIM UART connection\n", __FUNCTION__);
        return (FAILED);
    }

    if (test_type == TC_EXT_LPBK) {
        testname("TestCard UART External loopback");
        prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);
    } else if (test_type == TC_INT_LPBK) {
        /* To do TestCard UART internal loopback test,
         * need to set UART Host Loopback bit.
         * (bit 0 of FPGA register 0x10h, UART Register.)
         */
        if (tc_set_fpga_reg(UART_REG_OFFSET, UART_HOST_LPBK, ENABLE) != PASSED) {
            cterr('f', 0, "%s: FAILED to set UART Host Loopback mode.",
                          __FUNCTION__);
            return (FAILED);
        }

        testname("TestCard UART Internal loopback");
        prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);
    }

    /* Do UART loopback test */
    for (ctr = 0; ctr < test_total; ctr++) {

        result = uart_intf_test(UART_TTYS2_DEV, NULL, tc_uart_test_baud[ctr].baud_rate);
        if (result != PASSED) {
            break;
        }
    }

    /* Leave Host Loopback mode after internal loopback test is done. */
    if (test_type == TC_INT_LPBK) {
        if (tc_set_fpga_reg(UART_REG_OFFSET, UART_HOST_LPBK, DISABLE) != PASSED) {
            cterr('f', 0, "%s: FAILED to set UART Host Loopback mode.",
                          __FUNCTION__);
            return (FAILED);
        }
    }

    if (result != PASSED) {
        cterr('f', 0, "\n%s: FAILED to do UART interface %s test (baud rate = %s).",
                      __FUNCTION__, UART_TTYS2_DEV, tc_uart_test_baud[ctr].name);
    }        

    return (result);
}


/*******************************************************************************
 *
 * Function   : tc_set_uart_line_lpbk
 * Description: To set/release UART Loopback mode.
 * Inputs     : option - Host/Line loopback & ENABLE/DISABLE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_set_uart_lpbk (uint8_t option)
{
    char     opt_buf[TC_BUF_SIZE];
    int      ret = FAILED;
    uint16_t lpbk_type = 0;
    uint8_t  lpbk_opt = 0;

    /* Reset buffer */
    memset(opt_buf, 0, TC_BUF_SIZE);

    switch (option) {
    case UART_HOST_LPBK_DIS:
        lpbk_type = UART_HOST_LPBK;
        lpbk_opt  = DISABLE;
        sprintf(opt_buf, "Release %s%d TestCard UART Host loopback mode",
                         testcard_if_p->type_name, testcard_if_p->slot);
        break;
    case UART_HOST_LPBK_EN:
        lpbk_type = UART_HOST_LPBK;
        lpbk_opt  = ENABLE;
        sprintf(opt_buf, "Set %s%d TestCard UART Host loopback mode",
                         testcard_if_p->type_name, testcard_if_p->slot);
        break;
    case UART_LINE_LPBK_DIS:
        lpbk_type = UART_LINE_LPBK;
        lpbk_opt  = DISABLE;
        sprintf(opt_buf, "Release %s%d TestCard UART Line loopback mode",
                         testcard_if_p->type_name, testcard_if_p->slot);
        break;
    case UART_LINE_LPBK_EN:
        lpbk_type = UART_LINE_LPBK;
        lpbk_opt  = ENABLE;
        sprintf(opt_buf, "Set %s%d TestCard UART Line loopback mode",
                         testcard_if_p->type_name, testcard_if_p->slot);
        break;
    default:
        printf("%s: Invalid Type %d.\n", __FUNCTION__, option);
        return (FAILED);
    }

    ret = tc_set_fpga_reg(UART_REG_OFFSET, lpbk_type, lpbk_opt);
    if (ret == PASSED) {
        printf("\n%s Successfully.\n", opt_buf);
    } else {
        printf("\n*** %s Failed.\n", opt_buf);
    }

    return (ret);
}


/*******************************************************************************
 *
 * Function   : tc_get_uart_lpbk_stat
 * Description: Utility to get UART Loopback status.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_get_uart_lpbk_stat (void)
{
    uint16_t reg_val = 0;

    /* Read FPGA UART register */
    if (tc_fpga_reg_rd(UART_REG_OFFSET, &reg_val) != PASSED) {
        printf("%s: Failed to read TestCard FPGA UART Reg.\n", __FUNCTION__);
        return (FAILED);
    }

    printf("\nTestCard FPGA UART register(0x10h): 0x%04X.\n", reg_val);

    if (reg_val & UART_HOST_LPBK) {
        printf("TestCard UART Host Loopback is ON.\n");
    } else {
        printf("TestCard UART Host Loopback is OFF.\n");
    }

    if (reg_val & UART_LINE_LPBK) {
        printf("TestCard UART Line Loopback is ON.\n");
    } else {
        printf("TestCard UART Line Loopback is OFF.\n");
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: testcard_uart.c,v $
Revision 1.3  2018/12/18 13:29:11  hondwang
Fix CDETs CSCvn58971 with Goldschlager and Wallander on Tachi-L

Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.5  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.4  2015/09/26 05:20:42  alpeng
update console switch utils for intel

Revision 1.1.2.3  2015/09/23 09:06:17  alpeng
update console switch util to support cetus and nim; intel not yet

Revision 1.1.2.2  2015/09/04 06:07:28  alpeng
fix testcatd i2c r/w; supporting uart test

Revision 1.1.2.1  2015/07/31 10:40:05  alpeng
first check in for testcard


$Endlog$
*/

