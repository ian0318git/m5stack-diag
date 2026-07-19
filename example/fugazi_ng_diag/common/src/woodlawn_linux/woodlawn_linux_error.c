/* $Id: woodlawn_linux_error.c,v 1.2 2013/10/08 08:48:32 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/woodlawn_linux_error.c,v $
 *------------------------------------------------------------------
 * File: woodlawn_linux_error.c
 *
 * March 2011, Paul Tong
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
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
#include "term.h"
#include "common.h"
#include "dev_object.h"
#include "setjmps.h"
extern void longjmp(jmp_buf env, int val);

/* Needed to handle largest possible errmsg from lance test */
#define CBUFSIZ 2500

struct nvram nvram;  /* may need to map to actual NVRAM physical address */
unsigned long diagflag_xram;   /* ram global for additional diag flags */

extern jmp_buf *monjmpptr;

/* Buffer that will hold the name of the test been performed. */
#define TESTNAMEBUFSIZ 80
char testnamebuf[TESTNAMEBUFSIZ];
char test_progress_buf[CBUFSIZ/2];

/* Error counters. */
unsigned long testpass = 0;
unsigned long errcount = 0;
unsigned long err_accum = 0;
unsigned long warncount = 0;
static int stoponerr(void);

const char *gettestname()
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

    strcpy(testnamebuf, string);

    if (string) {
        va_list args;
        va_start(args, string);
        vprintf(string, args);
        va_end(args);
    }
    //if(!(envflag & INDIAG)) return;  /* only print testname in diags */
    
    if(DIAGFLAG & D_QUIETMODE) return;  /* unless in quiet mode */
    

    if(DIAGFLAG & D_CONTINUOUS && testpass)
        sprintf(testnamebuf, "%s test passs %ld", string, testpass);
    else
        sprintf(testnamebuf, "%s test ", string);
    
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
    
    printf(testnamebuf,errnum);
    printf("\n *** ");

    switch(errtype) {
    case 'f':
	errcount++;
	err_accum++;
	printf("Fatal error: ");
	break;
    case 'w':
	warncount++;
	printf("Warning: ");
	break; 
    case 'a':
	errcount++;
	err_accum++;
	printf("Test Abort: ");
	break;
    default:
	errcount++;
	err_accum++;
	printf("Fatal error: ");
	break;
    }

    if (errstr) {
        va_list args;
        va_start(args, errstr);
        vprintf(errstr, args);
        va_end(args);
    }
    printf("\n");
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
    if(DIAGFLAG & D_STOPONERR && monjmpptr) {
	printf("\n");
	puts(" test stopped on error \n");
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
    char *bptr, buffer[100];

    clearline;
    moveleft(100);
    
    bptr = buffer;
    if (pass) {
        bptr += sprintf(bptr, "pass %d, ", pass);
        printf(buffer);
    }
    if (msg) {
        va_list args;
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
    }
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

    printf("\n");

    if (msg) {
        va_list args;
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
    }
    printf("\n Total Errors = %d. Warnings = %ld", errcount, warncount);
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

    printf("\n ------ Test testname -------- \n");
    testname("Test printing to console");
    printf("\n");
    testname("Test Slot Number %d", slot);
    printf("\n");
    testname("Test Slot Number %d WIC Slot %d", slot, wic_slot);

    printf("\n");
    i = 1;
    prpass(testpass, "here %d ", i);
 
    printf("\n");
    i = 2;
    prpass(testpass, "here here %d ", i);

    printf("\n");
    cterr('f',0," Invalid value %d", i);

    printf("\n");
    cterr('f', 0, "Unexpected DMA complete interrupt during MIB DMA test");

    printf("\n");
    slot = 6;
    cterr('f', 0, "MIB DMA test did not generate DMA complete interrupt"
	  " for slot %d", slot);

    printf("\n");
    i = 4;
    cterr('w',0," Another almost Invalid value %d", i);

    /* Diags use the prcomplete function most commonly in this way. */
    prcomplete(testpass, errcount, (char *)0);
}

void logprintf(char *buf)
{
    printf("logprintf: fix me!!!!\n");
    return;
}

void
clrerrlog (void)
{
    /* ERRLOGPTR = errlog_start;*/
    printf("%s: not supported\n", __FUNCTION__);
}

void
dumperrlog (void)
{
    printf("%s: not supported\n", __FUNCTION__);
}

/*-------------------------------------------------
 * $Log: woodlawn_linux_error.c,v $
 * Revision 1.2  2013/10/08 08:48:32  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:11  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:26  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.2  2013/03/27 08:45:06  kuangik
 * Code cleanup
 *
 * Revision 1.1  2013/03/27 07:25:33  kuangik
 * Rename ovld_xx to woodlawn_xx
 *
 * Revision 1.2  2013/03/27 04:49:37  kuangik
 * Code cleanup after adding -Wall
 *
 * Revision 1.3  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * Revision 1.1.2.2  2011/06/20 23:04:27  ptong
 * add gettestname
 *
 * Revision 1.1.2.1  2011/04/05 19:59:38  ptong
 * Initial checkin
 *
 * $Endlog$
 *-------------------------------------------------
 */
