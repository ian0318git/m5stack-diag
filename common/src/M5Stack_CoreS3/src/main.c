/*
 * main.c - M5Stack CoreS3 Diagnostic System Entry Point
 *
 * ESP-IDF app_main: initialises all subsystems, builds the test suite,
 * registers extended menu commands, and enters the UART menu loop.
 *
 * Clean Architecture layers:
 *   - Domain:          diag_core.h (types, entities)
 *   - Interface Adapter: diag_menu.h/c (console presenter/controller)
 *                       diag_runner.h/c (test orchestration)
 *   - Frameworks/Drivers: src/hal/*.c (concrete HAL implementations)
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
#include "diag_runner.h"

#include "hal_screen.h"
#include "hal_touch.h"
#include "hal_rtc.h"
#include "hal_imu.h"
#include "hal_power.h"

static const char *TAG = "m5s3_diag";

/*===========================================================================*/
/* Test function implementations                                             */
/*===========================================================================*/

/**
 * @brief I2C bus scan — probe all 127 possible addresses and report
 *        which peripherals respond.
 */
static diag_result_t test_i2c_scan(void *context)
{
    (void)context;
    diag_menu_printf("Scanning I2C bus for devices...\r\n");

    /* We re-use the I2C bus handle from the HAL layer.
     * For the scan, we temporarily add a dummy device at each address
     * and see if ACK is received.
     */
    extern i2c_master_bus_handle_t hal_i2c_bus_get(void);
    i2c_master_bus_handle_t bus = hal_i2c_bus_get();
    if (!bus) {
        diag_menu_printf("  FAIL: I2C bus not available\r\n");
        return DIAG_FAILED;
    }

    int found = 0;
    const uint8_t known_addrs[] = {
        CONFIG_I2C_ADDR_TOUCH,   /* 0x38 — FT6336 touch         */
        CONFIG_I2C_ADDR_RTC,     /* 0x51 — BM8563 RTC           */
        CONFIG_I2C_ADDR_IMU,     /* 0x69 — BMI270 IMU           */
        CONFIG_I2C_ADDR_POWER,   /* 0x34 — AXP2101 PMU          */
    };
    const char *known_names[] = {
        "FT6336 (Touch)",
        "BM8563 (RTC)",
        "BMI270 (IMU)",
        "AXP2101 (Power)",
    };

    for (size_t i = 0; i < sizeof(known_addrs); i++) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = known_addrs[i],
            .scl_speed_hz    = CONFIG_I2C_CLOCK_HZ,
        };

        i2c_master_dev_handle_t dev;
        esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &dev);
        if (err != ESP_OK) {
            diag_menu_printf("  [FAIL] 0x%02X (%s) — bus error\r\n",
                             known_addrs[i], known_names[i]);
            continue;
        }

        /* Send a zero-length write to test ACK */
        err = i2c_master_transmit(dev, NULL, 0, 100);
        if (err == ESP_OK) {
            diag_menu_printf("  [ OK ] 0x%02X (%s)\r\n",
                             known_addrs[i], known_names[i]);
            found++;
        } else {
            diag_menu_printf("  [ -- ] 0x%02X (%s) — no ACK\r\n",
                             known_addrs[i], known_names[i]);
        }

        i2c_master_bus_rm_device(dev);
    }

    diag_menu_printf("I2C scan complete: %d device(s) found\r\n", found);

    /* Check that at least the PMU (always present) responded */
    return (found > 0) ? DIAG_PASSED : DIAG_FAILED;
}

/**
 * @brief Screen test — fill screen with colours, draw lines, show text.
 */
static diag_result_t test_screen(void *context)
{
    (void)context;

    diag_result_t r = hal_screen_init();
    if (r != DIAG_PASSED) return r;

    /* Colour bars */
    int w = hal_screen_width();
    int h = hal_screen_height();
    int strip_h = h / 5;

    hal_screen_fill(HAL_SCREEN_COLOR_RED);
    vTaskDelay(pdMS_TO_TICKS(400));
    hal_screen_fill(HAL_SCREEN_COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(400));
    hal_screen_fill(HAL_SCREEN_COLOR_BLUE);
    vTaskDelay(pdMS_TO_TICKS(400));
    hal_screen_fill(HAL_SCREEN_COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(200));

    /* Draw text */
    hal_screen_set_font(2);
    const char *lines[] = { "CoreS3", "Diagnostic", "System", NULL };
    int y = 60;
    for (int i = 0; lines[i]; i++) {
        int text_w = strlen(lines[i]) * hal_screen_font_width();
        int tx = (w - text_w) / 2;
        hal_screen_draw_text(tx, y, lines[i],
                             HAL_SCREEN_COLOR_CYAN, HAL_SCREEN_COLOR_BLACK);
        y += hal_screen_font_height() + 4;
    }

    /* Draw crosshairs */
    hal_screen_draw_line(w / 2, 0, w / 2, h - 1, HAL_SCREEN_COLOR_WHITE);
    hal_screen_draw_line(0, h / 2, w - 1, h / 2, HAL_SCREEN_COLOR_WHITE);

    diag_menu_printf("Screen test complete.\r\n");
    hal_screen_deinit();
    return DIAG_PASSED;
}

