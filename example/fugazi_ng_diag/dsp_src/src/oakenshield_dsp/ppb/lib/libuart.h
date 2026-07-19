/* $Id: libuart.h,v 1.2 2017/07/28 07:58:49 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libuart.h,v $
 *------------------------------------------------------------------
 * libuart.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * libpuart.h - basic routines to access UART from MP-cores
 *
 * NOTE: These routines perform no interlocking. Access control from multiple
 *       processors or multiple threads must be implemented at a higher level.
 *
 *  Created on: Aug 26, 2009
 *      Author: dokim
 */

#ifndef LIBPPBUART_H
#define LIBPPBUART_H

#include <stdint.h>

#define PRINT_STR(a)	sp_SerialPutS(a)
#define PRINT_DEC(a)	sp_SerialPutLong(a, 'd')
#define PRINT_HEX(a)	sp_SerialPutLong(a, 'h')

/* UART_MAX_CHARS used to allocate buffer space for outgoing UART messages */
#define UART_MAX_CHARS	256

/*

 * Baud rate divisor BAUDDIV = (UARTCLK/ {16 * Baud rate})
 *  where UARTCLK is the same as the PPB clock, i.e.:
 * 		- 1/2 of CKI to the SP2704 (e.g. 25 MHz) or
 * 		- 1/2 of DSSCLK from PLL (e.g. 375 MHz)
 *
 * The BAUDDIV is comprised of
 * 		the integer value (BAUD DIVINT) and
 * 		the fractional value (BAUD DIVFRAC - a 6-bit quantity)
 *
 * Example:
 * If the required baud rate is 9600 and UARTCLK = 25MHz (1/2 of CKI)
 * then:
 * Baud Rate Divisor = 25000000/(16 * 9600) = 162.76
 * Therefore, BRDI = 162 and BRDF = 0.76
 * Therefore, fractional part, m = integer((0.76 * 64) + 0.5) = 49
 * Generated baud rate divider = 162 + 49/64 = 162.766
 * Generated baud rate = 25000000/(16 * 162.766) = 9599.7 baud
 */

/* To avoid inclusion of the C floating point library, only use the
 * SP_INIT_SERIAL macro to setup UART when both UARTCLK and BAUD rate
 * are known at compile time so the constants can be calculated by the
 * preprocessor.
 * */
#define SP_INIT_SERIAL(_UARTCLK_, _BAUD_) \
	sp_InitSerial( (uint32_t) _UARTCLK_ * 1000000 / ((_BAUD_) * 16), \
		(uint32_t) ( (double) (_UARTCLK_ * 1000000.0 / ((_BAUD_) * 16.0) - \
		(int) (_UARTCLK_ * 1000000 / ((_BAUD_) * 16) ) ) * 64 + 0.5), 0)

void
sp_InitSerial(				/* set UART Baud rate */
	uint32_t brd_i,			/* in: BAUD DIVINT (0->65535) */
	uint32_t brd_f,			/* in: BAUD DIVFRAC (0->63) */
	uint32_t lbe);			/* in: Internal Loopback mode enable, only for debug purpose, should be '0' in most cases */
void
sp_SerialPutC(				/* Transmit a character “ch” via UART device */
	char ch);				/* in: character to be written */

char						/* ret: character recieved */
sp_SerialGetC(void);			/* Receive a character via UART device */

int32_t						/* ret: actual number of characters read */
sp_SerialGetS(				/* read string up to UART_MAX_CHARS long */
	char* rx_data);			/* in: pointer to buffer at least UART_MAX_CHARS long */

int32_t						/* ret: actual number of characters written */
sp_SerialPutS(				/* write string to UART */
	char* tx_data);			/* in: null-terminated string to be written */

int32_t
sp_SerialPutLong(			/* format and send numeric value */
	uint32_t data,			/* in: long data value to be sent */
	char format);			/* in: format: 'd' for decimal or 'h' for hex */

uint32_t
uart_rx_str(
        char *rx_data);

#endif /* LIBPPBUART_H */

/******** History ********
$Log: libuart.h,v $
Revision 1.2  2017/07/28 07:58:49  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:36  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/06/28 21:25:04  srane
add uart rx routine.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

