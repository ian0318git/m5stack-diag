/* $Id: ich9_i2c_regs.h,v 1.2 2012/03/28 00:38:11 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/ich9_i2c_regs.h,v $
 *------------------------------------------------------------------
 * Filename:	ich9_i2c_regs.h
 *
 * Description:	Intel ICH9 (Southbridge) I2C header file.
 *
 * Copyright (c) 2008-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __ICH9_I2C_REGS_H__
#define __ICH9_I2C_REGS_H__

/* Common Defines */
#define ICH9PAD volatile uchar

/* Device 31, Function 3: SMBus Controller Registers */
/*	PCI Configuration */
/* DID/VID (31, 3, 00h) */
#define ICH9_SMB_ID		0x29308086 /* PCI Vendor and Device IDs */

/* SMBMBAR0 - Memory Base Address Register 0 (31, 3, 10h) */
#define ICH9_SMB_SMBMBAR0_BA_M	0xFFFFFF00 /* Base address mask */
#define ICH9_SMB_SMBMBAR0_PREF	0x00000008 /* Prefetchable */
#define ICH9_SMB_SMBMBAR0_ADDRNG 0x00000004 /* Address Range */
#define ICH9_SMB_SMBMBAR0_M	0x00000000 /* Memory Space Indicator */

/* SMB_BASE - SMBus Base Address (31, 3, 20h) */
#define ICH9_SMB_SMB_BASE_BA_M	0x0000FFE0 /* Base address mask */
#define ICH9_SMB_SMB_BASE_IO	0x00000001 /* I/O Space Indicator */

/* HOSTC - Host Configuration (31, 3, 40h) */
#define ICH9_SMB_SVID		0x2C	/* 2C-2D Subsystem Vendor ID */
#define ICH9_SMB_SID		0x2E	/* 2E-2F Subsystem Identification */
#define ICH9_SMB_INT_LN		0x3C	/* 3C Interrupt Line */
#define ICH9_SMB_INT_PN		0x3D	/* 3D Interrupt Pin */
#define ICH9_SMB_HOSTC		0x40	/* Host Configuration */

#define ICH9_SMB_HOSTC_SSRESET	0x08	/* Soft SMBus Reset */
#define ICH9_SMB_HOSTC_I2C_EN	0x04	/* I2C type, not SMBus */
#define ICH9_SMB_HOSTC_SMI_EN	0x02	/* SMB Interrupt - SMI */
#define ICH9_SMB_HOSTC_HST_EN	0x01	/* SMBus Host enable */

/*	SMBus I/O and Memory Mapped I/O Registers */
typedef struct ich9_smb_reg_t_ {
    volatile uchar hst_sts;	/* 0 - Host Status */
    ICH9PAD reserve1;		/* Reserved */
    volatile uchar hst_cnt;	/* Host Control */
    volatile uchar hst_cmd;	/* Host Command */
    volatile uchar xmit_slva;	/* Transmit Slave Address */
    volatile uchar hst_d0;	/* Host Data 0 */
    volatile uchar hst_d1;	/* Host Data 1 */
    volatile uchar host_block_db; /* Host Block Data Byte */
    volatile uchar pec;		/* 8 - Packet Error Check */
    volatile uchar rcv_slva;	/* Receive Slave Address */
    volatile ushort slv_data;	/* Receive Slave Data */
    volatile uchar aux_sts;	/* Auxiliary Status */
    volatile uchar aux_ctl;	/* Auxiliary Control */
    volatile uchar smlink_pin_ctl; /* SMLink Pin Control (TCO Compatible Mode */
    volatile uchar smbus_pin_ctl; /* SMBus Pin Control */
    volatile uchar slv_sts;	/* 10 - Slave Status */
    volatile uchar slv_cmd;	/* Slave Command */
    ICH9PAD reserve12[2];	/* Reserved */
    volatile uchar notify_daddr; /* Notify Device Address */
    ICH9PAD reserve15;		/* Reserved */
    volatile uchar notify_dlow;	/* Notify Data Low Byte */
    volatile uchar notify_dhigh; /* Notify Data High Byte */
    ICH9PAD reserve18[8];	/* 18 - Reserved */
} ich9_smb_reg_t;

/* Registers offset defines */
#define ICH9_HST_STS		0	/* 0 - Host Status */
#define ICH9_HST_CNT		2	/* Host Control */
#define ICH9_HST_CMD		3	/* Host Command */
#define ICH9_XMIT_SLVA		4	/* Transmit Slave Address */
#define ICH9_HST_D0		5	/* Host Data 0 */
#define ICH9_HST_D1		6	/* Host Data 1 */
#define ICH9_HOST_BLOCK_DB	7	/* Host Block Data Byte */
#define ICH9_PEC		8	/* 8 - Packet Error Check */
#define ICH9_RCV_SLVA		9	/* Receive Slave Address */
#define ICH9_SLV_DATA		0x0A	/* Receive Slave Data */
#define ICH9_AUX_STS		0x0C	/* Auxiliary Status */
#define ICH9_AUX_CTL		0x0D	/* Auxiliary Control */
#define ICH9_SMLINK_PIN_CTL	0x0E	/* SMLink Pin Control (TCO Comp. Mode */
#define ICH9_SMBUS_PIN_CTL	0x0F	/* SMBus Pin Control */
#define ICH9_SLV_STS		0x10	/* 10 - Slave Status */
#define ICH9_SLV_CMD		0x11	/* Slave Command */
#define ICH9_NOTIFY_DADDR	0x14	/* Notify Device Address */
#define ICH9_NOTIFY_DLOW	0x16	/* Notify Data Low Byte */
#define ICH9_NOTIFY_DHIGH	0x17	/* Notify Data High Byte */

