/*
 * hal_touch.c — CoreS3 board adapter for FT6336 touch controller
 *
 * Board-specific: initialises I2C device on the CoreS3 bus,
 * then delegates to the common FT6336 chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_touch.h"
#include "hal_i2c_helpers.h"
#include "touch_FT6336.h"
#include "diag_config.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "hal_touch";

static i2c_master_dev_handle_t s_i2c_dev = NULL;
static bool s_initialised = false;

diag_result_t hal_touch_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Reset the chip */
    gpio_set_direction(CONFIG_TOUCH_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_TOUCH_RST_PIN, 0);
    esp_rom_delay_us(5000);
    gpio_set_level(CONFIG_TOUCH_RST_PIN, 1);
    esp_rom_delay_us(5000);

    /* INT as input */
    gpio_set_direction(CONFIG_TOUCH_INT_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_TOUCH_INT_PIN, GPIO_PULLUP_ONLY);

    /* Add I2C device */
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_TOUCH, 400000, &s_i2c_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (touch_FT6336_init(s_i2c_dev) != 0) {
        return DIAG_FAILED;
    }

    s_initialised = true;
    return DIAG_PASSED;
}

void hal_touch_deinit(void)
{
    if (!s_initialised) return;
    touch_FT6336_deinit();
    s_initialised = false;
    s_i2c_dev = NULL;
}

diag_result_t hal_touch_read(hal_touch_data_t *data)
{
    if (!data || !s_i2c_dev) return DIAG_FAILED;

    touch_FT6336_data_t raw;
    if (touch_FT6336_read(&raw) != 0) return DIAG_FAILED;

    data->point_count = raw.point_count;
    for (int i = 0; i < (int)raw.point_count && i < 2; i++) {
        data->points[i].x     = raw.points[i].x;
        data->points[i].y     = raw.points[i].y;
        data->points[i].id    = raw.points[i].id;
        data->points[i].event = raw.points[i].event;
    }
    return DIAG_PASSED;
}

uint8_t hal_touch_firmware_version(void)
{
    return touch_FT6336_firmware_version();
}

int hal_touch_max_points(void)
{
    return CONFIG_TOUCH_MAX_POINTS;
}
