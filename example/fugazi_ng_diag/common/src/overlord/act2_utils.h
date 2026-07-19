/* $Id: act2_utils.h,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/act2_utils.h,v $
 *----------------------------------------------------------------------------
 * act2_utils.h  Header file for ACT2 support/related code.
 *
 * May 2011: Alan O'Sullivan  
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
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
extern int act2_prog (boolean);
extern void act2_init_cont(void *con);
extern int act2_version(int);
extern int act2_reset(int);


#endif  /* __ACT2_UTILS_H__ */
/******** History ********
$Log: act2_utils.h,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.136.1  2018/09/27 09:46:23  alpeng
support tam lib and aikido for curie

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.4  2013/01/15 02:25:14  palin2
Reset Quack chip(ACT2) by FPGA right after Diag is up to
ensure Quack chip in a valid state.

Revision 1.3  2012/08/11 00:18:49  mcharon
support simpe mode and add menu to reset Act chip.

Revision 1.2  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
