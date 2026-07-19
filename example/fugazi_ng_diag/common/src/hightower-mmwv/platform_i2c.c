/* $Id: platform_i2c.c,v 1.2 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_i2c.c,v $
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
#include "platform_sfp_cookie.h"

static uint32 show_sfp_cookie(void); 

#ifndef HIGHRISE_TODO
extern int highrise_display_temp(void);
#endif
/*disable cterr*/
boolean g_i2c_read_cterr = FALSE;

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
    /* to PIM */

    /* I2C 1 */
    {
     .dev_name = "ACT2 Lite Secure Chip",
     .offset = 0,              /* need to be -1 to tell driver not to use offset !!! */
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

    {
     .dev_name = "SFP cookie 0x50",
     .offset = 0,
     .i2c_bus_type = MB_I2C_BUS_SFP,
     .i2c_dev = MB_I2C_ADDR_SFP, 
     .i2c_ctrl = MB_I2C_CTRL_SFP,
     .sub_addr_len = 0,
     .size = sizeof(uint8_t),
     .mux = MB_I2C_MUX_SFP,  
     .buf = NULL,
     }
    ,

    /* I2C 2 */
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
     },

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
    {"SFP cookie read  ", (PFT) show_sfp_cookie, 0,
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

/*
 * Contents Description table
 */
/*
 *  * Device display struct
 *   */

typedef struct at24c0n_desc_t_ {
    char *name;         /* Text of field */
    at_o offset;        /* Filed starting offset */
    at_o size;          /* Number of bytes in this field */
    uint8_t type;       /* Display type. Refer to at_desc_type enum */
} dev_at24c0n_desc_t;

typedef enum{
    AT_DESC_HEX = 0,    /* Hex byte */
    AT_DESC_DEC,        /* Decimal */
    AT_DESC_TXT,        /* String */
} at_desc_type;

static dev_at24c0n_desc_t sfp_desc_table[] = {
    {"Identifier",                 SFP_COO_ID,    SFP_COO_ID_L,   AT_DESC_HEX},
    {"Ext. Identifier",            SFP_COO_X_ID,  SFP_COO_XID_L,  AT_DESC_HEX},
    {"Connector",                  SFP_COO_CNT,   SFP_COO_CNT_L,  AT_DESC_HEX},
    {"Transceiver",                SFP_COO_XVR,   SFP_COO_XVR_L,  AT_DESC_HEX},
    {"Encoding",                   SFP_COO_ENC,   SFP_COO_ENC_L,  AT_DESC_HEX},
    {"BR, Nominal",                SFP_COO_BR_N,  SFP_COO_BR_N_L, AT_DESC_DEC},
    {"Length(9m) - km",            SFP_COO_L_9KM, SFP_COO_L_9KM_L,AT_DESC_DEC},
    {"Length (9m)",                SFP_COO_L_9M,  SFP_COO_L_9M_L, AT_DESC_DEC},
    {"Length (50m)",               SFP_COO_L_50,  SFP_COO_L_50_L, AT_DESC_DEC},
    {"Length (62.5m)",             SFP_COO_L_62,  SFP_COO_L_62_L, AT_DESC_DEC},
    {"Length (Copper)",            SFP_COO_L_CU,  SFP_COO_L_CU_L, AT_DESC_DEC},
    {"Vendor Name",                SFP_COO_VEND,  SFP_COO_VEND_L, AT_DESC_TXT},
    {"Channel Spacing",            SFP_COO_CH_S,  SFP_COO_CH_S_L, AT_DESC_DEC},
    {"Vendor OUI",                 SFP_COO_VEN_O, SFP_COO_VEN_O_L, AT_DESC_HEX},
    {"Vendor P/N",                 SFP_COO_VEN_PN,SFP_COO_VEN_P_L,AT_DESC_TXT},
    {"Vendor Rev",                 SFP_COO_VEN_R, SFP_COO_VEN_R_L,AT_DESC_TXT},
    {"Laser Wavelength",           SFP_COO_LSR_W, SFP_COO_LSR_W_L,AT_DESC_HEX},
    {"DWDM Wavelength Fraction",   SFP_COO_DWDM_W,SFP_COO_DWDM_L, AT_DESC_HEX},
    {"CC_BASE Checksum",           SFP_COO_CC_B,  SFP_COO_CC_B_L, AT_DESC_HEX},
    {"Options",                    SFP_COO_OPT,   SFP_COO_OPT_L,  AT_DESC_HEX},
    {"BR, Max",                    SFP_COO_BR_MAX,SFP_COO_BR_MX_L,AT_DESC_DEC},
    {"BR, Min",                    SFP_COO_BR_MIN,SFP_COO_BR_MN_L,AT_DESC_DEC},
    {"Vendor SN",                  SFP_COO_VEN_SN,SFP_COO_VEN_S_L,AT_DESC_TXT},
    {"Date code",                  SFP_COO_DATE,  SFP_COO_DATE_L, AT_DESC_DEC},
    {"Diagnostic Monitoring type", SFP_COO_DIAG,  SFP_COO_DIAG_L, AT_DESC_HEX},
    {"Enhanced Options",           SFP_COO_ENH,   SFP_COO_ENH_L,  AT_DESC_HEX},
    {"CC_EXT Checksum",            SFP_COO_CC_X,  SFP_COO_CC_X_L, AT_DESC_HEX},
    {"Vendor Specific",            SFP_COO_VEND_SP,SFP_COO_VN_SP_L,AT_DESC_HEX},
    {0, 0, 0, 0},       /* Terminator */
};


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
    int ret = 0;
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

        ret = n2g_i2c_read(&i2c_if);
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
static void add_i2c_scan_tes_err_report(void)
{
    fru_table_offset = MB;
    platform_fru_table[fru_table_offset].pid_string = mb_pid;
    platform_fru_table[fru_table_offset].location_string = mb_loc;
    cterr_add_component("Marvell Armada 7040", "I2C", "ACT2/TAM, TMP75, RTC");
    cterr_add_reg_dump((PFV)display_no_reg);
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
        add_i2c_scan_tes_err_report();
    }

    n2g_i2c_if_t i2c_if;
    uint32_t reg_val = 0, ret_val = FAILED, fail_ctr = 0;
    uint32_t ix, max_retry;
    uint8_t now_test = 0, test_end = 0;
    uchar *tname = (uchar *) "I2C scan";

    /*Disable Cterr*/
    g_i2c_read_cterr = (boolean) FALSE;

    max_retry = MAX_RETRY;

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* Setup end of test by calculate all I2C device number */
    test_end = (sizeof(cpu_i2c_dev) / sizeof(n2g_i2c_if_t));

    for (now_test = 0; now_test < test_end; now_test++) {
        /* Get I2C device structure */
        memcpy(&i2c_if, &cpu_i2c_dev[now_test], sizeof(n2g_i2c_if_t));
        i2c_if.buf = (char *) &reg_val;

        /* skip SFP so far, wait for HW provide present pin.
         * or using external loopback flag instead. */
        if ((i2c_if.i2c_ctrl == MB_I2C_CTRL_SFP) &&
            (i2c_if.i2c_dev == MB_I2C_ADDR_SFP)) {
            continue; 
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Now testing %2d: I2C bus %2d, Mux %d, %-29s(0x%.2X)... ",
                    now_test, i2c_if.i2c_bus_type, i2c_if.mux, i2c_if.dev_name,
                    (i2c_if.i2c_dev << 1));
        } else {
            prpass(testpass, "I2C_%d: %s \n", i2c_if.i2c_bus_type, i2c_if.dev_name);
        }

        /* Read I2C device Register 0 */
        for (fail_ctr = ix = 0; ix < max_retry; ix++) {
            ret_val = n2g_i2c_read(&i2c_if);
            if (ret_val == PASSED) {
                break;
            } else {
                fail_ctr++;
                cterr('f', 0, "%s failed rc:%d ", i2c_if.dev_name, ret_val);
            }
        }

        msleep(30);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Done\n");
    }


    /* Enable cterr */
    g_i2c_read_cterr = (boolean) TRUE;
    prpass(testpass, "%s, ", tname);
    prcomplete(testpass, errcount, (char *)0);
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
    /*
     * DTS will remove unused /dev/i2c-0
     * No need to open i2c-0 node here
     */ 

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
 * Description: return file descriptor for /dev/i2c
 * Inputs     : i2c_bus
 * Outputs    : file desriptor for /dev/i2c
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

