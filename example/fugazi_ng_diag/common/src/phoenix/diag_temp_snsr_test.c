/* $Id: diag_temp_snsr_test.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_temp_snsr_test.c,v $
 *------------------------------------------------------------------
 * Filename:  diag_temp_snsr_test.c
 *
 * Description: Tabei-L Tenperature Sensor device.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "proto.h"
#include "nvmonvars.h"
#include "error.h"
#include "menu.h"
#include "common.h"
#include "common_utils.h"
#include "mb_tests.h"
#include "diag_temp_snsr_test.h"
#include "defs.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "common_utils.h"
#include "byteswap.h"
#include "dash_fpga.h"
#include "diag_fpga.h"
#include "dnv_gpio_lib.h"
/******************************************************************************
 *                                   Externs
 ******************************************************************************/

/****************************************************************************** 
 *                               Function prototypes
 ******************************************************************************/
static uint32 show_temperature(int);
static uint32 register_test(int);
static uint32 alter_ts_reg(void);
static uint32 show_ts_reg(void);
uint32 show_temperature_all(void);
uint32 register_test_all(void); 
uint32 interrupt_test_all(void); 
int temp_sensor_int_test(int);

/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/

/*
 * Temperature sensor registers test table. This table is only used for register test.
 */

static char ts_name_phoenix[4][16] =
{
    "Inlet (0x90)",
    "Outlet (0x94)",
    "Inlet (0x92)",
    "Outlet (0x96)",
};

/* Temperature sensor registers table. This device has registers with different sizes.
 */
static reg_info_t ts_reg_table[] =
{
/*  { name, offset, rw type, 
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
 * Sub Menu used for MB Temperature Sensor tests
 */
submenu_xtable_t diag_ts_submenu_table[] = {
    {"Show temperature", (PFT)show_temperature_all, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0,(PFT)show_temperature_all, 0},
    {"Register test", (PFT)register_test_all, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT)register_test_all, 0},
    {"Interrupt test", (PFT)interrupt_test_all, 0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT)interrupt_test_all, 0},
    {"Show register", (PFT)show_ts_reg, 0, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, 0},
    {"Alter register", (PFT)alter_ts_reg, 0, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, 0},
};

#define MB_TS_SUBMENU_TABLE_SIZE (sizeof(diag_ts_submenu_table) / \
                                    sizeof(submenu_xtable_t))

