/* $Id: tam_act2_api_drv_support.h,v 1.4 2019/07/11 12:31:32 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/tam_act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * tam_act2_api_drv_support.h  Support for ACT2 API code.
 *
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef TAM_ACT2_API_DRV_SUPPORT_H_
#define TAM_ACT2_API_DRV_SUPPORT_H_

#define SUCCEED             0
#define DIAG_MAX_ERRMSG_LEN 1024
#define ACT2_READ_COMMAND   0
#define ACT2_WRITE_COMMAND  0
#define I2C_SLAVE           0x0703
#define I2C_ACT2_TIMEOUT    0x0709
#define SEGMENT_I2C_READ    511
#define ACT_RETRY           30
#define ACT_DELAY           200
#define ACT_RW_DELAY        8000

#define TAM_SPI_READ_BUF    (3000)

#define AIKIDO_FPGA_VER_REG 0x2010
#define AIKIDO_REG_SIZE     4
#define ONE_SEC             1000
#define AIKID_FPGA_TIME_OUT 360

#define TAM_I2C_ADAPTER         "/dev/i2c-0"

#define MAX_WAIT_TIMES  50

extern int diagact2_lib_initialize(int);
extern int tam_act2_i2c_initialize(void);
extern int is_tam_aikido_mbox_on(void);
extern unsigned int aikido_spi_write(uint, uint, uint, uint, uchar *);
extern unsigned int aikido_spi_read(uint, uint, uint, uint, uchar *);
#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */

/*-------------------------------------------------
$Log: tam_act2_api_drv_support.h,v $
Revision 1.4  2019/07/11 12:31:32  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
