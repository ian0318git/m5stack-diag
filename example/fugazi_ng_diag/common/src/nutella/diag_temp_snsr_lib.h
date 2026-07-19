/* $Id: diag_temp_snsr_lib.h,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_temp_snsr_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_temp_sensor_lib.h
 * Description: Header file of Ttemperature sensor Library
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
