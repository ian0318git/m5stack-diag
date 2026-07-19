/* $Id: debug_console.c,v 1.2 2017/07/28 07:58:36 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/src/debug_console.c,v $
 *------------------------------------------------------------------
 * debug_console.c 
 *      Oakenshield : DSS console routines
 *
 * Mar 2012,  Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <ag_mg_regs.h>
#include "uart.h"
#include "diag_dss.h"


extern void debug_console_puts (const char *str);
/* Parameters needed to manage putting a formatted string to a buffer */
typedef struct {
    char *buf;
    int len; /* sizeof(buf[]) */
    int fil; /* number of chars put to buf[] */
} _Buffer_t; 

#define UART_PUTC( ch ) { \
        while(CHK_REG_MASK(AG_MG_REGS_UARTFR_RA,AG_MG_REGS_UARTFR_BUSY_BM)) ; \
        REG32_WRITE(AG_MG_REGS_UARTDR_RA, ch); \
    }

static int debug_console_putc (_Buffer_t *b, char ch)
{
    int retval = 1;
    if (b->fil < b->len) {
        b->buf[b->fil] = ch;
        b->fil += 1;
    } else {
        retval = -1;
    }
    return (retval);
}

#define FLAG_DIGIT  0x00000020

#define DIGIT_BUFFER_SIZE ((2 * sizeof(unsigned long long)) + 8)
static int debug_console_format_digit (
    _Buffer_t *b,
    uint64_t num,
    int base,
    int *nchars)
{
    /* Upper case hex digits. */
    static const char UCdigits[] = "0123456789ABCDEFX";
    const char *dc = UCdigits;
    int size, i;

    /* Temp buffer big enough to hold largest formatted digit */
    char dbuf[DIGIT_BUFFER_SIZE], *db_end;

    db_end = &dbuf[DIGIT_BUFFER_SIZE - 1];
    *db_end = '\0';
    do {
        *--db_end = dc[(num % base)];
    } while((num /= base) && (db_end > dbuf) );
    /* Formatted digit starts at db_end */
    size = 0;
    for (i = 0; db_end[i] != '\0'; i++) {
        if (debug_console_putc(b, db_end[i]) < 0) {
            break;
        } else {
            size++;
        }
    }
    *nchars = size;
    if (db_end[i] == '\0') {
        return (1);
    } else {
        /* did not complete copying converted digit */
        return (-1);
    }
}

static int bsp_debug_format_output (_Buffer_t *b, const char* fmt, va_list ap)
{
    const char *pfmt, *pcspec, *sptr;
    char ch, cc;
    int flags, base, ells, nwrite = 0; /* number of chars written to buf */
    
    pfmt = fmt;
    while ((ch = *pfmt++) != '\0') {
        /* Find a conversion spec - starts with "%" char */
        if (ch != '%') {
            if (debug_console_putc(b, ch) < 0) {
                break;
            } else {
                ++nwrite; 
                continue;
            } 
        }
        ch = *pfmt++;
        if (ch == '%') {
            /* Escaping % */
            if (debug_console_putc(b, ch) < 0) {
                break;
            } else { 
                ++nwrite;
                continue;
            }
        }
        pcspec = pfmt - 2;
        /* skip past the flag characters */
        do {
            if ( (ch == '#') ||
                 (ch == '-') ||
                 (ch == ' ') ||
                 (ch == '0') ) {
            } else {
                break;
            }
            ch = *pfmt++;
        } while (ch != '\0');
        /* skip optional field width specifier */
        if ((ch != '\0') && diagisdigit(ch)) {
            while (diagisdigit(ch)) {
                ch = *pfmt++;
            }
        }
        if ((ch != '\0') && (ch == '.')) {
            /* Precission - not supported, so fake an invalid cspec*/
            ++pfmt;
            ch = '0';
        }
        /* Find optional length modifier */
        ells = 0;
        if (ch != '\0') {
            switch (ch) {
            case 'h':
                do {
                    --ells;
                    ch = *pfmt++;
                } while ((ch != '\0') && (ch == 'h'));
                break;
            case 'l':
                do {
                    ++ells;
                    ch = *pfmt++;
                } while ((ch != '\0') && (ch == 'l'));
                break;
            default:
                break;
            }
        }
        if ((ch == '\0') || (ells > 2)) {
            /* Not a valid convertion spec */
            while (pcspec != pfmt) {
                ch = *pcspec++;
                if (debug_console_putc(b, ch) < 0) {
                    break;
                } else {
                    ++nwrite;
                }
            }
            break;
        }

        /* Get the conversion specifier */
        flags = 0;
        base = 10;
        switch (ch) {
        case 's':
            sptr = va_arg(ap, char *);
            while ((ch = *sptr++) != '\0') {
                if (debug_console_putc(b, ch) < 0) {
                    goto done;
                } else {
                    ++nwrite;
                }
            }
            break;
        case 'c':
            cc = (char) va_arg(ap, int);
            if (debug_console_putc(b, cc) < 0) {
                goto done;
            } else {
                ++nwrite;
            }
            break;
        case 'X':
        case 'x':
            base = 16;
            /* break; */
        case 'i':
        case 'd':
        case 'u':
            flags |= FLAG_DIGIT;
            break;
        default:
            /* Not a valid convertion spec */
            while (pcspec != pfmt) {
                ch = *pcspec++;
                if (debug_console_putc(b, ch) < 0) {
                    break;
                } else {
                    ++nwrite;
                }
            }
            break;
        }
        if (flags & FLAG_DIGIT) {
            uint64_t unumber;
            int digit_len = 0;
            if (ells == 2) {
                unumber = va_arg(ap, unsigned long long);
            } else if (ells == 1) {
                unumber = va_arg(ap, unsigned long);
            } else {
                unumber = va_arg(ap, unsigned int);
            }
            {
                int retval = debug_console_format_digit(b,
                                                        unumber,
                                                        base,
                                                        &digit_len);
                nwrite += digit_len;
                if (retval < 0) {
                    goto done;
                }
            }
        }
    }
done:
    if (nwrite < b->len) {
        b->buf[nwrite] = '\0';
    } else {
        b->buf[b->len - 1] = '\0';
    }
    return (nwrite);
}

