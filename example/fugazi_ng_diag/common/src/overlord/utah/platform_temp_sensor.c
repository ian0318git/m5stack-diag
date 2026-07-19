/* $Id: platform_temp_sensor.c,v 1.8 2014/09/11 08:06:20 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_temp_sensor.c,v $
 *------------------------------------------------------------------
 * platform_temp_sensor.c
 *
 * Description: Digital Temperature Sensor Definitions.
 *              This file is based on TMP75/ADT75 Datasheet.
 *
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "endians.h"
#include "common.h"
#include "platform_temp_sensor.h"
#include "defs.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "common_utils.h"
#include "byteswap.h"
#include "dash_fpga.h"
/******************************************************************************
 *                                   Externs
 ******************************************************************************/

/****************************************************************************** 
 *                               Function prototypes
 ******************************************************************************/
static uint32 show_temperature (int);
static uint32 sanity_test (int);
static uint32 alter_ts_reg(int);
static uint32 show_ts_reg(int);
uint32 show_temperature_all(void);

/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/

/*
 * Temperature sensor registers test table. This table is only used for register test.
 */
#if 0
static reg_info_t ts_reg_test_table[] =
{
    {"Hysteresis Threshold",    TS_PTR_THYST,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF80, 0x0000},
    {"Overtemperature Shutdown", TS_PTR_TOS,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(uint)REG_EXT},
     0xFF80, 0x0000},
    {0, 0, 0, {0}, 0, 0},
};
#endif

static char ts_name[4][16] =
{
    "Bezel Side#0",
	"Bezel Side#1",
	"I/O side#0",
    "I/O side#1",
};

/* Temperature sensor registers table. This device has registers with different sizes.
 */
static reg_info_t ts_reg_table[] =
{
/*	{ name, offset, rw type, 
 *    size, mask, default value},
 */
    {"Temperature",		TS_PTR_TEMP,	READ_ONLY,
	{TS_PTR_TEMP_L},	0xFF80, 0x0000},
    {"Configuration",	TS_PTR_CFG,	READ_WRITE,
	{TS_PTR_CFG_L},	0x1F, 0x00},
    {"Hysteresis Threshold",	TS_PTR_THYST,	READ_WRITE,
	{TS_PTR_THYST_L},	0xFF80, 0x0000},
    {"Overtemperature Shutdown",	TS_PTR_TOS,	READ_WRITE,
	{TS_PTR_TOS_L},	0xFF80, 0x0000},
    {"One-shot mode",	TS_PTR_OS,	READ_WRITE,
	{TS_PTR_OS_L},	0xFF80, 0x0000},
    {0, 0, 0, {0}, 0, 0},
};

/******************************************************************************
 *                                   Menus
 ******************************************************************************/
/*
 * Temperature sensor Menu
 */
static submenu_xtable_t ts_menu_table[] = {
	// bezel side #0
    {"Bezel side #0-Show temperature", (PFT)show_temperature, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0,(PFT)show_temperature, TS_BEZEL_SIDE0},
    {"Bezel side #0-Register sanity test", (PFT)sanity_test, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_BEZEL_SIDE0},
    {"Bezel side #0-Show register", (PFT)show_ts_reg, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_BEZEL_SIDE0},
    {"Bezel side #0-Alter register", (PFT)alter_ts_reg, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_BEZEL_SIDE0},
	// bezel side #1
    {"Bezel side #1-Show temperature", (PFT)show_temperature, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)show_temperature, TS_BEZEL_SIDE1},
    {"Bezel side #1-Register sanity test", (PFT)sanity_test, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_BEZEL_SIDE1},
    {"Bezel side #1-Show register", (PFT)show_ts_reg, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_BEZEL_SIDE1},
    {"Bezel side #1-Alter register", (PFT)alter_ts_reg, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_BEZEL_SIDE1},
    //i/o side #0
    {"I/O side #0-Show temperature", (PFT)show_temperature, TS_IO_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)show_temperature, TS_IO_SIDE0},
    {"I/O side #0-Register sanity test", (PFT)sanity_test, TS_IO_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_IO_SIDE0},
    {"I/O side #0-Show register", (PFT)show_ts_reg, TS_IO_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_IO_SIDE0},
    {"I/O side #0-Alter register", (PFT)alter_ts_reg, TS_IO_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_IO_SIDE0},
    //i/o side #1
    {"I/O side #1-Show temperature", (PFT)show_temperature, TS_IO_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)show_temperature, TS_IO_SIDE1},
    {"I/O side #1-Register sanity test", (PFT)sanity_test, TS_IO_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_IO_SIDE1},
    {"I/O side #1-Show register", (PFT)show_ts_reg, TS_IO_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_IO_SIDE1},
    {"I/O side #1-Alter register", (PFT)alter_ts_reg, TS_IO_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_IO_SIDE1},
    {"Show all temperatures", (PFT)show_temperature_all, 0, 0,
     (type_t(*)())0, 0, (PFT)show_temperature_all, 0},
};

