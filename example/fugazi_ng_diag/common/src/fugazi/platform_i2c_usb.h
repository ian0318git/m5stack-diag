/* $Id: platform_i2c_usb.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_i2c_usb.h,v $
 *------------------------------------------------------------------
 * Filename:	platform_i2c_usb.h
 *
 * Description: Informers SFP Cookie structs and defines.
 *		This file is based on EDCS-275976 and SFP Transceiver MSA.
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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


/*-------------------------------------------------
 * $Log: platform_i2c_usb.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
