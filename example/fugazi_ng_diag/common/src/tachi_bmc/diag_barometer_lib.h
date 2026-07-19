/* $Id: diag_barometer_lib.h,v 1.2 2016/04/20 11:25:27 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_barometer_lib.h,v $ 
 *------------------------------------------------------------------
 *
 * diag_mcu_util.c - MCU Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef DIAG_BAROMETER_LIB_H_
#define DIAG_BAROMETER_LIB_H_

extern uint32_t alt_sensor_read (n2g_i2c_if_t *, uint32_t);
extern uint32_t alt_sensor_write (n2g_i2c_if_t *, uint32_t);
extern int get_alt_sensor_i2c_struct(n2g_i2c_if_t *);

#endif /* DIAG_BAROMETER_LIB_H_ */
/*---------------------------------------------------------------
$Log: diag_barometer_lib.h,v $
Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/31 07:11:20  hondwang
barometer library

$Endlog$
*/
