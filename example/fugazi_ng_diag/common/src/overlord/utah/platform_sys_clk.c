/* $Id: platform_sys_clk.c,v 1.21 2018/08/30 06:59:43 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_sys_clk.c,v $
 *------------------------------------------------------------------------------
 * platform_sys_clk.c - Utah SYS CLK chip main function/menu.
 *
 * IDT 9VRS420B clock gen. 
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */
#include <string.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include "cpu.h"
#include "platform_i2c.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "cli_cmd.h"
#include "platform_sys_clk.h"
#include "dash_fpga.h"


/******************************************************************************
 *                              Function proto
 ******************************************************************************/
static int show_reg(uint32);
static int output_clock_ctrl(uint);
static int init_dev_struct(n2g_i2c_if_t *);
static int get_regs_value(n2g_i2c_if_t *);
static void set_byte_count(int);

static void build_toggle_sys_clk_util_menu(void);

extern int reset_sys_by_watchdog(void);
extern int do_all_menu_items(struct menuinfo *);

/******************************************************************************
 *                                  Globals 
 ******************************************************************************/

/* for control extenal registers read */
boolean ext_reg_rd = FALSE;

/* Bit define for the pin usage for the cpu clocks */
static rs4420b_bit_t sys_clk_bit_d[] = {
    {"XDP Hook 4 & 5 (BCLK)",    0, 0, RS4420B_OUT_EN3, RS4420B_SRC1_OE},
    {"Rangeley HPLL",            0, 0, RS4420B_OUT_EN3, RS4420B_SRC2_OE},
    {"Rangeley DDR3_0_REF",      0, 0, RS4420B_OUT_EN3, RS4420B_SRC3_OE},
    {"Rangeley DDR3_1_REF",      0, 0, RS4420B_OUT_EN2, RS4420B_SRC4_OE},
    {"9ZXL1231 PCIe Buffer DIF_IN", 0, 0, RS4420B_OUT_EN2, RS4420B_SRC5_OE},
    {"Rangeley GBE_REFCLK",       0, 0, RS4420B_OUT_EN3, RS4420B_DIF0_OE},
    {"Rangeley SATA3_REFCLK",     0, 0, RS4420B_OUT_EN2, RS4420B_SATA_OE},
    {"Rangeley USB_REFCLK",       0, 0, RS4420B_OUT_EN3, RS4420B_DOT96_OE},
    {"FPGA",                      0, 0, RS4420B_OUT_EN1, RS4420B_25M_PCI4_OE},
    {"Rangeley CLK14_IN",         0, 0, RS4420B_OUT_EN1, RS4420B_REF0_OE},
    {0, 0, 0, 0, 0},
};

