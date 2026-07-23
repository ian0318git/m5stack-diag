/*
 * test_button.c — PWR Button Read Test (DFS §Input)
 *
 * On-demand test: configures GPIO41 as input with pull-up,
 * prompts the operator to press the side PWR button within
 * 5 seconds, and reports whether the press was detected.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * Per DFS §Input: the side (PWR) button is on GPIO41.
 * We configure it as input with internal pull-up so that
 * the idle state is high; pressing the button pulls it low.
 */

#define BUTTON_GPIO     CONFIG_BUTTON_PWR_PIN
#define BUTTON_POLL_MS  50
#define BUTTON_TIMEOUT_MS 5000   /* 5-second operator window per DFS */

diag_result_t test_button(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "BUTTON", "MB/GPIO");

    /*------------------------------------------------------------------------*/
    /* Configure GPIO41 as input with pull-up                                */
    /*------------------------------------------------------------------------*/

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "GPIO41 config failed (err=%d)", err);
            diag_err_set_debug(g_diag_err_ctx,
                               "Check ESP32-S3 GPIO controller",
                               NULL);
        }
        return DIAG_FAILED;
    }

    /*------------------------------------------------------------------------*/
    /* Prompt operator                                                        */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("\r\nButton Test: press the side PWR button within 5 seconds...\r\n");
    diag_menu_printf("  (GPIO41, active-low with pull-up)\r\n");

    /*------------------------------------------------------------------------*/
    /* Poll for button press (busy-wait, 50 ms intervals)                    */
    /*------------------------------------------------------------------------*/

    int elapsed = 0;
    int pressed = 0;

    while (elapsed < BUTTON_TIMEOUT_MS) {
        int level = gpio_get_level(BUTTON_GPIO);
        if (level == 0) {
            pressed = 1;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
        elapsed += BUTTON_POLL_MS;
    }

    /*------------------------------------------------------------------------*/
    /* Report result                                                          */
    /*------------------------------------------------------------------------*/

    if (pressed) {
        diag_menu_printf("  Button detected after ~%d ms\r\n", elapsed);
        diag_menu_printf("Button Test: PASSED\r\n");

        /* Debounce: wait for release */
        int release_wait = 0;
        while (release_wait < 500) {
            if (gpio_get_level(BUTTON_GPIO) == 1) break;
            vTaskDelay(pdMS_TO_TICKS(10));
            release_wait += 10;
        }

        return DIAG_PASSED;
    }

    diag_menu_printf("  No button press detected within %d ms\r\n", BUTTON_TIMEOUT_MS);
    diag_menu_printf("Button Test: SKIPPED (operator-dependent)\r\n");

    if (g_diag_err_ctx) {
        diag_err_add(g_diag_err_ctx,
                     "GPIO41 PWR button: no press within 5 s (advisory)");
        diag_err_set_debug(g_diag_err_ctx,
                           "Check button continuity to GND",
                           "Verify GPIO41 solder joint");
    }

    return DIAG_SKIPPED;
}