static mitem_t diag_ts_pri_items[MB_TS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t diag_ts_sec_items[MB_TS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo diag_ts_submenu = {
    "%s Menu",               /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    diag_ts_pri_items,
};

static struct menuinfo *diag_ts_submenup = &diag_ts_submenu;

/*******************************************************************************
 *
 * Function   : diag_temp_sensor_test
 * Description: Entry function of MB Temperature Sensor test
 * Inputs     : Test/Menu
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_snsr_menu (boolean diag_ts_items_executed)
{
    char *tname = "Temperature Sensor";

    testname(tname);

    build_primary_submenu(diag_ts_submenu_table,
                          MB_TS_SUBMENU_TABLE_SIZE, "Temperature Sensor",
                          &diag_ts_submenup);

    build_secondary_submenu(diag_ts_submenu_table,
                            MB_TS_SUBMENU_TABLE_SIZE,
                            diag_ts_sec_items);

    if (diag_ts_items_executed) {
        menu(&diag_ts_submenu, diag_ts_sec_items, 0);
    } else {
        do_all_menu_items(diag_ts_submenup);
    }

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
static int set_i2c_if_struct (uint32_t ts_id, n2g_i2c_if_t* i2c_if_p, 
                              int offset, char* buf_p, int size)
{
    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
	i2c_if_p->i2c_bus_type = IOFPGA_I2C;
	i2c_if_p->i2c_ctrl = I2C_CTRL_TWO;
	i2c_if_p->mux = I2C_MUX_ZERO;
	i2c_if_p->offset = offset;
	i2c_if_p->size = size;
	i2c_if_p->buf = buf_p;
    switch (ts_id) {
        case TS_INLET_SIDE0:
            i2c_if_p->i2c_dev = TS_INLET_ADDR0;
            break;
        case TS_INLET_SIDE1: 
            i2c_if_p->i2c_dev = TS_INLET_ADDR1; 
            break;
        case TS_OUTLET_SIDE0: 
            i2c_if_p->i2c_dev = TS_OUTLET_ADDR0; 
            break;
        case TS_OUTLET_SIDE1:
            i2c_if_p->i2c_dev = TS_OUTLET_ADDR1; 
            break;
        default:
            printf("No Match Thermal sensor.\n");
            return (FAILED);
    }       
    return (PASSED);
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
static int read_ts_reg (int ts_id, int addr_ptr_id, char * data_buf_p, int size)
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
static int write_ts_reg (int ts_id, int addr_ptr_id, char * data_buf_p, int size)
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
    msleep(WAITTIME_100_MS); /* sleep 100 ms after writing */
    return(rc);
}


/******************************************************************************
 *
 * function   : register_test
 * Description: A quick register read and write access test to device's R/W register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static uint32 register_test (int ts_id)
{
	int offset;
	ts_t backup_buf, new_buf, compare_buf;
	uint32_t rc = FAILED;
    prpass(testpass, "Register test ");
	for (offset = TS_PTR_THYST; offset <= TS_PTR_TOS; offset++) {
        /* Clear buff */
		backup_buf = new_buf = compare_buf = 0; 
		/* read and backup reg data */
		rc = read_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d backup old value: read_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
		/* write new data */
		new_buf = backup_buf + TS_TEST_PATTERN;
		rc = write_ts_reg (ts_id, offset, (char*) &new_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d write new value: write_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
		/* read and compare reg data */
		rc = read_ts_reg (ts_id, offset, (char*) &compare_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d read new value: read_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
		if (compare_buf != new_buf) {
			cterr('f', 0, "%s:%d compare result: value not matched (%#x != %#x)\n",
                      __FUNCTION__, __LINE__, new_buf, compare_buf);
			return(FAILED);
		}

		/* restore original data */
		rc = write_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t)); 
		if (rc != PASSED) {
			cterr('f', 0, "%s:%d restore old value: write_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
			return (FAILED);
		}
    }
    printf("%s(%d): Passed\n", __FUNCTION__, ts_id);
    prcomplete(testpass, errcount, (char *)0);
    return(rc);
}
/******************************************************************************
 *
 * function   : register_test_all
 * Description: A quick register read and write access test for all device's R/W register.
 * Inputs     : void
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
uint32 register_test_all(void) 
{
    int ix, rc, test_end;
    
    test_end = TS_OUTLET_SIDE1;
    for (ix = TS_INLET_SIDE0; ix <= test_end; ix++) {
        rc = register_test(ix);
        if (rc != PASSED) {
            cterr('f', 0, "Failed to thermal sensor register test \n"); 
            return (FAILED);
        }
    }
   
    return (PASSED);
}

/******************************************************************************
 *
 * Function   : show_temperature
 * Description: show temperature of the selected sensor
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32 show_temperature (int ts_id)
{
    uint32 rc;
    ts_t val;
    char ts_name[4][16];

    memset(ts_name, 0, sizeof(ts_name));
    memcpy(ts_name, ts_name_phoenix, sizeof(ts_name_phoenix));

    rc = read_ts_reg (ts_id, TS_PTR_TEMP, (char*) &val, sizeof(ts_t));
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                      __FUNCTION__, __LINE__, ts_id);
        return (FAILED);
    }
    val = (((val & 0xff00) >> 8) | ((val & 0x00ff) << 8));
    if (val <= TS_TEMP_MAX) {
        printf("%s temperature : %.4f Celcius\n", \
                            ts_name[ts_id], (val >> 4) * TS_TEMP_RESOLUTION);
    } else {
        printf("%s temperature : %.4f Celcius\n", \
                            ts_name[ts_id], ((val >> 4) - 4096) * TS_TEMP_RESOLUTION);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : show_temperature_all
 * Description: show temperature of all the sensor
 * Inputs     : void
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/

uint32 show_temperature_all (void) 
{
    int ix, rc, test_end;
    
    /* show_temperature_all() is called after disable nios, 
     * to read cpu temperature from FPGA, we need to disable nios, too. 
     */
    test_end = TS_OUTLET_SIDE1;
    for (ix = TS_INLET_SIDE0; ix <= test_end; ix++) {
       rc = show_temperature(ix);
       if (rc != PASSED) {
           cterr('f', 0, "Failed to display temperature \n"); 
           return (FAILED);
       }
   }
   
    return (PASSED);
}

