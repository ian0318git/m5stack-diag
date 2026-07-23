/*
 * test_camera.c — GC0308 Camera Register Test (DFS §Camera)
 *
 * On-demand test:
 *   1. AW9523B P1_0 CAM_RST control (10ms low → release)
 *   2. Probe I2C@0x21, read chip ID
 *   3. NACK → SKIPPED (camera flex cable is optional assembly)
 *   4. Unexpected NACK → check flex cable seating
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "gc0308.h"
#include "aw9523b.h"
#include "esp_rom_sys.h"

diag_result_t test_camera(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "CAMERA", "MB/CAM");

    diag_menu_printf("\r\nCamera Test (GC0308 @0x21)\r\n");

    /*------------------------------------------------------------------------*/
    /* Step 1: Init AW9523B P1_0 (CAM_RST)                                    */
    /*------------------------------------------------------------------------*/

    i2c_master_dev_handle_t aw_dev = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &aw_dev)
        != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "AW9523B not reachable — GPIO control failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Run GPIO Expander Test first",
                               "Check I2C bus 0x58");
        }
        return DIAG_SKIPPED;
    }

    if (aw9523b_init(&g_diag_i2c_adapter, (void *)aw_dev) != 0) {
        return DIAG_SKIPPED;
    }

    /* Configure CAM_RST (P1_0): GPIO mode, output */
    aw9523b_pin_set_gpio_mode(AW9523B_PIN_CAM_RST);
    aw9523b_pin_set_direction(AW9523B_PIN_CAM_RST, 1);

    /* Hardware reset: assert 10 ms, then release */
    aw9523b_pin_write(AW9523B_PIN_CAM_RST, 0);
    esp_rom_delay_us(10000);
    aw9523b_pin_write(AW9523B_PIN_CAM_RST, 1);
    esp_rom_delay_us(50000);

    /*------------------------------------------------------------------------*/
    /* Step 2: Probe GC0308 at 0x21                                           */
    /*------------------------------------------------------------------------*/

    i2c_master_dev_handle_t cam_dev = NULL;
    if (hal_i2c_add_device(GC0308_ADDR, 400000, &cam_dev) != DIAG_PASSED) {
        diag_menu_printf("  I2C@0x21: NACK — camera flex cable not detected\r\n");
        diag_menu_printf("Camera Test: SKIPPED (optional assembly)\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x21 GC0308: no ACK (flex cable may be absent)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check camera flex cable at both ends",
                               "Re-run GPIO Expander Test to verify P1_0 toggles");
        }
        return DIAG_SKIPPED;
    }

    if (gc0308_probe(&g_diag_i2c_adapter, (void *)cam_dev) != 0) {
        diag_menu_printf("  GC0308: chip ID mismatch\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x21 GC0308: chip ID 0x%02X, expected 0x%02X",
                         gc0308_chip_id(), GC0308_CHIP_ID_VAL);
            diag_err_set_debug(g_diag_err_ctx,
                               "Flex cable seated? Try reseating",
                               "Replace camera module");
        }
        return DIAG_FAILED;
    }

    diag_menu_printf("  GC0308 chip ID: 0x%02X\r\n", gc0308_chip_id());
    diag_menu_printf("Camera Test: PASSED\r\n");
    return DIAG_PASSED;
}
