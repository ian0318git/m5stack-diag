/* $Id: dev_ltc4215.h,v 1.3 2014/03/03 06:33:34 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_ltc4215/dev_ltc4215.h,v $
 *------------------------------------------------------------------
 * Filename: dev_ltc4215.h
 *
 * Description: header file of Xformer OIR chip.
 *
 * Copyright (c) 2006-2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DEV_LTC4215_H__
#define __DEV_LTC4215_H__

#include "types.h"
#include "i2c_api.h"
#include "dev_object.h"

/* LTC 4215 Registers map */
#define LTC4215_CONTROL_REG	0x00
#define LTC4215_ALERT_REG	0x01
#define LTC4215_STATUS_REG	0x02
#define LTC4215_FAULT_REG	0x03
#define LTC4215_SENSE_REG	0x04
#define LTC4215_SOURCE_REG	0x05

/* GPIO1 General Purpose Output */
#define LTC4215_GPIO1_GENRAL_PURPOSE_OUTPUT	0x40
/* GPIO1 General Purpose Input */
#define LTC4215_GPIO1_GENRAL_PURPOSE_INPUT	0xC0

/* GPIO1 Input */
#define LTC4215_GPIO1_INPUT_MASK	0x40
/* GPIO2 Input */
#define LTC4215_GPIO2_INPUT_MASK	0x04

/* GPIO1 Output */
#define LTC4215_GPIO1_OUTPUT_MASK	0x40
/* GPIO2 Output */
#define LTC4215_GPIO2_OUTPUT_MASK	0x40
/* GPIO3 Output */
#define LTC4215_GPIO3_OUTPUT            0x80

#define LTC4215_FET_ON_CONTROL		0x08
#define LTC4215_FET_ON_STATUS       0x80
#define LTC4215_FET_SHORT_PRESENT   0x20
#define LTC4215_POWER_BAD_STATUS    0x08

/* Sensed Source Voltage */
#define MIN_SOURCE_VOLTAGE	0x94
#define MAX_SOURCE_VOLTAGE	0xE7
#define SINGLE_SM_VOL		605

/* Sensed Source Current */
#define SINGLE_SM_CURRENT	503
#define DWIDE_SM_CURRENT	755
#define ISM_CURRENT             25167   

/*
 * dev_error_report message codes
 */
typedef enum {
    DEV_LTC4215_DEV_STATE = 0,
    DEV_LTC4215_ATTACH,
    DEV_LTC4215_SHOW,
}nm_adptr_cpld_report_code_t;

/* dev_error_report msg buffer size */
#define ERR_MSG_SZ              128

/*
 * device callin function vector table
 */
typedef struct ltc_4215_callin_fvt_t_ {
    int (*reg_rd)(dev_object_t *dev, uint8_t *d_buf, uint32_t offset);
    int (*reg_wr)(dev_object_t *dev, uint8_t *d_buf, uint32_t offset);
    int (*reg_test)(dev_object_t *dev);
    int (*pg_led)(dev_object_t *dev, boolean en);
    int (*gpio2_led)(dev_object_t *dev, boolean en);
    int (*pg_led_test)(dev_object_t *dev);
    int (*gpio2_led_test)(dev_object_t *dev);
} dev_ltc4215_callin_fvt_t;

/*
 * device callout function vector table
 */
typedef struct dev_ltc4215_callout_fvt_t_ {
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*init)(n2g_i2c_if_t *);
    uint32_t (*rd)(n2g_i2c_if_t *);
    uint32_t (*wr)(n2g_i2c_if_t *);
    uint32_t (*dma_wr)(n2g_i2c_if_t *);
} dev_ltc4215_callout_fvt_t;

/*
 * LTC4215 device object structure.
 */
typedef struct dev_ltc4215_object_t_ {
    dev_object_t	base;
    dev_ltc4215_callin_fvt_t	*callin_fvt;
    dev_ltc4215_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	*i2c_p;	/* I2C API interface pointer */
}dev_ltc4215_object_t;

/* Functions prototype */
extern int dev_ltc4215_create (dev_object_t *dev,
			     dev_error_report_t error_report_fn);

#endif /* __DEV_LTC4215_H__ */

 /*
 *------------------------------------------------------------------
$Log: dev_ltc4215.h,v $
Revision 1.3  2014/03/03 06:33:34  palin2
-Initial check-in ShrinkRay host side Diag.
-Add LTC4215 GPIO3 Output definition.

Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
