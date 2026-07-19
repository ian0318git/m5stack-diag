/* $Id: diag.c,v 1.6 2017/04/05 03:46:19 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag.c,v $
 *------------------------------------------------------------------
 *
 * diag.c - CSX-Tachi diagmon main menu and supporting wrappers.
 *
 * Jan 2015, Hsuan-Ming Yang adapted from Overload.
 *
 * Copyright (c) 2016-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "pcmap.h"
#include "monitor.h"
#include "mon_plat_defs.h"
#include "nvmonvars.h"
#include "menu.h"
#include "signals.h"
#include "queryflags.h"
#include "error.h"
#include "cross_platform.h"
#include "proto.h"
#include "slot.h"
#include "ngio.h"
#include "strings.h"
#include "diag_plat_cookie.h"
#include "diag_fpga_util.h"
#include "diag_i2c_util.h"
#include "diag_fan_util.h"
#include "diag_console_util.h"
#include "diag_power_lib.h"
#include "diag_margin_util.h"
#include "diag_led_util.h"
#include "diag_fpga_lib.h"
#include "diag_lewis_gesw_test.h"
#include "intel_tests.h"
#include "diag_nc_common.h"
#include "diag_mcu_util.h"
#include "diag_fpga_util.h"
#include "diag_fru_util.h"
#include "diag_rtc_test.h"
#include "diag_plat_cookie.h"

/* Function prototype */
extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem();
extern int  mb_board_type(void);
/* MDIO utilities */
extern int vm_test(int);
extern int mb_tests(int flag);
extern int intel_tests(int flag);
extern int daughtercard_test(void);
extern int act2_prog(int);
extern int io_iface_tests(int);
extern int check_lewis_alive(int);
extern int check_intel_linux_alive(int);
extern int print_all_slots(int);
extern char *banner_string;
int diag_show_rtc(void);

/* Macros */
#define BMC_UNAME_CMD "uname -a"
#define BMC_KERNEL_INFO "version.sh"

int reset_sys_by_watchdog(void);
int display_sys_info(void);
boolean is_tachi_high(void);
boolean fx3_switch_usb(void);
int get_board_ver(void);

/*
 * Cookie menu utility
 */
static struct mitem cookie_items[] = {
    {"alter MB CPU cookie",               0,    0,
     (type_t(*)())alter_mb_cookie,    &one, 0, (type_t(*)())0, 0},
    {"alter NIM cookie",          0,    0,
     (type_t(*)())alter_nim_cookie,      &one, 0, (type_t(*)())0, 0},
    {"alter PoE cookie",          0,    0,
     (type_t(*)())alter_poe_cookie,      &one, 0, (type_t(*)())0, 0},
    {"alter ISP cookie",          0,    0,
     (type_t(*)())alter_raid_cookie,      &one, 0, (type_t(*)())0, 0},
};

static struct menuinfo cookie_menu = {
    "Cookie utility Menu",
    0,
    0,
    0,
    sizeof(cookie_items)/sizeof(struct mitem),
    cookie_items,
};
static struct menuinfo *cookie_menup = &cookie_menu;


/*
 * Memory debug utility
 */
static struct mitem mem_debug_items[] = {
    {"alter memory",    	    0,		0,
     (PFT)alt_mem,   &one,	0, (type_t(*)())0, 0},
    {"compare memory block",   	    0,		0,
     (PFT)cmp_mem,		            &one,	0, (type_t(*)())0, 0},
    {"display memory",    	    0,		0,
     (PFT)dis_mem,		            &one,	0, (type_t(*)())0, 0},
    {"move memory block",    	    0,		0,
     (PFT)mov_mem,		            &one,	0, (type_t(*)())0, 0},
    {"fill memory",    	            0,		0,
     (PFT)fil_mem,		            &one,	0, (type_t(*)())0, 0},
    {"find memory",    	            0,		0,
     (PFT)memtest,		            &one,	0, (type_t(*)())0, 0},
    {"memory read or write loop",   0,		0,
     (PFT)memloop,		            &one,	0, (type_t(*)())0, 0},
    {"memory debug loop",    	    0,		0,
     (PFT)memdebug,		            &one,	0, (type_t(*)())0, 0},
    {"address loop",    	    0,		0,
     (PFT)addrloop,		            &one,	0, (type_t(*)())0, 0},
};

