/*
 * power_AXP2101.h — AXP2101 Power Management Unit (I2C)
 *
 * Common chip driver — platform-agnostic.
 * Caller provides an initialised diag_i2c_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Map                                                              */
/*                                                                           */
/* Verified against the AXP2101 datasheet bindings used by M5Stack's m5gfx   */
/* driver and the .NET nanoFramework AXP2101 device binding (Iot.Device):    */
/*   - 0x00 Status1: bit5 = VBUS good, bit3 = battery present                */
/*   - 0x01 Status2: bits[7:5] charge state (001=charging, 010=discharging)  */
/*   - 0x03 IcType: chip ID (real AXP2101 reads 0x4A)                        */
/*   - 0x30 ADC channel control: enable channels before reading              */
/*   - ADC data: 0x34 bat (13-bit H5L8), 0x38 VBUS (14-bit H6L8),            */
/*               0x3C die temp (14-bit H6L8) — LSB = 1 mV, raw in mV         */
/*   - 0x90 LDO enable bits: bit7=DLDO1 (LCD backlight), bit0=ALDO4 (touch)  */
/*   - 0x99 DLDO1 voltage: (mV - 500) / 100, 0x1C = 3.3 V                    */
/*===========================================================================*/

#define AXP2101_REG_PWR_STATUS1  0x00
#define AXP2101_REG_PWR_STATUS2  0x01
#define AXP2101_REG_CHIP_VER     0x03
#define AXP2101_REG_ADC_CTRL     0x30
#define AXP2101_REG_BAT_ADC_H    0x34
#define AXP2101_REG_BAT_ADC_L    0x35
#define AXP2101_REG_VBUS_ADC_H   0x38
#define AXP2101_REG_VBUS_ADC_L   0x39
#define AXP2101_REG_VSYS_ADC_H   0x3A
#define AXP2101_REG_VSYS_ADC_L   0x3B
#define AXP2101_REG_TEMP_H       0x3C
#define AXP2101_REG_TEMP_L       0x3D
#define AXP2101_REG_LDO_EN       0x90
#define AXP2101_REG_DLDO1_VOLT   0x99

/* I2C address (7-bit) */
#define AXP2101_ADDR            0x34

/* Status1 (0x00) bits */
#define AXP2101_PWR_VBUS        (1 << 5)   /* VBUS present and good */
#define AXP2101_PWR_BAT_EXIST   (1 << 3)   /* battery connected */

/* Status2 (0x01) charge state, bits [7:5] */
#define AXP2101_STAT2_CHG_STATE_SHIFT 5
#define AXP2101_STAT2_CHG_STATE_MASK  0x07
#define AXP2101_STAT2_CHG_CHARGING    0x01
#define AXP2101_STAT2_CHG_DISCHARGING 0x02

/* LDO enable (0x90) bits (CoreS3): DLDO1 = LCD backlight, ALDO4 = touch */
#define AXP2101_LDO_DLDO1        (1 << 7)
#define AXP2101_LDO_ALDO4        (1 << 0)

/* DLDO1 voltage register 0x99: value = (mV - 500) / 100 */
#define AXP2101_DLDO1_VOLT_3V3   0x1C

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef enum {
    AXP2101_FLAG_NONE         = 0,
    AXP2101_FLAG_USB          = (1 << 0),
    AXP2101_FLAG_BAT_CHARGING = (1 << 1),
    AXP2101_FLAG_BAT_DISCHARGING = (1 << 2),
} power_AXP2101_flags_t;

typedef struct {
    uint16_t battery_millivolts;
    uint8_t  battery_percent;
    uint16_t usb_millivolts;
    uint16_t system_millivolts;
    uint16_t charge_current_ma;
    uint8_t  temperature_celsius;
    power_AXP2101_flags_t flags;
} power_AXP2101_data_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the AXP2101 PMU.
 *
 * @param i2c   Abstract I2C transport.  Must remain valid for the
 *              driver's lifetime.
 * @param bus   I2C device handle (opaque, passed to transport callbacks).
 * @return 0 on success, -1 on error.
 */
int  power_AXP2101_init(const diag_i2c_t *i2c, void *bus);
void power_AXP2101_deinit(void);

/*===========================================================================*/
/* Status                                                                    */
/*===========================================================================*/

int  power_AXP2101_read(power_AXP2101_data_t *data);
uint8_t power_AXP2101_battery_percent(void);
bool power_AXP2101_is_vbus_present(void);

/**
 * @brief Read a single AXP2101 register (for bring-up debugging).
 * @param reg  Register address.
 * @param[out] val  Register value.
 * @return 0 on success, -1 on I2C error.
 */
int power_AXP2101_read_reg(uint8_t reg, uint8_t *val);

/*===========================================================================*/
/* Control                                                                   */
/*===========================================================================*/

void power_AXP2101_shutdown(void) __attribute__((noreturn));
uint8_t power_AXP2101_chip_version(void);

#ifdef __cplusplus
}
#endif
