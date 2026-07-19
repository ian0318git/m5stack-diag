 /* $Id: diag_press_sensor_test.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_press_sensor_test.c,v $
 *------------------------------------------------------------------
 * Filename:  diag_press_sensor_test.c
 *
 * Description: Pressure Sensor device.
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
#include "diag_press_sensor_test.h"
#include "defs.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "common_utils.h"
#include "byteswap.h"
#include "dash_fpga.h"
#include "diag_i2c_addr.h"
#include "dnv_gpio_lib.h"

/******************************************************************************
 *                                   Externs
 ******************************************************************************/

/****************************************************************************** 
 *                               Function prototypes
 ******************************************************************************/
static uint32 show_pressure (void);
static uint32 sanity_test (void);
int interrupt_test(void);
//static uint32 alter_ps_reg(void);
//static uint32 show_ps_reg(void);

/****************************************************************************** 
 *                                 Global variables
 ******************************************************************************/
static int is_Init = FALSE;
 

/******************************************************************************
 *                                   Menus
 ******************************************************************************/
/* 
 * Sub Menu used for MB Temperature Sensor tests
 */
submenu_xtable_t diag_ps_submenu_table[] = {
    
    /* Outlet (0x94) */
    {"Show Presure", (PFT)show_pressure, 
     0, MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT)show_pressure, 0},
    {"Register sanity test", (PFT)sanity_test,
     0, (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (PFT)sanity_test, 0},
    {"Interrupt test", (PFT)interrupt_test, 
     0, (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0, 0, (PFT)interrupt_test, 0},
};

#define MB_PS_SUBMENU_TABLE_SIZE (sizeof(diag_ps_submenu_table) / \
                                    sizeof(submenu_xtable_t))

static mitem_t diag_ps_pri_items[MB_PS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t diag_ps_sec_items[MB_PS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo diag_ps_submenu = {
    "%s Menu",               /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    diag_ps_pri_items,
};

static struct menuinfo *diag_ps_submenup = &diag_ps_submenu;

/*******************************************************************************
 *
 * Function   : diag_press_sensor_test
 * Description: Entry function of MB Temperature Sensor test
 * Inputs     : Test/Menu
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_ps_menu (boolean diag_ps_items_executed)
{
    char *tname = "Pressure Sensor";

    testname(tname);

    build_primary_submenu(diag_ps_submenu_table,
                          MB_PS_SUBMENU_TABLE_SIZE, "Pressure Sensor",
                          &diag_ps_submenup);

    build_secondary_submenu(diag_ps_submenu_table,
                            MB_PS_SUBMENU_TABLE_SIZE,
                            diag_ps_sec_items);

    if (diag_ps_items_executed) {
        menu(&diag_ps_submenu, diag_ps_sec_items, 0);
    } else {
        do_all_menu_items(diag_ps_submenup);
    }

    return (PASSED);
}


/*****************************************************************************
 *
 * Function   : set_i2c_if_struct
 * Description: fill n2g_i2c_if_t struct based on different mux_id.
 * Inputs     : i2c_if_p: pointer to n2g_i2c_if_t struct
 *              offset: i2c device offset value
 *              buf_p: pointer to i2c tx/rx buffer
 *              size: size of beffer
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************************/
static int 
set_i2c_if_struct (n2g_i2c_if_t* i2c_if_p, int offset, char* buf_p, int size)
{
    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
    i2c_if_p->i2c_ctrl = I2C_CTRL_TWO;
    i2c_if_p->mux = I2C_MUX_ZERO;
    i2c_if_p->offset = offset;
    i2c_if_p->size = size;
    i2c_if_p->buf = buf_p; 
    i2c_if_p->i2c_dev = MB_I2C_ADDR_PRESSURE; 
    return (PASSED);
}   


/******************************************************************************
 *
 * function   : read_ps_reg
 * Description: Wrapper to read pressure sensor's register.
 * Inputs     : addr_ptr_id - pointer register ID.
 *              data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
read_ps_reg (int addr_ptr_id, char * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API interface struct */
    set_i2c_if_struct(&i2c_if, addr_ptr_id, data_buf_p, size);
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
 * function   : write_ps_reg
 * Description: Wrapper to write pressure sensor's register.
 * Inputs     : addr_ptr_id - pointer register ID.
 *              data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
write_ps_reg (int addr_ptr_id, char * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;

    /* Setup I2C API interface struct */
    set_i2c_if_struct(&i2c_if, addr_ptr_id, data_buf_p, size);
    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to write offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, addr_ptr_id, size, rc);
        return (FAILED);
    }
    usleep(100000); /* sleep 100 ms after writing */
    
    if (diagflag_xram & D_TRACE) {
        uchar tmp;
        read_ps_reg(addr_ptr_id, (char*)&tmp, size);

        if((uchar)*data_buf_p != tmp)
            printf("[DBG] write_ps_reg %x / %x\n", (uchar)*data_buf_p, tmp);
    }

    return(rc);
}

