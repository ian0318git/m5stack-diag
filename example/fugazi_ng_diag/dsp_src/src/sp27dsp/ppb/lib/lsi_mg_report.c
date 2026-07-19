/* $Id: lsi_mg_report.c,v 1.3 2012/10/04 23:36:44 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/lsi_mg_report.c,v $
 *------------------------------------------------------------------
 * lsi_mg_report.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2011 LSI Inc. All Rights Reserved
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
 * $RCSfile: lsi_mg_report.c,v $
 *
 * Description:	SP27XX utility routines.
 *
 *****************************************************************************/

/* Header files */
#include "lsi_sp27xx_reg.h"
#include "libgeneric.h"

#define DELAY_VAL	100000

#if LSI_SP27XX_BUILT_FOR_DSS
/* keep compiler from optimizing out the delay loop */
static volatile uint32_t lsi_mg_spinWait;
#endif

void lsi_mg_delay(		/* busy-wait routine (scales delay if PLL is running) */
	uint32_t count)			/* in: number of loops */
{
	/* multiply count parameter by the PLL multiplier field if PLL is not in bypass mode */
	if (CHK_REG_MASK(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1BYP_BM)) {
		count *= (HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1CTL_RA) & LSI_SP27XX_PLL1CTL_PLL1F_BM);
	}
#if LSI_SP27XX_BUILT_FOR_DSS
	if (count != 0) {
		lsi_mg_spinWait = count;
		while (lsi_mg_spinWait-- != 0);
	}
#else
	int ii;

	for(ii = 0; ii < count; ii++) {
		__asm__ volatile("mov r0, r0");
	}
#endif
}

void lsi_mg_test_pass(void)		/* place to set BP for linear tests */
{
	REG32_SET_BITS(LSI_SP27XX_GPIO_DIR_RA, ALL_BITS_ON);
	while (1) {
		/* just toggle GPIO0 */
		REG32_WRITE((LSI_SP27XX_GPIO_DATA_RA+((ALL_BITS_ON)<<0x2)), 1);
		lsi_mg_delay(DELAY_VAL);
		REG32_WRITE((LSI_SP27XX_GPIO_DATA_RA+((ALL_BITS_ON)<<0x2)), 0);
		lsi_mg_delay(DELAY_VAL);
	}
}

void lsi_mg_test_fail(void)		/* place to set BP for linear tests */
{
	REG32_SET_BITS(LSI_SP27XX_GPIO_DIR_RA, ALL_BITS_ON);
	while (1) {
		/* toggle all 8 PPB GPIOs */
		REG32_WRITE((LSI_SP27XX_GPIO_DATA_RA+((ALL_BITS_ON)<<0x2)), ALL_BITS_ON);
		lsi_mg_delay(DELAY_VAL);
		REG32_WRITE((LSI_SP27XX_GPIO_DATA_RA+((ALL_BITS_ON)<<0x2)), 0);
		lsi_mg_delay(DELAY_VAL);
	}
}

void lsi_mg_report(			/* error logging routine - needs embellishment */
	uint32_t failnumber) {	/* in: code indicating reason for failure */

	lsi_mg_test_fail();
}

#define MICRON_NAND_FLASH_MANUFACTURER_ID_VAL	0x2C

