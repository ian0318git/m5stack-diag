/* $Id: mb_tests.c,v 1.43 2018/08/30 06:59:43 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
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
#include "slot.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "mon_plat_defs.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "platform_env.h"
#include "platform_pwr_seq.h"
#include "proto.h"
#include "platform_psu.h"
#include "linux_usb_test.h"
#include "dash_fpga.h"
#include "linux_api.h"
#include "platform_fru.h"
#include "cli_cmd.h" /* show margining */

/* M/B test flag defines */
#define MF_1	(MF_CONTINUOUS | MF_DOGRP)
#define MF_2	(MF_1 | MF_DOALL)
#define MF_3	(MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4	(MF_1 | MF_SHOW_ERRCOUNT)

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/* #define BYPASS_ENV  * */
extern int  linux_memory_tester(int);
extern int  linux_memory_tester_with_ecc_check(int);
extern int test_not_avail_yet (int);
extern int do_all_menu_items(struct menuinfo *);
extern int auxloopback(void);
extern int smartchip_authenticate_retest(uchar, uchar);
extern int smartchip_authenticate(uchar, uchar);
extern int mainmem_test(void);
extern int mb_io_fpga_test(void);
extern int mb_io_fpga_intr_test(void);
extern int spi_flash_test(boolean menu_option);
extern int compactflash_main(int);
extern int check_thermal_profile(int);
extern int cs_keys_tests(int);

extern int ovld_x86_i2c_scan_test(int);
extern int dash_rd_wr_test(int);

extern int gesw_test_main (int show_menu);
extern int eth_port_test_main(int);
extern int ctrl_plane_sgmii_ext_lpbk_test(void);
extern int ovld_pcie_10prbs_cdr_int_lpbk_test(void);
extern int ovld_pcie_8prbs_cdr_int_lpbk_test(void);
extern int pfix_empty_test(void);
extern int force_skip_ext_aux(void);
extern int force_skip_usb(int);
extern int force_skip_eusb(void);
extern int force_skip_msata(void);
extern int hts_tests(int show);
extern int generic_ovld_show_pcieinfo(void);
extern int gb_bootflash_test(void);
static int aux_loopback_test(int dummy);
static int aux_port_test(void);
static int usb_tests(int);
static int msata_tests(int);
static int eusb_tests(int);
static int emmc_tests(int);
static int dash_rd_wr_test_wrap(int);
static int pcie_lane_scan_wrap(void); 
extern int cpu_core_test(void); 

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar mb_phy_id[] = "Marvell 1548/1340";
uchar mb_sfp_id[] = "SFP-ID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";
uchar psu_pid[] = "PSU-PID";
uchar rps_pid[] = "RPS-PID";
uchar pvdm_pid[] = "PVDM-PID";
uchar backplane_pid[] = "Backplane-PID";
uchar risercard_pid[] = "RiserCard-PID";
uchar sm_pid[] = "SM-PID";
uchar wic_pid[] = "WIC-PID";
uchar dc_pid[] = "DC-PID";
uchar mb_loc[] = "MB";
uchar mb_xaui_loc[] = "MB_XAUI";
uchar mb_aux_loc[] = "MB_AUX";
uchar mb_i2c_loc[] = "MB/FPGA/I2C & MB/Rangeley/I2C";
uchar mb_fpga_reg_loc[] = "MB/FPGA-REG";
uchar mb_emmc_loc[] = "MB/eMMC";
uchar mb_eusb_loc[] = "MB/eUSB";
uchar mb_usb0_loc[] = "MB/USB0";
uchar mb_msata_loc[] = "MB/mSATA";
uchar mb_phy_loc[] = "MB/PHY";
uchar mb_sfp_loc[] = "MB/SFP";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar sfp0_loc[] = "MB/SFP0";
uchar sfp1_loc[] = "MB/SFP1";
uchar sfp2_loc[] = "MB/SFP2";
uchar sfp3_loc[] = "MB/SFP3";
uchar psu0_loc[] = "MB/PSU0";
uchar psu1_loc[] = "MB/PSU1";
uchar rps_loc[] = "MB/RPS";
uchar pvdm0_loc[] = "MB/PVDM0";
uchar backplane_loc[] = "MB/Backplane";
uchar risercard_loc[] = "MB/RiserCard";
uchar sm0_loc[] = "MB/SM0";
uchar sm1_loc[] = "MB/SM1";
uchar wic0_loc[] = "MB/WIC0";
uchar wic1_loc[] = "MB/WIC1";
uchar wic2_loc[] = "MB/WIC2";
uchar sm0wic_loc[] = "SM0/WIC";
uchar sm1wic_loc[] = "SM1/WIC";
uchar sm0pvdm0_loc[] = "SM0/PVDM0";
uchar sm0pvdm1_loc[] = "SM0/PVDM1";
uchar sm0pvdm2_loc[] = "SM0/PVDM2";
uchar sm1pvdm0_loc[] = "SM1/PVDM0";
uchar sm1pvdm1_loc[] = "SM1/PVDM1";
uchar sm1pvdm2_loc[] = "SM1/PVDM2";
uchar sm0wic0dc_loc[] = "SM0/WIC0/DC";
uchar sm1wic0dc_loc[] = "SM1/WIC0/DC";
uchar sm0wic1dc_loc[] = "SM0/WIC1/DC";
uchar sm1wic1dc_loc[] = "SM1/WIC1/DC";
uchar sm0dc_loc[] = "SM0/DC";
uchar sm1dc_loc[] = "SM1/DC";


fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { mb_pid,        mb_xaui_loc },
    { mb_pid,        mb_aux_loc },
    { mb_pid,        mb_i2c_loc},
    { mb_pid,        mb_fpga_reg_loc},
    { mb_pid,        mb_emmc_loc},
    { mb_pid,        mb_eusb_loc},
    { mb_pid,        mb_usb0_loc},
    { mb_pid,        mb_msata_loc},
    { mb_phy_id,     mb_phy_loc },
    { mb_sfp_id,     mb_sfp_loc },
    { dimm_pid,      dimm0_loc },
    { dimm_pid,      dimm1_loc },
    { sfp_pid,       sfp0_loc },
    { sfp_pid,       sfp1_loc },
    { sfp_pid,       sfp2_loc },
    { sfp_pid,       sfp3_loc },
    { psu_pid,       psu0_loc },
    { psu_pid,       psu1_loc },
    { rps_pid,       rps_loc },
    { pvdm_pid,      pvdm0_loc },
    { backplane_pid, backplane_loc },
    { risercard_pid, risercard_loc },
    { sm_pid,        sm0_loc },
    { sm_pid,        sm1_loc },
    { wic_pid,       wic0_loc },
    { wic_pid,       wic1_loc },
    { wic_pid,       wic2_loc },
    { wic_pid,       sm0wic_loc },
    { wic_pid,       sm1wic_loc },
    { pvdm_pid,      sm0pvdm0_loc },
    { pvdm_pid,      sm0pvdm1_loc },
    { pvdm_pid,      sm0pvdm2_loc },
    { pvdm_pid,      sm1pvdm0_loc },
    { pvdm_pid,      sm1pvdm1_loc },
    { pvdm_pid,      sm1pvdm2_loc },
    { dc_pid,        sm0wic0dc_loc },
    { dc_pid,        sm1wic0dc_loc },
    { dc_pid,        sm0wic1dc_loc },
    { dc_pid,        sm1wic1dc_loc },
    { dc_pid,        sm0dc_loc },
    { dc_pid,        sm1dc_loc },
};
/* 
 * Sub Menu used for Motherboard tests.
 */
