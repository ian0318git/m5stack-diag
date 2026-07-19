/* $Id: tam_act2_api_drv_support.h,v 1.2 2021/06/02 08:22:36 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/tam_act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * tam_act2_api_drv_support.h  Support for ACT2 API code.
 *
 * Copy from TSN
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
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

#define TAM_SPI_READ_BUF    (3000)

#define TAM_I2C_ADAPTER         "/dev/i2c-0"

extern int diagact2_lib_initialize(char *, int);
extern int tam_act2_i2c_initialize(void);
extern int is_tam_aikido_mbox_on(void);
extern int is_tam_aikido_on(void);

#define SEGMENT_I2C_READ                                    (511)
#define ACT2_RW_RETRY                                       (50)
#define ACT_RETRY                                           (30)
#define ACT_DELAY                                           (200)

extern int act2_drv_write(void *, char *, unsigned int);
extern int act2_drv_read(void *, unsigned char *, unsigned int);

#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */


/*-------------------------------------------------
 * $Log: tam_act2_api_drv_support.h,v $
 * Revision 1.2  2021/06/02 08:22:36  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:52  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/07/29 08:57:35  iachang
 * Code clean up.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:29  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */

