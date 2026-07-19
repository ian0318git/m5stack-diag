/* $Id: diag_mcu_test.c,v 1.3 2016/11/01 01:57:41 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mcu_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_mcu_test.c - MCU Test Functions
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
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "diag_mcu_test.h"
#include "diag_mcu_lib.h"
#include "i2c_api.h"
#include "proto.h"
#include "diag_fpga_lib.h"


#include <assert.h>

int diag_mcu_test(int);

static int diag_mcu_reg_test(int);

/* Simple registers read/write test table */
static mcu_reg_test_t simple_reg_test_tbl[] =
{
    {MCU_REG_TEST_OP1, ENV_MCU_FIRMWARE_REVISION},
    {MCU_REG_TEST_OP2, ENV_MCU_SCRATCH_PAD_0},
    {MCU_REG_TEST_OP2, ENV_MCU_SCRATCH_PAD_1},
    {MCU_REG_TEST_OP2, ENV_MCU_SCRATCH_PAD_2},
    {MCU_REG_TEST_OP2, ENV_MCU_SCRATCH_PAD_3},
    {MCU_REG_TEST_INVALID, 0},
};

/* Sub Menu used for MCU tests.
 */
static submenu_xtable_t mcu_tests_submenu_table[] = {
    {"Register test", (type_t(*)())diag_mcu_reg_test,   TRUE,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define MCU_TESTS_SUBMENU_TABLE_SIZE (sizeof(mcu_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mcu_tests_primary_items[MCU_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t mcu_tests_secondary_items[MCU_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t mcu_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mcu_tests_primary_items,
};
menuinfo_t *mcu_submenup = &mcu_subtest_menu;

int diag_mcu_test (int run_all_tests)
{
    set_nios_mode(NIOS_DISABLE_MODE);

    build_primary_submenu(mcu_tests_submenu_table,
			              MCU_TESTS_SUBMENU_TABLE_SIZE,
                          "MCU", &mcu_submenup);
    build_secondary_submenu(mcu_tests_submenu_table,
                            MCU_TESTS_SUBMENU_TABLE_SIZE,
                            mcu_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(mcu_submenup);
    } else {
        menu(mcu_submenup, mcu_tests_secondary_items, '\0');
    }

 
    msleep(EN_NIOS_MODE_DELAY);

 
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

static int diag_mcu_reg_test (int submenu)
{
    n2g_i2c_if_t i2c_if;
    mcu_reg_test_t *reg_tbl_p;
    uint32_t rc = PASSED;
    ren_t original_data, test_data, data, pat_1, pat_2, exp_pat_1, exp_pat_2;
    char err_buf[ERR_BUF_SIZE];

    if (submenu == TRUE) {
        testname("Simple Register Read/Write");
    } else {
        prpass(testpass, "Simple Register Read/Write Test");
    }

    /* Ready to test registers */
    reg_tbl_p = &simple_reg_test_tbl[0];

    while ((reg_tbl_p->option != MCU_REG_TEST_INVALID) && (rc == PASSED)) {

    prpass(testpass, "Offset %#x", reg_tbl_p->offset);
    rc = diag_mcu_reg_read(reg_tbl_p->offset, &original_data);
    if (rc != PASSED) {
        sprintf(err_buf, "env_reg_test() Unable to read original register "
                "@ %#x. rc = %#x", i2c_if.offset, rc);
        rc = FAILED;
        break;
    }
	switch (reg_tbl_p->option) {
	case MCU_REG_TEST_OP1:
	    /* Write 0 */
	    test_data = ENV_REG_TEST_OPTION1_PATTERN;

	    exp_pat_1 = pat_1 = exp_pat_2 = pat_2 = test_data;

	    rc = diag_mcu_reg_write(reg_tbl_p->offset, test_data);
	    if (rc == MCU_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }
	    rc = diag_mcu_reg_read(reg_tbl_p->offset, &data);
	    if (rc == MCU_WR_FAILED) {
		    /* Read ie rc == ENV_RD_FAILED */
		    sprintf(err_buf, "env_reg_test() Unable to read %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }


	    if (original_data != data) {
		    sprintf(err_buf, "env_reg_test() miscompare @ %#x. Expect %#x. "
				     "Read %#x", reg_tbl_p->offset,
				     original_data, test_data);
		    rc = FAILED;
	    }

	    break;
	case MCU_REG_TEST_OP2:
	    exp_pat_1 = pat_1 = ENV_REG_TEST_OPTION2_PATTERN1;
	    exp_pat_2 = pat_2 = ENV_REG_TEST_OPTION2_PATTERN2;
	    break;
	default:
	    assert(!"env_reg_test() Invalid option");
	    exp_pat_1 = pat_1 = exp_pat_2 = pat_2 = 0;
	    rc = FAILED;
	    break;
	} /* endof switch */

	if (rc != PASSED) {
	    break;
	}

	switch (reg_tbl_p->option) {
	case MCU_REG_TEST_OP2:
	    /* Test pattern 1 */
	    test_data = pat_1;
	    rc = diag_mcu_reg_write(reg_tbl_p->offset, test_data);
	    if (rc == MCU_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }
	    rc = diag_mcu_reg_read(reg_tbl_p->offset, &test_data);
	    if (rc == MCU_WR_FAILED) {
		    /* Read ie rc == ENV_RD_FAILED */
		    sprintf(err_buf, "env_reg_test() Unable to read %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }

	    /* Check the test result */
	    if ((rc == PASSED) && (test_data != exp_pat_1)) {
#ifndef ENV_REG_DEBUG
		sprintf(err_buf, "env_reg_test() pattern 1 miscompare @ %#x. "
				 "Expect %#x. Read %#x",  reg_tbl_p->offset,
				 exp_pat_1, test_data);
#else /* ENV_REG_DEBUG */
		printf("\n****env_reg_test() pattern 1 miscompare @ %#x. "
		       "Expect %#x. Read %#x\n", reg_tbl_p->offset,
		       exp_pat_1, test_data);
		printf("Re-read the register\n");
/*		msleep(ENV_REG_TEST_DELAY);   * delay 510 ms */
		rc = env_read(&i2c_if);

		if (rc != PASSED) {
		    printf("re-read failed @ %#x size %d\n", i2c_if.offset,
		            i2c_if.size);
		} else {
		    printf("read read data = %#x @ %#x size %d. ", test_data,
			   i2c_if.offset, i2c_if.size);
		    printf("buf @ %#x, test_data @ %#x i2c_if @ %#x\n",
			   i2c_if.buf, &test_data, &i2c_if);
		}

#endif /* ENV_REG_DEBUG */
		rc = (FAILED);
	    }

	    if (rc != PASSED) {
		break;
	    }

	    /* Test pattern 2 */
	    test_data = pat_2;
	    rc = diag_mcu_reg_write(reg_tbl_p->offset, test_data);
	    if (rc == MCU_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }
	    rc = diag_mcu_reg_read(reg_tbl_p->offset, &test_data);
	    if (rc == MCU_WR_FAILED) {
		    /* Read ie rc == ENV_RD_FAILED */
		    sprintf(err_buf, "env_reg_test() Unable to read %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }

	    /* Check the test result */
	    if ((rc == PASSED) && (test_data != exp_pat_2)) {
		sprintf(err_buf, "env_reg_test() pattern 2 miscompare @ %#x. "
				 "Expect %#x. Read %#x",  reg_tbl_p->offset,
				 exp_pat_2, test_data);
		rc = FAILED;
	    }

	    if (rc != PASSED) {
		break;
	    }

	    /* Restore the original value */
	    rc = diag_mcu_reg_write(reg_tbl_p->offset, original_data);
	    if (rc == MCU_WR_FAILED) {
		    /* Write failed */
		    sprintf(err_buf, "env_reg_test() Unable to write @ %#x. "
				     "rc = %#x", i2c_if.offset, i2c_if.err_no);
		    rc = (FAILED);
	    }

	    if (rc != PASSED) {
		sprintf(err_buf, "env_reg_test() Unable to restore the "
				 "original value @ %#x. rc = %#x",
				 i2c_if.offset, rc);
		rc = FAILED;
	    }
	    msleep(REN_I2C_PROC_TIME);
	    break;
	default:
	    break;
	} /* endof switch */

	reg_tbl_p++;
    } /* endof while */

    if (rc != PASSED) {
	cterr('f', 0, err_buf);
    }

    if (submenu == TRUE) {
	prcomplete(testpass, errcount, (char *)0);
    }



    return (rc);
}

/*---------------------------------------------------------------
$Log: diag_mcu_test.c,v $
Revision 1.3  2016/11/01 01:57:41  iachang
Add dealy time for en/disable nios mode for MCU test failure

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/10/15 06:23:21  benchen2
add set_nios_mode

Revision 1.1.2.3  2015/08/31 08:06:42  meho
Fixed MCU register test bug.

Revision 1.1.2.2  2015/07/31 07:41:52  hondwang
reg test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/
