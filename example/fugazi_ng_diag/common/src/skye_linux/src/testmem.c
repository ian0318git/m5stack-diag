/* $Id: testmem.c,v 1.2 2015/05/25 03:59:21 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/testmem.c,v $ 
 *------------------------------------------------------------------
 *
 * testmem.c: Test the specified range of memory as longwords
 *
 * May 09, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Test the specified range of memory as longwords.
**
** A tmemp->flag of INDIAGS means that the test was called from
** the diagnostic menu system.
**
** The following characteristics apply:
**   if tmemp->passcount == 0     Do test forever
**   else                         Do specified number of passes
**
**   if pass == 0                 Pass number will not be printed
*/

#include "endians.h"
#include "types.h"
#include "common.h"
#include "testmem.h"
#include "nvmonvars.h" 
#include "monitor.h"
#include "error.h"
#include "proto.h"
#include <stdio.h>

/* pattern used */
unsigned long num_pattern[] = {
    0xf8ffc800f8ffc800,
    0x8fee20e88fee20e8,
    0xeed1148eeed1148e,
    0x993993cc993993cc,
    0xd5a552b2d5a552b2,
    0x0000011100000111,
};

unsigned long num_pats = (sizeof(num_pattern)/sizeof(unsigned));

static char memerrmsg[] = 
"memory test error at location 0x%06x on pass %d\n\
  expected %#.18lx  is %#.18lx\n";
static int testmemphase();

int phase;
char *memphmsg;
static uint8_t byte_pat[] = {0xAA, 0x55, 0x33, 0xCC, 0x99, 0x66};
#define NUM_BYTE_PATS  sizeof(byte_pat)/sizeof(uint8_t)

/*
 * ********************* WARNING ********************* 
 * if using this routine, memphmsg is a pointer which
 * needs to be initialized by the calling routine
 * ********************* WARNING ********************* 
 */   
/******************************************************************************
 *
 * Function: testmem
 *
 * Description: This function to test memory
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
testmem (struct testmem *tmemp)
{
    register utype_t pat, newpat;
    register utype_t temp, value, val;
    register utype_t *end, *wrptr;

    int retval = 0;  /* init */
    short i;
    long delaycnt = 0, delay;

    wrptr = tmemp->start;
    end = (utype_t *)((unsigned long)tmemp->start + tmemp->length);
    if (wrptr >= end) {
        printf("\ntest aborted, start addr of %#lx >= end addr of %#lx",
                (unsigned long) wrptr, (unsigned long) end);
        return(-1);
    }
    phase = 1;
    while(1) {
        wrptr = tmemp->start;
	temp = 0;
	prpass(testpass, "phase %d, %s  ", phase++, memphmsg ? memphmsg : "");
        /* fill entire memory with a modulo 6 pattern */
        while(wrptr < end) {
	    if(!(tmemp->flag & MUTE)) {
                if (!(temp % ONE_MEG))
                    prpass(testpass, "phase %d, fill memory at %#x,",
			    phase++, wrptr);
	    }
            *wrptr++ = num_pattern[temp % num_pats];
	    temp++;
        }

        /* do refresh test if the platform has DRAM */
        prpass(testpass, "phase %d, memory refresh test,", phase++);
        if (tmemp->flag & DRAM) {
            for(i = 0; i < 375; i++){
                delay = delaycnt;
                while(delay > 0)   delay--; /* wait for about 15 seconds */
            }
        }
        /* read_verify the memory for this pattern
	 * and write complement of the pattern
	 */
	prpass(testpass, "phase %d, %s  ", phase++, memphmsg ? memphmsg : "");
        wrptr = tmemp->start;
	temp = 0;
        while(wrptr < end) {
	    if(!(tmemp->flag & MUTE)) {
                if (!(temp % ONE_MEG))
                    prpass(testpass, "phase %d, verify %#x then wr "
                           "complement pattern,", phase++, wrptr);
	    }
	    val = num_pattern[temp % num_pats];
            if ((value = *wrptr) != val) {
                cterr('f',0, memerrmsg, wrptr, testpass, val, value);
	        retval = -1;
	        if(tmemp->flag & QUITONERR) {
		    if(!(tmemp->flag & INDIAGS)) {
		        printf("test stopped on error\n");
		    }
		    return(-1);
	        }
            }
            wrptr++;
	    temp++;
        }
	/* if abbreviated test flag is ON, don't do comprehensive 
         * memory test
         */
	if (tmemp->flag & ABBR_TEST) {
	    if(tmemp->passcount == 1){
	        break;  /* we have reached our passcount */
            } else {
               prpass(testpass, "phase %d, %s  ", phase++, 
                      memphmsg ? memphmsg : "");
            }
	    if((unsigned int)(tmemp->passcount && testpass) >= 
	       (unsigned int)(tmemp->passcount))
	        break;  /* we have reached our passcount */
	    testpass++;
        }
	/* if abbreviated test flag is OFF, do a comprehensive 
         * memory test 
         */
	if (!(tmemp->flag & ABBR_TEST)) {
	    newpat = 0xfffffffefffffffe;
	    fillword((uint *)tmemp->start, tmemp->length, newpat);
            prpass(testpass, "phase %d, walking zero test,", phase++);
	    do {  /* walking zero test */
	        pat = newpat;
	        newpat = (newpat << 1) + 1;
	        if(newpat == 0xffffffffffffffff) newpat = 0x01;
	        if(testmemphase(tmemp, pat, newpat) < 0) return(-1);
	    } while(newpat != 0x01);
            prpass(testpass, "phase %d, walking one test,", phase++);
	    do {  /* walking one test */
	        pat = newpat;
	        newpat <<= 1;
	        if(testmemphase(tmemp, pat, newpat) < 0) return (-1);
	    } while(newpat);
	    if(tmemp->passcount == 1) {  /* one pass is finished */
	        break;
	    }
	    if((unsigned int)(tmemp->passcount && testpass) >= 
	       (unsigned int)(tmemp->passcount))
	        break;  /* we have reached our passcount */
	    testpass++;
        }
    }
    return(0);
}



