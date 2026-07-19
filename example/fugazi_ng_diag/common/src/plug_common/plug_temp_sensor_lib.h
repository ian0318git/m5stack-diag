/* $Id: plug_temp_sensor_lib.h,v 1.2 2018/01/20 04:53:29 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_temp_sensor_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : plug_temp_sensor_lib.h
 * Description: Header file of Pluggable GPIO Expander Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_TEMP_SENSOR_LIB_H__
#define __PLUG_TEMP_SENSOR_LIB_H__

#include "dev_tmpx75.h"

#define PLUG_I2C_ADDR_TEMP             (0x9C >> 1)

extern int plug_ts_dev_create(dev_tmpx75_object_t *);
extern int plug_ts_show_temp(void);

#endif

/*-------------------------------------------------
$Log: plug_temp_sensor_lib.h,v $
Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:40:40  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.2  2017/07/20 17:23:10  tirawan
Add Pluggable host implementation codes

Revision 1.1.2.1  2017/07/13 06:32:19  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.3  2017/06/22 19:27:12  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

