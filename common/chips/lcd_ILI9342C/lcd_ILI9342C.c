/*
 * lcd_ILI9342C.c — ILI9342C 320×240 IPS LCD Driver (SPI)
 *
 * Common chip driver — uses abstract diag_spi_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "lcd_ILI9342C.h"
#include "esp_rom_sys.h"
#include <string.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static const diag_spi_t *s_spi = NULL;
static void             *s_spi_bus = NULL;
static void (*s_dc_set)(int level) = NULL;
static void (*s_set_rst)(int level) = NULL;

/*===========================================================================*/
/* SPI Primitives (DC pin controlled via callback)                           */
/*===========================================================================*/

static void spi_cmd(uint8_t cmd)
{
    if (s_dc_set) s_dc_set(0);
    s_spi->transmit(s_spi_bus, &cmd, 1);
    if (s_dc_set) s_dc_set(1);
}

static void spi_data_buf(const uint8_t *data, size_t len)
{
    if (s_dc_set) s_dc_set(1);
    s_spi->transmit(s_spi_bus, data, len);
}

static void spi_cmd_data(uint8_t cmd, const uint8_t *data, size_t len)
{
    spi_cmd(cmd);
    spi_data_buf(data, len);
}

static void spi_cmd_d1(uint8_t cmd, uint8_t d1)
{
    spi_cmd_data(cmd, &d1, 1);
}

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int lcd_ILI9342C_init(const diag_spi_t *spi, void *spi_bus,
                       void (*dc_set)(int level),
                       void (*set_rst)(int level))
{
    if (!spi || !spi_bus) return -1;
    s_spi     = spi;
    s_spi_bus = spi_bus;
    s_dc_set  = dc_set;
    s_set_rst = set_rst;

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
    spi_cmd_data(0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5);
    spi_cmd_data(0xCF, (uint8_t[]){0x00, 0xC1, 0x30}, 3);
    spi_cmd_data(0xE8, (uint8_t[]){0x85, 0x00, 0x78}, 3);
    spi_cmd_data(0xEA, (uint8_t[]){0x00, 0x00}, 2);
    spi_cmd_data(0xB1, (uint8_t[]){0x00, 0x1B}, 2);
    spi_cmd_d1(0xB6, 0x0A);
    spi_cmd_d1(0xF2, 0x00);

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

    return 0;
}

void lcd_ILI9342C_deinit(void)
{
    if (s_set_rst) s_set_rst(0);
    s_spi = NULL;
    s_spi_bus = NULL;
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
    if (s_dc_set) s_dc_set(1);   /* data mode */

    while (count > 0) {
        size_t chunk = (count > 4096) ? 4096 : count;
        s_spi->transmit(s_spi_bus, pixels, chunk * sizeof(uint16_t));
        pixels += chunk;
        count  -= chunk;
    }
}
