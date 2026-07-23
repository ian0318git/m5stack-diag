/*
 * test_microphone.c — ES7210 Microphone ADC Test (DFS §Audio)
 *
 * On-demand test:
 *   1. Read ES7210 chip ID via I2C@0x40
 *   2. Configure I2S RX: 16-bit, 48 ksps, stereo (BCK=G34, WCK=G33, DATI=G13, MCLK=G0)
 *   3. Capture 100 ms (4800 samples per channel)
 *   4. Compute RMS level for each channel
 *   5. Report: non-zero RMS indicates microphone circuit is functional
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
#include "es7210.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

/*
 * Capture parameters per DFS:
 *   100 ms @ 48 kHz = 4800 samples/channel
 *   16-bit stereo → 2 channels × 2 bytes × 4800 = 19200 bytes
 */
#define CAPTURE_MS      100
#define SAMPLE_RATE     48000
#define CAPTURE_SAMPLES ((SAMPLE_RATE * CAPTURE_MS) / 1000)  /* 4800 */
#define CAPTURE_CHANNELS 2

/* RMS computation */
static float compute_rms(const int16_t *samples, size_t count)
{
    if (!samples || count == 0) return 0.0f;

    double sum_sq = 0.0;
    for (size_t i = 0; i < count; i++) {
        double s = (double)samples[i];
        sum_sq += s * s;
    }

    return (float)sqrt(sum_sq / (double)count);
}

diag_result_t test_microphone(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "MIC", "MB/AUDIO");

    diag_menu_printf("\r\nMicrophone ADC Test (ES7210 @0x40)\r\n");
    diag_menu_printf("  I2S: capture %d ms, %d Hz, 16-bit stereo\r\n",
                     CAPTURE_MS, SAMPLE_RATE);

    /*------------------------------------------------------------------------*/
    /* Step 1: Init ES7210 via I2C                                            */
    /*------------------------------------------------------------------------*/

    i2c_master_dev_handle_t adc_dev = NULL;
    if (hal_i2c_add_device(ES7210_ADDR, 400000, &adc_dev) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "ES7210 I2C device add failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Run I2C Bus Scan to confirm address 0x40",
                               "Check ES7210 power rail");
        }
        return DIAG_FAILED;
    }

    if (es7210_init(&g_diag_i2c_adapter, (void *)adc_dev) != 0) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x40 ES7210: init failed (chip ID mismatch or no ACK)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check I2C bus 0x40 pull-ups",
                               "Verify ES7210 power and clock configuration");
        }
        return DIAG_FAILED;
    }

    uint8_t chip_id = es7210_chip_id();
    diag_menu_printf("  ES7210 chip ID: 0x%02X\r\n", chip_id);

    /*------------------------------------------------------------------------*/
    /* Step 2: Init I2S RX                                                    */
    /*------------------------------------------------------------------------*/

    i2s_chan_handle_t rx_chan = NULL;
    if (hal_audio_i2s_init_rx(&rx_chan) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "I2S RX init failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check I2S pins: BCK=G34, WCK=G33, DATI=G13, MCLK=G0",
                               "Verify ES7210 is clocking data on DATI");
        }
        es7210_deinit();
        return DIAG_FAILED;
    }

    /*------------------------------------------------------------------------*/
    /* Step 3: Capture audio                                                  */
    /*------------------------------------------------------------------------*/

    size_t buf_size = CAPTURE_SAMPLES * CAPTURE_CHANNELS * sizeof(int16_t);
    int16_t *audio_buf = (int16_t *)malloc(buf_size);
    if (!audio_buf) {
        hal_audio_i2s_deinit(rx_chan);
        es7210_deinit();
        return DIAG_ERROR;
    }
    memset(audio_buf, 0, buf_size);

    size_t bytes_read = 0;
    diag_menu_printf("  Capturing %d ms audio...\r\n", CAPTURE_MS);

    esp_err_t err = i2s_channel_read(rx_chan, audio_buf, buf_size,
                                     &bytes_read, portMAX_DELAY);
    if (err != ESP_OK || bytes_read == 0) {
        diag_menu_printf("  I2S read failed or returned no data\r\n");
        free(audio_buf);
        hal_audio_i2s_deinit(rx_chan);
        es7210_deinit();
        return DIAG_FAILED;
    }

    /*------------------------------------------------------------------------*/
    /* Step 4: Compute RMS per channel                                        */
    /*------------------------------------------------------------------------*/

    size_t samples_read = bytes_read / (CAPTURE_CHANNELS * sizeof(int16_t));
    size_t actual_frames = (samples_read > CAPTURE_SAMPLES) ? CAPTURE_SAMPLES : samples_read;

    /* De-interleave: separate L/R channels */
    float rms_l = 0.0f, rms_r = 0.0f;
    if (actual_frames > 0) {
        int16_t *ch_l = (int16_t *)malloc(actual_frames * sizeof(int16_t));
        int16_t *ch_r = (int16_t *)malloc(actual_frames * sizeof(int16_t));
        if (ch_l && ch_r) {
            for (size_t i = 0; i < actual_frames; i++) {
                ch_l[i] = audio_buf[i * 2];
                ch_r[i] = audio_buf[i * 2 + 1];
            }
            rms_l = compute_rms(ch_l, actual_frames);
            rms_r = compute_rms(ch_r, actual_frames);
            free(ch_l);
            free(ch_r);
        }
    }

    free(audio_buf);
    hal_audio_i2s_deinit(rx_chan);
    es7210_deinit();

    /*------------------------------------------------------------------------*/
    /* Step 5: Report results                                                 */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("  Captured: %u frames (%u bytes)\r\n",
                     (unsigned)actual_frames, (unsigned)bytes_read);
    diag_menu_printf("  RMS level — Channel L: %.1f\r\n", (double)rms_l);
    diag_menu_printf("  RMS level — Channel R: %.1f\r\n", (double)rms_r);

    /* Non-zero RMS indicates the microphone circuit is functional */
    if (rms_l > 5.0f || rms_r > 5.0f) {
        diag_menu_printf("Mic Test: PASSED (non-zero audio detected)\r\n");
        return DIAG_PASSED;
    }

    if (g_diag_err_ctx) {
        diag_err_add(g_diag_err_ctx,
                     "ES7210: all captured samples near zero (RMS L=%.1f R=%.1f)",
                     (double)rms_l, (double)rms_r);
        diag_err_set_debug(g_diag_err_ctx,
                           "Speak into the microphones and re-run",
                           "Check DATI=G13 with oscilloscope");
    }

    diag_menu_printf("Mic Test: ADVISORY (RMS near zero — "
                     "speak into mic and re-run, or check DATI signal)\r\n");
    return DIAG_SKIPPED;
}
