/* $Id: debug_console.h,v 1.2 2017/07/28 07:58:37 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/debug_console.h,v $
 *------------------------------------------------------------------
 * debug_console.h
 *      Oakenshield display routines 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DEBUG_CONSOLE_H_
#define _DEBUG_CONSOLE_H_

#include <stdarg.h>

#define FLAG_DIGIT  0x00000020
#define DIGIT_BUFFER_SIZE ((2 * sizeof(unsigned long long)) + 8)
#define UART_PUTC( ch ) { \
        while(CHK_REG_M(UARTFR_RA,UARTFR_BUSY_BM)) ; \
        WRITE(UARTDR_RA, ch); \
    }
/* Parameters needed to manage putting a formatted string to a buffer */
typedef struct {
    char *buf;
    int len; /* sizeof(buf[]) */
    int fil; /* number of chars put to buf[] */
} _Buffer_t; 

extern void bsp_debug_printf(const char *fmt, ...);
extern int32_t debug_console_gets(char *rxBuf, uint32_t bsize);
extern int bsp_debug_format_output(_Buffer_t *b, const char* fmt, va_list ap);
extern void prcomplete(int pass, int errcount, char *msg, ...);
extern void cterr(char, int,const char*, ...);
extern unsigned long
gethex_answer(char *msgstr, unsigned long currentval, unsigned long min,
          unsigned long max);


#endif /* _DEBUG_CONSOLE_H_ */

/* 
 * $Log: debug_console.h,v $
 * Revision 1.2  2017/07/28 07:58:37  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:31  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.1.86.2  2017/03/30 10:25:49  harrchan
 * Add fpga upgrade utility
 *
 * Revision 1.1.86.1  2016/12/14 05:03:49  olin2
 * Initial commit code for Oakenshield
 *
 * Revision 1.1  2012/04/18 09:50:18  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
