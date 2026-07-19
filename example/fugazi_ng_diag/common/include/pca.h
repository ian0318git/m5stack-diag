/* $Id: pca.h,v 1.5 2012/10/18 10:15:24 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/pca.h,v $
 *------------------------------------------------------------------
 * pca.c
 *
 * This file contains read/write route for pca chip
 * Copyright (c) 2009-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: mcharon
 *------------------------------------------------------------------
 */
#ifndef __PCA__
#define __PCA__

/* 8 bit pca9557 definition */
#define INPUT_PORT_REG     0
#define OUTPUT_PORT_REG    1
#define POLARITY_INV_REG   2
#define CONFIGURATION_REG  3

/* 16 bit pca9555 definition */
#define INPUT_PORT0_REG     0
#define INPUT_PORT1_REG     1
#define OUTPUT_PORT0_REG    2
#define OUTPUT_PORT1_REG    3
#define POLARITY_INV_P0_REG   4
#define POLARITY_INV_P1_REG   5
#define CONFIGURATION_P0_REG  6
#define CONFIGURATION_P1_REG  7

#define BIT0      0x01
#define BIT1      0x02
#define BIT2      0x04
#define BIT3      0x08
#define BIT4      0x10
#define BIT5      0x20
#define BIT6      0x40
#define BIT7      0x80

#define DB_PRESENT_L       0x0   /* Bit 0 */
#define BOOT_SELECT        0x2   /* Bit 1 */
#define DB_RESET_L         0x4   /* Bit 2 */
#define PRIM_INTF_READY    0x8   /* Bit 3 */
#define UART_MUX           0x10  /* Bit 4 */

extern int io_port_8bit_i2c_read(void *i2c, int offset, unsigned char *data,  unsigned char);
extern int io_port_8bit_i2c_write(void *i2c, unsigned int offset, unsigned char *data);
extern void pca_set_i2c(void *dev);
extern void pca_init_i2c(void *dev);

#endif
/******** History ******** 
$Log: pca.h,v $
Revision 1.5  2012/10/18 10:15:24  alpeng
suppoer 16bit pca9555 for overdrive

Revision 1.4  2012/06/02 00:57:17  srane
Fix warnings.

Revision 1.3  2012/05/16 07:27:06  srane
Initial commit for Graffham NGVM.

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
