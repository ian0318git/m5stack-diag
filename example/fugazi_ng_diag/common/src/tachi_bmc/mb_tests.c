/* $Id: mb_tests.c,v 1.4 2017/03/30 08:30:54 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * Jan 2015, Hsuan-Ming Yang adapted from Overload.
 *
 * Copyright (c) 2015 - 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
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
#include "mon_plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "diag_blk_test.h"
#include "diag_i2c_test.h"
#include "diag_fpga_test.h"
#include "diag_mcu_test.h"
#include "diag_gephy_test.h"
#include "diag_geswitch_test.h"
#include "diag_temp_sensor_test.h"
#include "diag_fan_test.h"
#include "diag_rtc_test.h"
#include "diag_aux_test.h"
#include "diag_peci_test.h"
#include "daughtercard_test.h"
#include "diag_temp_sensor_util.h"
#include "diag_fpga_lib.h"
#include "diag_temp_sensor_lib.h"

/* M/B test flag defines */
#define MF_1	(MF_CONTINUOUS | MF_DOGRP)
#define MF_2	(MF_1 | MF_DOALL)
#define MF_3	(MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4	(MF_1 | MF_SHOW_ERRCOUNT)

/*
 * Global variables
 */
fru_table_t platform_fru_table[];
uchar sku_id[32];


/*
 * Global extern functions
 */
extern void display_uart_regs_cterr_wrapper(void);
extern int display_multiboot(int);
extern int get_mb_pid(char *);
extern type_t wic_iface_test(void);

/* #define BYPASS_ENV  * */
extern int  linux_memory_tester(int);
extern int test_not_avail_yet (int);
extern int do_all_menu_items(struct menuinfo *);

extern int check_menu_flag(uint);
extern boolean is_tachi_high(void);

int mb_peci_test(void);
int mb_rtc_test(boolean);
int mb_fan_test(boolean);
int mb_temp_sensor_test(boolean);
int mb_i2c_scan_test(void);
int mb_ncsi_gephy_test(boolean);
int mb_mcu_test(boolean);
int mb_fpga_test(boolean);
int mb_nand_flash_test(void);
int mb_bios_flash_test(void);
int mb_spi_flash_test(void);
int mb_memory_test(int);
int mb_sgpio_test(void);
void display_env(void); 

/* FRU PID and Location Strings */
uchar mb_loc[] = "MB";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar mb_memory_loc[] = "MB/memory";
uchar mb_spi_flash_loc[] = "MB/spi-flash";
uchar mb_bios_flash_loc[] = "MB/bios-flash";
uchar mb_nand_loc[] = "MB/nand";
uchar mb_fpga_loc[] = "MB/fpga";
uchar mb_mcu_loc[] = "MB/mcu";
uchar mb_gephy_loc[] = "88E1512";
uchar mb_ncsi_gephy_loc[] = "MB/ncsi-gephy";
uchar mb_geswitch_loc[] = "88E6320";
uchar mb_i2c_loc[] = "MB/i2c";
uchar mb_temp_sensor_loc[] = "MB/TEMP-SENSOR";
uchar mb_fan_loc[] = "MB/FAN";
uchar mb_rtc_loc[] = "MB/RTC";
uchar mb_peci_loc[] = "MB/PECI";
uchar mb_sgpio_loc[] = "MB/SGPIO";
uchar gesw_98DX[] = "MB-GESW-98DX";
uchar gesw_98DX_loc[] = "MB/GESW-98DX";
uchar gesw_98DX_i2c[] = "MB-GESW-98DX-I2C";
uchar gesw_98DX_i2c_loc[] = "MB/GESW-98DX/I2C";
uchar intel_i2c[] = "INTEL-i2c";
uchar intel_i2c_loc[] = "INTEL/i2c";
uchar intel_cpu[] = "INTEL-cpu";
uchar intel_cpu_loc[] = "INTEL/cpu";
uchar intel_mem[] = "INTEL-memory";
uchar intel_mem_loc[] = "INTEL/memory";
uchar intel_hdd[] = "INTEL-hdd";
uchar intel_hdd_loc[] = "INTEL/SATA/hdd";
uchar intel_usb[] = "INTEL-usb";
uchar intel_usb_loc[] = "INTEL/usb";
uchar intel_ssd[] = "INTEL-ssd";
uchar intel_ssd_loc[] = "INTEL/SATA/ssd";
uchar intel_emmc[] = "INTEL-emmc";
uchar intel_emmc_loc[] = "INTEL/usb/emmc";
uchar intel_bmcusb0[] = "INTEL-bmcusb0";
uchar intel_bmcusb0_loc[] = "INTEL/bmcusb0";
uchar intel_bmcusb1[] = "INTEL-bmcusb1";
uchar intel_bmcusb1_loc[] = "INTEL/bmcusb1";
uchar intel_i350[] = "INTEL-i350";
uchar intel_i350_loc[] = "INTEL/pcie/i350";
uchar intel_x710[] = "INTEL-x710";
uchar intel_x710_loc[] = "INTEL/pcie/x710";
uchar intel_i210[] = "INTEL-i210";
uchar intel_i210_loc[] = "INTEL/pcie/i210";
uchar intel_core[] = "INTEL-core";
uchar intel_core_loc[] = "INTEL";
uchar intel_pcie[] = "INTEL-pcie";
uchar intel_pcie_loc[] = "INTEL/pcie";
uchar intel_tpm20[] = "INTEL-tpm20";
uchar intel_tpm20_loc[] = "INTEL/SPI/tpm20";
uchar isp_test[] = "INTEL-isp-x710";
uchar isp_test_loc[] = "INTEL/10GKR/isp-x710";
uchar isp_test_uart[] = "ISP-UART";
uchar isp_test_uart_loc[] = "MB/ISP/UART";
uchar isp_test_sgmii[] = "ISP-SGMII";
uchar isp_test_sgmii_loc[] = "MB/ISP/SGMII";
uchar isp_raid[] = "INTEL-isp-RAID";
uchar isp_raid_loc[] = "INTEL/10GKR/SAS/SATA/HDD";
uchar isp_crypto[] = "INTEL-isp-crypto";
uchar isp_crypto_loc[] = "INTEL/10GKR/CRYPTO";
uchar nim_10gkr[] = "NIM-10G-KR-testcard";
uchar nim_10gkr_loc[] = "NIM/10GKR/TESTCARD";

