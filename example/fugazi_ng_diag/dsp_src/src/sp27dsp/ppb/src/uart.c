/* $Id: uart.c,v 1.3 2012/08/15 14:52:23 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/uart.c,v $
 *------------------------------------------------------------------
 * uart.c 
 *     sp2704 serial console 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*------------------------------------------------------------------
 * uart.c - sp27xx serial console
 *
 * November, 2010 pbecerra
 *
 * Copyright (c) 2010-2012 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include <stdint.h>
#include <stddef.h>
#if defined(USE_AG_MG_REGS)
#include "ag_mg.h"
#include "ag_mg_regs_regops.h"
#include <ag_mg_regs_uart.h>
#elif defined(USE_LSI_SP27XX_REGS)
#include <lsi_sp27xx_reg.h>
#include <lsi_sp27xx_uart.h>
#endif
#include "regs.h"
#include "uart.h"
#include "diag_dss.h"

typedef enum {
	/** @brief WLEN: b 00: 5-bit */
	UART_WLEN_0	= 0,
	/** @brief WLEN: b 01: 6-bit */
	UART_WLEN_1	= 1,
	/** @brief WLEN: b 10: 7-bit */
	UART_WLEN_2	= 2,
	/** @brief WLEN: b 11: 8-bit */
	UART_WLEN_3	= 3
} UART_WLEN_t;

typedef struct  {
	uint32_t baudRate;
	uint32_t baudDivInt;
	uint32_t baudDivFrac;
} UART_BAUDRATE_t;
#define UART_BAUDRATE_TABLE_SIZE 3

void uart_init (uint32_t rate)
{
    static UART_BAUDRATE_t baudRateTable[UART_BAUDRATE_TABLE_SIZE] = {
        {115200,203,29},
        {19200,1220,45},
        {9600,2441,26}
    };
    int i = 0;

    RESET_BITS_M(UARTCR_RA, UARTCR_UARTEN_BM);
    RESET_BITS_M(UARTLCR_H_RA, UARTLCR_H_FEN_BM);
    SET_BITS_M(UARTLCR_H_RA, UARTLCR_H_FEN_BM);
    
    while ( (rate != baudRateTable[i].baudRate)
            && (i < UART_BAUDRATE_TABLE_SIZE )) {
			i++;
    }

    WRITE(UARTIBRD_RA, baudRateTable[i].baudDivInt);
    WRITE(UARTFBRD_RA, baudRateTable[i].baudDivFrac);

    SET_BITS(UARTCR_RA, RM(UARTCR_TXE_BM) | RM(UARTCR_RXE_BM));
    SET_BITS(UARTLCR_H_RA, (UART_WLEN_3 << RM(UARTLCR_H_WLEN_BO)));
    SET_BITS(UARTCR_RA,
             RM(UARTCR_UARTEN_BM) |
             RM(UARTCR_TXE_BM)  |
             RM(UARTCR_RXE_BM));

}

/*
int uart_kbhit (void)
{
    return (!(CHK_REG_M(UARTFR_RA, UARTFR_RXFE_BM)));
}
*/

char uart_getch (void)
{
    char ch = 0;

    while (CHK_REG_M(UARTFR_RA, UARTFR_RXFE_BM))
        ;
    ch = ACCESS(UARTDR_RA) & 0xff;
    return (ch);
}
int uart_putch (char ch)
{
    int success = 0;
    if (!(CHK_REG_M(UARTFR_RA, UARTFR_BUSY_BM))) {
        WRITE(UARTDR_RA, ch);
        success = 1;
    }
    return (success);
}

void uart_unlock (void)
{
    *uart_getlock = FREE;
    *uart_mem = 0;
}

int uart_lock (int master)
{
    uint32_t timer;

    timer = 0xffff;
    /* start timer */
    while (1) {
        /* Check if the master is trying to lock the resource or
         * no other master is trying to acquire the uart */
        if ((*uart_mem == master) || (*uart_mem == 0))
            *uart_mem = master;
        else
           continue;
        if (*uart_getlock == FREE) {
            *uart_getlock = master;
            return (1);
        }
        //timer--;
    }
    return (0);
}

uint32_t uart_puts (const char *str)
{
	uint32_t num_char = 0;
    char ch;
	if (str == NULL) {
		return 0;
	}
    if (uart_lock(5) == 0) {
           uart_unlock();
        return(num_char);
    } 
    /* Print anyways may get some jumbled characters */

	while((ch = *str) != '\0') {
		while (uart_putch(ch) == 0);
		num_char++;
        str++;
	}
       uart_unlock();
 
	return (num_char);
}

void uart_put_long (uint32_t val, uint32_t base)
{
    char bufUltostr[33];
	char *str;
	int c;

	if( (base > 36) || (base < 2) ) {
        return;
    }


	str = bufUltostr + sizeof(bufUltostr);
	*--str = '\0';

	do {
		c = val % base;
		val /= base;
		*--str = (c > 9) ? ('a'- 10 + c):('0' + c);
	} while(val);
	if (base == 16) {
		uart_puts("0x");
	}
    uart_puts(str);
}

/******** History ********
$Log: uart.c,v $
Revision 1.3  2012/08/15 14:52:23  srane
cleanup code.

Revision 1.2  2012/06/28 21:25:56  srane
fix TDM isr, add delay for ethernet loopback etc

Revision 1.1  2012/04/18 09:44:03  srane
Initial checkin


$Endlog$
*/

