/*
 * test_touch.c — FT6336U Touch Controller test
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "hal_touch.h"

diag_result_t test_touch(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "TOUCH", "MB/TOUCH");

    diag_result_t r = hal_touch_init();
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x38 FT6336: init failed (no ACK)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check AXP2101 LDOIO0 touch power (reg 0x90)",
                               "Check I2C bus 0x38 pull-ups and INT/RST pins");
        }
        return r;
    }

    uint8_t fw = hal_touch_firmware_version();
    if (fw == 0) {
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x38 FT6336: firmware version read returned 0");
    }
    diag_menu_printf("Touch: FT6336 fw=0x%02X max_points=%d\r\n",
                     fw, hal_touch_max_points());

    hal_touch_data_t data;
    r = hal_touch_read(&data);
    if (r == DIAG_PASSED) {
        diag_menu_printf("Touch points: %d\r\n", data.point_count);
        for (uint8_t i = 0; i < data.point_count; i++) {
            diag_menu_printf("  Point %d: (%u, %u) event=%u id=%u\r\n",
                             i, data.points[i].x, data.points[i].y,
                             data.points[i].event, data.points[i].id);
        }
    } else {
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x38 FT6336: read touch data failed");
    }

    hal_touch_deinit();
    return (fw > 0 && r == DIAG_PASSED) ? DIAG_PASSED : DIAG_FAILED;
}
