/* $Id: platform_temp_sensor.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_temp_sensor.c,v $
 *------------------------------------------------------------------
 * platform_temp_sensor.c
 *
 * Description: Digital Temperature Sensor Definitions.
 *              This file is based on TMP75/ADT75 Datasheet.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "endians.h"
#include "common.h"
#include "platform_temp_sensor.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "common_utils.h"
#include "byteswap.h"

#include "i2c_dev.h"
#include "i2c_api.h"
#include "goofy_i2c.h"
#include "i2c_address.h"
#include "platform_i2c_usb.h"
#include "n2g_api_rc.h"

/******************************************************************************
 *                                   Externs
 ******************************************************************************/
extern int get_i2c_fd(int cpu);
extern void *mmap_device (char *path, size_t size, off_t offset);
extern uint32_t pci_config_read (uint32_t bus, uint16_t device, uint32_t fn, int offset);

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
static char ts_name[4][16] =
{
    "Memory side",
    "CPU    side",
};

/* Temperature sensor registers table. This device has registers with different sizes.
 */
static reg_info_t ts_reg_table[] =
{
/*      { name, offset, rw type, 
 *    size, mask, default value},
 */
    {"Temperature",             TS_PTR_TEMP,    READ_ONLY,
        {TS_PTR_TEMP_L},        0xFFF0, 0x0000},
    {"Configuration",   TS_PTR_CFG,     READ_WRITE,
        {TS_PTR_CFG_L}, 0xFF, 0x00},
    {"T Low",    TS_PTR_THYST,   READ_WRITE,
        {TS_PTR_THYST_L},       0xFFF0, 0x0000},
    {"T High",        TS_PTR_TOS,     READ_WRITE,
        {TS_PTR_TOS_L}, 0xFFF0, 0x0000},
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
    {"Side #0-Show temperature", (PFT)show_temperature, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0,(PFT)show_temperature, TS_BEZEL_SIDE0},
    {"Side #0-Register sanity test", (PFT)sanity_test, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_BEZEL_SIDE0},
    {"Side #0-Show register", (PFT)show_ts_reg, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_BEZEL_SIDE0},
    {"Side #0-Alter register", (PFT)alter_ts_reg, TS_BEZEL_SIDE0, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_BEZEL_SIDE0},
    // bezel side #1
    {"Side #1-Show temperature", (PFT)show_temperature, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)show_temperature, TS_BEZEL_SIDE1},
    {"Side #1-Register sanity test", (PFT)sanity_test, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)sanity_test, TS_BEZEL_SIDE1},
    {"Side #1-Show register", (PFT)show_ts_reg, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)show_ts_reg, TS_BEZEL_SIDE1},
    {"Side #1-Alter register", (PFT)alter_ts_reg, TS_BEZEL_SIDE1, 0,
     (type_t(*)())0, 0, (PFT)alter_ts_reg, TS_BEZEL_SIDE1},
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
    "Temperature Sensors Utility Menu",     /* title */
    0,                              /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,          /* shows major flags */
    0,                              /* generic prompt */
    0,                              /* size -- bumped by add_menu_item() */
    ts_menu_primary_items,
};

static struct menuinfo *tsdiagp = &tsdiag;

static int temp_i2c_fd = -1;


/******************************************************************************
 *
 * function   : build_ts_menu
 * Description: Build menu for temperature sensor related utility.
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

/******************************************************************************
 *
 * Function   : init_temp_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_temp_i2c_struct (n2g_i2c_dev_t *i2c_dev, uint32_t ts_id) {
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;
    switch(ts_id) {
    case TS_BEZEL_SIDE0:
        i2c_dev->dev_addr = TS_BEZEL_SIDE_ADDR0;
        break;
    case TS_BEZEL_SIDE1:
        i2c_dev->dev_addr = TS_BEZEL_SIDE_ADDR1;
        break;
    default:
        printf("%s: Unknown ts_id no. = %d.\n", __FUNCTION__, ts_id);
        return (FAILED);
        break;
    }

    temp_i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (temp_i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(temp_i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = temp_i2c_fd;
        }
    }
    return (PASSED);
}


/******************************************************************************
 *
 * function   : read_ts_reg
 * Description: Wrapper to read temperature sensor's register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 *              addr_ptr_id - pointer register ID.
 *                              data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
read_ts_reg (int ts_id, int addr_ptr_id, char * data_buf_p, int size)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      rc = FAILED;

    /* Init device structure */
    if (init_temp_i2c_struct(&i2c_dev, ts_id) != PASSED) {
        printf("Init Temp%d i2c_dev struct failed.", ts_id);
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    i2c_if.size = size; 
    i2c_if.offset = addr_ptr_id;
    i2c_if.buf = (char *)data_buf_p;

    rc = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc != PASSED) {
        /* Read failed */
        if (rc == E_I2C_INV_ACK) {
            printf("%s: Temp%d is not installed.\n",
                            __FUNCTION__, ts_id);
        } else {
            printf("%s: Temp I2C read failed(rc = %#.8x).\n",
                            __FUNCTION__, rc);
        }
        return(FAILED);
    }
    return (PASSED);
}

