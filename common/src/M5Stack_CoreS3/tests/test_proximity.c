/*
 * test_proximity.c — LTR-553ALS-WA Proximity + ALS Read Test (DFS §Proximity)
 *
 * On-demand test:
 *   1. Probe I2C@0x23, read part ID
 *   2. NACK → SKIPPED (on same optional flex cable as camera)
 *   3. Enable ALS + proximity sensors
 *   4. Read ALS CH0 (visible), CH1 (visible+IR), and proximity raw values
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "ltr553.h"

diag_result_t test_proximity(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "PROXIMITY", "MB/OPTICAL");

    diag_menu_printf("\r\nProximity + ALS Test (LTR-553 @0x23)\r\n");

    /*------------------------------------------------------------------------*/
    /* Step 1: Probe LTR-553 at 0x23                                          */
    /*------------------------------------------------------------------------*/

    i2c_master_dev_handle_t prox_dev = NULL;
    if (hal_i2c_add_device(LTR553_ADDR, 400000, &prox_dev) != DIAG_PASSED) {
        diag_menu_printf("  I2C@0x23: NACK — proximity sensor not detected\r\n");
        diag_menu_printf("Proximity Test: SKIPPED (on optional camera flex cable)\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x23 LTR-553: no ACK (flex cable may be absent)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check camera flex cable seating",
                               "Run Camera Test first — if camera present, LTR-553 should respond");
        }
        return DIAG_SKIPPED;
    }

    if (ltr553_probe(&g_diag_i2c_adapter, (void *)prox_dev) != 0) {
        diag_menu_printf("  LTR-553: part ID mismatch or init failed\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x23 LTR-553: probe failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check LTR-553 power and I2C connection",
                               "Replace camera flex module");
        }
        return DIAG_FAILED;
    }

    /*------------------------------------------------------------------------*/
    /* Step 2: Read ALS + proximity data                                      */
    /*------------------------------------------------------------------------*/

    ltr553_data_t data;
    if (ltr553_read_all(&data) != 0) {
        diag_menu_printf("  LTR-553: read failed\r\n");
        return DIAG_FAILED;
    }

    diag_menu_printf("  Part ID:      0x%02X\r\n", data.part_id);
    diag_menu_printf("  ALS CH0 (vis): %u\r\n",   data.als_ch0);
    diag_menu_printf("  ALS CH1 (vis+IR): %u\r\n", data.als_ch1);
    diag_menu_printf("  Proximity:     %u\r\n",    data.proximity);

    /* Check that at least the ALS sensor returns non-zero data.
     * Zero ALS in normal lighting suggests a sensor fault. */
    if (data.als_ch0 > 5 || data.als_ch1 > 5) {
        diag_menu_printf("Proximity Test: PASSED (ALS functional)\r\n");
        return DIAG_PASSED;
    }

    diag_menu_printf("Proximity Test: ADVISORY (ALS readings near zero — "
                     "check ambient lighting or sensor)\r\n");
    if (g_diag_err_ctx) {
        diag_err_add(g_diag_err_ctx,
                     "LTR-553: ALS readings near zero (CH0=%u, CH1=%u)",
                     data.als_ch0, data.als_ch1);
        diag_err_set_debug(g_diag_err_ctx,
                           "Ensure adequate ambient light",
                           "If in bright light, LTR-553 may be faulty");
    }
    return DIAG_SKIPPED;
}
