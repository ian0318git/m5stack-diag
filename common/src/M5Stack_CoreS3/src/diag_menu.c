/*
 * diag_menu.c - UART Console Menu Implementation
 *
 * INTERFACE ADAPTER layer.  Presents an interactive command-line menu
 * over UART, dispatches user commands to the test runner, and formats
 * test results for display.
 *
 * References: example/fugazi_ng_diag/common/src/fugazi/mb_tests.c
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_menu.h"
#include "diag_config.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

/*===========================================================================*/
/* UART driver wrapper (ESP-IDF)                                             */
/*===========================================================================*/

/*
 * We use ESP-IDF's UART driver directly.  The menu layer is the only
 * code that talks to the UART peripheral; every other layer prints via
 * diag_menu_printf().
 */
#include "driver/uart.h"
#include "driver/gpio.h"

static const uart_port_t s_uart_num = CONFIG_UART_NUM;

static diag_result_t uart_init(void)
{
    const uart_config_t cfg = {
        .baud_rate           = CONFIG_UART_BAUDRATE,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .source_clk          = UART_SCLK_DEFAULT,
    };

    esp_err_t err = uart_driver_install(s_uart_num,
                                        CONFIG_UART_BUF_SIZE,
                                        0, 0, NULL, 0);
    if (err != ESP_OK) return DIAG_FAILED;

    err = uart_param_config(s_uart_num, &cfg);
    if (err != ESP_OK) return DIAG_FAILED;

    err = uart_set_pin(s_uart_num,
                       CONFIG_UART_TX_PIN,
                       CONFIG_UART_RX_PIN,
                       UART_PIN_NO_CHANGE,
                       UART_PIN_NO_CHANGE);
    if (err != ESP_OK) return DIAG_FAILED;

    return DIAG_PASSED;
}

static void uart_putchar(char c)
{
    uart_write_bytes(s_uart_num, &c, 1);
}

static void uart_puts(const char *s)
{
    uart_write_bytes(s_uart_num, s, strlen(s));
}

static int uart_getchar(int timeout_ms)
{
    uint8_t c;
    int len = uart_read_bytes(s_uart_num, &c, 1,
                              pdMS_TO_TICKS(timeout_ms));
    return (len == 1) ? (int)c : -1;
}

static void uart_flush_rx(void)
{
    uart_flush_input(s_uart_num);
}

/*===========================================================================*/
/* Command table                                                             */
/*===========================================================================*/

#define MENU_MAX_CMDS  32

static diag_menu_cmd_t s_commands[MENU_MAX_CMDS];
static int             s_num_commands = 0;

diag_result_t diag_menu_register_cmd(const diag_menu_cmd_t *cmd)
{
    if (s_num_commands >= MENU_MAX_CMDS || !cmd || !cmd->name) {
        return DIAG_FAILED;
    }
    s_commands[s_num_commands++] = *cmd;
    return DIAG_PASSED;
}

/*===========================================================================*/
/* I/O helpers                                                               */
/*===========================================================================*/

void diag_menu_printf(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n > 0) {
        uart_puts(buf);
    }
}

void diag_menu_print_result(const char *test_name, diag_result_t result)
{
    const char *s = diag_result_str(result);
    diag_menu_printf("[ %-7s ] %s\r\n", s, test_name);
}

void diag_menu_print_record(const diag_test_record_t *rec)
{
    if (!rec) return;

    diag_menu_printf("[ %-7s ] %s  (%u ms)\r\n",
                     diag_result_str(rec->result),
                     rec->message,
                     (unsigned)rec->elapsed_ms);
}

/*===========================================================================*/
/* Line input (minimal line-editing)                                         */
/*===========================================================================*/

static int read_line(char *buf, size_t size)
{
    size_t pos = 0;
    int c;

    while (pos < size - 1) {
        c = uart_getchar(CONFIG_UART_RX_TIMEOUT_MS);
        if (c < 0) {
            continue;   /* timeout — keep waiting */
        }

        if (c == '\r' || c == '\n') {
            uart_puts("\r\n");
            buf[pos] = '\0';
            return (int)pos;
        }

        if (c == '\b' || c == 0x7F) {  /* backspace / DEL */
            if (pos > 0) {
                pos--;
                uart_puts("\b \b");     /* erase on terminal */
            }
            continue;
        }

        if (c >= ' ' && c <= '~') {     /* printable ASCII */
            buf[pos++] = (char)c;
            uart_putchar((char)c);
        }
    }

    /* Buffer full — discard rest of line */
    buf[size - 1] = '\0';
    while ((c = uart_getchar(50)) >= 0 && c != '\r' && c != '\n') {
        /* drain */ ;
    }
    uart_puts("\r\n");
    return (int)(size - 1);
}

