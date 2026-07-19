/* $Id: wifi_temp_sensor_test.h,v 1.2 2018/02/09 09:56:57 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/wifi_temp_sensor_test.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : wifi_temp_sensor_test.h
 * Description: Header file of WiFi module Temperature Sensor Test
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __WIFI_TEMP_SENSOR_TEST_H__
#define __WIFI_TEMP_SENSOR_TEST_H__

extern int wifi_temp_sensor_reg_test(void);
extern int wifi_temp_sensor_show_reg(void);
extern int wifi_temp_sensor_alter_reg(void);
extern int wifi_temp_sensor_show_temp(void);
extern int wifi_temp_sensor_test(boolean);

#endif

/*------------------------------------------------------------------
$Log: wifi_temp_sensor_test.h,v $
Revision 1.2  2018/02/09 09:56:57  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 05:57:50  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/15 14:18:40  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/04 15:08:39  palin2
Added Star wifi temperature sensor diag tests.

$Endlog$
*/

