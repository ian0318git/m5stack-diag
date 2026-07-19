/* $Id: ngio.h,v 1.2 2019/01/10 06:36:28 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/ngio.h,v $
 *------------------------------------------------------------------
 * 
 * ngio.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
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
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
