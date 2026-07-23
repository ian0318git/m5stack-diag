/*
 * hal_imu.c — CoreS3 board adapter for BMI270 IMU
 *
 * Board-specific — bridges between the abstract chip driver and the
 * ESP-IDF I2C implementation through the transport seam.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "hal_imu.h"
#include "hal_i2c_helpers.h"
#include "hal_i2c_adapter.h"
#include "imu_BMI270.h"
#include "diag_config.h"
#include "esp_log.h"

static const char *TAG = "hal_imu";
static i2c_master_dev_handle_t s_i2c_dev = NULL;
static bool s_initialised = false;

diag_result_t hal_imu_init(void)
{
    if (s_initialised) return DIAG_PASSED;

    if (hal_i2c_add_device(CONFIG_I2C_ADDR_IMU, 400000, &s_i2c_dev)
        != DIAG_PASSED) {
        return DIAG_FAILED;
    }

    if (imu_BMI270_init(&g_diag_i2c_adapter, (void *)s_i2c_dev) != 0) {
        return DIAG_FAILED;
    }

    s_initialised = true;
    return DIAG_PASSED;
}

void hal_imu_deinit(void)
{
    if (!s_initialised) return;
    imu_BMI270_deinit();
    if (s_i2c_dev) {
        i2c_master_bus_rm_device(s_i2c_dev);
        s_i2c_dev = NULL;
    }
    s_initialised = false;
}

diag_result_t hal_imu_read(hal_imu_data_t *data)
{
    if (!data || !s_i2c_dev) return DIAG_FAILED;

    imu_BMI270_data_t raw;
    if (imu_BMI270_read(&raw) != 0) return DIAG_FAILED;

    data->accel.x = raw.accel.x;
    data->accel.y = raw.accel.y;
    data->accel.z = raw.accel.z;
    data->gyro.x  = raw.gyro.x;
    data->gyro.y  = raw.gyro.y;
    data->gyro.z  = raw.gyro.z;
    data->chip_id = raw.chip_id;
    return DIAG_PASSED;
}

diag_result_t hal_imu_set_mode(hal_imu_mode_t mode)
{
    static const imu_BMI270_mode_t map[] = {
        [HAL_IMU_MODE_NORMAL]       = IMU_BMI270_MODE_NORMAL,
        [HAL_IMU_MODE_SUSPEND]      = IMU_BMI270_MODE_SUSPEND,
        [HAL_IMU_MODE_DEEP_SUSPEND] = IMU_BMI270_MODE_DEEP_SUSPEND,
    };
    if ((unsigned)mode >= sizeof(map)/sizeof(map[0])) return DIAG_FAILED;
    return (imu_BMI270_set_mode(map[mode]) == 0) ? DIAG_PASSED : DIAG_FAILED;
}

uint8_t hal_imu_chip_id(void)
{
    return imu_BMI270_chip_id();
}

uint8_t hal_imu_status(void)
{
    return imu_BMI270_status();
}
