/* $Id: szalinski_diag.c,v 1.2 2015/05/25 03:59:17 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/szalinski_diag.c,v $
 *------------------------------------------------------------------------------
 * 
 * szalinski_diag.c: Entry point of Szalinski FPGA tests and utilities.
 *
 * July 14, 2013 - palin2 created for ShrinkRay.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <gxio/common.h>
#include <gxio/gpio.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "errno.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "skye_i2c.h"
#include "fpga_szalinski.h"
#include "skye_led.h"
#include "types.h" 
#include "queryflags.h" 
#include "nvmonvars.h"
#include "skye_main.h"

#ifdef SKYE_ENHANCED_ERR_MSG
#include "platform_fru.h"
#endif   /* SKYE_ENHANCED_ERR_MSG */


/*******************************************************************************
 *                           Function Prototypes
 *******************************************************************************
 */
int        szalinski_reg_rd_util(int);
int        szalinski_reg_wr_util(int);
int        szalinski_reg_dump_util(int);
int        szalinski_show_ver(int);
int        sray_cpu_reset_util(int);
int        sray_dev_reset_util(int);
int        sys_pwr_save_util(int);
int        set_wdt_util(int);
int        cpu0_spi_mux_util(int);
int        usb_mode_ctrl_util(int);
int        szalinski_spirom_util(void);
int        check_cpu1_wdt(void);
static int szalinski_reg_test(void);
static int util_szalinski_image_upgrade(void);
static int szalinski_interrupt_test(void);
static int szalinski_thermal_int_test(void);
static int szalinski_wdt_test(void);
static int szalinski_check_cpu1_wdt(void);
static int skye_fpga_srom_prot_check(int, int);
int        util_prot_skye_fpga_gld(int);
int        util_fpga_srom_prot_get(void);

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern boolean is_cpu0();
extern uint64_t get_gxio_gpio();
extern boolean check_cpu(int);
boolean cpu_id;
extern void wdt_act(int);
extern int skye_clk_buf_i2c_write(uint16_t, uint16_t, uchar*);
extern int skye_clk_buf_i2c_read (uint16_t, uint16_t, uchar*);
extern int util_clock_buffer_reg_wr(void);
extern int util_clock_buffer_reg_rd(void);
extern int on_board_ts_reg_wr(uint16_t, boolean);
extern int dump_on_board_ts_reg_util(int);
extern int skye_on_board_thermal_rd(uint16_t, uchar *);
extern int skye_on_board_thermal_wr(uint16_t, uchar *);
extern int skye_i2c_mux_ctrl_reg_wr(uchar *);
extern int cpu_gpio_pin_state_wrap(void);
/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
#define TS_TEST_HYS   0

/* Szalinski FPGA Registers Table */
/* Szalinski FPGA Register Test Table */
static reg_info_t szalinski_reg_test_tbl[] = {
    {"LED Control",                             LED_CTRL_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x07, 0x04},
    {"Interrupt Enable",                        INTERRUPT_EN_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xBF, 0x00},
};

/* Szalinski FPGA General Registers */
static reg_info_t szalinski_gen_regs_tbl_cpu0[] = {
    {"Szalinski FPGA Revision",                 FPGA_REV_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Board ID",                                BOARD_ID_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)ONE_B_REG},  0x00, 0x00},
    {"CPU0 Attached Individual Device Reset",   CPU0_DEV_RST_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x3F, 0x00},
    {"LED Control",                             LED_CTRL_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x07, 0x04},
    {"CPU0 WDT",                                CPU0_WDT_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xFF, 0x00},
    {"GPIO Expander Low 8-bit",                 GPIO_EXP_L8_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x08, 0x23},
    {"GPIO Expander High 8-bit",                GPIO_EXP_H8_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)ONE_B_REG},  0x00, 0x00},
    {"Clock Margin Setting",                    CLK_MARGIN_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xFF, 0x40},
    {"CPU0 SPI MUX Control",                    CPU0_SPI_MUX_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x01, 0x02},
    {"System Power Saving",                     SYS_PWR_SAV_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x13, 0x03},
    {"10G-KR PRBS Control",                     PRBS_10GKR_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x04, 0x00},
    {"USB Mode Control Register",               USB_MOD_CTRL_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)ONE_B_REG},  0x00, 0x00},
    {"BIB and eUSB ROM WP Control(for CPU0)",   CPU0_ROM_WP_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x03, 0x00},
    {"eUSB  Control",                           EUSB_CTRL_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x01, 0x01},
    {"Reset Event Log",                         RST_LOG_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)ONE_B_REG},  0x00, 0x00},
    {"PCIE Mode Control",                       PCIE_MOD_CTRL_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)ONE_B_REG},  0x00, 0x01},
    {"Interrupt",                               INTERRUPT_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xFF, 0x00},
    {"Interrupt Enable",                        INTERRUPT_EN_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xBF, 0x00},
    {"Interrupt Access Select",                 INTR_ACC_SEL_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x00, 0x00},
    {"Multiboot Control",                       MBOOT_CTRL_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Multiboot Status",                        MBOOT_STAT_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Multiboot Header ID",                     MBOOT_HID_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Multiboot Header Date",                   MBOOT_HDATA_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Multiboot Header Flag",                   MBOOT_HFLAG_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Multiboot Magic Number",                  MBOOT_MAGNUM_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Multiboot State History",                 MBOOT_HISTORY_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"FPGA SPI Control",                        FPGA_SPI_CTRL_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"FPGA SPI Status",                         FPGA_SPI_STAT_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"FPGA SPI Read Size",                      FPGA_SPI_RSZ_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"FPGA SPI Data",                           FPGA_SPI_DATA_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"FPGA SPI Address & Opcode",               FPGA_SPI_ADDR_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Core Status Golden",          SB_CORE_STAT_G_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Check Status Golden",         SB_CHK_STAT_G_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Signature Address Golden",    SB_SIG_ADDR_G_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Signature Size Golden",       SB_SIG_SZ_G_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Core Status Upgrade",         SB_CORE_STAT_U_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Check Status Upgrade",        SB_CHK_STAT_U_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Signature Address Upgrade",   SB_SIG_ADDR_U_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
    {"Secure Boot Signature Size Upgrade",      SB_SIG_SZ_U_REG_OFF,
     (READ_ONLY | REG_ACCESS),                  {(uint)FOUR_B_REG}, 0x00, 0x00},
};

static reg_info_t szalinski_gen_regs_tbl_cpu1[] = {
    {"CPU1 Attached Individual Device Reset",   CPU1_DEV_RST_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x07, 0x00},
    {"CPU1 WDT",                                CPU1_WDT_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xFF, 0x00},
    {"BIB and eUSB ROM WP Control(for CPU1)",   CPU1_ROM_WP_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0x01, 0x00},
    {"Interrupt",                               INTERRUPT_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xFF, 0x00},
    {"Interrupt Enable",                        INTERRUPT_EN_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),  {(uint)ONE_B_REG},  0xBF, 0x00},
};

/*******************************************************************************
 *                                 Menu
 *******************************************************************************
 */
