/* $Id: lsi_mg_pll.c,v 1.1 2012/05/31 06:36:58 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/lsi_mg_pll.c,v $
 *------------------------------------------------------------------
 * lsi_mg_pll.c
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * lsi_mg_dss_pll.c - SP2704 PLL configuration routines for PLL1 (DSS PLL) & PLL2 (DDR3 PLL)
 */

#include "lsi_sp27xx_reg.h"

//#ifdef LSI_SP27XX_BUILT_FOR_DSS
//#include "lsi_mg_libsp27k.h"
//#define DELAY_VAL_50MHZ 0xff
//#else
#include "libgeneric.h"
/* PPB SYSCLK is 1/2 of DSS clock */
#define DELAY_VAL_50MHZ 0x7f
//#endif

#define PLL_NOT_BYP (1<<LSI_SP27XX_PLL1CTL_PLL1BYP_BM)
#define PLL_BYP		0

#define PLL_ENABLE	(1<<LSI_SP27XX_PLL1CTL_PLL1EN_BM)
#define PLL_DISABLE 0

#define PLL_PLLF(y) (((y)<<LSI_SP27XX_PLL1CTL_PLL1F_BO) & LSI_SP27XX_PLL1CTL_PLL1F_BM)

/* Note: PLL1 and PLL2 are identical (control register layout, etc.) except that
 * PLL2 has no output divider */

typedef struct PLLSETTINGS {
	uint32_t multMax;		/* largest multiplier for this setting */
	uint32_t ppzzz;			/* P1, P0, Z2, Z1 and Z0 values used for multipliers up to this value */
} PLLsettings_t;

/* Lookup table for P1, P0, Z2, Z1 and Z0 values */
static const PLLsettings_t PLL_ranges[] = {
	{ 4, 0x18 },
	{ 10, 0x19 },
	{ 18, 0x13 },
	{ 41, 0xD },
	{ 76, 0x7 },
	{ 170, 0x7 }		/* ppzzz value not used */
};

