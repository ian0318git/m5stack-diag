/* $Id: platform_i2c.h,v 1.3 2017/03/30 08:34:08 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_i2c.h,v $
 *------------------------------------------------------------------
 *
 * platform_i2c.h - Header file for Platform I2C 
 *
 * July 2015, Times Huang
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLATFORM_I2C__
#define __PLATFORM_I2C__

/* ACT2 */
#define MB_I2C_ADDR_ACT2                            (0x75)   
#define MB_I2C_MUX_ACT2                             (0)
#define MB_I2C_CTRL_ACT2                            (0)

/* FPGA I2C Controller 2 */
#define MB_I2C_ADDR_ENV_MCU                         (0x88 >> 1)/* 0x44 (after shifted) */
#define MB_I2C_ADDR_SENSOR                          (0xC0 >> 1)    /* 0x60 (after shifted) */
#define MB_I2C_ADDR_MB_TEMP                         (0x30 >> 1)    /* 0x18 (after shifted) : MAX1617A */
#define MB_I2C_ADDR_MB_TEMP_ALRT                    (0x18 >> 1)    /* 0x0C (after shifted) : MAX1617A Alert*/
#define MB_I2C_ADDR_BAROMETER                       (0xC0 >> 1)    /* 0x60 (after shifted) */
#define MB_I2C_ADDR_PCA9557                         (0x30 >> 1)    /* 0x18 (after shifted) */
#define MB_I2C_ADDR_CPLD_5M570                      (0x3E >> 1)    /* 0x1F (after shifted) */
#define MB_I2C_ADDR_TEMP_INLET_U27                  (0x90 >> 1)    /* 0x48 (after shifted) */
#define MB_I2C_ADDR_TEMP_INLET_U29                  (0x92 >> 1)    /* 0x49 (after shifted) */
#define MB_I2C_ADDR_TEMP_OUTLET_U39                 (0x94 >> 1)    /* 0x4a (after shifted) */
#define MB_I2C_ADDR_TEMP_OUTLET_U337                (0x96 >> 1)    /* 0x4b (after shifted) */
#define MB_I2C_PSU_FAN                              (0xB6 >> 1) 

/* FPGA I2C Controller 4 */
#define MB_I2C_ADDR_PEM0_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PEM0_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */
#define MB_I2C_ADDR_PEM1_EEPROM      (0xA6 >> 1)    /* 0x53 (after shifted) */
#define MB_I2C_ADDR_PEM1_MCNTRL      (0xB6 >> 1)    /* 0x58 (after shifted) */

/* FPGA I2C Controller 10 */
#define NGWIC_I2C_ADDR_IO_PORT                      (0x1C)
#define NIM_I2C_ADDR_IO_PORT_16b                    (0x21)  /* 0x42 >> 1 */
#define NIM_I2C_ADDR_ACT2                           (0x74)   /* Quack after >> 1*/
#define NIM_I2C_ADDR_OIR                            (0x4B)   /* OIR after >> 1*/

/* FPGA I2C Controller 8 */
#define POE_I2C_ADDR_ACT2                           (0x75)  /* Quack after >> 1*/
/* FPGA I2C Controller 10 */
#define RAID_I2C_ADDR_ACT2                          (0x73)   /* Quack after >> 1*/

/* Test card I2C Address */
#define TEST_CARD_I2C_ADDR                          (0x20)   /* 7-bit address */

/* Raid card I2C Address */
#define RAID_CARD_I2C_ADDR                          (0xA0)   /* 7-bit address */

/* RTC */
#define MB_I2C_ADDR_RTC		                        (0xD0) /* 0x68 RTC DS1337S+ */

/* IDT for freq margin */
#define MB_I2C_IDT286                               (0xF8)

/* Daughter Card */

typedef enum {
    CPU_I2C0 = 0,   /* BMC I2C Bus 0 */
    CPU_I2C1,       /* BMC I2C Bus 1 */
    CPU_I2C2,       /* BMC I2C Bus 2 */
    CPU_I2C3,       /* BMC I2C Bus 3 */
    CPU_I2C4,       /* BMC I2C Bus 4 */
    CPU_I2C5,       /* BMC I2C Bus 5 */
    CPU_I2C6,       /* BMC I2C Bus 6 */
    CPU_I2C7,       /* BMC I2C Bus 7 */
    IOFPGA_I2C,     /* FPGA I2C Controller */
    I2C_BUS_INVALID
} I2C_BUS;

typedef enum {
    MB_I2C_RTC = 0,         /* RTC */
    MB_I2C_1_INVALID        /* Invalid I2C */
} MB_I2C1_DEVICE;

