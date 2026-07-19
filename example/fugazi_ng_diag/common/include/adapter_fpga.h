/* $Id:
 * $Source:
 *------------------------------------------------------------------
 * Filename:    adapter_fpga.h
 *
 *
 * Copyright (c) 2012-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __ADAPTER_FPGA__
#define __ADAPTER_FPGA__

#include "types.h"

#define MAX_ADAPTER 5

/*
 *void adapter_uart_reset(int port);
 *int adapter_uart_tx(int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk);
 *int adapter_uart_rx(int port, int *rx_sz, char* rx_str);
 *int adapter_uart_lpbk_txrx(int port, char* test_str, int test_sz,
 *                           char* rx_str, int *rx_sz, int baud, int is_int_lpbk);
 */
struct adapter_uart_t {
    unsigned int mod_type;
    void (*adapter_uart_reset)(int);
    int (*adapter_uart_tx)(int, int, char*, int, int);
    int (*adapter_uart_rx)(int, int*, char*);
    int (*adapter_uart_lpbk_txrx)(int, char*, int,
                                  char*, int*, int, int);
};

struct adapter_uart_t* get_current_adapter_uart();
struct adapter_uart_t* get_adapter_uart(uint32_t index);
void adapter_uart_exit();
void adapter_uart_clean_up();
int adapter_uart_init(unsigned int mod_type,
                      void (*adapter_uart_reset)(int port),
                      int (*adapter_uart_tx)(int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk),
                      int (*adapter_uart_rx)(int port, int *rx_sz, char* rx_str),
                      int (*adapter_uart_lpbk_txrx)(int port, char* test_str,
                                                    int test_sz, char* rx_str,
                                                    int *rx_sz, int baud, int is_int_lpbk));
#endif /* #if __ADAPTER_FPGA__ */
