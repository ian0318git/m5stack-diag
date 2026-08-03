/*
 * hal_screen.h - Hardware Abstraction Layer: GC9A01 Circular LCD
 *
 * This header defines the interface that the application layer depends on.
 * The concrete implementation in src/hal/hal_screen.c drives the GC9A01
 * via SPI, but no caller should rely on those details.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Colour primitives (RGB565)                                                */
/*===========================================================================*/

typedef uint16_t hal_screen_colour_t;

#define HAL_SCREEN_COLOR_BLACK   0x0000
#define HAL_SCREEN_COLOR_WHITE   0xFFFF
#define HAL_SCREEN_COLOR_RED     0xF800
#define HAL_SCREEN_COLOR_GREEN   0x07E0
#define HAL_SCREEN_COLOR_BLUE    0x001F
#define HAL_SCREEN_COLOR_YELLOW  0xFFE0
#define HAL_SCREEN_COLOR_CYAN    0x07FF
#define HAL_SCREEN_COLOR_MAGENTA 0xF81F
#define HAL_SCREEN_COLOR_ORANGE  0xFC00

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the screen controller.
 * @return DIAG_PASSED on success, DIAG_FAILED if hardware init failed.
 */
diag_result_t hal_screen_init(void);

/**
 * @brief De-initialise the screen and release resources.
 */
void hal_screen_deinit(void);

/**
 * @brief Keep the panel awake (the panel blanks ~1 s after the last
 * SPI activity on this unit — re-send DISPON periodically).
 */
void hal_screen_keepalive(void);

/*===========================================================================*/
/* Basic drawing                                                             */
/*===========================================================================*/

/**
 * @brief Fill the entire screen with a single colour.
 */
void hal_screen_fill(hal_screen_colour_t colour);

/**
 * @brief Set one pixel at (x, y).
 */
void hal_screen_draw_pixel(int x, int y, hal_screen_colour_t colour);

/**
 * @brief Fill a rectangular region.
 */
void hal_screen_fill_rect(int x, int y, int w, int h,
                          hal_screen_colour_t colour);

/**
 * @brief Draw a single line (Bresenham).
 */
void hal_screen_draw_line(int x0, int y0, int x1, int y1,
                          hal_screen_colour_t colour);

/*===========================================================================*/
/* Text output                                                               */
/*===========================================================================*/

/**
 * @brief Draw a null-terminated string at (x, y).
 * @param x,y   Top-left anchor in pixels.
 * @param text  Null-terminated UTF-8 string (ASCII subset recommended).
 * @param fg    Foreground colour.
 * @param bg    Background colour (can match fill for transparent look).
 */
void hal_screen_draw_text(int x, int y, const char *text,
                          hal_screen_colour_t fg, hal_screen_colour_t bg);

/**
 * @brief Set the font size index.
 * @param size  0 = small (8x8), 1 = medium (12x16), 2 = large (16x24).
 */
void hal_screen_set_font(int size);

/**
 * @brief Return the advance width / height of the current font.
 */
int hal_screen_font_width(void);
int hal_screen_font_height(void);

/*===========================================================================*/
/* Backlight                                                                 */
/*===========================================================================*/

/**
 * @brief Set backlight brightness.
 * @param brightness  0 (off) – 100 (full).
 */
void hal_screen_set_backlight(uint8_t brightness);

/*===========================================================================*/
/* Display info                                                              */
/*===========================================================================*/

int  hal_screen_width(void);
int  hal_screen_height(void);

#ifdef __cplusplus
}
#endif
