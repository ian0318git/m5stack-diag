/*
 * hal_i2c_adapter.h — ESP-IDF adapter for diag_i2c_t transport seam
 *
 * ADAPTER layer — exposes the singleton ESP-IDF implementation of
 * the abstract diag_i2c_t interface.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Singleton ESP-IDF I2C transport adapter. */
extern const diag_i2c_t g_diag_i2c_adapter;

#ifdef __cplusplus
}
#endif