//#define PRINT_WRITE_VALUE
/******************************************************************************
 *
 * function   : write_ts_reg
 * Description: Wrapper to write temperature sensor's register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 *              addr_ptr_id - pointer register ID.
 *                              data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int 
write_ts_reg (int ts_id, int addr_ptr_id, char * data_buf_p, int size)
{
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      rc = FAILED;
#ifdef PRINT_WRITE_VALUE
	int i;
#endif
    /* Init device structure */
	
    if (init_temp_i2c_struct(&i2c_dev, ts_id) != PASSED) {
        printf("Init Temp%d i2c_dev struct failed.", ts_id);
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Write the bytes from PoE controller */
    i2c_if.size = size;
    i2c_if.offset = addr_ptr_id;
    i2c_if.buf = data_buf_p;

    rc = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);
    if (rc  != PASSED) {
        printf("%s: TWSI Write Failed !!!n", __FUNCTION__);
        printf("(Bus%d, Dev 0x%01X, offset 0x%01X)\n",
                           i2c_dev.bus_no, i2c_if.i2c_dev, i2c_if.offset);
        return (FAILED);
    }
#ifdef PRINT_WRITE_VALUE
    printf("%s: Done writing",__FUNCTION__);
	for(i=0;i<size;i++)
		printf(" 0x%02X",(uint8)data_buf_p[i]);
	printf(" to Temp reg.(0x%02X).\n",(uint8_t)(i2c_if.offset & 0xff));
#endif

    return (PASSED);
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
				backup_buf = DSWAP2(backup_buf);
			
                // write new data
                new_buf = (backup_buf + TS_TEST_PATTERN);
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
				compare_buf = DSWAP2(compare_buf);

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

float get_current_temperature (int ts_id)
{
	uint32 rc;
	ts_t val;
	float temp = 0;
	
	rc = read_ts_reg (ts_id, TS_PTR_TEMP, (char*) &val, sizeof(ts_t));
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                      __FUNCTION__, __LINE__, ts_id);
        return temp;
    }
	val = DSWAP2(val);
    if (val <= TS_TEMP_MAX) {
		temp = ((val >> 4) * TS_TEMP_RESOLUTION);
    } else {
		temp = (((val >> 4) - 4096) * TS_TEMP_RESOLUTION);
    }
	return temp;
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
        printf("%s temperature : %.4f Celsius\n", \
                            ts_name[ts_id], (val >> 4) * TS_TEMP_RESOLUTION);
    } else {
        printf("%s temperature : %.4f Celsius\n", \
                            ts_name[ts_id], ((val >> 4) - 4096) * TS_TEMP_RESOLUTION);
    }

    return (PASSED);
}


uint32
show_temperature_all(void) 
{
    int i, rc;

    for (i=TS_BEZEL_SIDE0; i<=TS_BEZEL_SIDE1; i++) {
            rc = show_temperature(i);
        if (rc != PASSED) {
            return (FAILED);
        }
    }
    printf("\n");
    
    return (PASSED);
}

