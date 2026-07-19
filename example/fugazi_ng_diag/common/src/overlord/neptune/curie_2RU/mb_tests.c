/* $Id: mb_tests.c,v 1.4 2021/04/17 16:29:27 leifen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/mb_tests.c,v $
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
#include "bcm57412_test.h"
#include "i350_test.h"
#include "curie2ru_test.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"
#include "curie2ru_xhci.h"


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
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern int get_pcie_cap_struct_ptr(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_cap(uint32_t, uint16_t, int, uint);

extern int linux_memory_tester(int);
extern int linux_memory_tester_with_ecc_check(int);
extern int do_all_menu_items(struct menuinfo *);
extern int smartchip_authenticate_retest(uchar, uchar);
extern int smartchip_authenticate(uchar, uchar);

extern int curie_x86_i2c_scan_test(int);
extern int dash_rd_wr_test(int);

extern int force_skip_eusb(void);
extern int force_skip_msata(void);
extern int check_menu_flag(uint);

extern int xpoint_test(int);
extern int cpu_core_test(void);
extern int fan_speed_test(void);
static int usb_tests(int);
static int eusb_tests(int);
static int emmc_tests(int);
static int m2_sata_test(int); 
static int m2_nvme_test(int); 
static int m2_combo_test(int); 
static int usb_exist(int);
static int show_m2_test(void);
int pcie_lane_scan_test(void); /* also used by linux_main.c */
int check_skip_test(char *);  /* also used by platform_i2c.c */
static type_t plug_iface_test(void);
extern boolean is_m2_testcard_in (void);

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
        (PFT)is_curie_1ru,		0,
        (PFT)0,  0},

    {"M.2 NVME/SATA test",
        (PFT)m2_combo_test,  0,                    MF_3,
        (PFT)is_curie_1ru,         0,
        (PFT)0,  0},

    {"M.2 NVME/SATA/eUSB test",
        (PFT)m2_combo_test,  0,                    MF_3,
        (PFT)show_m2_test,    0,
        (PFT)0,  0},

    {"eMMC 0 test",
        (PFT)emmc_tests,  0,                      MF_3,
	(type_t(*)())0,		0,
        (PFT)0,  0},

    {"PCIe lane scan test",
        (PFT)pcie_lane_scan_test,  0,                      MF_3,
        (type_t(*)())0,         0,
        (PFT)0,  0},

    {"I2C scan test",
        (PFT)curie_x86_i2c_scan_test,   0,       MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},
    
    {"DASH FPGA register test",
        (PFT)dash_rd_wr_test,      0,           MF_3,
        (type_t(*)())0,   0,
        (type_t(*)())0,   0},

    {"BCM57412 test",
     (PFT)ten_g_bcm57412_test, 1, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())ten_g_bcm57412_test,   0},

    {"I350 test",
     (PFT)i350_test, 1, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())i350_test,   0},

    {"BCM82757 test",
     (PFT)curie2ru_bcm82757_test, 0, MF_3,
     (PFT)is_uranium,   0,
     (type_t(*)())curie2ru_bcm82757_test,   1},

    {"BCM82752 test",
     (PFT)curie2ru_bcm82752_test, 0, MF_3,
     (PFT)is_thorium,   0,
     (type_t(*)())curie2ru_bcm82752_test,   1},

    {"CPU core test",
     (PFT)cpu_core_test, 0, MF_3,
     (type_t(*)())0,   0,
     (type_t(*)())cpu_core_test,   0},

/* Remove xpoint test for EDVT temporarily, as suggested by HW
 * EDVT also do not use SM test card.
 * make CURIE2RU_NO_CROSSPOINT=1 for EDVT compilication
 */
#ifndef CURIE2RU_NO_CROSSPOINT
    {"Crosspoint tests",
     (PFT) xpoint_test, TRUE, MF_3,
     (PFT) is_curie_2ru, 0,
     (PFT) xpoint_test, FALSE},
