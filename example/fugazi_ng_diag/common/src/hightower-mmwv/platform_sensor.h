/* $Id: 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_sensor.h,v $
 *------------------------------------------------------------------
 * Filename: platform_sensor.h
 *
 * Description: Digital Temperature Sensor Definitions.
 *		          This file is based on TMP75/ADT75 Datasheet.
 *
 * Copyright (c) 2016-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_SENSOR_H__
#define __PLATFORM_SENSOR_H__

#include "types.h"
#include "i2c_api.h"

extern uint32 show_temperature_all(void);

enum
{
    TMP432_WR_CFG_1 = 0x9, 
    TMP432_WR_CONVER_RATE,
    TMP432_WR_LT_HL_H,
    TMP432_WR_LT_LL_H,
    TMP432_WR_RT1_HL_H,
    TMP432_WR_RT1_LL_H,  
    TMP432_WR_ONE_SHOT, /* 0xF */
};


enum
{
    TMP432_LT_H  = 0,
    TMP432_RT1_H,
    TMP432_STATUS,
    TMP432_CFG_1,
    TMP432_CONVER_RATE,
    TMP432_LT_HL_H,
    TMP432_LT_LL_H,
    TMP432_RT1_HL_H,
    TMP432_RT1_LL_H,  /* 0x8 */
    TMP432_NONE0,
    TMP432_RT1_L,    /* 0x10 */
    TMP432_RT1_HL_L = 0x13,
    TMP432_RT1_LL_L,
    TMP432_RT2_HL_H,
    TMP432_RT2_LL_H,
    TMP432_RT2_HL_L,
    TMP432_RT2_LL_L,
    TMP432_R_THREM_L,
    TMP432_R2_THREM_L,
    TMP432_OPEN_STATUS,
    TMP432_CHANNEL_MSK = 0x1F,
    TMP432_L_THREM_L,
    TMP432_THREM_HYST,
    TMP432_CONSE_ALT,
    TMP432_RT2_H,
    TMP432_RT2_L,
    TMP432_CH1_BETA_RAN_SEL,
    TMP432_CH2_BETA_RAN_SEL,
    TMP432_NF_CORR_R1,
    TMP432_NF_CORR_R2,
    TMP432_LT_L,
    TMP432_HL_STATUS = 0x35,
    TMP432_LL_STATUS,
    TMP432_THEREM_STATUS,
    TMP432_LT_HL_L = 0x3D,
    TMP432_LT_LL_L,
    TMP432_CFG_2,
    TMP432_SOFT_RESET = 0xFC,
    TMP432_DEVICE_ID,
    TMP432_MFG_ID
};


#define TMP432_BUF_SIZE     0x1
#define TMP432_RESOL        0.0625
#define TS_TEST_PATTERN     0x7D      /* Reg test pattern */

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
#define TS_TEMP_MAX         0x7D00      /* 0x7D0: 125 Celcius */
#define TS_STATUS_OPEN      0x4


#endif /* __PLATFORM_SENSOR_H__ */

/*********************************************************************
 * $Log: platform_sensor.h,v $
 * Revision 1.2  2021/06/02 02:56:22  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.2  2020/09/28 09:39:41  alpeng
 * support temp open test
 *
 *
 *
 * $Endlog$
 */

