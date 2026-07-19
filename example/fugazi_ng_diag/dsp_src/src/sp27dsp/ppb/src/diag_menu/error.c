/* $Id: error.c,v 1.3 2012/07/17 20:46:23 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/diag_menu/error.c,v $
 *------------------------------------------------------------------
 * error.c - Adapted from ng_diags linux_error.c
 *
 * Mar 2012
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "menu.h"
#include "nvsysvars.h"
#include "common.h"
#include "setjmps.h"
#include "uart.h"
#include "debug_console.h"

extern void longjmp(jmp_buf env, int val);

/* Needed to handle largest possible errmsg from lance test */
#define CBUFSIZ 2500

struct nvram nvram;  /* may need to map to actual NVRAM physical address */
unsigned long diagflag_xram;   /* ram global for additional diag flags */

extern jmp_buf *monjmpptr;
extern short diagflag;

/* Buffer that will hold the name of the test been performed. */
#define TESTNAMEBUFSIZ 80
char testnamebuf[TESTNAMEBUFSIZ];
char test_progress_buf[CBUFSIZ/2];

/* Error counters. */
unsigned long testpass = 0;
unsigned long errcount = 0;
unsigned long err_accum = 0;
unsigned long warncount = 0;
unsigned long menu_display = 1;
static int stoponerr(void);

const char *gettestname(void)
{
    return testnamebuf;
}

/*
 * Function testname.
 *
 * Each test been run in a diagnostics environment should be given a test name.
 * This function will save the name of the test so that it can be displayed to
 * the user if the test should fail.
 */
void
testname(char *string, ...)
{
    /* Save name of the test in testnamebuf */
    unsigned char buffer[TESTNAMEBUFSIZ];
    
    strcpy(testnamebuf, string);

    if (string) {
        _Buffer_t b;
        va_list args;

        b.buf = (char *)buffer;
        b.len = 132;
        b.fil = 0;
        va_start(args, string);
        bsp_debug_format_output(&b, string, args);
        va_end(args);
    }
    //if(!(envflag & INDIAG)) return;  /* only print testname in diags */
    
    if(diagflag & D_QUIETMODE) return;  /* unless in quiet mode */
    
    if(diagflag & D_CONTINUOUS && testpass) {
        sprintf(testnamebuf, "%s test passes %ld", buffer, testpass);
    }
    else {
        sprintf(testnamebuf, "%s test ", buffer);
    }

    //clearline;
    //    moveleft(100);

}

void
flush_test_progress_buf(void)
{
    int i;

    for(i=0; i < CBUFSIZ/2; i++) {
        test_progress_buf[i] = 0x0;
    }
}


/*
 * Function cterr.
 *
 * Failure messages for diagnostics should use this function.
 */
void
cterr(char errtype, int errnum, char *errstr, ...)
{
    //    char buffer[CBUFSIZE];
    //    char *bptr = buffer;
    bsp_debug_printf("\r\n *** ");

    switch(errtype) {
    case 'f':
	errcount++;
	err_accum++;
	bsp_debug_printf("Fatal error: ");
	break;
    case 'w':
	warncount++;
	bsp_debug_printf("Warning: ");
	break; 
    case 'a':
	errcount++;
	err_accum++;
	bsp_debug_printf("Test Abort: ");
	break;
    default:
	errcount++;
	err_accum++;
	bsp_debug_printf("Fatal error: ");
	break;
    }

    if (errstr) {
        bsp_debug_printf(errstr);
#if 0
        va_list args;
        va_start(args, errstr);
        vprintf(errstr, args);
        va_end(args);
#endif
    }
    bsp_debug_printf(" ");
    printf(testnamebuf,errnum);
        
    bsp_debug_printf("\r\n");
    /* If "stop on error" FLAG is set then stop test execution. */
    if(stoponerr())  
        longjmp(*monjmpptr, 1);
      
}


/*
 * Function stoponerr.
 *
 * This function gets called when the if diagnostics enters cterr function
 * and the STOP ON ERROR flag is set.
 */
