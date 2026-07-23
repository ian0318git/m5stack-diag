/*
 * rtc_BM8563.c — BM8563 Real-Time Clock (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "rtc_BM8563.h"
#include <string.h>
#include <stdio.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;

/*===========================================================================*/
/* I2C helpers (use abstract transport)                                      */
/*===========================================================================*/

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return s_i2c->write(s_bus, BM8563_ADDR, buf, 2);
}

static int read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    return s_i2c->write_then_read(s_bus, BM8563_ADDR, &reg, 1, buf, len);
}

/*===========================================================================*/
/* BCD helpers                                                               */
/*===========================================================================*/

static inline uint8_t bcd2bin(uint8_t b) { return (b & 0x0F) + ((b >> 4) * 10); }
static inline uint8_t bin2bcd(uint8_t b) { return ((b / 10) << 4) | (b % 10); }

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int rtc_BM8563_init(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t sec = 0;
    if (read_regs(BM8563_REG_SEC, &sec, 1) != 0) {
        return -1;
    }

    /* Clear VL (validity) flag if set */
    if (sec & 0x80) {
        write_reg(BM8563_REG_SEC, sec & 0x7F);
    }

    return 0;
}

void rtc_BM8563_deinit(void)
{
    s_i2c = NULL;
    s_bus = NULL;
}

int rtc_BM8563_get_time(rtc_BM8563_time_t *t)
{
    if (!t || !s_i2c || !s_bus) return -1;

    uint8_t buf[7];
    if (read_regs(BM8563_REG_SEC, buf, 7) != 0) return -1;

    t->second  = bcd2bin(buf[0] & 0x7F);
    t->minute  = bcd2bin(buf[1] & 0x7F);
    t->hour    = bcd2bin(buf[2] & 0x3F);
    t->day     = bcd2bin(buf[3] & 0x3F);
    t->weekday = buf[4] & 0x07;
    t->month   = bcd2bin(buf[5] & 0x1F);
    t->year    = bcd2bin(buf[6]) + 2000;
    return 0;
}

int rtc_BM8563_set_time(const rtc_BM8563_time_t *t)
{
    if (!t || !s_i2c || !s_bus) return -1;

    uint8_t data[8] = {
        BM8563_REG_SEC,
        bin2bcd(t->second),
        bin2bcd(t->minute),
        bin2bcd(t->hour),
        bin2bcd(t->day),
        t->weekday & 0x07,
        bin2bcd(t->month),
        bin2bcd(t->year - 2000),
    };
    return s_i2c->write(s_bus, BM8563_ADDR, data, 8);
}

void rtc_BM8563_format(const rtc_BM8563_time_t *t, char *buf, size_t len)
{
    if (!t || !buf || len < 20) return;
    snprintf(buf, len, "%04u-%02u-%02u %02u:%02u:%02u",
             (unsigned)t->year, (unsigned)t->month, (unsigned)t->day,
             (unsigned)t->hour, (unsigned)t->minute, (unsigned)t->second);
}
