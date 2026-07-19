 /* $Id: diag_temp_snsr_test.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_temp_snsr_test.c,v $
 *------------------------------------------------------------------
 * Filename:  diag_temp_snsr_test.c
 *
 * Description: Tenperature Sensor device.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "dnv_gpio_lib.h"

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
static int temp_sensor_int_test (int);
uint32 show_temperature_all(void);

/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/

/*
 * Temperature sensor registers test table. This table is only used for register test.
 */

static char ts_name[2][32] =
{
    "Thermal Sensor 1 (0x94)",
    "Thermal Sensor 2 (0x96)",
};

/* Temperature sensor registers table. This device has registers with different sizes.
 */
static reg_info_t ts_reg_table[] =
{
/*  { name, offset, rw type, 
 *    size, mask, default value},
 */
    {"Temperature",        TS_PTR_TEMP,    READ_ONLY,
    {TS_PTR_TEMP_L},    0xFF80, 0x0000},
    {"Configuration",    TS_PTR_CFG,    READ_WRITE,
    {TS_PTR_CFG_L},    0x1F, 0x00},
    {"Hysteresis Threshold",    TS_PTR_THYST,    READ_WRITE,
    {TS_PTR_THYST_L},    0xFF80, 0x0000},
    {"Overtemperature Shutdown",    TS_PTR_TOS,    READ_WRITE,
    {TS_PTR_TOS_L},    0xFF80, 0x0000},
    {"One-shot mode",    TS_PTR_OS,    READ_WRITE,
    {TS_PTR_OS_L},    0xFF80, 0x0000},
    {0, 0, 0, {0}, 0, 0},
};

/******************************************************************************
 *                                   Menus
 ******************************************************************************/
/* 
 * Sub Menu used for MB Temperature Sensor tests
 */
submenu_xtable_t diag_ts_submenu_table[] = {
    /* TS1 (0x94) */
    {"TS1 Inlet (0x94)-Show temperature", (PFT)show_temperature, 
    TS_INLET_SIDE0, MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT)show_temperature, TS_INLET_SIDE0},
    {"TS1 Inlet (0x94)-Register sanity test", (PFT)sanity_test, TS_INLET_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_INLET_SIDE0},
    {"TS1 Inlet (0x94)-Show register", (PFT)show_ts_reg, TS_INLET_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_INLET_SIDE0},
    {"TS1 Inlet (0x94)-Alter register", (PFT)alter_ts_reg, TS_INLET_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_INLET_SIDE0},
    {"TS1 Inlet (0x94)-Interrupt test", (PFT)temp_sensor_int_test, TS_INLET_SIDE0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (PFT)temp_sensor_int_test, TS_INLET_SIDE0},

    /* TS2 (0x96) */
    {"TS2 Outlet (0x96)-Show temperature", (PFT)show_temperature, 
    TS_OUTLET_SIDE0, MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT)show_temperature, TS_OUTLET_SIDE0},
    {"TS2 Outlet (0x96)-Register sanity test", (PFT)sanity_test, TS_OUTLET_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_OUTLET_SIDE0},
    {"TS2 Outlet (0x96)-Show register", (PFT)show_ts_reg, TS_OUTLET_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_OUTLET_SIDE0},
    {"TS2 Outlet (0x96)-Alter register", (PFT)alter_ts_reg, TS_OUTLET_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_OUTLET_SIDE0},
    {"TS2 Outlet (0x96)-Interrupt test", (PFT)temp_sensor_int_test, TS_OUTLET_SIDE0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (PFT)temp_sensor_int_test, TS_OUTLET_SIDE0},
    {"Show all temperatures", (PFT)show_temperature_all, 0, 0,
     (type_t(*)())0, 0, (PFT)show_temperature_all, 0},
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
 *                 offset: i2c device offset value
 *              buf_p: pointer to i2c tx/rx buffer
 *                 size: size of beffer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int 
set_i2c_if_struct (uint32_t ts_id, n2g_i2c_if_t* i2c_if_p, int offset, char* buf_p, int size)
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
        case TS_OUTLET_SIDE0:
             i2c_if_p->i2c_dev = TS_OUTLET_ADDR0; 
             break;
        default:
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
 *                 data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