static struct menuinfo mem_debug_menu = {
    "Memory debug utility Menu",
    0,
    0,
    0,
    sizeof(mem_debug_items)/sizeof(struct mitem),
    mem_debug_items,
};
static struct menuinfo *mem_debug_menup = &mem_debug_menu;


/* 
 * Basic utilities
 */
static struct mitem utilmenuitems[] = {
    {"Cookie utility",        0, 0,
     (PFT)menu, (type_t *)&cookie_menup, 0,
     (type_t(*)())0,0},
    {"Memory debug utilities",        0, 0,
     (PFT)menu, (type_t *)&mem_debug_menup, 0,
     (type_t(*)())0,0},
    {"I2c Utilities",        0, 0,
     (PFT)diag_i2c_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"FPGA Utilities",        0, 0,
     (PFT)diag_fpga_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"MCU Utilities",        0, 0,
     (PFT)diag_mcu_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Console Switch Utility",        0, 0,
     (PFT)diag_console_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"RTC Utilities",        0, 0,
     (PFT)diag_show_rtc, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"FAN Utilities",        0, 0,
     (PFT)diag_fan_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Margin Utility",        0, 0,
     (PFT)diag_margin_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"FRU Utility",        0, 0,
     (PFT)diag_fru_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"LED Utilities",        0, 0,
     (PFT)diag_led_util, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Intel System Power Utility",        0, 0,
     (PFT)diag_intel_power_ctl, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Show system info",        0, 0,
     (PFT)display_sys_info, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Intel Linux Alive Check",        0, 0,
     (PFT)intel_linux_alive, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Intel Lewis Alive Check",        0, 0,
     (PFT)intel_lewis_alive, (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Poll Slots",        0, 0,
     (PFT)print_all_slots,   (type_t *)&one, 0,
     (type_t(*)())0,0},
    {"Reset system by watchdog", 0, 0, 
     (PFT)reset_sys_by_watchdog, (type_t *)&one, 0, (type_t(*)())0, 0},
};

static struct menuinfo utilmenu = {
    "Diagnostic Utilities Menu",
    0,
    0,
    0,
    sizeof(utilmenuitems)/sizeof(struct mitem),
    utilmenuitems,
};
struct menuinfo *utilmenup = &utilmenu;

/*
 * Main menu test flag defines
 */

#define MM_1	(MF_CONTINUOUS)
#define MM_2	(MM_1 | MF_DOALL)
#define MM_3	(MM_2 | MF_SHOW_ERRCOUNT)

/*=========================================
 * Main menu items
 *=========================================
 */
submenu_xtable_t main_menu_table[] = {
    {"i/o interface test",
     (PFT)io_iface_tests,       FALSE,          MM_1,
     (type_t(*)())0, 0,         (PFT)io_iface_tests, TRUE},
    {"ACT-2 utilities and programming",
     (PFT)smartchip,		FALSE,		MM_2,
     (type_t(*)())0, 0,		(PFT)smartchip,	TRUE},
    {"motherboard tests",
     (PFT)mb_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)mb_tests,	FALSE},
    {"Intel tests",
     (PFT)intel_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)intel_tests,	FALSE},
    {"Marvell switch tests",
     (PFT)diag_lewis_gesw_test,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)diag_lewis_gesw_test,	FALSE},
    {"NIM Slot 1 tests",
     (PFT)wic_test,             FIRST_SLOT,	MM_2,
     (type_t(*)())0, 0,		(PFT)wic_test,	FIRST_SLOT + MAX_WIC},
    {"NIM Slot 2 tests",
     (PFT)wic_test,             FIRST_SLOT+1,	MM_2,
     (PFT)is_tachi_high, 0,	(PFT)wic_test,	FIRST_SLOT+1 + MAX_WIC},
    {"NIM Slot 3 tests",
     (PFT)wic_test,             FIRST_SLOT+2,	MM_2,
     (PFT)is_tachi_high, 0,	(PFT)wic_test,	FIRST_SLOT+2 + MAX_WIC},    
    {"Internal Service Processor (ISP) test",
     (PFT)daughtercard_test, TRUE, MM_2,
     (type_t(*)())0, 0, (PFT)daughtercard_test,  FALSE},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "Main %s",			/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*************************************************************
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *************************************************************
 */