submenu_xtable_t mb_tests_submenu_table[] = {
    {"Main memory test with cache on",
     (PFT)linux_memory_tester,	FALSE,		MF_2,
     (type_t(*)())0, 0,		(PFT)linux_memory_tester,	TRUE},

    {"mSATA test",
        (PFT)msata_tests,  0,                   MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"USB 0 (/dev/usbdrv0) test",
        (PFT)usb_tests, 0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0, 0},

    {"USB 1 (/dev/usbdrv1) test",
        (PFT)usb_tests, 1,                      MF_3,
        (type_t(*)())is_utah,         0,
        (PFT)0, 0},

    {"eUSB test",
        (PFT)eusb_tests,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"eMMC 0 test",
        (PFT)emmc_tests,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"I2C scan test",
        (PFT)ovld_x86_i2c_scan_test,   0,       MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},
    
    {"DASH FPGA register test",
        (PFT)dash_rd_wr_test_wrap,      0,      MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"GESW tests",
       (PFT)gesw_test_main,	TRUE,		MF_3,
       (type_t(*)())0, 0,		
       (PFT)gesw_test_main,	FALSE},

    {"Control Plane CPU SGMII same port lpbk at GESW test",
        (PFT)ctrl_plane_sgmii_ext_lpbk_test,   0,  MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"AUX loopback test",
     (PFT)aux_loopback_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},
    /* require 3 wire loopback connector to test modem
       control signals (RTS/CTS & DTS/DTR). */
    {"AUX RTS/CTS & DTS/DTR connectivity test",
     (PFT)aux_port_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},
    {"GE and SFP ext/internal loopback tests",
       (PFT)eth_port_test_main,      TRUE,      MF_3,
       (type_t(*)())0, 0,
       (PFT)eth_port_test_main,      FALSE},
    {"FPGA XAUI Interface",
     (PFT)hts_tests,		FALSE,		MF_3,
     (type_t(*)())0, 0,		(PFT)hts_tests,	TRUE},
    {"PCIe Lane Check",
     (PFT)pcie_lane_scan_wrap, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},
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
 * Sub Menu used for Goldbeach Motherboard tests.
 */
submenu_xtable_t gb_mb_tests_submenu_table[] = {
    {"Main memory test with cache on ECC checking",
     (PFT)linux_memory_tester_with_ecc_check,	FALSE,		MF_2,
     (type_t(*)())0, 0,		(PFT)linux_memory_tester_with_ecc_check,	TRUE},

    {"mSATA test",
        (PFT)msata_tests,  0,                   MF_3,
        (type_t(*)())is_vg400,         0,
        (PFT)0,  0},

    {"USB 0 (/dev/usbdrv0) test",
        (PFT)usb_tests, 0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0, 0},
    {"eMMC 0 test",
        (PFT)emmc_tests,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"I2C scan test",
        (PFT)ovld_x86_i2c_scan_test,   0,       MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},
    
    {"DASH FPGA register test",
        (PFT)dash_rd_wr_test_wrap,      0,      MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},
    {"GE and SFP ext/internal loopback tests",
       (PFT)eth_port_test_main,      TRUE,      MF_3,
       (type_t(*)())0, 0,
       (PFT)eth_port_test_main,      FALSE},
   {"PCIe Lane Check",
     (PFT)pcie_lane_scan_wrap, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},
   {"Boot Flash Test",
     (PFT)gb_bootflash_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},
   {"CPU core test", 
       (PFT)cpu_core_test ,     0,      MF_3,
       (type_t(*)())0,   0,
       (type_t(*)())0,   0},
};

#define GB_MB_TESTS_SUBMENU_TABLE_SIZE (sizeof(gb_mb_tests_submenu_table) / \
                                     sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gb_mb_tests_primary_items[GB_MB_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];
static mitem_t gb_mb_tests_secondary_items[GB_MB_TESTS_SUBMENU_TABLE_SIZE +
						MAX_BASE_ITEMS];

menuinfo_t gb_mb_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    gb_mb_tests_primary_items,
};
menuinfo_t *gb_mb_submenup = &gb_mb_subtest_menu;

/* 
 * Sub Menu used for I/O Interface tests.
 */

submenu_xtable_t io_tests_submenu_table[] = {
    {"SM interface test",
     sm_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0,		0,	(type_t(*)())0,	0},
    {"WIC interface test",
     wic_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0,		0,	(type_t(*)())0,	0},
    {"VM interface test",
     vm_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0,		0,	(type_t(*)())0,	0},

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

static void
display_no_reg (void)
{
    cterr_db_print("This item has no reg to display\n"); 
}

static void
display_env (void)
{
    show_margins_x(0, CLI_MODE);
}

static void
display_uart_reg (void)
{
    /* cterr_db_print("Display UART regs:\n"); */
    display_uart_regs(0);
}

static void
add_aux_port_test_err_report (void)
{
    fru_table_offset = MB_AUX;
    platform_fru_table[MB_AUX].pid_string = mb_pid;
    platform_fru_table[MB_AUX].location_string = mb_aux_loc;
    cterr_add_component("MB", "UART", "AUX");
    cterr_add_reg_dump((PFV)display_uart_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether aux connector is plugged correctly",
            "Please try to replace the aux plug with another 3-wires loopback plug",
            "Please try to use uart utility to make sure uart is working");
}

static void
add_aux_loopback_err_report (void)
{
    fru_table_offset = MB_AUX;
    platform_fru_table[MB_AUX].pid_string = mb_pid;
    platform_fru_table[MB_AUX].location_string = mb_aux_loc;
    cterr_add_component("MB", "UART", "AUX");
    cterr_add_reg_dump((PFV)display_uart_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether aux connector is plugged correctly",
            "Please try to replace the aux plug with another one",
            "Please try to use uart utility to make sure uart is working");
}

static void
display_fpga_reg (void)
{
    cterr_db_print("Please use FPGA read/alter utilites in FPGA utility to read/write the FPGA regs\n");
    return;
}

static int
dash_rd_wr_test_wrap (int flag)
{
    int ret;

    fru_table_offset = MB_FPGA_REG;
    platform_fru_table[MB_FPGA_REG].pid_string = mb_pid;
    platform_fru_table[MB_FPGA_REG].location_string = mb_fpga_reg_loc;
    cterr_add_component("MB", "FPGA-REG");
    cterr_add_reg_dump((PFV)display_fpga_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please verify FPGA image version",
            "FPGA might be broken");

    ret = dash_rd_wr_test(flag);
    return ret;
}

static void
add_emmc_err_report (void)
{
    fru_table_offset = MB_EMMC;
    platform_fru_table[MB_EMMC].pid_string = mb_pid;
    platform_fru_table[MB_EMMC].location_string = mb_emmc_loc;
    cterr_add_component("MB", "eMMC");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether eMMC is plugged",
            "Please try to use another eMMC");
}

