/*
 * hal_i2c_adapter.c — ESP-IDF adapter for diag_i2c_t transport seam
 *
 * ADAPTER layer — implements the abstract diag_i2c_t interface using
 * the ESP-IDF I2C master driver.
 *
 * Each chip's device handle (i2c_master_dev_handle_t) is passed as the
 * opaque 'bus' pointer.  The slave address is baked into the handle at
 * device-add time, so the 'addr' parameter is unused by this adapter
 * but retained in the interface for non-ESP-IDF platforms.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_transport.h"
#include "diag_config.h"
#include "hal_i2c_helpers.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "i2c_adapter";

/*===========================================================================*/
/* Adapter callbacks                                                         */
/*===========================================================================*/

static int i2c_adapter_write(void *bus, uint16_t addr,
                              const void *data, size_t len)
{
    (void)addr;
    i2c_master_dev_handle_t dev = (i2c_master_dev_handle_t)bus;
    esp_err_t err = i2c_master_transmit(dev, data, len, -1);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "i2c write failed: %d", err);
        return -1;
    }
    return 0;
}

static int i2c_adapter_read(void *bus, uint16_t addr,
                             void *data, size_t len)
{
    (void)addr;
    i2c_master_dev_handle_t dev = (i2c_master_dev_handle_t)bus;
    esp_err_t err = i2c_master_receive(dev, data, len, -1);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "i2c read failed: %d", err);
        return -1;
    }
    return 0;
}

static int i2c_adapter_write_then_read(void *bus, uint16_t addr,
                                        const void *wdata, size_t wlen,
                                        void *rdata, size_t rlen)
{
    (void)addr;
    i2c_master_dev_handle_t dev = (i2c_master_dev_handle_t)bus;
    esp_err_t err = i2c_master_transmit_receive(dev, wdata, wlen,
                                                 rdata, rlen, -1);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "i2c wr-rd failed: %d", err);
        return -1;
    }
    return 0;
}

static int i2c_adapter_probe(void *bus, uint16_t addr)
{
    /* bus is the i2c_master_bus_handle_t, passed through from the caller */
    i2c_master_bus_handle_t h = (i2c_master_bus_handle_t)bus;
    if (!h) {
        h = hal_i2c_bus_get();
        if (!h) return -1;
    }
    return (i2c_master_probe(h, addr, 50) == ESP_OK) ? 0 : -1;
}

/*===========================================================================*/
/* Public singleton                                                          */
/*===========================================================================*/

const diag_i2c_t g_diag_i2c_adapter = {
    .write           = i2c_adapter_write,
    .read            = i2c_adapter_read,
    .write_then_read = i2c_adapter_write_then_read,
    .probe           = i2c_adapter_probe,
};
