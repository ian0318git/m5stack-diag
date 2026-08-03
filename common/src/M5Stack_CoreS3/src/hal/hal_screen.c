/*
 * hal_screen.c — CoreS3 board adapter for the ILI9342C display
 *
 * TRANSPORT: the LCD is driven by a GPIO bit-bang (SPI mode 0, CS held
 * low across each command+data sequence). This is the ONLY transport
 * verified to work on this panel — the ESP-IDF SPI master driver fails
 * on the CoreS3 (IOMUX-pins G36/G37, reserves G35, deasserts CS per
 * byte), and the working recipe was validated by the bit-bang lcdbb
 * test against M5Stack's m5gfx byte stream.
 *
 * Board-specific (CoreS3):
 *   - SPI: MOSI=G37, SCLK=G36, CS=G3, DC=G35 (also SD MISO — sequential)
 *   - RST via AW9523B P1_1 (needs GCR push-pull, reg 0x11 bit4)
 *   - Backlight via AXP2101 DLDO1 (reg 0x90 bit7) + SY7088 BOOST_EN
 *     (AW9523B P1_7); BLDO1 (reg 0x90 bit5) = LCD digital VDD
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_screen.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "hal_power.h"
#include "power_AXP2101.h"
#include "aw9523b.h"
#include "diag_config.h"
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "hal_screen";

static bool s_initialised = false;
static i2c_master_dev_handle_t s_aw9523b_dev = NULL;
static bool s_aw9523b_inited = false;

/*===========================================================================*/
/* AW9523B initialisation                                                     */
/*===========================================================================*/

static void lcd_rst_callback(int level)
{
    if (s_aw9523b_inited) {
        aw9523b_pin_write(AW9523B_PIN_LCD_RST, level);
    }
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

    /* AW9523B full configuration, replicating M5GFX's CoreS3 init.
     * CRITICAL: GCR (0x11) bit4 = push-pull output mode — without it
     * the port outputs are open-drain, so LCD RST (P1_1) writes of '1'
     * leave the pin high-Z and the panel stays in reset forever. */
    {
        uint8_t reg, val;
        static const uint8_t cfg[][2] = {
            { 0x04, 0x18 }, { 0x05, 0x0C },   /* CONFIG_P0/P1 (M5GFX)  */
            { 0x11, 0x10 },                   /* GCR: push-pull        */
            { 0x12, 0xFF }, { 0x13, 0xFF },   /* LEDMODE_P0/P1 (M5GFX) */
        };
        for (size_t i = 0; i < sizeof(cfg) / sizeof(cfg[0]); i++) {
            reg = cfg[i][0];
            val = cfg[i][1];
            i2c_master_transmit(s_aw9523b_dev, (uint8_t[]){ reg, val }, 2, -1);
        }
    }

    /* LCD_RST (P1_1): GPIO mode, output. NOT held low — the panel must
     * see RST high during power-up (the working recipe powers the panel
     * with RST released; the bring-up pulses it later). */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_LCD_RST);
    aw9523b_pin_set_direction(AW9523B_PIN_LCD_RST, 1);
    aw9523b_pin_write(AW9523B_PIN_LCD_RST, 1);

    s_aw9523b_inited = true;
    ESP_LOGI(TAG, "AW9523B LCD pins configured");
    return DIAG_PASSED;
}

