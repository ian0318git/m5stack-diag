/* $Id: ich_i2c.h,v 1.1 2013/05/09 05:42:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/ich_i2c.h,v $
 *------------------------------------------------------------------
 * pca.c
 *
 * This file contains read/write route for pca chip
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: mcharon
 *------------------------------------------------------------------
 */

#ifndef __ICH_I2C__
#define __ICH_I2C__

extern uint32_t mch_i2c_reset(void);
extern uint32_t mch_i2c_init(char);
extern uint32_t ich_i2c_reset(uint8_t);
extern uint32_t ich_i2c_init(uint8_t, char);
extern uint32_t ich_i2c_read(n2g_i2c_dev_t *, ulong, uint8_t, char *);
extern uint32_t ich_i2c_write(n2g_i2c_dev_t *, ulong, uint8_t, char *);
extern uint32_t retry_ich_i2c_read(n2g_i2c_dev_t *, ulong, uint8_t, char *, uint8_t);
extern uint32_t retry_ich_i2c_write(n2g_i2c_dev_t *, ulong, uint8_t, char *, uint8_t);
#endif

/******** History ******** 
$Log: ich_i2c.h,v $
Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
