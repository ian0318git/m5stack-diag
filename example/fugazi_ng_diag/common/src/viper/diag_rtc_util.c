 /* $Id: diag_rtc_util.c,v 1.4 2020/07/14 06:45:56 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_rtc_util.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_rtc_util.c
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "signals.h"
#include "error.h"
#include "proto.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "nvmonvars.h"
#include <string.h>
#include "diag_rtc_util.h"
#include "diag_rtc_test.h"


/*
 * static functions prototypes 
 */
static int show_rtc_regs (void);

/*
 * functions prototypes 
 */

/*
 * extern functions prototypes
 */
extern int do_all_menu_items(struct menuinfo *);
extern int getdec_answer(char *msgstr, uint, uint, uint);

/**********************************************************************
 *
 * Function: show_rtc_regs
 *
 * Description: To show RTC registers value
 *
 * Inputs:None
 *
 * Outputs: PASSED
 *
 **********************************************************************
 */
static int show_rtc_regs (void)
{
    BYTE data;

    rtc_init();

    printf("\n");
    data = read_byte(RTC_SECOND_BYTE_0);
    printf(" Seconds       (%02xh) : 0x%02x\n", RTC_SECOND_BYTE_0, data);
    data = read_byte(RTC_SECOND_ALARM_BYTE_1);
    printf(" Seconds Alarm (%02xh) : 0x%02x\n", RTC_SECOND_ALARM_BYTE_1, data);
    data = read_byte(RTC_MINUTE_BYTE_2);
    printf(" Minutes       (%02xh) : 0x%02x\n", RTC_MINUTE_BYTE_2, data);
    data = read_byte(RTC_MINUTE_ALARM_BYTE_3);
    printf(" Minutes Alarm (%02xh) : 0x%02x\n", RTC_MINUTE_ALARM_BYTE_3, data);
    data = read_byte(RTC_HOUR_BYTE_4);
    printf(" Hours         (%02xh) : 0x%02x\n", RTC_HOUR_BYTE_4, data);
    data = read_byte(RTC_HOUR_ALARM_BYTE_5);
    printf(" Hours Alarm   (%02xh) : 0x%02x\n", RTC_HOUR_ALARM_BYTE_5, data);
    data = read_byte(RTC_DAY_OF_WEEK_BYTE_6);
    printf(" Day of Week   (%02xh) : 0x%02x\n", RTC_DAY_OF_WEEK_BYTE_6, data);
    data = read_byte(RTC_DAY_OF_MONTH_BYTE_7);
    printf(" Day of Month  (%02xh) : 0x%02x\n", RTC_DAY_OF_MONTH_BYTE_7, data);
    data = read_byte(RTC_MONTH_BYTE_8);
    printf(" Month         (%02xh) : 0x%02x\n", RTC_MONTH_BYTE_8, data);
    data = read_byte(RTC_YEAR_BYTE_9);
    printf(" Year          (%02xh) : 0x%02x\n", RTC_YEAR_BYTE_9, data);
    data = read_byte(RTC_REG_A);
    printf(" Register A    (%02xh) : 0x%02x\n", RTC_REG_A, data);
    data = read_byte(RTC_UPDATE_CYCLE_BYTE_B);
    printf(" Register B    (%02xh) : 0x%02x\n", RTC_UPDATE_CYCLE_BYTE_B, data);
    data = read_byte(RTC_REG_C);
    printf(" Register C    (%02xh) : 0x%02x\n", RTC_REG_C, data);
    data = read_byte(RTC_REG_D);
    printf(" Register D    (%02xh) : 0x%02x\n", RTC_REG_D, data);
    data = read_byte(RTC_CENTURY);
    printf(" Century       (%02xh) : 0x%02x\n", RTC_CENTURY, data);
    printf("\n");

    rtc_finish();

    return (PASSED);
}

/*
 * RTC utils menu
 */
static submenu_xtable_t rtc_util_menu_table[] = {
    { "Display RTC Time", (PFT)utility_get_rtc,  0,
        RTC_MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
    { "Set RTC Time", (PFT)utility_set_rtc,  0,
        RTC_MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
    { "Dump RTC Registers", (PFT)show_rtc_regs,  0,
        RTC_MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
};

#define RTC_UTIL_MENU_TABLE_SIZE (sizeof(rtc_util_menu_table) / \
                                        sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t rtc_util_menu_primary_items[RTC_UTIL_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];
static mitem_t rtc_util_menu_secondary_items[RTC_UTIL_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];


static struct menuinfo rtcutildiag = {
    "RTC Utility Menu",      /* title */
    0,                            /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* generic prompt */
    0,                            /* size -- bumped by add_menu_item() */
    rtc_util_menu_primary_items,
};

static struct menuinfo *rtcutildiagp = &rtcutildiag;

/*************************************************************************
 * Function:    build_rtc_utils_menu
 *
 * Description: Entry to RTC chip utilities menu.
 *
 * Inputs:      None.
 *
 * Outputs:     PASSED.
 *
 *************************************************************************
 */
int build_rtc_utils_menu (boolean rtc_util_items_executed)
{
    build_primary_submenu(rtc_util_menu_table, RTC_UTIL_MENU_TABLE_SIZE,
                            "RTC Utility Main Menu", &rtcutildiagp);
    build_secondary_submenu(rtc_util_menu_table, RTC_UTIL_MENU_TABLE_SIZE,
                            rtc_util_menu_secondary_items);

    menu(rtcutildiagp, rtc_util_menu_secondary_items, '\0' );

    return PASSED;
}

/******** History ********
$Log: diag_rtc_util.c,v $
Revision 1.4  2020/07/14 06:45:56  harrchan
Fixed bug of RTC set/display utility(CSCvu92708)

Revision 1.3  2018/11/09 08:48:12  lucywang
Set RTC standard RAM byte offset 14 to zero for IOS

Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.3  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.2  2018/06/06 03:16:07  lucywang
Fixed CSCvj80723-RTC register test failed

Revision 1.1.2.1  2018/03/29 01:11:20  lucywang
Added RTC test and utility


$Endlog$
*/
