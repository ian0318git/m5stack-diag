/* $Id: ngio.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/ngio.h,v $
 *------------------------------------------------------------------
 * Filename:    ngio.h
 *
 *
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Fugazi does not have ngio, create this dummy file for
 * common source file cookie_4_core.c needs to include it.
 *
 *------------------------------------------------------------------
 */
#ifndef __NGIO_H__
#define __NGIO_H__

extern int ngiosm_i2c_unreset(void *);
extern int ngiosm_i2c_reset(void *);
extern int ngiovm_i2c_unreset(void *);
extern int ngiovm_i2c_reset(void *);
extern int ngiowic_i2c_unreset(void *);
extern int ngiowic_i2c_reset(void *);


#endif /*__NGIO_H__*/


/*-------------------------------------------------
 * $Log: ngio.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:34  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

