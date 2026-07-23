/*
 * imu_BMI270.c — BMI270 6-Axis IMU (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 * Init sequence and register map matched to M5Unified BMI270_Class
 * (M5Stack's official CoreS3 Arduino library).
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

/*===========================================================================*/
/* I2C helpers                                                               */
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
/* Config blob upload (per M5Unified BMI270_Class::_upload_file)            */
/*===========================================================================*/

static int load_config(void)
{
    static bool loaded = false;
    if (loaded) return 0;

    /* Write config in 32-byte chunks (per M5Unified BMI270_Class) */
    for (size_t i = 0; i < sizeof(bmi270_config_file); i += 32) {
        size_t chunk = sizeof(bmi270_config_file) - i;
        if (chunk > 32) chunk = 32;

        /* Set word address: split byte_index/2 across INIT_ADDR_0/1 */
        uint8_t addr[2] = {
            (uint8_t)((i >> 1) & 0x0F),   /* bits [3:0] */
            (uint8_t)(i >> 5),             /* bits [11:4] */
        };
        if (s_i2c->write(s_bus, BMI270_ADDR,
                         (uint8_t[]){BMI270_REG_INIT_ADDR_0, addr[0], addr[1]}, 3) != 0)
            return -1;

        /* Burst-write chunk to INIT_DATA */
        uint8_t buf[33] = {BMI270_REG_INIT_DATA};
        memcpy(&buf[1], &bmi270_config_file[i], chunk);
        if (s_i2c->write(s_bus, BMI270_ADDR, buf, 1 + chunk) != 0)
            return -1;
    }

    /* Trigger firmware load */
    write_reg(BMI270_REG_INIT_CTRL, 0x01);
    vTaskDelay(pdMS_TO_TICKS(20));

    /* Poll INTERNAL_STATUS for done */
    uint8_t status = 0;
    for (int r = 0; r < 60; r++) {
        if (read_reg(BMI270_REG_INT_STATUS, &status) == 0 && (status & BMI270_INT_STAT_DONE)) {
            loaded = true;
            vTaskDelay(pdMS_TO_TICKS(50));
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
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

    /* Step 1: Soft reset (CMD_REG = 0x7E per M5Unified) */
    write_reg(BMI270_REG_CMD, BMI270_CMD_SOFTRESET);
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Step 2: Disable advance power save (per M5Unified) */
    write_reg(BMI270_REG_PWR_CONF, 0x00);
    vTaskDelay(pdMS_TO_TICKS(2));

    /* Step 3: Upload config blob (base 8192 B) */
    if (load_config() != 0) {
        return -1;
    }

    /* Step 5: Enable data-ready interrupt mapping (M5Unified writes 0xFF) */
    if (write_reg(BMI270_REG_INT_MAP_DATA, 0xFF) != 0) return -1;

    /* Step 6: Set accel/gyro ODR and range */
    write_reg(BMI270_REG_ACC_CONF, BMI270_ACC_ODR_100HZ);
    write_reg(0x41, BMI270_ACC_RANGE_2G);       /* ACC_RANGE */
    write_reg(BMI270_REG_GYR_CONF, BMI270_GYR_ODR_100HZ);
    write_reg(0x43, BMI270_GYR_RANGE_2000DPS);  /* GYR_RANGE */
    vTaskDelay(pdMS_TO_TICKS(2));

    /* Step 7: Enable accel + gyro power (PWR_CTRL = 0x7D per M5Unified) */
    if (write_reg(BMI270_REG_PWR_CTRL, BMI270_ACC_EN | BMI270_GYR_EN) != 0) {
        return -1;
    }
    vTaskDelay(pdMS_TO_TICKS(50));

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

    /* Check INT_STATUS_1 (0x1D) for data-ready (per M5Unified)
     * bit 7 = accel ready, bit 6 = gyro ready */
    uint8_t drdy = 0;
    for (int retry = 0; retry < 500; retry++) {
        if (read_reg(BMI270_REG_INT_STATUS_1, &drdy) == 0 &&
            (drdy & (BMI270_DRDY_ACCEL | BMI270_DRDY_GYRO))) {
            break;
        }
        esp_rom_delay_us(100);
    }

    /* Read feature engine data frame from register 0x04.
     * Per M5Unified: 20-byte frame at AUX_X_LSB (0x04)
     *   buf[0..2] = mag (3 × int16)
     *   buf[3]    = ?
     *   buf[4..6] = accel (3 × int16)  ← at byte offset 8
     *   buf[7..9] = gyro  (3 × int16)  ← at byte offset 14 */
    int16_t frame[10];
    if (read_regs(BMI270_REG_DATA_FRAME, (uint8_t *)frame, 20) != 0) return -1;

    data->accel.x = frame[4];
    data->accel.y = frame[5];
    data->accel.z = frame[6];
    data->gyro.x  = frame[7];
    data->gyro.y  = frame[8];
    data->gyro.z  = frame[9];

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

uint8_t imu_BMI270_status(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t s = 0;
    read_reg(BMI270_REG_INT_STATUS_1, &s);
    return s;
}
