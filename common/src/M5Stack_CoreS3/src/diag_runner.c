/*
 * diag_runner.c - Test scheduling and orchestration
 *
 * INTERFACE ADAPTER layer.  Orchestrates test execution, measures
 * elapsed time, and collects results.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_runner.h"
#include "diag_config.h"
#include <string.h>
#include <stdlib.h>

/*===========================================================================*/
/* Internal structure                                                        */
/*===========================================================================*/

struct diag_runner_t {
    const diag_test_suite_t *suite;
    diag_test_record_t       records[DIAG_TEST_COUNT];
    bool                     has_run[DIAG_TEST_COUNT];
};

/*===========================================================================*/
/* Helper — wall-clock millisecond timer (ESP-IDF)                           */
/*===========================================================================*/

static int64_t s_start_time;  /* μs timestamp from esp_timer_get_time */

static inline void timer_start(void)
{
    /* Injected at compile time: link against esp_timer component. */
    extern int64_t esp_timer_get_time(void);
    s_start_time = esp_timer_get_time();
}

static inline uint32_t timer_elapsed_ms(void)
{
    extern int64_t esp_timer_get_time(void);
    return (uint32_t)((esp_timer_get_time() - s_start_time) / 1000);
}

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

diag_runner_t *diag_runner_create(const diag_test_suite_t *suite)
{
    if (!suite || !suite->tests || suite->count == 0) {
        return NULL;
    }

    diag_runner_t *r = (diag_runner_t *)calloc(1, sizeof(diag_runner_t));
    if (!r) {
        return NULL;
    }

    r->suite = suite;
    memset(r->has_run, 0, sizeof(r->has_run));
    return r;
}

void diag_runner_destroy(diag_runner_t *runner)
{
    free(runner);
}

/*===========================================================================*/
/* Execution                                                                 */
/*===========================================================================*/

diag_result_t diag_runner_run_one(diag_runner_t *runner,
                                  diag_test_id_t test_id,
                                  diag_test_record_t *out)
{
    if (!runner || !runner->suite) {
        return DIAG_ERROR;
    }

    /* Find the test entry by ID. */
    const diag_test_t *test = NULL;
    for (size_t i = 0; i < runner->suite->count; i++) {
        if (runner->suite->tests[i].id == test_id) {
            test = &runner->suite->tests[i];
            break;
        }
    }
    if (!test) {
        return DIAG_ERROR;
    }

    diag_test_record_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.test_id = test_id;

    uint32_t timeout = test->timeout_ms;
    if (timeout == 0) {
        timeout = CONFIG_DEFAULT_TEST_TIMEOUT_MS;
    }

    timer_start();
    diag_result_t result = DIAG_ERROR;

    /* Call the test function — the runner catches only a NULL function. */
    if (test->run) {
        result = test->run(test->context);
    } else {
        result = DIAG_SKIPPED;
        snprintf(rec.message, sizeof(rec.message),
                 "Test has no function assigned");
    }

    rec.elapsed_ms = timer_elapsed_ms();
    rec.result = result;

    /* If the test did not fill in a message, add a default one. */
    if (rec.message[0] == '\0') {
        snprintf(rec.message, sizeof(rec.message), "%s",
                 diag_result_str(result));
    }

    /* Store in the runner's record table. */
    if (test_id >= 0 && test_id < DIAG_TEST_COUNT) {
        runner->records[test_id] = rec;
        runner->has_run[test_id] = true;
    }

    if (out) {
        *out = rec;
    }

    return result;
}

int diag_runner_run_all(diag_runner_t *runner,
                        diag_runner_callback_t callback,
                        void *user_data)
{
    if (!runner || !runner->suite) {
        return -1;
    }

    int failures = 0;

    for (size_t i = 0; i < runner->suite->count; i++) {
        const diag_test_id_t id = runner->suite->tests[i].id;

        diag_test_record_t rec;
        diag_result_t result = diag_runner_run_one(runner, id, &rec);

        if (result != DIAG_PASSED && result != DIAG_SKIPPED) {
            failures++;
        }

        if (callback) {
            callback(&rec, user_data);
        }
    }

    return failures;
}

/*===========================================================================*/
/* Results                                                                   */
/*===========================================================================*/

const diag_test_record_t *diag_runner_get_record(const diag_runner_t *runner,
                                                  diag_test_id_t test_id)
{
    if (!runner || test_id >= DIAG_TEST_COUNT || !runner->has_run[test_id]) {
        return NULL;
    }
    return &runner->records[test_id];
}

const diag_test_suite_t *diag_runner_get_suite(const diag_runner_t *runner)
{
    return runner ? runner->suite : NULL;
}

void diag_runner_reset(diag_runner_t *runner)
{
    if (runner) {
        memset(runner->has_run, 0, sizeof(runner->has_run));
        memset(runner->records, 0, sizeof(runner->records));
    }
}

/*===========================================================================*/
/* Result string                                                             */
/*===========================================================================*/

const char *diag_result_str(diag_result_t r)
{
    switch (r) {
    case DIAG_PASSED:  return "PASSED";
    case DIAG_SKIPPED: return "SKIPPED";
    case DIAG_FAILED:  return "FAILED";
    case DIAG_ERROR:   return "ERROR";
    case DIAG_TIMEOUT: return "TIMEOUT";
    default:           return "UNKNOWN";
    }
}
