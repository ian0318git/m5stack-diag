/* $Id: diag_margin_util.h,v 1.2 2016/04/20 11:25:27 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_margin_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_margin_util.h - Header file for Margin Utility
 *
 * November 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_MARGIN_UTIL__
#define __DIAG_MARGIN_UTIL__

#define MB_IDT286_PLL0_CTRL_REG                     (0x0008)

typedef enum {
    VOLT_MARG_1P5_LOW=0,
    VOLT_MARG_1P5_HIGH,
    VOLT_MARG_1P5_NORM,
    VOLT_MARG_3P3_LOW,
    VOLT_MARG_3P3_HIGH,
    VOLT_MARG_3P3_NORM
} voltage_margin_t;

typedef enum {
    FREQ_MARG_HIGH=0,
    FREQ_MARG_LOW,
    FREQ_MARG_NORM
} freq_margin_t;

extern int diag_margin_util(void);

#endif /* __DIAG_MARGIN_UTIL__ */