static uint32 show_sfp_cookie (void) 
{
    uint32 rc;
    unsigned int ix; 
    n2g_i2c_if_t *i2c_if;
    dev_at24c0n_desc_t *pdesc;
    char *buf; 

    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(MB_I2C_CTRL_SFP,
              MB_I2C_MUX_SFP, MB_I2C_ADDR_SFP); 

    pdesc = &sfp_desc_table[0]; /* Get the first descriptor */

    while(pdesc->name) {
        i2c_if->size = pdesc->size;
        i2c_if->offset = pdesc->offset;

        buf = malloc(pdesc->size);
        if (buf == NULL) {
            printf("%s: malloc %d byte failed", __FUNCTION__, pdesc->size);
            return (FAILED);
        }

        i2c_if->buf = buf; 
        rc = n2g_i2c_read(i2c_if);

        if (rc != PASSED) {
            free(buf);
            printf("%s: read %s %d bytes @ %#x failed. rc = %#x",
                   __FUNCTION__, pdesc->name, pdesc->size, pdesc->offset, rc);
            return (FAILED);
        }

        printf("%s @ %#x : ", pdesc->name, pdesc->offset);
        switch(pdesc->type) {
        case AT_DESC_HEX:
            printf("0x");
            for (ix = 0; ix < pdesc->size; ix++) {
                printf("%02x ", (uint8_t)buf[ix]);
            } /* endof for */
            break;
        case AT_DESC_DEC:
            for (ix = 0; ix < pdesc->size; ix++) {
                printf("%d ", buf[ix]);
            }
            break;
        case AT_DESC_TXT:
            for (ix = 0; ix < pdesc->size; ix++) {
                printf("%c", buf[ix]);
            }
            break;
        default:
            printf("dev_24c0n_show - type\n");
            break;
        } /* endof switch type */
              printf("\n");
        free(buf);
        pdesc++;
    }

    return (PASSED); 
}



/*------------------------------------------------------------------
$Log: platform_i2c.c,v $
Revision 1.2  2021/06/02 02:56:21  alpeng
merge sears into trunk

Revision 1.1.4.2  2021/03/04 09:46:23  leschen
Remove unused I2c node /dev/i2c-0

Revision 1.1.4.1  2020/09/10 10:44:07  alpeng
fixed act2 i2c scan issue

Revision 1.1  2020/08/19 09:50:05  markzha
*** empty log message ***


$Endlog$
*/

