/* $Id: diag_temp_sensor_test.c,v 1.2 2016/04/20 11:25:25 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_temp_sensor_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_test.c - Temperature Sensor test functions
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
#include "common_utils.h"
#include "defs.h"
#include "diag_temp_sensor_lib.h"
#include "diag_temp_sensor_test.h"
#include "diag_fpga_lib.h"

int diag_temp_sensor_test(int);
static int diag_tempsensor_reg_test(void);

static int diag_temp_read_fn(unsigned long, int, unsigned long *, void *);
static int diag_temp_write_fn(unsigned long, int, unsigned long, void *);

static int temp_i2c_slv_addr;
static reg_info_t_ext temp_sensor_reg_ext = {4, diag_temp_read_fn, diag_temp_write_fn, 0};

/* Global variables */
/* Max1617A registers table. This device is command based. Register offset is
 * the command written to the device.
 */
static reg_info_t sensor_reg_table[] =
{
    {"Temperature Register", TPM75_TEMPERATURE_REG, READ_ONLY,
    {(unsigned long)&temp_sensor_reg_ext}, 0xFFF0, 0x000},
    {"Configuration Register", TPM75_CONFIGURATION_REG, READ_ONLY,
    {(unsigned long)&temp_sensor_reg_ext}, 0xFF, 0x000},
    {"T_LOW Register", TPM75_T_LOW_REG, TEMP_SENSOR_RW,
    {(unsigned long)&temp_sensor_reg_ext}, 0xFFF0, 0x4B00},
    {"T_HIGH Register", TPM75_T_HIGH_REG, TEMP_SENSOR_RW,
    {(unsigned long)&temp_sensor_reg_ext}, 0xFFF0, 0x5000},
    {0, 0, 0, {0}, 0, 0},
};

/* Sub Menu used for temperature sensor tests.
 */
static submenu_xtable_t tempsensor_tests_submenu_table[] = {
    {"Register test", (type_t(*)())diag_tempsensor_reg_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define TEMPSENSOR_TESTS_SUBMENU_TABLE_SIZE (sizeof(tempsensor_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tempsensor_tests_primary_items[TEMPSENSOR_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t tempsensor_tests_secondary_items[TEMPSENSOR_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t tempsensor_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    tempsensor_tests_primary_items,
};
menuinfo_t *tempsensor_submenup = &tempsensor_subtest_menu;

int diag_temp_sensor_test (int run_all_tests)
{
    set_nios_mode(NIOS_DISABLE_MODE);
    
    build_primary_submenu(tempsensor_tests_submenu_table,
			              TEMPSENSOR_TESTS_SUBMENU_TABLE_SIZE,
                          "Temperature Sensor", &tempsensor_submenup);
    build_secondary_submenu(tempsensor_tests_submenu_table,
                            TEMPSENSOR_TESTS_SUBMENU_TABLE_SIZE,
                            tempsensor_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(tempsensor_submenup);
    } else {
        menu(tempsensor_submenup, tempsensor_tests_secondary_items, '\0');
    }
    
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

static int diag_tempsensor_reg_test (void)
{
    int retval = PASSED;
    int ix;
    testname("Temperature Sensor Register");

    for (ix = 0; ix < TPM75_DEVICE_NUMBER; ix++) {
        temp_i2c_slv_addr = get_temp_sensor_device_addr(ix); 
        prpass(testpass, "Temp. Sensor (%#x) ", temp_i2c_slv_addr);
        if (register_tests(0, sensor_reg_table) == FAILED ) {
            retval = FAILED;
        }
    }

    prcomplete(testpass, errcount, 0);
    if (retval == FAILED) {
        cterr('f', 0, "temp sensor reg Test failed.");
    }
    return (retval);
}

static int diag_temp_read_fn (unsigned long addr, int size, unsigned long *buf, void *param)
{
    return (diag_temp_sensor_reg_read(temp_i2c_slv_addr, addr, (uint16_t *)buf));
}

static int diag_temp_write_fn (unsigned long addr, int size, unsigned long data, void *param)
{
    return (diag_temp_sensor_reg_write(temp_i2c_slv_addr, addr, data));
}

/*---------------------------------------------------------------
$Log: diag_temp_sensor_test.c,v $
Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/10/15 06:23:21  benchen2
add set_nios_mode

Revision 1.1.2.3  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.2  2015/08/22 06:09:39  benchen2
Add temp sensor test item

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
