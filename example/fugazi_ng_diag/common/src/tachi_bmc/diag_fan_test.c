/* $Id: diag_fan_test.c,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fan_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_fan_test.c - Temperature Sensor test functions
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "diag_fan_test.h"
#include "diag_fpga_lib.h"
#include "common_utils.h"
#include "defs.h"
#include "diag_fpga_lib.h"

int diag_fan_test(int);
static int diag_fan_override_test(void);
static int diag_fan_read_fn(unsigned long, int, unsigned long *, void *);
static int diag_fan_write_fn(unsigned long, int, unsigned long, void *);

static reg_info_t_ext fan_reg_ext = {4, diag_fan_read_fn,
                                     diag_fan_write_fn, 0};

static reg_info_t fan_reg_tbl[] = {
    {"Environmental Fan Control", ENV_FAN_CTL,
    FAN_RW, {(unsigned long)&fan_reg_ext},
    0X0f00, 0xf00},
    {"PWM", FAN_PWM_SLOPE,
    FAN_RW, {(unsigned long)&fan_reg_ext},
    0X07ff, 0x14},
    {"FAN1 TACH SPEED", FAN1_TACH_SPEED,
    FAN_RW, {(unsigned long)&fan_reg_ext},
    0X07d0, 0x2bc},
    {"FAN2 TACH SPEED", FAN2_TACH_SPEED,
    FAN_RW, {(unsigned long)&fan_reg_ext},
    0X07d0, 0x2bc},
    {"FAN3 TACH SPEED", FAN3_TACH_SPEED,
    FAN_RW, {(unsigned long)&fan_reg_ext},
    0X07d0, 0x2bc},
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Sub Menu used for FAN tests.
 */
static submenu_xtable_t fan_tests_submenu_table[] = {
    {"FAN Override test", (type_t(*)())diag_fan_override_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define FAN_TESTS_SUBMENU_TABLE_SIZE (sizeof(fan_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fan_tests_primary_items[FAN_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t fan_tests_secondary_items[FAN_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t fan_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fan_tests_primary_items,
};
menuinfo_t *fan_submenup = &fan_subtest_menu;

int diag_fan_test (int run_all_tests)
{
    set_nios_mode(NIOS_DISABLE_MODE);
    
    build_primary_submenu(fan_tests_submenu_table,
			              FAN_TESTS_SUBMENU_TABLE_SIZE,
                          "FAN", &fan_submenup);
    build_secondary_submenu(fan_tests_submenu_table,
                            FAN_TESTS_SUBMENU_TABLE_SIZE,
                            fan_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(fan_submenup);
    } else {
        menu(fan_submenup, fan_tests_secondary_items, '\0');
    }
    
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

static int diag_fan_override_test (void)
{
    int retval = PASSED;
    testname("Fan Register");
    prpass(testpass, "Fan Register Test");

    if (register_tests(0, fan_reg_tbl) == FAILED) {
        cterr('f', 0, "Fan Register Test Failed");
        retval = FAILED;
    }

    prcomplete(testpass, errcount, 0);
    return (retval);
}
 
static int diag_fan_read_fn (unsigned long addr, int size,
                              unsigned long *buf, void *param)
{
    return (diag_fpga_reg_read(addr, (int *)buf));
}


static int diag_fan_write_fn (unsigned long addr, int size,
                               unsigned long data, void *param)
{
    return (diag_fpga_reg_write(addr, data));
}

/*---------------------------------------------------------------
$Log: diag_fan_test.c,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/10/15 06:23:22  benchen2
add set_nios_mode

Revision 1.1.2.2  2015/09/17 05:26:26  benchen2
add fan reg test

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/
