/*
 * test_screen.c — ILI9342C Display test
 *
 * Initialises the display, shows colour bars, draws text and crosshairs.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "hal_screen.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

diag_result_t test_screen(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "SCREEN", "MB/LCD");

    diag_result_t r = hal_screen_init();
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "ILI9342C screen init failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check SPI bus (MOSI=G37, SCLK=G36, CS=G3, DC=G35)",
                               "Check AW9523B P1_1 LCD_RST and AXP2101 DLDO1 backlight");
        }
        return r;
    }

    int w = hal_screen_width();
    int h = hal_screen_height();

    hal_screen_fill(HAL_SCREEN_COLOR_RED);    vTaskDelay(pdMS_TO_TICKS(300));
    hal_screen_fill(HAL_SCREEN_COLOR_GREEN);  vTaskDelay(pdMS_TO_TICKS(300));
    hal_screen_fill(HAL_SCREEN_COLOR_BLUE);   vTaskDelay(pdMS_TO_TICKS(300));
    hal_screen_fill(HAL_SCREEN_COLOR_BLACK);  vTaskDelay(pdMS_TO_TICKS(200));

    hal_screen_set_font(2);
    const char *lines[] = { "CoreS3", "Diagnostic", "System", NULL };
    int y = 60;
    for (int i = 0; lines[i]; i++) {
        int text_w = strlen(lines[i]) * hal_screen_font_width();
        int tx = (w - text_w) / 2;
        hal_screen_draw_text(tx, y, lines[i],
                             HAL_SCREEN_COLOR_CYAN, HAL_SCREEN_COLOR_BLACK);
        y += hal_screen_font_height() + 4;
    }

    hal_screen_draw_line(w / 2, 0, w / 2, h - 1, HAL_SCREEN_COLOR_WHITE);
    hal_screen_draw_line(0, h / 2, w - 1, h / 2, HAL_SCREEN_COLOR_WHITE);

    diag_menu_printf("Screen test complete.\r\n");
    hal_screen_deinit();
    return DIAG_PASSED;
}
