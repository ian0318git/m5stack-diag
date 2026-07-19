/* $Id: diag_mcu_util.h,v 1.2 2019/01/10 06:36:27 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_mcu_util.h,v $
 *------------------------------------------------------------------
 * 
 * diag_mcu_util.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_MCU_UTIL_H__
#define __DIAG_MCU_UTIL_H__

/* Common defines */

/* Register map */
#define PLAT_MCU_VERSION_REG     0x00
#define PLAT_MCU_PS_MARGIN_REG   0x17

/* C1109-4P  */
#define PLAT_MCU_VP12P0_VOLTAGE_REG   0x20
#define PLAT_MCU_MB12V_CUR_REG        0x58

/* PS Margin Reg(0x17) */
#define PLAT_MCU_R17_PS_MSK      0x0003
#define PLAT_MCU_R17_PS_NORMAL   0x0000
#define PLAT_MCU_R17_PS_ML       0x0001
#define PLAT_MCU_R17_PS_MH       0x0002
#define THOUSAND                1000
/*CISCO MCU*/

/* 
 * EDCS-1263891 from Victory  Platform PSEQ
 */
#define PWR_SEQ_REV	    0x00
#define PWR_SEQ_PIN_CODE	0x01
#define PWR_SEQ_STA_S	0x02
#define PWR_SEQ_STA_C	0x03
#define PWR_SEQ_F0_S	0x04
#define PWR_SEQ_F1_S	0x05
#define PWR_SEQ_F0_C	0x06
#define PWR_SEQ_F1_C	0x07
#define PWR_SEQ_SCR0	0x08
#define PWR_SEQ_SCR1	0x09
#define PWR_SEQ_SCR2	0x0A
#define PWR_SEQ_SCR3	0x0B
#define PWR_SEQ_SPARE	0x0C
#define PWR_SEQ_RTC_YM	0x0D
#define PWR_SEQ_RTC_DH	0x0E
#define PWR_SEQ_RTC_MS	0x0F

#define PWR_SEQ_BB_C	0x10
#define PWR_SEQ_BB_D0	0x11
#define PWR_SEQ_BB_D1	0x12
#define PWR_SEQ_BB_D2	0x13
#define PWR_SEQ_DD_D3	0x14
#define PWR_SEQ_3_3_MAR	0x15
#define PWR_SEQ_2_5_MAR	0x16
#define PWR_SEQ_DDR_MAR	0x17
#define PWR_SEQ_1_2_MAR	0x18
#define PWR_SEQ_1_0_MAR	0x19
#define PWR_SEQ_WD_EN	0x1A
#define PWR_SEQ_WD_REF	0x1B
#define PWR_SEQ_WD_TO	0x1C
#define PWR_SEQ_PR_D_EN	0x1D
#define PWR_SEQ_PW_D_T	0x1E
#define PWR_SEQ_PM   	0x1F
/*0x1F - 0x7F Reserved */

#define PWR_SEQ_12_0_LR	    0x20
#define PWR_SEQ_12_0_MAX	0x21
#define PWR_SEQ_12_0_MIN	0x22
#define PWR_SEQ_12_0_RPT	0x23
#define PWR_SEQ_5_0_LR	    0x24
#define PWR_SEQ_5_0_MAX	    0x25
#define PWR_SEQ_5_0_MIN	    0x26
#define PWR_SEQ_5_0_RPT	    0x27
#define PWR_SEQ_3_3_STBYLR	0x28
#define PWR_SEQ_3_3_STBYMAX	0x29
#define PWR_SEQ_3_3_STBYMIN	0x2A
#define PWR_SEQ_3_3_STBYRPT	0x2B
#define PWR_SEQ_3_3_LR	0x2C
#define PWR_SEQ_3_3_MAX	0x2D
#define PWR_SEQ_3_3_MIN	0x2E
#define PWR_SEQ_3_3_RPT	0x2F

#define PWR_SEQ_2_5_LR	0x30
#define PWR_SEQ_2_5_MAX	0x31
#define PWR_SEQ_2_5_MIN	0x32
#define PWR_SEQ_2_5_RPT	0x33
#define PWR_SEQ_1_8_LR	0x34
#define PWR_SEQ_1_8_MAX	0x35
#define PWR_SEQ_1_8_MIN	0x36
#define PWR_SEQ_1_8_RPT	0x37
#define PWR_SEQ_1_2_LR	0x3C
#define PWR_SEQ_1_2_MAX	0x3D
#define PWR_SEQ_1_2_MIN	0x3E
#define PWR_SEQ_1_2_RPT	0x3F

#define PWR_SEQ_0_9_LR	0x48
#define PWR_SEQ_0_9_MAX	0x49
#define PWR_SEQ_0_9_MIN	0x4A
#define PWR_SEQ_0_9_RPT	0x4B
#define PWR_SEQ_0_6_LR	0x4C
#define PWR_SEQ_0_6_MAX	0x4D
#define PWR_SEQ_0_6_MIN	0x4E
#define PWR_SEQ_0_6_RPT	0x4F

