/*
 * diag_core.h - Core types, enums, and test entity definitions
 *
 * This file belongs to the DOMAIN layer (innermost) of Clean Architecture.
 * It has ZERO dependencies on hardware, ESP-IDF, or external frameworks.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Result / Status Types                                                     */
/*===========================================================================*/

/**
 * @brief Standard diagnostic test result
 *
 * Ordered from best to worst for aggregation logic.
 */
typedef enum {
    DIAG_PASSED  = 0,
    DIAG_SKIPPED = 1,
    DIAG_FAILED  = 2,
    DIAG_ERROR   = 3,   /* Internal error (e.g. test harness failure) */
    DIAG_TIMEOUT = 4,   /* Test did not complete within expected time */
} diag_result_t;

/**
 * @brief Human-readable string for a result value
 * @param r  Result value
 * @return   Pointer to static string (never NULL)
 */
const char *diag_result_str(diag_result_t r);

/*===========================================================================*/
/* Test Entity                                                               */
/*===========================================================================*/

/** Maximum length of a test name / description string */
#define DIAG_TEST_NAME_MAX    48
#define DIAG_TEST_DESC_MAX    128

/**
 * @brief Unique identifier for each known test.
 *
 * Add new IDs here when adding tests.  IDs remain stable once assigned
 * so they can be used in persistent skip-lists or logs.
 */
typedef enum {
    DIAG_TEST_I2C_SCAN   = 0,
    DIAG_TEST_SCREEN     = 1,
    DIAG_TEST_TOUCH      = 2,
    DIAG_TEST_RTC        = 3,
    DIAG_TEST_IMU        = 4,
    DIAG_TEST_POWER      = 5,
    DIAG_TEST_BACKLIGHT  = 6,
    DIAG_TEST_SPEAKER    = 7,
    DIAG_TEST_MICROPHONE = 8,
    DIAG_TEST_CAMERA     = 9,
    DIAG_TEST_PROXIMITY  = 10,
    DIAG_TEST_BUTTON     = 11,
    /* --- add new IDs above this line --- */
    DIAG_TEST_COUNT
} diag_test_id_t;

/**
 * @brief Test function signature.
 *
 * Every diagnostic test follows this contract:
 *   - Returns a diag_result_t.
 *   - Receives an opaque context pointer (may be NULL).
 *   - Must be re-entrant only if explicitly documented.
 */
typedef diag_result_t (*diag_test_fn_t)(void *context);

/**
 * @brief Test entity — the core domain object.
 *
 * This struct bundles everything the runner and menu need to know about
 * a single test WITHOUT coupling them to any concrete hardware or UI.
 */
typedef struct {
    diag_test_id_t  id;          /* Unique identifier                  */
    const char     *name;        /* Short display name                 */
    const char     *description; /* Longer description / help text     */
    diag_test_fn_t  run;         /* Entry point for execution          */
    void           *context;     /* Opaque pointer passed to run()     */
    uint32_t        timeout_ms;  /* Max wall-clock time, 0 = default   */
} diag_test_t;

/*===========================================================================*/
/* Test Result Record                                                        */
/*===========================================================================*/

/** Maximum length of a diagnostic message string */
#define DIAG_MSG_MAX      256

/**
 * @brief Record produced when a test finishes.
 *
 * The runner collects these into a result set that the menu (presenter)
 * displays to the user.
 */
typedef struct {
    diag_test_id_t  test_id;
    diag_result_t   result;
    uint32_t        elapsed_ms;   /* Measured execution time             */
    char            message[DIAG_MSG_MAX]; /* Free-text result message */
} diag_test_record_t;

/*===========================================================================*/
/* Test Suite                                                                */
/*===========================================================================*/

/**
 * @brief A collection of tests that can be run in sequence.
 *
 * Suites can be nested or flat; the runner iterates the table.
 */
typedef struct {
    const char     *name;         /* Suite name (e.g. "I2C Peripherals") */
    const diag_test_t *tests;     /* Pointer to array of test entries    */
    size_t           count;       /* Number of entries in tests[]        */
} diag_test_suite_t;

/*===========================================================================*/
/* Utility macros                                                            */
/*===========================================================================*/

/** Compute the number of elements in a static array. */
#define DIAG_ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))

/**
 * @brief Declare a test entry for a static test table.
 *
 * Usage:
 *   static const diag_test_t my_tests[] = {
 *       DIAG_TEST_ENTRY(DIAG_TEST_I2C_SCAN, "I2C Scan", i2c_scan_test, NULL),
 *       ...
 *   };
 */
#define DIAG_TEST_ENTRY(id_, name_, desc_, fn_, ctx_, timeout_) \
    { .id = (id_),                                                \
      .name = (name_),                                            \
      .description = (desc_),                                     \
      .run = (fn_),                                                \
      .context = (ctx_),                                           \
      .timeout_ms = (timeout_) }

#ifdef __cplusplus
}
#endif