/********************* enable_pll() **********************************/
int							/* ret: 0 => success, -1 => not locked, or parameter error indicator */
enable_pll(					/* set up PLL - usually done by PPB */
	uint32_t multiplier,	/* in: multiplier (4 .. 170) */
	uint32_t fin,			/* in: input frequency (5 < fin < 450) */
	uint32_t output_divider)/* in: post-scale by 1, 2, 4 or 8 */
{
uint32_t f, out_div, ppzzz, ctrl_val;
int ii;
uint32_t pll1_locked = 0;

	if ( LSI_SP27XX_IS_V10_DEVICE() ) {
		/* enable PLL - this is a work-around for HW-Bug 1073 present in the initial SP2704 silicon */
		REG32_SET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1EN_BM);
	}

	/* From PLL40G18W4001800 data sheet (12/01/2009)
	 * Note: internal feedback divide is capable of 2-257, although only values
	 * from 2 to 170 are allowed.
	 */
	if ((multiplier > 170) || (multiplier < 4)) {
		return(1);
	}
	ii = 0;
	while (ii < sizeof(PLL_ranges)/sizeof(PLLsettings_t)) {
		if (multiplier > PLL_ranges[ii].multMax) {
			ii++;
		} else {
			/* past it, use last entry */
			break;
		}
	}
	ppzzz = PLL_ranges[ii - 1].ppzzz;

	/* Valid Reference Frequency Range from PLL40G18W4001800 data sheet (12/01/2009) is 4.7 to 450 MHz */
	if ( (fin < 5) | (fin > 450) ) {
		return(3);
	}

	/* check to ensure VCO will run within spec  */
	f = fin * multiplier;
	if ((f<800) || (f>1800)) {
		return(4);
	}
	
	/* check output divider setting (pll1sel) valid options are 1, 2, 4 & 8 */
	switch (output_divider) {
		case 1:	out_div = 0;
				break;
		case 2:	out_div = 1;
				break;
		case 4:	out_div = 2;
				break;
		case 8:	out_div = 3;
				break;
		default: return(5);
	}

	/* Bypass PLL before configuring it */
	REG32_RESET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1BYP_BM);

	/* turn off PLL */
	REG32_RESET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1EN_BM);

	/* use multiplier to calculate Feedback and insert P1,P0, Z2,Z1,Z0 values */
	ctrl_val = PLL_PLLF(multiplier - 2);
	ctrl_val |= (ppzzz << LSI_SP27XX_PLL1CTL_PLL1Z_BO) &
			(LSI_SP27XX_PLL1CTL_PLL1Z_BM | LSI_SP27XX_PLL1CTL_PLL1P_BM);

	/* copy to register */
	REG32_WRITE(LSI_SP27XX_CAR_PLL1CTL_RA, ctrl_val);

	/* set output_divider */
	REG32_WRITE(LSI_SP27XX_CAR_PLL1SEL_RA, (out_div & LSI_SP27XX_PLL1SEL_PLL1SEL_BM));

	/* enable PLL - this must be done after the rest of PLLCTL has been set */
	REG32_SET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1EN_BM);

	if ( LSI_SP27XX_IS_V10_DEVICE() ) {
		/* just wait for lock , don't poll, a work-around for HW-Bug 1073 present in the initial SP2704 silicon */
		lsi_mg_delay(DELAY_VAL_50MHZ * 50 / fin);
		pll1_locked = (CHK_REG_MASK(LSI_SP27XX_CAR_PLL1CTL_RA,
				       (LSI_SP27XX_PLL1CTL_PLL1LOCK_BM|LSI_SP27XX_PLL1CTL_PLL1CARLOCK_BM)));
	} else {
		for (ii = 0; ii < DELAY_VAL_50MHZ; ii++) {
			lsi_mg_delay(2);
			pll1_locked = (CHK_REG_MASK(LSI_SP27XX_CAR_PLL1CTL_RA,
								       (LSI_SP27XX_PLL1CTL_PLL1LOCK_BM|LSI_SP27XX_PLL1CTL_PLL1CARLOCK_BM)));
			if (pll1_locked) {
				break;
			}
		}
	}

	if (!pll1_locked) {
		return (-1);
	}

	/* switch PLL Output to system  */
	REG32_SET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1BYP_BM);
	
	/* set ECKO to CLKSYS/8 (001101) */
	REG32_SET_BITS(LSI_SP27XX_CAR_CLKOUT_RA, 0x0D);

    return(0);
}


