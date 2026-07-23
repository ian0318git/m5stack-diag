/*
 * hal_audio.c — CoreS3 I2S Audio HAL (shared by speaker + microphone)
 *
 * Manages the shared I2S bus (BCK=G34, WCK=G33, MCLK=G0).
 * Speaker uses TX (DATO=G14), microphone uses RX (DATI=G13).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_audio.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "aw9523b.h"
#include "aw88298.h"
#include "diag_config.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

static const char *TAG = "hal_audio";

/*===========================================================================*/
/* AW9523B GPIO expander init for audio control lines                       */
/*===========================================================================*/

static i2c_master_dev_handle_t s_aw9523b_dev = NULL;

diag_result_t hal_audio_gpio_init(void)
{
    if (s_aw9523b_dev) return DIAG_PASSED;

    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &s_aw9523b_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (aw9523b_init(&g_diag_i2c_adapter, (void *)s_aw9523b_dev) != 0) {
        return DIAG_FAILED;
    }

    /* P0_2 = AW_RST: GPIO mode, output, held in reset */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_SPK_RST);
    aw9523b_pin_set_direction(AW9523B_PIN_SPK_RST, 1);
    aw9523b_pin_write(AW9523B_PIN_SPK_RST, 0);

    /* P1_3 = AW_INT: GPIO mode, input */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_SPK_INT);
    aw9523b_pin_set_direction(AW9523B_PIN_SPK_INT, 0);

    ESP_LOGI(TAG, "Audio GPIO lines configured (AW_RST P0_2, AW_INT P1_3)");
    return DIAG_PASSED;
}

void hal_audio_amp_reset(int level)
{
    aw9523b_pin_write(AW9523B_PIN_SPK_RST, level);
}

int hal_audio_amp_int_read(void)
{
    int level = 0;
    aw9523b_pin_read(AW9523B_PIN_SPK_INT, &level);
    return level;
}

/*===========================================================================*/
/* I2S channel management                                                    */
/*===========================================================================*/

diag_result_t hal_audio_i2s_init_tx(i2s_chan_handle_t *handle)
{
    if (!handle) return DIAG_FAILED;

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        CONFIG_I2S_NUM, I2S_ROLE_MASTER);

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = 48000,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = CONFIG_I2S_MCLK_PIN,
            .bclk = CONFIG_I2S_BCK_PIN,
            .ws   = CONFIG_I2S_WCK_PIN,
            .dout = CONFIG_I2S_DATA_OUT_PIN,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };

    esp_err_t err = i2s_new_channel(&chan_cfg, handle, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S TX channel alloc failed: %d", err);
        return DIAG_FAILED;
    }

    err = i2s_channel_init_std_mode(*handle, &std_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S TX init failed: %d", err);
        i2s_del_channel(*handle);
        *handle = NULL;
        return DIAG_FAILED;
    }

    err = i2s_channel_enable(*handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S TX enable failed: %d", err);
        i2s_del_channel(*handle);
        *handle = NULL;
        return DIAG_FAILED;
    }

    ESP_LOGI(TAG, "I2S TX ready (48 kHz, 16-bit, mono)");
    return DIAG_PASSED;
}

diag_result_t hal_audio_i2s_init_rx(i2s_chan_handle_t *handle)
{
    if (!handle) return DIAG_FAILED;

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        CONFIG_I2S_NUM, I2S_ROLE_MASTER);

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = 48000,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = CONFIG_I2S_MCLK_PIN,
            .bclk = CONFIG_I2S_BCK_PIN,
            .ws   = CONFIG_I2S_WCK_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din  = CONFIG_I2S_DATA_IN_PIN,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };

    esp_err_t err = i2s_new_channel(&chan_cfg, NULL, handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX channel alloc failed: %d", err);
        return DIAG_FAILED;
    }

    err = i2s_channel_init_std_mode(*handle, &std_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX init failed: %d", err);
        i2s_del_channel(*handle);
        *handle = NULL;
        return DIAG_FAILED;
    }

    err = i2s_channel_enable(*handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S RX enable failed: %d", err);
        i2s_del_channel(*handle);
        *handle = NULL;
        return DIAG_FAILED;
    }

    ESP_LOGI(TAG, "I2S RX ready (48 kHz, 16-bit, stereo)");
    return DIAG_PASSED;
}

void hal_audio_i2s_deinit(i2s_chan_handle_t handle)
{
    if (!handle) return;
    i2s_channel_disable(handle);
    i2s_del_channel(handle);
}
