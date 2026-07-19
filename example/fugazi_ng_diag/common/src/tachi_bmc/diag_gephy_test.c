/* $Id: diag_gephy_test.c,v 1.3 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_gephy_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_gephy_test.c - GE PHY 88E1512 Test Functions
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015 - 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "nvmonvars.h"
#include <sys/socket.h>
#include "diag_gephy_test.h"
#include "diag_gephy_util.h"
#include "diag_geswitch_test.h"
#include "diag_smi_lib.h"
#include "common_utils.h"
#include "proto.h"
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <sys/io.h>
#include "diag_eth_pkt_txrx.h"
#include <fcntl.h>
#include <sys/mman.h>
#include "diag_geswitch_test.h"
#include "platform_fru.h"
#include "mb_tests.h"

int diag_gephy_build_test(int);
static int diag_mgmt_gephy_test(int);
static int diag_ncsi_gephy_test(int);
static int diag_gephy_reg_test(void);
static int diag_gephy_int_lpbk_test(void);
static int diag_gephy_ext_lpbk_test(void);
int gephy_if_enable(char *, int);
static int diag_gephy_set_mode(void);
static int diag_gephy_force_cooper_speed(void);
static int gephy_check_link_status();
static int gephy_set_int_loopback(int);
static int gephy_enable_stub_test(int);
static int diag_smi_read_fn(unsigned long, int, unsigned long *, void *);
static int diag_smi_write_fn(unsigned long, int, unsigned long, void *);

static int diag_gephy_set_fd_adver(int);
static int diag_gephy_set_copper_ctrl_reg(int);
static int diag_gephy_dis_link_pulses(int);
static int diag_gephy_soft_reset();
static int diag_gephy_set_page_fa(int);
static int gephy_set_cop_pkt_gen(int);
static int gephy_intl_lpbk_reg_dump(void);
static int gephy_extl_lpbk_reg_dump(void);

boolean mgmt_gephy = TRUE;

static reg_info_t_ext gephy_reg_ext = {4, diag_smi_read_fn, diag_smi_write_fn, 0};

/* Page 0 - Copper */
static reg_info_t mrvl_1512_p0_reg_tbl[] = {
    {"Control Register", MRV88E1512C_CONTROL_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0x7500, 0x1040},
    {"Status Register", MRV88E1512C_STATUS_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x7969},
    {"Auto-Negotiation Advertisement Register", MRV88E1512C_AUTONEG_ADVR_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0xBFFF, 0x01e1},
    {"Link Partner Ability Register - Base Page", MRV88E1512C_LINK_PART_AV_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x21},
    {"Auto-Negotiation Expansion Register", MRV88E1512C_AUTONEG_EXPANSION_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x0004},
    {"Next Page Transmit Register", MRV88E1512C_NEXT_PAGE_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0xB7FF, 0x2001},
    {"Link Partner Next Page Register", MRV88E1512C_LP_NEXT_PAGE_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x0000},
    {"1000BASE-T Control Register", MRV88E1512C_1000B_CNTL_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0xFFFF, 0x0300},
    {"1000BASE-T Status Register", MRV88E1512C_1000B_STATUS_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x0000},
    {"Extended Status Register", MRV88E1512C_EXTENDED_STATUS_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x3000},
    {"Specific Control Register 1", MRV88E1512C_SPECIFIC_CONTROL1_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0x7F7F, 0x3060},
    {"Specific Status Register 1", MRV88E1512C_SPECIFIC_STATUS1_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x8040},
    {"Interrupt Enable Register", MRV88E1512C_SPECIFIC_INT_ENABLE_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0xFFFF, 0x0000},
    {"Specific Status Register 2", MRV88E1512C_INT_STATUS_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x0040},
    {"Specific Control Register 2", MRV88E1512C_SPECIFIC_CONTROL2_REG,
    		GEPHY_RW, {(unsigned long)&gephy_reg_ext},
        0xFFFF, 0x0020},
    {"Receive Error Counter Register", MRV88E1512C_REC_ERROR_COUNTER_REG,
    		GEPHY_RO, {(unsigned long)&gephy_reg_ext},
        0x0000, 0x0000},
    {"End of Page 0 registers", 0, 0, {0}, 0, 0},
};

/* Sub Menu used for Mgmt GE PHY Main tests.
 */
