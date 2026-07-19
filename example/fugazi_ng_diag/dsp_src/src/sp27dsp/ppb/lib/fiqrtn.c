/* $Id: fiqrtn.c,v 1.2 2012/05/10 22:48:10 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/fiqrtn.c,v $
 *------------------------------------------------------------------
 * fiqrtn.c
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
 * Copyright (c) 2011 LSI Inc.
 * All Rights Reserved
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
 * fiqrtn.c - ARM Fast Interrupt routing routine
 * 	NOTE: placed in a separate file so it can be replaced by user version
 *
 *  Created on: Jan. 7, 2011
 *      Author: bas
 */

#include "libppbint.h"
#include "libgeneric.h"
#include <stdlib.h>
#include "lsi_sp27xx_reg.h"
#include "mpcore_dist_intrc_io.h"

ISR_FNCPTR _fiq_isr[2][LSI_SP27XX_FIQ_MAX_FIQ];

void fiqrtn()
{
	int cpu, ii, fiqStat, fiqSelect;

	if (LSI_SP27XX_IS_V10_DEVICE()) {
		lsi_mg_test_fail(); 			/* SP2704v1.0 does not support FIQ */
	}
	cpu = sp_readMPcpuid();

	if (cpu) {		/* ARM1 */
		fiqStat = HW_REG_ACCESS(LSI_SP27XX_FIQ1STAT_RA);
	} else {		/* ARM0 */
		fiqStat = HW_REG_ACCESS(LSI_SP27XX_FIQ0STAT_RA);
	}

	/* This loop implies a priority that is likely not valid for most applications */
	fiqSelect = 1;
	for (ii = 0; ii < LSI_SP27XX_FIQ_MAX_FIQ; ii++) {
		fiqSelect = 1 << ii;
		if (fiqStat & fiqSelect) {
			if(_fiq_isr[cpu][ii] != 0) {
				(*_fiq_isr[cpu][ii])();
			}
		}
		/* Interrupt is latched in RAW_STAT register which is COW1 */
		if (cpu) {		/* ARM1 */
			REG32_WRITE(LSI_SP27XX_FIQ1RAW_RA, fiqSelect);
		} else {		/* ARM0 */
			REG32_WRITE(LSI_SP27XX_FIQ0RAW_RA, fiqSelect);
		}
	}
}

uint32_t					/* ret: SUCCESS or ERROR */
sp_SetFIQ(					/* install FIQ handler (call with interrupts disabled) */
	uint32_t fiq_id,		/* in: FIQ_BIT_POS use "_BO" value (see: lsi_sp27xx_armctl.h) */
	ISR_FNCPTR isr)			/* in: pointer to interrupt service routine code */
{
	int cpu, fiqStat, fiqMask;

	if (LSI_SP27XX_IS_V10_DEVICE()) {
		lsi_mg_test_fail(); 			/* SP2704v1.0 does not support FIQ */
	}
	cpu = sp_readMPcpuid();

	/* checking input arguments */
	if (fiq_id >= LSI_SP27XX_FIQ_MAX_FIQ) {
		return ERROR;							/* out of range */
	}

	if (isr == NULL) {
		return ERROR;							/* Null ISR */
	}

	fiqMask = 1 << fiq_id;						/* calculate associated mask */

	if (cpu) {		/* ARM1 */
		fiqStat = HW_REG_ACCESS(LSI_SP27XX_FIQ1STAT_RA);
		if (fiqStat & fiqMask) {
			/* interrupt is already pending, attempt to clear it */
			REG32_WRITE(LSI_SP27XX_FIQ1RAW_RA, fiqMask);
		}

		/* connect FIQ Handler */
		_fiq_isr[cpu][fiq_id] = isr;

		/* unmask the interrupt */
		REG32_SET_BITS(LSI_SP27XX_FIQ1MASK_RA, fiqMask);
	} else {		/* ARM0 */
		fiqStat = HW_REG_ACCESS(LSI_SP27XX_FIQ0STAT_RA);
		if (fiqStat & fiqMask) {
			/* interrupt is already pending, attempt to clear it */
			REG32_WRITE(LSI_SP27XX_FIQ0RAW_RA, fiqMask);
		}

		/* connect FIQ Handler */
		_fiq_isr[cpu][fiq_id] = isr;

		/* unmask the interrupt */
		REG32_SET_BITS(LSI_SP27XX_FIQ0MASK_RA, fiqMask);
	}

	return SUCCESS;
}

/******** History ********
$Log: fiqrtn.c,v $
Revision 1.2  2012/05/10 22:48:10  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