typedef enum {
    IOFPGA_I2C_ACT2 = 0,    /* ACT2 */
    IOFPGA_I2C_INVALID
} MB_FPGA_I2C_DEVICE;

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO                       (0)
#define I2C_CTRL_ONE                        (1)
#define I2C_CTRL_TWO                        (2)
#define I2C_CTRL_THREE                      (3)
#define I2C_CTRL_FOUR                       (4)
#define I2C_CTRL_FIVE                       (5)
#define I2C_CTRL_SIX                        (6)
#define I2C_CTRL_SEVEN                      (7)
#define I2C_CTRL_EIGHT                      (8)
#define I2C_CTRL_NIGHT                      (9)
#define I2C_CTRL_TEN                        (10)
#define I2C_CTRL_ELEVEN                     (11)
#define I2C_CTRL_TWELVE                     (12)
#define I2C_CTRL_THIRTEEN                   (13)
#define I2C_CTRL_FOURTEEN                   (14)
#define I2C_CTRL_FIFTEEN                    (15)
#define I2C_CTRL_SIXTEEN                    (16)
#define I2C_CTRL_SEVENTEEN                  (17)
#define I2C_CTRL_MAX I2C_CTRL_SEVENTEEN

/* Common definition for MUX */
#define I2C_MUX_ZERO                        (0)
#define I2C_MUX_ONE                         (1)
#define I2C_MUX_TWO                         (2)
#define I2C_MUX_THREE                       (3)
#define I2C_MUX_MAX I2C_MUX_THREE


extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void *platform_fpga_get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_get_wic_oir(int);
extern uint8_t get_wic_i2c_ctrl(int);
extern uint8_t get_daughter_card_i2c_ctrl(int);
extern uint8_t get_daughter_card_i2c_addr(int);

#endif /* __PLATFORM_I2C__ */

/*---------------------------------------------------------------
$Log: platform_i2c.h,v $
Revision 1.3  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.2  2016/04/20 11:25:30  benchen2
add tachi fru portion

Revision 1.1.2.24  2016/03/02 08:35:42  benchen2
add sbr vdd eeprom ping test

Revision 1.1.2.23  2016/01/11 11:19:37  benchen2
for P2 temp sensor change address 0x98-> 0x96

Revision 1.1.2.22  2016/01/11 10:28:16  tirawan
Add Test card menu to run FPGA i2c register test, btb test from x86 and Lewis

Revision 1.1.2.21  2015/12/23 11:16:14  alpeng
support PEM(PSU) utility and its fan utils

Revision 1.1.2.20  2015/12/16 05:30:00  huanngo
Add MB_I2C_ADDR_BAROMETER definition to fix compilation error

Revision 1.1.2.19  2015/12/16 01:55:53  huanngo
Add support for FPGA I2C device scan utility

Revision 1.1.2.18  2015/11/16 08:06:12  benchen2
add psu fan addr

Revision 1.1.2.17  2015/11/13 09:28:33  benchen2
modify raid card act address

Revision 1.1.2.16  2015/11/13 07:57:29  tirawan
Add Voltage and Frequency Margin

Revision 1.1.2.15  2015/11/02 10:22:56  tirawan
Add PoE Cookie Utility

Revision 1.1.2.14  2015/10/28 07:55:04  benchen2
add raid act2 cookies utility

Revision 1.1.2.13  2015/10/20 08:21:26  benchen2
add raid util(PCA9557, control switch)

Revision 1.1.2.12  2015/10/12 08:28:57  benchen2
add pca9557 i2c define

Revision 1.1.2.11  2015/09/18 06:58:54  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.10  2015/09/17 13:05:10  tirawan
Add Bios Boot Flash Test, fixes I2C controller number for NIM

Revision 1.1.2.9  2015/09/17 02:14:45  benchen2
fix i2c address error (0x96 -> 0x98)

Revision 1.1.2.8  2015/08/30 05:57:36  tirawan
To support NIM ACT2 R/W access using TAM library

Revision 1.1.2.7  2015/08/21 11:31:21  benchen2
add temperature sensor utility

Revision 1.1.2.6  2015/08/16 06:01:01  tirawan
Tachi bring up fix: SPI Flash Test, I2C Library for RTC Test, I2C scan Test, CPU ID fix for PECI test

Revision 1.1.2.5  2015/08/04 03:32:20  meho
Added RTC tests.

Revision 1.1.2.4  2015/07/31 08:41:01  hondwang
add barometer address

Revision 1.1.2.3  2015/07/28 09:32:38  alpeng
adding entry for ngio test on platform_slot.c

Revision 1.1.2.2  2015/07/26 06:02:22  tirawan
Add FPGA I2C read/write function and ACT2 cookie/smartchip programming

Revision 1.1.2.1  2015/07/24 03:39:36  tirawan
Add FPGA I2C read/write function, ACT2 cookie read/write function


$Endlog$
*/
