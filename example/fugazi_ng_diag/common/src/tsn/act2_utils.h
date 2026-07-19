/* $Id: act2_utils.h,v 1.2 2017/08/02 14:21:44 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/act2_utils.h,v $
 *----------------------------------------------------------------------------
 * act2_utils.h  Header file for ACT2 support/related code.
 *
 * May 2011: Alan O'Sullivan  
 *
 * Copyright (c) 2013-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __ACT2_UTILS_H__
#define __ACT2_UTILS_H__

#define TOKEN_CERT_SIZE  256
#define TOKEN_SIG_SIZE   256
#define CLIIP_SIZE       128

#define MAX_PID_LEN             31
#define MAX_VID_LEN             5
#define MAX_CSN_LEN             11
#define EEPROM_RD_WR_LENGTH     512
#define EEPROM_WRITE_ADDR       0x380

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
extern int act2_prog (boolean);
extern void act2_init_cont(void *con);
extern int act2_version(int);
extern int act2_reset(int);
extern int tam_act2_prog(boolean);


#endif  /* __ACT2_UTILS_H__ */
/******** History ********
$Log: act2_utils.h,v $
Revision 1.2  2017/08/02 14:21:44  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:40:58  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:03  steja
tsn-branch4 merge with maintrunk




$Endlog$
*/
