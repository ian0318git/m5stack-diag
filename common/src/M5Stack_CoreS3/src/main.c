/*
 * main.c — M5Stack CoreS3 Diagnostic System Entry Point
 *
 * ESP-IDF app_main: initialises all subsystems, builds the test suite
 * (both the flat runner suite and the fugazi-style hierarchical menu),
 * registers extended menu commands, and enters the UART menu loop.
 *
 * This file is the COMPOSITION ROOT — it wires together domain,
 * adapter, and infrastructure layers but contains no test logic.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "diag_core.h"
#include "diag_config.h"
#include "diag_menu.h"
#include "diag_menu_core.h"
#include "diag_runner.h"
#include "diag_error.h"
#include "diag_tests.h"

/* HAL includes for extended CLI commands */
#include "hal_screen.h"
#include "hal_touch.h"
#include "hal_rtc.h"
#include "hal_imu.h"
#include "hal_power.h"
#include "hal_wifi.h"
#include "diag_net.h"

static const char *TAG = "m5s3_diag";

/*===========================================================================*/
/* Global error context — shared by all test functions                       */
/*===========================================================================*/

diag_err_ctx_t *g_diag_err_ctx = NULL;
static diag_err_ctx_t s_err_ctx;

/*===========================================================================*/
/* Fugazi-style submenu definition                                           */
/*===========================================================================*/

#define F_I2C    (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT)
#define F_PERIPH (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT)

static const diag_menu_xtable_t s_main_menu[] = {
    { "I2C Bus Scan",             fugazi_test_i2c_scan, 0, F_I2C,   NULL, 0 },
    { "Display (ILI9342C)",       fugazi_test_screen,   0, F_PERIPH, NULL, 0 },
    { "Touch (FT6336)",           fugazi_test_touch,    0, F_PERIPH, NULL, 0 },
    { "RTC (BM8563)",             fugazi_test_rtc,      0, F_PERIPH, NULL, 0 },
    { "IMU (BMI270)",             fugazi_test_imu,      0, F_PERIPH, NULL, 0 },
    { "Power (AXP2101)",          fugazi_test_power,    0, F_PERIPH, NULL, 0 },
    { "Backlight (DLDO1)",        fugazi_test_backlight,0, F_PERIPH, NULL, 0 },
    { "Speaker (AW88298)",        fugazi_test_speaker,  0, F_PERIPH, NULL, 0 },
    { "Microphone (ES7210)",      fugazi_test_microphone,0,F_PERIPH, NULL, 0 },
    { "Camera (GC0308)",          fugazi_test_camera,   0, F_PERIPH, NULL, 0 },
    { "Proximity (LTR-553)",      fugazi_test_proximity,0, F_PERIPH, NULL, 0 },
    { "SD Card (microSD)",        fugazi_test_sdcard,   0, F_PERIPH, NULL, 0 },
    { "Button (PWR)",             fugazi_test_button,   0, F_PERIPH, NULL, 0 },
    { "Wi-Fi Connectivity",       fugazi_test_wifi,     0, F_PERIPH, NULL, 0 },
};
#define MAIN_MENU_COUNT (sizeof(s_main_menu) / sizeof(s_main_menu[0]))

static diag_menu_t s_fugazi_menu;

/*===========================================================================*/
/* Flat test suite (for the original CLI runner)                             */
/*===========================================================================*/

static const diag_test_t s_tests[] = {
    DIAG_TEST_ENTRY(DIAG_TEST_I2C_SCAN, "i2c-scan",  "Scan I2C bus for devices",
                    test_i2c_scan, NULL, CONFIG_I2C_SCAN_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_SCREEN,   "screen",    "Display colour bars and text",
                    test_screen,  NULL, CONFIG_LCD_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_TOUCH,    "touch",     "Read FT6336 touch controller",
                    test_touch,   NULL, CONFIG_TOUCH_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_RTC,      "rtc",       "Read BM8563 real-time clock",
                    test_rtc,     NULL, CONFIG_RTC_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_IMU,      "imu",       "Read BMI270 accelerometer/gyro",
                    test_imu,     NULL, CONFIG_IMU_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_POWER,    "power",     "Read AXP2101 PMU status",
                    test_power,   NULL, CONFIG_POWER_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_BACKLIGHT,"backlight", "Toggle LCD backlight",
                    test_backlight, NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_SPEAKER,  "speaker",   "Play 1 kHz tone via AW88298",
                    test_speaker,  NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_MICROPHONE,"mic",      "Capture audio via ES7210",
                    test_microphone, NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_CAMERA,   "camera",    "Probe GC0308 image sensor",
                    test_camera,   NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_PROXIMITY,"proximity", "Read LTR-553 ALS + proximity",
                    test_proximity,NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_SDCARD,   "sdcard",    "Mount/write/read microSD",
                    test_sdcard,   NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_BUTTON,   "button",    "Press side PWR button",
                    test_button,  NULL, CONFIG_DEFAULT_TEST_TIMEOUT_MS),
    DIAG_TEST_ENTRY(DIAG_TEST_WIFI,     "wifi",      "Join Wi-Fi AP, get IP, report RSSI",
                    test_wifi,    NULL, CONFIG_WIFI_TEST_TIMEOUT_MS),
};