/* Submenu output_clock_ctrl pin conversion table */
static sys_clk_conv_t cpu_clkbuf_conv_table[] = {
    {CLKBUF_ENT_CHAR, UTAH_XDP_HOOK_4AND5,      RS4420B_OUT_EN3, RS4420B_SRC1_OE},
    {CLKBUF_ENT_CHAR, UTAH_RANGELEY_HPLL,       RS4420B_OUT_EN3, RS4420B_SRC2_OE},
    {CLKBUF_ENT_CHAR, UTAH_RENGELEY_DDR3_0_REF, RS4420B_OUT_EN3, RS4420B_SRC3_OE},
    {CLKBUF_ENT_CHAR, UTAH_RANGELEY_DDR3_1_REF, RS4420B_OUT_EN2, RS4420B_SRC4_OE},
    {CLKBUF_ENT_CHAR, UTAH_PCIE_BIF_DIF_IN,     RS4420B_OUT_EN2, RS4420B_SRC5_OE},
    {CLKBUF_ENT_CHAR, UTAH_RANGELEY_GBE_REFCLK, RS4420B_OUT_EN3, RS4420B_DIF0_OE},
    {CLKBUF_ENT_CHAR, UTAH_RANGELEY_SATA3_REFCLK, RS4420B_OUT_EN2, RS4420B_SATA_OE},
    {CLKBUF_ENT_CHAR, UTAH_RANGELEY_USB_REFCLK, RS4420B_OUT_EN3, RS4420B_DOT96_OE},
    {CLKBUF_ENT_CHAR, UTAH_OPTION_FOR_FPGA,     RS4420B_OUT_EN1, RS4420B_25M_PCI4_OE},
    {CLKBUF_ENT_CHAR, UTAH_RANGELEY_CLK14_IN,   RS4420B_OUT_EN1, RS4420B_REF0_OE},
    {CLKBUF_ENT_END, 0, 0, 0},
};
/* Goldbeach : Bit define for the pin usage for the cpu clocks */
static rs4420b_bit_t gb_sys_clk_bit_d[] = {
    {"Rangeley PCIe REF",      0, 0, RS4420B_OUT_EN3, RS4420B_SRC1_OE},
    {"Rangeley HPLL",            0, 0, RS4420B_OUT_EN3, RS4420B_SRC2_OE},
    {"Rangeley DDR3_0_REF", 0, 0, RS4420B_OUT_EN3, RS4420B_SRC3_OE},
    {"NIM Slot 1",                  0, 0, RS4420B_OUT_EN2, RS4420B_SRC4_OE},
    {"Goldbeach FPGA",           0, 0, RS4420B_OUT_EN2, RS4420B_SRC5_OE},
    {"Rangeley USB_REFCLK",   0, 0, RS4420B_OUT_EN3, RS4420B_DIF0_OE},
    {"Rangeley SATA3_REFCLK",     0, 0, RS4420B_OUT_EN2, RS4420B_SATA_OE},
    {"Rangeley GBE_REFCLK",       0, 0, RS4420B_OUT_EN3, RS4420B_DOT96_OE},
    {"NIM Slot 2 ",                      0, 0, RS4420B_OUT_EN1, RS4420B_25M_PCI4_OE},
    {"Rangeley CLK14_IN",         0, 0, RS4420B_OUT_EN1, RS4420B_REF0_OE},
    {0, 0, 0, 0, 0},
};
/* Goldbeach : Submenu output_clock_ctrl pin conversion table */
static sys_clk_conv_t gb_cpu_clkbuf_conv_table[] = {
    {CLKBUF_ENT_CHAR, GB_PCIE_BIF_DIF_IN,      RS4420B_OUT_EN3, RS4420B_SRC1_OE},
    {CLKBUF_ENT_CHAR, GB_RANGELEY_HPLL,       RS4420B_OUT_EN3, RS4420B_SRC2_OE},
    {CLKBUF_ENT_CHAR, GB_RENGELEY_DDR3_0_REF, RS4420B_OUT_EN3, RS4420B_SRC3_OE},
    {CLKBUF_ENT_CHAR, GB_NIM_SLOT_1, RS4420B_OUT_EN2, RS4420B_SRC4_OE},
    {CLKBUF_ENT_CHAR, GB_OPTION_FOR_FPGA,     RS4420B_OUT_EN2, RS4420B_SRC5_OE},
    {CLKBUF_ENT_CHAR, GB_RANGELEY_USB_REFCLK, RS4420B_OUT_EN3, RS4420B_DIF0_OE},
    {CLKBUF_ENT_CHAR, GB_RANGELEY_SATA3_REFCLK, RS4420B_OUT_EN2, RS4420B_SATA_OE},
    {CLKBUF_ENT_CHAR, GB_RANGELEY_GBE_REFCLK, RS4420B_OUT_EN3, RS4420B_DOT96_OE},
    {CLKBUF_ENT_CHAR, GB_NIM_SLOT_2,     RS4420B_OUT_EN1, RS4420B_25M_PCI4_OE},
    {CLKBUF_ENT_CHAR, GB_RANGELEY_CLK14_IN,   RS4420B_OUT_EN1, RS4420B_REF0_OE},
    {CLKBUF_ENT_END, 0, 0, 0},
};

/* Register table */
static reg_info_t sq420d_default_reg_tbl[] = {
    {"Byte 0: Frequency Select, PD Config and SATA Source Select",      0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x83, 0x81},
    {"Byte 1: DOT96/SRC6 Control",                                      0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x80, 0x0},
    {"Byte 2: Output Enable Control ",                                  0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xDE, 0xFF},
    {"Byte 3: Output Enable Control ",                                  0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0x07, 0xFF},
    {"Byte 4: Output Enable and SS Enable Control ",                    0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0xFD, 0xFC},
    {"Byte 5: CLKREQ_A# and CLKREQB# Mapping",                          0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xDC, 0x10},
    {"Byte 6: DIF STOP and Standby Control ",                           0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x1F, 0x18},
    {"Byte 7: Vendor & Rev ID",                                  0,
     (READ_ONLY | READ_ONLY | REG_ACCESS),     {(uint)REG_EXT}, 0xFF, 0x11},
    {"Byte 8: Intentional PCI Skew control register ",           0,
     (READ_ONLY | READ_ONLY | REG_ACCESS),     {(uint)REG_EXT}, 0x00, 0x00},
    {"Byte 9: Byte Count",                                       0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x1F, 0x0A},
    {"Byte 10: Single-Ended Slew Rate Control ",                 0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0xFE, 0x15},
    {"Byte 11: Differential and Single Ended Slew Rate Ctrl ",   0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0xFF, 0xF5},
    {"Byte 12: M/N Programming Enable ",                         0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0x83, 0x00},
    {"Byte 13: Readback, PCI STOP and WLAN Enable Control ",     0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0x7E, 0x4F},
    {0, 0, 0, {0}, 0, 0},
};


