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
    /* spi_bus may be NULL with a self-contained transport (the bit-bang
     * adapter uses fixed board pins and ignores the bus handle). */
    if (!spi) return -1;
    s_spi     = spi;
    s_spi_bus = spi_bus;
    s_dc_set  = dc_set;
    s_set_rst = set_rst;

    /* Hardware reset: 20 ms low, then 64 ms before any command
     * (matches M5Unified: rst low 8 ms, high + 64 ms, then the list).
     * NO software reset — the hardware reset covers it, and the proven
     * m5gfx sequence relies on it. */
    if (s_set_rst) {
        s_set_rst(0);
        esp_rom_delay_us(20000);
        s_set_rst(1);
        esp_rom_delay_us(64000);
    }

    /* CS-asserted window for the whole init sequence (required by the
     * ILI9342C — per-byte CS pulses corrupt the command framing). */
    if (s_spi->begin) s_spi->begin(s_spi_bus);

    /* ILI9342C init — byte-for-byte the sequence proven on this panel
     * (m5gfx Panel_ILI9342 list0 + M5GFX begin, verified by the bit-bang
     * test): SETEXTC unlock first, DISPON BEFORE SLPOUT, and
     * COLMOD/MADCTL/INVON AFTER sleep-out — the panel may ignore
     * configuration commands while asleep. */
    spi_cmd_data(0xC8, (uint8_t[]){0xFF, 0x93, 0x42}, 3);  /* SETEXTC unlock */

    /* Power control */
    spi_cmd_data(0xC0, (uint8_t[]){0x12, 0x12}, 2);        /* PWCTR1 */
    spi_cmd_d1(0xC1, 0x03);                                /* PWCTR2 */
    spi_cmd_d1(0xC5, 0xF2);                                /* VMCTR1 */
    spi_cmd_d1(0xB0, 0xE0);
    spi_cmd_data(0xF6, (uint8_t[]){0x01, 0x00, 0x00}, 3);

    /* Gamma correction (ILI9342C values from m5gfx) */
    spi_cmd_data(0xE0, (uint8_t[]){
        0x00, 0x0C, 0x11, 0x04, 0x11, 0x08, 0x37, 0x89,
        0x4C, 0x06, 0x0C, 0x0A, 0x2E, 0x34, 0x0F
    }, 15);
    spi_cmd_data(0xE1, (uint8_t[]){
        0x00, 0x0B, 0x11, 0x05, 0x13, 0x09, 0x33, 0x67,
        0x48, 0x07, 0x0E, 0x0B, 0x2E, 0x33, 0x0F
    }, 15);

    /* Display function control + idle mode off */
    spi_cmd_data(0xB6, (uint8_t[]){0x08, 0x82, 0x1D, 0x04}, 4);
    spi_cmd(0x38);

    /* Display on, then sleep out (m5gfx list order) */
    spi_cmd(ILI9342_CMD_DISPON);
    spi_cmd(ILI9342_CMD_SLPOUT);
    esp_rom_delay_us(120000);

    /* Post-sleep-out configuration (m5gfx begin order: setColorDepth,
     * setRotation, invertDisplay): colour mode 16-bit RGB565 */
    spi_cmd_d1(ILI9342_CMD_COLMOD, 0x55);

    /* Memory access: BGR order, mirrored for CoreS3 orientation */
    spi_cmd_d1(ILI9342_CMD_MADCTL, ILI9342_MADCTL_MX |
                                    ILI9342_MADCTL_BGR);

    /* CoreS3 panel is inverted (m5gfx Panel_M5StackCoreS3: invert=true) */
    spi_cmd(ILI9342_CMD_INVON);
    esp_rom_delay_us(10000);

    if (s_spi->end) s_spi->end(s_spi_bus);   /* CS released */
    return 0;
}

void lcd_ILI9342C_deinit(void)
{
    /* Leave the panel out of reset (RST high) — a low here would keep
     * the panel deaf for the next init. */
    if (s_spi->end) s_spi->end(s_spi_bus);
    if (s_set_rst) s_set_rst(1);
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
