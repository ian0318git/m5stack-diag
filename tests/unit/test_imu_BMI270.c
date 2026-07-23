/*
 * test_imu_BMI270.c — Unit tests for BMI270 6-axis IMU chip driver
 *
 * Tests 12-bit accel sign-extension, gyro 16-bit two's complement,
 * and scale conversion through the abstract diag_i2c_t seam.
 *
 * Compile:
 *   gcc -I ../../common/chips/imu_BMI270 \
 *       -I ../../common/src/M5Stack_CoreS3/include \
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
#include <math.h>
#include <assert.h>

#include "mock_i2c.h"
#include "imu_BMI270.h"

static int s_passed = 0;
static int s_run = 0;
#define TEST(name) do { printf("  %s ... ", name); s_run++; } while (0)
#define PASS()     do { printf("PASSED\n"); s_passed++; } while (0)

/*
 * BMI270 raw data format:
 *
 * Accel (12-bit left-aligned in 16-bit, LSB first):
 *   buf[0] | (buf[1] << 8) = raw 16-bit (left-aligned 12-bit)
 *   value_u12 = raw_16 >> 4          (12-bit unsigned)
 *   value_s16 = sign_extend_12(value_u12)
 *   accel_mg  = value_s16 * 0.4883f
 *
 * Gyro (16-bit two's complement, LSB first):
 *   gx = (int16_t)(buf[6] | (buf[7] << 8))
 *   gyro_mdps = gx * 61.0f
 */

static void set_accel_raw(uint8_t *regs, int16_t accel_12bit)
{
    /* accel_12bit is a 12-bit signed value (-2048 to +2047) */
    /* Shift left by 4 for left-aligned 16-bit, LSB first */
    uint16_t raw = (uint16_t)(accel_12bit & 0x0FFF) << 4;
    regs[0x04] = (uint8_t)(raw & 0xFF);        /* ACC_X_LSB */
    regs[0x05] = (uint8_t)((raw >> 8) & 0xFF); /* ACC_X_MSB */
    regs[0x06] = (uint8_t)(raw & 0xFF);        /* ACC_Y_LSB */
    regs[0x07] = (uint8_t)((raw >> 8) & 0xFF); /* ACC_Y_MSB */
    regs[0x08] = (uint8_t)(raw & 0xFF);        /* ACC_Z_LSB */
    regs[0x09] = (uint8_t)((raw >> 8) & 0xFF); /* ACC_Z_MSB */
}

static void set_gyro_raw(uint8_t *regs, int16_t gyro_16bit)
{
    regs[0x0A] = (uint8_t)(gyro_16bit & 0xFF);        /* GYR_X_LSB */
    regs[0x0B] = (uint8_t)((gyro_16bit >> 8) & 0xFF); /* GYR_X_MSB */
    regs[0x0C] = (uint8_t)(gyro_16bit & 0xFF);        /* GYR_Y_LSB */
    regs[0x0D] = (uint8_t)((gyro_16bit >> 8) & 0xFF); /* GYR_Y_MSB */
    regs[0x0E] = (uint8_t)(gyro_16bit & 0xFF);        /* GYR_Z_LSB */
    regs[0x0F] = (uint8_t)((gyro_16bit >> 8) & 0xFF); /* GYR_Z_MSB */
}

/* init() also does soft-reset, config blob load, and power enable.
   For mock: pre-mark config as loaded (INT_STAT=1) so load_config() succeeds. */
static void set_defaults(uint8_t *regs)
{
    memset(regs, 0, 256);
    regs[BMI270_REG_CHIP_ID] = BMI270_CHIP_ID_VAL;   /* 0x24 */
    regs[BMI270_REG_STATUS]  = BMI270_STATUS_ACC_DRDY | BMI270_STATUS_GYR_DRDY;
    regs[BMI270_REG_INT_STATUS] = BMI270_INT_STAT_DONE;  /* config loaded */
}

