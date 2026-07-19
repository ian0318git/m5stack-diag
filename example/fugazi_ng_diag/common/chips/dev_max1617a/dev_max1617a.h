/* $Id: dev_max1617a.h,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_max1617a/dev_max1617a.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_max1617a.h
 *
 * Description:	Remote/Local Temperature Sensor (Max1617A) Defines.
 *		This file is based on Max1617A Datasheet.
 *
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_MAX1617_H__
#define __DEV_MAX1617_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define ERR_BUF_SIZE    80

typedef uint8_t sn_c;		/* Max1617A command/register type */
typedef uint8_t sn_d;		/* Max1617A data */

/*
 * dev_error_report message codes
 */
typedef enum {
    SNSR_DEV_STATE = 0,
    SNSR_ATTACH,
    SNSR_DETACH,
    SNSR_INIT,
    SNSR_SHOW,
    SNSR_DESTROY,
    SNSR_ALTER,
    SNSR_ALERT,
    SNSR_SHOW_TEMP,
    SNSR_READ,
    SNSR_WRITE,
} max1617a_report_code_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct snsr_callin_fvt_t_ {
    int (*peek_n_poke)(dev_object_t *dev, print_fn_t);	/* Peek-n-poke */
    int (*clear_alert)(dev_object_t *dev, print_fn_t);	/* Clear alert signal */
    int (*show_temp)(dev_object_t *, print_fn_t, int);	/* Display temperature*/
} snsr_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct snsr_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
} snsr_callout_fvt_t;

/*
 * Define the Max1617A device object structure
 */
typedef struct snsr_object_t {
    dev_object_t	base;
    snsr_callin_fvt_t	*callin_fvt;
    snsr_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	*i2c_p;		/* I2C API interace pointer */
    reg_info_t		*init_p;	/* Initialization table pointer */
} dev_max1617a_object_t;

/* Read/write registers pair struct */
typedef struct max1617a_rd_wr_t_ {
    sn_c	rd;		/* Read command */
    sn_c	wr;		/* Write command */
} max1617a_rd_wr_t;

/* Conversion rate control byte text struct */
typedef struct conv_rate_t_ {
    char *string;
} conv_rate_t;

/* Max1617A is Command driven. Commands defines - */
#define MAX1617_CMD_RLTS	0x00	/* Read local temperature */
#define MAX1617_CMD_RRTE	0x01	/* Read remote temperature */
#define MAX1617_CMD_RSL		0x02	/* Read status byte */
#define MAX1617_CMD_RCL		0x03	/* Read configuration byte */
#define MAX1617_CMD_RCRA	0x04	/* Read conversion rate byte */
#define MAX1617_CMD_RLHN	0x05	/* Read local THIGH limit */
#define MAX1617_CMD_RLLI	0x06	/* Read local TLOW limit */
#define MAX1617_CMD_RRHI	0x07	/* Read remote THIGH limit */
#define MAX1617_CMD_RRLS	0x08	/* Read remote TLOW limit */
#define MAX1617_CMD_WCA		0x09	/* Write configuraiton byte */
#define MAX1617_CMD_WCRW	0x0A	/* Write conversion rate byte */
#define MAX1617_CMD_WLHO	0x0B	/* Write local THIGH limit */
#define MAX1617_CMD_WLLM	0x0C	/* Write local TLOW limit */
#define MAX1617_CMD_WRHA	0x0D	/* Write remote THIGH limit */
#define MAX1617_CMD_WRLN	0x0E	/* Write remote TLOW limit */
#define MAX1617_CMD_OSHT	0x0F	/* One-shot command */
#define MAX1617_CMD_SPOR	0xFC	/* Write software POR */
#define MAX1617_CMD_MFGID	0xFE	/* Read manufacturer ID code */
#define MAX1617_CMD_DEVID	0xFF	/* Read Device ID code */

/* Status-Byte */
#define MAX1617_RSL_BUSY	0x80	/* ADC is busy converting */
#define MAX1617_RSL_LHIGH	0x40	/* Local high temperature alarm */
#define MAX1617_RSL_LLOW	0x20	/* Local low temperature alarm */
#define MAX1617_RSL_RHIGH	0x10	/* Remote high temperature alarm */
#define MAX1617_RSL_RLOW	0x08	/* Remote low temperature alarm */
#define MAX1617_RSL_OPEN	0x04	/* Open-circuit fault */

/* Configuration-Byte */
#define MAX1617_RCL_MASK	0x80	/* Mask - disable interrupt */
#define MAX1617_RCL_STOP	0x40	/* Run/Stop - Stops and enter standby */

/* Conversion-Rate Control Byte */
#define MAX1617_CRA_0625	0x00	/* 0.0625 Hz */
#define MAX1617_CRA_125		0x01	/* 0.125  Hz */
#define MAX1617_CRA_25		0x02	/* 0.25   Hz */
#define MAX1617_CRA_HALF	0x03	/* 0.5    Hz */
#define MAX1617_CRA_1HZ		0x04	/* 1      Hz */
#define MAX1617_CRA_2HZ		0x05	/* 2      Hz */
#define MAX1617_CRA_4HZ		0x06	/* 4      Hz */
#define MAX1617_CRA_8HZ		0x07	/* 8	  Hz */

/* Manufacturing and Device IDs */
#define MAX1617_MFG_ID		0x4D	/* Maxim 1617 */
#define MAX1617_DEV_ID		0x01
#define ADM1021_MFG_ID		0x41	/* ADM1021 - Analog Device */
#define ADM1021_DEV_ID		0x30

/* Functions prototype */
extern void max1617a_dev_create (dev_object_t *dev,
				 dev_error_report_t error_report_fn);

#endif /* __DEV_MAX1617_H__ */

/*------------------------------------------------------------------
$Log: dev_max1617a.h,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
