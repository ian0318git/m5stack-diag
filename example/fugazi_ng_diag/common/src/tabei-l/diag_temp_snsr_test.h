 /* $Id: diag_temp_snsr_test.h,v 1.2 2019/10/17 02:16:23 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_temp_snsr_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_temp_snsr_test.h
 *
 * Description: Tabei-L Temperature Sensor.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
typedef uint8_t ts_p;		/*  Pointer Register 		*/
typedef uint8_t ts_c;		/*  Configuration Register 	*/
typedef uint16_t ts_t;	/*  Temperature Registers 	*/

enum
{
	TS_INLET_SIDE0 = 0,
	TS_OUTLET_SIDE0,
	TS_INLET_SIDE1,
	TS_OUTLET_SIDE1,
	TS_INVALID
};
 
#define TS_INLET_ADDR0 0x48
#define TS_INLET_ADDR1 0x49
#define TS_OUTLET_ADDR0 0x4A
#define TS_OUTLET_ADDR1 0x4B

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
#define TS_TEST_PATTERN     0x3010      /* Reg test pattern */
#define TS_TEMP_MAX         0x7D00      /* 0x7D0: 125 Celcius */
#define TS_TEMP_RESOLUTION  (0.0625)    /* One LSB: 0.0625 Celcius */

#define CLEAR_INTERRUPT_LM75  0x7d
#define FORCE_INTERRUPT_LM75  0xc9
#define POLL_DELAY            100

#endif                          /* __DIAG_TEMP_SENSOR_TEST_H__ */

/*------------------------------------------------------------------
$Log: diag_temp_snsr_test.h,v $
Revision 1.2  2019/10/17 02:16:23  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.4.5  2019/05/24 09:56:11  kehuang2

1.Update Temp Interrupt test
2.Clean up code

Revision 1.1.4.4  2019/03/19 09:26:26  kehuang2
Merge Sku1 and Sku2 into same image

Revision 1.1.4.3  2018/10/22 11:29:32  harrchan
Temperature sensor

Revision 1.1.4.2  2018/10/02 01:50:01  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
