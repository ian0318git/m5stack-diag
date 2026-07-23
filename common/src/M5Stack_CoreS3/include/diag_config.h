/*
 * diag_config.h - System-wide configuration and pin definitions
 *
 * APPLICATION layer — centralises all board-specific constants so
 * other layers never reference magic numbers.
 *
 * Pin mapping verified against M5Stack CoreS3 official docs:
 *   https://docs.m5stack.com/zh_CN/core/CoreS3
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Console (USB Serial/JTAG) Configuration                                   */
/*===========================================================================*/

/*
 * CoreS3 connects to the host via the ESP32-S3 built-in USB Serial/JTAG
 * controller.  The port appears as /dev/ttyACM0 on Linux.
 */

#define CONFIG_CONSOLE_TX_TIMEOUT_MS   50
#define CONFIG_CONSOLE_RX_TIMEOUT_MS   10
#define CONFIG_CONSOLE_BUF_SIZE        4096

/*===========================================================================*/
/* I2C Bus Configuration                                                     */
/*===========================================================================*/

/*
 * CoreS3 uses a single internal I2C bus shared by all on-board ICs.
 */

#define CONFIG_I2C_NUM            0
#define CONFIG_I2C_SDA_PIN        GPIO_NUM_12
#define CONFIG_I2C_SCL_PIN        GPIO_NUM_11
#define CONFIG_I2C_CLOCK_HZ       400000       /* 400 kHz fast-mode         */
#define CONFIG_I2C_TIMEOUT_MS     50

/* I2C device addresses (7-bit) */
#define CONFIG_I2C_ADDR_TOUCH     0x38         /* FT6336U touch             */
#define CONFIG_I2C_ADDR_RTC       0x51         /* BM8563 RTC                */
#define CONFIG_I2C_ADDR_IMU       0x69         /* BMI270 IMU                */
#define CONFIG_I2C_ADDR_POWER     0x34         /* AXP2101 PMU               */
#define CONFIG_I2C_ADDR_AUDIO_ADC 0x40         /* ES7210 audio ADC          */
#define CONFIG_I2C_ADDR_SPK_AMP   0x36         /* AW88298 speaker amp       */
#define CONFIG_I2C_ADDR_GPIO_EXP  0x58         /* AW9523B GPIO expander     */
#define CONFIG_I2C_ADDR_CAMERA    0x21         /* GC0308 camera             */
#define CONFIG_I2C_ADDR_PROXIMITY 0x23         /* LTR-553ALS-WA proximity   */

/*===========================================================================*/
/* AW9523B GPIO Expander — Pin Assignments                                   */
/*===========================================================================*/

/*
 * AW9523B is an I2C GPIO expander (0x58) that controls several
 * peripheral reset and interrupt lines on the CoreS3.
 */

#define AW9523B_PIN_TOUCH_RST     0           /* P0_0: FT6336U RST         */
#define AW9523B_PIN_SPK_RST       2           /* P0_2: AW88298 RST         */
#define AW9523B_PIN_CAM_RST       8           /* P1_0: GC0308 RST          */
#define AW9523B_PIN_LCD_RST       9           /* P1_1: ILI9342C RST        */
#define AW9523B_PIN_TOUCH_INT     10          /* P1_2: FT6336U INT         */
#define AW9523B_PIN_SPK_INT       11          /* P1_3: AW88298 INT         */

/*===========================================================================*/
/* LCD / Screen Configuration (ILI9342C, SPI)                                */
/*===========================================================================*/

/*
 * CoreS3 uses an ILI9342C 320×240 IPS LCD driven over SPI.
 * RST is controlled via AW9523B P1_1 (not a direct GPIO).
 * Backlight is controlled via AXP2101 DLDO1 (not a GPIO).
 *
 * SPI bus is shared with the microSD card slot.
 */

