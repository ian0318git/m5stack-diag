/* $Id: platform_eeprom.c,v 1.2 2017/08/02 14:21:48 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_eeprom.c,v $
 *------------------------------------------------------------------
 * Filename:    platform_eeprom.c
 *
 * Description: Iformers 256 Bytes EEPROM I2C device.
 *
 * Copyright (c) 2014-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h> // for open
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
#include "i2c_dev.h"
#include "goofy_i2c.h"

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)

extern int do_all_menu_items(struct menuinfo *);

static unsigned int eeprom_size = 256;
static unsigned int eeprom_spd_size = 8;

/* Function prototypes */
static int write_eeprom(unsigned int, unsigned int);
static int show_eeprom(void);
static int alter_eeprom(void);
static int test_eeprom(int);
static int test_xbyte_eeprom(int c);
static int rw_lots_data_eeprom(int);

/* need to clean up by moving this to platform_i2c.c */

static n2g_i2c_if_t i2c_eeprom[] = {
    {
     .offset = 0,
     .i2c_bus_type = CPU_I2C0,
     .i2c_dev = MB_I2C_ADDR_EEPROM,
     .mux = MB_I2C_MUX_EEPROM,
     .i2c_ctrl = MB_I2C_CTRL_EEPROM,
     .sub_addr_len = 0,
     .size = sizeof(int16_t),   /* Read 2 bytes (16 bits) at a time */
     .rd_hd_size = 2,           /* not used */
     .wr_hd_size = 2,           /* not used */

     .buf = NULL,
     }
    ,
};

/* Global variables */
/*
 * 256 Bytes EEPROM Menu
 */
static submenu_xtable_t eeprom_menu_table[] = {
    {"Show contents", (PFT) show_eeprom, 0,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Alter contents", (PFT) alter_eeprom, 0,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Test EEPROM", (PFT) test_eeprom, TRUE,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"Write x bytes", (PFT) test_xbyte_eeprom, TRUE,
     MM_3, (type_t(*)())0, 0, (PFT) 0, 0},
    {"R/W lots of data into EEPROM", (PFT) rw_lots_data_eeprom, TRUE,
     0, (type_t(*)())0, 0, (PFT) 0, 0},
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
    "EEPROM Utility Menu",      /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    eeprom_menu_primary_items,
};

static struct menuinfo *eepromdiagp = &eepromdiag;

