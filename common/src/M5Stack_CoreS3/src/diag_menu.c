/*
 * diag_menu.c - USB Serial/JTAG Console Menu Implementation
 *
 * INTERFACE ADAPTER layer.  Presents an interactive command-line menu
 * over the ESP32-S3 built-in USB Serial/JTAG (visible as /dev/ttyACM0).
 * Dispatches user commands to the test runner and formats test results.
 *
 * References: example/fugazi_ng_diag/common/src/fugazi/mb_tests.c
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "diag_menu.h"
#include "diag_config.h"
#include "diag_menu_core.h"
#include "diag_error.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"

/*===========================================================================*/
/* USB Serial/JTAG driver wrapper (ESP-IDF)                                  */
/*===========================================================================*/

/*
 * CoreS3 uses the ESP32-S3 built-in USB Serial/JTAG controller.
 * No GPIO configuration needed — it is fixed-function hardware.
 */
#include "driver/usb_serial_jtag.h"

static bool s_console_initialised = false;

static diag_result_t console_init(void)
{
    if (s_console_initialised) return DIAG_PASSED;

    usb_serial_jtag_driver_config_t cfg = {
        .tx_buffer_size = CONFIG_CONSOLE_BUF_SIZE,
        .rx_buffer_size = CONFIG_CONSOLE_BUF_SIZE,
    };

    esp_err_t err = usb_serial_jtag_driver_install(&cfg);
    if (err != ESP_OK) {
        return DIAG_FAILED;
    }

    s_console_initialised = true;
    return DIAG_PASSED;
}

static void console_putchar(char c)
{
    usb_serial_jtag_write_bytes(&c, 1, CONFIG_CONSOLE_TX_TIMEOUT_MS);
}

static void console_puts(const char *s)
{
    if (!s) return;
    size_t len = strlen(s);
    if (len > 0) {
        usb_serial_jtag_write_bytes(s, len, CONFIG_CONSOLE_TX_TIMEOUT_MS);
    }
}

static int console_getchar(int timeout_ms)
{
    uint8_t c;
    int len = usb_serial_jtag_read_bytes(&c, 1, pdMS_TO_TICKS(timeout_ms));
    return (len == 1) ? (int)c : -1;
}

static void console_flush_rx(void)
{
    /* Drain any pending input bytes */
    uint8_t tmp[64];
    while (usb_serial_jtag_read_bytes(tmp, sizeof(tmp), 0) > 0) {
        /* discard */ ;
    }
}

/*===========================================================================*/
/* Command table                                                             */
/*===========================================================================*/

#define MENU_MAX_CMDS  32

static diag_menu_cmd_t s_commands[MENU_MAX_CMDS];
static int             s_num_commands = 0;

/* Fugazi-style menu engine + error context (set by main) */
static diag_menu_t    *s_fugazi_menu = NULL;
static diag_err_ctx_t *s_err_ctx     = NULL;

void diag_menu_set_fugazi(diag_menu_t *menu, diag_err_ctx_t *err_ctx)
{
    s_fugazi_menu = menu;
    s_err_ctx     = err_ctx;
}

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
        console_puts(buf);
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
        c = console_getchar(CONFIG_CONSOLE_RX_TIMEOUT_MS);
        if (c < 0) {
            continue;   /* timeout — keep waiting */
        }

        if (c == '\r' || c == '\n') {
            console_puts("\r\n");
            buf[pos] = '\0';
            return (int)pos;
        }

        if (c == '\b' || c == 0x7F) {  /* backspace / DEL */
            if (pos > 0) {
                pos--;
                console_puts("\b \b");   /* erase on terminal */
            }
            continue;
        }

        if (c >= ' ' && c <= '~') {     /* printable ASCII */
            buf[pos++] = (char)c;
            console_putchar((char)c);
        }
    }

    /* Buffer full — discard rest of line */
    buf[size - 1] = '\0';
    while ((c = console_getchar(50)) >= 0 && c != '\r' && c != '\n') {
        /* drain */ ;
    }
    console_puts("\r\n");
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

    int failures = diag_runner_run_all(runner, NULL, NULL);

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
    diag_menu_printf("  menu            Interactive fugazi-style number menu\r\n");
    diag_menu_printf("  errors          Show error report\r\n");
    diag_menu_printf("  reset           Clear all stored results\r\n");
    diag_menu_printf("  exit            Exit the menu (returns to monitor)\r\n");
    diag_menu_printf("\r\n");
    diag_menu_printf("Extended commands:\r\n");
    diag_menu_printf("  status          Show system status\r\n");
    diag_menu_printf("  rtc-set YYYY MM DD HH MM SS   Set RTC time\r\n");
    diag_menu_printf("  screen-on|off    Turn display on/off\r\n");
    diag_menu_printf("  reboot          Software reset\r\n");
    diag_menu_printf("  shutdown        Power off\r\n");
    diag_menu_printf("\r\n");
    diag_menu_printf("Navigation:\r\n");
    diag_menu_printf("  Commands are case-sensitive.\r\n");
    diag_menu_printf("  In 'menu' mode, type a number + Enter to run a test.\r\n");
    diag_menu_printf("\r\n");

    return DIAG_PASSED;
}

