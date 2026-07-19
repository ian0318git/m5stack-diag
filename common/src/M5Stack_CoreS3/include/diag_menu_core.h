/*
 * diag_menu_core.h — Hierarchical Menu Engine (fugazi-style)
 *
 * DOMAIN layer — pure menu entity definitions with zero I/O dependency.
 * The console presentation layer (diag_menu.c) renders these structures.
 *
 * References: example/fugazi_ng_diag/common/src/fugazi/menu.h
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "diag_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Menu Flags (matching fugazi MF_* semantics)                               */
/*===========================================================================*/

#define MF_NONE          0x00000000

/** Continue testing the next item even if this one fails. */
#define MF_CONTINUOUS    (1 << 0)

/** Treat this item as part of a group for batch execution. */
#define MF_DOGRP         (1 << 1)

/** Include this item when "run-all" / "do-all" is invoked. */
#define MF_DOALL         (1 << 2)

/** Print the accumulated error count after the test finishes. */
#define MF_SHOW_ERRCOUNT (1 << 3)

/** This entry is a sub-menu header (not directly executable). */
#define MF_SUBMENU       (1 << 4)

/** Skip this item by default (can be overridden). */
#define MF_SKIPPED       (1 << 5)

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/** Test function signature for menu items (mirrors fugazi's PFT). */
typedef diag_result_t (*diag_menu_func_t)(int param);

/**
 * @brief Extended menu table entry (submenu_xtable_t equivalent).
 *
 * Static table used to define all items in a menu.
 * The menu engine builds runtime menu items from this table.
 */
typedef struct {
    const char       *name;         /* Display name / test description    */
    diag_menu_func_t  func;         /* Test function (NULL = title only)  */
    int               param;        /* Opaque parameter to func           */
    uint32_t          flags;        /* MF_* flag bitmask                  */
    diag_menu_func_t  init_func;    /* Optional init before test (or NULL)*/
    int               init_param;   /* Parameter for init_func            */
} diag_menu_xtable_t;

/**
 * @brief Runtime menu item (mitem_t equivalent).
 *
 * Built from diag_menu_xtable_t by diag_menu_build().
 * Holds runtime state (last result, error count).
 */
typedef struct {
    const char       *name;         /* Display name                       */
    diag_menu_func_t  func;         /* Executable entry point             */
    int               param;        /* Parameter to func                  */
    uint32_t          flags;        /* MF_* flags (copied from xtable)    */
    int               last_result;  /* Result of the most recent run      */
    uint32_t          error_count;  /* Cumulative error count             */
    uint32_t          run_count;    /* How many times this item ran       */
} diag_mitem_t;

/**
 * @brief Menu definition (menuinfo_t equivalent).
 */
typedef struct {
    const char    *title;           /* Menu title / prompt                */
    diag_mitem_t  *items;           /* Array of menu items                */
    int            count;           /* Number of items                    */
    uint32_t       total_error_count; /* Error accumulator for this menu  */
    uint32_t       total_run_count;   /* Total tests executed             */
} diag_menu_t;

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

/** Maximum items per menu (prevents runaway allocation). */
#define DIAG_MENU_MAX_ITEMS    48

/** Maximum nesting depth for sub-menus. */
#define DIAG_MENU_MAX_DEPTH     8

/*===========================================================================*/
/* Menu Engine API                                                           */
/*===========================================================================*/

/**
 * @brief Build a runtime menu from a static xtable.
 *
 * Allocates and initialises the mitem_t array inside @p menu.
 * The caller must keep the xtable in scope for the menu's lifetime
 * (names are referenced, not copied).
 *
 * @param menu    Uninitialised menu structure (zeroed).
 * @param xtable  Static table of menu entries.
 * @param count   Number of entries in xtable.
 * @param title   Menu title string.
 * @return        DIAG_PASSED on success, DIAG_ERROR on allocation failure.
 */
diag_result_t diag_menu_build(diag_menu_t *menu,
                               const diag_menu_xtable_t *xtable,
                               int count, const char *title);

/**
 * @brief Release resources held by a menu.
 */
void diag_menu_destroy(diag_menu_t *menu);

/**
 * @brief Execute a single menu item by index.
 *
 * Calls the item's init_func (if set), then its func(param).
 * Updates last_result, error_count, and run_count.
 *
 * @param menu   The menu.
 * @param index  Index into menu->items[].
 * @return       Result of the test function.
 */
diag_result_t diag_menu_run_item(diag_menu_t *menu, int index);

/**
 * @brief Run all items with the MF_DOALL flag.
 *
 * @param menu   The menu.
 * @return       Number of items that did NOT pass.
 */
int diag_menu_run_all(diag_menu_t *menu);

/**
 * @brief Reset run state for all items in the menu.
 */
void diag_menu_reset(diag_menu_t *menu);

/**
 * @brief Look up an item index by name (case-sensitive).
 * @return Index, or -1 if not found.
 */
int diag_menu_find(diag_menu_t *menu, const char *name);

/**
 * @brief Get a printable summary line for an item's last result.
 */
const char *diag_menu_item_result_str(const diag_mitem_t *item);

#ifdef __cplusplus
}
#endif
