/* $Id: diag_fan_util.h,v 1.3 2016/07/22 06:58:17 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fan_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_fan_util.h - Header file for FAN Utility
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_FAN_UTIL__
#define __DIAG_FAN_UTIL__

extern int diag_fan_util(void);
extern void show_all_fan_rpm(void);

#define TACHI_FAN_NUM        3
#define ENV_FAN_STATUS       0x32200
#define ENV_FAN_CTRL         0x32204
#define FAN_PWM_SLOPE        0x32208
#define FAN_1_TACH_RPM       0x32210
#define FAN_2_TACH_RPM       0x32214
#define FAN_3_TACH_RPM       0x32218
#define FAN_4_TACH_RPM       0x3221C
#define FAN_1_TACH_SPEED     0x32220
#define FAN_2_TACH_SPEED     0x32224
#define FAN_3_TACH_SPEED     0x32228
#define FAN_4_TACH_SPEED     0x3222C
#define RPS_TO_RPM           0x3c
#define FAN_COMMAND_1        0x3B
#define REN_I2C_PROC_TIME    3   /* 800 microseconds. round up to 1ms */

#define FAN1_ROTATION        0x100
#define FAN2_ROTATION        0x200
#define FAN3_ROTATION        0x400

#endif /* __DIAG_FAN_UTIL__ */

/*---------------------------------------------------------------
$Log: diag_fan_util.h,v $
Revision 1.3  2016/07/22 06:58:17  benchen2
add fan alert feature

Revision 1.2  2016/04/20 11:25:29  benchen2
add tachi fru portion

Revision 1.1.2.6  2016/04/01 08:01:59  benchen2
add rtc/fan info

Revision 1.1.2.5  2015/11/16 07:52:32  benchen2
add raid utility

Revision 1.1.2.4  2015/10/08 04:42:30  benchen2
add fan util

Revision 1.1.2.3  2015/09/30 06:49:16  benchen2
add show rpm uti

Revision 1.1.2.2  2015/08/16 04:45:10  benchen2
add fan util function

Revision 1.1.2.1  2015/06/11 02:01:07  tirawan
Add files for Tachi BMC project


$Endlog$
*/
