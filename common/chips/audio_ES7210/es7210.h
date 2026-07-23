/*
 * es7210.h — ES7210 Audio ADC (Dual Microphone) Driver (I2C)
 *
 * Common chip driver — platform-agnostic.
 * Audio data is transferred via I2S (not covered by this driver).
 *
 * Reference: Everest ES7210 datasheet
 * CoreS3: dual microphones, I2S DATI=G13
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

#define ES7210_ADDR             0x40

#define ES7210_REG_RESET        0x00
#define ES7210_REG_CHIP_ID      0x01
#define ES7210_CHIP_ID_VAL      0x30   /* Expected chip ID */

#define ES7210_REG_MAIN_CTRL    0x02
#define ES7210_REG_ADC_CTRL     0x03
#define ES7210_REG_MIC1_L       0x04
#define ES7210_REG_MIC2_L       0x05
#define ES7210_REG_MIC1_R       0x06
#define ES7210_REG_MIC2_R       0x07
#define ES7210_REG_TIME_CTRL    0x08
#define ES7210_REG_AGC_CTRL     0x09
#define ES7210_REG_LOW_PWR      0x0A

/* MAIN_CTRL bits */
#define ES7210_MAIN_ENABLE      (1 << 0)
#define ES7210_MAIN_RESET       (1 << 1)

/* ADC_CTRL: sample rate */
#define ES7210_ADC_48KHZ        (0x00 << 2)

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef struct {
    uint8_t chip_id;
} es7210_info_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int  es7210_init(const diag_i2c_t *i2c, void *bus);
void es7210_deinit(void);

/*===========================================================================*/
/* Status                                                                    */
/*===========================================================================*/

uint8_t es7210_chip_id(void);
int     es7210_info(es7210_info_t *info);

#ifdef __cplusplus
}
#endif
