/*
 * hal_rtc.h - Hardware Abstraction Layer: BM8563 Real-Time Clock
 *
 * Interface for RTC operations.  Keeps time even in deep-sleep.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/** Broken-down time structure (BCD-friendly layout). */
typedef struct {
    uint16_t year;       /* 4-digit year (e.g. 2025)                  */
    uint8_t  month;      /* 1..12                                     */
    uint8_t  day;        /* 1..31                                     */
    uint8_t  hour;       /* 0..23                                     */
    uint8_t  minute;     /* 0..59                                     */
    uint8_t  second;     /* 0..59                                     */
    uint8_t  weekday;    /* 0=Sunday .. 6=Saturday                    */
} hal_rtc_time_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the RTC over I2C.
 * @return DIAG_PASSED on success, DIAG_FAILED if chip not responding.
 */
diag_result_t hal_rtc_init(void);

/**
 * @brief Release RTC resources.
 */
void hal_rtc_deinit(void);

/*===========================================================================*/
/* Time access                                                               */
/*===========================================================================*/

/**
 * @brief Get current time from the RTC.
 * @param[out] t  Filled with current time.
 * @return DIAG_PASSED on success, DIAG_FAILED on I2C error.
 */
diag_result_t hal_rtc_get_time(hal_rtc_time_t *t);

/**
 * @brief Set the RTC time.
 * @param t  Time to set.
 * @return DIAG_PASSED on success, DIAG_FAILED on I2C error.
 */
diag_result_t hal_rtc_set_time(const hal_rtc_time_t *t);

/*===========================================================================*/
/* Utility                                                                   */
/*===========================================================================*/

/**
 * @brief Format the RTC time into a string buffer.
 * @param t   Time to format.
 * @param buf Output buffer (must be >= 20 bytes).
 * @param len Size of buf.
 * @return Pointer to buf (for convenience).
 */
char *hal_rtc_format(const hal_rtc_time_t *t, char *buf, size_t len);

#ifdef __cplusplus
}
#endif
