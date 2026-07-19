/*
 * diag_error.h — Component Error Reporting System (fugazi-style cterr)
 *
 * DOMAIN layer — tracks errors by component/location and provides
 * debug guidance, mirroring the fugazi cterr infrastructure.
 *
 * References: example/fugazi_ng_diag/common/src/fugazi/error.h
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

#define DIAG_ERR_COMP_MAX    32   /* Max component name length            */
#define DIAG_ERR_LOC_MAX     32   /* Max location string length           */
#define DIAG_ERR_MSG_MAX     256  /* Max error message length             */
#define DIAG_ERR_DEBUG_MAX   128  /* Max debug suggestion length          */
#define DIAG_ERR_MAX_RECORDS 32   /* Max number of stored error records   */

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/**
 * @brief A single error record (fugazi cterr equivalent).
 *
 * Each call to diag_err_add() creates or updates a record identified
 * by (component, location).
 */
typedef struct {
    char  component[DIAG_ERR_COMP_MAX];   /* e.g. "I2C", "TOUCH"        */
    char  location[DIAG_ERR_LOC_MAX];     /* e.g. "MB/I2C"              */
    char  message[DIAG_ERR_MSG_MAX];      /* Formatted error message    */
    char  debug1[DIAG_ERR_DEBUG_MAX];     /* Debug suggestion #1        */
    char  debug2[DIAG_ERR_DEBUG_MAX];     /* Debug suggestion #2        */
    uint32_t count;                        /* How many times this fired  */
} diag_err_record_t;

/**
 * @brief Error reporting context.
 *
 * Tracks all error records for one test session.
 */
typedef struct {
    diag_err_record_t records[DIAG_ERR_MAX_RECORDS];
    int               num_records;
    char              current_component[DIAG_ERR_COMP_MAX];
    char              current_location[DIAG_ERR_LOC_MAX];
    uint32_t          total_error_count;
} diag_err_ctx_t;

/*===========================================================================*/
/* API                                                                       */
/*===========================================================================*/

/**
 * @brief Initialise the error context (call once per session).
 */
void diag_err_init(diag_err_ctx_t *ctx);

/**
 * @brief Set the current component/location for subsequent error adds.
 *
 * Equivalent to fugazi's cterr_add_component().
 */
void diag_err_set_component(diag_err_ctx_t *ctx,
                             const char *component, const char *location);

/**
 * @brief Add an error for the current component.
 *
 * Equivalent to fugazi's cterr() — logs a formatted error message.
 * If an identical (component, location) record exists, its count is
 * incremented rather than creating a duplicate.
 *
 * @param ctx  Error context.
 * @param fmt  printf-style format string.
 * @param ...  Format arguments.
 */
void diag_err_add(diag_err_ctx_t *ctx, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * @brief Set debug suggestions for the current component.
 *
 * Equivalent to fugazi's cterr_add_debug().
 *
 * @param ctx  Error context.
 * @param msg1 First debug suggestion (or NULL).
 * @param msg2 Second debug suggestion (or NULL).
 */
void diag_err_set_debug(diag_err_ctx_t *ctx,
                         const char *msg1, const char *msg2);

/**
 * @brief Print a full error report to the console presenter.
 *
 * @param ctx      Error context.
 * @param print_fn  Callback that outputs a string (e.g. diag_menu_printf).
 */
void diag_err_report(const diag_err_ctx_t *ctx,
                      void (*print_fn)(const char *fmt, ...));

/**
 * @brief Get total error count across all records.
 */
uint32_t diag_err_total(const diag_err_ctx_t *ctx);

/**
 * @brief Clear all error records (but keep the context).
 */
void diag_err_clear(diag_err_ctx_t *ctx);

#ifdef __cplusplus
}
#endif
