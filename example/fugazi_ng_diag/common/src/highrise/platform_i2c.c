/* $Id: platform_i2c.c,v 1.1 2020/08/19 09:49:35 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/platform_i2c.c,v $
 *-----------------------------------------------------------------------------
 * platform_i2c.c - Highrise I2C utility menu.
 *
 * Sept. 2019, Shuyuan Yu
 *
 * Copyright (c) 2019 ~ 2022 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "platform_i2c.h"
#include "platform_fru.h"
#include "hr_commn_util.h"

extern int highrise_display_temp(void);

/*i2c cterr*/
boolean g_i2c_read_cterr = TRUE;

/*
 * Functional prototype
 */
static int change_i2c_verbosity(void);
static int write_i2c(int);
static int dump_i2c(int);
static int read_i2c(int);
static int i2c_fd0 = -1;
static int i2c_fd1 = -1;
static int i2c_fd2 = -1;

unsigned char i2c_debug = 0;

static n2g_i2c_if_t cpu_i2c_dev[] = {
    /* I2C 0 */
    {
     .dev_name = "MAX5 CPLD",
     .offset = 0,
     .i2c_bus_type = MB_I2C_BUS_CPLD,
     .i2c_dev = MB_I2C_ADDR_CPLD,
     .i2c_ctrl = MB_I2C_CTRL_CPLD,
     .sub_addr_len = 0,
     .size = sizeof(uint32_t),
     .mux = MB_I2C_MUX_CPLD,
     .buf = NULL,
     }
    ,


    /* I2C 1 */

    /* I2C 2 */
    {
     .dev_name = "ACT2 Lite Secure Chip",
     .offset = 0,
     .i2c_bus_type = MB_I2C_BUS_ACT2,
     .i2c_dev = MB_I2C_ADDR_ACT2,
     .i2c_ctrl = MB_I2C_CTRL_ACT2,
     .sub_addr_len = 0,
     .size = sizeof(uint16_t),
     .mux = MB_I2C_MUX_ACT2,
     .buf = NULL,
     }
     ,

    {
     .dev_name = "RTC DS1337",
     .offset = 0,
     .i2c_bus_type = MB_I2C_BUS_RTC,
     .i2c_dev = MB_I2C_ADDR_RTC,
     .i2c_ctrl = MB_I2C_CTRL_RTC,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = MB_I2C_MUX_RTC,
     .buf = NULL,
     }
    ,

    {
     .dev_name = "Temperature Sensor(TMP75)",
     .offset = 0,
     .i2c_bus_type = MB_I2C_BUS_TMP75,
     .i2c_dev = MB_I2C_ADDR_TMP75,
     .i2c_ctrl = MB_I2C_CTRL_TMP75,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = MB_I2C_MUX_TMP75,
     .buf = NULL,
     }
    ,

    /* I2C 2 */
};


