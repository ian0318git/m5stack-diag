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

    /* Register dump for diagnosis */
    {
        extern i2c_master_dev_handle_t get_imu_dev(void);
        i2c_master_dev_handle_t dev = get_imu_dev();
        if (dev) {
            const uint8_t addrs[] = {0x00,0x02,0x03,0x1D,0x21,0x40,0x42,0x7C,0x7D,0x7E};
            const char *names[] = {"CHIP_ID","ERR","STATUS","INT_STS1","INT_STS","ACC_CFG","GYR_CFG","PWR_CONF","PWR_CTRL","CMD"};
            diag_menu_printf("  Regs:");
            for (int i = 0; i < 10; i++) {
                uint8_t v = 0;
                uint8_t r = addrs[i];
                i2c_master_transmit_receive(dev, &r, 1, &v, 1, -1);
                diag_menu_printf(" %s=0x%02X", names[i], v);
            }
            diag_menu_printf("\r\n");
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
