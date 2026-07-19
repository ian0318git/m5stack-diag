/*
 * hal_imu.h - Hardware Abstraction Layer: BMI270 6-Axis IMU
 *
 * Interface for accelerometer and gyroscope data.
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

/** Accelerometer data in milli-g (mg). */
typedef struct {
    int16_t x;     /* milli-g (e.g. +1000 = 1g)    */
    int16_t y;
    int16_t z;
} hal_imu_accel_t;

/** Gyroscope data in milli-degrees-per-second (mdps). */
typedef struct {
    int32_t x;     /* milli-dps                      */
    int32_t y;
    int32_t z;
} hal_imu_gyro_t;

/** All IMU data from a single read. */
typedef struct {
    hal_imu_accel_t accel;
    hal_imu_gyro_t  gyro;
    uint8_t         chip_id;    /* Chip identification byte */
} hal_imu_data_t;

/** IMU operating mode. */
typedef enum {
    HAL_IMU_MODE_NORMAL,
    HAL_IMU_MODE_SUSPEND,
    HAL_IMU_MODE_DEEP_SUSPEND,
} hal_imu_mode_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the BMI270 over I2C.
 * @return DIAG_PASSED on success, DIAG_FAILED if chip not found.
 */
diag_result_t hal_imu_init(void);

/**
 * @brief Release IMU resources.
 */
void hal_imu_deinit(void);

/*===========================================================================*/
/* Data access                                                               */
/*===========================================================================*/

/**
 * @brief Perform a single read of accelerometer and gyroscope.
 * @param[out] data  Filled with sensor data.
 * @return DIAG_PASSED on success, DIAG_FAILED on I2C error.
 */
diag_result_t hal_imu_read(hal_imu_data_t *data);

/**
 * @brief Set the IMU operating mode.
 */
diag_result_t hal_imu_set_mode(hal_imu_mode_t mode);

/**
 * @brief Read BMI270 chip ID register.
 * @return Chip ID byte, 0 on error.
 */
uint8_t hal_imu_chip_id(void);

#ifdef __cplusplus
}
#endif