#define TS_MENU_TABLE_SIZE (sizeof(ts_menu_table) / \
		sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_menu_primary_items[TS_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];
static mitem_t ts_menu_secondary_items[TS_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo tsdiag = {
    "Temperature Sensors Utility Menu",	    /* title */
    0,				    /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	    /* shows major flags */
    0,				    /* generic prompt */
    0,				    /* size -- bumped by add_menu_item() */
    ts_menu_primary_items,
};

static struct menuinfo *tsdiagp = &tsdiag;



/******************************************************************************
 *
 * function   : build_ts_menu
 * Description:	Build menu for temperature sensor related utility.
 * Inputs     : 
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int 
build_ts_menu (void)
{
    char t_name[ERR_BUF_SIZE];

    sprintf((char *)t_name, "Temperature Sensor");

    testname(t_name);
    
    build_primary_submenu(ts_menu_table, TS_MENU_TABLE_SIZE,
                          "Temperature Sensors Utility Menu", &tsdiagp);
    build_secondary_submenu(ts_menu_table, TS_MENU_TABLE_SIZE,
                            ts_menu_secondary_items);
    menu(&tsdiag, ts_menu_secondary_items, 0);

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : set_i2c_if_struct
 * Description: fill n2g_i2c_if_t struct based on different mux_id.
 * Inputs     : ts_id: 0. Bezel side, 1. I/O side
 *              i2c_if_p: pointer to n2g_i2c_if_t struct
 * 				offset: i2c device offset value
 *              buf_p: pointer to i2c tx/rx buffer
 * 				size: size of beffer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int 
set_i2c_if_struct(uint32_t ts_id, n2g_i2c_if_t* i2c_if_p, int offset, char* buf_p, int size)
{
    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
	i2c_if_p->i2c_bus_type = IOFPGA_I2C;
	i2c_if_p->i2c_ctrl = I2C_CTRL_TWO;
	i2c_if_p->mux = I2C_MUX_ZERO;
	i2c_if_p->offset = offset;
	i2c_if_p->size = size;
	i2c_if_p->buf = buf_p;
    switch (ts_id) {
        case TS_BEZEL_SIDE0:
			i2c_if_p->i2c_dev = TS_BEZEL_SIDE_ADDR0;
			break;
        case TS_BEZEL_SIDE1:
			i2c_if_p->i2c_dev = TS_BEZEL_SIDE_ADDR1;
			break;
        case TS_IO_SIDE0:
			i2c_if_p->i2c_dev = TS_IO_SIDE_ADDR0;
			break;
        case TS_IO_SIDE1:
			i2c_if_p->i2c_dev = TS_IO_SIDE_ADDR1;
			break;
        default:
            return FAILED;
    }       
    return PASSED;
}   


/******************************************************************************
 *
 * function   : read_ts_reg
 * Description: Wrapper to read temperature sensor's register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 *              addr_ptr_id - pointer register ID.
 * 				data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
read_ts_reg (int ts_id, int addr_ptr_id, char * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
	
    /* Setup I2C API interface struct */
    set_i2c_if_struct(ts_id, &i2c_if, addr_ptr_id, data_buf_p, size);
    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, addr_ptr_id, size, rc);
		return FAILED;
    }
	
    return (rc);
}


/******************************************************************************
 *
 * function   : write_ts_reg
 * Description: Wrapper to write temperature sensor's register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 *              addr_ptr_id - pointer register ID.
 * 				data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
write_ts_reg (int ts_id, int addr_ptr_id, char * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API interface struct */
    set_i2c_if_struct(ts_id, &i2c_if, addr_ptr_id, data_buf_p, size);
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to write offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, addr_ptr_id, size, rc);
        return (FAILED);
    }
    usleep(100000); /* sleep 100 ms after writing */
    return(rc);
}


