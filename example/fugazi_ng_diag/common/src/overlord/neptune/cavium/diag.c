/* $Id: diag.c,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/diag.c,v $
 *-----------------------------------------------------------------------------
 * diag.c - Menus for Neptune Cavium data plane
 *          Leverage from Overlord
 *
 * March 2011, Paul Tong
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "dash_fpga.h"
#include "../../cavium/host2dp_mbox.h"

#define F_GRP	     (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E	     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL	     (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

/*
 *  Externs
 */
extern char *banner_string;

extern int build_plat_dimm_util_menu(int);
extern int build_mux_menu(void);
extern int build_sfp_cookie_menu(int);
extern void show_dp_cpu_info(void);
extern int main_mem_test(void);
extern int eth_port_test_main(int show_menu);
extern int cavium_i2c_scan_test(void);
extern int fpga_sfp_intr_test(void);
extern void rtn_fail_msg(uint);
extern void rtn_pass_msg(uint);
extern int check_cavium_eeprom_loaded(void);
extern int pcie_cfg_reg_dump_util(void);

static int process_host_msg(void);
static int for_uart_msg_exh_test(void);

/*
 * Global variables
 */
fru_table_t platform_fru_table[];

/* FRU PID and Location Strings */
uchar mb_pid[] = "MB-PID";
uchar dimm_pid[] = "DIMM-PID";
uchar sfp_pid[] = "SFP-PID";

uchar mb_loc[] = "MB";
uchar dimm0_loc[] = "MB/DIMM0";
uchar dimm1_loc[] = "MB/DIMM1";
uchar sfp0_loc[] = "MB/SFP0";
uchar sfp1_loc[] = "MB/SFP1";
uchar sfp2_loc[] = "MB/SFP2";
uchar sfp3_loc[] = "MB/SFP3";

fru_table_t platform_fru_table[] = {
    { mb_pid,        mb_loc },
    { dimm_pid,      dimm0_loc },
    { dimm_pid,      dimm1_loc },
    { sfp_pid,       sfp0_loc },
    { sfp_pid,       sfp1_loc },
    { sfp_pid,       sfp2_loc },
    { sfp_pid,       sfp3_loc },
};

/*
 *  Globals  
 */
int netflashbooted = 1; /* menu.c need this */

/*
 * Basic utilities menu.
 */
