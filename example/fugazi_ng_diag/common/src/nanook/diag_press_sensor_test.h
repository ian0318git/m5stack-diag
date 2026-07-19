 /* $Id: diag_press_sensor_test.h,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_press_sensor_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_press_sensor_test.h
 *
 * Description: Pressure Sensor.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_PRESS_SENSOR_TEST_H__
#define __DIAG_PRESS_SENSOR_TEST_H__

#include "types.h"
#include "i2c_api.h"

extern uint32 show_temperature_all(void);
extern int ps_init(void);
extern int ps_deinit(void);

/* Common defines */
#define ERR_BUF_SIZE        80
#define POLL_DELAY          100

#define PS_REF_P_XL         0x08
#define PS_REF_P_L          0x09
#define PS_REF_P_H          0x0A

/* Temperature Sensor (TS) Registers defines - */
/* Pointer Register - */
#define PS_RES_CONF             0x10
#define PS_CTRL_REG1            0x20
#define PS_CTRL_REG2            0x21
#define PS_CTRL_REG3            0x22
#define PS_CTRL_REG4            0x23
#define PS_INTR_CONF            0x24
#define PS_FIFO_CTRL            0x2E
#define PS_STATUS_REG           0x27
#define PS_PRESS_OUT_XL         0x28
#define PS_PRESS_OUT_L          0x29
#define PS_PRESS_OUT_H          0x2A
#define PS_PHS_P_L              0x30
#define PS_PHS_P_H              0x31
#define PS_RPDS_L               0x39
#define PS_RDPS_H               0x3A

#define PS_ONE_BYTE         1
#define PS_TWO_BYTE         2
#define PS_POLL_TIME_OUT    100
#define PS_TEST_PATTERN     0x68

#endif                          /* __DIAG_PRESS_SENSOR_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_press_sensor_test.h,v $
Revision 1.3  2020/04/20 02:28:24  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.2  2019/12/11 10:10:30  lucywang
Merged Nanook to main trunk


$Endlog$
*/
