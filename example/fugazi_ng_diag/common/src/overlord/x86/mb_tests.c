/* $Id: mb_tests.c,v 1.42 2014/04/25 07:12:16 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/x86/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2008-2014 by Cisco Systems, Inc.
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
#include "setjmps.h"
#include "platform_env.h"
#include "platform_pwr_seq.h"
#include "proto.h"
#include "platform_psu.h"
#include "linux_usb_test.h"
#include "dash_fpga.h"
#include "linux_api.h"
#include "platform_fru.h"

/* M/B test flag defines */
#define MF_1	(MF_CONTINUOUS | MF_DOGRP)
#define MF_2	(MF_1 | MF_DOALL)
#define MF_3	(MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4	(MF_1 | MF_SHOW_ERRCOUNT)

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/*
 * Global extern functions
 */
extern void display_uart_regs_cterr_wrapper(void);
extern int display_multiboot(int);
extern void show_margins_cterr_wrapper(void);
extern void show_temp_cterr_wrapper(void);	
extern int get_mb_pid(char *);

/* #define BYPASS_ENV  * */
extern int  linux_memory_tester(int);
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
extern int cavecreek_sgmii_ext_lpbk_test(void);
extern int cavecreek_sgmii_port_test(void);
extern int ovld_pcie_10prbs_cdr_int_lpbk_test(void);
extern int ovld_pcie_8prbs_cdr_int_lpbk_test(void);
extern int build_ovld_pll_menu(int);
extern boolean is_overlord(void);
extern int check_menu_flag(uint);

static int aux_loopback_test(int dummy);
static int usb_tests(int);
static int cf_eusb_tests(int);

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
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

    {"USB 0 (/dev/usb0) test",
   	(PFT)usb_tests,	0,	         	MF_3,
	(type_t(*)())0,		0,
	(PFT)usb_tests,	0},

    {"USB 1 (/dev/usb1) test",
   	(PFT)usb_tests,	1,	        	MF_3,
	(type_t(*)())0,		0,
	(PFT)usb_tests,	1},

    {"Bootflash (/dev/cf or /dev/eusb) test",
   	(PFT)cf_eusb_tests,	0,	        	MF_3,
	(type_t(*)())0,		0,
	(PFT)cf_eusb_tests,	0},

    {"I2C scan test",
        (PFT)ovld_x86_i2c_scan_test,   0,       MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},
    
    {"DASH FPGA register test",
        (PFT)dash_rd_wr_test,      0,           MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"GESW tests",
       (PFT)gesw_test_main,	TRUE,		MF_3,
       (type_t(*)())0, 0,		
       (PFT)gesw_test_main,	FALSE},

    {"Cavecreek SGMII same port lpbk at GESW test",
        (PFT)cavecreek_sgmii_ext_lpbk_test,   0,  MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"Cavecreek SGMII 2-port lpbk at GESW test",
     (PFT)cavecreek_sgmii_port_test,   0,  MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},

    {"AUX loopback test",
     (PFT)aux_loopback_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},
    
    {"PCIe Switch 10-bit PRBS Master Internal loopback test (obsolete)",
        (PFT)ovld_pcie_10prbs_cdr_int_lpbk_test,   0,   MF_4,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"PLL test",
        (PFT)build_ovld_pll_menu,   FALSE,   MF_3,
        (PFT)is_overlord,             0,
        (PFT)build_ovld_pll_menu,   TRUE},

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
    {"SM interface test",
     sm_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())is_overlord,	0,	(type_t(*)())0,	0},
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
#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar mb_get_pid[FRU_SIZE] = {0};
    uchar mb_get_loc[FRU_SIZE] = {0};
#endif
    char *tname = "AUX port loopback";

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
 
    get_mb_pid((char *)mb_get_pid);	
    strcpy((char *)mb_get_loc, "MB-Aux-Port");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid ;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */
	
    /* Segment 4: Components used */
    cterr_add_component("Component A");
	
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)display_uart_regs_cterr_wrapper,
                        (PFI)display_multiboot);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_margins_cterr_wrapper,
                        (PFV)show_temp_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Action A","Action B","Action C");
#endif
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (uart_intf_test("/dev/ttyS1", NULL, B9600) != PASSED) {
        cterr('f', 0, "AUX loopback failed. Is loopback connector installed?");
        return(FAILED);
    }

    return(PASSED);

}

/*
 * Function: usb_tests
 *
 * Description : usb r/w tests. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
static int usb_tests (int slot) {

    int rc = FAILED;
    char *tname = "USB slot";

    testname("%s%d access", tname, slot);
    prpass(testpass, "%s%d, ", tname, slot);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback 
     * D_EXT_LOOPBACK = 1, disable ext. loopback 
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
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
 * Function: cf_eusb_tests
 *
 * Description : bootflash r/w tests.
 *
 * Inputs: slot - bootflash slot num
 *
 * Output: PASSED/FAILED
 */
static int cf_eusb_tests (int slot) {

    int rc = FAILED;
    char *tname = "bootflash slot";

    testname("%s%d access", tname);
    prpass(testpass, "%s, ", tname);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback  
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return(PASSED);
    }

    /* testname is printed on cf_slot_tests */
    /* o2 uses compact flash, juno uses eUSB */
    if (is_overlord())  {
        rc = cf_slot_tests(slot);
        if (rc == FAILED) {
            cterr('f',0,"Compact flash test failed.");
        }
    } else {
        rc = eusb_slot_tests(slot);
        if (rc == FAILED) {
            cterr('f',0,"eUSB test failed.");
        }
    }

    return(rc);
}

/*
 * Function: display_env
 *
 * Description: Display the Environment information
 *
 * Inputs: None
 *
 * Output: None
 *
 */
void
display_env(void)
{
    show_margins_cterr_wrapper();
    show_temp_cterr_wrapper();
}

/******** History ******** 
$Log: mb_tests.c,v $
Revision 1.42  2014/04/25 07:12:16  hroni
revert change on aux_port_test. ovld don't need this test

Revision 1.41  2014/04/23 12:59:34  hroni
add aux port test item, implemented in aux_port_test()

Revision 1.40  2014/03/26 19:23:22  siyen
Added Dynamo supports at the platform (CSCun82755).

Revision 1.39  2014/02/18 09:11:11  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.38  2014/01/27 23:53:39  ptong
Obsolete the ovld_pcie_10prbs_cdr_int_lpbk_test

Revision 1.37  2013/11/28 03:31:03  alpeng
support both cf and eusb test

Revision 1.36  2013/11/27 08:58:33  erwu2
fix compiler warning

Revision 1.35  2013/10/08 11:14:28  erwu2
enhanced err msg first check-in

Revision 1.34  2013/08/19 02:58:40  alpeng
MB PLL test not support on Juno anymore

Revision 1.33  2013/08/12 08:31:25  alpeng
display the test name for usb, aux and cf test before testing

Revision 1.32  2013/06/24 08:39:24  alpeng
support Ext. loopback flag checking before USB/CF diag

Revision 1.31  2013/06/20 07:46:56  alpeng
add function wrapper for usb/cf test

Revision 1.30  2013/05/31 12:51:04  danchung
Add checking board type for Juno.

Revision 1.29  2013/05/09 19:25:21  mcharon
remove unused header files. fixed dependancy compile problem

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
