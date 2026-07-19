 /* $Id: diag_fpga_test.c,v 1.5 2020/08/06 08:03:29 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_fpga_test.c,v $
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
#include "dnv_eth_lib.h"
#include "diag_i350_test.h"

/*
 * Global variables
 */


/* Local functions */
int diag_fpga_reg_test(void);
int build_tabeil_fpga_test_menu(boolean);
int fpga_reg_test_read_fn(ulong, int, ulong *, void *);
int fpga_reg_test_write_fn(ulong, int, ulong, void *);
int tabei_fpga_utils(int);
int tabei_show_fpga_ver(int);
int fpga_reg_rd_util(int);
int fpga_reg_wr_util(int);
int fpga_reg_dump_util(int);
int fpga_reg_dump_def_util(int);
int fpga_show_reset_reason_util(int);
int cpld_reg_rd_util(int opt);
int cpld_reg_wr_util(int opt);
int diag_ser_irq_intr_util(int dummy);
int diag_fpga_i2c_dump_sfp_util(void);
int diag_fpga_i2c_read_sfp_vendor_name(int, char *);

reg_info_t_ext tabei_fpga_reg_ext = {TABEI_FPGA_REG_WIDTH,
                                     fpga_reg_test_read_fn,
                                     fpga_reg_test_write_fn,
                                     0};


/*
 * FPGA register test
 */
static reg_info_t fpga_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Access Test Register R/W", FPGA_SCRATCHPAD_REG, FPGA_RW,
     {(unsigned long)&tabei_fpga_reg_ext}, 0xFFFF, 0},
    {"END",                                      0x00,       0,
     {0},                                         0x0,        0x0},
};

/*
 * Sub Menu used for "FPGA  test -> FPGA submenu test"
 */
submenu_xtable_t tabeil_fpga_submenu_table[] = {
    {"FPGA Utility",  
     (PFT) tabei_fpga_utils,      FALSE,
     0, (type_t(*)())0,                0,
     (type_t(*)())tabei_fpga_utils,   TRUE},

    {"FPGA Register Test",
     (PFT) diag_fpga_reg_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

};

#define TABEIL_FPGA_SUBMENU_TABLE_SIZE (sizeof(tabeil_fpga_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * "motherboard test -> fpga test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tabeil_fpga_primary_items[TABEIL_FPGA_SUBMENU_TABLE_SIZE +
                                  MAX_BASE_ITEMS];
static mitem_t tabeil_fpga_secondary_items[TABEIL_FPGA_SUBMENU_TABLE_SIZE +
                                    MAX_BASE_ITEMS];

menuinfo_t tabeil_fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    tabeil_fpga_primary_items,
};

menuinfo_t *tabeil_fpga_submenup = &tabeil_fpga_subtest_menu;


/*
 * FPGA Utilities
 */
