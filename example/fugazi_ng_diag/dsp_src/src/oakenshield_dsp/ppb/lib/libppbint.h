/* $Id: libppbint.h,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libppbint.h,v $
 *------------------------------------------------------------------
 * libppbint.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/******************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
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
 * libppbint.h - control the MP Core Distributed Interrupt Controller
 *
 *  Created on: Aug 26, 2009
 *      Author: dokim
 */

#ifndef LIBPPBINT_H_
#define LIBPPBINT_H_

#include <stdint.h>

/* H/W interrupt numbers
 * From: "ARM11 MPCore Processor Technical Reference Manual" DDI0360F_arm11_mpcore_r2p0_trm.pdf
 * Section 10.3 Interrupt Distributor - Hardware interrupts:
 * Hardware interrupts are triggered by programmable events on associated
 * interrupt input lines. MP11 CPUs can support up to 224 interrupt input lines.
 * The interrupt input lines can be configured to be edge sensitive (posedge) or
 * level sensitive (high level). Hardware interrupts start at ID32.
 */

/* In SP2700, there are 128 HW interrupts 128 + 32 = 160 */
#define MAX_NUM_INT_ID		160

/* HW Interrupt Numbers - MPCore Dist. Intr. Controller uses HW Interrupt IDs */

/* MPCORE internal interrupt Numbers
 * Each MP11 CPU has its own private timer and watchdog that can generate
 * interrupts, using ID29 and ID30.
 */

/* soft interrupts */
#define INT_ARM_IPI(n)						(-32+n)

#define INT_ARM_TIMER						-3
#define INT_ARM_WD_TIMER					-2
#define INT_nIRQ							-1 /* not used */

/*
 * External interrupt numbers from the "PPB Input Interrupt Table" (for ID, add 32)
 */
#define INT_TXD0_EXC						0
#define INT_TXD0_EOF	 					1
#define INT_TXD1_EXC 						2
#define INT_TXD1_EOF 						3

#define INT_PCE0_EXC 						4
#define INT_PCE0_EOF0 						5
#define INT_PCE0_EOF1 						6

#define INT_PCE1_EXC 						7
#define INT_PCE1_EOF0 						8
#define INT_PCE1_EOF1 						9

#define INT_MAC0STAT						10
#define INT_MAC0LINK						11
#define INT_MAC1STAT						12
#define INT_MAC1LINK						13

#define INT_MDIO							14

#define INT_DBM_DMAC0_CH0					15
#define INT_DBM_DMAC0_CH1					16
#define INT_DBM_DMAC0_ERR					17

#define INT_DBM_DMAC1_CH0					18
#define INT_DBM_DMAC1_CH1					19
#define INT_DBM_DMAC1_ERR					20

#define INT_GLOBAL_TIMER0					21
#define INT_GLOBAL_TIMER1					22

#define INT_PEI								23

#define INT_SRIO0							24
#define INT_SRIO1							25

#define INT_TRNG							26

#define INT_NAND_FLASH						27

#define INT_TDM								28

#define INT_DSS0_TO_ARM0_0					29
#define INT_DSS0_TO_ARM0_1					30
#define INT_DSS0_TO_ARM0_2					31
#define INT_DSS0_TO_ARM1_0					32
#define INT_DSS0_TO_ARM1_1					33
#define INT_DSS0_TO_ARM1_2					34
#define INT_DSS0_WD							35

#define INT_DSS1_TO_ARM0_0					36
#define INT_DSS1_TO_ARM0_1					37
#define INT_DSS1_TO_ARM0_2					38
#define INT_DSS1_TO_ARM1_0					39
#define INT_DSS1_TO_ARM1_1					40
#define INT_DSS1_TO_ARM1_2					41
#define INT_DSS1_WD							42

#define INT_DSS2_TO_ARM0_0					43
#define INT_DSS2_TO_ARM0_1					44
#define INT_DSS2_TO_ARM0_2					45
#define INT_DSS2_TO_ARM1_0					46
#define INT_DSS2_TO_ARM1_1					47
#define INT_DSS2_TO_ARM1_2					48
#define INT_DSS2_WD							49

#define INT_DSS3_TO_ARM0_0					50
#define INT_DSS3_TO_ARM0_1					51
#define INT_DSS3_TO_ARM0_2					52
#define INT_DSS3_TO_ARM1_0					53
#define INT_DSS3_TO_ARM1_1					54
#define INT_DSS3_TO_ARM1_2					55
#define INT_DSS3_WD							56

