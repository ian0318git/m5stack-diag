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
};

static const diag_test_suite_t s_suite = {
    .name  = "M5Stack CoreS3 Hardware Diagnostics",
    .tests = s_tests,
    .count = DIAG_ARRAY_SIZE(s_tests),
};

/*===========================================================================*/
/* Extended CLI commands                                                     */
/*===========================================================================*/

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
    hal_screen_init();
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
        { "status",     "Show system status",          cmd_status     },
        { "rtc-set",    "Set RTC: rtc-set YYYY MM DD HH MM SS", cmd_rtc_set },
        { "screen-on",  "Turn display on",             cmd_screen_on  },
        { "screen-off", "Turn display off",             cmd_screen_off },
        { "reboot",     "Software reset the system",    cmd_reboot     },
        { "shutdown",   "Power off the system",         cmd_shutdown   },
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