static int rw_lots_data_eeprom(int menu)
{
    int res, length1, length2;
    int i2c_adapter_fd;
    char i2c_adapter[] = "/dev/i2c-0";
    char send_buffer1[] = {0x00, 0x05, 0x98};
    char send_buffer2[] =
    {0x00,0x02,0x0b,0x03,0x04,0x19,0x02,0x02,0x0b,0x11,0x01,0x08,0x0a,0x00,0xfe,0x00,
     0x69,0x78,0x69,0x3c,0x69,0x11,0x18,0x81,0x20,0x08,0x3c,0x3c,0x01,0x40,0x83,0x01,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x01,0x02,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x80,0xce,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xaa,0x55,
     0x48,0x35,0x54,0x43,0x34,0x47,0x36,0x33,0x41,0x46,0x52,0x5f,0x50,0x42,0x41,0x20,
     0x20,0x20,0x45,0x31,0x80,0x2c,0x44,0x50,0x41,0x45,0x32,0x48,0x37,0x30,0x31,0x33,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    length1 = sizeof(send_buffer1);
    length2 = sizeof(send_buffer2);

    /*
     * Open /dev/i2c-%d adapter device
     */
    i2c_adapter_fd = open(i2c_adapter, O_RDWR);
    if (i2c_adapter_fd < 0) {
        printf("%s:open(%s)", __FUNCTION__, i2c_adapter);
        return (FAILED);
    }

    /*
     * Set the Slave I2C address for the accesses
     */
    if (ioctl(i2c_adapter_fd, I2C_SLAVE, MB_I2C_ADDR_EEPROM) < 0) {
        printf("%s:ioctl(I2C_SLAVE, 0x%x)", __FUNCTION__, MB_I2C_ADDR_EEPROM);
        close(i2c_adapter_fd);
        return (FAILED);
    }

    printf("%s() line %d write buffer length is %d\n", __FUNCTION__, __LINE__, length1);

    res = write(i2c_adapter_fd, send_buffer1, length1);
    if (res == -1) {
        printf("\n *** ERROR: unable to write from ACT2 device, error no is %d\n",
               errno);
        printf("Something went wrong with read()! Error description is: %s\n",
               strerror(errno));
        return (FAILED);
    }

    printf("%s() line %d read 1 byte\n", __FUNCTION__, __LINE__);

    res = read(i2c_adapter_fd, send_buffer1, 1);     // was bytes_to_read
    if (res == -1) {
        printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n",
               errno);
        printf("Something went wrong with read()! Error description is: %s\n",
               strerror(errno));
       return (FAILED);
    }

    printf("%s() line %d write buffer length is %d\n", __FUNCTION__, __LINE__, length2);

    res = write(i2c_adapter_fd, send_buffer2, length2);
    if (res == -1) {
        printf("\n *** ERROR: unable to write from ACT2 device, error no is %d\n",
               errno);
        printf("Something went wrong with read()! Error description is: %s\n",
               strerror(errno));
        return (FAILED);
    }

    printf("%s() line %d read 1 byte\n", __FUNCTION__, __LINE__);

    res = read(i2c_adapter_fd, send_buffer2, 1);     // was bytes_to_read
    if (res == -1) {
        printf("\n *** ERROR: unable to read from ACT2 device, errno no is : %d\n",
               errno);
        printf("Something went wrong with read()! Error description is: %s\n",
               strerror(errno));
       return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * function:    build_eeprom_menu
 *
 * Description: Build 256-byte EEPROM menu.
 *
 * Input:   None.
 *
 * Output:  None.
 *
 **********************************************************************
 */
void build_eeprom_menu(int menu_opt)
{
    char *tname = "256-byte EEPROM";

    testname(tname);

    build_primary_submenu(eeprom_menu_table, EEPROM_MENU_TABLE_SIZE,
                          "256-byte EEPROM Utility Menu", &eepromdiagp);
    build_secondary_submenu(eeprom_menu_table, EEPROM_MENU_TABLE_SIZE,
                            eeprom_menu_secondary_items);

    if (menu_opt) {
        /*
         * Entered with submenu
         */
        menu(&eepromdiag, eeprom_menu_secondary_items, 0);
    } else {
        do_all_menu_items(eepromdiagp);
    }
}

/*********************************************************************
 *
 * Function:    read_eeprom_block
 *
 * Description: Display 256-byte EEPROM Registers.
 *
 * Inputs:
 *              offset -- reg offset
 *              size   -- data len
 *              buf    -- eeprom content
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int
read_eeprom_block(unsigned int offset,
                  unsigned int size, unsigned char *buf)
{
    unsigned int d32;
    n2g_i2c_if_t i2c_if;
    uint32_t rc, ix;

    memcpy(&i2c_if, &i2c_eeprom[0], sizeof(i2c_if));
    memset(buf, 0, size);
    i2c_if.buf = (char *) &d32;

    for (ix = offset; ix < size; ix++) {
        d32 = 0;
        i2c_if.offset = ix;
        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "unable to read from eeprom");
            return FAILED;
        }
        buf[ix] = i2c_if.buf[0];
    }

    return (PASSED);

}

/*********************************************************************
 *
 * Function:    read_spd_eeprom_block
 *
 * Description: Display 256-byte EEPROM Registers.
 *
 * Inputs:
 *              offset -- reg offset
 *              size   -- data len
 *              buf    -- eeprom content
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int read_spd_eeprom_block(unsigned int offset,
                  unsigned int size, unsigned char *buf)
{
    unsigned int d32;
    n2g_i2c_if_t *i2c_if;
    uint32_t rc, ix;

    i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ZERO, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_EEPROM);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
              __FUNCTION__);
        return (FAILED);
    }

    memset(buf, 0, size);

    /*
     * Setup I2C API interface struct
     */
    i2c_if->buf = (char *) &d32;

    for (ix = 0; ix < size; ix++) {
        d32 = 0;
        i2c_if->offset = offset + ix ;
        rc = n2g_i2c_read(i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "unable to read from SPD eeprom");
            return FAILED;
        }
        buf[ix] = i2c_if->buf[0];
    }

    return (PASSED);

}


