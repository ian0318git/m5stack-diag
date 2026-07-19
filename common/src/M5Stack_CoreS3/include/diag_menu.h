/*
 * diag_menu.h - UART Console Menu Interface
 *
 * INTERFACE ADAPTER layer — implements the Console presenter/controller.
 * Reads user input from UART, dispatches commands to the runner, and
 * formats output for the terminal.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_runner.h"
#include "diag_menu_core.h"
#include "diag_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/** Menu command handler signature. */
typedef diag_result_t (*diag_menu_handler_t)(diag_runner_t *runner,
                                              int argc, char *argv[]);

/** Command table entry. */
typedef struct {
    const char            *name;        /* Command name (e.g. "run")    */
    const char            *help;        /* One-line help text           */
    diag_menu_handler_t    handler;     /* Handler function             */
} diag_menu_cmd_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

/**
 * @brief Initialise the UART console.
 *
 * Configures UART1 with the settings from diag_config.h.
 *
 * @return DIAG_PASSED on success.
 */
diag_result_t diag_menu_init(void);

/**
 * @brief Enter the main menu loop.
 *
 * Reads commands from UART, dispatches to registered handlers, and
 * loops until the user issues the "exit" command.  This function
 * blocks indefinitely.
 *
 * @param runner  Initialised runner instance.
 */
void diag_menu_loop(diag_runner_t *runner);

/*===========================================================================*/
/* I/O helpers (available to command handlers)                                */
/*===========================================================================*/

/**
 * @brief Print a formatted string to the UART console.
 */
void diag_menu_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief Print a test record in a standard format.
 */
void diag_menu_print_record(const diag_test_record_t *rec);

/**
 * @brief Print the standard result banner for a test.
 */
void diag_menu_print_result(const char *test_name, diag_result_t result);

/**
 * @brief Register a command in the menu.
 * @return DIAG_PASSED on success, DIAG_FAILED if table full.
 */
diag_result_t diag_menu_register_cmd(const diag_menu_cmd_t *cmd);

/**
 * @brief Register the fugazi-style menu + error context for the 'menu'
 *        and 'errors' commands.
 */
void diag_menu_set_fugazi(diag_menu_t *menu, diag_err_ctx_t *err_ctx);

#ifdef __cplusplus
}
#endif
