/* $Id: platform_eeprom.c,v 1.3 2017/07/10 02:51:58 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_eeprom.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_eeprom.c
 *
 * Description: Iformers 256 Bytes EEPROM I2C device.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "endians.h"
#include "common.h"
#include "dev_at24c0n.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "nvmonvars.h"
#include "cross_platform.h"
#include "platform_i2c.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "goofy_i2c.h"


extern int do_all_menu_items(struct menuinfo *);

static unsigned int eeprom_size = 256;
/* Function prototypes */
static int write_eeprom(unsigned int offset, unsigned int d32);

static int show_eeprom(void);
static int alter_eeprom(void);
static int test_eeprom(int);
static int test_xbyte_eeprom(int c);

/* need to clean up by moving this to platform_i2c.c */

static n2g_i2c_if_t i2c_eeprom[] =
    {
        {
            .offset = 0,
            .i2c_bus_type = IOFPGA_I2C,
            .i2c_dev = MB_I2C_ADDR_EEPROM,
            .mux        = MB_I2C_MUX_EEPROM,
            .i2c_ctrl = MB_I2C_CTRL_EEPROM,
            .sub_addr_len = 0,
            .size    = sizeof(int8_t),
            .rd_hd_size = 1, /* not used */
            .wr_hd_size = 1, /* not used */

            .buf        = NULL,
        },
    };

/* Global variables */
/*
 * 256 Bytes EEPROM Menu
 */
static submenu_xtable_t eeprom_menu_table[] = {
    {"Show contents",               (PFT)show_eeprom,          0,
        0,                          (type_t(*)())0, 0, (PFT)0, 0},
    {"Alter contents",              (PFT)alter_eeprom,         0,
        0,                          (type_t(*)())0, 0, (PFT)0, 0},
    {"Test EEPROM",                 (PFT)test_eeprom,       TRUE,
        (MF_CONTINUOUS | MF_DOALL), (type_t(*)())0, 0, (PFT)0, 0},
    {"Write x bytes",               (PFT)test_xbyte_eeprom, TRUE,
        0,                          (type_t(*)())0, 0, (PFT)0, 0},
};

#define EEPROM_MENU_TABLE_SIZE (sizeof(eeprom_menu_table) / \
		sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t eeprom_menu_primary_items[EEPROM_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];
static mitem_t eeprom_menu_secondary_items[EEPROM_MENU_TABLE_SIZE +
		MAX_BASE_ITEMS];

static struct menuinfo eepromdiag = {
    "EEPROM Utility Menu", /* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    eeprom_menu_primary_items,
};

static struct menuinfo *eepromdiagp = &eepromdiag;

/**********************************************************************
 *
 * function:	build_eeprom_menu
 *
 * Description:	Build 256-byte EEPROM menu.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void build_eeprom_menu (int menu_opt)
{
    testname("256-byte EEPROM");
    
    build_primary_submenu(eeprom_menu_table, EEPROM_MENU_TABLE_SIZE,
			  "256-byte EEPROM Utility Menu", &eepromdiagp);
    build_secondary_submenu(eeprom_menu_table, EEPROM_MENU_TABLE_SIZE,
			    eeprom_menu_secondary_items);

    if (menu_opt) {
        /* Entered with submenu */
        menu(&eepromdiag, eeprom_menu_secondary_items, 0);
    } else {
        do_all_menu_items(eepromdiagp);
    }
}

/*********************************************************************
 *
 * Function:	read_eeprom_eeprom
 *
 * Description:	Display 256-byte EEPROM Registers.
 *
 * Inputs:
 *              offset -- reg offset
 *              size   -- data len
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int
read_eeprom_block (unsigned int offset,
                  unsigned int size, unsigned char *buf)
{
    unsigned int d32;
    n2g_i2c_if_t i2c_if;
    uint32_t rc, i;

    memcpy(&i2c_if, &i2c_eeprom[0], sizeof(i2c_if));
    memset(buf, 0, size);
    i2c_if.buf = (char *)&d32;

    for (i = offset; i < size; i++) {
        d32 = 0;
        i2c_if.offset = i;
        rc = n2g_i2c_read(&i2c_if);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "unable to read from eeprom");
            return FAILED;
        }
        //        printf("%#x",d8);
        buf[i] = i2c_if.buf[0] ;
    }

    return(PASSED);

}

/*********************************************************************
 *
 * Function:	show_eeprom
 *
 * Description:	call read_eeprom_block to read a block of eeprom and
 *              display the data returned from read_eeprom_block()
 *
 * Inputs: NONE
 *
 * Outputs:	PASSED - No errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
show_eeprom (void)
{
    uint32_t rc, i;
    unsigned char data[AT24C04_MAX+1];

    rc = read_eeprom_block(0, eeprom_size, data);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "unable to read eeprom");
        return FAILED;
    }

    for (i = 0; i < eeprom_size; i++) {
        if ((i % 16) == 0) {
            printf("\n 0x%.4X : ", i);
        }
        printf("0x%.2x " ,data[i]);
    }
    /* Attach the device object */

    return(PASSED);

}


