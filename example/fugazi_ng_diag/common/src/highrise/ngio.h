/* $Id: ngio.h,v 1.1 2020/08/19 09:49:35 markzha Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/ngio.h,v $
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

#define FIRST_SLOT 1

extern int ngiosm_i2c_unreset(void *);
extern int ngiosm_i2c_reset(void *);
extern int ngiovm_i2c_unreset(void *);
extern int ngiovm_i2c_reset(void *);
extern int ngiowic_i2c_unreset(void *);
extern int ngiowic_i2c_reset(void *);

#endif /*__NGIO_H__*/

/*-------------------------------------------------
 * $Log: ngio.h,v $
 * Revision 1.1  2020/08/19 09:49:35  markzha
 * *** empty log message ***
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
