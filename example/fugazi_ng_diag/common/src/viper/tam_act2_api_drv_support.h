 /* $Id: tam_act2_api_drv_support.h,v 1.2 2018/08/06 02:31:52 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/tam_act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * tam_act2_api_drv_support.h  Support for ACT2 API code.
 *
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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

#define TAM_I2C_ADAPTER         "/dev/i2c-0"

extern int diagact2_lib_initialize(int);
extern int tam_act2_i2c_initialize(void);
#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */

/*-------------------------------------------------
 * $Log: tam_act2_api_drv_support.h,v $
 * Revision 1.2  2018/08/06 02:31:52  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.3  2018/06/14 01:28:46  harrchan
 * Remove Aikido option and Aikido keyword in whole source code
 *
 * Revision 1.1.2.2  2018/04/10 06:23:12  harrchan
 * Fix the bug of tam lib read/write
 *
 * Revision 1.1.2.1  2018/02/27 08:06:53  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
