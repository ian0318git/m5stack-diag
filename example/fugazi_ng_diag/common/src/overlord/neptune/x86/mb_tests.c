/* $Id: mb_tests.c,v 1.3 2019/06/26 08:49:42 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/mb_tests.c,v $
 *------------------------------------------------------------------
 *
 * mb_tests.c - M/B test wraps.
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
#include "cli_cmd.h"
#include "plat_defs.h"
#include "linux_pciutils.h"

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
extern int access_device_test(char *);
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern int get_pcie_cap_struct_ptr(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_cap(uint32_t, uint16_t, int, uint);

/* #define BYPASS_ENV  * */
extern int linux_memory_tester(int);
extern int linux_memory_tester_with_ecc_check(int);
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
extern int neptune_x86_ge_port_test(void);
extern int ovld_pcie_10prbs_cdr_int_lpbk_test(void);
extern int ovld_pcie_8prbs_cdr_int_lpbk_test(void);
extern int force_skip_eusb(void);
extern int force_skip_msata(void);
extern int build_ovld_pll_menu(int);
extern boolean is_overlord(void);
extern int check_menu_flag(uint);
extern int pi_rd_reg (uint32_t reg_addr);
extern int is_neptune(void);

static int aux_loopback_test(int dummy);
static int usb_tests(int);
static int eusb_tests(int);
static int emmc_tests(int);
static int m2_sata_test(int dummy); 
int pcie_lane_scan_test(void);
int check_skip_test(char *); 
int usb_exist(int);
static int sfp_side_band_signal_test(int dummy);

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";
uchar psu_pid[] = "PSU-PID";
uchar rps_pid[] = "RPS-PID";
uchar pvdm_pid[] = "PVDM-PID";
uchar backplane_pid[] = "Backplane-PID";
uchar risercard_pid[] = "RiserCard-PID";
uchar mb_emmc_loc[] = "MB/eMMC";
uchar mb_eusb_loc[] = "MB/eUSB";
uchar mb_msata_loc[] = "MB/mSATA";
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
    { mb_pid,        mb_emmc_loc},
    { mb_pid,        mb_eusb_loc},
    { mb_pid,        mb_msata_loc},
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
    {"Main memory test with cache on and ECC checking",
     (PFT)linux_memory_tester_with_ecc_check,	FALSE,		MF_3,
     (type_t(*)())0, 0,		(PFT)linux_memory_tester_with_ecc_check,	TRUE},

    {"USB 0 test (need USB3.0 drive)",
   	(PFT)usb_tests,	0,	         	MF_3,
	(type_t(*)())0,		0,
	(PFT)0,	0},

    {"USB 1 test (need USB3.0 drive)",
   	(PFT)usb_tests,	1,	        	MF_3,
	(type_t(*)())0,		0,
	(PFT)0,	0},
	
    {"eUSB test",
        (PFT)eusb_tests,  0,                      MF_3,
	(type_t(*)())0,		0,
        (PFT)0,  0},

    {"M.2 SATA test",
        (PFT)m2_sata_test,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"eMMC 0 test",
        (PFT)emmc_tests,  0,                      MF_3,
	(type_t(*)())0,		0,
        (PFT)0,  0},

    {"PCIe register test",
        (PFT) pi_rd_reg,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"PCIe lane scan test",
        (PFT)pcie_lane_scan_test,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

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

    {"Broadwell GE interface 2-port lpbk at GESW test",
     (PFT)neptune_x86_ge_port_test,   0,  MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},

    {"AUX loopback test",
     (PFT)aux_loopback_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())0,   0},

    {"SFP side band signal gest",
     (PFT)sfp_side_band_signal_test, 0, MF_3,
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
 * Sub Menu used for I/O Interface tests.
 */

submenu_xtable_t io_tests_submenu_table[] = {
    {"SM interface test",
     sm_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
     (type_t(*)())0,	0,	(type_t(*)())0,	0},
    {"WIC interface test",
     wic_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
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

/* Neptune mother board skippable plug-in items names.
 * Must match the name used in skip_test.txt generated by the script, and
 * Order in array must match enum MB_SKIP_ITEMS.
 * mb_skip_item_msg hold the skip message to be printed.
 */
char *mb_skip_item_name[] = { "eUSB", "M2", "30WPOE", "USB0", "USB1", "AUX" };
char mb_skip_item_msg[MB_SKIP_END][64];
extern type_t prt_skip_plugin(char *str);

static void
display_no_reg (void)
{
    cterr_db_print("This item has no reg to display\n"); 
}

static void
display_env_err (void)
{
    show_margins_x(0, CLI_MODE);
}

/*
 * Function: add_mb_item_skip_msg
 *
 * Description : Using the mitem_t feature. This function adds the
 * prt_skip_plugin to the mlfunc field of mitem_t to allow the
 * skipped plugin list to be appended after the mb menu items.
 *
 * Inputs: 
 *
 * Output: TRUE always
 */
int add_mb_item_skip_msg(submenu_xtable_t *px, int size, menuinfo_t **addr_submenup)
{
    MB_SKIP_ITEMS si;
    type_t (*x_pfunc)(), x_pparam;
    submenu_xtable_t *px_sav = px;
    int i, add_msg;
    mitem_t *pi;
    menuinfo_t *pm = *addr_submenup;

    for (si = EUSB_SK; si <= AUX_SK; si++) {
	x_pfunc = NULL;
	if (check_skip_test(mb_skip_item_name[si]) == TRUE) {
	    x_pparam = -1; /* init to a non valid number */
	    switch(si) {
	    case EUSB_SK:
	        x_pfunc = (type_t (*)()) &eusb_tests;
	        break;
	    case M2_SK:
		x_pfunc = (type_t (*)()) &m2_sata_test;
	        break;
	    case POECARD_SK:
		x_pfunc = (type_t (*)()) &ovld_x86_i2c_scan_test;
	        break;
	    case USB0_SK:
		x_pfunc = (type_t (*)()) &usb_tests;
		x_pparam = 0;
	        break;
	    case USB1_SK:
		x_pfunc = (type_t (*)()) &usb_tests;
		x_pparam = 1;
	        break;
	    case AUX_SK:
		x_pfunc = (type_t (*)()) &aux_loopback_test;
	        break;
	    default:
	        break;
	    }
	}

	if ((void *)x_pfunc != NULL) {
	    sprintf(mb_skip_item_msg[si], "...( %s ) skipped by user", mb_skip_item_name[si]);
	    add_msg = 0;
	    px = px_sav;
	    pi = pm->miptr + base_submenu_item_total;  /* pi is at  next item after base items */
	    for (i = 0; i < size; i++, px++, pi++) {
	      if ((void *)px->x_pfunc == (void *)x_pfunc) { /* function matched */
		    if ((void *)x_pfunc == (void *)&usb_tests) {
		        if (px->x_pparam == x_pparam) { /* check usb0 or usb1 */
			    add_msg = 1;
			}
		    }
		    else {
		        add_msg = 1;
		    }

		    if (add_msg) {
		        pi->mlfunc = &prt_skip_plugin;
			pi->mlparam = mb_skip_item_msg[si];
			break;
		    }
		}
	    }
	}
    }
    return (TRUE);
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
    build_primary_submenu(mb_tests_submenu_table, MB_TESTS_SUBMENU_TABLE_SIZE,
			    "Motherboard", &mb_submenup);
    build_secondary_submenu(mb_tests_submenu_table,
			    MB_TESTS_SUBMENU_TABLE_SIZE,
			    mb_tests_secondary_items);
    add_mb_item_skip_msg(mb_tests_submenu_table, MB_TESTS_SUBMENU_TABLE_SIZE, &mb_submenup);

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

/*Add extended error reporting functions here */
void add_emmc_err_report (void)
{
    fru_table_offset = MB_EMMC;
    platform_fru_table[MB_EMMC].pid_string = mb_pid;
    platform_fru_table[MB_EMMC].location_string = mb_emmc_loc;
    cterr_add_component("MB", "eMMC");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env_err);
    cterr_add_debug("Please check whether eMMC is plugged",
            "Please try to use another eMMC");
}

void add_eusb_err_report (void)
{
    fru_table_offset = MB_EUSB;
    platform_fru_table[MB_EUSB].pid_string = mb_pid;
    platform_fru_table[MB_EUSB].location_string = mb_eusb_loc;
    cterr_add_component("MB", "eUSB");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env_err);
    cterr_add_debug("Please check whether eUSB slot is not vacant",
            "Please try to use another eUSB device");
}

static void
add_msata_err_report (void)
{
    fru_table_offset = MB_MSATA;
    platform_fru_table[MB_MSATA].pid_string = mb_pid;
    platform_fru_table[MB_MSATA].location_string = mb_msata_loc;
    cterr_add_component("MB", "mSATA");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env_err);
    cterr_add_debug("Please check whether mSATA slot is not vacant",
            "Please try to use another mSATA device");
}

// End of extended error reporting functions

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
    testname("%s", tname);
    /* 
     * D_EXT_LOOPBACK = 0, enable ext. loopback 
     * D_EXT_LOOPBACK = 1, disable ext. loopback 
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return(PASSED);
    }

    if (check_skip_test(mb_skip_item_name[AUX_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

#ifdef ENHANCED_ERR_MSG_EXAMPLE
    uchar mb_get_pid[FRU_SIZE] = {0};
    uchar mb_get_loc[FRU_SIZE] = {0};
#endif

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
    prpass(testpass, "%s, ", tname);

    /* Set the CPU to AUX connection */
    system(SET_CPU_TO_AUX);

    if (uart_intf_test("/dev/ttyS1", NULL, B9600) != PASSED) {
        cterr('f', 0, "AUX loopback failed. Is loopback connector installed?");
        return(FAILED);
    }

    return(PASSED);
}

