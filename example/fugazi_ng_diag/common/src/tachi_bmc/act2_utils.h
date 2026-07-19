/* $Id: act2_utils.h,v 1.2 2016/04/20 11:25:24 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/act2_utils.h,v $
 *----------------------------------------------------------------------------
 * act2_utils.h  Header file for ACT2 support/related code.
 *
 * June 2015 - Times Huang ported from O2
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __ACT2_UTILS_H__
#define __ACT2_UTILS_H__

#define TOKEN_CERT_SIZE                         (256)
#define TOKEN_SIG_SIZE                          (256)
#define CLIIP_SIZE                              (128)

#define MAX_PID_LEN                             (31)
#define MAX_VID_LEN                             (5)
#define MAX_CSN_LEN                             (11)
#define EEPROM_RD_WR_LENGTH                     (512)
#define EEPROM_WRITE_ADDR                       (0x380)

extern int act2_switch_simple_mode(int);
extern int act2_install_CLIIP (void);
extern int act2_install_SUDI (void);
extern int act2_get_serial_num (void);
extern int act2_get_cskmp (void);
extern int act2_read_cskmp (void);
extern int act2_mfg_login (void);
extern int act2_get_session_id (void);
extern int act2_close_mfg_login (void);
extern int user_get_act2_version(void);
extern void *act2_get_n2g_i2c_if(void);
extern int act2_prog(boolean);
extern void act2_init_cont(void *con);
extern int act2_version(int);
extern int act2_reset(int);

#endif  /* __ACT2_UTILS_H__ */
/******** History ********
$Log: act2_utils.h,v $
Revision 1.2  2016/04/20 11:25:24  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.2  2015/07/24 03:39:35  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function

Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
Add files for Tachi BMC project


$Endlog$
*/