static n2g_i2c_if_t sys_clk_if[] = {
    {
        .offset = 0,
        .i2c_bus_type = CPU_I2C1,
        .i2c_dev = MB_I2C_ADDR_SYS_CLK,
        .size = sizeof(uint32_t),
        .rd_hd_size = I2C_SMBUS_BLOCK_DATA,
        .wr_hd_size = I2C_SMBUS_BLOCK_DATA,
        .sub_addr_len = 0,
        .mux = 0,
        .buf = NULL,
    },
};

/******************************************************************************
 *                                    Menus
 ******************************************************************************/ 
/*********************************************************************
 *              I2C devices characteristics tables.
 *    this not the full table from i2c_api.c
 *    and it should follow the order (enum) defined in platform_i2c.h
 *********************************************************************
 */
/*
/linux_diag/ptong/utah # ./i2cdump  0  0x69 s
I will probe file /dev/i2c-0, address 0x69, mode smbus block
Continue? [Y/n] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 01 00 ff ff fd 10 18 11 00 0a                      ?...????.?
*/

/*
 * System CLK Main menu
 */
static submenu_xtable_t sys_clk_menu_table[] = {
    {"Toggle CLKs utility",      (PFT)build_toggle_sys_clk_util_menu, TRUE,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
    {"Show SYS CLK registers",   (PFT)show_reg,                0,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
};

#define SYS_CLK_MENU_TABLE_SIZE \
        (sizeof(sys_clk_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t sys_clk_menu_primary_items[SYS_CLK_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t sys_clk_menu_secondary_items[SYS_CLK_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo sys_clkdiag = {
  "SYS CLK Utilities Menu",     /* title */
  0,				/* title string added by init_empty_menu */
  (PFT)menu_show_dflags,	/* shows major flags */
  0,				/* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  sys_clk_menu_primary_items,
};
static struct menuinfo *sys_clkdiagp = &sys_clkdiag;

/*
 * Toggle System CLKs utility menu
 */
#ifdef GOLDBEACH
static submenu_xtable_t toggle_clks_util_menu_table[] = {
    {"Rangeley PCIe REF",             (PFT)output_clock_ctrl,  GB_PCIE_BIF_DIF_IN,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley HPLL",                    (PFT)output_clock_ctrl,  GB_RANGELEY_HPLL, 
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley DDR3_0_REF",           (PFT)output_clock_ctrl,  GB_RENGELEY_DDR3_0_REF,  
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"NIM Slot 1",                          (PFT)output_clock_ctrl,  GB_NIM_SLOT_1,  
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Goldbeach FPGA ",                 (PFT)output_clock_ctrl,  GB_OPTION_FOR_FPGA,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley USB_REFCLK",           (PFT)output_clock_ctrl,  GB_RANGELEY_USB_REFCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley SATA3_REFCLK",         (PFT)output_clock_ctrl,  GB_RANGELEY_SATA3_REFCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley GBE_REFCLK",           (PFT)output_clock_ctrl,  GB_RANGELEY_GBE_REFCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"NIM Slot 2 ",                         (PFT)output_clock_ctrl,  GB_NIM_SLOT_2,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley CLK14_IN",             (PFT)output_clock_ctrl,  GB_RANGELEY_CLK14_IN,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
};
#else
static submenu_xtable_t toggle_clks_util_menu_table[] = {
    {"XDP Hook 4 & 5 (BCLK)",         (PFT)output_clock_ctrl,  UTAH_XDP_HOOK_4AND5,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley HPLL",                 (PFT)output_clock_ctrl,  UTAH_RANGELEY_HPLL, 
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley DDR3_0_REF",           (PFT)output_clock_ctrl,  UTAH_RENGELEY_DDR3_0_REF,  
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley DDR3_1_REF",           (PFT)output_clock_ctrl,  UTAH_RANGELEY_DDR3_1_REF,  
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"9ZXL1231 PCIe Buffer DIF_IN",   (PFT)output_clock_ctrl,  UTAH_PCIE_BIF_DIF_IN,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley GBE_REFCLK",           (PFT)output_clock_ctrl,  UTAH_RANGELEY_GBE_REFCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley SATA3_REFCLK",         (PFT)output_clock_ctrl,  UTAH_RANGELEY_SATA3_REFCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley USB_REFCLK",           (PFT)output_clock_ctrl,  UTAH_RANGELEY_USB_REFCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Option for FPGA ",              (PFT)output_clock_ctrl,  UTAH_OPTION_FOR_FPGA,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Rangeley CLK14_IN",             (PFT)output_clock_ctrl,  UTAH_RANGELEY_CLK14_IN,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
};
#endif

#define TOGGLE_CLKS_UTIL_MENU_TABLE_SIZE \
        (sizeof(toggle_clks_util_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t toggle_clks_util_menu_primary_items[TOGGLE_CLKS_UTIL_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t toggle_clks_util_menu_secondary_items[TOGGLE_CLKS_UTIL_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo toggle_clks_util = {
  "Toggle SYS CLKs Utility Menu",         /* title */
  0,                                      /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,                  /* shows major flags */
  0,                                      /* generic prompt */
  0,                                      /* size -- bumped by add_menu_item() */
  toggle_clks_util_menu_primary_items,
};
static struct menuinfo *toggle_clks_utilp = &toggle_clks_util;


/*******************************************************************************
 *
 * Function   : build_sys_clk_menu
 * Description: Build System CLK menu.
 * Inputs     : None.
 * Outputs    : None.
 *
 *******************************************************************************
 */
void build_sys_clk_menu (uint32_t menu_opt)
{
    build_primary_submenu(sys_clk_menu_table, SYS_CLK_MENU_TABLE_SIZE,
			  "SYS CLK Utilities Menu", &sys_clkdiagp);
    build_secondary_submenu(sys_clk_menu_table, SYS_CLK_MENU_TABLE_SIZE,
			    sys_clk_menu_secondary_items);

    if (menu_opt) {
        /* Entered with submenu */
        menu(&sys_clkdiag, sys_clk_menu_secondary_items, 0);
    } else {
        do_all_menu_items(sys_clkdiagp);
    }
}


/*******************************************************************************
 *
 * Function   : build_toggle_sys_clk_util_menu
 * Description: Build toggle System CLK utilities menu.
 * Inputs     : None.
 * Outputs    : None.
 *
 *******************************************************************************
 */
static void build_toggle_sys_clk_util_menu (void)
{
    build_primary_submenu(toggle_clks_util_menu_table,
                          TOGGLE_CLKS_UTIL_MENU_TABLE_SIZE,
                          "SYS CLK Utilities Menu",
                          &toggle_clks_utilp);
    build_secondary_submenu(toggle_clks_util_menu_table,
                            TOGGLE_CLKS_UTIL_MENU_TABLE_SIZE,
                            toggle_clks_util_menu_secondary_items);
    menu(&toggle_clks_util, toggle_clks_util_menu_secondary_items, 0);
}


/*******************************************************************************
 * 
 * Function   : init_dev_struct
 * Description: Init CLK device structure
 * Inputs     : sq420d - Points to CLK device object
 *              i2c_if - Points to I2C API interface struct
 *              clkbuf_reg - Points to CLK reg struct
 * Outputs    : None
 *
 *******************************************************************************
 */
static int init_dev_struct (n2g_i2c_if_t *i2c_if)
{
    memcpy(i2c_if, &sys_clk_if[SYS_CLK_DEF], sizeof(n2g_i2c_if_t));
    return (PASSED);
}

/*******************************************************************************
 * 
 * Function   : set_byte_count
 * Description: set byte count before reading or writing.
 * Inputs     : size - the num of byte we want to r/w.
 * Outputs    : None
 *
 *******************************************************************************
 */
void set_byte_count (int size)
{
    int rc = FAILED;
    n2g_i2c_if_t i2c_if;
    char reg_bytecount;

    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
                      __FUNCTION__);
        return;
    }

    /* Set byte count register offset and value */
    i2c_if.buf = &reg_bytecount;
    i2c_if.offset = RS4420B_BYTE_CNT;
    reg_bytecount = size;

    /* Write in byte count value(Byte 9) */
    rc = i2c_dev_wr((void *)&i2c_if, 1);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED to write data to system clock (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return;
    }

    return ;
}


/*******************************************************************************
 *
 * Function   : get_regs_value
 * Description: Get all SYS CLK registers value
 * Inputs     : *i2c_if - pointer of i2c interface data structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_regs_value (n2g_i2c_if_t *i2c_if)
{
    int rc = FAILED, result = -1;

    /* Get Registers value */
    result = i2c_dev_rd((void *)i2c_if);
    if (result < 0) {
        cterr('f', 0, "cpu i2c:FAILED to read registers from sys clock" );
        return (rc);
    }

#if 0
    if (ext_reg_rd) 
        check_size = SYS_CLK_EXT_BUF_SIZE;
    else 
        check_size = SYS_CLK_BUF_SIZE;

    if (result != check_size) {
        cterr('f', 0, "cpu i2c: expect %d bytes from 9VRS4420B  but received %d",
               check_size, result);
    } else {
        rc = PASSED;
    }
#endif

    return PASSED;
}


/*******************************************************************************
 *
 * Function   : dump_regs_value
 * Description: Dump all SYS CLK registers value
 * Inputs     : *i2c_if - pointer of i2c interface data structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dump_regs_value (n2g_i2c_if_t *i2c_if)
{
    int        rc = FAILED, ctr = 0;
    char      *reg_tmp;
    reg_info_t *reg_info_p;

    /* Get Registers value */
    rc = get_regs_value(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to get SYS_CLK 9VRS4420B registers"
                      " value (rc = %#x).", __FUNCTION__, rc);
        return (rc);
    }

    /* Dump Registers value */
    reg_tmp = i2c_if->buf;
    reg_info_p = &sq420d_default_reg_tbl[0];

    printf("\nSYS CLK registers:\n");
    for (ctr = 0; ctr < SYS_CLK_BUF_SIZE; ctr++, reg_info_p++, reg_tmp++) {
         printf("%-32s = 0x%.2X\n", reg_info_p->name, *(reg_tmp));
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : show_reg
 * Description:	Display CLKBUF registers
 * Inputs     : clkbuf_n - Clock buffer number, CPU_CLKBUF or SM_CLKBUF
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int show_reg (uint clkbuf_n)
{
    n2g_i2c_if_t i2c_if;
    int          rc = FAILED;
    char        reg_val[SYS_CLK_BUF_SIZE];

    /* set read byte count before reading */
    set_byte_count(SYS_CLK_BUF_SIZE); 

    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struc.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* Dump Registers value */
    i2c_if.buf = &reg_val[0];

    rc = dump_regs_value(&i2c_if);
    if (rc != PASSED) {
        return (FAILED);
    }

    set_byte_count(SYS_CLK_BUF_SIZE_DFT); 

    return (rc);
}


/*******************************************************************************
 *
 * Function   : output_clock_ctrl 
 * Description: Update one bit in CLKBUF registers
 * Inputs     : field_type - Bit to be changed.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int output_clock_ctrl (uint field_type)
{
    int rc = FAILED, index_ctr = 0;
    n2g_i2c_if_t i2c_if;
    char answer, *true, *false;
    sys_clk_conv_t *index_p;
    char reg_val[SYS_CLK_BUF_SIZE];

    if (is_goldbeach() || is_vg400()) {
        index_p = &gb_cpu_clkbuf_conv_table[0];
    } else {
        index_p = &cpu_clkbuf_conv_table[0];
    }
    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
                      __FUNCTION__);
        return (FAILED);
    }

    /* Traverse through the conversion table to find the entry */
    while(index_p->entry_type != CLKBUF_ENT_END) {
        if (index_p->sys_clk_type == field_type) {
            /* Got the entry */
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\nindex_p->offset = %#x.\n", index_p->offset);
            }
            break;
        }
        index_p++, index_ctr++;
    } /* endof while */

    /* Get Registers value */
    i2c_if.buf = &reg_val[0];

    rc = get_regs_value(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED to get SYS_CLK registers value (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return (FAILED);
    }

    /* Check the current state of the bit */
    switch(index_p->entry_type) {
    case CLKBUF_ENT_CHAR:
        true = "Enabled";
        false = "Disabled";
        if (is_goldbeach() || is_vg400()) {
            printf("Original %s is %s.\n", gb_sys_clk_bit_d[index_ctr].name,
                  (reg_val[index_p->offset] & index_p->mask) ? true : false);
        } else {
            printf("Original %s is %s.\n", sys_clk_bit_d[index_ctr].name,
                  (reg_val[index_p->offset] & index_p->mask) ? true : false);
        }
        printf("Enter 1 for %s or 0 for %s", true, false);
        answer = getc_answer(" - ", "01", '1');

        if (answer == '1') {
            /* Set the mask */
            answer = index_p->mask;
        } else {
            /* Clear the bit */
            answer = 0;
        }
        break;
    default:
        assert(!"clkbuf output_clock_ctrl entry_type");
        break;
    }  /* endof switch entry_type */

    reg_val[index_p->offset] &= (~index_p->mask);  /* Clear old value */
    reg_val[index_p->offset] |= answer;            /* Set new value */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("after: reg_val[%d] = %#x.\n",
                index_p->offset, reg_val[index_p->offset]);
    }

    /* Write in registers value */
    rc = i2c_dev_wr((void *)&i2c_if, SYS_CLK_BUF_SIZE);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED to write data to system clock (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    /* Dump all CLKs status */
    rc = get_regs_value(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to get SYS_CLK 9VRS4420B registers"
                      " value (rc = %#x).", __FUNCTION__, __LINE__, rc);
        return (rc);
    }
    if (is_goldbeach() || is_vg400()) {
        printf("\nNow %s is %s.\n", gb_sys_clk_bit_d[index_ctr].name,
               (reg_val[index_p->offset] & index_p->mask) ? true : false);
    } else {
        printf("\nNow %s is %s.\n", sys_clk_bit_d[index_ctr].name,
               (reg_val[index_p->offset] & index_p->mask) ? true : false);
    }

    return (rc);
}

/*******************************************************************************
 *
 * Function   : rs4420b_get_reg
 * Description: This function to get the desired register value from 9VRS4420B.
 * Inputs     : reg_offset - Offset of desired register
 *              val_buf - Pointer of buffer to store the desired data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int rs4420b_get_reg (int reg_offset, char *val_buf)
{
    n2g_i2c_if_t i2c_if;
    int          rc = FAILED;
    char        reg_val[SYS_CLK_BUF_SIZE];

    /* set read byte count before reading */
    set_byte_count(SYS_CLK_BUF_SIZE); 

    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
              __FUNCTION__);
        return (FAILED);
    }

    /* Dump Registers value */
    i2c_if.buf = &reg_val[0];

    /* Get Registers value */
    rc = get_regs_value(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to read registers(rc = %#x) from 9VRS4420D.",
                      __FUNCTION__, rc);
        return (FAILED);
    }

    *val_buf = reg_val[reg_offset];
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d val_bug = %#x.\n", __FUNCTION__, __LINE__, *val_buf);
    }

    set_byte_count(SYS_CLK_BUF_SIZE_DFT); 

    return (rc);
}

/*******************************************************************************
 *
 * Function   : rs4420b_get_ext_reg
 * Description: This function to get the extended register value from 9VRS4420B.
 * the data is composed : (reg 22|SS(reg17)|SS(reg16)|VCO N(reg15)|REF M(reg14))
 * Inputs     : val_buf - Pointer of buffer to store the desired data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int rs4420b_get_ext_reg (ulong *val_buf)
{
    n2g_i2c_if_t i2c_if;
    int          rc = FAILED;
    char        reg_val[SYS_CLK_EXT_BUF_SIZE];
    ulong        reg23, reg17, reg16, reg15, reg14, tmp = 0; 

    ext_reg_rd = TRUE;

    /* set read byte count before reading */
    set_byte_count(SYS_CLK_EXT_BUF_SIZE); 

    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
              __FUNCTION__);
        return (FAILED);
    }

    /* Dump Registers value */
    i2c_if.buf = &reg_val[0];

    /* Get Registers value */
    rc = get_regs_value(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to read registers(rc = %#x) from 9VRS4420D.",
                      __FUNCTION__, rc);
        return (FAILED);
    }

    reg23 = reg_val[RS4420B_EXT_BYTE23];
    reg17 = reg_val[RS4420B_EXT_BYTE17];
    reg16 = reg_val[RS4420B_EXT_BYTE16];
    reg15 = reg_val[RS4420B_EXT_BYTE15];
    reg14 = reg_val[RS4420B_EXT_BYTE14];
    
    tmp |= ((reg23 << RS4420B_FREQ_REG23_OFS) & RS4420B_FREQ_REG23_MSK);
    tmp |= ((reg17 << RS4420B_FREQ_REG17_OFS) & RS4420B_FREQ_REG17_MSK);
    tmp |= ((reg16 << RS4420B_FREQ_REG16_OFS) & RS4420B_FREQ_REG16_MSK);
    tmp |= ((reg15 << RS4420B_FREQ_REG15_OFS) & RS4420B_FREQ_REG15_MSK);
    tmp |= ((reg14 << RS4420B_FREQ_REG14_OFS) & RS4420B_FREQ_REG14_MSK);

    *val_buf = tmp;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d val_buf = %#lx.\n", __FUNCTION__, __LINE__, *val_buf);
    }

    ext_reg_rd = FALSE;

    set_byte_count(SYS_CLK_BUF_SIZE_DFT); 

    return (rc);
}