#define INT_AVSC_TURBO_MODE_DISABLED		57
#define INT_AVSC0							INT_AVSC_TURBO_MODE_DISABLED
/* unused in SP2704 / SP2716
#define INT_AVSC1							58
#define INT_AVSC2							59
#define INT_AVSC3							60
#define INT_AVSC4							61
#define INT_AVSC5							62
#define INT_AVSC6							63
#define INT_AVSC7							64
*/
#define INT_AVSC_WD							65

#define INT_GPIO0							66
#define INT_GPIO1							67
#define INT_GPIO2							68
#define INT_GPIO_COMB						69

#define INT_DBM_ECC							70
#define INT_DDR3							72
#define INT_SSP								73
#define INT_UART							74

#define INT_DBM_SEMS0						75
#define INT_DBM_SEMS1						76

#define INT_ERR_ARM_SCU_TAG_PARITY			110

#define INT_ERR_ARM0_DATA_CACHE_PARITY		112
#define INT_ERR_ARM0_INST_CACHE_PARITY		113
#define INT_ERR_ARM0_DATA_TAG_PARITY		114
#define INT_ERR_ARM0_INST_TAG_PARITY		115
#define INT_ERR_ARM0_DIRTY_PARITY			116
#define INT_ERR_ARM0_TRACE_PARITY			117
#define INT_ERR_ARM0_TLB_PARITY				118

#define INT_ERR_PPBMEM_MULT_ECC				119

#define INT_ERR_ARM1_DATA_CACHE_PARITY		120
#define INT_ERR_ARM1_INST_CACHE_PARITY		121
#define INT_ERR_ARM1_DATA_TAG_PARITY		122
#define INT_ERR_ARM1_INST_TAG_PARITY		123
#define INT_ERR_ARM1_DIRTY_PARITY			124
#define INT_ERR_ARM1_TRACE_PARITY			125
#define INT_ERR_ARM1_TLB_PARITY				126

#define INT_ERR_PPBMEM_SINGLE_ECC			127

/* H/W interrupts' types */

#define PULSE_TRIGGERED						0
#define EDGE_TRIGGERED						1
#define LEVEL_TRIGGERED						0

/* MPCORE internal interrupts */
#define TYPE_INT_ARM_IPI				    EDGE_TRIGGERED
#define TYPE_INT_ARM_TIMER					EDGE_TRIGGERED
#define TYPE_INT_ARM_WD_TIMER				EDGE_TRIGGERED
#define TYPE_INT_nIRQ						/* not used */
/******************************/

#define TYPE_INT_TXD0_EXC					EDGE_TRIGGERED
#define TYPE_INT_TXD0_EOF	 				EDGE_TRIGGERED
#define TYPE_INT_TXD1_EXC 					EDGE_TRIGGERED
#define TYPE_INT_TXD1_EOF 					EDGE_TRIGGERED

#define TYPE_INT_PCE0_EXC 					EDGE_TRIGGERED
#define TYPE_INT_PCE0_EOF0 					EDGE_TRIGGERED
#define TYPE_INT_PCE0_EOF1 					EDGE_TRIGGERED

#define TYPE_INT_PCE1_EXC 					EDGE_TRIGGERED
#define TYPE_INT_PCE1_EOF0 					EDGE_TRIGGERED
#define TYPE_INT_PCE1_EOF1 					EDGE_TRIGGERED

#define TYPE_INT_MAC0STAT					LEVEL_TRIGGERED
#define TYPE_INT_MAC0LINK					LEVEL_TRIGGERED
#define TYPE_INT_MAC1STAT					LEVEL_TRIGGERED
#define TYPE_INT_MAC1LINK					LEVEL_TRIGGERED

#define TYPE_INT_MDIO						PULSE_TRIGGERED

#define TYPE_INT_DBM_DMAC0_CH0				PULSE_TRIGGERED
#define TYPE_INT_DBM_DMAC0_CH1				PULSE_TRIGGERED
#define TYPE_INT_DBM_DMAC0_ERR				PULSE_TRIGGERED

#define TYPE_INT_DBM_DMAC1_CH0				PULSE_TRIGGERED
#define TYPE_INT_DBM_DMAC1_CH1				PULSE_TRIGGERED
#define TYPE_INT_DBM_DMAC1_ERR				PULSE_TRIGGERED