/*********************************************************************
 *
 * Function:	write_eeprom
 *
 * Description:	write 256-byte EEPROM Register one byte at a time.
 *
 * Inputs:	offset - register offset
 *              d32    - 32bit data. need 32bit even if data is only 8 bytes.
 *                       for goofy data fifo is 32bits.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
write_eeprom(unsigned int offset, unsigned int d32)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc;

    memcpy(&i2c_if, &i2c_eeprom[0], sizeof(i2c_if));
    i2c_if.offset = offset;
    i2c_if.buf = (char *)&d32;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "unable to write to eeprom");
        return FAILED;
    }

    /* got to wait at least 5ms for each write transaction ?? */
    msleep(AT24C0X_T_WR+1);
    
    /* Attach the device object */
    return(PASSED);

}

/*********************************************************************
 *
 * Function:	write_eeprom_block
 *
 * Description:	write 256-byte EEPROM Register one block at a time.
 *           Note: can write only 8 bytes per block (maybe 16?)
 *           functin doesn't check of data size is more than 8 bytes.
 *
 * Inputs:	offset - register offset
 *              d32    - 32bit data. need 32bit even if data is only 8 bytes.
 *                       for goofy data fifo is 32bits.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int
write_eeprom_block(unsigned int offset, unsigned int len,
                   unsigned char *buf)
{

    uint32_t rc, i;

    for (i = 0; i < len; i++) {
        rc = write_eeprom(i, buf[i]);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "unable to write new data to eeprom; error code %#x", rc);
            return FAILED;
        }
    }

    return(PASSED);

}

/*********************************************************************
 *
 * Function:	alter_eeprom
 *
 * Description:	utility allowing user to alter eeprom content
 *
 * Inputs:	NONE
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
alter_eeprom (void)
{
    unsigned int offset,  d32, rc;
    
    offset = gethex_answer("\nEnter reg offset", 0, 0, 0xFF);

    d32 = gethex_answer("\nEnter data", 0x89, 0, 0xFF);

    rc = write_eeprom(offset, d32);
    /* Attach the device object */
    return(rc);

}

/*********************************************************************
 *
 * Function:	test_eeprom
 *
 * Description:	Test 256-byte EEPROM.
 *
 * Inputs:	c_msg - Invoke prcomplete.
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
test_eeprom(int c_msg)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc, i;
    unsigned char sav_data[AT24C04_MAX+1];
    unsigned char new_data[AT24C04_MAX+1];
    

    testname("EEPROM Read/Write");
    prpass(testpass, (char *)NULL);

    memset(&i2c_if, 0, sizeof(i2c_if));
    memset(sav_data, 0, sizeof(sav_data));
    memset(new_data, 0, sizeof(new_data));

    /*save orginal eeprom data */
    rc = read_eeprom_block(0, eeprom_size, sav_data);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "%s: Unable to read eeprom", __FUNCTION__);
        return (FAILED);
    }

    /*write new data into eeprom byte by byte*/
    for (i = 0; i < eeprom_size; i++) {
        new_data[i] = i;
        rc = write_eeprom(i, i);
        if (rc != RC_I2C_OP_OK) {
            cterr('f', 0, "%s: Unable to write new data to eeprom;"
                          " error code(%#x)", __FUNCTION__, rc);
            return (FAILED);
        }

    }

    /*read back what we just wrote to eeprom */
    memset(new_data, 0, sizeof(new_data));
    rc = read_eeprom_block(0, eeprom_size, new_data);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "%s: Unable to read back after new data is written"
                      " to eeprom; error code(%#x)", __FUNCTION__, rc);
        return (FAILED);
    }

    /* compare data */
    for (i = 0; i < eeprom_size; i++) {
        if (new_data[i] != i) {
            cterr('f', 0, "%s: wrong data: @%#x=%#x; expecting %#x",
                          __FUNCTION__, i, new_data[i], i);
            break;
        }
    }

    fflush(stdout);
    rc = write_eeprom_block(0, eeprom_size, sav_data);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "%s: Unable to write orginal data back to eeprom;"
                      " error code(%#x)", __FUNCTION__, rc);
        return (FAILED);
    }

    if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
        printf("passed.\n");
    }

    return (PASSED);
}

/*********************************************************************
 *
 * Function:	test_xbyte_eeprom
 *
 * Description:	utility allowing user to test x number of bytes
 *
 * Inputs:	NONE
 *
 * Outputs:	PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
test_xbyte_eeprom (int c)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc, i, len;
    unsigned char new_data[AT24C04_MAX+1];
    

    memset(&i2c_if, 0, sizeof(i2c_if));
    memset(new_data, 0, sizeof(new_data));

    len = gethex_answer("\nEnter data len", 10, 0, 0xFF);
    for (i = 0; i < len; i++) {
        new_data[i] = 0xaa+i;
    }
        
    /*write new data into eeprom */
    rc = write_eeprom_block(0, len, new_data);
    if (rc != RC_I2C_OP_OK) {
        cterr('f', 0, "unable to write new data to eeprom; error code %#x", rc);
        return FAILED;
    }

    return(PASSED);
}

/*------------------------------------------------------------------
$Log: platform_eeprom.c,v $
Revision 1.3  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.2  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.8  2012/11/28 18:19:09  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.7  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.6  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.5  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.4  2012/09/26 18:02:14  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.3  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
