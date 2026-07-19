/* $Id: libavsc.c,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libavsc.c,v $
 *------------------------------------------------------------------
 * libavsc.c
 *
 * Sep 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2012 LSI Inc.  All Rights Reserved
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
 * $RCSfile: libavsc.c - setup, load, and enable AVS for closed loop operation
 *
 * enable_avs() function loads AVSC RAM image into AVS Controller and enables it.
 *
 * The Adaptive Voltage Scaling Controller (AVSC) uses on-chip ring oscillators
 * and delay lines to monitor changes in device characteristics because of
 * fluctuations in voltage and temperature. The AVSC interprets the data from
 * this logic and uses it to control a pulse width modulated (PWM) serial output
 * voltage control signal. This PWM signal is output on the AVS_VID[0] pin, and
 * it can be used to control the voltage adjust pin of an external regulator.
 *
 * NOTE: Proper use of the AVSC requires circuitry external to the SP2704/SP2716.
 * So discussion of a PWM signal duty cycle corresponding to a voltage is only
 * valid for a particular board/implementation.
 *
 *****************************************************************************/

#include "lsi_sp27xx_reg.h"

#include "lsi_sp27xx_avsc.h"
#include "libavsc.h"

#include "avsc_closedloop_ramimage24AprV1.3.h"

avscStat_t						/* ret: AVSC_SUCCESS, else error indication */
enable_avs(						/* setup, load, and enable AVS for closed loop operation */
	avscCLparam_t avscParam)	/* in: user-supplied parameters (see libavsc.h) */
{
	uint32_t *p_avs, *p_image, ctl3 = 0;
	int ii, avscImageSize, updateRate;
	avscStat_t rtnVal = AVSC_SUCCESS;

	p_image = (uint32_t *) avs_ram_closed_25_50;
	avscImageSize = sizeof(avs_ram_closed_25_50)/sizeof(avs_ram_closed_25_50[0]);

	switch (avscParam.ckiFreqInMHz) {
	case 19:
		ctl3 |= LSI_SP27XX_AVSC_CTL3_CKI19_44MHZ_BM;
		break;
	case 25:
		ctl3 |= LSI_SP27XX_AVSC_CTL3_HALF_CLK_MODE_BM;
		break;
	case 50:
		break;
	default:
		rtnVal = AVSC_BAD_PARAM;
	}

	switch (avscParam.mcmSlaveFailmode ) {
	case useRemainingSlaves:
		ctl3 |= LSI_SP27XX_AVSC_CTL3_MCM_SLAVE_FAILURE_MODE_BM;
		break;
	case defaultToVDDmax:
		break;
	default:
		rtnVal = AVSC_BAD_PARAM;
	}

	if ((avscParam.csmStopPoint < 4) || (avscParam.csmStopPoint > 0x28)) {
		rtnVal = AVSC_BAD_PARAM;
	}

	if (avscParam.csmUpdateRate > 1320) {
		rtnVal = AVSC_BAD_PARAM;
	}

	if ( (HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1CTL_RA) & LSI_SP27XX_PLL1CTL_PLL1BYP_BM) == 0) {
		rtnVal = AVSC_PLL_NOT_LOCKED;
	}

	if (rtnVal == AVSC_SUCCESS) {
		/* allow write access to AVSC registers */
		REG32_WRITE(LSI_SP27XX_AVSC_AVSKEY_RA, LSI_SP27XX_AVSC_AVSKEY_AVSKEY_VAL);

		/* reset AVS controller */
		REG32_WRITE(LSI_SP27XX_AVSC_CTL1_RA, LSI_SP27XX_AVSC_CTL1_AVSRST_BM);

		/* Allow RAM to be used */
		REG32_WRITE(LSI_SP27XX_AVSC_CTL0_RA, LSI_SP27XX_AVSC_CTL0_RAMEN_BM);

		p_avs = (uint32_t *) LSI_SP27XX_AVSC_RAM_BASE;
		for(ii = 0; ii < avscImageSize; ii++) {
			*p_avs++ = *p_image++;
		}

		/* coarse tuning using ring oscillators is not recommended for VDD < 1.0 V */
		ctl3 |= LSI_SP27XX_AVSC_CTL3_BYP_COARSE_BM;				/* use CSM not ring oscillators */
		ctl3 |= LSI_SP27XX_AVSC_CTL3_USE_ADAPTIVE_TURBO_BM;

		ctl3 |= LSI_SP27XX_AVSC_CTL3_FINE_TUNE_EN_BM;

		/* The CSM update rate is the delay from change in duty cycle until the AVS takes
		 * another measurement. This provides the voltage controller enough time to make a
		 * change to the device's supply voltage in response to the PWM change.
		 *
		 * Possible values (in seconds) are:
		 *		Fine Tune mode: 0.02, 0.04, 0.08, 0.16 and 0.32 seconds
		 *		Normal mode: 1.02, 1.04, 1.08, 1.16 and 1.32 seconds
		 */
		updateRate = avscParam.csmUpdateRate;
		if (updateRate >= 320) {
			/* Normal mode */
			updateRate -= 1000;
		} else {
			/* Fine tune mode */
			ctl3 |= LSI_SP27XX_AVSC_CTL3_FINE_TUNE_EN_BM;
			ctl3 |= LSI_SP27XX_AVSC_CTL3_FORCE_FINE_TUNE_BM;
		}

		if (updateRate <= 20) {
			/* use default of 20 mSec - no action required */
		} else {
			/* use value in INCR_DELAY field (bits 17:16 of AVS_CTRL3 */
			ctl3 |= LSI_SP27XX_AVSC_CTL3_INCR_DELAY_SEL_BM;
			if (updateRate <= 40) {
				ctl3 |= LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL40MILLISEC;
			} else if (updateRate <= 80) {
				ctl3 |= LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL80MILLISEC;
			} else if (updateRate <= 160) {
				ctl3 |= LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL160MILLISEC;
			} else if (updateRate <= 320) {
				ctl3 |= LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL320MILLISEC;
			}
		}

		if (avscParam.csmStopPoint != 7) {
			ctl3 |= LSI_SP27XX_AVSC_CTL3_CSM_STOP_PT_EN_BM;
			ctl3 |= avscParam.csmStopPoint << LSI_SP27XX_AVSC_CTL3_CSM_STOP_PT_BO;
		}

		REG32_SET_BITS(LSI_SP27XX_AVSC_CTL3_RA, ctl3);

		/* release AVS Controller */
		REG32_WRITE(LSI_SP27XX_AVSC_CTL1_RA, 0);

		/* deny write access to AVSC registers */
		REG32_WRITE(LSI_SP27XX_AVSC_AVSKEY_RA, 0);
	}

	return(rtnVal);
}


void reset_avs (void)
{
    /* allow write access to AVSC registers */
    REG32_WRITE(LSI_SP27XX_AVSC_AVSKEY_RA, LSI_SP27XX_AVSC_AVSKEY_AVSKEY_VAL);

    /* reset AVS controller */
    REG32_WRITE(LSI_SP27XX_AVSC_CTL1_RA, LSI_SP27XX_AVSC_CTL1_AVSRST_BM);
    /* deny write access to AVSC registers */
    REG32_WRITE(LSI_SP27XX_AVSC_AVSKEY_RA, 0);
}

/******** History ********
$Log: libavsc.c,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:34  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/09/10 06:07:50  srane
Initial commit for AVS code.


$Endlog$
*/