/*===========================================================================*/
/* Argument parsing                                                          */
/*===========================================================================*/

static int tokenise(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p) {
        /* Skip leading whitespace */
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        if (argc >= max_args) break;

        /* Extract one token */
        if (*p == '"') {
            /* Quoted string */
            p++;
            argv[argc] = p;
            while (*p && *p != '"') p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        } else {
            argv[argc] = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        }
        argc++;
    }

    return argc;
}

/*===========================================================================*/
/* Built-in command handlers                                                 */
/*===========================================================================*/

static diag_result_t cmd_help(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    (void)runner;

    diag_menu_printf("\r\nAvailable commands:\r\n");
    for (int i = 0; i < s_num_commands; i++) {
        diag_menu_printf("  %-12s %s\r\n",
                         s_commands[i].name,
                         s_commands[i].help ? s_commands[i].help : "");
    }
    return DIAG_PASSED;
}

static diag_result_t cmd_info(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    const diag_test_suite_t *suite = diag_runner_get_suite(runner);
    if (!suite) {
        diag_menu_printf("No test suite loaded.\r\n");
        return DIAG_ERROR;
    }

    diag_menu_printf("\r\nTest Suite: %s\r\n", suite->name);
    diag_menu_printf("  %u tests registered\r\n", (unsigned)suite->count);
    diag_menu_printf("\r\nTests:\r\n");

    for (size_t i = 0; i < suite->count; i++) {
        const diag_test_t *t = &suite->tests[i];
        const diag_test_record_t *rec = diag_runner_get_record(runner, t->id);

        diag_menu_printf("  [%2u] %-32s ", (unsigned)i, t->name);
        if (rec) {
            diag_menu_printf("%s (%u ms)",
                             diag_result_str(rec->result),
                             (unsigned)rec->elapsed_ms);
        } else {
            diag_menu_printf("-- not run --");
        }
        diag_menu_printf("\r\n");
    }

    return DIAG_PASSED;
}

static diag_result_t cmd_run(diag_runner_t *runner, int argc, char *argv[])
{
    const diag_test_suite_t *suite = diag_runner_get_suite(runner);
    if (!suite) return DIAG_ERROR;

    if (argc < 2) {
        diag_menu_printf("Usage: run <test-name|#>\r\n");
        diag_menu_printf("  Use 'info' to list tests by name and number.\r\n");
        return DIAG_FAILED;
    }

    /* Try to match by numeric index first, then by name. */
    int idx = -1;
    char *end = NULL;
    long n = strtol(argv[1], &end, 10);
    if (end && *end == '\0') {
        idx = (int)n;   /* numeric match */
    }

    const diag_test_t *test = NULL;
    for (size_t i = 0; i < suite->count; i++) {
        if (idx == (int)i) {
            test = &suite->tests[i];
            break;
        }
        if (strcmp(suite->tests[i].name, argv[1]) == 0) {
            test = &suite->tests[i];
            break;
        }
    }

    if (!test) {
        diag_menu_printf("Unknown test: %s\r\n", argv[1]);
        return DIAG_FAILED;
    }

    diag_menu_printf("Running: %s...\r\n", test->name);

    diag_test_record_t rec;
    diag_runner_run_one(runner, test->id, &rec);
    diag_menu_print_record(&rec);

    return rec.result;
}

static diag_result_t cmd_run_all(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    diag_menu_printf("Running ALL tests...\r\n\n");

    /* Per-test completion prints to UART. */
    int failures = diag_runner_run_all(runner, NULL, NULL);

    /* Print summary. */
    const diag_test_suite_t *suite = diag_runner_get_suite(runner);
    if (!suite) return DIAG_ERROR;

    diag_menu_printf("\r\n========== Summary ==========\r\n");
    int passed = 0, skipped = 0, failed = 0;
    for (size_t i = 0; i < suite->count; i++) {
        const diag_test_record_t *rec =
            diag_runner_get_record(runner, suite->tests[i].id);
        if (!rec) continue;

        diag_menu_print_record(rec);
        switch (rec->result) {
        case DIAG_PASSED:  passed++;  break;
        case DIAG_SKIPPED: skipped++; break;
        default:           failed++;  break;
        }
    }

    diag_menu_printf("\r\n%d passed, %d failed, %d skipped\r\n",
                     passed, failed, skipped);
    return (failures == 0) ? DIAG_PASSED : DIAG_FAILED;
}

