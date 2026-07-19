/* $Id: test_sanity.c,v 1.2 2012/07/17 20:34:22 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/src/test_sanity.c,v $
 *------------------------------------------------------------------
 * test_sanity.c
 *      test LSI StarPro 2704 DSP core basic operations
 *
 * Apr 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 *
 *------------------------------------------------------------------
 */
#include "ag_mg_regs.h"
#include "common.h"
#include "diag_common.h"
#include "diag_ppb.h"
#include <prototype.h>			/* For interrupt enable/disable */
//#include "ag_mg_dss_def.h"			
//#include "ag_mg_libsp26k.h"		/* required for ISR definitions & utils */

#define TEST_CMEM_BASE		0x002BFF00 /* 256bytes in system mem 10 */
#define TEST_CMEM_SIZE		0x00000100 

#define TEST_LMEM_BASE		0x4003FF00 /* 256bytes in local mem */
#define TEST_LMEM_SIZE		0x00000100

/* SYSMEM via SBI */
#define TEST_SMEM_BASE		0xC02BFF00
#define TEST_SMEM_SIZE		0x00000100

#define TIMER_VALUE_100MS		(97656)		/* 250 MHz/256 * .1 */
 
#define TIMER_VALUE (3 * TIMER_VALUE_100MS)

//uint32_t timer_interrupt_counter = 0;

typedef struct {
	uint32_t baseAddr;	/* start of memory block */
	uint32_t size;		/* # of byte addresses in memory block */
} memDescr_t;

const memDescr_t memTable[] = {
    { TEST_CMEM_BASE, TEST_CMEM_SIZE },
	{ TEST_LMEM_BASE, TEST_LMEM_SIZE },
	{ TEST_SMEM_BASE, TEST_SMEM_SIZE },
};

//extern volatile ag_mg_regs_dss_reg_s *dss_regs;
//extern volatile ag_mg_regs_icu_reg_s *icu_regs;
//extern volatile ag_mg_regs_car_reg_s *car_regs;
//extern volatile ag_mg_regs_timer_reg_s *timer_regs;
extern dspif_info_t *dh_if;
extern int sprintf(char *_Restrict, const char *_Restrict, ...);


static int test_instruction (void);
static int test_branch_control (void);
static int test_internal_ram (void);
//static int test_timer_interrupt (void);
static int subr_call (int); 

/***********************************************************************
 *
 * Function: test_sanity
 *
 * Description: Sanity check of DSP core's basic operations.
 *
 * Input : none
 *
 * Returns: FAILED/PASSED 
 *
 **********************************************************************
 */
UINT32 test_sanity()
{

    
    if (test_instruction()) {
		sprintf((char *)&(dh_if->errmsg), "Failed test instruction");
		return (FAILED); 
    }

    if (test_branch_control()) {
		sprintf((char *)&(dh_if->errmsg), "Failed branch instruction");
		return (FAILED); 
    }

	/* Internal memory spaces are loaded and allocated to run
	 * diagnostics FW on PPB(ARM) and DSS cores.
	 * Testing may corrupt the image and its execuation.
	 * So, test only very minimum space if possible. 
	 */
	if (test_internal_ram()) {
		sprintf((char *)&(dh_if->errmsg), "Failed internal ram ");
		return (FAILED); 
    }
   
#if 0
	if (test_timer_interrupt()){
		sprintf((char *)&(dh_if->errmsg), "Failed interrupt ");
		return (FAILED);
	}
#endif

	sprintf((char *)&(dh_if->errmsg), "Sanity test passed");
    return (PASSED);
}

/* 
 ******************************************************************* 
 * test_instruction 
 *		Run basic instrcution test.
 * input: none
 * output: passed/failed
 * 
 *******************************************************************
 */ 
static int test_instruction (void)
{ 
    UINT32 tmp1, tmp2, tmp3; 


     /* cpu phase */ 
    /* instruction test */ 
    tmp1 = 0xffffffff;                 /* load immediate */ 
    if (tmp1 != 0xffffffff) { 
		return (FAILED); 
    } 
    tmp2 = tmp1;                       /* copy memory to memory */ 
    if (tmp2 != tmp1) { 
		return (FAILED); 
    } 
    tmp3 = tmp2 - tmp1;                /* subtraction */ 
    if (tmp3 != 0) { 
		return (FAILED); 
     } 
    tmp2++;                            /* addition */ 
    tmp3--;                            /* subtraction */ 
    if (tmp2 != 0) { 
		return (FAILED); 
    } 
    if (tmp3 != 0xffffffff) { 
		return (FAILED); 
    } 
    tmp2 *= 2;                         /* multiplication */ 
    if (tmp2 != 0) { 
		return (FAILED); 
    } 
    tmp2 = 65535 * 2;                  /* multiplication */ 
    if (tmp2 != 131070) { 
		return (FAILED); 
    } 
    tmp2 /= 4;                         /* division */ 
    if (tmp2 != 32767) { 
		return (FAILED); 
    } 
    tmp2 &= 0x707;                     /* logical AND */ 
    if (tmp2 != 0x707) { 
		return (FAILED); 
    } 
    tmp2 |= 0xf0;                      /* logical OR */ 
    if (tmp2 != 0x7f7) { 
		return (FAILED); 
    } 
    return (PASSED); 
} 