#endif

    {"Fan speed test",
     (PFT)fan_speed_test, 0, MF_3,
     (type_t(*)())0,   0,
     (type_t(*)())fan_speed_test,   0},
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
    {"PIM interface test",
     plug_iface_test,		0,	MF_CONTINUOUS | MF_DOALL,
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

/* Curie 1RU mother board skippable plug-in items names.
 * Must match the name used in skip_test.txt generated by the script, and
 * Order in array must match enum MB_SKIP_ITEMS.
 * mb_skip_item_msg hold the skip message to be printed.
 */
char *mb_skip_item_name[] = { "eUSB", "M2", "USB0", "USB1" };
char mb_skip_item_msg[MB_SKIP_END][64];
extern type_t prt_skip_plugin(char *str);

static type_t plug_iface_test(void)
{
    return (plug_intf_test(PLUG_SLOT_1));
}

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
 * Function: Determines whether to display MENU
 *
 * Description :If M.2 test card is present,the original "m2_combo_test" should
 * be shielded.
 *
 * Inputs:
 *
 * Output: TRUE/FALSE
 */
static int show_m2_test(void)
{
    if (is_m2_testcard_in() == TRUE) {
        return FALSE;
    } else {
        return TRUE;
    }

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

    for (si = EUSB_SK; si < MB_SKIP_END; si++) {
	x_pfunc = NULL;
	if (check_skip_test(mb_skip_item_name[si]) == TRUE) {
	    x_pparam = -1; /* init to a non valid number */
	    switch(si) {
	    case EUSB_SK:
	        x_pfunc = (type_t (*)()) &eusb_tests;
	        break;
	    case M2_SK:
		x_pfunc = (type_t (*)()) &m2_combo_test;
	        break;
	    case USB0_SK:
		x_pfunc = (type_t (*)()) &usb_tests;
		x_pparam = 0;
	        break;
	    case USB1_SK:
		x_pfunc = (type_t (*)()) &usb_tests;
		x_pparam = 1;
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
#if 0 /* remove as tsn */
	if (diagflag_xram & D_XEC_AUTH) /* For MFG */
	    smartchip_authenticate_retest(MOTHER_BOARD, 0);
#ifdef AUTHENTICATION_TEST_Y 
	else if (diagflag_yram & D_AUTH_Y) /* For EDVT with retry */
	    smartchip_authenticate(MOTHER_BOARD, 0);
#endif /* AUTHENTICATION_TEST_Y	*/
#endif 
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
 * Function: usb_exist
 *
 * Description : Check USB device is available. 
 *              
 * Inputs: slot - usb slot num
 *
 * Output: PASSED/FAILED
 */
static int usb_exist (int slot)
{
    char check_usbdrv[50];
    char check_drvnode[64];
    int devfd, ix;
    size_t size = 0;
    sprintf(check_usbdrv, "/dev/usbdrv%d", slot);
    sprintf(check_drvnode, "ls -l %s", check_usbdrv);

    for (ix = 0; ix < 10; ix++) {
        if (file_exist(check_usbdrv, &size)) {
            printf("%s exist\n", check_usbdrv);
            break;
        } else {
            msleep(500);
        }
    }
    if (ix == 10) {
        printf("%s does NOT exist\n", check_usbdrv);
        system(check_drvnode);
        return (FAILED);
    }

    for (ix = 0; ix < 10; ix++) {
        devfd = open(check_usbdrv, O_RDWR);
        if(devfd < 0) {
            system(UDEVTRIGGER);
            sleep(1);
            close(devfd);
            continue;
        } else {
            break;
        }
    }
    if (devfd < 0) {
        close(devfd);
        printf("Device %s open failed. Is USB slot%d vacant?\n", check_usbdrv, slot);
        return (FAILED);
    } else {
        close(devfd);
        return (PASSED);
    }
}

int c2ru_usb_2p0_mode_set(unsigned int port_mask)
{
    msleep(4000);
    system(UNBIND_XHCI_CONTROLLER);
    msleep(2000);

    __c2ru_disable_usb3_ss(1, port_mask);

    system(BIND_XHCI_CONTROLLER);
    msleep(2000);

    reset_plat_dev(FPGA_RST_USB0_DIS | FPGA_RST_USB1_DIS);
    msleep(500);
    unreset_plat_dev(FPGA_RST_USB0_DIS | FPGA_RST_USB1_DIS);
    msleep(4000);

    system(UDEVTRIGGER);
    msleep(100);

    system("echo; lsusb -t");

    return PASSED;
}

int c2ru_usb_3p0_mode_set(unsigned int port_mask)
{
    msleep(4000);
    system(UNBIND_XHCI_CONTROLLER);
    msleep(2000);
    __c2ru_disable_usb3_ss(0, port_mask);


    system(BIND_XHCI_CONTROLLER);

    msleep(2000);

    reset_plat_dev(FPGA_RST_USB0_DIS | FPGA_RST_USB1_DIS);
    msleep(500);
    unreset_plat_dev(FPGA_RST_USB0_DIS | FPGA_RST_USB1_DIS);
    msleep(4000);

    system(UDEVTRIGGER);
    msleep(100);

    system("echo; lsusb -t");

    return PASSED;
}

static int c2ru_usb_tests (int slot)
{
    int i, retval = PASSED;
    char *tname = "USB slot";
    char buf[128] = "NULL";
    FILE *fp;
    char *check_usb3_file = "/tmp/usb_speed.txt";
    char check_usb2_spd[] = "480";
    uint32_t usb_port_mask;

    usb_port_mask = (slot == 0) ? C2RU_USB_PORT_MASK_FRONT_A :
                                  C2RU_USB_PORT_MASK_FRONT_C;

    testname("%s%d access", tname, slot);

    /* D_EXT_LOOPBACK = 0, enable ext. loopback
     * D_EXT_LOOPBACK = 1, disable ext. loopback */
    if (check_menu_flag(D_EXT_LOOPBACK)) {
        prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
        return(PASSED);
    }

    if (((slot == 0) && check_skip_test(mb_skip_item_name[USB0_SK])) ||
	    ((slot == 1) && check_skip_test(mb_skip_item_name[USB1_SK]))) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    system(REMOVE_USBSPD_FILE);

    if (usb_exist(slot) != PASSED) {
        printf("trying to find USB 3.0 device\n");
        for (i = 0; i < 5; i++) {
            c2ru_usb_3p0_mode_set(usb_port_mask);

            if (usb_exist(slot) == PASSED) {
                break;
            }
            printf("retry to find USB 3.0 device\n");
        }
        if (i == 5) {
            cterr('f',0, "Can't find USB slot-%d device node.", slot);
            fflush(stdout);
            return (FAILED);
        }
    }

    for (i = 0; i < 5; i++) {
        switch(slot) {
        case 0:
            system(GET_USB0_SPEED);
            break;
        case 1:
            system(GET_USB1_SPEED);
            break;
        }

        fp = fopen(check_usb3_file, "r");
        if (fp == NULL) {
            cterr('f',0, "Can't find USB3 speed file %s.", check_usb3_file);
            return (FAILED);
        }

        fgets(buf, sizeof(buf), fp);
        fclose(fp);
        system(REMOVE_USBSPD_FILE);

        if (strstr(buf, check_usb2_spd) == NULL) {
            break;
        }
        printf("Detected USB2.0 speed in slot-%d, redetect.\n", slot);
        c2ru_usb_3p0_mode_set(usb_port_mask);
    }

    if (i == 5) {
        cterr('f', 0, "USB2.0 in slot-%d, expected USB3.0.", slot);
        return FAILED;
    }

    /* First time USB test */
    prpass(testpass, "USB slot%d host XHCI controller default run\n", slot);
    fflush(stdout);
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller default run failed.", slot);
        fflush(stdout);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d host XHCI controller disable super speed run\n", slot);
    fflush(stdout);

    for (i = 0; i < 5; i++) {
        c2ru_usb_2p0_mode_set(usb_port_mask);

        /* Check USB device available after disable USB3 super speed */
        if (usb_exist(slot) == PASSED) {
            break;
        }
        printf("retry to set USB 2.0 mode\n");
    }

    if (i == 5) {
        cterr('f',0, "USB slot%d not available after disable USB3.0 super speed.", slot);
        system("dmesg | tail -n 200");
        fflush(stdout);
        retval = FAILED;
        goto out;
    }

    /* Second time USB test - disable super speed run */
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller disable super speed run failed.", slot);
        fflush(stdout);
        retval = FAILED;
        goto out;
    }

    prpass(testpass, "USB slot%d switch USB3.0 to USB2.0 run\n", slot);
    fflush(stdout);

out:
    for (i = 0; i < 5; i++) {
        c2ru_usb_3p0_mode_set(usb_port_mask);

        if (usb_exist(slot) == PASSED) {
            break;
        }
        printf("retry to back to USB 3.0 mode\n");
    }

    if (i == 5) {
        cterr('f', 0, "failed to back to XHCI USB3.0\n");
        retval = FAILED;
    }

    msleep(DELAY_USBCMD);

    return (retval);
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
    char *check_usb3_file = "/tmp/usb_speed.txt";
    char check_usb2_spd[] = "480";
    int ix;

    if (is_curie_2ru())
        return c2ru_usb_tests(slot);
    
    //mfix: removed below until FPGA release...
    unsigned int rdval = 0; 
    unsigned int fpga_min_ver, usb0_pwr_dis, usb1_pwr_dis;
    dash_fpga_reg_read(FPGA_VERTYPE, &rdval);
    fpga_min_ver = (rdval&0x0F00) >> 8;
    if (fpga_min_ver == 5) {
        usb0_pwr_dis = FPGA_RST_USB0_DIS;
        usb1_pwr_dis = FPGA_RST_USB1_DIS;
    } else {
        usb0_pwr_dis = FPGA_RST_USB1_DIS;
        usb1_pwr_dis = FPGA_RST_USB0_DIS;
    }

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

    /* Check USB stck is 2.0 or 3.0. Test requires USB 3.0 drives.
     * Retry 3 times in case USB drive quality is poor.
     */
    for (ix = 0; ix <= 3; ix++) {
	    system(REMOVE_USBSPD_FILE);

        for (ix = 0; ix < 5; ix++) {
            system(UDEVTRIGGER);
            msleep(500);
            if (usb_exist(slot) != PASSED) {
                if (ix == 4) {
                    cterr('f',0, "USB slot%d not available.", slot);
                    fflush(stdout);
                    return (FAILED);
                } 

                printf("power cycle USB slot%d, retry %d\n", slot, ix);
                /* Disable power to non-tested USB port 
                 * to avoid interference*/
                if (slot == 0) {
                    /* test USB slot-0, disable power to slot-1 */
                    system(UNBIND_XHCI_CONTROLLER);
                    msleep(1000);
                    reset_plat_dev(usb0_pwr_dis);
                    msleep(1000);
                    unreset_plat_dev(usb0_pwr_dis);
                    msleep(1000);
                    system(BIND_XHCI_CONTROLLER);
                    msleep(1000);
                } else {
                    /* test USB slot-1, disable power to slot-0 */
                    system(UNBIND_XHCI_CONTROLLER);
                    msleep(1000);
                    reset_plat_dev(usb1_pwr_dis);
                    msleep(1000);
                    unreset_plat_dev(usb1_pwr_dis);
                    msleep(1000);
                    system(BIND_XHCI_CONTROLLER);
                    msleep(1000);
                }
            } else {
	            break; /* break out of for loop */
            }
        }

	    /* Get USB drive speed in a txt file and check for the speed
	     */
	    switch(slot) {
	    case 0:
	        system(GET_USB0_SPEED);
	        break;
	    case 1:
	        system(GET_USB1_SPEED);
	        break;
	    }

        fp = fopen(check_usb3_file, "r");
        if (fp == NULL) {
            return (FALSE);
        }

	    fgets(buf, sizeof(buf), fp);
	    fclose(fp);
	    system(REMOVE_USBSPD_FILE);

        if (strstr(buf, check_usb2_spd) != NULL) {
	        if (ix == 3) {
	    	    system("lsusb -tv");
	    	    cterr('f',0, "Please plug USB3.0 stick into USB slot-%d.", slot);
                fflush(stdout);
	    	    return (FAILED);
	        }

            printf("Detected USB2.0 speed (480Mbit/s) in slot-%d. "
                   "Retry to enable SuperSpeed.\n", slot);
            fflush(stdout);
            system(UNBIND_XHCI_CONTROLLER);
            msleep(5000);
            system(ENABLE_USB3_SS);
            msleep(1000);
            system(BIND_XHCI_CONTROLLER);
            msleep(5000);
            system(UDEVTRIGGER);
            msleep(500);
        } else {
	        break; /* exit for loop */
        }
    }
    /* First time USB test */
    prpass(testpass, "USB slot%d host XHCI controller default run\n", slot);
    fflush(stdout);
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller default run failed.", slot);
        fflush(stdout);
        return (FAILED);
    }

    prpass(testpass, "USB slot%d host XHCI controller disable super speed run\n", slot);
    fflush(stdout);

    /* Disable USB 3.0 super speed */
    system(UNBIND_XHCI_CONTROLLER);
    msleep(5000);
    system(DISABLE_USB3_SS);
    msleep(1000);
    system(BIND_XHCI_CONTROLLER);
    msleep(5000);

    for (ix = 0; ix < 5; ix++) {
        system(UDEVTRIGGER);
        msleep(500);
        /* Check USB device available after disable USB3 super speed */
        if (usb_exist(slot) != PASSED) {
            if (ix == 4) {
                cterr('f',0, "USB slot%d not available after disable USB3.0 super speed.", slot);
                fflush(stdout);
                return (FAILED);
            } 

            printf("power cycle USB slot%d, retry %d\n", slot, ix);
            /* Disable power to non-tested USB port 
             * to avoid interference*/
            if (slot == 0) {
                /* test USB slot-0, disable power to slot-1 */
                system(UNBIND_XHCI_CONTROLLER);
                msleep(1000);
                reset_plat_dev(usb0_pwr_dis);
                msleep(1000);
                unreset_plat_dev(usb0_pwr_dis);
                msleep(1000);
                system(BIND_XHCI_CONTROLLER);
                msleep(1000);
            } else {
                /* test USB slot-1, disable power to slot-0 */
                system(UNBIND_XHCI_CONTROLLER);
                msleep(1000);
                reset_plat_dev(usb1_pwr_dis);
                msleep(1000);
                unreset_plat_dev(usb1_pwr_dis);
                msleep(1000);
                system(BIND_XHCI_CONTROLLER);
                msleep(1000);
            }
        } else {
	        break; /* break out of for loop */
        }
    }

    /* Second time USB test - disable super speed run */
    retval = usb_slot_tests(slot);
    if (retval == FAILED) {
        cterr('f',0, "USB slot%d host XHCI controller disable super speed run failed.", slot);
        fflush(stdout);
        return (FAILED);
    }

    /* Enable super speed and route USB port from EHCI controller to XHCI controller */
    system(UNBIND_XHCI_CONTROLLER);
    msleep(5000);
    system(ENABLE_USB3_SS);
    msleep(1000);
    system(BIND_XHCI_CONTROLLER);
    msleep(5000);

    for (ix = 0; ix < 5; ix++) {
        system(UDEVTRIGGER);
        msleep(500);
        /* Check USB device available after route USB port from EHCI to XHCI controller */
        if (usb_exist(slot) != PASSED) {
            if (ix == 4) {
                cterr('f',0, "USB slot%d not available after route USB port from EHCI to XHCI controller.", slot);
                fflush(stdout);
                return (FAILED);
            } else {
                printf("power cycle USB slot%d, retry %d\n", slot, ix);
                /* Disable power to non-tested USB port 
                 * to avoid interference*/
                if (slot == 0) {
                    /* test USB slot-0, disable power to slot-1 */
                    system(UNBIND_XHCI_CONTROLLER);
                    msleep(1000);
                    reset_plat_dev(usb0_pwr_dis);
                    msleep(1000);
                    unreset_plat_dev(usb0_pwr_dis);
                    msleep(1000);
                    system(BIND_XHCI_CONTROLLER);
                    msleep(1000);
                } else {
                    /* test USB slot-1, disable power to slot-0 */
                    system(UNBIND_XHCI_CONTROLLER);
                    msleep(1000);
                    reset_plat_dev(usb1_pwr_dis);
                    msleep(1000);
                    unreset_plat_dev(usb1_pwr_dis);
                    msleep(1000);
                    system(BIND_XHCI_CONTROLLER);
                    msleep(1000);
                }
            }
        } else {
            printf("USB slot%d XHCI super speed enabled\n", slot); 
            fflush(stdout);
            break;
        }
    }

    /* Wait until sys stable */
    msleep(DELAY_USBCMD);
    
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

#if 0
    uint rdval = 0;
    /* check FPGA eUSB present bit */
    dash_fpga_reg_read(M2_MODULE_STS_CTL_REG, &rdval);
    if (!(rdval & M2_USB_2p0_PRESENT_BIT)) {
        printf("eUSB does NOT exist. Skip the test...\n");
        return (PASSED);
    }
#endif

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
 * Function: pcie_lane_scan_test_1ru
 *
 * Description : PCI interface scan/check
 *
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
static int pcie_lane_scan_test_1ru (void)
{
    int ix, result = PASSED, scan_num = 3;
    uint32_t bus[5], reg_val[5], cap_val[5], sta_val[5];
    uint32_t cap_s, sta_s, sta_w; /*cap_w*/
    uint rdval = 0;
    char dev_name[4][20] = {"I350", "BCM57412", "FPGA", "NVME"};
    uint32_t dev_vid[4] = {PCIE_I350_VID, PCIE_BCM57412_VID, PCIE_FPGA_VID, PCIE_NVME_VID}; 
    uint32_t dev_did[4] = {PCIE_I350_DID, PCIE_BCM57412_DID, PCIE_FPGA_DID, PCIE_NVME_DID};
    uint32_t dev_width[4] = {PCI_EXP_LINK_STA_WID_2, PCI_EXP_LINK_STA_WID_4,
                             PCI_EXP_LINK_STA_WID_1, PCI_EXP_LINK_STA_WID_2};
    /* please note dev width is the same order as dev_vid and dev_did */

    char *tname = "PCI lane scan";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* Scan NVMe if present, check FPGA NVMe present bit */
    dash_fpga_reg_read(M2_MODULE_STS_CTL_REG, &rdval);
    if (rdval & M2_PCIE_PRESENT_BIT) {
        scan_num = 4;
    }

    for (ix = 0; ix < scan_num; ix++) {

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s : testing vendor id 0x%x, device id 0x%x, %s\n", __FUNCTION__,
                         dev_vid[ix], dev_did[ix], dev_name[ix]);
        }

        /* Check following: I350, BCM57412, and FPGA */
        bus[ix] = get_pcie_bus_num(dev_vid[ix], dev_did[ix]);

        if (bus[ix] == UNKNOWN_PCI_BUS_NUM) { 
            cterr('f',0, "Unknown PCI bus number for device %04x:%04x",
                  dev_vid[ix], dev_did[ix]);
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
        /* cap_w = (cap_val[ix] & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT; */
        sta_w = (sta_val[ix] & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

        if ((cap_s == PCI_EXP_LINK_STA_SPD_2DOT5) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
            prpass(testpass, "Link speed is 2.5G ");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_5GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
            prpass(testpass, "Link speed is 5G ");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_8GT)) {
            prpass(testpass, "Link speed is 8G ");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
            prpass(testpass, "Link speed is 2.5G ");
        } else {
            cterr('f',0, "Link speed is not 2.5G, 5G or 8G, device id 0x%x vendor id 0x%x"\
                         "device-%s capability speed is %x status speed is %x", 
                         dev_vid[ix], dev_did[ix], dev_name[ix], cap_s, sta_s);
            result = FAILED;  /* fail through */
        }
        //printf("\n");

        /* Curie FPGA x1, i350 x2, bcm57412 x4, NVMe x2 */
        if (sta_w == dev_width[ix]) {
            if (sta_w == PCI_EXP_LINK_STA_WID_1) {
                prpass(testpass, "Link width is x1 ");
            } else if (sta_w == PCI_EXP_LINK_STA_WID_2) {
                prpass(testpass, "Link width is x2 ");
            } else if (sta_w == PCI_EXP_LINK_STA_WID_4) {
                prpass(testpass, "Link width is x4 ");
            } else if (sta_w == PCI_EXP_LINK_STA_WID_8) {
                prpass(testpass, "Link width is x8 ");
            } else {
                cterr('f',0, "Link width is not x1 x2 x4 and x8, device id 0x%x vendor id 0x%x, device-%s"\
                             "status width = %x", dev_vid[ix], dev_did[ix], dev_name[ix], sta_w);
            }
        } else {
            cterr('f',0, "Link width is not correct, device id 0x%x vendor id 0x%x, device-%s",
                         dev_vid[ix], dev_did[ix], dev_name[ix]);
            result = FAILED; /* fail through for next device */
        }
        //printf("\n");
    }

    prpass(testpass, "PCIe lane scan success. ");
    return (result);
}

/*
 * Function: pcie_lane_scan_test_2ru
 * Description : PCI link speed string
 */
static const char *pcie_link_spd_string(uint32_t sta)
{
    const char *ptr;

    if (sta == PCI_EXP_LINK_STA_SPD_2DOT5)
        ptr = "2.5G";
    else if (sta == PCI_EXP_LINK_STA_SPD_5GT)
        ptr = "5.0G";
    else if (sta == PCI_EXP_LINK_STA_SPD_8GT)
        ptr = "8.0G";
    else
        ptr = "Unknown";

    return ptr;
}

/*
 * Function: pcie_lane_scan_test_2ru
 *
 * Description : PCI interface scan/check
 *
 * Inputs: void
 *
 * Output: PASSED/FAILED
 */
static int pcie_lane_scan_test_2ru (void)
{
    int ix, result = PASSED;
    uint32_t bus, reg_val, cap_val, sta_val;
    uint32_t cap_s, sta_s, sta_w; /*cap_w*/
    struct {
        const char name[16];
        uint32_t vid;
        uint32_t did;
        uint32_t inst;
        uint32_t width;
    } devs[] = {
        {"I350", PCIE_I350_VID, PCIE_I350_DID, 1, PCI_EXP_LINK_STA_WID_2},
        {"BCM57412_0", PCIE_BCM57412_VID, PCIE_BCM57412_DID, 1, PCI_EXP_LINK_STA_WID_4},
        {"BCM57412_1", PCIE_BCM57412_VID, PCIE_BCM57412_DID, 2, PCI_EXP_LINK_STA_WID_4},
        {"FPGA", PCIE_FPGA_VID, PCIE_FPGA_DID, 1, PCI_EXP_LINK_STA_WID_1},
    };

    char *tname = "PCI lane scan";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    for (ix = 0; ix < 4; ix++) {

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s : testing vendor id 0x%x, device id 0x%x, %s\n", __FUNCTION__,
                   devs[ix].vid, devs[ix].did, devs[ix].name);
        }

        /* Check following: I350, BCM57412, and FPGA */
        bus = get_pcie_bus_num3(devs[ix].vid, devs[ix].did, devs[ix].inst);

        if (bus == UNKNOWN_PCI_BUS_NUM) {
            cterr('f',0, "Unknown PCI bus number for device %04x:%04x inst %d",
                  devs[ix].vid, devs[ix].did, devs[ix].inst);
            return (FAILED);
        }

        prpass(testpass, "%s", devs[ix].name);
        reg_val = get_pcie_cap_struct_ptr(bus, PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
        if (reg_val == FAILED) {
            cterr('f',0, "Can't get PCI cap pointer");
            return (FAILED);
        }

        cap_val = get_pcie_link_cap(bus, PCI_DEV_0, PCI_FUN_0, reg_val);
        sta_val = get_pcie_link_status(bus, PCI_DEV_0, PCI_FUN_0, reg_val);

        /* Speed - bit 0~3 */
        cap_s = cap_val & PCI_EXP_LINK_STA_SPD_MASK;
        sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
        /* Width - bit 4~9 */
        /* cap_w = (cap_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT; */
        sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

        if ((cap_s == PCI_EXP_LINK_STA_SPD_2DOT5) && (sta_s == PCI_EXP_LINK_STA_SPD_2DOT5)) {
            prpass(testpass, "Link speed is 2.5G ");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_5GT) && (sta_s == PCI_EXP_LINK_STA_SPD_5GT)) {
            prpass(testpass, "Link speed is 5G ");
        } else if ((cap_s == PCI_EXP_LINK_STA_SPD_8GT) && (sta_s == PCI_EXP_LINK_STA_SPD_8GT)) {
            prpass(testpass, "Link speed is 8G ");
        } else {
            const char *cap_spd_str, *sta_spd_str;

            cap_spd_str = pcie_link_spd_string(cap_s);
            sta_spd_str = pcie_link_spd_string(sta_s);
            cterr('f',0, "Link speed is not correct, %s current %s, CAP %s.",
                  devs[ix].name, cap_spd_str, sta_spd_str);
            result = FAILED;  /* fail through */
        }

        /* Curie FPGA x1, i350 x2, bcm57412 x4 */
        if (sta_w == devs[ix].width) {

            if (sta_w == PCI_EXP_LINK_STA_WID_1) {
                prpass(testpass, "Link width is x1 ");
            } else if (sta_w == PCI_EXP_LINK_STA_WID_2) {
                prpass(testpass, "Link width is x2 ");
            } else if (sta_w == PCI_EXP_LINK_STA_WID_4) {
                prpass(testpass, "Link width is x4 ");
            } else if (sta_w == PCI_EXP_LINK_STA_WID_8) {
                prpass(testpass, "Link width is x8 ");
            } else {
                cterr('f',0, "Link width is not x1 x2 x4 and x8, "
                      "device id 0x%x vendor id 0x%x instance %d, device-%s",
                      devs[ix].vid, devs[ix].did, devs[ix].inst, devs[ix].name);
            }
        } else {
            cterr('f',0, "Link width is not correct, "
                  "device id 0x%x vendor id 0x%x instance %d, device-%s, "
                  "current %d, expect %d",
                  devs[ix].vid, devs[ix].did, devs[ix].inst, devs[ix].name,
                  sta_w, devs[ix].width);
            result = FAILED; /* fail through for next device */
        }
    }

    prpass(testpass, "PCIe lane scan success. ");
    return (result);
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
    if (is_curie_1ru())
        return pcie_lane_scan_test_1ru();
    else if (is_curie_2ru())
        return pcie_lane_scan_test_2ru();
    return 0;
}

 /*
 * Function: m2_nvme_test
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int m2_nvme_test (int dummy) {

    int rc = FAILED;
    char *tname = "NVME read/write";
    char *nvme_dev = "/dev/nvme0n1";

    testname("%s access", tname);

    if (check_skip_test(mb_skip_item_name[M2_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    if (get_enhance_err_flag()) {
        add_msata_err_report();
    }

    prpass(testpass, "%s, ", tname);
    
    rc = sata_tests((uchar *)nvme_dev);
    if (rc == FAILED) {
        cterr('f',0, "NVME test failed.");
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
static int m2_sata_test (int dummy) {

    int rc = FAILED;
    char *tname = "M.2 SATA read/write";
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

    rc = sata_tests((uchar *)m2_dev);
    if (rc == FAILED) {
        cterr('f',0, "mSATA test failed.");
    }

    return(rc);
}

 /*
 * Function: m2_eusb_test
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int m2_eusb_test (int dummy) {

    int rc = FAILED;
    char *tname = "M.2 eUSB read/write";
    char *nvme_dev = "/dev/m2eusb";

    testname("%s access", tname);

    if (check_skip_test(mb_skip_item_name[M2_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    if (get_enhance_err_flag()) {
        add_msata_err_report();
    }

    prpass(testpass, "%s, ", tname);

    rc = sata_tests((uchar *)nvme_dev);
    if (rc == FAILED) {
        cterr('f',0, "M.2 eUSB test failed.");
    }

    return(rc);
}

 /*
 * Function: m2_combo_tests
 * This is just a wrapper to support the M.2 SATA and NVME modules
 *
 * Inputs: dummy
 *
 * Output: PASSED/FAILED
 */
static int m2_combo_test (int dummy) {

    char sys_cmd[256];
    char *temp_file="temp_file";
    FILE *fp;

    int rc = FAILED;
    char *tname = "M.2 NVME or SATA read/write";
    char *nvme_dev = "nvme0n1";
    char *sata_dev = "m2sata";
    char *eusb_dev = "m2eusb";
    int word_count;

    if (is_curie_2ru())
        tname = "M.2 NVME or SATA or eUSB read/write";

    testname("%s access", tname);

    uint rdval = 0;
    /* check FPGA NVMe present bit */
    dash_fpga_reg_read(M2_MODULE_STS_CTL_REG, &rdval);
    if (is_curie_2ru()) {
        if (rdval & M2_PCIE_PRESENT_BIT) {
            printf("NVMe device present\n");
        } else if (rdval & M2_USB_2p0_PRESENT_BIT) {
            printf("M2 eUSB device present\n");
        } else if (rdval & M2_MODULE_PRESENT_BIT) {
            printf("M2 SATA device present\n");
        } else {
            printf("WARNING: no M2 device detected by FPGA: %x\n", rdval);
        }
    } else {
        if (!(rdval & M2_PCIE_PRESENT_BIT)) {
            printf("NVMe does NOT exist. Skip the test...\n");
            return (PASSED);
        }
    }

    if (check_skip_test(mb_skip_item_name[M2_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    /* Check if NVME module exists
     */
    sprintf(sys_cmd, "rm -f %s; ls /dev | grep %s | wc -w > %s;", temp_file, nvme_dev, temp_file);
    system(sys_cmd);

    fp = fopen(temp_file, "r");
    if (fp == NULL) {
        printf("Failed to open %s", temp_file);
        return (FAILED);
    }
    word_count = 0;
    fscanf(fp, "%d", &word_count); /* Scan in the value */
    fclose(fp);

    if (word_count != 0) {
        return(m2_nvme_test(0));
    }

    /* If NVME does not exist, check if M.2 SATA module exists
     */
    sprintf(sys_cmd, "rm -f %s; ls /dev | grep %s | wc -w > %s;", temp_file, sata_dev, temp_file);
    system(sys_cmd);

    fp = fopen(temp_file, "r");
    if (fp == NULL) {
        printf("Failed to open %s", temp_file);
        return (FAILED);
    }
    word_count = 0;
    fscanf(fp, "%d", &word_count); /* Scan in the value */
    fclose(fp);

    if (word_count != 0) {
        return(m2_sata_test(0));
    }

    if (is_curie_2ru()) {
        /* If M.2 SATA does not exist, check if M.2 eUSB module exists
         */
        sprintf(sys_cmd, "rm -f %s; ls /dev | grep %s | wc -w > %s;", temp_file, eusb_dev, temp_file);
        system(sys_cmd);

        fp = fopen(temp_file, "r");
        if (fp == NULL) {
            printf("Failed to open %s", temp_file);
            return (FAILED);
        }
        word_count = 0;
        fscanf(fp, "%d", &word_count); /* Scan in the value */
        fclose(fp);

        if (word_count != 0) {
            return(m2_eusb_test(0));
        }
    }

    /* If neither NVME or SATA device exists, return FAILED
     */
    prpass(testpass, "%s, ", tname);
    if (is_curie_2ru())
        cterr('f',0, "No M.2 NVME or SATA or eUSB device found in system.");
    else
        cterr('f',0, "No M.2 NVME or SATA device found in system.");

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
    const char *result_file;

    if (is_curie_2ru()) {
        result_file = "/curie-2RU-diag/skip_test.txt";
    } else {
        result_file = "/curie-1RU-diag/skip_test.txt";
    }

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
 *-----------------------------------------------------------------------------
$Log: mb_tests.c,v $
Revision 1.4  2021/04/17 16:29:27  leifen
*** empty log message ***

Revision 1.3  2020/08/11 10:48:22  jiajliu
Leverage the fan speed test of Curie 1RU

Revision 1.2  2020/03/11 17:50:36  jiajliu
Fix BCM57412 MAC programming and bump version to 1.2.0

Revision 1.1  2020/01/09 01:02:00  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
