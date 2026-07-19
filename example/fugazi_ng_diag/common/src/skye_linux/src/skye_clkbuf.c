/* $Id: skye_clkbuf.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_clkbuf.c,v $
 *------------------------------------------------------------------
 *
 * skye_clkbuf.c: Skye Clock Buffer function and utility.
 *
 * Dec 18 2013, steja.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "string.h"
#include "skye_i2c.h"
#include "skye_clkbuf.h"
#include "skye_comm_lib.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
static int dump_all_clock_buf_util(int);
static int util_clock_buf_reg_rd(int);
static int util_clock_buf_reg_wr(int);
static int util_clock_allbuf_reg_wr(int);
static int clk_buf_register_test(void);
static int idt_program(uint16_t, uchar, uchar);
static int util_clock_buf_holdover_normal_wr_block (int opt);
static int util_clock_buf_holdover_plus_wr_block (int opt);
static int util_clock_buf_holdover_minus_wr_block (int opt);
static int test_skye_set_freq_margin_by_nc(int);


/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int skye_clk_buf_i2c_read(uint16_t, uint16_t, uchar*);
extern int skye_clk_buf_i2c_write(uint16_t, uint16_t, uchar*);
extern int skye_clk_buf_i2c_read_x(int, uint*);
extern int skye_clk_buf_i2c_write_x(int, int);
extern int getdec_answer(char *, uint, uint, uint max);
extern uchar ps_mux_ch;   /* Power Sequencer I2C Mux channel: 0 */


/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
/* reg_info_t extension for i2c access */
static reg_info_t_ext clk_buf_access = {1,
        skye_clk_buf_i2c_read_x,
        skye_clk_buf_i2c_write_x,
        0};

/* Skye GPIO Ctrl Registers */
static reg_info_t skye_gpio_ctrl_tbl[] = {
    {"GPIO direction control",    CLK_IDT8T49N283I_GPIO_CTRL_REG6E,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GPI Select 2",    CLK_IDT8T49N283I_GPIO_CTRL_REG6F,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GPI Select 1",    CLK_IDT8T49N283I_GPIO_CTRL_REG70,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GPI Select 0",    CLK_IDT8T49N283I_GPIO_CTRL_REG71,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GPO Select 2",    CLK_IDT8T49N283I_GPIO_CTRL_REG72,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GP0 Select 1",    CLK_IDT8T49N283I_GPIO_CTRL_REG73,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GP0 Select 0",    CLK_IDT8T49N283I_GPIO_CTRL_REG74,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GP0 rsvd 1",    CLK_IDT8T49N283I_GPIO_CTRL_REG75,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"GP0 rsvd 2",    CLK_IDT8T49N283I_GPIO_CTRL_REG76,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(unsigned long)&clk_buf_access},             0xFF, 0x00},
    {"end", 0x00, 0, {0}, 0, 0},
};


#ifdef DEBUG
/* Skye Startup Control Registers */
static reg_info_t sray_startup_ctrl_tbl[] = {
    {"Startup Control Reg 0",         CLK_IDT8T49N283I_STARTUP_CTRL_REG0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)ONE_B_REG},                             0x000F, 0x000B},
    {"Startup Control Reg 1",         CLK_IDT8T49N283I_STARTUP_CTRL_REG1,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)ONE_B_REG},                             0x00F0, 0x0050},   /* NOT Support SERIAL EEPROM 15-bit addressing mode */
};

/* Skye Startup Control Registers */
static reg_info_t sray_device_id_ctrl_tbl[] = {
    {"Device ID Control Reg 2",    CLK_IDT8T49N283I_DEV_ID_CTRL_REG2,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)ONE_B_REG},                             0x0000, 0x0000},
    {"Device ID Control Reg 3",    CLK_IDT8T49N283I_DEV_ID_CTRL_REG3,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)ONE_B_REG},                             0x0000, 0x0000},
    {"Device ID Control Reg 4",    CLK_IDT8T49N283I_DEV_ID_CTRL_REG4,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)ONE_B_REG},                             0x0000, 0x0000},
    {"Device ID Control Reg 5",    CLK_IDT8T49N283I_DEV_ID_CTRL_REG5,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)ONE_B_REG},                             0x000F, 0x0067},
};

