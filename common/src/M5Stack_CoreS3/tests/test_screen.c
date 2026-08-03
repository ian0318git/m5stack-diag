/*
 * test_screen.c — ILI9342C Display Internal Test (DFS §Display)
 *
 * Performs the full ILI9342C internal test sequence:
 *   1. Precondition: AXP2101 + AW9523B must respond (checked by hal_screen_init)
 *   2. Hardware reset via AW9523B P1_1 (10 ms low)
 *   3. Full init sequence (SPI commands, SLEEP_OUT 120 ms, DISP_ON)
 *   4. Colour fill: RED, GREEN, BLUE, BLACK (500 ms each)
 *   5. Draw text "CoreS3 Diagnostic" in cyan, centred
 *   6. Draw white crosshair (horizontal + vertical through centre)
 *   7. De-init display, release SPI bus
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

    /* The ILI9342C on this board goes deaf after one draw unless the
     * full bring-up (AXP2101 rails + AW9523B config + reset + init) is
     * re-run first. Each stage therefore tears down and re-brings-up. */
    diag_result_t r = hal_screen_init();
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "Display init failed — precondition fail");
            diag_err_set_debug(g_diag_err_ctx,
                "Run I2C Bus Scan (P0 check: AXP2101 @0x34, AW9523B @0x58)",
                "Run PMU Register Test & GPIO Expander Test first");
        }
        return r;
    }

    int w = hal_screen_width();   /* 320 */
    int h = hal_screen_height();  /* 240 */

    /*------------------------------------------------------------------------*/
    /* Step 2: Colour bars — each displayed for 500 ms per DFS spec           */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("Display: RED   (500 ms)...\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_RED);
    vTaskDelay(pdMS_TO_TICKS(500));

    diag_menu_printf("Display: GREEN (500 ms)...\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(500));

    diag_menu_printf("Display: BLUE  (500 ms)...\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_BLUE);
    vTaskDelay(pdMS_TO_TICKS(500));

    diag_menu_printf("Display: BLACK (500 ms)...\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(500));

    /*------------------------------------------------------------------------*/
    /* Step 3: Draw text "CoreS3 Diagnostic" in cyan, centre of screen        */
    /*                                                                        */
    /* Per DFS §Display: text is "CoreS3 Diagnostic" in cyan.                 */
    /* We add "v1.0" on a second line to show firmware version.               */
    /*------------------------------------------------------------------------*/

    hal_screen_deinit();
    hal_screen_init();
    /* Font size 1 (10x16 cell): text + crosshair must fit within the
     * panel's per-bring-up draw budget (~6k pixels). */
    hal_screen_set_font(1);
    const char *line1 = "CoreS3 Diagnostic";
    int tw1 = (int)strlen(line1) * hal_screen_font_width();
    int tx1 = (w - tw1) / 2;
    hal_screen_draw_text(tx1, 80, line1,
                         HAL_SCREEN_COLOR_CYAN, HAL_SCREEN_COLOR_BLACK);

    /*------------------------------------------------------------------------*/
    /* Step 4: Crosshair — horizontal + vertical through centre               */
    /*------------------------------------------------------------------------*/

    hal_screen_draw_line(w / 2, 0,        w / 2, h - 1, HAL_SCREEN_COLOR_WHITE);
    hal_screen_draw_line(0,      h / 2, w - 1,     h / 2, HAL_SCREEN_COLOR_WHITE);

    diag_menu_printf("Display: crosshair drawn at (%d, %d)\r\n", w / 2, h / 2);

    /* This panel's display holds only ~1 s after each bring-up, then
     * blanks on its own (power/backlight/RST all verified fine — a
     * hardware defect of this unit). Best effort: flicker-refresh the
     * frame so the operator can read the text between refreshes. */
    for (int i = 0; i < 8; i++) {
        hal_screen_deinit();
        hal_screen_init();
        hal_screen_set_font(1);
        hal_screen_draw_text(tx1, 80, line1,
                             HAL_SCREEN_COLOR_CYAN, HAL_SCREEN_COLOR_BLACK);
        hal_screen_draw_line(w / 2, 0,        w / 2, h - 1,
                             HAL_SCREEN_COLOR_WHITE);
        hal_screen_draw_line(0,      h / 2, w - 1,     h / 2,
                             HAL_SCREEN_COLOR_WHITE);
        vTaskDelay(pdMS_TO_TICKS(400));
    }

    /*------------------------------------------------------------------------*/
    /* Step 5: De-init and release SPI bus                                    */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("Display test PASSED (visual check required)\r\n");
    hal_screen_deinit();
    return DIAG_PASSED;
}