/*********************************************************************
 *
 * Function:    show_eeprom
 *
 * Description: call read_eeprom_block to read a block of eeprom and
 *              display the data returned from read_eeprom_block()
 *
 * Inputs: NONE
 *
 * Outputs: PASSED - No errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int show_eeprom(void)
{
    uint32_t rc, i;
    unsigned char data[AT24C04_MAX + 1];
    char *tname = "EEPROM Show contents";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    rc = read_eeprom_block(0, eeprom_size, data);

    if (rc != PASSED) {
        cterr('f', 0, "unable to read eeprom");
        return FAILED;
    }

    for (i = 0; i < eeprom_size; i++) {
        if ((i % 16) == 0) {
            printf("\n 0x%.4X : ", i);
        }
        printf("0x%.2x ", data[i]);
    }
    /*
     * Attach the device object
     */

    return (PASSED);

}

/*********************************************************************
 *
 * Function:    write_eeprom
 *
 * Description: write 256-byte EEPROM Register one byte at a time.
 *
 * Inputs:      offset - register offset
 *              d32    - 32bit data. need 32bit even if data is only 8 bytes.
 *                       for goofy data fifo is 32bits.
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int write_eeprom(unsigned int offset, unsigned int d32)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc;

    memcpy(&i2c_if, &i2c_eeprom[0], sizeof(i2c_if));
    i2c_if.offset = offset;
    i2c_if.buf = (char *) &d32;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "unable to write to eeprom");
        return (FAILED);
    }

    /*
     * got to wait at least 5ms for each write transaction ??
     */
    msleep(AT24C0X_T_WR + 1);

    /*
     * Attach the device object
     */
    return (PASSED);

}

/*********************************************************************
 *
 * Function:    write_spd_eeprom
 *
 * Description: write 256-byte EEPROM Register one byte at a time.
 *
 * Inputs:      offset - register offset
 *              d32    - 32bit data. need 32bit even if data is only 8 bytes.
 *                       for goofy data fifo is 32bits.
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int write_spd_eeprom(unsigned int offset, unsigned int d32)
{
    n2g_i2c_if_t *i2c_if;
    uint32_t rc;
    i2c_if = (n2g_i2c_if_t *) get_n2g_i2c_if(I2C_CTRL_ZERO, I2C_MUX_ZERO,
                                             MB_I2C_ADDR_EEPROM);
    if (i2c_if == NULL) {
        cterr('f', 0, "%s: Failed to get I2C interface structure.",
              __FUNCTION__);
        return (FAILED);
    }

    /*
     * Setup I2C API interface struct
     */
    i2c_if->buf = (char *) &d32;
    i2c_if->offset = offset;

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "unable to write to SPD eeprom");
        return (FAILED);
    }

    /*
     * got to wait at least 5ms for each write transaction ??
     */
    msleep(AT24C0X_T_WR + 1);

    /*
     * Attach the device object
     */
    return (PASSED);

}

