/* $Id: mb_tests.c,v 1.4 2021/04/17 16:29:22 leifen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/mb_tests.c,v $
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
#include "bcm82752_test.h"
#include "i350_test.h"
#include "plug_slot.h"
#include "plug_host_fpga_lib.h"


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

extern int check_menu_flag(uint);
extern int access_device_test(char *);

extern int cpu_core_test(void);
extern int fan_speed_test(void);
extern boolean is_m2_testcard_in (void);

static int usb_tests(int);
static int eusb_tests(int);
static int emmc_tests(int);
static int m2_nvme_test(int); 
static int m2_usb_tests(int);
static int m2_combo_test(int); 
static int usb_exist(int);
static int show_m2_test(void);
int pcie_lane_scan_test(int); /* also used by linux_main.c */
int check_skip_test(char *);  /* also used by platform_i2c.c */
static type_t plug_iface_test(void);

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
uchar mb_m2usb_loc[] = "MB/m2USB";
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
    { mb_pid,        mb_m2usb_loc},
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
	
    {"(Obsolete after P1C) eUSB test",
     (PFT)eusb_tests,  0,                      MF_3,
	(type_t(*)())0,		0,
        (PFT)0,  0},

    {"M.2 slot NVMe/USB test",
        (PFT)m2_combo_test,  0,                    MF_3,
        (PFT)show_m2_test,        0,
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
     (PFT)ten_g_bcm57412_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())ten_g_bcm57412_test, 1},

    {"BCM82752 test",
     (PFT)ten_g_bcm8275x_test, 0, MF_3,
     (PFT)is_curie_1ru_p2_and_later,   0, 
     (type_t(*)())ten_g_bcm8275x_test,   1},

    {"I350 test",
     (PFT)i350_test, 1, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())i350_test,   0},

    {"CPU core test",
     (PFT)cpu_core_test, 0, MF_3,
     (type_t(*)())0,   0, 
     (type_t(*)())cpu_core_test,   0},

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
char *mb_skip_item_name[] = { "M2", "USB0", "USB1" };
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

    for (si = M2_SK; si < MB_SKIP_END; si++) {
	x_pfunc = NULL;
	if (check_skip_test(mb_skip_item_name[si]) == TRUE) {
	    x_pparam = -1; /* init to a non valid number */
	    switch(si) {
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

void add_m2usb_err_report (void)
{
    fru_table_offset = MB_M2USB;
    platform_fru_table[MB_M2USB].pid_string = mb_pid;
    platform_fru_table[MB_M2USB].location_string = mb_m2usb_loc;
    cterr_add_component("MB", "m2USB");
    cterr_add_reg_dump((PFV)display_no_reg);
    cterr_add_env_dump((PFV)display_env_err);
    cterr_add_debug("Please check whether m2USB slot is not vacant",
            "Please try to use another m2USB device");
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
    char *check_usb3_file = "/curie-1RU-diag/usb_speed.txt";
    char check_usb2_spd[] = "480";
    int ix, jx;
    
    //mfix: removed below until FPGA release...
    unsigned int rdval = 0; 
    unsigned int fpga_min_ver, usb0_pwr_dis, usb1_pwr_dis;
    dash_fpga_reg_read(FPGA_VERTYPE, &rdval);
    fpga_min_ver = (rdval&0x0F00) >> 8;
    if (fpga_min_ver < 5) {
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

        for (jx = 0; jx < 5; jx++) {
            system(UDEVTRIGGER);
            msleep(500);
            if (usb_exist(slot) != PASSED) {
                if (jx == 4) {
                    cterr('f',0, "USB slot%d not available.", slot);
                    fflush(stdout);
                    return (FAILED);
                } 

                printf("power cycle USB slot%d, retry %d\n", slot, jx);
                if (slot == 0) {
                    system(UNBIND_XHCI_CONTROLLER);
                    msleep(1000);
                    reset_plat_dev(usb0_pwr_dis);
                    msleep(1000);
                    unreset_plat_dev(usb0_pwr_dis);
                    msleep(1000);
                    system(BIND_XHCI_CONTROLLER);
                    msleep(1000);
                } else {
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
                   "Retry to enable SuperSpeed. ix=%d\n", slot, ix);
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
    char *tname = "eUSB access";
    uint rdval = 0;

    testname("%s", tname);

    prpass(testpass, "eUSB is obsoleted after HW P1C, ");
    rc = PASSED;
    if (rdval == 0) {
        return(rc);
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
 * Inputs: 0-check devices soldered on board or Device-ID 
 *
 * Output: PASSED/FAILED
 */
int pcie_lane_scan_test (int dev_id) 
{
    int ix, scan_num;
    uint32_t bus[4], reg_val[4], cap_val[4], sta_val[4];
    uint32_t cap_s, sta_s, sta_w; /*cap_w*/
    char dev_name[4][20] = {"I350", "BCM57412", "FPGA", "NVME"};
    uint32_t dev_vid[4] = {PCIE_I350_VID, PCIE_BCM57412_VID, PCIE_FPGA_VID, PCIE_NVME_VID}; 
    uint32_t dev_did[4] = {PCIE_I350_DID, PCIE_BCM57412_DID, PCIE_FPGA_DID, PCIE_NVME_DID};
    uint32_t dev_width[4] = {PCI_EXP_LINK_STA_WID_2, PCI_EXP_LINK_STA_WID_4,
                             PCI_EXP_LINK_STA_WID_1, PCI_EXP_LINK_STA_WID_2};
    /* please note dev width is the same order as dev_vid and dev_did */

    char *tname;

    if (dev_id == 0) {
        /* This is the case to check all on board devices
	 */
	tname = "Motherboard PCI lane scan";
	ix = 0;
        scan_num = 3;
    }
    else if (dev_id == PCIE_NVME_DID) {
        /* This is the case to check the plug-in M.2 NVMe card 
	 */
	tname = "M.2 NVMe PCI lane scan";
        ix = 3;
	scan_num = 4;
    }
    else {
        /* Nothing to check
	 */
        ix = scan_num = 0;
	return (PASSED);
    }
    testname("%s", tname);
      
    for (; ix < scan_num; ix++) {

        if ((NVRAM)->diagflag & D_VERBOSE) { 
            printf("%s : testing device %s, vendor id 0x%x, device id 0x%x, link width %d\n", __FUNCTION__,
		   dev_name[ix], dev_vid[ix], dev_did[ix], dev_width[ix]); 
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
            return(FAILED);
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
		return(FAILED);
            }
        } else {
            cterr('f',0, "Link width is not correct, device id 0x%x vendor id 0x%x, device-%s",
                         dev_vid[ix], dev_did[ix], dev_name[ix]);
            return(FAILED);
        }
        //printf("\n");
    }
	
    prpass(testpass, "PCIe lane scan passed. ");
    return (PASSED);
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
    char *tname = "M.2 NVMe access";
    char *nvme_dev = "/dev/m2nvme01";

    testname("%s", tname);

    if (check_skip_test(mb_skip_item_name[M2_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    if (get_enhance_err_flag()) {
        add_msata_err_report();
    }

    prpass(testpass, "M.2 NVMe read/write, ", tname);
    
    rc = access_device_test(nvme_dev);
    if (rc == FAILED) {
        cterr('f',0, "NVMe test failed.");
    }

    return(rc);
}

/*
 * Function: m2_usb_tests
 *
 * Description : M.2 USB r/w tests.
 *
 * Inputs: slot - M.2 USB slot num
 *
 * Output: PASSED/FAILED
 */
static int m2_usb_tests (int slot) 
{
    int rc = FAILED;
    char *tname = "M.2 USB access";

    testname("%s", tname);

    if (get_enhance_err_flag()) {
        add_m2usb_err_report();
    }

    prpass(testpass, "M.2 USB read/write, ", tname);

    /* testname is printed on usb_slot_tests */
    rc = m2usb_slot_tests(slot);
    if (rc == FAILED) {
        cterr('f',0,"M.2 USB test failed.");
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

    char *tname = "M.2 NVME or USB read/write";
    char *nvme_dev = "m2nvme01";
    /* pfix- Come back to fix this after udev rule change. Need to change this to use m2USB to correctly name the
     * /dev in kernel.
     */
    char *m2usb_dev = "m2usb";
    int word_count;
    uint rdval = 0;

    testname("%s access", tname);

    /* Check if test is skipped by user
     */
    if (check_skip_test(mb_skip_item_name[M2_SK]) == TRUE) {
        prpass(testpass, "Test skipped by user. ");
        return(PASSED);
    }

    /* check FPGA which M.2 module present */
    dash_fpga_reg_read(M2_MODULE_STS_CTL_REG, &rdval);
    if (rdval & M2_MODULE_PRESENT_BIT) {
        /* Is it NVME on PCIe bus?
	 */
        if (rdval & M2_PCIE_PRESENT_BIT) {
	    /* NVME module exists
	     */
	    prpass(testpass, "NVMe PCIe interface present bit is set ");

	    /* Check the M.2 NVMe PCIe lane scan
	     */
	    if (pcie_lane_scan_test(PCIE_NVME_DID) == FAILED) {
	        return(FAILED);
	    }
	    else {
	        printf("\n");
	    }

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
	    else {
	        prpass(testpass, "%s, ", tname);
		cterr('f',0, "NVME device not found in kernel /dev directory.");
		return (FAILED);
	    }
	}

        /* Is it Cisco M.2 USB ?
	 */
        if (rdval & M2_USB_2p0_PRESENT_BIT) {
	    /* Cisco M.2 USB module exists
	     */
	    prpass(testpass, "USB interface present bit is set ");
	    sprintf(sys_cmd, "rm -f %s; ls /dev | grep %s | wc -w > %s;", temp_file, m2usb_dev, temp_file);
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
	        return(m2_usb_tests(0));
	    }
	    else {
	        prpass(testpass, "%s, ", tname);
		cterr('f',0, "M.2 USB device not found in kernel /dev directory.");
		return (FAILED);
	    }
	}

	prpass(testpass, "%s, ", tname);
	cterr('f',0, "M.2 module present bit is set but none of NVME and USB interface bit is set. ");
	return (FAILED);
    }
    else {
        cterr('w', 0, "M.2 slot vacant. ");
	return (PASSED);
    }
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
    char *result_file = "/curie-1RU-diag/skip_test.txt";

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

/******** History ******** 
$Log: mb_tests.c,v $
Revision 1.4  2021/04/17 16:29:22  leifen
*** empty log message ***

Revision 1.3  2020/08/01 19:09:04  ptong
Add Curie-1RU fan speed test in mb_test menu

Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.44  2019/04/16 11:46:48  meho
Show M.2 slot vacant message as warning.

Revision 1.1.2.43  2019/03/27 01:13:51  ptong
Release curie 1ru V1.3.1 : Separate motheboard PCIe and M.2 NVMe care PCIe scan tests. Use /dev/m2usb for M.2 USB module according to kernel change. Check FPGA M.2 module bits correctly to support M.2 slot test

Revision 1.1.2.42  2019/03/25 21:57:04  ptong
Fix PCIe lane scan test did not check both M.2 Module and PCIe intergace bit problem. Remove m2sata test.

Revision 1.1.2.41  2019/03/20 09:31:52  leschen
Modify to support checking board type is p2 build and later.

Revision 1.1.2.40  2019/03/20 03:41:29  leschen
Hide bcm57412 ext lpbk tests for P2 build and bcm82752 tests for P1C and older build

Revision 1.1.2.39  2019/03/18 02:28:47  meho
Fixed Retry enable USB SuperSpeed bug

Revision 1.1.2.38  2019/03/12 09:08:57  meho
Changed M.2 NVMe device name for udev rule.

Revision 1.1.2.37  2019/03/12 07:52:26  leschen
Support BCM82752

Revision 1.1.2.36  2019/02/26 02:55:47  meho
check NVMe/eUSB exist by FPGA

Revision 1.1.2.35  2019/02/23 06:41:21  meho
Check M.2 present bit for NVMe PCIe lane scan.

Revision 1.1.2.34  2019/02/21 08:02:43  meho
Removed USB power cycle when detect the wrong USB speed.

Revision 1.1.2.33  2019/02/20 03:33:35  meho
Don't scan NVMe PCI lane until FPGA present bit ready.

Revision 1.1.2.32  2019/02/20 03:05:27  meho
Support NVMe in PCIe lane scan.

Revision 1.1.2.31  2019/02/13 07:45:26  meho
Removed power down non-test USB slot before USB test.

Revision 1.1.2.30  2019/01/24 03:40:11  meho
Fixed USB detection issue.

Revision 1.1.2.29  2019/01/17 07:26:44  meho
Removed switch to EHCI in USB test.

Revision 1.1.2.28  2018/12/27 09:47:02  leschen
Print out more info when PCIe lane scan failure occur.

Revision 1.1.2.27  2018/12/25 08:52:43  meho
Added disable/enable non-test USB power before/after USB test.

Revision 1.1.2.26  2018/12/14 22:10:57  ptong
Fix bug in usb3.0 test which does not fail when 2.0 stick is used instead of 3.0

Revision 1.1.2.25  2018/11/01 23:23:05  ptong
Fix USB test. Use FPGA device reset register to reset USB stick to retry.

Revision 1.1.2.24  2018/10/22 09:33:32  meho
Added pluggable in i/o interface test.

Revision 1.1.2.23  2018/10/08 21:54:36  ptong
Combine NVME and M.2 SATA device test in one menu item

Revision 1.1.2.22  2018/10/04 22:13:00  ptong
Improve USB XHCI and EHCI test messages

Revision 1.1.2.21  2018/10/03 03:38:59  alpeng
enhance message for usb recover to XHCI

Revision 1.1.2.20  2018/09/27 09:46:24  alpeng
support tam lib and aikido for curie

Revision 1.1.2.19  2018/09/27 08:05:51  meho
Added multi-core test.

Revision 1.1.2.18  2018/09/13 23:18:43  ptong
Add Thallium support in linux_main.c and mb_tests.c

Revision 1.1.2.17  2018/09/12 09:33:59  meho
Added 5sec delay in the end of USB test.

Revision 1.1.2.16  2018/09/07 00:09:36  ptong
Make skip plugin work on Cuire-1RU. Remove AUX port test

Revision 1.1.2.15  2018/09/06 01:30:39  ptong
Fixed hard coded /nep-diag to /curie-1RU-diag

Revision 1.1.2.14  2018/08/31 00:53:29  ptong
Curie 1RU does not support AUX port. Remove test from menu

Revision 1.1.2.13  2018/08/24 19:07:04  meho
Fixed plug i2c address

Revision 1.1.2.12  2018/08/15 21:58:24  leschen
Support NVME test and add I350 menu

Revision 1.1.2.11  2018/08/15 19:37:27  alpeng
 update i2c scan test for PSU and USB redriver

Revision 1.1.2.10  2018/08/10 08:15:54  alpeng
update eth num for sm1; support sync signal test for curie; skip plx pcie check (wait for rommon); skip ge1 on SM for both tlk10232 and xaui loopback test (wait for rommon); remove NIM slot2 test; fixed pcie lane scan test

Revision 1.1.2.9  2018/08/09 18:36:39  meho
Removed check sata present in M.2 SATA Test(HW removed the FPGA check function)

Revision 1.1.2.8  2018/08/08 09:02:26  alpeng
fixed typo for scan FPGA, update list from NVME on header file

Revision 1.1.2.7  2018/08/08 07:46:25  alpeng
update pcie devices for pcie scan test on mb_test

Revision 1.1.2.6  2018/08/02 08:43:17  leschen
Support BCM57412

Revision 1.1.2.5  2018/07/30 08:15:39  alpeng
remove nim3, sm2,3,4 entry; update pcie scan test, except nvme (vid/did/ need to verify with HW

Revision 1.1.2.4  2018/07/12 09:46:51  alpeng
add mb and FPGA board type for curie 1RU; max sm and nim slots; clean up mb_test

Revision 1.1.2.3  2018/07/09 10:00:14  alpeng
remove pcie switch bus num init from linux_main.c and add nvme menu on mb_test.c

Revision 1.1.2.2  2018/06/28 10:19:19  alpeng
remove bcm gesw files from Makefile and put its functions into platform_stub.c for NGIO reference; will follow GB method on NGIO GE SW portion

Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.2  2018/05/30 07:23:09  alpeng
able to compile curie_diag

Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
porting neptune x86 to curie

$Endlog$
*/
