/* $Id: platform_vtg_mntr.h,v 1.1 2020/01/09 01:02:05 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_vtg_mntr.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_vtg_mntr.h
 *
 * Description: Operation-Overlord Voltage Monitor. 
 *              This file is based on EDCS-618748.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_VTG_MNTR_H__
#define __PLATFORM_VTG_MNTR_H__

#include "dev_csco_10698.h"

/* Voltage Margin Control Register - 0xC0 */
#define VTG_MNTR_3_3V_MRGN_HI   0x0002  /* 3.3V Margin High */
#define VTG_MNTR_3_3V_MRGN_LO   0x0001  /* 3.3V Margin Low */

/* Voltage Margin enums */
typedef enum {
	VTG_MRGN_GET_3_3V = 0,   /* Display 3.3/3.0V Margin */
    VTG_MRGN_SET_3_3V_NORM,  /* Set 3.3V/3.0V Margin to normal */
    VTG_MRGN_SET_3_3V_HI,    /* Set 3.3V/3.0V Margin to high */
    VTG_MRGN_SET_3_3V_LO,    /* Set 3.3V/3.0V Margin to low */
} voltage_margin_t;

/* Functions prototype */
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *);
extern uint32_t err_report(dev_object_t *, char *, uint32_t);
extern void build_vtg_mntr_tst_menu(int);
extern int init_vtg_mntr(int);
extern int vtg_mrgn(int);
extern int vtg_get_version(uint16_t *);

#endif /* __PLATFORM_VTG_MNTR_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: platform_vtg_mntr.h,v $
Revision 1.1  2020/01/09 01:02:05  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