/*===========================================================================*/
/* Backlight control via AXP2101 (DLDO1 + BLDO1 + ALDO1-4)                   */
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

    /* CoreS3 power rails (per M5Unified CoreS3 power init, 0x90 = 0xBF):
     *   1. AW9523B P1_7 = SY7088 BOOST_EN (must be high)
     *   2. AXP2101 reg 0x90 = 0xBF enables ALL LDOs:
     *      bit7 DLDO1 = LCD backlight (feeds the SY7088 boost)
     *      bit5 BLDO1 = LCD digital VDD  ← CRITICAL: without it the
     *      panel has no power and stays black despite the backlight.
     *      The AXP2101 POR default (0x0B) leaves BLDO1 off.
     *      reg 0x99   = DLDO1 voltage, value = (mV - 500) / 100
     */
    if (s_aw9523b_inited) {
        aw9523b_pin_set_gpio_mode(AW9523B_PIN_BOOST_EN);
        aw9523b_pin_set_direction(AW9523B_PIN_BOOST_EN, 1);
        aw9523b_pin_write(AW9523B_PIN_BOOST_EN, 1);
    } else {
        ESP_LOGW(TAG, "AW9523B not ready — SY7088 BOOST_EN not set");
    }

    uint8_t cmd[2] = { AXP2101_REG_LDO_EN, 0xBF };
    esp_err_t err = i2c_master_transmit(pmu, cmd, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 LDO enable (0xBF) failed: %d", err);
        return DIAG_FAILED;
    }

    /* LDO voltage registers (M5Unified CoreS3 values):
     * ALDO1 = 1.8 V (0x92), ALDO2/3/4 = 3.3 V (0x93-0x95) */
    static const uint8_t ldo_volts[][2] = {
        { 0x92, 0x0D }, { 0x93, 0x1C }, { 0x94, 0x1C }, { 0x95, 0x1C },
    };
    for (size_t i = 0; i < sizeof(ldo_volts) / sizeof(ldo_volts[0]); i++) {
        cmd[0] = ldo_volts[i][0];
        cmd[1] = ldo_volts[i][1];
        i2c_master_transmit(pmu, cmd, 2, -1);
    }

    cmd[0] = AXP2101_REG_DLDO1_VOLT;
    cmd[1] = AXP2101_DLDO1_VOLT_3V3;
    err = i2c_master_transmit(pmu, cmd, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 DLDO1 voltage write failed: %d", err);
        return DIAG_FAILED;
    }

    /* Read back to confirm the writes actually took effect */
    uint8_t r90 = 0, r99 = 0, reg = AXP2101_REG_LDO_EN;
    if (i2c_master_transmit_receive(pmu, &reg, 1, &r90, 1, -1) == ESP_OK) {
        reg = AXP2101_REG_DLDO1_VOLT;
        if (i2c_master_transmit_receive(pmu, &reg, 1, &r99, 1, -1) != ESP_OK) {
            r99 = 0xFF;
        }
        ESP_LOGI(TAG, "LDO read-back: reg 0x90 = 0x%02X (DLDO1=%d BLDO1=%d), "
                      "reg 0x99 = 0x%02X", r90, (r90 & 0x80) ? 1 : 0,
                      (r90 & 0x20) ? 1 : 0, r99);
        if (r90 != 0xBF || r99 != AXP2101_DLDO1_VOLT_3V3) {
            ESP_LOGW(TAG, "LDO read-back mismatch — writes may not stick");
        }
    }

    return DIAG_PASSED;
}

/*===========================================================================*/
/* Bit-bang SPI transport (SPI mode 0, CS held low per sequence)             */
/*===========================================================================*/

static void bb_delay(void)
{
    esp_rom_delay_us(1);   /* ~500 kHz */
}

static void bb_byte(uint8_t b)
{
    for (int i = 7; i >= 0; i--) {
        gpio_set_level(CONFIG_LCD_MOSI_PIN, (b >> i) & 1);
        gpio_set_level(CONFIG_LCD_SCLK_PIN, 1);
        bb_delay();
        gpio_set_level(CONFIG_LCD_SCLK_PIN, 0);
        bb_delay();
    }
}

static void bb_cs_low(uint8_t dc, const uint8_t *data, size_t len)
{
    gpio_set_level(CONFIG_LCD_DC_PIN, dc);
    for (size_t i = 0; i < len; i++) bb_byte(data[i]);
}

static void bb_cmd(uint8_t c)              { bb_cs_low(0, &c, 1); }
static void bb_cmd_data(uint8_t c, const uint8_t *d, size_t n)
{
    bb_cmd(c);
    bb_cs_low(1, d, n);
}

