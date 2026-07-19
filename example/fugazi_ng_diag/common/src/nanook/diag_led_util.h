 /* $Id: diag_led_util.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_led_util.h,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.h - This file is LED utility header 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_LED_UTIL_H__
#define __DIAG_LED_UTIL_H__


extern int diag_sys_led_util(void);
extern int diag_async_led_util (void);
extern int diag_88E1543_RJ45_led_util(void);
extern int diag_88E1543_SFP_led_util (void);
extern int diag_gephy_led_util(int);
extern int diag_all_async_leds_on (void);
extern int diag_all_async_leds_off (void);
extern int diag_all_green_leds_on (void);
extern int diag_all_yellow_leds_on (void);
extern int diag_all_leds_off (void);


//Temp define
#define FPGA_PWR_SUPPLY_LED          0x00408
#define FPGA_ENV_LED                 0x00418
#define FPGA_I350_RJ45_LED           0x0042C
#define FPGA_I350_SFP_LED            0x00430
#define FPGA_WATCHDOG_REG            0x80800
///* FPGA LED Reg */
#define LED_OFF                     (0 << 0)
#define ENV_LED_YELLOW               0x14000
#define ENV_LED_GREEN                0x28000
#define PWR_SUP_LED_YELLOW           0x5
#define PWR_SUP_LED_GREEN            0xA
#define RJ45_LED_YELLOW              0x44
#define RJ45_LED_GREEN               0x8888
#define SFP_LED_YELLOW               0x4444
#define SFP_LED_GREEN                0x8888
#define DELAY_FOR_LED_TEST      1500

#define FPGA_LED_DBG_REG                 0x00438
#define FPGA_LED_DBG_GREEN                 0xA0
#define FPGA_LED_DBG_AMBER                0xB0
#define FPGA_LED_DBG_CYCLE                 0xC0
#define FPGA_LED_DBG_DEFAULT            0x0

#define FPGA_LED_CROCUS_FPGA_CTRL_REG    0x0043C
#define FPGA_LED_CROCUS_FPGA_LED_GREEN    0xF3
#define FPGA_LED_CROCUS_FPGA_LED_OFF    0x0
#define CROCUS_32_FPGA_LED3    (1 << 7)
#define CROCUS_32_FPGA_LED2    (1 << 6)
#define CROCUS_32_FPGA_LED1    (1 << 5)
#define CROCUS_32_FPGA_LED0    (1 << 4)
#define CROCUS_16_FPGA_LED1    (1 << 1)
#define CROCUS_16_FPGA_LED0    (1 << 0)

#define FPGA_LED_BLINK_EN_REG                 0x00424
#define SFP_PORT1_BLINK_CTRL    (0x11 << 10)
#define SFP_PORT0_BLINK_CTRL    (0x11 << 8)
#define RJ45_PORT1_BLINK_CTRL    (0x11 << 2)
#define RJ45_PORT0_BLINK_CTRL    (0x11 << 0)
#define BLINK_OFF_PATTERN        0x00
#define BLINK_ONCE_PATTERN      0x01
#define BLINK_TWICE_PATTERN    0x10
#define BLINK_THREE_PATTERN    0x11

#define FPGA_LED_RJ45_ONOFF_REG             0x0042C
#define FPGA_RJ45_PORT1_GREEN_LINK_LED    (1 << 7)
#define FPGA_RJ45_PORT1_YELLOW_LINK_LED    (1 << 6)
#define FPGA_RJ45_PORT1_SPEED_LED    (1 << 4)
#define FPGA_RJ45_PORT0_GREEN_LINK_LED    (1 << 3)
#define FPGA_RJ45_PORT0_YELLOW_LINK_LED    (1 << 2)
#define FPGA_RJ45_PORT0_SPEED_LED    (1 << 0)

#define FPGA_LED_SFP_ONOFF_REG             0x00430
#define FPGA_SFP_PORT1_GREEN_LINK_LED    (1 << 7)
#define FPGA_SFP_PORT1_YELLOW_LINK_LED    (1 << 6)
#define FPGA_SFP_PORT1_SPEED_LED    (1 << 4)
#define FPGA_SFP_PORT0_GREEN_LINK_LED    (1 << 3)
#define FPGA_SFP_PORT0_YELLOW_LINK_LED    (1 << 2)
#define FPGA_SFP_PORT0_SPEED_LED    (1 << 0)




#endif                          /* __DIAG_LED_TEST_H__ */
/*-------------------------------------------------
 * $Log: diag_led_util.h,v $
 * Revision 1.2  2019/12/11 10:10:30  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
