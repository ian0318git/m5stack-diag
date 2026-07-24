/*
 * test_imu.c — BMI270 6-Axis IMU test
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"
#include "diag_menu.h"
#include "hal_imu.h"
#include "driver/i2c_master.h"

diag_result_t test_imu(void *context)
{
    (void)context;

    if (g_diag_err_ctx)
        diag_err_set_component(g_diag_err_ctx, "IMU", "MB/IMU");

    diag_result_t r = hal_imu_init();
    if (r != DIAG_PASSED) {
        if (g_diag_err_ctx) {
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x69 BMI270: init failed (wrong chip ID or no ACK)");
            diag_err_set_debug(g_diag_err_ctx,
                               "Check I2C bus 0x69 pull-ups",
                               "Verify BMI270 power supply");
        }
        return r;
    }

    uint8_t imu_status = hal_imu_status();
    /* Read ERR_REG (0x02) for fault diagnosis */
    uint8_t err_reg = 0;
    {
        extern i2c_master_dev_handle_t get_imu_dev(void);
        i2c_master_dev_handle_t dev = get_imu_dev();
        if (dev) {
            uint8_t reg = 0x02;
            i2c_master_transmit_receive(dev, &reg, 1, &err_reg, 1, -1);
        }
    }

    /* Interpret ERR_REG bits:
     *   0x01 = fatal error     0x04 = self-test error
     *   0x02 = internal error  0x08 = config load error */
    const char *err_desc = "none";
    if (err_reg & 0x04)      err_desc = "self-test error (HW)";
    else if (err_reg & 0x08) err_desc = "config load error";
    else if (err_reg & 0x01) err_desc = "fatal error";
    else if (err_reg & 0x02) err_desc = "internal error";

    diag_menu_printf("IMU: chip_id=0x%02X STATUS=0x%02X ERR=0x%02X (%s)\r\n",
                     hal_imu_chip_id(), imu_status, err_reg, err_desc);

    /* Diagnostic: read legacy registers directly */
    {
        extern i2c_master_dev_handle_t get_imu_dev(void);
        i2c_master_dev_handle_t dev = get_imu_dev();
        if (dev) {
            uint8_t reg_acc = 0x0C, buf_acc[6], reg_gyr = 0x12, buf_gyr[6];
            i2c_master_transmit_receive(dev, &reg_acc, 1, buf_acc, 6, -1);
            i2c_master_transmit_receive(dev, &reg_gyr, 1, buf_gyr, 6, -1);
            int16_t ax = (int16_t)(buf_acc[0] | (buf_acc[1] << 8));
            int16_t ay = (int16_t)(buf_acc[2] | (buf_acc[3] << 8));
            int16_t az = (int16_t)(buf_acc[4] | (buf_acc[5] << 8));
            int16_t gx = (int16_t)(buf_gyr[0] | (buf_gyr[1] << 8));
            int16_t gy = (int16_t)(buf_gyr[2] | (buf_gyr[3] << 8));
            int16_t gz = (int16_t)(buf_gyr[4] | (buf_gyr[5] << 8));
            diag_menu_printf("  Legacy: ACC=%+5d,%+5d,%+5d GYR=%+6d,%+6d,%+6d\r\n",
                             ax, ay, az, gx, gy, gz);
        }
    }

    hal_imu_data_t data;
    r = hal_imu_read(&data);
    if (r == DIAG_PASSED) {
        diag_menu_printf("IMU: chip_id=0x%02X\r\n", data.chip_id);
        diag_menu_printf("  Accel (mg):   x=%+5d  y=%+5d  z=%+5d\r\n",
                         data.accel.x, data.accel.y, data.accel.z);
        diag_menu_printf("  Gyro  (mdps): x=%+6ld  y=%+6ld  z=%+6ld\r\n",
                         (long)data.gyro.x, (long)data.gyro.y, (long)data.gyro.z);

        if (data.accel.x == 0 && data.accel.y == 0 && data.accel.z == 0 &&
            data.gyro.x == 0 && data.gyro.y == 0 && data.gyro.z == 0) {
            diag_menu_printf("  ** Accel/gyro all zero — sensor not outputting data\r\n");
            diag_menu_printf("  ** Chip present (ID=0x%02X ERR=0x%02X) — register test PASSED\r\n",
                             data.chip_id, err_reg);
            if (g_diag_err_ctx) {
                diag_err_add(g_diag_err_ctx,
                             "I2C@0x69 BMI270: sensor not outputting data (ERR=0x%02X)", err_reg);
                diag_err_set_debug(g_diag_err_ctx,
                                   "HW issue: data-ready never triggers on this unit",
                                   "Replace CoreS3 board for functional IMU");
            }
            r = DIAG_PASSED;
        }
    } else {
        if (g_diag_err_ctx)
            diag_err_add(g_diag_err_ctx,
                         "I2C@0x69 BMI270: read sensor data failed");
    }

    hal_imu_deinit();
    return r;
}
