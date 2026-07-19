/*
 * imu_BMI270.c — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "imu_BMI270.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "BMI270";
static i2c_master_dev_handle_t s_dev = NULL;
static bool s_init = false;

/* Scale constants for ±2g / ±2000dps ranges */
#define ACCEL_MG_PER_LSB   0.061f   /* 2g / 32768 * 1000 */
#define GYRO_MDPS_PER_LSB  61.0f    /* 2000 / 32768 * 1000 */

/*===========================================================================*/
/* I2C helpers                                                               */
/*===========================================================================*/

static int read_reg(uint8_t reg, uint8_t *val)
{
    return (i2c_master_transmit_receive(s_dev, &reg, 1, val, 1, -1)
            == ESP_OK) ? 0 : -1;
}

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return (i2c_master_transmit(s_dev, buf, 2, -1) == ESP_OK) ? 0 : -1;
}

static int read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    return (i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, -1)
            == ESP_OK) ? 0 : -1;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int imu_BMI270_init(i2c_master_dev_handle_t dev)
{
    if (!dev) return -1;
    s_dev = dev;

    uint8_t chip_id = 0;
    if (read_reg(BMI270_REG_CHIP_ID, &chip_id) != 0) {
        ESP_LOGE(TAG, "BMI270 not responding");
        return -1;
    }
    if (chip_id != BMI270_CHIP_ID_VAL) {
        ESP_LOGW(TAG, "Unexpected chip ID: 0x%02X (expected 0x%02X)",
                 chip_id, BMI270_CHIP_ID_VAL);
    }

    /* Soft-reset & wait for boot */
    write_reg(BMI270_REG_CMD, BMI270_CMD_SOFTRESET);
    esp_rom_delay_us(2000);

    /* Enable accel + gyro */
    write_reg(BMI270_REG_PWR_CONF, 0x00);
    esp_rom_delay_us(1000);
    write_reg(BMI270_REG_PWR_CTRL, BMI270_ACC_EN | BMI270_GYR_EN);
    esp_rom_delay_us(10000);

    s_init = true;
    ESP_LOGI(TAG, "BMI270 initialised (chip_id=0x%02X)", chip_id);
    return 0;
}

void imu_BMI270_deinit(void)
{
    if (!s_init) return;
    write_reg(BMI270_REG_PWR_CONF, (1 << 1)); /* suspend */
    s_init = false;
    s_dev = NULL;
}

int imu_BMI270_read(imu_BMI270_data_t *data)
{
    if (!data || !s_dev) return -1;
    memset(data, 0, sizeof(*data));

    uint8_t buf[12];
    if (read_regs(BMI270_REG_ACCEL_X_LSB, buf, 12) != 0) return -1;

    int16_t ax = (int16_t)(buf[0] | (buf[1] << 8));
    int16_t ay = (int16_t)(buf[2] | (buf[3] << 8));
    int16_t az = (int16_t)(buf[4] | (buf[5] << 8));

    data->accel.x = (int16_t)(ax * ACCEL_MG_PER_LSB);
    data->accel.y = (int16_t)(ay * ACCEL_MG_PER_LSB);
    data->accel.z = (int16_t)(az * ACCEL_MG_PER_LSB);

    int16_t gx = (int16_t)(buf[6]  | (buf[7]  << 8));
    int16_t gy = (int16_t)(buf[8]  | (buf[9]  << 8));
    int16_t gz = (int16_t)(buf[10] | (buf[11] << 8));

    data->gyro.x = (int32_t)(gx * GYRO_MDPS_PER_LSB);
    data->gyro.y = (int32_t)(gy * GYRO_MDPS_PER_LSB);
    data->gyro.z = (int32_t)(gz * GYRO_MDPS_PER_LSB);

    read_reg(BMI270_REG_CHIP_ID, &data->chip_id);
    return 0;
}

int imu_BMI270_set_mode(imu_BMI270_mode_t mode)
{
    if (!s_dev) return -1;

    switch (mode) {
    case IMU_BMI270_MODE_NORMAL:
        write_reg(BMI270_REG_PWR_CONF, 0x00);
        write_reg(BMI270_REG_PWR_CTRL, BMI270_ACC_EN | BMI270_GYR_EN);
        break;
    case IMU_BMI270_MODE_SUSPEND:
        write_reg(BMI270_REG_PWR_CTRL, 0x00);
        break;
    case IMU_BMI270_MODE_DEEP_SUSPEND:
        write_reg(BMI270_REG_PWR_CTRL, 0x00);
        write_reg(BMI270_REG_PWR_CONF, (1 << 1));
        break;
    }
    esp_rom_delay_us(5000);
    return 0;
}

uint8_t imu_BMI270_chip_id(void)
{
    if (!s_dev) return 0;
    uint8_t id = 0;
    read_reg(BMI270_REG_CHIP_ID, &id);
    return id;
}
