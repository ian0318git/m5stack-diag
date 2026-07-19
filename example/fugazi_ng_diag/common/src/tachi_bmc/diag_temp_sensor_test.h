/* $Id: diag_temp_sensor_test.h,v 1.2 2016/04/20 11:25:25 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_temp_sensor_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_test.h - Header file for Temperature Sensor Test
 *
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_TEMPSENSOR_TEST__
#define __DIAG_TEMPSENSOR_TEST__

extern int diag_temp_sensor_test(int);
#define TPM75_TEMPERATURE_REG       0x00
#define TPM75_CONFIGURATION_REG     0x01
#define TPM75_T_LOW_REG             0x02
#define TPM75_T_HIGH_REG            0x03
#define TPM75_DEVICE_NUMBER         4

#define TEMP_SENSOR_RW         (READ_WRITE | SAVE_RESTORE | REG_ACCESS)
#endif /* __DIAG_TEMPSENSOR_TEST__ */

/*---------------------------------------------------------------
$Log: diag_temp_sensor_test.h,v $
Revision 1.2  2016/04/20 11:25:25  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/22 06:09:39  benchen2
Add temp sensor test item

Revision 1.1.2.1  2015/06/11 02:01:09  tirawan
Add files for Tachi BMC project


$Endlog$
*/
