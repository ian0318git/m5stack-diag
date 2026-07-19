/* $Id: diag_rtc_test.c,v 1.3 2021/10/21 10:05:15 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_rtc_test.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_rtc_test.c
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
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
#include "diag_rtc_test.h"


/*
 * extern functions prototypes
 */
extern int do_all_menu_items(struct menuinfo *);
extern int getdec_answer(char *msgstr, uint, uint, uint);


int diag_rtc_exec (int test, unsigned int *param1, unsigned int *param2, unsigned int *param3, char *param4)
{
    FILE *fp;
    char buftmp[BUF_MAX], *p = buftmp;
    int c, len = 0, rtn = 0;
    char cmd[BUF_MAX] = {0};
    unsigned int sec, min, hour, day, month, year;

    switch (test) {
        case GET_SYS_RTC_TIME:
        sprintf(cmd, "cat /sys/class/rtc/rtc0/time");
        fp = popen(cmd, "r");
        break;

        case GET_SYS_RTC_DATE:
        sprintf(cmd, "cat /sys/class/rtc/rtc0/date");
        fp = popen(cmd, "r");
        break;

        /* date --set "$yr-$mon-$day" > /dev/null */
        case SET_HW_DATE:
        if (!param4) {
            return FAILED;
        }

        sprintf(cmd, "date --set \"%s\" > /dev/null", param4);
        fp = popen(cmd, "r");
        break;

        case SET_HW_TIME:
        if (!param4) {
            return FAILED;
        }
        sprintf(cmd, "date --set \"%s\" > /dev/null", param4);
        fp = popen(cmd, "r");
        break;

        case SET_HW_CLOCK:
        sprintf(cmd, "hwclock -w > /dev/null 2>&1");
        fp = popen(cmd, "r");
        break;

        /* date +"%Y-%m-%d"	*/
        case GET_HW_DATE:
        strcpy(cmd, "date +\"\%Y-\%m-\%d\"");
        fp = popen(cmd, "r");
        break;

        /* date +"%H:%M:%S" */
        case GET_HW_TIME:
        strcpy(cmd, "date +\"\%H:\%M:\%S\"");
        fp = popen(cmd, "r");
        break;

        default:
        fp = NULL;
        break;
    }

    if (fp == NULL) {
        cterr('f',0,"popen fail");
        return (FAILED);
    }

    memset(buftmp, 0, BUF_MAX);
    while((c = fgetc(fp)) != EOF) {
        if(len >= BUF_MAX) {
            len = 0;
            memset(buftmp, 0, BUF_MAX);
        }
        if(c == 0x0a) {
            p[len++] = (char)c;
            switch (test) {
                case GET_SYS_RTC_DATE:
                case GET_HW_DATE:
                    rtn = sscanf(buftmp, "%4d-%2d-%2d", &year, &month, &day);
                if (rtn != 3) {
                    if (test == GET_SYS_RTC_DATE) {
                        cterr('f',0," Get /sys/class/rtc/rtc0/date Fail!\n");
                    } else {
                        cterr('f',0," Get date +\"%Y-%m-%d\" Fail!\n");
                    }
                    pclose(fp);
                    return FAILED;
                }
                    *param1 = year;
                    *param2 = month;
                    *param3 = day;
                break;

                case GET_SYS_RTC_TIME:
                case GET_HW_TIME:
                    rtn = sscanf(buftmp, "%2d:%2d:%2d", &hour, &min, &sec);
                if (rtn != 3) {
                    if (test == GET_SYS_RTC_TIME) {
                        cterr('f',0," Get /sys/class/rtc/rtc0/time Fail!\n");
                    } else {
                        cterr('f',0," Get date +\"%H:%M:%S\" Fail!\n");
                    }
                    pclose(fp);
                    return FAILED;
                }
                    *param1 = hour;
                    *param2 = min;
                    *param3 = sec;
                break;

                default:
                    printf("%s", buftmp);
                break;
            }
            pclose(fp);
            return (PASSED);
        } else {
            p[len++] = (char)c;			
        }
    }
    pclose(fp);

    return (FAILED);
}

