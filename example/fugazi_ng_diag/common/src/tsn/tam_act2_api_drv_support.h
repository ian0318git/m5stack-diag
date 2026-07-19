/* $Id: tam_act2_api_drv_support.h,v 1.2 2017/08/02 14:21:50 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/tam_act2_api_drv_support.h,v $
 *----------------------------------------------------------------------------
 * tam_act2_api_drv_support.h  Support for ACT2 API code.
 *
 * Jan 2015: Kody Ko
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
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
#define ACT_RETRY           100
#define ACT_DELAY           200

#define TAM_SPI_READ_BUF    (3000)

#define TAM_I2C_ADAPTER         "/dev/i2c-0"

extern int diagact2_lib_initialize(char *, int);
extern int tsn_mem_write32(uint , uint);
extern int tsn_mem_read32(uint , uint *);
extern int tam_act2_i2c_initialize(void);
extern int is_tam_aikido_mbox_on(void);
extern int is_tam_aikido_on(void);
#endif /* TAM_ACT2_API_DRV_SUPPORT_H_ */

/******** History ********
$Log: tam_act2_api_drv_support.h,v $
Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.4  2016/09/13 14:35:47  steja
Commit Aikido / TAM Mailbox code

Revision 1.1.4.3  2016/08/09 09:47:54  iachang
Supported FPGA/Aikido firmware upgrade.

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.2  2016/05/30 02:31:34  palin2
Updated code after ACT2 bring up.

Revision 1.1.2.1  2016/03/24 10:35:05  steja
Add Cookie and Act2 programming


$Endlog $
*/