static submenu_xtable_t mgmt_gephy_main_tests_submenu_table[] = {
    {"88E1512 Test", (type_t(*)())diag_mgmt_gephy_test,   TRUE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())diag_mgmt_gephy_test,FALSE},
    {"88E1512 Utility", (type_t(*)())diag_gephy_util,   TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())diag_gephy_util,   TRUE},
};

#define MGMT_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(mgmt_gephy_main_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mgmt_gephy_main_tests_primary_items[MGMT_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE +
						   MAX_BASE_ITEMS];
static mitem_t mgmt_gephy_main_tests_secondary_items[MGMT_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE +
						     MAX_BASE_ITEMS];

menuinfo_t mgmt_gephy_main_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mgmt_gephy_main_tests_primary_items,
};
menuinfo_t *mgmt_gephy_main_submenup = &mgmt_gephy_main_subtest_menu;

/******************************************************************/
/* Sub Menu used for Ncsi GE PHY Main tests.
 */
static submenu_xtable_t ncsi_gephy_main_tests_submenu_table[] = {
    {"88E1512 Test", (type_t(*)())diag_ncsi_gephy_test,   TRUE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())diag_ncsi_gephy_test,   FALSE},
    {"88E1512 Utility", (type_t(*)())diag_gephy_util,   TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())diag_gephy_util,   TRUE},
};

#define NCSI_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(ncsi_gephy_main_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ncsi_gephy_main_tests_primary_items[NCSI_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE +
						     MAX_BASE_ITEMS];
static mitem_t ncsi_gephy_main_tests_secondary_items[NCSI_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE +
						       MAX_BASE_ITEMS];

menuinfo_t ncsi_gephy_main_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ncsi_gephy_main_tests_primary_items,
};
menuinfo_t *ncsi_gephy_main_submenup = &ncsi_gephy_main_subtest_menu;


/* Sub Menu used for Mgmt GE PHY tests.
 */
