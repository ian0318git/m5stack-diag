/*
 * hal_spi_adapter.c — ESP-IDF adapter for diag_spi_t transport seam
 *
 * ADAPTER layer — implements the abstract diag_spi_t interface using
 * the ESP-IDF SPI master driver.
 *
 * The SPI device handle (spi_device_handle_t) is passed as the opaque
 * 'bus' pointer.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_transport.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "spi_adapter";

/*===========================================================================*/
/* Adapter callbacks                                                         */
/*===========================================================================*/

static int spi_adapter_transmit(void *bus, const void *data, size_t len)
{
    spi_device_handle_t spi = (spi_device_handle_t)bus;
    spi_transaction_t t = {
        .length    = len * 8,        /* SPI transaction length in bits */
        .tx_buffer = data,
        .rx_buffer = NULL,
    };
    esp_err_t err = spi_device_transmit(spi, &t);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "spi transmit failed: %d", err);
        return -1;
    }
    return 0;
}

static int spi_adapter_transfer(void *bus, const void *tx_data,
                                 void *rx_data, size_t len)
{
    spi_device_handle_t spi = (spi_device_handle_t)bus;
    spi_transaction_t t = {
        .length    = len * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    esp_err_t err = spi_device_transmit(spi, &t);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "spi transfer failed: %d", err);
        return -1;
    }
    return 0;
}

/*===========================================================================*/
/* Public singleton                                                          */
/*===========================================================================*/

const diag_spi_t g_diag_spi_adapter = {
    .transmit = spi_adapter_transmit,
    .transfer = spi_adapter_transfer,
};