int							/* ret: 0 => success, -1 => not locked, or parameter error indicator */
enable_ddr3pll(				/* set up DDR3 PLL - usually done by PPB */
	uint32_t multiplier,	/* in: multiplier (4 .. 170) */
	uint32_t fin)			/* in: input frequency (5 < fin < 450) */
{
int retVal = 0;
uint32_t f, ppzzz, ctrl_val, pll1ByPassVal = 0;
int ii;
volatile uint32_t pll2_locked = 0;

	if ( LSI_SP27XX_IS_V10_DEVICE() ) {
		pll1ByPassVal = HW_REG_ACCESS(LSI_SP27XX_CAR_PLL1CTL_RA) & LSI_SP27XX_PLL1CTL_PLL1BYP_BM;
		/* bypass PLL1 to allow proper delay values */
		REG32_RESET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, LSI_SP27XX_PLL1CTL_PLL1BYP_BM);

		/* enable PLL twice - this is a work-around for HW-Bug 1073 present in the initial SP2704 silicon
		 * Internal ROM does the first enable for PLL1 so there is extra code here for DDR3 PLL
		 * */
		REG32_SET_BITS(LSI_SP27XX_CAR_PLL2CTL_RA, LSI_SP27XX_PLL2CTL_PLL2EN_BM);
		lsi_mg_delay(0xff);
		REG32_SET_BITS(LSI_SP27XX_CAR_PLL2CTL_RA, LSI_SP27XX_PLL2CTL_PLL2EN_BM);
	}

	/* From PLL40G18W4001800 data sheet (12/01/2009)
	 * Note: internal feedback divide is capable of 2-257, although only values
	 * from 2 to 170 are allowed.
	 */
	if ((multiplier > 170) || (multiplier < 4)) {
		retVal = 1;
	}
	ii = 0;
	while (ii < sizeof(PLL_ranges)/sizeof(PLLsettings_t)) {
		if (multiplier > PLL_ranges[ii].multMax) {
			ii++;
		} else {
			/* past it, use last entry */
			break;
		}
	}
	ppzzz = PLL_ranges[ii - 1].ppzzz;

	/* Valid Reference Frequency Range from PLL40G18W4001800 data sheet (12/01/2009) is 4.7 to 450 MHz */
	if (retVal == 0) {
		if ( (fin < 5) | (fin > 450) ) {
			retVal = 3;
		}

		/* check to ensure VCO will run within spec  */
		f = fin * multiplier;
		if ((f<800) || (f>1800)) {
			retVal = 4;
		}
	}

	if (retVal == 0) {
		/* turn off PLL */
		REG32_RESET_BITS(LSI_SP27XX_CAR_PLL2CTL_RA, LSI_SP27XX_PLL2CTL_PLL2EN_BM);

		/* use multiplier to calculate Feedback and insert P1,P0, Z2,Z1,Z0 values */
		ctrl_val = PLL_PLLF(multiplier - 2);
		ctrl_val |= (ppzzz << LSI_SP27XX_PLL2CTL_PLL2Z_BO) &
				(LSI_SP27XX_PLL2CTL_PLL2Z_BM | LSI_SP27XX_PLL2CTL_PLL2P_BM);

		/* copy to register */
		REG32_WRITE(LSI_SP27XX_CAR_PLL2CTL_RA, ctrl_val);

		/* enable PLL - this must be done after the rest of PLLCTL has been set */
		REG32_SET_BITS(LSI_SP27XX_CAR_PLL2CTL_RA, LSI_SP27XX_PLL2CTL_PLL2EN_BM);

		if ( LSI_SP27XX_IS_V10_DEVICE() ) {
			/* just wait for lock , don't poll, a work-around for HW-Bug 1073 present in the initial SP2704 silicon */
			lsi_mg_delay(DELAY_VAL_50MHZ * 50 / fin);
			pll2_locked = (CHK_REG_MASK(LSI_SP27XX_CAR_PLL2CTL_RA,
					       (LSI_SP27XX_PLL2CTL_PLL2LOCK_BM|LSI_SP27XX_PLL2CTL_PLL2CARLOCK_BM)));
		} else {
			for (ii = 0; ii < DELAY_VAL_50MHZ; ii++) {
				lsi_mg_delay(2);
				pll2_locked = (CHK_REG_MASK(LSI_SP27XX_CAR_PLL2CTL_RA,
									       (LSI_SP27XX_PLL2CTL_PLL2LOCK_BM|LSI_SP27XX_PLL2CTL_PLL2CARLOCK_BM)));
				if (pll2_locked) {
					break;
				}
			}
		}

		if (!pll2_locked) {
			retVal = -1;
		}
	}

	/* set ECKO to CLKSYS/8 (001101) */
	REG32_SET_BITS(LSI_SP27XX_CAR_CLKOUT_RA, 0x0D);
	if ( LSI_SP27XX_IS_V10_DEVICE() ) {
		/* restore original PLL1 bypass value (v1.0 only) */
		REG32_SET_BITS(LSI_SP27XX_CAR_PLL1CTL_RA, pll1ByPassVal);
	}

	return(retVal);
}

/******** History ********
$Log: lsi_mg_pll.c,v $
Revision 1.1  2012/05/31 06:36:58  srane
Initial checkin.


$Endlog$
*/