static submenu_xtable_t fpga_utils_tbl[] = {
    {"Show FPGA version",   (type_t(*)())tabei_show_fpga_ver,  0, 0,
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
    {"Aikido Program FPGA SPI PROM image", (type_t(*)())program_reggio_spi_prom, 1, 0,
     (type_t(*)())0, 0, (type_t(*)())0,   0},
#ifdef AIKIDO_DEV_KEY
    {"Program Aikido FPGA DEV keys (Development phase)", (type_t(*)())program_aikido_dev_key, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
#endif
    {"Check serial IRQ intr util", (type_t(*)())diag_ser_irq_intr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
     {"FPGA I2C read SFP", (type_t(*)())diag_fpga_i2c_read_sfp_util, 0, 0,
     (type_t(*)())is_promethium, 0,     (type_t(*)())diag_fpga_i2c_read_sfp_util, 0},
     {"FPGA I2C write SFP", (type_t(*)())diag_fpga_i2c_write_sfp_util, 0, 0,
     (type_t(*)())is_promethium, 0,     (type_t(*)())diag_fpga_i2c_write_sfp_util, 0},
};


#define FPGA_UTILS_TBL_SIZE (sizeof(fpga_utils_tbl) / sizeof(submenu_xtable_t))

/* FPGA Utils items (filled in from xtable) */
static mitem_t fpga_utils_pri_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_utils_sec_items[FPGA_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* FPGA Utils submenu */
menuinfo_t tabeil_fpga_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_utils_pri_items,
};
menuinfo_t *tabeil_fpga_utils_menup = &tabeil_fpga_utils_menu;


/*******************************************************************************
 *
 * Function   : build_tabeil_fpga_test_menu
 * Description: build fpga test submenu 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_tabeil_fpga_test_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "FPGA Test";
    testname(tname);

    build_primary_submenu(tabeil_fpga_submenu_table, TABEIL_FPGA_SUBMENU_TABLE_SIZE,
                          "FPGA test SubMenu", &tabeil_fpga_submenup);
    build_secondary_submenu(tabeil_fpga_submenu_table, TABEIL_FPGA_SUBMENU_TABLE_SIZE,
                            tabeil_fpga_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&tabeil_fpga_subtest_menu, tabeil_fpga_secondary_items, 0);
    } else {
        do_all_menu_items(tabeil_fpga_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : tabei_fpga_utils
 * Description : Function to show FPGA utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tabei_fpga_utils (int opt)
{
    build_primary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                          "FPGA Utilities", &tabeil_fpga_utils_menup);
    build_secondary_submenu(fpga_utils_tbl, FPGA_UTILS_TBL_SIZE,
                            fpga_utils_sec_items);

    menu(tabeil_fpga_utils_menup, fpga_utils_sec_items, '\0');

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
 * Function   : tabei_show_fpga_ver
 * Description: Function to show FPGA version.
 *              This is by reading FPGA Revision Reg(0x88C).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tabei_show_fpga_ver (int opt)
{
    uint reg_addr = (uint)FPGA_MASTER_REV_REG;
    uint fpga_ver = 0;

    if (fpga_read_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Master Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("Master FPGA Revision (0x84): %08X\n", fpga_ver);

    reg_addr = (uint)FPGA_REVISION_REG;
    if (fpga_read_reg(reg_addr, &fpga_ver) != PASSED) {
        printf("Failed to read FPGA Revision Reg(0x%04X).\n", reg_addr);
        return (FAILED);
    }
    printf("FPGA Revision        (0x8C): %08X\n", fpga_ver);


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

    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffff): ",
                               FPGA_MASTER_REV_REG, 0, 0xffff);

    if (fpga_read_reg(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("FPGA register(0x%04X) = 0x%08X\n", reg_offset, reg_val);
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

    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffff): ",
                               0, 0, 0xffff);
    if (fpga_read_reg(reg_offset, &orig_val) != PASSED) {
        return (FAILED);
    }
    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (fpga_write_reg(reg_offset, reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to FPGA register(0x%04X).\n",
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
 * Function: diag_fpga_i2c_read_sfp_vendor_name
 *
 * Description: Utility to read SFP ID by the I2C of FPGA
 *
 * INPUT : which_sfp - Which SFP
 *         buf       - Vendor ID data
 *
 * OUTPUT: PASSED / FAILED
 *------------------------------------------------------------------
 */
int diag_fpga_i2c_read_sfp_vendor_name (int which_sfp, char *buf)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *i2c_if_p = &i2c_if;
    char read_data[8] = {0};
    char *buf_p = read_data;
    int regnum = 0, size = 1;

    if (which_sfp == I350_PORT2) {
        if (switch_sfp_mux(SFP0) == FAILED) {
            printf("Can't switch sfp mux\n");
            return (rc);
        }
    } else {
        if (switch_sfp_mux(SFP1) == FAILED) {
            printf("Can't switch sfp mux\n");
            return (rc);
        }
    }

    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));

    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
    i2c_if_p->i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if_p->mux = I2C_MUX_ZERO;
    i2c_if_p->size = size;
    i2c_if_p->buf = buf_p;
    i2c_if_p->i2c_dev = MB_I2C_SFP_DEV;


    for (regnum = SFP_VENDOR_NAME_20; regnum <= SFP_VENDOR_NAME_35; regnum++) {

        i2c_if_p->offset = regnum;

        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                          __FUNCTION__, __LINE__, regnum, size, rc);
            return (rc);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("read EEPROM reg %d = %02x\n", regnum, (unsigned char)*read_data);
        }
        buf[regnum - SFP_VENDOR_NAME_20] = (unsigned char)*read_data;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("SFP vendor ID %s\n", buf);
    }

    return (rc);
}

