/* $Id: cetus_gesw_tests.c,v 1.2 2016/04/20 11:25:25 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/cetus_gesw_tests.c,v $
 *------------------------------------------------------------------
 *
 * cetus_gesw_tests.c - CETUS tests.
 *
 * Aug 2015, Mecca Ho
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <linux/types.h>

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "menu.h"
#include "proto.h"
#include "error.h"
#include "queryflags.h"
#include "router_if.h"
#include "cetus_gesw_defs.h"

/* Sub Menu used for Ethernet port tests.
 */
static submenu_xtable_t gesw_tests_submenu_table[] = {
};

#define GESW_TESTS_SUBMENU_TABLE_SIZE (sizeof(gesw_tests_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t gesw_tests_primary_items[GESW_TESTS_SUBMENU_TABLE_SIZE +
				       MAX_BASE_ITEMS];
static mitem_t gesw_tests_secondary_items[GESW_TESTS_SUBMENU_TABLE_SIZE +
					 MAX_BASE_ITEMS];

menuinfo_t gesw_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    gesw_tests_primary_items,
};
menuinfo_t *gesw_submenup = &gesw_subtest_menu;

/*------------------------------------------------------------------
 *
 * Function: gesw_run_all_tests
 * Run all the GE switch standalone tests
 *
 * Input: void
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
static int gesw_run_all_tests(void)
{
	printf("To be developed...\n");
    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: gesw_test_main
 *	This is the entry point for the GESW main test.
 *
 * Input:  show_menu = 0 show submenu, !=0 perform all tests
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
gesw_test_main (int show_menu)
{
    build_primary_submenu(gesw_tests_submenu_table,
			  GESW_TESTS_SUBMENU_TABLE_SIZE,
                          "CETUS GE switch", &gesw_submenup);
    build_secondary_submenu(gesw_tests_submenu_table,
                            GESW_TESTS_SUBMENU_TABLE_SIZE,
                            gesw_tests_secondary_items);

    if (show_menu) {
        return(gesw_run_all_tests());
    } else {
        menu(gesw_submenup, gesw_tests_secondary_items, '\0' );
    }

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_gesw_line_loopback
 * Set the line loopback of the port.
 * The port_num uses the GESW_PBMP_CMIC_START numbering order.
 * User should use the ovld_get_ge_sw_port_num to get the port_num.
 *
 * Input: 
 * port_num - the GE port number
 * onoff - 1 for on, 0 for off
 *
 * Return: PASS/FAIL
 *
 *------------------------------------------------------------------
 */
int set_gesw_line_loopback(int port_num, int onoff)
{
	printf("To be developed...\n");
	return (PASSED);
}

/******** History ******** 
$Log: cetus_gesw_tests.c,v $
Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/08/11 07:44:29  meho
Added f35 nim tests.



$Endlog$
*/
