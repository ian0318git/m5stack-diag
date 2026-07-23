/*
 * hal_spi_adapter.h — ESP-IDF adapter for diag_spi_t transport seam
 *
 * ADAPTER layer — exposes the singleton ESP-IDF implementation of
 * the abstract diag_spi_t interface.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Singleton ESP-IDF SPI transport adapter. */
extern const diag_spi_t g_diag_spi_adapter;

#ifdef __cplusplus
}
#endif
