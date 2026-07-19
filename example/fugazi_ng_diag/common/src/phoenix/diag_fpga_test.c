/* $Id: diag_fpga_test.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_fpga_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_test.c - FPGA Test
 *
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "diag_fpga_upgrade.h"
#include "plat_defs.h"
#include "diag_fpga_i2c_lib.h"
#include "dnv_gpio_lib.h"
#include "diag_common.h"
#include "diag_cpld_lib.h"
#include "tam_aikido_upgrade.h"
#include "platform_i2c.h"
#include "i2c_dev.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "diag_i2c_addr.h"
#include "diag_fpga_lib.h"
#include "nios_mbox_api.h"

/*
 * Global variables
 */


/* Local functions */
int diag_fpga_reg_test(void);
int diag_nios_test(void);
int build_phoenix_fpga_test_menu(boolean);
int fpga_reg_test_read_fn(ulong, int, ulong *, void *);
int fpga_reg_test_write_fn(ulong, int, ulong, void *);
int phoenix_fpga_utils(int);
int phoenix_show_fpga_ver(int);
int fpga_reg_rd_util(int);
int fpga_reg_wr_util(int);
int fpga_reg_dump_util(int);
int fpga_reg_dump_def_util(int);
int fpga_show_reset_reason_util(int);
int cpld_reg_rd_util(int);
int cpld_reg_wr_util(int);
int diag_ser_irq_intr_test(int);
int phoenix_aikido_reg_test(void);
int build_aikido_test_menu(int);
int build_aikido_utils(int);
void phoenix_show_nios_ver(void);
int phoenix_show_nios_mailbox_msg(int);

reg_info_t_ext phoenix_fpga_reg_ext = {PHOENIX_FPGA_REG_WIDTH,
                                     fpga_reg_test_read_fn,
                                     fpga_reg_test_write_fn,
                                     0};


/*
 * FPGA register test
 */
