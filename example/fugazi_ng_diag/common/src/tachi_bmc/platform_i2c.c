/* $Id: platform_i2c.c,v 1.2 2016/04/20 11:25:30 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_i2c.c,v $
 *------------------------------------------------------------------
 *
 * platform_i2c.c - Platform I2C related function
 * 
 * July 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "error.h"
#include "types.h"
#include "i2c_api.h"
#include "n2g_api_rc.h"
#include "platform_i2c.h"
#include "ngio.h"
#include <assert.h>

void *platform_i2c_get_quack(uint8_t, uint8_t);
void *platform_fpga_get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
static n2g_i2c_if_t wic_oir[MAX_WIC+FIRST_SLOT];
static char wic_oir_buf[MAX_WIC+FIRST_SLOT][256];
#ifdef FOXCONN_FPGA
static uint8_t wic_i2c_ctrl[] = {0,  I2C_CTRL_TEN, 0, 0};
#else
static uint8_t wic_i2c_ctrl[] = {0,  I2C_CTRL_TWELVE, I2C_CTRL_THIRTEEN,
				 I2C_CTRL_FOURTEEN};
#endif

static uint8_t daughter_card_i2c_ctrl[] = {0,  I2C_CTRL_SEVEN, I2C_CTRL_FIFTEEN, 0};
static uint8_t daughter_card_i2c_addr[] = {0,  POE_I2C_ADDR_ACT2, RAID_I2C_ADDR_ACT2, 0};

static n2g_i2c_if_t fpga_i2c_dev[] = {
    {
        .dev_name = "ACT2",
        .offset = -1,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_ACT2,
        .i2c_ctrl = I2C_CTRL_ZERO,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
	{
        .dev_name = "ENV MCU",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_ENV_MCU,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
	{
        .dev_name = "Altitude Sensor",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_SENSOR,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor(TPM75)",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_MB_TEMP,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor Alert",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_MB_TEMP_ALRT,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor Inlet for NIM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP_INLET_U29,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor Inlet",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP_INLET_U27,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor Outlet U39",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP_OUTLET_U39,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "Temperature Sensor Outlet U337",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_TEMP_OUTLET_U337,
        .i2c_ctrl = I2C_CTRL_TWO,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = " RAID PCA9557",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PCA9557,
        .i2c_ctrl = I2C_CTRL_FIFTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PEM0 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_PSU_FAN,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "CPLD 5M570",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_CPLD_5M570,
        .i2c_ctrl = I2C_CTRL_FIFTEEN,
        .sub_addr_len = 0,
        .size = sizeof(uint8_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PEM0 EEPROM",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PEM0_EEPROM,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 0,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "PEM0 Microcontroller",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = MB_I2C_ADDR_PEM0_MCNTRL,
        .i2c_ctrl = I2C_CTRL_FOUR,
        .sub_addr_len = 1,
        .size = sizeof(uint32_t),
        .mux = I2C_MUX_ZERO,
        .buf = NULL,
    },
    {
        .dev_name = "ACT2-NIM1",
        .offset = -1,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = NIM_I2C_ADDR_ACT2,
#ifdef FOXCONN_FPGA
        .i2c_ctrl = I2C_CTRL_TEN,
#else
        .i2c_ctrl = I2C_CTRL_TWELVE,
#endif
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
    {
        .dev_name = "ACT2-NIM2",
        .offset = -1,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = NIM_I2C_ADDR_ACT2,
        .i2c_ctrl = I2C_CTRL_THIRTEEN,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },
    {
        .dev_name = "ACT2-NIM3",
        .offset = -1,  
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = NIM_I2C_ADDR_ACT2,
        .i2c_ctrl = I2C_CTRL_FOURTEEN,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO, 
        .buf        = NULL,
    },    
    {
        .dev_name = "ACT2-RAID",
        .offset = -1,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = RAID_I2C_ADDR_ACT2,
        .i2c_ctrl = I2C_CTRL_FIFTEEN,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
    {
        .dev_name = "ACT2-POE",
        .offset = -1,
        .i2c_bus_type = IOFPGA_I2C,
        .i2c_dev = POE_I2C_ADDR_ACT2,
        .i2c_ctrl = I2C_CTRL_SEVEN,
        .sub_addr_len = 0,
        .size    = sizeof(uint16_t),
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
};

static n2g_i2c_if_t ngio_oir[] = {
    {
        .dev_name = "OIR",
        .offset = 0,
        .i2c_bus_type = IOFPGA_I2C,
        .size    = sizeof(uint16_t),
        .sub_addr_len = 1,
        .mux = I2C_MUX_ZERO,
        .buf        = NULL,
    },
};

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
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    int ix;

    for (ix = 0; ix < size; ix++) {
        if (fpga_i2c_dev[ix].i2c_dev == addr) {
            fpga_i2c_dev[ix].i2c_ctrl = ctrl_no;
            return ((void *)(&fpga_i2c_dev[ix]));
        }
    }
    
    return (NULL);
}


void *platform_fpga_get_n2g_i2c_if (uint8_t i2c, uint8_t mux, uint8_t addr)
{
    int size = (sizeof(fpga_i2c_dev)/sizeof(n2g_i2c_if_t));
    int ix;
    
    for (ix = 0; ix < size; ix++) {
        if (fpga_i2c_dev[ix].i2c_dev == addr &&
			fpga_i2c_dev[ix].mux == mux &&
			fpga_i2c_dev[ix].i2c_ctrl == i2c) {
            return ((void *)(&fpga_i2c_dev[ix]));
        }
    }

	printf("%s: Problem trying to get n2g_i2c_if; i2c_ctrl=%d, mux=%d, addr=%#x\n", 
		   __FUNCTION__, i2c, mux, addr);
	fflush(stdout);
    
    return (NULL);
}

/*******************************************************************************
 *
 * Function   : platform_get_wic_oir
 * Description: returns WIC OIR
 *
 * Inputs     : slot number
 *
 * Outputs    : pointer to OIR
 *
 *******************************************************************************
 */
