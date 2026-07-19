/* $Id: diag_fpga_test.c,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_test.c - FPGA Test Functions
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "proto.h"
#include "common_utils.h"
#include "diag_fpga_lib.h"
#include "diag_fpga_test.h"

int diag_fpga_test(int);

static int diag_fpga_reg_test(void);
static int diag_fpga_int_test(void);
static int diag_fpga_read_fn(unsigned long, int, unsigned long *, void *);
static int diag_fpga_write_fn(unsigned long, int, unsigned long, void *);

static reg_info_t_ext fpga_reg_ext = {4, diag_fpga_read_fn,
                                         diag_fpga_write_fn, 0};
static reg_info_t fpga_test_regs[] = {
    {"Scratchpad Register", FPGA_SCRATCHPAD_REG, FPGA_RW,
    {(unsigned long)&fpga_reg_ext}, 0xFFFFFFFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t fpga_tests_submenu_table[] = {
    {"Register test", (type_t(*)())diag_fpga_reg_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Interrupt test", (type_t(*)())diag_fpga_int_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define FPGA_TESTS_SUBMENU_TABLE_SIZE (sizeof(fpga_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_tests_primary_items[FPGA_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t fpga_tests_secondary_items[FPGA_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fpga_tests_primary_items,
};
menuinfo_t *fpga_submenup = &fpga_subtest_menu;

int diag_fpga_test (int run_all_tests)
{
    build_primary_submenu(fpga_tests_submenu_table,
			              FPGA_TESTS_SUBMENU_TABLE_SIZE,
                          "FPGA", &fpga_submenup);
    build_secondary_submenu(fpga_tests_submenu_table,
                            FPGA_TESTS_SUBMENU_TABLE_SIZE,
                            fpga_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(fpga_submenup);
    } else {
        menu(fpga_submenup, fpga_tests_secondary_items, '\0');
    }
    return (PASSED);
}

static int diag_fpga_reg_test (void)
{
    testname("FPGA Register");
    prpass(testpass, "FPGA Register Test");

    if (register_tests(0, fpga_test_regs) == FAILED) {
        cterr('f', 0, "FPGA Register Test Failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}

static int diag_fpga_int_test (void)
{
    int ix, intr_flag;
    int ret = PASSED;

    testname("FPGA Interrupt");
    prpass(testpass, "Force Interrupt Test");
    /* Start the interrupt Test which will enable FPGA interrupt and force interrupt */
    diag_fpga_start_int_test(); 

    intr_flag = 0;
    for (ix = 0; ix < FPGA_INTR_TEST_TOUT; ix++) {
        diag_fpga_get_intr_test_result(&intr_flag);

        if (intr_flag == 1) {
            break;
        }
        msleep(10);
    }

    if (intr_flag == 0) {
        cterr('f', 0, "FPGA Interrupt Test Failed");
        ret = FAILED;
    }

    diag_fpga_stop_int_test();

    prcomplete(testpass, errcount, 0);
    return (ret);
}

static int diag_fpga_read_fn (unsigned long addr, int size, 
                              unsigned long *buf, void *param)
{
    return (diag_fpga_reg_read(addr, (int *)buf));
}


static int diag_fpga_write_fn (unsigned long addr, int size,
                               unsigned long data, void *param)
{
    return (diag_fpga_reg_write(addr, data));
}

int diag_sgpio_test(void) {

    int rv = PASSED;
    int ret_val;
    
    if (diag_fpga_reg_read(FPGA_DEBUG_REG, &ret_val) == FAILED) {
        rv = FAILED;
    }

    system("echo 1 > /proc/nuova/gpio/led_status_amber_buf");
    system("echo 1 > /proc/nuova/gpio/led_status_green_buf");
    
    if (diag_fpga_reg_read(FPGA_DEBUG_REG, &ret_val) == FAILED) {
        rv = FAILED;
    }
   
    printf("\nDebug reg 0xF0 value is 0x%x\n", ret_val);
    
    if ((ret_val & SGPIO_BIT_8_9) != SGPIO_BIT_8_9) {
        rv = FAILED;
    }

    
    system("echo 0 > /proc/nuova/gpio/led_status_amber_buf");
    system("echo 0 > /proc/nuova/gpio/led_status_green_buf");
    
    if (diag_fpga_reg_read(FPGA_DEBUG_REG, &ret_val) == FAILED) {
        rv = FAILED;
    }

    printf("Debug reg value 0xF0 is 0x%x\n", ret_val);
    
    if (ret_val & SGPIO_BIT_8_9) {
       rv = FAILED;
    }

    return (rv);
}


/*---------------------------------------------------------------
$Log: diag_fpga_test.c,v $
Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.4  2016/03/07 07:10:06  benchen2
sgpio test

Revision 1.1.2.3  2015/11/13 00:50:32  tirawan
Remove FPGA SGPIO and add FPGA Interrupt

Revision 1.1.2.2  2015/08/04 10:57:44  tirawan
Add END address for FPGA register test

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/