SP27XXEvalType_t
sp_check27XXboardConfig()
{
	uint32_t matrix_secureVal, ident;
	SP27XXEvalType_t dspType = UNKNOWN;

	/* read Device ID register */
	ident = HW_REG_ACCESS(LSI_SP27XX_CAR_DEVICEID_RA);
	if ( ident & 0x80 ) {
		switch (ident & 0x3) {
		case 0:
			dspType = SP2716_DSP0;
			break;
		case 1:
			dspType = SP2716_DSP1;
			break;
		case 2:
			dspType = SP2716_DSP2;
			break;
		case 3:
			dspType = SP2716_DSP3;
			break;
		}
	} else if ( (HW_REG_ACCESS(LSI_SP27XX_CAR_CHIPID_RA) & LSI_SP27XX_CHIPID_CHIPID_BM) == 0 ) {
		/* Software Development Board has all CHIPID pins tied low */
		dspType = SP2704SDB_DSP0;
	} else {
		uint32_t tcr0, tcr1;
		/* Must be a DSP2704 Eval board, use configuration of NAND Flash to identify the board.
		 * Frist, make sure we can see the NAND Flash Controller
		 * */
		matrix_secureVal = HW_REG_ACCESS(LSI_SP27XX_CAR_MATRIX_SECURE_RA);
		REG32_RESET_BITS(LSI_SP27XX_CAR_MATRIX_SECURE_RA, LSI_SP27XX_MATRIX_SECURE_NAND_FLASH_BM);

		/* save user's Timing Control Register values */
		tcr0 = HW_REG_ACCESS(LSI_SP27XX_NAND_FLASH_TIMCTL0_RA);
		tcr1 = HW_REG_ACCESS(LSI_SP27XX_NAND_FLASH_TIMCTL1_RA);

		REG32_WRITE(LSI_SP27XX_NAND_FLASH_TIMCTL0_RA, 0x020A0A0A);
		REG32_WRITE(LSI_SP27XX_NAND_FLASH_TIMCTL1_RA, 0x1F200404);

		/* try to access the ID registers for the NAND flash on 'chip select 1'
		 * On DSP0 of the SP2704 Evaluation board, both 'chip select' lines are populated.
		 * On DSP1, only 'chip select 0' is populated.
		 */
		REG32_WRITE(LSI_SP27XX_NAND_FLASH_CMDREG_RA, 0x190);	/* issue 'READ ID' command */

		lsi_mg_delay(0xFF);			/* allow time for read to complete */

		ident = HW_REG_ACCESS(LSI_SP27XX_NAND_FLASH_ID0_RA) & LSI_SP27XX_NAND_FLASH_ID0_ID0_0_BM;

		if ( ident == MICRON_NAND_FLASH_MANUFACTURER_ID_VAL ) {
			dspType = SP2704Eval_DSP0;
		} else if (ident == 0) {
			dspType = SP2704Eval_DSP1;
		} else {
			/* Gee! That's not what I expected! */
			dspType = UNKNOWN;
		}
		/* restore user's Timing Control Register values */
		REG32_WRITE(LSI_SP27XX_NAND_FLASH_TIMCTL0_RA, tcr0);
		REG32_WRITE(LSI_SP27XX_NAND_FLASH_TIMCTL1_RA, tcr1);
		/* restore original value to Matrix Secure */
		REG32_WRITE(LSI_SP27XX_CAR_MATRIX_SECURE_RA, matrix_secureVal);
	}

	return (dspType);
}

SP27XXDevId_t
sp_check27xxDevId(void)
{
	uint32_t temp;
	REG32_READ(LSI_SP27XX_CAR_DEVICEID_RA, temp);

	return (SP27XXDevId_t)(((temp >> 5) & 0x4)|(temp & 0x3));
}

uint32_t
sp_check27xxRevId(void)
{
	uint32_t temp;
	REG32_READ(LSI_SP27XX_CAR_CHIPID_RA, temp);

	return ((temp >> 29) & 0x3);
}

uint32_t
sp_check27xxChipId(void)
{
	uint32_t temp;
	REG32_READ(LSI_SP27XX_CAR_CHIPID_RA, temp);

	return (temp & 0x1F);
}

uint32_t
sp_check27xxPFUSE123(void)
{
	uint32_t temp;
	REG32_READ(0x9801226C, temp);

	return (temp);
}

#if LSI_SP27XX_BUILT_FOR_DSS
void sp_init31bitCycleCounter(void)
{
	REG32_WRITE(LSI_SP27XX_OCE_ECNT_VAL_RA, 0);
	/* enable OCE counter */
	REG32_WRITE(LSI_SP27XX_OCE_ECNT_CTRL_RA, 0x000001FC);
}
#endif

uint32_t					/* ret: number of sysclk cycles for RTC; 0 if timeoutINuS is out of range */
sp_timeoutValUsing32bitRTC(	/* calculate approx. RTC value for timeout in uS - max depends on PLL setting */
	uint32_t timeoutINuS,	/* in: approximate timeout in uS (best to round up); MAX = 4210752 */
	uint8_t ckiInMHz)		/* in: frequency of CKI in MHz, typically 25 or 50 */
{
	/* Under normal operating conditions (750 MHz) the maximum delay is about 6 seconds.
	 * For AVS designs running up to 1.0 Ghz, the maximum delay will be about 4.2 seconds.
	 */
	int64_t timeoutInCycles = timeoutINuS * (ckiInMHz + 1);

	/* divide by 2 for SYSCLK cycles */
	timeoutInCycles = timeoutInCycles >> 1;

	/* multiply count parameter by the PLL multiplier field if PLL is not in bypass mode */
	if (CHK_REG_MASK(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1BYP_BM)) {
		/* PLL is engaged, multiply by the multiplier value (PLL1F) */
		timeoutInCycles *= (HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1CTL_RA) & LSI_SP27XX_PLL1CTL_PLL1F_BM) + 2;
		/* adjust for output divider */
		timeoutInCycles = timeoutInCycles >> HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1SEL_RA);
	}
	if (timeoutInCycles > 0xFFFFFFFFLL) {
	      	return (0xFFFFFFFF);
	} else {
		return((uint32_t)timeoutInCycles);
	}
}

