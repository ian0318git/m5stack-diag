/* $Id: platform_rtc.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_rtc.c,v $
 *------------------------------------------------------------------
 * 
 * Filename   : 
 * Description: .
 *
 * Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/rtc.h>
#include <time.h>
#include <sys/ioctl.h>
#include "types.h"
#include "defs.h"
#include "common.h"
#include "menu.h"
#include "cli_cmd.h" /*for using cli_time sturcture */
#include "error.h"
#include "queryflags.h"

#define YEAR_OFFSET 1900
#define RTC_TEST_INTERVAL   3

int utility_get_rtc(int check);
int linux_set_rtc(int, cli_time *);
static int utility_set_rtc(int check);
int rtc_utility_main(int);


#define RTC_SUBMENU_TABLE_SZ \
	(sizeof(rtc_submenu_table) / sizeof(submenu_xtable_t))
/*
 * RTC Main Test Menu
 */
static  submenu_xtable_t        rtc_menu_table[] = {

    { "RTC get time",	(PFT)utility_get_rtc,	0,
	MF_CONTINUOUS,			(type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RTC set time",	(PFT)utility_set_rtc,	0,
	MF_CONTINUOUS,			(type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define RTC_MENU_TABLE_SZ \
	(sizeof(rtc_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static  mitem_t rtc_pri_items[RTC_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static  mitem_t rtc_sec_items[RTC_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static  struct menuinfo    rtc_menu = {
    "RTC Main Menu",
    0,			/* mtparam added by init_empty_menu */
    (PFT)show_endnote,	/* notes missing WICs in combos */
    0,			/* use generic prompt */
    0,			/* size (bumped by add_menu_item() */
    rtc_pri_items,
};
static struct menuinfo *rtc_menup = &rtc_menu;


/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */
static int rtc_no = 0; 


void set_rtc_dev(int num)
{
    rtc_no = num;
}

int rtc_ioctl(int cmd, void *arg)
{
    int fd, retval;
    char rtc[20];

    sprintf(rtc, "/dev/rtc%d", rtc_no);
    
    fd = open(rtc, O_RDONLY);

    if (fd ==  -1) {
        perror(rtc);
        exit(errno);
    }

    /* Turn on update interrupts (one per second) */
    retval = ioctl(fd, cmd, (void *)arg);
    close(fd);
    
    if (retval == -1) {
        perror("RTC ioctl error:");
    }

    return retval;
}

static int
get_rtc (struct tm *rtc)
{
   if (rtc_ioctl(RTC_RD_TIME, rtc) < 0) {
       cterr('f', 0, "utility set rtc: ioctl->RTC_GET_TIME: can't read time\n");
       return FAILED;
   }
   return PASSED;
}

int utility_get_rtc (int show)
{
   struct tm rtc;
   memset(&rtc, 0, sizeof(rtc));
   if (get_rtc(&rtc))
       return FAILED;
   
   printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\r\n",
              rtc.tm_mon+1, rtc.tm_mday, rtc.tm_year+YEAR_OFFSET,
              rtc.tm_hour, rtc.tm_min, rtc.tm_sec);

   return PASSED;
}

int 
linux_set_rtc(int type, cli_time *timeval) {

    struct tm rtc;    

    memset(&rtc, 0, sizeof(rtc));
    if (get_rtc(&rtc))
       return FAILED;
	  
    if (type == CLI_SET_DATE) {
        rtc.tm_year  = timeval->year;
        rtc.tm_mon   = timeval->month;
        rtc.tm_mday  = timeval->date;
        rtc.tm_mon--;
        rtc.tm_year -= YEAR_OFFSET;        
    } else if (type == CLI_SET_TIME) {
        rtc.tm_hour  = timeval->hour;
        rtc.tm_min   = timeval->minute;
        rtc.tm_sec   = timeval->second;
    }
    
    if (rtc_ioctl(RTC_SET_TIME, &rtc) < 0) {
        cterr('f', 0, "utility set rtc: ioctl->RTC_SET_TIME: can't set time\n");
        return FAILED;
    }
    	  
    /*show rtc again for check */ 
    utility_get_rtc(1); 
    	  
    return PASSED;
	
}

static int
utility_set_rtc (int check)
{
    struct tm rtc;

    memset(&rtc, 0, sizeof(rtc));
    get_rtc(&rtc);
    rtc.tm_year  = getdec_answer("Enter Year", rtc.tm_year+YEAR_OFFSET, 2000, 2030);
    rtc.tm_mon   = getdec_answer("Enter Month", rtc.tm_mon+1, 1, 12);
    rtc.tm_mday  = getdec_answer("Enter Date", rtc.tm_mday, 1, 31);
    rtc.tm_hour  = getdec_answer("Enter Hour (24 hour)", rtc.tm_hour, 0, 23);
    rtc.tm_min   = getdec_answer("Enter Minute", rtc.tm_min, 0, 59);
    rtc.tm_sec   = getdec_answer("Enter Second", rtc.tm_sec, 0, 59);

    rtc.tm_mon--;
    rtc.tm_year -= YEAR_OFFSET;
    
    if (rtc_ioctl(RTC_SET_TIME, &rtc) < 0) {
        cterr('f', 0, "utility set rtc: ioctl->RTC_SET_TIME: can't set time\n");
        return FAILED;
    }

    return PASSED;
}

int
rtc_utility_main (int show_menu)
{
    testname("Real Time Clock");

    build_primary_submenu(rtc_menu_table, RTC_MENU_TABLE_SZ,
			  "RTC", &rtc_menup);
    build_secondary_submenu(rtc_menu_table, RTC_MENU_TABLE_SZ,
			    rtc_sec_items);

    menu(rtc_menup, rtc_sec_items, '\0');

    return (PASSED);
}

extern void msleep (int t);
int rtc_tests (int dummy)
{
	struct tm rtc1,rtc2;
	int interval;
	char *tname = "rtc";

	testname("%s access", tname);

	memset(&rtc1, 0, sizeof(rtc1));
	memset(&rtc2, 0, sizeof(rtc2));

	if (get_rtc(&rtc1))
	{
		cterr('f',0,"get rtc1 failed.");
		return FAILED;
	}
	msleep(RTC_TEST_INTERVAL * 1000);
	if (get_rtc(&rtc2))
    {
		cterr('f',0,"get rtc2 failed.");
        return FAILED;
    }

	interval = difftime(mktime(&rtc2),mktime(&rtc1));

	if(interval != RTC_TEST_INTERVAL)
	{
		cterr('f',0,"check rtc interval failed. expect %d, got %d",RTC_TEST_INTERVAL,interval);
        return FAILED;
	}
	prpass(testpass, NULL);
	return PASSED;
}

/*-------------------- End of File ----------------------*/
/*
 *------------------------------------------------------------------
 * $Log: platform_rtc.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.2  2018/12/27 03:49:00  mikech2
 * Modify prpass usage
 *
 * Revision 1.1.2.1  2018/10/22 08:02:29  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.2  2018/10/22 01:36:11  mikech2
 * Fix RTC test interval count error issue
 *
 * Revision 1.1.2.1  2018/10/18 02:32:25  mikech2
 * Add RTC test in mb_tests.c
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
