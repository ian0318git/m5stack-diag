/* $Id: dev_1337.h,v 1.2 2012/03/28 00:38:06 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_1337_maxim/dev_1337.h,v $
 ***********************************************************************
 * File Name:	dev_1337.h
 *
 * Description:	Header file for DS1337 common object.
 *		This file is ported from Steelers with some modifications.
 *
 * Copyright (c)2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */

#ifndef __DEV_1337_H__
#define __DEV_1337_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE    80

typedef uint8_t rtc_t;		/* RTC register type */
typedef uint8_t rtc_o;		/* RTC offset */

#define BITS_0_TO_2(x)		(((uchar)x) & (0x07))
#define BITS_0_TO_3(x)		(((uchar)x) & (0x0F))
#define BITS_4_TO_5(x)		((((uchar)x) >> 4) & (0x03))
#define BITS_4_TO_6(x)		((((uchar)x) >> 4) & (0x07))
#define BITS_4_TO_7(x)		((((uchar)x) >> 4) & (0x0F))
#define GET_BIT_4(x)		((((uchar)x) >> 4) & (0x01))

#define HALF_BYTE		4 

#define DS1337_REG_WIDTH	8
#define DS1337_CENTURY_MASK	0x80

/*
 * The order of this struct is the same as the registers in RTC.
 * A burst read can be issued to read these registers without the Stop.
 * Do not change the order of this struct.
 */
typedef struct ds1337_rtc_data_ {
    rtc_t second;
    rtc_t minute;
    rtc_t hour;
    rtc_t day_of_week;
    rtc_t date;
    rtc_t month;
    rtc_t year;
} ds1337_rtc_data_t;

typedef struct ds1337_time_t_ {
    int second;
    int minute;
    int hour;
    int date;
    int month;
    int day_of_week;
    int year;
} ds1337_time_t;

/*
 * dev_error_report message codes
 */
typedef enum {
    DS1337_DEV_STATE = 0,
    DS1337_ATTACH,
    DS1337_DETACH,
    DS1337_INIT,
    DS1337_SHOW,
    DS1337_DESTROY,
    DS1337_REG_TEST,
    DS1337_RTC_READ,
    DS1337_RTC_WRITE,
    DS1337_RTC_WRITE_SECOND,
    DS1337_RTC_PROGRAM,
    DS1337_TIME_STOP,
    DS1337_TIME_JUMP,
    DS1337_ABORT,
    DS1337_ALTER,
    DS1337_I2C_READ,
    DS1337_I2C_WRITE,
} ds1337_report_code_t;

/*
 * Device callin functions - Service provided and defined by the device
 */
typedef struct dev_ds1337_callin_fvt_t_ {
    int (*peek_n_poke)(dev_object_t *dev, print_fn_t);	/* Peek-n-poke */
    uint32 (*display_rtc)(dev_object_t *);
    uint32 (*set_rtc)(dev_object_t *);
    uint32 (*show_rtc_reg)(dev_object_t *);
    uint32 (*mux_clk_to_dout)(dev_object_t *, uchar);
    uint32 (*register_test)(dev_object_t *);
    uint32 (*time_validity_test)(dev_object_t *);
} dev_ds1337_callin_fvt_t;

/*
 * Device callout functions - Service needed by the device and
 *                            defined by platform
 */
typedef struct dev_ds1337_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} dev_ds1337_callout_fvt_t;

/*
 * Define the Dallas DS1337 RTL device object structure.
 */
typedef struct dev_ds1337_object_t_ {
    dev_object_t		base;
    dev_ds1337_callin_fvt_t 	*callin_fvt;
    dev_ds1337_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t		*i2c_p;		/* I2C API interace pointer */
    reg_info_t			*reg_p;		/* Register table pointer */
    ds1337_time_t		*dt;		/* Points to date and time */
} dev_ds1337_object_t;

/*
 * RTC register map
 */
#define DS1337_SECONDS_REG	0x00	/* Seconds reg		*/
#define DS1337_MINUTES_REG	0x01	/* Minutes reg		*/
#define DS1337_HOURS_REG	0x02	/* Hours reg		*/
#define DS1337_DAY_REG		0x03	/* Day reg		*/
#define DS1337_DATE_REG		0x04	/* Date reg		*/
#define DS1337_MONTH_REG	0x05	/* Century - Month reg	*/
#define DS1337_YEAR_REG		0x06	/* Year reg		*/
#define DS1337_A1_SEC_REG	0x07	/* Alarm 1 Seconds reg	*/
#define DS1337_A1_MIN_REG	0x08	/* Alarm 1 Minutes reg	*/
#define DS1337_A1_HR_REG	0x09	/* Alarm 1 Hour reg	*/
#define DS1337_A1_DAY_REG	0x0A	/* Alarm 1 Day/Date reg */
#define DS1337_A2_MIN_REG	0x0B	/* Alarm 2 Minutes reg	*/
#define DS1337_A2_HR_REG	0x0C	/* Alarm 2 Hour reg	*/
#define DS1337_A2_DAY_REG	0x0D	/* Alarm 2 Day/Date reg */
#define DS1337_CONTROL_REG	0x0E	/* Control register	*/
#define DS1337_STATUS_REG	0x0F	/* Status register	*/

/* Seconds reg */
#define DS1337_SEC_MIN		0x00	/* Seconds minimum	*/
#define DS1337_SEC_MAX		0x59	/* Seconds maximum	*/

/* Minutes reg */
#define DS1337_MIN_MIN		0x00	/* Minutes minimum	*/
#define DS1337_MIN_MAX		0x59	/* Minutes maximum	*/

