/*
 * hal_touch.c — CoreS3 board adapter for FT6336 touch controller
 *
 * Board-specific: initialises AXP2101 touch power rail and I2C
 * device, then delegates to the common FT6336 chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_touch.h"
#include "hal_i2c_helpers.h"
#include "hal_power.h"
#include "touch_FT6336.h"
#include "diag_config.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hal_touch";

static i2c_master_dev_handle_t s_i2c_dev = NULL;
static bool s_initialised = false;

/*
 * CoreS3 touch power (FT6336 VCC) is supplied by AXP2101 LDOIO0.
 * The PMU must be initialised and configured before the FT6336
 * will respond on the I2C bus.
 */

/* AXP2101 register to control LDOIO0 (GPIO0) */
#define AXP2101_REG_GPIO0_LDO     0x90
/* Value to set GPIO0 as 3.3V LDO output */
#define AXP2101_GPIO0_3V3         0x07

static diag_result_t touch_power_on(void)
{
    /* Initialise the AXP2101 PMU so its LDO outputs work */
    if (hal_power_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "AXP2101 init failed — touch power may be off");
        return DIAG_FAILED;
    }

    /* Now write the LDOIO0 register to enable 3.3V touch power */
    i2c_master_dev_handle_t pmu_dev = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &pmu_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    uint8_t cmd[2] = { AXP2101_REG_GPIO0_LDO, AXP2101_GPIO0_3V3 };
    esp_err_t err = i2c_master_transmit(pmu_dev, cmd, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 touch LDO reg write failed: %d", err);
        return DIAG_FAILED;
    }

    ESP_LOGI(TAG, "AXP2101 LDOIO0 enabled for touch (3.3V)");
    return DIAG_PASSED;
}

diag_result_t hal_touch_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Step 1: Enable touch power via AXP2101 */
    if (touch_power_on() != DIAG_PASSED) {
        ESP_LOGE(TAG, "Touch power-on failed");
        return DIAG_FAILED;
    }
    esp_rom_delay_us(50000);   /* 50 ms for LDO to stabilise */

    /* Step 2: Hardware reset the FT6336 */
    gpio_set_direction(CONFIG_TOUCH_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_TOUCH_RST_PIN, 0);
    esp_rom_delay_us(10000);   /* hold reset ≥ 10 ms */
    gpio_set_level(CONFIG_TOUCH_RST_PIN, 1);
    esp_rom_delay_us(100000);  /* 100 ms for chip startup after reset */

    /* Step 3: Configure INT pin */
    gpio_set_direction(CONFIG_TOUCH_INT_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_TOUCH_INT_PIN, GPIO_PULLUP_ONLY);

    /* Step 4: Probe touch controller at known possible addresses */
    const uint16_t touch_addrs[] = {
        CONFIG_I2C_ADDR_TOUCH,  /* 0x38 — FT6336 default       */
        0x3A,                    /* 0x3A — FT6336 alt (ADDR=H)  */
        0x40,                    /* 0x40 — observed on some bds */
    };
    const char *addr_labels[] = { "0x38 (default)", "0x3A (alt)", "0x40" };

    int found_addr = -1;
    extern i2c_master_bus_handle_t hal_i2c_bus_get(void);
    i2c_master_bus_handle_t bus = hal_i2c_bus_get();

    if (bus) {
        for (int i = 0; i < 3; i++) {
            if (i2c_master_probe(bus, touch_addrs[i], 50) == ESP_OK) {
                found_addr = touch_addrs[i];
                ESP_LOGI(TAG, "Touch controller found at %s",
                         addr_labels[i]);
                break;
            }
        }
    }

    if (found_addr < 0) {
        ESP_LOGE(TAG, "No touch controller found at 0x38, 0x3A, or 0x40");
        return DIAG_FAILED;
    }

    /* Step 5: Add I2C device at the found address */
    if (hal_i2c_add_device((uint16_t)found_addr, 400000, &s_i2c_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    /* Step 6: Init the chip driver */
    if (touch_FT6336_init(s_i2c_dev) != 0) {
        ESP_LOGE(TAG, "FT6336 init failed at %s",
                 (found_addr == 0x38) ? "0x38" :
                 (found_addr == 0x3A) ? "0x3A" : "0x40");
        return DIAG_FAILED;
    }

    s_initialised = true;
    ESP_LOGI(TAG, "FT6336 touch initialised");
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