/*
 * Function: usb_exist
 *
 * Description : Check USB device is available. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
int usb_exist (int slot)
{
    char check_usbdrv[50];
    int devfd, ix;
    sprintf(check_usbdrv, "/dev/usbdrv%d", slot);

    for (ix = 0; ix < 10; ix++) {
        devfd = open(check_usbdrv, O_RDWR);
        if(devfd < 0) {
            sleep(1);
            close(devfd);
            continue;
        } else {
            break;
        }
    }
    if (devfd < 0) {
        close(devfd);
        perror("there is no device file descriptor available. ");
        printf("Can not access device at USB slot%d. is slot vacant?", slot);
        return (FAILED);
    } else {
        close(devfd);
        return (PASSED);
    }
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
static int usb_tests (int slot) 
{
    int retval = PASSED;
    char *tname = "USB slot";
    char buf[128] = "NULL";
    FILE *fp;
    char *check_usb3_file = "/nep-diag/usb_speed.txt";
    char check_usb2_spd[] = "480";
    int ix, is_usb2_stick;

    testname("%s%d access", tname, slot);
    /* 
     * D_EXT_LOOPBACK = 0, enable ext. loopback 
     * D_EXT_LOOPBACK = 1, disable ext. loopback 
     */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return(PASSED);
    }

    if (((slot == 0) && check_skip_test(mb_skip_item_name[USB0_SK])) ||
	((slot == 1) && check_skip_test(mb_skip_item_name[USB1_SK]))) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    /* Temporary disable showing kernel messages because unbind and bind XHCI controller to XHCI driver */
    system(SUPPRESS_MESG);

    /* Check USB stick is 2.0 or 3.0, return fail if is USB 2.0 stick */
    for (ix = 0; ix < 5; ix++) {
        is_usb2_stick = 0;
        if (slot) {
            if (usb_exist(slot) != PASSED) {
                cterr('f',0, "Can't find USB slot-%d device node.", slot);
                return (FAILED);
            }
            system(GET_USB1_SPEED);
        } else {
            if (usb_exist(slot) != PASSED) {
                cterr('f',0, "Can't find USB slot-%d device node.", slot);
                return (FAILED);
            }
            system(GET_USB0_SPEED);
        }

        fp = fopen(check_usb3_file, "r");
        if (fp == NULL) {
            return (FALSE);
        }

        while (!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, check_usb2_spd) != NULL) {
                if (ix == 4) {
                    fclose(fp);
                    system("lsusb -tv");
                    cterr('f',0, "Please plug USB3 stick into USB slot-%d.", slot);
                    system(REMOVE_USBSPD_FILE);
                    /* Enable kernel message */
                    system(OPEN_MESG);
                    return (FAILED);
                } else {
                    is_usb2_stick = 1;
                    break;
                }
            } 
        }

        if (is_usb2_stick == 1) {
            printf("Detect USB2 stick at slot-%d\n", slot);
            system(UNBIND_XHCI_CONTROLLER);
            msleep(100);
            system(BIND_XHCI_CONTROLLER);
            msleep(500);
            system(UDEVTRIGGER);
            msleep(100);
            fclose(fp);
            system(REMOVE_USBSPD_FILE);
        } else {
            fclose(fp);
            goto out;
        }
    }