static int
stoponerr(void)
{
    if(diagflag & D_STOPONERR && monjmpptr) {
	bsp_debug_printf("\r\n");
	uart_puts(" test stopped on error \n");
	return(1);
    } else return(0);
}


/*
 * Function prpass.
 *
 * This function is used by diagnostics to indicate progress. 
 */
void
prpass(int pass, char *msg, ...)
{
#define SCREEN_WIDTH   80

    char *bptr, buffer[SCREEN_WIDTH];

    /* Clear current line
     */
    fflush(stdout);
    bsp_debug_printf("");
    bsp_debug_printf("\r");
    memset(buffer, 0x20, sizeof(buffer)); /* ascii fow white space */
    buffer[SCREEN_WIDTH-1] = 0; /* NULL char */
    bsp_debug_printf(buffer);
    bsp_debug_printf("\r");

    bptr = buffer;
    if (pass) {
        bptr += sprintf(bptr, "pass %d, ", pass);
        bsp_debug_printf(buffer);
    }
    if (msg) {
        bsp_debug_printf(msg);
#if 0
        va_list args;
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
#endif
    }
    sprintf(buffer, "%s", testnamebuf);
    bsp_debug_printf(buffer);
    
    fflush(stdout);

}


/*
 * Function prcomplete.
 *
 * This function is used by diagnostics to indicate that the test has
 * completed.
 */
void
prcomplete(int pass, int errcount, char *msg, ...)
{

    bsp_debug_printf("\r\n");

    if (msg) {
        bsp_debug_printf(msg);
#if 0
        va_list args;
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
#endif
    }
    bsp_debug_printf("\r\n errors = %d  warnings = %d\n", errcount, warncount);
}


/*
 * test_printing_to_console
 *
 * Test function which can be called from basic utilities tests printing to
 * the console on the cavium CPU.
 */
void 
test_printing_to_console(int i)
{
    int slot = 3;
    int wic_slot = 4;

    bsp_debug_printf("\r\n ------ Test testname -------- \n");
    testname("Test printing to console");
    bsp_debug_printf("\r\n");
    testname("Test Slot Number %d", slot);
    bsp_debug_printf("\r\n");
    testname("Test Slot Number %d WIC Slot %d", slot, wic_slot);

    bsp_debug_printf("\r\n");
    i = 1;
    prpass(testpass, "here %d ", i);
 
    bsp_debug_printf("\r\n");
    i = 2;
    prpass(testpass, "here here %d ", i);

    bsp_debug_printf("\r\n");
    cterr('f',0," Invalid value %d", i);

    bsp_debug_printf("\r\n");
    cterr('f', 0, "Unexpected DMA complete interrupt during MIB DMA test");

    bsp_debug_printf("\r\n");
    slot = 6;
    cterr('f', 0, "MIB DMA test did not generate DMA complete interrupt"
	  " for slot %d", slot);

    bsp_debug_printf("\r\n");
    i = 4;
    cterr('w',0," Another almot Invalid value %d", i);

    /* Diags use the prcomplete function mot commonly in this way. */
    prcomplete(testpass, errcount, (char *)0);
}

void logprintf(char *buf)
{
    bsp_debug_printf("logprintf: fix me!!!!\n");
    return;
}

void
clrerrlog (void)
{
    /* ERRLOGPTR = errlog_start;*/
    bsp_debug_printf("%s: not supported\n", __FUNCTION__);
}

int
dumperrlog (void)
{
    bsp_debug_printf("%s: not supported\n", __FUNCTION__);
    return (1);
}

/*-------------------- End of File ---------------------*/
/******** History ******** 
$Log: error.c,v $
Revision 1.3  2012/07/17 20:46:23  srane
use ethernet to send/receive command/result to the host. General cleanup.

Revision 1.2  2012/06/28 21:31:37  srane
add support routines for menu display.

Revision 1.1  2012/04/18 09:44:12  srane
Initial checkin


$Endlog$
*/
