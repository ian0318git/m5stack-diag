/*
 * test_imu_BMI270.c — Unit tests for BMI270 6-axis IMU chip driver
 *
 * Tests through the abstract diag_i2c_t transport seam with a mock.
 *
 * Compile:
 *   gcc -I . -I freertos -I esp_rom \
 *       -I ../../common/chips/imu_BMI270 \
 *       -I ../../common/src/M5Stack_CoreS3/include \
 *       -include esp_stubs.h \
 *       -o test_imu_BMI270 test_imu_BMI270.c \
 *       ../../common/chips/imu_BMI270/imu_BMI270.c \
 *       -lm
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "mock_i2c.h"
#include "imu_BMI270.h"

static int s_passed = 0;
static int s_run = 0;
#define TEST(name) do { printf("  %s ... ", name); s_run++; } while (0)
#define PASS()     do { printf("PASSED\n"); s_passed++; } while (0)

static void set_defaults(uint8_t *regs)
{
    memset(regs, 0, 256);
    regs[BMI270_REG_CHIP_ID] = BMI270_CHIP_ID_VAL;
    regs[BMI270_REG_INT_STATUS] = BMI270_INT_STAT_DONE;
}

/*===================================================================*/
/* Chip ID: read register 0x00, expect 0x24                         */
/*===================================================================*/
static void test_chip_id(void)
{
    TEST("chip ID = 0x24");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    assert(imu_BMI270_chip_id() == BMI270_CHIP_ID_VAL);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Init succeeds with correct register setup                          */
/*===================================================================*/
static void test_init_success(void)
{
    TEST("init succeeds");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Init fails when chip doesn't respond                               */
/*===================================================================*/
static void test_init_fails_no_chip(void)
{
    TEST("init fails when chip missing");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    memset(regs, 0, 256);  /* no chip ID */

    assert(imu_BMI270_init(&mock_transport, bus) == -1);
    PASS();
}

/*===================================================================*/
/* Read returns zero when registers are zero                          */
/*===================================================================*/
static void test_read_zero(void)
{
    TEST("read returns zero data");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);
    assert(d.accel.x == 0 && d.accel.y == 0 && d.accel.z == 0);
    assert(d.gyro.x == 0 && d.gyro.y == 0 && d.gyro.z == 0);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Read returns correct values from M5Unified legacy regs            */
/* ACC at 0x0C (6 bytes, 3 × int16 LSB first)                       */
/* GYR at 0x12 (6 bytes, 3 × int16 LSB first)                       */
/* Scale: accel mg = raw × 8000 / 32768 (±8g)                      */
/*        gyro mdps = raw × 2000000 / 32768 (±2000dps)             */
/*===================================================================*/
static void test_read_positive(void)
{
    TEST("read accel=+500mg gyro=+1000dps");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    /* Set accel raw = 2048 at 0x0C: 2048 × 8000 / 32768 ≈ 500 mg */
    int16_t a_raw = 2048;
    for (int i = 0; i < 3; i++) {
        regs[0x0C + i*2]     = (uint8_t)(a_raw & 0xFF);
        regs[0x0C + i*2 + 1] = (uint8_t)((a_raw >> 8) & 0xFF);
    }

    /* Set gyro raw = 16384 at 0x12: 16384 × 2000000 / 32768 = 1000000 mdps */
    int16_t g_raw = 16384;
    for (int i = 0; i < 3; i++) {
        regs[0x12 + i*2]     = (uint8_t)(g_raw & 0xFF);
        regs[0x12 + i*2 + 1] = (uint8_t)((g_raw >> 8) & 0xFF);
    }

    /* Verify mock registers are correctly set */
    assert(regs[0x0C] == 0x00 && regs[0x0D] == 0x08);  /* 2048 little-endian */
    assert(regs[0x12] == 0x00 && regs[0x13] == 0x40);  /* 16384 little-endian */

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);

    printf("accel=%d,%d,%d gyro=%ld,%ld,%ld ",
           d.accel.x, d.accel.y, d.accel.z,
           (long)d.gyro.x, (long)d.gyro.y, (long)d.gyro.z);

    /* Should be ≈500mg and ≈1000000mdps on all axes */
    assert(d.accel.x >= 450 && d.accel.x <= 550);
    assert(d.gyro.x >= 900000 && d.gyro.x <= 1100000);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Main                                                              */
/*===================================================================*/
int main(void)
{
    setbuf(stdout, NULL);
    printf("\nBMI270 IMU Unit Tests\n");
    printf("=====================\n\n");

    test_chip_id();
    test_init_success();
    test_init_fails_no_chip();
    test_read_zero();
    test_read_positive();

    printf("\n=====================\n");
    printf("Result: %d/%d passed\n", s_passed, s_run);
    return (s_passed == s_run) ? 0 : 1;
}
