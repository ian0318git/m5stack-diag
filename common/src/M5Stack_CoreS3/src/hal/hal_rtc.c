/*
 * hal_rtc.c — CoreS3 board adapter for BM8563 RTC
 *
 * Board-specific — bridges between the abstract chip driver and the
 * ESP-IDF I2C implementation through the transport seam.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_rtc.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "rtc_BM8563.h"
#include "diag_config.h"
#include "esp_log.h"

static const char *TAG = "hal_rtc";
static i2c_master_dev_handle_t s_i2c_dev = NULL;
static bool s_initialised = false;

diag_result_t hal_rtc_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    if (hal_i2c_add_device(CONFIG_I2C_ADDR_RTC, 400000, &s_i2c_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (rtc_BM8563_init(&g_diag_i2c_adapter, (void *)s_i2c_dev) != 0) {
        return DIAG_FAILED;
    }

    s_initialised = true;
    return DIAG_PASSED;
}

void hal_rtc_deinit(void)
{
    if (!s_initialised) return;
    rtc_BM8563_deinit();
    if (s_i2c_dev) {
        i2c_master_bus_rm_device(s_i2c_dev);
        s_i2c_dev = NULL;
    }
    s_initialised = false;
}

diag_result_t hal_rtc_get_time(hal_rtc_time_t *t)
{
    if (!t || !s_i2c_dev) return DIAG_FAILED;

    rtc_BM8563_time_t raw;
    if (rtc_BM8563_get_time(&raw) != 0) return DIAG_FAILED;

    t->year    = raw.year;
    t->month   = raw.month;
    t->day     = raw.day;
    t->hour    = raw.hour;
    t->minute  = raw.minute;
    t->second  = raw.second;
    t->weekday = raw.weekday;
    return DIAG_PASSED;
}

diag_result_t hal_rtc_set_time(const hal_rtc_time_t *t)
{
    if (!t || !s_i2c_dev) return DIAG_FAILED;

    rtc_BM8563_time_t raw = {
        .year    = t->year,
        .month   = t->month,
        .day     = t->day,
        .hour    = t->hour,
        .minute  = t->minute,
        .second  = t->second,
        .weekday = t->weekday,
    };
    return (rtc_BM8563_set_time(&raw) == 0) ? DIAG_PASSED : DIAG_FAILED;
}

char *hal_rtc_format(const hal_rtc_time_t *t, char *buf, size_t len)
{
    if (!t || !buf || len < 20) return buf;
    rtc_BM8563_time_t raw = {
        .year = t->year, .month = t->month, .day = t->day,
        .hour = t->hour, .minute = t->minute, .second = t->second,
    };
    rtc_BM8563_format(&raw, buf, len);
    return buf;
}