static const diag_test_suite_t s_suite = {
    .name  = "M5Stack CoreS3 Hardware Diagnostics",
    .tests = s_tests,
    .count = DIAG_ARRAY_SIZE(s_tests),
};

/*===========================================================================*/
/* Extended CLI commands                                                     */
/*===========================================================================*/

static diag_result_t cmd_burnin(diag_runner_t *runner, int argc, char *argv[])
{
    int iterations = 100;   /* DFS default */
    if (argc >= 2) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10000) iterations = 10000;
    }

    diag_menu_printf("\r\n========== Burn-In Test ==========\r\n");
    diag_menu_printf("  Target: %d iterations\r\n", iterations);
    diag_menu_printf("  Running all P0+P1 tests in sequence...\r\n\n");

    int total_failures = 0;
    int stop_on_fail = 1;  /* stop at first failure per DFS */
    int completed = 0;

    for (int i = 1; i <= iterations; i++) {
        diag_menu_printf("--- Iteration %d/%d ---\r\n", i, iterations);

        int failures = diag_runner_run_all(runner, NULL, NULL);
        total_failures += failures;
        completed = i;  /* track actual completed count even on early break */

        if (failures > 0) {
            diag_menu_printf("** FAILED (%d test(s) failed)\r\n", failures);
        } else {
            diag_menu_printf("** PASSED\r\n");
        }

        if (failures > 0 && stop_on_fail) {
            diag_menu_printf("\r\nBurn-In aborted: first failure at iteration %d\r\n", i);
            break;
        }

        /* Small delay between iterations to let hardware settle */
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    diag_menu_printf("\r\n========== Burn-In Summary ==========\r\n");
    diag_menu_printf("  Completed: %d/%d iterations\r\n",
                     completed, iterations);
    diag_menu_printf("  Total failures: %d\r\n", total_failures);

    if (total_failures == 0) {
        diag_menu_printf("  Result: PASSED\r\n");
    } else {
        diag_menu_printf("  Result: FAILED\r\n");
    }

    return (total_failures == 0) ? DIAG_PASSED : DIAG_FAILED;
}

static diag_result_t cmd_status(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc; (void)argv; (void)runner;

    diag_menu_printf("\r\n========== System Status ==========\r\n");
    diag_menu_printf("ESP32-S3\r\n");

    if (hal_power_init() == DIAG_PASSED) {
        hal_power_data_t pwr;
        if (hal_power_read(&pwr) == DIAG_PASSED) {
            diag_menu_printf("Battery: %u mV (%u%%)\r\n",
                             pwr.battery_millivolts, pwr.battery_percent);
            diag_menu_printf("USB: %s\r\n",
                             (pwr.flags & HAL_POWER_FLAG_USB) ?
                             "connected" : "disconnected");
        }
        hal_power_deinit();
    }

    if (hal_rtc_init() == DIAG_PASSED) {
        hal_rtc_time_t t;
        if (hal_rtc_get_time(&t) == DIAG_PASSED) {
            char buf[24];
            hal_rtc_format(&t, buf, sizeof(buf));
            diag_menu_printf("RTC: %s\r\n", buf);
        }
        hal_rtc_deinit();
    }

    if (hal_imu_init() == DIAG_PASSED) {
        diag_menu_printf("IMU: online\r\n");
        hal_imu_deinit();
    } else {
        diag_menu_printf("IMU: offline\r\n");
    }

    diag_menu_printf("====================================\r\n");
    return DIAG_PASSED;
}

