 /* $Id: ngio.h,v 1.2 2018/08/06 02:31:52 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/ngio.h,v $
 *------------------------------------------------------------------
 * Filename:    ngio.h
 *
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Viper does not have ngio, create this dummy file for
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
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.1  2018/02/27 08:06:51  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
