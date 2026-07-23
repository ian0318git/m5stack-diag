/*
 * test_power_AXP2101.c — Unit tests for AXP2101 PMU chip driver
 *
 * Tests through the abstract diag_i2c_t transport seam with a mock.
 * Compiles and runs on the host (Linux/macOS) with gcc.
 *
 * Compile:
 *   gcc -I ../../common/chips/power_AXP2101 \
 *       -I ../../common/src/M5Stack_CoreS3/include \
 *       -o test_power_AXP2101 test_power_AXP2101.c \
 *       ../../common/chips/power_AXP2101/power_AXP2101.c \
 *       -lm
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "mock_i2c.h"
#include "power_AXP2101.h"

static int s_passed = 0;
static int s_run = 0;

#define TEST(name) do { printf("  %s ... ", name); s_run++; } while (0)
#define PASS()     do { printf("PASSED\n"); s_passed++; } while (0)

/* 12-bit ADC: value = (buf[0] << 4) | (buf[1] & 0x0F) */
/* Set 12-bit ADC value and return the millivolt value the driver will compute.
   AXP2101 stores bits [11:4] in reg_h and bits [3:0] in reg_h+1 lower nibble. */
static int set_adc_mv(uint8_t *regs, uint8_t reg_h, uint16_t raw, uint16_t lsb)
{
    regs[reg_h]     = (uint8_t)(raw >> 4);
    regs[reg_h + 1] = (uint8_t)(raw & 0x0F);
    return (int)((unsigned)raw * lsb / 1000);
}

static void set_adc_12(uint8_t *regs, uint8_t reg_h, uint16_t raw)
{
    regs[reg_h]     = (uint8_t)(raw >> 4);
    regs[reg_h + 1] = (uint8_t)(raw & 0x0F);
}

static void set_defaults(uint8_t *regs)
{
    memset(regs, 0, 256);
    regs[AXP2101_REG_CHIP_VER] = 0x15;
    regs[AXP2101_REG_PWR_STATUS] = AXP2101_PWR_BAT_EXIST;
}

/*===================================================================*/
/* init reads chip version register                                  */
/*===================================================================*/
static void test_init_reads_chip_version(void)
{
    TEST("init reads chip version");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    assert(power_AXP2101_chip_version() == 0x15);
    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* init fails when chip version is 0 (no chip)                       */
/*===================================================================*/
static void test_init_fails_when_chip_missing(void)
{
    TEST("init fails when chip missing");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    memset(regs, 0, 256);   /* version stays 0 */

    assert(power_AXP2101_init(&mock_transport, bus) == -1);
    PASS();
}

/*===================================================================*/
/* Battery percentage lookup table tests                              */
/*                                                                   */
/*   mv = raw * BAT_LSB_MV / 1000  (BAT_LSB_MV = 1100)             */
/*   ≥4200 → 100%                                                   */
/*   3700-4200 → linear: (mv-3700)*100/500                          */
/*   3400-3700 → linear: (mv-3400)*10/300                           */
/*   <3400 → 0%                                                     */
/*   BAT_EXIST=0 → 0% (overrides all above)                         */
/*===================================================================*/
static void test_battery_pct_lookup(void)
{
    TEST("battery % lookup table");

    /* raw = 3819 → 3819*1100/1000 = 4200 mV → 100% (≥4200) */
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    int mv = set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3819, 1100);
    assert(mv == 4200);

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 4200);
    assert(d.battery_percent == 100);

    /* raw = 3364 → 3364*1100/1000 = 3700 mV → 0% (boundary) */
    set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3364, 1100);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3700);
    assert(d.battery_percent == 0);

    /* raw = 3591 → 3591*1100/1000 = 3950 mV → 50% (midpoint) */
    set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3591, 1100);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3950);
    assert(d.battery_percent == 50);

    /* raw = 3091 → 3091*1100/1000 = 3400 mV → 0% (3400 boundary) */
    set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3091, 1100);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3400);
    assert(d.battery_percent == 0);

    /* raw = 3228 → 3228*1100/1000 = 3550 mV → 5% */
    set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3228, 1100);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3550);
    assert(d.battery_percent == 5);

    /* <3400: raw = 3000 → 3000*1100/1000 = 3300 mV → 0% */
    set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3000, 1100);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3300);
    assert(d.battery_percent == 0);

    /* Battery absent: BAT_EXIST=0, voltage=4200 but % forced to 0 */
    set_adc_mv(regs, AXP2101_REG_BAT_ADC_H, 3819, 1100);
    regs[AXP2101_REG_PWR_STATUS] = 0;   /* no battery */
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_percent == 0);
    regs[AXP2101_REG_PWR_STATUS] = AXP2101_PWR_BAT_EXIST;  /* restore */

    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* VBUS voltage (LSB = 1700)                                         */
/*===================================================================*/
static void test_vbus_voltage(void)
{
    TEST("VBUS voltage");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    int mv = set_adc_mv(regs, AXP2101_REG_VBUS_ADC_H, 3000, 1700);
    /* 3000 * 1700 / 1000 = 5100 */
    assert(mv == 5100);

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.usb_millivolts == 5100);
    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* Power status flags: USB connected + charging                      */
/*===================================================================*/
static void test_power_status_flags(void)
{
    TEST("power status flags: USB + charging");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    set_adc_12(regs, AXP2101_REG_BAT_ADC_H, 3818);
    regs[AXP2101_REG_PWR_STATUS] = AXP2101_PWR_BAT_EXIST |
                                    AXP2101_PWR_VBUS |
                                    AXP2101_PWR_BAT_CHG;

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.flags & AXP2101_FLAG_USB);
    assert(d.flags & AXP2101_FLAG_BAT_CHARGING);
    assert(!(d.flags & AXP2101_FLAG_BAT_FULL));
    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* Main                                                              */
/*===================================================================*/
int main(void)
{
    printf("\nAXP2101 PMU Unit Tests\n");
    printf("======================\n\n");

    test_init_reads_chip_version();
    test_init_fails_when_chip_missing();
    test_battery_pct_lookup();
    test_vbus_voltage();
    test_power_status_flags();

    printf("\n======================\n");
    printf("Result: %d/%d passed\n", s_passed, s_run);
    return (s_passed == s_run) ? 0 : 1;
}