static diag_result_t cmd_rtc_set(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner;
    if (argc < 7) {
        diag_menu_printf("Usage: rtc-set YYYY MM DD HH MM SS\r\n");
        return DIAG_FAILED;
    }

    int y = atoi(argv[1]), mo = atoi(argv[2]), d = atoi(argv[3]);
    int h = atoi(argv[4]), mi = atoi(argv[5]), s = atoi(argv[6]);

    if (mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || s > 59) {
        diag_menu_printf("Invalid date/time values\r\n");
        return DIAG_FAILED;
    }

    if (hal_rtc_init() != DIAG_PASSED) {
        diag_menu_printf("RTC init failed\r\n");
        return DIAG_FAILED;
    }

    hal_rtc_time_t t = { .year = y, .month = mo, .day = d,
                         .hour = h, .minute = mi, .second = s, .weekday = 0 };
    diag_result_t r = hal_rtc_set_time(&t);
    hal_rtc_deinit();

    if (r == DIAG_PASSED) {
        diag_menu_printf("RTC set to %04d-%02d-%02d %02d:%02d:%02d\r\n",
                         y, mo, d, h, mi, s);
    } else {
        diag_menu_printf("RTC set failed\r\n");
    }
    return r;
}

static diag_result_t cmd_screen_on(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    diag_result_t r = hal_screen_init();
    if (r != DIAG_PASSED) {
        diag_menu_printf("Screen init failed\r\n");
        return r;
    }
    hal_screen_fill(HAL_SCREEN_COLOR_BLACK);
    diag_menu_printf("Screen ON\r\n");
    return DIAG_PASSED;
}

static diag_result_t cmd_screen_off(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    hal_screen_deinit();
    diag_menu_printf("Screen OFF\r\n");
    return DIAG_PASSED;
}

static diag_result_t cmd_reboot(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    diag_menu_printf("Rebooting...\r\n");
    esp_restart();
    return DIAG_PASSED;
}

static diag_result_t cmd_shutdown(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    diag_menu_printf("Powering off...\r\n");
    vTaskDelay(pdMS_TO_TICKS(100));

    if (hal_power_init() == DIAG_PASSED) {
        hal_power_shutdown();
    }
    return DIAG_ERROR;
}

/*---------------------------------------------------------------------------*/
/* Wi-Fi / network commands                                                  */
/*---------------------------------------------------------------------------*/

static diag_result_t cmd_wifi(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner;

    char ssid[64], pass[64];
    hal_wifi_cfg_get_ssid(ssid, sizeof(ssid));
    hal_wifi_cfg_get_password(pass, sizeof(pass));

    diag_menu_printf("\r\n========== Wi-Fi Status ==========\r\n");
    diag_menu_printf("SSID: %s\r\n",
                     ssid[0] ? ssid : "(none — use 'wifi-set ssid <name>')");

    if (argc >= 2 && strcmp(argv[1], "connect") == 0) {
        if (ssid[0] == '\0') {
            diag_menu_printf("Cannot connect: no SSID\r\n");
            return DIAG_FAILED;
        }
        diag_menu_printf("Connecting...\r\n");
        diag_result_t r = hal_wifi_connect(ssid, pass,
                                           CONFIG_WIFI_CONNECT_TIMEOUT_MS);
        if (r == DIAG_PASSED) {
            hal_wifi_info_t info;
            if (hal_wifi_get_info(&info) == DIAG_PASSED) {
                diag_menu_printf("Connected: IP %u.%u.%u.%u, RSSI %d dBm, "
                                 "ch %u\r\n",
                                 info.ip[0], info.ip[1], info.ip[2],
                                 info.ip[3], info.rssi, info.channel);
            }
        } else {
            diag_menu_printf("Connect failed (reason=%d)\r\n",
                             hal_wifi_get_disconnect_reason());
        }
        hal_wifi_deinit();
        return r;
    }

    if (argc >= 2 && strcmp(argv[1], "disconnect") == 0) {
        hal_wifi_deinit();
        diag_menu_printf("Wi-Fi disconnected\r\n");
    }

    if (argc >= 2 && strcmp(argv[1], "ping") == 0) {
        if (ssid[0] == '\0') {
            diag_menu_printf("Cannot ping: no SSID\r\n");
            return DIAG_FAILED;
        }
        diag_menu_printf("Connecting...\r\n");
        diag_result_t cr = hal_wifi_connect(ssid, pass,
                                            CONFIG_WIFI_CONNECT_TIMEOUT_MS);
        if (cr != DIAG_PASSED) {
            diag_menu_printf("Connect failed (reason=%d)\r\n",
                             hal_wifi_get_disconnect_reason());
            hal_wifi_deinit();
            return cr;
        }

        const char *target = (argc >= 3) ? argv[2] : "1.1.1.1";
        diag_menu_printf("Pinging %s...\r\n", target);
        uint32_t sent = 0, received = 0, min_ms = 0, avg_ms = 0, max_ms = 0;
        diag_result_t r = diag_net_ping(target, 0, &sent, &received,
                                        &min_ms, &avg_ms, &max_ms);
        if (r == DIAG_PASSED) {
            diag_menu_printf("Ping %s: %lu/%lu replies, RTT "
                             "%lu/%lu/%lu ms (min/avg/max)\r\n",
                             target, (unsigned long)received,
                             (unsigned long)sent,
                             (unsigned long)min_ms, (unsigned long)avg_ms,
                             (unsigned long)max_ms);
        } else {
            diag_menu_printf("Ping %s: FAILED (%lu/%lu replies)\r\n",
                             target, (unsigned long)received,
                             (unsigned long)sent);
        }
        hal_wifi_deinit();
        return r;
    }

    diag_menu_printf("====================================\r\n");
    return DIAG_PASSED;
}