/**
 * @brief Touch test — read touch controller and report status.
 */
static diag_result_t test_touch(void *context)
{
    (void)context;

    diag_result_t r = hal_touch_init();
    if (r != DIAG_PASSED) return r;

    uint8_t fw = hal_touch_firmware_version();
    int max_pts = hal_touch_max_points();
    diag_menu_printf("Touch: FT6336 fw=0x%02X max_points=%d\r\n", fw, max_pts);

    /* Read touch state once */
    hal_touch_data_t data;
    r = hal_touch_read(&data);
    if (r == DIAG_PASSED) {
        diag_menu_printf("Touch points: %d\r\n", data.point_count);
        for (uint8_t i = 0; i < data.point_count; i++) {
            diag_menu_printf("  Point %d: (%u, %u) event=%u id=%u\r\n",
                             i, data.points[i].x, data.points[i].y,
                             data.points[i].event, data.points[i].id);
        }
    }

    hal_touch_deinit();
    return (fw > 0) ? DIAG_PASSED : DIAG_FAILED;
}

/**
 * @brief RTC test — read current time from BM8563.
 */
static diag_result_t test_rtc(void *context)
{
    (void)context;

    diag_result_t r = hal_rtc_init();
    if (r != DIAG_PASSED) return r;

    hal_rtc_time_t t;
    r = hal_rtc_get_time(&t);
    if (r == DIAG_PASSED) {
        char buf[24];
        hal_rtc_format(&t, buf, sizeof(buf));
        diag_menu_printf("RTC time: %s\r\n", buf);
    }

    hal_rtc_deinit();
    return r;
}

/**
 * @brief IMU test — read BMI270 accelerometer and gyroscope.
 */
static diag_result_t test_imu(void *context)
{
    (void)context;

    diag_result_t r = hal_imu_init();
    if (r != DIAG_PASSED) return r;

    hal_imu_data_t data;
    r = hal_imu_read(&data);
    if (r == DIAG_PASSED) {
        diag_menu_printf("IMU: chip_id=0x%02X\r\n", data.chip_id);
        diag_menu_printf("  Accel (mg):   x=%+5d  y=%+5d  z=%+5d\r\n",
                         data.accel.x, data.accel.y, data.accel.z);
        diag_menu_printf("  Gyro  (mdps): x=%+6ld  y=%+6ld  z=%+6ld\r\n",
                         (long)data.gyro.x, (long)data.gyro.y, (long)data.gyro.z);
    }

    hal_imu_deinit();
    return r;
}

/**
 * @brief Power test — read AXP2101 PMU status.
 */
static diag_result_t test_power(void *context)
{
    (void)context;

    diag_result_t r = hal_power_init();
    if (r != DIAG_PASSED) return r;

    hal_power_data_t pwr;
    r = hal_power_read(&pwr);
    if (r == DIAG_PASSED) {
        diag_menu_printf("Power: version=0x%02X\r\n",
                         hal_power_chip_version());
        diag_menu_printf("  Battery: %u mV (%u%%)\r\n",
                         pwr.battery_millivolts, pwr.battery_percent);
        diag_menu_printf("  USB:     %u mV %s\r\n",
                         pwr.usb_millivolts,
                         (pwr.flags & HAL_POWER_FLAG_USB) ? "connected" : "disconnected");
        diag_menu_printf("  Charge:  %u mA %s\r\n",
                         pwr.charge_current_ma,
                         (pwr.flags & HAL_POWER_FLAG_BAT_CHARGING) ? "charging" :
                         (pwr.flags & HAL_POWER_FLAG_BAT_FULL) ? "full" : "idle");
        diag_menu_printf("  Temp:    %u C\r\n", pwr.temperature_celsius);
    }

    hal_power_deinit();
    return r;
}

