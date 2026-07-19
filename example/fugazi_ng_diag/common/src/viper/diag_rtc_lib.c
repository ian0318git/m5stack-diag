 /* $Id: diag_rtc_lib.c,v 1.3 2018/11/09 08:48:12 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_rtc_lib.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_rtc_lib.c
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
#include "diag_rtc_lib.h"

/*
 * static functions prototypes 
 */
static unsigned short clock_ctl_addr = RTC_FIXED_IO_CTL_REGISTER;
static unsigned short clock_data_addr = RTC_FIXED_IO_DATA_REGISTER;

/*
 * Global variables
 */
char rtc_inited = 0;

/*******************************************************************************
 *
 * Function   : rtc_init
 * Description: Function to init rtc
 * Inputs     : none
 * Outputs    : none
 *
 *******************************************************************************
 */
void rtc_init(void)
{
	if ( setuid(0) == -1 ) {
		perror("setuid 0 (needed by ioperm) failed");
		exit(1);
	}
	ioperm(clock_ctl_addr, 2, 1);
	rtc_inited = 1;
}

/*******************************************************************************
 *
 * Function   : rtc_finish
 * Description: Function to finish write process 
 * Inputs     : none
 * Outputs    : none
 *
 *******************************************************************************
 */
void rtc_finish(void)
{
	ioperm(clock_ctl_addr, 2, 0);
	rtc_inited = 0;
}

/*******************************************************************************
 *
 * Function   : read_byte
 * Description: Function to read byte
 * Inputs     : offset - offset
 * Outputs    : a - read value
 *
 *******************************************************************************
 */
unsigned char read_byte(char offset)
{
	unsigned char a;

    if (rtc_inited == 0) {
		rtc_init();
    }

	outb(offset, clock_ctl_addr);
	a = inb(clock_data_addr);

	return(a);
}

/*******************************************************************************
 *
 * Function   : write_byte
 * Description: Function to write byte
 * Inputs     : offset - offset
 *              data   - data want to write
 * Outputs    : none
 *
 *******************************************************************************
 */
void write_byte(char offset, char data)
{
    if (rtc_inited == 0) {
		rtc_init();
    }

	outb(offset, clock_ctl_addr);
	outb(data, clock_data_addr);
}

/*******************************************************************************
 *
 * Function   : rtc_word
 * Description: Function to read word
 * Inputs     : offs - offset
 * Outputs    : none
 *
 *******************************************************************************
 */
unsigned int rtc_word(char offs)
{
	unsigned char a,b;
	a = read_byte(offs);
	b = read_byte(offs+1);
	return((unsigned int)a+256*(unsigned int)b);
}

/*******************************************************************************
 *
 * Function   : rtc_set_time
 * Description: Function to set rtc time
 * Inputs     :  *tv - pointer of record time
 * Outputs    : none
 *
 *******************************************************************************
 */
void rtc_set_time(rtc_time_t *tv)
{
	write_byte(RTC_SECOND_BYTE_0, tv->second);
	write_byte(RTC_MINUTE_BYTE_2, tv->minute);
	write_byte(RTC_HOUR_BYTE_4, tv->hour);
    write_byte(RTC_DAY_OF_MONTH_BYTE_7, tv->date);
	write_byte(RTC_MONTH_BYTE_8, tv->month);
	write_byte(RTC_YEAR_BYTE_9, tv->year);
	write_byte(RTC_CENTURY, CENTURY_DEFAULT);	/* CSCvn21696 : Based on IOS requested, set offset 14 to 0 */
}

/*******************************************************************************
 *
 * Function   : rtc_read_time
 * Description: Function to read rtc time
 * Inputs     : *tv - pointer of record time
 * Outputs    : none
 *
 *******************************************************************************
 */
void rtc_read_time(rtc_time_t *tv)
{
	tv->second 	= read_byte(RTC_SECOND_BYTE_0);
	tv->minute 	= read_byte(RTC_MINUTE_BYTE_2);
	tv->hour 	= read_byte(RTC_HOUR_BYTE_4);
    tv->date	= read_byte(RTC_DAY_OF_MONTH_BYTE_7);
	tv->month 	= read_byte(RTC_MONTH_BYTE_8);
	tv->year 	= read_byte(RTC_YEAR_BYTE_9);
}

/******** History ********
$Log: diag_rtc_lib.c,v $
Revision 1.3  2018/11/09 08:48:12  lucywang
Set RTC standard RAM byte offset 14 to zero for IOS

Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.3  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.2  2018/06/06 03:16:07  lucywang
Fixed CSCvj80723-RTC register test failed

Revision 1.1.2.1  2018/03/29 01:11:19  lucywang
Added RTC test and utility


$Endlog$
*/
