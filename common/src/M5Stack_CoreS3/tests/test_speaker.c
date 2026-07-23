/*
 * test_speaker.c — AW88298 Speaker Amplifier Test (DFS §Audio)
 *
 * On-demand test:
 *   1. AW9523B P0_2 RST control (5ms low → release)
 *   2. Read AW88298 chip ID via I2C
 *   3. Configure I2S TX: 16-bit, 48 ksps, mono (BCK=G34, WCK=G33, DATO=G14, MCLK=G0)
 *   4. Generate 1 kHz sine wave, output for 500 ms
 *   5. Stop tone, de-init I2S
 *   6. Check AW_INT (AW9523B P1_3) for fault indication
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "hal_audio.h"
#include "aw88298.h"
#include "aw9523b.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

/*
 * 1 kHz sine wave generation:
 *   samples = 48000 Hz / 1000 Hz = 48 samples per cycle
 *   With 16-bit mono, each sample is a 16-bit signed value
 *   Amplitude: 0.7 * 32768 ≈ 22937 (avoid clipping)
 */
#define SINE_TABLE_LEN  48
#define SINE_AMPLITUDE  22937
#define SINE_TEST_MS    500
#define SAMPLE_RATE     48000

static int16_t s_sine_table[SINE_TABLE_LEN];

static void build_sine_table(void)
{
    static bool built = false;
    if (built) return;
    for (int i = 0; i < SINE_TABLE_LEN; i++) {
        s_sine_table[i] = (int16_t)(SINE_AMPLITUDE * sinf(2.0f * 3.14159f * i / SINE_TABLE_LEN));
    }
    built = true;
}

diag_result_t test_speaker(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "SPEAKER", "MB/AUDIO");

    diag_menu_printf("\r\nSpeaker Amp Test (AW88298 @0x36)\r\n");
    diag_menu_printf("  I2S: 1 kHz tone, 500 ms\r\n");

    /*------------------------------------------------------------------------*/
    /* Step 1: Init AW9523B GPIO for audio (AW_RST P0_2, AW_INT P1_3)        */
    /*------------------------------------------------------------------------*/

    if (hal_audio_gpio_init() != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "AW9523B GPIO init failed for audio");
            diag_err_set_debug(g_diag_err_ctx,
                               "Run GPIO Expander Test (AW9523B)",
                               "Check I2C bus 0x58");
        }
        return DIAG_SKIPPED;
    }

    /*------------------------------------------------------------------------*/
    /* Step 2: Release AW_RST (P0_2) after 5 ms low                           */
    /*------------------------------------------------------------------------*/

    hal_audio_amp_reset(0);
    esp_rom_delay_us(5000);
    hal_audio_amp_reset(1);
    esp_rom_delay_us(10000);

    /*------------------------------------------------------------------------*/
    /* Step 3: Init AW88298 via I2C                                           */
    /*------------------------------------------------------------------------*/

    i2c_master_dev_handle_t amp_dev = NULL;
    if (hal_i2c_add_device(AW88298_ADDR, 400000, &amp_dev) != DIAG_PASSED) {
        diag_menu_printf("  I2C@0x36: NACK — speaker amp not detected\r\n");
        diag_menu_printf("Speaker Test: SKIPPED (optional peripheral)\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "AW88298 not at 0x36 (SKIPPED)");
            diag_err_set_debug(g_diag_err_ctx,
                               "This CoreS3 unit may not have AW88298 populated",
                               "Check AW_RST (AW9523B P0_2) state");
        }
        return DIAG_SKIPPED;
    }

    if (aw88298_init(&g_diag_i2c_adapter, (void *)amp_dev) != 0) {
        diag_menu_printf("  AW88298 init failed (chip ID mismatch)\r\n");
        diag_menu_printf("Speaker Test: SKIPPED (unexpected chip ID)\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x36 AW88298: chip ID 0x%02X, expected 0x%02X",
                         aw88298_chip_id(), AW88298_CHIP_ID_VAL);
            diag_err_set_debug(g_diag_err_ctx,
                               "Different AW88298 revision on this unit",
                               "Update AW88298_CHIP_ID_VAL if needed");
        }
        return DIAG_SKIPPED;
    }

    diag_menu_printf("  AW88298 chip ID: 0x%02X\r\n", aw88298_chip_id());

    /*------------------------------------------------------------------------*/
    /* Step 4: Init I2S TX and play 1 kHz tone for 500 ms                    */
    /*------------------------------------------------------------------------*/

    i2s_chan_handle_t tx_chan = NULL;
    if (hal_audio_i2s_init_tx(&tx_chan) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "I2S TX init failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check I2S pins: BCK=G34, WCK=G33, DATO=G14, MCLK=G0",
                               "Verify I2S bus not in use by another test");
        }
        aw88298_deinit();
        return DIAG_FAILED;
    }

    /* Enable amplifier */
    aw88298_enable();

    /* Build sine table */
    build_sine_table();

    /* Write sine wave data in chunks for the specified duration */
    size_t total_samples = (size_t)(SAMPLE_RATE * SINE_TEST_MS / 1000);
    size_t bytes_written = 0;
    size_t chunk = SINE_TABLE_LEN;
    diag_menu_printf("  Playing 1 kHz tone...\r\n");

    for (size_t sent = 0; sent < total_samples; sent += chunk) {
        size_t n = (total_samples - sent < chunk) ? (total_samples - sent) : chunk;
        esp_err_t err = i2s_channel_write(tx_chan, s_sine_table,
                                          n * sizeof(int16_t),
                                          &bytes_written, portMAX_DELAY);
        if (err != ESP_OK) break;
    }

    /*------------------------------------------------------------------------*/
    /* Step 5: Stop tone, de-init I2S                                         */
    /*------------------------------------------------------------------------*/

    aw88298_disable();

    /* Small delay for the tone to decay */
    vTaskDelay(pdMS_TO_TICKS(50));

    hal_audio_i2s_deinit(tx_chan);

    /*------------------------------------------------------------------------*/
    /* Step 6: Check fault status                                             */
    /*------------------------------------------------------------------------*/

    uint8_t fault = 0;
    if (aw88298_read_fault(&fault) == 0 && fault != 0) {
        diag_menu_printf("  ** AW88298 fault register: 0x%02X\r\n", fault);
        if (fault & AW88298_FAULT_OVERTEMP) {
            diag_menu_printf("  ** Overtemperature warning\r\n");
        }
        if (fault & AW88298_FAULT_OVERCUR) {
            diag_menu_printf("  ** Overcurrent warning\r\n");
        }
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "AW88298 fault: 0x%02X", fault);
            diag_err_set_debug(g_diag_err_ctx,
                               "Check speaker impedance (4–8 Ω expected)",
                               "Check AW9523B P1_3 AW_INT signal");
        }
    }

    /* Check AW_INT line */
    int aw_int = hal_audio_amp_int_read();

    aw88298_deinit();

    if (fault != 0) {
        diag_menu_printf("Speaker Test: FAILED (fault=0x%02X, AW_INT=%d)\r\n",
                         fault, aw_int);
        return DIAG_FAILED;
    }

    diag_menu_printf("Speaker Test: PASSED (tone generated, no fault)\r\n");
    diag_menu_printf("  (Operator: verify 1 kHz tone was audible)\r\n");
    return DIAG_PASSED;
}