static submenu_xtable_t mgmt_gephy_tests_submenu_table[] = {
    {"Register test", (type_t(*)())diag_gephy_reg_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Internal Loopback test", (type_t(*)())diag_gephy_int_lpbk_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"External Loopback test", (type_t(*)())diag_gephy_ext_lpbk_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define MGMT_GEPHY_TESTS_SUBMENU_TABLE_SIZE (sizeof(mgmt_gephy_tests_submenu_table) / \
					     sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t mgmt_gephy_tests_primary_items[MGMT_GEPHY_TESTS_SUBMENU_TABLE_SIZE +
					      MAX_BASE_ITEMS];
static mitem_t mgmt_gephy_tests_secondary_items[MGMT_GEPHY_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t mgmt_gephy_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    mgmt_gephy_tests_primary_items,
};
menuinfo_t *mgmt_gephy_submenup = &mgmt_gephy_subtest_menu;

/**********************************************************************/
/* Sub Menu used for Ncsi GE PHY tests.
 */
static submenu_xtable_t ncsi_gephy_tests_submenu_table[] = {
    {"Register test", (type_t(*)())diag_gephy_reg_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"Internal Loopback test", (type_t(*)())diag_gephy_int_lpbk_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
	{"88E6320 Loopback test", (type_t(*)())diag_gephy_ext_lpbk_test,   0,
	MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define NCSI_GEPHY_TESTS_SUBMENU_TABLE_SIZE (sizeof(ncsi_gephy_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ncsi_gephy_tests_primary_items[NCSI_GEPHY_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t ncsi_gephy_tests_secondary_items[NCSI_GEPHY_TESTS_SUBMENU_TABLE_SIZE +
						  MAX_BASE_ITEMS];

menuinfo_t ncsi_gephy_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    ncsi_gephy_tests_primary_items,
};
menuinfo_t *ncsi_gephy_submenup = &ncsi_gephy_subtest_menu;


int diag_mgmt_gephy_build_test (int run_all_tests)
{
    mgmt_gephy = TRUE;
    build_primary_submenu(mgmt_gephy_main_tests_submenu_table,
			  MGMT_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY", &mgmt_gephy_main_submenup);
    build_secondary_submenu(mgmt_gephy_main_tests_submenu_table,
                            MGMT_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            mgmt_gephy_main_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(mgmt_gephy_main_submenup);
    } else {
        menu(mgmt_gephy_main_submenup, mgmt_gephy_main_tests_secondary_items, '\0');
    }
    return (PASSED);
}

int diag_ncsi_gephy_build_test (int run_all_tests)
{
    mgmt_gephy = FALSE;
    build_primary_submenu(ncsi_gephy_main_tests_submenu_table,
			  NCSI_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY", &ncsi_gephy_main_submenup);
    build_secondary_submenu(ncsi_gephy_main_tests_submenu_table,
                            NCSI_GEPHY_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            ncsi_gephy_main_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(ncsi_gephy_main_submenup);
    } else {
        menu(ncsi_gephy_main_submenup, ncsi_gephy_main_tests_secondary_items, '\0');
    }
    return (PASSED);
}


static int diag_mgmt_gephy_test (int run_all_tests)
{
    build_primary_submenu(mgmt_gephy_tests_submenu_table,
			  MGMT_GEPHY_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY", &mgmt_gephy_submenup);
    build_secondary_submenu(mgmt_gephy_tests_submenu_table,
                            MGMT_GEPHY_TESTS_SUBMENU_TABLE_SIZE,
                            mgmt_gephy_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(mgmt_gephy_submenup);
    } else {
        menu(mgmt_gephy_submenup, mgmt_gephy_tests_secondary_items, '\0');
    }
    return (PASSED);
}

static int diag_ncsi_gephy_test (int run_all_tests)
{
    build_primary_submenu(ncsi_gephy_tests_submenu_table,
			  NCSI_GEPHY_TESTS_SUBMENU_TABLE_SIZE,
                          "GE PHY", &ncsi_gephy_submenup);
    build_secondary_submenu(ncsi_gephy_tests_submenu_table,
                            NCSI_GEPHY_TESTS_SUBMENU_TABLE_SIZE,
                            ncsi_gephy_tests_secondary_items);    
                            
    if (run_all_tests) {
        exec_doall_menu_items(ncsi_gephy_submenup);
    } else {
        menu(ncsi_gephy_submenup, ncsi_gephy_tests_secondary_items, '\0');
    }
    return (PASSED);
}

static void
add_mb_gephy_reg_test_err_report(void)
{
    fru_table_offset = MB_GEPHY;
    platform_fru_table[MB_GEPHY].pid_string = sku_id;
    platform_fru_table[MB_GEPHY].location_string = mb_gephy_loc;
    cterr_add_component("BMC", "88E1512");
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Check the 88E1512 SMI IF");
}

static int diag_gephy_reg_test (void)
{
    int retval = PASSED;
    
    if (get_enhance_err_flag()) {
        add_mb_gephy_reg_test_err_report();
    }
    
    testname("GEPHY Register");
    prpass(testpass, "GEPHY Register Test");

    if (register_tests(0, mrvl_1512_p0_reg_tbl) == FAILED) {
    	retval = FAILED;
        cterr('f', 0, "gephy reg test fail\n");
    }
    return (retval);
}

static void
add_mb_gephy_intl_test_err_report(void)
{
    fru_table_offset = MB_GEPHY;
    platform_fru_table[MB_GEPHY].pid_string = sku_id;
    platform_fru_table[MB_GEPHY].location_string = mb_gephy_loc;
    cterr_add_component("BMC", "88E1512");
    cterr_add_reg_dump((PFV)gephy_intl_lpbk_reg_dump);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do 88E1512 Register test",
                    "Check the 88E1512 SMI IF");
}

static int gephy_intl_lpbk_reg_dump(void)
{
    int reg_val;
    cterr_db_print("Dump 88E1512 internal loopback test related register\n");

    reg_val = diag_gephy_reg_value_get(MRV88E1512_REG_PAGE_0,
              MRV88E1512C_CONTROL_REG);
    cterr_db_print("Page 0 Register 0: 0x%x\n", reg_val);
    
    reg_val = diag_gephy_reg_value_get(MRV88E1512_REG_PAGE_0,
              MRV88E1512C_STATUS_REG);
    cterr_db_print("Page 0 Register 1: 0x%x\n", reg_val);
    
    reg_val = diag_gephy_reg_value_get(MRV88E1512_REG_PAGE_6,
              MRV88E1512_CHECK_CONTROL_REG);
    cterr_db_print("Page 6 Register 18: 0x%x\n", reg_val);
     
    return (PASSED);
}

static int diag_gephy_int_lpbk_test (void)
{
	int retval;
    
    if (get_enhance_err_flag()) {
        add_mb_gephy_intl_test_err_report();
    }
    
    testname("GEPHY INTERNAL LOOPBACK TEST\n");
	prpass(testpass, "GEPHY INTERNAL LOOPBACK TEST\n");

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("ifconfig eth0 down\n");
    }

    if (gephy_if_enable(PHY_DEVICE_NAME, FALSE) == FAILED) {
        printf("Bring %s up FAIL\n", PHY_DEVICE_NAME);
	    retval = FAILED;
    }
	
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("1. SET P0R9 1000BASE-T CONTROL REG\n");
    }

	/* force master, 1000Mbps Full-duplex advertise (page 0, reg 9) */
    if (diag_gephy_set_fd_adver(TRUE) == FAILED) {
        printf("Set Full-Duplex Advertise Fail!\n");
	    retval = FAILED;
    }

	if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("2. SET P0R0 COPPER CONTROL REG  SOFT RESET\n");
	}

    /* set copper speed (page 0, reg 0) */
    /* 1000Mbps, full-duplex, auto-nego enabled, */
    if (diag_gephy_set_copper_ctrl_reg(TRUE) == FAILED) {
        printf("Set Copper Speed Fail!\n"); 
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("3. SET P0R16 COPPER SPECIFIC CTRL REG 1\n");
    }

    if (diag_gephy_dis_link_pulses(TRUE) == FAILED) {
        printf("Disable Link Pulses Fail!\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("4. SET P0R0 COPPER CONTROL REG  SOFT RESET\n");
    }

    if (diag_gephy_soft_reset() == FAILED) {
        printf("Soft Reset Fail!\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("5. SET P250R1 0X0418\n");
        printf("6. SET P250R1 0X020C\n");
    }

    if (diag_gephy_set_page_fa(TRUE) == FAILED) {
        printf("Set Page FA Fail!\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("7. SET P0R0 COPPER CONTROL REG  LPBK BIT\n");
    }

    if (gephy_set_int_loopback(TRUE) == FAILED) {
        printf("Set Internal Loopback Bit Fail!!\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("8. SET P6R16  COOPER PORT PKT GEN\n");
    }

    if (gephy_set_cop_pkt_gen(TRUE) == FAILED) {
        printf("Set Copper Packet Generator Fail!!\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("9. Check Link status\n");
    }
    if (gephy_check_link_status() == FAILED) {
        printf("Check Link Status Fail\n");
	    retval = FAILED;
    }

    msleep(500);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("10. ifconfig eth0 up\n");
    }

    if (gephy_if_enable(PHY_DEVICE_NAME, TRUE) == FAILED) {
        printf("Bring %s up FAIL\n", PHY_DEVICE_NAME);
	    retval = FAILED;
    }

    if (mgmt_gephy == TRUE) {
	if (eth_pkt_txrx (PHY_DEVICE_NAME, PACKET_COUNT, FALSE) == FAILED) {
	    retval = FAILED;
	}
    } else {
	if (eth_pkt_txrx (ETH1, PACKET_COUNT, FALSE) == FAILED) {
        retval = FAILED;
	}
    }

    /* force master, 1000Mbps Full-duplex advertise (page 0, reg 9) */
    if (diag_gephy_set_fd_adver(FALSE) == FAILED) {
        printf("Set Full-Duplex To Original Fail!\n");
	    retval = FAILED;
    }

    if (diag_gephy_set_copper_ctrl_reg(FALSE) == FAILED) {
        printf("Set Copper Speed To Original Fail\n");
	    retval = FAILED;
    }

    if (diag_gephy_dis_link_pulses(FALSE) == FAILED) {
        printf("Disable Link Pulses To Original Fail\n");
	    retval = FAILED;
    }

    if (diag_gephy_soft_reset() == FAILED) {
        printf("Soft Reset Fail!\n");
	    retval = FAILED;
    }

    if (diag_gephy_set_page_fa(FALSE) == FAILED) {
        printf("Set Page FA To Original Fail!\n");
	    retval = FAILED;
    }

    if (gephy_set_int_loopback(FALSE) == FAILED) {
        printf("Set Loopback Bit To Original!\n");
	    retval = FAILED;
    }

    if (gephy_set_cop_pkt_gen(FALSE) == FAILED) {
        printf("Set Copper Packet Gen To Fail!\n");
	    retval = FAILED;
    }

    if(retval == FAILED){
        cterr('f', 0, "Internal Loopback Test Failed\n");
    }

    return (retval);
}

static void
add_mb_gephy_extl_test_err_report(void)
{
    fru_table_offset = MB_GEPHY;
    platform_fru_table[MB_GEPHY].pid_string = sku_id;
    platform_fru_table[MB_GEPHY].location_string = mb_gephy_loc;
    cterr_add_component("BMC", "88E1512", "LOOPBACK CABLE");
    cterr_add_reg_dump((PFV)gephy_extl_lpbk_reg_dump);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Do 88E1512 Register test",
                    "Do 88E1512 internal loopback test",
                    "Check the 88E1512 SMI IF",
                    "Install loopback cable ?");
}

static int diag_gephy_ext_lpbk_test (void)
{
    int retval = PASSED, i;
    
    if (get_enhance_err_flag()) {
        add_mb_gephy_extl_test_err_report();
    }
    
    testname("GEPHY EXTERNAL LOOPBACK TEST\n");

    prpass(testpass, "initialize the register...\n");
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (retval);
    }

    /* If this is the NCSI PHY, enable 88E6320 loopback */
    if (mgmt_gephy != TRUE) {
	    if (geswitch_loopback_enable(TRUE, MRVL6320_PORT_0)) {
	        retval = FAILED;
	    }
    }
    
    if (gephy_if_enable(PHY_DEVICE_NAME, TRUE) == FAILED) {
        printf("Bring %s up FAIL\n", PHY_DEVICE_NAME);
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("1. set mode\n");
    }

    if (diag_gephy_set_mode() == FAILED) {
        printf("SET RGMII to COPPER FAIL\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("2. force copper to 1000mbps\n");
    }

    msleep(500);

    if (diag_gephy_force_cooper_speed() == FAILED) {
        printf("FORCE COPPER SPEED FAIL\n");
	    retval = FAILED;
    }

    msleep(500);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("3. set external loopback\n");
    }

    if (gephy_enable_stub_test(TRUE) == FAILED) {
        printf("ENABLE STUB TEST FAIL\n");
	    retval = FAILED;
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("4. Check Link status\n");
    }

    /* Give 2 retries before timing out */
    for (i = 0; i < 3; i++) {
	    if (gephy_check_link_status() == PASSED) {
	        break;
	    }
    }
    
    if (i == 3) {
        cterr('f', 0, "Page 0 Register 1 bit 2 indicate link is down...\n");
        gephy_enable_stub_test(FALSE);
        return (FAILED);
    }
    
    prpass(testpass, "Doing the loopback test\n");

    if (eth_pkt_txrx (PHY_DEVICE_NAME, PACKET_COUNT, FALSE) == FAILED) {
	    retval = FAILED;
    }

    prpass(testpass, "Restore the register setting\n");
    if (gephy_enable_stub_test(FALSE) == FAILED) {
        printf("DISABLE STUB TEST FAIL\n");
	    retval = FAILED;
    }
    
    msleep(1000);

    /* If this is the NCSI PHY, disable 88E6320 loopback */
    if (mgmt_gephy != TRUE) {
	    if (geswitch_loopback_enable(FALSE, MRVL6320_PORT_0)) {
	        retval = FAILED;
	    }
    }

    if(retval == FAILED){
        cterr('f', 0, "External Loopback Test Failed\n");
    }
    
    return (retval);
}

int gephy_if_enable (char *dev_name, int enable)
{
    struct ifreq ifr;
    int sock;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("%s: Failed to create ioctl socket\n", __FUNCTION__);
        return (FAILED);
    }

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);

    /* Setup promiscous flag in order to receive all frames */
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
        close (sock);
        printf("%s: Get Iface flag fails!\n", __FUNCTION__);
        return (FAILED);
    }

    /* Return if the interface is already up */
    if ((ifr.ifr_flags & IFF_UP) && (enable == TRUE)) {
        close(sock);
        return (PASSED);
    }

    if (enable == TRUE) {
        ifr.ifr_flags |= IFF_UP;
    } else {
        ifr.ifr_flags &= ~IFF_UP;
    }

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) == -1) {
        close (sock);
        printf("%s: Set Iface flag fails!\n", __FUNCTION__);
        return (FAILED);
    }

    close (sock);
    return (PASSED);
}

int gephy_enable_stub_test (int enable)
{
	ulong reg_d;
	
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_6) == FAILED) {
		printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
		return (FAILED);
	}

	if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_CHECK_CONTROL_REG, &reg_d) == FAILED) {
		printf("Read Register 0X%x FAIL\n", MRV88E1512_CHECK_CONTROL_REG);
		return (FAILED);
	}

	if (enable == TRUE) {
	    reg_d |= MRV88E1512_ENABLE_STUB_TEST;
	} else {
		reg_d &= (~MRV88E1512_ENABLE_STUB_TEST);
	}

	if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_CHECK_CONTROL_REG, reg_d) == FAILED) {
	    printf("Write Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
		return (FAILED);
	}
    
    /*Back to P0*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }
    
    return (PASSED);
}

int gephy_check_link_status ()
{
	ulong reg_d;
	int ix;

    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
	    printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
		return (FAILED);
	}

	for (ix = 0; ix < PHY_RESET_TIMEOUT; ix++) {
	    if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
            MRV88E1512C_STATUS_REG, &reg_d) == FAILED) {
		    printf("Read Register 0X%x FAIL\n", MRV88E1512C_STATUS_REG);
			return (FAILED);
	    }

	    /* See if the reset bit is already cleared out */
		if (reg_d & PHY_STATUS_LINK_UP) {
		    return (PASSED);
		} else {
		    /*Delay 10ms  for re-check link status */
		    usleep(10000);
		    continue;
		}
	}
    return (FAILED);
}

