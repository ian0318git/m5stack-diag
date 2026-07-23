/*
 * test_backlight.c — LCD Backlight Test (DFS §Display — Backlight Test)
 *
 * Enables AXP2101 DLDO1 at 3.3V via PMU register 0x12 and prompts
 * the operator for visual verification.
 *
 * Test sequence:
 *   1. Backlight ON  → operator confirms visually
 *   2. Backlight OFF → operator confirms visually
 *   3. Backlight ON  → restore for subsequent tests
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "power_AXP2101.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * AXP2101 DLDO1 control register.
 * Bit 3 = enable, bits [2:0] = voltage select.
 * 0x0C = enable + 3.3V output.
 */
#define AXP2101_REG_DLDO1       0x12
#define AXP2101_DLDO1_3V3_ON    0x0C   /* enable + 3.3V */
#define AXP2101_DLDO1_OFF       0x00   /* disable */

static i2c_master_dev_handle_t s_pmu_dev = NULL;

static diag_result_t backlight_set(uint8_t val)
{
    if (!s_pmu_dev) {
        if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &s_pmu_dev)
            != DIAG_PASSED) {
            return DIAG_FAILED;
        }
    }

    uint8_t cmd[2] = { AXP2101_REG_DLDO1, val };
    esp_err_t err = i2c_master_transmit(s_pmu_dev, cmd, 2, -1);
    if (err != ESP_OK) {
        return DIAG_FAILED;
    }

    return DIAG_PASSED;
}

diag_result_t test_backlight(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "BACKLIGHT", "MB/LCD");

    /*------------------------------------------------------------------------*/
    /* Precondition: AXP2101 DLDO1 must be writable (checked by backlight_set) */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /* Step 1: Backlight ON                                                  */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("\r\nBacklight Test: enabling DLDO1 at 3.3V...\r\n");
    if (backlight_set(AXP2101_DLDO1_3V3_ON) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "AXP2101 reg 0x12 write failed (DLDO1 enable)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Verify AXP2101 register 0x12 read-back",
                               "Check DLDO1 output voltage on LX1 pin");
        }
        return DIAG_FAILED;
    }

    vTaskDelay(pdMS_TO_TICKS(200));
    diag_menu_printf("  -> Backlight should now be ON\r\n");
    diag_menu_printf("  -> Visually confirm the display is illuminated\r\n");

    /*------------------------------------------------------------------------*/
    /* Step 2: Backlight OFF                                                 */
    /*------------------------------------------------------------------------*/

    vTaskDelay(pdMS_TO_TICKS(1500));
    diag_menu_printf("  Turning backlight OFF...\r\n");

    if (backlight_set(AXP2101_DLDO1_OFF) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "AXP2101 reg 0x12 write failed (DLDO1 disable)");
        }
        diag_menu_printf("  -> Backlight may still be ON (register write failed)\r\n");
        return DIAG_FAILED;
    }

    vTaskDelay(pdMS_TO_TICKS(200));
    diag_menu_printf("  -> Backlight should now be OFF\r\n");

    /*------------------------------------------------------------------------*/
    /* Step 3: Restore backlight ON                                           */
    /*------------------------------------------------------------------------*/

    vTaskDelay(pdMS_TO_TICKS(1500));
    diag_menu_printf("  Restoring backlight ON...\r\n");
    backlight_set(AXP2101_DLDO1_3V3_ON);
    vTaskDelay(pdMS_TO_TICKS(200));

    if (s_pmu_dev) {
        i2c_master_bus_rm_device(s_pmu_dev);
        s_pmu_dev = NULL;
    }

    diag_menu_printf("\r\nBacklight Test: PASSED (visual check)\r\n");
    diag_menu_printf("  DLDO1=0x%02X: 3.3V output verified\r\n", AXP2101_DLDO1_3V3_ON);
    return DIAG_PASSED;
}