uchar isp_raid_pca9557[] = "RAID/9557";
uchar isp_raid_pca9557_loc[] = "RAID-9557";
uchar isp_raid_5m570[] = "RAID/5m570";
uchar isp_raid_5m570_loc[] = "RAID-5m570";
uchar isp_raid_sgpio[] = "RAID/SGPIO";
uchar isp_raid_sgpio_loc[] = "RAID-SGPIO";
uchar isp_raid_vdd[] = "RAID/vdd";
uchar isp_raid_vdd_loc[] = "RAID-vdd";
uchar isp_raid_sbr[] = "RAID/sbr";
uchar isp_raid_sbr_loc[] = "RAID-sbr";


fru_table_t platform_fru_table[] = {
    { sku_id,    mb_loc },
    { sku_id,    dimm0_loc },
    { sku_id,    dimm1_loc },
    { sku_id,    mb_memory_loc},
    { sku_id,    mb_spi_flash_loc},
    { sku_id,    mb_bios_flash_loc},
    { sku_id,    mb_nand_loc},
    { sku_id,    mb_fpga_loc},
    { sku_id,    mb_mcu_loc},
    { sku_id,    mb_gephy_loc},
    { sku_id,    mb_ncsi_gephy_loc},
    { sku_id,    mb_geswitch_loc},
    { sku_id,    mb_i2c_loc},
    { sku_id,    mb_temp_sensor_loc},
    { sku_id,    mb_fan_loc},
    { sku_id,    mb_rtc_loc},
    { sku_id,    mb_peci_loc},
    { sku_id,    mb_sgpio_loc},
    { gesw_98DX,    gesw_98DX_loc},
    { gesw_98DX_i2c,    gesw_98DX_i2c_loc},
    { intel_i2c,     intel_i2c_loc},
    { intel_cpu,     intel_cpu_loc},
    { intel_mem,     intel_mem_loc},
    { intel_hdd,     intel_hdd_loc},
    { intel_usb,     intel_usb_loc},
    { intel_ssd,     intel_ssd_loc},
    { intel_emmc,    intel_emmc_loc},
    { intel_bmcusb0,    intel_bmcusb0_loc},
    { intel_bmcusb1,    intel_bmcusb1_loc},
    { intel_i350,    intel_i350_loc},
    { intel_x710,    intel_x710_loc},
    { intel_i210,    intel_i210_loc},
    { intel_core,    intel_core_loc},
    { intel_pcie,    intel_pcie_loc},
    { intel_tpm20,    intel_tpm20_loc},
    { isp_test,      isp_test_loc},
    { isp_test_uart,      isp_test_uart_loc},
    { isp_test_sgmii,      isp_test_sgmii_loc},
    { isp_raid,      isp_raid_loc},
    { isp_crypto,    isp_crypto_loc},
    { nim_10gkr,     nim_10gkr_loc},
    { isp_raid_pca9557,    isp_raid_pca9557_loc},
    { isp_raid_5m570,    isp_raid_5m570_loc},
    { isp_raid_sgpio,    isp_raid_sgpio_loc},
    { isp_raid_vdd,    isp_raid_vdd_loc},
    { isp_raid_sbr,    isp_raid_sbr_loc},
};
/* 
 * Sub Menu used for Motherboard tests.
 */
