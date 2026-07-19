/* $Id: diag_mem.c,v 1.2 2016/04/20 11:25:28 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mem.c,v $
 *------------------------------------------------------------------
 *
 * diag_mem.c - Memory Test
 *
 * Jan 2015, Hsuan-Ming Yang adapted from Overload.
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <fcntl.h>
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
#include "strings.h"

/* Function prototype */
extern int  alt_mem(), dis_mem(), fil_mem(), mov_mem(), cmp_mem();
extern int  memtest(), memloop(), addrloop(), find_mem();
extern int  mb_board_type(void);
/* MDIO utilities */
extern int vm_test(int);
extern int mb_tests(int flag);
extern int act2_prog(int);

int reset_sys_by_watchdog(void);

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

    {"Memory debug utilities",        0, 0,
     (PFT)menu, (type_t *)&mem_debug_menup, 0,
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
    {"motherboard tests",
     (PFT)mb_tests,		TRUE,		MM_2,
     (type_t(*)())0, 0,		(PFT)mb_tests,	FALSE},
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


/*---------------------------------------------------------------
$Log: diag_mem.c,v $
Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project



$Endlog$
*/
