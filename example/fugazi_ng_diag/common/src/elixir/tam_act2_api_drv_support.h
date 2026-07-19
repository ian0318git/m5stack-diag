/* $Id: tam_act2_api_drv_support.h,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/tam_act2_api_drv_support.h,v $
 *------------------------------------------------------------------
 * 
 * tam_act2_api_drv_support.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
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
#define ACT_RETRY           100
#define ACT_DELAY           200

#define TAM_SPI_READ_BUF    (3000)

#define TAM_I2C_ADAPTER         "/dev/i2c-0"

extern int diagact2_lib_initialize(char *, int);
extern int diagact2_close_i2c_adapter(void);
extern int plat_mem_write32(uint , uint);
extern int plat_mem_read32(uint , uint *);
extern int tam_act2_i2c_initialize(void);
extern int is_tam_aikido_mbox_on(void);
extern int is_tam_aikido_on(void);
#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */

/*-------------------------------------------------
 * $Log: tam_act2_api_drv_support.h,v $
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/07/30 06:22:00  harrchan
 * Closing i2c adapter after leaving the cookie menu
 *
 * Revision 1.1.2.1  2020/09/09 09:09:54  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:29  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