void *
platform_get_wic_oir (int slot)
{
    if (slot == 0) {
        assert(!"slot is 0");
    }
    memcpy(&wic_oir[slot], ngio_oir, sizeof(n2g_i2c_if_t));
    wic_oir[slot].buf = wic_oir_buf[slot];
    wic_oir[slot].i2c_ctrl = get_wic_i2c_ctrl(slot);
    wic_oir[slot].i2c_dev = NIM_I2C_ADDR_OIR;
    return (void *)&wic_oir[slot];
}

/*************************************************************************
 *
 * Function   : get_wic_i2c_ctrl (int slot)
 * Description: returns i2c address of wic i2c controller
 *
 * Inputs     : slot number
 *
 * Outputs    : i2c address
 *
 *************************************************************************
 */
uint8_t get_wic_i2c_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_wic_i2c_ctrl");
    }
    return (wic_i2c_ctrl[slot]);

}

/*************************************************************************
 *
 * Function   : get_daughter_card_i2c_ctrl (int slot)
 * Description: returns i2c ctrl number of daughter card i2c controller
 *
 * Inputs     : slot number
 *
 * Outputs    : i2c address
 *
 *************************************************************************
 */
uint8_t get_daughter_card_i2c_ctrl (int slot)
{
    if (slot == 0) {
        assert(!"get_daughter_card_i2c_ctrl");
    }
    return (daughter_card_i2c_ctrl[slot]);
}

/*************************************************************************
 *
 * Function   : get_daughter_card_i2c_addr (int slot)
 * Description: returns i2c address of daughter card i2c controller
 *
 * Inputs     : slot number
 *
 * Outputs    : i2c address
 *
 *************************************************************************
 */
uint8_t get_daughter_card_i2c_addr (int slot)
{
    if (slot == 0) {
        assert(!"get_daughter_card_i2c_addr");
    }
    return (daughter_card_i2c_addr[slot]);
}
/*---------------------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.20  2016/03/02 08:35:42  benchen2
add sbr vdd eeprom ping test

Revision 1.1.2.19  2015/12/23 11:16:14  alpeng
support PEM(PSU) utility and its fan utils

Revision 1.1.2.18  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.17  2015/11/16 08:06:12  benchen2
add psu fan addr

Revision 1.1.2.16  2015/11/13 09:28:32  benchen2
modify raid card act address

Revision 1.1.2.15  2015/11/13 08:05:42  benchen2
modify raid card act2

Revision 1.1.2.14  2015/11/02 10:22:56  tirawan
Add PoE Cookie Utility

Revision 1.1.2.13  2015/10/28 07:55:04  benchen2
add raid act2 cookies utility

Revision 1.1.2.12  2015/10/23 07:42:25  benchen2
fix raid card utility

Revision 1.1.2.11  2015/10/12 08:22:27  benchen2
add raid card cpld upgrade

Revision 1.1.2.10  2015/09/18 06:58:54  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.9  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.8  2015/08/31 08:06:42  meho
Fixed MCU register test bug.

Revision 1.1.2.7  2015/08/30 05:57:36  tirawan
To support NIM ACT2 R/W access using TAM library

Revision 1.1.2.6  2015/08/28 02:33:52  tirawan
To support ACT2 M/B cookie programming using Foxconn FPGA

Revision 1.1.2.5  2015/08/21 11:31:21  benchen2
add temperature sensor utility

Revision 1.1.2.4  2015/07/31 08:31:14  hondwang
change temp sensor name

Revision 1.1.2.3  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.2  2015/07/26 06:02:22  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/
