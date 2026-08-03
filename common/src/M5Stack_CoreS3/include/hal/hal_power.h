/*
 * hal_power.h - Hardware Abstraction Layer: AXP2101 Power Management
 *
 * Interface for battery monitoring and power management.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/** Power supply status bitmask. */
typedef enum {
    HAL_POWER_FLAG_NONE             = 0,
    HAL_POWER_FLAG_USB              = (1 << 0),   /* USB/VBUS present      */
    HAL_POWER_FLAG_BAT_CHARGING     = (1 << 1),   /* Battery charging      */
    HAL_POWER_FLAG_BAT_DISCHARGING  = (1 << 2),   /* Battery discharging   */
} hal_power_flags_t;

/** PMU status snapshot. */
typedef struct {
    uint16_t   battery_millivolts;     /* Battery voltage (mV)         */
    uint8_t    battery_percent;        /* Estimated capacity 0..100    */
    uint16_t   usb_millivolts;         /* VBUS voltage (mV)            */
    uint16_t   system_millivolts;      /* System output voltage (mV)   */
    uint8_t    temperature_celsius;    /* Chip temperature (°C)        */
    hal_power_flags_t flags;           /* Status flags                 */
} hal_power_data_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the AXP2101 PMU over I2C.
 * @return DIAG_PASSED on success, DIAG_FAILED if chip not responding.
 */
diag_result_t hal_power_init(void);

/**
 * @brief Release PMU resources.
 */
void hal_power_deinit(void);

/*===========================================================================*/
/* Status                                                                    */
/*===========================================================================*/

/**
 * @brief Read full power status from the PMU.
 * @param[out] data  Filled with power data.
 * @return DIAG_PASSED on success, DIAG_FAILED on I2C error.
 */
diag_result_t hal_power_read(hal_power_data_t *data);

/**
 * @brief Get estimated battery percentage (derived from voltage curve).
 * @return 0..100, or 0 if no battery detected.
 */
uint8_t hal_power_battery_percent(void);

/**
 * @brief Check whether USB power is connected.
 * @return true if VBUS present.
 */
bool hal_power_is_vbus_present(void);

/*===========================================================================*/
/* Control                                                                   */
/*===========================================================================*/

/**
 * @brief Power off the system via the PMU.
 *
 * This function does not return on success.
 */
void hal_power_shutdown(void) __attribute__((noreturn));

/**
 * @brief Read the PMU chip version register.
 * @return Version byte, 0 on error.
 */
uint8_t hal_power_chip_version(void);

#ifdef __cplusplus
}
#endif