/* Skye Startup Control Registers */
static reg_info_t sray_clk_buf_tbl[] = {
    {"Device ID Control",                               CURRENT_SHUNT_VOL_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Serial Interface Control",                                 CURRENT_BUS_VOL_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Digital PLL0 Control",                                       CURRENT_POWER_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Digital PLL1 Control",                                     CURRENT_CURRENT_REG,
     (READ_ONLY),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"GPIO Control",                                 CURRENT_CALIBRATION_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Output Clock Control",                                 CURRENT_MASK_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Analog PLL0 Control",                                 CURRENT_ALERT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Analog PLL1 Control",                                      CURRENT_ID_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},

    {"Power-Down Control",                                 CURRENT_MASK_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Input Monitor Control",                                 CURRENT_ALERT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Interrupt Enable",                                      CURRENT_ID_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Interrupt Status",                                 CURRENT_MASK_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Output Phase Adjustment Status",                                 CURRENT_ALERT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Digital PLL0 Status",                                      CURRENT_ID_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Digital PLL1 Status",                                      CURRENT_ID_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"General-Purpose Input Status",                CURRENT_ALERT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Global Interrupt and Boot Status",            CURRENT_ID_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
};
#endif

/******************************************************************************
 *                                 Menus
 ******************************************************************************/
/*
 * Clock Buffer utilities SubMenu Table
 */
static submenu_xtable_t clock_buf_util_table[] = {
    {"Clock Buffer Reg Test.",     (PFT)clk_buf_register_test,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Dump Skye Clock Buffer status ",(PFT)dump_all_clock_buf_util,          TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Read Clock Buffer Reg.",      (PFT)util_clock_buf_reg_rd,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Write Clock Buffer Reg.",     (PFT)util_clock_buf_reg_wr,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Write All Clock Buffer Reg.",     (PFT)util_clock_allbuf_reg_wr, TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Use normal reg_wr  (0ppm)",     (PFT)util_clock_buf_holdover_normal_wr_block, TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Use plus reg_wr  (+50ppm)",     (PFT)util_clock_buf_holdover_plus_wr_block, TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Use minus reg_wr  (-50ppm)",     (PFT)util_clock_buf_holdover_minus_wr_block, TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
     {"Test freq margin for nc",             (PFT)test_skye_set_freq_margin_by_nc, TRUE,
      0,                               (type_t(*)())0,                     0,
      (type_t(*)())0,                  0},
};

#define CLOCK_BUF_UTIL_TABLE_SZ \
        (sizeof(clock_buf_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t clock_buf_util_primary_items[CLOCK_BUF_UTIL_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t clock_buf_util_secondary_items[CLOCK_BUF_UTIL_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t clock_buf_util_menu = {
    "%s Utilities SubMenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    clock_buf_util_primary_items,
};

menuinfo_t *clock_buf_util_submenup = &clock_buf_util_menu;


/*******************************************************************************
 *
 * Function   : build_clock_buf_util_menu
 * Description: Function to build Skye Clock Buffer utility submenu.
 * Inputs     : num - number of current
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int build_clock_buf_util_menu (int num)
{
    char menu_title[32];
    snprintf(menu_title, sizeof(menu_title), "Clock Buffer");

    build_primary_submenu(clock_buf_util_table, CLOCK_BUF_UTIL_TABLE_SZ,
                          menu_title, &clock_buf_util_submenup);
    build_secondary_submenu(clock_buf_util_table, CLOCK_BUF_UTIL_TABLE_SZ,
                            clock_buf_util_secondary_items);

    /* Display Utility Menu */
    menu(clock_buf_util_submenup, clock_buf_util_secondary_items, 0);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : dump_all_clock_buf_util
 * Description: Wrapped uility to dump all Skye clock buffer.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
dump_all_clock_buf_util (int opt)
{
    int        reserved = 0;
    uint16_t off = 0, dec = 0;
    uchar   rdata = 0;

    reserved = opt;

    off = 0x0000;
    while(off < 0x0317) {
    if (skye_clk_buf_i2c_read(off, 0, (uchar *)&rdata) != PASSED) {
        printf("%s: Failed to read clock buffer register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    if ((dec % 8) == 0) {
        printf("\n");
        printf("(0x%04X): 0x%02X,", off, rdata);
    } else {
        printf("0x%02X,", rdata);
    }

    off++;
    dec++;
    }

	return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_buf_reg_rd
 * Description: Wrapped uility to read Skye clock buffer.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_clock_buf_reg_rd (int opt)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   rlen = 0, dev_addr = 0, offset = 0;
    char       devname[32];
    uchar      buf[1000];
    uchar      rdata = 0;
    uint       ix = 0;

    memset(devname, 0, sizeof(devname));
    memset(buf, 0, sizeof(buf));

    bus_no = 2;
    dev_addr = SR_CLK_BUF_I2C_ADDR;
    addr_size = 2;
    offset = gethex_answer("Enter Register offset", 0, 0, 0xFFFF);
    rlen = getdec_answer("Enter bytes number you want to read", 1, 1, 791);

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

#ifdef DEBUG
    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
        }
    }
#endif
    for (ix = 0; ix < rlen; ix++) {
        if (skye_i2c_read(fd, dev_addr, addr_size,
                           (offset + ix), ONE_B_REG, &rdata) != PASSED) {
            printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
            close(fd);
            return (FAILED);
        }
        buf[ix]=rdata;
    }

    printf("\nTotal %d bytes Data read from I2C%d, Addr. 0x%02X, "
           "from offset = 0x%02X:",
           rlen, bus_no, dev_addr, offset);
    for (ctr = 0; ctr < rlen; ctr++) {
        if ((ctr % 16) == 0) {
            printf("\n");
        }
        printf("0x%02X ", buf[ctr]);
    }
    printf("\n");

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_buf_reg_wr
 * Description: Wrapped uility to write Skye clock buffer.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_clock_buf_reg_wr (int opt)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   wlen = 0, dev_addr = 0, offset = 0;
    char       devname[32], msg[256];
    uchar      wdata[1000];
    uchar      buf[1000];
    uchar      rdata = 0;

    memset(devname, 0, sizeof(devname));
    memset(wdata, 0, sizeof(wdata));
    memset(msg, 0, sizeof(msg));

    bus_no = 2;
    dev_addr = SR_CLK_BUF_I2C_ADDR;
    addr_size = 2;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

#ifdef DEBUG
    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            return (FAILED);
        }
    }
#endif
    offset = gethex_answer("Enter Register offset", 0, 0, 0xFFFF);
    wlen = getdec_answer("Enter bytes number you want to write", 1, 1, 791);
    for (ctr = 0; ctr < wlen; ctr++) {
        if (skye_i2c_read(fd, dev_addr, addr_size,
                           (offset + ctr), ONE_B_REG, &rdata) != PASSED) {
            printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
            close(fd);
            return (FAILED);
        }
        buf[ctr]=rdata;
        snprintf(msg, sizeof(msg), "Enter data that you want to"
                                   " write into 0x%02X.",
                                   (offset + ctr));
        wdata[ctr] = gethex_answer(msg, buf[ctr], 0, 0xFF);
    }

    for (ctr = 0; ctr < wlen; ctr++) {
        if (skye_i2c_write(fd, dev_addr, addr_size,
                                (offset + ctr), ONE_B_REG, (wdata + ctr)) != PASSED) {
            printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                   __FUNCTION__, bus_no, dev_addr);
            close(fd);
            return (FAILED);
        }
    }

    printf("\nDone writing (total %d bytes) to I2C%d, Addr. 0x%02X,"
           " offset = 0x%02X.\n",
           wlen, bus_no, dev_addr, offset);

    for (ctr = 0; ctr < wlen; ctr++) {
        if (skye_i2c_read(fd, dev_addr, addr_size,
                           (offset + ctr), ONE_B_REG, &rdata) != PASSED) {
            printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
            close(fd);
            return (FAILED);
        }
        buf[ctr]=rdata;
    }

    for (ctr = 0; ctr < wlen; ctr++) {
        printf("Offset:0x%02X = 0x%02X\n", (offset + ctr), buf[ctr]);
    }

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_allbuf_reg_wr
 * Description: Wrapped uility to write Skye all clock buffer.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_clock_allbuf_reg_wr (int opt)
{
    int        fd = -1, bus_no = 0, addr_size = 0, ctr = 0;
    uint16_t   wlen = 0, dev_addr = 0, offset = 0;
    char       devname[32], msg[256];
    uchar      wdata[1000];
    uchar *clock_buffer_code = 0;
    uint16_t clock_buffer_code_size;
    uchar clock_mode;
    uchar      rdata = 0;
    extern int skye_clock_margin_normal_size;
    extern unsigned char skye_clock_margin_normal[];
    extern int skye_clock_margin_fast_size;
    extern unsigned char skye_clock_margin_fast[];
    extern int skye_clock_margin_slow_size;
    extern unsigned char skye_clock_margin_slow[];
    extern int skye_clock_margin_debug_otp_size;
    extern unsigned char skye_clock_margin_debug_otp[];

    memset(devname, 0, sizeof(devname));
    memset(wdata, 0, sizeof(wdata));
    memset(msg, 0, sizeof(msg));

    bus_no = 2;
    dev_addr = SR_CLK_BUF_I2C_ADDR;
    addr_size = 2;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    /* Power up DPLL0 and DPPL1 */
    uchar wdpll = 0x000C;  /* Bit 2 and 3 */
    offset = 0x00B8; /* Power down control register */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x00F3;
    rdata |= wdpll;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x000C) != 0x000C)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x000C);

    /* DSM_ORDm for PLL0 and PLL1 */
    uchar wdsmord = 0x00C0;  /* Bit 6 and 7 */
    offset = 0x003A; /* PLL0 */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x003F;
    rdata |= wdsmord;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x00C0) != 0x00C0)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x00C0);


    /* DSM_ORDm for PLL0 and PLL1 */
    offset = 0x006D; /* PLL1 */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x003F;
    rdata |= wdsmord;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x00C0) != 0x00C0)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x00C0);

    /* STATEx = Holdover */
    uchar wstatex = 0x0003;  /* Bit 0 and 1 */
    offset = 0x000A; /* PLL0 */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x00FC;
    rdata |= wstatex;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x0003) != 0x0003)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x0003);

    offset = 0x003D; /* PLL1 */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x00FC;
    rdata |= wstatex;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x0003) != 0x0003)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x0003);


    printf("Choose speed to test = \n");
    printf("0 = Normal speed\n");
    printf("1 = Slow speed\n");
    printf("2 = Fast speed\n");
    printf("3 = debug_write\n");
    clock_mode = getdec_answer("Enter speed buffer mode number you want to write", 0, 0, 3);
    if (clock_mode == 3) {
        clock_buffer_code_size = (uint16_t)skye_clock_margin_debug_otp_size;
        clock_buffer_code = (uchar *)skye_clock_margin_debug_otp;
    } else if (clock_mode == 1) { /* slow */
        clock_buffer_code_size = (uint16_t)skye_clock_margin_slow_size;
        clock_buffer_code = (uchar *)skye_clock_margin_slow;
    } else if (clock_mode == 2) {  /* fast */
        clock_buffer_code_size = (uint16_t)skye_clock_margin_fast_size;
        clock_buffer_code = (uchar *)skye_clock_margin_fast;
    } else {/* Normal Mode */
        clock_buffer_code_size = (uint16_t)skye_clock_margin_normal_size;
        clock_buffer_code = (uchar *)skye_clock_margin_normal;
    }
    wlen = clock_buffer_code_size;
    for (ctr = 0; ctr < wlen; ctr++) {
        wdata[ctr] = clock_buffer_code[ctr];
    }

