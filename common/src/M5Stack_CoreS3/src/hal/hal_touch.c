/*
 * hal_touch.c — CoreS3 board adapter for FT6336U touch controller
 *
 * Board-specific:
 *   - Touch power via AXP2101 LDOIO0 (register 0x90)
 *   - TOUCH_RST via AW9523B GPIO expander P0_0 (NOT a direct GPIO)
 *   - TOUCH_INT via AW9523B GPIO expander P1_2 (NOT a direct GPIO)
 *
 * Verified against: https://docs.m5stack.com/zh_CN/core/CoreS3
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_touch.h"
#include "hal_i2c_helpers.h"
#include "hal_power.h"
#include "touch_FT6336.h"
#include "aw9523b.h"
#include "diag_config.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hal_touch";

static i2c_master_dev_handle_t s_i2c_dev = NULL;
static i2c_master_dev_handle_t s_aw9523b_dev = NULL;
static bool s_initialised = false;

/*
 * CoreS3 touch power (FT6336U VCC) is supplied by AXP2101 LDOIO0.
 * The PMU must be initialised before the FT6336 will respond on I2C.
 */

#define AXP2101_REG_GPIO0_LDO     0x90
#define AXP2101_GPIO0_3V3         0x07

static diag_result_t touch_power_on(void)
{
    if (hal_power_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "AXP2101 init failed");
        return DIAG_FAILED;
    }

    i2c_master_dev_handle_t pmu = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &pmu)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    uint8_t cmd[2] = { AXP2101_REG_GPIO0_LDO, AXP2101_GPIO0_3V3 };
    esp_err_t err = i2c_master_transmit(pmu, cmd, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 LDO write failed: %d", err);
        return DIAG_FAILED;
    }

    ESP_LOGI(TAG, "Touch power enabled (AXP2101 LDOIO0 = 3.3V)");
    return DIAG_PASSED;
}

/*
 * AW9523B GPIO expander initialisation for touch control lines.
 * Called once; subsequent calls are no-ops.
 */
static diag_result_t gpio_exp_init(void)
{
    if (s_aw9523b_dev) return DIAG_PASSED;

    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &s_aw9523b_dev)
        != DIAG_PASSED) {
        ESP_LOGE(TAG, "AW9523B I2C add failed");
        return DIAG_FAILED;
    }

    if (aw9523b_init(s_aw9523b_dev) != 0) {
        ESP_LOGE(TAG, "AW9523B init failed");
        return DIAG_FAILED;
    }

    /*
     * Per-pin init using read-modify-write only for the specific pins
     * we need.  Never write full registers (0x12/0x13 = 0x00) — that
     * affects ALL pins and may toggle undcoumented USB_OTG_EN signals.
     */

    /* P0_0 = TOUCH_RST: GPIO mode, output, held low (assert reset) */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_TOUCH_RST);
    aw9523b_pin_set_direction(AW9523B_PIN_TOUCH_RST, 1);
    aw9523b_pin_write(AW9523B_PIN_TOUCH_RST, 0);

    /* P1_2 = TOUCH_INT: GPIO mode, input */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_TOUCH_INT);
    aw9523b_pin_set_direction(AW9523B_PIN_TOUCH_INT, 0);

    ESP_LOGI(TAG, "AW9523B touch pins configured");
    return DIAG_PASSED;
}

diag_result_t hal_touch_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Step 1: Enable touch power via AXP2101 */
    if (touch_power_on() != DIAG_PASSED) {
        return DIAG_FAILED;
    }
    esp_rom_delay_us(50000);

    /* Step 2: Initialise AW9523B GPIO expander for RST/INT control */
    if (gpio_exp_init() != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    /* Step 3: Hardware reset FT6336 via AW9523B P0_0 */
    aw9523b_pin_write(AW9523B_PIN_TOUCH_RST, 0);
    esp_rom_delay_us(10000);
    aw9523b_pin_write(AW9523B_PIN_TOUCH_RST, 1);
    esp_rom_delay_us(50000);

    /* Step 4: Probe for touch controller address (0x38 default, try 0x3A/0x40) */
    const uint16_t addrs[] = { 0x38, 0x3A, 0x40 };
    const char *labels[]   = { "0x38", "0x3A", "0x40" };
    int found = -1;
    extern i2c_master_bus_handle_t hal_i2c_bus_get(void);
    i2c_master_bus_handle_t bus = hal_i2c_bus_get();
    if (bus) {
        for (int i = 0; i < 3; i++) {
            if (i2c_master_probe(bus, addrs[i], 50) == ESP_OK) {
                found = addrs[i];
                ESP_LOGI(TAG, "Touch found at %s", labels[i]);
                break;
            }
        }
    }

    if (found < 0) {
        ESP_LOGE(TAG, "No touch at 0x38/0x3A/0x40");
        return DIAG_FAILED;
    }

    /* Step 5: Add I2C device and init chip driver */
    if (hal_i2c_add_device((uint16_t)found, 400000, &s_i2c_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (touch_FT6336_init(s_i2c_dev) != 0) {
        ESP_LOGE(TAG, "FT6336 init failed at 0x%02X", found);
        return DIAG_FAILED;
    }

    s_initialised = true;
    ESP_LOGI(TAG, "FT6336 touch ready");
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
