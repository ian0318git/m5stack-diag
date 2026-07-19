/* $Id: dev_tmpx75.h,v 1.2 2018/01/20 04:21:32 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_tmpx75/dev_tmpx75.h,v $
 *------------------------------------------------------------------
 * Filename:	dev_tmpx75.h
 *
 * Description:	16-bit GPIO Expander TMPX75 Header
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_TMPX75_H__
#define __DEV_TMPX75_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */
#define TMPX75_ERR_BUF_SIZE            (80)

typedef enum {
    TMPX75_DEV_STATE = 0,
    TMPX75_ATTACH,
    TMPX75_DETACH,
    TMPX75_INIT,
    TMPX75_SHOW,
    TMPX75_DESTROY,
    TMPX75_ALTER,
    TMPX75_ALERT,
    TMPX75_DISPLAY,
    TMPX75_REG_TEST,
    TMPX75_SHOW_TEMP,
    TMPX75_READ,
    TMPX75_WRITE,
} tmpx75_report_code_t;

#define TS_PTR_TEMP                     (0x00) /* Temperature   (Read only)  */
#define TS_PTR_CFG                      (0x01) /* Configuration (Read/Write) */
#define TS_PTR_THYST                    (0x02) /* Thyst         (Read/Write) */
#define TS_PTR_TOS                      (0x03) /* Tos           (Read/Write) */
#define TS_PTR_OS                       (0x04) /* One Shot Mode (Read/Write) */

#define TS_TEMP_MAX                     (0x7D00)      /* 0x7D0: 125 Celcius */
#define TS_TEMP_RESOLUTION              (0.0625)      /* One LSB: 0.0625 Celcius */

typedef struct tmpx75_callin_fvt_t_ {
    int (*register_test)(dev_object_t *dev);     /* Register Test */
    int (*alter_register)(dev_object_t *dev);    /* Alter register */
    int (*dump_register)(dev_object_t *dev);     /* Dump register */
    int (*show_temp)(dev_object_t *dev);         /* Show Temperature */
} tmpx75_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *			     platform
 */
typedef struct tmpx75_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*rd)(uint32, ushort *);
    uint32_t (*wr)(uint32, ushort *);
} tmpx75_callout_fvt_t;

/*
 * Define the TMPX75 device object structure
 */
typedef struct tmpx75_object_t {
    dev_object_t	    base;
    tmpx75_callin_fvt_t	    *callin_fvt;
    tmpx75_callout_fvt_t	*callout_fvt;
    reg_info_t		    *init_p;	/* Initialization table pointer */
    int i2c_addr;
} dev_tmpx75_object_t;

extern  void tmpx75_dev_create(dev_object_t *, dev_error_report_t);

#endif
