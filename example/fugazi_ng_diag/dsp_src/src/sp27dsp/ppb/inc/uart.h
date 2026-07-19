/* $Id: uart.h,v 1.3 2012/07/17 20:34:28 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/uart.h,v $
 *------------------------------------------------------------------
 * uart.h
 *      Uart header file 
 *
 * Mar 2012, Smita Rane
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

#define diagisdigit(c)      ((c)>= 0x30 && (c)<=0x39)
#define diagislower(c)      ((c)>=0x61 && (c)<=0x7a)
#define diagisupper(c)      ((c)>=0x41 && (c)<=0x5a)
#define diagisxdigit(c)     (((c)>=0x30 && (c)<=0x39) || ((c)>=0x41 && (c)<=0x46) || ((c)>=0x61 && (c)<=0x66))
static inline unsigned char
//__attribute__ ((section(".uboottext")))
__diagtoupper (unsigned char c)
{
    if (diagislower(c))
        c -= 'a'-'A';
    return c;
}
#define diagtoupper(c) __diagtoupper(c)

extern void     uart_init(uint32_t rate);
extern int      uart_kbhit(void);
extern char     uart_getch(void);
extern int      uart_putch(char ch);
extern uint32_t uart_puts(const char *str);
extern void     uart_put_long(uint32_t num, uint32_t base);
extern int      uart_lock(int core);
extern void     uart_unlock(void);

#endif /* _UART_H_ */

/******** History ********
$Log: uart.h,v $
Revision 1.3  2012/07/17 20:34:28  srane
cleanup

Revision 1.2  2012/05/31 06:40:49  srane
Add define, cleanup.

Revision 1.1  2012/04/18 09:50:19  srane
Initial checkin


$Endlog$
*/

