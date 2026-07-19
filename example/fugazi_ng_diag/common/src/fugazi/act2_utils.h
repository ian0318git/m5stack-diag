 /* $Id: act2_utils.h,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/act2_utils.h,v $
 *----------------------------------------------------------------------------
 * act2_utils.h  Header file for ACT2 support/related code.
 *
 * May 2011: Alan O'Sullivan  
 *
 * Copyright (c) 2013-2020 by Cisco Systems, Inc.
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
extern int act2_install_CLIIP(void);
extern int act2_install_SUDI(void);
extern int act2_get_serial_num(void);
extern int act2_get_cskmp(void);
extern int act2_read_cskmp(void);
extern int act2_mfg_login(void);
extern int act2_get_session_id(void);
extern int act2_close_mfg_login(void);
extern int user_get_act2_version(void);
extern void *act2_get_n2g_i2c_if(void);
extern int act2_prog(boolean);
extern void act2_init_cont(void *con);
extern int act2_version(int);
extern int act2_reset(int);
extern int tam_act2_prog(boolean);
extern void *tam_act2_get_n2g_i2c_if(void);


#endif  /* __ACT2_UTILS_H__ */


/******** History ********
$Log: act2_utils.h,v $
Revision 1.2  2021/06/02 08:22:34  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.1.8.2  2020/08/26 02:37:47  iachang
Merge Fugazi code into main trunk

Revision 1.1.6.3  2020/07/29 03:02:41  iachang
Code clean up

Revision 1.1.6.2  2019/03/14 03:48:24  letsai
Initial check in.



$Endlog$
*/