submenu_xtable_t mb_tests_submenu_table[] = {
    {"Memory Test",
     (PFT)mb_memory_test,	FALSE,		MF_2,
     (type_t(*)())0, 0,		(PFT)mb_memory_test,	TRUE},
    {"SPI Flash test",
     (PFT)mb_spi_flash_test,	0,		MF_3,
     (type_t(*)())0, 0,		(PFT)0,	0},
    {"BIOS Flash test",
     (PFT)mb_bios_flash_test,	0,		MF_3,
     (type_t(*)())0, 0,		(PFT)0,	0},
    {"NAND Flash test",
     (PFT)mb_nand_flash_test,	0,		MF_3,
     (type_t(*)())0, 0,		(PFT)0,	0},
    {"FPGA test",
     (PFT)mb_fpga_test,	    TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)mb_fpga_test,	FALSE},
    {"MCU test",
     (PFT)mb_mcu_test,	    TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)mb_mcu_test,	    FALSE},
    {"Mgmt 88E1512 test",
     (PFT)diag_mgmt_gephy_build_test, TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)diag_mgmt_gephy_build_test,	FALSE},
    {"Ncsi 88E1512 test",
     (PFT)mb_ncsi_gephy_test,TRUE,		MF_3,
     (PFT)is_tachi_high, 0,	(PFT)mb_ncsi_gephy_test,	FALSE},
    {"88E6320 test",
     (PFT)diag_geswitch_build_test,	TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)diag_geswitch_build_test,	FALSE},
    {"I2C Device Scan test",
     (PFT)mb_i2c_scan_test,	0,		MF_3,
     (type_t(*)())0, 0,		(PFT)mb_i2c_scan_test,	0},
    {"Board Temperature Sensor test",
     (PFT)mb_temp_sensor_test,	TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)mb_temp_sensor_test,	FALSE},
    {"FAN test",
     (PFT)mb_fan_test,	TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)mb_fan_test,	FALSE},
    {"RTC test",
     (PFT)mb_rtc_test,	TRUE,		MF_3,
     (type_t(*)())0, 0,		(PFT)mb_rtc_test,	0},
    {"PECI test",
     (PFT)mb_peci_test,	0,		MF_3,
     (type_t(*)())0, 0,		(PFT)0,	0},
    {"SGPIO test",
     (PFT)mb_sgpio_test,	0,		MF_3,
     (type_t(*)())0, 0,		(PFT)0,	0},
};
#define MB_TESTS_SUBMENU_TABLE_SIZE (sizeof(mb_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mb_tests_primary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t mb_tests_secondary_items[MB_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t mb_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mb_tests_primary_items,
};
menuinfo_t *mb_submenup = &mb_subtest_menu;

/*
 * Sub Menu used for I/O Interface tests.
 */

