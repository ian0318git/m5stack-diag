/* $Id: diag_geswitch_test.c,v 1.5 2019/09/10 01:03:39 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_geswitch_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_geswitch_test.c - GE Switch 88E6320 Test Functions
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "diag_geswitch_test.h"
#include "diag_geswitch_util.h"
#include "diag_smi_lib.h"
#include "defs.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "diag_gephy_test.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_smi_lib.h"
#include "proto.h"
#include <sys/wait.h>
#include "platform_fru.h"
#include "diag_nc_common.h"
#include "mb_tests.h"
#include "queryflags.h"

int geswitch_apply_errata(void);
int diag_geswitch_build_test(int);
int diag_geswitch_i350_eth1_lpbk_test(void);
int geswitch_loopback_enable(int, int);
static int diag_geswitch_test(int);
static int diag_geswitch_reg_test(void);
static int diag_geswitch_int_lpbk_test(void);
static int diag_geswitch_x710_eth1_mac1_lpbk_test(void);
static int diag_smi_6320_read_fn(unsigned long, int, unsigned long *, void *);
static int diag_smi_6320_write_fn(unsigned long, int, unsigned long, void *);
static int check_port5_vlan(void);
static int eth1_tx_rx_only_test();
static int eth1_mac1_tx_rx_only_test();
static int diag_geswitch_i350_datapath_test(void);
static int diag_geswitch_int_test(void);
static int en_6320_interrupt(int, int);
static int chk_gpio5_val(void);
static int register_mac(void);
static int geswitch_i350_reg_dump(void);
static int geswitch_interrupt_reg_dump(void);
static int geswitch_internal_lpbk_reg_dump(void);

static reg_info_t_ext geswitch_reg_ext = {4, diag_smi_6320_read_fn,
		                                      diag_smi_6320_write_fn, 0};
/* Page 0 - Copper */
static reg_info_t mrvl_6320_p0_reg_tbl[] = {
    {"Physical Ctrl", MRVL6320_PHY_CTRL_REG,
    GESWITCH_RO, {(unsigned long)&geswitch_reg_ext},
    0Xd8bf, 0x4002},
    {"Jamming Ctrl", MRVL6320_JAMMING_CTRL_REG,
    GESWITCH_RW, {(unsigned long)&geswitch_reg_ext},
    0Xffff, 0xff00},
    {"Port Ctrl", MRVL6320_PORT_CTRL_REG,
    GESWITCH_RW, {(unsigned long)&geswitch_reg_ext},
    0Xffff, 0x7f},
    {"Port Ctrl 1", MRVL6320_PORT_CTRL_1_REG,
    GESWITCH_RW, {(unsigned long)&geswitch_reg_ext},
    0Xcfff, 0x0000},
    {"Port Based VLAN MAP", MRVL6320_PORT_BASED_VLAN_MAP_REG,
    GESWITCH_RW, {(unsigned long)&geswitch_reg_ext},
    0Xffff, 0x5f},
    {"Port Ctrl 2", MRVL6320_PORT_CTRL_2_REG,
    GESWITCH_RW, {(unsigned long)&geswitch_reg_ext},
    0Xfffe, 0x2080},
    {"Egress Rate Ctrl", MRVL6320_EGRESS_RATE_CTRL_REG,
    GESWITCH_RW, {(unsigned long)&geswitch_reg_ext},
    0X0f7f, 0x1},
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Sub Menu used for GE PHY Main tests.
 */
static submenu_xtable_t geswitch_main_tests_submenu_table[] = {
    {"88E6320 Test", (type_t(*)())diag_geswitch_test,   TRUE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0,
     (type_t(*)())diag_geswitch_test, FALSE},
    {"88E6320 Utility", (type_t(*)())diag_geswitch_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())diag_geswitch_util, TRUE},
};