read_ts_reg (int ts_id, int addr_ptr_id, ushort * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    
    /* Setup I2C API interface struct */
    set_i2c_if_struct(ts_id, &i2c_if, addr_ptr_id, (char*)data_buf_p, size);
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
 *                 data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
write_ts_reg (int ts_id, int addr_ptr_id, ushort * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API interface struct */
    set_i2c_if_struct(ts_id, &i2c_if, addr_ptr_id, (char*)data_buf_p, size);
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to write offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, addr_ptr_id, size, rc);
        return (FAILED);
    }
    usleep(100000); /* sleep 100 ms after writing */
    
    if (diagflag_xram & D_TRACE) {
        ushort tmp;
        read_ts_reg(ts_id, addr_ptr_id, (ushort*)&tmp, size);
    
        if(*data_buf_p != tmp)
            printf("\n[DBG] write_ts_reg %x / %x\n", *data_buf_p, tmp);
    }
	
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
static uint32 sanity_test (int ts_id)
{
    //printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
    int offset;
    ts_t backup_buf, new_buf, compare_buf;
    uint32_t rc = FAILED;
    for (offset = TS_PTR_THYST; offset <= TS_PTR_TOS; offset++) {
        backup_buf = new_buf = compare_buf = 0; // clear
        // read and backup reg data
        rc = read_ts_reg (ts_id, offset, &backup_buf, sizeof(ts_t)); 
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d backup old value: read_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }
        // write new data
        new_buf = backup_buf + TS_TEST_PATTERN;
        rc = write_ts_reg (ts_id, offset, &new_buf, sizeof(ts_t)); 
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d write new value: write_ts_reg failed\n",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }
        // read and compare reg data
        rc = read_ts_reg (ts_id, offset, &compare_buf, sizeof(ts_t)); 
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
        rc = write_ts_reg (ts_id, offset,  &backup_buf, sizeof(ts_t)); 
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
static uint32 show_temperature (int ts_id)
{
    //printf("%s(): ts_id %d\n",__FUNCTION__, ts_id);
    uint32 rc;
    ts_t val;

    rc = read_ts_reg (ts_id, TS_PTR_TEMP, &val, sizeof(ts_t));
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

uint32 show_temperature_all(void) 
{
    int i, rc;
    
    /* show_temperature_all() is called after disable nios, 
     * to read cpu temperature from FPGA, we need to disable nios, too. 
     */

    for (i = TS_INLET_SIDE0; i <= TS_OUTLET_SIDE0; i++) {
        rc = show_temperature(i);
        if (rc != PASSED) {
            printf("Failed to display temperature \n"); 
            return (FAILED);
        }
    }
    printf("\n");
    
    return (PASSED);
}

/********************************************************************
 *
 * Function:    show_ts_reg
 * Description:    show temperature sensor register.
 * Inputs:    -
 * Outputs:    PASSED - No errors encounterd.
 *        FAILED - Errors encounterd.
 *
 *
 *********************************************************************
 */
static uint32 show_ts_reg(int ts_id)
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
        rc = read_ts_reg (ts_id, offset, (ushort*)buf_p, size);
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
 * Function:    alter_ts_reg
 * Description:    alter temperature sensor register.
 * Inputs:    -
 * Outputs:    PASSED - No errors encounterd.
 *        FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32 alter_ts_reg(int ts_id)
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
        reg_p++;    /* update the register table pointer */
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
             rc = read_ts_reg (ts_id, offset, (ushort*)buf_p, size); 
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
             rc = write_ts_reg (ts_id, offset, (ushort*)buf_p, size); 
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

/*******************************************************************************
 *
 * Function    : temp_sensor_int_test
 * Description : Function to execute Temperature Sensor Interrupt Test
 * Inputs      : ts_id - sensor id
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static int temp_sensor_int_test (int ts_id)
{
    int ix, max_retry = 20, rc = 0, rc1 = 0, rc2 = 0;
    uint gpio_value;
    ts_t reg_val = 0, reg_val2 = 0, tos_val = 0, thyst_val = 0, conf_val = 0, reg_val3 = 0;
    ts_t tmp_val = 0, tmp_val1, tmp_val2 = 0;
    unsigned char ts_gpio_pin;
    
    if (ts_id == TS_INLET_SIDE0) {
        ts_gpio_pin = DNV_GPIO_5;
    } else if (ts_id == TS_OUTLET_SIDE0) {
        ts_gpio_pin = DNV_GPIO_6;
    }

    testname("Temperature Sensor Interrupt");

    rc = read_ts_reg(ts_id, TS_PTR_TOS, &tos_val, sizeof(ts_t));
    rc1 = read_ts_reg(ts_id, TS_PTR_THYST, &thyst_val, sizeof(ts_t));
    rc2 = read_ts_reg(ts_id, TS_PTR_CFG, &conf_val, sizeof(ts_t));
    if (diagflag_xram & D_TRACE) {
        printf("[DBG]TOS=%x, THYST=%x, CONF=%x\n", tos_val, thyst_val, conf_val);
    }
    
    /* Set to Default Value */
    reg_val = DEFAULT_TOS_LM75;
    reg_val2 = DEFAULT_THYST_LM75;
    reg_val3 = DEFAULT_CONF_LM75;
    write_ts_reg (ts_id, TS_PTR_TOS, &reg_val, sizeof(ts_t));
    write_ts_reg (ts_id, TS_PTR_THYST, &reg_val2, sizeof(ts_t));
    write_ts_reg (ts_id, TS_PTR_CFG, &reg_val3, sizeof(ts_t));
    
    if ((rc != PASSED) || (rc1 != PASSED) || (rc2 != PASSED)) {
        printf("Can not read THYST register\n");
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
       
        rc = write_ts_reg (ts_id, TS_PTR_TOS, &reg_val, sizeof(ts_t));
        rc1 = write_ts_reg (ts_id, TS_PTR_THYST, &reg_val2, sizeof(ts_t));
        read_ts_reg(ts_id, TS_PTR_CFG, &tmp_val2, sizeof(ts_t));
        
        read_ts_reg(ts_id, TS_PTR_TOS, &tmp_val, sizeof(ts_t));
        read_ts_reg(ts_id, TS_PTR_THYST, &tmp_val1, sizeof(ts_t));
        read_ts_reg(ts_id, TS_PTR_CFG, &tmp_val2, sizeof(ts_t));
        if (diagflag_xram & D_TRACE) {
            printf("[DBG]After clear interrupt value= %x/%x/%x\n", tmp_val, tmp_val1, tmp_val2);
        }
        
        if ((rc != PASSED) || (rc1 != PASSED)) {
            printf("Clear Interrupt failed\n");
            return (FAILED);
        }

        prpass(testpass, "Check CPU Interrupt again(Before the test)");
     
        for (ix = 0; ix < max_retry; ix++) {
            rc = dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value);
            if (rc != PASSED) {
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
    
    rc = write_ts_reg (ts_id, TS_PTR_TOS, &reg_val, sizeof(ts_t));
    rc1 = write_ts_reg (ts_id, TS_PTR_THYST, &reg_val2, sizeof(ts_t));
    if ((rc != PASSED) || (rc1 != PASSED)) {
        printf("Enable and Force Interrupt failed\n");
        return (FAILED);
    }

    prpass(testpass, "Check CPU Interrupt (After the test)");
     
    for (ix = 0; ix < max_retry; ix++) {
        rc = dnv_gpio_read_rx_val(ts_gpio_pin, &gpio_value);
        if (rc != PASSED) {
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
    rc = write_ts_reg (ts_id, TS_PTR_TOS, &tos_val, sizeof(ts_t));
    rc1 = write_ts_reg (ts_id, TS_PTR_THYST, &thyst_val, sizeof(ts_t));
    rc2 = write_ts_reg (ts_id, TS_PTR_CFG, &conf_val, sizeof(ts_t));
    if ((rc != PASSED) || (rc1 != PASSED) || (rc2 != PASSED)) {
        printf("Disable Interrupt failed\n");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * function   : read_ts_nios_reg
 * Description: Wrapper to read temperature sensor's register from NIOS.
 * Inputs     : ts_id - sensor ID
 *                 data_buf_p - pointer to data buffer
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
read_ts_nios_reg (int ts_id, ushort * data_buf_p)
{
    uint32_t rc = PASSED;
    volatile uint16_t *nios_reg;
    
    switch (ts_id) {
        case TS_INLET_SIDE0: 
            nios_reg = (volatile uint16_t *)(dash_fpga + NIOS_MAILBOX_TEMP_INLET_OFFSET);
            break;
        case TS_OUTLET_SIDE0:
            nios_reg = (volatile uint16_t *)(dash_fpga + NIOS_MAILBOX_TEMP_OUTLET_OFFSET);
            break;
        case TS_CPU:
            nios_reg = (volatile uint16_t *)(dash_fpga + NIOS_MAILBOX_TEMP_CPU_OFFSET);
            break;
        default:
            return (FAILED);
    }
    
    *data_buf_p = *nios_reg;
    
    return (rc);
}


/*********************************************************************
 *
 * Function:    nios_test_temp_reg
 *
 * Description:    Check Temperature Sensor registers set.
 *
 * Inputs:    none
 *
 * Outputs:    PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int nios_test_temp_reg(void)
{
    uint32 rc = PASSED;
    ts_t val;
    int ix, temp, max, min;
    char *sensor_str;
    char err_buf[256];

    for (ix = TS_INLET_SIDE0; ix <= TS_CPU; ix++) {
        rc = read_ts_nios_reg (ix, &val);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                          __FUNCTION__, __LINE__, ix);
            return (FAILED);
        }
        temp = val;
        
        if (ix == TS_INLET_SIDE0) {
            min = NIOS_MAILBOX_TEMP_INTEL_MIN;
            max = NIOS_MAILBOX_TEMP_INTEL_MAX;
            sensor_str = "Inlet";
        } else if (ix == TS_OUTLET_SIDE0) {
            min = NIOS_MAILBOX_TEMP_OUTTEL_MIN;
            max = NIOS_MAILBOX_TEMP_OUTTEL_MAX;
            sensor_str = "Outlet";
        } else {
            min = NIOS_MAILBOX_TEMP_CPU_MIN;
            max = NIOS_MAILBOX_TEMP_CPU_MAX;
            sensor_str = "CPU";
        }
        
        if ((temp > max) || (temp < min)) {
            sprintf(err_buf, "%s Sensor temperature : Max = %#x "
                 "Min = %#x. Current = %#x.\n",
                                 sensor_str, max, min, temp);
            rc = FAILED;
            break;
        }

        if (diagflag_xram & D_TRACE) {
            printf("%s Sensor temperature\t: Max = %#x Min = %#x. Current = %#x. pass\n", 
                                 sensor_str, max, min, temp);
        }
    }
    
    if (rc != PASSED) {
        cterr('f', 0, err_buf);
    }
    return rc;
}

/*------------------------------------------------------------------
$Log: diag_temp_snsr_test.c,v $
Revision 1.3  2020/04/20 02:28:24  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/
