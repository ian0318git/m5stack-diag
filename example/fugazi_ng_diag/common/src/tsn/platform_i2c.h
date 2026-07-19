/* $Id: platform_i2c.h,v 1.5 2018/05/24 09:47:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_i2c.h,v $
 *------------------------------------------------------------------
 * Filename: platform_i2c.h
 *
 * Description: Platform specific I2C header file.
 *
 * Copyright (c) 2013-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __I2C_ADDR_H__
#define __I2C_ADDR_H__

/* TSN Intel side I2C Bus number enumeration */
typedef enum {
    CPU_I2C0 = 0,
    CPU_I2C1,
    CPU_I2C2,
    CPU_I2C3,
    PLUG_FPGA,         /* I2C bus on the Pluggable FPGA */
    IOFPGA_I2C = PLUG_FPGA,
    I2C_BUS_INVALID,            /* Invalid I2C bus */
} I2C_BUS;

/* CPU I2C (CPU_I2C2 bus) */
typedef enum {
    MB_I2C_RTC = 0,             /* RTC DS1337S+ */
    MB_I2C_POE_CTRL,            /* PoE TPS23861PW */
    MB_I2C_POE_EEPROM,          /* PoE EEPROM */
    MB_I2C_MCU,                 /* MCU */
    MB_I2C_MCU_BOOTLOADER,      /* MCU bootloader */
    MB_I2C_WIFI_ACT2,           /* Wifi ACT2 */
    MB_I2C_WIFI_TEMP,           /* Wifi Temp */
    MB_I2C_WIFI_STAR_TEMP,      /* Wifi Temp */
    MB_I2C_2_INVALID,           /* Invalid I2C */
} MB_I2C2_DEVICE;

/* CPU I2C Master (CPU_I2C1 bus) */
typedef enum {
    MB_I2C_MB_TEMP = 0,         /* Mother Board Temperature Sensor */
    MB_I2C_SFP,                 /* SFP port */
    MB_I2C_SFP_INT_REG,         /* SFP Internal Reg port */
    MB_I2C_1_INVALID,           /* Invalid I2C */
} MB_I2C1_DEVICE;

/* CPU I2C (CPU_I2C0 bus) */
typedef enum {
    MB_I2C_EEPROM = 0,      /* EEPROM SPD */
    MB_I2C_ACT2,            /* Secure Chip */
    MB_I2C_AIKIDO_ACT2,     /* Aikido Secure Chip */
    MB_I2C_0_INVALID,   /* Invalid I2C */
} MB_I2C0_DEVICE;

/* PLUG FPGA I2C Master */
typedef enum {
    PLUG_FPGA_I2C_0_TEMP = 0,   /* Pluggable Temperature Sensor */
    PLUG_FPGA_I2C_0_ACT2,   /* Pluggable ACT2 */
    PLUG_FPGA_I2C_0_GPIO_EXP,   /* Pluggable GPIO Expander */
    PLUG_FPGA_INVALID, /* Invalid I2C */
} PLUG_FPGA_I2C0_DEVICE;

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO        0
#define I2C_CTRL_ONE         1
#define I2C_CTRL_TWO         2
#define I2C_CTRL_THREE       3

/* Common definition for MUX */
#define I2C_MUX_ZERO         0
#define I2C_MUX_ONE          1
#define I2C_MUX_TWO          2
#define I2C_MUX_THREE        3

#define I2CBUS0      "/dev/i2c-0"
#define I2CBUS1      "/dev/i2c-1"
#define I2CBUS2      "/dev/i2c-2"

#define    MAX_RETRY            1
#define    WIFI_ACT2_MAX_RETRY  300 /* 9 secs */ 

#define HD_SIZE_2            2
/* Externs */
extern int tsn_x64_i2c_scan_test(int);
extern int tsn_i2c_reg_rw_test(int);
extern void *get_n2g_i2c_if(uint8_t, uint8_t, uint8_t);
extern void *platform_i2c_get_quack(uint8_t, uint8_t);
extern void build_i2c_menu(void);
extern boolean g_i2c_read_cterr;
#endif                          /* __I2C_ADDR_H__ */

/*------------------------------------------------------------------
$Log: platform_i2c.h,v $
Revision 1.5  2018/05/24 09:47:10  steja
CSCvj57981-Enhance SFP GLC-GE-100FX Support

Revision 1.4  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3.16.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.3  2017/09/06 12:14:20  steja
1.Fix TSN WIFI ACT2 i2c scan test failed at first time after power on (CSCvf83218)
2. Remove Discrete ACT2 utility and I2C Scan for Discrete ACT2 only for Development phase(CSCvf81035)

Revision 1.2.4.1  2017/08/15 14:18:39  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.5.6.4  2017/07/31 10:49:59  lucywang
add pluggable serial code of host and module

Revision 1.1.4.5.6.3  2017/07/10 06:56:45  hondwang
add mcu upgrade function

Revision 1.1.4.5.6.2  2017/06/30 13:37:55  hondwang
Fix Star platform I2c scan issue and add this_is_star function

Revision 1.1.4.5.6.1  2017/06/13 06:54:14  shjung
Add pluggable FPGA I2C read/write function

Revision 1.1.4.5.2.2  2017/07/20 11:29:02  steja
Code cleanup

Revision 1.1.4.5.2.1  2017/07/11 13:46:07  steja
1. Add Check Motherboard Aikido cookie ID
2. Add Check Aikido I2C Scan
2. Add Check SFP present I2C Scan

Revision 1.1.4.5  2016/09/13 14:53:52  steja
Fix hardcoded

Revision 1.1.4.4  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.3  2016/07/14 12:57:00  steja
Add POE cookie eeprom programming

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.8  2016/06/21 04:36:33  palin2
Added voltage margin utility and MCU register R/W utilities.

Revision 1.1.2.7  2016/06/17 10:37:40  steja
Fix I2C scan

Revision 1.1.2.6  2016/05/24 01:18:11  palin2
Updated Thermal sensor and ACT2 chip I2C bus number based on P1A HW changes

Revision 1.1.2.5  2016/05/09 08:06:55  steja
Fixed POE i2c address R/W

Revision 1.1.2.4  2016/05/06 16:10:18  steja
Bring up I2C-2 for RTC

Revision 1.1.2.3  2016/04/11 14:12:27  steja
Update code i2c utility for bringup

Revision 1.1.2.2  2016/03/20 05:32:40  steja
1. Add i2c scan and i2c read write utility
2. Add sensor temperature and eeprom utility

Revision 1.1.2.1  2016/03/08 09:55:11  steja
Initial Check-in


$Endlog$
*/
