 /* $Id: diag_rtc_lib.h,v 1.2 2019/12/11 10:10:31 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_rtc_lib.h,v $
 *------------------------------------------------------------------
 * Filename: diag_rtc_lib.h
 *
 * Description: Diag rtc library header file.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_RTC_LIB_H__
#define __DIAG_RTC_LIB_H__

typedef unsigned int  BYTE;
typedef struct rtc_time_t_ {
    BYTE second;
    BYTE minute;
    BYTE hour;
    BYTE date;
    BYTE month;
    BYTE day_of_week;
    BYTE year;
} rtc_time_t;

#define RTC_FIXED_IO_CTL_REGISTER	0x70
#define RTC_FIXED_IO_DATA_REGISTER	0x71

#define RTC_REG_D			0xd
#define RTC_REG_C			0xc
#define RTC_UPDATE_CYCLE_BYTE_B		0xb
#define RTC_REG_A			0xa
#define RTC_YEAR_BYTE_9			0x9
#define RTC_MONTH_BYTE_8		0x8
#define RTC_DAY_OF_MONTH_BYTE_7		0x7
#define RTC_DAY_OF_WEEK_BYTE_6		0x6
#define RTC_HOUR_ALARM_BYTE_5		0x5
#define RTC_HOUR_BYTE_4			0x4
#define RTC_MINUTE_ALARM_BYTE_3		0x3
#define RTC_MINUTE_BYTE_2		0x2
#define RTC_SECOND_ALARM_BYTE_1		0x1
#define RTC_SECOND_BYTE_0		0x0

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
#define BIN_TO_BCD(val) ((val)=(((val)/10)<<4) + (val)%10)

void rtc_init(void);
void rtc_finish(void);
unsigned char read_byte(char offset);
void write_byte(char offset, char data);
void rtc_set_time(rtc_time_t *tv);
void rtc_read_time(rtc_time_t *tv);

#endif /* __DIAG_RTC_LIB_H__ */

/******** History ********
$Log: diag_rtc_lib.h,v $
Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/
