/* $Id: libgeneric.h,v 1.3 2012/10/04 23:36:44 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libgeneric.h,v $
 *------------------------------------------------------------------
 * libgeneric.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2011 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.  This
 * copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *----------------------------------------------------------------------------
 *
 * libgeneric.h - useful utility functions for the SP2700 ARM-MP cores
 *
 *  Created on: Oct 16, 2009
 *      Author: dokim
 */

#ifndef LIBGENERIC_H_
#define LIBGENERIC_H_

#include <stdint.h>
#include <stdbool.h>

#define MPCORE_CPU0   	0
#define MPCORE_CPU1		1

#define EVNT_CNT0		0
#define EVNT_CNT1		0

#define CYCLE_CNT_DIV64			1
#define CYCLE_CNT_EVERY_CLK		0

#define INTR_ON			1
#define INTR_OFF		0

#define	ALL_BITS_ON	0xFF
#define	SELECT_ALL	0xFF
#define	PASS_VAL	1
#define	FAIL_VAL	ALL_BITS_ON

#ifndef ERROR_FATAL
#define ERROR_FATAL			0xFFFFFFFF
#define ERROR_BAD_PARAM		0xFFFFFFFE
#endif

typedef enum SP27XXEvalType {
	SP2704Eval_DSP0 = 0,
	SP2704Eval_DSP1,
	SP2704SDB_DSP0,
	SP2716_DSP0,
	SP2716_DSP1,
	SP2716_DSP2,
	SP2716_DSP3,
	UNKNOWN = 0xFFFF
} SP27XXEvalType_t;

typedef enum {
	devSP2704 = 0,
	devSP2701,
	devSP2702,
	devSP2703,
	devSP2716_D0,
	devSP2716_D1,
	devSP2716_D2,
	devSP2716_D3
}SP27XXDevId_t;

SP27XXEvalType_t				/* ret: SP2704Eval_DSP0, SP2704SDB_DSP0, etc.  */
sp_check27XXboardConfig(void);		/* determine environment device is running in */

SP27XXDevId_t					/* ret: SP2704, SP2701, etc. */
sp_check27xxDevId(void);			/* read and return device id to calling function */

uint32_t						/* ret: revision id */
sp_check27xxRevId(void);			/* read and return revision id to calling function */

uint32_t						/* ret: chip id */
sp_check27xxChipId(void);			/* read and return chip id to calling function */

uint32_t						/* ret: SP2702/SP2704 */
sp_check27xxPFUSE123(void);			/* read and return PFUSE123 to calling function */

typedef struct load_record {
	uint32_t load_address;
	uint32_t long_count;
	uint32_t data[125];
} load_rec_t;

#define DSS0RELEASE	(1<<0)
#define DSS1RELEASE	(1<<1)
#define DSS2RELEASE	(1<<2)
#define DSS3RELEASE	(1<<3)
#define DSSRESETALL	(1<<4)

#define NUMBEROFDSS	4

#if 0
void
sp_ReleaseDSS(					/* optionally load and start DSS execution after reset */
	int32_t	numrecs,			/* in: number for records to load */
	load_rec_t records[],		/* in: array of load records */
	uint32_t startAddress,		/* in: VBA register value (location of reset vector) */
	uint32_t DSSreleaseMask);	/* in: DSS subsystems to start */
#endif

void lsi_mg_delay(uint32_t count);
void lsi_mg_test_pass(void);				/* place to set BP for linear tests */
void lsi_mg_test_fail(void);				/* place to set BP for linear tests */
void lsi_mg_report(uint32_t failnumber);		/* error logging routine */

/* cpu id */
inline int 						/* ret: 0 for ARM0 or 1 for ARM1 */
sp_readMPcpuid(void);				/* ARM-MP core CPU ID */

#define sp_read_cpuid()		sp_readMPcpuid()

void
sp_ARM1release(void);				/* ONLY called by ARM0 to release ARM1 from reset at location 0 */

void
sp_ARM1reset(void);				
/* performance monitoring - arm_cpu_perf_monitor.c
 *
 * The Cycle Counter Register counts the processor clock cycles. It is a 32-bit counter that
 * can trigger an interrupt on overflow. You can use it in conjunction with the Performance
 * Monitor Control Register and the two Counter Registers to provide a variety of useful
 * metrics that enable you to optimize system performance.
 * The Cycle Counter Register can be configured to count every 64th clock cycle by the
 * Performance Monitor Control Register.
 */
inline void
sp_ConfigARMcycleCnt(			/* configure the ARM CPU cycle counter */
	int div_64,					/* in: increment on every 64th clock */
	int intr_en);				/* in: even (e.g. '0' => disable overflow interrupt,  '1' => enable */

inline void
sp_ResetARMcycleCnt(void);			/* set ARM CPU cycle counter to 0 */

/* From ARM-MP TRM, Table 3-41 Performance monitoring events
 *	Event
 *	number	Event definition
 *	0x00 	Instruction cache miss to a cachable location requires fetch from external memory.
 *	0x01 	Stall because instruction buffer cannot deliver an instruction. This can indicate an instruction cache
 *			miss or an instruction MicroTLB miss. This event occurs every cycle where the condition is present.
 *	0x02 	Stall because of a data dependency. This event occurs every cycle where the condition is present.
 *	0x03 	Instruction MicroTLB miss.
 *	0x04 	Data MicroTLB miss.
 *	0x05 	Branch instruction executed, branch might or might not have changed program flow.
 *	0x06 	Branch not predicted.
 *	0x07 	Branch mispredicted.
 *	0x08 	Instruction executed.
 *	0x09 	Folded instruction executed.
 *	0x0A 	Data cache read access, not including cache operations. This event occurs for each non-sequential
 *			access to a cache line.
 *	0x0B 	Data cache read miss, not including cache operations.
 *	0x0C 	Data cache write access.
 *	0x0D 	Data cache write miss.
 *	0x0E 	Data cache line eviction, not including cache operations.
 *	0x0F 	Software changed the PC and there is not a mode change.
 *	0x10 	Main TLB miss.
 *	0x11 	External memory request (cache refill, noncachable, write-back).
 *	0x12 	Stall because of Load Store Unit request queue being full.
 *	0x13 	The number of times the Store buffer was drained because of LSU ordering constraints or CP15 operations.
 *	0x14 	Buffered write merged in a store buffer slot.
 *	0xFF 	An increment each cycle.
 *	All other values Reserved. Unpredictable behavior.
 */

