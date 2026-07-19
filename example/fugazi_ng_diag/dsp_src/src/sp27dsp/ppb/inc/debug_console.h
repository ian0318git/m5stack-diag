/* $Id: debug_console.h,v 1.1 2012/04/18 09:50:18 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/inc/debug_console.h,v $
 *------------------------------------------------------------------
 * debug_console.h
 *      Graffham display routines 
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DEBUG_CONSOLE_H_
#define _DEBUG_CONSOLE_H_

#include <stdarg.h>

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
extern void cterr(char, int, char*, ...);


#endif /* _DEBUG_CONSOLE_H_ */

/* 
 * $Log: debug_console.h,v $
 * Revision 1.1  2012/04/18 09:50:18  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
