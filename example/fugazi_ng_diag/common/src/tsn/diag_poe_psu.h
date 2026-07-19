/* $Id: diag_poe_psu.h,v 1.2 2017/08/02 14:21:45 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_poe_psu.h,v $ 
 *------------------------------------------------------------------
 *
 * Filename   : diag_poe_psu.h
 * Description: Header file of TSN PoE PSU Diag tests
 * 
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_POE_PSU_H__
#define __DIAG_POE_PSU_H__

/* Extern */
extern void diag_poe_psu(int);
extern int  diag_psu_reg_test(int);
extern int  diag_psu_intr_test(int);

#endif

/*------------------------------------------------------------------
$Log: diag_poe_psu.h,v $
Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.4.2  2017/07/29 03:41:02  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.2.1  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

$Endlog$
*/