void
diag_menu(int argc, char *argv[]) 
{
    char arg;

    if (argc > 1)
    	arg = *argv[1];
    else
    	arg = 0;
    (NVRAM)->pollcon = 1;		/* poll the console */
    /*envflag = INDIAG;*/			/* set the environment flag */
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    menu(&maindiag, main_menu_secondary_items, arg);
}

/**********************************************************************
 *
 * Function: cli_main_menu_table_size()
 *
 * This routine is used for cli command to return the menu size
 *
 * Input : none
 *
 * Output: MAIN_MENU_TABLE_SIZE
 *
 **********************************************************************
 */
int
cli_main_menu_table_size(void)
{
    return (MAIN_MENU_TABLE_SIZE);
}

/**********************************************************************
 *
 * Function: display_sys_info
 *
 * Description: display whole system info, ie subsystem firmware version,
 *              subsystem status,
 *
 * Input :
 *
 * Output:
 *
 **********************************************************************
 */
int display_sys_info(void) {
    int ret;    

    /* Show MCU version */
    printf("SYS BASIC INFO:\n");
    ret = diag_mcu_show_ver();

    /* Show FPGA Version */
    ret = diag_fpga_ver_display();

    /* Show POE/non-POR SKU info */
    ret = is_poe_sku();

    /* BMC system information */
    printf("BMC SYS INFO:\n");
    system(BMC_KERNEL_INFO);
    system(BMC_UNAME_CMD);

    /* Show BMC diagnostic application version */
    printf("\nBMC APP INFO:\n");
    printf("%s",banner_string);

    /* Show Intel info */
    printf("\nx86 SYS INFO:\n");
    if (check_intel_linux_alive(PRE_PING_CHECK_LOOP) == PASSED) {
        printf("x86 SYS status: ON\n");
        ret = diag_nc_intel_fw_version();
        diag_nc_nim_fw_version();
    } else {
        printf("x86 SYS status: OFF\n");
    }

    /* Show GESW info */
    printf("\nGESW SYS INFO:\n");
    if (check_lewis_alive(PRE_PING_CHECK_LOOP) == PASSED) {
        printf("GESW SYS status: ON\n");
        gesw_image_info();
    } else {
        printf("GESW SYS status: OFF\n");
    }
    return (PASSED);
}


/**********************************************************************
 *
 * Function: display_brd_info
 *
 * Description: display board info, ie version number, revision number,
 *              etc...
 *
 * Input :
 *
 * Output:
 *
 **********************************************************************
 */


/**********************************************************************
 *
 * Function: power_ngio
 *
 * Description: utility to turn on/off ngio power
 *
 * Input : val -- not used
 *
 * Output: PASSED
 *
 **********************************************************************
 */

/**********************************************************************
 *
 * Function: aux_sel
 *
 * Description: entry point to modify aux selection 
 *
 * Input :
 *
 * Output:
 *
 **********************************************************************
 */

/**********************************************************************
 *
 * Function: set_uart_lpbk
 *
 * Description: entry point to put FPGA uart in lpbk
 *
 * Input :
 *
 * Output: 
 *
 **********************************************************************
 */


/**********************************************************************
 *
 * Function: reset_sys_by_watchdog
 *
 * Description: reboot system by enableing watchdog timer.
 *              this test will reboot system!!!
 *
 * Input : NONE
 *                     
 * Output: PASSED
 *
 **********************************************************************
 */
