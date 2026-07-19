/*
 * diag_config.h - System-wide configuration and pin definitions
 *
 * APPLICATION layer — centralises all board-specific constants so
 * other layers never reference magic numbers.
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
/* Console (UART) Configuration                                              */
/*===========================================================================*/

#define CONFIG_UART_NUM           1           /* UART1 for console          */
#define CONFIG_UART_TX_PIN        GPIO_NUM_43 /* CoreS3 USB-TTL TX         */
#define CONFIG_UART_RX_PIN        GPIO_NUM_44 /* CoreS3 USB-TTL RX         */
#define CONFIG_UART_BAUDRATE      115200
#define CONFIG_UART_BUF_SIZE      4096
#define CONFIG_UART_RX_TIMEOUT_MS 10

/*===========================================================================*/
/* I2C Bus Configuration                                                     */
/*===========================================================================*/

/*
 * CoreS3 uses a single I2C bus (I2C_NUM_0) shared by all on-board
 * peripheral ICs.
 */

#define CONFIG_I2C_NUM            0
#define CONFIG_I2C_SDA_PIN        GPIO_NUM_12
#define CONFIG_I2C_SCL_PIN        GPIO_NUM_11
#define CONFIG_I2C_CLOCK_HZ       400000       /* 400 kHz fast-mode         */
#define CONFIG_I2C_TIMEOUT_MS     50

/* I2C device addresses (7-bit, left-justified) */
#define CONFIG_I2C_ADDR_TOUCH     0x38         /* FT6336                    */
#define CONFIG_I2C_ADDR_RTC       0x51         /* BM8563                    */
#define CONFIG_I2C_ADDR_IMU       0x69         /* BMI270 (ALT=0x68)         */
#define CONFIG_I2C_ADDR_POWER     0x34         /* AXP2101                   */

/*===========================================================================*/
/* LCD / Screen Configuration (GC9A01, SPI)                                  */
/*===========================================================================*/

#define CONFIG_LCD_SPI_NUM        2            /* SPI2 (aka HSPI)           */
#define CONFIG_LCD_MOSI_PIN       GPIO_NUM_14
#define CONFIG_LCD_MISO_PIN       GPIO_NUM_NC  /* GC9A01 has no MISO       */
#define CONFIG_LCD_SCLK_PIN       GPIO_NUM_21
#define CONFIG_LCD_CS_PIN         GPIO_NUM_15
#define CONFIG_LCD_DC_PIN         GPIO_NUM_7   /* Data / Command            */
#define CONFIG_LCD_RST_PIN        GPIO_NUM_5
#define CONFIG_LCD_BL_PIN         GPIO_NUM_20

#define CONFIG_LCD_WIDTH          240
#define CONFIG_LCD_HEIGHT         240
#define CONFIG_LCD_SPI_CLOCK_HZ   80000000     /* 80 MHz                    */

/*===========================================================================*/
/* Touch Configuration (FT6336)                                              */
/*===========================================================================*/

#define CONFIG_TOUCH_INT_PIN      GPIO_NUM_3
#define CONFIG_TOUCH_RST_PIN      GPIO_NUM_1
#define CONFIG_TOUCH_MAX_POINTS   2

/*===========================================================================*/
/* SD Card Configuration (SPI mode)                                          */
/*===========================================================================*/

#define CONFIG_SD_SPI_NUM         2            /* Shares SPI bus with LCD   */
#define CONFIG_SD_CS_PIN          GPIO_NUM_4
#define CONFIG_SD_MISO_PIN        GPIO_NUM_13
#define CONFIG_SD_SPI_CLOCK_HZ    20000000     /* 20 MHz                    */

/*===========================================================================*/
/* Default timeouts                                                          */
/*===========================================================================*/

#define CONFIG_DEFAULT_TEST_TIMEOUT_MS  30000  /* 30 s                      */
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
