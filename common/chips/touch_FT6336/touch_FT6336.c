/*
 * touch_FT6336.c — FT6336 Capacitive Touch Controller (I2C)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "touch_FT6336.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "FT6336";
static i2c_master_dev_handle_t s_dev = NULL;
static bool s_init = false;

/*===========================================================================*/
/* I2C helpers                                                               */
/*===========================================================================*/

static int read_reg(uint8_t reg, uint8_t *val)
{
    esp_err_t e = i2c_master_transmit_receive(s_dev, &reg, 1, val, 1, -1);
    return (e == ESP_OK) ? 0 : -1;
}

static int read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    esp_err_t e = i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, -1);
    return (e == ESP_OK) ? 0 : -1;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int touch_FT6336_init(i2c_master_dev_handle_t dev)
{
    if (!dev) return -1;
    s_dev = dev;

    /* Verify presence */
    uint8_t mode = 0;
    if (read_reg(FT6336_REG_DEVICE_MODE, &mode) != 0) {
        ESP_LOGE(TAG, "FT6336 not responding");
        return -1;
    }

    s_init = true;
    ESP_LOGI(TAG, "FT6336 initialised (mode=0x%02X, fw=0x%02X)",
             mode, touch_FT6336_firmware_version());
    return 0;
}

void touch_FT6336_deinit(void)
{
    s_init = false;
    s_dev = NULL;
}

int touch_FT6336_read(touch_FT6336_data_t *data)
{
    if (!data || !s_dev) return -1;
    memset(data, 0, sizeof(*data));

    uint8_t tds = 0;
    if (read_reg(FT6336_REG_TD_STATUS, &tds) != 0) return -1;

    tds &= 0x0F;
    if (tds > 2) tds = 2;
    data->point_count = tds;

    for (uint8_t i = 0; i < tds; i++) {
        uint8_t base = (i == 0) ? 0x03 : 0x09;
        uint8_t buf[6];
        if (read_regs(base, buf, 6) != 0) return -1;

        data->points[i].x     = ((uint16_t)(buf[0] & 0x0F) << 8) | buf[1];
        data->points[i].y     = ((uint16_t)(buf[2] & 0x0F) << 8) | buf[3];
        data->points[i].event = (buf[4] >> 6) & 0x03;
        data->points[i].id    = buf[4] & 0x0F;
    }
    return 0;
}

uint8_t touch_FT6336_firmware_version(void)
{
    if (!s_dev) return 0;
    uint8_t ver = 0;
    read_reg(FT6336_REG_FW_VERSION, &ver);
    return ver;
}
