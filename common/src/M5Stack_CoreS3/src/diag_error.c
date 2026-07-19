/*
 * diag_error.c — Component Error Reporting System (fugazi-style cterr)
 *
 * Tracks errors by component/location and generates structured reports
 * with debug guidance.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_error.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

void diag_err_init(diag_err_ctx_t *ctx)
{
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
}

/*===========================================================================*/
/* Component tracking                                                        */
/*===========================================================================*/

void diag_err_set_component(diag_err_ctx_t *ctx,
                             const char *component, const char *location)
{
    if (!ctx) return;

    if (component) {
        snprintf(ctx->current_component, DIAG_ERR_COMP_MAX, "%s", component);
    }
    if (location) {
        snprintf(ctx->current_location, DIAG_ERR_LOC_MAX, "%s", location);
    }
}

/*===========================================================================*/
/* Find or create an error record                                            */
/*===========================================================================*/

static diag_err_record_t *find_or_add(diag_err_ctx_t *ctx)
{
    if (!ctx) return NULL;

    /* Try to find an existing record for (component, location). */
    for (int i = 0; i < ctx->num_records; i++) {
        if (strcmp(ctx->records[i].component, ctx->current_component) == 0 &&
            strcmp(ctx->records[i].location,  ctx->current_location) == 0) {
            return &ctx->records[i];
        }
    }

    /* Create a new record if space allows. */
    if (ctx->num_records >= DIAG_ERR_MAX_RECORDS) {
        return NULL;
    }

    diag_err_record_t *rec = &ctx->records[ctx->num_records];
    memset(rec, 0, sizeof(*rec));
    snprintf(rec->component, DIAG_ERR_COMP_MAX, "%s", ctx->current_component);
    snprintf(rec->location,  DIAG_ERR_LOC_MAX,  "%s", ctx->current_location);
    ctx->num_records++;

    return rec;
}

/*===========================================================================*/
/* Error recording                                                           */
/*===========================================================================*/

void diag_err_add(diag_err_ctx_t *ctx, const char *fmt, ...)
{
    if (!ctx || !fmt) return;

    diag_err_record_t *rec = find_or_add(ctx);
    if (!rec) return;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(rec->message, DIAG_ERR_MSG_MAX, fmt, ap);
    va_end(ap);

    rec->count++;
    ctx->total_error_count++;
}

void diag_err_set_debug(diag_err_ctx_t *ctx,
                         const char *msg1, const char *msg2)
{
    if (!ctx) return;

    diag_err_record_t *rec = find_or_add(ctx);
    if (!rec) return;

    if (msg1) {
        snprintf(rec->debug1, DIAG_ERR_DEBUG_MAX, "%s", msg1);
    }
    if (msg2) {
        snprintf(rec->debug2, DIAG_ERR_DEBUG_MAX, "%s", msg2);
    }
}

/*===========================================================================*/
/* Report                                                                    */
/*===========================================================================*/

void diag_err_report(const diag_err_ctx_t *ctx,
                      void (*print_fn)(const char *fmt, ...))
{
    if (!ctx || !print_fn) return;

    if (ctx->num_records == 0) {
        print_fn("  No errors recorded.\r\n");
        return;
    }

    print_fn("\r\n========== Error Report ==========\r\n");
    print_fn("  Total errors: %u\r\n", (unsigned)ctx->total_error_count);
    print_fn("  Unique failures: %d\r\n", ctx->num_records);
    print_fn("\r\n");

    for (int i = 0; i < ctx->num_records; i++) {
        const diag_err_record_t *rec = &ctx->records[i];
        print_fn("  [%d] %s / %s (x%u)\r\n",
                 i + 1,
                 rec->component,
                 rec->location,
                 (unsigned)rec->count);
        print_fn("      : %s\r\n", rec->message);

        if (rec->debug1[0]) {
            print_fn("      > %s\r\n", rec->debug1);
        }
        if (rec->debug2[0]) {
            print_fn("      > %s\r\n", rec->debug2);
        }
        print_fn("\r\n");
    }

    print_fn("==================================\r\n");
}

uint32_t diag_err_total(const diag_err_ctx_t *ctx)
{
    return ctx ? ctx->total_error_count : 0;
}

void diag_err_clear(diag_err_ctx_t *ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(*ctx));
    }
}