/*******************************************************************************
 *
 * Function   : mgn_regs_val_convert
 * Description: convert the margin register values for rs4420b reversion D.
 * Inputs     : desired_freq - margin register values for reversion B
 * Outputs    : margin register values for reversion D 
 *
 *******************************************************************************
 */
static ulong mgn_regs_val_convert (ulong desired_freq)
{
    ulong desired_freq_rev_d;

    switch (desired_freq) {
        case RS4420B_FREQ_SEL_M3P0:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_M3P0;
            break;
        case RS4420B_FREQ_SEL_M2P5:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_M2P5;
            break;
        case RS4420B_FREQ_SEL_M2P0:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_M2P0;
            break;
        case RS4420B_FREQ_SEL_M1P5:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_M1P5;
            break;
        case RS4420B_FREQ_SEL_M1P0:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_M1P0;
            break;
        case RS4420B_FREQ_SEL_M0P5:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_M0P5;
            break;
        case RS4420B_FREQ_SEL_NORM:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_NORM;
            break;
        case RS4420B_FREQ_SEL_P0P5:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_P0P5;
            break;
        case RS4420B_FREQ_SEL_P1P0:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_P1P0;
            break;
        case RS4420B_FREQ_SEL_P1P5:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_P1P5;
            break;
        case RS4420B_FREQ_SEL_P2P0:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_P2P0;
            break;
        case RS4420B_FREQ_SEL_P2P5:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_P2P5;
            break;
        case RS4420B_FREQ_SEL_P3P0:
            desired_freq_rev_d = RS4420BD_FREQ_SEL_P3P0;
            break;
        default:
            printf("\nUnknown desired frequency margin %#lx\n", desired_freq);
            assert(!"Unknown desired frequency margin");
            break;
        }

    return desired_freq_rev_d;
}

