/* $Id: diag_fan_test.h,v 1.2 2016/04/20 11:25:29 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fan_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_fan_test.h - Header file for FAN Test
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FAN_TEST__
#define __DIAG_FAN_TEST__

extern int diag_fan_test(int);

#define ENV_FAN_CTL        (0x32204)
#define FAN_PWM_SLOPE   (0x32208)
#define FAN1_TACH_SPEED (0x32220)
#define FAN2_TACH_SPEED (0x32224)
#define FAN3_TACH_SPEED (0x32228)
#define FAN_RW     (READ_WRITE | SAVE_RESTORE | REG_ACCESS)

#endif /* __DIAG_FAN_TEST__ */

/*---------------------------------------------------------------
$Log: diag_fan_test.h,v $
Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/09/17 05:26:26  benchen2
add fan reg test

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/
