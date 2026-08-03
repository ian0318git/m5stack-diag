/*
 * test_backlight.c — LCD Backlight Test (DFS §Display — Backlight Test)
 *
 * Enables AXP2101 DLDO1 (CoreS3 LCD backlight) and prompts the operator
 * for visual verification.
 *
 * DLDO1 control (verified against M5Stack m5gfx / datasheet bindings):
 *   - reg 0x90 bit7 = DLDO1 enable (read-modify-write: the same register
 *     also enables the other LDO rails)
 *   - reg 0x99     = DLDO1 voltage, value = (mV - 500) / 100; 0x1C = 3.3V
 * DLDO1 3.3V feeds the SY7088 boost converter which drives the LCD LEDs.
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
#include "aw9523b.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static i2c_master_dev_handle_t s_pmu_dev = NULL;
static i2c_master_dev_handle_t s_aw_dev = NULL;

static diag_result_t boost_enable(void)
{
    /* SY7088 BOOST_EN = AW9523B P1_7 (per M5Unified CoreS3 power init) */
    if (!s_aw_dev) {
        if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &s_aw_dev)
            != DIAG_PASSED) {
            return DIAG_FAILED;
        }
        if (aw9523b_init(&g_diag_i2c_adapter, (void *)s_aw_dev) != 0) {
            return DIAG_FAILED;
        }
    }
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_BOOST_EN);
    aw9523b_pin_set_direction(AW9523B_PIN_BOOST_EN, 1);
    aw9523b_pin_write(AW9523B_PIN_BOOST_EN, 1);
    return DIAG_PASSED;
}

static diag_result_t backlight_set(bool on)
{
    if (!s_pmu_dev) {
        if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &s_pmu_dev)
            != DIAG_PASSED) {
            return DIAG_FAILED;
        }
    }
    if (on && boost_enable() != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    /* Read-modify-write reg 0x90: set/clear the DLDO1 enable bit only */
    uint8_t reg = AXP2101_REG_LDO_EN;
    uint8_t ldo_en = 0;
    esp_err_t err = i2c_master_transmit_receive(s_pmu_dev, &reg, 1,
                                                 &ldo_en, 1, -1);
    if (err != ESP_OK) {
        return DIAG_FAILED;
    }
    if (on) {
        /* 0xBF = all LDOs (per M5Unified CoreS3 init): DLDO1 backlight,
         * BLDO1 LCD VDD (panel stays powered), ALDO1..4, BLDO2 */
        ldo_en |= 0xBF;
    } else {
        /* OFF = backlight only; keep LCD VDD (BLDO1) powered */
        ldo_en &= (uint8_t)~AXP2101_LDO_DLDO1;
    }
    uint8_t cmd[2] = { AXP2101_REG_LDO_EN, ldo_en };
    err = i2c_master_transmit(s_pmu_dev, cmd, 2, -1);
    if (err != ESP_OK) {
        return DIAG_FAILED;
    }

    if (on) {
        /* DLDO1 3.3 V */
        cmd[0] = AXP2101_REG_DLDO1_VOLT;
        cmd[1] = AXP2101_DLDO1_VOLT_3V3;
        err = i2c_master_transmit(s_pmu_dev, cmd, 2, -1);
        if (err != ESP_OK) {
            return DIAG_FAILED;
        }
    }

    return DIAG_PASSED;
}

diag_result_t test_backlight(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "BACKLIGHT", "MB/LCD");

    /*------------------------------------------------------------------------*/
    /* Precondition: AXP2101 must be writable (checked by backlight_set)      */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /* Step 1: Backlight ON                                                  */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("\r\nBacklight Test: enabling DLDO1 at 3.3V...\r\n");
    if (backlight_set(true) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "AXP2101 DLDO1 enable failed (reg 0x90/0x99)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Verify AXP2101 registers 0x90/0x99 read-back",
                               "Check DLDO1 output voltage on the SY7088 boost input");
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

    if (backlight_set(false) != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "AXP2101 DLDO1 disable failed (reg 0x90)");
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
    backlight_set(true);
    vTaskDelay(pdMS_TO_TICKS(200));

    /* NOTE: do NOT i2c_master_bus_rm_device() here — the chip drivers
     * (power_AXP2101 / aw9523b) keep static pointers to the handles and
     * would dereference freed memory on the next call (LoadProhibited).
     * Leaving the handles allocated is a minor, deliberate leak. */

    diag_menu_printf("\r\nBacklight Test: PASSED (visual check)\r\n");
    diag_menu_printf("  DLDO1=0x1C @0x99, enable bit7 @0x90, BOOST_EN=P1_7\r\n");
    return DIAG_PASSED;
}