out:
    /* First time USB test */
    prpass(testpass, "USB slot%d host XHCI controller default run\n", slot);
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller default run failed.", slot);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d host XHCI controller disable super speed run\n", slot);

    /* Disable USB 3.0 super speed */
    system(DISABLE_USB3_SS);

    for (ix = 0; ix < 5; ix++) {
        /* Unbind and bind XHCI controller to XHCI driver to make disable super speed setting active */
        system(UNBIND_XHCI_CONTROLLER);
        msleep(100);
        system(BIND_XHCI_CONTROLLER);
        msleep(500);
        system(UDEVTRIGGER);
        msleep(100);

        /* Check USB device available after disable USB3 super speed */
        if (usb_exist(slot) != PASSED) {
            if (ix == 4) {
                cterr('f',0, "USB slot%d not available after disable USB3 super speed.", slot);
                return (FAILED);
            } 
        } else {
            goto done;
        }
    }
done:

    /* Second time USB test - disable super speed run */
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller disable super speed run failed.", slot);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d switch USB3 from XHCI to EHCI controller run\n", slot);
    /* Route USB ports from XHCI controller to EHCI controller */
    system(ROUTE_USB2_TO_EHCI);
    msleep(500);
    system(UDEVTRIGGER);
    msleep(100);

    /* Check USB device available after route USB from XHCI to EHCI controller */
    if (usb_exist(slot) != PASSED) {
        cterr('f',0, "USB slot%d not available after route USB from XHCI to EHCI controller.", slot);
        return (FAILED);
    }

    /* Final USB test  */
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d switch USB3 from XHCI to EHCI controller run failed.", slot);
    }

    /* Enable super speed and route USB port from EHCI controller to XHCI controller */
    system(ENABLE_USB3_SS);
    msleep(100);
    system(ROUTE_USB2_TO_XHCI);
    msleep(1000);
    system(UDEVTRIGGER);
    msleep(100);

    for (ix = 0; ix < 5; ix++) {
        /* Check USB device available after route USB port from EHCI to XHCI controller */
        if (usb_exist(slot) != PASSED) {
            if (ix == 4) {
                cterr('f',0, "USB slot%d not available after route USB port from EHCI to XHCI controller.", slot);
                return (FAILED);
            } else {
                /* Unbind and bind XHCI controller to XHCI driver */
                system(UNBIND_XHCI_CONTROLLER);
                msleep(100);
                system(BIND_XHCI_CONTROLLER);
                msleep(500);
                system(UDEVTRIGGER);
                msleep(100);
            }
        } else {
            goto exit;
        }
    }