int gephy_set_int_loopback (int enable)
{
	ulong reg_d;

    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

	if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512C_CONTROL_REG, &reg_d) == FAILED) {
	    printf("Read Register 0X%x FAIL\n", MRV88E1512C_CONTROL_REG);
		return (FAILED);
	}

	if (enable == TRUE) {
	    reg_d |= (PHY_LOOPBACK_REG);
	} else {
	    reg_d &= (~PHY_LOOPBACK_REG);
	}

    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512C_CONTROL_REG, reg_d) == FAILED) {
		printf("Write Register 0X%x FAIL\n", MRV88E1512C_CONTROL_REG);
		return (FAILED);
	}
    return (PASSED);
}


int diag_gephy_set_mode()
{    
    ulong reg_val;
      
    /*switch to P18*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_18) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

    /*read value of P18R20 */
    if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
        MRVL88E1512_GENERAL_CONTROL_REG, &reg_val) == FAILED) {
        printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
        return (FAILED);
    }

    reg_val &= ~MRV88E1512_P18_R20_MODE;
    reg_val |= MRVL88E1512_GEN_CTRL_MODE_RGMII_COPPER;

    
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRVL88E1512_GENERAL_CONTROL_REG, reg_val) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }
    
    /*Back to P0*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

    return (PASSED);
}


int diag_gephy_force_cooper_speed()
{
    ulong reg_val;
    
    /*switch to P0*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }
    
    if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512C_CONTROL_REG, &reg_val) == FAILED) {
        printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
        return (FAILED);
    }

    reg_val &= (~MRV88E1512_CONTROL_SPEED_MASK);
    reg_val |= MRV88E1512_CONTROL_FORCE_1000_M | MRV88E1512_CONTROL_FULL_DUPLEX |
               MRV88E1512_CONTROL_AUTONEG_ENABLE | MRV88E1512_CONTROL_PHY_RESET_VALUE;
    
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512C_CONTROL_REG, reg_val) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

    /*switch to P0*/
    if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
        MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
        printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
        return (FAILED);
    }

    return (PASSED);
}