#ifdef DEBUG
    if ((bus_no == SR_CPU_I2CM2) && (shifted_dev_addr != SR_I2C_MUX_ADDR)) {
        /* Set-up I2C MUX */
        if (skye_i2c_mux_ctrl_reg_wr(&mux_data) != PASSED) {
            printf("%s: Failed to Enable I2C Mux all channels.\n",
                   __FUNCTION__);
            return (FAILED);
        }
    }
#endif
    offset = 0;
    printf("offset = %d, wlen = %d\n", offset, wlen);
    for (ctr = 0; ctr < wlen; ctr++) {
        if (skye_i2c_write(fd, dev_addr, addr_size,
                            (offset + ctr), ONE_B_REG, (wdata + ctr)) != PASSED) {
            printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
            close(fd);
            return (FAILED);
        }
    }

    printf("\nDone writing (total %d bytes) to I2C%d, Addr. 0x%02X,"
           " offset = 0x%02X.\n",
           wlen, bus_no, dev_addr, offset);

    /* Set WDC0 = 1 for release
     */
    uchar wdco_en_bit = 0x0004;  /* Bit 2 */
    offset = 0x000A; /* PLL0 */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x00FB;
    rdata |= wdco_en_bit;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x0004) != 0x0004)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x0004);

    offset = 0x003D; /* PLL1 */
    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= 0x00FB;
    rdata |= wdco_en_bit;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & 0x0004) != 0x0004)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, 0x0004);