/* I2C Utility Menu.  */
static submenu_xtable_t i2c_menu_table[] = {
    { "Change I2C verbosity", (PFT) change_i2c_verbosity, FALSE, 
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read (no offset)", (PFT) read_i2c, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write (no offset)", (PFT) write_i2c, FALSE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C read", (PFT) read_i2c, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},
    {"I2C write", (PFT) write_i2c, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},

    {"I2C dump", (PFT) dump_i2c, TRUE,
     0, (PFT) 0, 0,
     (PFT) 0, 0},

};

#define I2C_MENU_TABLE_SIZE \
        (sizeof(i2c_menu_table) / sizeof(submenu_xtable_t))
#define ENHANCE_ERROR_MSG_RDY 1

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t i2c_menu_primary_items[I2C_MENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t i2c_menu_secondary_items[I2C_MENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

static struct menuinfo i2cdiag = {
    "I2C Utility Menu",         /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT) menu_show_dflags,     /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    i2c_menu_primary_items,
};

static struct menuinfo *i2cdiagp = &i2cdiag;


/**********************************************************************
 *
 * Function: build_i2c_menu
 *
 * Description: Build I2C menu.
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
void build_i2c_menu (void)
{
    build_primary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                          "I2C Utility Menu", &i2cdiagp);
    build_secondary_submenu(i2c_menu_table, I2C_MENU_TABLE_SIZE,
                            i2c_menu_secondary_items);
    menu(&i2cdiag, i2c_menu_secondary_items, 0);
}

static int change_i2c_verbosity(void)
{
    int verbosity = 0;
    verbosity = getdec_answer("\nEnter i2c_debug value ", 0, 0, 1);
    i2c_debug = verbosity;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : read_i2c_reg (int yes_offset)
 * Description: generic i2c read funtcion, allowing user to manually read from
 *              i2c register.
 * Inputs     : yes_offset. not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int read_i2c_reg (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, offset, rc, ix, bus;
    uchar d32[80];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    addr = gethex_answer("Enter 7 bit slave address ", 0x1c, 0x0, 0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0, 0, 0xFF);
        i2c_if.offset = offset;
    } else {
        i2c_if.offset = 0;
    }

    size =
        gethex_answer("Enter length you want to read(in bytes)", 2, 1, 10);
    i2c_if.size = size;

    memset(d32, 0, sizeof(d32));
    i2c_if.buf = (char *) d32;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to read i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    printf("\n");
    for (ix = 0; ix < size; ix++) {
        printf("0x%02x ", d32[ix]);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : dump_i2c_reg (int yes_offset)
 * Description: dump i2c regs, the params are passed in like CLI mode
 * Inputs     : yes_offset. not used
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int dump_i2c_reg (int yes_offset)
{
    int idx = 0;
    int jdx = 0;
    unsigned int  bus = 0;
    unsigned int  dev = 0;
    unsigned int  off = 0;
    unsigned int  len = 0;
    unsigned int  cnt = 0;
    char          buf[80] = {[0 ... sizeof(buf) - 1] = 0};
    unsigned char d32[16] = {[0 ... sizeof(d32) - 1] = 0};
    n2g_i2c_if_t i2c_if;


    _DRAIN_STDIN();
    printf("All input values in hex format and in sequence:\n");
    printf("  bus 7bit-dev off size count = ");
    fflush(stdout);
    fgets(buf, sizeof(buf) - 1,  stdin);

    ERR_RET_COND(5 != sscanf(buf, "%x %x %x %x %x", &bus, &dev, &off, &len, &cnt),
        FAILED, "Input params in sequence 'bus 7bit-dev off size count'\n");

    ERR_RET_COND(!(bus >= 0 && bus <= 3), FAILED, "Invalid bus :%#x\n", bus);
    ERR_RET_COND(!(dev >  0 && dev <= 0xef), FAILED, "Invalid dev :%#x\n", dev);
    ERR_RET_COND(!(len == 1 || len == 2 || len == 4), FAILED,
                "Invalid size:%#x, should be 1, 2 or 4 byte(s)\n", len);
    ERR_RET_COND(off > (0xffff - len), FAILED,
                "Invalid offset:%#x, should be within [0, %#x]\n", off, 0xffff - len);
    ERR_RET_COND(off + len * cnt > (0xffff - len), FAILED,
                "Invalid count:%#x, 'off + size * count' should be within [0, %#x]\n", len, 0xffff - len);


    memset(&i2c_if, 0, sizeof(i2c_if));
    i2c_if.i2c_bus_type = bus;
    i2c_if.i2c_ctrl     = i2c_if.i2c_bus_type;
    i2c_if.mux          = 0;
    i2c_if.i2c_dev      = dev;
    i2c_if.size         = len;

    printf("\n");
    for(idx = 0; idx < cnt; idx++) {
        memset(d32, 0, sizeof(d32));
        i2c_if.buf = (char *) d32;
        i2c_if.offset = off + idx * len;

        ERR_RET_COND(PASSED != n2g_i2c_read(&i2c_if), FAILED,
            "Unable to read i2c %#x %#x %#x.\n", bus, dev, i2c_if.offset);

        printf("%04x:", i2c_if.offset);
        for (jdx = 0; jdx < len; jdx++) {
            printf("0x%02x ", d32[jdx]);
        }
        printf("\n");
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : write_i2c_reg (int yes_offset)
 * Description: generic i2c write funtcion, allowing user to manually write to
 *              i2c register.
 *                example:
 *                    byte3 -  byte0 = 00 12 34 56
 *                    *buf is 0x56;  (little endian)
 *                    *(buf+1) is 0x34
 *                    *(buf+2) is 0x12
 *                    *(buf+3) is 0x0
 *    if we want to send data out, we need to shift to the left by 8 bits.
 *    we copy this to the data fifo. data fifo.
 *    format of fifo is byte0 to byte3.
 *    we will copy *(buf+3) to byte0 of fifo, *(buf+2) to byte1 of fifo. etc...
 *
 *
 * Inputs     : yes_offset, flag set if user is expected to enter register offset
 *
 * Outputs    : PASSED or FAILED
 *
 *******************************************************************************
 */
int write_i2c_reg (int yes_offset)
{
    n2g_i2c_if_t i2c_if;
    unsigned int addr, size, rc, ix, bus;
    int offset;
    char msg[80];
    uint8_t d8[32];

    memset(&i2c_if, 0, sizeof(i2c_if));

    bus = getdec_answer("\nEnter i2c bus number", 2, 0, 3);

    i2c_if.i2c_bus_type = bus;

    i2c_if.i2c_ctrl = i2c_if.i2c_bus_type;

    i2c_if.mux = 0;

    addr =
        gethex_answer("Enter 7 bit slave address (on mux 0)", 0x1c, 1,
                      0xFF);
    i2c_if.i2c_dev = addr;

    if (yes_offset) {
        offset = gethex_answer("Enter reg offset", 0x0, 0, 0xFF);
        i2c_if.offset = offset;
    } else {
        i2c_if.offset = 0;
    }

    size = gethex_answer("Enter length you want to write", 2, 1, 20);
    i2c_if.size = size;

    for (ix = 0; ix < size; ix++) {
        sprintf(msg, "Enter bytes %d", ix);
        d8[ix] = gethex_answer(msg, 0x0, 0, 0xFF);
    }

    i2c_if.buf = (char *) &d8[0];

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to write i2c %#x. "
              "rc = %#x", __FUNCTION__, __FILE__, i2c_if.i2c_dev, rc);
        return (FAILED);
    }

    return (PASSED);
}