int rtc_test_feature (void)
{
    unsigned int hw_sec, hw_min, hw_hour, hw_day, hw_month, hw_year;
    unsigned int _hw_sec, _hw_min, _hw_hour, _hw_day, _hw_month, _hw_year;
    unsigned int sys_sec, sys_min, sys_hour, sys_day, sys_month, sys_year;
    char date[16] = {"2018-12-31"};
    char time[16] = {"23:59:59"};

    /* back up original time */
    diag_rtc_exec(GET_HW_DATE, &_hw_year, &_hw_month, &_hw_day, NULL);
    diag_rtc_exec(GET_HW_TIME, &_hw_hour, &_hw_min, &_hw_sec, NULL);


    printf("\n Set Date: %s and Time: %s\n", date, time);

    diag_rtc_exec(SET_HW_DATE, NULL, NULL, NULL, date);
    diag_rtc_exec(SET_HW_TIME, NULL, NULL, NULL, time);
    diag_rtc_exec(SET_HW_CLOCK, NULL, NULL, NULL, NULL);

    printf("\n Delay 3 seconds to test RTC.\n");
    msleep(RTC_TEST_WAIT_TIME);

    diag_rtc_exec(GET_SYS_RTC_DATE, &sys_year, &sys_month, &sys_day, NULL);
    diag_rtc_exec(GET_SYS_RTC_TIME, &sys_hour, &sys_min, &sys_sec, NULL);

    printf("\n RTC TIME: %.4d/%.2d/%.2d, %.2d:%.2d:%.2d\n", 
        sys_year, sys_month, sys_day, sys_hour, sys_min, sys_sec);

    diag_rtc_exec(GET_HW_DATE, &hw_year, &hw_month, &hw_day, NULL);
    diag_rtc_exec(GET_HW_TIME, &hw_hour, &hw_min, &hw_sec, NULL);

    printf("\n HW TIME: %.4d/%.2d/%.2d, %.2d:%.2d:%.2d\n", 
        hw_year, hw_month, hw_day, hw_hour, hw_min, hw_sec);

    if (sys_year != hw_year || sys_month != hw_month || sys_day != hw_day 
        || sys_hour != hw_hour || sys_min != hw_min 
        || (sys_sec-hw_sec > 2) || (hw_sec-sys_sec > 2)) {
        cterr('f',0,"\n Test RTC Feature Fail!\n");
        return (FAILED);
    }
    printf("\n Test RTC Feature Pass!\n");


    /* restore original time */
    _hw_sec += (RTC_TEST_WAIT_TIME/1000);
    sprintf(date, "%d-%d-%d", _hw_year, _hw_month, _hw_day);
    sprintf(time, "%d:%d:%d", _hw_hour, _hw_min, _hw_sec);
    printf("\n Restore HW Date: %s and Time: %s\n", date, time);
    diag_rtc_exec(SET_HW_DATE, NULL, NULL, NULL, date);
    diag_rtc_exec(SET_HW_TIME, NULL, NULL, NULL, time);
    diag_rtc_exec(SET_HW_CLOCK, NULL, NULL, NULL, NULL);

    return (PASSED);
} /* end of rtc_test_feature */

/**********************************************************************
 *
 * Function: rtc_test_regs
 *
 * Description: Function to RTC register test
 *
 * Inputs: None.
 *
 * Outputs: PASSED
 *
 **********************************************************************
 */
