/* $Id: diag_raid_test.c,v 1.4 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_raid_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_raid_test.c -
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "nvmonvars.h"
#include "diag_raid_test.h"
#include "diag_raid_lib.h"
#include "diag_raid_util.h"
#include "common_utils.h"
#include "proto.h"
#include "diag_nc_common.h"
#include "intel_tests.h"
#include "linux_api.h"
#include "platform_fru.h"
#include "diag_fpga_lib.h"

int diag_raidcard_build_test(int);
static uint32_t raid_hdd_test(void);
static uint32_t raid_9557_reg_test(void);
static uint32_t raid_sgpio_test(void);
static uint32_t cpld_reg_test(void);
static uint32_t isp_raid_pci_test(void);
static uint32_t vdd_eeprom_test(void);
static uint32_t sbr_eeprom_test(void);
static uint32_t raid_uart_test(void);
int diag_raid_io_test(void);

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/* Sub Menu used for GE PHY Main tests.
 */
static submenu_xtable_t raidcard_main_tests_submenu_table[] = {
    {"HDD Test", (type_t(*)())raid_hdd_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"PCA9557 Register Test", (type_t(*)())raid_9557_reg_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"CPLD 5M570 Register Test", (type_t(*)())cpld_reg_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"ISP RAID PCI bus Test", (type_t(*)())isp_raid_pci_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"VPD eeprom Test", (type_t(*)())vdd_eeprom_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SBR eeprom Test", (type_t(*)())sbr_eeprom_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"UART Test", (type_t(*)())raid_uart_test,   0,
    MF_4, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SGPIO Test", (type_t(*)())raid_sgpio_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Raid Utility",          (type_t(*)())diag_raid_util,     0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define RAIDCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(raidcard_main_tests_submenu_table) / \
                       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t raidcard_main_tests_primary_items[RAIDCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                       MAX_BASE_ITEMS];
static mitem_t raidcard_main_tests_secondary_items[RAIDCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                     MAX_BASE_ITEMS];

menuinfo_t raidcard_main_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    raidcard_main_tests_primary_items,
};
menuinfo_t *raidcard_main_submenup = &raidcard_main_subtest_menu;

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
int diag_raidcard_build_test (int run_all_tests)
{
    build_primary_submenu(raidcard_main_tests_submenu_table,
                          RAIDCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "RAID CARD", &raidcard_main_submenup);
    build_secondary_submenu(raidcard_main_tests_submenu_table,
                            RAIDCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            raidcard_main_tests_secondary_items);

    if (run_all_tests) {
        exec_doall_menu_items(raidcard_main_submenup);
    } else {
        menu(raidcard_main_submenup, raidcard_main_tests_secondary_items, '\0');
    }
    return (PASSED);
}

static uint32_t raid_hdd_test (void)
{
    int rc;
    sighandler_t old_handler;

    testname("Check INTEL linux Ready");
    prpass(testpass, "Check INTEL linux Ready");
    /* Backup SIGNAL before system call  */
    old_handler = signal(SIGCHLD, SIG_DFL);
    if (check_intel_linux_ready()) {
        /* recover system call SIGNAL  */
        signal(SIGCHLD, old_handler);
        cterr('f', 0, "Check INTEL linux ready Failed");
        return (FAILED);
    }
    /* recover system call SIGNAL  */
    signal(SIGCHLD, old_handler);
    prcomplete(testpass, errcount, 0);

    rc = diag_nc_intel_hdd_test();
    return (rc);

}

static uint32_t isp_raid_pci_test (void)
{
    int rc;
    sighandler_t old_handler;

    testname("Check INTEL linux Ready");
    prpass(testpass, "Check INTEL linux Ready");
    /* Backup SIGNAL before system call  */
    old_handler = signal(SIGCHLD, SIG_DFL);
    if (check_intel_linux_ready()) {
        /* recover system call SIGNAL  */
        signal(SIGCHLD, old_handler);
        cterr('f', 0, "Check INTEL linux ready Failed");
        return (FAILED);
    }
    /* recover system call SIGNAL  */
    signal(SIGCHLD, old_handler);
    prcomplete(testpass, errcount, 0);

    rc = diag_intel_isp_raid_pci_if_test();
    return (rc);
}

static void
add_raid_9557_test_err_report(void)
{
    fru_table_offset = ISP_RAID_PCA9557;
    platform_fru_table[ISP_RAID_PCA9557].pid_string = isp_raid_pca9557;
    platform_fru_table[ISP_RAID_PCA9557].location_string = isp_raid_pca9557_loc;
    cterr_add_component("ISP_RAID_PCA9557");
    cterr_add_debug("Test MB FPGA R/W test",
                    "Check i2c interface with HW");
}

static uint32_t raid_9557_reg_test (void)
{
    int retval = PASSED;
    if (get_enhance_err_flag()) {
        add_raid_9557_test_err_report();
    }
    
    testname("PCA9557 Register");
    prpass(testpass, "PCA9557 Register Test");

    if (raid_9557_reg_test_lib() == FAILED ) {
        retval = FAILED;
        cterr('f', 0, "pca9557 reg test fail\n");
    }
    return (retval);
}

static void
add_raid_5m570_test_err_report(void)
{
    fru_table_offset = ISP_RAID_5M570;
    platform_fru_table[ISP_RAID_5M570].pid_string = isp_raid_5m570;
    platform_fru_table[ISP_RAID_5M570].location_string = isp_raid_5m570_loc;
    cterr_add_component("ISP_RAID_5M570");
    cterr_add_debug("Test MB FPGA R/W test",
                    "Check i2c interface with HW");
}

static uint32_t cpld_reg_test (void)
{
    int retval = PASSED;
    if (get_enhance_err_flag()) {
        add_raid_5m570_test_err_report();
    }
    testname("CPLD 5M570 Register");
    prpass(testpass, "CPLD 5M570 Register Test");

    if (raid_cpld_reg_test_lib() == FAILED ) {
        retval = FAILED;
        cterr('f', 0, " cpld 5m570 reg test fail\n");
    }
    return (retval);
}

static void
add_raid_sgpio_test_err_report(void)
{
    fru_table_offset = ISP_RAID_SGPIO;
    platform_fru_table[ISP_RAID_SGPIO].pid_string = isp_raid_sgpio;
    platform_fru_table[ISP_RAID_SGPIO].location_string = isp_raid_sgpio_loc;
    cterr_add_component("ISP_RAID_SGPIO");
    cterr_add_debug("Test MB FPGA R/W test",
    				"INTEL linux has boot up or not",
                    "Check SGPIO interface with HW");
}

static uint32_t raid_sgpio_test (void)
{
    int retval = PASSED;
    unsigned int hdd1reg, hdd2reg;

    if (get_enhance_err_flag()) {
        add_raid_sgpio_test_err_report();
    }
    testname("SGPIO interface");
    prpass(testpass, "SGPIO interface Test");

    /* Check INTEL work */
    if (check_intel_linux_ready()) {
        cterr('f', 0, "Check INTEL linux ready Failed");
        return (FAILED);
    }
    set_nios_mode(NIOS_DIAG_MODE);
    /* enable FPAG link LED SGPIO */
    diag_fpga_reg_write(FPGA_NIOS_SPECIAL_CTRL_REG, ENABLE);

    msleep(RAID_FPGA_SGPIO_LINK_TIME); /* HW suggest 5 second */
    /* To got HDD LED status by SGPIO */
    diag_fpga_reg_read(FPGA_HDD1_STATUS_LED_REG, &hdd1reg);
    diag_fpga_reg_read(FPGA_HDD2_STATUS_LED_REG, &hdd2reg);
    /* Clean Yellow LED setting */
    hdd1reg = hdd1reg & RAID_FPGA_SGPIO_LINK_PASS;
    hdd2reg = hdd2reg & RAID_FPGA_SGPIO_LINK_PASS;
    if ((hdd1reg != RAID_FPGA_SGPIO_LINK_PASS) || (hdd2reg != RAID_FPGA_SGPIO_LINK_PASS)) {
    	retval = FAILED;
        cterr('f', 0, " RAID SGPIO test fail\n");
    }

    /* disable FPAG link LED SGPIO */
    diag_fpga_reg_write(FPGA_NIOS_SPECIAL_CTRL_REG, DISABLE);
    set_nios_mode(NIOS_DISABLE_MODE); 
    return (retval);
}

static void
add_raid_sbr_test_err_report(void)
{
    fru_table_offset = ISP_RAID_SBR;
    platform_fru_table[ISP_RAID_SBR].pid_string = isp_raid_sbr;
    platform_fru_table[ISP_RAID_SBR].location_string = isp_raid_sbr_loc;
    cterr_add_component("ISP_RAID_SBR");
    cterr_add_debug("Check bmc <-> i2c interface with HW");
}
static uint32_t sbr_eeprom_test (void)
{
    int retval = PASSED;
    
    if (get_enhance_err_flag()) {
        add_raid_sbr_test_err_report();
    }

    testname("SBR EEPROM");
    prpass(testpass, "SBR EEPROM Test");
    
    if (raid_sbr_ctrl(SBR_EN_PROGRAM) != PASSED) {
        retval = FAILED;
    }
    
    system("I2cBusScan 5");
    system("I2cBusScan 5 > sbr.txt");

    int ret = system("cat sbr.txt | grep -qI 0xA0");
    if (ret !=  PASSED) {
        retval = FAILED;
        cterr('f', 0, "can't find SBR device"); 
    } 

    system("rm sbr.txt");
    
    if (raid_sbr_ctrl(SBR_DIS_PROGRAM) != PASSED) {
        retval = FAILED;
    }
    return (retval);
}

static void
add_raid_vdd_test_err_report(void)
{
    fru_table_offset = ISP_RAID_VDD;
    platform_fru_table[ISP_RAID_VDD].pid_string = isp_raid_vdd;
    platform_fru_table[ISP_RAID_VDD].location_string = isp_raid_vdd_loc;
    cterr_add_component("ISP_RAID_VPD");
    cterr_add_debug("Check bmc <-> i2c interface with HW");
}
static uint32_t vdd_eeprom_test (void)
{
    int retval = PASSED;
    if (get_enhance_err_flag()) {
        add_raid_vdd_test_err_report();
    }
    testname("VPD EEPROM");
    prpass(testpass, "VPD EEPROM Test");

    if (raid_switch_ctrl(CH200) == FAILED ) {
        retval = FAILED;
    }
        
    system("I2cBusScan 5");
    system("I2cBusScan 5 > vdd.txt");
    int ret = system("cat vdd.txt | grep -qI 0x02");
    if (ret !=  PASSED) {
        retval = FAILED;
        cterr('f', 0, "can't find LSI device"); 
    } 
    
    ret = system("cat vdd.txt | grep -qI 0xA0");
    if (ret !=  PASSED) {
        retval = FAILED;
        cterr('f', 0, "can't find VPD device"); 
    } 

    system("rm vdd.txt");
    return (retval);
}

static uint32_t raid_uart_test (void)
{
    int rv = PASSED;
    testname("RAID UART");
    prpass(testpass, "RAID UART Test");
    
    system("fpgautil write 904 5");
   
    const int maxlen = 28;
    char test_if[maxlen];
    snprintf(test_if, maxlen-1, "/dev/ttyS2");
    if (raid_uart_intf_test(test_if, NULL, B115200) != PASSED) {
        rv = FAILED;
    }
    
    system("fpgautil write 904 0");

    return (rv);
}

int diag_raid_io_test(void)
{
    int retval = PASSED;

    if (raid_9557_reg_test() == FAILED){
        retval = FAILED;
    }

    if (cpld_reg_test() == FAILED){
        retval = FAILED;
    }
    
    return (retval);
}
/*---------------------------------------------------------------
$Log: diag_raid_test.c,v $
Revision 1.4  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.3.10.1  2017/03/16 10:22:28  hondwang
Fix SGPIO issue

Revision 1.3  2016/08/09 07:44:47  hondwang
Add RAID SGPIO testing

Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.9  2016/03/29 07:04:10  benchen2
correct raid card test name

Revision 1.1.2.8  2016/03/26 05:27:53  benchen2
add raid io interface test

Revision 1.1.2.7  2016/03/23 07:04:15  benchen2
change attribute off raid uart test

Revision 1.1.2.6  2016/03/08 08:28:19  benchen2
add raid card enhance error message

Revision 1.1.2.5  2016/03/03 11:25:02  benchen2
add raid uart test

Revision 1.1.2.4  2016/03/02 08:35:41  benchen2
add sbr vdd eeprom ping test

Revision 1.1.2.3  2016/02/26 09:00:22  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.2  2016/01/12 00:29:01  uid259484
modify to add INTEL NC utility show HDD, DIMM and linux version.
And add RAID card and BTB testing to daughter card item.

Revision 1.1.2.1  2015/11/13 07:19:23  benchen2
Add raid card entrance menu

$Endlog$
*/