bool								/* ret: false -> bit clear, no timeout, true -> timeout or RTC in use */
sp_timeoutDuringWaitForRegBitClear(	/* wait for timeout ticks of RTC for bit to be cleared */
	uint32_t regAddr,				/* in: register address */
	uint32_t bitMask,				/* in: bit to test for being set to '0' */
	uint32_t timeout)    			/* in: timeout in RTC clock (SYSCLK) ticks */
{
	uint32_t lastRTCval = 0;
	bool retVal = true;

	if (CHK_REG_MASK(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM)) {
		/* someone else is using RTC so we can't - indicate timeout */
		return(true);
	}
	/* enable RTC */
	REG32_SET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM);

	/* WARNING: RTC is free-running, stepping through the following code could cause an
	 * WARNING: 	erroneous timeout condition
	 * */
	REG32_SET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCRST_BM); /* reset RTC */
	do {
		if (!CHK_REG_MASK(regAddr, bitMask))
			break;
		lastRTCval = HW_REG_ACCESS(LSI_SP27XX_CAR_RTC0_RA);
	} while (lastRTCval < timeout);
	/* WARNING: RTC is free-running, stepping through above loop could cause timeout */
	if (lastRTCval > timeout) {
		retVal = true;
	} else {
		retVal = false;
	}
	/* disable RTC */
	REG32_RESET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM);
	return (retVal);
}

bool								/* ret: false -> bit set, no timeout, true -> timeout */
sp_timeoutDuringWaitForRegBitSet(	/* wait for timeout ticks of RTC for bit to be set */
	uint32_t regAddr,				/* in: register address */
	uint32_t bitMask,				/* in: bit to test for being set to '1' */
	uint32_t timeout)    			/* in: timeout in RTC clock (SYSCLK) ticks */
{
	uint32_t lastRTCval = 0;
	bool retVal = true;

	if (CHK_REG_MASK(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM)) {
		/* someone else is using RTC so we can't - indicate timeout */
		return(true);
	}
	/* enable RTC */
	REG32_SET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM);

	/* WARNING: RTC is free-running, stepping through the following code could cause an
	 * WARNING: 	erroneous timeout condition
	 * */
	REG32_SET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCRST_BM); /* reset RTC */
	do {
		if (CHK_REG_MASK(regAddr, bitMask))
			break;
		lastRTCval = HW_REG_ACCESS(LSI_SP27XX_CAR_RTC0_RA);
	} while (lastRTCval < timeout);
	/* WARNING: RTC is free-running, stepping through above loop could cause timeout */
	if (lastRTCval > timeout) {
		retVal = true;
	} else {
		retVal = false;
	}
	/* disable RTC */
	REG32_RESET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM);
	return (retVal);
}

int32_t 							/* ret: 0 -> success, -1 -> fail */
sp_delayUsingRTC(					/* wait a specific of time (in uS) */
		uint32_t timeoutINuS,		/* in: approximate timeout in uS (best to round up) */
		uint8_t ckiInMHz)			/* in: frequency of CKI in MHz, typically 25 or 50 */
{
int64_t timeoutInCycles = timeoutINuS * (ckiInMHz + 1);

	if (CHK_REG_MASK(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM)) {
		/* Someone else is using RTC so we can't - return error */
		return(-1);
	}

	/* divide by 2 for SYSCLK cycles */
	timeoutInCycles = timeoutInCycles >> 1;

	/* multiply count parameter by the PLL multiplier field if PLL is not in bypass mode */
	if (CHK_REG_MASK(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1BYP_BM)) {
		/* PLL is engaged, multiply by the multiplier value (PLL1F) */
		timeoutInCycles *= (HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1CTL_RA) & LSI_SP27XX_PLL1CTL_PLL1F_BM) + 2;
		/* adjust for output divider */
		timeoutInCycles = timeoutInCycles >> HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1SEL_RA);
	}

	/* Enable RTC */
	REG32_SET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM);

	/* Reset RTC */
	REG32_SET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCRST_BM);

	/* Wait the specified amount of time */
	while (*(volatile unsigned long long*)LSI_SP27XX_CAR_RTC0_RA < timeoutInCycles);

	/* Disable RTC */
	REG32_RESET_BITS(LSI_SP27XX_CAR_RTCCTL_RA, LSI_SP27XX_RTCCTL_RTCEN_BM);

	return(0);
}
/******** History ********
$Log: lsi_mg_report.c,v $
Revision 1.3  2012/10/04 23:36:44  srane
Add support for SP2702.

Revision 1.2  2012/05/10 22:48:11  srane
clean up and modify files for exception handling and TDM tests.

Revision 1.1  2012/04/18 09:47:31  srane
Initial checkin


$Endlog$
*/