/*********************************************************************
 *
 * Function:    write_eeprom_block
 *
 * Description: write 256-byte EEPROM Register one block at a time.
 *           Note: can write only 8 bytes per block (maybe 16?)
 *           functin doesn't check of data size is more than 8 bytes.
 *
 * Inputs:  offset - register offset
 *              d32    - 32bit data. need 32bit even if data is only 8 bytes.
 *                       for goofy data fifo is 32bits.
 *
 * Outputs: PASSED/FAILED.
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
        if (rc != PASSED) {
            cterr('f', 0,
                  "unable to write new data to eeprom; error code %#x",
                  rc);
            return FAILED;
        }
    }

    return (PASSED);

}

/*********************************************************************
 *
 * Function:    write_spd_eeprom_block
 *
 * Description: write 256-byte EEPROM Register one block at a time.
 *              Note: can write only 8 bytes per block (maybe 16?)
 *              functin doesn't check of data size is more than 8 bytes.
 *
 * Inputs:      offset - register offset
 *              d32    - 32bit data. need 32bit even if data is only 8 bytes.
 *                       for goofy data fifo is 32bits.
 *
 * Outputs:     PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int write_spd_eeprom_block(unsigned int offset, unsigned int len,
                   unsigned char *buf)
{
    uint32_t rc, i;

    for (i = 0; i < len; i++) {
        rc = write_spd_eeprom(i + offset, buf[i]);
        if (rc != PASSED) {
            cterr('f', 0,
                  "unable to write new data to SPD eeprom; error code %#x",
                  rc);
            return FAILED;
        }
    }
    return (PASSED);
}


/*********************************************************************
 *
 * Function:    alter_eeprom
 *
 * Description: utility allowing user to alter eeprom content
 *
 * Inputs:  NONE
 *
 * Outputs: PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int alter_eeprom(void)
{
    unsigned int offset, d32;
    uint32_t rc;
    char *tname = "Alter contents";

    testname(tname);
    prpass(testpass, "Alter contents, ");

    offset = gethex_answer("\nEnter reg offset", 0, 0, 0xFF);

    d32 = gethex_answer("\nEnter data", 0x89, 0, 0xFF);

    rc = write_eeprom(offset, d32);
    /*
     * Attach the device object
     */
    return (rc);

}

/*********************************************************************
 *
 * Function:    test_eeprom
 *
 * Description: Test 256-byte EEPROM.
 *
 * Inputs:  c_msg - Invoke prcomplete.
 *
 * Outputs: PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int test_eeprom(int c_msg)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc, i, rc_compare = PASSED;
    unsigned char sav_data[AT24C04_MAX + 1];
    unsigned char new_data[AT24C04_MAX + 1];
    char *tname = "Test EEPROM";

    testname(tname);
    prpass(testpass, "EEPROM Read/Write, ");

    memset(&i2c_if, 0, sizeof(i2c_if));
    memset(sav_data, 0, sizeof(sav_data));
    memset(new_data, 0, sizeof(new_data));

    /*
     * save orginal eeprom data
     */
    rc = read_eeprom_block(0, eeprom_size, sav_data);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to read eeprom.", __FUNCTION__);
        return (FAILED);
    }

    /*
     * write new data into eeprom byte by byte
     */
    for (i = 0; i < eeprom_size; i++) {
        new_data[i] = i;
        rc = write_eeprom(i, i);
        if (rc != PASSED) {
            cterr('f', 0, "%s: Unable to write new data to eeprom;"
                  " error code(%#x)", __FUNCTION__, rc);
            return (FAILED);
        }

    }

    /*
     * read back what we just wrote to eeprom
     */
    memset(new_data, 0, sizeof(new_data));
    rc = read_eeprom_block(0, eeprom_size, new_data);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to read back after new data is written"
              " to eeprom; error code(%#x)", __FUNCTION__, rc);
        return (FAILED);
    }

    /*
     * compare data
     */
    for (i = 0; i < eeprom_size; i++) {
        if (new_data[i] != i) {
            cterr('f', 0, "%s: wrong data: @%#x=%#x; expecting %#x",
                  __FUNCTION__, i, new_data[i], i);
            break;
        }
    }

    fflush(stdout);
    rc = write_eeprom_block(0, eeprom_size, sav_data);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to write orginal data back to eeprom;"
              " error code(%#x)", __FUNCTION__, rc);
        return (FAILED);
    }

    if ((!((NVRAM)->diagflag & D_CONTINUOUS)) && (rc_compare != FAILED)) {
        printf("passed.\n");
    }

    return (PASSED);
}