/******************************************************************************
 *
 * Function: testmemphase
 *
 * Description: This function to test memory by phase
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
testmemphase (tmemp, pat, newpat)
    register struct testmem *tmemp;
    register unsigned long pat, newpat;
{
    register utype_t value;
    register utype_t *wrptr, *rdptr, *end;
    int retval = 0;  /* init */
    
    if(!(tmemp->flag & MUTE)) {
	prpass(testpass, "phase %d, %s  ", phase++, memphmsg ? memphmsg : "");
    }
    end = (utype_t *)((unsigned long)tmemp->start + tmemp->length);
    wrptr = tmemp->start;
    if(tmemp->flag & RDADDR) rdptr = tmemp->rdaddr;
    else rdptr = wrptr;
    while(wrptr < end) {
	if((value = *rdptr) != pat) {  /* check location for pattern */
	    cterr('f',0, memerrmsg, rdptr, testpass, pat, value);
	    retval = -1;
	    if(tmemp->flag & QUITONERR) {
		if(!(tmemp->flag & INDIAGS)) {
		    printf("test stopped on error\n");
		}
		return(-1);
	    }
	}
	*wrptr++ = newpat;  /* write new pattern into location */
	rdptr++;
    }
    return(retval);
}

static int cmpvalue();

/******************************************************************************
 *
 * Function: memrefresh
 *
 * Description: This function to Perform memory refresh test
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int
memrefresh (struct testmem *tmemp)
{
    short i;
    long delaycnt = 0, delay;

    fillword((uint *)tmemp->start, tmemp->length, (unsigned)0x5a5aa5a5);
    for(i = 0; i < 375; i++){
	delay = delaycnt;
	while(delay > 0)   delay--; /* wait for about 15 seconds */
    }
    if(cmpvalue(tmemp->start, tmemp->length, (unsigned)0x5a5aa5a5)) {
	cterr('f',0,"system failed to refresh the memory");
	return(-1);
    } 
    else return(0);
}

