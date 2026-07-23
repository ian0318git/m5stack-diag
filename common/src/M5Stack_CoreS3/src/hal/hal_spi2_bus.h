/*
 * hal_spi2_bus.h — Shared SPI2 Bus Manager (LCD + SD card)
 *
 * CoreS3 shares SPI2 (MOSI=G37, MISO=G35, SCK=G36) between the
 * ILI9342C LCD (CS=G3) and the microSD card slot (CS=G4).
 *
 * This module provides reference-counted bus lifecycle management
 * so both peripherals can coexist without re-initialising the bus.
 *
 * Usage:
 *   // LCD init:
 *   hal_spi2_bus_init();                              // ref++
 *   hal_spi2_add_lcd_device(&spi_handle);             // add LCD device
 *
 *   // SD card init:
 *   hal_spi2_bus_init();                              // ref++ (no-op if ready)
 *   hal_spi2_add_sd_device(&sdspi_handle);            // add SD device via sdspi
 *
 *   // SD card deinit:
 *   hal_spi2_remove_sd_device();                      // remove SD device
 *   hal_spi2_bus_deinit();                            // ref-- (keeps bus if LCD alive)
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_core.h"
#include "driver/spi_master.h"
#include "driver/sdspi_host.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Bus lifecycle (ref-counted)                                               */
/*===========================================================================*/

/**
 * @brief Initialise SPI2 bus (idempotent).
 *
 * First call initialises the bus; subsequent calls increment a ref count.
 * Caller must pair each init() with a deinit().
 */
diag_result_t hal_spi2_bus_init(void);

/**
 * @brief Release SPI2 bus (ref-counted).
 *
 * Decrements ref count; when it reaches zero the bus is freed.
 */
void hal_spi2_bus_deinit(void);

/*===========================================================================*/
/* LCD device management                                                     */
/*===========================================================================*/

/**
 * @brief Add LCD device to the shared SPI2 bus.
 *
 * Configures CS=G3, 80 MHz, mode 0, half-duplex.
 *
 * @param[out] handle  SPI device handle for the LCD.
 * @return DIAG_PASSED on success.
 */
diag_result_t hal_spi2_add_lcd_device(spi_device_handle_t *handle);

/**
 * @brief Remove LCD device from SPI2 bus.
 */
void hal_spi2_remove_lcd_device(void);

/*===========================================================================*/
/* SD card device management (via sdspi_host)                                */
/*===========================================================================*/

/**
 * @brief Initialise sdspi_host on the shared SPI2 bus and add SD device.
 *
 * Configures CS=G4.  The sdspi_host layer shares the bus that was
 * already initialised by hal_spi2_bus_init(), avoiding double-init.
 *
 * @param[out] out_handle  sdspi device handle.
 * @return DIAG_PASSED on success.
 */
diag_result_t hal_spi2_add_sd_device(sdspi_dev_handle_t *out_handle);

/**
 * @brief Remove SD device from the shared bus.
 */
void hal_spi2_remove_sd_device(void);

#ifdef __cplusplus
}
#endif
