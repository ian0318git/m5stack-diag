/* $Id: diag_temp_sensor_lib.h,v 1.2 2016/04/20 11:25:24 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_temp_sensor_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_temp_sensor_lib.h - Temp sensor Library Function
 *
 * July 2015, Times Huang  
 * 
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef DIAG_TEMP_SENSOR_LIB_H_
#define DIAG_TEMP_SENSOR_LIB_H_

#define TEMP_REG_OFFSET                             (0)
#define CONF_REG_OFFSET                             (1)
#define THRES_LOW_REG_OFFSET                        (2)
#define THRES_HIGH_REG_OFFSET                       (3)

#define REN_I2C_PROC_TIME   3   /* 800 microseconds. round up to 1ms */
extern int diag_temp_sensor_reg_write(uint32_t, uint32_t, uint16_t);
extern int diag_temp_sensor_reg_read(uint32_t, uint32_t, uint16_t *);
extern int get_temp_sensor_device_addr(int);
#define GET_ADDRESS(X)  X>>1

#define GET_TPM75_DEV_ADDR(X)  X<<1
#endif /* DIAG_TEMP_SENSOR_LIB_H_ */




/*---------------------------------------------------------------
$Log: diag_temp_sensor_lib.h,v $
Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/09/21 13:09:16  tirawan
Display temperature sensor and FPGA version during boot up

Revision 1.1.2.2  2015/08/22 06:09:40  benchen2
Add temp sensor test item

Revision 1.1.2.1  2015/07/31 07:26:42  hondwang
temp sensor lib

$Endlog$
*/
