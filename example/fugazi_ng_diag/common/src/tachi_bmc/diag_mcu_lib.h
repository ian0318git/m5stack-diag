/* $Id: diag_mcu_lib.h,v 1.3 2016/08/09 02:44:20 jimmyya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_mcu_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_mcu_lib.h - Header file for MCU Library
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __DIAG_MCU_LIB__
#define __DIAG_MCU_LIB__

#define MCU_VER_REG_OFFSET                          (0)
/* Power sequencer FW upgrade */
#define PWR_SEQ_FW_CMD_REG  0xFE
#define PWR_SEQ_FW_DATA_REG 0xFF
#define PWR_SEQ_CMD_UPDATE  0xFFEE
#define PWR_SEQ_CMD_REBOOT  0xFFED
#define PWR_SEQ_CMD_MAX_ADDR    0xFBFF
#define PWR_SEQ_EEPROM_END  0xFC00

#define REN_I2C_PROC_TIME   3   /* 800 microseconds. round up to 1ms */

extern int diag_mcu_reg_write(uint32_t, uint16_t);
extern int diag_mcu_reg_read(uint32_t, uint16_t *);

#endif /* __DIAG_MCU_LIB__ */

/*---------------------------------------------------------------
$Log: diag_mcu_lib.h,v $
Revision 1.3  2016/08/09 02:44:20  jimmyya
add MCU software updating utility

Revision 1.2  2016/04/20 11:25:28  benchen2
add tachi fru portion

Revision 1.1.2.3  2015/09/25 02:18:24  tirawan
Correct MCU reg read/write (not to byte swap) and display MCU version

Revision 1.1.2.2  2015/07/31 07:35:13  hondwang
add external function

Revision 1.1.2.1  2015/07/26 06:02:21  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming



$Endlog$
*/
