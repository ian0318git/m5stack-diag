/*
 * hal_power.c — CoreS3 board adapter for AXP2101 PMU
 *
 * Board-specific — bridges between the abstract chip driver and the
 * ESP-IDF I2C implementation through the transport seam.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_power.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "power_AXP2101.h"
#include "diag_config.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hal_power";
static i2c_master_dev_handle_t s_i2c_dev = NULL;
static bool s_initialised = false;

diag_result_t hal_power_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Create ESP-IDF device handle for the AXP2101 */
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &s_i2c_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    /* Init the chip driver through the abstract transport seam */
    if (power_AXP2101_init(&g_diag_i2c_adapter, (void *)s_i2c_dev) != 0) {
        ESP_LOGE(TAG, "AXP2101 init failed");
        return DIAG_FAILED;
    }

    s_initialised = true;
    return DIAG_PASSED;
}

void hal_power_deinit(void)
{
    if (!s_initialised) return;
    power_AXP2101_deinit();
    if (s_i2c_dev) {
        i2c_master_bus_rm_device(s_i2c_dev);
        s_i2c_dev = NULL;
    }
    s_initialised = false;
}

diag_result_t hal_power_read(hal_power_data_t *data)
{
    if (!data || !s_i2c_dev) return DIAG_FAILED;

    power_AXP2101_data_t raw;
    if (power_AXP2101_read(&raw) != 0) return DIAG_FAILED;

    data->battery_millivolts = raw.battery_millivolts;
    data->battery_percent    = raw.battery_percent;
    data->usb_millivolts     = raw.usb_millivolts;
    data->system_millivolts  = raw.system_millivolts;
    data->charge_current_ma  = raw.charge_current_ma;
    data->temperature_celsius = raw.temperature_celsius;

    data->flags = HAL_POWER_FLAG_NONE;
    if (raw.flags & AXP2101_FLAG_USB)          data->flags |= HAL_POWER_FLAG_USB;
    if (raw.flags & AXP2101_FLAG_BAT_CHARGING) data->flags |= HAL_POWER_FLAG_BAT_CHARGING;
    if (raw.flags & AXP2101_FLAG_BAT_FULL)     data->flags |= HAL_POWER_FLAG_BAT_FULL;

    return DIAG_PASSED;
}

uint8_t hal_power_battery_percent(void) { return power_AXP2101_battery_percent(); }
bool    hal_power_is_vbus_present(void)  { return power_AXP2101_is_vbus_present(); }
void    hal_power_shutdown(void)         { power_AXP2101_shutdown(); }
uint8_t hal_power_chip_version(void)     { return power_AXP2101_chip_version(); }
