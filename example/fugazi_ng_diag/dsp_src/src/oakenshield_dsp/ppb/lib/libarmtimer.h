/* $Id: libarmtimer.h,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libarmtimer.h,v $
 *------------------------------------------------------------------
 * libarmtimer.h
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
 * libppbarmtimer.h - control for integrated MPCORE timers and watchdog timers
 *
 *  Created on: Sep 1, 2009
 *      Author: dokim
 */

#ifndef LIBPPBARMTIMER_H_
#define LIBPPBARMTIMER_H_

#include <stdint.h>

/* NOTE: watchdog timer can be configured as a general purpose timer also */
#define	TIMER_GENERIC		 	0
#define TIMER_WATCHDOG	 		1

#define WD_DISABLE_KEY0			0x12345678
#define WD_DISABLE_KEY1			0x87654321

#define SINGLE_SHOT				0
#define AUTO_RELOAD				1

#define NO_PRESCALE				0
#define PRESCALE(n)				(n)

/*
 * NOTE: The period between two timer expiration 'events' is calculated as:
 * 		((PRESCALER_value+1) x (Load_value+1) x 2) / CPU_CLK_frequency
 */

void
sp_InitMPtimer(				/* initialize timer_n - MUST BE called first */
	uint32_t timer_n,		/* in: TIMER_GENERIC or TIMER_WATCHDOG */
	uint32_t mode,			/* in: SINGLE_SHOT or AUTO_RELOAD */
	uint32_t pre_scale);	/* in: prescaler value */

void
sp_LoadMPtimer(			/* set Timer Load Register for timer_n */
	uint32_t timer_n,		/* in: TIMER_GENERIC or TIMER_WATCHDOG */
	uint32_t load_value);	/* in: Load_value */

void
sp_StartMPtimer(				/* start timer initialized by arm_tmr_init() */
	uint32_t timer_n);		/* in: TIMER_GENERIC or TIMER_WATCHDOG */

uint32_t					/* ret: current value of Timer Counter Register */
sp_ReadMPtimer(				/* read Timer Counter Register for timer_n */
	uint32_t timer_n);		/* in: TIMER_GENERIC or TIMER_WATCHDOG */

void
sp_StopMPtimer(				/* stop timer started by arm_tmr_start() */
	uint32_t timer_n);		/* in: TIMER_GENERIC or TIMER_WATCHDOG */

/*----------------------------------------------------------------------------
 * interrupt related functions
*/
void
sp_EnableInterruptMPtimer(				/* enable timer interrupts from timer_n */
	uint32_t timer_n);		/* in: TIMER_GENERIC or TIMER_WATCHDOG */

void
sp_DisableInterruptMPtimer(			/* disable timer interrupts from timer_n */
	uint32_t timer_n);		/* in: TIMER_GENERIC or TIMER_WATCHDOG */

void
sp_ClearInterrupMPtimer(			/* clear any pending interrupts from timer_n */
	uint32_t timer_n);		/* in: TIMER_GENERIC or TIMER_WATCHDOG */

/*----------------------------------------------------------------------------
 * watchdog only functions
*/
void
sp_EnableMPwatchDogTimer(void);		/* enable watchdog feature for TIMER_WATCHDOG */

void
sp_DisableMPwatchDogTimer(void);		/* disable watchdog feature for TIMER_WATCHDOG */

#endif /* LIBPPBARMTIMER_H_ */

/******** History ********
$Log: libarmtimer.h,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