/*===================================================================*/
/* Chip ID                                                           */
/*===================================================================*/
static void test_chip_id(void)
{
    TEST("chip ID = 0x24");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    /* init reads chip ID; no config blob needed for mock */
    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    assert(imu_BMI270_chip_id() == BMI270_CHIP_ID_VAL);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Accel: +500 mg in X, Y, Z                                         */
/*   accel_12bit = 500 / 0.4883 ≈ 1024                               */
/*   After left-align shift (<<4) and LSB-first encoding              */
/*===================================================================*/
static void test_accel_positive(void)
{
    TEST("accel +500 mg X/Y/Z");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BMI270_REG_STATUS] |= BMI270_STATUS_ACC_DRDY;

    /* 1024 * 0.4883 = 500.0 mg (using nearby integer for 12-bit) */
    set_accel_raw(regs, 1024);  /* ≈ +500 mg after scale */
    /* set STATUS to indicate accel data ready */
    regs[BMI270_REG_STATUS] |= BMI270_STATUS_ACC_DRDY;

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);

    /* Accel scale: 1024 * 0.4883 = 500.0... allow ±1 rounding */
    int ax_mg = d.accel.x;
    printf("ax=%d ay=%d az=%d ", d.accel.x, d.accel.y, d.accel.z);
    assert(ax_mg >= 495 && ax_mg <= 505);
    assert(d.accel.y == d.accel.x);  /* all axes same raw value */
    assert(d.accel.z == d.accel.x);

    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Accel: -500 mg (negative 12-bit sign extension)                   */
/*   -1024 in 12-bit two's complement = 0x400 (bit 11 set)          */
/*   After shift: ax_r & 0x0FFF = 0x0400                            */
/*   Sign-extend: bit 11 set → 0xF400 → -1024 as int16              */
/*   Scale: -1024 * 0.4883 ≈ -500                                   */
/*===================================================================*/
static void test_accel_negative(void)
{
    TEST("accel -500 mg (sign extension)");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BMI270_REG_STATUS] |= BMI270_STATUS_ACC_DRDY;

    set_accel_raw(regs, -1024);  /* ≈ -500 mg */

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);

    printf("ax=%d ", d.accel.x);
    assert(d.accel.x >= -505 && d.accel.x <= -495);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Accel: zero (all axes)                                             */
/*===================================================================*/
static void test_accel_zero(void)
{
    TEST("accel zero");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BMI270_REG_STATUS] |= BMI270_STATUS_ACC_DRDY;

    set_accel_raw(regs, 0);

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);
    assert(d.accel.x == 0);
    assert(d.accel.y == 0);
    assert(d.accel.z == 0);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Gyro: +1000 dps in X                                              */
/*   gx = (int16_t)(buf[6] | (buf[7] << 8))                          */
/*   For +1000: gx = 1000 / 61.0 * 61.0 = 1000 raw? No...            */
/*   raw = 1000 / 61.0 ≈ 16 (since 61 mdps/LSB)                     */
/*   Actually: 1000 dps = 1000000 mdps / 61 mdps/LSB = 16393         */
/*   1000 * 1000 / 61 = 16393 (rounded)                               */
/*===================================================================*/
static void test_gyro_positive(void)
{
    TEST("gyro +1000 dps X");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BMI270_REG_STATUS] |= BMI270_STATUS_GYR_DRDY;

    /* For ~1000 dps: raw = 1000 / 61 * 1000 = 16393 */
    /* But the scale factor is 61 mdps/LSB, so 1000 dps = 1000000 mdps */
    /* raw = 1000000 / 61 = 16393 */
    int16_t raw = (int16_t)(1000000.0f / 61.0f + 0.5f);
    set_gyro_raw(regs, raw);
    set_accel_raw(regs, 1);  /* non-zero so data-ready check passes */

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);

    /* Expected: raw * 61.0 ≈ 1000000 mdps = 1000 dps */
    printf("gx=%ld ", (long)d.gyro.x);
    assert(d.gyro.x >= 900000 && d.gyro.x <= 1100000);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Gyro: negative direction                                           */
/*===================================================================*/
static void test_gyro_negative(void)
{
    TEST("gyro -500 dps X");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BMI270_REG_STATUS] |= BMI270_STATUS_GYR_DRDY;

    /* raw = -500000 / 61 ≈ -8197 */
    int16_t raw = (int16_t)(-500000.0f / 61.0f + 0.5f);
    set_gyro_raw(regs, raw);
    set_accel_raw(regs, 1);

    assert(imu_BMI270_init(&mock_transport, bus) == 0);
    imu_BMI270_data_t d;
    assert(imu_BMI270_read(&d) == 0);

    printf("gx=%ld ", (long)d.gyro.x);
    assert(d.gyro.x >= -550000 && d.gyro.x <= -450000);
    imu_BMI270_deinit();
    PASS();
}

/*===================================================================*/
/* Main                                                              */
/*===================================================================*/
int main(void)
{
    printf("\nBMI270 IMU Unit Tests\n");
    printf("=====================\n\n");

    test_chip_id();
    test_accel_positive();
    test_accel_negative();
    test_accel_zero();
    test_gyro_positive();
    test_gyro_negative();

    printf("\n=====================\n");
    printf("Result: %d/%d passed\n", s_passed, s_run);
    return (s_passed == s_run) ? 0 : 1;
}
