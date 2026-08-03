/*
 * hal_spi_adapter.c — Bit-bang SPI adapter for the CoreS3 ILI9342C LCD
 *
 * The ESP-IDF SPI master driver cannot drive this panel reliably on the
 * CoreS3: it IOMUX-pins G36/G37, reserves G35 (shared MISO/D-C), and
 * deasserts CS per transaction, all of which the ILI9342C rejects.
 * The proven-working recipe (verified by the bit-bang lcdbb test against
 * M5Stack's m5gfx stream) is a GPIO bit-bang with CS held low across
 * the whole command+data sequence — implemented here.
 *
 * ADAPTER layer — implements the abstract diag_spi_t interface.
 * The 'bus' handle is unused (the pins are fixed CoreS3 pins).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_transport.h"
#include "diag_config.h"
#include "hal/gpio_ll.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

static const char *TAG = "spi_adapter";

static bool s_cs_asserted = false;

/* ~500 kHz bit clock */
static void bb_delay(void)
{
    esp_rom_delay_us(1);
}

/* SPI mode 0: sample on rising edge, SCK idles low */
static void bb_byte(uint8_t b)
{
    for (int i = 7; i >= 0; i--) {
        gpio_ll_set_level(&GPIO, CONFIG_LCD_MOSI_PIN, (b >> i) & 1);
        gpio_ll_set_level(&GPIO, CONFIG_LCD_SCLK_PIN, 1);
        bb_delay();
        gpio_ll_set_level(&GPIO, CONFIG_LCD_SCLK_PIN, 0);
        bb_delay();
    }
}

/* Pin take-over. Uses low-level register writes (not gpio_config) so it
 * works even when the ESP-IDF SPI driver has reserved/pinned the pins
 * (e.g. after an SD card session claimed MISO=G35 and IOMUX-pinned
 * G36/G37). */
static void lcd_pins_takeover(void)
{
    /* G36 (FSPICLK) / G37 (FSPIQ): back to GPIO function via IOMUX */
    PIN_FUNC_SELECT(IO_MUX_GPIO36_REG, PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(IO_MUX_GPIO37_REG, PIN_FUNC_GPIO);

    /* G3 (CS) and G35 (D/C, shared MISO): GPIO output via the matrix */
    gpio_ll_set_output_signal_matrix_source(&GPIO, CONFIG_LCD_CS_PIN,
                                            SIG_GPIO_OUT_IDX, false);
    gpio_ll_output_enable(&GPIO, CONFIG_LCD_CS_PIN);
    gpio_ll_set_output_signal_matrix_source(&GPIO, CONFIG_LCD_DC_PIN,
                                            SIG_GPIO_OUT_IDX, false);
    gpio_ll_output_enable(&GPIO, CONFIG_LCD_DC_PIN);

    gpio_ll_set_level(&GPIO, CONFIG_LCD_CS_PIN, 1);
    gpio_ll_set_level(&GPIO, CONFIG_LCD_DC_PIN, 1);
    gpio_ll_set_level(&GPIO, CONFIG_LCD_SCLK_PIN, 0);
    gpio_ll_set_level(&GPIO, CONFIG_LCD_MOSI_PIN, 0);
}

/*===========================================================================*/
/* Adapter callbacks                                                         */
/*===========================================================================*/

static int spi_adapter_begin(void *bus)
{
    (void)bus;
    lcd_pins_takeover();
    gpio_ll_set_level(&GPIO, CONFIG_LCD_CS_PIN, 0);   /* CS asserted */
    s_cs_asserted = true;
    return 0;
}

static int spi_adapter_end(void *bus)
{
    (void)bus;
    gpio_ll_set_level(&GPIO, CONFIG_LCD_CS_PIN, 1);   /* CS released */
    s_cs_asserted = false;
    return 0;
}

static int spi_adapter_transmit(void *bus, const void *data, size_t len)
{
    (void)bus;
    if (!s_cs_asserted) {
        ESP_LOGW(TAG, "transmit without begin() — asserting CS");
        spi_adapter_begin(NULL);
    }
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        bb_byte(p[i]);
    }
    return 0;
}

static int spi_adapter_transfer(void *bus, const void *tx_data,
                                 void *rx_data, size_t len)
{
    (void)bus; (void)tx_data; (void)rx_data; (void)len;
    return -1;   /* full-duplex not supported by the bit-bang adapter */
}

/*===========================================================================*/
/* Public singleton                                                          */
/*===========================================================================*/

const diag_spi_t g_diag_spi_adapter = {
    .transmit = spi_adapter_transmit,
    .transfer = spi_adapter_transfer,
    .begin    = spi_adapter_begin,
    .end      = spi_adapter_end,
};