static void
add_eusb_err_report (void)
{
    fru_table_offset = MB_EUSB;
    platform_fru_table[MB_EUSB].pid_string = mb_pid;
    platform_fru_table[MB_EUSB].location_string = mb_eusb_loc;
    cterr_add_component("MB", "eUSB");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether eUSB slot is not vacant",
            "Please try to use another eUSB device");
}
static void
add_usb_err_report (void)
{
    fru_table_offset = MB_USB0;
    platform_fru_table[MB_USB0].pid_string = mb_pid;
    platform_fru_table[MB_USB0].location_string = mb_usb0_loc;
    cterr_add_component("MB", "USB0");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether USB slot is not vacant",
            "Please try to use another USB device");
}

static void
add_msata_err_report (void)
{
    fru_table_offset = MB_MSATA;
    platform_fru_table[MB_MSATA].pid_string = mb_pid;
    platform_fru_table[MB_MSATA].location_string = mb_msata_loc;
    cterr_add_component("MB", "mSATA");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Please check whether mSATA slot is not vacant",
            "Please try to use another mSATA device");
}

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
int 
mb_tests (boolean mb_test_items_executed)
{
    if ((!is_goldbeach()) && (!is_vg400())) {
        build_primary_submenu(mb_tests_submenu_table, MB_TESTS_SUBMENU_TABLE_SIZE,
    			    "Motherboard", &mb_submenup);
        build_secondary_submenu(mb_tests_submenu_table,
    			    MB_TESTS_SUBMENU_TABLE_SIZE,
    			    mb_tests_secondary_items);
        if (mb_test_items_executed) {
            do_all_menu_items(&mb_subtest_menu);
    	if (diagflag_xram & D_XEC_AUTH) /* For MFG */
    	    smartchip_authenticate_retest(MOTHER_BOARD, 0);
#ifdef AUTHENTICATION_TEST_Y 
    	else if (diagflag_yram & D_AUTH_Y) /* For EDVT with retry */
    	    smartchip_authenticate(MOTHER_BOARD, 0);
#endif /* AUTHENTICATION_TEST_Y	*/
        } else {
            menu(&mb_subtest_menu, mb_tests_secondary_items, '\0');
        }
    } else { /* Build Goldbeach Motherboard Tests Menu */
        build_primary_submenu(gb_mb_tests_submenu_table, GB_MB_TESTS_SUBMENU_TABLE_SIZE,
    			    "Motherboard", &gb_mb_submenup);
        build_secondary_submenu(gb_mb_tests_submenu_table,
    			    GB_MB_TESTS_SUBMENU_TABLE_SIZE,
    			    gb_mb_tests_secondary_items);
        if (mb_test_items_executed) {
            do_all_menu_items(&gb_mb_subtest_menu);
    	if (diagflag_xram & D_XEC_AUTH) /* For MFG */
    	    smartchip_authenticate_retest(MOTHER_BOARD, 0);
#ifdef AUTHENTICATION_TEST_Y 
    	else if (diagflag_yram & D_AUTH_Y) /* For EDVT with retry */
    	    smartchip_authenticate(MOTHER_BOARD, 0);
#endif /* AUTHENTICATION_TEST_Y	*/
        } else {
            menu(&gb_mb_subtest_menu, gb_mb_tests_secondary_items, '\0');
        }
    }
    return PASSED;
}

/*
 * Function:	io_iface_tests
 * Description:	First build the primary & secondary submenus 
 *		for the i/o interface diags based on the 
 *		_xtable_ io_tests_submenu_table.  If the given
 * 		arg is FALSE, execute all the tests in the menu 
 *		and return the result.  Otherwise, present the 
 *		menu to the user for interaction.
 * Inputs:	io_test_items_executed: TRUE/FALSE
 * Output:	PASS
 */