static void bb_pins_takeover(void)
{
    /* G36 (FSPICLK) / G37 (FSPIQ) may be IOMUX-pinned by a prior SD
     * session — force them back to GPIO function. */
    PIN_FUNC_SELECT(IO_MUX_GPIO36_REG, PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(IO_MUX_GPIO37_REG, PIN_FUNC_GPIO);
    gpio_set_direction(CONFIG_LCD_CS_PIN,   GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_LCD_DC_PIN,   GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_LCD_SCLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_LCD_MOSI_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
    gpio_set_level(CONFIG_LCD_DC_PIN, 1);
    gpio_set_level(CONFIG_LCD_SCLK_PIN, 0);
    gpio_set_level(CONFIG_LCD_MOSI_PIN, 0);
}

static void bb_set_window(uint16_t x0, uint16_t y0,
                          uint16_t x1, uint16_t y1)
{
    uint8_t col[4] = { (uint8_t)(x0 >> 8), (uint8_t)x0,
                       (uint8_t)(x1 >> 8), (uint8_t)x1 };
    uint8_t row[4] = { (uint8_t)(y0 >> 8), (uint8_t)y0,
                       (uint8_t)(y1 >> 8), (uint8_t)y1 };
    bb_cmd_data(0x2A, col, 4);
    bb_cmd_data(0x2B, row, 4);
    bb_cmd(0x2C);
}

/* Fill `count` pixels with one colour (big-endian RGB565) */
static void bb_fill_chunk(const uint16_t colour, size_t count)
{
    uint8_t buf[256];
    uint8_t hi = (uint8_t)(colour >> 8), lo = (uint8_t)(colour & 0xFF);
    for (size_t i = 0; i < 128; i++) {
        buf[2 * i] = hi;
        buf[2 * i + 1] = lo;
    }
    while (count > 0) {
        size_t n = (count > 128) ? 128 : count;
        bb_cs_low(1, buf, n * 2);
        count -= n;
    }
}

/* Panel bring-up — the exact sequence validated by the bit-bang lcdbb
 * test (m5gfx Panel_ILI9342 list0 + M5GFX begin order). */
static void bb_lcd_init(void)
{
    /* RST pulse via AW9523B P1_1 */
    lcd_rst_callback(0);
    esp_rom_delay_us(20000);
    lcd_rst_callback(1);
    esp_rom_delay_us(64000);

    /* Init list with CS held low (m5gfx startWrite/endWrite behaviour) */
    gpio_set_level(CONFIG_LCD_CS_PIN, 0);
    bb_cmd_data(0xC8, (uint8_t[]){0xFF, 0x93, 0x42}, 3);  /* SETEXTC unlock */
    bb_cmd_data(0xC0, (uint8_t[]){0x12, 0x12}, 2);        /* PWCTR1 */
    bb_cmd_data(0xC1, (uint8_t[]){0x03}, 1);              /* PWCTR2 */
    bb_cmd_data(0xC5, (uint8_t[]){0xF2}, 1);              /* VMCTR1 */
    bb_cmd_data(0xB0, (uint8_t[]){0xE0}, 1);
    bb_cmd_data(0xF6, (uint8_t[]){0x01, 0x00, 0x00}, 3);
    bb_cmd_data(0xE0, (uint8_t[]){0x00,0x0C,0x11,0x04,0x11,0x08,0x37,0x89,
                                  0x4C,0x06,0x0C,0x0A,0x2E,0x34,0x0F}, 15);
    bb_cmd_data(0xE1, (uint8_t[]){0x00,0x0B,0x11,0x05,0x13,0x09,0x33,0x67,
                                  0x48,0x07,0x0E,0x0B,0x2E,0x33,0x0F}, 15);
    bb_cmd_data(0xB6, (uint8_t[]){0x08, 0x82, 0x1D, 0x04}, 4);  /* DFUNCTR */
    bb_cmd(0x38);                                               /* IDMOFF */
    bb_cmd(0x29);                                               /* DISPON */
    bb_cmd(0x11);                                               /* SLPOUT */
    esp_rom_delay_us(120000);
    bb_cmd_data(0x3A, (uint8_t[]){0x55}, 1);                    /* COLMOD */
    bb_cmd_data(0x36, (uint8_t[]){0x48}, 1);                    /* MADCTL MX|BGR */
    bb_cmd(0x21);                                               /* INVON */
    esp_rom_delay_us(10000);
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

diag_result_t hal_screen_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    /* Step 1: Init AW9523B for LCD RST + BOOST_EN */
    if (gpio_exp_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "GPIO expander init failed");
        return DIAG_FAILED;
    }

    /* Step 2: Enable the power rails (backlight + LCD VDD) */
    if (backlight_init() != DIAG_PASSED) {
        ESP_LOGW(TAG, "Backlight init failed — display may be dark");
    }

    /* Step 3: Let the panel's VDD stabilise (verified requirement —
     * the working recipe waits 500 ms after 0xBF before touching the
     * panel; without it the panel stays black). */
    vTaskDelay(pdMS_TO_TICKS(500));

    /* Step 4: Bit-bang bring-up (proven recipe) */
    bb_pins_takeover();
    bb_lcd_init();

    s_initialised = true;
    ESP_LOGI(TAG, "CoreS3 screen HAL ready (ILI9342C)");
    return DIAG_PASSED;
}

/* The panel blanks ~1 s after the last SPI activity (a sleep/quirk of
 * this ILI9342C unit). DISPON alone does not wake it — it needs the
 * full SLPOUT + wait + DISPON sequence. */
void hal_screen_keepalive(void)
{
    if (!s_initialised) return;
    gpio_set_level(CONFIG_LCD_CS_PIN, 0);
    bb_cmd(0x11);               /* SLPOUT — wake from sleep */
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
    esp_rom_delay_us(120000);
    gpio_set_level(CONFIG_LCD_CS_PIN, 0);
    bb_cmd(0x29);               /* DISPON */
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
}

void hal_screen_deinit(void)
{
    if (!s_initialised) return;
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);   /* CS released */
    lcd_rst_callback(1);                     /* RST high (panel stays out of reset) */
    s_initialised = false;
    /* Reset the AW9523B guard so the next init re-runs the FULL
     * configuration (GCR/LEDMODE/CONFIG) — the lcdbb recipe re-writes
     * everything before every draw, and skipping the AW9523B config
     * leaves the panel deaf after the first fill. */
    s_aw9523b_inited = false;
}

