/* $Id: diag_temp_sensor_util.c,v 1.3 2017/03/30 08:30:54 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_temp_sensor_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_util.c - Temperature Sensor Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2017 by Cisco Systems, Inc.
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
#include "defs.h"
#include "common_utils.h"
#include "diag_temp_sensor_util.h"
#include "diag_temp_sensor_lib.h"
#include "i2c_api.h"
#include "diag_i2c_api.h"
#include "diag_fpga_i2c.h"
#include "diag_fpga_lib.h"
#include "proto.h"
#include "extern.h"
#include "uio_utils.h"
#include "linux_api.h"
#include "diag_fpga_lib.h"

int diag_temp_sensor_util(void);
int diag_show_temperature(void);
int diag_show_temperature_lib(void);

static int diag_temp_sensor_disp_reg(void);
static int diag_temp_sensor_alter_reg(void);

temp_desc board_temp[] = {
    {0x90, "Inlet (0x90)"},
    {0x92, "Inlet for NIM (0x92)"},
    {0x94, "Outlet (0x94)"},
    {0x96, "Outlet (0x96)"},
};


/* Sub Menu used for FPGA utility.
 */
static submenu_xtable_t temp_sensor_util_submenu_table[] = {
    {"Display Register", (type_t(*)())diag_temp_sensor_disp_reg,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Alter Register", (type_t(*)())diag_temp_sensor_alter_reg,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Show Temp.", (type_t(*)())diag_show_temperature,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define TEMP_SENSOR_UTIL_SUBMENU_TABLE_SIZE (sizeof(temp_sensor_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t temp_sensor_util_primary_items[TEMP_SENSOR_UTIL_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t temp_sensor_util_secondary_items[TEMP_SENSOR_UTIL_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t temp_sensor_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    temp_sensor_util_primary_items,
};
menuinfo_t *temp_sensor_util_submenup = &temp_sensor_util_subtest_menu;

int diag_temp_sensor_util (void)
{
    set_nios_mode(NIOS_DISABLE_MODE);
    build_primary_submenu(temp_sensor_util_submenu_table,
			              TEMP_SENSOR_UTIL_SUBMENU_TABLE_SIZE,
                          "Temperature Sensor Utility", &temp_sensor_util_submenup);
    build_secondary_submenu(temp_sensor_util_submenu_table,
                            TEMP_SENSOR_UTIL_SUBMENU_TABLE_SIZE,
                            temp_sensor_util_secondary_items);    
                            
    menu(temp_sensor_util_submenup, temp_sensor_util_secondary_items, '\0');
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

int diag_show_temperature (void)
{
    set_nios_mode(NIOS_DISABLE_MODE);
    
    if (diag_show_temperature_lib() ==FAILED ){
        return (FAILED);
    }
    
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

int diag_show_temperature_lib(void)
{
    int ix;
    uint16_t reg_val;
    printf("Temperature Info:\n");

    for (ix = 0; ix < BOARD_TEMP_SIZE; ix++) {
        if (diag_temp_sensor_reg_read(board_temp[ix].addr, TEMP_REG_OFFSET, &reg_val) 
            == FAILED) {
            printf("%s: Register read failed\n", __func__);
            return (FAILED);
        }

        printf("%-20s : ", board_temp[ix].desc);
        if (reg_val <= TS_TEMP_MAX) {
            printf("%.4f Celcius\n", (reg_val >> 4) * TS_TEMP_RESOLUTION);
        } else {
            printf("%.4f Celcius\n", ((reg_val >> 4) - 4096) * TS_TEMP_RESOLUTION);
        }
    }
    
    return (PASSED);
}
static int diag_temp_sensor_alter_reg (void)
{
    uint tpm75_offset, fifo_val, reg_offset;
    int option;

    option = gethex_answer("TPM75 offset(0:0x90, 1:0x92, 2:0x94, 3:0x96):", 0x00, 0x00, 0x03);
    tpm75_offset = get_temp_sensor_device_addr(option);
    reg_offset = gethex_answer("register offset:", 0x01, 0x0, 0x03);

    fifo_val = gethex_answer("register value :", 0x00, 0x0, 0xff);

    if (diag_temp_sensor_reg_write(tpm75_offset, reg_offset, fifo_val) == FAILED) {
        return (FAILED);
    }

    return(PASSED);

}

static int diag_temp_sensor_disp_reg (void)
{
    uint tpm75_offset, reg_offset;
    uint16_t fifo_val;
    int option;

    option = gethex_answer("TPM75 offset(0:0x90, 1:0x92, 2:0x94, 3:0x96):", 0x00, 0x00, 0x03);
    tpm75_offset = get_temp_sensor_device_addr(option);

    reg_offset = gethex_answer("Register offset:", 0x00, 0x0, 0x12);

    if (diag_temp_sensor_reg_read(tpm75_offset, reg_offset, &fifo_val) == FAILED) {
        return (FAILED);
    }

    printf("Read offset 0x%x = 0x%x\n", reg_offset, fifo_val);
    return(PASSED);
}

/*---------------------------------------------------------------
$Log: diag_temp_sensor_util.c,v $
Revision 1.3  2017/03/30 08:30:54  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2016/11/04 19:08:54  benchen2
Modify Enhanced error message

Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.10  2016/01/11 11:19:37  benchen2
for P2 temp sensor change address 0x98-> 0x96

Revision 1.1.2.9  2015/12/04 09:15:11  benchen2
for fix cdets CSCux41949

Revision 1.1.2.8  2015/10/15 06:23:21  benchen2
add set_nios_mode

Revision 1.1.2.7  2015/10/08 03:15:43  benchen2
add show temp

Revision 1.1.2.6  2015/10/01 08:38:21  tirawan
Update Temperature sensor description and add Intel power on/off utility

Revision 1.1.2.5  2015/09/21 13:09:16  tirawan
Display temperature sensor and FPGA version during boot up

Revision 1.1.2.4  2015/08/22 06:09:40  benchen2
Add temp sensor test item

Revision 1.1.2.3  2015/08/21 11:31:21  benchen2
add temperature sensor utility

Revision 1.1.2.2  2015/07/31 07:43:05  hondwang
r, w

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/

