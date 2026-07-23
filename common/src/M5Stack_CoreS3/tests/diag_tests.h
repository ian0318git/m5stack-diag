/*
 * diag_tests.h — Test function declarations
 *
 * Every diagnostic test follows the diag_test_fn_t signature.
 * All functions are declared here so main.c can register them
 * without knowing their implementation.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_core.h"
#include "diag_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Global error context pointer — set by main.c before any test runs. */
extern diag_err_ctx_t *g_diag_err_ctx;

/* Test functions (each in its own compilation unit) */
diag_result_t test_i2c_scan(void *context);
diag_result_t test_screen(void *context);
diag_result_t test_touch(void *context);
diag_result_t test_rtc(void *context);
diag_result_t test_imu(void *context);
diag_result_t test_power(void *context);
diag_result_t test_backlight(void *context);
diag_result_t test_button(void *context);

/* Fugazi-style wrappers (int param signature for the menu engine) */
diag_result_t fugazi_test_i2c_scan(int param);
diag_result_t fugazi_test_screen(int param);
diag_result_t fugazi_test_touch(int param);
diag_result_t fugazi_test_rtc(int param);
diag_result_t fugazi_test_imu(int param);
diag_result_t fugazi_test_power(int param);
diag_result_t fugazi_test_backlight(int param);
diag_result_t fugazi_test_button(int param);

#ifdef __cplusplus
}
#endif