#define GESWITCH_MAIN_TESTS_SUBMENU_TABLE_SIZE (\
 sizeof(geswitch_main_tests_submenu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t geswitch_main_tests_primary_items
[GESWITCH_MAIN_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t geswitch_main_tests_secondary_items
[GESWITCH_MAIN_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t geswitch_main_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    geswitch_main_tests_primary_items,
};
menuinfo_t *geswitch_main_submenup = &geswitch_main_subtest_menu;

/* Sub Menu used for GE PHY tests.
 */
static submenu_xtable_t geswitch_tests_submenu_table[] = {
    {"Register test", (type_t(*)())diag_geswitch_reg_test,   0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Interrupt test", (type_t(*)())diag_geswitch_int_test,   0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Internal Loopback test", (type_t(*)())diag_geswitch_int_lpbk_test,   0,
     MF_CONTINUOUS, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"X710 Loopback test", 
    (type_t(*)())diag_geswitch_x710_eth1_mac1_lpbk_test, 0,
     MF_CONTINUOUS, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"6320<->I350 datapath test", 
    (type_t(*)())diag_geswitch_i350_datapath_test,0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},    
    {"eth1(Debug)", (type_t(*)())eth1_tx_rx_only_test, 0,
     MF_CONTINUOUS, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"eth1_mac1(Debug)", (type_t(*)())eth1_mac1_tx_rx_only_test, 0,
     MF_CONTINUOUS, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"I350 Loopback(Debug)", (type_t(*)())diag_geswitch_i350_eth1_lpbk_test,   0,
     MF_CONTINUOUS, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define GESWITCH_TESTS_SUBMENU_TABLE_SIZE (\
sizeof(geswitch_tests_submenu_table) /sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t geswitch_tests_primary_items
[GESWITCH_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t geswitch_tests_secondary_items
[GESWITCH_TESTS_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t geswitch_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    geswitch_tests_primary_items,
};
menuinfo_t *geswitch_submenup = &geswitch_subtest_menu;

int diag_geswitch_build_test (int run_all_tests)
{
    build_primary_submenu(geswitch_main_tests_submenu_table,
			              GESWITCH_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "GE Switch", &geswitch_main_submenup);
    build_secondary_submenu(geswitch_main_tests_submenu_table,
                            GESWITCH_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            geswitch_main_tests_secondary_items);   
                            
    if (run_all_tests) {
        exec_doall_menu_items(geswitch_main_submenup);
    } else {
        menu(geswitch_main_submenup, geswitch_main_tests_secondary_items, '\0');
    }
    return (PASSED);
}

static int diag_geswitch_test (int run_all_tests)
{
    build_primary_submenu(geswitch_tests_submenu_table,
			              GESWITCH_TESTS_SUBMENU_TABLE_SIZE,
                          "GE Switch", &geswitch_submenup);
    build_secondary_submenu(geswitch_tests_submenu_table,
                            GESWITCH_TESTS_SUBMENU_TABLE_SIZE,
                            geswitch_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(geswitch_submenup);
    } else {
        menu(geswitch_submenup, geswitch_tests_secondary_items, '\0');
    }
    return (PASSED);
}

int geswitch_apply_errata (void) {
    
    int retval = PASSED;
    char echo_cmd[64];
     
    /* According to Marvell's Errata set these registers for 6320 
     * config Port4 and Port5 regs to reduce device receive CRC errors
     * config Global2 reg to change ZPR/ZNR for waveform , for more
     * details please see CDETS - CSCvr07313
     * */

    /*  Do write process*/
    if (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
            MRVL6320_PORT_5, MRVL6320_REG_26, MRVL6320_ERRATA_1) == FAILED ) {
        retval = FAILED;
    }

    if (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
            MRVL6320_PORT_4, MRVL6320_REG_26, MRVL6320_ERRATA_2) == FAILED ) {
        retval = FAILED;
    }

    if (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
            MRVL6320_PORT_4, MRVL6320_REG_26, MRVL6320_ERRATA_3) == FAILED ) {
        retval = FAILED;
    }

    if (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
         MRVL6320_GLOBAL_2, MRVL6320_REG_26, MRVL6320_ERRATA_4) == FAILED ) {
        retval = FAILED;
    }

    if(retval == FAILED){
        cterr('f', 0, "GE Switch Errata Setting failed");
    }
    /*After Marvell 6320 Errata settings, need eth1 down up to reconnect NC*/

    system("ifconfig eth1 down");
    msleep(MRVL6320_INIT_DELAY);

    system("ifconfig eth1 up");
    msleep(MRVL6320_INIT_DELAY);

    system("ifconfig eth1_mac1 up");
    msleep(MRVL6320_INIT_DELAY);

    sprintf(echo_cmd, MARVL_ERRATA_SET);
    system(echo_cmd);

    return (retval);
}



static void
add_mb_geswitch_reg_test_err_report(void)
{
    fru_table_offset = MB_GESWITCH;
    platform_fru_table[MB_GESWITCH].pid_string = sku_id;
    platform_fru_table[MB_GESWITCH].location_string = mb_geswitch_loc;
    cterr_add_component("BMC", "88E6320");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Ask HW to check SMI IF");
}

/***************************************************************************
 * Function Name: diag_geswitch_reg_test

 * This function: tests Marvell 6320 registers.
 *
 * Input : void
 *
 * Output: PASSED/FAILED
 ***************************************************************************/

static int diag_geswitch_reg_test (void)
{
    int retval = PASSED;

    if (get_enhance_err_flag()) {
        add_mb_geswitch_reg_test_err_report();
    }
    
    testname("MARVELL 6320 Register");
    prpass(testpass, "MARVELL 6320 Register Test");

    if (register_tests(0, mrvl_6320_p0_reg_tbl) == FAILED) {
        retval = FAILED;
    }

    if(retval == FAILED){
        cterr('f', 0, "88E6320 reg test failed.");
    }
    
    return (retval);
}

static void
add_mb_geswitch_int_test_err_report(void)
{
    fru_table_offset = MB_GESWITCH;
    platform_fru_table[MB_GESWITCH].pid_string = sku_id;
    platform_fru_table[MB_GESWITCH].location_string = mb_geswitch_loc;
    cterr_add_component("BMC", "88E6320");
    cterr_add_reg_dump((PFV)geswitch_interrupt_reg_dump);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Ask HW to check interrupt pin");
}


static int geswitch_interrupt_reg_dump(void)
{
    int reg_val;
    cterr_db_print("Dump BMC<->88E6320 interrupt test related register\n");
    
    if (diag_smi_6320_reg_read(ETH1, MRV88E6320_SWITCH_ID,
        MRVL6320_GLOBAL_1, MRVL6320_SWITCH_CTRL_REG, &reg_val) == FAILED) {
        return (FAILED);
    }
    
    cterr_db_print("Global 1 Register 4: 0x%x\n", reg_val);
     
    return (PASSED);
}

/***************************************************************************
 * Function Name: diag_geswitch_int_test

 * This function: interrupt tests Marvell 6320.
 *
 * Input : void
 *
 * Output: PASSED/FAILED
 ***************************************************************************/

static int diag_geswitch_int_test (void)
{
    int retval = PASSED;

    if (get_enhance_err_flag()) {
        add_mb_geswitch_int_test_err_report();
    }

    testname("MARVELL 6320 interrupt");
    prpass(testpass, "Enable 88E6320 Interrupt...");

    if (en_6320_interrupt(TRUE, MRVL6320_GLOBAL_1) == FAILED) {
        retval = FAILED;
    }
    
    prpass(testpass, "Checking GPIO5 value...");
    if (chk_gpio5_val() == FAILED) {
        retval = FAILED;
    }
    
    prpass(testpass, "Disable 88E6320 Interrupt...");
    if (en_6320_interrupt(FALSE, MRVL6320_GLOBAL_1) == FAILED) {
        retval = FAILED;
    }
    
    if(retval == FAILED){
        cterr('f', 0, "88E6320 interrupt test failed.");
    }
    
    return (retval);
}

static void
add_mb_geswitch_intl_test_err_report(void)
{
    fru_table_offset = MB_GESWITCH;
    platform_fru_table[MB_GESWITCH].pid_string = sku_id;
    platform_fru_table[MB_GESWITCH].location_string = mb_geswitch_loc;
    cterr_add_component("BMC", "88E6320");
    cterr_add_reg_dump((PFV)geswitch_internal_lpbk_reg_dump);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do register test",
                    "more /prco/nuova/ncsi/eth1/p1_c0/active_channel\n",
                    "Check ncsi protocol is working (dmesg| tail)",
                    "Ping 192.123.123.2 (bmc<-ncsi->intel)",
                    "Ask HW to check SMI IF");
}

static int geswitch_internal_lpbk_reg_dump(void)
{
    int reg_val;
    cterr_db_print("Dump BMC<->88E6320 internal lpbk test related register\n");
    
    if (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        MRVL6320_PORT_5, MRVL6320_PORT_BASED_VLAN_MAP, &reg_val) == FAILED ) {
        return (FAILED);
    }
    
    cterr_db_print("Port 5 Register 6: 0x%x\n", reg_val);
     
    return (PASSED);
}

