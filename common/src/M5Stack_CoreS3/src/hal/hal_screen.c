/*
 * hal_screen.c — CoreS3 board adapter for ILI9342C display
 *
 * Board-specific (CoreS3):
 *   - SPI: MOSI=G37, SCLK=G36, CS=G3, DC=G35, MISO=G35 (unused)
 *   - RST via AW9523B GPIO expander P1_1
 *   - Backlight via AXP2101 DLDO1
 *
 * Verified against: https://docs.m5stack.com/zh_CN/core/CoreS3
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_screen.h"
#include "hal_i2c_helpers.h"
#include "hal_power.h"
#include "aw9523b.h"
#include "lcd_ILI9342C.h"
#include "diag_config.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hal_screen";

static spi_device_handle_t s_spi = NULL;
static bool s_initialised = false;
static i2c_master_dev_handle_t s_aw9523b_dev = NULL;

/*===========================================================================*/
/* AW9523B initialisation (shared — also used by hal_touch.c)                */
/*===========================================================================*/

/*
 * lcd_rst_callback — called by the ILI9342C chip driver to assert/de-assert
 * the LCD reset line via AW9523B P1_1.
 */
static void lcd_rst_callback(int level)
{
    if (s_aw9523b_dev) {
        aw9523b_pin_write(AW9523B_PIN_LCD_RST, level);
    }
}

static diag_result_t gpio_exp_init(void)
{
    if (s_aw9523b_dev) return DIAG_PASSED;

    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &s_aw9523b_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (aw9523b_init(s_aw9523b_dev) != 0) {
        return DIAG_FAILED;
    }

    /* LCD_RST (P1_1) as output, held in reset */
    aw9523b_pin_set_direction(AW9523B_PIN_LCD_RST, 1);
    aw9523b_pin_write(AW9523B_PIN_LCD_RST, 0);

    ESP_LOGI(TAG, "AW9523B LCD pins configured");
    return DIAG_PASSED;
}

/*===========================================================================*/
/* Backlight control via AXP2101 DLDO1                                      */
/*===========================================================================*/

static diag_result_t backlight_init(void)
{
    if (hal_power_init() != DIAG_PASSED) {
        ESP_LOGW(TAG, "AXP2101 init failed — backlight may not work");
        return DIAG_FAILED;
    }

    /* AXP2101 DLDO1 output voltage register */
    i2c_master_dev_handle_t pmu = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &pmu) != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    /* AXP2101 register 0x12: DLDO1 control (bit 3 = enable, bits 0-2 = voltage) */
    /* 0x0C = enable + 3.3V (0b00001100) */
    uint8_t cmd[2] = { 0x12, 0x0C };
    esp_err_t err = i2c_master_transmit(pmu, cmd, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 DLDO1 write failed: %d", err);
    }

    return DIAG_PASSED;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