exit:
    system(REMOVE_USBSPD_FILE);
    /* Enable kernel message */
    system(OPEN_MESG);
    return (retval);
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

/*
 * Function: eusb_tests
 *
 * Description : eUSB r/w tests.
 *
 * Inputs: slot - eUSB slot num
 *
 * Output: PASSED/FAILED
 */
static int eusb_tests (int slot) 
{
    int rc = FAILED;
    char *tname = "eUSB";

    testname("%s access", tname);

    if (check_skip_test(mb_skip_item_name[EUSB_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    if (get_enhance_err_flag()) {
        add_eusb_err_report();
    }

    prpass(testpass, "%s, ", tname);

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
static int emmc_tests (int dummy) 
{
    int rc = FAILED;
    char *tname = "emmc0";
    
    if (get_enhance_err_flag()) {
        add_emmc_err_report();
    }

    /* we only have one emmc */
    testname("%s access", tname);
    prpass(testpass, "%s, ", tname);

    /* testname is printed on usb_slot_tests */
    rc = emmc_slot_tests(dummy);
    if (rc == FAILED) {
        cterr('f',0,"emmc0 test failed.");
    }

    return(rc);
}

/*
 * Function: pcie_lane_scan_test
 *
 * Description : PCI interface scan/check
 *
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
int pcie_lane_scan_test (void) 
{
    int ix;
    uint32_t bus[5], reg_val[5], cap_val[5], sta_val[5];
    uint32_t cap_s, cap_w, sta_s, sta_w;
    char dev_name[5][20] = {"Cavium", "Pericom", "I211", "FPGA", "Broadcom Switch"};
    uint32_t dev_vid[5] = {NEP_CAV_VID, NEP_PCIE_SW_VID, NEP_I211_VID, NEP_FPGA_VID, NEP_BROADCOM_SW_VID};
    /* Neptune and Triton share the same DID */
    uint32_t nep_dev_did[5] = {NEP_CAV_DID, NEP_PCIE_SW_DID, NEP_I211_DID, NEP_FPGA_DID, NEP_BROADCOM_SW_DID};

    char *tname = "PCI lane scan";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    for (ix = CAVIUM; ix <= sizeof(PCIE_DEVICES); ix++) {
        /* Check following: Pericom, Broadcom, Cavium, FPGA, i211 */
        bus[ix] = get_pcie_bus_num(dev_vid[ix], nep_dev_did[ix]);

        if (bus[ix] == UNKNOWN_PCI_BUS_NUM) { 
            cterr('f',0, "Unknown PCI bus number");
            return (FAILED);
        }

        prpass(testpass, "%s", dev_name[ix]);
        reg_val[ix] = get_pcie_cap_struct_ptr(bus[ix], PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
        if (reg_val[ix] == FAILED) {
            cterr('f',0, "Can't get PCI cap pointer");
            return (FAILED);
        }

        cap_val[ix] = get_pcie_link_cap(bus[ix], PCI_DEV_0, PCI_FUN_0, reg_val[ix]);
        sta_val[ix] = get_pcie_link_status(bus[ix], PCI_DEV_0, PCI_FUN_0, reg_val[ix]);

        /* Speed - bit 0~3 */
        cap_s = cap_val[ix] & PCI_EXP_LINK_STA_SPD_MASK;
        sta_s = sta_val[ix] & PCI_EXP_LINK_STA_SPD_MASK;
        /* Width - bit 4~9 */
        cap_w = (cap_val[ix] & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;
        sta_w = (sta_val[ix] & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

        if (ix == BROADCOM_SW) {
            /* BIOS is limiting the link speed to Gen1 because of an errata in the Broadcom switch */
            if (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5) {
                prpass(testpass, "Link speed is 2.5G ");
            } else {
                cterr('f',0, "Link speed is not correct");
                return (FAILED);
            }
        } else {
            if ((cap_s == PCI_EXP_LINK_STA_SPD_2DOT5) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
                prpass(testpass, "Link speed is 2.5G ");
            } else if ((cap_s == PCI_EXP_LINK_STA_SPD_5GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
                prpass(testpass, "Link speed is 5G ");
            } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_8GT)) {
                prpass(testpass, "Link speed is 8G ");
            } else {
                cterr('f',0, "Link speed is not correct");
                return (FAILED);
            }
        }

        if ((cap_w == PCI_EXP_LINK_STA_WID_1) && (sta_w == PCI_EXP_LINK_STA_WID_1)) {
            prpass(testpass, "Link width is x1 ");
        } else if ((cap_w == PCI_EXP_LINK_STA_WID_2) &&(sta_w == PCI_EXP_LINK_STA_WID_2)) {
            prpass(testpass, "Link width is x2 ");
        } else if ((cap_w == PCI_EXP_LINK_STA_WID_4) && (sta_w == PCI_EXP_LINK_STA_WID_4)) {
            prpass(testpass, "Link width is x4 ");
        } else if ((cap_w == PCI_EXP_LINK_STA_WID_8) && (sta_w == PCI_EXP_LINK_STA_WID_8)) {
            prpass(testpass, "Link width is x8 ");
        } else {
            cterr('f',0, "Link width is not correct");
            return (FAILED);
        }
    }
	
    prpass(testpass, "PCIe lane scan success. ");
    return (PASSED);
}

 /*
 * Function: msata_tests
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int m2_sata_test (int dummy) {

    int rc = FAILED;
    char *tname = "M.2 read/write";
    char *m2_dev = "/dev/m2sata";
    
    testname("%s access", tname);

    if (check_skip_test(mb_skip_item_name[M2_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    if (get_enhance_err_flag()) {
        add_msata_err_report();
    }

    /* so far we have only one mSATA */
    prpass(testpass, "%s, ", tname);
    
    /* FPGA 0x320A0 bit 4 (SATA HDD 1 Present) for Utah mSATA present */
    if(!is_sata_present(SATA_NUM_ONE)) {
        cterr('f',0, "M.2 SATA is not present, slot vacant");
        return (PASSED);
    }

    rc = sata_tests((uchar *)m2_dev);
    if (rc == FAILED) {
        cterr('f',0, "mSATA test failed.");
    }

    return(rc);
}

/*
 * Function: check_skip_test 
 *
 * Description : Check which tests need to be skipped.
 *
 * Inputs: item - Item name 
 *
 * Output: TRUE/FALSE
 */
int check_skip_test (char *item) 
{
    char buf[128];
    char skip_all[] = "ALL";
    FILE *fp;
    char *result_file = "/nep-diag/skip_test.txt";

    fp = fopen(result_file, "r");
    if (fp == NULL) {
        return (FALSE);
    }

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if ((strstr(buf, skip_all) != NULL) || (strstr(buf, item) != NULL)) {
            fclose(fp);
            return (TRUE);
        } 
    }

    fclose(fp);
    return (FALSE);
}


 /*
 * Function: sfp_side_band_test
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int sfp_side_band_signal_test (int dummy) 
{
   unsigned int dev, status; 
   unsigned int sfp_sts[MAX_NEP_NUM_SFP] = {FPGA_SFP0_INTR, 
                                            FPGA_SFP1_INTR, 
                                            FPGA_SFP_P0_INTR, /* sfp+ 0 */
                                            FPGA_SFP_P1_INTR};

   testname("DP SFP side band signal"); 

   /* 4 dev for two sfp and 2 sfp+ */
   for (dev = 0; dev < MAX_NEP_NUM_SFP; dev++) {
       /* check sfp present, skip test if not available */

       /* clean up interrupt by read fpga */
       prpass(testpass, "clean up status reg and disable interrupt.");
       disable_platform_sfp_intr(dev); 

       /* enable - low to high level */
       prpass(testpass, "enable interrupt.   ");
       enable_platform_sfp_intr(dev); 
    
       /* trigger interrupt, sfp port num on this function 
        * is start from 1 not 0. 
        */ 
       prpass(testpass, "override interrupt.   ");
       enable_platform_sfp_override_intr(dev+1); 

       /* check status */
       status = get_platform_sfp_intr_sts(); 

       if (status & sfp_sts[dev]) { 
           prpass(testpass, "Dev%d Interrupt detected on 0x%x with status reg 0x%x",
                             dev, sfp_sts[dev], status);
       } else {
           cterr('f',0, "dev%d Interrupt status = 0x%x, 0x%x is not set \n",
                        dev, status, sfp_sts[dev]); 
           return (FAILED); /* return here to keep error state. */
       }
 
       /* disable - high to low level */
       disable_platform_sfp_intr(dev); 

   } 

   return (PASSED); 
}



/******** History ******** 
$Log: mb_tests.c,v $
Revision 1.3  2019/06/26 08:49:42  alpeng
support side band signal test for neptune; remove local intr check for sfp, since fpga is not support anymore.

Revision 1.2  2018/05/18 09:24:59  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.27  2018/03/06 09:35:15  alpeng
remove VM from io interface submenu

Revision 1.1.2.26  2017/05/15 03:20:28  leschen
Adding counter to check device node is available for usb testing.

Revision 1.1.2.25  2017/03/15 08:20:33  leschen
Add counter for checking USB device node.

Revision 1.1.2.24  2017/03/13 08:39:37  leschen
Triton share the same device id with Neptune. Change the test name to pcie_lane_scan_test.

Revision 1.1.2.23  2017/03/09 03:10:36  leschen
Initialize buf variable for usb_tests and support Triton PCI lane scan test.

Revision 1.1.2.22  2017/03/01 01:44:12  ptong
Fix a minor bug in add_mb_item_skip_msg

Revision 1.1.2.21  2017/01/26 08:12:32  leschen
Support USB tests.

Revision 1.1.2.20  2017/01/18 23:31:02  ptong
Fail diag when GESW failed to set 10GKR on NGIO

Revision 1.1.2.19  2017/01/12 02:12:22  ptong
Restore prpass statement in emmc_tests

Revision 1.1.2.18  2017/01/10 23:42:34  ptong
Print item skipped msg in the mb submenu

Revision 1.1.2.17  2017/01/06 01:30:40  leschen
Display USB test output message clearly.

Revision 1.1.2.16  2017/01/04 07:40:05  leschen
Modify display message format.

Revision 1.1.2.15  2016/12/26 07:00:49  leschen
Support to skip pluggable modules tests and clean up codes.

Revision 1.1.2.14  2016/12/15 09:46:23  leschen
Do not declare pcie lane scan function as static function.

Revision 1.1.2.13  2016/12/15 08:46:22  leschen
Modify PCIe lane scan test, provide vendor/device ids to get bus number automatically, get pcie cap struct to detect link speed/width.

Revision 1.1.2.12  2016/12/13 00:23:42  ptong
Added GESW port list util, host port send pkt to GESW test support for Neptune

Revision 1.1.2.11  2016/12/09 09:00:06  alpeng
remove is_overlord to enable sm for io intf test

Revision 1.1.2.10  2016/12/08 01:12:17  leschen
Set the CPU to AUX connection for AUX lpbk testing.

Revision 1.1.2.9  2016/11/04 06:04:57  leschen
Support USB3/USB2 testing.

Revision 1.1.2.8  2016/10/19 17:54:15  leschen
Modify Neptune PCIe lane scan and USB test.

Revision 1.1.2.7  2016/08/04 02:35:09  leschen
Support memory testing with ECC checking.

Revision 1.1.2.6  2016/07/28 07:26:20  leschen
Add USB3.0 and USB2.0 test.

Revision 1.1.2.5  2016/06/21 21:39:07  jskow
Add SM4 skeleton code, add eUSB/emmc check, add msata test

Revision 1.1.2.4  2016/06/02 22:04:01  jskow
Move Overlord/x86 specific files to Neptune/x86.

Revision 1.1.2.3  2016/06/01 23:14:17  jskow
Update Makefile for Neptune, add mb_test structures for PCIe IF test and PCIe register check

Revision 1.1.2.1  2016/05/06 00:32:04  jskow
Add emmc and eusb tests, update banner to say Neptune, modify Makefiles and move files to boot nepx86_diag on Overlord

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