int
io_iface_tests (int io_test_items_executed)
{
    build_primary_submenu(io_tests_submenu_table, IO_TESTS_SUBMENU_TABLE_SIZE,
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


/*
 * Function:	aux_uart_write
 * Description:	write to aux uart register
 * Inputs:	offset: uart register offset and val: value to be written 
 * Output:	PASS
 */
static int
aux_uart_write(off_t offset, char val)
{   
    int fdio;
    char buf = val;
    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }
    
    assert(fdio);

    lseek(fdio, (offset + AUX_UART_UIO_BASE), SEEK_SET);
    write(fdio, &buf, 1);
    usleep(100);
    close(fdio);
    return PASSED;
}   

/*
 * Function:	aux_uart_read
 * Description:	read from aux uart register
 * Inputs:	offset: uart register offset and buf: value is returned to this buffer
 * Output:	PASS
 */
static int
aux_uart_read(off_t offset, char *buf)
{   
    int fdio;
    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }

    assert(fdio);

    lseek(fdio, (offset + AUX_UART_UIO_BASE), SEEK_SET);
    read(fdio, buf, 1);
    close(fdio);
    usleep(5000);
    return PASSED;
}   

/*
 * Function:	aux_loopback
 * Description:	do aux internal loopback test 
 * Inputs:	baud: set the uart baud rate
 *              int_lpbk; 1 if internal loopback is on, 0 otherwise.
 * Output:	PASS, or FAIL
 */
int 
aux_loopback (int baud, int int_lpbk)
{
    char test_str[] = "1234567890ABCDE";
    int i, ret = PASS;
    char read_buf;
    unsigned int quot;
    char recv_str[256] = {0};

    if (baud <= 0)
        return FAILED;

    quot = 50000000 / baud;

    /* aux uart config */
    aux_uart_write(UART_FCR, 0xc6); /* reset fifo */
    aux_uart_write(UART_LCR, 0x83); /* turn on divisor latch access bit */
    aux_uart_write(UART_DLL, (quot & 0xFF)); /* rate 9600: 0x1458. DLL: 0x58 */
    aux_uart_write(UART_DLM, (quot & 0xFF00 >> 8)); /* 9600: 0x1458 DLM: 0x14 */
    aux_uart_write(UART_LCR, 0x3); /* turn off divisor latch access bit */
    aux_uart_write(UART_FCR, 0x1); /* enable fifo and 1 byte trigger level */
    if (int_lpbk)
        aux_uart_write(UART_MCR, 0x10); /* turn on loopback mode */

    /* feed data */
    for (i=0; i<strlen(test_str); i++) {
        aux_uart_write(UART_TX, test_str[i]);
    }

    msleep(100);
    /* read data */
    for (i=0; i<strlen(test_str); i++) {
        aux_uart_read(UART_LSR, &read_buf);
        if (read_buf & 0x1) {
            aux_uart_read(UART_RX, &read_buf);
            recv_str[i] = read_buf; 
            if (read_buf != test_str[i]) {
                ret = FAIL;
                goto test_done;
            }
        } else { 
            /* should have some more but found no more data in buf*/
            ret = FAIL;
            goto test_done;
        }
    }
    
test_done:
    aux_uart_write(UART_MCR, 0); /* turn off loopback mode */
    aux_uart_write(UART_FCR, 0xc6); /* reset fifo */
    if (ret == FAIL) {
        cterr_db_print("rx/tx string differ [rx = %s] [tx = %s].", recv_str, test_str);
    }
    return ret;
}

/*
 * Function:	aux_port_test
 * Description:	do aux port write/read test
 * Inputs: -
 * Output: PASS
 */