/********************************************************************
 *
 * Function:    show_ts_reg
 *
 * Description: show temperature sensor register.
 *
 * Inputs:      -
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
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
   
    while (reg_p->name) {
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
			if (reg_p->size.size == sizeof(ts_t))
            {
				float temp = 0;
            	ts_t val;

				val = buffer;
				val = DSWAP2(val);
			    if (val <= TS_TEMP_MAX) {
			        temp = ((val >> 4) * TS_TEMP_RESOLUTION);
			    } else {
			        temp = (((val >> 4) - 4096) * TS_TEMP_RESOLUTION);
			    }
                printf("%-36s (0x%02X), data: 0x%04X.(%.4f Celsius)\n", reg_p->name,
                      	reg_p->offset, buffer,temp);
       		}else
           		printf("%-36s (0x%02X), data: 0x%04X.\n", reg_p->name,
                      	reg_p->offset, buffer);
        }
		reg_p++;
	}
    return PASSED;
}


/********************************************************************
 *
 * Function:    alter_ts_reg
 *
 * Description: alter temperature sensor register.
 *
 * Inputs:      -
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
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
                reg_p++;        /* update the register table pointer */
    } /* endof while */

    /* Get the register to peek-n-poke */
    offset = gethex_answer("Enter the register number:", 0, 0, TS_PTR_TOS);
    
    /* Check if the register is read/writeable */
    /* Find the register text in the register table */
    reg_p = &ts_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != offset) {
                /* Not requested register */
                reg_p++;        /* update the register table pointer */
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
							    ts_t val;
								
								val = DSWAP2(old_temp_data);
								if (val <= TS_TEMP_MAX)
									val = (val >> 4);
								else
									val = ((val >> 4) - 4096);
								printf("Current setting : %.4f Celsius\n",val * TS_TEMP_RESOLUTION);
									
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

static int trigger_ts_alert(int ts_id, int bTrigger)
{
#define T_LOW_DEFAULT	0x4B00	// 75 Celsius
#define T_HIGH_DEFAULT	0x5000	// 80 Celsius
#define T_TRIGGER		0xC400	// -60 Celsius

	int rc = FAILED;
	ts_t temp_data;
	char* buf_p;

	buf_p = (char *)&temp_data;	

	//Set T Low
	if(bTrigger)
		temp_data = T_TRIGGER;
	else
		temp_data = T_LOW_DEFAULT;
	rc = write_ts_reg (ts_id, TS_PTR_THYST, buf_p, sizeof(ts_t)); 
	if(rc != PASSED)
	{
		cterr('f', 0, "%s:%d T Low write_ts_reg failed\n",__FUNCTION__, __LINE__);
		return rc;
	}
	
	//Set T High
    if(bTrigger)
        temp_data = T_TRIGGER;
    else
        temp_data = T_HIGH_DEFAULT;
    rc = write_ts_reg (ts_id, TS_PTR_TOS, buf_p, sizeof(ts_t));
    if(rc != PASSED)
    {
        cterr('f', 0, "%s:%d T High write_ts_reg failed\n",__FUNCTION__, __LINE__);
        return rc;
    }

	//printf("reg after Trigger_ts_alert(%d)\n",bTrigger);
	//show_ts_reg(ts_id);
	
	msleep(500);

	return rc;
}

int temperature_sensor_alert_test(int ts_id)
{
    uint32_t smb_base, offset_val;
    off_t mmap_start;
    unsigned long addr = 0;
    uint8 val = 0;
	int rc = FAILED;

    smb_base = pci_config_read(0x00, 0x1F, 0x03, 0x10);
    mmap_start = smb_base & ~(getpagesize()-1);
    addr = (unsigned long)mmap_device("mem", 0x1000, mmap_start);
	offset_val = 0x00;

    if(addr!=0)
    {

		trigger_ts_alert(ts_id, TRUE);
        val = *((uint8 *)(addr +  offset_val));
		//printf("HST_STS(0x%x) value:0x%x\n",offset_val,val);

		trigger_ts_alert(ts_id, FALSE);

		if(val & (1<<5))
		{
			register_write((addr +  offset_val), (1<<5), BW_8BITS);	
    	    //val = *((uint8 *)(addr +  offset_val));
	        //printf("After clear HST_STS(0x%x) value:0x%x\n",offset_val,val);
			rc = PASSED;
		}else
		{
			cterr('f', 0, "%s:%d Didn't get SMBALERT_STS\n",__FUNCTION__, __LINE__);
			show_ts_reg(ts_id);
		}

        munmap((void *)addr, 0x1000);
    }else
		cterr('f', 0, "%s:%d mmap Failed\n",__FUNCTION__, __LINE__);

    return rc;
}

/*
 *------------------------------------------------------------------
 * $Log: platform_temp_sensor.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.6  2019/04/22 06:58:08  mikech2
 * Fix -5 degree mb test fail issue
 *
 * Revision 1.1.2.5  2019/03/11 03:17:33  mikech2
 * Modify temperature sensor alert test threshold
 *
 * Revision 1.1.2.4  2019/03/04 07:39:16  mikech2
 * Add temperature sensor alert test in Interrupt auto test
 *
 * Revision 1.1.2.3  2019/02/25 08:46:45  mikech2
 * Modify temperature sensor show register
 *
 * Revision 1.1.2.2  2019/01/29 01:54:22  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.3  2019/01/21 07:28:57  mikech2
 * Modify temp. sensor name
 *
 * Revision 1.1.2.2  2018/12/12 09:06:42  mikech2
 * Fix typo
 *
 * Revision 1.1.2.1  2018/10/22 08:02:30  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.3  2018/09/07 03:14:21  peteteng
 * Add system info utility
 *
 * Revision 1.1.2.2  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.1  2018/07/17 11:33:20  benlu
 * For temperature diag test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

