/* $Id: dev_tmp421.h,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_tmp421/dev_tmp421.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_tmp421.h
 *
 * Description:	Digital Temperature Sensor (TMP421) Defines.
 *		This file is based on TMP421 Datasheet.
 *
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_TMP421_H__
#define __DEV_TMP421_H__

#include "types.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define TMP421_ERR_BUF_SIZE    80

/* Registers type defines */
typedef uint8_t tmp421_p;		/* TMP421 One Byte Size Register */

/*
 * dev_error_report message codes
 */
typedef enum {
    TMP421_DEV_STATE = 0,
    TMP421_ATTACH,
    TMP421_DETACH,
    TMP421_INIT,
    TMP421_SHOW,
    TMP421_DESTROY,
    TMP421_ALTER,
    TMP421_ALERT,
    TMP421_DISPLAY,
    TMP421_SHOW_TEMP,
    TMP421_REG_TRST,
    TMP421_CHECK_ID,
    TMP421_READ,
    TMP421_WRITE,
} tmp421_report_code_t;

/*
 * device callin function - service provided and defined by the device
 */
typedef struct ts_callin_fvt_t_ {
    int (*register_test)(dev_object_t *dev);            /* Register Test */
    int (*peek_n_poke)(dev_object_t *dev, print_fn_t);	/* Peek-n-poke */
    int (*show_temp)(dev_object_t *dev);	/* Display temperature */
    int (*alter_register)(dev_object_t *dev);    /* Alter register */
    int (*dump_register)(dev_object_t *dev);    /* Dump register */
    int (*check_chip_id)(dev_object_t *dev);    /* Check chip ID */
} ts_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct ts_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*open)(n2g_i2c_if_t *);
    uint32_t (*close)(n2g_i2c_if_t *);
    uint32_t (*rd)(uint32, uint *);
    uint32_t (*wr)(uint32, uint *);
} ts_callout_fvt_t;

/*
 * Define the TMP421 device object structure
 */
typedef struct ts_object_t {
    dev_object_t	    base;
    ts_callin_fvt_t	    *callin_fvt;
    ts_callout_fvt_t	*callout_fvt;
    n2g_i2c_if_t	    *i2c_p;		/* I2C API interace pointer */
    reg_info_t		    *init_p;	/* Initialization table pointer */
} dev_tmp421_object_t;

/* Registers defines - */
/* Pointer Register - */
/* Local Temperature high Byte (Read only) */
#define TMP421_PTR_LOC_TEMP_HIGH    0x00
/* Remote Temperature 1 high byte (Read only) */
#define TMP421_PTR_RM_TEMP_1_HIGH	0x01
/* Status Register (Read only) */
#define TMP421_PTR_STATUS	        0x08
/* Configuration Register 1 (Read/Write) */
#define TMP421_PTR_CONF_1		    0x09
/* Configuration Register 2 (Read/Write) */
#define TMP421_PTR_CONF_2           0x0A
/* Conversion Rate Register (Read/Write) */
#define TMP421_PTR_CV_RATE_1        0x0B
/* Local Temperature low Byte (Read only) */
#define TMP421_PTR_LOC_TEMP_LOW     0x10
/* Remote Temperature 1 low byte (Read only) */
#define TMP421_PTR_RM_TEMP_1_LOW    0x11
/* N Correction 1 Register (Read/Write) */
#define TMP421_PTR_N_CORRECTION_1   0x21
/* Manufacturer ID (Read only) */
#define TMP421_PTR_MANFAC_ID        0xFE
/* TMP421 ID (Read only) */
#define TMP421_PTR_DEVICE_ID        0xFF

/* Registers size */
#define TMP421_PTR_TWO_BYTES		2	/* Two Bytes Size (Read only) */
#define TMP421_PTR_ONE_BYTE 		1	/* One Byte Size (Read/Write) */

/* Temperature register shift bits */
#define SIGN_BITS_SHIFT             7
#define FLOATING_BITS_SHIFT         4

/* TMP421 Device ID */
#define TMP421_DEVICE_ID            0x21

/* TMP421 multiple 10000 */
#define TMP_MULTIPLE_10000          10000

/* Functions prototype */
extern void tmp421_dev_create (dev_object_t *, dev_error_report_t);

#endif /* __DEV_TMP421_H__ */

/*------------------------------------------------------------------
 * $Log: dev_tmp421.h,v $
 * Revision 1.2  2013/10/08 08:48:25  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:48  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:58:02  tirawan
 * First Woodlawn linux integration
 *
 * Revision 1.1  2013/03/13 06:42:14  kuangik
 * Add for the first time
 *
 * Revision 1.4  2012/10/24 10:52:38  leslie
 * Fix and clean up code.
 *
 * Revision 1.3  2012/08/03 10:16:51  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/03/26 07:23:53  kody
 * Add TMP421 temperature sensor device driver.
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */
