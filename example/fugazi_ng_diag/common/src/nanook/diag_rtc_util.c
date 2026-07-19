 /* $Id: diag_rtc_util.c,v 1.2 2019/12/11 10:10:31 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_rtc_util.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_rtc_util.c
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
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
#include "cross_platform.h"
#include "nvmonvars.h"
#include <string.h>
#include "diag_rtc_util.h"
#include "diag_rtc_test.h"


/*
 * static functions prototypes 
 */
static int utility_display_time (int check);
static int show_rtc_regs (void);
static int rtc_set (void);

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
 * Function: utility_display_time
 *
 * Description: Utility to display rtc time
 *
 * Inputs: check - did not use
 *
 * Outputs: PASSED
 *
 **********************************************************************
 */
static int utility_display_time (int check)
{
    rtc_time_t tv_old;

    rtc_init();

    rtc_read_time(&tv_old);
    BCD_TO_BIN(tv_old.month);
    BCD_TO_BIN(tv_old.date);
    BCD_TO_BIN(tv_old.year);
    BCD_TO_BIN(tv_old.hour);
    BCD_TO_BIN(tv_old.minute);
    BCD_TO_BIN(tv_old.second);

    printf("\n 20%02d/%.2d/%.2d, %.2d:%.2d:%.2d\n", tv_old.year, tv_old.month, tv_old.date, tv_old.hour, tv_old.minute, tv_old.second);

    rtc_finish();

    return (PASSED);
} /* end of utility_display_time */

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
    printf("\n");

    rtc_finish();

    return (PASSED);
}

/**********************************************************************
 *
 * Function: rtc_set
 *
 * Description: To set RTC time
 *
 * Inputs:None
 *
 * Outputs: PASSED
 *
 **********************************************************************
 */
static int rtc_set (void)
{
    rtc_time_t tv_old, tv;
    unsigned int sec, min, hour, day, month, year;
    
    rtc_init();
    rtc_read_time(&tv_old);
    BCD_TO_BIN(tv_old.month);
    BCD_TO_BIN(tv_old.date);
    BCD_TO_BIN(tv_old.year);
    BCD_TO_BIN(tv_old.hour);
    BCD_TO_BIN(tv_old.minute);
    BCD_TO_BIN(tv_old.second);

    tv.year = getdec_answer(" Enter year(2000 ~ 2050): ", tv_old.year+2000, 2000, 2050);
    tv.month  = getdec_answer(" Enter Month", tv_old.month, 1, 12);
    tv.date   = getdec_answer(" Enter Date", tv_old.date, 1, 31);
    tv.hour   = getdec_answer(" Enter Hour (24 hour)", tv_old.hour, 0, 23);
    tv.minute = getdec_answer(" Enter Minute", tv_old.minute, 0, 59);
    tv.second = getdec_answer(" Enter Second", tv_old.second, 0, 59);

    printf("\n Set Date: %.4d/%.2d/%.2d and Time: %.2d:%.2d:%.2d\n", 
                tv.year, tv.month, tv.date, tv.hour, tv.minute, tv.second);

    tv.year = tv.year - 2000;

    /* Abort update cycle */
    write_byte(RTC_UPDATE_CYCLE_BYTE_B, 0x82);

    BIN_TO_BCD(tv.month);
    BIN_TO_BCD(tv.date);
    BIN_TO_BCD(tv.year);
    BIN_TO_BCD(tv.hour);
    BIN_TO_BCD(tv.minute);
    BIN_TO_BCD(tv.second);
    rtc_set_time(&tv);

    if (diag_rtc_exec(GET_SYS_RTC_DATE, &year, &month, &day, NULL) == FAILED
        || diag_rtc_exec(GET_SYS_RTC_TIME, &hour, &min, &sec, NULL) == FAILED) {
        
        printf("\n Invalid date or time!\n");
        BIN_TO_BCD(tv_old.month);
        BIN_TO_BCD(tv_old.date);
        BIN_TO_BCD(tv_old.year);
        BIN_TO_BCD(tv_old.hour);
        BIN_TO_BCD(tv_old.minute);
        BIN_TO_BCD(tv_old.second);
        rtc_set_time(&tv_old);
    }
    
    /* Update cycle normally */
    write_byte(RTC_UPDATE_CYCLE_BYTE_B, 0x0);
    write_byte(RTC_UPDATE_CYCLE_BYTE_B, 0x02);
 
    rtc_read_time(&tv);
    BCD_TO_BIN(tv.month);
    BCD_TO_BIN(tv.date);
    BCD_TO_BIN(tv.year);
    BCD_TO_BIN(tv.hour);
    BCD_TO_BIN(tv.minute);
    BCD_TO_BIN(tv.second);

    printf("\n Get Date: 20%.2d/%.2d/%.2d and Time: %.2d:%.2d:%.2d\n", 
    tv.year, tv.month, tv.date, tv.hour, tv.minute, tv.second);

    rtc_finish();
    
    return PASSED;
}

/*
 * RTC utils menu
 */
static submenu_xtable_t rtc_util_menu_table[] = {
    { "Display RTC Time", (PFT)utility_display_time,  0,
        RTC_MM_3,               (type_t(*)())0, 0, 
        (type_t(*)())0, 0 },
    { "Set RTC Time", (PFT)rtc_set,  0,
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
Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/
