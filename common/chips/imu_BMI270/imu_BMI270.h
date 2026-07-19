/*
 * imu_BMI270.h — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Map                                                              */
/*===========================================================================*/

#define BMI270_REG_CHIP_ID      0x00
#define BMI270_REG_ACCEL_X_LSB  0x03
#define BMI270_REG_GYRO_X_LSB   0x09
#define BMI270_REG_STATUS       0x21
#define BMI270_REG_CMD          0x7C
#define BMI270_REG_PWR_CONF     0x7D
#define BMI270_REG_PWR_CTRL     0x7E
#define BMI270_REG_INIT_CTRL    0x59
#define BMI270_REG_INIT_DATA    0x5E
#define BMI270_INIT_ADDR_0      0x5B
#define BMI270_INIT_ADDR_1      0x5C

#define BMI270_CHIP_ID_VAL      0x24
#define BMI270_CMD_SOFTRESET    0xB6
#define BMI270_ACC_EN           (1 << 0)
#define BMI270_GYR_EN           (1 << 1)

/* STATUS register bits */
#define BMI270_STATUS_ACC_DRDY  (1 << 0)
#define BMI270_STATUS_GYR_DRDY  (1 << 1)

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef struct {
    int16_t x;   /* milli-g */
    int16_t y;
    int16_t z;
} imu_BMI270_accel_t;

typedef struct {
    int32_t x;   /* milli-dps */
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

int  imu_BMI270_init(i2c_master_dev_handle_t dev);
void imu_BMI270_deinit(void);

/*===========================================================================*/
/* Data                                                                      */
/*===========================================================================*/

int  imu_BMI270_read(imu_BMI270_data_t *data);
int  imu_BMI270_set_mode(imu_BMI270_mode_t mode);
uint8_t imu_BMI270_chip_id(void);

#ifdef __cplusplus
}
#endif
