 /* $Id: diag_temp_snsr_test.h,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_temp_snsr_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_temp_snsr_test.h
 *
 * Description: Temperature Sensor.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_TEMP_SENSOR_TEST_H__
#define __DIAG_TEMP_SENSOR_TEST_H__

#include "types.h"
#include "i2c_api.h"

extern uint32 show_temperature_all(void);

/* Common defines */
#define ERR_BUF_SIZE    80

/* Registers type defines */
typedef uint8_t ts_p;        /*  Pointer Register         */
typedef uint8_t ts_c;        /*  Configuration Register     */
typedef uint16_t ts_t;    /*  Temperature Registers     */

enum
{
    TS_INLET_SIDE0 = 0,
    TS_OUTLET_SIDE0,
    TS_CPU,
    TS_INVALID
};
 
#define TS_INLET_ADDR0 0x4A
#define TS_OUTLET_ADDR0 0x4B

/* Temperature Sensor (TS) Registers defines - */
/* Pointer Register - */
#define TS_PTR_TEMP     0x00    /* Temperature   (Read only)  */
#define TS_PTR_CFG      0x01    /* Configuration (Read/Write) */
#define TS_PTR_THYST    0x02    /* Thyst         (Read/Write) */
#define TS_PTR_TOS      0x03    /* Tos              (Read/Write) */
#define TS_PTR_OS       0x04    /* One Shot Mode (Read/Write) */

/* Registers size (byte) */
#define TS_PTR_TEMP_L      2    /* Temperature   (Read only) */
#define TS_PTR_CFG_L       1    /* Configuration (Read/Write) */
#define TS_PTR_THYST_L     2    /* Thyst         (Read/Write) */
#define TS_PTR_TOS_L       2    /* Tos           (Read/Write) */
#define TS_PTR_OS_L        2    /* One Shot Mode (Read/Write) */

/* Configuration Register */
#define TS_CFG_SMBUS_ALERT_MASK 0x80    /* OS/SMBUS Alert Mask */
#define TS_CFG_RESV_MASK        0x40    /* Reserved Mask */
#define TS_CFG_ONE_SHOT_MASK    0x20    /* One Shot Mode Mask */
#define TS_CFG_FQ_MASK          0x18    /* Fault Queue Mask */
#define TS_CFG_OS_POL           0x04    /* O. S./Alert Pin Polarity active low/high */
#define TS_CFG_INT_MODE         0x02    /* Interrupt mode */
#define TS_CFG_SHUTDOWN         0x01    /* Shutdown low power mode */

/* Temperature, Thyst, and Tos Registers */
#define TS_TEMP_MASK         0xFFF0      /* Temperature Mask */
#define TS_TEMP_SHIFT        4           /* Temperature shift count */
#define TS_TEST_PATTERN      0x0012      /* Reg test pattern */
#define TS_TEMP_MAX          0x7D00      /* 0x7D0: 125 Celcius */
#define TS_TEMP_RESOLUTION   (0.0625)    /* One LSB: 0.0625 Celcius */

/* Interrupt test parameters */
#define CLEAR_INTERRUPT_LM75  0x7d
#define FORCE_INTERRUPT_LM75  0xc9
#define DEFAULT_CONF_LM75     0x0
#define DEFAULT_TOS_LM75      0x50
#define DEFAULT_THYST_LM75    0x4b
#define POLL_DELAY            100

/* NIOS Mailbox Temperature Register */
#define NIOS_MAILBOX_TEMP_INLET_OFFSET    0x34240
#define NIOS_MAILBOX_TEMP_OUTLET_OFFSET   0x34244
#define NIOS_MAILBOX_TEMP_CPU_OFFSET      0x3424A

/* NIOS Mailbox Temperature Limit */
#define NIOS_MAILBOX_TEMP_INTEL_MIN       (-5)
#define NIOS_MAILBOX_TEMP_INTEL_MAX       (70)
#define NIOS_MAILBOX_TEMP_OUTTEL_MIN      (-5)
#define NIOS_MAILBOX_TEMP_OUTTEL_MAX      (80)
#define NIOS_MAILBOX_TEMP_CPU_MIN         (-5)
#define NIOS_MAILBOX_TEMP_CPU_MAX         (80)

#endif /* __DIAG_TEMP_SENSOR_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_temp_snsr_test.h,v $
Revision 1.3  2020/04/20 02:28:24  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.2  2019/12/11 10:10:31  lucywang
Merged Nanook to main trunk


$Endlog$
*/
