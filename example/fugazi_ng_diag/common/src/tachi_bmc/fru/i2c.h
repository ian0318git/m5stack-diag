/* $Id: i2c.h,v 1.2 2016/04/20 08:41:37 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/i2c.h,v $
    Copyright (c) 2016 by Cisco Systems, Inc.
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef I2C_H 
#define I2C_H 

// These codes are error codes returned by the IOCTL routines.
// The flags indicate the general location where the error was first found.
// If an error code comes back without a flag bit set, then it means the
// error happened in the actual requested transaction. The error mask define
// is the mask for the actual error.
#define I2C_IOCTL_PREMUX_FLAG                                    (1 << 7)
#define I2C_IOCTL_POSTMUX_FLAG                                   (1 << 6)
#define I2C_IOCTL_ISR_FLAG                                       (1 << 5)
#define I2C_IOCTL_ERROR_MASK                                      0x1F
#define I2C_IOCTL_SUCCESS                                        (0x00)
#define I2C_IOCTL_SUCCESS                                        (0x00)
#define I2C_IOCTL_GENERAL_FAILURE                                (0x01)
#define I2C_IOCTL_INVALID_BUS_NUMBER                             (0x02)
#define I2C_IOCTL_WAIT_CONTROLLER_EVENT_INTERRUPTIBLE_FAIL1      (0x03)
#define I2C_IOCTL_WAIT_CONTROLLER_EVENT_INTERRUPTIBLE_FAIL2      (0x04)
#define I2C_IOCTL_STOP_DETECTION_FAILURE                          0x05 
#define I2C_IOCTL_MUX_ASSIGNMENTS_FAILED                         (0x06)
#define I2C_IOCTL_FIFO_BUFFER_OVERRUN                            (0x07)
#define I2C_IOCTL_PRE_MUXING_FUNCTION_FAIL                       (0x08)
#define I2C_IOCTL_CONTROLLER_IS_STOPPED                          (0x09)
#define I2C_IOCTL_CAN_NOT_ACCESS_APP_MEMORY                      (0x0A)
#define I2C_IOCTL_CAN_NOT_ALLOCATE_KERNEL_MEM                    (0x0B)
#define I2C_IOCTL_CAN_NOT_COPY_FROM_USER_SPACE                   (0x0C)
#define I2C_IOCTL_CAN_NOT_COPY_FROM_KERNEL_SPACE                 (0x0D)
#define I2C_IOCTL_WAIT_TRANSACTION_EVENT_INTERRUPTIBLE_FAIL2     (0x0E)
#define I2C_IOCTL_WAIT_TRANSACTION_EVENT_INTERRUPTIBLE_FAIL1     (0x0F) 
#define I2C_IOCTL_I2C_BUS_IS_BUSY                                (0x10)
#define I2C_IOCTL_UNSTICK_BUS_ERROR                               0x11
#define I2C_IOCTL_TRANSACTION_TX_ABORT                            0x12
#define I2C_IOCTL_TRANSACTION_PENDING                             0x13
#define I2C_IOCTL_BAD_RECEIVE_COUNT                               0x14
#define I2C_IOCTL_TRANSACTION_KILLED							  0x15
#define I2C_IOCTL_INVALID_TRANSACTION							  0x16
#define I2C_IOCTL_CONTROLLER_STOPPED                              0x17
#define I2C_IOCTL_BUFFER_OVERRUN								  0x18

// These codes are determined by the ISR routines.
#define I2C_CTLR_ISR_TRANSACTION_COMPLETE_NORMALLY    (0x00)
#define I2C_CTLR_ISR_TRANSACTION_PENDING \
					(I2C_IOCTL_ISR_FLAG | I2C_IOCTL_TRANSACTION_PENDING)                                                                   
#define I2C_CTLR_ISR_TRANSACTION_TX_ABORT \
					(I2C_IOCTL_ISR_FLAG | I2C_IOCTL_TRANSACTION_TX_ABORT)                                                                   
#define I2C_CTLR_ISR_TRANSACTION_BAD_RECEIVE_COUNT  \
					(I2C_IOCTL_ISR_FLAG | I2C_IOCTL_BAD_RECEIVE_COUNT)   

#define I2C_CTLR_ISR_TRANSACTION_KILLED \
					(I2C_IOCTL_ISR_FLAG | I2C_IOCTL_TRANSACTION_KILLED)

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

typedef struct {
           unsigned char I2cNumElements;
           sI2CDrvBufInfoType *I2cMultTransStruct;
} sI2CDrvBufMultTrans;

/*****************************************************************************/
typedef struct {
           unsigned char  u8Channel;        /** I2C bus channel */
           unsigned char  u8InitMode;       /** Initial mode (IPMB or PI2C) */
           unsigned char  u8CurMode;        /** Current mode (IPMB or PI2C) */
           unsigned char  u8SlaveAddr;      /** IPMB slave address */
           unsigned char  u8Frequency;      /** I2C bus frequency selection */
  volatile unsigned char  u8ErrorStatus;    /** Bus error status */
           unsigned short u16DriverID;      /** ID for kernel event handler */
           unsigned int   u32RecFlag;       /** IPMB msg received flag */
           unsigned short u16CurStartCount; /** Current start tracking count */
           unsigned short u16CurStopCount;  /** Current stop tracking count */
           unsigned char  u8BusStatus;      /** Bus status */
           unsigned char  u8CtrlHW;         /** Control hardware */
} sI2CDrvBusInfoType;