/* HST_STS - Host Status Register (00h) */
#define ICH9_HST_STS_DS		0x80	/* Byte Done Status */
#define ICH9_HST_STS_INUSE	0x40	/* In use status */
#define ICH9_HST_STS_SMBALERT	0x20	/* SMBAlert Status */
#define ICH9_HST_STS_FAILED	0x10	/* Failed */
#define ICH9_HST_STS_BUS_ERR	0x08	/* Bus error - Transaction collision */
#define ICH9_HST_STS_DEV_ERR	0x04	/* Device error */
#define ICH9_HST_STS_INTR	0x02	/* Interrupt - succesful completion */
#define ICH9_HST_STS_HOST_BUSY	0x01	/* Running a command */
#define ICH9_HST_STS_RESET_MASK (ICH9_HST_STS_FAILED | ICH9_HST_STS_BUS_ERR | \
				 ICH9_HST_STS_DEV_ERR | ICH9_HST_STS_INTR | \
				 ICH9_HST_STS_HOST_BUSY)

/* HST_CNT - Host Control Register (01h) */
#define ICH9_HST_CNT_PEC_EN	0x80	/* Packet Error Checking enable */
#define ICH9_HST_CNT_START	0x40	/* Initiates the SBM command */
#define ICH9_HST_CNT_LAST_BYTE	0x20	/* Block Read next byte is last byte */
#define ICH9_HST_CNT_SMB_CMD	0x1C	/* SMB command mask */
/*	SMBus Commands */
#define ICH9_HST_CNT_SMB_QUICK	0x00	/* Quick Command */
#define ICH9_HST_CNT_SMB_BYTE	0x04	/* Byte Command */
#define ICH9_HST_CNT_SMB_BYTE_D	0x08	/* Byte Data Command */
#define ICH9_HST_CNT_SMB_WORD_D	0x0C	/* Word Data Command */
#define ICH9_HST_CNT_SMB_PROC_C	0x10	/* Process Call Command */
#define ICH9_HST_CNT_SMB_BLOCK	0x14	/* Block Command */
#define ICH9_HST_CNT_SMB_I2C_R	0x18	/* I2C Read Command */
#define ICH9_HST_CNT_SMB_BLK_P	0x1C	/* Block Process Command */
#define ICH9_HST_CNT_KILL	0x02	/* Kills current host transaction */
#define ICH9_HST_CNT_INTREN	0x01	/* Enable interrupt generation */

/* XMIT_SLVA - Transmit Slave Address Register (04h) */
#define ICH9_XMIT_SLVA_ADDR_M	0xFE	/* Address mask */
#define ICH9_XMIT_SLVA_SHIFT	1
#define ICH9_XMIT_SLVA_RD	0x01	/* Read */
#define ICH9_XMIT_SLVA_WR	0x00	/* Write */

/* HOST_BLOCK_DB - Host Block data Byte (07h) */
#define ICH9_I2C_SRAM_SIZE		32

/* RCV_SLVA - Receive Slave Address Register (09h) */
#define ICH9_RCV_SLVA_DEFAULT	0x44	/* Default Slave address */

/* AUX_STS - Auxiliary Status Register (0Ch) */
#define ICH9_AUX_STS_STCO	0x02	/* SMBus TCO Mode */
#define ICH9_AUX_STS_CRCE	0x01	/* CRC Error */

/* AUX_CTL - Auxiliary Control Register (0Dh) */
#define ICH9_AUX_CTL_E32B	0x02	/* Enable 32-Byte Buffer */
#define ICH9_AUX_CTL_AAC	0x01	/* Automatically Append CRC */

/* SMLINK_PIN_CTL - SMLink Pin Control Register (0Eh) */
#define ICH9_SMLINK_CLK_CTL	0x04	/* SMLink0 pin not overdriven low */
#define ICH9_SMLINK1_CUR_STS	0x02	/* SMLink1 pin - High */
#define ICH9_SMLINK0_CUR_STS	0x01	/* SMLink0 pin - High */

/* SMBUS_PIN_CTL - SMBus Pin Control Regiseter (0Fh) */
#define ICH9_SMBCLK_CTL		0x04	/* SMBCLK pin driven low */
#define ICH9_SMBDATA_CUR_STS	0x02	/* SMBDATA pin - High */
#define ICH9_SMBCLK_CUR_STS	0x01	/* SMBCLK pin - High */

/* SLV_STS - Slave Status Register (10h) */
#define ICH9_HOST_NOTIFY_STS	0x1	/* Received Host Notify Command */

/* SLV_CMD - Slave Command Register (11h) */
#define ICH9_SMBALERT_DIS	0x04	/* Disable interrupt or SMI# */
#define ICH9_HOST_NOTIFY_WKEN	0x02	/* Enable reception of Host Notify
					 * Command as wake event */
#define ICH9_HOST_NOTIFY_INTREN	0x01	/* Enable interrupt of SMI# when
					 * HOST_NOTIFY_STS is set */

/* NOTIFY_DADDR - Notify Device Address Register (14h) */
#define ICH9_DEVICE_ADDRESS_M	0xFE	/* Device address mask */


#endif /* __ICH9_I2C_REGS_H__ */

/*------------------------------------------------------------------
$Log: ich9_i2c_regs.h,v $
Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