/***************************************************************************
 * Function Name: diag_geswitch_int_lpbk_test

 * This function: Do Marvell 6320 internal loopback test.
 *
 * Input : void
 *
 * Output: PASSED/FAILED
 ***************************************************************************/
static int diag_geswitch_int_lpbk_test (void)
{
    int retval = PASSED, mrvl6320_port;
    int packet_cnt;

    if (get_enhance_err_flag()) {
        add_mb_geswitch_intl_test_err_report();
    }
    
    testname("geswitch  internal loopback Test");
    
    prpass(testpass, "Config 88E6320 Reg....");

    mrvl6320_port = MRVL6320_PORT_5;
    diag_nc_intel_disable_i350_rx();
    system("ifconfig eth1_mac1 down");
    msleep(500);
    
    if (geswitch_loopback_enable(TRUE, mrvl6320_port) == FAILED) {
	    retval = FAILED;
    }
    msleep(500);
    
    prpass(testpass, "Doing Loopback testing....");

    packet_cnt = input_packet_cnt;
    if (eth_pkt_txrx (ETH1, packet_cnt, FALSE) == FAILED) {
        retval = FAILED;
    }

    msleep(500);
    
    prpass(testpass, "Restore Reg.....");
    if (geswitch_loopback_enable(FALSE, mrvl6320_port) == FAILED) {
        retval = FAILED;
    }
    
    msleep(500);
    system("ifconfig eth1_mac1 up");
    msleep(500);

    prpass(testpass, "MAC registration....");
    register_mac();
    msleep(10000);
    
    if(retval == FAILED){
        cterr('f', 0, "mrvl6320 intl lpbk test failed.");
    }

    return (retval);
}

