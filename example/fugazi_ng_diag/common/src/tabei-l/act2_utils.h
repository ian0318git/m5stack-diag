 /* $Id: act2_utils.h,v 1.2 2019/10/17 02:16:19 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/act2_utils.h,v $
 *----------------------------------------------------------------------------
 * act2_utils.h  Header file for ACT2 support/related code.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
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
#define MAX_MAC_LEN             14
#define MAX_MAC_ADDR_BLK_LEN    4
#define EEPROM_RD_WR_LENGTH     512
#define EEPROM_WRITE_ADDR       0x380

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
Revision 1.2  2019/10/17 02:16:19  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.3  2018/12/07 08:19:59  kodko
Support to write BIOS EEPROM String 1~8 fields from fetching cookie content.

Revision 1.1.2.2  2018/11/13 09:00:53  kodko
Support to program BIOS eeprom from getting information of cookie PID/SN/MAC ADDRESS content.

Revision 1.1.2.1  2018/10/02 01:49:57  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