static struct mitem utilmenuitems[] = {
    {"alter memory",                0,0,(PFT)alt_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"compare memory block",        0,0,(PFT)cmp_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"move memory block",           0,0,(PFT)mov_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"display memory",              0,0,(PFT)dis_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"fill memory",                 0,0,(PFT)fil_mem,                   (type_t *)&one,            0,            (type_t(*)())0, 0},
    {"DIMM utility",                0,0,(PFT)build_plat_dimm_util_menu, (type_t *)&one,        0,            (type_t(*)())0, 0},
    {"Mux utility",                 0,0,(PFT)build_mux_menu,            (type_t *)&zero,       0,            (type_t(*)())0, 0},
    {"SFP 0 cookie utility",         0,0,(PFT)build_sfp_cookie_menu,     (type_t *)&one,        0,            (type_t(*)())0, 0},
    {"SFP 1 cookie utility",         0,0,(PFT)build_sfp_cookie_menu,     (type_t *)&two,        0,            (type_t(*)())0, 0},
    {"SFP+ 0 cookie utility",         0,0,(PFT)build_sfp_cookie_menu,     (type_t *)&three,      0,            (type_t(*)())0, 0},
    {"SFP+ 1 cookie utility",         0,0,(PFT)build_sfp_cookie_menu,     (type_t *)&four,       0,            (type_t(*)())0, 0},
    {"Show data plane cpu info",    0,0,(PFT)show_dp_cpu_info,          (type_t *)&zero,       0,            (type_t(*)())0, 0},    
    {"Show PCIE config reg",        0,0,(PFT)pcie_cfg_reg_dump_util,    (type_t *)&zero,       0,         (type_t(*)())0, 0},
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
 * Main Menu.
 */
submenu_xtable_t main_menu_table[] = {
    /*
     * Note: Please do not add or delete menu item before
     * the dummy item for uart test.
     */
    {"Memory test",                 (PFT)main_mem_test,   0,
	    F_ALL_E, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Ethernet port test",          (PFT)eth_port_test_main,     1,
	    F_ALL, (type_t(*)())0, 0, (type_t(*)())eth_port_test_main,   0},
    {"DASH FPGA register test",     (PFT)dash_rd_wr_test, 0,
        F_ALL_E, (type_t(*)())0, 0, (PFT)0, 0},
    {"Cavium I2C scan test",        (PFT)cavium_i2c_scan_test, 0,
        F_ALL_E, (type_t(*)())0, 0, (PFT)0, 0},
    {"FPAG SFP intr test",          (PFT)fpga_sfp_intr_test, 0,
        F_ALL_E, (type_t(*)())0, 0, (PFT)0, 0},

    /* This item must be fixed in this position in the menu.
     * The host side need it for testing the uart interface.
     */
    {"Dummy item to send string to UART", (PFT)for_uart_msg_exh_test, 0,
        0, (type_t(*)())0, 0, (PFT)0, 0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
  "DP (Data Plane) %s",                        /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
  main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 * The menu driven diagnostics are called from either the low level
 * power-on routine or from the monitor.  Frommon will be non-zero if
 * called from the monitor.  Note that this value is actually the
 * argc parameter (a little slight of hand here), which is always
 * greater than zero.
 *
 * The main menu is now defined in an _xtable_.  Both the primary items
 * and the secondary (shadow) items are built with function calls that
 * operate on it and insert the appropriate base items into the menu.
 */
void
diag_menu (int argc, char *argv[])
{
    char arg;
    
    if (argc > 1) {
        arg = *argv[1];
    } else {
        arg = 0;
    }

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    /* If arg == 1, the diag is invoked via the Linux nc command from
     * the host side.
     */
    if(arg == '1') {
	process_host_msg();
    }
    else {
        menu(&maindiag, main_menu_secondary_items, arg);
    }
}

/*
 * Function: process_host_msg
 *
 * Description: Get a message from host from the mail box and execute it
 *
 * Input: void
 *
 * Return: PASS/FAIL
 */
int process_host_msg (void)
{
    mbox_msg_t msg_ctl, msg_id;
    uint result;

    if (is_mbox_empty(in_mbxp)) {
	printf("DP: Inbox is empty.\n");
        rtn_fail_msg(MBOX_MSG_DP_UNKNOWN);
        return(FAIL);
    }

    msg_ctl = get_msg();
    msg_id = get_msg_id(msg_ctl);
    ack_msg(); /* clean valid bit */

    printf("DP: Received message %#.8x from host.\n", msg_id);

    if (msg_id & MBOX_FLAG_MSG_DP_GE_INT_LPBK) {
        diagflag_xram |= D_GE_INT_LOOPBACK;
    } else {
        diagflag_xram &= ~D_GE_INT_LOOPBACK;
    }

    err_accum = 0;
    switch (msg_id & 0xFFFF) {
    case MBOX_MSG_DP_MAIN_TEST:
	printf("DP: Start main test.\n");
	exec_doall_menu_items(&maindiag);
	break;

    case MBOX_MSG_DP_MAIN_NOEXT_TEST:
	printf("DP: Start main test (no ext loopback)\n");
        (NVRAM)->diagflag |= D_EXT_LOOPBACK; /* set 1 to turn off flag */
	exec_doall_menu_items(&maindiag);
	break;

    case MBOX_MSG_DP_HELLO_TEST:
	printf("DP: Reply hello message.\n");
	break;
    default:
        err_accum = 1;
	printf("DP: Unknow message from host. No test is run.\n");
        break;
    }

    if (err_accum) {
        rtn_fail_msg(msg_id);
	printf("DP: Test msg %#.8x failed.\n", msg_id);
        result = FAIL;
    } else {
        rtn_pass_msg(msg_id);
	printf("DP: Test msg %#.8x passed.\n", msg_id);
	result = PASS;
    }
    return(result);
}

/*
 * Function: show_dp_cpu_info
 *
 * Description: This a utility menu item to show the Cavium CPU information
 *
 * Input: none
 *
 * Return: void
 */
void show_dp_cpu_info(void)
{
    printf("%s", banner_string);
    printf("Data Plane CPU Info:\n");

    FILE *fp;
    char buf[128], fld_name[32];
    char *fname;

    /* Get info from /proc/cpuinfo file
     */
    fname = "/proc/cpuinfo";
    fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("%s Failed to open %s\n", __FUNCTION__, fname);
	return;
    }

    while (!feof(fp)) {
	fgets(buf, sizeof(buf), fp); // get each line

	if ((strstr(buf, "system type") != NULL) ||
	    (strstr(buf, "processor") != NULL) ||
	    (strstr(buf, "cpu model") != NULL)) {
	    printf("%s", buf);
	}
    }    
    fclose(fp);


    /* Get info from /proc/octeon_info
     */
    fname = "/proc/octeon_info";
    fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("%s Failed to open %s\n", __FUNCTION__, fname);
	return;
    }

    while (!feof(fp)) {
      fscanf(fp, "%s %s", fld_name, buf);

      if (strstr(fld_name, "processor_id") != NULL) {
	  printf("Processor ID:  %s\n", buf);
      }
      if (strstr(fld_name, "dram_size") != NULL) {
	  printf("Dram size:     %s MB\n", buf);
      }
      if (strstr(fld_name, "eclock") != NULL) {
	  printf("Core clock:    %s Hz\n", buf);
      }
      if (strstr(fld_name, "io_clock") != NULL) {
	  printf("IO clock:      %s Hz\n", buf);
      }
      if (strstr(fld_name, "dclock") != NULL) {
	  printf("DDR clock:     %s Hz\n", buf);
      }
    }    
    fclose(fp);

    return;
}

/*
 * Function: for_uart_msg_exh_test()
 *
 * 'uname -a' to generate strings for CP to catch via
 * UART.
 * This is not test item for DP.
 *
 * Input : NONE
 * Output: PASS
 */
int
for_uart_msg_exh_test (void) {

    /* using 'uname -a' to dispay system info as a string.
     * x86 side will compare string for uart test 
     */ 
    system("uname -a");
    return PASS;
}


/*-------------------------------------------------------------------
 *
 * Function : is_goldbeach
 * Description: Return TRUE if platform is Goldbeach
 *              This function returns FALSE by default. If platform
 *              is Goldbeach, declare this function in platform code
 *
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int is_goldbeach (void)
    __attribute__((weak, alias("__is_goldbeach")));
int __is_goldbeach (void)
{
    return (FALSE);
}

/*-------------------------------------------------
$Log: diag.c,v $
Revision 1.2  2018/05/18 09:24:56  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.10  2018/05/17 10:50:23  alpeng
 sync with trunk <trunk-051618>

Revision 1.1.2.9  2018/02/07 03:24:49  meho
Corrected SFP+ cookie utility name.

Revision 1.1.2.8  2017/07/05 02:38:12  alpeng
fixed for prrq comments

Revision 1.1.2.7  2016/12/27 08:55:48  meho
Fixed show error count bug.

Revision 1.1.2.6  2016/12/27 08:22:42  meho
Corrected the print Pass location.

Revision 1.1.2.5  2016/12/27 02:01:42  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.1.2.4  2016/12/09 09:01:37  alpeng
enable phy related test from do_grp to do_all; since phy test is done

Revision 1.1.2.3  2016/11/08 10:00:32  alpeng
upate fpga table for SFP+

Revision 1.1.2.2  2016/11/03 08:26:54  alpeng
merge octeon_test.c with o2

Revision 1.1.2.1  2016/06/06 05:58:50  xiaoyizh
Initial Check-in for Neptune Data Plane diags.

Revision 1.24  2013/11/13 11:47:13  hroni
use one, two, three, and four from menu.h

Revision 1.23  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.22  2012/12/12 01:32:47  ptong
Added show diag version to util menu

Revision 1.21  2012/11/02 00:55:51  ptong
Add comment and clean-up

Revision 1.20  2012/11/01 19:17:50  ptong
Support checking Cavium PCIe BAR 0-2 setting loaded from EEPROM

Revision 1.19  2012/10/20 01:27:00  ptong
Bug fix: CSCuc79132 SGMII and SFP ext loopback failing randomly on different ports

Revision 1.18  2012/10/03 11:51:18  danchung
Add an utility to check the eeprom content.

Revision 1.17  2012/09/17 08:28:56  alpeng
revert uart_intf_test(), add uart_msg_exh_test()

Revision 1.16  2012/09/14 06:43:12  alpeng
move uart_loopback_test() to secondary item to avoid main test will invoke this item

Revision 1.15  2012/09/12 02:34:08  alpeng
do cavium uart loopback test with trigger cavium diag item

Revision 1.14  2012/08/11 00:00:18  ptong
Remove complile flag RELEASE_CVMX_DIAG

Revision 1.13  2012/06/25 07:02:14  alpeng
revert method for storing diag flag

Revision 1.12  2012/06/14 22:37:43  ptong
Add FPGA SFP interrupt test

Revision 1.11  2012/06/06 08:29:24  alpeng
clean up compiler warnings.

Revision 1.10  2012/06/01 22:58:25  ptong
Update util to print multi-core cpu info

Revision 1.9  2012/05/30 09:36:54  alpeng
suppoted i2c scan test on cavium side, removing useless definition on platform_i2c.h

Revision 1.8  2012/05/12 00:00:07  ptong
Added DP_MAIN_NOEXT_TEST to cavium

Revision 1.7  2012/05/08 00:05:15  ptong
Improve test printing

Revision 1.6  2012/04/29 04:38:30  ptong
Fix diag flag problem

Revision 1.5  2012/04/17 22:01:26  ptong
Added more utility to run DP test from host.

Revision 1.4  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.3  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:20  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module

$Endlog$
*/