/******************************************************************************
 *
 * function   : sanity_test
 * Description: A quick sanity read and write access test to device's R/W register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static uint32 
sanity_test (int ts_id)
{
    //printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
	int offset;
	ts_t backup_buf, new_buf, compare_buf;
	uint32_t rc = FAILED;
	for (offset = TS_PTR_THYST; offset <= TS_PTR_TOS; offset++) {
		backup_buf = new_buf = compare_buf = 0; // clear
		// read and backup reg data
		rc = read_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d backup old value: read_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
		// write new data
		new_buf = backup_buf + TS_TEST_PATTERN;
		rc = write_ts_reg (ts_id, offset, (char*) &new_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d write new value: write_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
		// read and compare reg data
		rc = read_ts_reg (ts_id, offset, (char*) &compare_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d read new value: read_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
		if (compare_buf != new_buf) {
			cterr('f', 0, "%s:%d compare result: value not matched (%#x != %#x)\n",
                      __FUNCTION__, __LINE__, new_buf, compare_buf);
			return(FAIL);
		}

		/* restore original data */
		rc = write_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d restore old value: write_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
    }
    printf("%s(): Passed\n", __FUNCTION__);
    return(rc);
}

/******************************************************************************
 *
 * Function   : show_temperature
 * Description: show temperature of the selected sensor
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32 
show_temperature (int ts_id)
{
    //printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
    uint32 rc;
    ts_t val;

    rc = read_ts_reg (ts_id, TS_PTR_TEMP, (char*) &val, sizeof(ts_t));
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                      __FUNCTION__, __LINE__, ts_id);
        return (FAILED);
    }
    val = DSWAP2(val);
    if (val <= TS_TEMP_MAX) {
        printf("%s temperature : %.4f Celcius\n", \
                            ts_name[ts_id], (val >> 4) * TS_TEMP_RESOLUTION);
    } else {
        printf("%s temperature : %.4f Celcius\n", \
                            ts_name[ts_id], ((val >> 4) - 4096) * TS_TEMP_RESOLUTION);
    }

    return (PASSED);
}


uint32
show_temperature_all(void) 
{
    int i, rc;
    
    /* show_temperature_all() is called after disable nios, 
     * to read cpu temperature from FPGA, we need to disable nios, too. 
     */
    show_cpu_temperature(); 

    for (i=TS_BEZEL_SIDE0; i<=TS_IO_SIDE1; i++) {
        /* for sword, Bezel side temp sensor 0 doesn't exist 
         * for dagger, Bezel side0 and I/O side1 temp sensors don't exist 
         */
        if ((is_sword() && (i == TS_BEZEL_SIDE0)) ||
            (is_dagger() && ((i == TS_BEZEL_SIDE0) || (i == TS_IO_SIDE1)))) {
            continue;
        } else {
            rc = show_temperature(i);
            if (rc != PASSED) {
                return (FAILED);
            }
        }
    }
    printf("\n");
    
    return (PASSED);
}

/********************************************************************
 *
 * Function:	show_ts_reg
 *
 * Description:	show temperature sensor register.
 *
 * Inputs:	-
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32
show_ts_reg(int ts_id)
{
    printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t buffer = 0;
    
    reg_p = &ts_reg_table[0];
    
    printf("\nRegister number:\n");
    while (reg_p->name) {
		printf("   %02x - %s\n", reg_p->offset, reg_p->name);
		reg_p++;	
    } 

    /* Get the register to be read */
    offset = gethex_answer("Enter the register number:", 0, 0, TS_PTR_OS);
    
    reg_p = &ts_reg_table[0]; /* Points to the beginning of the table */
    while (reg_p->name && reg_p->offset != offset) {
		/* Not requested register */
		reg_p++;	
    }
    
    if (reg_p->name) {
		/* Read the register. */
		if (reg_p->size.size == sizeof(ts_c)) {
			/* Configuration register */
			size = sizeof(ts_c);
		} else {
			/* Temperature registers */
			size = sizeof(ts_t);
		}
		
		buf_p = (char *)&buffer;
		offset = reg_p->offset;
		rc = read_ts_reg (ts_id, offset, buf_p, size);
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
						  __FUNCTION__, __LINE__, ts_id);
			return (FAILED);
		}
		else {
			printf("%-36s (0x%02X), data: 0x%04X.\n", reg_p->name,
				   reg_p->offset, buffer);
		}
	}
    return PASSED;
}


