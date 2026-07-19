/*
 * imu_BMI270.c — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "imu_BMI270.h"
#include "imu_BMI270_config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include <stdbool.h>

static const char *TAG = "BMI270";
static i2c_master_dev_handle_t s_dev = NULL;
static bool s_init = false;

/* Scale constants: accel ±2g (12-bit), gyro ±2000dps (16-bit) */
#define ACCEL_MG_PER_LSB   0.4883f  /* 2g / 4096 * 1000               */
#define GYRO_MDPS_PER_LSB  61.0f    /* 2000 / 32768 * 1000            */

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
/* Config blob loading (required for sensor data output)                     */
/*===========================================================================*/

static int load_config(void)
{
    static bool loaded = false;
    if (loaded) return 0;

    uint16_t addr = BMI270_CONFIG_START_ADDR;
    write_reg(0x5B, (uint8_t)(addr & 0xFF));
    write_reg(0x5C, (uint8_t)((addr >> 8) & 0xFF));

    /* Write config in 32-byte chunks */
    for (size_t i = 0; i < sizeof(bmi270_config_file); i += 32) {
        size_t chunk = sizeof(bmi270_config_file) - i;
        if (chunk > 32) chunk = 32;
        uint8_t buf[33];
        buf[0] = 0x5E;
        memcpy(&buf[1], &bmi270_config_file[i], chunk);
        if (i2c_master_transmit(s_dev, buf, 1 + chunk, -1) != ESP_OK) {
            ESP_LOGE(TAG, "Config write failed at offset %u", (unsigned)i);
            return -1;
        }
    }

    write_reg(0x59, 1);   /* Trigger config loading */

    /* Poll INIT_CTRL every 5ms, up to 300ms */
    uint8_t ctrl = 0xFF;
    for (int r = 0; r < 60 && ctrl != 0; r++) {
        vTaskDelay(pdMS_TO_TICKS(5));
        read_reg(0x59, &ctrl);
    }

    if (ctrl != 0) {
        ESP_LOGW(TAG, "Config init timeout (INIT_CTRL=0x%02X)", ctrl);
        return -1;
    }

    loaded = true;
    ESP_LOGI(TAG, "Config loaded (%u B)", (unsigned)sizeof(bmi270_config_file));
    return 0;
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

    /* Soft-reset then enable sensors */
    write_reg(BMI270_REG_CMD, BMI270_CMD_SOFTRESET);
    esp_rom_delay_us(10000);

    write_reg(BMI270_REG_PWR_CONF, 0x00);
    esp_rom_delay_us(1000);
    write_reg(BMI270_REG_PWR_CTRL, BMI270_ACC_EN | BMI270_GYR_EN);
    esp_rom_delay_us(50000);

    /* Try loading config blob (advisory — basic data works without it) */
    load_config();

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

    /* Check STATUS register — wait for data ready (up to 5 ms) */
    for (int retry = 0; retry < 50; retry++) {
        uint8_t status = 0;
        if (read_reg(BMI270_REG_STATUS, &status) == 0 &&
            (status & (BMI270_STATUS_ACC_DRDY | BMI270_STATUS_GYR_DRDY))) {
            break;
        }
        esp_rom_delay_us(100);
    }

    uint8_t buf[12];
    if (read_regs(BMI270_REG_ACC_X_LSB, buf, 12) != 0) return -1;

    /* Accel: 12-bit left-aligned in 16-bit, LSB first. Shift right by 4. */
    uint16_t ax_r = (uint16_t)(buf[0] | (buf[1] << 8)) >> 4;
    uint16_t ay_r = (uint16_t)(buf[2] | (buf[3] << 8)) >> 4;
    uint16_t az_r = (uint16_t)(buf[4] | (buf[5] << 8)) >> 4;

    /* Sign-extend 12-bit to 16-bit */
    int16_t ax_s = (int16_t)(ax_r & 0x0FFF);
    if (ax_s & 0x0800) ax_s |= 0xF000;
    int16_t ay_s = (int16_t)(ay_r & 0x0FFF);
    if (ay_s & 0x0800) ay_s |= 0xF000;
    int16_t az_s = (int16_t)(az_r & 0x0FFF);
    if (az_s & 0x0800) az_s |= 0xF000;

    data->accel.x = (int16_t)(ax_s * ACCEL_MG_PER_LSB);
    data->accel.y = (int16_t)(ay_s * ACCEL_MG_PER_LSB);
    data->accel.z = (int16_t)(az_s * ACCEL_MG_PER_LSB);

    /* Gyro: 16-bit two's complement, LSB first */
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
