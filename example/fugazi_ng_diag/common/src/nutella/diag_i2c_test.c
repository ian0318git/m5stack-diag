/* $Id: diag_i2c_test.c,v 1.4 2019/07/11 12:31:28 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_i2c_test.c,v $
 *-----------------------------------------------------------------------------
 * diag_i2c_test.c - For I2C test
 *
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "diag_i2c_test.h"
#include "diag_i2c_addr.h"
#include "diag_i2c_lib.h"
#include "linux_i2c_scan_test.h"

static n2g_i2c_if_t cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM SPD",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_DIMM0,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Temperature Sensor(LM75BD)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP_LM75,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "DMI EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_FRU_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C FPGA
     */
    {
     .dev_name = "Aikido ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = IOFPGA_I2C,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    {
     .dev_name = "END",
     .offset = 0,              /* end of struct */
     .i2c_bus_type = 0,
     .i2c_dev = 0,
     .i2c_ctrl = 0,
     .sub_addr_len = 0,
     .size = 0,
     .mux = 0,
     .buf = NULL,
     }
     ,
};

/*
 * Functional prototypn2g_i2c_if_te
 */
int diag_i2c_scan_test(int);

/*****************************************************************************
 *
 * Function   : diag_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on nutella
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int diag_i2c_scan_test (int option)
{
    int ret = FAILED, num_i2c_dev;
    uchar *tname = (uchar *) "I2C scan";

    testname("%s", tname);

    num_i2c_dev = sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t);

    ret = linux_i2c_scan_test(option, cpu_i2c_dev, num_i2c_dev);

    if (ret != PASSED) {
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    } else {
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

}

/*-------------------------------------------------
$Log: diag_i2c_test.c,v $
Revision 1.4  2019/07/11 12:31:28  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
