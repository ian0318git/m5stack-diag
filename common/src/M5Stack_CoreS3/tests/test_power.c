/*
 * test_power.c — AXP2101 PMU Power Management test
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "hal_power.h"

diag_result_t test_power(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "POWER", "MB/PMU");

    diag_result_t r = hal_power_init();
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x34 AXP2101: init failed (no ACK)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check I2C bus 0x34 pull-ups",
                               "Check battery connection and PMU power rails");
        }
        return r;
    }

    hal_power_data_t pwr;
    r = hal_power_read(&pwr);
    if (r == DIAG_PASSED) {
        diag_menu_printf("Power: version=0x%02X\r\n",
                         hal_power_chip_version());
        diag_menu_printf("  Battery: %u mV (%u%%)\r\n",
                         pwr.battery_millivolts, pwr.battery_percent);
        diag_menu_printf("  USB:     %u mV %s\r\n",
                         pwr.usb_millivolts,
                         (pwr.flags & HAL_POWER_FLAG_USB) ? "connected" : "disconnected");
        diag_menu_printf("  Charge:  %u mA %s\r\n",
                         pwr.charge_current_ma,
                         (pwr.flags & HAL_POWER_FLAG_BAT_CHARGING) ? "charging" :
                         (pwr.flags & HAL_POWER_FLAG_BAT_FULL) ? "full" : "idle");
        diag_menu_printf("  Temp:    %u C\r\n", pwr.temperature_celsius);
    } else {
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x34 AXP2101: read PMU data failed");
    }

    hal_power_deinit();
    return r;
}
