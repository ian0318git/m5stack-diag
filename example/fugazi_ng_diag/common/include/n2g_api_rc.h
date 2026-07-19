/* $Id: n2g_api_rc.h,v 1.2 2012/03/28 00:38:11 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/n2g_api_rc.h,v $
 *------------------------------------------------------------------
 * Filename: n2g_api_rc.h
 *
 * Description:	N2G API Return Codes. The return codes are 32 bits with
 *		the format of:
 *		16 bits - API type. 0 is device independent.
 *		16 bits - device specifics.
 *
 * Copyright (c) 2006-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __N2G_API_RC_H__
#define __N2G_API_RC_H__

/* N2G API Device Codes */
#define NA_I2C_RC	0x80010000	/* I2C APIs */
#define NA_SMI_RC	0x80020000	/* SMI APIs */

/* I2C APIs Return Code */
/*	I2C Device dependent return code has the format of 8 bits of
 *	I2C return code class, and 8 bits of class specifics.
 */
#define NA_I2C_GEN	(NA_I2C_RC | 0x0100)	/* Generic Class */
#define NA_I2C_CT	(NA_I2C_RC | 0x0200)	/* Controller generated */
#define NA_I2C_I_REG	(NA_I2C_RC | 0x0300)	/* Invalid TWSI register */
#define NA_I2C_S_STAT	(NA_I2C_RC | 0x0400)	/* Invalid Start status */
#define NA_I2C_RA_STAT	(NA_I2C_RC | 0x0500)	/* Inv. Read Slave Addr Stat */
#define NA_I2C_WA_STAT	(NA_I2C_RC | 0x0600)	/* Inv. Write Slave Addr Stat */
#define NA_I2C_RD_STAT	(NA_I2C_RC | 0x0700)	/* Invalid read status */
#define NA_I2C_WD_STAT	(NA_I2C_RC | 0x0800)	/* Invalid write status */

/* Generic I2C return codes */
#define E_I2C_INV_P	(NA_I2C_GEN | 0x01)	/* Invalid parameters passed */
#define E_I2C_INV_DEV	(NA_I2C_GEN | 0x02)	/* Invalid device */
#define E_I2C_STAT_TO	(NA_I2C_GEN | 0x03)	/* Status read timedout */
#define E_I2C_INV_STAT	(NA_I2C_GEN | 0x04)	/* Invalid status received */
#define E_I2C_NOT_LOCKED (NA_I2C_GEN | 0x05)	/* Device not locked */
#define E_I2C_LOCKED	(NA_I2C_GEN | 0x06)	/* Dev locked by another pid */
#define E_I2C_MUX_BUSY	(NA_I2C_GEN | 0x07)	/* Other device busy on 1:4 */
#define E_I2C_TIMEDOUT	(NA_I2C_GEN | 0x08)	/* Transmit timedout */
#define E_I2C_RX_NACK	(NA_I2C_GEN | 0x09)	/* NACK */
#define E_I2C_INV_ACK	(NA_I2C_GEN | 0x0A)	/* Unexpected ACK or NACK */
#define E_I2C_BUSY	(NA_I2C_GEN | 0x0B)	/* I2C bus busy */

/* TWSI (I2C) controller error codes */
#define E_I2C_CTL_ERR	NA_I2C_CT

/* Invalid TWSI register */
#define E_I2C_INV_REG	NA_I2C_I_REG

/* Invalid status after Start bit sent */
#define E_I2C_INV_SSTAT	NA_I2C_S_STAT

/* Invalid status after slave address with read bit sent */
#define E_I2C_INV_RA_ST	NA_I2C_RA_STAT

/* Invalid status after slave address with write bit sent */
#define E_I2C_INV_WA_ST	NA_I2C_WA_STAT

/* Invalid status received during the data read */
#define E_I2C_INV_RD_ST	NA_I2C_RD_STAT

/* Invalid status received during the data write */
#define E_I2C_INV_WD_ST	NA_I2C_WD_STAT

/* SMI APIs Return Codes */
/*	SMI Device dependent return code has the format of 8 bits of
 *	SMI return code class, and 8 bits of class specifics.
 */
#define NA_SMI_GEN	(NA_SMI_RC | 0x0100)	/* Generic Class */
/*#define NA_SMI_BUSY	(NA_SMI_RC | 0x0200)	* Busy with device */

/* Generic SMI return codes */
#define E_SMI_NOT_LOCKED (NA_SMI_GEN | 0x02)	/* Device not opened */
#define E_SMI_INV_P	(NA_SMI_GEN | 0x03)	/* Invalid parameters passed */
#define E_SMI_INV_DEV	(NA_SMI_GEN | 0x04)	/* Invalid device */
#define E_SMI_INV_BD	(NA_SMI_GEN | 0x05)	/* Unsupported platform */
#define E_SMI_TIMEOUT	(NA_SMI_GEN | 0x06)	/* Read/write timed out */
#define E_SMI_LOCKED	(NA_SMI_GEN | 0x07)	/* SMI busy */
#define E_SMI_NULL_BASE	(NA_SMI_GEN | 0x08)	/* Unable to get SMI base */
#define E_SMI_ENG_TIMEOUT (NA_SMI_GEN | 0x09)	/* SMI Micro Engine timeout */
#define E_SMI_INV_STATE	(NA_SMI_GEN | 0x0A)	/* Invalid state */
#define E_SMI_BUSY	(NA_SMI_GEN | 0x0B)	/* Device busy with other proc*/
#define E_SMI_INV_STAT	(NA_SMI_GEN | 0x0C)	/* Invalid status detected */

/*#define E_SMI_BUSY	(NA_SMI_BUSY)		* SMI busy */

#endif /* __N2G_API_RC_H__ */

/*------------------------------------------------------------------
$Log: n2g_api_rc.h,v $
Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
