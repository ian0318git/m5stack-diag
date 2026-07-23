/*
 * imu_BMI270.h — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver — platform-agnostic.
 * Register map and init sequence matched to M5Unified BMI270_Class
 * (M5Stack's official CoreS3 Arduino library).
 *
 * Key facts discovered via M5Unified reverse engineering:
 *   - CMD_REG = 0x7E, PWR_CONF = 0x7C, PWR_CTRL = 0x7D
 *   - Data-ready on INT_STATUS_1 (0x1D) not STATUS (0x03)
 *   - Accel/Gyro data in feature engine frame at 0x04
 *   - INT_MAP_DATA (0x58) = 0xFF enables data-ready interrupts
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Map (per M5Unified BMI270_Class)                                 */
/*===========================================================================*/

#define BMI270_ADDR              0x69

/* Base registers */
#define BMI270_REG_CHIP_ID       0x00
#define BMI270_REG_ERR_REG       0x02
#define BMI270_REG_STATUS        0x03

/* Feature engine data frame at 0x04 (20 bytes: mag[3]+?+accel[3]+gyro[3]) */
#define BMI270_REG_DATA_FRAME    0x04

/* Interrupt status */
#define BMI270_REG_INT_STATUS_1  0x1D   /* bits 7=accel DRDY, 6=gyro DRDY, 5=mag DRDY */
#define BMI270_DRDY_ACCEL        (1 << 7)
#define BMI270_DRDY_GYRO         (1 << 6)

/* Config loader */
#define BMI270_REG_INIT_CTRL     0x59
#define BMI270_REG_INIT_ADDR_0   0x5B
#define BMI270_REG_INIT_ADDR_1   0x5C
#define BMI270_REG_INIT_DATA     0x5E

/* Feature/interrupt mapping */
#define BMI270_REG_INT_MAP_DATA  0x58   /* enable data-ready interrupt mapping */

/* Accel/Gyro configuration */
#define BMI270_REG_ACC_CONF      0x40
#define BMI270_ACC_ODR_100HZ     (0x0A << 4)
#define BMI270_ACC_RANGE_2G      0x00

#define BMI270_REG_GYR_CONF      0x42
#define BMI270_GYR_ODR_100HZ     (0x08 << 4)
#define BMI270_GYR_RANGE_2000DPS 0x00

/* Power / command (M5Unified addresses: CMD=0x7E, PWR_CONF=0x7C, PWR_CTRL=0x7D) */
#define BMI270_REG_CMD           0x7E
#define BMI270_REG_PWR_CONF      0x7C
#define BMI270_REG_PWR_CTRL      0x7D

/* Command values */
#define BMI270_CMD_SOFTRESET     0xB6

/* Power control bits */
#define BMI270_ACC_EN            (1 << 0)
#define BMI270_GYR_EN            (1 << 1)

/* Internal status (config loader) */
#define BMI270_REG_INT_STATUS    0x21
#define BMI270_INT_STAT_DONE     (1 << 0)

#define BMI270_CHIP_ID_VAL       0x24

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} imu_BMI270_accel_t;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} imu_BMI270_gyro_t;

typedef struct {
    imu_BMI270_accel_t accel;
    imu_BMI270_gyro_t  gyro;
    uint8_t            chip_id;
} imu_BMI270_data_t;

typedef enum {
    IMU_BMI270_MODE_NORMAL,
    IMU_BMI270_MODE_SUSPEND,
    IMU_BMI270_MODE_DEEP_SUSPEND,
} imu_BMI270_mode_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int  imu_BMI270_init(const diag_i2c_t *i2c, void *bus);
void imu_BMI270_deinit(void);

/*===========================================================================*/
/* Data                                                                      */
/*===========================================================================*/

int  imu_BMI270_read(imu_BMI270_data_t *data);
int  imu_BMI270_set_mode(imu_BMI270_mode_t mode);
uint8_t imu_BMI270_chip_id(void);
uint8_t imu_BMI270_status(void);

#ifdef __cplusplus
}
#endif
