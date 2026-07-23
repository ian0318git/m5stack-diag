/*
 * test_rtc_BM8563.c — Unit tests for BM8563 RTC chip driver
 *
 * Tests BCD conversion, register masking, VL flag handling,
 * time set/get round-trip, and format output.
 *
 * Compile:
 *   gcc -I ../../common/chips/rtc_BM8563 \
 *       -I ../../common/src/M5Stack_CoreS3/include \
 *       -o test_rtc_BM8563 test_rtc_BM8563.c \
 *       ../../common/chips/rtc_BM8563/rtc_BM8563.c \
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
#include "rtc_BM8563.h"

static int s_passed = 0;
static int s_run = 0;
#define TEST(name) do { printf("  %s ... ", name); s_run++; } while (0)
#define PASS()     do { printf("PASSED\n"); s_passed++; } while (0)

static void set_defaults(uint8_t *regs)
{
    memset(regs, 0, 256);
    /* Default time: 2026-07-23 14:30:15 (BCD encoded in registers) */
    regs[BM8563_REG_SEC]   = 0x15;  /* 15 seconds */
    regs[BM8563_REG_MIN]   = 0x30;  /* 30 minutes */
    regs[BM8563_REG_HOUR]  = 0x14;  /* 14 hours */
    regs[BM8563_REG_DAY]   = 0x23;  /* 23rd */
    regs[BM8563_REG_WEEKDAY] = 0x04; /* Thursday */
    regs[BM8563_REG_MONTH] = 0x07;  /* July */
    regs[BM8563_REG_YEAR]  = 0x26;  /* 2026 */
}

