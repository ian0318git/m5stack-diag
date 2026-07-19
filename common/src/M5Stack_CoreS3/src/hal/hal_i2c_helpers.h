/*
 * hal_i2c_helpers.h - Shared I2C bus helper for CoreS3 HAL modules
 *
 * All on-board peripherals share one I2C bus.  This internal helper
 * provides a single initialisation point so each HAL driver does not
 * need to open its own bus handle.
 *
 * This header is internal to src/hal/ and should NOT be included by
 * application-layer code.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "driver/i2c_master.h"
#include "diag_core.h"
#include "diag_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the shared I2C master bus handle.
 *
 * Initialises the bus on first call; subsequent calls return the
 * existing handle.
 *
 * @return Bus handle, or NULL on failure.
 */
i2c_master_bus_handle_t hal_i2c_bus_get(void);

/**
 * @brief Add a device to the shared I2C bus.
 *
 * Convenience wrapper around i2c_master_bus_add_device() using the
 * bus returned by hal_i2c_bus_get().
 *
 * @param addr      7-bit I2C address.
 * @param scl_speed  SCL clock speed (Hz), or 0 for default.
 * @param[out] dev   Receives the device handle.
 * @return DIAG_PASSED on success.
 */
diag_result_t hal_i2c_add_device(uint16_t addr,
                                  uint32_t scl_speed,
                                  i2c_master_dev_handle_t *dev);

/**
 * @brief Release the shared bus (call during deinit).
 */
void hal_i2c_bus_release(void);

#ifdef __cplusplus
}
#endif
