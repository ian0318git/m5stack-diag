/*
 * ltr553.h — LTR-553ALS-WA Proximity + Ambient Light Sensor (I2C)
 *
 * Common chip driver — platform-agnostic.
 *
 * Reference: Lite-On LTR-553ALS-WA datasheet
 * CoreS3: on optional camera flex cable, I2C@0x23
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

#define LTR553_ADDR             0x23

#define LTR553_REG_PART_ID      0x86
#define LTR553_PART_ID_VAL      0x93   /* Expected part ID */

#define LTR553_REG_ALS_CONTR    0x80   /* ALS control */
#define LTR553_ALS_CONTR_ON     (1 << 0)

#define LTR553_REG_ALS_CH1_0    0x88   /* ALS channel 1 (visible+IR) low byte */
#define LTR553_REG_ALS_CH1_1    0x89   /* ALS channel 1 high byte */
#define LTR553_REG_ALS_CH0_0    0x8A   /* ALS channel 0 (visible) low byte */
#define LTR553_REG_ALS_CH0_1    0x8B   /* ALS channel 0 high byte */

#define LTR553_REG_PROX_CONTR   0x81   /* Proximity control */
#define LTR553_PROX_CONTR_ON    (1 << 0)

#define LTR553_REG_PROX_DATA    0x8F   /* Proximity data (8-bit) */

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef struct {
    uint16_t als_ch0;    /* Visible light */
    uint16_t als_ch1;    /* Visible + IR */
    uint8_t  proximity;  /* Proximity (0–255) */
    uint8_t  part_id;
} ltr553_data_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Probe the LTR-553 sensor.
 *
 * Reads the part ID register.  Returns -1 if the device does not ACK
 * or the part ID is unexpected (caller should report SKIPPED since
 * the flex cable is an optional assembly).
 *
 * @param i2c   Abstract I2C transport.
 * @param bus   I2C device handle.
 * @return 0 on success, -1 on error.
 */
int ltr553_probe(const diag_i2c_t *i2c, void *bus);

/*===========================================================================*/
/* Data                                                                      */
/*===========================================================================*/

/**
 * @brief Read all sensor data (ALS + proximity).
 *
 * @param[out] data  Filled with sensor readings.
 * @return 0 on success, -1 on error.
 */
int ltr553_read_all(ltr553_data_t *data);

#ifdef __cplusplus
}
#endif
