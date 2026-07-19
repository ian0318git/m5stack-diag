/* $Id: diag_i2c_lib.h,v 1.2 2019/01/10 06:36:26 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_i2c_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_i2c_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_I2C_LIB_H__
#define __DIAG_I2C_LIB_H__

#include "i2c_api.h"
/* Externs */
extern uint32_t n2g_i2c_open(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_close(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_read(n2g_i2c_if_t *);
extern uint32_t n2g_i2c_write(n2g_i2c_if_t *);
#define SCL_DRIVE_TIMES   100

#define PLUG_FPGA_I2C_OP_DELAY                      (3)
#define PLUG_FPGA_I2C_OP_TOUT                       (500)
#define PLUG_FPGA_I2C_IDLE_TIMEOUT                   30
#define PLUG_FPGA_REG_WRITE_DELAY                   1000

extern int plug_fpga_i2c_rd(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);
extern int plug_fpga_i2c_wr(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);
extern int get_i2c_fd(int);
extern int plug_fpga_i2c_ack_check(int, uint8_t, uint32_t, int32_t, uint32_t, uint32_t, uchar *);



/* CPU I2C Controller 1 Device Addresses */
/* based on hardware spec address shift 1 bit to the right */
/* I2C 0 Device Addresses */
#define MB_I2C_ADDR_EEPROM	(0xAA >> 1)     /* 0x55 EEPROM 2kbit */

/* I2C 0 Device Addresses */
#define MB_I2C_ADDR_MB_TEMP	(0x38 >> 1)     /* 0x1C Temp sensor MAX31730AUB+ */

/* I2C 2 Device Addresses */
#define MB_I2C2_POE_CONTR       0x30        /* I2C2 0x30 PoE(TI, TPS2386B) controller */
#define MB_I2C_ADDR_POE_30W_CTRLER (0x60 >> 1) /* 0x30 (after shifted) mux 1 (TI TPS2386PW) */
#define MB_I2C_ADDR_POE_EEPROM (0xA4 >> 1)  /* 0x52 EEPROM POE */
#define MB_I2C2_MCU            (0x80 >> 1)  /* 0x40 MCU */
#define MB_I2C2_MCU_BOOTLOADER   (0xA2 >> 1)  /* 0x51 MCU bootloader mode */
#define	WIFI_I2C_PLAT_ADDR_TEMP     (0x48)       /* Platform WiFi temp. sensor I2C addr. */
#define	WIFI_I2C_ADDR_TEMP     (0x38 >> 1)     /* 0x1C Wifi Temp sensor MAX31730AUB+ */


/* Pluggable I2C Device Address */
#define PLUG_I2C_ADDR_TEMP             (0x9C >> 1)    
#define PLUG_I2C_ADDR_ACT2             (0xE6 >> 1)
#define PLUG_TC_I2C_ADDR_GPIO_EXP      (0x38 >> 1)   /* Pluggable Test Card GPIO Expander */
#define PLUG_TC_I2C_ADDR_PHY           (0xB8 >> 1)   /* Pluggable Test Card 88E1112 PHY */
#define PLUG_MAN_I2C_ADDR_GPIO_EXP     (0x4E >> 1)   /* Pluggable LTE Mandatory GPIO Expander */
#define PLUG_OPT_I2C_ADDR_GPIO_EXP     (0x4C >> 1)   /* Pluggable LTE Optional GPIO Expander */

/* ACT2 Lite */
#define MB_I2C_ADDR_ACT2     (0xE0 >> 1)        /* 0x70 Secure Chip */
#define MB_I2C_ADDR_AIKIDO_ACT2  0x77           /* 0x77 AIKIDO Chip */
#define MB_I2C_MUX_ACT2            0
#define MB_I2C_CTRL_ACT2           0
#define WIFI_I2C_ADDR_ACT2   (0xE0 >> 1)      /* TBD */
#define WIFI_I2C_MUX_ACT2          0          /* TBD */
#define WIFI_I2C_CTRL_ACT2         0          /* TBD */
#define POE_I2C_ADDR_ACT2    (0xFE >> 1)      /* TBD */
#define POE_I2C_MUX_ACT2           0          /* TBD */
#define POE_I2C_CTRL_ACT2          0          /* TBD */

/* EEPROM */
#define MB_I2C_ADDR_SYS_EEPROM0	(0xA4 >> 1)     /* 0x52 EEPROM 512kbit */
#define MB_I2C_ADDR_SYS_EEPROM1	(0xA6 >> 1)     /* 0x53 EEPROM 512kbit */
#define MB_I2C_MUX_EEPROM       0
#define MB_I2C_CTRL_EEPROM      0

/* DIMM */
#define MB_I2C_ADDR_DIMM0	(0xA0 >> 1)     /* 0x50 */
#define MB_I2C_ADDR_DIMM1	(0xA2 >> 1)     /* 0x51 */

/* RTC */
#define MB_I2C_ADDR_RTC		(0xD0 >> 1)    /* 0x68 RTC DS1337S+ */

/* USB CONSOLE */
#define MB_I2C_ADDR_USB_CONSOLE_FW_DL  (0x66 >> 1)    /* 0x33 (after shifted) */
#define MB_I2C_ADDR_USB_CONSOLE  (0xC6 >> 1)    /* 0x63 (after shifted) */

/* SFP */
#define MB_I2C_ADDR_SFP0    (0xA0 >> 1)    /* 0x50 88E1112 SFP*/
#define MB_I2C_ADDR_SFP0_INT_REG    (0xAC >> 1)    /* 0x56 88E1112 SFP Internal Register */


#endif   /* __DIAG_I2C_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_i2c_lib.h,v $
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
