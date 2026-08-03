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
#include "power_AXP2101.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "aw9523b.h"
#include "diag_config.h"
#include "driver/i2c_master.h"
#include "soc/io_mux_reg.h"
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

static void dump_aw9523b(void);   /* defined below (shared with lcdbb) */

static diag_result_t cmd_pmu(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;

    if (hal_power_init() != DIAG_PASSED) {
        diag_menu_printf("AXP2101 init failed\r\n");
        return DIAG_FAILED;
    }

    diag_menu_printf("\r\n========== AXP2101 Register Dump ==========\r\n");

    static const struct { const char *name; uint8_t reg; } regs[] = {
        { "Status1(0x00)", 0x00 }, { "Status2(0x01)", 0x01 },
        { "ChipID (0x03)", 0x03 }, { "Power  (0x10)", 0x10 },
        { "Batfet(0x12)", 0x12 },
        { "WdtChg (0x18)", 0x18 }, { "Wdt    (0x19)", 0x19 },
        { "ADC    (0x30)", 0x30 }, { "LDOEn  (0x90)", 0x90 },
        { "DLDO1  (0x99)", 0x99 },
    };
    for (size_t i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
        uint8_t v = 0;
        if (power_AXP2101_read_reg(regs[i].reg, &v) == 0) {
            diag_menu_printf("  %s = 0x%02X\r\n", regs[i].name, v);
        } else {
            diag_menu_printf("  %s = <I2C error>\r\n", regs[i].name);
        }
    }

    /* ADC data registers (13/14-bit big-endian, LSB = 1 mV) */
    static const struct { const char *name; uint8_t reg; uint8_t bits; } adc[] = {
        { "Battery ADC (0x34)", 0x34, 13 },
        { "VBUS ADC   (0x38)", 0x38, 14 },
        { "VSYS ADC   (0x3A)", 0x3A, 14 },
        { "Temp ADC   (0x3C)", 0x3C, 14 },
    };
    for (size_t i = 0; i < sizeof(adc) / sizeof(adc[0]); i++) {
        uint8_t h = 0, l = 0;
        uint16_t raw = 0;
        if (power_AXP2101_read_reg(adc[i].reg, &h) == 0 &&
            power_AXP2101_read_reg(adc[i].reg + 1, &l) == 0) {
            uint8_t mask = (adc[i].bits == 13) ? 0x1F : 0x3F;
            raw = (uint16_t)(((h & mask) << 8) | l);
        }
        if (adc[i].bits == 13) {
            diag_menu_printf("  %s = %u (0x%04X) mV\r\n", adc[i].name,
                             raw, raw);
        } else {
            int deg = (adc[i].reg == 0x3C)
                ? 22 + ((7274 - (int)raw) / 20) : -1;
            if (deg >= 0) {
                diag_menu_printf("  %s = %u (0x%04X) -> %d C\r\n",
                                 adc[i].name, raw, raw, deg);
            } else {
                diag_menu_printf("  %s = %u (0x%04X) mV\r\n",
                                 adc[i].name, raw, raw);
            }
        }
    }

    /* Write test: 0x06 is a user data buffer register (plain RAM, no
     * hardware effect). If it does not stick, the chip's write path is
     * dead — reads work but every write is silently dropped. */
    {
        i2c_master_dev_handle_t pmu = NULL;
        if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &pmu)
            == DIAG_PASSED) {
            uint8_t before = 0, after = 0;
            power_AXP2101_read_reg(0x06, &before);
            uint8_t wr[2] = { 0x06, 0x5A };
            i2c_master_transmit(pmu, wr, 2, -1);
            power_AXP2101_read_reg(0x06, &after);
            diag_menu_printf("  Write test (reg 0x06): wrote 0x5A, "
                             "read-back 0x%02X (was 0x%02X) %s\r\n",
                             after, before,
                             (after == 0x5A) ? "-> WRITE OK"
                                             : "-> WRITE IGNORED");
            wr[1] = 0x00;   /* restore */
            i2c_master_transmit(pmu, wr, 2, -1);
            i2c_master_bus_rm_device(pmu);
        }
    }

    diag_menu_printf("===========================================\r\n");
    hal_power_deinit();

    /* AW9523B state (P1_7 BOOST_EN, P1_1 LCD RST) — the backlight/panel
     * control lines. Check after a screen blanking to see if they drop. */
    diag_menu_printf("\r\n========== AW9523B Register Dump ==========\r\n");
    dump_aw9523b();
    diag_menu_printf("===========================================\r\n");
    return DIAG_PASSED;
}