/**********************************************************************
 *
 * Function: read_i2c
 *
 * Description: entry point to read i2c device
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int read_i2c (int has_offset)
{
    read_i2c_reg(has_offset);
    return (PASSED);
}

/**********************************************************************
 * Function    : dump_i2c
 * Description : entry point to dump i2c device regs
 * Input       : has_offset -- (Not used now)
 * Output      : PASSED
 **********************************************************************
 */
static int dump_i2c(int has_offset)
{
    return dump_i2c_reg(TRUE);
}


/**********************************************************************
 *
 * Function: write_i2c
 *
 * Description: entry point to write i2c device
 *
 * Input : has_offset -- flag set if user wants to specify reg
 *                       offset when sending request
 *
 * Output: PASSED
 *
 **********************************************************************
 */
static int write_i2c (int has_offset)
{
    write_i2c_reg(has_offset);
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function : is_cterr_print_on
 * Description: Return TRUE if i2c read cterr is turned on
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_cterr_print_on (void)
{
    return (g_i2c_read_cterr);
}

static void display_no_reg (void)
{
    cterr_db_print("This item has no reg to display\n");
}
/**************************************************************
 * Enhance Error Function
 * 1. Subtests of the test function will reuse all variables
 * 2. All variables will be cleared automatically when
 *    entering and leaving each menu item.
 * Segment 1: PID | Unique_string : slot_info
 *      fru_table_offset should be set, otherwise, it will not
 *      go to enhanced error message format in cterr()
 *      set fru_table_offset to get the predefine value
 *      or change mb_pid & mb_loc
 * Segment 2: Test step captured from prpass
 * Segment 3: Failure message captured from cterr
 * Segment 4: Components used
 * Segment 5: register and memory dump
 * Segment 6: Platform Environment initialized here
 * Segment 7: Top 3 Debugging Steps
 **************************************************************/
static void add_i2c_scan_test_err_report(void)
{
    fru_table_offset = MB;
    platform_fru_table[fru_table_offset].pid_string = mb_pid;
    platform_fru_table[fru_table_offset].location_string = mb_loc;
    cterr_add_component("Marvell Armada 7040", "I2C", "ACT2/TAM, TMP75, RTC");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)highrise_display_temp);
    cterr_add_debug("1. Check I2C unreset",
                    "2. use 'i2cdetect -y'",
                    "3. Contact hardware engineer to check signal");

}

/*****************************************************************************
 *
 * Function   : highrise_i2c_scan_test (int option)
 *
 * Description: scan all i2c devices on Highrise
 *
 * Inputs     : option ,for future use.
 *
 * Outputs    : PASSED or FAILED
 *
 ****************************************************************************
 */
