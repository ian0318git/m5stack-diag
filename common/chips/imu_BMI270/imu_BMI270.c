/*
 * imu_BMI270.c — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 * FreeRTOS delay functions (vTaskDelay) and esp_rom_delay_us are
 * the only remaining platform dependencies.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "imu_BMI270.h"
#include "imu_BMI270_config.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdbool.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;

/* Scale constants: accel ±2g (12-bit), gyro ±2000dps (16-bit) */
#define ACCEL_MG_PER_LSB   0.4883f
#define GYRO_MDPS_PER_LSB  61.0f

/*===========================================================================*/
/* I2C helpers (use abstract transport)                                      */
/*===========================================================================*/

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, BMI270_ADDR, &reg, 1, val, 1);
}

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return s_i2c->write(s_bus, BMI270_ADDR, buf, 2);
}

static int read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    return s_i2c->write_then_read(s_bus, BMI270_ADDR, &reg, 1, buf, len);
}

/*===========================================================================*/
/* Config blob loading                                                       */
/*===========================================================================*/

static int load_config(void)
{
    static bool loaded = false;
    if (loaded) return 0;

    /* Step 1: Disable config load (clear INIT_CTRL bit 0) */
    write_reg(0x59, 0x00);

    /* Write config 32 bytes per chunk. */
    for (size_t i = 0; i < sizeof(bmi270_config_file); i += 32) {
        size_t chunk = sizeof(bmi270_config_file) - i;
        if (chunk > 32) chunk = 32;

        uint16_t word_addr = (uint16_t)(i / 2);
        write_reg(0x5B, (uint8_t)(word_addr & 0x0F));
        write_reg(0x5C, (uint8_t)((word_addr >> 4) & 0xFF));

        uint8_t buf[33];
        buf[0] = 0x5E;
        memcpy(&buf[1], &bmi270_config_file[i], chunk);
        if (s_i2c->write(s_bus, BMI270_ADDR, buf, 1 + chunk) != 0) {
            return -1;
        }
    }

    /* Step 2: Enable config load (set INIT_CTRL bit 0) */
    write_reg(0x59, 0x01);

    /* Poll INTERNAL_STATUS (0x21) bit 0, up to 300ms */
    uint8_t status = 0;
    for (int r = 0; r < 60; r++) {
        vTaskDelay(pdMS_TO_TICKS(5));
        read_reg(BMI270_REG_INT_STATUS, &status);
        if (status & BMI270_INT_STAT_DONE) {
            loaded = true;
            vTaskDelay(pdMS_TO_TICKS(50));
            return 0;
        }
    }

    return -1;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int imu_BMI270_init(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t chip_id = 0;
    if (read_reg(BMI270_REG_CHIP_ID, &chip_id) != 0) {
        return -1;
    }

    /* Soft-reset */
    if (write_reg(BMI270_REG_CMD, BMI270_CMD_SOFTRESET) != 0) {
        return -1;
    }
    esp_rom_delay_us(10000);

    /*
     * CRITICAL: Disable advance power save BEFORE loading the firmware
     * config blob.  The BMI270 powers up with APS enabled; the config
     * loader port (INIT_DATA / INIT_ADDR / INIT_CTRL) is inaccessible
     * while APS is active.  Per Bosch app note BST-BMI270-AN002-01:
     *
     *   1. Disable advanced power save: PWR_CONF.adv_power_save = 0
     *   2. Wait >= 450 us
     *   3. Upload config blob
     */
    if (write_reg(BMI270_REG_PWR_CONF, 0x00) != 0) return -1;
    esp_rom_delay_us(1000);

    /* Load firmware config (required for sensor data) */
    if (load_config() != 0) {
        return -1;
    }

    if (write_reg(BMI270_REG_PWR_CTRL, BMI270_ACC_EN | BMI270_GYR_EN) != 0) {
        return -1;
    }
    esp_rom_delay_us(50000);

    return 0;
}

void imu_BMI270_deinit(void)
{
    if (!s_i2c || !s_bus) return;
    write_reg(BMI270_REG_PWR_CONF, (1 << 1)); /* suspend */
    s_i2c = NULL;
    s_bus = NULL;
}

int imu_BMI270_read(imu_BMI270_data_t *data)
{
    if (!data || !s_i2c || !s_bus) return -1;
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
    if (!s_i2c || !s_bus) return -1;

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
    if (!s_i2c || !s_bus) return 0;
    uint8_t id = 0;
    read_reg(BMI270_REG_CHIP_ID, &id);
    return id;
}
