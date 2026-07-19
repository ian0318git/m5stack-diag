/*
 * main.c - M5Stack CoreS3 Diagnostic System Entry Point
 *
 * ESP-IDF app_main: initialises all subsystems, builds the test suite
 * (both the flat runner suite and the fugazi-style hierarchical menu),
 * registers extended menu commands, and enters the UART menu loop.
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
#include "driver/i2c_master.h"

/*===========================================================================*/
/* HAL includes                                                              */
/*===========================================================================*/

#include "hal_screen.h"
#include "hal_touch.h"
#include "hal_rtc.h"
#include "hal_imu.h"
#include "hal_power.h"

static const char *TAG = "m5s3_diag";

/*===========================================================================*/
/* Global error context (shared by all test functions)                       */
/*===========================================================================*/

static diag_err_ctx_t s_err_ctx;

/*===========================================================================*/
/* Test function implementations                                             */
/*===========================================================================*/

static diag_result_t test_i2c_scan(void *context)
{
    (void)context;

    diag_err_set_component(&s_err_ctx, "I2C", "MB/I2C");
    diag_menu_printf("Scanning full I2C address range 0x01-0x7F...\r\n");

    extern i2c_master_bus_handle_t hal_i2c_bus_get(void);
    i2c_master_bus_handle_t bus = hal_i2c_bus_get();
    if (!bus) {
        diag_err_add(&s_err_ctx, "I2C bus not available");
        diag_err_set_debug(&s_err_ctx,
                           "Check power supply to the I2C bus",
                           "Check SDA/SCL pull-up resistors");
        return DIAG_FAILED;
    }

    int found = 0;
    diag_menu_printf("\r\n  Found devices:\r\n");

    /* Full scan: probe every 7-bit address from 0x01 to 0x7F */
    for (uint16_t addr = 1; addr < 0x80; addr++) {
        esp_err_t err = i2c_master_probe(bus, addr, 50);
        if (err == ESP_OK) {
            /* Known devices get named labels */
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

    /* Cross-check: warn if any expected device is missing */
    const uint8_t expected[] = {
        CONFIG_I2C_ADDR_RTC,
        CONFIG_I2C_ADDR_IMU,
        CONFIG_I2C_ADDR_POWER,
        CONFIG_I2C_ADDR_TOUCH,
    };
    const char *expected_names[] = {
        "BM8563 (RTC)",
        "BMI270 (IMU)",
        "AXP2101 (Power)",
        "FT6336 (Touch)",
    };
    const char *expected_hints[] = {
        "RTC powered by AXP2101 RTC_VDD — check PMU power status",
        "IMU powered by AXP2101 SYS_3V3 rail",
        "PMU is the root power device — check board power input",
        "FT6336U requires AXP2101 LDOIO0 (reg 0x90) + AW9523B P0_0 RST release. Run Touch test to power on, or probe is expected NACK",
    };

    int missing = 0;
    for (size_t i = 0; i < sizeof(expected); i++) {
        if (i2c_master_probe(bus, expected[i], 50) != ESP_OK) {
            diag_menu_printf("  ** MISSING: 0x%02X %s\r\n",
                             expected[i], expected_names[i]);
            diag_err_add(&s_err_ctx, "I2C@0x%02X %s: device not found on bus",
                         expected[i], expected_names[i]);
            if (expected_hints[i]) {
                diag_err_set_debug(&s_err_ctx, expected_hints[i], NULL);
            }
            missing++;
        }
    }

    /* Report optional devices absent (not a failure) */
    {
        const uint8_t opt[] = { 0x36, 0x21, 0x23 };
        const char *on[] = { "AW88298 (Speaker)", "GC0308 (Camera)", "LTR-553 (Prox)" };
        for (size_t i = 0; i < sizeof(opt); i++) {
            if (i2c_master_probe(bus, opt[i], 50) != ESP_OK) {
                diag_menu_printf("  -- 0x%02X %s — optional, skip\r\n", opt[i], on[i]);
            }
        }
    }

    /* Check alt touch address 0x3A */
    if (i2c_master_probe(bus, 0x3A, 50) == ESP_OK) {
        diag_menu_printf("  ** NOTE: Touch found at 0x3A (not 0x38)\r\n");
        diag_err_add(&s_err_ctx, "FT6336: found at 0x3A, not expected 0x38");
    }

    if (missing > 0) {
        diag_err_set_debug(&s_err_ctx,
                           "Check power supply to the missing device",
                           "Verify pull-up resistors and I2C connections");
        return DIAG_FAILED;
    }

    return DIAG_PASSED;
}

static diag_result_t test_screen(void *context)
{
    (void)context;

    diag_err_set_component(&s_err_ctx, "SCREEN", "MB/LCD");

    diag_result_t r = hal_screen_init();
    if (r != DIAG_PASSED) {
        diag_err_add(&s_err_ctx, "ILI9342C screen init failed");
        diag_err_set_debug(&s_err_ctx,
                           "Check SPI bus (MOSI=G37, SCLK=G36, CS=G3, DC=G35)",
                           "Check AW9523B P1_1 LCD_RST and AXP2101 DLDO1 backlight");
        return r;
    }

    int w = hal_screen_width();
    int h = hal_screen_height();

    hal_screen_fill(HAL_SCREEN_COLOR_RED);    vTaskDelay(pdMS_TO_TICKS(300));
    hal_screen_fill(HAL_SCREEN_COLOR_GREEN);  vTaskDelay(pdMS_TO_TICKS(300));
    hal_screen_fill(HAL_SCREEN_COLOR_BLUE);   vTaskDelay(pdMS_TO_TICKS(300));
    hal_screen_fill(HAL_SCREEN_COLOR_BLACK);  vTaskDelay(pdMS_TO_TICKS(200));

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

    hal_screen_draw_line(w / 2, 0, w / 2, h - 1, HAL_SCREEN_COLOR_WHITE);
    hal_screen_draw_line(0, h / 2, w - 1, h / 2, HAL_SCREEN_COLOR_WHITE);

    diag_menu_printf("Screen test complete.\r\n");
    hal_screen_deinit();
    return DIAG_PASSED;
}

static diag_result_t test_touch(void *context)
{
    (void)context;

    diag_err_set_component(&s_err_ctx, "TOUCH", "MB/TOUCH");

    diag_result_t r = hal_touch_init();
    if (r != DIAG_PASSED) {
        diag_err_add(&s_err_ctx,
                     "I2C@0x38 FT6336: init failed (no ACK)");
        diag_err_set_debug(&s_err_ctx,
                           "Check AXP2101 LDOIO0 touch power (reg 0x90)",
                           "Check I2C bus 0x38 pull-ups and INT/RST pins");
        return r;
    }

    uint8_t fw = hal_touch_firmware_version();
    if (fw == 0) {
        diag_err_add(&s_err_ctx,
                     "I2C@0x38 FT6336: firmware version read returned 0");
    }
    diag_menu_printf("Touch: FT6336 fw=0x%02X max_points=%d\r\n",
                     fw, hal_touch_max_points());

    hal_touch_data_t data;
    r = hal_touch_read(&data);
    if (r == DIAG_PASSED) {
        diag_menu_printf("Touch points: %d\r\n", data.point_count);
        for (uint8_t i = 0; i < data.point_count; i++) {
            diag_menu_printf("  Point %d: (%u, %u) event=%u id=%u\r\n",
                             i, data.points[i].x, data.points[i].y,
                             data.points[i].event, data.points[i].id);
        }
    } else {
        diag_err_add(&s_err_ctx,
                     "I2C@0x38 FT6336: read touch data failed");
    }

    hal_touch_deinit();
    return (fw > 0 && r == DIAG_PASSED) ? DIAG_PASSED : DIAG_FAILED;
}

static diag_result_t test_rtc(void *context)
{
    (void)context;

    diag_err_set_component(&s_err_ctx, "RTC", "MB/RTC");

    diag_result_t r = hal_rtc_init();
    if (r != DIAG_PASSED) {
        diag_err_add(&s_err_ctx,
                     "I2C@0x51 BM8563: init failed (no ACK)");
        diag_err_set_debug(&s_err_ctx,
                           "Check I2C bus 0x51 pull-ups",
                           "Check RTC battery backup voltage");
        return r;
    }

    hal_rtc_time_t t;
    r = hal_rtc_get_time(&t);
    if (r == DIAG_PASSED) {
        char buf[24];
        hal_rtc_format(&t, buf, sizeof(buf));
        diag_menu_printf("RTC time: %s\r\n", buf);
    } else {
        diag_err_add(&s_err_ctx,
                     "I2C@0x51 BM8563: read time failed");
    }

    hal_rtc_deinit();
    return r;
}

static diag_result_t test_imu(void *context)
{
    (void)context;

    diag_err_set_component(&s_err_ctx, "IMU", "MB/IMU");

    diag_result_t r = hal_imu_init();
    if (r != DIAG_PASSED) {
        diag_err_add(&s_err_ctx,
                     "I2C@0x69 BMI270: init failed (wrong chip ID or no ACK)");
        diag_err_set_debug(&s_err_ctx,
                           "Check I2C bus 0x69 pull-ups",
                           "Verify BMI270 power supply");
        return r;
    }

    hal_imu_data_t data;
    r = hal_imu_read(&data);
    if (r == DIAG_PASSED) {
        diag_menu_printf("IMU: chip_id=0x%02X\r\n", data.chip_id);
        diag_menu_printf("  Accel (mg):   x=%+5d  y=%+5d  z=%+5d\r\n",
                         data.accel.x, data.accel.y, data.accel.z);
        diag_menu_printf("  Gyro  (mdps): x=%+6ld  y=%+6ld  z=%+6ld\r\n",
                         (long)data.gyro.x, (long)data.gyro.y, (long)data.gyro.z);
    } else {
        diag_err_add(&s_err_ctx,
                     "I2C@0x69 BMI270: read sensor data failed");
    }

    hal_imu_deinit();
    return r;
}

static diag_result_t test_power(void *context)
{
    (void)context;

    diag_err_set_component(&s_err_ctx, "POWER", "MB/PMU");

    diag_result_t r = hal_power_init();
    if (r != DIAG_PASSED) {
        diag_err_add(&s_err_ctx,
                     "I2C@0x34 AXP2101: init failed (no ACK)");
        diag_err_set_debug(&s_err_ctx,
                           "Check I2C bus 0x34 pull-ups",
                           "Check battery connection and PMU power rails");
        return r;
    }

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
    } else {
        diag_err_add(&s_err_ctx,
                     "I2C@0x34 AXP2101: read PMU data failed");
    }

    hal_power_deinit();
    return r;
}

/*===========================================================================*/
/* Fugazi-style test function wrappers (int param signature)                 */
/*===========================================================================*/

/*
 * These wrappers allow the existing test functions to be called from the
 * fugazi-style menu which uses the (int param) function signature.
 */

static diag_result_t fugazi_test_i2c_scan(int param)  { (void)param; return test_i2c_scan(NULL); }
static diag_result_t fugazi_test_screen(int param)    { (void)param; return test_screen(NULL); }
static diag_result_t fugazi_test_touch(int param)     { (void)param; return test_touch(NULL); }
static diag_result_t fugazi_test_rtc(int param)       { (void)param; return test_rtc(NULL); }
static diag_result_t fugazi_test_imu(int param)       { (void)param; return test_imu(NULL); }
static diag_result_t fugazi_test_power(int param)     { (void)param; return test_power(NULL); }

/*===========================================================================*/
/* Fugazi-style submenu definition                                           */
/*===========================================================================*/

/*
 * Main fugazi-style menu — all tests in one interactive menu.
 *
 * In the future, sub-groups can be split into separate submenu_xtable_t
 * arrays (I2C tests, peripheral tests, etc.) and nested via MF_SUBMENU.
 */

#define F_I2C    (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT)
#define F_PERIPH (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT)

static const diag_menu_xtable_t s_main_menu[] = {
    { "I2C Bus Scan",             fugazi_test_i2c_scan, 0, F_I2C,   NULL, 0 },
    { "Display (ILI9342C)",       fugazi_test_screen,   0, F_PERIPH, NULL, 0 },
    { "Touch (FT6336)",           fugazi_test_touch,    0, F_PERIPH, NULL, 0 },
    { "RTC (BM8563)",             fugazi_test_rtc,      0, F_PERIPH, NULL, 0 },
    { "IMU (BMI270)",             fugazi_test_imu,      0, F_PERIPH, NULL, 0 },
    { "Power (AXP2101)",          fugazi_test_power,    0, F_PERIPH, NULL, 0 },
};
#define MAIN_MENU_COUNT (sizeof(s_main_menu) / sizeof(s_main_menu[0]))

/* Fugazi menu runtime instance */
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

    /* Initialise error context */
    diag_err_init(&s_err_ctx);

    /* Build the fugazi-style interactive menu */
    if (diag_menu_build(&s_fugazi_menu, s_main_menu, MAIN_MENU_COUNT,
                        "CoreS3 Diagnostics") == DIAG_PASSED) {
        /* Register with the CLI so 'menu' and 'errors' commands work */
        diag_menu_set_fugazi(&s_fugazi_menu, &s_err_ctx);
    }

    /* Register extended CLI commands */
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
