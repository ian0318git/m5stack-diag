/* $Id: wifi_temp_sensor_lib.h,v 1.2 2018/02/09 09:56:57 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/wifi_temp_sensor_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : wifi_temp_sensor_lib.h
 * Description: Header file of WiFi module temperature sensor Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __WIFI_TEMP_SENSOR_LIB_H__
#define __WIFI_TEMP_SENSOR_LIB_H__

#include "dev_nxp_lm75b.h"

extern int wifi_ts_dev_create(dev_lm75b_object_t *);
extern int wifi_ts_show_temp(void);

#endif

/*------------------------------------------------------------------
$Log: wifi_temp_sensor_lib.h,v $
Revision 1.2  2018/02/09 09:56:57  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 05:57:49  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/15 14:18:40  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/04 15:08:39  palin2
Added Star wifi temperature sensor diag tests.

$Endlog$
*/

