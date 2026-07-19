/* $Id: platform_sensor.h,v 1.1 2013/05/09 05:42:40 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_sensor.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_sensor.h
 *
 * Description: Overlord Diode Sensor. This file is based on Max1617A datasheet.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SENSOR_H__
#define __PLATFORM_SENSOR_H__

#include "dev_max1617a.h"

/* Common defines */
#define SNSR_CONV_TIME		1000	/* 1 Hz --> 1 second */

/* Functions prototype */
extern int init_snsr(void);
extern int show_sensor_temp(int err_log, int format);
extern int set_1617_alert(int delta, sn_d *cur_t_hi, char *err_buf);
extern int restore_1617_alert(sn_d cur_t_hi, char *err_buf);
extern int clear_snsr_alert(void);
extern int gen_snsr_alert(void);

#endif /* __PLATFORM_SENSOR_H__ */

/*------------------------------------------------------------------
$Log: platform_sensor.h,v $
Revision 1.1  2013/05/09 05:42:40  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:24  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:10  ptong
Initial archive of ng_diag module


$Endlog$
*/