int rtc_test_regs (void)
{
    rtc_time_t tv;
    unsigned int sec, min, hour, day, month, year;
    unsigned int _sec, _min, _hour, _day, _month, _year;
    int ret = PASSED;

    rtc_init();
    /* Abort update cycle */
    write_byte(RTC_UPDATE_CYCLE_BYTE_B, 
               RTC_UPDATE_CYCLE_ABORT | RTC_24_HOUR_MODE);

    /* read date */
    diag_rtc_exec(GET_SYS_RTC_DATE, &year, &month, &day, NULL);
    /* back up original date */
    diag_rtc_exec(GET_SYS_RTC_DATE, &_year, &_month, &_day, NULL);

    /* day */
    printf("\n Current day: %02d\n", day);
    if (day <= 15) {
        tv.date = 16;
    } else if (day > 15) {
        tv.date = 14;
    }
    printf(" Set day: %.2d\n", tv.date);
    BIN_TO_BCD(tv.date);
    write_byte(RTC_DAY_OF_MONTH_BYTE_7, tv.date);
    tv.date = read_byte(RTC_DAY_OF_MONTH_BYTE_7);
    BCD_TO_BIN(tv.date);
    printf(" New day: %.2d\n", tv.date);
    
    /* read again */
    diag_rtc_exec(GET_SYS_RTC_DATE, &year, &month, &day, NULL);
    if (day == tv.date) {
        printf(" Day register test PASS!\n");
    } else {
        cterr('f',0," Day register test Fail!\n");
        ret = FAILED;
    }
    
    /* month */
    printf("\n Current month: %02d\n", month);
    if (month < 7) {
        tv.month = month + 1;
    } else {
        tv.month = month - 1;
    }
    printf(" Set month: %.2d\n", tv.month);
    BIN_TO_BCD(tv.month);
    write_byte(RTC_MONTH_BYTE_8, tv.month);
    tv.month = read_byte(RTC_MONTH_BYTE_8);
    BCD_TO_BIN(tv.month);
    printf(" New month: %.2d\n", tv.month);
    
    /* read again */
    diag_rtc_exec(GET_SYS_RTC_DATE, &year, &month, &day, NULL);
    if (month == tv.month) {
        printf(" Month register test PASS!\n");
    } else {
        cterr('f',0," Month register test Fail!\n");
        ret = FAILED;
    }
    
    /* year */
    year -= 2000;
    printf("\n Current year: 20%02d\n", year);
    if (year < 26) {
        tv.year = year + 1;
    } else {
        tv.year = year - 1;
    }
    printf(" Set year: 20%.2d\n", tv.year);
    BIN_TO_BCD(tv.year);
    write_byte(RTC_YEAR_BYTE_9, tv.year);
    tv.year = read_byte(RTC_YEAR_BYTE_9);
    BCD_TO_BIN(tv.year);
    printf(" New year: 20%.2d\n", tv.year);
    
    /* read again */
    diag_rtc_exec(GET_SYS_RTC_DATE, &year, &month, &day, NULL);
    if ((year - 2000) == tv.year) {
        printf(" Year register test PASS!\n");
    } else {
        cterr('f',0," Year register test Fail!\n");
        ret = FAILED;
    }

    /* Time */
    diag_rtc_exec(GET_SYS_RTC_TIME, &hour, &min, &sec, NULL);
    /* back up original time */
    diag_rtc_exec(GET_SYS_RTC_TIME, &_hour, &_min, &_sec, NULL);

    /* hour */
    printf("\n Current hour: %02d\n", hour);
    if (hour < 13) {
        tv.hour = hour + 1;
    } else {
        tv.hour = hour - 1;
    }
    printf(" Set hour: %.2d\n", tv.hour);
    BIN_TO_BCD(tv.hour);
    write_byte(RTC_HOUR_BYTE_4, tv.hour);
    tv.hour = read_byte(RTC_HOUR_BYTE_4);
    BCD_TO_BIN(tv.hour);
    printf(" New hour: %.2d\n", tv.hour);
    
    /* read again */
    diag_rtc_exec(GET_SYS_RTC_TIME, &hour, &min, &sec, NULL);
    if (hour == tv.hour) {
        printf(" Hour register test PASS!\n");
    } else {
        cterr('f',0," Hour register test Fail!\n");
        ret = FAILED;
    }
    
    /* minute */
    printf("\n Current minute: %02d\n", min);
    if (min < 30) {
        tv.minute = min + 1;
    } else {
        tv.minute = min - 1;
    }
    printf(" Set minute: %.2d\n", tv.minute);
    BIN_TO_BCD(tv.minute);
    write_byte(RTC_MINUTE_BYTE_2, tv.minute);
    tv.minute = read_byte(RTC_MINUTE_BYTE_2);
    BCD_TO_BIN(tv.minute);
    printf(" New minute: %.2d\n", tv.minute);
    
    /* read again */
    diag_rtc_exec(GET_SYS_RTC_TIME, &hour, &min, &sec, NULL);
    if (min == tv.minute) {
        printf(" Minute register test PASS!\n");
    } else {
        cterr('f',0," Minute register test Fail!\n");
        ret = FAILED;
    }

    /* second */
    printf("\n Current second: %02d\n", sec);
    if (sec < 30) {
        tv.second = sec + 1;
    } else {
        tv.second = sec - 1;
    }
    printf(" Set second: %.2d\n", tv.second);
    BIN_TO_BCD(tv.second);
    write_byte(RTC_SECOND_BYTE_0, tv.second);
    tv.second = read_byte(RTC_SECOND_BYTE_0);
    BCD_TO_BIN(tv.second);
    printf(" New second: %.2d\n", tv.second);
    
    /* read again */
    diag_rtc_exec(GET_SYS_RTC_TIME, &hour, &min, &sec, NULL);
    if (sec == tv.second) {
        printf(" Second register test PASS!\n");
    } else {
        cterr('f',0," Second register test Fail!\n");	
        ret = FAILED;
    }


    /* restore original time*/
    tv.date = _day;
    tv.month = _month;
    tv.year = _year;
    tv.year -= 2000;
    tv.hour = _hour;
    tv.minute = _min;
    tv.second = _sec;
    printf("\n Restore RTC Date: 20%02d-%02d-%02d and Time: %02d:%02d:%02d\n",
            tv.year, tv.month, tv.date, tv.hour, tv.minute, tv.second); 
    BIN_TO_BCD(tv.date);
    BIN_TO_BCD(tv.month);
    BIN_TO_BCD(tv.year);
    BIN_TO_BCD(tv.hour);
    BIN_TO_BCD(tv.minute);
    BIN_TO_BCD(tv.second);
    write_byte(RTC_DAY_OF_MONTH_BYTE_7, tv.date);
    write_byte(RTC_MONTH_BYTE_8, tv.month);
    write_byte(RTC_YEAR_BYTE_9, tv.year);
    write_byte(RTC_HOUR_BYTE_4, tv.hour);
    write_byte(RTC_MINUTE_BYTE_2, tv.minute);
    write_byte(RTC_SECOND_BYTE_0, tv.second);


    /* Update cycle normally */
    write_byte(RTC_UPDATE_CYCLE_BYTE_B, RTC_UPDATE_CYCLE_NORMALLY);
    write_byte(RTC_UPDATE_CYCLE_BYTE_B, RTC_24_HOUR_MODE);
    rtc_finish();

    return ret;
}

