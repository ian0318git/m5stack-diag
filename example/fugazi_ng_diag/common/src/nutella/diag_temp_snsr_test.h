/* $Id: diag_temp_snsr_test.h,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_temp_snsr_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_temp_snsr_test.h
 *
 * Description: Nutella Diode Sensor.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_TEMP_SENSOR_TEST_H__
#define __DIAG_TEMP_SENSOR_TEST_H__

#define CLEAR_INTERRUPT_LM75  0x7d
#define FORCE_INTERRUPT_LM75  0xc9
#define POLL_DELAY            100

extern int diag_temp_sensor_reg_test(void);
extern int diag_temp_sensor_show_reg(void);
extern int diag_temp_sensor_alter_reg(void);
extern int diag_temp_sensor_show_temp(void);
extern int build_snsr_menu(boolean);

#endif                          /* __DIAG_TEMP_SENSOR_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_temp_snsr_test.h,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