diag_result_t hal_screen_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Step 1: Init AW9523B for LCD RST */
    if (gpio_exp_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "GPIO expander init failed");
        return DIAG_FAILED;
    }

    /* Step 2: Enable backlight via AXP2101 DLDO1 */
    backlight_init();

    /* Step 3: Initialise SPI bus */
    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = CONFIG_LCD_MOSI_PIN,
        .miso_io_num     = CONFIG_LCD_MISO_PIN,
        .sclk_io_num     = CONFIG_LCD_SCLK_PIN,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = CONFIG_LCD_WIDTH * CONFIG_LCD_HEIGHT * 2 + 8,
    };

    esp_err_t err = spi_bus_initialize(CONFIG_LCD_SPI_NUM, &bus_cfg,
                                       SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %d", err);
        return DIAG_FAILED;
    }

    /* Step 4: Attach SPI device */
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = CONFIG_LCD_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = CONFIG_LCD_CS_PIN,
        .queue_size     = 1,
        .flags          = SPI_DEVICE_HALFDUPLEX,
    };
    err = spi_bus_add_device(CONFIG_LCD_SPI_NUM, &dev_cfg, &s_spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %d", err);
        spi_bus_free(CONFIG_LCD_SPI_NUM);
        return DIAG_FAILED;
    }

    /* Step 5: Init the ILI9342C chip driver */
    if (lcd_ILI9342C_init(s_spi, CONFIG_LCD_DC_PIN, lcd_rst_callback) != 0) {
        ESP_LOGE(TAG, "ILI9342C init failed");
        return DIAG_FAILED;
    }

    /* Fill black */
    size_t total = CONFIG_LCD_WIDTH * CONFIG_LCD_HEIGHT;
    lcd_ILI9342C_set_window(0, 0, CONFIG_LCD_WIDTH - 1, CONFIG_LCD_HEIGHT - 1);
    uint16_t black = HAL_SCREEN_COLOR_BLACK;
    for (size_t i = 0; i < total; i += 4096) {
        size_t chunk = (total - i < 4096) ? (total - i) : 4096;
        lcd_ILI9342C_write_pixels(&black, chunk);
    }

    s_initialised = true;
    ESP_LOGI(TAG, "CoreS3 screen HAL ready (ILI9342C)");
    return DIAG_PASSED;
}

void hal_screen_deinit(void)
{
    if (!s_initialised) return;
    lcd_ILI9342C_deinit();
    spi_bus_remove_device(s_spi);
    spi_bus_free(CONFIG_LCD_SPI_NUM);
    s_spi = NULL;
    s_initialised = false;
}

void hal_screen_fill(hal_screen_colour_t colour)
{
    lcd_ILI9342C_set_window(0, 0, CONFIG_LCD_WIDTH - 1, CONFIG_LCD_HEIGHT - 1);
    size_t total = CONFIG_LCD_WIDTH * CONFIG_LCD_HEIGHT;
    for (size_t i = 0; i < total; i += 4096) {
        size_t chunk = (total - i < 4096) ? (total - i) : 4096;
        lcd_ILI9342C_write_pixels(&colour, chunk);
    }
}

void hal_screen_draw_pixel(int x, int y, hal_screen_colour_t colour)
{
    if (x < 0 || x >= CONFIG_LCD_WIDTH || y < 0 || y >= CONFIG_LCD_HEIGHT) return;
    lcd_ILI9342C_set_window((uint16_t)x, (uint16_t)y,
                             (uint16_t)x, (uint16_t)y);
    lcd_ILI9342C_write_pixels(&colour, 1);
}

void hal_screen_fill_rect(int x, int y, int w, int h,
                          hal_screen_colour_t colour)
{
    if (w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > CONFIG_LCD_WIDTH)  w = CONFIG_LCD_WIDTH  - x;
    if (y + h > CONFIG_LCD_HEIGHT) h = CONFIG_LCD_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    lcd_ILI9342C_set_window((uint16_t)x, (uint16_t)y,
                             (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));
    size_t total = (size_t)w * h;
    for (size_t i = 0; i < total; i += 4096) {
        size_t chunk = (total - i < 4096) ? (total - i) : 4096;
        lcd_ILI9342C_write_pixels(&colour, chunk);
    }
}

void hal_screen_draw_line(int x0, int y0, int x1, int y1,
                          hal_screen_colour_t colour)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        hal_screen_draw_pixel(x0, y0, colour);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

/*===========================================================================*/
/* Font + text rendering                                                     */
/*===========================================================================*/

