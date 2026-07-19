/* $Id: uart.h,v 1.2 2017/07/28 07:58:35 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/uart.h,v $
 *------------------------------------------------------------------
 * uart.h
 *      Graffham - DSS uart 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*------------------------------------------------------------------
 * uart.h - sp27xx serial console
 *
 * November, 2010 pbecerra
 *
 * Copyright (c) 2010 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
//in makefile #define USE_AG_MG_REGS 1
//#include <sp_ppb_std.h>

#define diagisdigit(c)      ((c)>= 0x30 && (c)<=0x39)
#define diagislower(c)      ((c)>=0x61 && (c)<=0x7a)
#define diagisupper(c)      ((c)>=0x41 && (c)<=0x5a)
#define diagisxdigit(c)     (((c)>=0x30 && (c)<=0x39) || ((c)>=0x41 && (c)<=0x46) || ((c)>=0x61 && (c)<=0x66))


extern void     uart_init(uint32_t rate);
extern int      uart_kbhit(void);
extern char     uart_getch(void);
extern int      uart_putch(char ch);
extern uint32_t uart_puts(int core, const char *str);
extern void uart_put_long (int core, uint32_t val, uint32_t base);
extern void     uart_unlock(void);
extern int      uart_lock(int master);
extern uint32_t uartputs(const char *str);


#endif /* _UART_H_ */

/* 
 * $Log: uart.h,v $
 * Revision 1.2  2017/07/28 07:58:35  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:29  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.2  2012/05/31 06:40:19  srane
 * Cleanup.
 *
 * Revision 1.1  2012/04/18 22:08:17  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

