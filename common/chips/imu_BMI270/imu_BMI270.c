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
    /* Set word address to 0 (start of config) */
    uint8_t addr[2] = {0, 0};
    if (s_i2c->write(s_bus, BMI270_ADDR,
                     (uint8_t[]){BMI270_REG_INIT_ADDR_0, addr[0], addr[1]}, 3) != 0)
        return -1;

    /* Write entire config in one burst (matches M5Unified BMI270_Class) */
    size_t total = sizeof(bmi270_config_file);
    /* Split into I2C-burst-sized chunks (max 128 bytes per transaction) */
    for (size_t offset = 0; offset < total; ) {
        size_t chunk = total - offset;
        if (chunk > 128) chunk = 128;

        uint8_t buf[129] = {BMI270_REG_INIT_DATA};
        memcpy(&buf[1], &bmi270_config_file[offset], chunk);
        if (s_i2c->write(s_bus, BMI270_ADDR, buf, 1 + chunk) != 0)
            return -1;
        offset += chunk;

        /* Update word address for next chunk */
        uint16_t word_addr = (uint16_t)(offset / 2);
        addr[0] = (uint8_t)(word_addr & 0x0F);
        addr[1] = (uint8_t)(word_addr >> 4);
        if (offset < total) {
            if (s_i2c->write(s_bus, BMI270_ADDR,
                             (uint8_t[]){BMI270_REG_INIT_ADDR_0, addr[0], addr[1]}, 3) != 0)
                return -1;
        }
    }

    /* Trigger firmware load (per M5Unified order: INIT_CTRL → INT_MAP_DATA) */
    write_reg(BMI270_REG_INIT_CTRL, 0x01);
    write_reg(BMI270_REG_INT_MAP_DATA, 0xFF);

    /* Wait for feature engine ready (M5Unified polls for any non-zero) */
    uint8_t status = 0;
    for (int r = 0; r < 50; r++) {
        if (read_reg(BMI270_REG_INT_STATUS, &status) == 0 && status != 0) {
            vTaskDelay(pdMS_TO_TICKS(20));
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(2));
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
    if (chip_id != BMI270_CHIP_ID_VAL) {
        return -1;
    }

    /* Step 1: Soft reset */
    write_reg(BMI270_REG_CMD, BMI270_CMD_SOFTRESET);

    /* Step 2: Wait for reset to complete (PWR_CONF becomes non-zero per M5Unified) */
    {
        int retry = 30;
        uint8_t pwr = 0;
        do {
            vTaskDelay(pdMS_TO_TICKS(1));
            read_reg(BMI270_REG_PWR_CONF, &pwr);
        } while (pwr == 0 && --retry);
    }

    /* Step 3: Disable advance power save */
    write_reg(BMI270_REG_PWR_CONF, 0x00);
    vTaskDelay(pdMS_TO_TICKS(1));

    /* Step 3: Upload config blob (base 8192 B) */
    if (load_config() != 0) {
        return -1;
    }

    /* Step 5: Sensors enabled by the config blob's feature engine.
     * INT_MAP_DATA already written inside load_config(). */

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

    /* Read accel from legacy registers at 0x0C (per M5Unified ACC_X_LSB_ADDR)
     * and gyro from 0x12 (per M5Unified GYR_X_LSB_ADDR).
     * Each axis is 16-bit two's complement, LSB first, 6 bytes each. */
    {
        /* Read feature engine data frame from register 0x04 (AUX_X_LSB).
         * Per M5Unified BMI270_Class::getImuRawData():
         *   - Check INT_STATUS_1 (0x1D): bit 7=accel DRDY, bit 6=gyro DRDY
         *   - Read 20 bytes from 0x04 into int16_t[10]
         *   - buf[4..6] = accel, buf[7..9] = gyro (16-bit two's complement)
         *   - Scale: 1 LSB = 8g/32768 (accel), 2000dps/32768 (gyro) */
        uint8_t intstat = 0;
        read_reg(BMI270_REG_INT_STATUS_1, &intstat);

        int16_t frame[10];
        if (read_regs(BMI270_REG_DATA_FRAME, (uint8_t *)frame, 20) != 0) return -1;

        if (intstat & BMI270_DRDY_ACCEL) {
            data->accel.x = (int16_t)((int32_t)frame[4] * 8000 / 32768);
            data->accel.y = (int16_t)((int32_t)frame[5] * 8000 / 32768);
            data->accel.z = (int16_t)((int32_t)frame[6] * 8000 / 32768);
        }
        if (intstat & BMI270_DRDY_GYRO) {
            data->gyro.x  = (int32_t)((int64_t)frame[7] * 2000000 / 32768);
            data->gyro.y  = (int32_t)((int64_t)frame[8] * 2000000 / 32768);
            data->gyro.z  = (int32_t)((int64_t)frame[9] * 2000000 / 32768);
        }
    }

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