static diag_result_t cmd_wifi_set(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner;
    if (argc < 3 && !(argc == 2 && strcmp(argv[1], "clear") == 0)) {
        diag_menu_printf("Usage: wifi-set ssid|pass|url|ntp|mqtt <value> "
                         "| wifi-set clear\r\n");
        diag_menu_printf("  Credentials are stored in NVS and override "
                         "menuconfig\r\n");
        return DIAG_FAILED;
    }

    diag_result_t r;
    if (argc == 2 && strcmp(argv[1], "clear") == 0) {
        r = hal_wifi_cfg_clear();
        diag_menu_printf("wifi-set clear: %s\r\n",
                         r == DIAG_PASSED ? "NVS overrides erased" : "FAILED");
        return r;
    }

    if      (strcmp(argv[1], "ssid") == 0) r = hal_wifi_cfg_set_ssid(argv[2]);
    else if (strcmp(argv[1], "pass") == 0) r = hal_wifi_cfg_set_password(argv[2]);
    else if (strcmp(argv[1], "url")  == 0) r = hal_wifi_cfg_set_upload_url(argv[2]);
    else if (strcmp(argv[1], "ntp")  == 0) r = hal_wifi_cfg_set_ntp(argv[2]);
    else if (strcmp(argv[1], "mqtt") == 0) r = hal_wifi_cfg_set_mqtt_url(argv[2]);
    else {
        diag_menu_printf("Unknown key: %s\r\n", argv[1]);
        return DIAG_FAILED;
    }
    diag_menu_printf("wifi-set %s: %s\r\n", argv[1],
                     r == DIAG_PASSED ? "saved to NVS" : "FAILED");
    return r;
}

