/*
 * hal_audio.h — CoreS3 Audio HAL Interface
 *
 * Manages shared I2S bus (BCK=G34, WCK=G33, MCLK=G0) and
 * AW9523B GPIO lines for audio (AW_RST P0_2, AW_INT P1_3).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_core.h"
#include "driver/i2s_std.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* GPIO control (AW9523B for audio)                                          */
/*===========================================================================*/

diag_result_t hal_audio_gpio_init(void);
void          hal_audio_amp_reset(int level);
int           hal_audio_amp_int_read(void);

/*===========================================================================*/
/* I2S channel management                                                    */
/*===========================================================================*/

diag_result_t hal_audio_i2s_init_tx(i2s_chan_handle_t *handle);
diag_result_t hal_audio_i2s_init_rx(i2s_chan_handle_t *handle);
void          hal_audio_i2s_deinit(i2s_chan_handle_t handle);

#ifdef __cplusplus
}
#endif
