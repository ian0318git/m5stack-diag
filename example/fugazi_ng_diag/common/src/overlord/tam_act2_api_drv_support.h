/* $Id: tam_act2_api_drv_support.h,v 1.2 2019/08/06 06:56:10 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/tam_act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * tam_act2_api_drv_support.h  Support for ACT2 API code.
 *
 * Copy from TSN
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

/******** History ********
$Log: tam_act2_api_drv_support.h,v $
Revision 1.2  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2018/09/27 09:46:23  alpeng
support tam lib and aikido for curie

Revision 1.1.2.1  2018/09/10 01:45:35  alpeng
port header file from tsn, will clean up later


$Endlog $
*/