/*---------------------------------------------------------------------------*/
/* lcdbb — bit-banged SPI test for the ILI9342C (bypasses the ESP-IDF SPI   */
/* master driver entirely). Decisive split: if this lights the panel, the   */
/* SPI driver transport is the problem; if not, the panel/RST side is.      */
/*---------------------------------------------------------------------------*/

static void bb_delay(void)
{
    esp_rom_delay_us(2);   /* ~250 kHz */
}

/* SPI mode 0: sample on rising edge, SCK idles low */
static void bb_byte(uint8_t b)
{
    for (int i = 7; i >= 0; i--) {
        gpio_set_level(CONFIG_LCD_MOSI_PIN, (b >> i) & 1);
        gpio_set_level(CONFIG_LCD_SCLK_PIN, 1);
        bb_delay();
        gpio_set_level(CONFIG_LCD_SCLK_PIN, 0);
        bb_delay();
    }
}

/* Bytes with CS held LOW the whole time (ILI9342C requires CS to stay
 * asserted across a command+data sequence — m5gfx holds CS for the
 * entire init list and per draw). */
static void bb_cs_low(uint8_t dc, const uint8_t *data, size_t len)
{
    gpio_set_level(CONFIG_LCD_DC_PIN, dc);
    for (size_t i = 0; i < len; i++) bb_byte(data[i]);
}

static void bb_cmd(uint8_t c)                   { bb_cs_low(0, &c, 1); }
static void bb_cmd_data(uint8_t c, const uint8_t *d, size_t n)
{
    bb_cmd(c);
    bb_cs_low(1, d, n);
}

/* Dump AW9523B state — verifies whether our pin writes actually stick.
 * P1_1 (LCD RST) = bit1 of reg 0x03 (output), 0x05 (direction),
 * 0x13 (LED mode). Chip ID at 0x10 (expect 0x23). */
static void dump_aw9523b(void)
{
    i2c_master_dev_handle_t aw = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &aw)
        != DIAG_PASSED) {
        diag_menu_printf("  AW9523B: <init failed>\r\n");
        return;
    }
    static const uint8_t regs[] = { 0x10, 0x02, 0x03, 0x04, 0x05, 0x12, 0x13 };
    static const char *names[] = {
        "ID(0x10)", "P0Out(0x02)", "P1Out(0x03)", "P0Dir(0x04)",
        "P1Dir(0x05)", "P0LED(0x12)", "P1LED(0x13)",
    };
    for (size_t i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
        uint8_t reg = regs[i], val = 0;
        if (i2c_master_transmit_receive(aw, &reg, 1, &val, 1, -1) == ESP_OK) {
            diag_menu_printf("  AW9523B %s = 0x%02X\r\n", names[i], val);
        } else {
            diag_menu_printf("  AW9523B %s = <I2C error>\r\n", names[i]);
        }
    }
    i2c_master_bus_rm_device(aw);
}

