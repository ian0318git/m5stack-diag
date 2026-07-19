/* $Id: platform_vtg_mntr.h,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_vtg_mntr.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_vtg_mntr.h
 *
 * Description: Operation-Overlord Voltage Monitor. 
 *              This file is based on EDCS-618748.
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_VTG_MNTR_H__
#define __PLATFORM_VTG_MNTR_H__

#include "dev_csco_10698.h"

/* Voltage Margin Control Register - 0xCC */
#define CMD_PAGE             0x00
#define CMD_OPERATION        0x01
#define CMD_VOUT_MARGIN_HIGH 0x25
#define CMD_VOUT_MARGIN_LOW  0x26
#define CMD_MFR_SPECIFIC_02  0xD2

#define OP_MARGIN_HIGH       0xA4
#define OP_MARGIN_LOW        0x94
#define OP_MARGIN_NONE       0x80

#define MARGIN_3460MV        0x7C /* 4*(245 + 5*0x7C) = 3460mV */
#define MARGIN_3140MV        0x6C /* 4*(245 + 5*0x6C) = 3140mV */

#define PAGE_A               0x00
#define SEL_PMBUS            0x01

/* Voltage Margin enums */
typedef enum {
	VTG_MRGN_GET_3_3V = 0,   /* Display 3.3/3.0V Margin */
    VTG_MRGN_SET_3_3V_NORM,  /* Set 3.3V/3.0V Margin to normal */
    VTG_MRGN_SET_3_3V_HI,    /* Set 3.3V/3.0V Margin to high */
    VTG_MRGN_SET_3_3V_LO,    /* Set 3.3V/3.0V Margin to low */
    VTG_MRGN_SET_3_3V_EN,    /* Set 3.3V/3.0V Margin to low */
} voltage_margin_t;

/* Functions prototype */
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *);
extern uint32_t err_report(dev_object_t *, char *, uint32_t);
extern void build_vtg_mntr_tst_menu(int);
extern int init_vtg_mntr(int);
extern int vtg_mrgn(int);
extern int show_mrgn(void);
extern int vtg_get_version(uint16_t *);

#endif /* __PLATFORM_VTG_MNTR_H__ */


/*-------------------------------------------------
 * $Log: platform_vtg_mntr.h,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.5  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.4  2019/05/13 07:35:28  letsai
 * Add utility to show current margin.
 *
 * Revision 1.1.6.3  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:29  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