/******************************************************************************
 *
 * Function: cmpvalue
 *
 * Description: This function to compare memory value
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int
cmpvalue (unsigned int *addr, unsigned int length, unsigned int val)
{
    unsigned int tmpval;
    register unsigned int *end = (unsigned int *)((unsigned long)addr + length);

    while(addr < end) {
	tmpval = *addr++;
	if (tmpval != val) return(1);
    }
    return(0);
}

/*************************************************************************
 *
 * testmem_byte - Peforms a memory test using byte access.
 *
 *************************************************************************
 */
int
testmem_byte (struct testmem *tmemp, uint8_t *force_rd_cycle)
{
    register unsigned char *end, *pat_wrptr;
    unsigned char *wrptr;
    uint32_t pat_cnt;
    uint8_t  dummy_read;

    end = (unsigned char *)((unsigned long)tmemp->start + tmemp->length);
    /*
     *  Fill memory area with Zeroes.
     */
    wrptr = (unsigned char *)tmemp->start;
    hkeepflags &=~H_INCFILL;
    filbyte(wrptr, (uint32_t)tmemp->length,0);
    /*
     *  Verify that memory area was initialized to zeroes.
     */
    while (wrptr < end ) {
        if (*wrptr++ != 0 ) {
            cterr('f',0,"%s"
                  "\nUnable to Initialize Memory space to 0"
                  "Failing address = 0x%x",memphmsg ? memphmsg : "",--wrptr);
            return(FAILED);
        }
    }

    wrptr = (unsigned char *)tmemp->start;

    while (wrptr < end ) {
        if ( *wrptr != 0 ) {
            cterr('f',0,"%s "
                  "\nRam Contents Disturbed at Addr 0x%x"
                  "\nExpected 0 Read 0x%2x",memphmsg ? memphmsg : "",
                  wrptr, *wrptr);
            return(FAILED);
        }
        pat_wrptr = wrptr;
        for (pat_cnt = 0; pat_cnt < NUM_BYTE_PATS; pat_cnt++ ) {
            *pat_wrptr = byte_pat[pat_cnt];
            if (force_rd_cycle)
                dummy_read = *force_rd_cycle;
            if (*pat_wrptr != byte_pat[pat_cnt]) {
                cterr('f',0,"%s "
                      "\nPattern Failure at addr 0x%x"
                      "\nWrote 0x%2x  Read 0x%2x",
                      memphmsg ? memphmsg : "",
                      pat_wrptr,byte_pat[pat_cnt],*pat_wrptr);
                return(FAILED);
            }
        }
        *wrptr = 0xff;
        wrptr++;
    }

    end = (unsigned char *)tmemp->start;
    wrptr = (unsigned char *)((unsigned long)tmemp->start + tmemp->length);
    wrptr--;
    while (wrptr >= end ) {
        if ( *wrptr != 0xff ) {
            cterr('f',0,"%s"
                  "\nRam Contents Disturbed at Addr 0x%x"
                  "\nExpected 0xFF Read 0x%2x",memphmsg ? memphmsg : "",
                  wrptr, *wrptr);
            return(FAILED);
        }
        pat_wrptr = wrptr;
        for (pat_cnt = 0; pat_cnt < NUM_BYTE_PATS; pat_cnt++ ) {
            *pat_wrptr = byte_pat[pat_cnt];
            if (force_rd_cycle)
                dummy_read = *force_rd_cycle;
            if (*pat_wrptr != byte_pat[pat_cnt]) {
                cterr('f',0,"%s "
                      "\nPattern Failure at addr 0x%x"
                      "\nWrote 0x%2x  Read 0x%2x",
                      memphmsg ? memphmsg : "",
                      pat_wrptr,byte_pat[pat_cnt],*pat_wrptr);
            }
        }
        *wrptr = 0;
        wrptr--;
    }

    return(PASSED);
}


/******** History *********/
/*
 *------------------------------------------------------------------
 * $Log: testmem.c,v $
 * Revision 1.2  2015/05/25 03:59:21  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/04/30 08:33:54  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:44  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:57:00  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:48  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.3  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.4.2  2013/09/13 07:00:11  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/05/09 04:19:13  iachang
 * Support memory debug utilities
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