/* 
 *******************************************************************
 * test_branch_control 
 * 		test branch operation
 * 
 * input: none
 * output: passed/failed
 *******************************************************************
 */ 
static int test_branch_control (void)
{ 
    int call_parm; 

    /* control test */ 
    call_parm = 0x70f0; 
    call_parm = subr_call(call_parm);  /* subroutine branch */ 
    if (call_parm != 0xf0f) { 
		return (FAILED); 
    }   
    return (PASSED); 
} 

/* 
 *******************************************************************
 * test_internal_ram
 * Test some unused internal memory.  Unused memory is any internal memory 
 * that does not contain memory-mapped registers, the 
 * stack, or any resident program code. In LSI case, ålmost all memory spaces 
 * have been allocated by ARM and DSS cores.
 * 
 * input: none
 * output: passed/failed
 *******************************************************************
 */ 
int test_internal_ram(void)
{ 
volatile uint32_t *memptr;
int32_t ii;
int32_t counter = 0;

	for (ii = 0; ii < (sizeof(memTable)/sizeof(memDescr_t)); ii++) {
		memptr = ((uint32_t *) memTable[ii].baseAddr);
		while (memptr < (uint32_t *) (memTable[ii].baseAddr + memTable[ii].size)) {
			*memptr = (uint32_t) memptr;
			if ((uint32_t) memptr != (uint32_t) *memptr) {
				counter++;
			}
			memptr++;
		}
		if (counter > 0) {
			return (FAILED);
		}
	}

    return (PASSED); 
} 


#if 0
/* 
 *******************************************************************
 * Timer_0_isr
 * Replace Timer0 Interrupt Service Routine for interrupt test
 * input: none
 * output: none
 *******************************************************************
 */ 
void Timer_0_isr()
{
	/* Timer 0 interrupt avtive */
	if (timer_regs->timer[0].intmis.fields.mis)
	{
		/* Clear interrupt status */
		timer_regs->timer[0].intclr.fields.intclr = 1;
		timer_interrupt_counter++;
	}
}

/* 
 *******************************************************************
 * test_timer_interrupt
 * Turn on timer 0 and check it isr is called by checking counter.
 * Ported from LSI interrupt test code.
 * input: none
 * output: passed/failed
 *******************************************************************
 */ 
int test_timer_interrupt(void)
{
	timer_interrupt_counter = 0;

	di();		// disable core interrupts - see prototype.h
	/* clear any pending core ints */
	dss_regs->clearint0.reg = 0xFFFFFFFF;
	dss_regs->clearint1.reg = 0xFFFFFFFF;
	dss_regs->clearint2.reg = 0xFFFFFFFF;

	/***************************************/
	/* enable clocks to timers, SYSCLK/2   */
	/* one timer tick is 4 DSP core cycles */
	/***************************************/
	dss_regs->sysconfg.fields.tim0div = AGR_SP26XX_DSS_SYSCONFG_Div2;
	dss_regs->sysconfg.fields.timen = 1;

	/* set the interrupt priority for Timer0 to 4 */
	icu_regs->mipl[Timer_0_offset].fields.ipl = 4;
	icu_regs->mipl[Combined_Timer_offset].fields.ipl = 4;
	
	/* enable both timer interrupts & common interrupt to core */
	dss_regs->maskstat0.fields.timmsk = 7;
	timer_regs->timer[0].ctrl.reg = TIMER_ENABLE | TIMER_PERIODIC_MOD |
		TIMER_INT_ENABLE | TIMERPRE_DIV256 | TIMER_32_COUNTER |
		TIMER_WRAPPING_MODE ;
	/* load the countdown value */
	timer_regs->timer[0].load.reg = TIMER_VALUE;		/* Max Value */

	ei();			/* enable core interrupts - see prototype.h */

	while (timer_interrupt_counter == 0) {
		/* If interrupt has not happened and
		 * timer current value (count down) 
		 * is 0.
		 */
		if (timer_regs->timer[0].val.reg  == 0) {
			break;
		}
	}	

	di();

	if (timer_interrupt_counter) {
		return (PASSED);
	} else {
		return (FAILED);
	}
}
#endif

/* 
 *******************************************************************
 * subr_call 
 *		Test subroutine call operation
 * input: pass in value
 * outout: return value 
 * 
 *******************************************************************
 */ 
 
static int subr_call (int call_var) 
{ 
    if ((call_var - 0x70f0) != 0) {    /* conditional branch */ 
        call_var = 0xffff; 
    } else { 
        call_var = 0xf0f; 
    } 
 
    if (call_var < 0) {                /* branch ge zero */ 
        call_var = 0x1; 
    } else if (call_var <= 0) {        /* branch gt zero */ 
        call_var = 0x2; 
    } 
    return (call_var); 
}

/* 
 * $Log: test_sanity.c,v $
 * Revision 1.2  2012/07/17 20:34:22  srane
 * cleanup
 *
 * Revision 1.1  2012/05/31 06:36:44  srane
 * Initial checkin.
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */
