/*
 * screen_GC9A01.c — GC9A01 Circular LCD Driver (SPI)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "screen_GC9A01.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "GC9A01";

static spi_device_handle_t s_spi = NULL;
static int s_dc_gpio = -1;

/*===========================================================================*/
/* SPI Primitives                                                            */
/*===========================================================================*/

/*
 * GC9A01 uses 4-wire SPI where the DC (Data/Command) pin distinguishes
 * command bytes (DC=0) from data bytes (DC=1).
 *
 * We control DC via direct GPIO writes rather than SPI bit-banding,
 * which is simpler and works with ESP-IDF's SPI master driver.
 */

void screen_GC9A01_send_cmd(uint8_t cmd)
{
    gpio_set_level(s_dc_gpio, 0);   /* command */
    spi_transaction_t t = {
        .length    = 8,
        .tx_buffer = &cmd,
        .flags     = SPI_TRANS_USE_TXDATA,
    };
    spi_device_transmit(s_spi, &t);
}

void screen_GC9A01_send_data(uint8_t data)
{
    gpio_set_level(s_dc_gpio, 1);   /* data */
    spi_transaction_t t = {
        .length    = 8,
        .tx_buffer = &data,
        .flags     = SPI_TRANS_USE_TXDATA,
    };
    spi_device_transmit(s_spi, &t);
}

void screen_GC9A01_send_cmd_data(uint8_t cmd, const uint8_t *data, size_t len)
{
    screen_GC9A01_send_cmd(cmd);
    for (size_t i = 0; i < len; i++) {
        screen_GC9A01_send_data(data[i]);
    }
}

void screen_GC9A01_set_window(uint16_t x0, uint16_t y0,
                               uint16_t x1, uint16_t y1)
{
    uint8_t col[4] = { (uint8_t)(x0 >> 8), (uint8_t)x0,
                       (uint8_t)(x1 >> 8), (uint8_t)x1 };
    uint8_t row[4] = { (uint8_t)(y0 >> 8), (uint8_t)y0,
                       (uint8_t)(y1 >> 8), (uint8_t)y1 };
    screen_GC9A01_send_cmd_data(GC9A01_CASET, col, 4);
    screen_GC9A01_send_cmd_data(GC9A01_RASET, row, 4);
    screen_GC9A01_send_cmd(GC9A01_RAMWR);
}

void screen_GC9A01_write_pixels(const uint16_t *pixels, size_t count)
{
    gpio_set_level(s_dc_gpio, 1);   /* data */

    while (count > 0) {
        size_t chunk = (count > 4096) ? 4096 : count;
        spi_transaction_t t = {
            .length    = chunk * 16,
            .tx_buffer = pixels,
        };
        spi_device_transmit(s_spi, &t);
        pixels += chunk;
        count  -= chunk;
    }
}

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int screen_GC9A01_init(spi_device_handle_t spi, int dc_gpio, int rst_gpio)
{
    if (!spi) return -1;
    s_spi    = spi;
    s_dc_gpio = dc_gpio;

    /* Configure DC as output */
    gpio_set_direction(dc_gpio, GPIO_MODE_OUTPUT);

    /* Hardware reset */
    if (rst_gpio >= 0) {
        gpio_set_direction(rst_gpio, GPIO_MODE_OUTPUT);
        gpio_set_level(rst_gpio, 0);
        esp_rom_delay_us(10000);
        gpio_set_level(rst_gpio, 1);
        esp_rom_delay_us(10000);
    }

    /* Software reset */
    screen_GC9A01_send_cmd(GC9A01_SWRESET);
    esp_rom_delay_us(5000);

    /* Colour mode: 16-bit (RGB565) */
    screen_GC9A01_send_cmd_data(GC9A01_COLMOD, (uint8_t[]){0x55}, 1);

    /* Memory access: BGR order */
    screen_GC9A01_send_cmd_data(GC9A01_MADCTL,
                                 (uint8_t[]){GC9A01_MADCTL_BGR}, 1);

    /* Sleep out */
    screen_GC9A01_send_cmd(GC9A01_SLPOUT);
    esp_rom_delay_us(5000);

    /* Display on */
    screen_GC9A01_send_cmd(GC9A01_DISPON);
    esp_rom_delay_us(5000);

    ESP_LOGI(TAG, "GC9A01 initialised");
    return 0;
}

void screen_GC9A01_deinit(void)
{
    screen_GC9A01_send_cmd(GC9A01_DISPOFF);
    screen_GC9A01_send_cmd(GC9A01_SLPIN);
    s_spi = NULL;
}