#ifdef DBG_CLKBUF
    for (ctr = 0; ctr < wlen; ctr++) {
        if (skye_i2c_read(fd, dev_addr, addr_size,
                           (offset + ctr), ONE_B_REG, &rdata) != PASSED) {
            printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
               __FUNCTION__, bus_no, dev_addr);
            close(fd);
            return (FAILED);
        }
        buf[ctr]=rdata;
    }

    for (ctr = 0; ctr < wlen; ctr++) {
        if (buf[ctr] != wdata[ctr]) {
            printf("data mismatch(0x%x), write= %#x, read= %#x\n",
                (ctr), wdata[ctr], buf[ctr]);
        }
    }
#endif
    close(fd);
    return (PASSED);
}


/******************************************************************************
 *
 * Function: clk_buf_register_test
 *
 * Description: This function performs the TLK 10232 register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
clk_buf_register_test (void)
{
    testname("Clock Buffer IDT8T49N283I Register");

    if (register_tests(0, skye_gpio_ctrl_tbl) == FAILED) {
        return (FAILED);
    }

    prpass(testpass, "Clock Buffer IDT8T49N283I Register Test Success");
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : idt_program
 * Description: write uility to program IDT chips
 * Inputs     : pllx - IDT offset register
 *              value - IDT Register value
 *              mask  - IDT Reg mask
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int idt_program (uint16_t pllx, uchar value, uchar mask)
{
    int        fd = -1, bus_no = 0, addr_size = 0;
    uint16_t   dev_addr = 0, offset = 0;
    char       devname[32];
    uchar      rdata = 0;
    offset = pllx;

    memset(devname, 0, sizeof(devname));

    bus_no = 2;
    dev_addr = SR_CLK_BUF_I2C_ADDR;
    addr_size = 2;

    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("%s: Failed to open /dev/i2c-%d.\n", __FUNCTION__, bus_no);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    rdata &= mask;
    rdata |= value;

    if (skye_i2c_write(fd, dev_addr, addr_size,
        offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C write to I2C%d, Addr: 0x%02X.\n",
                __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }

    if (skye_i2c_read(fd, dev_addr, addr_size,
                       offset, ONE_B_REG, &rdata) != PASSED) {
        printf("\n%s: Failed to do I2C read from I2C%d, Addr: 0x%02X.\n",
           __FUNCTION__, bus_no, dev_addr);
        close(fd);
        return (FAILED);
    }
    if ((rdata & value) != value)
        printf("\nAddr: 0x%02X = 0x%04X but expected [0x%04X]\n", offset, rdata, value);

    close(fd);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_buf_holdover_normal_wr_block
 * Description: Wrapped uility to use holdover offset programming steps
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_clock_buf_holdover_normal_wr_block (int opt)
{
    int rc;

    /* Arthur tunning new setting
     *
     * For 100MHz and 125MHz clock setting.
         0ppm setting
         exit holdover mode:                 Reg3D:0XF1
     * */

    rc = idt_program(0x003D, 0x00F1, 0x0000); /* Reg 3D = 0xF1 */

    if (rc != PASSED) {
        printf("\n%s: Failed to do program to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_buf_holdover_plus_wr_block
 * Description: Wrapped uility to use holdover offset programming steps
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_clock_buf_holdover_plus_wr_block (int opt)
{
    int rc;

    /* Arthur tunning new setting
     *
     * For 100MHz and 125MHz clock setting.
         +50ppm setting
         exit holdover mode:                 Reg3D:0XF1
         enable DPLL                         RegB8:0x04
         set DSM_ORD1:                       Reg6D:0XD0
         set offset ppm                      Reg5B:0x01
                                             Reg5C:0xDF
                                             Reg5D:0x3B
         holdover mode:                      Reg3D:0XF7
     * */

    rc = idt_program(0x003D, 0x00F1, 0x0000); /* Reg 3D = 0xF1 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 3D = 0xF1 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x00B8, 0x0004, 0x0000); /* Reg B8 = 0x04 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg B8 = 0x04 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x006D, 0x00D0, 0x0000); /* Reg 6D = 0xD0 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 6D = 0xD0 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x005B, 0x0001, 0x0000); /* Reg 5B = 0x01 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 5B = 0x01 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x005C, 0x00DF, 0x0000); /* Reg 5C = 0xDF */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 5C = 0xDF to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x005D, 0x003B, 0x0000); /* Reg 5D = 0x3B */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 5D = 0x3B to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x003D, 0x00F7, 0x0000); /* Reg 3D = 0xF7 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 3D = 0xF7 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : util_clock_buf_holdover_minus_wr_block
 * Description: Wrapped uility to use holdover offset programming steps
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_clock_buf_holdover_minus_wr_block (int opt)
{
    int rc;

    /* Arthur tunning new setting
     *
     * For 100MHz and 125MHz clock setting.
         -50ppm setting
         exit holdover mode:                 Reg3D:0XF1
         enable DPLL                         RegB8:0x04
         set DSM_ORD1:                       Reg6D:0XD0
         set offset ppm                      Reg5B:0x00
                                             Reg5C:0x20
                                             Reg5D:0xC5
         holdover mode:                      Reg3D:0XF7
     * */

    rc = idt_program(0x003D, 0x00F1, 0x0000); /* Reg 3D = 0xF1 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 3D = 0xF1 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x00B8, 0x0004, 0x0000); /* Reg B8 = 0x04 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg B8 = 0x04 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x006D, 0x00D0, 0x0000); /* Reg 6D = 0xD0 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 6D = 0xD0 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x005B, 0x0000, 0x0000); /* Reg 5B = 0x00 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 5B = 0x00 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x005C, 0x0020, 0x0000); /* Reg 5C = 0x20 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 5C = 0x20 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x005D, 0x00C5, 0x0000); /* Reg 5D = 0xC5*/
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 5D = 0xC5 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }
    rc = idt_program(0x003D, 0x00F7, 0x0000); /* Reg 3D = 0xF7 */
    if (rc != PASSED) {
        printf("\n%s: Failed to do program Reg 3D = 0xF7 to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_set_freq_margin
 * Description: Function to set Voltage margin by access Skye clock buffer
 *              related register(s).
 * Inputs     : op_mode  - wanted margin mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_set_freq_margin (uchar op_mode)
{
    /* Enable I2C Mux channel 0 */
    if (skye_i2c_mux_ctrl_reg_wr(&ps_mux_ch) != PASSED) {
        printf("\n%s: Failed to Enable I2C Mux channel %d.\n",
               __FUNCTION__, ps_mux_ch);
        return (FAILED);
    }

    if (op_mode == FREQ_NORMAL)
    {
        printf("DBG:Run Freq Normal\n");
        if (util_clock_buf_holdover_normal_wr_block(0) == FAILED) {
            printf("Failed setup normal frequency margin\n ");
            return (FAILED);
        }
    } else if (op_mode == FREQ_MARGIN_PLUS)
    {
        printf("DBG:Run Freq Plus\n");
        if (util_clock_buf_holdover_plus_wr_block(0) == FAILED) {
            printf("Failed setup normal frequency margin\n ");
            return (FAILED);
        }
    } else if (op_mode == FREQ_MARGIN_MINUS)
    {
        printf("DBG:Run Freq Minus\n");
        if (util_clock_buf_holdover_minus_wr_block(0) == FAILED) {
            printf("Failed setup normal frequency margin\n ");
            return (FAILED);
        }
    } else {
        printf("\n Unknown op_mode !!\n");
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_set_freq_margin_by_nc
 * Description: Function to set Skye frequency margin by NC command.
 * Inputs     : *nc_cmd - buffer to put margin setup from user
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_set_freq_margin_by_nc (char *nc_cmd)
{
    char            *token = NULL;
    uchar           op_mode = 0x00;
    char            fm_name[64];
    FILE *fp;

    fp = fopen(FREQ_MARGIN_TMP_RESULT, "a+");
    if (fp == NULL) {
        printf("%s: Open %s fails\n", __FUNCTION__, FREQ_MARGIN_TMP_RESULT);
        return (FAILED);
    }

    /* Get the NC command set(s) */
    token = strtok(nc_cmd, ";");

    if (token == NULL) {
        printf("%s: Got NULL NC command string.\n", __FUNCTION__);
        fprintf(fp, "%s: Got NULL NC command string.\n", __FUNCTION__);
        fclose(fp);
        return (FAILED);
    }

    if (strcmp("P", token) == 0) {
        sprintf(fm_name, "Plus (+50 PPM)");
        fprintf(fp, "Plus (+50 PPM)");
        op_mode = FREQ_MARGIN_PLUS;
    } else if (strcmp("N", token) == 0) {
        sprintf(fm_name, "Normal (0 PPM)");
        fprintf(fp, "Normal (0 PPM)");
        op_mode = FREQ_NORMAL;
    } else if (strcmp("M", token) == 0) {
        sprintf(fm_name, "Minus (-50 PPM)");
        fprintf(fp, "Minus (-50 PPM)");
        op_mode = FREQ_MARGIN_MINUS;
    } else {
        printf("%s: Unknown command %s.\n", __FUNCTION__, token);
        fprintf(fp, "%s: Unknown command %s.\n", __FUNCTION__, token);
        fclose(fp);
        return (FAILED);
    }

    /* Set voltage margin */
    if (skye_set_freq_margin(op_mode) != PASSED) {
        printf("%s: Failed to set %s.\n", __FUNCTION__, fm_name);
        fprintf(fp, "%s: Failed to set %s.\n", __FUNCTION__, fm_name);
        fclose(fp);
        return (FAILED);
    }

    fclose(fp);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : test_skye_set_freq_margin_by_nc
 * Description: Function to Debug Skye frequency margin by NC command.
 * Inputs     : opt - optional
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
static test_skye_set_freq_margin_by_nc (int opt)
{
    char buff[10];
    int clock_mode = 0;

    printf("Choose speed to test = \n");
    printf("0 = Normal speed\n");
    printf("1 = Slow speed\n");
    printf("2 = Fast speed\n");

    clock_mode = getdec_answer("Enter speed buffer mode number you want to write", 0, 0, 2);
    if (clock_mode == 2) {
        strcpy(buff, "P;");
    } else if (clock_mode == 0) {
        strcpy(buff, "N;");
    } else if (clock_mode == 1) {
        strcpy(buff, "M;");
    }

    if (skye_set_freq_margin_by_nc(buff) != PASSED) {
        printf("test skye set freq margin failed\n");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : disable_clk_buf
 * Description: Function to Disable Clock generator channel 4 to ch 7
 * Inputs     : opt - optional
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int disable_clk_buf (void)
{
    int rc;
    /* Enable I2C Mux channel 0 */
    if (skye_i2c_mux_ctrl_reg_wr(&ps_mux_ch) != PASSED) {
        printf("\n%s: Failed to Enable I2C Mux channel %d.\n",
               __FUNCTION__, ps_mux_ch);
        return (FAILED);
    }

    rc = idt_program(0x0077, 0x000F, 0x0000); /* Reg 77 = 0x0f */

    if (rc != PASSED) {
        printf("\n%s: Failed to do program to idt chips\n",
                __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/******** History ********
$Log: skye_clkbuf.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.4  2015/05/11 13:45:45  steja
Code clean up <CSCuu14285>

Revision 1.1.4.3  2015/04/30 08:33:53  steja
Clean up code

Revision 1.1.4.2  2015/04/29 11:36:34  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.9  2015/04/22 12:53:59  steja
To disable clock generator ch4 to ch7 to increase performance on the clock.

Revision 1.1.2.8  2015/01/26 01:14:24  steja
Add function for frequency margin to host side menu utilities through NC

Revision 1.1.2.7  2014/12/31 09:25:58  steja
Remove debug info and remove unused code

Revision 1.1.2.6  2014/12/12 00:50:39  steja
Check in for tune clock buffer debugging.

Revision 1.1.2.5  2014/11/10 09:48:02  steja
Update Clock buffer utility

Revision 1.1.2.4  2014/10/07 13:53:29  steja
Update read and write clock buffer function

Revision 1.1.2.3  2014/10/03 07:29:46  steja
Update dump clock buffer and debug table

Revision 1.1.2.2  2014/08/08 03:45:04  steja
Add Clock Buffer Register Test

Revision 1.1.2.1  2014/07/21 01:56:54  palin2
Initial check-in Skye module side Diag code.

------------------------------------------------------------
Revision 1.2  2014/02/27 15:01:48  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.2  2014/02/07 03:33:33  steja
add debug test

Revision 1.1.2.1  2014/01/13 17:19:41  steja
Add Clock Buffer utility Read and Write to Basic Utilities

------------------------------------------------------------
$Endlog$
*/