static int diag_smi_read_fn (unsigned long addr, int size,
                              unsigned long *buf, void *param)
{
    return (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID, addr,
				  (unsigned long *)buf));
}


static int diag_smi_write_fn (unsigned long addr, int size,
                               unsigned long data, void *param)
{
    return (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, addr, data));
}


static int diag_gephy_set_fd_adver(int enable)
{
    ulong reg_val;

     /*PAGE 0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*read value of P0R9 */
     if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_1000B_CNTL_REG, &reg_val) == FAILED) {
         printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
         return (FAILED);
     }

    if (enable == TRUE) {
        reg_val &= ~(MRVL1512_1000B_T_CTRL_HALF_DUPLEX);
        reg_val |= MRVL1512_1000B_T_CTRL_1G_SETTING ;
    } else {
        reg_val &= (~MRVL1512_1000B_T_CTRL_1G_SETTING);
        reg_val |= MRVL1512_1000B_T_CTRL_HALF_DUPLEX ;
    }

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_1000B_CNTL_REG, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*Back to P0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     return (PASSED);
}

static int diag_gephy_set_copper_ctrl_reg(int enable)
{
    ulong reg_val;

     /*PAGE 0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*read value of P0R0 */
     if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_CONTROL_REG, &reg_val) == FAILED) {
         printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
         return (FAILED);
     }

    if (enable == TRUE) {
        reg_val &= ~(MRV88E1512_CONTROL_AUTONEG_ENABLE);
        reg_val |= MRV88E1512_CONTROL_PHY_RESET | MRV88E1512_CONTROL_FULL_DUPLEX;
    } else {
        reg_val &= (~MRV88E1512_CONTROL_FULL_DUPLEX);
        reg_val |= MRV88E1512_CONTROL_AUTONEG_ENABLE | MRV88E1512_CONTROL_PHY_RESET;
    }

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_CONTROL_REG, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*Back to P0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     return (PASSED);
}
static int diag_gephy_dis_link_pulses(int enable)
{
    ulong reg_val;

     /*PAGE 0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*read value of P0R16 */
     if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_SPECIFIC_CONTROL1_REG, &reg_val) == FAILED) {
         printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
         return (FAILED);
     }

    if (enable == TRUE) {
        reg_val |= MRVL1512_COPPER_SPEC_CTRL_DIS_LINK_PULSES;
    } else {
        reg_val &= ~MRVL1512_COPPER_SPEC_CTRL_DIS_LINK_PULSES;
    }

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_SPECIFIC_CONTROL1_REG, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*Back to P0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     return (PASSED);
}
static int diag_gephy_soft_reset()
{
    ulong reg_val;

     /*PAGE 0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*read value of P0R0 */
     if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_CONTROL_REG, &reg_val) == FAILED) {
         printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
         return (FAILED);
     }

     reg_val |= MRV88E1512_CONTROL_PHY_RESET ;

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512C_CONTROL_REG, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*Back to P0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     return (PASSED);
}
static int diag_gephy_set_page_fa(int enable)
{
    ulong reg_val;

     /*PAGE 250*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_250) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     if (enable) {
         reg_val = MRV88E1512_REG_P250_R1;
     } else {
         reg_val = MRV88E1512_REG_P250_R1_ORI;
     }

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
             MRV88E1512_REG_PAGE_250_REG_1, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     if (enable) {
         reg_val = MRV88E1512_REG_P250_R7;
     } else {
         reg_val = MRV88E1512_REG_P250_R7_ORI;
     }

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_REG_PAGE_250_REG_7, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*Back to P0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     return (PASSED);
}
static int gephy_set_cop_pkt_gen(int enable)
{
    ulong reg_val;

     /*PAGE 6*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_6) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*read value of P6R16 */
     if (diag_smi_reg_read(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_COPPER_PORT_PKT_GEN, &reg_val) == FAILED) {
         printf("Read Register 0X%x FAIL\n", MRV88E1512M_SPECIFIC_CONTROL2_REG);
         return (FAILED);
     }

    if (enable == TRUE) {
        reg_val |= MRV88E1512_COPPER_PORT_PKT_GEN_EN_CRC;
    } else {
        reg_val &= ~MRV88E1512_COPPER_PORT_PKT_GEN_EN_CRC;
    }

     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_COPPER_PORT_PKT_GEN, reg_val) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     /*Back to P0*/
     if (diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,
         MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0) == FAILED) {
         printf("Set %s Page FAIL\n", PHY_DEVICE_NAME);
         return (FAILED);
     }

     return (PASSED);
}