static void
add_mb_geswitch_x710_lpbk_test_err_report(void)
{
    fru_table_offset = MB_GESWITCH;
    platform_fru_table[MB_GESWITCH].pid_string = sku_id;
    platform_fru_table[MB_GESWITCH].location_string = mb_geswitch_loc;
    cterr_add_component("BMC", "88E6320", "X710", "LOOPBACK CABLE");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do register test",
                    "more /prco/nuova/ncsi/eth1/p1_c0/active_channel",
                    "Check ncsi protocol is working (dmesg| tail)",
                    "Ping 192.123.123.2 (bmc<-ncsi->intel)",
                    "Install loopback cable?",
                    "Ask HW to check SMI IF");
}

static int diag_geswitch_x710_eth1_mac1_lpbk_test (void)
{
    int retval = PASSED;
    
    if (get_enhance_err_flag()) {
        add_mb_geswitch_x710_lpbk_test_err_report();
    }
    
    testname("x710 loopback Test");
    
    prpass(testpass, "x710 loopback Test");
    if (eth_pkt_txrx (ETH1_MAC1, PACKET_COUNT, FALSE) == FAILED) {
        cterr('f', 0, "x710 eth1_mac1 intl lpbk test failed.");
        retval = FAILED;
    }

    return (retval);
}

static void
add_mb_geswitch_i350_test_err_report(void)
{
    fru_table_offset = MB_GESWITCH;
    platform_fru_table[MB_GESWITCH].pid_string = sku_id;
    platform_fru_table[MB_GESWITCH].location_string = mb_geswitch_loc;
    cterr_add_component("BMC", "88E6320", "I350");
    cterr_add_reg_dump((PFV)geswitch_i350_reg_dump);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do register test",
                    "more /prco/nuova/ncsi/eth1/p1_c0/active_channel",
                    "Check ncsi protocol is working (dmesg| tail)",
                    "Ping 192.123.123.2 (bmc<-ncsi->intel)",
                    "Ensure loopback stub is installed in i350",
                    "Ask HW to check SMI IF");
}