/********************************************************************
 *
 * Function:	alter_ts_reg
 *
 * Description:	alter temperature sensor register.
 *
 * Inputs:	-
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32
alter_ts_reg(int ts_id)
{
    printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t temp_data, old_temp_data;
    ts_c cfg_data, old_cfg_data;
    
    reg_p = &ts_reg_table[0];
    
    printf("\nRegister number:\n");
    /* Get the register to peek-n-poke */
    while (reg_p->name) {
		if (reg_p->type & READ_ONLY) {
			/* Read only register */
		} else {
			/* Write only or read/write register */
			printf("   %02x - %s\n", reg_p->offset, reg_p->name);
		}
		reg_p++;	/* update the register table pointer */
    } /* endof while */

    /* Get the register to peek-n-poke */
    offset = gethex_answer("Enter the register number:", 0, 0, TS_PTR_OS);
    
    /* Check if the register is read/writeable */
    /* Find the register text in the register table */
    reg_p = &ts_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != offset) {
		/* Not requested register */
		reg_p++;	/* update the register table pointer */
    }
    
    if (reg_p->name) {
		/* Got the register. */
		if (reg_p->type == READ_WRITE) {
			/* Read/Writeable. Read the register first. */
			if (reg_p->size.size == sizeof(ts_c)) {
				/* Configuration register */
				size = sizeof(ts_c);
				buf_p = (char *)&old_cfg_data;
			} else {
				/* Temperature registers */
				size = sizeof(ts_t);
				buf_p = (char *)&old_temp_data;
			}
			
			offset = reg_p->offset;

				
			/* Configuration register */
			rc = read_ts_reg (ts_id, offset, buf_p, size);
			if (rc != PASSED) {
				cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
							  __FUNCTION__, __LINE__, ts_id);
				return (FAILED);
			}

			/* Get the new data */
			if (reg_p->size.size == sizeof(ts_c)) {
				/* Configuration register */
				cfg_data = gethex_answer("Enter the data:", old_cfg_data, 0,
							0x1F);
				buf_p = (char*)&cfg_data;
			} else {
				/* Temperature registers */
				temp_data = gethex_answer("Enter the data:",
							old_temp_data, 0, TS_TEMP_MASK);
				buf_p = (char *)&temp_data;
			}

			/* Write the new data */
			rc = write_ts_reg (ts_id, offset, buf_p, size); 
			if (rc != PASSED) {
				cterr('f', 0, "%s:%d restore old value: write_ts_reg failed\n",
						  __FUNCTION__, __LINE__);
				return (FAILED);
			}
		} else {
			/* Not read/writeable */
			printf("alter_ts_reg() %s is a read only register", reg_p->name);
			return (FAILED);
		}
    } else {
		/* Invalid register */
		printf("alter_ts_reg() %#x is not a valid register", offset);
		return (FAILED);
    }
    return PASSED;
}


/*------------------------------------------------------------------
$Log: platform_temp_sensor.c,v $
Revision 1.8  2014/09/11 08:06:20  alpeng
dump cpu temperature during diag boot up


revision 1.7
date: 2014/07/22 11:59:51;  author: danchung;  state: Exp;  lines: +12 -4
1. For sword, remove bezel0(UT8) temp sensor related code
2. For dagger, remove bezel0(UT8) and IO1(UT7) temp sensors related code

revision 1.6
date: 2014/01/08 07:56:10;  author: hroni;  state: Exp;  lines: +10 -3
use enable_nios() instead of reseting NIOS

revision 1.5
date: 2013/11/26 08:40:38;  author: hroni;  state: Exp;  lines: +3 -3
branches:  1.5.6;
fix compiler warning

revision 1.4
date: 2013/07/24 17:31:12;  author: hroni;  state: Exp;  lines: +27 -5
during diag startup, show the temperature of bezel side and i/o side sensors

revision 1.3
date: 2013/07/23 18:04:59;  author: hroni;  state: Exp;  lines: +60 -30
1. Add more menu, Bezel part and IO part have 2 temperature sensors 2. Fix sanity test 3. Fix temperature display

revision 1.2
date: 2013/07/18 17:17:06;  author: mcharon;  state: Exp;  lines: +6 -9
add -Wal and clean up compile warnings

revision 1.1
date: 2013/06/19 09:45:40;  author: hroni;  state: Exp;
add utilities for I/O side and Bezel side temperature sensors

revision 1.5.6.1
date: 2014/04/17 08:23:26;  author: jamlin;  state: Exp;  lines: +10 -3
Pick up updated code from main trunk

$Endlog$
*/