/***************************************************************************
 * Function Name: diag_ncsi_gephy_pass_through_start
 * This function: set the NCSI GE PHY in pass through mode start
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 ***************************************************************************/
int diag_ncsi_gephy_pass_through_start (void)
{
    int retval = PASSED, i;
    boolean save_mgmt_gephy;

    save_mgmt_gephy = mgmt_gephy;
    mgmt_gephy = FALSE;
    
    if (gephy_if_enable(PHY_DEVICE_NAME, TRUE) == FAILED) {
        printf("Bring %s up FAIL\n", PHY_DEVICE_NAME);
	retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("1. set mode\n");
    }

    if (diag_gephy_set_mode() == FAILED) {
        printf("SET RGMII to COPPER FAIL\n");
	    retval = FAILED;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("2. force copper to 1000mbps\n");
    }

    msleep(500);

    if (diag_gephy_force_cooper_speed() == FAILED) {
        printf("FORCE COPPER SPEED FAIL\n");
	    retval = FAILED;
    }

    msleep(500);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("3. set external loopback\n");
    }

    if (gephy_enable_stub_test(TRUE) == FAILED) {
        printf("ENABLE STUB TEST FAIL\n");
	    retval = FAILED;
    }
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("4. Check Link status\n");
    }

    /* Give 2 retries before timing out */
    for (i = 0; i < 3; i++) {
	    if (gephy_check_link_status() == PASSED) {
	        break;
	    }
    }
    
    if (i == 3) {
	    printf("CHECK LINK STATUS FAIL\n");
	    retval = FAILED;
    }
	
    if(retval == FAILED){
        cterr('f', 0, "NCSI GE PHY Pass Through Start Test Failed\n");
    }
    
    mgmt_gephy = save_mgmt_gephy;
    return (retval);
}


