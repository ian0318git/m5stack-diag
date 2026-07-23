/*
 * imu_BMI270.h — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver — platform-agnostic.
 * Uses abstract diag_i2c_t transport.
 *
 * NOTE: Delay functions (vTaskDelay, esp_rom_delay_us) are the
 * remaining platform dependency.  Caller must ensure FreeRTOS
 * (or compatible delay) is available.
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
/* Register Map                                                              */
/*===========================================================================*/

#define BMI270_ADDR           0x69

#define BMI270_REG_CHIP_ID      0x00
#define BMI270_REG_ERR_REG      0x02
#define BMI270_REG_STATUS       0x03
#define BMI270_REG_ACC_X_LSB    0x04
#define BMI270_REG_ACC_X_MSB    0x05
#define BMI270_REG_ACC_Y_LSB    0x06
#define BMI270_REG_ACC_Y_MSB    0x07
#define BMI270_REG_ACC_Z_LSB    0x08
#define BMI270_REG_ACC_Z_MSB    0x09
#define BMI270_REG_GYR_X_LSB    0x0A
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

/* Accel / Gyro configuration (need explicit setup after config load) */
#define BMI270_REG_ACC_CONF     0x40   /* bits[7:4]=ODR, bits[1:0]=filter */
#define BMI270_ACC_ODR_100HZ    (0x0A << 4)
#define BMI270_ACC_ODR_200HZ    (0x0B << 4)
#define BMI270_ACC_ODR_400HZ    (0x0C << 4)
#define BMI270_ACC_RANGE_2G     0x00

#define BMI270_REG_GYR_CONF     0x42   /* bits[7:4]=ODR, bits[1:0]=filter */
#define BMI270_GYR_ODR_100HZ    (0x08 << 4)
#define BMI270_GYR_ODR_200HZ    (0x09 << 4)
#define BMI270_GYR_RANGE_2000DPS 0x00

#define BMI270_CHIP_ID_VAL      0x24
#define BMI270_CMD_SOFTRESET    0xB6
#define BMI270_ACC_EN           (1 << 0)
#define BMI270_GYR_EN           (1 << 1)

#define BMI270_STATUS_ACC_DRDY  (1 << 0)
#define BMI270_STATUS_GYR_DRDY  (1 << 1)

#define BMI270_REG_INT_STATUS   0x21
#define BMI270_INT_STAT_DONE    (1 << 0)

#define BMI270_CONFIG_START_ADDR  0x8000
#define BMI270_INIT_WAIT_MS       150

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

#ifdef __cplusplus
}
#endif
