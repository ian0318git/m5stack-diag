/* $Id: debug_console.c,v 1.2 2017/07/28 07:58:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/diag_menu/debug_console.c,v $
 *------------------------------------------------------------------
 * debug_console.c
 *      console routines 
 *
 * Mar 2012, Smita Rane
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
#include "debug_console.h"
#if defined(USE_AG_MG_REGS)
#include <ag_mg_cfg.h>
#include <ag_mg_regs.h>
#elif defined(USE_LSI_SP27XX_REGS)
#include "lsi_sp27xx_reg.h"
#endif
#include "regs.h"
#include "uart.h"
#include "common.h"

int getnnum(char *, int , unsigned int *, int);

/**********************************************************************
 *
 * Function: debug_console_putc
 *
 * description: this function is to put char to buffer. 
 *
 * Input : *b - buffer
 *	       ch - console put character
 *
 * Output: retval
 *
 **********************************************************************
 */
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

/**********************************************************************
 *
 * Function: debug_console_console_format_digit
 *
 * description: This function is parse console format  
 *
 **********************************************************************
 */
static int debug_console_format_digit (
    _Buffer_t *b,
    uint64_t num,
    int base,
    int *nchars)
{
    /* Upper case hex digits. */
    static const char UCdigits[] = "0123456789ABCDEFX";
    const char *dc = UCdigits;

    /* Temp buffer big enough to hold largest formatted digit */
    char dbuf[DIGIT_BUFFER_SIZE], *db_end;

    db_end = &dbuf[DIGIT_BUFFER_SIZE - 1];
    *db_end = '\0';
    do {
        *--db_end = dc[(num % base)];
    } while((num /= base) && (db_end > dbuf) );
    /* Formatted digit starts at db_end */
    int ix, size = 0;
    for (ix = 0; db_end[ix] != '\0'; ix++) {
        if (debug_console_putc(b, db_end[ix]) < 0) {
            break;
        } else {
            size++;
        }
    }
    *nchars = size;
    if (db_end[ix] == '\0') {
        return (1);
    } else {
        /* did not complete copying converted digit */
        return (-1);
    }
}
int bsp_debug_format_output (_Buffer_t *b, const char* fmt, va_list ap)
{
    const char *pfmt, *pcspec, *sptr;
    char ch, cc;
    int nwrite = 0; /* number of chars written to buf */
    
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
        int ells = 0;
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
        int flags = 0;
        int base = 10;
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
/* This function wiil print  parameter on screen */
void bsp_debug_printf (const char *fmt, ...)
{
#define DEBUG_CONSOLE_BUF_SIZE 132
    va_list ap;
    _Buffer_t b;
    
    char buf[DEBUG_CONSOLE_BUF_SIZE];
    b.buf = buf;
    b.len = DEBUG_CONSOLE_BUF_SIZE;
    b.fil = 0;
    va_start(ap, fmt);
    bsp_debug_format_output(&b, fmt, ap);
    va_end(ap);
    uart_puts(buf);
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
        while (CHK_REG_M(UARTFR_RA, UARTFR_RXFE_BM)) {
            int i=20;
            while (i>0) i--;
        }
        if (rxCount < bsize) {
            READ(UARTDR_RA,ch);
            ch = ch & 0xff;
            //ch = *(volatile int *)AG_MG_REGS_UARTDR_RA & 0xff;
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


char check_space(int c)
{
    return(((c == '\r') || (c == '\n') || (c == ' ') || (c == '\t') ||
	    (c == '\f') || (c == '\v')) ? TRUE : FALSE);
}


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Function: take_0x_addr
 * Given addr_p, a pointer to an input buffer containing a hex address,
 * shift the pointer past the optional prefix 0x and return the new ptr;
 * otherwise (no prefix), simply return the input pointer.
 */
char *
take_0x_addr (char *addr_p)
{
    char c, b;

    b = check_space(c);

    for (c = *addr_p; b; c = *(++addr_p)) {
        /* scan to first non-whitespace char */
    }
    if ((c == '0') && (*(addr_p + 1) == 'x')) {
        addr_p += 2;  /* pass "0x" prefix */
    }
    return(addr_p);
}

/*
** Return the value for the ascii hex character or -1 if invalid.
*/
char atoh(char c)
{
    if (c >= '0' && c <= '9') {
        return(c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return(c - ('A' - 10));
    }
    if (c >= 'a' && c <= 'f') {
        return(c - ('a' - 10));
    } 

    return(-1);
}


/*
** Convert the ascii string pointed to by cptr to binary according to base.
** Result is placed in *longret.
** Return value is the number of characters processed.
** Maxchars defines the maximum number of characters to process.  If
** maxchars == 0, process until an invalid character occurs.
** Getnum exists for historical reasons.
*/
int
getnum(char *cptr, int base, unsigned int *longret)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
    return(getnnum(cptr, base, longret, 0));
}


int
getnnum(char *cptr, int base, unsigned int *longret, int maxchars)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
    char cval = 0;
    unsigned long value = 0; /* init */
    int count = 0; /* init */

    while (1) {
        cval = atoh(*cptr);
            if (cval >= base) {
                break;  /* invalid character encountered */
            }
        value = (value * base) + cval;
        cptr++;
        count++;
        if(maxchars && count == maxchars) {
            break;
        }
    }
    *longret = value;  /* place result */
    return(count);
}


unsigned long
gethex_answer(char *msgstr, unsigned long currentval, unsigned long min, 
	      unsigned long max)
{
    char buffer[32];
    unsigned int newval;


    while(1) {
        bsp_debug_printf("\r\n%s [0x%x]:  ", msgstr, currentval);
        debug_console_gets(buffer,sizeof(buffer));


        if (buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n') {
            return(currentval);
        }
        if ((getnum(take_0x_addr(buffer), 16, &newval)) <= 0 || 
              (newval < min) || (newval > max)) {
            bsp_debug_printf("valid entry 0x%x to 0x%x...try again\n", min, max);
            continue;
        } else {
            return((unsigned long)newval);
        }
    }
}



/******** History ********
$Log: debug_console.c,v $
Revision 1.2  2017/07/28 07:58:51  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
Initial commit code for Oakenshield

Revision 1.1.86.1  2016/12/14 04:57:39  olin2
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:44:12  srane
Initial checkin


$Endlog$
*/