/***************************************************************************
 * Function Name: diag_ncsi_gephy_pass_through_stop
 * This function: set the NCSI GE PHY in pass through mode stop
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 ***************************************************************************/
int diag_ncsi_gephy_pass_through_stop (void)
{
    int retval = PASSED;
    boolean save_mgmt_gephy;

    save_mgmt_gephy = mgmt_gephy;
    mgmt_gephy = FALSE;
    
    if (gephy_if_enable(PHY_DEVICE_NAME, FALSE ) == FAILED) {
        printf("Bring %s up FAIL\n", PHY_DEVICE_NAME);
	    retval = FAILED;
    }

    msleep(500);

    if (gephy_enable_stub_test(FALSE) == FAILED) {
        printf("DISABLE STUB TEST FAIL\n");
	    retval = FAILED;
    }
    
    msleep(1000);

    if(retval == FAILED){
        cterr('f', 0, "NCSI GE PHY Pass Through Stop Test Failed\n");
    }
    mgmt_gephy = save_mgmt_gephy;
    return (retval);
}

static int gephy_extl_lpbk_reg_dump(void)
{
    int reg_val;
    cterr_db_print("Dump 88E1512 external loopback test related register\n");

    reg_val = diag_gephy_reg_value_get(MRV88E1512_REG_PAGE_0,
              MRV88E1512C_CONTROL_REG);
    cterr_db_print("Page 0 Register 0: 0x%x\n", reg_val);
    
    reg_val = diag_gephy_reg_value_get(MRV88E1512_REG_PAGE_0,
              MRV88E1512C_STATUS_REG);
    cterr_db_print("Page 0 Register 1: 0x%x\n", reg_val);
    
    reg_val = diag_gephy_reg_value_get(MRV88E1512_REG_PAGE_6,
              MRV88E1512_CHECK_CONTROL_REG);
    cterr_db_print("Page 6 Register 18: 0x%x\n", reg_val);
     
    return (PASSED);
}