int reset_sys_by_watchdog (void)
{
    int fd_wd;

    /* Reset system by watchdog */
    fd_wd = open("/dev/watchdog", O_RDWR);
    if (fd_wd == -1) {
        cterr('f',0,"open /dev/watchdog failed. \n");
        return (FAILED);
    } else {
        printf("System will be reboot by watchdog after 1 mins.\n");
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: display_regs
 *
 * Description: display uart reg
 *
 * Input :
 *                     
 * Output:
 *
 **********************************************************************
 */

/**********************************************************************
 *
 * Function: enable_uart
 *
 * Description: enable uart
 *
 * Input :
 *                     
 * Output:
 *
 **********************************************************************
 */

/**************************************************************************
 *
 * Name: is_tachi_high
 *
 * Description: This routine reads the FPGA register FPGA_HW_TYPE_REV_RE
 * and return TRUE if it's a tachi High, FALSE if tachi Low
 *
 * Inputs: None
 *
 * Outputs: TRUE/FALSE
 *
 *************************************************************************/
boolean
is_tachi_high(void)
{
    int hw_type;

    diag_fpga_reg_read(FPGA_HW_TYPE_REV_REG, &hw_type);

    if((hw_type & HW_BOARD_TYPE_MASK) == 0) {
	return FALSE;
    } else {
	return TRUE;
    }

}

int diag_show_rtc(void)
{
    diag_rtc_utils(FALSE);
    return (PASSED);
}


/**************************************************************************
 *
 * Name: get_board_ver
 *
 * Description: Get the Fab Version and PCB Revision cookie field values,
 *              check and show what's the Tachi-L board type.
 *              Board Type    Fab Version    PCB Revision
 *                 P1B             02             01
 *                 P2A             03             02
 *                 P2B             04             05
 *                 P2B             04             0A
 *                 P2C             05             01
 *
 * Inputs: None
 *
 * Outputs: Board type value
 *
 *************************************************************************/
int get_board_ver (void)
{
    int fab_ver;
    char fab_version[COOKIE_FIELD_LEN], pcb_revision[COOKIE_FIELD_LEN];

    /* Initialize the char array. */
    memset(fab_version, 0, sizeof(fab_version));
    memset(pcb_revision, 0, sizeof(pcb_revision));

    /* Get Fab version back from cookie field. */
    get_cookie_field_val(FAB_VERSION_TYPE, fab_version);
    
    /* Get PCB revision back from cookie field. */
    get_cookie_field_val(PCB_REVISION_TYPE, pcb_revision);

    /* Fab version is 1 byte digital value. */
    fab_ver = (int) fab_version[0];

    if ((fab_ver == P1B_FAB_FIELD_VAL) &&
        !strcmp(pcb_revision, P1B_PCB_REVISION_VAL)) {
        printf("Tachi-L P1B board\n");
        return (BOARD_P1B);
    } else if ((fab_ver == P2A_FAB_FIELD_VAL) &&
               !strcmp(pcb_revision, P2A_PCB_REVISION_VAL)) {
        printf("Tachi-L P2A board\n");
        return (BOARD_P2A);
    } else if (((fab_ver == P2B_FAB_FIELD_VAL) && 
               !strcmp(pcb_revision, P2B_PCB_REVISION_VAL_05)) ||
               ((fab_ver == P2B_FAB_FIELD_VAL) &&
                !strcmp(pcb_revision, P2B_PCB_REVISION_VAL_0A))) {
        printf("Tachi-L P2B board\n");
        return (BOARD_P2B);
    } else if ((fab_ver == P2C_FAB_FIELD_VAL) &&
               !strcmp(pcb_revision, P2C_PCB_REVISION_VAL)) {
        printf("Tachi-L P2C board\n");
        return (BOARD_P2C);
    } else {
        printf("\nUnrecognized board\n");
    }

    return (BOARD_UNKNOW);
}


/**************************************************************************
 *
 * Name: fx3_switch_usb
 *
 * Description: Check if it has USB3.0 bus only. 
 *              P1B/P2C have USB2.0/USB3.0 bus.
 *              P2A/P2B only have USB3.0 bus.
 * Inputs: None
 *
 * Outputs: TRUE/FALSE
 *
 *************************************************************************/
boolean fx3_switch_usb (void)
{
    int board_type;

    /* Get the board type. */
    board_type = get_board_ver();
	/* Default firmware status supports both USB2.0 and USB3.0. */
    if ((board_type == BOARD_P2A) || (board_type == BOARD_P2B)) {
	    /* P2A and P2B don't have the USB2.0 bus. */
        return (TRUE);
    }

    return (FALSE);
}

/* --- TO BE IMPLEMENTED --- */

/*---------------------------------------------------------------
$Log: diag.c,v $
Revision 1.6  2017/04/05 03:46:19  kodko
CSCvd79127: Adds a PCB revision type for P2B.

Revision 1.5  2017/02/24 03:31:02  haohsu
Modify alter RAID cookie to alter ISP cookie

Revision 1.4  2017/01/25 01:13:13  kodko
Get the Fab Version and PCB Revision cookie field values to distinguish the board type and do the USB3.0/USB2.0 or USB3.0 only test.

Revision 1.3  2016/08/12 03:15:11  jimmyya
Add MCU utilities

Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.26  2016/04/20 00:37:59  huanngo
Remove the code for Tachi-H

Revision 1.1.2.25  2016/04/07 03:57:50  benchen2
fix rtc utility

Revision 1.1.2.24  2016/04/01 07:33:19  alpeng
support poll slots

Revision 1.1.2.23  2016/03/04 09:16:18  benchen2
add fru util

Revision 1.1.2.22  2016/03/03 04:55:59  alpeng
 add nim version into bmc util

Revision 1.1.2.21  2016/02/25 03:52:16  jimmyya
Add utility to check all the systemin info

Revision 1.1.2.20  2016/02/04 03:45:32  benchen2
change the DC test name

Revision 1.1.2.19  2016/01/20 07:13:56  hondwang
Modify for INTEL linux and lewis check utility and INTEL NC flag

Revision 1.1.2.18  2016/01/14 03:35:25  benchen2
fix DC test name

Revision 1.1.2.17  2016/01/11 10:28:15  tirawan
Add Test card menu to run FPGA i2c register test, btb test from x86 and Lewis

Revision 1.1.2.16  2016/01/07 12:35:52  benchen2
integrate led to main utility

Revision 1.1.2.15  2015/12/28 06:12:30  hondwang
Add and modify files for INTEL NC command support

Revision 1.1.2.14  2015/12/25 03:15:31  tirawan
add Marvell Lewis test menu entry

Revision 1.1.2.13  2015/12/22 17:26:02  huanngo
Support Tachi-High tests and menus CSCux15587

Revision 1.1.2.12  2015/11/13 07:57:29  tirawan
Add Voltage and Frequency Margin

Revision 1.1.2.11  2015/11/13 07:34:31  benchen2
add raid card test and utility

Revision 1.1.2.10  2015/11/02 10:22:55  tirawan
Add PoE Cookie Utility

Revision 1.1.2.9  2015/10/28 07:55:04  benchen2
add raid act2 cookies utility

Revision 1.1.2.8  2015/10/21 09:38:23  alpeng
add i/o interface entry

Revision 1.1.2.7  2015/10/01 08:38:21  tirawan
Update Temperature sensor description and add Intel power on/off utility

Revision 1.1.2.6  2015/09/30 09:15:29  benchen2
add poe act2

Revision 1.1.2.5  2015/08/20 00:53:39  alpeng
entry for nim test

Revision 1.1.2.4  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.3  2015/07/24 03:39:35  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.2  2015/07/12 06:52:45  tirawan
Add Console Switch Utility, SPI driver and FPGA programming

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/