#define TYPE_INT_GLOBAL_TIMER0				PULSE_TRIGGERED
#define TYPE_INT_GLOBAL_TIMER1				PULSE_TRIGGERED

#define TYPE_INT_PEI						LEVEL_TRIGGERED

#define TYPE_INT_SRIO0						LEVEL_TRIGGERED
#define TYPE_INT_SRIO1						LEVEL_TRIGGERED

#define TYPE_INT_TRNG						LEVEL_TRIGGERED

#define TYPE_INT_NAND_FLASH					LEVEL_TRIGGERED

#define TYPE_INT_TDM						PULSE_TRIGGERED

#define TYPE_INT_DSS0_TO_ARM0_0				EDGE_TRIGGERED
#define TYPE_INT_DSS0_TO_ARM0_1				EDGE_TRIGGERED
#define TYPE_INT_DSS0_TO_ARM0_2				EDGE_TRIGGERED
#define TYPE_INT_DSS0_TO_ARM1_0				EDGE_TRIGGERED
#define TYPE_INT_DSS0_TO_ARM1_1				EDGE_TRIGGERED
#define TYPE_INT_DSS0_TO_ARM1_2				EDGE_TRIGGERED
#define TYPE_INT_DSS0_WD					EDGE_TRIGGERED

#define TYPE_INT_DSS1_TO_ARM0_0				EDGE_TRIGGERED
#define TYPE_INT_DSS1_TO_ARM0_1				EDGE_TRIGGERED
#define TYPE_INT_DSS1_TO_ARM0_2				EDGE_TRIGGERED
#define TYPE_INT_DSS1_TO_ARM1_0				EDGE_TRIGGERED
#define TYPE_INT_DSS1_TO_ARM1_1				EDGE_TRIGGERED
#define TYPE_INT_DSS1_TO_ARM1_2				EDGE_TRIGGERED
#define TYPE_INT_DSS1_WD					EDGE_TRIGGERED

#define TYPE_INT_DSS2_TO_ARM0_0				EDGE_TRIGGERED
#define TYPE_INT_DSS2_TO_ARM0_1				EDGE_TRIGGERED
#define TYPE_INT_DSS2_TO_ARM0_2				EDGE_TRIGGERED
#define TYPE_INT_DSS2_TO_ARM1_0				EDGE_TRIGGERED
#define TYPE_INT_DSS2_TO_ARM1_1				EDGE_TRIGGERED
#define TYPE_INT_DSS2_TO_ARM1_2				EDGE_TRIGGERED
#define TYPE_INT_DSS2_WD					EDGE_TRIGGERED

#define TYPE_INT_DSS3_TO_ARM0_0				EDGE_TRIGGERED
#define TYPE_INT_DSS3_TO_ARM0_1				EDGE_TRIGGERED
#define TYPE_INT_DSS3_TO_ARM0_2				EDGE_TRIGGERED
#define TYPE_INT_DSS3_TO_ARM1_0				EDGE_TRIGGERED
#define TYPE_INT_DSS3_TO_ARM1_1				EDGE_TRIGGERED
#define TYPE_INT_DSS3_TO_ARM1_2				EDGE_TRIGGERED
#define TYPE_INT_DSS3_WD					EDGE_TRIGGERED

#define TYPE_INT_AVSC0						PULSE_TRIGGERED
#define TYPE_INT_AVSC1						PULSE_TRIGGERED
#define TYPE_INT_AVSC2						PULSE_TRIGGERED
#define TYPE_INT_AVSC3						PULSE_TRIGGERED
#define TYPE_INT_AVSC4						PULSE_TRIGGERED
#define TYPE_INT_AVSC5						PULSE_TRIGGERED
#define TYPE_INT_AVSC6						PULSE_TRIGGERED
#define TYPE_INT_AVSC7						PULSE_TRIGGERED

#define TYPE_INT_AVSC_WD					PULSE_TRIGGERED

#define TYPE_INT_GPIO0						LEVEL_TRIGGERED
#define TYPE_INT_GPIO1						LEVEL_TRIGGERED
#define TYPE_INT_GPIO2						LEVEL_TRIGGERED
#define TYPE_INT_GPIO_COMB					LEVEL_TRIGGERED

#define TYPE_INT_DBM_ECC					PULSE_TRIGGERED
#define TYPE_INT_DDR3						LEVEL_TRIGGERED
#define TYPE_INT_SSP						LEVEL_TRIGGERED
#define TYPE_INT_UART						LEVEL_TRIGGERED

