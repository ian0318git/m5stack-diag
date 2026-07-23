/*
 * test_rtc.c — BM8563 Real-Time Clock test
 *
 * Reads the RTC, validates time range, checks VL flag,
 * and verifies the clock is ticking (2-second elapsed test).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "hal_rtc.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

diag_result_t test_rtc(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "RTC", "MB/RTC");
    hal_rtc_time_t t1, t2;

    diag_result_t r = hal_rtc_init();
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x51 BM8563: init failed (no ACK)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check I2C bus 0x51 pull-ups",
                               "Check RTC battery backup voltage");
        }
        return r;
    }

    /* Read time #1 */
    r = hal_rtc_get_time(&t1);
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx, "I2C@0x51 BM8563: first read failed");
        hal_rtc_deinit();
        return r;
    }
    diag_menu_printf("RTC T1: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                     t1.year, t1.month, t1.day,
                     t1.hour, t1.minute, t1.second);

    /* Verify RTC is ticking: wait 2 seconds, read again */
    vTaskDelay(pdMS_TO_TICKS(2000));

    r = hal_rtc_get_time(&t2);
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx, "I2C@0x51 BM8563: second read failed");
        hal_rtc_deinit();
        return r;
    }
    diag_menu_printf("RTC T2: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                     t2.year, t2.month, t2.day,
                     t2.hour, t2.minute, t2.second);

    /* Elapsed seconds (handle minute rollover) */
    int elapsed = (int)t2.second - (int)t1.second;
    if (elapsed < 0) elapsed += 60;

    bool time_valid = (t1.year >= 2024 && t1.month >= 1 && t1.month <= 12 &&
                       t1.day >= 1 && t1.day <= 31);

    if (!time_valid) {
        diag_menu_printf("  ** VL flag was set — RTC time invalid, setting default...\r\n");

        hal_rtc_time_t def;
        memset(&def, 0, sizeof(def));
        const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
        char mstr[4] = { __DATE__[0], __DATE__[1], __DATE__[2], 0 };
        const char *p = strstr(months, mstr);
        def.month = p ? (int)((p - months) / 3 + 1) : 1;
        def.day   = (__DATE__[4] >= '0' && __DATE__[4] <= '9')
                        ? (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0')
                        : (__DATE__[5] - '0');
        def.year  = (__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100
                  + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');
        def.hour   = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
        def.minute = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
        def.second = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');

        diag_menu_printf("  Setting RTC to build time: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                         def.year, def.month, def.day,
                         def.hour, def.minute, def.second);

        if (hal_rtc_set_time(&def) != DIAG_PASSED) {
            if (g_diag_err_ctx)
                diag_err_add(g_diag_err_ctx, "I2C@0x51 BM8563: set default time failed");
            hal_rtc_deinit();
            return DIAG_FAILED;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
        hal_rtc_get_time(&t1);
        diag_menu_printf("RTC now: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                         t1.year, t1.month, t1.day,
                         t1.hour, t1.minute, t1.second);

        vTaskDelay(pdMS_TO_TICKS(2000));
        r = hal_rtc_get_time(&t2);
        if (r != DIAG_PASSED) {
            if (g_diag_err_ctx)
                diag_err_add(g_diag_err_ctx, "I2C@0x51 BM8563: read after set failed");
            hal_rtc_deinit();
            return DIAG_FAILED;
        }

        elapsed = (int)t2.second - (int)t1.second;
        if (elapsed < 0) elapsed += 60;

        if (elapsed >= 1 && elapsed <= 3) {
            diag_menu_printf("RTC tick: OK (%d s elapsed)\r\n", elapsed);
            r = DIAG_PASSED;
        } else {
            diag_menu_printf("  ** RTC not ticking after set (elapsed=%d s)\r\n", elapsed);
            if (g_diag_err_ctx) {
                diag_err_add(g_diag_err_ctx, "I2C@0x51 BM8563: not ticking after set");
                diag_err_set_debug(g_diag_err_ctx,
                                   "RTC oscillator stopped — check XTAL/battery",
                                   "Re-init control registers and retry");
            }
            r = DIAG_FAILED;
        }
    } else if (elapsed < 1 || elapsed > 3) {
        diag_menu_printf("  ** RTC not ticking (elapsed=%d s, expected ~2 s)\r\n", elapsed);
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x51 BM8563: not ticking (elapsed=%d s)", elapsed);
            diag_err_set_debug(g_diag_err_ctx,
                               "RTC oscillator stopped — check battery/XTAL",
                               "Re-init control registers and retry");
        }
        r = DIAG_FAILED;
    } else {
        diag_menu_printf("RTC tick: OK (%d s elapsed)\r\n", elapsed);
    }

    hal_rtc_deinit();
    return r;
}