/*-------------------------------------------------------------------
 *
 * Function: diag_ser_irq_intr_util 
 *
 * Test serial IRQ interrupt by writing to force interrupt register.
 * cpld.ko handles the interrupt and will keep track of number of interrupt
 * generated.
 *
 * INPUT : dummy , not used
 * OUTPUT: passed
 *------------------------------------------------------------------
 */
int diag_ser_irq_intr_util (int dummy)
{
    printf("Check serial irq interrupt\n");

    cpld_set_irq(CPLD_IRQ0);

    if (cpld_check_irq(CPLD_IRQ0) == TRUE) {
        printf("Detect IRQ0\n");
    } else {
        cterr('f', 0, "Not Detect IRQ0");
        return (FAILED);
    }

    cpld_set_irq(CPLD_IRQ6);

    if (cpld_check_irq(CPLD_IRQ6) == TRUE) {
        printf("Detect IRQ6\n");
    } else {
        cterr('f', 0, "Not Detect IRQ6");
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function: diag_fpga_i2c_dump_sfp_util 
 *
 * Description: Utility to dump SFP EEPROM by the I2C of FPGA
 *
 * INPUT : None
 * OUTPUT: PASSED / FAILED
 *------------------------------------------------------------------
 */
int diag_fpga_i2c_dump_sfp_util (void)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *i2c_if_p = &i2c_if;
    char buf[8] = {0};
    char *buf_p = buf;
    int regnum = 0, size = 1;

    if (switch_sfp_mux_util() == FAILED) {
        cterr('f', 0, "%s:%d Failed to switch SFP.", __FUNCTION__, __LINE__);
        return (rc);
    }
    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));

    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
    i2c_if_p->i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if_p->mux = I2C_MUX_ZERO;
    i2c_if_p->size = size;
    i2c_if_p->buf = buf_p;
    i2c_if_p->i2c_dev = MB_I2C_SFP_DEV;

    for (regnum = EEPROM_DATA_ADDR_0; regnum < EEPROM_DATA_ADDR_64; regnum++) {

        i2c_if_p->offset = regnum;

        rc = n2g_i2c_read(&i2c_if);
        if (rc != PASSED) {
            cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                          __FUNCTION__, __LINE__, regnum, size, rc);
            return (rc);
        }

        if (regnum%8 == 0) {
            printf("\nread EEPROM reg %.2d: ", regnum);
        }
            printf(" 0x%02x", (unsigned char)*buf);
    }

    return (rc);
}

/*-------------------------------------------------------------------
 *
 * Function: diag_fpga_i2c_read_sfp_util 
 *
 * Description: Utility to read SFP EEPROM by the I2C of FPGA
 *
 * INPUT : None
 * OUTPUT: PASSED / FAILED
 *------------------------------------------------------------------
 */
int diag_fpga_i2c_read_sfp_util (void)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *i2c_if_p = &i2c_if;
    char buf[8];
    char *buf_p = buf;
    int regnum, size = 1;

    if (switch_sfp_mux_util() == FAILED) {
        cterr('f', 0, "%s:%d Failed to switch SFP.", __FUNCTION__, __LINE__);
        return (rc);
    }
    regnum = gethex_answer("\nEnter reg: ", 0, 0, 0xFF);

    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
    i2c_if_p->i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if_p->mux = I2C_MUX_ZERO;
    i2c_if_p->offset = regnum;
    i2c_if_p->size = size;
    i2c_if_p->buf = buf_p;
    i2c_if_p->i2c_dev = MB_I2C_SFP_DEV;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, regnum, size, rc);
        return (rc);
    }
    printf ("Read %d: %#.2x", regnum,(unsigned int)* buf);
    return (rc);
}

/*-------------------------------------------------------------------
 *
 * Function: diag_fpga_i2c_write_sfp_util 
 *
 * Description: Utility to write SFP EEPROM by the I2C of FPGA
 *
 * INPUT : None
 * OUTPUT: PASSED / FAILED
 *------------------------------------------------------------------
 */