/*===========================================================================*/
/* Extended menu commands                                                    */
/*===========================================================================*/

/** Command: full system status — runs all quick checks and summarises. */
static diag_result_t cmd_status(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    diag_menu_printf("\r\n========== System Status ==========\r\n");

    /* Chip info */
    diag_menu_printf("ESP32-S3\r\n");
    diag_menu_printf("Flash size: TODO\r\n");
    diag_menu_printf("PSRAM:      TODO\r\n");

    /* Power (always available) */
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

    /* RTC */
    if (hal_rtc_init() == DIAG_PASSED) {
        hal_rtc_time_t t;
        if (hal_rtc_get_time(&t) == DIAG_PASSED) {
            char buf[24];
            hal_rtc_format(&t, buf, sizeof(buf));
            diag_menu_printf("RTC: %s\r\n", buf);
        }
        hal_rtc_deinit();
    }

    /* IMU */
    if (hal_imu_init() == DIAG_PASSED) {
        diag_menu_printf("IMU: online\r\n");
        hal_imu_deinit();
    } else {
        diag_menu_printf("IMU: offline\r\n");
    }

    diag_menu_printf("====================================\r\n");
    return DIAG_PASSED;
}

/** Command: screen-on — turn display on. */
static diag_result_t cmd_screen_on(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    hal_screen_init();
    hal_screen_fill(HAL_SCREEN_COLOR_BLACK);
    diag_menu_printf("Screen ON\r\n");
    return DIAG_PASSED;
}

/** Command: screen-off — turn display off. */
static diag_result_t cmd_screen_off(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    hal_screen_deinit();
    diag_menu_printf("Screen OFF\r\n");
    return DIAG_PASSED;
}

/** Command: reboot — software reset. */
static diag_result_t cmd_reboot(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    diag_menu_printf("Rebooting...\r\n");
    esp_restart();
    return DIAG_PASSED; /* never reached */
}

/** Command: shutdown — power off via PMU. */
static diag_result_t cmd_shutdown(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;
    diag_menu_printf("Powering off...\r\n");
    vTaskDelay(pdMS_TO_TICKS(100));

    if (hal_power_init() == DIAG_PASSED) {
        hal_power_shutdown();  /* noreturn */
    }
    return DIAG_ERROR;
}

/*===========================================================================*/
/* Test suite definition                                                     */
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
};

static const diag_test_suite_t s_suite = {
    .name  = "M5Stack CoreS3 Hardware Diagnostics",
    .tests = s_tests,
    .count = DIAG_ARRAY_SIZE(s_tests),
};

/*===========================================================================*/
/* Entry point                                                               */
/*===========================================================================*/

void app_main(void)
{
    /* Initialise NVS (needed by some ESP-IDF components) */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        esp_err_t ret = nvs_flash_erase();
        if (ret == ESP_OK) {
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS init failed (%d)", ret);
        }
    }

    ESP_LOGI(TAG, "M5Stack CoreS3 Diagnostic System v1.0 starting...");

    /* Initialise UART console */
    if (diag_menu_init() != DIAG_PASSED) {
        ESP_LOGE(TAG, "Failed to initialise UART console");
        return;
    }

    /* Register extended commands */
    static const diag_menu_cmd_t ext_cmds[] = {
        { "status",     "Show system status",          cmd_status     },
        { "screen-on",  "Turn display on",             cmd_screen_on  },
        { "screen-off", "Turn display off",             cmd_screen_off },
        { "reboot",     "Software reset the system",    cmd_reboot     },
        { "shutdown",   "Power off the system",         cmd_shutdown   },
    };
    for (size_t i = 0; i < DIAG_ARRAY_SIZE(ext_cmds); i++) {
        diag_menu_register_cmd(&ext_cmds[i]);
    }

    /* Create the test runner */
    diag_runner_t *runner = diag_runner_create(&s_suite);
    if (!runner) {
        ESP_LOGE(TAG, "Failed to create test runner");
        return;
    }

    /* Enter the main menu loop */
    diag_menu_printf("System ready.  Type 'help' for commands.\r\n");
    diag_menu_loop(runner);

    /* Clean-up on exit (rarely reached) */
    diag_runner_destroy(runner);
    ESP_LOGI(TAG, "Diagnostic system terminated");
}
