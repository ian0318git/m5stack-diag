/* $Id: diag_fpga_uart.c,v 1.3 2017/03/30 08:30:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fpga_uart.c,v $
 *------------------------------------------------------------------
 *
 * diag_fpga_uart.c - FPGA UART Library
 *
 * June 2015, Alan Peng
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "diag_fpga_lib.h"
#include "cross_platform.h"

/* prototype */
unsigned int get_platform_uart_addr(int);
void diag_fpga_uart_reset(int);
int uart_lpbk_txrx(int, char*, int, char*, int *, int, int); 
int diag_fpga_uart_tx(int, int, char*, int, int);
int diag_fpga_uart_rx(int, int *, char*);


unsigned int FPGA_UART_PORT[] = 
                       {UART0_OFFSET, UART1_OFFSET, UART2_OFFSET, 
                        UART3_OFFSET, UART4_OFFSET, UART5_OFFSET, 
                        UART6_OFFSET, UART7_OFFSET, UART8_OFFSET};

unsigned int get_platform_uart_addr (int i)
{
    return (FPGA_UART_PORT[i]);
}

void
diag_fpga_uart_reset (int port)
{
    unsigned int uart, uart_addr, buf; 
    uart = get_platform_uart_addr(port); 

    uart_addr = uart + FPGA_UART_IIR_FCR_REG; 
    diag_fpga_reg_read(uart_addr, &buf);  
    buf = 0xC6;  /* tx rx reset */
    diag_fpga_reg_write(uart_addr, buf);  

    uart_addr = uart + FPGA_UART_MCR_REG; 
    diag_fpga_reg_read(uart_addr, &buf);  
    buf &= ~0x10; /* turn off loopback mode */
    diag_fpga_reg_write(uart_addr, buf);  
   
    return;
}

/*-------------------------------------------------------------------
 *
 * Function : uart_lpbk_txrx
 * Description: write a string to a given uart port and try to retreive data
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
uart_lpbk_txrx (int port, char* test_str, int test_sz, char* rx_str,
                int *rx_sz, int baud, int is_int_lpbk)
{

    diag_fpga_uart_reset(port);

    diag_fpga_uart_tx(port, baud, test_str, test_sz, is_int_lpbk);
    diag_fpga_uart_rx(port, rx_sz, rx_str);

    diag_fpga_uart_reset(port);

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : diag_fpga_uart_tx
 * Description: write a string to a given uart port
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
diag_fpga_uart_tx (int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk)
{
    unsigned int uart, uart_addr, buf;
    unsigned int idx;
    unsigned int quot;
    char dll, dlm; // division latch least significant and most significant

    uart = get_platform_uart_addr(port);

    quot = 50000000 / baud;
    dll = quot & 0xFF;
    dlm = (quot & 0xFF00) >> 8;

    /* tx rx reset */
    uart_addr = uart + FPGA_UART_IIR_FCR_REG;
    diag_fpga_reg_read(uart_addr, &buf);
    buf = 0xC6; 
    diag_fpga_reg_write(uart_addr, buf);

    /* setup baud rate */
    uart_addr = uart + FPGA_UART_LCR_REG; 
    diag_fpga_reg_read(uart_addr, &buf);
    buf = 0x83;  
    diag_fpga_reg_write(uart_addr, buf);

    uart_addr = uart + FPGA_UART_RBRTHRDLL_REG; 
    diag_fpga_reg_read(uart_addr, &buf);
    buf = dll; 
    diag_fpga_reg_write(uart_addr, buf);

    uart_addr = uart + FPGA_UART_IER_DLM_REG;
    diag_fpga_reg_read(uart_addr, &buf);
    buf = dlm;  
    diag_fpga_reg_write(uart_addr, buf);

    uart_addr = uart + FPGA_UART_LCR_REG; 
    diag_fpga_reg_read(uart_addr, &buf);
    buf = 3; 
    diag_fpga_reg_write(uart_addr, buf);

    uart_addr = uart + FPGA_UART_IIR_FCR_REG;
    diag_fpga_reg_read(uart_addr, &buf);
    buf = 0x1;  /* enable FIFO and 1 byte trigger level */
    diag_fpga_reg_write(uart_addr, buf);

    if (!tx_sz)
        return (PASSED);

    if (is_int_lpbk) {
        /* if (is_utah() || is_sword() || is_dagger()) { */
        /* assume tachi is following USD */
        if (1) {  
            /* In Utah the flow control bits are enabled so in a
             * loopback configuration you need to enable DTR and RTS
             */
            uart_addr = uart + FPGA_UART_MCR_REG; 
            diag_fpga_reg_read(uart_addr, &buf);
            buf = 0x13; /* looopback mode, enable DTR and RTS */
            diag_fpga_reg_write(uart_addr, buf);
        } else {
            uart_addr = uart + FPGA_UART_MCR_REG; 
            diag_fpga_reg_read(uart_addr, &buf);
            buf = 0x13; /* looopback mode */
            diag_fpga_reg_write(uart_addr, buf);
        }
    } else {
        uart_addr = uart + FPGA_UART_MCR_REG; 
        diag_fpga_reg_read(uart_addr, &buf);
        buf &= ~0x10; /* turn off looopback mode */
        diag_fpga_reg_write(uart_addr, buf);

        /* if (is_utah() || is_sword() || is_dagger()) { */
        /* assume tachi is following USD */
        if (1) { 
            uart_addr = uart + FPGA_UART_MCR_REG; 
            diag_fpga_reg_read(uart_addr, &buf);
            buf = 0x3; /* enable DTR and RTS */
            diag_fpga_reg_write(uart_addr, buf);
        }

    }
    uart_addr = uart + FPGA_UART_RBRTHRDLL_REG;
    for (idx = 0; idx < tx_sz; idx++) {
        buf = (tx_str[idx] & 0xFF); 
        diag_fpga_reg_write(uart_addr, buf);
        usleep(1000);
    }

    return (PASSED);
}