static diag_result_t cmd_ntp_sync(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner;
    char server[64], ssid[64], pass[64];
    if (argc >= 2)
        snprintf(server, sizeof(server), "%s", argv[1]);
    else
        hal_wifi_cfg_get_ntp(server, sizeof(server));

    hal_wifi_cfg_get_ssid(ssid, sizeof(ssid));
    hal_wifi_cfg_get_password(pass, sizeof(pass));

    if (ssid[0] == '\0') {
        diag_menu_printf("NTP sync failed: no Wi-Fi credentials configured\r\n");
        return DIAG_FAILED;
    }

    /* Retry the whole connect+sync cycle once (same pattern as mqtt-pub):
       right after a run-all the Wi-Fi driver can still be settling. */
    diag_result_t r = DIAG_FAILED;
    for (int attempt = 1; attempt <= 2; attempt++) {
        if (attempt > 1) {
            diag_menu_printf("NTP sync: retry %d/2 in 3 s...\r\n", attempt);
            vTaskDelay(pdMS_TO_TICKS(3000));
        }

        diag_menu_printf("NTP sync: connecting to '%s'...\r\n", ssid);
        if (hal_wifi_connect(ssid, pass, CONFIG_WIFI_CONNECT_TIMEOUT_MS) != DIAG_PASSED) {
            diag_menu_printf("NTP sync failed: Wi-Fi connect (reason=%d)\r\n",
                             hal_wifi_get_disconnect_reason());
            hal_wifi_deinit();
            continue;
        }

        diag_menu_printf("NTP sync: waiting for %s...\r\n", server);
        r = diag_net_ntp_sync(server, CONFIG_NTP_SYNC_TIMEOUT_MS);
        hal_wifi_deinit();
        if (r == DIAG_PASSED)
            break;
    }

    if (r != DIAG_PASSED) {
        diag_menu_printf("NTP sync FAILED (RTC unchanged)\r\n");
        return r;
    }

    if (hal_rtc_init() == DIAG_PASSED) {
        hal_rtc_time_t t;
        if (hal_rtc_get_time(&t) == DIAG_PASSED) {
            char buf[24];
            diag_menu_printf("RTC set: %s\r\n",
                             hal_rtc_format(&t, buf, sizeof(buf)));
        }
        hal_rtc_deinit();
    }
    return DIAG_PASSED;
}

static diag_result_t cmd_upload(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc; (void)argv;
    char url[256], ssid[64], pass[64];
    hal_wifi_cfg_get_upload_url(url, sizeof(url));
    if (url[0] == '\0') {
        diag_menu_printf("Upload failed: no URL. Use: wifi-set url "
                         "http://host:port/endpoint\r\n");
        return DIAG_FAILED;
    }

    hal_wifi_cfg_get_ssid(ssid, sizeof(ssid));
    hal_wifi_cfg_get_password(pass, sizeof(pass));
    if (ssid[0] == '\0') {
        diag_menu_printf("Upload failed: no Wi-Fi credentials configured\r\n");
        return DIAG_FAILED;
    }

    /* Retry the whole connect+upload cycle once (same pattern as mqtt-pub). */
    int http_status = 0;
    diag_result_t r = DIAG_FAILED;
    for (int attempt = 1; attempt <= 2; attempt++) {
        if (attempt > 1) {
            diag_menu_printf("Upload: retry %d/2 in 3 s...\r\n", attempt);
            vTaskDelay(pdMS_TO_TICKS(3000));
        }

        diag_menu_printf("Upload: connecting to '%s'...\r\n", ssid);
        if (hal_wifi_connect(ssid, pass, CONFIG_WIFI_CONNECT_TIMEOUT_MS) != DIAG_PASSED) {
            diag_menu_printf("Upload failed: Wi-Fi connect (reason=%d)\r\n",
                             hal_wifi_get_disconnect_reason());
            hal_wifi_deinit();
            continue;
        }

        hal_wifi_info_t info;
        if (hal_wifi_get_info(&info) != DIAG_PASSED)
            memset(&info, 0, sizeof(info));

        r = diag_net_upload_results(runner, g_diag_err_ctx,
                                    url, &info, &http_status);
        hal_wifi_deinit();
        if (r == DIAG_PASSED)
            break;
    }
    diag_menu_printf("Upload: HTTP %d — %s\r\n", http_status,
                     r == DIAG_PASSED ? "PASSED" : "FAILED");
    return r;
}

