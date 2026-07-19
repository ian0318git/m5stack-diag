/* $Id: diag_raid_lib.h,v 1.3 2016/08/09 07:44:47 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_raid_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_raid_lib.h
 * 
 * Oct. 2015, iyc
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <termios.h>

#ifndef DIAG_RAID_LIB_H_
#define DIAG_RAID_LIB_H_

extern int raid_switch_ctrl(int);
extern int platform_cpld_reg_dump(void);
extern int platform_5m570_i2c_w (uint32_t, uchar);
extern int platform_pca9557_i2c_w(uint32_t, uchar);
extern int platform_pca9557_i2c_r(uint32_t,uchar *);
extern int platform_5m570_i2c_r (uint32_t, uchar *);
extern int diag_pca9557_write_fn(unsigned long, int, unsigned long, void *);
extern int platform_cpld_write_fn (unsigned long, int,unsigned long, void *);
extern int diag_pca9557_read_fn(unsigned long, int, unsigned long *, void *);
extern int platform_cpld_read_fn (unsigned long, int, unsigned long *, void *);
extern int raid_cpld_reg_test_lib(void);
extern int raid_9557_reg_test_lib(void);
extern long platform_pca9557_init (void);
extern long platform_cpld_jtag_ctl (boolean);
extern long platform_pca9557_program_cpld(unsigned char *);
extern long platform_simply_program_cpld (unsigned char *);
extern long platform_pca9557_power_cycle_cpld (void);
extern char * parse_cpld_data(void);
extern int diag_raid_sbr_fw_upgrade(unsigned long, unsigned char *);
extern int raid_sbr_ctrl(int);
extern int raid_uart_intf_test (char *, const char *, speed_t);


#define REN_I2C_PROC_TIME               (3)   /* 800 microseconds. round up to 1ms */
#define SMB_SAS_ISOLATE                 (0x10)

#define PCA9557_TDI_GPIO2_OUTPUT        (0x1 << 2)
#define PCA9557_TDI_GPIO4_OUTPUT        (0x1 << 4)
#define PCA9557_TDO_GPIO5_INPUT         (0x1 << 5)
#define PCA9557_TCK_GPIO6_OUTPUT        (0x1 << 6)
#define PCA9557_TMS_GPIO7_OUTPUT        (0x1 << 7)

/* JTAG Control (Offset 6) */
#define CPLD_JTAG_ON                    (0x1 << 4)
#define CPLD_JTAG_TCK_ST                (0x1 << 3)
#define CPLD_JTAG_TMS_ST                (0x1 << 2)
#define CPLD_TDI_ST                     (0x1 << 1)
#define CPLD_TDO_ST                     (0x1 << 0)

/* Power Sequence Status (Offset 5) */
#define CPLD_POWER_RESTART              (0x1 << 7)
#define CPLD_VP3P3_LDO_GOOD             (0x1 << 3)
#define CPLD_VP1P8_LDO_GOOD_L           (0x1 << 2)
#define CPLD_VP3P3_LDO_EN               (0x1 << 1)
#define CPLD_VP1P8_LDO_EN               (0x1 << 0)

/* CPLD Program */
#define CPLD_ERASE_COMMAND              (0x013d)
#define CPLD_SAMPLE_PRELOAD             (0x0280)
#define CPLD_ISP_ENABLE                 (0x0266)
#define CPLD_ADDRESS_SHIFT              (0x0301)
#define CPLD_ISP_READ                   (0x0281)
#define CPLD_ISC_PROG                   (0x00bd)
#define CPLD_ISC_PROG_DONE_1            (0xFFDF)
#define CPLD_ISC_PROG_DONE_2            (0xFFFF)
#define CPLD_ISP_DISABLE                (0x019a)
#define CPLD_BYPASS                     (0x03ff)

#define PCA9557_RW                      (READ_WRITE | SAVE_RESTORE | REG_ACCESS)
#define CPLD_RONLY                      (READ_ONLY | REG_ACCESS)
#define CPLD_RW                         (READ_WRITE | REG_ACCESS)
#define EPM570_FILE_LOCATION            "/mnt/datastore/epm570.c"

/* RAID SBR EEPROM */
#define DC_I2C_SBR                          (0xA0)
#define SBR_EN_PROGRAM                      (0x1)
#define SBR_DIS_PROGRAM                     (0)
#define SBR_CTRL_LSI                        (0x20)
#define SMB_SAS_ISOLATE                     (0x10)

/* RAID SGPIO CHECK */
#define RAID_FPGA_SGPIO_LINK_PASS         (0x1064)
#define RAID_FPGA_SGPIO_LINK_TIME         5000

/* Define PCA9557 register offset */
typedef enum {
    PCA9557_NGIO_EXPANDER_INPUT = 0,
    PCA9557_NGIO_EXPANDER_OUTPUT,
    PCA9557_POLARITY_INVERSION,
    PCA9557_CONFIGURATION,
} pca9557_offset_t;

/* Define CPLD register offset */
typedef enum {
    CPLD_NGIO_EXPANDER_INPUT = 0,
    CPLD_NGIO_EXPANDER_OUTPUT,
    CPLD_RESERVED,
    CPLD_NGIO_EXPANDER_DIR,
    CPLD_ZL30363_CONT_STS,
    CPLD_POW_SEQ_STS,
    CPLD_JTAG_CTL,
    CPLD_VERSION,
    CPLD_GPIO_DIRECTION,
    CPLD_SPEED_UP,
    CPLD_TEN,
    CPLD_ELEVEN,
    CPLD_TWELVE,
    CPLD_THIRTEEN,
    CPLD_FOURTEEN,
    CPLD_FIFTEEN,
} cpld_offset_t;

#endif /* DIAG_RAID_LIB_H_ */
