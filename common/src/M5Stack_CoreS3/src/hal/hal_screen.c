/*
 * hal_screen.c — CoreS3 board adapter for ILI9342C display
 *
 * Board-specific (CoreS3):
 *   - SPI: MOSI=G37, SCLK=G36, CS=G3, DC=G35, MISO=G35 (unused)
 *   - RST via AW9523B GPIO expander P1_1
 *   - Backlight via AXP2101 DLDO1
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_screen.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "hal_spi_adapter.h"
#include "hal_spi2_bus.h"
#include "hal_power.h"
#include "aw9523b.h"
#include "lcd_ILI9342C.h"
#include "diag_config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hal_screen";

static spi_device_handle_t s_spi = NULL;
static bool s_initialised = false;
static i2c_master_dev_handle_t s_aw9523b_dev = NULL;
static bool s_aw9523b_inited = false;

/*===========================================================================*/
/* AW9523B initialisation (shared with hal_touch.c)                          */
/*===========================================================================*/

static void lcd_rst_callback(int level)
{
    if (s_aw9523b_inited) {
        aw9523b_pin_write(AW9523B_PIN_LCD_RST, level);
    }
}

static void dc_callback(int level)
{
    gpio_set_level(CONFIG_LCD_DC_PIN, level);
}

static diag_result_t gpio_exp_init(void)
{
    if (s_aw9523b_inited) return DIAG_PASSED;

    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &s_aw9523b_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (aw9523b_init(&g_diag_i2c_adapter, (void *)s_aw9523b_dev) != 0) {
        return DIAG_FAILED;
    }

    /* LCD_RST (P1_1): GPIO mode, output, held in reset */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_LCD_RST);
    aw9523b_pin_set_direction(AW9523B_PIN_LCD_RST, 1);
    aw9523b_pin_write(AW9523B_PIN_LCD_RST, 0);

    /* Configure DC pin as GPIO output */
    gpio_set_direction(CONFIG_LCD_DC_PIN, GPIO_MODE_OUTPUT);

    s_aw9523b_inited = true;
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

    i2c_master_dev_handle_t pmu = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &pmu) != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    /* AXP2101 register 0x12: DLDO1 control — enable + 3.3V */
    uint8_t cmd[2] = { 0x12, 0x0C };
    esp_err_t err = i2c_master_transmit(pmu, cmd, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 DLDO1 write failed: %d", err);
        return DIAG_FAILED;
    }

    return DIAG_PASSED;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

diag_result_t hal_screen_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Step 1: Init AW9523B for LCD RST + DC GPIO */
    if (gpio_exp_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "GPIO expander init failed");
        return DIAG_FAILED;
    }

    /* Step 2: Enable backlight via AXP2101 DLDO1 */
    if (backlight_init() != DIAG_PASSED) {
        ESP_LOGW(TAG, "Backlight init failed — display may be dark");
    }

    /* Step 3: Initialise SPI2 bus (shared with SD card) */
    if (hal_spi2_bus_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "SPI2 bus init failed");
        return DIAG_FAILED;
    }

    /* Step 4: Attach LCD device to shared SPI2 bus */
    spi_device_handle_t spi = NULL;
    if (hal_spi2_add_lcd_device(&spi) != DIAG_PASSED) {
        ESP_LOGE(TAG, "LCD SPI device add failed");
        hal_spi2_bus_deinit();
        return DIAG_FAILED;
    }
    s_spi = spi;

    /* Step 5: Init ILI9342C chip driver through abstract transport */
    if (lcd_ILI9342C_init(&g_diag_spi_adapter, (void *)s_spi,
                           dc_callback, lcd_rst_callback) != 0) {
        ESP_LOGE(TAG, "ILI9342C init failed");
        hal_spi2_remove_lcd_device();
        hal_spi2_bus_deinit();
        s_spi = NULL;
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
    hal_spi2_remove_lcd_device();
    hal_spi2_bus_deinit();
    s_spi = NULL;
    s_initialised = false;
}

/* Drawing functions remain unchanged — they use the chip driver primitives */
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

/* Font data (s_font6x8) and text rendering remain unchanged */
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

void hal_screen_set_backlight(uint8_t b) { (void)b; }
int  hal_screen_width(void)  { return CONFIG_LCD_WIDTH; }
int  hal_screen_height(void) { return CONFIG_LCD_HEIGHT; }