static diag_result_t cmd_mqtt_pub(diag_runner_t *runner, int argc, char *argv[])
{
    char broker[256], ssid[64], pass[64];
    hal_wifi_cfg_get_mqtt_url(broker, sizeof(broker));
    if (broker[0] == '\0') {
        diag_menu_printf("MQTT publish failed: no broker. Use: wifi-set mqtt "
                         "mqtt://host:1883\r\n");
        return DIAG_FAILED;
    }

    hal_wifi_cfg_get_ssid(ssid, sizeof(ssid));
    hal_wifi_cfg_get_password(pass, sizeof(pass));
    if (ssid[0] == '\0') {
        diag_menu_printf("MQTT publish failed: no Wi-Fi credentials "
                         "configured\r\n");
        return DIAG_FAILED;
    }

    const char *topic = (argc >= 2) ? argv[1] : NULL;
    bool pub_ok = false;
    diag_result_t r = DIAG_FAILED;

    /* Retry the whole connect+publish cycle once: immediately after a
       run-all the Wi-Fi driver can still be settling, and a fresh
       init+connect occasionally fails transiently. */
    for (int attempt = 1; attempt <= 2; attempt++) {
        if (attempt > 1) {
            diag_menu_printf("MQTT: retry %d/2 in 3 s...\r\n", attempt);
            vTaskDelay(pdMS_TO_TICKS(3000));
        }

        diag_menu_printf("MQTT: connecting to '%s'...\r\n", ssid);
        if (hal_wifi_connect(ssid, pass, CONFIG_WIFI_CONNECT_TIMEOUT_MS) != DIAG_PASSED) {
            diag_menu_printf("MQTT publish failed: Wi-Fi connect (reason=%d)\r\n",
                             hal_wifi_get_disconnect_reason());
            hal_wifi_deinit();
            continue;
        }

        hal_wifi_info_t info;
        if (hal_wifi_get_info(&info) != DIAG_PASSED)
            memset(&info, 0, sizeof(info));

        r = diag_net_publish_mqtt(runner, g_diag_err_ctx,
                                  broker, topic, &info, &pub_ok);
        hal_wifi_deinit();
        if (r == DIAG_PASSED)
            break;
    }

    diag_menu_printf("MQTT publish: %s (%s)\r\n",
                     r == DIAG_PASSED ? "PASSED" : "FAILED",
                     pub_ok ? "acknowledged" : "no ack");
    return r;
}

/*===========================================================================*/
/* Entry point                                                               */
/*===========================================================================*/

void app_main(void)
{
    /* Initialise NVS */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        esp_err_t ret = nvs_flash_erase();
        if (ret == ESP_OK) ret = nvs_flash_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS init failed (%d)", ret);
        }
    }

    ESP_LOGI(TAG, "M5Stack CoreS3 Diagnostic System v1.0 starting...");

    /* Initialise UART console */
    if (diag_menu_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "Failed to initialise console");
        return;
    }

    /* Initialise error context and set global pointer */
    diag_err_init(&s_err_ctx);
    g_diag_err_ctx = &s_err_ctx;

    /* Build the fugazi-style interactive menu */
    if (diag_menu_build(&s_fugazi_menu, s_main_menu, MAIN_MENU_COUNT,
                        "CoreS3 Diagnostics") == DIAG_PASSED) {
        diag_menu_set_fugazi(&s_fugazi_menu, &s_err_ctx);
    }

    /* Register extended CLI commands */
    static const diag_menu_cmd_t ext_cmds[] = {
        { "burnin",    "Burn-in: burnin [iterations]",  cmd_burnin     },
        { "status",     "Show system status",          cmd_status     },
        { "rtc-set",    "Set RTC: rtc-set YYYY MM DD HH MM SS", cmd_rtc_set },
        { "screen-on",  "Turn display on",             cmd_screen_on  },
        { "screen-off", "Turn display off",             cmd_screen_off },
        { "reboot",     "Software reset the system",    cmd_reboot     },
        { "shutdown",   "Power off the system",         cmd_shutdown   },
        { "wifi",       "Wi-Fi status: wifi [connect|disconnect|ping <host>]", cmd_wifi },
        { "wifi-set",   "Set Wi-Fi/NTP/upload config: wifi-set ssid|pass|url|ntp|mqtt <v>|clear", cmd_wifi_set },
        { "ntp-sync",   "Sync RTC from NTP: ntp-sync [server]",  cmd_ntp_sync },
        { "upload",     "POST JSON test report to upload URL",   cmd_upload },
        { "mqtt-pub",   "Publish JSON test report to MQTT broker: mqtt-pub [topic]", cmd_mqtt_pub },
    };
    for (size_t i = 0; i < DIAG_ARRAY_SIZE(ext_cmds); i++) {
        diag_menu_register_cmd(&ext_cmds[i]);
    }

    /* Create the flat test runner */
    diag_runner_t *runner = diag_runner_create(&s_suite);
    if (!runner) {
        ESP_LOGE(TAG, "Failed to create test runner");
        return;
    }

    /* Enter the main CLI loop */
    diag_menu_printf("System ready.  Type 'help' for commands.\r\n");
    diag_menu_loop(runner);

    diag_runner_destroy(runner);
    diag_menu_destroy(&s_fugazi_menu);
    ESP_LOGI(TAG, "Diagnostic system terminated");
}
