/* $Id: dev_nxp_lm75b.h,v 1.2 2018/01/20 04:14:46 hondwang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_nxp_lm75b/dev_nxp_lm75b.h,v $
 *------------------------------------------------------------------
 * Filename   : dev_nxp_lm75b.h
 * Description: Header of NXP LM75B, a digital temperature sensor
 *              and thermal watchdog.
 *              (Type number: LM75BD/LM75BDP/LM75BGD/LM75BTP)
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_NXP_LM75B_H__
#define __DEV_NXP_LM75B_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

/* Common defines */

#define LM75B_ERR_BUF_SIZE            (80)

typedef enum {
    LM75B_DEV_STATE = 0,
    LM75B_ATTACH,
    LM75B_DETACH,
    LM75B_INIT,
    LM75B_GET_REG,
    LM75B_DESTROY,
    LM75B_ALTER,
    LM75B_ALERT,
    LM75B_DISPLAY,
    LM75B_REG_TEST,
    LM75B_SHOW_TEMP,
    LM75B_READ,
    LM75B_WRITE,
} lm75b_report_code_t;

/* NXP LM75B register name definition */
#define LM75B_TEMP    (0x00) /* Temperature reg. (Read only) */
#define LM75B_CONF    (0x01) /* Configuration reg. (R/W) */
#define LM75B_THYST   (0x02) /* Thsteresis reg. (R/W) */
#define LM75B_TOS     (0x03) /* Overtemperature shutdown threshold reg. (R/W) */

/* Temperature Reg.(0x0) */
#define LM75B_TEMP_SIGN_BIT     (0x8000)  /* D10(bit7 of MSByte) */
#define LM75B_TEMP_RESOLUTION   (0.125)   /* One LSB: 0.125 Celcius */

typedef struct lm75b_callin_fvt_t_ {
    int (*register_test)(dev_object_t *dev);     /* Register Test */
    int (*show_register)(dev_object_t *dev);     /* Get register */
    int (*alter_register)(dev_object_t *dev);    /* Alter register */
    int (*dump_register)(dev_object_t *dev);     /* Dump register */
    int (*show_temp)(dev_object_t *dev);         /* Show Temperature */
} lm75b_callin_fvt_t;

/*
 * NXP LM75B device callout function:
 * Service needed by the device, need to be defined by platform.
 */
typedef struct lm75b_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    uint32_t (*rd)(uint32, ushort *);
    uint32_t (*wr)(uint32, ushort *);
} lm75b_callout_fvt_t;

/*
 * NXP LM75B device object structure definition
 */
typedef struct lm75b_object_t {
    dev_object_t          base;
    lm75b_callin_fvt_t    *callin_fvt;
    lm75b_callout_fvt_t   *callout_fvt;
    reg_info_t	          *init_p;   /* Initialization table pointer */
    int                   i2c_addr;
} dev_lm75b_object_t;

extern void lm75b_dev_create(dev_object_t *, dev_error_report_t);

#endif   /* __DEV_NXP_LM75B_H__ */

/*------------------------------------------------------------------
$Log: dev_nxp_lm75b.h,v $
Revision 1.2  2018/01/20 04:14:46  hondwang
merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/09 09:20:25  hondwang
Add for Star project

Revision 1.1.2.1  2017/07/04 15:07:52  palin2
Created common driver for NXP LM75B digital temperature sensor.

$Endlog$
*/
