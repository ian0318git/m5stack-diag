/* $Id: i2c_address.h,v 1.1 2020/08/19 09:49:35 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/i2c_address.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDRESS__
#define __I2C_ADDRESS__

/* CPU I2C Controller 1 Device Addresses */
/* based on hardware spec address shift 1 bit to the right */
/* I2C 0 Device Addresses */
/* MAX5 CPLD */
#define HR_I2C0_ADDR_22	0x22

/* I2C 1 Device Addresses */
#define HR_I2C1_ADDR_48	(0x48 >> 1)     /* TMP75 Temp Sensor */
#define HR_I2C1_ADDR_E8 (0xE8 >> 1)     /* ACT2 */
#define HR_I2C1_ADDR_68 (0x68 >> 1)     /* DS1337 RTC */

#endif
/*-------------------------------------------------
$Log: i2c_address.h,v $
Revision 1.1  2020/08/19 09:49:35  markzha
*** empty log message ***


$Endlog$
*/