/******************************************************************************
 *
 * function   : sanity_test
 * Description: A quick sanity read and write access test to device's R/W register.
 * Inputs     : none
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static uint32 sanity_test (void)
{
    int offset;
    uint16_t backup_buf, new_buf, compare_buf;
    uint32_t rc = FAILED;
    for (offset = PS_REF_P_XL; offset <= PS_REF_P_H; offset++) {
        backup_buf = new_buf = compare_buf = 0; // clear
        // read and backup reg data
        rc = read_ps_reg (offset, (char*)&backup_buf, PS_ONE_BYTE); 
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d backup old value: read_ps_reg failed\n",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }

        // write new data
        new_buf = backup_buf + PS_TEST_PATTERN;

        rc = write_ps_reg (offset, (char*)&new_buf, PS_ONE_BYTE); 
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d write new value: write_ps_reg failed\n",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }
        // read and compare reg data
        rc = read_ps_reg (offset, (char*)&compare_buf, PS_ONE_BYTE);

        if (rc != PASSED) {
            cterr('f', 0, "%s:%d read new value: read_ps_reg failed\n",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }
        if (compare_buf != new_buf) {
            cterr('f', 0, "%s:%d compare result: value not matched (%#x != %#x)\n",
                      __FUNCTION__, __LINE__, new_buf, compare_buf);
            return(FAIL);
        }

        /* restore original data */
        rc = write_ps_reg (offset,  (char*)&backup_buf, PS_ONE_BYTE); 
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d restore old value: write_ps_reg failed\n",
                      __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function   : show_pressure
 * Description: show pressure of the Pressure sensor
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32 show_pressure (void)
{
    uchar buf_p;
    uint32_t hPa = 0;
    int ix;
    
    for (ix = 0; ix < PS_POLL_TIME_OUT; ix++) {
        /* read pressure data ready bit */
        read_ps_reg (PS_STATUS_REG, (char*)&buf_p, PS_ONE_BYTE);

        if (buf_p & 0x02) {
            read_ps_reg (PS_PRESS_OUT_XL, (char*)&buf_p, PS_ONE_BYTE);
            hPa = buf_p;
            read_ps_reg (PS_PRESS_OUT_L, (char*)&buf_p, PS_ONE_BYTE);
            hPa |= (buf_p << 8);
            read_ps_reg (PS_PRESS_OUT_H, (char*)&buf_p, PS_ONE_BYTE);
            hPa |= (buf_p << 16);
    
            hPa = hPa/4096;
            printf("Current Pressure is %d hPa\n", hPa);
            break;
        } else {
            if (ix == PS_POLL_TIME_OUT -1) {
                printf("Can't get current pressure!\n");
                return (FAILED);
            }
            /* max data update frequence 25hz */
            msleep(40);
        }
    }

    return (PASSED);
}

#if 0
/********************************************************************
 *
 * Function:	show_ps_reg
 * Description:	show pressure sensor register.
 * Inputs:	-
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *
 *********************************************************************
 */