/*******************************************************************************
 *
 * Function   : rs4420b_margin
 * Description: This function enable/disable the margin on rs4420b.
 * Inputs     : en_mn_prog - enable M/N programming is to enable the margin. 
 *              To disable the margin, we just disable M/N programming. 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
void rs4420b_margin (boolean en_mn_prog)
{
    int rc = FAILED;
    n2g_i2c_if_t i2c_if;
    char reg_val[SYS_CLK_BUF_SIZE];

    /* set read byte count before reading */
    set_byte_count(SYS_CLK_BUF_SIZE); 

    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
                      __FUNCTION__);
        return;
    }

    /* Get Registers value */
    i2c_if.buf = &reg_val[0];

    rc = get_regs_value(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED to get SYS_CLK registers value (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return;
    }

    if (en_mn_prog) {
        reg_val[RS4420B_FR_SE] |= RS4420B_SATA_SEL; /* fixed SATA report warning */
        reg_val[RS4420B_MN_PROG] |= RS4420B_DIF_SRC_PCI_MN_EN;
        reg_val[RS4420B_OUT_EN3] &= ~RS4420B_SRC_PLL_EN;
    } else { 
        reg_val[RS4420B_FR_SE] &= ~RS4420B_SATA_SEL; /* fixed SATA report warning */
        reg_val[RS4420B_MN_PROG] &= ~RS4420B_DIF_SRC_PCI_MN_EN;
        reg_val[RS4420B_OUT_EN3] &= ~RS4420B_SRC_PLL_EN;
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("after: reg_val[%d] = %#x.\n",
                RS4420B_MN_PROG, reg_val[RS4420B_MN_PROG]);
    }

    /* Write in registers value */
    rc = i2c_dev_wr((void *)&i2c_if, SYS_CLK_BUF_SIZE);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED to write data to system clock (rc = %#x).",
                      __FUNCTION__, __LINE__, rc);
        return;
    }

    set_byte_count(SYS_CLK_BUF_SIZE_DFT); 

    return; 
}

