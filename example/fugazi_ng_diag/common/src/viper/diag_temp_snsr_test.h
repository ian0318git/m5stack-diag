 /* $Id: diag_temp_snsr_test.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_temp_snsr_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_temp_snsr_test.h
 *
 * Description: Viper Diode Sensor.
 *
 * Copyright (c) 2014-2018 by Cisco Systems, Inc.
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
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.3  2018/04/02 07:18:30  lucywang
Added Interrupt test for Thermal Sensor

Revision 1.1.2.2  2018/03/28 07:55:52  lucywang
Changed Thermal sersor to LM75B, TBD : bug fix

Revision 1.1.2.1  2018/02/27 08:06:46  harrchan
Initial viper application code base



$Endlog$
*/
