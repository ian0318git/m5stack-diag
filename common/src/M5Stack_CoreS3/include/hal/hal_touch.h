/*
 * hal_touch.h - Hardware Abstraction Layer: FT6336 Capacitive Touch
 *
 * Interface for the touch controller.  Application code depends on this
 * header, never on the concrete I2C driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/** A single touch point reported by the controller. */
typedef struct {
    uint16_t x;          /* X coordinate (0..240)                      */
    uint16_t y;          /* Y coordinate (0..240)                      */
    uint8_t  id;         /* Touch ID (0 or 1 for dual-touch)           */
    uint8_t  event;      /* 0 = down, 1 = up, 2 = contact, 3 = none   */
} hal_touch_point_t;

/** Snapshot of all touch state at one instant. */
typedef struct {
    uint8_t          point_count;    /* Number of valid points (0..2)  */
    hal_touch_point_t points[2];     /* Raw touch data                 */
} hal_touch_data_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the touch controller over I2C.
 * @return DIAG_PASSED on success, DIAG_FAILED if chip not responding.
 */
diag_result_t hal_touch_init(void);

/**
 * @brief Release touch resources.
 */
void hal_touch_deinit(void);

/*===========================================================================*/
/* Read                                                                      */
/*===========================================================================*/

/**
 * @brief Read current touch state from the controller.
 * @param[out] data  Filled with touch data.
 * @return DIAG_PASSED on successful read, DIAG_FAILED on I2C error.
 */
diag_result_t hal_touch_read(hal_touch_data_t *data);

/*===========================================================================*/
/* Info                                                                      */
/*===========================================================================*/

/**
 * @brief Return the firmware version of the FT6336.
 * @return Version byte, or 0 if unavailable.
 */
uint8_t hal_touch_firmware_version(void);

/**
 * @brief Return the maximum number of simultaneous touches supported.
 */
int hal_touch_max_points(void);

#ifdef __cplusplus
}
#endif
