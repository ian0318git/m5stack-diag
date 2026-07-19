/* $Id: platform_led.c,v 1.2 2017/08/02 14:21:48 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_led.c,v $
 *------------------------------------------------------------------
 *
 * platform_led.c
 *
 * Copyright (c) 2014-2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"

extern int do_all_menu_items(struct menuinfo *);
extern int tsn_all_green_leds_on(int);
extern int tsn_all_yellow_leds_on(int);
extern int tsn_all_leds_off(int);
extern int tsn_leds_test(int);
/*
 * LED Test Menu
 */

static submenu_xtable_t led_test_table[] = {
    {"LED test", (type_t(*)())tsn_leds_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"turn all Green LED on", (type_t(*)())tsn_all_green_leds_on, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"turn all Yellow LED on", (type_t(*)())tsn_all_yellow_leds_on, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"turn all LED off", (type_t(*)())tsn_all_leds_off, TRUE,
     0,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define LED_TEST_TABLE_SZ \
        (sizeof(led_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_pri_test_items[LED_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t led_sec_test_items[LED_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t led_test_menu = {
    "LED Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    led_pri_test_items,
};
static menuinfo_t *led_test_menup = &led_test_menu;


/**********************************************************************
 *
 * Function: build_led_test_menu
 *
 * Description: Build LED test menu.
 *
 * Inputs: db_test_items_executed - TRUE for do all of tests. FALSE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int build_led_test_menu (int db_test_items_executed)
{
    int rc = FAILED;
    char *tname = "LED";

    testname(tname);

    build_primary_submenu(led_test_table,
                          LED_TEST_TABLE_SZ, "LED",
                          &led_test_menup);

    build_secondary_submenu(led_test_table,
                            LED_TEST_TABLE_SZ,
                            led_sec_test_items);

    if (db_test_items_executed) {
        do_all_menu_items(&led_test_menu);
    } else {
        menu(&led_test_menu, led_sec_test_items, '\0');
    }

    return (rc);
}
/*-------------------------------------------------
$Log: platform_led.c,v $
Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.6.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.4.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.2.1  2016/07/10 10:29:34  steja
Add LED test



*/
