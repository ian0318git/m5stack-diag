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
#define BMI270_REG_ERR_REG      0x02
#define BMI270_REG_STATUS       0x03
#define BMI270_REG_ACC_X_LSB    0x04  /* 12-bit accel, LSB first */
#define BMI270_REG_ACC_X_MSB    0x05
#define BMI270_REG_ACC_Y_LSB    0x06
#define BMI270_REG_ACC_Y_MSB    0x07
#define BMI270_REG_ACC_Z_LSB    0x08
#define BMI270_REG_ACC_Z_MSB    0x09
#define BMI270_REG_GYR_X_LSB    0x0A  /* 16-bit gyro, LSB first */
#define BMI270_REG_GYR_X_MSB    0x0B
#define BMI270_REG_GYR_Y_LSB    0x0C
#define BMI270_REG_GYR_Y_MSB    0x0D
#define BMI270_REG_GYR_Z_LSB    0x0E
#define BMI270_REG_GYR_Z_MSB    0x0F
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

/* Internal status (0x21) — config load completion */
#define BMI270_REG_INT_STATUS   0x21
#define BMI270_INT_STAT_DONE    (1 << 0)

/* Config load sequence */
#define BMI270_CONFIG_START_ADDR  0x8000
#define BMI270_INIT_WAIT_MS       150

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
