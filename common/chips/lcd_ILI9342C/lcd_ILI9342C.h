/*
 * lcd_ILI9342C.h — ILI9342C 320×240 IPS LCD Driver (SPI)
 *
 * Common chip driver — platform-agnostic.
 * Caller provides an initialised diag_spi_t transport and a
 * callback for reset control (typically via AW9523B expander).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Commands                                                         */
/*===========================================================================*/

#define ILI9342_CMD_SWRESET     0x01
#define ILI9342_CMD_SLPIN       0x10
#define ILI9342_CMD_SLPOUT      0x11
#define ILI9342_CMD_DISPOFF     0x28
#define ILI9342_CMD_DISPON      0x29
#define ILI9342_CMD_CASET       0x2A
#define ILI9342_CMD_RASET       0x2B
#define ILI9342_CMD_RAMWR       0x2C
#define ILI9342_CMD_INVON       0x21
#define ILI9342_CMD_MADCTL      0x36
#define ILI9342_CMD_COLMOD      0x3A

/* MADCTL flags */
#define ILI9342_MADCTL_MY       (1 << 7)
#define ILI9342_MADCTL_MX       (1 << 6)
#define ILI9342_MADCTL_MV       (1 << 5)
#define ILI9342_MADCTL_BGR      (1 << 3)

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the ILI9342C display.
 *
 * @param spi       Abstract SPI transport.
 * @param spi_bus   SPI device handle (opaque, passed to transport callbacks).
 * @param dc_set    Callback to set Data/Command GPIO level (0=cmd, 1=data).
 *                  May be NULL if DC is handled externally.
 * @param set_rst   Callback to assert/de-assert RST (1=release, 0=hold).
 *                  May be NULL if RST is handled externally.
 * @return 0 on success, -1 on error.
 */
int  lcd_ILI9342C_init(const diag_spi_t *spi, void *spi_bus,
                        void (*dc_set)(int level),
                        void (*set_rst)(int level));
void lcd_ILI9342C_deinit(void);

/*===========================================================================*/
/* Drawing Primitives                                                        */
/*===========================================================================*/

void lcd_ILI9342C_set_window(uint16_t x0, uint16_t y0,
                              uint16_t x1, uint16_t y1);
void lcd_ILI9342C_write_pixels(const uint16_t *pixels, size_t count);

#ifdef __cplusplus
}
#endif
