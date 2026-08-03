/*
 * hal_spi2_bus.c — Shared SPI2 Bus Manager (LCD + SD card)
 *
 * CoreS3 shares SPI2 (MOSI=G37, MISO=G35, SCK=G36) between the
 * ILI9342C LCD (CS=G3) and the microSD card slot (CS=G4).
 *
 * IMPORTANT: G35 is BOTH the LCD D/C line and the SD card MISO line.
 * The bus is therefore initialised with or without MISO depending on
 * which peripheral is active (LCD: no MISO, G35 stays a plain GPIO
 * output driving D/C — this is exactly how the working reference
 * CoreS3_Guard project configures it; SD: with MISO=G35). The tests
 * are strictly sequential, so each side re-initialises the bus.
 *
 * This module uses a reference count so both peripherals can share
 * the bus without double-initialising or causing contention.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_spi2_bus.h"
#include "diag_config.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hal_spi2_bus";

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static int s_refcount = 0;
static spi_device_handle_t s_lcd_spi = NULL;
static sdspi_dev_handle_t  s_sd_handle = -1;

/*===========================================================================*/
/* Bus lifecycle                                                             */
/*===========================================================================*/

diag_result_t hal_spi2_bus_init(bool with_miso)
{
    if (s_refcount > 0) {
        s_refcount++;
        return DIAG_PASSED;
    }

    spi_bus_config_t bus_cfg = {
        .mosi_io_num     = CONFIG_LCD_MOSI_PIN,
        /* with_miso=false (LCD): G35 stays as the D/C GPIO output.
         * If MISO were claimed here, the ESP-IDF SPI driver reserves
         * G35 and reconfigures it, silently killing the D/C line. */
        .miso_io_num     = with_miso ? CONFIG_LCD_MISO_PIN : -1,
        .sclk_io_num     = CONFIG_LCD_SCLK_PIN,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = CONFIG_LCD_WIDTH * CONFIG_LCD_HEIGHT * 2 + 8,
    };

    esp_err_t err = spi_bus_initialize(CONFIG_LCD_SPI_NUM, &bus_cfg,
                                       SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI2 bus init failed: %d", err);
        return DIAG_FAILED;
    }

    s_refcount = 1;
    ESP_LOGI(TAG, "SPI2 bus initialised (ref=1)");
    return DIAG_PASSED;
}

void hal_spi2_bus_deinit(void)
{
    if (s_refcount <= 0) return;

    s_refcount--;
    if (s_refcount > 0) {
        ESP_LOGD(TAG, "SPI2 bus ref-- (%d remaining)", s_refcount);
        return;
    }

    /* Last user: free the bus */
    spi_bus_free(CONFIG_LCD_SPI_NUM);
    s_refcount = 0;
    ESP_LOGI(TAG, "SPI2 bus freed");
}

/*===========================================================================*/
/* LCD device                                                                */
/*===========================================================================*/

diag_result_t hal_spi2_add_lcd_device(spi_device_handle_t *handle)
{
    if (!handle) return DIAG_FAILED;
    if (s_lcd_spi) {
        *handle = s_lcd_spi;
        return DIAG_PASSED;
    }

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = CONFIG_LCD_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = CONFIG_LCD_CS_PIN,
        .queue_size     = 1,
        .flags          = SPI_DEVICE_HALFDUPLEX,
    };

    esp_err_t err = spi_bus_add_device(CONFIG_LCD_SPI_NUM, &dev_cfg,
                                       &s_lcd_spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD SPI device add failed: %d", err);
        return DIAG_FAILED;
    }

    *handle = s_lcd_spi;
    ESP_LOGI(TAG, "LCD SPI device added (CS=G3)");
    return DIAG_PASSED;
}

void hal_spi2_remove_lcd_device(void)
{
    if (!s_lcd_spi) return;
    spi_bus_remove_device(s_lcd_spi);
    s_lcd_spi = NULL;
}

/*===========================================================================*/
/* SD card device (via sdspi_host)                                           */
/*===========================================================================*/

diag_result_t hal_spi2_add_sd_device(sdspi_dev_handle_t *out_handle)
{
    if (!out_handle) return DIAG_FAILED;
    if (s_sd_handle >= 0) {
        *out_handle = s_sd_handle;
        return DIAG_PASSED;
    }

    /* sdspi_host_init() is a no-op in IDF v6.0 — bus already inited above */
    sdspi_host_init();

    sdspi_device_config_t slot_cfg = {
        .host_id   = CONFIG_LCD_SPI_NUM,
        .gpio_cs   = CONFIG_SD_CS_PIN,
        .gpio_cd   = SDSPI_SLOT_NO_CD,
        .gpio_wp   = SDSPI_SLOT_NO_WP,
        .gpio_int  = SDSPI_SLOT_NO_INT,
        .gpio_wp_polarity = SDSPI_IO_ACTIVE_LOW,
        .duty_cycle_pos = 0,
        .wait_for_miso = 0,
    };

    esp_err_t err = sdspi_host_init_device(&slot_cfg, &s_sd_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SD SPI device add failed: %d", err);
        return DIAG_FAILED;
    }

    *out_handle = s_sd_handle;
    ESP_LOGI(TAG, "SD SPI device added (CS=G4, handle=%d)", s_sd_handle);
    return DIAG_PASSED;
}

void hal_spi2_remove_sd_device(void)
{
    if (s_sd_handle < 0) return;
    sdspi_host_remove_device(s_sd_handle);
    s_sd_handle = -1;
}
