/* $Id: diag_i2c_api.h,v 1.2 2016/04/20 11:25:33 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_i2c_api.h,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_api.h - Header file for I2C API functions
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_I2C_API__
#define __DIAG_I2C_API__

#include "i2c_api.h"

/*
 * Error code returned by the I2C low level write/read operation
 */
enum {
  RC_I2C_OP_OK = 0,
  RC_I2C_BUSY,
  RC_I2C_TIMEOUT, 
  RC_I2C_DMA_ADDR_NOT_64ALIGN,
  RC_I2C_SLV_NACK,
  RC_I2C_SLV_SUB_ADDR_NACK,
  RC_I2C_BUS_ERR,
  RC_I2C_UNKNOWN,  /* always last item */
};

extern uint32_t n2g_i2c_reset(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_init(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_open(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_close(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *);
extern char * i2c_err_str(int);
extern unsigned char get_wic_i2c_ctrl(int);

#endif /* __DIAG_I2C_API__ */

/*---------------------------------------------------------------
$Log: diag_i2c_api.h,v $
Revision 1.2  2016/04/20 11:25:33  benchen2
add tachi fru portion

Revision 1.1.2.2  2015/08/27 01:24:26  alpeng
update i2c utils; add ngio init on linux_main.c

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/