/*---------------------------------------------------------------
$Log: diag_gephy_test.c,v $
Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.2  2016/11/09 04:33:38  benchen2
add gephy, geswitch reg dump

Revision 1.2.14.1  2016/11/04 19:08:55  benchen2
Modify Enhanced error message

Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.14  2016/02/02 07:25:25  benchen2
add enhanced error message

Revision 1.1.2.13  2015/12/22 17:26:03  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.12  2015/10/02 08:52:32  benchen2
add 1512 extloopback flag

Revision 1.1.2.11  2015/10/02 06:46:00  benchen2
BST, remove external loopback

Revision 1.1.2.10  2015/09/23 11:21:21  benchen2
fix gephy reg test

Revision 1.1.2.9  2015/09/23 08:18:39  benchen2
modify read only bit

Revision 1.1.2.8  2015/09/17 02:39:39  benchen2
add cterr

Revision 1.1.2.7  2015/09/15 06:50:56  benchen2
fix not define reval

Revision 1.1.2.6  2015/09/15 01:24:34  benchen2
add cterr

Revision 1.1.2.5  2015/09/14 07:30:43  benchen2
phy 1512 utility

Revision 1.1.2.4  2015/09/14 07:09:47  benchen2
phy1512 lpbk test

Revision 1.1.2.3  2015/08/04 02:27:25  hondwang
add lpbk test

Revision 1.1.2.2  2015/07/31 07:31:45  hondwang
gephy test

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/
