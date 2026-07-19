/* $Id: diag_i2c_lib.h,v 1.2 2016/04/20 11:25:06 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_lib.h - Header file for I2C Library
 *
 * June 2015, Times Huang ported from mfgdiag
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_I2C_LIB__
#define __DIAG_I2C_LIB__

#define I2C_DRIVER_NAME                 "/dev/i2c_drv"

/* IOCTL command */
#define AESS_I2CDRV_IOC_MAGIC       0xB7
#define AESS_I2CDRV_INIT    _IOWR(AESS_I2CDRV_IOC_MAGIC, 0, sI2CDrvBusInfoType)
#define AESS_I2CDRV_CONFIG  _IOWR(AESS_I2CDRV_IOC_MAGIC, 1, sI2CDrvBusInfoType)
#define AESS_I2CDRV_WR      _IOWR(AESS_I2CDRV_IOC_MAGIC, 2, sI2CDrvBufInfoType)
#define AESS_I2CDRV_GET_MSG _IOWR(AESS_I2CDRV_IOC_MAGIC, 3, sI2CDrvBufInfoType)
#define AESS_I2CDRV_RESET   _IOWR(AESS_I2CDRV_IOC_MAGIC, 4, sI2CDrvBusInfoType)

/*****************************************************************************/
/* The following data structure came out of the IPMI stack.  I have no       */
/* idea why the element 'u8ErrorStatus' needs to be a volatile, but I        */
/* didn't want to change it.                                                 */
/*****************************************************************************/
typedef struct {
           unsigned char  u8Channel;           /** I2C bus channel */
           unsigned char  u8DeviceAddr;        /** device slave address */
  volatile unsigned char  u8ErrorStatus;       /** Bus error status */
           unsigned char *pu8MsgSendBuffer;    /** Msg transmit buffer */
           unsigned char  u8MsgSendDataSize;   /** Msg transmit data length */
           unsigned char *pu8MsgRecBuffer;     /** Msg receive buffer */
           unsigned char  u8MsgRecDataSize;    /** Msg receive data length */
		   unsigned int app_id; /* This is the application_id enumeration */
} sI2CDrvBufInfoType;

extern int diag_i2c_ping(unsigned char, unsigned char, int);
extern int diag_i2c_byte_access_variable(int, unsigned char, unsigned char, 
                                         unsigned char *, int, unsigned char *,
                                         int, int);
extern int diag_i2c_read(unsigned int, unsigned int, unsigned int, unsigned int, char *);
extern int diag_i2c_write(unsigned int, unsigned int, unsigned int, unsigned int, char *);
extern unsigned char diag_i2c_byte_read(unsigned char, unsigned char, int, unsigned char *);
extern unsigned char diag_i2c_byte_write(unsigned char, unsigned char, int, unsigned char);

#endif /* __DIAG_I2C_LIB__ */

/*---------------------------------------------------------------
$Log: diag_i2c_lib.h,v $
Revision 1.2  2016/04/20 11:25:06  benchen2
add tachi fru portion

Revision 1.1.2.4  2015/11/13 08:07:30  tirawan
Change uchar to unsigned char

Revision 1.1.2.3  2015/11/13 07:57:29  tirawan
Add Voltage and Frequency Margin

Revision 1.1.2.2  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.1  2015/06/11 02:01:08  tirawan
Add files for Tachi BMC project


$Endlog$
*/