static const uint8_t s_font6x8[95][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00,0x00},
    {0x00,0x07,0x00,0x07,0x00,0x00},{0x14,0x7F,0x14,0x7F,0x14,0x00},
    {0x24,0x2A,0x7F,0x2A,0x12,0x00},{0x23,0x13,0x08,0x64,0x62,0x00},
    {0x36,0x49,0x55,0x22,0x50,0x00},{0x00,0x05,0x03,0x00,0x00,0x00},
    {0x00,0x1C,0x22,0x41,0x00,0x00},{0x00,0x41,0x22,0x1C,0x00,0x00},
    {0x08,0x2A,0x1C,0x2A,0x08,0x00},{0x08,0x08,0x3E,0x08,0x08,0x00},
    {0x00,0x50,0x30,0x00,0x00,0x00},{0x08,0x08,0x08,0x08,0x08,0x00},
    {0x00,0x60,0x60,0x00,0x00,0x00},{0x20,0x10,0x08,0x04,0x02,0x00},
    {0x3E,0x51,0x49,0x45,0x3E,0x00},{0x00,0x42,0x7F,0x40,0x00,0x00},
    {0x42,0x61,0x51,0x49,0x46,0x00},{0x21,0x41,0x45,0x4B,0x31,0x00},
    {0x18,0x14,0x12,0x7F,0x10,0x00},{0x27,0x45,0x45,0x45,0x39,0x00},
    {0x3C,0x4A,0x49,0x49,0x30,0x00},{0x01,0x71,0x09,0x05,0x03,0x00},
    {0x36,0x49,0x49,0x49,0x36,0x00},{0x06,0x49,0x49,0x29,0x1E,0x00},
    {0x00,0x36,0x36,0x00,0x00,0x00},{0x00,0x56,0x36,0x00,0x00,0x00},
    {0x00,0x08,0x14,0x22,0x41,0x00},{0x14,0x14,0x14,0x14,0x14,0x00},
    {0x41,0x22,0x14,0x08,0x00,0x00},{0x02,0x01,0x51,0x09,0x06,0x00},
    {0x32,0x49,0x79,0x41,0x3E,0x00},{0x7E,0x11,0x11,0x11,0x7E,0x00},
    {0x7F,0x49,0x49,0x49,0x36,0x00},{0x3E,0x41,0x41,0x41,0x22,0x00},
    {0x7F,0x41,0x41,0x22,0x1C,0x00},{0x7F,0x49,0x49,0x49,0x41,0x00},
    {0x7F,0x09,0x09,0x01,0x01,0x00},{0x3E,0x41,0x41,0x51,0x32,0x00},
    {0x7F,0x08,0x08,0x08,0x7F,0x00},{0x00,0x41,0x7F,0x41,0x00,0x00},
    {0x20,0x40,0x41,0x3F,0x01,0x00},{0x7F,0x08,0x14,0x22,0x41,0x00},
    {0x7F,0x40,0x40,0x40,0x40,0x00},{0x7F,0x02,0x04,0x02,0x7F,0x00},
    {0x7F,0x04,0x08,0x10,0x7F,0x00},{0x3E,0x41,0x41,0x41,0x3E,0x00},
    {0x7F,0x09,0x09,0x09,0x06,0x00},{0x3E,0x41,0x51,0x21,0x5E,0x00},
    {0x7F,0x09,0x19,0x29,0x46,0x00},{0x46,0x49,0x49,0x49,0x31,0x00},
    {0x01,0x01,0x7F,0x01,0x01,0x00},{0x3F,0x40,0x40,0x40,0x3F,0x00},
    {0x1F,0x20,0x40,0x20,0x1F,0x00},{0x7F,0x20,0x18,0x20,0x7F,0x00},
    {0x63,0x14,0x08,0x14,0x63,0x00},{0x03,0x04,0x78,0x04,0x03,0x00},
    {0x61,0x51,0x49,0x45,0x43,0x00},{0x00,0x00,0x7F,0x41,0x41,0x00},
    {0x02,0x04,0x08,0x10,0x20,0x00},{0x41,0x41,0x7F,0x00,0x00,0x00},
    {0x04,0x02,0x01,0x02,0x04,0x00},{0x40,0x40,0x40,0x40,0x40,0x00},
    {0x00,0x01,0x02,0x04,0x00,0x00},{0x20,0x54,0x54,0x54,0x78,0x00},
    {0x7F,0x48,0x44,0x44,0x38,0x00},{0x38,0x44,0x44,0x44,0x20,0x00},
    {0x38,0x44,0x44,0x48,0x7F,0x00},{0x38,0x54,0x54,0x54,0x18,0x00},
    {0x08,0x7E,0x09,0x01,0x02,0x00},{0x08,0x14,0x54,0x54,0x3C,0x00},
    {0x7F,0x08,0x04,0x04,0x78,0x00},{0x00,0x44,0x7D,0x40,0x00,0x00},
    {0x20,0x40,0x44,0x3D,0x00,0x00},{0x00,0x7F,0x10,0x28,0x44,0x00},
    {0x00,0x41,0x7F,0x40,0x00,0x00},{0x7C,0x04,0x18,0x04,0x78,0x00},
    {0x7C,0x08,0x04,0x04,0x78,0x00},{0x38,0x44,0x44,0x44,0x38,0x00},
    {0x7C,0x14,0x14,0x14,0x08,0x00},{0x08,0x14,0x14,0x18,0x7C,0x00},
    {0x7C,0x08,0x04,0x04,0x08,0x00},{0x48,0x54,0x54,0x54,0x20,0x00},
    {0x04,0x3F,0x44,0x40,0x20,0x00},{0x3C,0x40,0x40,0x20,0x7C,0x00},
    {0x1C,0x20,0x40,0x20,0x1C,0x00},{0x3C,0x40,0x30,0x40,0x3C,0x00},
    {0x44,0x28,0x10,0x28,0x44,0x00},{0x0C,0x50,0x50,0x50,0x3C,0x00},
    {0x44,0x64,0x54,0x4C,0x44,0x00},{0x00,0x08,0x36,0x41,0x00,0x00},
    {0x00,0x00,0x7F,0x00,0x00,0x00},{0x00,0x41,0x36,0x08,0x00,0x00},
    {0x08,0x04,0x08,0x10,0x08,0x00},
};

