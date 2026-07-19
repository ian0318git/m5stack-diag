/*
 * lcd_ILI9342C.c — ILI9342C 320×240 IPS LCD Driver (SPI)
 *
 * Common chip driver.  Initialisation sequence targets the ILI9342C
 * variant used on the M5Stack CoreS3.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "lcd_ILI9342C.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "ILI9342C";

static spi_device_handle_t s_spi = NULL;
static int s_dc_gpio = -1;
static void (*s_set_rst)(int level) = NULL;

/*===========================================================================*/
/* SPI Primitives (DC pin controlled via GPIO)                               */
/*===========================================================================*/

static void spi_cmd(uint8_t cmd)
{
    gpio_set_level(s_dc_gpio, 0);
    spi_transaction_t t = {
        .length    = 8,
        .tx_buffer = &cmd,
        .flags     = SPI_TRANS_USE_TXDATA,
    };
    spi_device_transmit(s_spi, &t);
}

static void spi_data(uint8_t data)
{
    gpio_set_level(s_dc_gpio, 1);
    spi_transaction_t t = {
        .length    = 8,
        .tx_buffer = &data,
        .flags     = SPI_TRANS_USE_TXDATA,
    };
    spi_device_transmit(s_spi, &t);
}

static void spi_cmd_data(uint8_t cmd, const uint8_t *data, size_t len)
{
    spi_cmd(cmd);
    for (size_t i = 0; i < len; i++) {
        spi_data(data[i]);
    }
}

static inline void spi_cmd_d1(uint8_t cmd, uint8_t d1)
{
    spi_cmd_data(cmd, &d1, 1);
}

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int lcd_ILI9342C_init(spi_device_handle_t spi, int dc_gpio,
                       void (*set_rst)(int level))
{
    if (!spi) return -1;
    s_spi    = spi;
    s_dc_gpio = dc_gpio;
    s_set_rst = set_rst;

    gpio_set_direction(dc_gpio, GPIO_MODE_OUTPUT);

    /* Hardware reset */
    if (s_set_rst) {
        s_set_rst(0);
        esp_rom_delay_us(10000);
        s_set_rst(1);
        esp_rom_delay_us(10000);
    }

    /* Software reset */
    spi_cmd(ILI9342_CMD_SWRESET);
    esp_rom_delay_us(5000);

    /* Common ILI9341-compatible init sequence (valid for ILI9342C) */
    spi_cmd_data(0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5);  /* POWERA */
    spi_cmd_data(0xCF, (uint8_t[]){0x00, 0xC1, 0x30}, 3);             /* POWERB */
    spi_cmd_data(0xE8, (uint8_t[]){0x85, 0x00, 0x78}, 3);             /* DTCA   */
    spi_cmd_data(0xEA, (uint8_t[]){0x00, 0x00}, 2);                   /* DTCB   */
    spi_cmd_data(0xB1, (uint8_t[]){0x00, 0x1B}, 2);                   /* FRAMERATE */
    spi_cmd_d1(0xB6, 0x0A);                                            /* DISPCTRL */
    spi_cmd_d1(0xF2, 0x00);                                            /* 3GAMMA_EN */

    /* Colour mode: 16-bit RGB565 */
    spi_cmd_d1(ILI9342_CMD_COLMOD, 0x55);

    /* Memory access: BGR order, mirrored for CoreS3 orientation */
    spi_cmd_d1(ILI9342_CMD_MADCTL, ILI9342_MADCTL_MX |
                                    ILI9342_MADCTL_BGR);

    /* Gamma correction */
    spi_cmd_data(0xE0, (uint8_t[]){
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00
    }, 15);
    spi_cmd_data(0xE1, (uint8_t[]){
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F
    }, 15);

    /* Sleep out & display on */
    spi_cmd(ILI9342_CMD_SLPOUT);
    esp_rom_delay_us(120000);
    spi_cmd(ILI9342_CMD_DISPON);
    esp_rom_delay_us(10000);

    ESP_LOGI(TAG, "ILI9342C initialised (320x240)");
    return 0;
}

void lcd_ILI9342C_deinit(void)
{
    if (s_set_rst) s_set_rst(0);
    s_spi = NULL;
}

/*===========================================================================*/
/* Drawing                                                                   */
/*===========================================================================*/

void lcd_ILI9342C_set_window(uint16_t x0, uint16_t y0,
                              uint16_t x1, uint16_t y1)
{
    uint8_t col[4] = { (uint8_t)(x0 >> 8), (uint8_t)x0,
                       (uint8_t)(x1 >> 8), (uint8_t)x1 };
    uint8_t row[4] = { (uint8_t)(y0 >> 8), (uint8_t)y0,
                       (uint8_t)(y1 >> 8), (uint8_t)y1 };
    spi_cmd_data(ILI9342_CMD_CASET, col, 4);
    spi_cmd_data(ILI9342_CMD_RASET, row, 4);
    spi_cmd(ILI9342_CMD_RAMWR);
}

void lcd_ILI9342C_write_pixels(const uint16_t *pixels, size_t count)
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
