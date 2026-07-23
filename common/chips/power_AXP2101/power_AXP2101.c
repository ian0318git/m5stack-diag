/*
 * power_AXP2101.c — AXP2101 Power Management Unit (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "power_AXP2101.h"
#include <string.h>

/* Voltage reference: raw ADC × LSB ÷ 1000 → mV */
#define VBUS_LSB_MV   1700
#define BAT_LSB_MV    1100
#define TEMP_LSB_C    14     /* 0.14 °C per LSB ×100 */

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

/* Read 12-bit big-endian ADC value from two consecutive registers */
static uint16_t read_adc_12(uint8_t reg_h)
{
    uint8_t buf[2] = { 0, 0 };
    if (s_i2c->write_then_read(s_bus, AXP2101_ADDR, &reg_h, 1, buf, 2) != 0) {
        return 0;
    }
    return ((uint16_t)buf[0] << 4) | (buf[1] & 0x0F);
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

    uint8_t status = 0;
    if (read_reg(AXP2101_REG_PWR_STATUS, &status) != 0) return -1;

    data->flags = AXP2101_FLAG_NONE;
    if (status & AXP2101_PWR_VBUS)      data->flags |= AXP2101_FLAG_USB;
    if (status & AXP2101_PWR_BAT_CHG)   data->flags |= AXP2101_FLAG_BAT_CHARGING;
    if (status & AXP2101_PWR_BAT_FULL)  data->flags |= AXP2101_FLAG_BAT_FULL;

    uint16_t bat_adc = read_adc_12(AXP2101_REG_BAT_ADC_H);
    data->battery_millivolts = (uint16_t)((bat_adc * BAT_LSB_MV) / 1000);

    uint16_t vbus_adc = read_adc_12(AXP2101_REG_VBUS_ADC_H);
    data->usb_millivolts = (uint16_t)((vbus_adc * VBUS_LSB_MV) / 1000);

    uint16_t chg_adc = read_adc_12(AXP2101_REG_CHG_CUR_H);
    data->charge_current_ma = chg_adc;

    uint16_t temp_adc = read_adc_12(AXP2101_REG_TEMP_H);
    data->temperature_celsius = (uint8_t)((temp_adc * TEMP_LSB_C) / 100);

    /* Voltage-based battery percentage estimate */
    uint16_t mv = data->battery_millivolts;
    if (mv >= 4200)      data->battery_percent = 100;
    else if (mv >= 3700) data->battery_percent = (uint8_t)((mv - 3700) * 100 / 500);
    else if (mv >= 3400) data->battery_percent = (uint8_t)((mv - 3400) * 10 / 30);
    else                 data->battery_percent = 0;

    if (!(status & AXP2101_PWR_BAT_EXIST)) data->battery_percent = 0;

    data->system_millivolts = data->battery_millivolts;
    return 0;
}

uint8_t power_AXP2101_battery_percent(void)
{
    power_AXP2101_data_t d;
    if (power_AXP2101_read(&d) != 0) return 0;
    return d.battery_percent;
}

bool power_AXP2101_is_vbus_present(void)
{
    uint8_t status = 0;
    if (read_reg(AXP2101_REG_PWR_STATUS, &status) != 0) return false;
    return (status & AXP2101_PWR_VBUS) != 0;
}

void power_AXP2101_shutdown(void)
{
    /* Set bit 7 of register 0x10 to trigger power-off */
    write_reg(0x10, 0x80);
    while (1) {}
}

uint8_t power_AXP2101_chip_version(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t ver = 0;
    read_reg(AXP2101_REG_CHIP_VER, &ver);
    return ver;
}