void bsp_debug_printf (int core, const char *fmt, ...)
{
#define DEBUG_CONSOLE_BUF_SIZE 132
    va_list ap;
    _Buffer_t b;
    
//int i;
    // The debug_console process is not up yet
    char buf[DEBUG_CONSOLE_BUF_SIZE];

//for (i=0;i<50;i++) {
while(1) {
    if (uart_lock(core) == 0) {
        ;
    } else {
        break;
    }
//if ( i == 50) {

//uartputs("\r\n ***CANNOT PRINT LOCKED BY *uart_getlock");
//uartputlong(*uart_getlock, 16);
//return;
//}
}

    // copy prompt first
    //memcpy(buf, prompt[bsp_get_core_id()], PROMPT_LEN);
    //b.buf = buf+PROMPT_LEN;
    //b.len = DEBUG_CONSOLE_BUF_SIZE-PROMPT_LEN;
    b.buf = buf;
    b.len = DEBUG_CONSOLE_BUF_SIZE;
    b.fil = 0;
    va_start(ap, fmt);
    bsp_debug_format_output(&b, fmt, ap);
    va_end(ap);
    debug_console_puts(buf);
    //uart_puts(buf);
    uart_unlock();
}

/* receive string data via UART device */
int32_t debug_console_gets(char *rxBuf, uint32_t bsize)
{
        int ch = 0;
        char * cs = rxBuf;
        uint32_t rxCount = 0;

    if ((rxBuf == NULL) || (bsize == 0)) {
        return (rxCount);
    }
    --bsize;
    while (ch != '\r'){
        while (CHK_REG_MASK(AG_MG_REGS_UARTFR_RA, AG_MG_REGS_UARTFR_RXFE_BM)) {
#if 0
            union SIGNAL *sig;

            sig = receive_w_tmo(MS_TO_TICKS(2), all);
            if (sig) {
                if (sig->sig_no == SIG_DEBUG_PRINTF) {
                    debug_console_puts(sig->debug_printf_sig.str);
                } 
                free_buf(&sig);
            }
#endif /* SR ?? */
            int i=20;
            while (i>0) i--;
        }
        if (rxCount < bsize) {
            ch = *(volatile int *)AG_MG_REGS_UARTDR_RA & 0xff;
            /*
             * If input char is backspace or delete
             */
            if ((ch == 0x8) || (ch == 0x7F)) {
                if (rxCount > 0) {
                    --cs;               /* forget last char typed */
                    rxCount--;
                    UART_PUTC(0x8);     /* and strike it out on the console */
                    UART_PUTC(' ');
                    UART_PUTC(0x8);
                }
            } else {
                *cs++ = ch;
                UART_PUTC(ch);
                rxCount++;
            }
        } else {
            break;
        }
    }
    UART_PUTC('\n');
    *cs++ = '\0';
    return rxCount;
}

/******** History ********
 * $Log: debug_console.c,v $
 * Revision 1.2  2017/07/28 07:58:36  harrchan
 * Collapse Oakenshield-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2017/06/29 08:14:30  harrchan
 * Initial commit code for Oakenshield
 *
 * Revision 1.2  2012/05/10 22:45:14  srane
 * Add DSS sanity test for all cores.
 *
 * Revision 1.1  2012/04/18 18:08:36  srane
 * Initial checkin
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