static diag_result_t cmd_reset(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    diag_runner_reset(runner);
    diag_menu_printf("Results cleared.\r\n");
    return DIAG_PASSED;
}

static diag_result_t cmd_help_verbose(diag_runner_t *runner,
                                       int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    (void)runner;

    diag_menu_printf("\r\n");
    diag_menu_printf("M5Stack CoreS3 Diagnostic System\r\n");
    diag_menu_printf("===============================\r\n");
    diag_menu_printf("\r\n");
    diag_menu_printf("Built-in commands:\r\n");
    diag_menu_printf("  help            Show this message\r\n");
    diag_menu_printf("  info            List all tests and their status\r\n");
    diag_menu_printf("  run <name|#>    Run a single test by name or index\r\n");
    diag_menu_printf("  run-all         Execute every test in sequence\r\n");
    diag_menu_printf("  reset           Clear all stored results\r\n");
    diag_menu_printf("  exit            Exit the menu (returns to monitor)\r\n");
    diag_menu_printf("\r\n");
    diag_menu_printf("Navigation:\r\n");
    diag_menu_printf("  Commands are case-sensitive.  Use TAB completion\r\n");
    diag_menu_printf("  is not yet implemented — type full command names.\r\n");
    diag_menu_printf("\r\n");

    return DIAG_PASSED;
}

/*===========================================================================*/
/* Built-in command table                                                    */
/*===========================================================================*/

static const diag_menu_cmd_t s_builtin_cmds[] = {
    { "help",    "Show this help message",        cmd_help_verbose },
    { "info",    "List all tests and status",     cmd_info         },
    { "run",     "Run <name|#> once",             cmd_run          },
    { "run-all", "Execute all tests sequentially", cmd_run_all      },
    { "reset",   "Clear stored results",          cmd_reset        },
};

#define NUM_BUILTIN_CMDS (sizeof(s_builtin_cmds) / sizeof(s_builtin_cmds[0]))

static void register_builtins(void)
{
    for (size_t i = 0; i < NUM_BUILTIN_CMDS; i++) {
        diag_menu_register_cmd(&s_builtin_cmds[i]);
    }
}

/*===========================================================================*/
/* Command dispatch                                                          */
/*===========================================================================*/

static diag_result_t dispatch(diag_runner_t *runner, int argc, char *argv[])
{
    if (argc == 0) {
        return DIAG_SKIPPED;   /* empty line */
    }

    /* Check for exit / quit */
    if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) {
        diag_menu_printf("Bye.\r\n");
        return DIAG_PASSED;   /* sentinel: caller breaks loop */
    }

    for (int i = 0; i < s_num_commands; i++) {
        if (strcmp(argv[0], s_commands[i].name) == 0) {
            return s_commands[i].handler(runner, argc, argv);
        }
    }

    diag_menu_printf("Unknown command: %s  (type 'help')\r\n", argv[0]);
    return DIAG_FAILED;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

diag_result_t diag_menu_init(void)
{
    diag_result_t r = uart_init();
    if (r != DIAG_PASSED) {
        return r;
    }

    register_builtins();
    return DIAG_PASSED;
}

void diag_menu_loop(diag_runner_t *runner)
{
    if (!runner) return;

    uart_puts("\r\n");
    uart_puts("============================================\r\n");
    uart_puts("   M5Stack CoreS3 Diagnostic System v1.0\r\n");
    uart_puts("============================================\r\n");
    uart_puts("Type 'help' for available commands.\r\n");
    uart_puts("\r\n");

    char line[CONFIG_MENU_LINE_BUF_SIZE];
    char *argv[CONFIG_MENU_MAX_ARGS];

    while (1) {
        uart_puts(CONFIG_MENU_PROMPT);
        uart_flush_rx();

        int len = read_line(line, sizeof(line));
        if (len < 0) continue;

        int argc = tokenise(line, argv, CONFIG_MENU_MAX_ARGS);
        if (argc == 0) continue;

        diag_result_t status = dispatch(runner, argc, argv);

        /* 'exit' or 'quit' returns DIAG_PASSED; break on that sentinel. */
        if (status == DIAG_PASSED &&
            (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0)) {
            break;
        }
    }
}
