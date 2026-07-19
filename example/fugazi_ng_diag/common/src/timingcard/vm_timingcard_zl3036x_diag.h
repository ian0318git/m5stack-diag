/* $Id: vm_timingcard_zl3036x_diag.h,v 1.2 2015/02/14 12:48:43 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_zl3036x_diag.h,v $
 *******************************************************************************
 * File Name: vm_timingcard_zl3036x_diag.h
 *
 * Description: Timing Card NGVM ZL3036X main header file
 *
 * Author: Kody Ko
 *
 * Copyright (c) 2013 - 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#ifndef VM_TIMINGCARD_ZL3036X_DIAG_H_
#define VM_TIMINGCARD_ZL3036X_DIAG_H_

#define WAIT_MAX_180_SECONDS    180
#define WAIT_20_SECONDS         20

/* Define ZL3036X reference clock number */
typedef enum {
    ZL3036X_REF_0 = 0,
    ZL3036X_REF_1,
    ZL3036X_REF_2,
} zl3036x_ref_no_t;

extern long build_timingcard_3036x_menu(int);

#endif /* VM_TIMINGCARD_ZL3036X_DIAG_H_ */

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_zl3036x_diag.h,v $
 * Revision 1.2  2015/02/14 12:48:43  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.2  2014/04/25 06:56:34  kodko
 * Support ZL30361 reference 2 clock input test.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:06  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
