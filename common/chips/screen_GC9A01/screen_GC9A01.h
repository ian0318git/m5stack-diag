/*
 * screen_GC9A01.h — GC9A01 Circular LCD Driver (SPI)
 *
 * Common chip driver — reusable across any board/CPU.
 * Caller must provide an already-initialised SPI device handle.
 *
 * Register definitions and chip-specific API exported here.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Map                                                              */
/*===========================================================================*/

#define GC9A01_SWRESET     0x01
#define GC9A01_SLPIN       0x10
#define GC9A01_SLPOUT      0x11
#define GC9A01_DISPOFF     0x28
#define GC9A01_DISPON      0x29
#define GC9A01_CASET       0x2A
#define GC9A01_RASET       0x2B
#define GC9A01_RAMWR       0x2C
#define GC9A01_MADCTL      0x36
#define GC9A01_COLMOD      0x3A

/* Memory access control bits */
#define GC9A01_MADCTL_MY   (1 << 7)
#define GC9A01_MADCTL_MX   (1 << 6)
#define GC9A01_MADCTL_MV   (1 << 5)
#define GC9A01_MADCTL_BGR  (1 << 3)

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the GC9A01 display controller.
 *
 * Issues the hardware reset + initialisation sequence.
 *
 * @param spi       Initialised SPI device handle.
 * @param dc_gpio   GPIO number for the Data/Command control pin.
 * @param rst_gpio  GPIO number for the hardware reset pin (or -1 if not used).
 * @return          DIAG_PASSED on success.
 */
int screen_GC9A01_init(spi_device_handle_t spi, int dc_gpio, int rst_gpio);

/**
 * @brief De-initialise (display off, sleep).
 */
void screen_GC9A01_deinit(void);

/*===========================================================================*/
/* SPI Primitives (available for direct access)                              */
/*===========================================================================*/

void screen_GC9A01_send_cmd(uint8_t cmd);
void screen_GC9A01_send_data(uint8_t data);
void screen_GC9A01_send_cmd_data(uint8_t cmd, const uint8_t *data, size_t len);

/*===========================================================================*/
/* Drawing Primitives                                                        */
/*===========================================================================*/

void screen_GC9A01_set_window(uint16_t x0, uint16_t y0,
                               uint16_t x1, uint16_t y1);
void screen_GC9A01_write_pixels(const uint16_t *pixels, size_t count);

#ifdef __cplusplus
}
#endif