/* RTC Menu */

/*
 * RTC main menu
 */
static submenu_xtable_t rtc_main_menu_table[] = {
    { "Register Test", (PFT)rtc_test_regs, 0, RTC_MM_3, 
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    { "Feature Test", (PFT)rtc_test_feature, 0, RTC_MM_3,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define RTC_MAIN_MENU_TABLE_SIZE (sizeof(rtc_main_menu_table) / \
                                    sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t rtc_main_menu_primary_items[RTC_MAIN_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];
static mitem_t rtc_main_menu_secondary_items[RTC_MAIN_MENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];


static struct menuinfo rtcmaindiag2 = {
    "RTC Test Main Menu",      /* title */
    0,                            /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* generic prompt */
    0,                            /* size -- bumped by add_menu_item() */
    rtc_main_menu_primary_items,
};

static struct menuinfo *rtcmaindiagp2 = &rtcmaindiag2;

/**********************************************************************
 *
 * Function: build_rtc_menu
 *
 * Description: Build RTC chip tests/utils menu.
 *
 * Inputs:      show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: None.
 *
 **********************************************************************
 */
int build_rtc_menu (boolean rtc_items_executed)
{
    char *tname = "RTC";

    testname(tname);

    build_primary_submenu(rtc_main_menu_table, RTC_MAIN_MENU_TABLE_SIZE,
                            "RTC Tests Main Menu", &rtcmaindiagp2);
    build_secondary_submenu(rtc_main_menu_table, RTC_MAIN_MENU_TABLE_SIZE,
                            rtc_main_menu_secondary_items);

    if (rtc_items_executed) {
        do_all_menu_items(rtcmaindiagp2);
    } else {
        menu(rtcmaindiagp2, rtc_main_menu_secondary_items, '\0' );
    }

    return (PASSED);
}