static int geswitch_i350_reg_dump(void)
{
    int reg_val;
    cterr_db_print("Dump BMC<->88E6320<->i350 test related register\n");
    
    if (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        MRVL6320_PORT_5, MRVL6320_PORT_BASED_VLAN_MAP, &reg_val) == FAILED ) {
        return (FAILED);
    }
    
    cterr_db_print("Port 5  Register 6: 0x%x\n", reg_val);
     
    return (PASSED);
}
    
int diag_geswitch_i350_eth1_lpbk_test (void)
{
    int retval = PASSED, mrvl6320_i350_port, mrvl6320_x710_port,
                 mrvl6320_bmc_port;

    mrvl6320_i350_port = MRVL6320_PORT_6;
    mrvl6320_x710_port = MRVL6320_PORT_2;
    mrvl6320_bmc_port = MRVL6320_PORT_5;

    if (get_enhance_err_flag()) {
        add_mb_geswitch_i350_test_err_report();
    }
    
    testname("i350 loopback Test");

    prpass(testpass, "Force i350 link up...");
    diag_nc_intel_force_i350_link();
    
    system("ifconfig eth1_mac1 down");
    msleep(500);
   
    prpass(testpass, "Checking Port5 VLAN...");
    if (check_port5_vlan() == FAILED) {
        retval = FAILED;
    }

    prpass(testpass, "Doing loopback testing...");
    if (eth_pkt_txrx (ETH1, PACKET_COUNT, FALSE) == FAILED) {
        retval = FAILED;
    }
    
    msleep(500);
    prpass(testpass, "Restore to the default setting...");
    system("ifconfig eth1_mac1 up");
    msleep(500);
    
    prpass(testpass, "MAC registration...");
    register_mac();
    msleep(10000);

    if (retval == FAILED) {
        cterr('f', 0, "i350 eth1 intl lpbk test failed.");
    }
    
    return (retval);
}

static int check_port5_vlan(void)
{
    int reg_data;
    int retval = PASSED;
    
    if (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        MRVL6320_PORT_5, MRVL6320_PORT_BASED_VLAN_MAP, &reg_data) == FAILED ) {
        retval = FAILED;
    }
    
    printf("Port 5 Register 6 is 0x%x\n",reg_data);
    
    if (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        MRVL6320_PORT_5, MRVL6320_PORT_BASED_VLAN_MAP,
        MRVL6320_P5_VLAN_ALL_ONE) == FAILED ) {
        retval = FAILED;
    }
    
    if (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        MRVL6320_PORT_5, MRVL6320_PORT_BASED_VLAN_MAP, &reg_data) == FAILED ) {
        retval = FAILED;
    }
    
    if (reg_data != MRVL6320_P5_VLAN_ALL_ONE) {
        retval = FAILED;
    }

    return (retval);
}

/***************************************************************************
 * Function Name: tachi_l_geswitch_loopback_enable

 * This function: set routing port
 *
 * Input : enable loopback or not
 *
 * Output: PASSED/FAILED
 ***************************************************************************/
int geswitch_loopback_enable(int enable, int port)
{
    int reg_val, reg_data;
    int retval = PASSED;

    if (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        port, MRVL6320_PORT_BASED_VLAN_MAP, &reg_data) == FAILED ) {
        retval = FAILED;
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("P5R6 is 0x%x\n",reg_data);
    }
    
    if (enable == TRUE) {
	    reg_val = 0x7F;
    } else {
	    reg_val = (~(1 << port) & 0x7F);
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("P5R6 0x%x\n",reg_val);
    }


    if (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        port, MRVL6320_PORT_BASED_VLAN_MAP, reg_val) == FAILED ) {
        retval = FAILED;
    }

    if (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRV88E6320_SWITCH_ID,
        port, MRVL6320_PORT_BASED_VLAN_MAP, &reg_data) == FAILED ) {
        retval = FAILED;
    }
  
    if (reg_data != reg_val) {
        retval = FAILED;
    }
    
    return (retval);
}


