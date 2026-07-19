/* $Id: libddr3.h,v 1.2 2017/07/28 07:58:48 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/lib/libddr3.h,v $
 *------------------------------------------------------------------
 * libddr3.h 
 * Description: interface to DDR3 configuration functions 
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*****************************************************************************
 *                             NOTIFICATION
 *
 * Copyright (c) 2010 LSI Inc.
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
 * libddr3.h : interface to DDR3 configuration functions
 *
 * Authors    :   JWB / LNJ
 */

#ifndef DDR3LIB_H_
#define DDR3LIB_H_

typedef enum DDR3_FREQ {
	DDR3_FREQ_525 = 0,
	DDR3_FREQ_400
} ddr3Freq_t;

typedef enum DDR3_STATUS {
	DDR3_SUCCESS			= 0,
	DDR3_FAIL_INIT_UNKNOWN,
	DDR3_FAIL_NO_MEMORY,
	DDR3_FAIL_CTRLR_INIT,
	DDR3_FAIL_PLL_INIT,
	DDR3_FAIL_ODT_INIT,
	DDR3_FAIL_PHY_INIT_CTRLR_PARAMS_NOT_SET,
	DDR3_FAIL_PHY_INIT_QTR_CYC_TRAIN_TIMEOUT,
	DDR3_FAIL_PHY_INIT_FEEDBACK_TRAIN_TIMEOUT,
	DDR3_FAIL_PHY_INIT_DFI_TRAIN_TIMEOUT,
	DDR3_FAIL_DFI_INIT_COMPLETE_ST_CHANGE_TIMEOUT,
	DDR3_FAIL_WRITE_LEVELING,
	DDR3_FAIL_GATE_TRAINING,
	DDR3_FAIL_READ_LEVELING,
	DDR3_FAIL_READ_BIT_ALIGNMENT,
	DDR3_FAIL_WR_DESKEWING_ALIGNMENT,
	DDR3_FAIL_RTC_IN_USE				/* RTC is used for timeouts by sp_DDR3init() */
} ddr3Stat_t ;

typedef struct ddr3boardParameters
{
	uint32_t configFlags;				/* ODT/DS/SLR on/off, 2/4 data lanes, ECC on/off */
	uint32_t sdramMR1data;				/* value for Denali Control Register 138 (SDRAM mode register 1) */
	uint32_t dp_io_vref;				/* value for DDR3 PHY DP_IO_VREF_SETTING register */
	uint32_t ap_io_setting[3];			/* values for DDR3 PHY AP_IO_SETTING<0-3> register */
	uint32_t ckiInMHz;					/* supplied CKI value (typically 25 or 50) round down for fractions */
} ddr3boardParameters_t;


ddr3Stat_t				/* ret: DDR3_SUCCESS or other error flag */
ddr3_initEval(			/* initialize DDR3 Memory Controller & associated circuitry */
	ddr3Freq_t freq);	/* in: DDR3_FREQ_525, DDR3_FREQ_400, ... */

int						/* ret: 0 => no errors */
ddr3_hasECCerrors(void);	/* check for any DDR3 controller caught single or multi-bit ECC errors */

/* sp_DDR3init() is meant for use with custom DDR3 configurations.
 * Please contact your LSI sales representative for the proper initialization parameters.
 */
ddr3Stat_t								/* ret: DDR3_SUCCESS or DDR3_FAIL_XXX_INIT */
sp_DDR3init(							/* initialize DDR3 Memory Controller & associated circuitry */
	uint32_t *p_ddr3ControllerRegs,		/* in: array containing initial controller parameters (obtained from LSI) */
	ddr3boardParameters_t *bparam);		/* in: structure with board-specific mode, ODT, DS and SLR parameters */

#endif /* DDR3LIB_H_ */

/******** History ********
$Log: libddr3.h,v $
Revision 1.2  2017/07/28 07:58:48  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:35  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/06/07 22:34:33  srane
Initial checkin for ECC memory test.
 

$Endlog$
*/