#define CONFIG_LCD_SPI_NUM        2
#define CONFIG_LCD_MOSI_PIN       GPIO_NUM_37
#define CONFIG_LCD_MISO_PIN       GPIO_NUM_35
#define CONFIG_LCD_SCLK_PIN       GPIO_NUM_36
#define CONFIG_LCD_CS_PIN         GPIO_NUM_3
#define CONFIG_LCD_DC_PIN         GPIO_NUM_35   /* Data / Command            */

#define CONFIG_LCD_WIDTH          320
#define CONFIG_LCD_HEIGHT         240
#define CONFIG_LCD_SPI_CLOCK_HZ   80000000      /* 80 MHz                    */

/*===========================================================================*/
/* Touch Configuration (FT6336U)                                             */
/*===========================================================================*/

/*
 * FT6336U RST and INT are controlled via AW9523B GPIO expander,
 * NOT through direct GPIO pins.
 */

#define CONFIG_TOUCH_MAX_POINTS   2

/*===========================================================================*/
/* SD Card Configuration (SPI mode)                                          */
/*===========================================================================*/

/*
 * Shares SPI bus with LCD (CONFIG_LCD_SPI_NUM).
 */

#define CONFIG_SD_CS_PIN          GPIO_NUM_4
#define CONFIG_SD_SPI_CLOCK_HZ    20000000      /* 20 MHz                    */

/*===========================================================================*/
/* Audio Subsystem                                                           */
/*===========================================================================*/

#define CONFIG_I2S_NUM            0
#define CONFIG_I2S_BCK_PIN        GPIO_NUM_34
#define CONFIG_I2S_WCK_PIN        GPIO_NUM_33
#define CONFIG_I2S_DATA_IN_PIN    GPIO_NUM_13   /* ES7210 ADC data           */
#define CONFIG_I2S_DATA_OUT_PIN   GPIO_NUM_14   /* AW88298 amp data          */
#define CONFIG_I2S_MCLK_PIN       GPIO_NUM_0

/*===========================================================================*/
/* External I/O Ports (HY2.0-4P)                                             */
/*===========================================================================*/

#define CONFIG_PORT_A_SDA_PIN     GPIO_NUM_2
#define CONFIG_PORT_A_SCL_PIN     GPIO_NUM_1
#define CONFIG_PORT_B_PIN         GPIO_NUM_9    /* PORT.B yellow wire        */
#define CONFIG_PORT_B_PIN_ALT     GPIO_NUM_8    /* PORT.B white wire         */
#define CONFIG_PORT_C_PIN         GPIO_NUM_17   /* PORT.C yellow wire        */
#define CONFIG_PORT_C_PIN_ALT     GPIO_NUM_18   /* PORT.C white wire         */

/*===========================================================================*/
/* Button Configuration                                                      */
/*===========================================================================*/

#define CONFIG_BUTTON_PWR_PIN     GPIO_NUM_41   /* Side power button         */
#define CONFIG_BUTTON_BOOT_PIN    GPIO_NUM_0    /* Boot button (hold=flash)  */

/*===========================================================================*/
/* Default timeouts                                                          */
/*===========================================================================*/

#define CONFIG_DEFAULT_TEST_TIMEOUT_MS  30000
#define CONFIG_I2C_SCAN_TIMEOUT_MS      5000
#define CONFIG_LCD_TEST_TIMEOUT_MS      15000
#define CONFIG_TOUCH_TEST_TIMEOUT_MS    30000
#define CONFIG_RTC_TEST_TIMEOUT_MS      5000
#define CONFIG_IMU_TEST_TIMEOUT_MS      5000
#define CONFIG_POWER_TEST_TIMEOUT_MS    5000

/*===========================================================================*/
/* Menu configuration                                                        */
/*===========================================================================*/

#define CONFIG_MENU_PROMPT        "diag> "
#define CONFIG_MENU_MAX_ARGS      12
#define CONFIG_MENU_LINE_BUF_SIZE 128

#ifdef __cplusplus
}
#endif
