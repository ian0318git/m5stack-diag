/*
 * test_wifi.c — Connectivity: Wi-Fi Station Test (DFS §Connectivity)
 *
 * Joins a Wi-Fi AP in station mode and waits for a DHCP-assigned IP.
 * Reports IP, RSSI (with quality rating) and channel on success.
 *
 * Result mapping (DFS §Connectivity):
 *   no credentials configured -> SKIPPED (advisory)
 *   AP not found              -> SKIPPED (advisory)
 *   auth-class failure        -> FAILED  (password hint)
 *   other connect/DHCP failure-> FAILED  (reachability hint)
 *
 * Copyright (c) 2026 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "diag_tests.h"
#include "diag_menu.h"
#include "diag_config.h"
#include "hal_wifi.h"
#include "esp_wifi.h"

diag_result_t test_wifi(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "WIFI", "MB/WIFI");

    /*------------------------------------------------------------------------*/
    /* Resolve credentials (NVS overrides Kconfig defaults)                   */
    /*------------------------------------------------------------------------*/

    char ssid[64], pass[64];
    hal_wifi_cfg_get_ssid(ssid, sizeof(ssid));
    hal_wifi_cfg_get_password(pass, sizeof(pass));

    if (ssid[0] == '\0') {
        diag_menu_printf("Wi-Fi Test: SKIPPED (no credentials configured)\r\n");
        diag_menu_printf("  Use: wifi-set ssid <name>; wifi-set pass <password>\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "No Wi-Fi credentials configured (advisory)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Run 'wifi-set ssid <name>' and 'wifi-set pass <pw>'",
                               "Or set CONFIG_WIFI_DIAG_SSID in menuconfig");
        }
        return DIAG_SKIPPED;
    }

    /*------------------------------------------------------------------------*/
    /* Connect and wait for IP                                                */
    /*------------------------------------------------------------------------*/

    diag_menu_printf("Wi-Fi Test: joining '%s'...\r\n", ssid);
    if (hal_wifi_init() != DIAG_PASSED) {
        diag_menu_printf("Wi-Fi Test: FAILED (driver init)\r\n");
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx, "Wi-Fi driver init failed");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check radio hardware",
                               "Check NVS partition for WiFi calibration data");
        }
        return DIAG_FAILED;
    }

    diag_result_t r = hal_wifi_connect(ssid, pass, CONFIG_WIFI_CONNECT_TIMEOUT_MS);
    if (r != DIAG_PASSED) {
        int reason = hal_wifi_get_disconnect_reason();
        hal_wifi_deinit();

        if (reason == (int)WIFI_REASON_NO_AP_FOUND) {
            diag_menu_printf("Wi-Fi Test: SKIPPED (AP '%s' not found)\r\n", ssid);
            if (g_diag_err_ctx) {
                diag_err_add(g_diag_err_ctx,
                             "AP '%s' not found (advisory)", ssid);
                diag_err_set_debug(g_diag_err_ctx,
                                   "Check AP is powered and in range",
                                   "Verify SSID spelling");
            }
            return DIAG_SKIPPED;
        }

        if (reason == (int)WIFI_REASON_AUTH_FAIL ||
            reason == (int)WIFI_REASON_HANDSHAKE_TIMEOUT ||
            reason == (int)WIFI_REASON_MIC_FAILURE ||
            reason == (int)WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY) {
            diag_menu_printf("Wi-Fi Test: FAILED (authentication, reason %d)\r\n",
                             reason);
            if (g_diag_err_ctx) {
                diag_err_add(g_diag_err_ctx,
                             "Wi-Fi auth failed (reason=%d)", reason);
                diag_err_set_debug(g_diag_err_ctx,
                                   "Check password with 'wifi-set pass <pw>'",
                                   "Check AP security mode (WPA2/WPA3)");
            }
            return DIAG_FAILED;
        }

        diag_menu_printf("Wi-Fi Test: FAILED (connect timeout, reason %d)\r\n",
                         reason);
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "Wi-Fi connect timeout (reason=%d)", reason);
            diag_err_set_debug(g_diag_err_ctx,
                               "Check AP reachability and DHCP server",
                               "Try 'wifi connect' and view the reason code");
        }
        return DIAG_FAILED;
    }

    /*------------------------------------------------------------------------*/
    /* Report status                                                          */
    /*------------------------------------------------------------------------*/

    hal_wifi_info_t info;
    if (hal_wifi_get_info(&info) != DIAG_PASSED)
        memset(&info, 0, sizeof(info));

    const char *strength = (info.rssi > -50) ? "excellent"
                         : (info.rssi > -67) ? "good"
                         : (info.rssi > -75) ? "fair"
                         :                    "poor";
    diag_menu_printf("Wi-Fi Test: connected, IP %u.%u.%u.%u, RSSI %d dBm (%s), "
                     "ch %u\r\n",
                     info.ip[0], info.ip[1], info.ip[2], info.ip[3],
                     info.rssi, strength, info.channel);
    diag_menu_printf("Wi-Fi Test: PASSED\r\n");

    hal_wifi_deinit();
    return DIAG_PASSED;
}
