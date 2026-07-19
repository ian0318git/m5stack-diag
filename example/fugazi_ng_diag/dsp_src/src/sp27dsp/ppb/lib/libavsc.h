/* $Id: libavsc.h,v 1.1 2012/09/10 06:07:50 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/libavsc.h,v $
 *------------------------------------------------------------------
 * libavsc.h
 *
 * Sep 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
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
 * $RCSfile: libavsc.h,v $
 *
 * The Adaptive Voltage Scaling Controller (AVSC) uses on-chip ring oscillators
 * and delay lines to monitor changes in device characteristics because of
 * fluctuations in voltage and temperature. The AVS controller interprets the
 * data from this logic and uses it to control a pulse width modulated (PWM)
 * serial output voltage control signal. This PWM signal is output on the
 * AVS_VID[0] pin, and it can be used to control the voltage adjust pin of an
 * external regulator.
 *
 * Main program Loads AVSC RAM image into AVS Controller and enables it.
 * Options are provided to enable open-loop test mode so user can select the PWM
 * duty cycle to be presented on the AVS_VID[0] pin.
 *
 * NOTE: Proper use of the AVSC requires circuitry external to the SP2704/SP2716.
 * So discussion of a PWM signal duty cycle corresponding to a voltage is only
 * valid for a particular board/implementation.
 *
 * On the SP2704 Software Development Board, a voltmeter can be used to observe
 * this change in voltage. Monitor the VDD (TP4) before the AVS code is run and
 * watch the voltage decrease from the default .93 to a lower (device dependent)
 * voltage as the AVS controller is enabled.
 *
 * NOTE: when using open loop mode on the SDB,
 * 	 a 0% duty cycle (Low DC level on AVS_VID[0] pin) corresponds to VDDmax (~0.93 V)
 *   a 100% duty cycle (High DC level on AVS_VID[0] pin) corresponds to VDDmin (~0.81 V)
 *
 * ARM spins blinking LEDs after enabling AVSC.
 *
 *****************************************************************************/

#ifndef LIBAVSC_H_
#define LIBAVSC_H_

#include <stdint.h>

typedef enum AVSC_MODE {
	closedLoop = 0,
	openLoop = 1
} avscMode_t ;

typedef enum AVSC_MCM_SLAVE_FAIL_MODE {
	defaultToVDDmax = 0,
	useRemainingSlaves = 1
} avscMCMslaveFailMode_t ;

typedef enum AVSC_STATUS {
	AVSC_SUCCESS = 0,
	AVSC_BAD_PARAM,
	AVSC_PLL_NOT_LOCKED
} avscStat_t ;

typedef struct AVSC_CLOSED_LOOP_PARAM {
	uint8_t ckiFreqInMHz;		/* 19 (actually 19.44), 25 or 50 */
	uint8_t csmStopPoint;		/* minimum 4, maximum 0x28, hardware default is 7 */
	uint16_t csmUpdateRate;		/* in milliseconds 20, 40, 80, ... , 1160, 1320 */
	avscMCMslaveFailMode_t mcmSlaveFailmode;	/* defaultToVDDmax or useRemainingSlaves */
} avscCLparam_t ;

typedef struct AVSC_OPEN_LOOP_PARAM {
	uint8_t ckiFreqInMHz;		/* 19 (actually 19.44), 25 or 50 */
	uint8_t csmStopPoint;		/* minimum 4, maximum 0x28, hardware default is 7 */
	uint16_t csmUpdateRate;		/* in milliseconds 20, 40, 80, ... , 1160, 1320 */
	avscMCMslaveFailMode_t mcmSlaveFailmode;	/* defaultToVDDmax or useRemainingSlaves */
	uint16_t vidPinOverrideVal;	/* OL Mode only: LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_97_66KHZ, etc. */
	uint8_t olDutyCycle;		/* OL Mode only: PWM duty cycle value: 0 to 100 percent */
} avscOLparam_t ;

extern const char* avs_ram_closed_loop_25_50_version;

avscStat_t						/* ret: AVSC_SUCCESS, else error indication */
enable_avs(						/* setup, load, and enable AVS for closed loop operation */
	avscCLparam_t avscParam);	/* in: user-supplied parameters (see libavsc.h) */

avscStat_t						/* ret: AVSC_SUCCESS, else error indication */
update_avs_openloopDutyCycle(	/* set duty cycle for AVSC running in open loop mode */
	uint8_t olDutyCycle);		/* in: user-supplied duty cycle in % (0 to 100) */

avscStat_t						/* ret: AVSC_SUCCESS, else error indication */
enable_avs_open(				/* setup, load, and enable AVS for open loop operation */
	avscOLparam_t avscParam);	/* in: user-supplied parameters (see libavsc.h) */
void reset_avs (void);

#endif	/* LIBAVSC_H_ */

/******** History ********
$Log: libavsc.h,v $
Revision 1.1  2012/09/10 06:07:50  srane
Initial commit for AVS code.


$Endlog$
*/