static int diag_smi_6320_read_fn (unsigned long addr, int size,
                              unsigned long *buf, void *param)
{

    return (diag_smi_6320_reg_read(PHY_DEVICE_NAME, MRVL6320_ADDRESS,
				   MRVL6320_PORT_5, addr, (int *)buf));
}


static int diag_smi_6320_write_fn (unsigned long addr, int size,
                               unsigned long data, void *param)
{
    return (diag_smi_6320_reg_write(PHY_DEVICE_NAME, MRVL6320_ADDRESS,
					MRVL6320_PORT_5, addr, data));
}

static int eth1_mac1_tx_rx_only_test(void)
{
    int retval =PASSED;
    if (eth_pkt_txrx (ETH1_MAC1, PACKET_COUNT, TRUE) == FAILED) {
        retval = FAILED;
    }
    return (retval);
}
static int eth1_tx_rx_only_test(void)
{
    int retval =PASSED;
    if (eth_pkt_txrx (ETH1, PACKET_COUNT, TRUE) == FAILED) {
        retval = FAILED;
    }
    return (retval);
}

static void
add_mb_geswitch_i350_datapath_test_err_report(void)
{
    fru_table_offset = MB_GESWITCH;
    platform_fru_table[MB_GESWITCH].pid_string = sku_id;
    platform_fru_table[MB_GESWITCH].location_string = mb_geswitch_loc;
    cterr_add_component("BMC", "88E6320", "I350");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Open /proc/i350_vid to check its contents");
}

static int diag_geswitch_i350_datapath_test(void)
{
    FILE *fp;
    int val;

    if (get_enhance_err_flag()) {
        add_mb_geswitch_i350_datapath_test_err_report();
    }    
    
    testname("6320<->I350 datapath test");
    prpass(testpass, "6320<->I350 datapath test");
    
    fp = fopen("/proc/i350_vid", "r");
    if (!fp) {
        cterr('f', 0, "\n%s: Can't open '%s'\n\n", __FUNCTION__,
              I350_VID_PROC_PATH);
        return (FAILED);
    }
    
    if (fscanf(fp, "%x", &val) == EOF) {
	printf("problem scanning number\n");
	goto out;
    }

    printf("\nVendor ID: %x\n", val);

    if (val != I350_VID) {
	    cterr('f', 0, "Wrong Vendor ID, expect 0x%04x, got 0x%04x", I350_VID, 
              val);
	return (FAILED);
    }
    
out:
    fclose(fp);

    return (PASSED);
}

static int en_6320_interrupt(int en, int port) {
    int retval = PASSED;
    int reg_val;

    if (diag_smi_6320_reg_read(ETH1, MRV88E6320_SWITCH_ID,
        port, MRVL6320_SWITCH_CTRL_REG, &reg_val) == FAILED) {
        retval = FAILED;
    }
   
    if (en) {
        reg_val &= (~MRVL6320_EE_INT_EN);
    } else {
        reg_val |= MRVL6320_EE_INT_EN;

    }

    if (diag_smi_6320_reg_write(ETH1, MRV88E6320_SWITCH_ID,
        port, MRVL6320_SWITCH_CTRL_REG, reg_val) == FAILED) {
        retval = FAILED;
    }

    if (diag_smi_6320_reg_read(ETH1, MRV88E6320_SWITCH_ID,
        port, MRVL6320_SWITCH_CTRL_REG, &reg_val) == FAILED) {
        retval = FAILED;
    }
    
    printf("reg_val 0x%x\n", reg_val);

    return (retval);
}

static int chk_gpio5_val(void) {
    int retval = PASSED;
    system("cat /proc/nuova/gpio/sgpio_led_mux_select > gpio5.txt");
    
    int ret = system("cat gpio5.txt | grep -qI 1");
    if (ret !=  PASSED) {
        retval = FAILED;
    } 

    system("rm gpio5.txt");
    return (retval);
}