/*******************************************************************************
 *
 * Function   : rs4420b_set_freq
 * Description: This function to set the desired frequency to 9VRS4420B.
 * Inputs     : desired_freq - the desired frequency value
 *              dummy1(spread) - only used for CLI mode 
 *              dummy2(mode) - for menu mode or CLI mode 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int rs4420b_set_freq (ulong desired_freq, boolean dummy1, boolean dummy2)
{
    n2g_i2c_if_t i2c_if;
    char        reg_val[SYS_CLK_EXT_BUF_SIZE];
    int          rc = FAILED;
    uint         reg17, reg16, reg15, reg14;
    char         chip_rev;

    /* Set Byte12 bit7 = 1, Byte4 bit0 = 0 to enable M & N programing */
    rs4420b_margin(ENABLE);

    /* set read byte count before reading */
    set_byte_count(SYS_CLK_EXT_BUF_SIZE); 

    /* Init device structure */
    rc = init_dev_struct(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
              __FUNCTION__);
        return (FAILED);
    }

    /* Dump Registers value */
    i2c_if.buf = &reg_val[0];

    /* Get Registers value */
    rc = get_regs_value(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s: Failed to get registers(rc = %#x) from 9VRS4420B.",
                      __FUNCTION__, rc);
        return (FAILED);
    }

    /* get 9VRS4420B reversion ID */
    rc = rs4420b_get_reg(RS4420B_REV_VID, &chip_rev);
    if (rc != PASSED) {
        return (FAILED);
    }

    /* if rs4420 is reversion D, convert the margin register values */
    if ((chip_rev >> RS4420B_REV_ID_BITS_OFS) == RS4420B_REV_D_ID) {
        desired_freq = mgn_regs_val_convert(desired_freq);
    } else if ((chip_rev >> RS4420B_REV_ID_BITS_OFS) != RS4420B_REV_B_ID) {
        cterr('f', 0, "%s: The RS4420B reversion(rev. ID = 0x%x) is not supported.",
                      __FUNCTION__, rc);
    }

    reg17 = (uint)((desired_freq & RS4420B_FREQ_REG17_MSK) >> RS4420B_FREQ_REG17_OFS);
    reg16 = (uint)((desired_freq & RS4420B_FREQ_REG16_MSK) >> RS4420B_FREQ_REG16_OFS);
    reg15 = (uint)((desired_freq & RS4420B_FREQ_REG15_MSK) >> RS4420B_FREQ_REG15_OFS);
    reg14 = (uint)((desired_freq & RS4420B_FREQ_REG14_MSK) >> RS4420B_FREQ_REG14_OFS);
    
    reg_val[RS4420B_EXT_BYTE23] = RS4420B_FREQ_MN_BYTE23; 
    reg_val[RS4420B_EXT_BYTE17] = reg17; 
    reg_val[RS4420B_EXT_BYTE16] = reg16; 
    reg_val[RS4420B_EXT_BYTE15] = reg15; 
    reg_val[RS4420B_EXT_BYTE14] = reg14; 

    /* Write in registers value */
    rc = i2c_dev_wr((void *)&i2c_if, SYS_CLK_EXT_BUF_SIZE);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED(rc=%#x) to write data to device 9VRS4420B.",
                      __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    /* Set Byte12 bit7 = 0, Byte4 bit0 = 1 to disable M & N margining. */
    if ((desired_freq == RS4420B_FREQ_SEL_NORM) || 
        (desired_freq == RS4420BD_FREQ_SEL_NORM)) {
        rs4420b_margin(DISABLE);
    }

    set_byte_count(SYS_CLK_BUF_SIZE_DFT); 

    return (rc);
}


