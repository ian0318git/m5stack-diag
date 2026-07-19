/* $Id: diag_mcu_lib.h,v 1.2 2019/01/10 06:36:26 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_mcu_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_mcu_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_MCU_LIB_H__
#define __DIAG_MCU_LIB_H__

#define DIAG_MCU_BOOTLOADER_MODE_ADDR    0x00008000
#define DIAG_MCU_APP_MODE_ADDR           0x00009000
#define DIAG_MCU_TEST_MODE_ADDR          0x0000C000  /* Not effect APP */

/* Example  address 0x12345678
 * addr[0] = 0x78
 * addr[1] = 0x56
 * addr[2] = 0x34
 * addr[3] = 0x12
* */
#define SPEAD_ADD(value, offset)  ((uint8)(((value) >> (offset * 8)) & 0xFF))
        

#define MCU_FD_FAIL                 -1
#define MCU_ZERO                     0

#define DIAG_MCU_BLMODE_ADD         0x51 /* 0xA2 >> 1 */
#define DIAG_MCU_APPMODE_ADD        0x40 /* 0x80 >> 1 */

#define DIAG_MCU_UPGRADE_OFFSET     0x00FE
#define DIAG_MCU_UPGRADE_SET        0x00AA
#define DIAG_MCU_UPGRADE_ENABLE     0x7F
#define DIAG_MCU_UPGRADE_MAGIC      0x00

#define DIAG_MCU_RW_MAX_SIZE        32 /* MCU limite */

#define MCU_ADDR_SIZE               4
#define MCU_CHARACT_MARK            0xFF

/* 2 comment + 5 address + 2 count + 1 checksum */
#define DIAG_MCU_WRITE_EXTRA_SIZE   10

/* 2 comment + 5 address + 2 count */
#define DIAG_MCU_READ_EXTRA_SIZE    9

/* 2 comment + 5 address */
#define DIAG_MCU_GO_EXTRA_SIZE      7

#define DIAG_MCU_COMMAND_SIZE_1    1

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


/*MCU I2C start times for each command */
#define DIAG_MCU_SET_WRITE_F1_S_TIMES  3
#define DIAG_MCU_SET_WRITE_F2_S_TIMES  6
#define DIAG_MCU_SET_WRITE_F3_S_TIMES  1
#define DIAG_MCU_WRITE_S_TIMES         2
#define DIAG_MCU_SET_READ_F1_S_TIMES   3
#define DIAG_MCU_SET_READ_F2_S_TIMES   6
#define DIAG_MCU_SET_READ_F3_S_TIMES   3
#define DIAG_MCU_READ_S_TIMES          2
#define DIAG_MCU_SET_GO_S_TIMES        3
#define DIAG_MCU_GO_S_TIMES            6

#define DIAG_MCU_FLAG_WRITE            0
#define DIAG_MCU_FLAG_READ             1

#define DIAG_MCU_CLEAN_BUF             0

#define MCU_SWITCH_MODE_WAIT        50000    
#define MCU_UPGRADE_READ_WAIT       10000    
#define MCU_UPGRADE_MODE_WAIT       5000    

extern uint32_t mcu_fw_upgrade(void);
extern int mcu_fw_verno(void);
extern int mcu_volcur_check(void);
extern uint32_t write_pwr_seq_fw_util(int);
extern int pwr_seq_eeprom_wr_util(void);
extern int pwr_seq_eeprom_rd_util(void);
extern int get_pwr_seq_fw_rev(int);
extern int show_reg(void);
extern int alter_reg(void);
extern int dump_pwr_seq_fw(void);
extern uint32_t mcu_fw_upg(void);
extern uint32_t mcu_fw_show(void);
#endif /* __DIAG_MCU_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_mcu_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