static int register_mac(void)
{
    FILE *stream;
    char mac[100];
    char cmd[64];

    system("more /proc/nuova/ncsi/eth1/p1_c0/macaddr > macaddr.txt");

    stream = fopen("macaddr.txt","r");
    if (stream == NULL) {
        printf("open file failed!\n");
        return (FAILED);
    } else {
        fscanf(stream, "%s", mac);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("eth1_mac1 mac is %s\n", mac);
        }
        fclose(stream);
    }

    sprintf(cmd, "arp -s 192.123.123.2 %s", mac);
    system(cmd);
    system("arp -s 192.123.123.11 00:00:00:00:00:99");
    system("rm macaddr.txt");
    return (PASSED);
}


/*---------------------------------------------------------------
$Log: diag_geswitch_test.c,v $
Revision 1.5  2019/09/10 01:03:39  haohsu
[CSCvr07313]-Marvell 6320 to BMC eth1 frame error issue

Revision 1.4  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.3.12.3  2016/11/09 04:33:38  benchen2
add gephy, geswitch reg dump

Revision 1.3.12.2  2016/11/08 18:06:26  benchen2
add bmc<->6320<->i350 loopback test reg dump

Revision 1.3.12.1  2016/11/04 19:08:55  benchen2
Modify Enhanced error message

Revision 1.3  2016/05/04 06:14:21  benchen2
tachil:check 6320 port 5 status

Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.29  2016/04/20 00:38:00  huanngo
Remove the code for Tachi-H

Revision 1.1.2.28  2016/04/08 07:33:29  benchen2
move i350 ncsi lpbk to intel

Revision 1.1.2.27  2016/03/31 08:17:18  benchen2
add lewis arp

Revision 1.1.2.26  2016/03/28 04:16:45  benchen2
modify i350 lpbk test

Revision 1.1.2.25  2016/03/11 19:00:16  huanngo
Fix the bug in diag_geswitch_i350_datapath_test

Revision 1.1.2.24  2016/03/10 00:42:43  huanngo
Adding I350 datapath test

Revision 1.1.2.23  2016/03/09 07:36:47  benchen2
add 6320 interrupt test

Revision 1.1.2.22  2016/02/02 07:25:25  benchen2
add enhanced error message

Revision 1.1.2.21  2016/01/06 03:54:03  benchen2
for new bmc kernel, fix 6320/x710/i350 loopback test

Revision 1.1.2.20  2016/01/05 17:35:14  huanngo
Fix the bug in geswitch_loopback_enable

Revision 1.1.2.19  2015/12/28 06:12:30  hondwang
Add and modify files for INTEL NC command support

Revision 1.1.2.18  2015/12/25 07:37:11  benchen2
modify 6320 internal loopback test

Revision 1.1.2.17  2015/12/25 06:55:29  benchen2
modify 6320 loopback test

Revision 1.1.2.16  2015/12/24 14:08:55  benchen2
modify i350and x710 lpbk code

Revision 1.1.2.15  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.14  2015/12/04 09:20:27  benchen2
for fix CDETS CSCux25853

Revision 1.1.2.13  2015/10/08 02:51:29  benchen2
for stable x710 test

Revision 1.1.2.12  2015/10/03 06:49:11  benchen2
modify x710 lpbk issue

Revision 1.1.2.11  2015/10/02 04:35:53  benchen2
fix 710 lpbtest phase 1

Revision 1.1.2.10  2015/10/02 02:23:14  benchen2
remove i250/x710 DOALL flag

Revision 1.1.2.9  2015/09/23 09:24:15  benchen2
set ctrl reg default value

Revision 1.1.2.8  2015/09/23 09:22:10  benchen2
disable debug msg

Revision 1.1.2.7  2015/09/21 07:41:06  benchen2
Fix 6320 intloopback intermittent fail

Revision 1.1.2.6  2015/09/17 02:39:39  benchen2
add cterr

Revision 1.1.2.5  2015/09/15 06:46:53  benchen2
add i350 lpbk func

Revision 1.1.2.4  2015/08/14 05:53:59  benchen2
add verbose flag

Revision 1.1.2.3  2015/08/04 02:26:31  hondwang
add i350, x710 lpbk test

Revision 1.1.2.2  2015/07/31 07:33:23  hondwang
geswitch test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/
