/*
 * gc0308.h — GC0308 0.3 MP Camera Sensor Driver (I2C)
 *
 * Common chip driver — platform-agnostic.
 * CAM_RST is controlled via AW9523B P1_0 (handled by HAL).
 * DVP bus data path is not covered by this driver.
 *
 * Reference: GalaxyCore GC0308 datasheet
 * CoreS3: on optional flex cable, I2C@0x21
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

#define GC0308_ADDR             0x21

#define GC0308_REG_CHIP_ID      0x00
#define GC0308_CHIP_ID_VAL      0x9d   /* Expected chip ID */

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Probe the GC0308 camera sensor.
 *
 * Reads the chip ID register.  Returns -1 if the device does not ACK
 * or the chip ID is unexpected (caller should report SKIPPED, not FAILED,
 * since the camera flex cable is an optional assembly).
 *
 * @param i2c   Abstract I2C transport.
 * @param bus   I2C device handle (opaque).
 * @return 0 on success (chip ID matches), -1 on error.
 */
int  gc0308_probe(const diag_i2c_t *i2c, void *bus);

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

void gc0308_deinit(void);

/*===========================================================================*/
/* Chip Info                                                                 */
/*===========================================================================*/

uint8_t gc0308_chip_id(void);

#ifdef __cplusplus
}
#endif
