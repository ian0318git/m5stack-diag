/*
 * fugazi_wrappers.c — Fugazi-style wrappers (int param signature)
 *
 * These wrappers bridge between the fugazi menu engine which expects
 * a (int param) signature and the standard test functions.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_tests.h"

diag_result_t fugazi_test_i2c_scan(int param)  { (void)param; return test_i2c_scan(NULL); }
diag_result_t fugazi_test_screen(int param)    { (void)param; return test_screen(NULL); }
diag_result_t fugazi_test_touch(int param)     { (void)param; return test_touch(NULL); }
diag_result_t fugazi_test_rtc(int param)       { (void)param; return test_rtc(NULL); }
diag_result_t fugazi_test_imu(int param)       { (void)param; return test_imu(NULL); }
diag_result_t fugazi_test_power(int param)     { (void)param; return test_power(NULL); }
diag_result_t fugazi_test_backlight(int param)  { (void)param; return test_backlight(NULL); }
diag_result_t fugazi_test_speaker(int param)    { (void)param; return test_speaker(NULL); }
diag_result_t fugazi_test_microphone(int param) { (void)param; return test_microphone(NULL); }
diag_result_t fugazi_test_button(int param)     { (void)param; return test_button(NULL); }