submenu_xtable_t io_tests_submenu_table[] = {
    {"NIM interface test",
     wic_iface_test,            0,      MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0,            0,      (type_t(*)())0, 0},
    {"ISP interface test",
     (PFT)daughtercard_iface_test,            0,      MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0,            0,      (PFT)0, 0},
};
#define IO_TESTS_SUBMENU_TABLE_SIZE \
        (sizeof(io_tests_submenu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t io_tests_primary_items[IO_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];
static mitem_t io_tests_secondary_items[IO_TESTS_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];
menuinfo_t io_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    io_tests_primary_items,
};
menuinfo_t *io_submenup = &io_subtest_menu;


/*-------------------------------------------------------------------
 *
 * Function: mb_tests()
 *
 * First build the primary & secondary submenus for the motherboard
 * diags based on the _xtable_ mb_tests_submenu_table.  If the given
 * arg is TRUE, execute all the tests in the menu flagged with
 * MF_DOALL, and return the result.  Otherwise, present the menu to the
 * user for interaction.
 *
 */
int mb_tests (boolean mb_test_items_executed)
{

    build_primary_submenu(mb_tests_submenu_table, MB_TESTS_SUBMENU_TABLE_SIZE,
			    "Motherboard", &mb_submenup);
    build_secondary_submenu(mb_tests_submenu_table,
			    MB_TESTS_SUBMENU_TABLE_SIZE,
			    mb_tests_secondary_items);
    if (mb_test_items_executed) {
        do_all_menu_items(&mb_subtest_menu);
    } else {
        menu(&mb_subtest_menu, mb_tests_secondary_items, '\0');
    }    

    return (PASSED);
}

/*
 * Function:    io_iface_tests
 * Description: First build the primary & secondary submenus
 *              for the i/o interface diags based on the
 *              _xtable_ io_tests_submenu_table.  If the given
 *              arg is FALSE, execute all the tests in the menu
 *              and return the result.  Otherwise, present the
 *              menu to the user for interaction.
 * Inputs:      io_test_items_executed: TRUE/FALSE
 * Output:      PASS
 */
int
io_iface_tests (int io_test_items_executed)
{
    build_primary_submenu(io_tests_submenu_table,
                          IO_TESTS_SUBMENU_TABLE_SIZE,
                          "I/O Interface", &io_submenup);
    build_secondary_submenu(io_tests_submenu_table,
                            IO_TESTS_SUBMENU_TABLE_SIZE,
                            io_tests_secondary_items);

    if (io_test_items_executed) {
        menu(&io_subtest_menu, io_tests_secondary_items, '\0');
        return PASS;
    } else if (!exec_doall_menu_items(&io_subtest_menu)) {
        /*
         * User did <BREAK>.  Display accumulated errors here only if
         * not a continuous run because display will occur in menu() as
         * a result of <BREAK>.
         */
        if (!(DIAGFLAG & D_CONTINUOUS)) {
            menu_pr_err_accum();
        }
        if (monjmpptr) {
            longjmp(*monjmpptr, 1);  /* Back to previous point */
        }
    }
    return PASS;
}

static void
add_mb_memory_test_err_report(void)
{
    fru_table_offset = MB_MEMORY;
    platform_fru_table[MB_MEMORY].pid_string = sku_id;
    platform_fru_table[MB_MEMORY].location_string = mb_spi_flash_loc;
    cterr_add_component("BMC", "MEMORY");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the interface between BMC and DDR3",
                    "Replace one DDR3 and redo the test");
}
/*
 * Function: mb_memory_test
 *
 * Description : motherboard memory test
 * Inputs: int
 *
 * Output: PASSED/FAILED
 */
