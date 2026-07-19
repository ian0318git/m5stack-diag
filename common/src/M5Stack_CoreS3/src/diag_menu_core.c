/*
 * diag_menu_core.c — Hierarchical Menu Engine
 *
 * Builds runtime menu items from static xtables, executes tests,
 * and manages batch ("run-all") flow.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_menu_core.h"
#include <string.h>
#include <stdlib.h>

/*===========================================================================*/
/* Build / Destroy                                                           */
/*===========================================================================*/

diag_result_t diag_menu_build(diag_menu_t *menu,
                               const diag_menu_xtable_t *xtable,
                               int count, const char *title)
{
    if (!menu || !xtable || count <= 0 || count > DIAG_MENU_MAX_ITEMS) {
        return DIAG_ERROR;
    }

    diag_mitem_t *items = (diag_mitem_t *)calloc(count, sizeof(diag_mitem_t));
    if (!items) {
        return DIAG_ERROR;
    }

    for (int i = 0; i < count; i++) {
        items[i].name        = xtable[i].name;
        items[i].func        = xtable[i].func;
        items[i].param       = xtable[i].param;
        items[i].flags       = xtable[i].flags;
        items[i].last_result = -1;   /* "never run" sentinel */
        items[i].error_count = 0;
        items[i].run_count   = 0;
    }

    menu->title             = title;
    menu->items             = items;
    menu->count             = count;
    menu->total_error_count = 0;
    menu->total_run_count   = 0;

    return DIAG_PASSED;
}

void diag_menu_destroy(diag_menu_t *menu)
{
    if (menu && menu->items) {
        free(menu->items);
        menu->items = NULL;
    }
    if (menu) {
        menu->count = 0;
    }
}

/*===========================================================================*/
/* Execution                                                                 */
/*===========================================================================*/

diag_result_t diag_menu_run_item(diag_menu_t *menu, int index)
{
    if (!menu || !menu->items || index < 0 || index >= menu->count) {
        return DIAG_ERROR;
    }

    diag_mitem_t *item = &menu->items[index];

    /* Items without a function are informational headers — skip. */
    if (!item->func) {
        item->last_result = DIAG_SKIPPED;
        return DIAG_SKIPPED;
    }

    item->run_count++;
    menu->total_run_count++;

    diag_result_t result = item->func(item->param);
    item->last_result = (int)result;

    if (result != DIAG_PASSED && result != DIAG_SKIPPED) {
        item->error_count++;
        menu->total_error_count++;
    }

    return result;
}

int diag_menu_run_all(diag_menu_t *menu)
{
    if (!menu || !menu->items) return 0;

    int failures = 0;

    for (int i = 0; i < menu->count; i++) {
        diag_mitem_t *item = &menu->items[i];

        /* Skip items not flagged for "do-all". */
        if (!(item->flags & MF_DOALL)) {
            continue;
        }

        /* Skip items explicitly marked as skipped. */
        if (item->flags & MF_SKIPPED) {
            item->last_result = DIAG_SKIPPED;
            continue;
        }

        diag_result_t result = diag_menu_run_item(menu, i);
        if (result != DIAG_PASSED && result != DIAG_SKIPPED) {
            failures++;
        }

        /* If not continuous, stop on first failure. */
        if (!(item->flags & MF_CONTINUOUS) && result != DIAG_PASSED) {
            break;
        }
    }

    return failures;
}

void diag_menu_reset(diag_menu_t *menu)
{
    if (!menu || !menu->items) return;

    for (int i = 0; i < menu->count; i++) {
        menu->items[i].last_result = -1;
        menu->items[i].error_count = 0;
        menu->items[i].run_count   = 0;
    }
    menu->total_error_count = 0;
    menu->total_run_count   = 0;
}

/*===========================================================================*/
/* Lookup                                                                    */
/*===========================================================================*/

int diag_menu_find(diag_menu_t *menu, const char *name)
{
    if (!menu || !menu->items || !name) return -1;

    for (int i = 0; i < menu->count; i++) {
        if (menu->items[i].name &&
            strcmp(menu->items[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*===========================================================================*/
/* Result string                                                             */
/*===========================================================================*/

const char *diag_menu_item_result_str(const diag_mitem_t *item)
{
    if (!item || item->last_result < 0) {
        return "----";
    }
    return diag_result_str((diag_result_t)item->last_result);
}