/******************************************************************************
 *
 * Function   : temp_sensor_int_test
 * Description: Function to execute Temperature Sensor Interrupt Test
 * Inputs     : ts_id
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/

int temp_sensor_int_test (int ts_id) 
{
	int ix, max_retry = 20, rc1 = 0, rc2 = 0;
    uint gpio_value;
    ts_t  reg_val = 0, reg_val2 = 0, tos_val = 0,  thyst_val = 0;

    unsigned char ts_gpio_pin = DNV_GPIO_5;
	rc1 = read_ts_reg (ts_id, TS_PTR_TOS, (char*) &tos_val, sizeof(ts_t)); 
    rc2 = read_ts_reg (ts_id, TS_PTR_THYST, (char*) &thyst_val, sizeof(ts_t));
    if ((rc1 != PASSED) || (rc2 != PASSED)) {
        printf("Can not read register\n");
        return (FAILED);
    }

    if (dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value) != PASSED) {
        cterr('f', 0, "Can not read CPU GPIO");
        return (FAILED);
    }

    /* Check if interrupt is already asserted before the test */
    if (gpio_value == GPIO_LOW) {
        printf("Temperature Sensor Interrupt is already asserted\n");
        reg_val = CLEAR_INTERRUPT_LM75;
        reg_val2 = CLEAR_INTERRUPT_LM75;
     
        rc1 = write_ts_reg (ts_id, TS_PTR_TOS, (char*) &reg_val, sizeof(ts_t)); 
        rc2 = write_ts_reg (ts_id, TS_PTR_THYST, (char*) &reg_val2, sizeof(ts_t));
        if ((rc1 != PASSED) || (rc2 != PASSED)) {
            printf("Clear Interrupt failed\n");
     
        }	  
        prpass(testpass, "Check CPU Interrupt again(Before the test)");
     	
        for (ix = 0; ix < max_retry; ix++) {
            rc1 = dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value);
            if (rc1 != PASSED) {
                cterr('f', 0, "Read GPIO Value Fails");
            }
     
            if (gpio_value == GPIO_HIGH) {
                break;
            }
            msleep (POLL_DELAY);
        }
     
        if (ix == max_retry) {
            cterr('f', 0, "Interrupt is not clear");
            return (FAILED);
        }

    }
    prpass(testpass, "Enable and Force Interrupt");
    reg_val = FORCE_INTERRUPT_LM75;
    reg_val2 = FORCE_INTERRUPT_LM75;
    rc1 = write_ts_reg (ts_id, TS_PTR_TOS, (char*) &reg_val, sizeof(ts_t)); 
    rc2 = write_ts_reg (ts_id, TS_PTR_THYST, (char*) &reg_val2, sizeof(ts_t));
    if ((rc1 != PASSED) || (rc2 != PASSED)) {
        printf("Enable and Force Interrupt failed\n");
        return (FAILED);

    }	  

    prpass(testpass, "Check CPU Interrupt (After the test)");
    for (ix = 0; ix < max_retry; ix++) {
        rc1 = dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value);
        if (rc1 != PASSED) {
            cterr('f', 0, "Read GPIO Value Fails");
            return (FAILED);
        }

        if (gpio_value == GPIO_LOW) {
            break;
        }
        msleep (100);
    }

    if (ix == max_retry) {
        cterr('f', 0, "Interrupt is not detected");
        return (FAILED);
    }
    /* Disable Interrupt */
    prpass(testpass, "Disable Interrupt");
    rc1 = write_ts_reg (ts_id, TS_PTR_TOS, (char*) &tos_val, sizeof(ts_t)); 
    rc2 = write_ts_reg (ts_id, TS_PTR_THYST, (char*) &thyst_val, sizeof(ts_t));
    if ((rc1 != PASSED) || (rc2 != PASSED)) {
      printf("Disable Interrupt failed\n");
      return (FAILED);
   }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);

}
/******************************************************************************
 *
 * Function   : interrupt_test_all
 * Description: do all the interrupt test
 * Inputs     : void
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/

uint32 interrupt_test_all (void) 
{
    int ix, rc, test_end;
    
    test_end = TS_OUTLET_SIDE1;
    for (ix = TS_INLET_SIDE0; ix <= test_end; ix++) {
        rc = temp_sensor_int_test(ix); 
        if (rc != PASSED) {
            cterr('f', 0, "Failed in Interrupt test\n"); 
            return (FAILED);
        }
   }
   
    return (PASSED);
}

/********************************************************************
 *
 * Function:	show_ts_reg
 * Description:	show temperature sensor register.
 * Inputs:	-
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *
 *********************************************************************
 */
static uint32 show_ts_reg(void)
{
    int size, offset, ts_id;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t buffer = 0;
    
    reg_p = &ts_reg_table[0];
    ts_id = gethex_answer("Enter the thermal sensor number:\n"
            "(0.Inlet(0x90), 1.Outlet(0x94), 2.Inlet(0x92), 3.Outlet(0x96))",
             TS_INLET_SIDE0, TS_INLET_SIDE0, TS_OUTLET_SIDE1);
    printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
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
        buffer = (((buffer & 0xff00) >> 8) | ((buffer & 0x00ff) << 8));
        if (rc != PASSED) {
             cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                    __FUNCTION__, __LINE__, ts_id);
             return (FAILED);
        } else {
             printf("%-36s (0x%02X), data: 0x%04X.\n", reg_p->name,
             reg_p->offset, buffer);
        }
    }
    return (PASSED);
}


/********************************************************************
 *
 * Function:	alter_ts_reg
 * Description:	alter temperature sensor register.
 * Inputs:	-
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32 alter_ts_reg(void)
{
    int size, offset, ts_id;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t temp_data, old_temp_data;
    ts_c cfg_data, old_cfg_data;
    
    reg_p = &ts_reg_table[0];
    ts_id = gethex_answer("Enter the thermal sensor number:\n"
            "(0.Inlet(0x90), 1.Outlet(0x94), 2.Inlet(0x92), 3.Outlet(0x96))",
            TS_INLET_SIDE0, TS_INLET_SIDE0, TS_OUTLET_SIDE1);
    printf("%s(): ts_id %d\n", __FUNCTION__, ts_id);
   
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
         reg_p++; /* update the register table pointer */
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
                  cfg_data = gethex_answer("Enter the data:", old_cfg_data, 0, 0x1F); 
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
                   cterr('f', 0, "%s:%d restore old value: write_ts_reg failed\n", __FUNCTION__, __LINE__); 
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
    return (PASSED);
}