int diag_fpga_i2c_write_sfp_util (void)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t i2c_if;
    n2g_i2c_if_t *i2c_if_p = &i2c_if;
    char buf[8];
    char *buf_p = buf;
    int regnum, size = 1;

    if (switch_sfp_mux_util() == FAILED) {
        cterr('f', 0, "%s:%d Failed to switch SFP.", __FUNCTION__, __LINE__);
        return (rc);
    }
    regnum = gethex_answer("\nEnter reg: ", 0, 0, 0xFF);
    *buf = gethex_answer("\nEnter data: ", 0, 0, 0xFF);

    memset(i2c_if_p, 0, sizeof(n2g_i2c_if_t));
    i2c_if_p->i2c_bus_type = IOFPGA_I2C;
    i2c_if_p->i2c_ctrl = I2C_CTRL_FIVE;
    i2c_if_p->mux = I2C_MUX_ZERO;
    i2c_if_p->offset = regnum;
    i2c_if_p->size = size;
    i2c_if_p->buf = buf_p;
    i2c_if_p->i2c_dev = MB_I2C_SFP_DEV;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, regnum, size, rc);
        return (rc);
    }
    return (rc);
}


/******** History ******** 
 * $Log: diag_fpga_test.c,v $
 * Revision 1.5  2020/08/06 08:03:29  kehuang2
 * Collapse Promethium into main trunk
 *
 * Revision 1.4  2019/12/30 06:04:47  kehuang2
 * CSCvs55860: Enhance FPGA IRQ util
 *
 * Revision 1.3  2019/11/25 08:55:51  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:21  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.4.25  2019/09/10 08:49:08  olin2
 * Add cterr for IRQ check failure
 *
 * Revision 1.1.4.24  2019/09/05 08:50:36  olin2
 * Support FPGA serial IRQ interrupt util
 *
 * Revision 1.1.4.23  2019/09/04 09:32:51  kehuang2
 * Update FPGA dump function
 *
 * Revision 1.1.4.22  2019/08/27 07:45:06  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.21  2019/08/21 03:31:36  kehuang2
 * Update the content of POE log
 *
 * Revision 1.1.4.19  2019/07/15 09:40:02  kehuang2
 * Update Voltage Margin utility
 *
 * Revision 1.1.4.18  2019/05/29 07:00:39  olin2
 * Implement show FPGA revision
 *
 * Revision 1.1.4.17  2019/05/16 08:48:02  kehuang2
 * Clean up code by the comment of code review.
 *
 * Revision 1.1.4.16  2019/04/29 08:14:26  kehuang2
 * Clean up code
 *
 * Revision 1.1.4.15  2019/04/24 07:59:21  kehuang2
 * Update CPLD access
 *
 * Revision 1.1.4.14  2019/04/19 03:25:40  kehuang2
 * Support CPLD access
 *
 * Revision 1.1.4.13  2019/04/18 09:18:13  olin2
 * support program Aikido dev key
 *
 * Revision 1.1.4.12  2019/04/15 09:04:16  olin2
 * Support Aikido firmware upgrade
 *
 * Revision 1.1.4.11  2019/03/26 06:09:16  olin2
 * Support Dreamliner on Tabei-L
 *
 * Revision 1.1.4.10  2019/03/13 09:01:50  olin2
 * Update FPGA test menu
 *
 * Revision 1.1.4.9  2018/12/25 12:00:27  harrchan
 * Update FPGA address
 *
 * Revision 1.1.4.8  2018/12/25 07:24:38  olin2
 * Clean up code
 *
 * Revision 1.1.4.7  2018/12/05 06:50:34  olin2
 * initial commit for Aikido
 *
 * Revision 1.1.4.6  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.4.5  2018/11/06 07:17:31  kodko
 * Fix FPGA register test.
 *
 * Revision 1.1.4.4  2018/10/17 06:14:28  olin2
 * Support FPGA I2C scan
 *
 * Revision 1.1.4.3  2018/10/15 12:30:12  kodko
 * Add CPLD register read/write function.
 *
 * Revision 1.1.4.2  2018/10/02 01:49:58  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 * */