#define PWR_SEQ_3_0_LR	0x50
#define PWR_SEQ_3_0_MAX	0x51
#define PWR_SEQ_3_0_MIN	0x52
#define PWR_SEQ_3_0_RPT	0x53

#define PWR_SEQ_12_MB_LR	0x58
#define PWR_SEQ_12_MB_MAX	0x59
#define PWR_SEQ_12_MB_MIN	0x5A
#define PWR_SEQ_12_MB_RPT	0x5B
#define PWR_SEQ_DEBUG_LR	0x78
#define PWR_SEQ_DEBUG_MAX	0x79
#define PWR_SEQ_DEBUG_MIN	0x7A
#define PWR_SEQ_DEBUG_RPT	0x7B
#define PWR_SEQ_VREF_V_LR	0x80
#define PWR_SEQ_VREF_V_MAX	0x81
#define PWR_SEQ_VREF_V_MIN	0x82
#define PWR_SEQ_VREF_V_RPT	0x83
#define PWR_SEQ_TMP_LR	    0x84
#define PWR_SEQ_TMP_MAX	    0x85
#define PWR_SEQ_TMP_MIN	    0x86
#define PWR_SEQ_TMP_RPT	    0x87

#define PWR_SEQ_DEBUG_PON_MIN	0xE0
#define PWR_SEQ_DEBUG_POFF_MIN	0xE1
#define PWR_SEQ_DEBUG_MAX_OP	0xE2
#define PWR_SEQ_DEBUG_MIN_OP	0xE3

#define PWR_SEQ_VREF_V_PON	    0xE8
#define PWR_SEQ_VREF_V_POFF	    0xE9
#define PWR_SEQ_VREF_V_MAX_OP	0xEA
#define PWR_SEQ_VREF_V_MIN_OP	0xEB
#define PWR_SEQ_TMP_PON      	0xEC
#define PWR_SEQ_TMP_POFF	    0xED
#define PWR_SEQ_TMP_MAX_OP	    0xEE
#define PWR_SEQ_TMP_MIN_OP	    0xEF

/* 0xF0 - 0xFF reserved */
#define SREC_DATA_SZ 32
#define BIN_FW "mcuimg.bin"
#define PWR_SEQ_FW_DEFAULT_PATH "pseq.s19"
#define FW_BIN_SZ 0x10000
#define MAX_REGION 2
/* Firmware Revision - 0x00 */
#define PS_REV_MAJOR_MSK     0xFF00 /* Major Revision */
#define PS_REV_MAJOR_SHIFT        8 /* Major Revision Shift */
#define PS_REV_MINOR_MSK     0x00FF /* Minor Revision */
/*CISCO MCU*/
#define I2C2_DEVICE_PATH    "/dev/i2c-2"
#define DIAG_MCU_FILE     	"/firmware/mcuimg"
#define DIAG_MCU_AP_SIZE    0x5000  /* noraml 0x3000 =0x9000 ~ 0xC000 */
#define MCU_BOOT_SIZE       1024*2
#define STAR_BUF_SIZE       256
/* CISCO MCU */
/* Power sequencer FW upgrade */
#define PWR_SEQ_FW_CMD_REG  0xFE
#define PWR_SEQ_FW_DATA_REG 0xFF
#define PWR_SEQ_CMD_UPDATE  0xFFEE
#define PWR_SEQ_CMD_REBOOT  0xFFED
#define PWR_SEQ_CMD_MAX_ADDR    0xFBFF
#define PWR_SEQ_EEPROM_END  0xFC00
/*
 *  * Error code returned by the I2C low level write/read operation
 *   */
enum {
    RC_I2C_OP_OK = 0,
    RC_I2C_BUSY,
    RC_I2C_TIMEOUT, 
    RC_I2C_DMA_ADDR_NOT_64ALIGN,
    RC_I2C_SLV_NACK,
    RC_I2C_SLV_SUB_ADDR_NACK,
    RC_I2C_BUS_ERR,
    RC_I2C_UNKNOWN,  /* always last item */
};
/* Externs */
extern int plat_mcu_utils(int);
extern int diag_volts_margin_util(int);
extern int plat_display_voltage(void);
extern int plat_mcu_reg_rd(uint32_t, uint16_t *);
extern int plat_mcu_reg_wr(uint32_t, uint16_t);
extern int srec2bin_main(int, char *argv[], int*, int*);

typedef struct plat_mcu_table_t {
    char      *p_regs;          /* point to a reg string */
    uint16_t  reg_addr;       /* register I2C address */
} plat_mcu_table;

#endif /* __DIAG_MCU_UTIL_H__ */

/*-------------------------------------------------
 * $Log: diag_mcu_util.h,v $
 * Revision 1.2  2019/01/10 06:36:27  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