static diag_result_t cmd_lcdbb(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner;

    bool skip_rst = (argc >= 2 && strcmp(argv[1], "nors") == 0);
    bool m5_aw    = (argc >= 2 && strcmp(argv[1], "awm5") == 0);

    diag_menu_printf("\r\nBit-bang LCD test: GPIO-drive SPI2 (no SPI driver)%s%s\r\n",
                     skip_rst ? " [nors]" : "",
                     m5_aw ? " [awm5]" : "");

    /* Enable ALL LDO rails (0xBF): DLDO1 backlight + BLDO1 LCD VDD —
     * without BLDO1 the panel is unpowered and shows nothing. */
    i2c_master_dev_handle_t pmu = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_POWER, 400000, &pmu)
        == DIAG_PASSED) {
        uint8_t cmd[2] = { AXP2101_REG_LDO_EN, 0xBF };
        i2c_master_transmit(pmu, cmd, 2, -1);

        /* LDO voltage registers (M5Unified writes ALDO1-4 = 0x92-0x95) */
        if (m5_aw) {
            static const uint8_t vols[][2] = {
                { 0x92, 13 }, { 0x93, 28 }, { 0x94, 28 }, { 0x95, 28 },
            };
            for (size_t i = 0; i < sizeof(vols) / sizeof(vols[0]); i++) {
                cmd[0] = vols[i][0];
                cmd[1] = vols[i][1];
                i2c_master_transmit(pmu, cmd, 2, -1);
            }
            diag_menu_printf("  ALDO1-4 voltages set (M5Unified values)\r\n");
        }
        /* Report current LDO voltage settings */
        for (uint8_t r = 0x92; r <= 0x99; r++) {
            uint8_t v = 0;
            if (i2c_master_transmit_receive(pmu, &r, 1, &v, 1, -1) == ESP_OK) {
                diag_menu_printf("  AXP2101 0x%02X (LDO volt) = 0x%02X\r\n",
                                 r, v);
            }
        }
    }

    /* Hypothesis H1: the ILI9342C needs time for its internal power to
     * stabilise after BLDO1 (VDD) comes up. The only successful draw so
     * far happened when the panel had been powered for minutes (UserDemo
     * state); every cold attempt initialised it within ~100 ms of power.
     * Give it 500 ms before the reset pulse. */
    diag_menu_printf("  Waiting 500 ms for panel power to stabilise...\r\n");
    esp_rom_delay_us(500000);

    /* Take over the pins from the SPI driver, if the bus is up.
     * CRITICAL: the ESP-IDF SPI driver pins G36 (FSPICLK) and G37
     * (FSPIQ) via the IOMUX — gpio_set_direction() cannot override
     * that, so force the IOMUX function back to GPIO first. */
    PIN_FUNC_SELECT(IO_MUX_GPIO36_REG, PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(IO_MUX_GPIO37_REG, PIN_FUNC_GPIO);
    gpio_set_direction(CONFIG_LCD_CS_PIN,   GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_LCD_DC_PIN,   GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_LCD_SCLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_LCD_MOSI_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
    gpio_set_level(CONFIG_LCD_DC_PIN, 1);
    gpio_set_level(CONFIG_LCD_SCLK_PIN, 1);

    /* AW9523B configuration. Variant awm5 replicates M5GFX's exact
     * CoreS3 config (GCR push-pull, LED modes, CONFIG) before touching
     * the pins. */
    i2c_master_dev_handle_t aw_dev = NULL;
    if (hal_i2c_add_device(CONFIG_I2C_ADDR_GPIO_EXP, 400000, &aw_dev)
        == DIAG_PASSED) {
        if (m5_aw) {
            uint8_t reg, val;
            static const uint8_t cfg[][2] = {
                { 0x04, 0x18 }, { 0x05, 0x0C },   /* CONFIG_P0/P1 */
                { 0x11, 0x10 },                   /* GCR: P0 push-pull */
                { 0x12, 0xFF }, { 0x13, 0xFF },   /* LEDMODE_P0/P1 */
            };
            for (size_t i = 0; i < sizeof(cfg) / sizeof(cfg[0]); i++) {
                reg = cfg[i][0];
                val = cfg[i][1];
                i2c_master_transmit(aw_dev, (uint8_t[]){ reg, val }, 2, -1);
            }
            diag_menu_printf("  AW9523B configured like M5GFX (GCR/LED/CONFIG)\r\n");
        }
        aw9523b_init(&g_diag_i2c_adapter, (void *)aw_dev);
        if (!skip_rst) {
            aw9523b_pin_set_gpio_mode(AW9523B_PIN_LCD_RST);
            aw9523b_pin_set_direction(AW9523B_PIN_LCD_RST, 1);
            aw9523b_pin_write(AW9523B_PIN_LCD_RST, 0);
            esp_rom_delay_us(20000);
            aw9523b_pin_write(AW9523B_PIN_LCD_RST, 1);
            esp_rom_delay_us(64000);   /* M5Unified: 64 ms after RST release */
        } else {
            aw9523b_pin_set_gpio_mode(AW9523B_PIN_LCD_RST);
            aw9523b_pin_set_direction(AW9523B_PIN_LCD_RST, 1);
            aw9523b_pin_write(AW9523B_PIN_LCD_RST, 1);
            diag_menu_printf("  RST pulse SKIPPED (P1_1 = high)\r\n");
        }
        /* SY7088 BOOST_EN (P1_7): without it the backlight LED has no
         * current — the panel may draw but the screen stays dark. */
        aw9523b_pin_set_gpio_mode(AW9523B_PIN_BOOST_EN);
        aw9523b_pin_set_direction(AW9523B_PIN_BOOST_EN, 1);
        aw9523b_pin_write(AW9523B_PIN_BOOST_EN, 1);
        diag_menu_printf("  LCD RST %s + BOOST_EN on (P1_1/P1_7)\r\n",
                         skip_rst ? "held high" : "released");
    } else {
        diag_menu_printf("  WARNING: AW9523B init failed — RST may stay low\r\n");
    }
    dump_aw9523b();

    /* RDID (0x04) read-back: definitive panel-liveness check.
     * G35 doubles as MISO — switch it to input for the read phase. */
    {
        uint8_t id[3] = { 0, 0, 0 };
        gpio_set_direction(CONFIG_LCD_DC_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(CONFIG_LCD_CS_PIN, 0);
        gpio_set_level(CONFIG_LCD_DC_PIN, 0);        /* command mode */
        uint8_t rdid = 0x04;
        for (int i = 7; i >= 0; i--) {
            gpio_set_level(CONFIG_LCD_MOSI_PIN, (rdid >> i) & 1);
            gpio_set_level(CONFIG_LCD_SCLK_PIN, 1);
            bb_delay();
            gpio_set_level(CONFIG_LCD_SCLK_PIN, 0);
            bb_delay();
        }
        gpio_set_direction(CONFIG_LCD_DC_PIN, GPIO_MODE_INPUT);  /* MISO */
        /* 1 dummy clock before the panel drives data (m5gfx beginRead) */
        gpio_set_level(CONFIG_LCD_SCLK_PIN, 1);
        bb_delay();
        gpio_set_level(CONFIG_LCD_SCLK_PIN, 0);
        bb_delay();
        for (int b = 0; b < 3; b++) {
            uint8_t v = 0;
            for (int i = 7; i >= 0; i--) {
                gpio_set_level(CONFIG_LCD_SCLK_PIN, 1);
                bb_delay();
                v = (uint8_t)((v << 1) | gpio_get_level(CONFIG_LCD_DC_PIN));
                gpio_set_level(CONFIG_LCD_SCLK_PIN, 0);
                bb_delay();
            }
            id[b] = v;
        }
        gpio_set_direction(CONFIG_LCD_DC_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(CONFIG_LCD_CS_PIN, 1);
        diag_menu_printf("  Panel RDID: 0x%02X 0x%02X 0x%02X %s\r\n",
                         id[0], id[1], id[2],
                         (id[1] == 0xE3) ? "(ILI9342C!)" : "(no response)");
    }

    /* m5gfx Panel_ILI9342 list0 EXACTLY as M5Unified sends it (no
     * SWRESET — hardware RST covers it; DISPON BEFORE SLPOUT as in
     * the m5gfx list; COLMOD/MADCTL/INVON afterwards as M5Unified's
     * setColorDepth/setRotation do). CS held low for the whole list. */
    gpio_set_level(CONFIG_LCD_CS_PIN, 0);
    bb_cmd_data(0xC8, (uint8_t[]){0xFF, 0x93, 0x42}, 3);        /* SETEXTC */
    bb_cmd_data(0xC0, (uint8_t[]){0x12, 0x12}, 2);
    bb_cmd_data(0xC1, (uint8_t[]){0x03}, 1);
    bb_cmd_data(0xC5, (uint8_t[]){0xF2}, 1);
    bb_cmd_data(0xB0, (uint8_t[]){0xE0}, 1);
    bb_cmd_data(0xF6, (uint8_t[]){0x01, 0x00, 0x00}, 3);
    bb_cmd_data(0xE0, (uint8_t[]){0x00,0x0C,0x11,0x04,0x11,0x08,0x37,0x89,
                                  0x4C,0x06,0x0C,0x0A,0x2E,0x34,0x0F}, 15);
    bb_cmd_data(0xE1, (uint8_t[]){0x00,0x0B,0x11,0x05,0x13,0x09,0x33,0x67,
                                  0x48,0x07,0x0E,0x0B,0x2E,0x33,0x0F}, 15);
    bb_cmd_data(0xB6, (uint8_t[]){0x08, 0x82, 0x1D, 0x04}, 4);
    bb_cmd(0x38);                                               /* IDMOFF */
    bb_cmd(0x29);                                               /* DISPON */
    bb_cmd(0x11);                                               /* SLPOUT */
    esp_rom_delay_us(120000);
    bb_cmd_data(0x3A, (uint8_t[]){0x55}, 1);                    /* COLMOD */
    bb_cmd_data(0x36, (uint8_t[]){0x48}, 1);                    /* MADCTL MX|BGR */
    bb_cmd(0x21);                                               /* INVON */
    esp_rom_delay_us(10000);
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);   /* init window done */

    /* Fill RED top bar (rows 0-59) — quick ~1 s test at 250 kHz.
     * CS held low across the whole fill (m5gfx behaviour). */
    diag_menu_printf("  Starting red bar fill (rows 0-59)...\r\n");
    gpio_set_level(CONFIG_LCD_CS_PIN, 0);
    bb_cmd_data(0x2A, (uint8_t[]){0x00,0x00,0x01,0x3F}, 4);   /* col 0-319 */
    bb_cmd_data(0x2B, (uint8_t[]){0x00,0x00,0x00,0x3B}, 4);   /* row 0-59  */
    bb_cmd(0x2C);
    {
        uint8_t red[128];
        memset(red, 0xF8, 128);   /* RGB565 red */
        for (int y = 0; y < 60; y++)
            for (int x = 0; x < 20; x++)
                bb_cs_low(1, red, sizeof(red));
    }
    gpio_set_level(CONFIG_LCD_CS_PIN, 1);
    diag_menu_printf("  Done.\r\n");

    diag_menu_printf("Bit-bang done: top 60 rows should now be RED\r\n");
    return DIAG_PASSED;
}

/* scr2 — display sequence with explicit checkpoints: RED, GREEN, BLUE,
 * then white text + line. 2 s pauses so each stage is unmistakable. */
static diag_result_t cmd_scr2(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;

    diag_menu_printf("\r\nscr2: display sequence test\r\n");
    if (hal_screen_init() != DIAG_PASSED) {
        diag_menu_printf("  hal_screen_init FAILED\r\n");
        return DIAG_FAILED;
    }
    diag_menu_printf("  init done\r\n");

    diag_menu_printf("  RED fill start\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_RED);
    diag_menu_printf("  RED done\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    diag_menu_printf("  GREEN fill start\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_GREEN);
    diag_menu_printf("  GREEN done\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    diag_menu_printf("  BLUE fill start\r\n");
    hal_screen_fill(HAL_SCREEN_COLOR_BLUE);
    diag_menu_printf("  BLUE done\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    diag_menu_printf("  text + line\r\n");
    hal_screen_set_font(2);
    hal_screen_draw_text(60, 100, "LCD OK", HAL_SCREEN_COLOR_WHITE,
                         HAL_SCREEN_COLOR_BLACK);
    hal_screen_draw_line(0, 120, 319, 120, HAL_SCREEN_COLOR_WHITE);
    diag_menu_printf("  scr2 done — expect RED, GREEN, BLUE, then "
                     "white 'LCD OK' + line\r\n");
    return DIAG_PASSED;
}

/* scr3 — three 60-row strips: RED (0-59), GREEN (60-119), BLUE (120-179).
 * Decisive: does the panel accept repeated short fills, or does each
 * RAMWR window cap at ~60 rows? */
static diag_result_t cmd_scr3(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;

    diag_menu_printf("\r\nscr3: three 60-row strips\r\n");
    if (hal_screen_init() != DIAG_PASSED) {
        diag_menu_printf("  hal_screen_init FAILED\r\n");
        return DIAG_FAILED;
    }
    diag_menu_printf("  init done\r\n");

    diag_menu_printf("  strip RED 0-59\r\n");
    hal_screen_fill_rect(0, 0, 320, 60, HAL_SCREEN_COLOR_RED);
    diag_menu_printf("  strip RED done\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    diag_menu_printf("  strip GREEN 60-119\r\n");
    hal_screen_fill_rect(0, 60, 320, 60, HAL_SCREEN_COLOR_GREEN);
    diag_menu_printf("  strip GREEN done\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    diag_menu_printf("  strip BLUE 120-179\r\n");
    hal_screen_fill_rect(0, 120, 320, 60, HAL_SCREEN_COLOR_BLUE);
    diag_menu_printf("  strip BLUE done\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    diag_menu_printf("  scr3 done — expect RED, GREEN, BLUE strips "
                     "top-to-bottom\r\n");
    return DIAG_PASSED;
}

/* scr4 — full re-init (RST pulse + init list) before EACH fill, like
 * the lcdbb debug command does. If the colours appear, the panel needs
 * a re-init per draw (it falls asleep/goes deaf ~0.6 s after the last
 * activity). Timestamps correlate the failure with any timeout. */
static diag_result_t cmd_scr4(diag_runner_t *runner, int argc, char *argv[])
{
    (void)runner; (void)argc; (void)argv;

    diag_menu_printf("\r\nscr4: re-init before each fill\r\n");

    hal_screen_init();
    diag_menu_printf("  t=%lld init done\r\n", (long long)esp_timer_get_time());
    hal_screen_fill(HAL_SCREEN_COLOR_RED);
    diag_menu_printf("  t=%lld RED done\r\n", (long long)esp_timer_get_time());
    vTaskDelay(pdMS_TO_TICKS(2000));

    hal_screen_deinit();
    hal_screen_init();
    diag_menu_printf("  t=%lld re-init + GREEN\r\n", (long long)esp_timer_get_time());
    hal_screen_fill(HAL_SCREEN_COLOR_GREEN);
    diag_menu_printf("  t=%lld GREEN done\r\n", (long long)esp_timer_get_time());
    vTaskDelay(pdMS_TO_TICKS(2000));

    hal_screen_deinit();
    hal_screen_init();
    diag_menu_printf("  t=%lld re-init + BLUE\r\n", (long long)esp_timer_get_time());
    hal_screen_fill(HAL_SCREEN_COLOR_BLUE);
    diag_menu_printf("  t=%lld BLUE done\r\n", (long long)esp_timer_get_time());
    vTaskDelay(pdMS_TO_TICKS(2000));

    hal_screen_deinit();
    hal_screen_init();
    diag_menu_printf("  t=%lld re-init + text\r\n", (long long)esp_timer_get_time());
    hal_screen_set_font(2);
    hal_screen_draw_text(60, 100, "LCD OK", HAL_SCREEN_COLOR_WHITE,
                         HAL_SCREEN_COLOR_BLACK);
    hal_screen_draw_line(0, 120, 319, 120, HAL_SCREEN_COLOR_WHITE);
    diag_menu_printf("  t=%lld scr4 done\r\n", (long long)esp_timer_get_time());
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
        { "pmu",        "Dump AXP2101 registers (bring-up debug)", cmd_pmu },
        { "lcdbb",      "Bit-bang LCD test (bypasses SPI driver)", cmd_lcdbb },
        { "scr2",       "Display sequence test with checkpoints", cmd_scr2 },
        { "scr3",       "Three 60-row strips test",             cmd_scr3 },
        { "scr4",       "Re-init before each fill test",        cmd_scr4 },
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
