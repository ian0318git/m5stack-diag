/*
 * diag_runner.h - Test scheduling and orchestration interface
 *
 * INTERFACE ADAPTER layer — mediates between the domain test entities
 * and the console menu (presenter).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/** Opaque runner instance handle. */
typedef struct diag_runner_t diag_runner_t;

/** Callback invoked after every individual test completes. */
typedef void (*diag_runner_callback_t)(const diag_test_record_t *record,
                                       void *user_data);

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Create a test runner instance.
 *
 * The runner owns a copy of the suite pointer (not a deep copy — the
 * caller must keep the suite and its test table alive for the runner's
 * lifetime).
 *
 * @param suite   Pointer to the test suite to manage.
 * @return        New runner, or NULL on allocation failure.
 */
diag_runner_t *diag_runner_create(const diag_test_suite_t *suite);

/**
 * @brief Destroy a runner and release its resources.
 * @param runner  Runner to destroy (NULL-safe).
 */
void diag_runner_destroy(diag_runner_t *runner);

/*===========================================================================*/
/* Execution                                                                 */
/*===========================================================================*/

/**
 * @brief Run a single test by its ID.
 *
 * @param runner     Runner instance.
 * @param test_id    ID of the test to run.
 * @param[out] out   Optional pointer to receive the result record.
 * @return           Test result (convenience).
 */
diag_result_t diag_runner_run_one(diag_runner_t *runner,
                                  diag_test_id_t test_id,
                                  diag_test_record_t *out);

/**
 * @brief Run all tests in the suite sequentially.
 *
 * @param runner    Runner instance.
 * @param callback  Optional per-test completion callback.
 * @param user_data  Opaque pointer passed to callback.
 * @return          Number of tests that did NOT pass (0 = all passed).
 */
int diag_runner_run_all(diag_runner_t *runner,
                        diag_runner_callback_t callback,
                        void *user_data);

/*===========================================================================*/
/* Results                                                                   */
/*===========================================================================*/

/**
 * @brief Get the result record for the most recent run of a test.
 *
 * @param runner   Runner instance.
 * @param test_id  Test identifier.
 * @return         Pointer to the record, or NULL if the test has not been run.
 */
const diag_test_record_t *diag_runner_get_record(const diag_runner_t *runner,
                                                  diag_test_id_t test_id);

/**
 * @brief Get a pointer to the managed suite.
 */
const diag_test_suite_t *diag_runner_get_suite(const diag_runner_t *runner);

/**
 * @brief Reset all stored results (but keep the test definitions).
 */
void diag_runner_reset(diag_runner_t *runner);

#ifdef __cplusplus
}
#endif
