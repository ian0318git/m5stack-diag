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
/*===========================================================================*/

#define AXP2101_REG_PWR_STATUS  0x00
#define AXP2101_REG_CHIP_VER    0x01
#define AXP2101_REG_VBUS_ADC_H  0x15
#define AXP2101_REG_VBUS_ADC_L  0x16
#define AXP2101_REG_BAT_ADC_H   0x34
#define AXP2101_REG_BAT_ADC_L   0x35
#define AXP2101_REG_CHG_CUR_H   0x62
#define AXP2101_REG_CHG_CUR_L   0x63
#define AXP2101_REG_TEMP_H      0x94
#define AXP2101_REG_TEMP_L      0x95

/* I2C address (7-bit) */
#define AXP2101_ADDR            0x34

#define AXP2101_PWR_VBUS        (1 << 0)
#define AXP2101_PWR_BAT_CHG     (1 << 1)
#define AXP2101_PWR_BAT_FULL    (1 << 2)
#define AXP2101_PWR_BAT_EXIST   (1 << 3)

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef enum {
    AXP2101_FLAG_NONE         = 0,
    AXP2101_FLAG_USB          = (1 << 0),
    AXP2101_FLAG_BAT_CHARGING = (1 << 1),
    AXP2101_FLAG_BAT_FULL     = (1 << 2),
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

/*===========================================================================*/
/* Control                                                                   */
/*===========================================================================*/

void power_AXP2101_shutdown(void) __attribute__((noreturn));
uint8_t power_AXP2101_chip_version(void);

#ifdef __cplusplus
}
#endif