#define TYPE_INT_DBM_SEMS0					LEVEL_TRIGGERED
#define TYPE_INT_DBM_SEMS1					LEVEL_TRIGGERED

#define TYPE_INT_ERR_ARM_SCU_TAG_PARITY		PULSE_TRIGGERED

#define TYPE_INT_ERR_ARM0_DATA_CACHE_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_INST_CACHE_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_DATA_TAG_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_INST_TAG_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_DIRTY_PARITY		PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_TRACE_PARITY		PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_TLB_PARITY		PULSE_TRIGGERED

#define TYPE_INT_ERR_PPBMEM_MULT_ECC		PULSE_TRIGGERED

#define TYPE_INT_ERR_ARM1_DATA_CACHE_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM1_INST_CACHE_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM1_DATA_TAG_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM1_INST_TAG_PARITY	PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_DIRTY_PARITY		PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_TRACE_PARITY		PULSE_TRIGGERED
#define TYPE_INT_ERR_ARM0_TLB_PARITY		PULSE_TRIGGERED

#define TYPE_INT_ERR_PPBMEM_SINGLE_ECC		PULSE_TRIGGERED

#define HW_INT_NUM_TO_ID(num)	(num+32)
#define HW_INT_ID_TO_NUM(id)	(id-32)

#define PRIORITY0	0x0		/* Highest Priority */
#define PRIORITY1	0x1
#define PRIORITY2	0x2
#define PRIORITY3	0x3
#define PRIORITY4	0x4
#define PRIORITY5	0x5
#define PRIORITY6	0x6
#define PRIORITY7	0x7
#define PRIORITY8	0x8
#define PRIORITY9	0x9
#define PRIORITY10	0xA
#define PRIORITY11	0xB
#define PRIORITY12	0xC
#define PRIORITY13	0xD
#define PRIORITY14	0xE
#define PRIORITY15	0xF		/* Lowest Priority */

typedef void (*ISR_FNCPTR)(void);

extern ISR_FNCPTR _intr_isr[2][160];

extern ISR_FNCPTR _fiq_isr[2][32];


void fiqrtn(void);
void errrtn(void);
void irqrtn(void);

/* Call intr_init() routine after all intr_int_set() calls */
void
sp_InitInterruptController(void);	/* must be called once at startup with interrupts disabled */

uint32_t					/* ret: SUCCESS or ERROR */
sp_SetInterrupt(			/* install interrupt handler (call with interrupts disabled) */
	uint32_t int_id,		/* in: use HW_INT_NUM_TO_ID(intNum) INT_<name> from list */
	uint32_t detect_type,	/* in: TYPE_INT_<name> from list */
	uint32_t priority,		/* in: PRIORITY0 (highest) to PRIORITY15 (lowest) */
	ISR_FNCPTR isr);		/* in: pointer to interrupt service routine code */

uint32_t					/* ret: SUCCESS or ERROR */
sp_ResetInterrupt(			/* remove interrupt handler */
	uint32_t int_id);		/* in: use HW_INT_NUM_TO_ID(intNum) INT_<name> from list */

uint32_t					/* ret: SUCCESS or ERROR */
sp_ClearInterrupt(			/* clear any pending interrupt status for int_id */
	uint32_t int_id);		/* in: use HW_INT_NUM_TO_ID(intNum) INT_<name> from list */


void
sp_GenSWInterrupt(			/* force specified interrupt in software */
	uint32_t sw_int_id);	/* in: ID to set (0 .. 160) */

void
sp_InterruptDSS(			/* generate GPINT0 or GPINT1 to dss_core_id */
	uint32_t dss_core_id,	/* in: 0 .. 3 for DSS0 .. DSS3 */
	uint32_t intr_num);		/* in: 0 or 1 for GPINT0 and GPINT1 */

uint32_t					/* ret: SUCCESS or ERROR */
sp_SetFIQ(					/* install FIQ handler (call with interrupts disabled) */
	uint32_t fiq_id,		/* in: FIQ_BIT_POS (see: lsi_sp27xx_armctl.h) */
	ISR_FNCPTR isr);		/* in: pointer to interrupt service routine code */



/* temporarily disable interrupts */
#define disable_interrupts() __asm__ volatile ( "cpsid if\n"  )

/* enable temporarily disabled interrupts */
#define enable_interrupts()  __asm__ volatile ( "cpsie if\n" )

#endif /* LIBPPBINT_H_ */

/******** History ********
$Log: libppbint.h,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:35  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

