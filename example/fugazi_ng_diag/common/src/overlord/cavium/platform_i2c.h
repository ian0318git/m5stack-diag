/* $Id: platform_i2c.h,v 1.3 2012/05/30 09:36:54 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2006-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__


/* Transformers (and N2G) I2C Bus enumeration */
typedef enum {
    CPU_I2C0 = 0,	/* Cavium TWSI 0 */
    CPU_I2C1,		/* Cavium TWSI 1 */
    I2C_BUS_INVALID,	/* Invalid I2C bus */
} I2C_BUS;

/* Overlord Cavium TWSI 0 */
typedef enum {
    OVLD_CAVIUM_TWSI_DIMM = 0,   /* DIMM 0 */
    OVLD_CAVIUM_TWSI_0_INVALID,  /* Invalid I2C */
} MB_I2C0_DEVICE;

/* Overlord Cavium TWSI 1 */
typedef enum {
    OVLD_CAVIUM_TWSI_MUX = 0,    /* 1:4 Mux */
    OVLD_CAVIUM_TWSI_SFP0,       /* SFP0 */
    OVLD_CAVIUM_TWSI_SFP1,       /* SFP1 */
    OVLD_CAVIUM_TWSI_SFP2,       /* SFP2 */
    OVLD_CAVIUM_TWSI_SFP3,       /* SFP3 */
    OVLD_CAVIUM_TWSI_1_INVALID,  /* Invalid I2C */
} OVLD_CAVIUM_TWSI1_DEVICE;

/* I2C Device address defines */
/* Cavium MUX */
#define OVLD_CAVIUM_MUX_I2C_ADDR   (0xE0 >> 1)

/* MUX port mask */
#define OVLD_MUX_PORT0_MASK        MUX9545_PORT0_MASK
#define OVLD_MUX_PORT1_MASK        MUX9545_PORT1_MASK
#define OVLD_MUX_PORT2_MASK        MUX9545_PORT2_MASK
#define OVLD_MUX_PORT3_MASK        MUX9545_PORT3_MASK
#define OVLD_MUX_PORT_NULL_MASK    MUX9545_PORT_NULL_MASK
#define OVLD_MUX_PORT_ALL_MASK     MUX9545_PORT_ALL_MASK

/* SFP ADDR */
#define OVLD_SFP_I2C_ADDR       (0xA0 >> 1)

/*	CPU I2C Controller 0 */
#define MB_I2C_ADDR_DIMM0	0x50	/* DIMM0 */


/* Function prototypes */

#endif /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
$Log: platform_i2c.h,v $
Revision 1.3  2012/05/30 09:36:54  alpeng
suppoted i2c scan test on cavium side, removing useless definition on platform_i2c.h

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