int highrise_i2c_scan_test (int option)
{
    if (get_enhance_err_flag()) {
        add_i2c_scan_test_err_report();
    }

    int rc = PASSED;
    uint32_t ret_val = FAILED;
    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0;
    uint32_t ix, max_retry;
    uint8_t now_test = 0, test_end = 0;
    uchar *tname = (uchar *) "I2C scan";

    max_retry = MAX_RETRY;

    testname("%s", tname);
    prpass(testpass, "Start ");
    printf("\n");

    /* Setup end of test by calculate all I2C device number */
    test_end = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));

    for (now_test = 0; now_test < test_end; now_test++) {

        /* Get I2C device structure */
        memcpy(&i2c_if, &cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        i2c_if.buf = (char *) &reg_val;

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Now testing %2d: I2C bus %2d, MUX %d, %-29s(0x%.2X)... \n",
                    now_test, i2c_if.i2c_bus_type, i2c_if.mux, i2c_if.dev_name,
                    (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "I2C_%d: %s \n", i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /* Read I2C device Register 0 */
        for (ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val == PASSED) {
                break;
            } else {
                printf("\nWarning: failed to access I2C %s, rc:%d retry %d\n",
                        i2c_if.dev_name, ret_val, ix);
            }
            msleep(30);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Done ...\n");
        }

        if (ret_val != PASSED) {
            cterr('f', 0, "failed to access I2C %s, rc:%d\n",
                    i2c_if.dev_name, ret_val);
            rc = FAILED; //return rc until all I2C Dev loops finished 
        }
    }

    prpass(testpass, "%s ", (rc == PASSED ? "Passed" : "Failed"));
    //prcomplete(testpass, errcount, (char *)0);
    return (rc);
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

    size_cpu = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    for (ix = 0; ix < size_cpu; ix++) {
        if (cpu_i2c_dev[ix].i2c_dev == addr &&
                cpu_i2c_dev[ix].mux == mux &&
                cpu_i2c_dev[ix].i2c_ctrl == i2c) {
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }
    printf("Error: fail to get n2g_i2c_if; i2c_ctrl=%d, mux=%d, addr=%#x\n",
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

    size = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));
    for (ix = 0; ix < size; ix++) {
        if ((addr == cpu_i2c_dev[ix].i2c_dev) &&
                (ctrl_no == cpu_i2c_dev[ix].i2c_ctrl)) {
            return ((void *) (&cpu_i2c_dev[ix]));
        }
    }

    printf("Error: No I2C device found with addr:%#x, ctrl_no:%d\n", addr, ctrl_no);
    return (void *) NULL;
}


/*-------------------------------------------------------------------
 *
 * Function : is_need_dswap
 * Description: for declare is need dswap on this Highrise platform
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE
 * -------------------------------------------------------------------
 */
boolean is_need_dswap (void)
{
    return (TRUE);
}

int i2c_open (char *dev_file, int *fd)
{

    *fd = open(dev_file, O_RDWR);
    if (*fd <= 0) {
        printf("Open %s failed, err(%s, %d)\n", dev_file, strerror(errno), errno);
        return (-1);
    }

    return (0);
}


int highrise_init_i2c(void)
{
    int rc = 0;
    rc = i2c_open(I2CBUS0, &i2c_fd0);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to open %s.rc = %#x",
                __FUNCTION__, __FILE__, I2CBUS0, rc);
        return (FAILED);
    }

    rc = i2c_open(I2CBUS1, &i2c_fd1);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to open %s.rc = %#x",
                __FUNCTION__, __FILE__, I2CBUS1, rc);
        return (FAILED);
    }

    rc = i2c_open(I2CBUS2, &i2c_fd2);
    if (rc != PASSED) {
        cterr('f', 0, "%s at %s: unable to open %s.rc = %#x",
                __FUNCTION__, __FILE__, I2CBUS2, rc);
        return (FAILED);
    }

    return (PASSED);
}

/*****************************************************************************
 *
 * Function   : get_i2c_fd
 * Description: return file descriptor for /dev/i2c1
 * Inputs     : i2c_bus
 * Outputs    : file desriptor for /dev/i2c1
 *
 *****************************************************************************/
int get_i2c_fd(int i2c_bus)
{
    if (i2c_bus == 0) {
        return i2c_fd0;

    } else if (i2c_bus == 1) {
        return i2c_fd1;

    } else if (i2c_bus == 2) {
        return i2c_fd2;

    } else {
        return -1;
    }

}

/*------------------------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.1  2020/08/19 09:49:35  markzha
*** empty log message ***


$Endlog$
*/