static uint32 show_ps_reg(void)
{
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    uint16_t buffer = 0;
    
    reg_p = &ps_reg_table[0];
    
    printf("\nRegister number:\n");
    while (reg_p->name) {
        printf("   %02x - %s\n", reg_p->offset, reg_p->name);
        reg_p++;
    } 

    /* Get the register to be read */
    offset = gethex_answer("Enter the register number:", 0, 0, 0x3A);
    
    reg_p = &ps_reg_table[0]; /* Points to the beginning of the table */
    while (reg_p->name && reg_p->offset != offset) {
         /* Not requested register */
         reg_p++;
    }
    
    if (reg_p->name) {
        /* Read the register. */
        size = PS_ONE_BYTE;
        #if 0
        if (reg_p->size.size == sizeof(ts_c)) {
            /* Configuration register */
            size = sizeof(ts_c);
        } else {
            /* Temperature registers */
            size = sizeof(ts_t);
        }
        #endif 

        buf_p = (char *)&buffer;
        offset = reg_p->offset;
        rc = read_ps_reg (offset, (char*)&buf_p, size);
        if (rc != PASSED) {
             cterr('f', 0, "%s:%d Failed to read Pressure sensor",
                    __FUNCTION__, __LINE__);
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
 * Function:	alter_ps_reg
 * Description:	alter pressure sensor register.
 * Inputs:	-
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32 alter_ps_reg(void)
{
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    uint16_t temp_data, old_temp_data;
    
    reg_p = &ps_reg_table[0];
    
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
    offset = gethex_answer("Enter the register number:", 0, 0, 0x3A);
    
    /* Check if the register is read/writeable */
    /* Find the register text in the register table */
    reg_p = &ps_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != offset) {
         /* Not requested register */
         reg_p++; /* update the register table pointer */
    }
    
    if (reg_p->name) {
         /* Got the register. */
         if (reg_p->type == READ_WRITE) {
             /* Read/Writeable. Read the register first. */
             size = PS_ONE_BYTE; 
             buf_p = (char *)&old_temp_data;

             offset = reg_p->offset; 

             /* Configuration register */ 
             rc = read_ps_reg (offset, (char*)&buf_p, size); 
             if (rc != PASSED) { 
                  cterr('f', 0, "%s:%d Failed to read temperature sensor",
							  __FUNCTION__, __LINE__);
                  return (FAILED); 
             }

             /* Get the new data */ 
             temp_data = gethex_answer("Enter the data:", 
             old_temp_data, 0, 0x1F); 
             buf_p = (char *)&temp_data; 

             /* Write the new data */ 
             rc = write_ps_reg (offset, (char*)buf_p, size); 
             if (rc != PASSED) { 
                   cterr('f', 0, "%s:%d restore old value: write_ps_reg failed\n", __FUNCTION__, __LINE__); 
                   return (FAILED); 
              } 
          } else { 
               /* Not read/writeable */ 
               printf("alter_ps_reg() %s is a read only register", reg_p->name); 
               return (FAILED); 
          }
    } else { 
          /* Invalid register */ 
          printf("alter_ps_reg() %#x is not a valid register", offset); 
           return (FAILED);
    }
    return (PASSED);
}
#endif

int set_interrupt (int onoff)
{
    uchar val;
    
    if (onoff == 1) {
        val = 0xC8;
        write_ps_reg(PS_CTRL_REG1, (char*)&val, PS_ONE_BYTE);
        val = 0x81;
        write_ps_reg(PS_CTRL_REG3, (char*)&val, PS_ONE_BYTE);
        val = 0x1;
        write_ps_reg(PS_INTR_CONF, (char*)&val, PS_ONE_BYTE);
        
    } else {
        val = 0x94;
        write_ps_reg(PS_CTRL_REG1, (char*)&val, PS_ONE_BYTE);
        val = 0x80;
        write_ps_reg(PS_CTRL_REG3, (char*)&val, PS_ONE_BYTE);
        val = 0x0;
        write_ps_reg(PS_INTR_CONF, (char*)&val, PS_ONE_BYTE);
    }
    
    /* max data update frequence 25hz */
    msleep(40);
    
    return (PASSED);
}

static void set_to_default_registers (void)
{
    uchar val;
    val = 0x0;
    write_ps_reg(PS_REF_P_XL, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_REF_P_L, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_REF_P_H, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_CTRL_REG2, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_CTRL_REG4, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_PHS_P_L, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_PHS_P_H, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_RPDS_L, (char*)&val, PS_ONE_BYTE);
    val = 0x0;
    write_ps_reg(PS_RDPS_H, (char*)&val, PS_ONE_BYTE);
    
}

int interrupt_test(void)
{
    int ix = 0, max_retry = 200, rc = 0, val;
    uchar buf_p, buf_p1, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    uint gpio_value = 0;
    
    if (diagflag_xram & D_TRACE) {
        read_ps_reg (PS_PHS_P_L, (char*)&buf_p, PS_ONE_BYTE);
        read_ps_reg (PS_PHS_P_H, (char*)&buf_p1, PS_ONE_BYTE);
        read_ps_reg (PS_CTRL_REG1, (char*)&tmp1, PS_ONE_BYTE);
        read_ps_reg (PS_CTRL_REG2, (char*)&tmp2, PS_ONE_BYTE);
        read_ps_reg (PS_CTRL_REG3, (char*)&tmp3, PS_ONE_BYTE);
        read_ps_reg (PS_CTRL_REG4, (char*)&tmp4, PS_ONE_BYTE);
        read_ps_reg (PS_REF_P_XL, (char*)&tmp5, PS_ONE_BYTE);
        read_ps_reg (PS_REF_P_L, (char*)&tmp6, PS_ONE_BYTE);
        read_ps_reg (PS_REF_P_H, (char*)&tmp7, PS_ONE_BYTE);
        read_ps_reg (PS_RES_CONF, (char*)&tmp8, PS_ONE_BYTE);
        read_ps_reg (PS_INTR_CONF, (char*)&tmp9, PS_ONE_BYTE);
        read_ps_reg (PS_FIFO_CTRL, (char*)&tmp10, PS_ONE_BYTE);
        printf("\n[DBG]PHS_P_L=%x, PHS_P_H=%x, CTRL_REG1=%x, CTRL_REG2=%x, CTRL_REG3=%x, CTRL_REG4=%x, \
            PS_REF_P_XL=%x, PS_REF_P_L=%x, PS_REF_P_H=%x, PS_RES_CONF=%x, PS_INTR_CONF=%x, PS_FIFO_CTRL=%x\n", 
            buf_p, buf_p1, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10);
	}
            
    set_to_default_registers();

    if (dnv_gpio_read_rx_val(DNV_GPIO_8, &gpio_value) != PASSED) {
        cterr('f', 0, "Can not read CPU GPIO");
        return (FAILED);
    }

    prpass(testpass, "Check CPU Interrupt again(Before the test)");

    /* Check if interrupt is already asserted before the test */
    if (gpio_value == GPIO_HIGH) {
        set_interrupt(1);
        prpass(testpass, "Enable and Force Interrupt");
        msleep(500);

        for (ix = 0; ix < max_retry; ix++) {
            rc = dnv_gpio_read_rx_val(DNV_GPIO_8, &gpio_value);
            if (rc != PASSED) {
                cterr('f', 0, "Read GPIO Value Fails");
            }

            if (gpio_value == GPIO_LOW) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    read_ps_reg (0x25, (char*)&val, PS_ONE_BYTE);
                    printf("interrupt val = %d\n", val);
                }   
                prpass(testpass, "Check CPU Interrupt (After the test)");
                break;
            }
            msleep (POLL_DELAY);
        }

        if (ix == max_retry) {
            set_interrupt(0);
            cterr('f', 0, "Interrupt is not clear");
            return (FAILED);
        }
    } else {
        set_interrupt(0);
        cterr('f', 0, "Pressure Sensor Interrupt is already asserted");
        return (FAILED);
    }    

    prpass(testpass, "Disable Interrupt");
    set_interrupt(0);

    return (PASSED);
}

