/* $Id: pca9545a.h,v 1.2 2012/03/28 00:38:11 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/pca9545a.h,v $
 *------------------------------------------------------------------
 * Filename: pca9545a.h
 *
 * Description: Texas Instruments PCA9545A 1:4 Mux defines. This file is
 *		based on PCA9545A Datasheet.
 *
 * Copyright (c) 2006-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PCA9545A_H__
#define __PCA9545A_H__

typedef struct mux9545_i2c {
    uchar ctrl;			/* 0 - Control register */
} mux9545_i2c_t;

#define MUX_PS1		0x01	/* Enable PS1  (channel 0) */
#define MUX_PS2		0x02	/* Enable PS2  (channel 1) */
#define MUX_SFP1	0x04	/* Enable SFP1 (channel 2) */
#define MUX_SFP2	0x08	/* Enable SFP2 (channel 3) */

#define MUX9545_PORT0_MASK        0x01
#define MUX9545_PORT1_MASK        0x02
#define MUX9545_PORT2_MASK        0x04
#define MUX9545_PORT3_MASK        0x08
#define MUX9545_PORT_NULL_MASK    0x00
#define MUX9545_PORT_ALL_MASK     0x0f

#endif /* __PCA9545A_H__ */

/*------------------------------------------------------------------
$Log: pca9545a.h,v $
Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
