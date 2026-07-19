 /* $Id: diag_temp_snsr_lib.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_temp_snsr_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_temp_sensor_lib.h
 * Description: Header file of Ttemperature sensor Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_TEMP_SENSOR_LIB_H__
#define __DIAG_TEMP_SENSOR_LIB_H__

#include "dev_nxp_lm75b.h"

extern int diag_ts_dev_create(dev_lm75b_object_t *);
extern int diag_ts_show_temp(void);

#endif

/*------------------------------------------------------------------
$Log: diag_temp_snsr_lib.h,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.3  2018/04/11 08:52:32  lucywang
Modified Thermal Interrupt test to use chip object

Revision 1.1.2.2  2018/04/02 07:18:30  lucywang
Added Interrupt test for Thermal Sensor

Revision 1.1.2.1  2018/03/28 07:55:52  lucywang
Changed Thermal sersor to LM75B, TBD : bug fix


$Endlog$
*/
