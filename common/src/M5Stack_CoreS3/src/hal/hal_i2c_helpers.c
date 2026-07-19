/*
 * hal_i2c_helpers.c - Shared I2C bus for CoreS3 HAL modules
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_i2c_helpers.h"

static i2c_master_bus_handle_t s_bus = NULL;
static int s_refcount = 0;

i2c_master_bus_handle_t hal_i2c_bus_get(void)
{
    if (s_bus == NULL) {
        const i2c_master_bus_config_t cfg = {
            .i2c_port    = CONFIG_I2C_NUM,
            .sda_io_num  = CONFIG_I2C_SDA_PIN,
            .scl_io_num  = CONFIG_I2C_SCL_PIN,
            .clk_source  = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
        };

        esp_err_t err = i2c_new_master_bus(&cfg, &s_bus);
        if (err != ESP_OK) {
            s_bus = NULL;
            return NULL;
        }
    }
    s_refcount++;
    return s_bus;
}

diag_result_t hal_i2c_add_device(uint16_t addr,
                                  uint32_t scl_speed,
                                  i2c_master_dev_handle_t *dev)
{
    i2c_master_bus_handle_t bus = hal_i2c_bus_get();
    if (!bus) return DIAG_FAILED;

    if (scl_speed == 0) {
        scl_speed = CONFIG_I2C_CLOCK_HZ;
    }

    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = addr,
        .scl_speed_hz    = scl_speed,
    };

    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, dev);
    if (err != ESP_OK) {
        return DIAG_FAILED;
    }

    return DIAG_PASSED;
}

void hal_i2c_bus_release(void)
{
    if (s_bus == NULL) return;

    s_refcount--;
    if (s_refcount <= 0) {
        i2c_del_master_bus(s_bus);
        s_bus = NULL;
        s_refcount = 0;
    }
}
