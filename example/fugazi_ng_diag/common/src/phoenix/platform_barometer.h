/* $Id: platform_barometer.h,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_barometer.h,v $
 *------------------------------------------------------------------
 * Filename   : platform_barometer.h
 *
 * Description: Definitions for Operation Barometer.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_BAROMETER_H__
#define __PLATFORM_BAROMETER_H__

/* ST LPS25H Barometer */
#define ST_REF_P_XL        0x8
#define ST_REF_P_L         0x9
#define ST_REF_P_H         0xA
#define ST_WHO_AM_I        0xF
#define ST_RES_CONF        0x10
#define ST_STRL_REG1       0x20
#define ST_STRL_REG2       0x21
#define ST_STRL_REG3       0x22
#define ST_STRL_REG4       0x23
#define ST_INT_CFG         0x24
#define ST_INT_SOURCE      0x25
#define ST_STATUS_REG      0x27
#define ST_PRESS_OUT_XL    0x28
#define ST_PRESS_OUT_L     0x29
#define ST_PRESS_OUT_H     0x2A
#define ST_TEMP_OUT_L      0x2B
#define ST_TEMP_OUT_H      0x2C
#define ST_FIFO_CTRL       0x2E
#define ST_FIFO_STATUS     0x2F
#define ST_THS_P_L         0x30
#define ST_THS_P_H         0x31
#define ST_RPDS_L          0x39
#define ST_RPDS_H          0x3A

typedef struct barometer_reg_info {
    char * name;
    unsigned int offset;
} barometer_reg_info_t;

extern int show_barometer_info(void);
extern int display_barometer_reg(void);

#endif   /* __PLATFORM_BAROMETER_H__ */