static int
aux_port_test (void)
{
    off_t wr_off = 0x2FC; /* write offset */
    off_t rd_off = 0x2FE; /* read offset  */
    char wr_comp, rd_comp;
    char wr_buf, rd_buf, data;
    int ix, ret = FAILED;
    int rd_shift_bit, wr_shift_bit;
    int fdio;

    char *tname = "AUX port test";

    if (get_enhance_err_flag()) {
        add_aux_port_test_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK) || force_skip_ext_aux()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off or force skip usb is enabled. ");
        return(PASSED);
    }

    fdio = open("/dev/port", O_RDWR | O_NDELAY);
    if (fdio < 0) {
        perror("can't open /dev/port");
        return FAILED;
    }

    assert(fdio);

    /* 
     * Note: delay after read/write is required for the value to be updated
     */
    for (ix = 0; ix < 0x2; ix++) { /* test value: 00 and 11 */
        wr_buf = 0;
        rd_buf = 0;

        lseek(fdio, wr_off, SEEK_SET);
        read(fdio, &wr_buf, 1);
        usleep(5000);
        /* printf("wr buf before write: 0x%X\n", wr_buf); */
    
        /* aux port test only modify bit#0 and/or bit#1 */
        data = 0x03; /* modify both bit#0 and bit#1 */
        wr_buf  = (ix == 0) ?  wr_buf & ~(data) : wr_buf | data; 
        /* printf("wr buf to be written: 0x%X\n", wr_buf); */

        lseek(fdio, wr_off, SEEK_SET);
        write(fdio, &wr_buf, 1);
        usleep(5000);

        lseek(fdio, rd_off, SEEK_SET);
        read(fdio, &rd_buf, 1);
        usleep(5000);
        /* printf("rd buf after write: 0x%X\n", rd_buf); */
    
        /* 
         * wr_shift_bit:
         * if bit#0 and bit#1 are written it should be shifted by 0,
         * if only bit#0 is written, it should be shifted by 0,
         * if only bit#1 is written, it should be shifted by 1.
         */
        wr_shift_bit = 0;
        wr_comp = (wr_buf & (data << wr_shift_bit)) >> wr_shift_bit;
        /* 
         * rd_shift_bit:
         * if bit#0 and bit#1 are written it should be shifted by 4,
         * if only bit#0 is written, it should be shifted by 5,
         * if only bit#1 is written, it should be shifted by 4.
         */
        rd_shift_bit = 4;
        rd_comp = (rd_buf & (data << rd_shift_bit)) >> rd_shift_bit;
        /* printf(" wr %d, rd %d\n", wr_comp, rd_comp); */

        if (wr_comp == rd_comp) {
            ret = PASSED;
        } else {
            ret = FAILED;
            goto fun_end;
        }

    }

fun_end:
    close(fdio);

    if (ret == PASSED) {
        cterr_db_print("Test passed!");
    } else {
        cterr('f', 0, "Read data not match [wr byte= 0x%02X, rd byte = 0x%02X]. Test failed! Is 3-wires loopback connector installed?\n", wr_buf, rd_buf);
    }
    return ret;
}

/*
 * Function: aux_loopback_test
 *
 * Description : Aux loopback test. support external only . no  internal
 *               lpbbk.
 * Inputs: dummy - not used
 *
 * Output: PASSED/FAILED
 */
int
aux_loopback_test (int dummy)
{
    char *tname = "AUX port loopback";

    if (get_enhance_err_flag()) {
        add_aux_loopback_err_report();
    }

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    
    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK) || force_skip_ext_aux()) {
        if (aux_loopback(9600, 1) != PASSED) {
            cterr('f', 0, "AUX internal loopback failed.");
            return(FAILED);
        }
        return(PASSED);
    }

    if (aux_loopback(9600, 0) != PASSED) {
        cterr('f', 0, "AUX external failed. Is loopback connector installed?");
        return(FAILED);
    }
    
#ifdef UART_INTF_TEST
    if (uart_intf_test("/dev/ttyS1", NULL, B9600) != PASSED) {
        cterr('f', 0, "AUX loopback failed. Is loopback connector installed?");
        return(FAILED);
    }
#endif
    return(PASSED);
}

/*
 * Function: usb_tests
 *
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
static int usb_tests (int slot) {

    int rc = FAILED;
    char *tname = "USB slot";

    if (get_enhance_err_flag()) {
        add_usb_err_report();
    }

    testname("%s%d access", tname, slot);
    prpass(testpass, "%s%d, ", tname, slot);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK) || force_skip_usb(slot)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off or force skip usb is enabled. ");
        return(PASSED);
    }

    /* testname is printed on usb_slot_tests */
    rc = usb_slot_tests(slot);
    if (rc == FAILED) {
        cterr('f',0, "usb %d test failed.", slot);
    }

    return(rc);
}

/*
 * Function: msata_tests
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int msata_tests (int dummy) {

    int rc = FAILED;
    char *tname = "mSATA read/write";
    
    if (get_enhance_err_flag()) {
        add_msata_err_report();
    }

    /* so far we have only one mSATA */
    testname("%s access", tname);
    prpass(testpass, "%s, ", tname);
    
    if (force_skip_msata()) {
        prpass(testpass, "Test skipped. Force skip msata is enabled. ");
        return (PASSED);
    }

    /* FPGA 0x320A0 bit 4 (SATA HDD 1 Present) for Utah mSATA present */
    if(!is_sata_present(SATA_NUM_ONE)) {
        cterr('f',0, "mSATA is not present, slot vacant");
        return (PASSED);
    }

    rc = msata_slot_tests(dummy);
    if (rc == FAILED) {
        cterr('f',0, "mSATA test failed.");
    }

    return(rc);
}

