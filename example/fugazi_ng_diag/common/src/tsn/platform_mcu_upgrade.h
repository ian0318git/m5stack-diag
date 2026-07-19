/* $Id: platform_mcu_upgrade.h,v 1.2 2018/02/09 09:56:56 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_mcu_upgrade.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : platform_mcu.h
 * Description: STAR MCU upgrade.
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
 
#ifndef __PLATFORM_MCU_UPGRADE_H__
#define __PLATFORM_MCU_UPGRADE_H__

#define STAR_MCU_BOOTLOADER_MODE_ADDR    0x00008000
#define STAR_MCU_APP_MODE_ADDR           0x00009000
#define STAR_MCU_TEST_MODE_ADDR          0x0000C000  /* Not effect APP */

/* Example  address 0x12345678
 * addr[0] = 0x78
 * addr[1] = 0x56
 * addr[2] = 0x34
 * addr[3] = 0x12
* */
#define SPEAD_ADD(value, offset)  ((uint8)(((value) >> (offset * 8)) & 0xFF))
        

#define MCU_FD_FAIL                 -1
#define MCU_ZERO                     0

#define STAR_MCU_BLMODE_ADD         0x51 /* 0xA2 >> 1 */
#define STAR_MCU_APPMODE_ADD        0x40 /* 0x80 >> 1 */

#define STAR_MCU_UPGRADE_OFFSET     0x00FE
#define STAR_MCU_UPGRADE_SET        0x00AA
#define STAR_MCU_UPGRADE_ENABLE     0x7F
#define STAR_MCU_UPGRADE_MAGIC      0x00

#define STAR_MCU_RW_MAX_SIZE        32 /* MCU limite */

#define MCU_ADDR_SIZE               4
#define MCU_CHARACT_MARK            0xFF

/* 2 comment + 5 address + 2 count + 1 checksum */
#define STAR_MCU_WRITE_EXTRA_SIZE   10

/* 2 comment + 5 address + 2 count */
#define STAR_MCU_READ_EXTRA_SIZE    9

/* 2 comment + 5 address */
#define STAR_MCU_GO_EXTRA_SIZE      7

#define STAR_MCU_COMMAND_SIZE_1    1

/* MCU COMMAND */
#define  GT_COMMAND           0x00   /* Get command */
#define  RM_COMMAND           0x11   /* Read Memory command */
#define  GO_COMMAND           0x21   /* Go command */
#define  WM_COMMAND           0x31   /* Write Memory command */
#define  EM_COMMAND           0x43   /* Erase Memory command */
#define  ENV_GT_COMMAND       0xFF   /* Get command */
#define  ENV_RM_COMMAND       0xEE   /* Read Memory command */
#define  ENV_GO_COMMAND       0xDE   /* Go command */
#define  ENV_WM_COMMAND       0xCE   /* Write Memory command */
#define  ENV_EM_COMMAND       0xBC   /* Erase Memory command */


/*MCU I2C star times for each command */
#define STAR_MCU_SET_WRITE_F1_S_TIMES  3
#define STAR_MCU_SET_WRITE_F2_S_TIMES  6
#define STAR_MCU_SET_WRITE_F3_S_TIMES  1
#define STAR_MCU_WRITE_S_TIMES         2
#define STAR_MCU_SET_READ_F1_S_TIMES   3
#define STAR_MCU_SET_READ_F2_S_TIMES   6
#define STAR_MCU_SET_READ_F3_S_TIMES   3
#define STAR_MCU_READ_S_TIMES          2
#define STAR_MCU_SET_GO_S_TIMES        3
#define STAR_MCU_GO_S_TIMES            6

#define STAR_MCU_FLAG_WRITE            0
#define STAR_MCU_FLAG_READ             1

#define STAR_MCU_CLEAN_BUF             0

#define MCU_SWITCH_MODE_WAIT        50000    
#define MCU_UPGRADE_READ_WAIT       10000    
#define MCU_UPGRADE_MODE_WAIT       5000    

extern uint32_t mcu_fw_upgrade(void);
extern int mcu_fw_verno(void);
extern int mcu_volcur_check(void);
extern uint32_t write_pwr_seq_fw(int);
extern int pwr_seq_eeprom_wr(void);
extern int pwr_seq_eeprom_rd(void);
extern int get_pwr_seq_fw_rev(int);
extern int show_reg(void);
extern int alter_reg(void);
extern int dump_pwr_seq_fw(void);
extern uint32_t mcu_fw_upg(void);
extern uint32_t mcu_fw_show(void);
#endif /* __PLATFORM_MCU_UPGRADE_H__ */

/*******************************
$Log: platform_mcu_upgrade.h,v $
Revision 1.2  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 05:57:49  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.5  2017/12/13 21:10:13  steja
Update MCU to support both CISCO MCU and others MCU

Revision 1.1.4.4  2017/12/09 04:14:19  steja
Support CISCO MCU upgrade

Revision 1.1.4.3  2017/10/20 04:41:53  hondwang
Add C949-4P MCU voltage and currently display by HW request.

Revision 1.1.4.2  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.1.2.1  2017/07/10 06:37:39  hondwang
add MCU firmware upgrade



*/