submenu_xtable_t szalinski_diag_tbl[] = {
    {"Szalinski SW Interrupt Test",        (PFT)szalinski_interrupt_test,  FALSE,
     (MF_CONTINUOUS | MF_DOALL),           (type_t(*)())check_cpu,         0,
     (PFT)szalinski_interrupt_test,         TRUE},
    {"Szalinski Thermal Interrupt Test",   (PFT)szalinski_thermal_int_test,false,
     (MF_CONTINUOUS | MF_DOALL),           (type_t(*)())0,         0,
     (PFT)szalinski_thermal_int_test,       TRUE},
    {"Szalinski registers test",           (PFT)szalinski_reg_test,        FALSE,
     (MF_CONTINUOUS | MF_DOALL),           (type_t(*)())check_cpu,         0,
     (PFT)szalinski_reg_test,              TRUE},
    {"Szalinski watchdog test",            (PFT)szalinski_wdt_test,    FALSE,
     (MF_CONTINUOUS | MF_DOALL),           (type_t(*)())0,                 0,
     (PFT)szalinski_wdt_test,              TRUE},
    {"Check CPU1 WDT status utility",      (PFT)szalinski_check_cpu1_wdt,  FALSE,
     0,                                    (type_t(*)())check_cpu,         0,
     (PFT)szalinski_check_cpu1_wdt,              TRUE},
    {"Szalinski show version utility",     (PFT)szalinski_show_ver,        TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)szalinski_show_ver,              FALSE},
    {"Szalinski reg. dump utility",        (PFT)szalinski_reg_dump_util,   TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
    {"Szalinski reg. read utility",        (PFT)szalinski_reg_rd_util,     TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
    {"Szalinski reg. write utility",       (PFT)szalinski_reg_wr_util,     TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
    {"Device Reset utility",               (PFT)sray_dev_reset_util,       TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)sray_dev_reset_util,             FALSE},
    {"Szalinski LED utility",              (PFT)skye_fpga_led_util,   TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)0,                               0},
    {"CPU Reset utility",                  (PFT)sray_cpu_reset_util,       TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)sray_cpu_reset_util,             FALSE},
    {"System power saving utility",        (PFT)sys_pwr_save_util,         TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)sys_pwr_save_util,               FALSE},
    {"WatchDog Timer utility",             (PFT)set_wdt_util,              TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)set_wdt_util,                    FALSE},
    {"CPU0 SPI Mux Control utility",       (PFT)cpu0_spi_mux_util,         TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)cpu0_spi_mux_util,               FALSE},
    {"USB Mode Control utility",           (PFT)usb_mode_ctrl_util,        TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)usb_mode_ctrl_util,              FALSE},
    {"Szalinski SPIROM utility",           (PFT)szalinski_spirom_util,     TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)szalinski_spirom_util,            FALSE},
    {"Szalinski FPGA Flash Image Update",  (PFT)util_szalinski_image_upgrade, TRUE,
     0,                                    (type_t(*)())is_cpu0,           0,
     (PFT)util_szalinski_image_upgrade,     FALSE},
    {"Check CPU GPIO by pin",              (PFT)cpu_gpio_pin_state_wrap,   TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
    {"Lock SPIROM Golden Sectors",         (PFT)util_prot_skye_fpga_gld,   ENABLE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
    {"Unlock SPIROM Golden Sectors",       (PFT)util_prot_skye_fpga_gld,   DISABLE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
    {"Get FPGA SPIROM protection state",   (PFT)util_fpga_srom_prot_get,   TRUE,
     0,                                    (type_t(*)())0,                 0,
     (PFT)0,                               0},
};

#define SZALINSKI_DIAG_TBL_SIZE \
        (sizeof(szalinski_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t szalinski_diag_pri_items[SZALINSKI_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t szalinski_diag_sec_items[SZALINSKI_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo szalinski_diag = {
    "%s SubMenu",		/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    szalinski_diag_pri_items,
};
static struct menuinfo *szalinski_diagp = &szalinski_diag;


/*******************************************************************************
 *
 * Function   : fpga_szalinski_diag
 * Description: Entry function for Szalinski FPGA Diag tests and utilities.
 * Inputs     : menu_opt - parameter determines to show menu or not 
 * Outputs    : None
 *
 *******************************************************************************
 */
int
fpga_szalinski_diag (int menu_opt)
{
    build_primary_submenu(szalinski_diag_tbl, SZALINSKI_DIAG_TBL_SIZE,
                          "Szalinski Diag", &szalinski_diagp);
    build_secondary_submenu(szalinski_diag_tbl, SZALINSKI_DIAG_TBL_SIZE,
                            szalinski_diag_sec_items);

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_FPGA;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "Szalinski FPGA");
	
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)szalinski_reg_dump_util);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check Szalinski FW revision is correct.",
                    "Check I2C bus.",
                    "Measure Szalinski power.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
	
    if (menu_opt) {
        menu(&szalinski_diag, szalinski_diag_sec_items, 0);
    } else {
        menu_exec_doall_diags(szalinski_diagp);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : szalinski_reg_dump_util
 * Description: Wrapped uility to dump all Szalinski registers.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_reg_dump_util (int opt)
{
    int        reserved = 0, ctr = 0, total_num = 0, r_ctr = 0, out_ctr = 0;
    reg_info_t *reg_p = 0;
    uchar      rd_val[256];

    reserved = opt;

    memset(&rd_val, 0, sizeof(rd_val));

    if (cpu_id == MASTER_CPU) {
        /* CPU0 */
        reg_p = &szalinski_gen_regs_tbl_cpu0[0];
        total_num = (sizeof(szalinski_gen_regs_tbl_cpu0) / sizeof(reg_info_t));
    } else {
        /* CPU1 */
        reg_p = &szalinski_gen_regs_tbl_cpu1[0];
        total_num = (sizeof(szalinski_gen_regs_tbl_cpu1) / sizeof(reg_info_t));
    }

    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        if (skye_fpga_i2c_read(reg_p->offset, reg_p->size.size,
                                    (uchar *)&rd_val[r_ctr]) != PASSED) {
            printf("\nFailed to read from Szalinski reg. 0x%02X.\n",
                   reg_p->offset);
            return (FAILED);
        }
        r_ctr += reg_p->size.size;
    }

    /* Dump registers */
    printf("\nSzalinski Registers Dump(accessable for CPU%d)", cpu_id);
    printf("\n-------------------------------------------------------\n");

    if (cpu_id == MASTER_CPU) {
        reg_p = &szalinski_gen_regs_tbl_cpu0[0];
    } else {
        reg_p = &szalinski_gen_regs_tbl_cpu1[0];
    }

    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        printf("%-37s (0x%02X):", reg_p->name, reg_p->offset);

        printf("0x");
        for (r_ctr = 0; r_ctr < reg_p->size.size; r_ctr++) {
            printf("%02X", rd_val[(out_ctr + (reg_p->size.size - r_ctr - 1))]);
        }
        printf("\n");
        out_ctr += reg_p->size.size;
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : szalinski_reg_rd_util
 * Description: Wrapped uility to read Szalinski register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_reg_rd_util (int opt)
{
    int      reserved = 0, ctr = 0;
    uint16_t offset = 0, rd_size = 0;
    uchar    rd_val[256];

    reserved = opt;

    memset(&rd_val, 0, sizeof(rd_val));

    offset = (uint16_t)gethex_answer("Enter read starting offset", 0, 0, 0x7C);
    rd_size = (uint16_t)getdec_answer("Enter read size(by byte)", 1, 1, 256);

    if (skye_fpga_i2c_read(offset, rd_size, (uchar *)&rd_val) != PASSED) {
        printf("\nFailed to read %d byte(s) from Szalinski reg. 0x%02X.\n",
               rd_size, offset);
        return (FAILED);
    }

    printf("\nTotal %d byte(s) read back from Szalinski reg. 0x%02X:",
           rd_size, offset);
    for (ctr = 0; ctr < rd_size; ctr++) {
        if ((ctr % 8) == 0) {
            printf("\n0x%02X:", (offset + ctr));
        }
        printf(" 0x%02X", rd_val[ctr]);
    }
    printf("\n\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : szalinski_reg_wr_util
 * Description: Wrapped uility to write Szalinski register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_reg_wr_util (int opt)
{
    int      reserved = 0;
    uint16_t offset = 0;
    uchar    val_now = 0, w_data = 0;

    reserved = opt;

    offset = (uint16_t)gethex_answer("Enter reg. offset that you want to alter",
                                     0, 0, 0x7C);

    if (skye_fpga_i2c_read(offset, ONE_BYTE, (uchar *)&val_now) != PASSED) {
        printf("\nFailed to read from Szalinski reg. 0x%02X.\n", offset);
        return (FAILED);
    }

    w_data = (uchar)gethex_answer("Enter data that you want to write-in",
                                  val_now, 0, 0xFF);

    if (skye_fpga_i2c_write(offset, sizeof(w_data), &w_data) != PASSED) {
        printf("\nFailed to write 0x%02X to Szalinski reg. 0x%02X.\n",
               w_data, offset);
        return (FAILED);
    }

    /* Read Reg. value back and show to user */
    val_now = 0;

    if (skye_fpga_i2c_read(offset, ONE_BYTE, (uchar *)&val_now) != PASSED) {
        printf("\nFailed to read from Szalinski reg. 0x%02X.\n", offset);
        return (FAILED);
    }

    printf("\nNow Szalinski reg. 0x%02X = 0x%02X.\n", offset, val_now);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : szalinski_show_ver
 * Description: Uility to show Szalinski FPGA FW revision.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_show_ver (int opt)
{
    int reserved = 0;
    uint32_t fpga_rev = 0;

    reserved = opt;

    if (skye_fpga_i2c_read(FPGA_REV_REG_OFF, sizeof(fpga_rev),
                                (uchar *)&fpga_rev) != PASSED) {
        printf("\nFailed to read FPGA revision register(0x%02X).\n",
               FPGA_REV_REG_OFF);
        return (FAILED);
    }

    printf("\nSzalinski FW info(YEAR/MON/DAY/HOUR): %x/%x/%x/%x.\n",
           ((fpga_rev & FPGA_CREATE_YEAR_MSK) >> FPGA_CREATE_YEAR_OFF),
           ((fpga_rev & FPGA_CREATE_MON_MSK) >> FPGA_CREATE_MON_OFF),
           ((fpga_rev & FPGA_CREATE_DAY_MSK) >> FPGA_CREATE_DAY_OFF),
           (fpga_rev & FPGA_CREATE_HOUR_MSK));

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : szalinski_reg_test
 * Description: Szalinski FPGA register test.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
szalinski_reg_test (void)
{
    uint32_t   ctr = 0, test_ctr = 0, test_size = 0;
    uchar      ori_val = 0, test_data = 0, check_data = 0;
    reg_info_t *reg_p = 0;

    testname("Skye FPGA(Szalinski) register");
    prpass(testpass, " ");

    reg_p = &szalinski_reg_test_tbl[0];
    test_size = (sizeof(szalinski_reg_test_tbl) / sizeof(reg_info_t));

    for (ctr = 0; ctr < test_size; ctr++, reg_p++) {
        if (!(reg_p->type & READ_ONLY) || (reg_p->type & WRITE_ONLY)) {
            /* Backup Original value */
            if (skye_fpga_i2c_read(reg_p->offset, sizeof(ori_val),
                                        (uchar *)&ori_val) != PASSED) {
                cterr('f', 0, "%s: Failed to read Skye FPGA Reg %#x"
                              " for restore before test.",
                              __FUNCTION__, reg_p->offset);
                return (FAILED);
            }

            /*
             * Ripple 1 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(SR_FPGA_ADDR_SZ) * 8);
                 test_ctr++) {
                test_data = ((1 << test_ctr) & reg_p->mask);
                if (!test_data) {
                    continue;
                }

                /* Write Test Data in */
                if (skye_fpga_i2c_write(reg_p->offset, sizeof(test_data),
                                             (uchar *)&test_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to wrote Ripple 1 pattern(0x%04X)"
                                  " to Skye FPGA Reg %#x.",
                                  __FUNCTION__, test_data, reg_p->offset);
                }

                /* Read the register value back for double check */
                if (skye_fpga_i2c_read(reg_p->offset, sizeof(check_data),
                                            (uchar *)&check_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to read Skye FPGA Reg %#x"
                                  " back after write-in Ripple 1 pattern",
                                  __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple 1 test FAILED, "
                                  "read back = 0x%04x and expected = 0x%04x.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 1 Test */

            /*
             * Ripple 0 test
             */
            for (test_ctr = 0; test_ctr < (sizeof(SR_FPGA_ADDR_SZ) * 8);
                 test_ctr++) {
                test_data = (1 << test_ctr);
                if (!test_data) {
                    continue;
                }

                test_data = ((~(1 << test_ctr)) & reg_p->mask);

                /* Write Test Data in */
                if (skye_fpga_i2c_write(reg_p->offset, sizeof(test_data),
                                             (uchar *)&test_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to wrote Ripple 0 pattern(0x%04X)"
                                  " to Skye FPGA Reg %#x.",
                                  __FUNCTION__, test_data, reg_p->offset);
                }

                /* Read the register value back for double check */
                check_data = 0;
                if (skye_fpga_i2c_read(reg_p->offset, sizeof(check_data),
                                            (uchar *)&check_data) != PASSED) {
                    cterr('f', 0, "%s: Failed to read Skye FPGA Reg %#x"
                                  " back after write-in Ripple 0 pattern",
                                  __FUNCTION__, reg_p->offset);
                    return (FAILED);
                }

                /* Data Comparation */
                if (check_data != test_data) {
                    cterr('f', 0, "%s Reg. Ripple 0 test FAILED, "
                                  "read back = 0x%04x and expected = 0x%04x.",
                                  reg_p->name, check_data, test_data);
                    return (FAILED);
                }
            }   /* End of Ripple 0 Test */

           /* Restore the value before test */
           if (skye_fpga_i2c_write(reg_p->offset, sizeof(ori_val),
                                        (uchar *)&ori_val) != PASSED) {
               cterr('f', 0, "%s: Failed to restore (0x%04X) "
                             "to Skye FPGA Reg %#x.",
                             __FUNCTION__, ori_val, reg_p->offset);
           }
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: szalinski_wdt_test
 *
 * Description: reboot CPU by enableing watchdog timer.
 *              this test will reboot system!!!
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int szalinski_wdt_test (void)
{
    unsigned cpu_no;
    uchar watchdog_time = 0, gpio_expander = 0;
    uint32_t time_out = 0;

    testname("Szalinski Watchdog");
    if (cpu_id == MASTER_CPU) {
        cpu_no = 0;
    } else {
        cpu_no = 1;
    }
    if (cpu_id == MASTER_CPU) {
        prpass(testpass, "CPU0 WDT ");
        /* Reset the WDT */
        watchdog_time = 0;
        if (skye_fpga_i2c_write(CPU0_WDT_REG_OFF, sizeof(watchdog_time),
                                 (uchar *)&watchdog_time) != PASSED) {
            cterr('f', 0, "%s: Failed to wrote CPU0_WDT_REG_OFF(0x%04X)",
                      __FUNCTION__, CPU0_WDT_REG_OFF);
        } 
        /* Enable Primary Interface Ready Pin. */ 
        if (skye_fpga_i2c_read(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                                 (uchar *)&gpio_expander) != PASSED) {
            cterr('f', 0, "%s: Failed to wrote GPIO_EXP_L8_REG_OFF(0x%04X)",
                      __FUNCTION__, GPIO_EXP_L8_REG_OFF);
        } 
        gpio_expander |= PRI_INTF_READY;
        if (skye_fpga_i2c_write(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                                 (uchar *)&gpio_expander) != PASSED) {
            cterr('f', 0, "%s: Failed to wrote GPIO_EXP_L8_REG_OFF(0x%04X)",
                      __FUNCTION__, GPIO_EXP_L8_REG_OFF);
        } 
        if (skye_fpga_i2c_read(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                                 (uchar *)&gpio_expander) != PASSED) {
            cterr('f', 0, "%s: Failed to wrote GPIO_EXP_L8_REG_OFF(0x%04X)",
                      __FUNCTION__,  GPIO_EXP_L8_REG_OFF);
        } 
        if (DIAGFLAG & D_VERBOSE) {
	        printf("\nPrimary Ready Pin values 0x%x...", gpio_expander);   
        }
        if ((gpio_expander & PRI_INTF_READY) != PRI_INTF_READY) {
    	    cterr('f', 0, "Primary Interface Is Not Ready");
        }
        msleep(500);

        /* When Timer is non-zero value indicates the duration is number of 0.1s 
           as the unit.*/
        watchdog_time = 20;
        if (skye_fpga_i2c_write(CPU0_WDT_REG_OFF, sizeof(watchdog_time),
                                 (uchar *)&watchdog_time) != PASSED) {
            cterr('f', 0, "%s: Failed to wrote CPU0_WDT_REG_OFF(0x%04X)",
                      __FUNCTION__, CPU0_WDT_REG_OFF);
        }
        prpass(testpass, "Check Primary Ready Pin ");
        /* Activity GPIO signal to WDT */
        wdt_act(watchdog_time / 10);
        time_out = (watchdog_time / 4);
        while (time_out) {
            if (skye_fpga_i2c_read(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                                     (uchar *)&gpio_expander) != PASSED) {
                cterr('f', 0, "%s: Failed to wrote GPIO_EXP_L8_REG_OFF(0x%04X)",
                          __FUNCTION__, GPIO_EXP_L8_REG_OFF);
            } 
            if (DIAGFLAG & D_VERBOSE) {
	            printf("\nGPIO values 0x%x", gpio_expander);   
            }
            if ((gpio_expander & PRI_INTF_READY) != PRI_INTF_READY) {
                break;
            }
            msleep(500);
            time_out--;
        }
        if (time_out == 0) {
    	    cterr('f', 0, "Primary Interface Is Not to Low");
            return (FAILED);
        }
        if (DIAGFLAG & D_VERBOSE) {
            printf("\nHost Primary Interface Ready Pin to Low \n");
        }
    }
    if (cpu_no == SLAVE_CPU) {
        prpass(testpass, "CPU1 WDT Test");
        /* When Timer is non-zero value indicates the duration is number of 0.1s 
           as the unit.*/
        /* Reset the WDT */
        watchdog_time = 0;
        if (skye_fpga_i2c_write(CPU1_WDT_REG_OFF, sizeof(watchdog_time),
                                 (uchar *)&watchdog_time) != PASSED) {
            cterr('f', 0, "%s: Failed to wrote CPU0_WDT_REG_OFF(0x%04X)",
                      __FUNCTION__, CPU0_WDT_REG_OFF);
        } 
        msleep(500);
        /* Reset the WDT 5 sec. */
        watchdog_time = 50;
        printf("\nCPU1 Watchdog Timer = %d s\n",watchdog_time /10);
            if (skye_fpga_i2c_write(CPU1_WDT_REG_OFF, sizeof(watchdog_time),
                                     (uchar *)&watchdog_time) != PASSED) {
                cterr('f', 0, "%s: Failed to wrote CPU0_WDT_REG_OFF(0x%04X)",
                          __FUNCTION__, CPU0_WDT_REG_OFF);
            }
        /* Activity GPIO signal to WDT */
        watchdog_time = 5;
        wdt_act(watchdog_time); /* Toggle GPIO 5 sec. */
    }

    return (PASSED);
}
/**********************************************************************
 *
 * Function: check_cpu1_wdt
 *
 * Description: When CPU1 WDT assert,CPU0 check reg 0x18 bit0=1��b1
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int check_cpu1_wdt (void)
{
    uchar    wd_int = 0;
    /* Reset Event Comparation */
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(wd_int),
                            (uchar *)&wd_int) != PASSED) {
        printf("\n%s: Failed to read FPGA Interrupt Register(0x%02X).\n",
                __FUNCTION__,INTERRUPT_REG_OFF);
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nInterrupt register 0x18 = 0x%X,", wd_int);   
    }
    /* Data Comparation */
    if ((wd_int & CPU1_WDT_TIMEOUT) == CPU1_WDT_TIMEOUT) {
        return (PASSED);
    }
    return (FAILED);
}

/**********************************************************************
 *
 * Function: szalinski_check_cpu1_wdt
 *
 * Description: When CPU1 WDT assert,CPU0 check reg 0x18 bit0=1��b1
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int szalinski_check_cpu1_wdt (void)
{
    uchar    interrupt_val = 0, tmp = 0, data = 0;
    uint32_t time_out = 0;
    uchar    wd_int = 0;

    /* Disable Interrupt */
    interrupt_val = 0;
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(interrupt_val),
                             (uchar *)&interrupt_val) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    wd_int = 0;
    if (skye_fpga_i2c_write(INTERRUPT_REG_OFF, sizeof(wd_int),
                             (uchar *)&wd_int) != PASSED) {
        cterr('f', 0, "%s Reg. Clear WDT for CPU1 Register FAILED",
                      "Interrupt");
        return (FAILED);
    }
    msleep(500);
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(wd_int),
                            (uchar *)&wd_int) != PASSED) {
        printf("\n%s: Failed to read FPGA Interrupt Register(0x%02X).\n",
                __FUNCTION__,INTERRUPT_REG_OFF);
        return (FAILED);
    }
	printf("\nFPGA Interrupt Register 0x%X = 0x%X\n", INTERRUPT_REG_OFF,wd_int);   
    /* Enable WDT for CPU1*/
    if (skye_fpga_i2c_read(INTERRUPT_EN_REG_OFF, sizeof(interrupt_val),
                            (uchar *)&interrupt_val) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    interrupt_val &= ~(SW_INTRUPT_SIM);
    interrupt_val |= CPU1_WDT_TIMEOUT;
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(interrupt_val),
                             (uchar *)&interrupt_val) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    printf("\nPlease execute CPU1 Szalinski watchdog test:");   
    tmp = getchar();

    /* Interrupt Event Comparation 20 sec. */
    time_out = 20;
    while (time_out) {
        /* Data Comparation */
        if (check_cpu1_wdt() == PASSED) {
            break;
        }
        msleep(1000);
        time_out--;
        printf("%d sec\r",20 - time_out);
        fflush(0);
    }
    if (time_out == 0) {
        cterr('f', 0, "No INT from WDT for CPU1");
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nINT from WDT for CPU1 %d sec", 20 - time_out);   
    }
    /* Disable WDT for CPU1*/
    if (skye_fpga_i2c_read(INTERRUPT_EN_REG_OFF, sizeof(interrupt_val),
                            (uchar *)&interrupt_val) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    interrupt_val &= ~(CPU1_WDT_TIMEOUT);
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(interrupt_val),
                             (uchar *)&interrupt_val) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    printf("\nTest Pass and Reset CPU 1\n");   
    /* Reset CPU1 */
    if (skye_fpga_i2c_read(CPU_RST_CTRL_REG_OFF, sizeof(data),
                                (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, CPU_RST_CTRL_REG_OFF);
        return (FAILED);
    }
    data &= ~CPU1_RST;
    if (skye_fpga_i2c_write(CPU_RST_CTRL_REG_OFF, sizeof(data),
                                (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, CPU_RST_CTRL_REG_OFF);
        return (FAILED);
    }
    msleep(1000);
    data |= CPU1_RST;
    if (skye_fpga_i2c_write(CPU_RST_CTRL_REG_OFF, sizeof(data),
                                (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, CPU_RST_CTRL_REG_OFF);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: szalinski_interrupt_test
 *
 * Description: SW Interrupt simulation test.
 *              SW writes bit7 to simulate or clear interrupt for CPU0
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */

int szalinski_interrupt_test (void)
{
    uchar    wd_int = 0;
    uint64_t val = 0;
    testname("Szalinski SW Interrupt");
    prpass(testpass, "Enable interrupt ");
    /* Step1 : Enable interrupt */
    if (skye_fpga_i2c_read(INTERRUPT_EN_REG_OFF, sizeof(wd_int),
                            (uchar *)&wd_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    wd_int |= INTRUPT_TEST_EN;
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(wd_int),
                             (uchar *)&wd_int) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    prpass(testpass, "Check GPIO Interrupt Pin ");
    /* Step2 : Check CPU0 FPGA interrupt GPIO Pin doesn't assert */
    msleep(500);  /* Waiting for CPU0 boot up*/
    val = get_gxio_gpio();
    if (DIAGFLAG & D_VERBOSE) {
	    printf(" GPIO  values 0x%016lx...\n", val);   
    }
    if ((val & CPU_INT_TO_CPU0_REGISTER) == CPU_INT_TO_CPU0_REGISTER) {
        cterr('f', 0, "CPU GPIO  = 0x%016lx ",val);
        return (FAILED);
    }
    msleep(500);  /* Waiting for CPU0 boot up*/
    prpass(testpass, "Enable SW Interrupt test ");
    /* Step3 : Enable SW Interrupt test */
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(wd_int),
                            (uchar *)&wd_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    wd_int |= SW_INTRUPT_SIM;
    if (skye_fpga_i2c_write(INTERRUPT_REG_OFF, sizeof(wd_int),
                             (uchar *)&wd_int) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    prpass(testpass, "Check GPIO Interrupt Pin ");
    /* Step4 : Check CPU0 FPGA interrupt assert */
    msleep(500);  /* Waiting for CPU0 boot up*/
    val = get_gxio_gpio();
    if (DIAGFLAG & D_VERBOSE) {
	    printf(" GPIO  values 0x%016lx...\n", val);   
    }
    if ((val & CPU_INT_TO_CPU0_REGISTER) != CPU_INT_TO_CPU0_REGISTER) {
        cterr('f', 0, "CPU GPIO  = 0x%016lx ",val);
        return (FAILED);
    }
    msleep(500);  /* Waiting for CPU0 boot up*/

    prpass(testpass, "SW Interrupt Normal operation ");
    /* Step5 : SW Interrupt Normal operation */
    wd_int &= ~(SW_INTRUPT_SIM);
    if (skye_fpga_i2c_write(INTERRUPT_REG_OFF, sizeof(wd_int),
                             (uchar *)&wd_int) != PASSED) {
        cterr('f', 0, "%s Reg. Clear WDT for CPU1 Register FAILED",
                      "Interrupt");
        return (FAILED);
    }
    /* Step6 : Check CPU0 FPGA interrupt doesn't assert */
    msleep(500);  /* Waiting for CPU0 boot up*/
    val = get_gxio_gpio();
    if (DIAGFLAG & D_VERBOSE) {
	    printf(" GPIO  values 0x%016lx...\n", val);   
    }
    if ((val & CPU_INT_TO_CPU0_REGISTER) == CPU_INT_TO_CPU0_REGISTER) {
        cterr('f', 0, "CPU GPIO  = 0x%016lx ",val);
        return (FAILED);
    }
    msleep(500);  /* Waiting for CPU0 boot up*/

    return (PASSED);
}
/**********************************************************************
 *
 * Function: szalinski_thermal_int_test
 *
 * Description: Modify the Temperature Limits to trigger over temperature 
 *              interrupt with register (0x18) 
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */

int szalinski_thermal_int_test (void)
{
    uchar    temp_int = 0, curr_temp = 0, curr_hys = 0x0A, wdata = 0, rdata = 0;
    int      ctr = 0, ret_val = FAILED;
    unsigned cpu_no;

    testname("Szalinski Thermal Interrupt");

    if (cpu_id == MASTER_CPU) {
        cpu_no = 0;
    } else {
        cpu_no = 1;
    }

    /* This is workaround code:
     * We use temporarily to pass Skye P1B 2nd CPU BST I2C intermittent issue.
     * Added 2 times re-try here w/o power cycle module, will keep debug this issue with
     * HW team later.
     */
    if (cpu_id == SLAVE_CPU) {
        for (ctr = 1; ctr <= SKYE_I2C_RETRY_MAX; ctr++) {
            ret_val = FAILED;
            ret_val = skye_on_board_thermal_rd(TS_ID_REG_OFF, (uchar *)&rdata);

            if (ret_val == PASSED) {
                break;
            } else if (ctr == SKYE_I2C_RETRY_MAX) {
                cterr('f', 0, "%s: Failed to read Thermal sensor Reg.(0x%.2X)",
                              __FUNCTION__, TS_ID_REG_OFF);
                return (FAILED);
            }

            printf("\n\n===[Retry %d] CPU%d I2C scan Thermal sensor !!===\n\n",
                   ctr, cpu_id);
            msleep(500);
        }
    }

    /* Setup All limit to High  */
    wdata = 0xff;
    if (skye_on_board_thermal_wr(TS_CPU0_OVER_REG, (uchar *)&wdata) != 
                                      PASSED) {
        printf("%s: Failed to write 0x%0X to Thermal Sensor register"
               "(reg = 0x%02X).\n", __FUNCTION__, wdata, TS_CPU0_OVER_REG);
        return (FAILED);
    }
    if (skye_on_board_thermal_wr(TS_W_CPU0_HIGH_REG, (uchar *)&wdata) != 
                                      PASSED) {
        printf("%s: Failed to write 0x%0X to Thermal Sensor register"
               "(reg = 0x%02X).\n", __FUNCTION__, wdata, TS_W_CPU0_HIGH_REG);
        return (FAILED);
    }
    if (skye_on_board_thermal_wr(TS_W_TEMP_HIGH_REG, (uchar *)&wdata) != 
                                      PASSED) {
        printf("%s: Failed to write 0x%0X to Thermal Sensor register"
               "(reg = 0x%02X).\n", __FUNCTION__, wdata, TS_W_TEMP_HIGH_REG);
        return (FAILED);
    }

    /**** Test Case 1 : CPU Tj interrupt ****/
    prpass(testpass, "CPU Tj interrupt(ALERT_L) ");
    /* Step1 : Enable interrupt */
    if (skye_fpga_i2c_read(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                            (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    if (cpu_no == MASTER_CPU) {
        temp_int = CPU0_PCB_TS_EN;
    } else {
        temp_int = CPU1_AMB_TS_EN;
    }
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                             (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    /* Step2 : Setup CPU1 Tj high limit to low  */
    if (cpu_no == MASTER_CPU) {
        on_board_ts_reg_wr(TS_W_CPU0_HIGH_REG, FALSE);
    } else {
        on_board_ts_reg_wr(TS_W_CPU1_HIGH_REG, FALSE);
    }
    /* Step3 : Check CPU0 Tj over limit  */
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(temp_int),
                            (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nInterrupt Enable(0x%02X) = %x.\n",INTERRUPT_REG_OFF, temp_int);
	    dump_on_board_ts_reg_util(0);  
    }
    /* Data Comparation */
    if (cpu_no == MASTER_CPU) {
        if ((temp_int & CPU0_AMB_OVERTEMP) != CPU0_AMB_OVERTEMP) {
            cterr('f', 0, "Failed CPU0 ambient Interrupt, Register(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    } else {
        if ((temp_int & CPU1_PCB_OVERTEMP) != CPU1_PCB_OVERTEMP) {
            cterr('f', 0, "Failed CPU1 hot-spot Interrupt, Register(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    }    
    /* Step4 : Setup CPU1 Tj high limit to High  */
    if (cpu_no == MASTER_CPU) {
        on_board_ts_reg_wr(TS_W_CPU0_HIGH_REG, TRUE);
    } else {
        on_board_ts_reg_wr(TS_W_CPU1_HIGH_REG, TRUE);
    }
    /* Step5 : Check CPU0 Tj lower than limit  */
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(temp_int),
                            (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nInterrupt Diable(0x%02X) = %x.\n",INTERRUPT_REG_OFF, temp_int);
	    dump_on_board_ts_reg_util(0);  
    }
    /* Data Comparation */
    if (cpu_no == MASTER_CPU) {
        if ((temp_int & CPU0_AMB_OVERTEMP) == CPU0_AMB_OVERTEMP) {
            cterr('f', 0, "Failed CPU0 ambient Interrupt, Reg(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    } else {
        if ((temp_int & CPU1_PCB_OVERTEMP) == CPU1_PCB_OVERTEMP) {
            cterr('f', 0, "Failed CPU1 hot-spot Interrupt, Reg(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    }

    /**** Test Case 2 : CPU Ambient interrupt ****/
    if (skye_on_board_thermal_rd(TS_TEMP_REG, &curr_temp) != PASSED) {
        printf("%s:%d Failed to read on board Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, __LINE__, TS_TEMP_REG);
        return (FAILED);
    }

    if (curr_temp == 0) {
        if (cpu_no == MASTER_CPU) {
            prpass(testpass, "Current Ambient Temp. equals or lower than 0'C, "
                             "so SKIP CPU Ambient interrupt(ALERT_L) ");
        } else {
            prpass(testpass, "Current PCB Hot-spot Temp. equals or lower than 0'C, "
                             "so SKIP CPU Ambient interrupt(ALERT_L) ");
        }
    } else {
        prpass(testpass, "CPU Ambient interrupt(ALERT_L) ");

        /* Step1 : Enable interrupt */
        if (skye_fpga_i2c_read(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                                    (uchar *)&temp_int) != PASSED) {
            cterr('f', 0, "Failed to read Interrupt Enable Register(0x%02X).\n",
                          INTERRUPT_EN_REG_OFF);
            return (FAILED);
        }

        if (cpu_no == MASTER_CPU) {
            temp_int = CPU0_PCB_TS_EN;
        } else {
            temp_int = CPU1_AMB_TS_EN;
        }
        if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                                     (uchar *)&temp_int) != PASSED) {
            cterr('f', 0, "Failed to write Interrupt Enable Register(0x%02X).\n",
                          INTERRUPT_EN_REG_OFF);
            return (FAILED);
        }

        /* Step2 : Setup CPU1 Tj high limit to low  */
        on_board_ts_reg_wr(TS_W_TEMP_HIGH_REG, FALSE);

        /* Step3 : Check CPU0 Ambient over limit  */
        if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(temp_int),
                                    (uchar *)&temp_int) != PASSED) {
            cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                          INTERRUPT_EN_REG_OFF);
            return (FAILED);
        }
        if (DIAGFLAG & D_VERBOSE) {
            printf("\nInterrupt Enable(0x%02X) = %x.\n",INTERRUPT_REG_OFF, temp_int);
	    dump_on_board_ts_reg_util(0);  
        }

        /* Data Comparation */
        if (cpu_no == MASTER_CPU) {
            if ((temp_int & CPU0_AMB_OVERTEMP) != CPU0_AMB_OVERTEMP) {
                cterr('f', 0, "Failed CPU0 ambient Interrupt, Register(0x%02X) = %x.\n",
                              INTERRUPT_REG_OFF, temp_int);
                dump_on_board_ts_reg_util(0);  
                return (FAILED);
            } 
        } else {
            if ((temp_int & CPU1_PCB_OVERTEMP) != CPU1_PCB_OVERTEMP) {
                cterr('f', 0, "Failed CPU1 hot-spot Interrupt, Register(0x%02X) = %x.\n",
                              INTERRUPT_REG_OFF, temp_int);
                dump_on_board_ts_reg_util(0);  
                return (FAILED);
            } 
        }

        /* Step4 : Setup CPU1 Ambient high limit to High  */
        on_board_ts_reg_wr(TS_W_TEMP_HIGH_REG, TRUE);

        /* Step5 : Check CPU0 Ambient lower than limit  */
        if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(temp_int),
                                    (uchar *)&temp_int) != PASSED) {
            cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                          INTERRUPT_EN_REG_OFF);
            return (FAILED);
        }
        if (DIAGFLAG & D_VERBOSE) {
            printf("\nInterrupt Diable(0x%02X) = %x.\n",INTERRUPT_REG_OFF, temp_int);
	    dump_on_board_ts_reg_util(0);  
        }

        /* Data Comparation */
        if (cpu_no == MASTER_CPU) {
            if ((temp_int & CPU0_AMB_OVERTEMP) == CPU0_AMB_OVERTEMP) {
                cterr('f', 0, "Failed CPU0 ambient Interrupt, Reg(0x%02X) = %x.\n",
                              INTERRUPT_REG_OFF, temp_int);
                dump_on_board_ts_reg_util(0);  
                return (FAILED);
            } 
        } else {
            if ((temp_int & CPU1_PCB_OVERTEMP) == CPU1_PCB_OVERTEMP) {
                cterr('f', 0, "Failed CPU1 hot-spot Interrupt, Reg(0x%02X) = %x.\n",
                              INTERRUPT_REG_OFF, temp_int);
                dump_on_board_ts_reg_util(0);  
                return (FAILED);
            } 
        }
    }

    /**** Test Case 3 : CPU Over Temperature interrupt ****/
    prpass(testpass, "CPU Over Temp. interrupt(OVERT_L) ");

    /* 3-1 Set Overtemperature Hysteresis to 0 for testing purpose. */
    if (skye_on_board_thermal_rd(TS_OVER_HYS_REG, &curr_hys) != PASSED) {
        printf("%s:%d Failed to read on board Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, __LINE__, TS_OVER_HYS_REG);
        return (FAILED);
    }

    wdata = TS_TEST_HYS;
    if (skye_on_board_thermal_wr(TS_OVER_HYS_REG, &wdata) != PASSED) {
        printf("%s:%d Failed to set Thermal Sensor register(offset = 0x%02X) to"
               "0x%02X.\n", __FUNCTION__, __LINE__, TS_OVER_HYS_REG, wdata);
        return (FAILED);
    }

    /* Step1 : Enable interrupt */
    if (skye_fpga_i2c_read(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                            (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    if (cpu_no == MASTER_CPU) {
        temp_int = CPU0_TJ_TS_EN;
    } else {
        temp_int = CPU1_TJ_TS_EN;
    }
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                             (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    /* Step2 : Setup CPU1 Tj high limit to low  */
    on_board_ts_reg_wr(TS_CPU0_OVER_REG, FALSE);
    /* Step3 : Check CPU0 Ambient over limit  */
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(temp_int),
                            (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    /* Data Comparation */
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nInterrupt Enable(0x%02X) = %x.\n",INTERRUPT_REG_OFF, temp_int);
	    dump_on_board_ts_reg_util(0);  
    }
    if (cpu_no == MASTER_CPU) {
        if ((temp_int & CPU0_TJ_OVERTEMP) != CPU0_TJ_OVERTEMP) {
            cterr('f', 0, "Failed CPU0 ambient Interrupt, Register(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    } else {
        if ((temp_int & CPU1_TJ_OVERTEMP) != CPU1_TJ_OVERTEMP) {
            cterr('f', 0, "Failed CPU1 hot-spot Interrupt, Register(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    }    
    /* Step4 : Setup CPU1 Ambient high limit to High  */
    on_board_ts_reg_wr(TS_CPU0_OVER_REG, TRUE);
    /* Step5 : Check CPU0 Ambient lower than limit  */
    if (skye_fpga_i2c_read(INTERRUPT_REG_OFF, sizeof(temp_int),
                            (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to read Interrupt Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    /* Data Comparation */
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nInterrupt Diable(0x%02X) = %x.\n",INTERRUPT_REG_OFF, temp_int);
	    dump_on_board_ts_reg_util(0);  
    }
    if (cpu_no == MASTER_CPU) {
        if ((temp_int & CPU0_TJ_OVERTEMP) == CPU0_TJ_OVERTEMP) {
            cterr('f', 0, "Failed CPU0 ambient Interrupt, Reg(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    } else {
        if ((temp_int & CPU1_TJ_OVERTEMP) == CPU1_TJ_OVERTEMP) {
            cterr('f', 0, "Failed CPU1 hot-spot Interrupt, Reg(0x%02X) = %x.\n",
                          INTERRUPT_REG_OFF, temp_int);
            dump_on_board_ts_reg_util(0);  
            return (FAILED);
        } 
    }
    /* Disable All Interrupt */
    temp_int = 0x0;
    if (skye_fpga_i2c_write(INTERRUPT_EN_REG_OFF, sizeof(temp_int),
                             (uchar *)&temp_int) != PASSED) {
        cterr('f', 0, "Failed to write Interrupt Enable Register(0x%02X).\n",
                      INTERRUPT_EN_REG_OFF);
        return (FAILED);
    }
    /* Setup All limit to High  */
    on_board_ts_reg_wr(TS_CPU0_OVER_REG, TRUE);
    on_board_ts_reg_wr(TS_W_CPU0_HIGH_REG, TRUE);
    on_board_ts_reg_wr(TS_W_TEMP_HIGH_REG, TRUE);

    /* Recover Overtemp Hysteresis */
    wdata = curr_hys;
    if (skye_on_board_thermal_wr(TS_OVER_HYS_REG, &wdata) != PASSED) {
        printf("%s:%d Failed to set Thermal Sensor register(offset = 0x%02X) to"
               "0x%02X.\n", __FUNCTION__, __LINE__, TS_OVER_HYS_REG, wdata);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : szalinski_get_led_status
 * Description: Function to show LED status that are controlled by Szalinski.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_get_led_status (void)
{
    uchar    sys_data = 0, eusb_data = 0;
    uint16_t offset = 0;
    uint16_t reg_size = ONE_B_REG;
    char     sys_led_buf[32];

    memset(sys_led_buf, 0, sizeof(sys_led_buf));

    offset = LED_CTRL_REG_OFF;
    if (skye_fpga_i2c_read(offset, reg_size,
                                (uchar *)&sys_data) != PASSED) {
        printf("%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }

    offset = EUSB_CTRL_REG_OFF;
    if (skye_fpga_i2c_read(offset, reg_size,
                                (uchar *)&eusb_data) != PASSED) {
        printf("%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }

    if ((sys_data & SYS_LED_COLOR) == SYS_LED_GREEN) {
        if (sys_data & SYS_LED_BLINK_MSK) {
            sprintf(sys_led_buf, "Blinking in Green");
        } else {
            sprintf(sys_led_buf, "Solid in Green");
        }
    } else if ((sys_data & SYS_LED_COLOR) == SYS_LED_YELLOW) {
        if (sys_data & SYS_LED_BLINK_MSK) {
            sprintf(sys_led_buf, "Blinking in Yellow");
        } else {
            sprintf(sys_led_buf, "Solid in Yellow");
        }
    } else {
        sprintf(sys_led_buf, "OFF");
    }

    printf("Szalinski LED status:\n");
    printf("System LED control Reg.(0x%02X) = 0x%02X\n",
           LED_CTRL_REG_OFF, sys_data);
    printf("  -System LED is %s.\n", sys_led_buf);
    printf("eUSB control Reg.(0x%02X) = 0x%02X\n",
           EUSB_CTRL_REG_OFF, eusb_data);
    printf("  -eUSB LED Test Enable is %s.\n",
           (eusb_data & EUSB_LED_TEST_EN) ? "ON" : "OFF");
    printf("  -eUSB LED is %s.\n",
           (eusb_data & EUSB_LED_TEST) ? "OFF" : "ON");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : szalinski_led_ctrl
 * Description: Function to control LED that are controlled by Szalinski.
 * Inputs     : opt - LED type 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_led_ctrl (int opt)
{
    uchar    data = 0;
    uint16_t offset = 0, reg_size = ONE_B_REG;

    switch (opt) {
    case SYS_LED_GREEN_BLINK:
        offset = LED_CTRL_REG_OFF;
        data = ((SYS_LED_BLINKING << SYS_LED_BLINK_OFF) | SYS_LED_GREEN);
    break;
    case SYS_LED_YELLOW_BLINK:
        offset = LED_CTRL_REG_OFF;
        data = ((SYS_LED_BLINKING << SYS_LED_BLINK_OFF) | SYS_LED_YELLOW);
    break;
    case SYS_LED_GREEN_SOLID:
        offset = LED_CTRL_REG_OFF;
        data = SYS_LED_GREEN;
    break;
    case SYS_LED_YELLOW_SOLID:
        offset = LED_CTRL_REG_OFF;
        data = SYS_LED_YELLOW;
    break;
    case SYS_LED_TURN_OFF:
        offset = LED_CTRL_REG_OFF;
        data = SYS_LED_OFF;
    break;
    case EUSB_LED_TEST_DIS:
        offset = EUSB_CTRL_REG_OFF;
        data = EUSB_PWR_EN;
    break;
    case EUSB_LED_TURN_ON:
        offset = EUSB_CTRL_REG_OFF;
        data = (EUSB_LED_TEST_EN | EUSB_PWR_EN);
	data &= (uchar)(~(EUSB_LED_TEST_OFF));
    break;
    case EUSB_LED_TURN_OFF:
        offset = EUSB_CTRL_REG_OFF;
        data = (EUSB_LED_TEST_EN | EUSB_PWR_EN);
        data |= EUSB_LED_TEST_OFF;
    break;
    default:
        printf("\n%s: Unspported !!!(%d).\n", __FUNCTION__, opt);
        return (FAILED);
    }

    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : sray_dev_reset_util
 * Description: Uility to Reset Skye Device.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
sray_dev_reset_util (int opt)
{
    int      choice = 0;
    uchar    data = 0;
    uint16_t offset = 0;
    uint16_t reg_size = (uint16_t)ONE_B_REG;

    printf("\nSkye Device Reset utility List:\n");

    if (cpu_id == MASTER_CPU) {
        offset = (uint16_t)CPU0_DEV_RST_REG_OFF;

        printf("1. Get value of Device Reset register\n");
        printf("2. CPU0 I2C Mux reset control\n");
        printf("3. External 10G-KR GE Switch Reset\n");
        printf("4. External CPU0 eUSB Reset\n");
        printf("5. External CPU0 USB PHY1 Reset\n");
        printf("6. External CPU0 USB PHY0 Reset\n");
        printf("7. External GE PHY0 Reset\n");
        choice = getdec_answer("Please enter your choice: ", 1, 1, 7);
    
        if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
            printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
                   __FUNCTION__, offset);
            return (FAILED);
        }
        switch (choice) {
        case 1:
            printf("\nDevice Reset Reg.(0x%02X) = 0x%02X.\n", offset, data);
            return (PASSED);
        case 2:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << CPU0_I2C_MUX_RST_OFF);
            } else {
                data &= ~(BIT_0 << CPU0_I2C_MUX_RST_OFF);
            }
        break;
        case 3:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << EXT_10GKR_RST_OFF);
            } else {
                data &= ~(BIT_0 << EXT_10GKR_RST_OFF);
            }
        break;
        case 4:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << EXT_CPU0_EUSB_RST_OFF);
            } else {
                data &= ~(BIT_0 << EXT_CPU0_EUSB_RST_OFF);
            }
        break;
        case 5:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << EXT_CPU0_USB1_RST_OFF);
            } else {
                data &= ~(BIT_0 << EXT_CPU0_USB1_RST_OFF);
            }
        break;
        case 6:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << EXT_CPU0_USB0_RST_OFF);
            } else {
                data &= ~(BIT_0 << EXT_CPU0_USB0_RST_OFF);
            }
        break;
        case 7:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0);
            } else {
                data &= ~(BIT_0);
            }
        break;
        default:
            printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
            return (FAILED);
        }
        printf("\nDevice Reset Reg.(0x%02X) = 0x%02X.\n", offset, data);
        if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
            printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
                   __FUNCTION__, data, offset);
            return (FAILED);
        }
    } else {
        offset = (uint16_t)CPU1_DEV_RST_REG_OFF;

        printf("1. Get value of Device Reset register\n");
        printf("2. CPU1 I2C Mux reset control\n");
        printf("3. External CPU1 USB PHY0 Reset\n");
        printf("4. External GE PHY1 Reset\n");
        choice = getdec_answer("Please enter your choice: ", 1, 1, 4);
    
        if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
            printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
                   __FUNCTION__, offset);
            return (FAILED);
        }
        switch (choice) {
        case 1:
            printf("\nDevice Reset Reg.(0x%02X) = 0x%02X.\n", offset, data);
            return (PASSED);
        case 2:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << CPU1_I2C_MUX_RST_OFF);
            } else {
                data &= ~(BIT_0 << CPU1_I2C_MUX_RST_OFF);
            }
        break;
        case 3:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0 << EXT_CPU1_USB0_RST_OFF);
            } else {
                data &= ~(BIT_0 << EXT_CPU1_USB0_RST_OFF);
            }
        break;
        case 4:
            choice = getdec_answer("0=Reset ; 1=Normal Operation ", 1, 0, 1);
            if (choice == 1) {
                data |= (BIT_0);
            } else {
                data &= ~(BIT_0);
            }
        break;
        default:
            printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
            return (FAILED);
        }
        printf("\nDevice Reset Reg.(0x%02X) = 0x%02X.\n", offset, data);
        if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
            printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
                   __FUNCTION__, data, offset);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : sray_cpu_reset_util
 * Description: Uility to Reset Skye CPUs.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
sray_cpu_reset_util (int opt)
{
    int      choice = 0;
    uchar    data = 0;
    uint16_t offset = (uint16_t)CPU_RST_CTRL_REG_OFF;
    uint16_t reg_size = (uint16_t)ONE_B_REG;

    printf("\nSkye CPU Reset utility List:\n");
    printf("1. Get value of CPU Reset register\n");
    printf("2. CPU1 Reset\n");
    printf("3. CPU1 Un-Reset\n");
    printf("4. CPU0 Reset\n");
    choice = getdec_answer("Please enter your choice: ", 1, 1, 4);

    if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }

    switch (choice) {
    case 1:
        printf("\nCPU Reset Reg.(0x%02X) = 0x%02X.\n", offset, data);
        return (PASSED);
    case 2:
        if ((data & CPU1_RST) == 0) {
            printf("\n%s: CPU1 already under RESET mode.\n", __FUNCTION__);
            return (PASSED);
        }
        data &= ~CPU1_RST;
    break;
    case 3:
        data |= CPU1_RST;
    break;
    case 4:
        if ((data & CPU0_RST) == 0) {
            printf("\n%s: CPU0 already under RESET mode.\n", __FUNCTION__);
            return (PASSED);
        }
        data &= ~CPU0_RST;
    break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }

    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : sys_pwr_save_util
 * Description: Uility for system power saving.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
sys_pwr_save_util (int opt)
{
    int      choice = 0;
    uchar    data = 0;
    uint16_t offset = (uint16_t)SYS_PWR_SAV_REG_OFF;
    uint16_t reg_size = (uint16_t)ONE_B_REG;

    printf("\nSystem power saving utility List:\n");
    printf("1. Get value of system power saving register\n");
    printf("2. Put PSE2 in Suspend mode\n");
    printf("3. Put PSE2 in Normal mode\n");
    printf("4. Put 10G-KR channel B in Normal mode\n");
    printf("5. Put 10G-KR channel B in Power Down mode\n");
    printf("6. Put 10G-KR channel A in Normal mode\n");
    printf("7. Put 10G-KR channel A in Power Down mode\n");
    choice = getdec_answer("Please enter your choice: ", 1, 1, 7);

    if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }

    switch (choice) {
    case 1:
        printf("\nSystem Power Saving Reg.(0x%02X) = 0x%02X.\n",
               offset, data);
        return (PASSED);
    case 2:
        if (data & PSE2_SUSPEND) {
            printf("\nPSE2 already in Suspend mode, no need to change.\n");
            return (PASSED);
        }
        data |= PSE2_SUSPEND;
    break;
    case 3:
        if ((data & PSE2_SUSPEND) == 0) {
            printf("\nPSE2 already in Normal mode, no need to change.\n");
            return (PASSED);
        }
        data &= ~PSE2_SUSPEND;
    break;
    case 4:
        if (data & PWR_MODE_10GKR_B) {
            printf("\n10G-KR channel B already in Normal mode, no need to change.\n");
            return (PASSED);
        }
        data |= PWR_MODE_10GKR_B;
    break;
    case 5:
        if ((data & PWR_MODE_10GKR_B) == 0) {
            printf("\n10G-KR channel B already in Power Down mode, no need to change.\n");
            return (PASSED);
        }
        data &= ~PWR_MODE_10GKR_B;
    break;
    case 6:
        if (data & PWR_MODE_10GKR_A) {
            printf("\n10G-KR channel A already in Normal mode, no need to change.\n");
            return (PASSED);
        }
        data |= PWR_MODE_10GKR_A;
    break;
    case 7:
        if ((data & PWR_MODE_10GKR_A) == 0) {
            printf("\n10G-KR channel A already in Power Down mode, no need to change.\n");
            return (PASSED);
        }
        data &= ~PWR_MODE_10GKR_A;
    break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }
    
    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_wdt_util
 * Description: Uility to set WatchDog Timer.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
set_wdt_util (int opt)
{
    int      cpu_num = 0;
    uchar    data = 0;
    uint16_t offset = 0, reg_size = ONE_B_REG;

    cpu_num = getdec_answer("Enter number of CPU WatchDog Timer "
                            "you want to setup: ", 1, 0, 1);
    data = (uchar)gethex_answer("Enter the Time you want to set(Unit: 0.1sec) ",
                                0x0, 0x0, 0xFF);

    if (cpu_num == SKYE_CPU0) {
        offset = CPU0_WDT_REG_OFF; 
    } else {
        offset = CPU1_WDT_REG_OFF; 
    } 

    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : cpu0_spi_mux_util
 * Description: Uility to set CPU0 SPU PROM Mux.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
cpu0_spi_mux_util (int opt)
{
    int      choice = 0;
    uchar    data = 0;
    uint16_t offset = (uint16_t)CPU0_SPI_MUX_REG_OFF;
    uint16_t reg_size = (uint16_t)ONE_B_REG;

    printf("\nCPU0 SPI Mux Control utility List:\n");
    printf("1. Dump CPU0 SPI Mux connect status\n");
    printf("2. Connect to Bitstream config. ROM\n");
    printf("3. Connect to CPU0 SPI Flash\n");
    choice = getdec_answer("Please enter your choice: ", 1, 1, 3);

    if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }

    switch (choice) {
    case 1:
        printf("\nCPU0 SPI Mux Control Reg.(0x%02X) = 0x%02X.\n",
               offset, data);
        printf(" -Connect to %s.\n",
               (data & CPU0_SPI_MUX_SEL)? "Bitstream conf. ROM": "CPU0 SPI Flash");
        return (PASSED);
    case 2:
        if (data & CPU0_SPI_MUX_SEL) {
            printf("\nAlready connect to Bitstream conf. ROM, no need to change.\n");
            return (PASSED);
        }
        data |= TO_BS_CFG_ROM;
    break;
    case 3:
        if ((data & CPU0_SPI_MUX_SEL) == 0) {
            printf("\nAlready connect to CPU0 SPI Flash, no need to change.\n");
            return (PASSED);
        }
        data &= (uint16_t)(~TO_BS_CFG_ROM);
    break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }

    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : cpu0_usb_ctrl_util
 * Description: Uility to set CPU0 USB Mode.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
usb_mode_ctrl_util (int opt)
{
    int      choice = 0;
    uchar    data = 0;
    uint16_t offset = (uint16_t)USB_MOD_CTRL_REG_OFF;
    uint16_t reg_size = (uint16_t)ONE_B_REG;

    printf("\nUSB Mode Control utility List:\n");
    printf("1. Dump USB status\n");
    printf("2. Set CPU0 USB0 to Host Mode\n");
    printf("3. Set CPU0 USB0 to Device Mode\n");
    choice = getdec_answer("Please enter your choice: ", 1, 1, 3);

    if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }

    switch (choice) {
    case 1:
        printf("\nUSB Mode Control Reg.(0x%02X) = 0x%02X.\n", offset, data);
        printf(" -USB0 Power status: %s.\n",
               (data & USB0_PWR_STAT)? "OK": "Fault");
        printf(" -CPU0 USB0 in %s Mode.\n",
               (data & CPU0_USB0_MOD_CTRL)? "Host": "Device");
        return (PASSED);
    case 2:
        if (data & CPU0_USB0_MOD_CTRL) {
            printf("\nCPU0 USB0 is already in Host mode, no need to change.\n");
            return (PASSED);
        }
        data |= USB0_HOST_MOD; 
    break;
    case 3:
        if ((data & CPU0_USB0_MOD_CTRL) == 0) {
            printf("\nCPU0 USB0 is already in Device mode, no need to change.\n");
            return (PASSED);
        }
        data &= (uint16_t)(~CPU0_USB0_MOD_CTRL);
        data |= USB0_DEV_MOD;
    break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }

    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : sr_spirom_read
 * Description: Szalinski SPIROM Read API.
 * Inputs     : srom_no - SPI ROM device no.
 *            : offset  - data offset.
 *            : nbytes  - data number.
 *            : buf   - data value.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sr_spirom_read (unsigned int srom_no,
                 unsigned int offset,
                 unsigned int nbytes,
                 unsigned char *buf)
{
    char devname[32];
    int  fd = -1;

    memset(devname, 0, 32);
    snprintf(devname, sizeof (devname), "/dev/srom/%d", srom_no);

    fd = open(devname, O_RDONLY);
    if (fd < 0) {
        printf("can't open /dev/srom/%d", srom_no);
        return (FAILED);
    }
    
    if (lseek(fd, offset, SEEK_SET) < 0) {
        printf("can't find offset %d", offset);
        close(fd);
        return (FAILED);
    }

    if (read(fd, buf, nbytes) != nbytes) {
        printf("can't read %d bytes", nbytes);
        close(fd);
        return (FAILED);
    }

    if (close(fd)) {
        printf("can't close /dev/srom/%d", srom_no);
        return (FAILED);
    }

    return PASSED;
}
/*******************************************************************************
 *
 * Function   : sr_spirom_write
 * Description: Szalinski SPIROM Write API.
 * Inputs     : srom_no - SPI ROM device no.
 *            : offset  - data offset.
 *            : nbytes  - data number.
 *            : buf   - data value.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sr_spirom_write (unsigned int srom_no, 
                  unsigned int offset,
                  unsigned int nbytes,
                  unsigned char *buf) 
{
    char devname[32];
    int  fd = -1;

    memset(devname, 0, 32);
    snprintf(devname, sizeof (devname), "/dev/srom/%d", srom_no);

    fd = open(devname, O_RDWR);
    if (fd < 0) {
        printf("can't open /dev/srom/%d", srom_no);
        return (FAILED);
    }
    
    if (lseek(fd, offset, SEEK_SET) < 0) {
        printf("can't find offset %d", offset);
        return (FAILED);
    }

    if (write(fd, buf, nbytes) != nbytes) {
        printf("can't write %d bytes", nbytes);
        return (FAILED);
    }

    if (close(fd)) {
        printf("can't close /dev/srom/%d", srom_no);
        return (FAILED);
    }

    return PASSED;
}

/*******************************************************************************
 *
 * Function   : szalinski_spirom_read
 * Description: Function to read Szalinski SPI ROM.
 * Inputs     : srom_no - SPI ROM device no.
 *            : offset  - data offset.
 *            : nbytes  - data number.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int szalinski_spirom_read (unsigned int srom_no,
                      unsigned int offset,
                      unsigned int nbytes)
{
    unsigned char buf[4096];
    int i, status;

    if (nbytes > 4096 || nbytes < 1) {
        printf("Invalid read size, value range 1-4096 bytes\n");
        return (FAILED);
    }

    memset(buf, 0, 4096);
    status = sr_spirom_read(srom_no, offset, nbytes, (unsigned char *)&buf);

    if (status == FAILED) {
        printf("Failed to SPIROM read (%d).\n:", status);
        return (FAILED);
    } else {
        for (i = 0; i < nbytes; i++) {
            if ((i % 16) == 0)
                printf("\n0X%6X  ",offset + i );
                printf("%2X ", buf[i]);
        }
    } 
    printf("\n");
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : szalinski_spirom_write
 * Description: Function to write Szalinski SPI ROM.
 * Inputs     : srom_no - SPI ROM device no.
 *            : offset  - data offset.
 *            : nbytes  - data number.
 *            : value   - data value.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int szalinski_spirom_write (unsigned int srom_no,
                       unsigned int offset,
                       unsigned int nbytes,
                       unsigned int value)
{
    unsigned char buf[4096];
    int status;

    if (nbytes > 4096 || nbytes < 1) {
        printf("Invalid write size, value range 1-4096 bytes\n");
        return (FAILED);
    }

    if (value > 255 ) {
        printf("Invalid value to write, value range 0x00-0xFF\n");
        return (FAILED);
    }

    memset(buf, value, nbytes);
    status = sr_spirom_write(srom_no, offset, nbytes, buf);

    if (status == FAILED) {
        printf("Failed to SPIROM Write (%d).\n:", status);
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/*******************************************************************************
 *
 * Function   : szalinski_spirom_util
 * Description: Uility to R/W SPIROM.
 * Input      : None.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
szalinski_spirom_util (void)
{
    int      choice = 0, retval;
    uchar    data, save_data = 0;
    uint16_t offset = (uint16_t)CPU0_SPI_MUX_REG_OFF;
    uint16_t reg_size = (uint16_t)ONE_B_REG;
    unsigned int srom_no = 3, nbytes, value;
    uint32_t spi_offset;   
    printf("\nSzalinski SPIROM utility List:\n");
    printf("1. Read SPIROM Data.\n");
    printf("2. Write SPIROM Data.\n");
    printf("3. Exit.\n");
    choice = getdec_answer("Please enter your choice: ", 3, 1, 3);

    if (choice == 3) {
        printf("\nJust Exit based on user request.\n");
        return (PASSED);
    }

    if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }
    save_data = data;

    if ((data & CPU0_SPI_MUX_SEL) == TO_BS_CFG_ROM) {
        printf("\nAlready connect to Bitstream conf. ROM, no need to change.\n");
    } else {
        printf("\nSet CPU0 SPI Mux connect to Bitstream conf. ROM... ");

        data |= TO_BS_CFG_ROM;

        if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
            printf("\n%s: Failed to set CPU0 SPI Mux to connect to "
                   "Bitstream conf. ROM(by wrote 0x%02X to Szalinski Reg. 0x%02X).\n",
                   __FUNCTION__, data, offset);
            return (FAILED);
        }
        printf("Done.\n");
    }

    switch (choice) {
    case 1:
        spi_offset = gethex_answer("Please enter starting offset(in Hex): ",
                                   0, 0, 0xffffff);
        nbytes = getdec_answer("Please enter read bytes(in Dec): ", 1, 1, 4096);
        retval = szalinski_spirom_read(srom_no, spi_offset, nbytes);
        break;
    case 2:
        spi_offset = gethex_answer("Please enter starting offset(in Hex): ",
                                   0, 0, 0xffffff);
        nbytes = 1;
	value = gethex_answer("\nEnter write data:", 0, 0, 0xff);
        retval = szalinski_spirom_write(srom_no, spi_offset, nbytes, value);
        msleep(100);
        retval = szalinski_spirom_read(srom_no, spi_offset, nbytes);
        break;
    default:
        printf("\n%s: Invalid choice (%d).\n", __FUNCTION__, choice);
        return (FAILED);
    }

    /* Save back original mode */
    data = save_data;

    if ((data & CPU0_SPI_MUX_SEL) != TO_BS_CFG_ROM) {
        printf("\nSet CPU0 SPI Mux back connect to SPI Flash... ");

        data |= TO_BS_CFG_ROM;
        if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
            printf("\n%s: Failed to set CPU0 SPI Mux back connect to SPI Flash"
                   "(by wrote 0x%02X to Szalinski Reg. 0x%02X).\n",
                   __FUNCTION__, data, offset);
            return (FAILED);
        }
        printf("Done.\n");
    }
    return (retval);
}


/**********************************************************************
 *
 * Function: util_szalinski_image_upgrade
 *
 * Description: Program Szalinski image to SPI Flash.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
util_szalinski_image_upgrade (void)
{
    unsigned int srom_no = 3;
    unsigned char buf[4096];
    uint32_t spi_offset;
    uint8_t *fpga_code;
    uint32_t fpga_code_size, i, j, leng;
    uint8_t ans;
    extern int szalinski_fw_fw_size;
    extern unsigned char szalinski_fw_fw[];
    uint16_t offset = (uint16_t)CPU0_SPI_MUX_REG_OFF;
    uint16_t reg_size = (uint16_t)ONE_B_REG;
    uchar    data, save_data = 0;

    printf("\n\n");
    testname("Szalinski FPGA Image Upgrade");
    printf("\n\nProceed with Image Upgrade? (y/n) ");
    ans = getchar();
    putchar(ans);
	printf("\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nImage Upgrade ABORT! FPGA Image is not been Updated.\n\n");
        return (PASSED);
    }
    /* Switch SPI Mux to SPI ROM  */
    if (skye_fpga_i2c_read(offset, reg_size, (uchar *)&data) != PASSED) {
        printf("\n%s: Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, offset);
        return (FAILED);
    }
    save_data = data;
    if (data & CPU0_SPI_MUX_SEL) {
        printf("\nAlready connect to Bitstream conf. ROM, no need to change.\n");
    }
    data |= TO_BS_CFG_ROM;
    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }

    fpga_code_size = (uint32_t)szalinski_fw_fw_size;
    fpga_code = (uint8_t *)szalinski_fw_fw;

    printf("\nProgram Pages:\n");
    spi_offset = SZALINSKI_UPGRADE_IMAGE_BASE;
    for (i = 0; i < fpga_code_size; i += leng) {
        if ((fpga_code_size - i) > SZALINSKI_FLASH_MAX_LEN) {
            leng = SZALINSKI_FLASH_MAX_LEN;
        } else {
            leng = fpga_code_size - i;
        }
        if (!(i % 0x2000)) {
            prpass(0, "Programming addr %#.6x  ", spi_offset + i);
        }
        if (sr_spirom_write(srom_no, spi_offset + i, leng, (fpga_code + i))) {
            cterr('f',0,"Upgrade image: prog page fail, program addr= %#x\n",
                        (spi_offset + i));
            return (FAILED);
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            for (j = 0; j < leng; j++) {
                if ((j % 16) == 0)
                    printf("\n0X%6X  ",spi_offset + i + j);
                printf("%2X ", *(fpga_code + i));
            }
        }
    } 
    printf("\nVerify Data:\n");
    for (i = 0; i < fpga_code_size; i += leng) {
        if ((fpga_code_size - i) > SZALINSKI_FLASH_MAX_LEN) {
            leng = SZALINSKI_FLASH_MAX_LEN;
        } else {
            leng = fpga_code_size - i;
        }
        if (!(i % 0x2000)) {
            prpass(0, "Verify addr %#.6x  ", spi_offset + i);
        }
        if (sr_spirom_read(srom_no, spi_offset + i, leng, (unsigned char *)&buf)) {
            cterr('f',0,"Upgrade image: prog page fail, program addr= %#x\n",
                        (spi_offset + i));
            return (FAILED);
        }
        if (diagflag_xram & D_DEBUG_OPTIONS) {
            for (j = 0; j < leng; j++) {
                if ((j % 16) == 0)
                    printf("\n0X%6X  ",spi_offset + i + j);
                printf("%2X ", buf[j]);
            }
        }
        for (j = 0; j < leng; j++) {
            if (buf[j] != *(fpga_code + i + j)) {
                cterr('f', 0, "data mismatch(0x%x), write= %#x, read= %#x\n",
					(i + j), *(fpga_code + i + j), buf[j]);
                return (FAILED);
            }
        }
    } 
    /* Save back original mode */
    data = save_data;
    if (skye_fpga_i2c_write(offset, reg_size, (uchar*)&data) != PASSED) {
        printf("\n%s: Failed to write 0x%02X to Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, data, offset);
        return (FAILED);
    }

    printf("\nSzalinski Firmware Upgrade Done.\n");
    printf("Please Power Cycle Module To Active Upgraded Image .\n");
    return (PASSED);
}

/**********************************************************************
 *
 * Function: check_10gcap
 *
 * Description: Check Host support 10GKR capability.
 *
 * Input : none
 *
 * Output: TRUE = Support 10GKR
 *         FALSE = No Support 10GKR
 *
 **********************************************************************
 */
boolean
check_10gcap (void)
{
    uchar gpio_expander = 0;

    if (skye_fpga_i2c_read(GPIO_EXP_H8_REG_OFF, sizeof(gpio_expander),
                             (uchar *)&gpio_expander) != PASSED) {
        cterr('f', 0, "%s: Failed to read GPIO_EXP_H8_REG_OFF(0x%04X)",
                  __FUNCTION__,  GPIO_EXP_H8_REG_OFF);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("\nHost support 10GKR 0x%x...", gpio_expander);
    }
    if ((gpio_expander & 0x1) == 0x1) {
        return (TRUE);
    }

    return (FALSE);
}


/**********************************************************************
 *
 * Function: check_non10gcap_
 *
 * Description: Check Host non 10GKR capability.
 *
 * Input : none
 *
 * Output: TRUE = No Support 10GKR
 *         FALSE = Support 10GKR
 *
 **********************************************************************
 */
boolean
check_non10gcap (void)
{
    boolean rv = !check_10gcap();

    return (rv);
}


/*******************************************************************************
 *
 * Function    : fpga_do_all_wrapper
 * Description : Wrapper for fpga do all test items
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int
fpga_do_all_wrapper (void)
{
    int rc = PASSED;
    int cpu = CPU0;
    int test_item = FALSE;

    if (cpu == cpu_id) {
        test_item = TRUE;
    }

    if (test_item == TRUE) {
        if (szalinski_interrupt_test() == FAILED) {
            return (FAILED);
        }
    }

    if (szalinski_thermal_int_test() == FAILED) {
        return (FAILED);
    }

    if (test_item == TRUE) {
        if (szalinski_reg_test() == FAILED) {
            return (FAILED);
        }
    }

    if (szalinski_wdt_test() == FAILED) {
        return (FAILED);
    }

    return (rc);
}


/*******************************************************************************
 *
 * Function    : skye_is_ready
 * Description : check skye is ready and set the bit 3
 * Inputs      : none
 * Outputs     : none
 *
 *******************************************************************************
 */
void skye_is_ready (void)
{
    uchar gpio_expander = 0;

    if (cpu_id == MASTER_CPU) {
        /* Enable Primary Interface Ready Pin. */
        if (skye_fpga_i2c_read(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                             (uchar *)&gpio_expander) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read GPIO_EXP_L8_REG_OFF(0x%04X)",
                  __FUNCTION__, __LINE__, GPIO_EXP_L8_REG_OFF);
        }

        gpio_expander |= PRI_INTF_READY;
        if (skye_fpga_i2c_write(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                             (uchar *)&gpio_expander) != PASSED) {
            cterr('f', 0, "%s:%d Failed to wrote GPIO_EXP_L8_REG_OFF(0x%04X)",
                  __FUNCTION__, __LINE__, GPIO_EXP_L8_REG_OFF);
        }

        gpio_expander = 0;
        if (skye_fpga_i2c_read(GPIO_EXP_L8_REG_OFF, sizeof(gpio_expander),
                             (uchar *)&gpio_expander) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read GPIO_EXP_L8_REG_OFF(0x%04X)",
                  __FUNCTION__, __LINE__, GPIO_EXP_L8_REG_OFF);
        }

        if (DIAGFLAG & D_VERBOSE) {
            printf("\nPrimary Ready Pin values 0x%x...", gpio_expander);
        }
        if ((gpio_expander & PRI_INTF_READY) != PRI_INTF_READY) {
            cterr('f', 0, "Primary Interface Is Not Ready");
        }
        msleep(500);
    }
}

/*******************************************************************************
 *
 * Function   : skye_fpga_set_spimux
 * Description: Function to set Skye SPI Mux.
 * Inputs     : conn_to - the device that SPI Mux will connect to
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int skye_fpga_set_spimux (uchar conn_to)
{
    uchar    set_data = 0, read_back = 0;
    uint16_t r_off = (uint16_t)CPU0_SPI_MUX_REG_OFF;
    uint16_t r_size = (uint16_t)ONE_B_REG;

    if (skye_fpga_i2c_read(r_off, r_size, (uchar *)&set_data) != PASSED) {
        printf("%s:%d Failed to read Szalinski Reg. 0x%02X.\n",
               __FUNCTION__, __LINE__, r_off);
        return (FAILED);
    }

    if ((set_data & (uchar)CPU0_SPI_MUX_SEL) != (uchar)conn_to) {
        set_data &= (uchar)CPU0_SPI_MUX_SEL;
        set_data |= conn_to;

        if (skye_fpga_i2c_write(r_off, r_size, (uchar*)&set_data) != PASSED) {
            printf("%s:%d Failed to write Szalinski Reg. 0x%02X to 0x%02X.\n",
                   __FUNCTION__, __LINE__, r_off, set_data);
            return (FAILED);
        }

        if (skye_fpga_i2c_read(r_off, r_size, (uchar *)&read_back) != PASSED) {
            printf("%s:%d Failed to read Szalinski Reg. 0x%02X.\n",
                   __FUNCTION__, __LINE__, r_off);
            return (FAILED);
        }
        if (read_back != set_data) {
            printf("%s:%d Failed to set Szalinski Reg. 0x%02X.\n",
                   __FUNCTION__, __LINE__, r_off);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_srom_status_get
 * Description: Function to get Skye SPI flash status register value.
 * Inputs     : srom_conn - SPI flash connect to(for Skye, CPU/FPGA)
 *              rback - buffer to put read back data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int skye_srom_status_get (int srom_conn, char *rback)
{
    int   fd = -1, read_sz = TWO_BYTES, read_data = 0;
    char  devname[40], rbuf[3];
    uchar spimux_opt = 0;
    char  spimux_optstr[16];

    /* 1. For Skye, need to set SPI Mux to connect to CPU/FPGA SPIROM first. */
    memset(spimux_optstr, 0, sizeof(spimux_optstr));

    switch (srom_conn) {
    case SKYE_CPU_SROM:
        spimux_opt = (uchar)TO_CPU0_SPI_FLASH;
        snprintf(spimux_optstr, sizeof(spimux_optstr), "CPU");
    break;
    case SKYE_FPGA_SROM:
        spimux_opt = (uchar)TO_BS_CFG_ROM;
        snprintf(spimux_optstr, sizeof(spimux_optstr), "FPGA");
    break;
    default:
        printf("\n%s: Unspported !!!(%d).\n", __FUNCTION__, srom_conn);
        return (FAILED);
    }

    if (skye_fpga_set_spimux(spimux_opt) != PASSED) {
        printf("%s:%d Failed to set Skye SPI mux connect to %s SPIROM.\n",
               __FUNCTION__, __LINE__, spimux_optstr);
        return (FAILED);
    }

    /* 2. Get SPIROM status register value. */
    memset(rbuf, 0, sizeof(rbuf));
    memset(devname, 0, sizeof(devname));
    snprintf(devname,
             sizeof(devname),
             "/sys/class/srom/%d/status_register", srom_conn);

    fd = open(devname, O_RDONLY);
    if (fd < 0) {
        printf("%s:%d Can't open %s: %s.\n",
               __FUNCTION__, __LINE__, devname, strerror(errno));
        return (FAILED);
    }

    if (read(fd, (void *)rbuf, read_sz) != read_sz) {
        printf("%s:%d Can't read %s: %s.\n",
               __FUNCTION__, __LINE__, devname, strerror(errno));

        if (close(fd) < 0) {
            printf("%s:%d Can't close %s: %s.\n",
                   __FUNCTION__, __LINE__, devname, strerror(errno));
        }
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d %s = %s.\n",
               __FUNCTION__, __LINE__, devname, rbuf);
    }

    read_data = atoi(rbuf);
    *rback = ((read_data/10*16) + (read_data%10));
    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d %s = 0x%02X.\n",
               __FUNCTION__, __LINE__, devname, *rback);
    }

    if (close(fd) < 0) {
        printf("%s:%d Can't close %s: %s.\n",
               __FUNCTION__, __LINE__, devname, strerror(errno));
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_srom_status_set
 * Description: Function to set Skye SPI flash status register value.
 * Inputs     : srom_conn - SPI flash connect to(for Skye, CPU/FPGA)
 *              set_data - data that wants to set to status register
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int skye_srom_status_set (int srom_conn, char set_data)
{
    char  devname[40], wr_cmd[52];
    uchar spimux_opt = 0;
    char  spimux_optstr[16];

    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d set_data = 0x%02X.\n", __FUNCTION__, __LINE__, set_data);
    }

    /* 1. For Skye, need to set SPI Mux to connect to CPU/FPGA SPIROM first. */
    memset(spimux_optstr, 0, sizeof(spimux_optstr));

    switch (srom_conn) {
    case SKYE_CPU_SROM:
        spimux_opt = (uchar)TO_CPU0_SPI_FLASH;
        snprintf(spimux_optstr, sizeof(spimux_optstr), "CPU");
    break;
    case SKYE_FPGA_SROM:
        spimux_opt = (uchar)TO_BS_CFG_ROM;
        snprintf(spimux_optstr, sizeof(spimux_optstr), "FPGA");
    break;
    default:
        printf("\n%s: Unspported !!!(%d).\n", __FUNCTION__, srom_conn);
        return (FAILED);
    }

    if (skye_fpga_set_spimux(spimux_opt) != PASSED) {
        printf("%s:%d Failed to set Skye SPI mux connect to %s SPIROM.\n",
               __FUNCTION__, __LINE__, spimux_optstr);
        return (FAILED);
    }

    /* 2. Set SPIROM status register. */
    memset(wr_cmd, 0, sizeof(wr_cmd));
    memset(devname, 0, sizeof(devname));
    snprintf(devname,
             sizeof(devname),
             "/sys/class/srom/%d/status_register", srom_conn);

    snprintf(wr_cmd, sizeof(wr_cmd), "echo \"0x%02X\" > %s", set_data, devname);
    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d wr_cmd = \"%s\".\n", __FUNCTION__, __LINE__, wr_cmd);
    }
    system(wr_cmd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_fpga_golden_prot
 * Description: Function to enable/disable Skye FPGA golden sectors protection.
 * Inputs     : prot_opt - to enable/disable protection
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int skye_fpga_golden_prot (int prot_opt)
{
    char reg_val = 0, prot_val = (char)LOCK_FPGA_SROM_GLD, check_val = 0;

    if (prot_opt == DISABLE) {
        prot_val = (char)UNLOCK_FPGA_SROM_GLD;
    }

    /* 1. Set SPI flash status register */
    /* 1-1 Get current status register value from SPI flash */
    if (skye_srom_status_get(SKYE_FPGA_SROM, &reg_val) != PASSED) {
        printf("%s:%d Failed to get FPGA SROM status register value.\n",
               __FUNCTION__, __LINE__);
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d curr_val = 0x%02X.\n", __FUNCTION__, __LINE__, reg_val);
    }

    /* 1-2 Set SPI flash status reg. block protect bits if needed */
    if ((reg_val & SROM_STATUS_BP_MSK) != prot_val) {
        /* Set SPI flash status register */
        if (skye_srom_status_set(SKYE_FPGA_SROM, prot_val) != PASSED) {
            printf("%s:%d Failed to set FPGA SROM status register.\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
        msleep(200);

        /* Read SPI flash status register again for confirm */
        if (skye_srom_status_get(SKYE_FPGA_SROM, &check_val) != PASSED) {
            printf("%s:%d Failed to get FPGA SROM status reg. for confirm.\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
        if (DIAGFLAG & D_VERBOSE) {
             printf("%s:%d check_val = 0x%02X.\n",
                    __FUNCTION__, __LINE__, check_val);
        }

        if ((check_val & SROM_STATUS_BP_MSK) != prot_val) {
            printf("%s:%d Protection setup is not what we want:\n",
                   __FUNCTION__, __LINE__);
            printf("current is 0x%02X, and wanted is 0x%02X.\n",
                   (check_val & SROM_STATUS_BP_MSK), prot_val);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_srom_prot_check
 * Description: Function to check if Skye SPIROM is protected by address.
 * Inputs     : srom_num - SPIROM that want to check
 *              test_addr - memory address that want to know if it's protected
 *              result - buffer to put protection check result
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int skye_srom_prot_check (int srom_num, int test_addr, int *result)
{
    unsigned char orig_val = 0, test_val = 0x11, confirm_val = 0x2;

    /* Read Original value */
    if (sr_spirom_read((unsigned int)srom_num,
                       (unsigned int)test_addr,
                       (unsigned int)ONE_BYTE,
                       &orig_val) != PASSED) {
        printf("%s:%d Failed to read /srom/%d\n",
               __FUNCTION__, __LINE__, srom_num);
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("orig_val = 0x%02X.\n", orig_val);
    }

    /* Write testing value in */
    if (sr_spirom_write((unsigned int)srom_num, 
                        (unsigned int)test_addr,
                        (unsigned int)ONE_BYTE,
                        &test_val) != PASSED) {
        printf("%s:%d Failed to write /srom/%d\n",
               __FUNCTION__, __LINE__, srom_num);
        return (FAILED);
    }

    /* Read again for confirm */
    if (sr_spirom_read((unsigned int)srom_num,
                       (unsigned int)test_addr,
                       (unsigned int)ONE_BYTE,
                       &confirm_val) != PASSED) {
        printf("%s:%d Failed to read /srom/%d\n",
               __FUNCTION__, __LINE__, srom_num);
        return (FAILED);
    }
    if (DIAGFLAG & D_VERBOSE) {
        printf("confirm_val = 0x%02X.\n", confirm_val);
    }

    /* Check block is protected or not */
    if (confirm_val == orig_val) {
        *result = ENABLE;
    } else {
        *result = DISABLE;

        /* Recover data back to original */
        if (sr_spirom_write((unsigned int)srom_num, 
                            (unsigned int)test_addr,
                            (unsigned int)ONE_BYTE,
                            &orig_val) != PASSED) {
            printf("%s:%d Failed to write /srom/%d\n",
                   __FUNCTION__, __LINE__, srom_num);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_fpga_srom_prot_check
 * Description: Function to check Skye FPGA SPIROM block protections.
 * Inputs     : prot_opt - Enable/Disable Skye FPGA SPIROM Golden protection
 *              test_opt - Enable/Disable Test mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int skye_fpga_srom_prot_check (int prot_opt, int test_opt)
{
    int test_addr = 0, prot_val = 0;

    /* Set prot_val default value */
    if (prot_opt == ENABLE) {
        prot_val = DISABLE;
    } else {
        prot_val = ENABLE;
    }

    /* Check FPGA SPIROM Golden protection */
    test_addr = (int)SZALINSKI_GOLDEN_IMAGE_BASE;
    if (skye_srom_prot_check((int)SKYE_FPGA_SROM,
                             test_addr,
                             &prot_val) != PASSED) {

        if (test_opt == ENABLE) {
            printf("Failed to check FPGA Golden sectors protection.\n");
        } else {
            printf("Skye FPGA Golden sectors: Unknown protection state.\n");
            goto SKYE_FPGA_UGD_PROT_CHK;
        }
        return (FAILED);
    }

    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d prot_val = 0x%02X; and opt = 0x%02X.\n",
               __FUNCTION__, __LINE__, prot_val, prot_opt);
    }

    if (test_opt == DISABLE) {
        printf("Skye FPGA Golden sectors: %s.\n",
               (prot_val == ENABLE)?"Locked":"Unlocked");
    } else {
        /* Test mode, check if current protect mode is expected. */
        if (prot_val != prot_opt) {
            printf("Failed !!! Skye FPGA Upgrade sectors are NOT %s.\n",
                   (prot_opt == ENABLE)?"Locked":"Unlocked");
            return (FAILED);
        }
    }

SKYE_FPGA_UGD_PROT_CHK:
    /* Check FPGA SPIROM Upgrade protection */
    /* For Skye, FPGA SPIROM Upgrade sectors are always Unlocked. */
    test_addr = (int)SZALINSKI_UPGRADE_IMAGE_BASE;
    prot_val = ENABLE;
    if (skye_srom_prot_check((int)SKYE_FPGA_SROM,
                             test_addr,
                             &prot_val) != PASSED) {

        if (test_opt == ENABLE) {
            printf("Failed to check FPGA Upgrade sectors protection.\n");
        } else {
            printf("Skye FPGA Upgrade sectors: Unknown protection state.\n");
        }
        return (FAILED);
    }

    if (DIAGFLAG & D_VERBOSE) {
        printf("%s:%d prot_val = 0x%02X.\n", __FUNCTION__, __LINE__, prot_val);
    }

    if (test_opt == DISABLE) {
        printf("Skye FPGA Upgrade sectors: %s.\n",
               (prot_val == ENABLE)?"Locked":"Unlocked");
    } else {
        if (prot_val != DISABLE) {
            printf("Failed !!! Skye FPGA Upgrade sectors are Locked.\n");
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_fpga_srom_prot_get
 * Description: Function to get Skye FPGA SPI flash block protection setups.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int util_fpga_srom_prot_get (void)
{
    char curr_val = 0;

    /* Get Skye FPGA SPI flash status register value */
    if (skye_srom_status_get(SKYE_FPGA_SROM, &curr_val) != PASSED) {
        return (FAILED);
    }
    printf("Current FPGA SPI flash status register = 0x%02X.\n", curr_val);

    /* Get current Skye FPGA SPI flash protection state */
    if (skye_fpga_srom_prot_check(DISABLE, DISABLE) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_prot_skye_fpga_gld
 * Description: Utility to enable/disable Skye FPGA golden sectors protection.
 * Inputs     : prot_opt - to enable/disable protection
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int util_prot_skye_fpga_gld (int prot_opt)
{
    /* 1. Check if Skye FPGA Golden is protected,
     *    or try to Enable Skye FPGA Golden protection if needed.
     */
    if (skye_fpga_golden_prot(prot_opt) != PASSED) {
        printf("\nFailed to %s Skye FPGA Golden sectors.\n", 
               (prot_opt == ENABLE)?"Lock":"Unlock");
        return (FAILED);
    }

    /* 2. Check if Skye FPGA Golden and Upgrade block protection
     *    are set correctly.
     */
    if (skye_fpga_srom_prot_check(prot_opt, ENABLE) != PASSED) {
        printf("\n***Failed to %s Skye FPGA Golden sectors.\n", 
               (prot_opt == ENABLE)?"Lock":"Unlock");
        return (FAILED);
    }

    printf("\nSkye FPGA Golden sectors are %s.\n", 
           (prot_opt == ENABLE)?"Locked":"Unlocked");
    return (PASSED);
}


/*----------------------------------------------------
$Log: szalinski_diag.c,v $
Revision 1.2  2015/05/25 03:59:17  steja
Add Support Skye SM

Revision 1.1.4.6  2015/05/23 19:06:25  palin2
Add utilites to lock/unlock Skye FPGA SPI flash.

Revision 1.1.4.5  2015/05/11 13:45:46  steja
Code clean up <CSCuu14285>

Revision 1.1.4.4  2015/04/30 03:01:43  palin2
code clean up.

Revision 1.1.4.3  2015/04/30 01:57:11  palin2
Picked up HW Szalinski FW upgrade to 15/04/14/12, and this is for pilot FA.

Revision 1.1.4.2  2015/04/29 11:36:37  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.17  2015/02/13 05:34:44  palin2
Updated Szalinski SPIROM utilities.

Revision 1.1.2.16  2015/01/20 00:49:23  palin2
Temporarily added retry when CPU1 of 2-CPUs I2C access thermal sensor.

Revision 1.1.2.15  2015/01/13 09:44:17  palin2
Corrected error message in function "skye_is_ready" and make it more clear.

Revision 1.1.2.14  2014/12/31 09:25:58  steja
Remove debug info and remove unused code

Revision 1.1.2.13  2014/12/12 00:50:39  steja
Check in for tune clock buffer debugging.

Revision 1.1.2.12  2014/11/10 09:48:02  steja
Update Clock buffer utility

Revision 1.1.2.11  2014/10/14 06:31:10  steja
Fix appropriate return failed for do all test.

Revision 1.1.2.10  2014/09/29 18:36:22  palin2
Added utility to get CPU GPIO pin state.

Revision 1.1.2.9  2014/09/23 07:03:15  steja
Update code for checking Primary Interface Ready (GPIO3)

Revision 1.1.2.8  2014/09/18 07:22:26  palin2
Updated enhanced error message - debugging steps.

Revision 1.1.2.7  2014/09/17 04:35:08  palin2
Updated Skye enhanced error message.

Revision 1.1.2.6  2014/09/12 14:38:43  steja
Update code for CPU do all test

Revision 1.1.2.5  2014/08/28 02:54:26  steja
Support Do all test for NC command

Revision 1.1.2.4  2014/08/27 11:20:44  palin2
Update szalinski utilities for Skye.

Revision 1.1.2.3  2014/08/13 11:51:49  steja
Add 10GKR determine function

Revision 1.1.2.2  2014/08/08 08:34:33  steja
Add Do all test

Revision 1.1.2.1  2014/07/21 01:56:56  palin2
Initial check-in Skye module side Diag code.

------------------------------------------------------
szalinski_diag.c:
Revision 1.2.8.3  2014/07/09 02:21:09  palin2
Support I2C scan test for Shrinkray.

Revision 1.2.8.2  2014/06/27 09:40:22  palin2
Fixed Shrinkray CPU ambient thermal sensor interrupt test Failed(CSCup56001).

Revision 1.2.8.1  2014/06/06 12:06:11  steja
(CSCup23725)Fix Shrinkray LED utility not working properly

Revision 1.2  2014/02/27 15:01:46  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.25  2014/02/13 09:31:13  palin2
Update code based on review comments.

Revision 1.1.4.24  2014/01/13 17:19:41  steja
Add Clock Buffer utility Read and Write to Basic Utilities

Revision 1.1.4.23  2014/01/13 07:00:53  iachang
Support FPGA Device Reset utility

Revision 1.1.4.22  2014/01/13 03:25:41  iachang
CSCum50313 : CPU0 thermal interrupt test

Revision 1.1.4.21  2013/12/31 05:32:52  iachang
CSCum32877 : Fixed CPU1 WatchDog test issue

Revision 1.1.4.20  2013/12/25 09:11:21  steja
Fix the clock buffer programming issue

Revision 1.1.4.19  2013/12/25 08:54:59  iachang
Check CPU1 WDT timeout reg. during toggle GPIO pin

Revision 1.1.4.18  2013/12/25 02:33:06  iachang
Toggle GPIO to CPU1 WDT test

Revision 1.1.4.17  2013/12/18 05:03:11  steja
1. support PSE2 backplane loopback test
2. support BIB change MAC address utility

Revision 1.1.4.16  2013/12/11 01:48:24  iachang
Szalinski FPGA Multi-boot/Secure-Boot/SPI Control Registers dump

Revision 1.1.4.15  2013/12/10 15:26:22  iachang
WDT Activity GPIO signal to CPU0
Support CPU0/1 Szalinski WDT Test
Szalinski FPGA Flash Image verify

Revision 1.1.4.14  2013/11/22 08:48:15  iachang
Support Szalinski FW verify utility.

Revision 1.1.4.13  2013/11/18 14:34:05  iachang
Support CPU1 Szalinski watchdog test

Revision 1.1.4.12  2013/11/18 03:28:14  iachang
Support CPU0 Szalinski watchdog test.

Revision 1.1.4.11  2013/11/07 06:46:39  iachang
Support Szalinski interrupt test.

Revision 1.1.4.10  2013/11/01 05:52:29  iachang
Modify CPU Reset utility

Revision 1.1.4.9  2013/10/31 11:42:20  iachang
Support CPU1 un-reset item

Revision 1.1.4.8  2013/10/30 11:38:31  iachang
Support Szalinski firmware upgrade (Ver 2013-10-29-22)

Revision 1.1.4.7  2013/10/30 10:49:20  iachang
Modify CPU GPIO initial

Revision 1.1.4.6  2013/10/30 03:08:33  palin2
1. Add Szalinski register write utility support.
2. Change some Szalinski utilities' item order.

Revision 1.1.4.5  2013/10/28 06:09:39  palin2
1. Add setup I2C Mux suport in ShrinkRay I2C read/write utilities.
2. Add utilities to read & dump Szalinski registers.
3. Update Szalinski show version utility.

Revision 1.1.4.4  2013/09/27 09:32:15  palin2
Updated eUSB LED test control utility.

Revision 1.1.4.3  2013/09/18 03:02:06  palin2
Update code based on code review comments.

Revision 1.1.4.2  2013/09/13 07:00:10  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.8  2013/08/14 11:36:09  palin2
1. Add ShrinkRay GE LED utility High-level code.
2. Initial check-in "skye_led.c" for ShrinkRay LED related test and utilities.
3. Move Szalinski LED utility to "skye_led.c".

Revision 1.1.2.7  2013/07/30 11:52:44  palin2
Add "FPGA(Szalinski) Test" item into ShrinkRay SM side main tests.

Revision 1.1.2.6  2013/07/30 08:17:59  palin2
Add "Frequnency Margin utility".

Revision 1.1.2.5  2013/07/29 13:32:26  palin2
Add "WatchDog Timer", "CPU0 SPI Mux control", and "USB mode control" utilty support.

Revision 1.1.2.4  2013/07/29 10:50:12  palin2
Add "CPU Reset", "Szalinski LED", and "System power saving" utility support.

Revision 1.1.2.3  2013/07/26 10:14:44  iachang
Add Watch Doag Test
Support CPU GPIO initial and CPU ID detect

Revision 1.1.2.2  2013/07/23 14:49:36  palin2
Revised "szalinski_show_ver" utility based on updated FPGA bit definition.

Revision 1.1.2.1  2013/07/15 21:55:01  palin2
Initial check-in for ShrinkRay FPGA(Szalinski) Diag test and utility.

------------------------------------------------------
$Endlog$
*/