inline void
sp_ConfigARMEventCnt(			/* configure event counter */
	int cnt,					/* in: 0 => counter0, 1 => counter1 */
	int event_num,				/* in: Performance monitoring event (see above) */
	int intr_en);				/* in: even (e.g. '0' => disable overflow interrupt, '1' => enable */


inline void
sp_ResetARMEventCounters(void);	/* reset both Count Registers to 0x0. */

inline void
sp_EnableARMcounters(void);		/* enable all three counters */

inline int						/* ret: current value of ARM CPU cycle counter */
sp_ReadARMCycleCnt(void);		/* read CPU cycle counter */

inline int						/* ret: current value of selected event counter */
sp_ReadARMeventCounter(			/* read one of the CPU event counters */
	int cnt);					/* in: 0 => counter0, 1 => counter1 */

inline void
sp_LoadARMCycleCnt(				/* load ARM CPU cycle counter */
	int load);					/* in: value to load into counter */

inline void
sp_LoadARMeventCounter(			/* load selected event counter */
	int cnt,					/* in: 0 => counter0, 1 => counter1 */
	int load);					/* in: value to load into counter */


inline void
sp_init31bitCycleCounter(void);				/* set up simple 32-bit counter with roll-over */

/*
 * NOTE: moved to sp_read31bitCounter.h header file to allow proper inlining outside of library
 */
// inline int									/* ret: count of CPU cycles */
// sp_read31bitCycleCounterCountValue(void);	/* retrieve elapsed time since last call to sp_begin32bitCounterCounting() */

/* branch prediction - arm_program_flow.c */
inline void
sp_EnableProgramFlowPrediction(void);

inline void
sp_DisableProgramFlowPrediction(void);

/* functions from lsi_mg_pll.c */
extern int					/* ret: 0 => success, -1 => not locked, or parameter error indicator */
enable_pll(					/* set up PLL - usually done by PPB */
	uint32_t multiplier,	/* in: multiplier (4 .. 170) */
	uint32_t fin,			/* in: input frequency (5 < fin < 450) */
	uint32_t output_divider);	/* in: post-scale by 1, 2, 4 or 8 */

extern int					/* ret: 0 => success, -1 => not locked, or parameter error indicator */
enable_ddr3pll(				/* set up DDR3 PLL - usually done by PPB */
	uint32_t multiplier,	/* in: multiplier (4 .. 170) */
	uint32_t fin);			/* in: input frequency (5 < fin < 450) */

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * These timeout functions are only to be used from one thread / processor.
 * They use the single Real Time Counter of the sp2704 and will reset it on
 * each call. They are meant to provide a failsafe timeout capability during
 * device initialization. They also only provide support for testing a single
 * bit in a register, not the value of a multi-bit field.
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */
/* The timeoutDuringWaitForRegBitClear() and timeoutDuringWaitForRegBitSet() functions reset the
 * CAR Real Time Clock. Don't use them if RTC is used for other purposes.
 */
uint32_t					/* ret: number of sysclk cycles for RTC; 0 if timeoutINuS is out of range */
sp_timeoutValUsing32bitRTC(	/* calculate approx. RTC value for timeout in uS - max depends on PLL setting */
	uint32_t timeoutINuS,	/* in: approximate timeout in uS (best to round up); MAX = 4210752 */
	uint8_t ckiInMHz);		/* in: frequency of CKI in MHz, typically 25 or 50 */


bool								/* ret: false -> bit clear, no timeout, true -> timeout or RTC in use */
sp_timeoutDuringWaitForRegBitClear(	/* wait for timeout ticks of RTC for bit to be cleared */
	uint32_t regAddr,				/* in: register address */
	uint32_t bitMask,				/* in: bit to test for being set to '0' */
	uint32_t timeout);    			/* in: timeout in RTC clock (SYSCLK) ticks */

bool								/* ret: false -> bit set, no timeout, true -> timeout */
sp_timeoutDuringWaitForRegBitSet(	/* wait for timeout ticks of RTC for bit to be set */
	uint32_t regAddr,				/* in: register address */
	uint32_t bitMask,				/* in: bit to test for being set to '1' */
	uint32_t timeout);    			/* in: timeout in RTC clock (SYSCLK) ticks */

int32_t 							/* ret: 0 -> success, -1 -> fail */
sp_delayUsingRTC(					/* wait a specific of time (in uS) */
		uint32_t timeoutINuS,		/* in: approximate timeout in uS (best to round up) */
		uint8_t ckiInMHz);			/* in: frequency of CKI in MHz, typically 25 or 50 */

#endif /* LIBGENERIC_H_ */

/******** History ********
$Log: libgeneric.h,v $
Revision 1.3  2012/10/04 23:36:44  srane
Add support for SP2702.

Revision 1.2  2012/09/10 06:33:41  srane
Add declaraton for ARM11 CPU1 reset routine.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

