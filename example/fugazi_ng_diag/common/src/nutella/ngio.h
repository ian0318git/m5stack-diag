/* $Id: ngio.h,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/ngio.h,v $
 *------------------------------------------------------------------
 * Filename:    ngio.h
 *
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 * Nutella does not have ngio, create this dummy file for
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
$Log: ngio.h,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