/*
 * Function: eusb_tests
 *
 * Description : eUSB r/w tests.
 *
 * Inputs: slot - eUSB slot num
 *
 * Output: PASSED/FAILED
 */
static int eusb_tests (int slot) {

    int rc = FAILED;
    char *tname = "eUSB";

    if (get_enhance_err_flag()) {
        add_eusb_err_report();
    }

    testname("%s access", tname);
    prpass(testpass, "%s, ", tname);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK) || force_skip_eusb()) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off or force skip eUSB is enabled.");
        return(PASSED);
    }

    /* testname is printed on usb_slot_tests */
    rc = eusb_slot_tests(slot);
    if (rc == FAILED) {
        cterr('f',0,"eUSB test failed.");
    }

    return(rc);
}

/*
 * Function: emmc_tests
 *
 * Description : emmc r/w tests.
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int emmc_tests (int dummy) {

    int rc = FAILED;
    char *tname = "emmc0";
    
    if (get_enhance_err_flag()) {
        add_emmc_err_report();
    }

    /* we only have one emmc */
    testname("%s access", tname);
    prpass(testpass, "%s, ", tname);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return(PASSED);
    }

    /* testname is printed on usb_slot_tests */
    rc = emmc_slot_tests(dummy);
    if (rc == FAILED) {
        cterr('f',0,"emmc0 test failed.");
    }

    return(rc);
}

/*
 * Function: pcie_lane_scan_wrap
 *
 * Description : pcie lane scan test
 *
 * Inputs: NONE
 *
 * Output: PASSED/FAILED
 */
static int pcie_lane_scan_wrap (void) {

    testname("PCIe Lane Scan");

    if (generic_ovld_show_pcieinfo() == FAILED) {
        cterr('f',0,"PCIe lanes scan failed!!\n");
        return (FAILED);
    }

    return (PASSED);
}

