/*
 * test_power_AXP2101.c — Unit tests for AXP2101 PMU chip driver
 *
 * Tests through the abstract diag_i2c_t transport seam with a mock.
 * Compiles and runs on the host (Linux/macOS) with gcc.
 *
 * Register map verified against the AXP2101 datasheet bindings used by
 * M5Stack's m5gfx driver and the .NET nanoFramework Iot.Device.Axp2101:
 *   - Status1 (0x00): bit5 = VBUS good, bit3 = battery present
 *   - Status2 (0x01): bits[7:5] = charge state (001 charging, 010 discharging)
 *   - IcType (0x03): real AXP2101 reads 0x4A
 *   - Battery voltage 0x34: 13-bit H5L8, LSB = 1 mV
 *   - VBUS voltage 0x38:    14-bit H6L8, LSB = 1 mV
 *   - VSYS voltage 0x3A:    14-bit H6L8, LSB = 1 mV
 *   - Die temp 0x3C:        14-bit H6L8, T = 22 + (7274 - raw) / 20
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

/* 14-bit H6L8: high reg bits [5:0], low reg bits [7:0] (LSB = 1 mV) */
static void set_adc_14(uint8_t *regs, uint8_t reg_h, uint16_t raw)
{
    regs[reg_h]     = (uint8_t)((raw >> 8) & 0x3F);
    regs[reg_h + 1] = (uint8_t)(raw & 0xFF);
}

/* 13-bit H5L8: high reg bits [4:0], low reg bits [7:0] (LSB = 1 mV) */
static void set_adc_13(uint8_t *regs, uint8_t reg_h, uint16_t raw)
{
    regs[reg_h]     = (uint8_t)((raw >> 8) & 0x1F);
    regs[reg_h + 1] = (uint8_t)(raw & 0xFF);
}

static void set_defaults(uint8_t *regs)
{
    memset(regs, 0, 256);
    regs[AXP2101_REG_CHIP_VER] = 0x4A;        /* real AXP2101 chip ID */
    regs[AXP2101_REG_PWR_STATUS1] = AXP2101_PWR_BAT_EXIST;
}

/*===================================================================*/
/* init reads chip ID register and enables the ADC channels          */
/*===================================================================*/
static void test_init_reads_chip_id(void)
{
    TEST("init reads chip ID");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    assert(power_AXP2101_chip_version() == 0x4A);
    /* ADC channels must be enabled for reads to be valid */
    assert(mock_regs[AXP2101_REG_ADC_CTRL] == 0x3F);
    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* init fails when chip ID is 0 (no chip)                             */
/*===================================================================*/
static void test_init_fails_when_chip_missing(void)
{
    TEST("init fails when chip missing");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    memset(regs, 0, 256);   /* chip ID stays 0 */

    assert(power_AXP2101_init(&mock_transport, bus) == -1);
    PASS();
}

/*===================================================================*/
/* Battery percentage lookup table tests                              */
/*                                                                   */
/*   13-bit ADC, LSB = 1 mV → raw value is in mV directly            */
/*   ≥4200 → 100%                                                   */
/*   3700-4200 → linear: (mv-3700)*100/500                          */
/*   3400-3700 → linear: (mv-3400)*10/300                           */
/*   <3400 → 0%                                                     */
/*   BAT_EXIST=0 → 0% (overrides all above)                         */
/*===================================================================*/
static void test_battery_pct_lookup(void)
{
    TEST("battery % lookup table");

    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    /* 4200 mV → 100% (≥4200) */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 4200);
    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 4200);
    assert(d.battery_percent == 100);

    /* 3700 mV → 0% (boundary) */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 3700);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3700);
    assert(d.battery_percent == 0);

    /* 3950 mV → 50% (midpoint) */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 3950);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3950);
    assert(d.battery_percent == 50);

    /* 3400 mV → 0% (3400 boundary) */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 3400);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3400);
    assert(d.battery_percent == 0);

    /* 3550 mV → 5% */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 3550);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3550);
    assert(d.battery_percent == 5);

    /* <3400: 3300 mV → 0% */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 3300);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_millivolts == 3300);
    assert(d.battery_percent == 0);

    /* Battery absent: BAT_EXIST=0, voltage=4200 but % forced to 0 */
    set_adc_13(regs, AXP2101_REG_BAT_ADC_H, 4200);
    regs[AXP2101_REG_PWR_STATUS1] = 0;   /* no battery */
    assert(power_AXP2101_read(&d) == 0);
    assert(d.battery_percent == 0);
    regs[AXP2101_REG_PWR_STATUS1] = AXP2101_PWR_BAT_EXIST;  /* restore */

    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* VBUS voltage: 14-bit H6L8, LSB = 1 mV; requires VBUS-good bit     */
