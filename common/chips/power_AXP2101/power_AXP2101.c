/*
 * power_AXP2101.c — AXP2101 Power Management Unit (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 *
 * Register map verified against the AXP2101 datasheet bindings used by
 * M5Stack's m5gfx driver and the .NET nanoFramework Iot.Device.Axp2101.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "power_AXP2101.h"
#include <string.h>

/* ADC enable: bits 0-5 = battery voltage, TS, VBUS, VSYS, die temp, ... */
#define AXP2101_ADC_ENABLE_ALL   0x3F

/*===========================================================================*/
/* Module state — set once at init, never touched by other modules           */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;

/*===========================================================================*/
/* I2C helpers (use abstract transport)                                      */
/*===========================================================================*/

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, AXP2101_ADDR, &reg, 1, val, 1);
}

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return s_i2c->write(s_bus, AXP2101_ADDR, buf, 2);
}

/* 14-bit ADC value: high reg bits [5:0] + low reg bits [7:0] (H6L8) */
static uint16_t read_adc_14(uint8_t reg_h)
{
    uint8_t buf[2] = { 0, 0 };
    if (s_i2c->write_then_read(s_bus, AXP2101_ADDR, &reg_h, 1, buf, 2) != 0) {
        return 0;
    }
    return (uint16_t)(((buf[0] & 0x3F) << 8) | buf[1]);
}

/* 13-bit ADC value: high reg bits [4:0] + low reg bits [7:0] (H5L8) */
static uint16_t read_adc_13(uint8_t reg_h)
{
    uint8_t buf[2] = { 0, 0 };
    if (s_i2c->write_then_read(s_bus, AXP2101_ADDR, &reg_h, 1, buf, 2) != 0) {
        return 0;
    }
    return (uint16_t)(((buf[0] & 0x1F) << 8) | buf[1]);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int power_AXP2101_init(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t ver = 0;
    if (read_reg(AXP2101_REG_CHIP_VER, &ver) != 0) {
        return -1;
    }
    /* DFS: chip ID register (0x03) must return non-zero value */
    if (ver == 0) {
        return -1;
    }

    /* Enable ADC channels so battery/VBUS/temperature reads are valid */
    if (write_reg(AXP2101_REG_ADC_CTRL, AXP2101_ADC_ENABLE_ALL) != 0) {
        return -1;
    }

    return 0;
}

void power_AXP2101_deinit(void)
{
    s_i2c = NULL;
    s_bus = NULL;
}

int power_AXP2101_read(power_AXP2101_data_t *data)
{
    if (!data || !s_i2c || !s_bus) return -1;
    memset(data, 0, sizeof(*data));

    uint8_t status1 = 0, status2 = 0;
    if (read_reg(AXP2101_REG_PWR_STATUS1, &status1) != 0) return -1;
    if (read_reg(AXP2101_REG_PWR_STATUS2, &status2) != 0) return -1;

    data->flags = AXP2101_FLAG_NONE;
    if (status1 & AXP2101_PWR_VBUS)      data->flags |= AXP2101_FLAG_USB;
    if (status1 & AXP2101_PWR_BAT_EXIST) {
        uint8_t chg = (status2 >> AXP2101_STAT2_CHG_STATE_SHIFT)
                      & AXP2101_STAT2_CHG_STATE_MASK;
        if (chg == AXP2101_STAT2_CHG_CHARGING)
            data->flags |= AXP2101_FLAG_BAT_CHARGING;
        else if (chg == AXP2101_STAT2_CHG_DISCHARGING)
            data->flags |= AXP2101_FLAG_BAT_DISCHARGING;
    }

    /* Battery voltage: 13-bit ADC, LSB = 1 mV (raw value is in mV) */
    uint16_t bat_raw = read_adc_13(AXP2101_REG_BAT_ADC_H);
    data->battery_millivolts = bat_raw;

    /* VBUS voltage: 14-bit ADC, LSB = 1 mV; 0 when VBUS not good */
    if (data->flags & AXP2101_FLAG_USB) {
        uint16_t vbus_raw = read_adc_14(AXP2101_REG_VBUS_ADC_H);
        data->usb_millivolts = (vbus_raw >= 16375) ? 0 : vbus_raw;
    }

    /* System rail voltage: 14-bit ADC, LSB = 1 mV */
    data->system_millivolts = read_adc_14(AXP2101_REG_VSYS_ADC_H);

    /* Die temperature: T = 22 + (7274 - raw) / 20  (°C) */
    uint16_t temp_raw = read_adc_14(AXP2101_REG_TEMP_H);
    data->temperature_celsius = (uint8_t)(22 + ((7274 - (int)temp_raw) / 20));

    /* Voltage-based battery percentage estimate */
    uint16_t mv = data->battery_millivolts;
    if (mv >= 4200)      data->battery_percent = 100;
    else if (mv >= 3700) data->battery_percent = (uint8_t)((mv - 3700) * 100 / 500);
    else if (mv >= 3400) data->battery_percent = (uint8_t)((mv - 3400) / 30);
    else                 data->battery_percent = 0;

    if (!(status1 & AXP2101_PWR_BAT_EXIST)) data->battery_percent = 0;

    return 0;
}

uint8_t power_AXP2101_battery_percent(void)
{
    power_AXP2101_data_t d;
    if (power_AXP2101_read(&d) != 0) return 0;
    return d.battery_percent;
}

int power_AXP2101_read_reg(uint8_t reg, uint8_t *val)
{
    if (!val || !s_i2c || !s_bus) return -1;
    return read_reg(reg, val);
}

bool power_AXP2101_is_vbus_present(void)
{
    uint8_t status1 = 0;
    if (read_reg(AXP2101_REG_PWR_STATUS1, &status1) != 0) return false;
    return (status1 & AXP2101_PWR_VBUS) != 0;
}

void power_AXP2101_shutdown(void)
{
    /* Set bit 0 of register 0x10 to trigger power-off */
    write_reg(0x10, 0x01);
    while (1) {}
}

uint8_t power_AXP2101_chip_version(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t ver = 0;
    read_reg(AXP2101_REG_CHIP_VER, &ver);
    return ver;
}