/******** History ******** 
$Log: mb_tests.c,v $
Revision 1.43  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.42  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.41  2014/08/11 10:37:52  danchung
1. in linux_main.c call cterr to print WARNING if pcie lane check fail
2. in motherboard test call cterr to print ERROR if pcie lane check fail

Revision 1.40  2014/08/07 11:11:09  danchung
add pcie lane check in motherboard test submenu

Revision 1.39  2014/07/29 17:46:00  mcharon
add xaui test under motherboard test

Revision 1.38  2014/05/21 22:51:02  mcharon
bring back aux port 3 wiret

Revision 1.37  2014/05/21 01:57:04  mcharon
remove modem control signal test for now...manufacturing not ready yet

Revision 1.36  2014/04/25 10:04:22  hroni
add aux_port_test()

Revision 1.35  2014/02/21 07:39:59  hroni
add enchance err msg for emmc, eusb, usb, and msata tests

Revision 1.34  2014/02/21 06:53:44  hroni
add enhance error messages for i2c scan test, aux loopback test, and dash fpga register test

Revision 1.33  2014/02/19 09:11:34  alpeng
suport enhanced error code on loobpack tests

Revision 1.32  2014/02/18 09:11:11  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.31  2014/02/10 07:13:31  hroni
add -d msata and -d eusb to exclude msata and eusb, correspondingly.

Revision 1.30  2014/02/07 07:03:13  alpeng
using error instead of warning when mSATA is not present

Revision 1.29  2014/01/29 01:26:17  mcharon
use 5000 for usleep insde aux_uart_read; aux loopbackuart_intf_test with aux_loopback

Revision 1.28  2013/12/26 02:33:58  hroni
fixed typo

Revision 1.27  2013/12/24 05:59:00  hroni
1. enhance aux_internal_loopback test debug message. 2. rename related parameters

Revision 1.26  2013/12/23 04:21:43  hroni
1. add force skip usb and aux. 2. add aux internal loopback test

Revision 1.25  2013/12/12 09:09:41  hroni
remove pfix message for aux loopback test is ready

Revision 1.24  2013/12/04 21:34:46  ptong
Fixed typo

Revision 1.23  2013/12/04 18:53:55  mcharon
add entry for xaui to fru table

Revision 1.22  2013/11/15 10:12:07  danchung
Remove "USB1 test" menu item for Sword and Dagger

Revision 1.21  2013/11/13 11:06:27  danchung
Change "cf test" to "eUSB test" for Utah

Revision 1.20  2013/11/07 00:53:50  danchung
Add emmc0 test.

Revision 1.19  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.18  2013/09/09 05:49:09  ptong
Replace cavecreek_sgmii_ext_lpbk_test with ctrl_plane_sgmii_ext_lpbk_test

Revision 1.17  2013/09/05 01:58:27  alpeng
support mSATA test on Utah

Revision 1.16  2013/08/24 00:51:00  ptong
Minor change and clean up for 0.2.0 release

Revision 1.15  2013/08/22 18:28:00  ptong
Adjust menu items

Revision 1.14  2013/08/13 00:07:06  hroni
support Rangeley control plane SGMII to GE same port loopback test

Revision 1.13  2013/08/12 08:31:25  alpeng
display the test name for usb, aux and cf test before testing

Revision 1.12  2013/08/08 21:54:53  hroni
*** empty log message ***

Revision 1.11  2013/08/01 03:26:59  danchung
Eliminate PLL test from Utah diag menu.

Revision 1.10  2013/07/22 20:42:41  ptong
Minor changes

Revision 1.9  2013/07/16 01:33:38  ptong
Minor change on funcition names

Revision 1.8  2013/07/03 23:50:20  ptong
Modify for proper init and menu setup

Revision 1.7  2013/06/24 08:39:24  alpeng
support Ext. loopback flag checking before USB/CF diag

Revision 1.6  2013/06/20 07:46:57  alpeng
add function wrapper for usb/cf test

Revision 1.5  2013/06/14 10:22:23  alpeng
follow O2 menu structure

Revision 1.4  2013/05/31 11:03:41  alpeng
support front panel GE loopback test

Revision 1.3  2013/05/14 03:09:39  hroni
fix compile error

Revision 1.2  2013/05/09 07:37:15  alpeng
updating files

Revision 1.1  2013/05/09 05:52:59  alpeng
add utah tree

Revision 1.28  2013/05/01 20:39:58  mcharon
let 10bit prbs run in submenu only

Revision 1.27  2013/04/15 21:15:01  mcharon
remove idt switch internal loopback from motherboard test

Revision 1.26  2013/02/15 10:31:55  palin2
Update UART test by add a new parameter to allow using specific baud rate.

Revision 1.25  2012/11/17 02:25:24  palin2
Add PLL test support.

Revision 1.24  2012/11/07 18:21:17  mcharon
cleanup

Revision 1.23  2012/10/25 21:37:47  mcharon
use cterr instead of printf for auxtest. always run test regardless of ext_lpbk flag

Revision 1.22  2012/10/25 06:15:42  mcharon
support ext lpbk flag for aux test

Revision 1.21  2012/09/19 22:42:52  palin2
Rename "FPGA I2C scan test" to "I2C scan test".

Revision 1.20  2012/09/19 07:29:02  palin2
1. Add "PCIe Switch 10-bit PRBS Master Internal loopback test"
   and related debug utilities support in Overlord Diag.
2. Add "PCIe 10-bit PRBS Master External Loopback test" and
   related debug utilities support at NGSM TestCard side.

Revision 1.19  2012/09/17 22:00:39  ptong
Add testname for AUX loopback test

Revision 1.18  2012/09/15 01:22:47  ptong
Set diag menu items with MF_SHOW_ERRCOUNT flag

Revision 1.17  2012/09/14 17:16:30  mcharon
add cterr in aux_loopback_test

Revision 1.16  2012/09/14 00:28:20  mcharon
add i/o interface

Revision 1.15  2012/09/12 09:21:13  alpeng
remove SATA test from mbtest and integrate SATA test into ngwic3 test

Revision 1.14  2012/08/29 07:45:37  alpeng
add cterr for fail case on uart_intf_test()

Revision 1.13  2012/08/29 06:20:09  alpeng
fixed fail case for return value of uart_intf_test()

Revision 1.12  2012/08/22 09:32:40  alpeng
supporting uart test for cavium

Revision 1.11  2012/07/27 17:10:15  mcharon
move uart test to common code

Revision 1.10  2012/07/25 19:33:12  mcharon
handle case when aux loopback fails so test won't hang

Revision 1.9  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.8  2012/05/16 07:21:51  ptong
Code clean up

Revision 1.7  2012/05/16 02:48:53  alpeng
modified HDD present function for displaying MB menu

Revision 1.6  2012/05/11 08:13:27  alpeng
support HDD present to MB test menu

Revision 1.5  2012/05/10 07:48:05  alpeng
moving DASH FPGA reg test to MB test, fixed printing message

Revision 1.4  2012/05/09 08:28:14  alpeng
moving FPGA I2C scan test to MB test menu

Revision 1.3  2012/05/07 08:10:10  alpeng
fixed message, remove prcomplete

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
