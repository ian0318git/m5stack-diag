/* $Id: platform_sys_clk.c,v 1.4 2017/07/10 02:51:58 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_sys_clk.c,v $
 *------------------------------------------------------------------------------
 * platform_sys_clk.c - Overlord SYS CLK chip main function/menu.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
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


/******************************************************************************
 *                              Function proto
 ******************************************************************************/
static int reg_test(void); 
static int show_reg(uint32);
static int alter_clock(uint);
static int init_dev_struct(n2g_i2c_if_t *);
static int get_regs_value(n2g_i2c_if_t *);

static void build_toggle_sys_clk_util_menu(void);

extern int reset_sys_by_watchdog(void);
extern int do_all_menu_items(struct menuinfo *);

/******************************************************************************
 *                                  Globals 
 ******************************************************************************/
/* Registers test table */
static reg_info_t sq420d_reg_test_table[] = {
    {"Byte 1: Output Enable", SQ420D_OUT_EN1,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),
           {0}, 0xFF, 0xFF},
    {"Byte 8: Byte Count", SQ420D_BYTE_CNT,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
           {0}, 0xFF, 0x0A},
    {0, 0, 0, {0}, 0, 0},
};

/* Bit define for the pin usage for the cpu clocks */
static sq420d_bit_t sys_clk_bit_d[] = {
    {"Cave Creek SATA_CLK100",    0, 0, SQ420D_OUT_EN0, SQ420D_SRC0_EN},
    {"Cave Creek PCIe_EP_CLK100", 0, 0, SQ420D_OUT_EN0, SQ420D_SRC1_EN},
    {"Cave Creek CRU_CLK100",     0, 0, SQ420D_OUT_EN0, SQ420D_SRC2_EN},
    {"Gladden BCLK",              0, 0, SQ420D_OUT_EN1, SQ420D_CPU0_EN},
    {"XDP REF_CLK",               0, 0, SQ420D_OUT_EN1, SQ420D_CPU1_EN},
    {"DB1200 DIF_IN",             0, 0, SQ420D_OUT_EN1, SQ420D_CPU2_EN},
    {"Cave Creek DMI_CLK100",     0, 0, SQ420D_OUT_EN1, SQ420D_CPU3_EN},
    {"Cave Creek USB_CLK96",      0, 0, SQ420D_OUT_EN0, SQ420D_DOT96_EN},
    {"Cave Creek PCICLK",         0, 0, SQ420D_OUT_EN2, SQ420D_PCI0_EN},
    {"FPGA",                      0, 0, SQ420D_OUT_EN2, SQ420D_PCI1_EN},
    {"Debug",                     0, 0, SQ420D_OUT_EN2, SQ420D_PCI2_EN},
    {"Cave Creek UART clock",     0, 0, SQ420D_OUT_EN2, SQ420D_48M_EN},
    {"Cave Creek REF_CLK14",      0, 0, SQ420D_OUT_EN1, SQ420D_REF14_EN},
    {0, 0, 0, 0, 0},
};

/* Submenu alter_clock pin conversion table */
static sys_clk_conv_t cpu_clkbuf_conv_table[] = {
    {CLKBUF_ENT_CHAR, OVLD_CAVE_SATA_CLK100, SQ420D_OUT_EN0, SQ420D_SRC0_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_PCIE_CLK100, SQ420D_OUT_EN1, SQ420D_SRC1_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_CRU_CLK100,  SQ420D_OUT_EN1, SQ420D_SRC2_EN},
    {CLKBUF_ENT_CHAR, OVLD_GLADDEN_BCLK,     SQ420D_OUT_EN2, SQ420D_CPU0_EN},
    {CLKBUF_ENT_CHAR, OVLD_XDP_REF_CLK,      SQ420D_OUT_EN2, SQ420D_CPU1_EN},
    {CLKBUF_ENT_CHAR, OVLD_DB1200_DIF_IN,    SQ420D_OUT_EN2, SQ420D_CPU2_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_DMI_CLK100,  SQ420D_OUT_EN2, SQ420D_CPU3_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_USB_CLK96,   SQ420D_OUT_EN0, SQ420D_DOT96_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_PCICLK,      SQ420D_OUT_EN2, SQ420D_PCI0_EN},
    {CLKBUF_ENT_CHAR, OVLD_FPGA,             SQ420D_OUT_EN2, SQ420D_PCI1_EN},
    {CLKBUF_ENT_CHAR, OVLD_DEBUG,            SQ420D_OUT_EN2, SQ420D_PCI2_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_UART_CLK,    SQ420D_OUT_EN2, SQ420D_48M_EN},
    {CLKBUF_ENT_CHAR, OVLD_CAVE_CLK14,       SQ420D_OUT_EN1, SQ420D_REF14_EN},
    {CLKBUF_ENT_END, 0, 0, 0},
};