uchar ori_ctrl_reg1;
uchar ori_res_conf;
uchar ori_fifo_ctrl;
uchar ori_ctrl_reg3;

int ps_init(void)
{
    int rc;
    uchar val;

    if (!is_Init) {
        read_ps_reg(PS_CTRL_REG1, (char*)&ori_ctrl_reg1, PS_ONE_BYTE);
        val = 0x94;
        write_ps_reg(PS_CTRL_REG1, (char*)&val, PS_ONE_BYTE);
        read_ps_reg(PS_RES_CONF, (char*)&ori_res_conf, PS_ONE_BYTE);
        val = 0xF;
        write_ps_reg(PS_RES_CONF, (char*)&val, PS_ONE_BYTE);
        read_ps_reg(PS_FIFO_CTRL, (char*)&ori_fifo_ctrl, PS_ONE_BYTE);
        val = 0xDF;
        write_ps_reg(PS_FIFO_CTRL, (char*)&val, PS_ONE_BYTE);
        /* HW design is active low */
        read_ps_reg(PS_CTRL_REG3, (char*)&ori_ctrl_reg3, PS_ONE_BYTE);
        val = 0x80;
        write_ps_reg(PS_CTRL_REG3, (char*)&val, PS_ONE_BYTE);
        /* max data update frequence 25hz */
        msleep(40);
        is_Init = TRUE;
    }
    
    rc = show_pressure();

    return (rc);
}

int ps_deinit(void)
{
    if (is_Init) {
        write_ps_reg(PS_CTRL_REG1, (char*)&ori_ctrl_reg1, PS_ONE_BYTE);
        write_ps_reg(PS_RES_CONF, (char*)&ori_res_conf, PS_ONE_BYTE);
        write_ps_reg(PS_FIFO_CTRL, (char*)&ori_fifo_ctrl, PS_ONE_BYTE);
        write_ps_reg(PS_CTRL_REG3, (char*)&ori_ctrl_reg3, PS_ONE_BYTE);
        is_Init = FALSE;
    }
	
	return 0;
}

/*------------------------------------------------------------------
$Log: diag_press_sensor_test.c,v $
Revision 1.3  2020/04/20 02:28:24  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