/*===========================================================================*/
/* Drawing — bit-bang primitives with CS-hold per draw                       */
/*===========================================================================*/

/* The panel accepts at most ~19200 pixels (~60 rows) per bring-up
 * session before going deaf. Fill the full screen as 60-row strips,
 * each with its own bring-up. */
#define BB_STRIP_ROWS 60

void hal_screen_fill(hal_screen_colour_t colour)
{
    for (int strip = 0; strip < CONFIG_LCD_HEIGHT; strip += BB_STRIP_ROWS) {
        /* Re-bring-up before each strip (tear down resets the guards) */
        hal_screen_deinit();
        if (hal_screen_init() != DIAG_PASSED) return;

        gpio_set_level(CONFIG_LCD_CS_PIN, 0);   /* CS-asserted draw window */
        bb_set_window(0, (uint16_t)strip,
                      CONFIG_LCD_WIDTH - 1,
                      (uint16_t)(strip + BB_STRIP_ROWS - 1));
        bb_fill_chunk(colour,
                      CONFIG_LCD_WIDTH * BB_STRIP_ROWS);
        gpio_set_level(CONFIG_LCD_CS_PIN, 1);
    }
}

void hal_screen_draw_pixel(int x, int y, hal_screen_colour_t colour)
{
    if (x < 0 || x >= CONFIG_LCD_WIDTH || y < 0 || y >= CONFIG_LCD_HEIGHT) return;
    gpio_set_level(CONFIG_LCD_CS_PIN, 0);   /* CS-asserted draw window */
    bb_set_window((uint16_t)x, (uint16_t)y, (uint16_t)x, (uint16_t)y);
    uint8_t b[2] = { (uint8_t)(colour >> 8), (uint8_t)(colour & 0xFF) };
    bb_cs_low(1, b, 2);
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
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

    gpio_set_level(CONFIG_LCD_CS_PIN, 0);   /* CS-asserted draw window */
    bb_set_window((uint16_t)x, (uint16_t)y,
                  (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));
    bb_fill_chunk(colour, (size_t)w * h);
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
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

/* Font data (s_font6x8) and text rendering unchanged */
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
    /* Scale the 6x8 glyph into the font cell (size 1: 1x2, size 2: 2x3) */
    int sx = fw / 6, sy = fh / 8;
    if (sx < 1) sx = 1;
    if (sy < 1) sy = 1;
    int cx = x;

    while (*text) {
        unsigned char ch = (unsigned char)*text;
        if (ch == '\n') { y += fh; cx = x; text++; continue; }
        if (ch < 0x20 || ch > 0x7E) { cx += fw; text++; continue; }

        const uint8_t *g = s_font6x8[ch - 0x20];
        for (int col = 0; col < 6; col++) {
            uint8_t bits = g[col];
            for (int row = 0; row < 8; row++) {
                hal_screen_colour_t c = (bits & (1 << row)) ? fg : bg;
                for (int dy = 0; dy < sy; dy++)
                    for (int dx = 0; dx < sx; dx++)
                        hal_screen_draw_pixel(cx + col * sx + dx,
                                              y + row * sy + dy, c);
            }
        }
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
