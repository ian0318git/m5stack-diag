/* $Id: ngio.h,v 1.2 2017/08/02 14:21:47 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/ngio.h,v $
 *------------------------------------------------------------------
 * Filename:    ngio.h
 *
 *
 * Copyright (c) 2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 * TSN does not have ngio, create this dummy file for
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

/*------------------------------------------------------------------
$Log: ngio.h,v $
Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 09:17:41  iachang
clean up code

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:49  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility


$Endlog $
*/