/*******************************************************************************
 *
 * Function   : utah_sys_clk_i2c_scan_test
 * Description: This function to check Utah System Clock(9VRS4420B)
 *              by reading register through I2C interface.
 * Inputs     : errbuf - buffer to put error messages
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int utah_sys_clk_i2c_scan_test (char *errbuf)
{
    n2g_i2c_if_t i2c_if;
    char        reg_val[SYS_CLK_BUF_SIZE];

    /* set read byte count before reading */
    set_byte_count(SYS_CLK_BUF_SIZE); 

    /* Init device structure */
    if (init_dev_struct(&i2c_if) != PASSED) {
        sprintf(errbuf, "%s: FAILED to init SYS_CLK (9VRS4420B) I2C"
                        " interface data struct.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Get Registers value */
    i2c_if.buf = &reg_val[0];

    if (get_regs_value(&i2c_if) != PASSED) {
        sprintf(errbuf, "%s: Failed to read 9VRS4420B reg.\n", __FUNCTION__);
        return (FAILED);
    }

    set_byte_count(SYS_CLK_BUF_SIZE_DFT); 

    return (PASSED);
}


/******** History ********
*----------------------------------------------------
$Log: platform_sys_clk.c,v $
Revision 1.21  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.20  2017/07/10 03:21:55  leschen
Remove unused variable

Revision 1.19  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.18  2014/06/11 09:44:58  danchung
Remove useless code

Revision 1.17  2014/06/10 10:26:29  danchung
Disable spread spectrum after resetting the frequency margin to 0%

Revision 1.16  2014/06/05 08:53:32  danchung
Support clock chip version D

Revision 1.15  2014/02/07 09:25:31  alpeng
do not feed margin-capable clock to SATA clock

Revision 1.14  2014/01/30 02:03:42  ptong
Call set_byte_count(SYS_CLK_BUF_SIZE_DFT) to set the byte size to vendor default. Utah BIOS expect that default

Revision 1.13  2014/01/29 13:13:43  danchung
Delete size check in get_regs_value()

Revision 1.12  2014/01/29 12:39:46  danchung
Add declearation of check_size

Revision 1.11  2014/01/29 09:53:15  danchung
Fix the incorrection of displaying currect frequency margin value.

Revision 1.10  2014/01/20 09:13:15  danchung
Fix frequency margin display issue on Utah

Revision 1.9  2013/11/26 08:40:38  hroni
fix compiler warning

Revision 1.8  2013/10/30 10:59:06  danchung
Remove the delay for frequency margin

Revision 1.7  2013/10/14 12:15:34  danchung
Correct the frequency margin programming table to solve the hang issue

Revision 1.6  2013/10/08 15:30:37  danchung
Remove redundant register read of system clock when doing frequency margin.

Revision 1.5  2013/07/18 17:17:05  mcharon
add -Wal and clean up compile warnings

Revision 1.4  2013/07/03 09:57:19  alpeng
add set_byte_count before accessing byte

Revision 1.3  2013/07/03 03:34:56  alpeng
fixed the access type to smbus block

Revision 1.2  2013/06/18 03:38:30  alpeng
support margining normal

Revision 1.1  2013/06/17 11:14:50  alpeng
support chip 9VRS4420B and freq margin

Revision 1.2  2013/05/23 01:09:26  palin2
Improved error print-out of Overlord I2C device related tests.

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.10  2013/03/22 22:25:27  mcharon
change printf to cterr

Revision 1.9  2013/01/31 10:48:46  alpeng
supported CLI cmds for voltage and freq margin

Revision 1.8  2012/11/28 18:19:10  palin2
To make I2C utilities SubMenu more intuitive.

Revision 1.7  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.6  2012/09/26 18:02:15  palin2
Uniformed the print out format of I2C devices defult tests.

Revision 1.5  2012/09/19 22:30:50  palin2
Add I2C scan test support those I2C devices that are connected to Cavecreek.

Revision 1.4  2012/06/05 11:44:37  palin2
Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:33  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
