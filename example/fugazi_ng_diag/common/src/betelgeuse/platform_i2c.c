/* $Id: platform_i2c.c,v 1.2 2019/01/10 06:36:25 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/platform_i2c.c,v $
 *------------------------------------------------------------------
 * 
 * platform_i2c.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_dimm_util.h"
#include "diag_mcu_util.h"
#include "diag_temp_sensor_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_poe_psu_lib.h"

unsigned char i2c_debug = 0;

static n2g_i2c_if_t cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Aikido ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    /*
     * I2C 1
     */
    {
     .dev_name = "Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "SFP Module",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_SFP0,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C 2
     */
    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC Controller",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_EEPROM,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU Bootloader",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU_BOOTLOADER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "WiFi ACT2",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "WiFi Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

static n2g_i2c_if_t plat_cpu_i2c_dev[] = {
    /*
     * I2C 0
     */
    {
     .dev_name = "EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  /* dont' use sub address slave register */
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Aikido ACT 2 Lite Secure Chip",
     .offset = -1,              /* need to be -1 to tell driver not to use offset !!! */
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_AIKIDO_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
     ,
    /*
     * I2C 1
     */
    {
     .dev_name = "Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_MB_TEMP,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "SFP Module",
     .offset = 0,
     .i2c_bus_type = CPU_I2C1,
     .i2c_dev = MB_I2C_ADDR_SFP0,
     .i2c_ctrl = I2C_CTRL_ONE,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    /*
     * I2C 2
     */
    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC Controller",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "PoE DC EEPROM",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C_ADDR_POE_EEPROM,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "MCU",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = MB_I2C2_MCU,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
     {
     .dev_name = "WiFi Temperature Sensor(MAX31730AUB+T)",
     .offset = 0,
     .i2c_bus_type = CPU_I2C2,
     .i2c_dev = WIFI_I2C_PLAT_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_TWO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

static n2g_i2c_if_t plug_fpga_tc_i2c_dev[] = {
    
    /* I2C Device for Pluggable FPGA */
    {
     .dev_name = "Pluggable Test Card Temperature Sensor(LM75BDP)",
     .offset = 0,
     .i2c_bus_type = PLUG_FPGA,
     .i2c_dev = PLUG_I2C_ADDR_TEMP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable Test Card ACT2",
     .offset = -1,
     .i2c_bus_type = PLUG_FPGA,
     .i2c_dev = PLUG_I2C_ADDR_ACT2,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
    {
     .dev_name = "Pluggable Test Card GPIO Expander",
     .offset = 0,
     .i2c_bus_type = PLUG_FPGA,
     .i2c_dev = PLUG_TC_I2C_ADDR_GPIO_EXP,
     .i2c_ctrl = I2C_CTRL_ZERO,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),  
     .mux = I2C_MUX_ZERO,
     .buf = NULL,
     }
    ,
};

/*****************************************************************************
 *
 * Function   : plat_i2c_reg_temp_reg_rw_test (int option)
 *
 * Description: do Temperature register R/W test for i2c compontents
 *              (not all registers are W/R register)
 *
 * Inputs     : option , for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int plat_i2c_temp_reg_rw_test (int option)
{
    uint32_t ret_val = FAILED;
    uchar *tname = (uchar *) "Temperature register R/W test";

    testname("%s", tname);
    printf("\n");
    if (diag_temp_sensor_reg_test() == FAILED) {
        ret_val = FAILED;
    }

    prpass(testpass, "%s, ", tname);

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : get_n2g_i2c_if
 *
 * Description: return i2c structure
 *
 * Inputs     : i2c, mux, addr 
 *
 * Outputs    : i2c structure pointer or NULL
 *
 *******************************************************************************
 */
void *get_n2g_i2c_if (uint8_t i2c, uint8_t mux, uint8_t addr)
{
    int ix;
    int size_cpu = 0;
    int size_tc_plug_fpga = (sizeof(plug_fpga_tc_i2c_dev) / sizeof(n2g_i2c_if_t));

    size_cpu = (sizeof(plat_cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    for (ix = 0; ix < size_cpu; ix++) {
        if (plat_cpu_i2c_dev[ix].i2c_dev == addr &&
            plat_cpu_i2c_dev[ix].mux == mux &&
            plat_cpu_i2c_dev[ix].i2c_ctrl == i2c) {
                return ((void *) (&plat_cpu_i2c_dev[ix]));
            }
    }

    for (ix = 0; ix < size_tc_plug_fpga; ix++) {
        if (plug_fpga_tc_i2c_dev[ix].i2c_dev == addr &&
            plug_fpga_tc_i2c_dev[ix].mux == mux && 
            plug_fpga_tc_i2c_dev[ix].i2c_ctrl == i2c) {
            return ((void *) (&plug_fpga_tc_i2c_dev[ix]));
        }
    }
    printf("problem trying to get n2g_i2c_if; i2c_ctrl=%d, mux=%d, addr=%#x\n",
         i2c, mux, addr);
    fflush(stdout);
    return (void *) NULL;
}

/*******************************************************************************
 *
 * Function   : platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
 * Description: give address and controller number, return i2c structure
 *
 * Inputs     : addr: i2c addres; ctrl_no: i2c controller number
 *
 * Outputs    : pointer to i2c structure, or NULL if i2c struct is not found.
 *
 *******************************************************************************
 */
void *platform_i2c_get_quack (uint8_t addr, uint8_t ctrl_no)
{
    int ix;
    int size = 0;
    int size_tc_plug_fpga = (sizeof(plug_fpga_tc_i2c_dev) / sizeof(n2g_i2c_if_t));

    size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    for (ix = 0; ix < size; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr) {
            /*
             * to support different type of module/motherboard, etc...
             */
            cpu_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }

    for (ix = 0; ix < size_tc_plug_fpga; ix++) {
        if (plug_fpga_tc_i2c_dev[ix].i2c_dev == addr) {
            /*
             * to support different type of module/motherboard, etc...
             */
            plug_fpga_tc_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *) (&plug_fpga_tc_i2c_dev[ix]));
        }
    }
    /*
     * check mdoules now
     */
    printf("Software has no support for device at addr %#x; ctrl_no = %#x\n]",
         addr, ctrl_no);

    return (void *) NULL;
}


/*-------------------------------------------------------------------
 *
 * Function : is_need_dswap
 * Description: for declare is need dswap on platform 
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE 
 * -------------------------------------------------------------------
 */
boolean is_need_dswap (void)
{
    return (TRUE);
}

/*-------------------------------------------------
 * $Log: platform_i2c.c,v $
 * Revision 1.2  2019/01/10 06:36:25  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
