/* $Id: platform_temp_sensor.h,v 1.2 2019/08/06 06:56:14 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_temp_sensor.h,v $
 *------------------------------------------------------------------
 * Filename: platform_temp_sensor.h
 *
 * Description: Digital Temperature Sensor Definitions.
 *		          This file is based on TMP75/ADT75 Datasheet.
 *
 * Copyright (c) 2016-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_TEMP_SENSOR_H__
#define __PLATFORM_TEMP_SENSOR_H__

#include "types.h"
#include "i2c_api.h"

extern uint32 show_temperature_all(void);

/* Common defines */
#define ERR_BUF_SIZE    80

/* Registers type defines */
typedef uint8_t ts_p;		/*  Pointer Register 		*/
typedef uint8_t ts_c;		/*  Configuration Register 	*/
typedef uint16_t ts_t;	/*  Temperature Registers 	*/

enum
{
	TS_BEZEL_SIDE0 = 0,
	TS_BEZEL_SIDE1,
	TS_IO_SIDE0,
	TS_IO_SIDE1,
	TS_INVALID
};
 
#define TS_IO_SIDE_ADDR0 0x48
#define TS_IO_SIDE_ADDR1 0x49
#define TS_BEZEL_SIDE_ADDR0 0x4A
#define TS_BEZEL_SIDE_ADDR1 0x4B

/* Temperature Sensor (TS) Registers defines - */
/* Pointer Register - */
#define TS_PTR_TEMP		0x00	/* Temperature   (Read only)  */
#define TS_PTR_CFG		0x01	/* Configuration (Read/Write) */
#define TS_PTR_THYST	0x02	/* Thyst         (Read/Write) */
#define TS_PTR_TOS		0x03	/* Tos		 	 (Read/Write) */
#define TS_PTR_OS		0x04	/* One Shot Mode (Read/Write) */

/* Registers size (byte) */
#define TS_PTR_TEMP_L		2	/* Temperature   (Read only) */
#define TS_PTR_CFG_L		1	/* Configuration (Read/Write) */
#define TS_PTR_THYST_L		2	/* Thyst         (Read/Write) */
#define TS_PTR_TOS_L		2	/* Tos           (Read/Write) */
#define TS_PTR_OS_L			2	/* One Shot Mode (Read/Write) */

/* Configuration Register */
#define TS_CFG_SMBUS_ALERT_MASK	0x80	/* OS/SMBUS Alert Mask */
#define TS_CFG_RESV_MASK		0x40	/* Reserved Mask */
#define TS_CFG_ONE_SHOT_MASK	0x20	/* One Shot Mode Mask */
#define TS_CFG_FQ_MASK			0x18	/* Fault Queue Mask */
#define TS_CFG_OS_POL			0x04	/* O. S./Alert Pin Polarity active low/high */
#define TS_CFG_INT_MODE			0x02	/* Interrupt mode */
#define TS_CFG_SHUTDOWN			0x01	/* Shutdown low power mode */

/* Temperature, Thyst, and Tos Registers */
#define TS_TEMP_MASK		0xFFF0	    /* Temperature Mask */
#define TS_TEMP_SHIFT		4 		    /* Temperature shift count */
#define TS_TEST_PATTERN     0x3012      /* Reg test pattern */
#define TS_TEMP_MAX         0x7D00      /* 0x7D0: 125 Celcius */
#define TS_TEMP_RESOLUTION  (0.0625)    /* One LSB: 0.0625 Celcius */


#endif /* __PLATFORM_TEMP_SENSOR_H__ */