/*-------------------------------------------------------------------
 *
 * Function : diag_fpga_uart_rx
 * Description: try to retreive data at the uart port
 * INPUT:  port         - uart port
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
diag_fpga_uart_rx (int port, int *rx_sz, char* rx_str)
{
    unsigned int uart, uart_addr, buf;
    int cnt = 0;
    char* c;

    uart = get_platform_uart_addr(port);

    cnt = 0;
    c = rx_str;

    for (;;) { 
        uart_addr = uart + FPGA_UART_LSR_REG;
        diag_fpga_reg_read(uart_addr, &buf);
        if (buf & 1) {
            uart_addr = uart + FPGA_UART_RBRTHRDLL_REG; 
            diag_fpga_reg_read(uart_addr, &buf);
            c[cnt] = buf; 
            cnt++;
            if (*rx_sz > 0) {
                if (cnt >= *rx_sz)
                    return (PASSED);
            }
            usleep(2000); /*delay is important: works for baud 9600 */
        } else { 
            break;
        }
    }

    *rx_sz = cnt;
    return (PASSED);
}


/*-------------------------------------------------------------------
 *
 * Function : dash_uart_tx
 * Description: write a string to a given uart port
 * INPUT:  port         - uart port
 *         test_str     - test data that will be transmitted
 *         test sz      - size of test data
 *         baud    - baud rate
 *  
 *         is_int_lpbk  - enable/disable UART FPGA internal loopback setting
 *                        TRUE : enable internal loopback
 *                        FALSE: disable internal loopback
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
dash_uart_tx (int port, int baud, char* tx_str, int tx_sz, int is_int_lpbk)
{
#ifdef TACHI
    return (FALSE);
#else
    unsigned int idx;
    uart_t *uart;
    unsigned int quot;
    char dll, dlm; // division latch least significant and most significant

    uart = (uart_t *)get_platform_uart_addr(port);

    quot = 50000000 / baud;
    dll = quot & 0xFF;
    dlm = (quot & 0xFF00) >> 8;

    uart->fcr = DYNAMO_TX_RX_RESET;   /* tx rx reset */

    /* setup baud rate */
    uart->lcr = DYNAMO_SETUP_RATE;   /* 0xc */
    uart->dll = dll;    
    uart->dlm = dlm;

    uart->lcr = 3;
    uart->fcr = DYNAMO_ENB_FIFO; /*enable FIFO and 1 byte trigger level */

    if (!tx_sz)
        return (PASSED);

    if (is_int_lpbk) {
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach()) {
            /* In Utah the flow control bits are enabled so in a 
             * loopback configuration you need to enable DTR and RTS 
             */
            uart->mcr = DYNAMO_LPK_ENB_DTR_RTS;   /* looopback mode, enable DTR and RTS */
        } else {
            uart->mcr = DYNAMO_LPK_ON;   /* turn on looopback mode */
        }
    } else {
        uart->mcr &= ~DYNAMO_LPK_ON;     /* turn off looopback mode */
        if (is_utah() || is_sword() || is_dagger() || is_goldbeach()) {
            uart->mcr = DYNAMO_ENB_DTR_RTS;    /* enable DTR and RTS */
        }
    }
    for (idx = 0; idx < tx_sz; idx++) {
        uart->dll = (tx_str[idx] & 0xFF);
        usleep(1000);
    }
    return (PASSED);

#endif
}

/*-------------------------------------------------------------------
 *
 * Function : uart_rx
 * Description: try to retreive data at the uart port
 * INPUT:  port         - uart port
 * OUTPUT:
 *          rx_str   -   data received
 *          rx_sz    -   size of data received
 * RETURN:
 *         PASSED/FAILED
 * -------------------------------------------------------------------
*/
int
dash_uart_rx (int port, int *rx_sz, char* rx_str)
{
    uart_t *uart;
    int cnt = 0;
    char* c;

    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    cnt = 0;
    c = rx_str;
    while (uart->lsr & 1) {
        c[cnt] = uart->dll;
        cnt++;
        if (*rx_sz > 0) {
            if (cnt >= *rx_sz)
                return (PASSED);
        }
        usleep(2000); /*delay is important: works for baud 9600 */
    }
    *rx_sz = cnt;
    return (PASSED);
}

void
dash_uart_reset (int port)
{
    uart_t *uart;
    uart = (uart_t *)get_platform_uart_addr(port); /* uart 8 */
    uart->fcr = 0xC6;   /* tx rx reset */
    uart->mcr &= ~0x10; /* turn off loopback mode */
    return;
}





/*---------------------------------------------------------------
$Log: diag_fpga_uart.c,v $
Revision 1.3  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.2.14.2  2017/02/21 03:51:52  haohsu
Add NIM Dynamo to TACHI

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.1  2015/07/31 10:39:59  alpeng
first check in for testcard

$Endlog$
*/