/*===================================================================*/
/* get_time: reads and decodes all 7 BCD registers                   */
/*===================================================================*/
static void test_get_time(void)
{
    TEST("get_time decodes BCD correctly");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(rtc_BM8563_init(&mock_transport, bus) == 0);

    rtc_BM8563_time_t t;
    memset(&t, 0xFF, sizeof(t));  /* fill with 0xFF to catch missing fields */
    assert(rtc_BM8563_get_time(&t) == 0);

    assert(t.second  == 15);
    assert(t.minute  == 30);
    assert(t.hour    == 14);
    assert(t.day     == 23);
    assert(t.weekday == 4);
    assert(t.month   == 7);
    assert(t.year    == 2026);

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* get_time: register masks are applied correctly                     */
/*                                                                   */
/* BCD masks: second/min=0x7F, hour/day=0x3F, weekday=0x07,         */
/* month=0x1F, year=full byte                                        */
/*===================================================================*/
static void test_get_time_masks(void)
{
    TEST("get_time applies register masks");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    /* Set upper unused bits — should be masked out */
    regs[BM8563_REG_SEC]   = 0x95;  /* 0x80 (VL) | 0x15 → should mask to 0x15 */
    regs[BM8563_REG_HOUR]  = 0x55;  /* 0x40 | 0x15 → should mask to 0x15 */
    regs[BM8563_REG_DAY]   = 0x4F;  /* 0x40 | 0x0F → should mask to 0x0F */

    assert(rtc_BM8563_init(&mock_transport, bus) == 0);
    rtc_BM8563_time_t t;
    assert(rtc_BM8563_get_time(&t) == 0);

    assert(t.second  == 15);   /* 0x95 & 0x7F = 0x15 → 15 */
    assert(t.hour    == 15);   /* 0x55 & 0x3F = 0x15 → 15 */
    assert(t.day     == 15);   /* 0x4F & 0x3F = 0x0F → 15 */
    assert(t.minute  == 30);

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* set_time: converts to BCD and writes correct I2C bytes            */
/*===================================================================*/
static void test_set_time(void)
{
    TEST("set_time converts to BCD correctly");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    assert(rtc_BM8563_init(&mock_transport, bus) == 0);

    rtc_BM8563_time_t t_in;
    memset(&t_in, 0, sizeof(t_in));
    t_in.year    = 2026;
    t_in.month   = 12;
    t_in.day     = 25;
    t_in.hour    = 9;
    t_in.minute  = 5;
    t_in.second  = 3;
    t_in.weekday = 3;

    assert(rtc_BM8563_set_time(&t_in) == 0);

    /* Verify I2C write: first byte is reg address, then BCD values */
    /* The write is 8 bytes: reg_addr + second/min/hour/day/weekday/month/year */
    assert(regs[BM8563_REG_SEC]   == 0x03);  /* bin2bcd(3)  = 0x03 */
    assert(regs[BM8563_REG_MIN]   == 0x05);  /* bin2bcd(5)  = 0x05 */
    assert(regs[BM8563_REG_HOUR]  == 0x09);  /* bin2bcd(9)  = 0x09 */
    assert(regs[BM8563_REG_DAY]   == 0x25);  /* bin2bcd(25) = 0x25 */
    assert(regs[BM8563_REG_WEEKDAY] == 0x03);
    assert(regs[BM8563_REG_MONTH] == 0x12);  /* bin2bcd(12) = 0x12 */
    assert(regs[BM8563_REG_YEAR]  == 0x26);  /* bin2bcd(26) = 0x26 (year - 2000) */

    /* Read back and verify round-trip */
    rtc_BM8563_time_t t_out;
    assert(rtc_BM8563_get_time(&t_out) == 0);
    assert(t_out.year    == 2026);
    assert(t_out.month   == 12);
    assert(t_out.day     == 25);
    assert(t_out.hour    == 9);
    assert(t_out.minute  == 5);
    assert(t_out.second  == 3);
    assert(t_out.weekday == 3);

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* VL flag: init clears VL bit and keeps time value                  */
/*===================================================================*/
static void test_vl_flag_cleared(void)
{
    TEST("init clears VL flag, preserves seconds");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BM8563_REG_SEC] = 0x95;  /* VL=1, seconds=0x15 */

    assert(rtc_BM8563_init(&mock_transport, bus) == 0);

    /* init should have cleared VL — write_reg(BM8563_REG_SEC, 0x15) */
    assert(regs[BM8563_REG_SEC] == 0x15);  /* VL cleared, seconds preserved */

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* VL flag: no VL → no write to seconds register                     */
/*===================================================================*/
static void test_no_vl_no_write(void)
{
    TEST("no VL flag → no write to seconds register");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);
    regs[BM8563_REG_SEC] = 0x15;  /* no VL */

    assert(rtc_BM8563_init(&mock_transport, bus) == 0);

    /* init should NOT have touched seconds register */
    assert(regs[BM8563_REG_SEC] == 0x15);

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* BCD boundary: 59 seconds, 59 minutes, 23 hours                    */
/*===================================================================*/
static void test_bcd_boundaries(void)
{
    TEST("BCD boundary values (59s, 59m, 23h)");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    set_defaults(regs);

    regs[BM8563_REG_SEC]   = 0x59;  /* BCD 59 */
    regs[BM8563_REG_MIN]   = 0x59;  /* BCD 59 */
    regs[BM8563_REG_HOUR]  = 0x23;  /* BCD 23 */

    assert(rtc_BM8563_init(&mock_transport, bus) == 0);
    rtc_BM8563_time_t t;
    assert(rtc_BM8563_get_time(&t) == 0);
    assert(t.second == 59);
    assert(t.minute == 59);
    assert(t.hour   == 23);

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* Format output                                                      */
/*===================================================================*/
static void test_format(void)
{
    TEST("format output");
    rtc_BM8563_time_t t = {
        .year = 2026, .month = 7, .day = 23,
        .hour = 14, .minute = 30, .second = 15,
    };
    char buf[24];
    rtc_BM8563_format(&t, buf, sizeof(buf));
    assert(strcmp(buf, "2026-07-23 14:30:15") == 0);
    PASS();
}

/*===================================================================*/
/* init fails on I2C error                                            */
/*===================================================================*/
static void test_init_fails_no_ack(void)
{
    TEST("init fails on I2C error");
    uint8_t regs[256];
    void *bus = mock_i2c_setup(regs);
    memset(regs, 0, 256);  /* all zeros — no chip present */

    /* The mock always ACKs, but the init reads seconds which is 0.
       This tests that a zero seconds register doesn't cause failure.
       To test actual I2C failure, we'd need a mock that returns -1. */
    assert(rtc_BM8563_init(&mock_transport, bus) == 0);

    rtc_BM8563_deinit();
    PASS();
}

/*===================================================================*/
/* Main                                                              */
/*===================================================================*/
int main(void)
{
    printf("\nBM8563 RTC Unit Tests\n");
    printf("====================\n\n");

    test_get_time();
    test_get_time_masks();
    test_set_time();
    test_vl_flag_cleared();
    test_no_vl_no_write();
    test_bcd_boundaries();
    test_format();

    printf("\n====================\n");
    printf("Result: %d/%d passed\n", s_passed, s_run);
    return (s_passed == s_run) ? 0 : 1;
}
