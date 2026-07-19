/* $Id: platform_i2c_usb.h,v 1.1 2013/05/09 05:42:39 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_i2c_usb.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_i2c_usb.h
 *
 * Description: Informers SFP Cookie structs and defines.
 *		This file is based on EDCS-275976 and SFP Transceiver MSA.
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_I2C_USB_H__
#define __PLATFORM_I2C_USB_H__

#include "dev_cy7c64215.h"

/* Common defines */
#define USB_REV_REG	0	/* Revision register offset */
#define USB_CONSOLE_CABLE_SWITCH_TIME	5	/* Minutes */
#define ONE_SECOND	1000	/* Milliseconds per Second */
#define SEC_PER_MIN	60	/* Seconds per minutes */

/* Functions prototype */
extern int show_usb_ver(int);

#endif /* __PLATFORM_I2C_USB_H__ */

/*------------------------------------------------------------------
$Log: platform_i2c_usb.h,v $
Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