static int s_font_idx = 0;
static const int s_fw[] = { 6, 10, 16 };
static const int s_fh[] = { 8, 16, 24 };

void hal_screen_draw_text(int x, int y, const char *text,
                          hal_screen_colour_t fg, hal_screen_colour_t bg)
{
    if (!text) return;
    int fw = s_fw[s_font_idx], fh = s_fh[s_font_idx];
    int cx = x;

    while (*text) {
        unsigned char ch = (unsigned char)*text;
        if (ch == '\n') { y += fh; cx = x; text++; continue; }
        if (ch < 0x20 || ch > 0x7E) { cx += fw; text++; continue; }

        const uint8_t *g = s_font6x8[ch - 0x20];
        for (int col = 0; col < fw && col < 6; col++) {
            uint8_t bits = g[col];
            for (int row = 0; row < fh && row < 8; row++)
                hal_screen_draw_pixel(cx + col, y + row,
                                      (bits & (1 << row)) ? fg : bg);
            for (int row = 8; row < fh; row++)
                hal_screen_draw_pixel(cx + col, y + row, bg);
        }
        for (int col = 6; col < fw; col++)
            for (int row = 0; row < fh; row++)
                hal_screen_draw_pixel(cx + col, y + row, bg);
        cx += fw;
        text++;
    }
}

void hal_screen_set_font(int size)
{
    if (size < 0) size = 0;
    if (size > 2) size = 2;
    s_font_idx = size;
}

int hal_screen_font_width(void)  { return s_fw[s_font_idx]; }
int hal_screen_font_height(void) { return s_fh[s_font_idx]; }

void hal_screen_set_backlight(uint8_t b)
{
    (void)b;
    /* Backlight controlled via AXP2101 DLDO1 — see hal_screen_init() */
}

int hal_screen_width(void)  { return CONFIG_LCD_WIDTH; }
int hal_screen_height(void) { return CONFIG_LCD_HEIGHT; }
