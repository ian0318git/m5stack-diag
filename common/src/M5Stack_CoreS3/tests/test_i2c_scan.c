/*
 * test_i2c_scan.c — I2C Bus Scan test
 *
 * Probes every address 0x01–0x7F, cross-references known devices,
 * and flags missing mandatory (P0) devices per DFS.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_config.h"
#include "diag_menu.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include <string.h>

diag_result_t test_i2c_scan(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "I2C", "MB/I2C");
    diag_menu_printf("Scanning full I2C address range 0x01-0x7F...\r\n");

    i2c_master_bus_handle_t bus_h = hal_i2c_bus_get();
    if (!bus_h) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "I2C bus not available");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check power supply to the I2C bus",
                               "Check SDA/SCL pull-up resistors");
        }
        return DIAG_FAILED;
    }

    /* Use the abstract transport seam for probing */
    const diag_i2c_t *i2c = &g_diag_i2c_adapter;
    void *bus = (void *)bus_h;

    int found = 0;
    diag_menu_printf("\r\n  Found devices:\r\n");

    for (uint16_t addr = 1; addr < 0x80; addr++) {
        int err = i2c->probe(bus, addr);
        if (err == 0) {
            const char *name = NULL;
            switch (addr) {
                case CONFIG_I2C_ADDR_TOUCH:     name = "FT6336U (Touch)";    break;
                case 0x3A:                      name = "FT6336 alt (Touch)"; break;
                case CONFIG_I2C_ADDR_RTC:       name = "BM8563 (RTC)";       break;
                case CONFIG_I2C_ADDR_IMU:       name = "BMI270 (IMU)";       break;
                case CONFIG_I2C_ADDR_POWER:     name = "AXP2101 (PMU)";      break;
                case CONFIG_I2C_ADDR_AUDIO_ADC: name = "ES7210 (Audio ADC)"; break;
                case CONFIG_I2C_ADDR_SPK_AMP:   name = "AW88298 (Speaker)";  break;
                case CONFIG_I2C_ADDR_GPIO_EXP:  name = "AW9523B (GPIO Exp)"; break;
                case CONFIG_I2C_ADDR_CAMERA:    name = "GC0308 (Camera)";    break;
                case CONFIG_I2C_ADDR_PROXIMITY: name = "LTR-553ALS (Proximity)"; break;
            }
            if (name) {
                diag_menu_printf("    [ OK ] 0x%02X — %s\r\n", addr, name);
            } else {
                diag_menu_printf("    [ OK ] 0x%02X — UNKNOWN\r\n", addr);
            }
            found++;
        }
    }

    diag_menu_printf("\r\n  %d device(s) found on I2C bus\r\n", found);

    /* Cross-check: P0 mandatory devices */
    {
        const uint8_t mandatory[] = { CONFIG_I2C_ADDR_POWER, CONFIG_I2C_ADDR_GPIO_EXP };
        const char *m_names[] = { "AXP2101 (PMU)", "AW9523B (GPIO Exp)" };
        const char *m_hints[] = {
            "PMU is root power — check USB/battery input",
            "GPIO expander controls all peripheral RST lines",
        };
        int m_missing = 0;
        for (size_t i = 0; i < sizeof(mandatory); i++) {
            if (i2c->probe(bus, mandatory[i]) != 0) {
                diag_menu_printf("  ** MISSING: 0x%02X %s — P0 mandatory\r\n",
                                 mandatory[i], m_names[i]);
                if (g_diag_err_ctx) {
                    diag_err_add(g_diag_err_ctx, "I2C@0x%02X %s: P0 mandatory device missing",
                                 mandatory[i], m_names[i]);
                    diag_err_set_debug(g_diag_err_ctx, m_hints[i], NULL);
                }
                m_missing++;
            }
        }
        if (m_missing > 0) return DIAG_FAILED;
    }

    /* Advisory P1 devices */
    {
        const uint8_t advisory[] = {
            CONFIG_I2C_ADDR_RTC, CONFIG_I2C_ADDR_IMU, CONFIG_I2C_ADDR_TOUCH,
        };
        const char *a_names[] = { "BM8563 (RTC)", "BMI270 (IMU)", "FT6336U (Touch)" };
        const char *a_hints[] = {
            "RTC powered by AXP2101 RTC_VDD",
            "IMU powered by AXP2101 SYS_3V3",
            "FT6336U needs AXP2101 LDOIO0 + AW9523B P0_0 — run Touch test to power on",
        };
        for (size_t i = 0; i < sizeof(advisory); i++) {
            if (i2c->probe(bus, advisory[i]) != 0) {
                diag_menu_printf("  -- 0x%02X %s — no ACK\r\n", advisory[i], a_names[i]);
                diag_menu_printf("     > %s\r\n", a_hints[i]);
                if (g_diag_err_ctx) {
                    diag_err_add(g_diag_err_ctx, "I2C@0x%02X %s: no ACK (advisory)",
                                 advisory[i], a_names[i]);
                    diag_err_set_debug(g_diag_err_ctx, a_hints[i], NULL);
                }
            }
        }
    }

    /* Optional devices */
    {
        const uint8_t opt[] = { 0x36, 0x21, 0x23 };
        const char *on[] = { "AW88298 (Speaker)", "GC0308 (Camera)", "LTR-553 (Prox)" };
        for (size_t i = 0; i < sizeof(opt); i++) {
            if (i2c->probe(bus, opt[i]) != 0) {
                diag_menu_printf("  -- 0x%02X %s — optional, skip\r\n", opt[i], on[i]);
            }
        }
    }

    /* Alt touch address check */
    if (i2c->probe(bus, 0x3A) == 0) {
        diag_menu_printf("  ** NOTE: Touch found at 0x3A (not 0x38)\r\n");
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx, "FT6336: found at 0x3A, not expected 0x38");
    }

    diag_menu_printf("\r\n  All P0 mandatory devices present.\r\n");
    return DIAG_PASSED;
}