/*===========================================================================*/
/* Fugazi-style interactive menu handler                                     */
/*===========================================================================*/

static diag_result_t cmd_menu(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argv;
    (void)argc;
    (void)runner;

    if (!s_fugazi_menu) {
        diag_menu_printf("No fugazi-style menu registered.\r\n");
        return DIAG_ERROR;
    }

    diag_menu_t *m = s_fugazi_menu;
    char line[32];

    while (1) {
        /* ---- Title ---- */
        diag_menu_printf("\r\n========== %s ==========\r\n", m->title);
        diag_menu_printf("  Errors: %u  |  Run: %u\r\n",
                         (unsigned)m->total_error_count,
                         (unsigned)m->total_run_count);

        /* ---- List items ---- */
        for (int i = 0; i < m->count; i++) {
            const diag_mitem_t *item = &m->items[i];
            const char *status = diag_menu_item_result_str(item);
            char marker = ' ';

            if (item->flags & MF_SKIPPED)      marker = 's';
            else if (item->flags & MF_SUBMENU) marker = '>';
            else if (item->flags & MF_DOALL)   marker = '*';

            diag_menu_printf("  %c[%2d] %-32s %s\r\n",
                             marker, i + 1, item->name, status);

            if (item->error_count > 0) {
                diag_menu_printf("       errors: %u\r\n",
                                 (unsigned)item->error_count);
            }
        }

        /* ---- Prompt ---- */
        diag_menu_printf("\r\n  [a]ll  [r]eset  [e]rrors  [q]uit\r\n");
        diag_menu_printf("  Select test #: ");
        console_flush_rx();

        /* Read one line */
        int len = read_line(line, sizeof(line));
        if (len <= 0) continue;

        /* Parse command */
        if (strcmp(line, "q") == 0 || strcmp(line, "quit") == 0) break;

        if (strcmp(line, "a") == 0 || strcmp(line, "all") == 0) {
            diag_menu_printf("Running all tests...\r\n");
            int failures = diag_menu_run_all(m);
            diag_menu_printf("Done: %d failures.\r\n", failures);
            continue;
        }

        if (strcmp(line, "r") == 0 || strcmp(line, "reset") == 0) {
            diag_menu_reset(m);
            diag_err_clear(s_err_ctx);
            diag_menu_printf("Results and errors cleared.\r\n");
            continue;
        }

        if (strcmp(line, "e") == 0 || strcmp(line, "errors") == 0) {
            diag_err_report(s_err_ctx, diag_menu_printf);
            continue;
        }

        /* Try numeric index (1-based user input → 0-based array index) */
        char *end = NULL;
        long idx = strtol(line, &end, 10);
        if (end && *end == '\0' && idx >= 1 && idx <= m->count) {
            const diag_mitem_t *item = &m->items[idx - 1];
            diag_menu_printf("Running [%ld] %s...\r\n", idx, item->name);

            /* Clear error context, then set component from item name */
            if (s_err_ctx) {
                diag_err_clear(s_err_ctx);
                diag_err_set_component(s_err_ctx, item->name, "Menu");
            }

            diag_result_t result = diag_menu_run_item(m, (int)(idx - 1));

            const char *res_str = diag_result_str(result);
            diag_menu_printf("Result: %s", res_str);
            if (result != DIAG_PASSED && result != DIAG_SKIPPED) {
                diag_menu_printf("  (errors: %u)", (unsigned)item->error_count);
            }
            diag_menu_printf("\r\n");
            continue;
        }

        diag_menu_printf("Invalid selection.\r\n");
    }

    return DIAG_PASSED;
}

/*===========================================================================*/
/* Error report command handler                                              */
/*===========================================================================*/

static diag_result_t cmd_errors(diag_runner_t *runner, int argc, char *argv[])
{
    (void)argv;
    (void)argc;
    (void)runner;

    if (!s_err_ctx) {
        diag_menu_printf("No error context registered.\r\n");
        return DIAG_ERROR;
    }

    diag_err_report(s_err_ctx, diag_menu_printf);
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
    { "menu",    "Interactive fugazi-style menu",  cmd_menu         },
    { "errors",  "Show error report",             cmd_errors       },
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
    diag_result_t r = console_init();
    if (r != DIAG_PASSED) {
        return r;
    }

    register_builtins();
    return DIAG_PASSED;
}

void diag_menu_loop(diag_runner_t *runner)
{
    if (!runner) return;

    console_puts("\r\n");
    console_puts("============================================\r\n");
    console_puts("   M5Stack CoreS3 Diagnostic System v1.0\r\n");
    console_puts("============================================\r\n");
    console_puts("Type 'help' for available commands.\r\n");
    console_puts("\r\n");

    char line[CONFIG_MENU_LINE_BUF_SIZE];
    char *argv[CONFIG_MENU_MAX_ARGS];

    while (1) {
        console_puts(CONFIG_MENU_PROMPT);
        console_flush_rx();

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
