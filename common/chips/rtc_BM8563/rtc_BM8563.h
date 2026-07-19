/*
 * rtc_BM8563.h — BM8563 Real-Time Clock (I2C)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Map                                                              */
/*===========================================================================*/

#define BM8563_REG_CTRL1      0x00
#define BM8563_REG_CTRL2      0x01
#define BM8563_REG_SEC        0x02
#define BM8563_REG_MIN        0x03
#define BM8563_REG_HOUR       0x04
#define BM8563_REG_DAY        0x05
#define BM8563_REG_WEEKDAY    0x06
#define BM8563_REG_MONTH      0x07
#define BM8563_REG_YEAR       0x08

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  weekday;
} rtc_BM8563_time_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int  rtc_BM8563_init(i2c_master_dev_handle_t dev);
void rtc_BM8563_deinit(void);

/*===========================================================================*/
/* Time access                                                               */
/*===========================================================================*/

int  rtc_BM8563_get_time(rtc_BM8563_time_t *t);
int  rtc_BM8563_set_time(const rtc_BM8563_time_t *t);
void rtc_BM8563_format(const rtc_BM8563_time_t *t, char *buf, size_t len);

#ifdef __cplusplus
}
#endif