/*********************************************************************
 *
 * Function:    test_spd_eeprom
 *
 * Description: Test 256-byte SPD EEPROM.
 *
 * Inputs:  c_msg - Invoke prcomplete.
 *
 * Outputs: PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
int test_spd_eeprom(int c_msg)
{
    uint32_t rc, i, rc_compare = PASSED;
    unsigned char sav_data[eeprom_spd_size + 1];
    unsigned char new_data[eeprom_spd_size + 1];
    char *tname = "Test SPD EEPROM";

    testname(tname);
    prpass(testpass, "SPD EEPROM Read/Write, ");

    memset(sav_data, 0, sizeof(sav_data));
    memset(new_data, 0, sizeof(new_data));

    /*
     * save orginal eeprom data
     */
    rc = read_spd_eeprom_block(0xf0, eeprom_spd_size, sav_data);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to read SPD eeprom.", __FUNCTION__);
        return (FAILED);
    }

    /*
     * write new data into eeprom byte by byte
     */
    for (i = 0; i < eeprom_spd_size; i++) {
        new_data[i] = i;
        rc = write_spd_eeprom(i+0xf0, i);
        if (rc != PASSED) {
            cterr('f', 0, "%s: Unable to write new data to SPD eeprom;"
                  " error code(%#x)", __FUNCTION__, rc);
            return (FAILED);
        }

    }

    /*
     * read back what we just wrote to eeprom
     */
    memset(new_data, 0, sizeof(new_data));
    rc = read_spd_eeprom_block(0xf0, eeprom_spd_size, new_data);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to read back after new data is written"
              " to eeprom; error code(%#x)", __FUNCTION__, rc);
        return (FAILED);
    }

    /*
     * compare data
     */
    for (i = 0; i < eeprom_spd_size; i++) {
        if (new_data[i] != i) {
            cterr('f', 0, "%s: wrong data: @%#x=%#x; expecting %#x",
                  __FUNCTION__, i, new_data[i], i);
            rc_compare = FAILED;
            break;
        }
    }

    fflush(stdout);
    rc = write_spd_eeprom_block(0xf0, eeprom_spd_size, sav_data);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Unable to write orginal data back to SPD eeprom;"
              " error code(%#x)", __FUNCTION__, rc);
        return (FAILED);
    }

    if ((!((NVRAM)->diagflag & D_CONTINUOUS)) && (rc_compare != FAILED)) {
        printf("passed.\n");
    }

    return (PASSED);
}


/*********************************************************************
 *
 * Function:    test_xbyte_eeprom
 *
 * Description: utility allowing user to test x number of bytes
 *
 * Inputs:  NONE
 *
 * Outputs: PASSED/FAILED.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int test_xbyte_eeprom(int c)
{
    n2g_i2c_if_t i2c_if;
    uint32_t rc, i, len;
    unsigned char new_data[AT24C04_MAX + 1];
    char *tname = "Write x bytes";

    testname(tname);
    prpass(testpass, "EEPROM Read/Write, ");

    memset(&i2c_if, 0, sizeof(i2c_if));
    memset(new_data, 0, sizeof(new_data));

    len = gethex_answer("\nEnter data len", 10, 0, 0xFF);
    for (i = 0; i < len; i++) {
        new_data[i] = 0xaa + i;
    }

    /*
     * write new data into eeprom
     */
    rc = write_eeprom_block(0, len, new_data);
    if (rc != PASSED) {
        cterr('f', 0, "unable to write new data to eeprom; error code %#x",
              rc);
        return FAILED;
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: platform_eeprom.c,v $
Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.2  2016/04/11 14:12:27  steja
Update code i2c utility for bringup

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility


$Endlog$
*/