// IOCTL command
#define AESS_I2CDRV_IOC_MAGIC       0xB7
#define AESS_I2CDRV_INIT    _IOWR(AESS_I2CDRV_IOC_MAGIC, 0, sI2CDrvBusInfoType)
#define AESS_I2CDRV_CONFIG  _IOWR(AESS_I2CDRV_IOC_MAGIC, 1, sI2CDrvBusInfoType)
#define AESS_I2CDRV_WR      _IOWR(AESS_I2CDRV_IOC_MAGIC, 2, sI2CDrvBufInfoType)
#define AESS_I2CDRV_GET_MSG _IOWR(AESS_I2CDRV_IOC_MAGIC, 3, sI2CDrvBufInfoType)
#define AESS_I2CDRV_RESET   _IOWR(AESS_I2CDRV_IOC_MAGIC, 4, sI2CDrvBusInfoType)
#define AESS_I2CDRV_GET_STATUS      \
							_IOWR(AESS_I2CDRV_IOC_MAGIC, 5, sI2CDrvBusInfoType)

#define AESS_I2CDRV_GET_HW_STATUS   \
							_IOWR(AESS_I2CDRV_IOC_MAGIC, 6, sI2CDrvBusInfoType)

#define AESS_I2CDRV_CTRL_HW _IOWR(AESS_I2CDRV_IOC_MAGIC, 7, sI2CDrvBusInfoType)

#define AESS_I2CDRV_GET_DBG_INFO \
							_IOWR(AESS_I2CDRV_IOC_MAGIC, 8, sI2CDrvBusInfoType)

#define AESS_I2CDRV_WR_NOMUX _IOWR(AESS_I2CDRV_IOC_MAGIC, 9, sI2CDrvBufInfoType)
#define AESS_I2CDRV_WR_DSBL_BUS_UNSTK _IOWR(AESS_I2CDRV_IOC_MAGIC, 10, int)
#define AESS_I2CDRV_WR_ENBL_BUS_UNSTK _IOWR(AESS_I2CDRV_IOC_MAGIC, 11, int)
#define I2CDRV_WR_MULTPL_TRANS _IOWR(AESS_I2CDRV_IOC_MAGIC, 12, \
													sI2CDrvBufMultTrans)

#define I2CDRV_GET_I2C_LOG _IOR(AESS_I2CDRV_IOC_MAGIC, 13, i2c_log_buffer)
#define I2CDRV_TOGGLE_INTERACTIVE _IOWR(AESS_I2CDRV_IOC_MAGIC, 14, \
													i2c_interact_control)
#define I2CDRV_RETRIEVE_INTERACTIVE_REQ _IOWR(AESS_I2CDRV_IOC_MAGIC, 15, \
													i2c_tx_info)
#define I2CDRV_RESPOND_TO_INTERACTIVE_REQ _IOWR(AESS_I2CDRV_IOC_MAGIC, 16, \
													i2c_tx_info)

#define I2C_DEV_NODE            "i2c_drv"
#define I2C_DEV_NODE_NAME       "/dev/" I2C_DEV_NODE
#define I2C_DRIVER_FILE_NAME    I2C_DEV_NODE_NAME

#endif   /* PILOT2_I2C_APP_H */