int
mb_memory_test(int march_c_test)
{
    char *tname = "MB memory Test";

    if (get_enhance_err_flag()) {
        add_mb_memory_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    
    if (linux_memory_tester(march_c_test) != PASSED) {
        cterr('f', 0, "memory Test failed.");
        return(FAILED);
    }
    
    return(PASSED);
}

static void
add_mb_spi_flash_test_err_report(void)
{
    fru_table_offset = MB_SPI_FLASH;
    platform_fru_table[MB_SPI_FLASH].pid_string = sku_id;
    platform_fru_table[MB_SPI_FLASH].location_string = mb_spi_flash_loc;
    cterr_add_component("BMC", "SPI-FLASH");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Replace SPI flash with golden sample.");
}

/*
 * Function: mb_spi_flash_test
 *
 * Description : motherboard spi flash test
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int
mb_spi_flash_test(void)
{
    msleep(500);
    char *tname = "MB SPI Flash Test";

    if (get_enhance_err_flag()) {
        add_mb_spi_flash_test_err_report();
    }
    
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_spi_flash_test() != PASSED) {
        cterr('f', 0, "SPI Flash Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_bios_flash_test_err_report(void)
{
    fru_table_offset = MB_BIOS_FLASH;
    platform_fru_table[MB_BIOS_FLASH].pid_string = sku_id;
    platform_fru_table[MB_BIOS_FLASH].location_string = mb_bios_flash_loc;
    cterr_add_component("BMC", "BIOS-FLASH");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Run I2C device scan test check SPI MUX is accessible",
                    "Replace the bios flash with golden sample");
}
/*
 * Function: mb_bios_flash_test
 *
 * Description : motherboard bios flash test
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int
mb_bios_flash_test(void)
{
    msleep(1);
    char *tname = "MB BIOS Flash Test";

    if (get_enhance_err_flag()) {
        add_mb_bios_flash_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_bios_flash_test() != PASSED) {
        cterr('f', 0, "BIOS Flash Test failed.");
        return(FAILED);
    }

    return(PASSED);
}


static void
add_mb_nand_test_err_report(void)
{
    fru_table_offset = MB_NAND;
    platform_fru_table[MB_NAND].pid_string = sku_id;
    platform_fru_table[MB_NAND].location_string = mb_nand_loc;
    cterr_add_component("BMC", "NAND-FLASH");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Replace the nand flash with golden sample");
}

/*
 * Function: mb_nand_flash_test
 *
 * Description : motherboard nand flash test
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int
mb_nand_flash_test(void)
{
    msleep(500);
    char *tname = "MB NAND Flash Test";

    if (get_enhance_err_flag()) {
        add_mb_nand_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_nand_flash_test() != PASSED) {
        cterr('f', 0, "NAND Flash Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_fpga_test_err_report(void)
{
    fru_table_offset = MB_FPGA;
    platform_fru_table[MB_FPGA].pid_string = sku_id;
    platform_fru_table[MB_FPGA].location_string = mb_fpga_loc;
    cterr_add_component("BMC", "FPGA");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Probe the interrupt between BMC and FPGA");
}

/*
 * Function: mb_fpga_test
 *
 * Description : motherboard fpga test
 * Inputs: boolean
 *
 * Output: PASSED/FAILED
 */
int
mb_fpga_test(boolean run_all_tests)
{
    char *tname = "MB FPGA Test";

    if (get_enhance_err_flag()) {
         add_mb_fpga_test_err_report();
    }
    
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_fpga_test(run_all_tests) != PASSED) {
        cterr('f', 0, "FPGA Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_mcu_test_err_report(void)
{
    fru_table_offset = MB_MCU;
    platform_fru_table[MB_MCU].pid_string = sku_id;
    platform_fru_table[MB_MCU].location_string = mb_mcu_loc;
    cterr_add_component("BMC", "MCU");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do fpga test first", "Do MCU basic R/W",
                    "Replace the MCU with golden sample");
}
/*
 * Function: mb_mcu_test
 *
 * Description : motherboard mcu test
 * Inputs: boolean
 *
 * Output: PASSED/FAILED
 */
int
mb_mcu_test(boolean run_all_tests)
{
    char *tname = "MB mcu Test";

    if (get_enhance_err_flag()) {
        add_mb_mcu_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_mcu_test(run_all_tests) != PASSED) {
        cterr('f', 0, "MCU Test failed.");
        return(FAILED);
    }

    return(PASSED);
}



static void
add_mb_ncsi_gephy_test_err_report(void)
{
    fru_table_offset = MB_NCSI_GEPHY;
    platform_fru_table[MB_NCSI_GEPHY].pid_string = sku_id;
    platform_fru_table[MB_NCSI_GEPHY].location_string = mb_ncsi_gephy_loc;
    cterr_add_component("BMC", "NCSI_gephy");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Action A","Action B","Action C");
}

/*
 * Function: mb_ncsi_gephy_test
 *
 * Description : motherboard ncsi gephy test
 * Inputs: boolean
 *
 * Output: PASSED/FAILED
 */
int
mb_ncsi_gephy_test(boolean run_all_tests)
{
    char *tname = "MB NCSI gephy Test";

    if (get_enhance_err_flag()) {
        add_mb_ncsi_gephy_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_ncsi_gephy_build_test(run_all_tests) != PASSED) {
        cterr('f', 0, "ncsi gephy Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_i2c_test_err_report(void)
{
    fru_table_offset = MB_I2C;
    platform_fru_table[MB_I2C].pid_string = sku_id;
    platform_fru_table[MB_I2C].location_string = mb_i2c_loc;
    cterr_add_component("BMC", "All I2C <-> BMC");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Ask HW to check the i2c IF");
}

/*
 * Function: mb_i2c_scan_test
 *
 * Description : motherboard i2c scan test
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int
mb_i2c_scan_test(void)
{
    char *tname = "MB I2C scan Test";

    if (get_enhance_err_flag()) {
        add_mb_i2c_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_i2c_scan_test() != PASSED) {
        cterr('f', 0, "I2C Scan Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_temp_sensor_test_err_report(void)
{
    fru_table_offset = MB_TEMP_SENSOR;
    platform_fru_table[MB_TEMP_SENSOR].pid_string = sku_id;
    platform_fru_table[MB_TEMP_SENSOR].location_string = mb_temp_sensor_loc;
    cterr_add_component("BMC", "Temp-Sensor");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do fpga test first",
                    "Do basic temp-sensor R/W",
                    "Ask HW to check this path");
}

/*
 * Function: mb_temp_sensor_test
 *
 * Description : motherboard Temperature Sensor test
 * Inputs: boolean run_all_tests
 *
 * Output: PASSED/FAILED
 */
int
mb_temp_sensor_test(boolean run_all_tests)
{
    char *tname = "MB Temperature Sensor Test";

    if (get_enhance_err_flag()) {
        add_mb_temp_sensor_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_temp_sensor_test(run_all_tests) != PASSED) {
        cterr('f', 0, "temp_sensor Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_fan_test_err_report(void)
{
    fru_table_offset = MB_FAN;
    platform_fru_table[MB_FAN].pid_string = sku_id ;
    platform_fru_table[MB_FAN].location_string = mb_fan_loc;
    cterr_add_component("BMC", "FAN");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do fpga register test");
}

/*
 * Function: mb_fan_test
 *
 * Description : motherboard fan test
 * Inputs: boolean run_all_tests
 *
 * Output: PASSED/FAILED
 */
int
mb_fan_test(boolean run_all_tests)
{
    char *tname = "MB Fan Test";

    if (get_enhance_err_flag()) {
        add_mb_fan_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_fan_test(run_all_tests) != PASSED) {
        cterr('f', 0, "Fan Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void
add_mb_rtc_test_err_report(void)
{
    fru_table_offset = MB_RTC;
    platform_fru_table[MB_RTC].pid_string = sku_id;
    platform_fru_table[MB_RTC].location_string = mb_rtc_loc;
    cterr_add_component("BMC", "RTC");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Run I2C scan test ",
                    "Replace chip with golden sample");
}

/*
 * Function: mb_rtc_test
 *
 * Description : motherboard rtc test
 * Inputs: boolean run_all_tests
 *
 * Output: PASSED/FAILED
 */
int
mb_rtc_test(boolean run_all_tests)
{
    char *tname = "MB RTC Test";

    if (get_enhance_err_flag()) {
        add_mb_rtc_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_rtc_test(run_all_tests) != PASSED) {
        cterr('f', 0, "RTC Test failed.");
        return(FAILED);
    }

    return(PASSED);
}


static void
add_mb_peci_test_err_report(void)
{
    fru_table_offset = MB_PECI;
    platform_fru_table[MB_PECI].pid_string = sku_id;
    platform_fru_table[MB_PECI].location_string = mb_peci_loc;
    cterr_add_component("BMC", "PECI", "INTEL");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Make sure Intel ME is loaded",
                    "Check one-wire PECI IF between BMC and Broadwell-DE");
}

/*
 * Function: mb_peci_test
 *
 * Description : motherboard peci test (mb<peci>intel)
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int
mb_peci_test(void)
{
    char *tname = "MB PECI Test";

    if (get_enhance_err_flag()) {
        add_mb_peci_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_peci_test() != PASSED) {
        cterr('f', 0, "PECI Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

static void add_mb_sgpio_test_err_report(void)
{
    fru_table_offset = MB_SGPIO;
    platform_fru_table[MB_SGPIO].pid_string = sku_id;
    platform_fru_table[MB_SGPIO].location_string = mb_sgpio_loc;
    cterr_add_component("BMC", "SGPIO ITF");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do fpga test first", "Check SGPIO interface");
}

int mb_sgpio_test(void)
{
    char *tname = "MB SGPIO Test";

    if (get_enhance_err_flag()) {
        add_mb_sgpio_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (diag_sgpio_test() != PASSED) {
        cterr('f', 0, "SGPIO Test failed.");
        return(FAILED);
    }

    return(PASSED);
}

void display_env(void) 
{
    int data;
    int ix;
    uint16_t reg_val;

    cterr_db_print("Voltage Margin Status:\n");
    cterr_db_print("1.5V:");
    
    diag_fpga_reg_read(FPGA_VOLT_MARG_REG, &data);
   
    if (((data >> VOLT_1P5_SHIFT) & VOLT_MARG_MASK) == VOLT_MARG_LOW) {
        cterr_db_print("Low\n");
    } else if (((data >> VOLT_1P5_SHIFT) & VOLT_MARG_MASK) == VOLT_MARG_HIGH) {
        cterr_db_print("High\n");
    } else {
        cterr_db_print("Normal\n");
    }
    
    cterr_db_print("3.3V:");
    
    if ((data & VOLT_MARG_MASK) == VOLT_MARG_LOW) {
        cterr_db_print("Low\n");
    } else if ((data & VOLT_MARG_MASK) == VOLT_MARG_HIGH) {
        cterr_db_print("High\n");
    } else {
        cterr_db_print("Normal\n\n");
    }

    cterr_db_print("Temperature Info:\n");
    set_nios_mode(NIOS_DISABLE_MODE);
    
    for (ix = 0; ix < 4; ix++) {
        if (diag_temp_sensor_reg_read(board_temp[ix].addr, TEMP_REG_OFFSET,
            &reg_val) 
            == FAILED) {
            cterr_db_print("%s: Register read failed\n", __func__);
        }

        cterr_db_print("%-20s : ", board_temp[ix].desc);
        if (reg_val <= TS_TEMP_MAX) {
            cterr_db_print("%.4f Celcius\n",
            (reg_val >> 4) * TS_TEMP_RESOLUTION);
        } else {
            cterr_db_print("%.4f Celcius\n",
            ((reg_val >> 4) - 4096) * TS_TEMP_RESOLUTION);
        }
    }
    set_nios_mode(NIOS_DIAG_MODE);

    return;
}

/*---------------------------------------------------------------
$Log: mb_tests.c,v $
Revision 1.4  2017/03/30 08:30:54  hondwang
Tachi-L brach merge

Revision 1.3.10.1  2016/11/04 19:08:54  benchen2
Modify Enhanced error message

Revision 1.3  2016/08/09 07:44:47  hondwang
Add RAID SGPIO testing

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.23  2016/04/20 00:38:00  huanngo
Remove the code for Tachi-H

Revision 1.1.2.22  2016/04/11 14:18:33  hondwang
Add TPM20 testing function

Revision 1.1.2.21  2016/03/10 05:39:05  uid421098
Add ISP test card io test

Revision 1.1.2.20  2016/03/08 08:28:19  benchen2
add raid card enhance error message

Revision 1.1.2.19  2016/03/08 03:07:07  jimmyya
Add ISP testcard uart test

Revision 1.1.2.18  2016/03/07 07:10:06  benchen2
sgpio test

Revision 1.1.2.17  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.16  2016/03/03 09:46:37  jimmyya
add GESW I2C test

Revision 1.1.2.15  2016/02/26 09:00:58  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.14  2016/02/16 23:41:15  jskow
Add enhanced error messaging to Lewis GESW

Revision 1.1.2.13  2016/02/02 07:25:25  benchen2
add enhanced error message

Revision 1.1.2.12  2015/12/25 03:16:31  tirawan
move Marvell Lewis tests entry to top menu

Revision 1.1.2.11  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.10  2015/12/21 22:34:28  jskow
Rename files from lewis to lewis_gesw for clarity.  Implement external loopback flag for external loopback menu item.  Implement port internal loopback test.

Revision 1.1.2.9  2015/12/08 00:47:59  jskow
Add Lewis test file and header, modify mb_tests to include Lewis in menu

Revision 1.1.2.8  2015/11/01 06:17:02  tirawan
Remove Cypress FX3S Test and Aux Loopback Test

Revision 1.1.2.7  2015/10/21 09:38:23  alpeng
add i/o interface entry

Revision 1.1.2.6  2015/10/01 07:24:15  tirawan
Correct M/B Test name

Revision 1.1.2.5  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.4  2015/09/17 02:58:32  benchen2
add BIOS test item

Revision 1.1.2.3  2015/09/15 06:53:56  benchen2
remove bios test item

Revision 1.1.2.2  2015/08/04 03:32:20  meho
Added RTC tests.

Revision 1.1.2.1  2015/06/11 02:01:10  tirawan
Add files for Tachi BMC project


$Endlog$
*/
