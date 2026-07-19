/* $Id: diag_fpga_test.c,v 1.3 2015/02/14 12:48:41 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_fpga_test.c,v $ 
 *-----------------------------------------------------------------------------
 * diag_fpga_test.c - Woodlawn FPGA Test Menu
 *
 * February 2012, Times Huang
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include "diag_fpga_lib.h"
#include "diag_fpga_fw.h"
#include "diag_fpga_test.h"
#include <stdio.h>
#include "i2c_api.h"
#include "platform_i2c.h"
#include "platform_sfp_cookie.h"

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define FPGA_RONLY    (READ_ONLY | REG_ACCESS)
#define FPGA_RW       (READ_WRITE | REG_ACCESS)

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

int fpga_test(int);
int fpga_do_all_wrapper(void);
extern uint32_t get_gpio_rx_dat_bits(uint32_t);
extern void msleep(unsigned long);

static int fpga_register_test(void);
static int fpga_force_interrupt_test(void);
static int program_fpga(void);
static int display_fpga_version(void);
static int set_fpga_boot_flag(void);
static int fpga_utility(int);
static int diag_fpga_read_fn(ulong, int, ulong *, void *);
static int diag_fpga_write_fn(ulong, int, ulong, void *);
int display_fpga_regs(void);
int alter_fpga_regs(void);
static int display_fpga_flash_content(void);
static int erase_fpga_flash_sector(void);

static int diag_fpga_local_bus_read_fn(ulong, int, ulong *, void *);
static int diag_fpga_local_bus_write_fn(ulong, int, ulong, void *);
int fpga_local_bus_register_test(void);
int fpga_burst_mode_register_test(void);
int fpga_local_bus_read(void);
int fpga_local_bus_write(void);
int config_fpga_clk_mux_sel(uint8_t);
int fpga_read_burst_mode(void);
int fpga_write_burst_mode(void);

/* Sub Menu used for FPGA tests.*/
static submenu_xtable_t fpga_tests_submenu_table[] = {
   {"FPGA Utility", (type_t(*)())fpga_utility,   FALSE,
       0, NULL, 0, (type_t(*)())fpga_utility,   TRUE}, 
   {"FPGA I2C Register Test", (type_t(*)()) fpga_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"FPGA Local Bus Register Test", (type_t(*)()) fpga_local_bus_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"FPGA Force Interrupt Test", (type_t(*)()) fpga_force_interrupt_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"FPGA I2C Burst Mode Register Test", (type_t(*)()) fpga_burst_mode_register_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
   {"FPGA SFP I2C Read Test", (type_t(*)()) fpga_sfp_i2c_read_test,   0,
       MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define FPGA_TESTS_SUBMENU_TABLE_SIZE (sizeof(fpga_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fpga_tests_primary_items[FPGA_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];
static mitem_t fpga_tests_secondary_items[FPGA_TESTS_SUBMENU_TABLE_SIZE +
                                       MAX_BASE_ITEMS];

menuinfo_t fpga_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fpga_tests_primary_items,
};
menuinfo_t *fpga_submenup = &fpga_subtest_menu;


/* List of FPGA Utilities */
static submenu_xtable_t fpga_util_items[] = {
    {"Program FPGA", (type_t(*)())program_fpga, 0, 0, (type_t(*)())0,
     0, (type_t(*)())0, 0},
    {"Display FPGA Version", (type_t(*)())display_fpga_version, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display FPGA Register", (type_t(*)())display_fpga_regs, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Alter FPGA Register", (type_t(*)())alter_fpga_regs, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Set FPGA Boot Flag (Golden/Upgrade)", (type_t(*)())set_fpga_boot_flag, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Display FPGA Flash content", (type_t(*)())display_fpga_flash_content, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Erase FPGA Flash", (type_t(*)())erase_fpga_flash_sector, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Read Burst Mode", (type_t(*)())fpga_read_burst_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Write Burst Mode", (type_t(*)())fpga_write_burst_mode, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA local bus read", (type_t(*)())fpga_local_bus_read, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA local bus write", (type_t(*)())fpga_local_bus_write, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Clock Mux Select GE0", (type_t(*)())config_fpga_clk_mux_sel,
     FPGA_CLK_MUX_GE0, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA Clock Mux Select GE1", (type_t(*)())config_fpga_clk_mux_sel, 
     FPGA_CLK_MUX_GE1, 0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA SYNC_CLK_Out Verification", (type_t(*)())verify_fpga_sync_clk_out, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"FPGA SYNC_TRIG_Out Verification", (type_t(*)())verify_fpga_sync_trig_out, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define FPGA_TESTS_UTIL_SIZE (sizeof(fpga_util_items) / \
                                     sizeof(submenu_xtable_t))

/*
 * fpga util items (filled in from xtable)
 */
static mitem_t fpga_tests_primary_util_items[FPGA_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];
static mitem_t fpga_tests_secondary_util_items[FPGA_TESTS_UTIL_SIZE +
                                     MAX_BASE_ITEMS];

/*
 * FPGA Utils submenu
 */
menuinfo_t fpga_util_menu = {
    "FPGA Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    fpga_tests_primary_util_items,
};

menuinfo_t *fpga_util_menup = &fpga_util_menu;


static reg_info_t_ext fpga_reg_ext = {1, diag_fpga_read_fn,
                                      diag_fpga_write_fn, 0};

static reg_info_t_ext fpga_local_bus_reg_ext = {1, diag_fpga_local_bus_read_fn,
                                      diag_fpga_local_bus_write_fn, 0};

static reg_info_t fpga_test_regs[] = {
    /* NAME                    OFFSET TYPE          SIZE  MASK  RESET VAL*/
    {"Scratchpad Register",       FPGA_SCRATCHPAD_REG, FPGA_RW,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};

static reg_info_t fpga_local_bus_test_regs[] = {
    /* NAME                    OFFSET TYPE          SIZE  MASK  RESET VAL*/
    {"Scratchpad Register",       FPGA_SCRATCHPAD_REG, FPGA_RW,
        {(unsigned long)&fpga_local_bus_reg_ext},   0xFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};

static reg_info_t fpga_regs[] = {
    /* NAME                    OFFSET TYPE          SIZE  MASK  RESET VAL*/
    {"NGIO GPIO Expander 0 Register",     FPGA_NGIO_GPIO_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"CPU Reset Control Register",       FPGA_CPU_RST_CTRL_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Power Status and Control Register", FPGA_PWR_STS_CTRL_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Power On Retry Counter Register",   FPGA_PWR_ON_RTRY_CTR_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"SM_RESET_L Device Enable Register", FPGA_SM_RST_DEV_EN_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Reset Signal Register",             FPGA_RST_SIG_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Device Setting Register",           FPGA_DEV_SETTING_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Device Status Signal Register",     FPGA_DEV_STATUS_SIG_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Clock Mux Status Register",         FPGA_CLK_MUX_STATUS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Jitter Cleaner Status Register",    FPGA_JITT_CLEANER_STS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"SFP TX FAULT Status and Ctrl Register",FPGA_SFP_TX_FAULT_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"SFP Present Status and Ctrl Register",FPGA_SFP_PRESENT_STS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"SFP RX LOSS Status and Ctrl Register",FPGA_SFP_RX_LOSS_CTRL_STS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"SFP3 Rate Sel Status and Ctrl Register",FPGA_SFP3_RATE_SEL_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"SFP4 TX Disable Status and Ctrl Register",FPGA_SFP4_DIS_CTRL_STS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Interrupt Status Register",       FPGA_INT_STS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Interrupt Mask Register",       FPGA_INT_MASK_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Remote Update Configuration Register",FPGA_REMOTE_UPD_CFG_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Remote Update Control Register",       FPGA_REMOTE_UPD_CTRL_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"Remote Update Status Register",       FPGA_REMOTE_UPD_STS_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"FPGA Low Version Register",       FPGA_LOW_VER_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"FPGA High Version Register",       FPGA_HIGH_VER_REG, FPGA_RONLY,
        {(unsigned long)&fpga_reg_ext},   0xFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};


/*
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 */

int fpga_test (int show_menu)
{
    build_primary_submenu(fpga_tests_submenu_table,
                          FPGA_TESTS_SUBMENU_TABLE_SIZE,
                          "FPGA", &fpga_submenup);
    build_secondary_submenu(fpga_tests_submenu_table,
                            FPGA_TESTS_SUBMENU_TABLE_SIZE,
                            fpga_tests_secondary_items);

    if (show_menu) {
        menu(fpga_submenup, fpga_tests_secondary_items, '\0' );
    } else {
        menu_exec_doall_diags(fpga_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : fpga_do_all_wrapper
 * Description : Wrapper for FPGA do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int fpga_do_all_wrapper (void)
{
    int rc = PASSED;

    if (fpga_register_test() == FAILED) {
        rc = FAILED;
    }

    if (fpga_local_bus_register_test() == FAILED) {
        rc = FAILED;
    }

    if (fpga_force_interrupt_test() == FAILED) {
        rc = FAILED;
    }

    if (fpga_burst_mode_register_test() == FAILED) {
        rc = FAILED;
    }

    if (fpga_sfp_i2c_read_test() == FAILED) { 
        rc = FAILED;
    }
    return (rc);
}


/*******************************************************************************
 *
 * Function    : fpga_utility
 * Description :
 * Inputs      : menu_option - display menu instead of running all temp. sensor tests.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */

static int fpga_utility (int show_menu)
{
    build_primary_submenu(fpga_util_items, FPGA_TESTS_UTIL_SIZE,
                          "FPGA Utilities Menu", &fpga_util_menup);
    build_secondary_submenu(fpga_util_items, FPGA_TESTS_UTIL_SIZE,
                            fpga_tests_secondary_util_items);

    if (show_menu) {
        menu(fpga_util_menup, fpga_tests_secondary_util_items, '\0' );
    } else {
        menu_exec_doall_diags(fpga_util_menup);
        prcomplete(testpass, errcount, (char *)0);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_register_test
 *
 * Description: This function performs the FPGA register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int fpga_register_test (void)
{
    testname("FPGA Register");

    if (register_tests(0, fpga_test_regs) == FAILED) {
        cterr('f', 0, "FPGA Register Test Failed");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_local_bus_register_test
 *
 * Description: This function performs the FPGA register test through local bus.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_local_bus_register_test (void)
{
    testname("FPGA Local Bus Register");

    if (register_tests(0, fpga_local_bus_test_regs) == FAILED) {
        cterr('f', 0, "FPGA Local Bus Register Test Failed");
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_burst_mode_register_test
 *
 * Description: This function performs the FPGA burst mode register test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_burst_mode_register_test (void)
{
    char reg_addr;
    uchar burst_reg_val[BURST_MODE_SIZE];
    uchar original_burst_buf[BURST_MODE_SIZE] = {0};
    uchar burst_buf[BURST_MODE_SIZE] = {0};
    uchar write_val[BURST_MODE_SIZE] = {0x00, 0x55, 0xaa, 0xff, 0x00, 0x55, 0xaa, 0xff};
    int ix, jx, rv;
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_FPGA;
    
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, CAVIUM_FPGA, CPU_I2C1) == FAILED) {
        cterr('f', 0, "Fail to open the i2c interface.");
        return (FAILED);
    }
    
    reg_addr = FPGA_GEP0_G0_SFP_CTL; 

    /* Save original 8 byte value */
    rv = read_i2c_reg(&i2c_dev, original_burst_buf, reg_addr, BURST_MODE_SIZE);
    if (rv != PASSED) {
        cterr('f', 0, "FPGA read burst mode fail at addr %x", reg_addr);
        return (FAILED);
    } 

    srand(time(NULL));
    for (jx = 0; jx < BURST_MODE_TIMES; jx++) {
        /* Alter register with new value */
        for (ix = 0; ix < BURST_MODE_SIZE; ix++) {
            burst_reg_val[ix] = write_val[rand()%BURST_MODE_SIZE];
        }
    
        rv = write_i2c_reg(&i2c_dev, burst_reg_val, reg_addr, BURST_MODE_SIZE);
        if (rv != PASSED) {
            cterr('f', 0, "FPGA write burst mode fail at addr %x", reg_addr);
            goto restore_original_val;
        }

        /* Read new 8 byte value */
        rv = read_i2c_reg(&i2c_dev, burst_buf, reg_addr, BURST_MODE_SIZE);
        if (rv != PASSED) {
            cterr('f', 0, "FPGA read burst mode fail at addr %x", reg_addr);
            goto restore_original_val;
        } 

        for (ix = 0; ix < BURST_MODE_REG_TEST_SIZE; ix++) {
            if (burst_buf[ix] != burst_reg_val[ix]) {
                cterr('f', 0, "Compare val fail - expected %x, read %x", burst_reg_val[ix], burst_buf[ix]);
                rv = FAILED;
                goto restore_original_val;
            }
        }
    }

    restore_original_val:
    /* Write back original val */
    if (write_i2c_reg(&i2c_dev, original_burst_buf, reg_addr, BURST_MODE_SIZE) == FAILED) {
        cterr('f', 0, "FPGA write back original val fail at addr %x", reg_addr);
        return (FAILED);
    }
    return (rv);
}

/******************************************************************************
 *
 * Function: fpga_force_interrupt_test
 *
 * Description: This function performs the FPGA force interrupt test.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int fpga_force_interrupt_test (void)
{
    uint32_t fpga_intr_bit;

    testname("FPGA Force Interrupt");
    /* Before we do interrupt test, need to clean up previous interrupt status by 
         write value 1 to interrupt status register 0x10 */
    fpga_reg_write(FPGA_INT_STS_REG, CLEAR_INTR_STATUS);
    
    /* Trigger the FPGA force interrupt by writting scratch pad register 0x4F
     * to 0x55 */
    fpga_reg_write(FPGA_SCRATCHPAG_REG, IRQL_LOW);

    msleep(10);

    /* Check if the Cavium GPIO bit is asserted.
     * The FPGA implemented the assertion of the interrupt
     * to be low.
     */
    fpga_intr_bit = get_gpio_rx_dat_bits(FPGA_INTR_GPIO10_MASK);
    if (fpga_intr_bit != 0) {
        cterr('f', 0, "FPGA did not assert force interrupt signal");
        return(FAILED);
    }

    /* Clear the FPGA force interrupt by writting scratch pad register 0x4F
     * to 0x0 */
    fpga_reg_write(FPGA_SCRATCHPAG_REG, CLEAR_IRQL_LOW);

    msleep(10);

    /* Check if the Cavium GPIO bit is de-asserted.
     */
    fpga_intr_bit = get_gpio_rx_dat_bits(FPGA_INTR_GPIO10_MASK);
    if (fpga_intr_bit == 0) {
        cterr('f', 0, "FPGA did not de-assert force interrupt signal");
        return(FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: erase_fpga_flash_sector
 *
 * Description: This function erases specific sector of FPGA Flash
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int erase_fpga_flash_sector (void)
{
    int sector;

    printf("\nErase FPGA Flash Utility:\n");
    sector = getdec_answer("Enter sector number", REMOTE_UPDATE_GOLDEN_SECT_0,
                           REMOTE_UPDATE_GOLDEN_SECT_0, REMOTE_UPDATE_NORMAL_SECT_3);

    printf("Warning! Sector-%d will be erased!\n", sector);
    if (getc_answer("Continue? (y/n)", "yn", 'n') != 'y') {
        printf("User abort\n");
        return (PASSED);
    }

    /* 1. Set reg0x70.bit5=1 for starting flash update */
    fpga_reg_or(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);

    printf("Erasing Sector-%d...", sector);
    fflush(stdout);
    /* 2~4. Erase sector */
    if (fpga_upgrade_sector_erase(sector) == FAILED) {
        printf("FAIL\n");
    }
    printf("OK\n");

    /* Disable flash update enable */
    fpga_reg_nand(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);

    printf("\n");

    return (PASSED);
}


/******************************************************************************
 *
 * Function: display_fpga_flash_content
 *
 * Description: This function displays flash content of FPGA
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int display_fpga_flash_content (void)
{
    int sector, page;
    int ix;
    uchar buf;

    printf("\nDisplay FPGA Flash Utility:\n");
    sector = getdec_answer("Enter sector number", REMOTE_UPDATE_GOLDEN_SECT_0,
                           REMOTE_UPDATE_GOLDEN_SECT_0, REMOTE_UPDATE_NORMAL_SECT_3);

    page = getdec_answer("Enter page number", 0, 0, 255);

    /* 1. Set reg0x70.bit5=1 for starting flash update */
    fpga_reg_or(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);

    /* 2. Writing registers 0x7A~0x7C as the byte address for updating or
     * reading data
     */

    fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR2, page);
    fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR3, sector);

    printf("\nSector-%d Page-%d:\n", sector, page);

    printf("0x00: ");
    for (ix = 0; ix < 256; ix++) {
        fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR1, ix);

        if (ix && ((ix % 16) == 0)) {
            printf("\n");
            printf("0x%02x: ", ix);
        }

        fpga_reg_read(FPGA_REMOTE_UPD_DATA_OUT, (char *)&buf);

        printf("%02x ", buf);
    }

    /* Disable flash update enable */
    fpga_reg_nand(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);

    printf("\n");

    return (PASSED);

}

/******************************************************************************
 *
 * Function: program_fpga
 *
 * Description: This function support Cavium program the FPGA thru I2C.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int program_fpga (void)
{
    int normal_sector[] = {REMOTE_UPDATE_NORMAL_SECT_0, 
                           REMOTE_UPDATE_NORMAL_SECT_1,
                           REMOTE_UPDATE_NORMAL_SECT_2,
                           REMOTE_UPDATE_NORMAL_SECT_3};
    
    int start_sector, sector_size, *array_sector;
    int ix;
    char answer, page, byte_addr;
    
    page = REMOTE_UPDATE_START_PAGE;
    byte_addr = REMOTE_UPDATE_START_BYTE_ADDR;
    
    /* Prompt user to confirm whether FPGA needs to be upgraded or not */
    printf("Current FPGA Version:");
    display_fpga_version();
    printf("\nFPGA Version will be upgraded to 'V%02x.%02x'\n", fpga_fw_major_rev,
            fpga_fw_minor_rev);
    printf("*****Do not remove power during FPGA upgrade*****\n"); 

    if (getc_answer("Continue? (y/n)", "yn", 'n') != 'y') {
        printf("User abort\n");
        return (PASSED);
    }

    if (answer == 'a') {
        printf("User aborted\n");
        return (PASSED);
    }

    /* Normal Image */
    start_sector = REMOTE_UPDATE_NORMAL_SECT_0;
    array_sector = &normal_sector[0];
    sector_size  = sizeof(normal_sector)/sizeof(int);

    printf("Start programming FPGA...\n");

    /* 1. Set reg0x70.bit5=1 for starting flash update */
    fpga_reg_or(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);

    printf("Erasing Upgrade Sectors...");
    fflush(stdout);
    /* 2~4. Erase sector */
    for (ix = 0; ix < sector_size; ix++) {
        if (fpga_upgrade_sector_erase(array_sector[ix]) == FAILED) {
            return (FAILED);
        } 
    }
    printf("OK\n");
    
    /* 5. Writing registers 0x7A~0x7C as the byte address for updating or
     * reading data
     */
    fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR1, byte_addr);
    fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR2, page);
    fpga_reg_write(FPGA_REMOTE_UPD_SPI_ADDR3, start_sector);

    printf("Upgrading FPGA now...(Total = %lu bytes)\n", fpga_normal_image_fw_size);

    /* 6. Write the SPI Flash data through reg0x7D sequentially */
    for (ix = 0; ix < fpga_normal_image_fw_size; ix++) {
        if(fpga_reg_write(FPGA_REMOTE_UPD_DATA_IN, 
           fpga_normal_image_fw_array[ix]) == FAILED) {
            printf("\nERROR!!! Fail writing data (%02x) to location %08x\n",
                    fpga_normal_image_fw_array[ix], ix);
            /* Disable flash update enable */
            fpga_reg_nand(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);
            return (FAILED);
        }
        if((ix % 1000) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    
    printf("\nFPGA Program is done.\n");
    printf("Please reboot the system to load new FPGA\n");

    /* Disable flash update enable */
    fpga_reg_nand(FPGA_REMOTE_UPD_CFG_REG, REMOTE_UPDATE_FLASH_UPDATE_EN);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: display_fpga_version
 *
 * Description: This function shows the FPGA version.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int display_fpga_version (void)
{
    unsigned char low_ver, high_ver;
    unsigned int major_ver;
    char ver_str[16];

    fpga_reg_read(FPGA_LOW_VER_REG, (char *)&low_ver);
    fpga_reg_read(FPGA_HIGH_VER_REG, (char *)&high_ver);

    /* Major version is lowest 4 bits */
    major_ver = high_ver & 0xf;

    if (high_ver & 0x80) {
        sprintf(ver_str, "Upgrade");
    } else {
        sprintf(ver_str, "Golden");
    }

    printf("\nFPGA (%s) Version is V%02x.%02x\n", ver_str, major_ver, low_ver);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: alter_fpga_regs
 *
 * Description: This function supports to alter the FPGA register.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int alter_fpga_regs (void)
{
    char reg_addr, reg_val;

    printf("Alter FPGA Register.\n");
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ", 0, 0, 0xff); 

    /* Display original value */
    if (fpga_reg_read((int)reg_addr, &reg_val) == FAILED) {
        printf("Read FPGA register %#.8x failed\n", reg_addr);
        return (FAILED);
    } else {
        printf("Original data of register %#.8x = %#.8x\n", reg_addr, reg_val);
    }

    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", reg_val, 0,
                            0xFF);

    if (fpga_reg_write((int)reg_addr, reg_val) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", reg_val, reg_addr);
        return (FAILED);
    }

    /* Display the value again */
    if (fpga_reg_read((int)reg_addr, &reg_val)) {
        printf("Read FPGA register %#.8x failed\n", reg_addr);
        return (FAILED);
    } else {
        printf("New data of register %#.8x = %#.8x\n", reg_addr, reg_val);
        return (PASSED);
    }
}

/******************************************************************************
 *
 * Function: display_fpga_regs
 *
 * Description: This function supports to display the FPGA registers.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int display_fpga_regs (void)
{
    uchar buf;
    int ix;

    for (ix = 0; ix < 23; ix++) {
        fpga_reg_read(fpga_regs[ix].offset, (char *)&buf);
        printf("offset %#.8x value %#.8x\n", fpga_regs[ix].offset, buf);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_local_bus_read
 *
 * Description: This function supports to read the FPGA registers through local bus.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_local_bus_read (void)
{
    unsigned char reg_addr;
    uchar *fpga_offset; 
    
    printf("Read FPGA register through local bus\n");

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    } 
    
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xff): ", 0, 0, 0xff);
    fpga_offset += reg_addr;
    printf("Read FPGA register %#.8x, val = %#.8x\n", reg_addr, *fpga_offset);
    
    return (PASSED);
}

/******************************************************************************
 *
 * Function: fpga_local_bus_write
 *
 * Description: This function supports to write the FPGA registers through local bus.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int fpga_local_bus_write (void)
{
    unsigned char reg_addr, reg_val;
    uchar *fpga_offset;

    printf("Alter FPGA register through local bus\n");

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    } 
        
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xff): ", 0, 0, 0xff);
    fpga_offset += reg_addr;
    printf("Read register %#.8x, original val = %#.8x\n", reg_addr, *fpga_offset);
    
    /* Alter register with new value */
    reg_val = gethex_answer("Enter the new data (hex): ", *fpga_offset, 0, 0xff);
    *fpga_offset = reg_val;
    printf("Read register %#.8x, new val = %#.8x\n", reg_addr, *fpga_offset);
        
    return (PASSED);
}

/******************************************************************************
 *  
 * Function: fpga_read_burst_mode
 *    
 * Description: This function supports to read 8 byte FPGA registers 
 *              continuously.
 *   
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
int fpga_read_burst_mode(void)
{
    char reg_addr;
    uchar burst_buf[BURST_MODE_SIZE];
    int ix, rv;
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_FPGA;
    
    printf("FPGA read burst mode.\n");
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, CAVIUM_FPGA, CPU_I2C1) == FAILED) {
        cterr('f', 0, "Fail to open the i2c interface.");
        return (FAILED);
    }
    
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ", 0, 0, 0xff); 

    rv = read_i2c_reg(&i2c_dev, (uchar *)&burst_buf, reg_addr, BURST_MODE_SIZE);
    if (rv != PASSED) {
        printf("FPGA read burst mode fail at addr %x\n", reg_addr);
        return (FAILED);
    } else {
        for (ix = 0; ix < BURST_MODE_SIZE; ix++) {
            printf(" %02x ", burst_buf[ix]);
        }
        printf("\n");
    }

    return (PASSED);
}

/******************************************************************************
 *    
 * Function: fpga_write_burst_mode
 *        
 * Description: This function supports to write 8 byte FPGA registers 
 *              continuously.
 *           
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *      
 *******************************************************************************/
int fpga_write_burst_mode(void)
{
    char reg_addr;
    char burst_reg_val[BURST_MODE_SIZE];
    uchar burst_buf[BURST_MODE_SIZE] = {0};
    int ix, rv;
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_FPGA;
    
    printf("FPGA write burst mode.\n");
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, CAVIUM_FPGA, CPU_I2C1) == FAILED) {
        cterr('f', 0, "Fail to open the i2c interface.");
        return (FAILED);
    }
    
    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xFF): ", 0, 0, 0xff); 

    /* Read original 8 byte value */
    rv = read_i2c_reg(&i2c_dev, (uchar *)&burst_buf, reg_addr, BURST_MODE_SIZE);
    if (rv != PASSED) {
        printf("FPGA read burst mode fail at addr %x\n", reg_addr);
        return (FAILED);
    } else {
        for (ix = 0; ix < BURST_MODE_SIZE; ix++) {
            printf(" %02x ", burst_buf[ix]);
        }
        printf("\n");
    }
  
    /* Alter register with new value */
    for (ix = 0; ix < BURST_MODE_SIZE; ix++) {
        burst_reg_val[ix] = gethex_answer("Enter the new data (hex): ", burst_reg_val[ix], 0, 0xFF);
    }
    
    rv = write_i2c_reg(&i2c_dev, (uchar *)burst_reg_val, reg_addr, BURST_MODE_SIZE);
    if (rv != PASSED) {
        printf("FPGA write burst mode fail at addr %x\n", reg_addr);
        return (FAILED);
    }

    /* Read new 8 byte value */
    rv = read_i2c_reg(&i2c_dev, (uchar *)&burst_buf, reg_addr, BURST_MODE_SIZE);
    if (rv != PASSED) {
        printf("FPGA read burst mode fail at addr %x\n", reg_addr);
        return (FAILED);
    } else {
        for (ix = 0; ix < BURST_MODE_SIZE; ix++) {
            printf(" %02x ", burst_buf[ix]);
        }
        printf("\n");
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: set_fpga_boot_flag 
 *
 * Description: When get into normal image, write register 0x71 bit 0 to 1 can reload 
 *                   golden image. In golden image, write register 0x71 bit 0 to 1 can reload
 *                   normal image.
 *
 * Inputs      : void
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int set_fpga_boot_flag (void)
{
    /* When we reload golden image, it'll jump back to normal image, 
       it'll stay in golden image when normal image is damage */

    printf("Reload FPGA with golden image\n");
    /* reload fpga golden or normal image */
    fpga_reg_or(FPGA_REMOTE_UPD_CTRL_REG, REMOTE_UPDATE_ALTRU_RELOAD);
    return (PASSED);
}
   
/**********************************************************************
 *
 * Function: diag_fpga_read_fn
 *
 * Description: FPGA register read called by register_display
 *
 * Inputs      : addr - FPGA register offset
 *               size - FPGA register size
 *               *buf - pointer to the data buf
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_fpga_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    char temp;
    uint8_t *data_buf = (uint8_t *)buf;

    *buf = 0;

    fpga_reg_read(addr, &temp);

    data_buf[3] = temp;

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_fpga_write_fn
 *
 * Description: handoff FPGA register write called by register_display
 *
 * Inputs      : addr - FPGA register offset
 *               size - FPGA register size
 *               data - the write data value
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_fpga_write_fn (ulong addr, int size, ulong data, void *param)
{
    fpga_reg_write((int)addr, data);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_fpga_local_bus_read_fn
 *
 * Description: FPGA register read called by register_display through local bus
 *
 * Inputs      : addr - FPGA register offset
 *               size - FPGA register size
 *               *buf - pointer to the data buf
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_fpga_local_bus_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    uchar *fpga_offset;
    uint8_t *data_buf = (uint8_t *)buf;

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    } 

    data_buf[3] = *(fpga_offset + addr);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_fpga_local_bus_write_fn
 *
 * Description: handoff FPGA register write called by register_display through local bus
 *
 * Inputs      : addr - FPGA register offset
 *               size - FPGA register size
 *               data - the write data value
 *               *param - pointer to param
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int diag_fpga_local_bus_write_fn (ulong addr, int size, ulong data, void *param)
{
    uchar *fpga_offset;

    fpga_offset = fpga_get_local_bus_addr();
    if (fpga_offset == NULL) {
        return (FAILED);
    } 

    fpga_offset += (uchar)addr;

    *fpga_offset = (uchar)data;
    
    return (PASSED);
}

/******************************************************************************
 *
 * Function: config_fpga_clk_mux_sel
 *
 * Description: config FPGA clock mux selection
 *
 * Inputs      : Clock source from MRVL1548 PHY or CPU
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int config_fpga_clk_mux_sel (uint8_t clk_source)
{
    if (fpga_reg_write(FPGA_CLK_MUX_STATUS_REG, clk_source) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", clk_source, FPGA_CLK_MUX_STATUS_REG);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: config_fpga_trig_mux_sel
 *
 * Description: config FPGA clock mux selection
 *
 * Inputs      : Clock source from PHY or CPU
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int config_fpga_trig_mux_sel (uint8_t clk_source)
{
    if (fpga_reg_write(FPGA_TRIG_MUX_CTRL_REG, clk_source) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", clk_source, FPGA_CLK_MUX_STATUS_REG);
        return (FAILED);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_fpga_test.c,v $
 * Revision 1.3  2015/02/14 12:48:41  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.2.8.1  2014/04/30 13:47:22  kodko
 * Support 1548P/2222P clock/trigger in/out verification.
 *
 * Revision 1.2  2013/10/08 08:48:27  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:51  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.4  2013/06/17 11:05:52  leschen
 * Remove static declaration from alter and dump utility
 *
 * Revision 1.1.2.3  2013/06/05 08:03:28  leschen
 * Add don't remove power warning message when upgrade FPGA
 *
 * Revision 1.1.2.2  2013/05/20 06:43:35  leschen
 * Add fpga i2c read test
 *
 * Revision 1.1.2.1  2013/04/24 10:37:15  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.7  2013/04/10 09:48:48  kuangik
 * Implement do all wrapper for host run sm test
 *
 * Revision 1.6  2013/03/27 07:57:15  leslie
 * Add FPGA local bus register test and r/w utility
 *
 * Revision 1.5  2013/03/27 04:49:35  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.4  2013/03/20 10:48:27  leslie
 * Update for FPGA i2c burst mode reg test
 *
 * Revision 1.3  2013/03/20 06:44:41  leslie
 * Add FPGA i2c burst mode register test
 *
 * Revision 1.2  2013/03/13 10:06:29  leslie
 * Add FPGA r/w burst mode utility.
 *
 * Revision 1.23  2013/03/12 11:25:34  leslie
 * Fix submenu flag for show error number message
 *
 * Revision 1.22  2013/03/07 15:29:25  kuangik
 * Add FPGA Flash Debug Utility
 *
 * Revision 1.21  2013/03/07 02:24:04  kuangik
 * Add Show error count
 *
 * Revision 1.19  2013/01/18 06:26:33  leslie
 * Fix and clean up code.
 *
 * Revision 1.18  2013/01/15 23:36:41  leslie
 * Move get sku id function to diag_fpga_lib.c file.
 *
 * Revision 1.17  2013/01/13 23:56:02  leslie
 * Add function to read fpga register to distinguish SKU type.
 *
 * Revision 1.16  2012/11/19 02:29:44  leslie
 * Clean up fpga intr status reg before do fpga intr test and check gpio bit 10.
 *
 * Revision 1.15  2012/10/24 09:37:08  leslie
 * Fix argument type.
 *
 * Revision 1.14  2012/10/04 03:10:45  leslie
 * Fix the FPGA program utility.
 *
 * Revision 1.13  2012/09/21 11:43:33  kody
 * Modify the dump fpga registers function.
 *
 * Revision 1.12  2012/08/30 06:32:28  leslie
 * Add pass message of fpga register test.
 *
 * Revision 1.10  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.7  2012/07/19 06:32:24  leslie
 * Use cterr instead of use err
 *
 * Revision 1.6  2012/05/18 10:19:09  kody
 * Fix the type warning during compile.
 *
 * Revision 1.5  2012/04/16 12:33:40  kuangik
 * Add FPGA Firmware upgrade function
 *
 * Revision 1.4  2012/04/06 06:05:59  kuangik
 * Update for FPGA Test Item
 *
 * Revision 1.3  2012/02/13 03:31:10  leslie
 * Add function prototype.
 *
 * Revision 1.2  2012/02/10 06:41:21  leslie
 * Add symbol  for cvs comment history.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