/* Hours reg */
#define DS1337_HR_MASK          0x1F    /* Hour mask		*/
#define DS1337_HR_24MIN		0x00	/* Hours minimum 24 hr	*/
#define DS1337_HR_12MIN		0x01	/* Hours minimum 12hr	*/
#define DS1337_HR_24MAX		0x23	/* Hours maximum 24hr	*/
#define DS1337_HR_12MAX		0x63	/* Hours maximum 12hr	*/
#define DS1337_HR_12            0x40    /* 12/24- format	*/
#define DS1337_HR_PM            0x20    /* PM for 12 format	*/

/* Day reg */
#define DS1337_DAY_MIN		0x1	/* Day minimum		*/
#define DS1337_DAY_MAX		0x7	/* Day maximum		*/

/* Date reg */
#define DS1337_DATE_MIN		0x01	/* Date minimum		*/
#define DS1337_DATE_MAX		0x31	/* Date maximum		*/

/* Century - Month reg */
#define DS1337_CENTURY_MASK	0x80	/* Century		*/
#define DS1337_MN_MIN		0x01	/* Month minimum	*/
#define DS1337_MN_MAX		0x12	/* Month maximum	*/

/* Year reg */
#define DS1337_YR_MIN		0x00	/* Year minimum		*/
#define DS1337_YR_MAX		0x99	/* Year maximum		*/

/* Alarm 1 Seconds reg */
#define DS1337_A1_A1M1		0x80	/* A1M1			*/

/* Alarm 1 Minutes reg */
#define DS1337_A1_A1M2		0x80	/* A1M2			*/

/* Alarm 1 Hour reg. Same format as Century-Month reg */
#define DS1337_A1_A1M3		0x80	/* A1M3			*/

/* Alarm 1 Day/Date reg */
#define DS1337_A1_A1M4		0x80	/* A1M4			*/
#define DS1337_DAY		0x40	/* Day/Date- format	*/

/* Alarm 2 Minutes reg */
#define DS1337_A2_A2M2		0x80	/* A2M2			*/

/* Alarm 2 Hour reg. Same format as Century-Month reg */
#define DS1337_A2_A2M3		0x80	/* A2M3			*/

/* Alarm 2 Day/Date reg */
#define DS1337_A2_A2M4		0x80	/* A2M4			*/

/* Control register */
#define DS1337_CTL_EOSC_DIS	0x80	/* Enable Oscillator - active-low */
#define DS1337_CTL_RS_MASK	0x18	/* Rate Select Mask	*/
#define DS1337_CTL_RS_1HZ	0x00	/* 1 Hz Square-wave	*/
#define DS1337_CTL_RS_4KHZ	0x08	/* 4.096 kHz		*/
#define DS1337_CTL_RS_8KHZ	0x10	/* 8.192 kHz		*/
#define DS1337_CTL_RS_32KHZ	0x18	/* 32.768 kHz		*/
#define DS1337_CTL_INTCN_EN	0x04	/* Interrupt Control	*/
#define DS1337_CTL_A2IE_EN	0x02	/* Alarm 2 Interrupt Enable */
#define DS1337_CTL_A1IE_EN	0x01	/* Alarm 1 Interrupt Enable */

/* Status register */
#define DS1337_STAT_OSF		0x80	/* Oscillator Stop Flag	*/
#define DS1337_STAT_A2F		0x02	/* Alarm 2 Flag		*/
#define DS1337_STAT_A1F		0x01	/* Alarm 1 Flag		*/

#define DS1337_SINGLE_SEC_MASK	0x0F
#define DS1337_10_SEC_MASK	0x70
#define DS1337_SEC_MASK		(DS1337_SINGLE_SEC_MASK | DS1337_10_SEC_MASK)
#define DS1337_SINGLE_MIN_MASK	DS1337_SINGLE_SEC_MASK
#define DS1337_10_MIN_MASK	DS1337_10_SEC_MASK
#define DS1337_12HR_24HR_MASK	0x40
#define DS1337_SINGLE_HOUR_MASK	DS1337_SINGLE_SEC_MASK
#define DS1337_AM_PM_MASK	0x20
#define DS1337_AM_PM_10_HOUR_MASK 0x10
#define DS1337_WEEK_DAY_MASK	0x07
#define DS1337_SINGLE_DATE_MASK	DS1337_SINGLE_SEC_MASK
#define DS1337_10_DATE_MASK	DS1337_10_SEC_MASK
#define DS1337_SINGLE_MONTH_MASK DS1337_SINGLE_SEC_MASK
#define DS1337_SINGLE_YEAR_MASK	DS1337_SINGLE_SEC_MASK

#define DS1337_DEFAULT_YEAR	0x70
#define CURRENT_CENTURY		2000
#define TEST_TIME_DELAY		5	/* Seconds to wait for validity test */
#define SECONDS_PER_MIN		60
#define V_TEST_START_DELTA	1	/* Time validity start time delta */
#define RTC_TEST_TOLERANCE	3	/* Validity test tolerance. Verified
					 * with HW team (Eric) */

/* Prototypes */
extern void dev_1337_create(dev_object_t *, dev_error_report_t);
extern int  rtc_init(int);
extern int  get_rtc(ds1337_time_t *);
extern int  set_rtc(ds1337_time_t *);

#endif /* __DEV_1337_H__ */

/***********************************************************************
$Log: dev_1337.h,v $
Revision 1.2  2012/03/28 00:38:06  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
