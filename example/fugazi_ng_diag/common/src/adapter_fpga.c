/* $Id:
 * $Source:
 *------------------------------------------------------------------
 *
 * Filename: adapter_fpga.c
 *
 * Description: SM-NIM and NIM-PIM Adapter fpga related code
 * Copyright (c) 2014-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "adapter_fpga.h"

static void adapter_uart_reset(int port)
{
}

static int adapter_uart_tx(int port, int baud,
                           char* tx_str, int tx_sz, int is_int_lpbk)
{
    return PASSED;
}

static int adapter_uart_rx(int port, int *rx_sz, char* rx_str)
{
    return PASSED;
}

static int adapter_uart_lpbk_txrx(int port, char* test_str,
                                  int test_sz, char* rx_str,
                                  int *rx_sz, int baud, int is_int_lpbk)
{
    return PASSED;
}

static struct adapter_uart_t adapter_uart[MAX_ADAPTER] = {
    {
        .mod_type = 0,
        .adapter_uart_reset = adapter_uart_reset,
        .adapter_uart_tx = adapter_uart_tx,
        .adapter_uart_rx = adapter_uart_rx,
        .adapter_uart_lpbk_txrx = adapter_uart_lpbk_txrx,
    },
};

static int adapter_pointer = 0;

struct adapter_uart_t* get_current_adapter_uart()
{
    return &adapter_uart[adapter_pointer];
}

struct adapter_uart_t* get_adapter_uart(uint32_t index)
{
    struct adapter_uart_t *padapter_uart = NULL;

    if (index <= adapter_pointer)
        padapter_uart = &adapter_uart[index];

    return padapter_uart;
}

int adapter_uart_init(unsigned int mod_type,
                      void (*adapter_uart_reset)(int port),
                      int (*adapter_uart_tx)(int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk),
                      int (*adapter_uart_rx)(int port, int *rx_sz, char* rx_str),
                      int (*adapter_uart_lpbk_txrx)(int port, char* test_str,
                                                    int test_sz, char* rx_str,
                                                    int *rx_sz, int baud, int is_int_lpbk))
{
    int rc = FAILED;
    if (adapter_pointer < MAX_ADAPTER) {
        adapter_pointer++;
        adapter_uart[adapter_pointer].mod_type = mod_type;
        adapter_uart[adapter_pointer].adapter_uart_reset = adapter_uart_reset;
        adapter_uart[adapter_pointer].adapter_uart_tx = adapter_uart_tx;
        adapter_uart[adapter_pointer].adapter_uart_rx = adapter_uart_rx;
        adapter_uart[adapter_pointer].adapter_uart_lpbk_txrx = adapter_uart_lpbk_txrx;
        rc = PASSED;
    }

    return rc;
}

void adapter_uart_exit()
{
    if (adapter_pointer > 0)
        adapter_pointer--;
}

void adapter_uart_clean_up()
{
    adapter_pointer = 0;
}