/* Register test table */
static reg_info_t sq420d_default_reg_tbl[] = {
    {"Byte 0: Output Enable",             0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0xFF},
    {"Byte 1: Output Enable",             0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x9E, 0x9E},
    {"Byte 2: Output Enable",             0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x3F, 0x3F},
    {"Byte 3: Reserved",                  0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0x00, 0x00},
    {"Byte 4: Reserved",                  0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0x00, 0x00},
    {"Byte 5: NS_SAS_SRC Freq. select",   0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x0F, 0x0F},
    {"Byte 6: CPU/SRC/PCI Freq. select",  0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0x18, 0x18},
    {"Byte 7: Vendor & Rev ID",           0,
     (READ_ONLY | READ_ONLY | REG_ACCESS),     {(uint)REG_EXT}, 0x31, 0x31},
    {"Byte 8: Byte Count",                0,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)REG_EXT}, 0xFF, 0x0A},
    {"Byte 9: Device ID",                 0,
     (READ_ONLY | SAVE_RESTORE | REG_ACCESS),  {(uint)REG_EXT}, 0xFF, 0x17},
    {0, 0, 0, {0}, 0, 0},
};


static n2g_i2c_if_t sys_clk_if[] = {
    {
        .offset = 0,
        .i2c_bus_type = CPU_I2C1,
        .i2c_dev = MB_I2C_ADDR_SYS_CLK,
        .size = sizeof(uint8_t),
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
/*
 * System CLK Main menu
 */
static submenu_xtable_t sys_clk_menu_table[] = {
    {"Toggle CLKs utility",      (PFT)build_toggle_sys_clk_util_menu, TRUE,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
    {"Show SYS CLK registers",   (PFT)show_reg,                0,
     0,                          (type_t(*)())0, 0, (PFT)0,    0},
    {"SYS CLK registers test",   (PFT)reg_test,                0, 
     (MF_CONTINUOUS | MF_DOALL), (type_t(*)())0, 0, (PFT)0,    0},
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
static submenu_xtable_t toggle_clks_util_menu_table[] = {
    {"Toggle SATA_CLK100",            (PFT)alter_clock,  OVLD_CAVE_SATA_CLK100,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle PCIe_EP_CLK100",         (PFT)alter_clock,  OVLD_CAVE_PCIE_CLK100,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle CRU_CLK100",             (PFT)alter_clock,  OVLD_CAVE_CRU_CLK100,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Gladden BCLK",           (PFT)alter_clock,  OVLD_GLADDEN_BCLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle XDP REF_CLK",            (PFT)alter_clock,  OVLD_XDP_REF_CLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle DB1200 DIF_IN",          (PFT)alter_clock,  OVLD_DB1200_DIF_IN,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Cave Creek DMI_CLK100",  (PFT)alter_clock,  OVLD_CAVE_DMI_CLK100,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Cave Creek USB_CLK96",   (PFT)alter_clock,  OVLD_CAVE_USB_CLK96,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Cave Creek PCICLK",      (PFT)alter_clock,  OVLD_CAVE_PCICLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle FPGA",                   (PFT)alter_clock,  OVLD_FPGA,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Debug",                  (PFT)alter_clock,  OVLD_DEBUG,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Cave Creek UART clock",  (PFT)alter_clock,  OVLD_CAVE_UART_CLK,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
    {"Toggle Cave Creek REF_CLK14",   (PFT)alter_clock,  OVLD_CAVE_CLK14,
     0,                               (type_t(*)())0,    0,
     (PFT)0,                          0},
};

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
 * Function   : get_regs_value
 * Description: Get all SYS CLK registers value
 * Inputs     : *i2c_if - pointer of i2c interface data structure
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_regs_value (n2g_i2c_if_t *i2c_if)
{
    int        rc = FAILED, result = -1;

    /* Get Registers value */
    result = i2c_dev_rd((void *)i2c_if);
    if (result < 0) {
        cterr('f', 0, "cpu i2c:FAILED to read registers from sys clock" );
        return (rc);
    }

    if (result != SYS_CLK_BUF_SIZE) {
        cterr('f', 0, "cpu i2c: expect %d bytes from 932SQ420D  but received %d",
               SYS_CLK_BUF_SIZE, result);
    } else {
        rc = PASSED;
    }

    return (rc);
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
        cterr('f', 0, "%s: Failed to get SYS_CLK 932SQ420D registers"
                      " value (rc = %#x).", __FUNCTION__, rc);
        return (rc);
    }

    /* Dump Registers value */
    reg_tmp = (char *)i2c_if->buf;
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

    return (rc);
}


/*******************************************************************************
 *
 * Function   : alter_clock
 * Description: Update one bit in CLKBUF registers
 * Inputs     : field_type - Bit to be changed.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int alter_clock (uint field_type)
{
    int rc = FAILED, index_ctr = 0;
    n2g_i2c_if_t i2c_if;
    char answer, *true, *false;
    sys_clk_conv_t *index_p;
    char reg_val[SYS_CLK_BUF_SIZE];

    index_p = &cpu_clkbuf_conv_table[0];

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
        printf("Original %s is %s.\n", sys_clk_bit_d[index_ctr].name,
               (reg_val[index_p->offset] & index_p->mask) ? true : false);
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
        assert(!"clkbuf alter_clock entry_type");
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
        cterr('f', 0, "%s:%d Failed to get SYS_CLK 932SQ420D registers"
                      " value (rc = %#x).", __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    printf("\nNow %s is %s.\n", sys_clk_bit_d[index_ctr].name,
           (reg_val[index_p->offset] & index_p->mask) ? true : false);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : sq420d_reg_test
 * Description: This function tests 932SQ420D registers.
 *              Also check the ID of the chip.
 * Inputs     : i2c_if_p - Pointer to the i2c interface
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int sq420d_reg_test (n2g_i2c_if_t *i2c_if_p)
{
    char  reg_val[SYS_CLK_BUF_SIZE];
    uint32 rc = FAILED;

    /* Assign buffer */
    i2c_if_p->buf = &reg_val[0];

    /* Get Registers value */
    rc = get_regs_value(i2c_if_p);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to read(rc = %#x) from device 932SQ420D", rc);
        return (FAILED);
    }

    if ((reg_val[SQ420D_DEV_ID] & SQ420D_DEV_ID_MASK) != SQ420D_DEV_ID_DFT) {
        /* ID does not match to the chip type */
        cterr('f', 0, "%s:%d Wrong ID 0x%.2x(Expect = 0x%.2x)",
               __FUNCTION__, __LINE__, reg_val[SQ420D_DEV_ID],
               SQ420D_DEV_ID_DFT);
        return (FAILED);
    }

    /* Do Register test */
    rc = register_tests(0, &sq420d_reg_test_table[0]);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : reg_test
 * Description:	Test CLKBUF registers
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int reg_test (void)
{
    n2g_i2c_if_t i2c_if;

    testname("System clock Register");

    /* Init device structure */
    if (init_dev_struct(&i2c_if) != PASSED) {
        cterr('f', 0, "%s: FAILED to init SYS_CLK I2C interface data struct.",
              __FUNCTION__);
        return (FAILED);
    }

    /* Test the registers */
    prpass(testpass, (char *)NULL);

    if (sq420d_reg_test(&i2c_if) != PASSED) {
        /* Registers test failed */
        cterr('f', 0, "932SQ420D register test Failed.");
        return (FAILED);
    }

    if (!((NVRAM)->diagflag & D_CONTINUOUS)) {
        printf("passed.\n");
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : sq420d_get_reg
 * Description: This function to get the desired register value from 932SQ420D.
 * Inputs     : reg_offset - Offset of desired register
 *              val_buf - Pointer of buffer to store the desired data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sq420d_get_reg (int reg_offset, char *val_buf)
{
    n2g_i2c_if_t i2c_if;
    int          rc = FAILED;
    char        reg_val[SYS_CLK_BUF_SIZE];

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
        cterr('f', 0, "%s: Failed to read registers(rc = %#x) from 932SQ420D.",
                      __FUNCTION__, rc);
        return (FAILED);
    }

    *val_buf = reg_val[reg_offset];
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d val_bug = %#x.\n", __FUNCTION__, __LINE__, *val_buf);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : sq420d_set_freq
 * Description: This function to set the desired frequency to 932SQ420D.
 * Inputs     : desired_freq - the desired frequency value
 *              spread - only used for CLI mode 
 *              mode - for menu mode or CLI mode 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sq420d_set_freq (char desired_freq, boolean spread, boolean mode)
{
    n2g_i2c_if_t i2c_if;
    char        reg_val[SYS_CLK_BUF_SIZE];
    int          ctr = 0, rc = FAILED;

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
        cterr('f', 0, "%s: Failed to get registers(rc = %#x) from 932SQ420D.",
                      __FUNCTION__, rc);
        return (FAILED);
    }

    if (mode != CLI_MODE) { /* MENU_MODE */
        if (getc_answer("Want to Enable Spread? (y/n)", "yn", 'n') == 'y') {
            reg_val[SQ420D_OUT_EN1] |= SQ420D_SS_EN;      /* Enable Spread */
        } else {
            reg_val[SQ420D_OUT_EN1] &= (~SQ420D_SS_EN);   /* Clear old value */
        }
    } else { /* CLI mode */
        if (spread) {
            reg_val[SQ420D_OUT_EN1] |= SQ420D_SS_EN;      /* Enable Spread */
        } else {
            reg_val[SQ420D_OUT_EN1] &= (~SQ420D_SS_EN);   /* Clear old value */
        }
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val[%d] = %#x.\n",
               __FUNCTION__, __LINE__,
               SQ420D_OUT_EN1, reg_val[SQ420D_OUT_EN1]);
    }

    reset_sys_by_watchdog();

    reg_val[SQ420D_CPU_FQ_SEL] &= (~SQ420D_CPU_FS_MASK);  /* Clear old value */
    reg_val[SQ420D_CPU_FQ_SEL] |= desired_freq;           /* Set new value */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d reg_val[%d] = %#x.\n",
               __FUNCTION__, __LINE__,
               SQ420D_CPU_FQ_SEL, reg_val[SQ420D_CPU_FQ_SEL]);
    }

    /* Write in registers value */
    rc = i2c_dev_wr((void *)&i2c_if, SYS_CLK_BUF_SIZE);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d FAILED(rc=%#x) to write data to device 932SQ420D.",
                      __FUNCTION__, __LINE__, rc);
        return (rc);
    }

    for (ctr = 0; ctr < 100; ctr++) {
        sleep(1);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function   : ovld_sys_clk_i2c_scan_test
 * Description: This function to check Overlord System Clock(932SQ420D)
 *              by reading register through I2C interface.
 * Inputs     : errbuf - buffer to put error messages
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int ovld_sys_clk_i2c_scan_test (char *errbuf)
{
    n2g_i2c_if_t i2c_if;
    char        reg_val[SYS_CLK_BUF_SIZE];

    /* Init device structure */
    if (init_dev_struct(&i2c_if) != PASSED) {
        sprintf(errbuf, "%s: FAILED to init SYS_CLK (932SQ420D) I2C"
                        " interface data struct.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Get Registers value */
    i2c_if.buf = &reg_val[0];

    if (get_regs_value(&i2c_if) != PASSED) {
        sprintf(errbuf, "%s: Failed to read 932SQ420D reg.\n", __FUNCTION__);
        return (FAILED);
    }

    return (PASSED);
}


/******** History ********
*----------------------------------------------------
$Log: platform_sys_clk.c,v $
Revision 1.4  2017/07/10 02:51:58  leschen
Remove unused variable

Revision 1.3  2013/11/26 08:40:37  hroni
fix compiler warning

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