static reg_info_t fpga_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Access Test Register R/W", FPGA_SCRATCHPAD_REG, FPGA_RW,
     {(unsigned long)&phoenix_fpga_reg_ext}, 0xFFFF, 0},
    {"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};

/*
 * Sub Menu used for "FPGA  test -> FPGA submenu test"
 */
submenu_xtable_t phoenix_fpga_submenu_table[] = {
    {"Logic FPGA Utility",
     (PFT) phoenix_fpga_utils,      FALSE,
     0, (type_t(*)())0,                0,
     (type_t(*)())phoenix_fpga_utils,   TRUE},

    {"FPGA Register Test",
     (PFT) diag_fpga_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Check serial IRQ intr test",
     (PFT) diag_ser_irq_intr_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"NIOS Enable/Disable Test",
     (PFT) diag_nios_test, FALSE, 
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define PHOENIX_FPGA_SUBMENU_TABLE_SIZE (sizeof(phoenix_fpga_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "motherboard test -> fpga test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t phoenix_fpga_primary_items[PHOENIX_FPGA_SUBMENU_TABLE_SIZE +
                                  MAX_BASE_ITEMS];
static mitem_t phoenix_fpga_secondary_items[PHOENIX_FPGA_SUBMENU_TABLE_SIZE +
                                    MAX_BASE_ITEMS];

menuinfo_t phoenix_fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    phoenix_fpga_primary_items,
};

menuinfo_t *phoenix_fpga_submenup = &phoenix_fpga_subtest_menu;


/*
 * FPGA Utilities
 */
static submenu_xtable_t fpga_utils_tbl[] = {
    {"Show FPGA version",   (type_t(*)())phoenix_show_fpga_ver,  0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Read",  (type_t(*)())fpga_reg_rd_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Write", (type_t(*)())fpga_reg_wr_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPLD register Read",  (type_t(*)())cpld_reg_rd_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPLD register Write", (type_t(*)())cpld_reg_wr_util,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA I2C scan",       (type_t(*)())fpga_i2c_scan_addr,   0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA registers Dump", (type_t(*)())fpga_reg_dump_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Program FPGA Golden image without header", (type_t(*)())program_reggio_spi_prom_old, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Program FPGA Upgrade image with header", (type_t(*)())program_reggio_spi_prom_old, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Erase FPGA Upgrade image header", (type_t(*)())erase_header_spi_prom_image, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Set revision and date in the header of FPGA Upgrade image", (type_t(*)())set_date_revision, 1, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Show NIOS mailbox message", (type_t(*)())phoenix_show_nios_mailbox_msg, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
};

#define FPGA_UTILS_TBL_SIZE (sizeof(fpga_utils_tbl) / sizeof(submenu_xtable_t))

/* FPGA Utils items (filled in from xtable) */
static mitem_t fpga_utils_pri_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_utils_sec_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* FPGA Utils submenu */
menuinfo_t phoenix_fpga_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_utils_pri_items,
};
menuinfo_t *phoenix_fpga_utils_menup = &phoenix_fpga_utils_menu;


/*
 *  Aikido Utilities
 */
static submenu_xtable_t aikido_utils_tbl[] = {
    {"Aikido Program FPGA SPI PROM image",
     (type_t(*)())program_reggio_spi_prom, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

#ifdef AIKIDO_DEV_KEY
    {"Program Aikido FPGA DEV keys (Development phase)",
     (type_t(*)())program_aikido_dev_key, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
#endif

    {"AIKIDO SPI read",
     (type_t(*)())aikido_spi_read_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"AIKIDO SPI write",
     (type_t(*)())aikido_spi_write_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"toggle flag : aikido_mailbox_flag",
     (type_t(*)())aikido_flag_mailbox, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"toggle flag: aikido_act2_flag",
     (type_t(*)())aikido_flag_act2, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define AIKIDO_UTILS_TBL_SIZE (sizeof(aikido_utils_tbl) / sizeof(submenu_xtable_t))

/* AIKIDO Utils items (filled in from xtable) */
static mitem_t aikido_utils_pri_items[AIKIDO_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t aikido_utils_sec_items[AIKIDO_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* AIKIDO Utils submenu */
menuinfo_t phoenix_aikido_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    aikido_utils_pri_items,
};

menuinfo_t *phoenix_aikido_utils_menup = &phoenix_aikido_utils_menu;

/*
 * Test Menu used for "Aikido test"
 */
submenu_xtable_t aikido_test_tbl[] = {
    {"Aikido Utilities",
     (PFT) build_aikido_utils,      FALSE,
     0, (type_t(*)())0,                0,
     (type_t(*)())0,   0},

    {"Aikido Register Test",
     (PFT) phoenix_aikido_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define AIKIDO_TEST_TBL_SIZE (sizeof(aikido_test_tbl) / sizeof(submenu_xtable_t))

static mitem_t aikido_test_pri_items[AIKIDO_TEST_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t aikido_test_sec_items[AIKIDO_TEST_TBL_SIZE + MAX_BASE_ITEMS];

/* AIKIDO test submenu */
menuinfo_t phoenix_aikido_test_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    aikido_test_pri_items,
};

menuinfo_t *phoenix_aikido_test_menup = &phoenix_aikido_test_menu;


/*******************************************************************************
 *
 * Function   : build_phoenix_fpga_test_menu
 * Description: build fpga test submenu 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_phoenix_fpga_test_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "Logic FPGA Test";
    testname(tname);

    build_primary_submenu(phoenix_fpga_submenu_table, PHOENIX_FPGA_SUBMENU_TABLE_SIZE,
                          "Logic FPGA test SubMenu", &phoenix_fpga_submenup);
    build_secondary_submenu(phoenix_fpga_submenu_table, PHOENIX_FPGA_SUBMENU_TABLE_SIZE,
                            phoenix_fpga_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&phoenix_fpga_subtest_menu, phoenix_fpga_secondary_items, 0);
    } else {
        do_all_menu_items(phoenix_fpga_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : phoenix_fpga_utils
 * Description : Function to show FPGA utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int phoenix_fpga_utils (int opt)
{
    build_primary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                          "Logic FPGA Utilities", &phoenix_fpga_utils_menup);
    build_secondary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                            fpga_utils_sec_items);

    menu(phoenix_fpga_utils_menup, fpga_utils_sec_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : build_aikido_utils
 * Description : Function to show AIKIDO utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int build_aikido_utils (int opt)
{
    build_primary_submenu(aikido_utils_tbl, AIKIDO_UTILS_TBL_SIZE,
                          "Aikido Utilities", &phoenix_aikido_utils_menup);
    build_secondary_submenu(aikido_utils_tbl, AIKIDO_UTILS_TBL_SIZE,
                            aikido_utils_sec_items);

    menu(phoenix_aikido_utils_menup, aikido_utils_sec_items, '\0');

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : build_aikido_test_menu
 * Description : Function to show AIKIDO test submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int build_aikido_test_menu (int opt)
{
    build_primary_submenu(aikido_test_tbl, AIKIDO_TEST_TBL_SIZE,
                          "Aikido Test", &phoenix_aikido_test_menup);
    build_secondary_submenu(aikido_test_tbl, AIKIDO_TEST_TBL_SIZE,
                            aikido_test_sec_items);

    if (opt) {
        /* Entered with submenu */
        menu(phoenix_aikido_test_menup, aikido_test_sec_items, '\0');
    } else {
        do_all_menu_items(phoenix_aikido_test_menup);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_fpga_reg_test
 *
 * Description: Function to test FPGA register R/W
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_fpga_reg_test (void)
{
    char *tname ="FPGA Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (register_tests(0, fpga_reg_test_tbl) != PASSED) {
        cterr('f', 0, "FPGA Register test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : fpga_reg_test_read_fn
 * Description: FPGA register read function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              *buf   - pointer to read buffer
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int fpga_reg_test_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    if (fpga_read_reg((uint)addr, (uint *)buf) != PASSED) {
        printf("%s: Failed to read FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : fpga_reg_test_write_fn
 * Description: FPGA register write function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              data   - write in data
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int fpga_reg_test_write_fn (ulong addr, int size, ulong data, void *param)
{   
    if (fpga_write_reg((uint)addr, (uint)data) != PASSED) {
        printf("%s: Failed to write FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : phoenix_show_fpga_ver
 * Description: Function to show FPGA version.
 *              This is by reading FPGA Revision Reg(0x88C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int phoenix_show_fpga_ver (int opt)
{
    uint reg_addr = (uint)FPGA_MASTER_REV_REG;
    uint fpga_ver = 0;

    if (fpga_read_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Master Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("Master FPGA Revision (0x84): %08X\n", fpga_ver);
    printf("Board Type:%01X, Board Revision:%01X, Master Major:%02X, Master Minor:%02X\n", 
            FPGA_MAS_BOARD_TYPE_MASK(fpga_ver), FPGA_MAS_BOARD_REV_MASK(fpga_ver),
            FPGA_MAS_MAJOR_REV_MASK(fpga_ver), FPGA_MAS_MINOR_REV_MASK(fpga_ver));

    reg_addr = (uint)FPGA_REVISION_REG;
    if (fpga_read_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("FPGA Revision        (0x8C): %08X\n", fpga_ver);
    printf("Year:%02X, Month:%02X, Day:%02X, Major:%01X, Minor:%01X\n", 
            FPGA_REV_YEAR_MASK(fpga_ver), FPGA_REV_MON_MASK(fpga_ver), 
            FPGA_REV_DAY_MASK(fpga_ver), FPGA_REV_MAJOR_MASK(fpga_ver), 
            FPGA_REV_MINOR_MASK(fpga_ver));

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_reg_rd_util
 * Description : Utility to read FPGA register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_rd_util (int opt)
{
    uint32_t reg_offset = 0, reg_val = 0;

    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffffff): ",
                               FPGA_MASTER_REV_REG, 0, 0xffffff);

    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%06X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("FPGA register(0x%06X) = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}



/*******************************************************************************
 *
 * Function    : fpga_reg_wr_util
 * Description : Utility to write FPGA register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;

    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffffff): ",
                               0, 0, 0xffffff);
    if (fpga_read_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }
    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%06X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : cpld_reg_rd_util
 * Description : Utility to read CPLD register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int cpld_reg_rd_util (int opt)
{
    uint32_t reg_offset = 0, reg_val = 0;

    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffff): ",
                               0, 0, 0xffff);

    if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read CPLD register 0x%04X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("CPLD register(0x%04X) = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : cpld_reg_wr_util
 * Description : Utility to write CPLD register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int cpld_reg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;

    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffff): ",
                               0, 0, 0xffff);
    if (cpld_read_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }
    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to CPLD register(0x%04X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_reg_dump_util
 * Description : Utility to dump FPGA all registers.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_reg_dump_util (int opt)
{   
    uint reg_val = 0;
    int ctr = 0;
    int reg_offset, reg_size = 0;

    reg_offset = gethex_answer("Enter register offset (0x0 ~ 0xffffff): ",
                                0, 0, 0xffffff);
    reg_size = gethex_answer("Enter register size (0x0 ~ 0xffffff): ",
                              0x100, 0, 0xffffff);
    /* Read function read 4 bytes at a time, 
     * move for every 4 bytes to reduce redundent information.
     */
    for (ctr = reg_offset; ctr < reg_offset + reg_size; ctr += 4) {
        reg_val = 0;
        if (fpga_read_reg(ctr, &reg_val) != PASSED) {
            printf("Failed to read FPGA Reg(0x%04X).\n",ctr);
            return (FAILED);
        } else {
            printf("FPGA Reg(0x%04X): 0x%08X\n",
                   ctr, reg_val);
        }
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_vol_margin
 * Description : Utility to do voltage margin
 * Inputs      : Which setting
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_vol_margin (uint8_t v_status)
{   
    uint reg_val = 0;
    switch (v_status) {
        case NO_MARGIN:
            reg_val = MARGIN_CONTROL_BASE | NO_MARGIN;
            break;
        case MARGIN_LOW:
            reg_val = MARGIN_CONTROL_BASE | MARGIN_LOW;
            break;
        case MARGIN_HIGH:
            reg_val = MARGIN_CONTROL_BASE | MARGIN_HIGH;
            break;
        default:
            printf("Wrong margin mode!");
            return (FAILED);
            break;
    }
    if (fpga_write_reg(VOLTAGE_MARGIN_REG, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg(0x%x).\n",
               __FUNCTION__, VOLTAGE_MARGIN_REG);
        return (FAILED);
    }
    printf("Done\n");
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_poe_detect_util
 * Description : Utility to detect poe 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
extern int fpga_poe_detect_util(void);
 *******************************************************************************
 */
int fpga_poe_detect_util (void)
{
    testname("FPGA POE Detect");
    prpass(testpass, "POE Detection Test\n");
    if (fpga_poe_detect() == FAILED) {
        cterr('f', 0, "\nPOE -54V is not detected.");
        return (FAILED);
    }
    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : fpga_poe_detect
 * Description : Detect the poe is present or not 
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int fpga_poe_detect (void)
{
    uint reg_val = 0;
    if (fpga_read_reg(FPGA_POE_STATE_REG, &reg_val) == FAILED) {
        return (FAILED);
    }   
    reg_val &= POE_PRESENT_MASK;
    if (reg_val == POE_POWER_SUPPLY_PRESENT) {
        printf("POE -54V: detected\n");
    } else {
        printf("POE -54V: not detected\n");
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: diag_ser_irq_intr_test
 *
 * Test serial IRQ interrupt by writing to force interrupt register.
 * cpld.ko handles the interrupt and will keep track of number of interrupt
 * generated.
 *
 * INPUT : dummy , not used
 * OUTPUT: passed
 *------------------------------------------------------------------
 */
int diag_ser_irq_intr_test (int dummy)
{
    printf("Check serial irq interrupt\n");

    cpld_set_irq(CPLD_IRQ0);

    if (cpld_check_irq(CPLD_IRQ0) == TRUE) {
        printf("Detect IRQ0\n");
    } else {
        cterr('f', 0, "Not Detect IRQ0");
    }

    cpld_set_irq(CPLD_IRQ6);

    if (cpld_check_irq(CPLD_IRQ6) == TRUE) {
        printf("Detect IRQ6\n");
    } else {
        cterr('f', 0, "Not Detect IRQ6");
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_nios_test
 *
 * Description : Function to test nios
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_nios_test (void)
{
    char *tname ="NIOS Enable/Disable";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* set NIOS to normal mode, and confirm nios status is running */
    if (set_nios_mode(NIOS_NORMAL_MODE) != PASSED) {
        cterr('f', 0, "set NIOS normal mode failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    msleep(500);

    /* reverty to disable mode in order not to affect other diag tests */
    if (set_nios_mode(NIOS_DISABLE_MODE) != PASSED) {
        cterr('f', 0, "Failed to set NIOS mode back to disabled");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);

    return (PASSED);
}


/******************************************************************************
 *
 * Function: phoenix_aikido_reg_test
 *
 * Description : Function to test aikido register for Phoenix.
 *               pattern size must be 4 bytes.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int phoenix_aikido_reg_test(void)
{
    unsigned int addr = PHOENIX_AIKIDO_SCR_REG;
    unsigned char ptn[4] = { 0x55, 0xAA, 0x00, 0xFF };
    unsigned char ptn2[4] = { 0xAA, 0x55, 0xFF, 0x00 };
    int ret;

    testname("Aikido SPI Register");

    prpass(testpass, "");

    printf("\nPattern #1\n");
    ret = aikido_reg_test(addr, ptn);
    if (ret != PASSED) {
        cterr('f', 0, "Failed to read/write Aikido register.");
        goto _test_end;
    }

    printf("Pattern #2\n");
    ret = aikido_reg_test(addr, ptn2);
    if (ret != PASSED) {
        cterr('f', 0, "Failed to read/write Aikido register.");
    }

_test_end:
    prcomplete(testpass, errcount, (char *)0);

    return (ret);
}


/*******************************************************************************
 *
 * Function   : phoenix_show_nios_ver
 * Description: Function to show NIOS version.
 *              This is to reading LPC NIOS Version Reg(0x90).
 * Inputs     : None.
 * Outputs    : None.
 *
 *******************************************************************************
 */
void phoenix_show_nios_ver(void)
{
    uint32_t reg = LPC_NIOS_VER_REG, val = 0;

    if (PASSED != cpld_read_reg(reg, &val)) {
        printf("Failed to read CPLD register 0x%04X.\n", reg);
        return;
    }

    printf("NIOS Version: %X\n", val);
}


/*******************************************************************************
 *
 * Function   : phoenix_show_nios_mailbox_msg
 * Description: Function to show NIOS mailbox message.
 *              Not all messages can be shown.
 * Inputs     : opt -- not used.
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int phoenix_show_nios_mailbox_msg(int opt)
{
    unsigned int en_nios = 0;
    unsigned int offset = 0;

    en_nios = gethex_answer("\n0: Disable NIOS\n1: Enable NIOS)"
                            "\nEnter (0 ~ 1): ", 0, 0, 1);

    if (en_nios != 0) {
        if (set_nios_mode(NIOS_NORMAL_MODE) != PASSED) {
            printf("Failed to set NIOS to normal mode\n");
        }
        msleep(500);
    }

    while (1) {
        /* Show all available offsets */
        nios_mbox_show_all_offset();

        /* Select offset address to show */
        offset = gethex_answer("\nDump All: 0xDDDD"
                               "\nExit: 0xEEEE"
                               "\nSelect mailbox offset(0x0 ~ 0xE80): ",
                               0, 0, 0xEEEE);

        if (offset == 0xEEEE) {
            break;
        } else if (offset == 0xDDDD) {
            nios_mbox_show_message_all();
        } else {
            nios_mbox_show_message(offset);
        }
    }

    if (en_nios != 0) {
        if (set_nios_mode(NIOS_DISABLE_MODE) != PASSED) {
            printf("Failed to disable NIOS\n");
        }
        msleep(500);
    }

    return (PASSED);
}