/*===================================================================*/
static void test_vbus_voltage(void)
{
    TEST("VBUS voltage");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[AXP2101_REG_PWR_STATUS1] = AXP2101_PWR_BAT_EXIST | AXP2101_PWR_VBUS;
    set_adc_14(regs, AXP2101_REG_VBUS_ADC_H, 5100);   /* 5.1 V */

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.usb_millivolts == 5100);
    assert(d.flags & AXP2101_FLAG_USB);

    /* No VBUS-good bit → voltage forced to 0 */
    regs[AXP2101_REG_PWR_STATUS1] = AXP2101_PWR_BAT_EXIST;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.usb_millivolts == 0);
    assert(!(d.flags & AXP2101_FLAG_USB));

    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* Power status flags: USB + battery charge state (Status2)          */
/*===================================================================*/
static void test_power_status_flags(void)
{
    TEST("power status flags: USB + charge state");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[AXP2101_REG_PWR_STATUS1] = AXP2101_PWR_BAT_EXIST | AXP2101_PWR_VBUS;

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;

    /* Status2 bits[7:5] = 001 → charging */
    regs[AXP2101_REG_PWR_STATUS2] = AXP2101_STAT2_CHG_CHARGING
                                    << AXP2101_STAT2_CHG_STATE_SHIFT;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.flags & AXP2101_FLAG_USB);
    assert(d.flags & AXP2101_FLAG_BAT_CHARGING);
    assert(!(d.flags & AXP2101_FLAG_BAT_DISCHARGING));

    /* Status2 bits[7:5] = 010 → discharging */
    regs[AXP2101_REG_PWR_STATUS2] = AXP2101_STAT2_CHG_DISCHARGING
                                    << AXP2101_STAT2_CHG_STATE_SHIFT;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.flags & AXP2101_FLAG_BAT_DISCHARGING);
    assert(!(d.flags & AXP2101_FLAG_BAT_CHARGING));

    /* No battery present → no charge state flags at all */
    regs[AXP2101_REG_PWR_STATUS1] = AXP2101_PWR_VBUS;
    regs[AXP2101_REG_PWR_STATUS2] = AXP2101_STAT2_CHG_CHARGING
                                    << AXP2101_STAT2_CHG_STATE_SHIFT;
    assert(power_AXP2101_read(&d) == 0);
    assert(!(d.flags & AXP2101_FLAG_BAT_CHARGING));
    assert(!(d.flags & AXP2101_FLAG_BAT_DISCHARGING));

    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* Die temperature: T = 22 + (7274 - raw) / 20                       */
/*===================================================================*/
static void test_temperature(void)
{
    TEST("die temperature formula");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    /* raw = 7274 → 22 °C */
    set_adc_14(regs, AXP2101_REG_TEMP_H, 7274);
    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.temperature_celsius == 22);

    /* raw = 6874 → 42 °C */
    set_adc_14(regs, AXP2101_REG_TEMP_H, 6874);
    assert(power_AXP2101_read(&d) == 0);
    assert(d.temperature_celsius == 42);

    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* Single-register read (bring-up debug helper)                      */
/*===================================================================*/
static void test_read_reg(void)
{
    TEST("single-register read");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[0x90] = 0xAF;
    regs[0x99] = 0x1C;

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    uint8_t v = 0;
    assert(power_AXP2101_read_reg(0x90, &v) == 0);
    assert(v == 0xAF);
    assert(power_AXP2101_read_reg(0x99, &v) == 0);
    assert(v == 0x1C);
    power_AXP2101_deinit();
    PASS();
}

/*===================================================================*/
/* VSYS voltage: 14-bit H6L8, LSB = 1 mV                             */
/*===================================================================*/
static void test_system_voltage(void)
{
    TEST("system (VSYS) voltage");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    set_adc_14(regs, AXP2101_REG_VSYS_ADC_H, 3300);

    assert(power_AXP2101_init(&mock_transport, bus) == 0);
    power_AXP2101_data_t d;
    assert(power_AXP2101_read(&d) == 0);
    assert(d.system_millivolts == 3300);
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

    test_init_reads_chip_id();
    test_init_fails_when_chip_missing();
    test_battery_pct_lookup();
    test_vbus_voltage();
    test_power_status_flags();
    test_temperature();
    test_system_voltage();
    test_read_reg();

    printf("\n======================\n");
    printf("Result: %d/%d passed\n", s_passed, s_run);
    return (s_passed == s_run) ? 0 : 1;
}
