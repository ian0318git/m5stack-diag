/* $Id: lsi_sp27xx_avsc.h,v 1.1 2012/09/12 23:45:51 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/lib/lsi_sp27xx_avsc.h,v $
 *------------------------------------------------------------------
 * lsi_sp27xx_avsc.h
 *
 * Sep 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*
 * lsi_sp27xx_avsc.h
 *
 * Copyright (c) 2012 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * Based on StarPro® SP2700 DSP Family Register Programming Guide, version 1.3, Preliminary.
 *
 * The Adaptive Voltage Scaling Controller (AVSC) uses on-chip ring oscillators
 * and delay lines to monitor changes in device characteristics because of
 * fluctuations in voltage and temperature. The AVS controller interprets the
 * data from this logic and uses it to control a pulse width modulated (PWM)
 * serial output voltage control signal. This PWM signal is output on the
 * AVS_VID[0] pin, and it can be used to control the voltage adjust pin of an
 * external regulator.
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *
 *
 *  CTL0
 *  CTL1
 *  CTL3
 *  CTL4
 *  AVSKEY
 *  
 * Prefix naming conventions :
 *   LSI_SP27XX_XXX_RA : register/memory physical address
 *   LSI_SP27XX_XXX_RO : register/memory address offset
 *   LSI_SP27XX_XXX_RM : register mask
 *   LSI_SP27XX_XXX_BO : bit/field offset from LSB
 *   LSI_SP27XX_XXX_BM : bit/field mask
 *   LSI_SP27XX_XXX_U  : bitfields in C union typedef
 *   LSI_SP27XX_XXX_S  : registers in C struct typedef
 *   LSI_SP27XX_XX_RPT : number of identical registers in array
 *   LSI_SP27XX_XX_IVL : interval between registers in array
 *
 * NOTE: user may redefine lsi_sp27xx_register
 *       in lsi_sp27xx_regops.h if necessary
 * NOTE: access mode of individual bit fields matches that
 *       of containing register unless indicated otherwise
 */

#ifndef LSI_SP27XX_AVSC_REGISTERS_H
#define LSI_SP27XX_AVSC_REGISTERS_H

#include "lsi_sp27xx_regops.h"

/*
 * Hand Generated
 */


/*
 * Access mode: Read / Write
 * CTL0 (AVSC Control Register 0)
 * Initialization value: 0x00000000  Initialization mask: 0x00000002
 */

#define LSI_SP27XX_AVSC_CTL0_RO					0x00000000
#define LSI_SP27XX_AVSC_CTL0_RM					0x00000002

#define LSI_SP27XX_AVSC_CTL0_RAMEN_BO			0x00000001
#define LSI_SP27XX_AVSC_CTL0_RAMEN_BM			0x00000002

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef union LSI_SP27XX_AVSC_CTL0_U
{
	struct
	{
		lsi_sp27xx_register
			fill0 : 1,
			ramen : 1,
			fill1 : 30;
	} fields;
	lsi_sp27xx_register reg;
} lsi_sp27xx_avsc_ctl0_u;
#endif

/*
 * Access mode: Read / Write
 * CTL1 (AVSC Control Register 1)
 * Initialization value: 0x00000000  Initialization mask: 0x00000007
 */

#define LSI_SP27XX_AVSC_CTL1_RO						0x00000004
#define LSI_SP27XX_AVSC_CTL1_RM						0x00000007

#define LSI_SP27XX_AVSC_CTL1_AVSRST_BO				0x00000000
#define LSI_SP27XX_AVSC_CTL1_AVSRST_BM				0x00000001

#define LSI_SP27XX_AVSC_CTL1_AVSSTOP_BO				0x00000001
#define LSI_SP27XX_AVSC_CTL1_AVSSTOP_BM				0x00000002

#define LSI_SP27XX_AVSC_CTL1_RAMADRMUX_BO			0x00000002
#define LSI_SP27XX_AVSC_CTL1_RAMADRMUX_BM			0x00000004

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef union LSI_SP27XX_AVSC_CTL1_U
{
	struct
	{
		lsi_sp27xx_register
			avsrst : 1,
			avsstop : 1,
			ramadrmux : 1,
			fill0 : 29;
	} fields;
	lsi_sp27xx_register reg;
} lsi_sp27xx_avsc_ctl1_u;
#endif

/*
 * Access mode: Read / Write
 * CTL2 (AVSC Control Register 2)
 * Initialization value: 0x00000000  Initialization mask: 0x80007FFF
 */

#define LSI_SP27XX_AVSC_CTL2_RO							0x00000008
#define LSI_SP27XX_AVSC_CTL2_RM							0x80007FFF

#define LSI_SP27XX_AVSC_CTL2_OL_DUTY_CYCLE_BO			0x00000000
#define LSI_SP27XX_AVSC_CTL2_OL_DUTY_CYCLE_BM			0x00000FFF

#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_BO		0x0000000C
#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_BM		0x00007000
#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_97_66KHZ	0x00003000
#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_48_83KHZ	0x00004000
#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_24_41KHZ	0x00005000
#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_12_21KHZ	0x00006000
#define LSI_SP27XX_AVSC_CTL2_VID_PIN_OVERRIDE_6_1KHZ	0x00007000

#define LSI_SP27XX_AVSC_CTL2_OPEN_LOOP_EN_BO			0x0000001F
#define LSI_SP27XX_AVSC_CTL2_OPEN_LOOP_EN_BM			0x80000000

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef union LSI_SP27XX_AVSC_CTL2_U
{
	struct
	{
		lsi_sp27xx_register
			ol_duty_cycle : 12,
			vid_pin_override : 3,
			fill0 : 16,
			open_loop_en : 1;
	} fields;
	lsi_sp27xx_register reg;
} lsi_sp27xx_avsc_ctl2_u;
#endif

/*
 * Access mode: Read / Write
 * CTL3 (AVSC Control Register 3)
 * Initialization value: 0x00000000  Initialization mask: 0x001FFFC3
 */

#define LSI_SP27XX_AVSC_CTL3_RO							0x0000000C
#define LSI_SP27XX_AVSC_CTL3_RM							0x001FFFC3

#define LSI_SP27XX_AVSC_CTL3_FORCE_FINE_TUNE_BO			0x00000000
#define LSI_SP27XX_AVSC_CTL3_FORCE_FINE_TUNE_BM			0x00000001

#define LSI_SP27XX_AVSC_CTL3_FINE_TUNE_EN_BO			0x00000001
#define LSI_SP27XX_AVSC_CTL3_FINE_TUNE_EN_BM			0x00000002

#define LSI_SP27XX_AVSC_CTL3_FORCE_MCM_MODE_BO			0x00000006
#define LSI_SP27XX_AVSC_CTL3_FORCE_MCM_MODE_BM			0x00000040

#define LSI_SP27XX_AVSC_CTL3_MCM_MASTER_EN_BO			0x00000007
#define LSI_SP27XX_AVSC_CTL3_MCM_MASTER_EN_BM			0x00000080

#define LSI_SP27XX_AVSC_CTL3_HALF_CLK_MODE_BO			0x00000008
#define LSI_SP27XX_AVSC_CTL3_HALF_CLK_MODE_BM			0x00000100

#define LSI_SP27XX_AVSC_CTL3_CSM_STOP_PT_BO				0x00000009
#define LSI_SP27XX_AVSC_CTL3_CSM_STOP_PT_BM				0x00007E00

#define LSI_SP27XX_AVSC_CTL3_CSM_STOP_PT_EN_BO			0x0000000F
#define LSI_SP27XX_AVSC_CTL3_CSM_STOP_PT_EN_BM			0x00008000

#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_BO				0x00000010
#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_BM				0x00030000

#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL320MILLISEC	0x00030000
#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL160MILLISEC	0x00020000
#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL80MILLISEC	0x00010000
#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_VAL40MILLISEC	0x00000000

#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_SEL_BO			0x00000012
#define LSI_SP27XX_AVSC_CTL3_INCR_DELAY_SEL_BM			0x00040000

#define LSI_SP27XX_AVSC_CTL3_BYP_COARSE_BO				0x00000013
#define LSI_SP27XX_AVSC_CTL3_BYP_COARSE_BM				0x00080000

#define LSI_SP27XX_AVSC_CTL3_USE_ADAPTIVE_TURBO_BO		0x00000014
#define LSI_SP27XX_AVSC_CTL3_USE_ADAPTIVE_TURBO_BM		0x00100000

#define LSI_SP27XX_AVSC_CTL3_FORCE_OPEN_LOOP_BO			0x00000015
#define LSI_SP27XX_AVSC_CTL3_FORCE_OPEN_LOOP_BM			0x00200000

#define LSI_SP27XX_AVSC_CTL3_MCM_SLAVE_FAILURE_MODE_BO	0x00000016
#define LSI_SP27XX_AVSC_CTL3_MCM_SLAVE_FAILURE_MODE_BM	0x00400000

#define LSI_SP27XX_AVSC_CTL3_CKI19_44MHZ_BO				0x00000017
#define LSI_SP27XX_AVSC_CTL3_CKI19_44MHZ_BM				0x00800000

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef union LSI_SP27XX_AVSC_CTL3_U
{
	struct
	{
		lsi_sp27xx_register
			force_fine_tune : 1,
			fine_tune_en : 1,
			fill0 : 4,
			force_mcm_mode : 1,
			mcm_master_en : 1,
			half_clk_mode : 1,
			csm_stop_pt : 6,
			csm_stop_pt_en : 1,
			incr_delay : 2,
			incr_delay_sel : 1,
			byp_coarse : 1,
			use_adaptive_turbo : 1,
			force_open_loop : 1,
			mcm_slave_failure_mode : 1,
			cki19_44mhz: 1,
			fill1 : 8;
	} fields;
	lsi_sp27xx_register reg;
} lsi_sp27xx_avsc_ctl3_u;
#endif

/*
 * Access mode: Read / Write
 * CTL4 (AVSC Control Register 4)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 */

#define LSI_SP27XX_AVSC_CTL4_RO							0x00000010
#define LSI_SP27XX_AVSC_CTL4_RM							0xFFFFFFFF

#define LSI_SP27XX_AVSC_CTL4_FINE_TUNE_SLEEP_VAL_BO		0x00000000
#define LSI_SP27XX_AVSC_CTL4_FINE_TUNE_SLEEP_VAL_BM		0x7FFFFFFF

#define LSI_SP27XX_AVSC_CTL4_FINE_TUNE_SLEEP_SEL_BO		0x0000001F
#define LSI_SP27XX_AVSC_CTL4_FINE_TUNE_SLEEP_SEL_BM		0x80000000

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef union LSI_SP27XX_AVSC_CTL4_U
{
	struct
	{
		lsi_sp27xx_register
			fine_tune_sleep_val : 31,
			fine_tune_sleep_sel : 1;
	} fields;
	lsi_sp27xx_register reg;
} lsi_sp27xx_avsc_ctl4_u;
#endif


/*
 * Access mode: Read / Write
 * AVSKEY (AVS Key Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 */

#define LSI_SP27XX_AVSC_AVSKEY_RO				0x00000020
#define LSI_SP27XX_AVSC_AVSKEY_RM				0xFFFFFFFF

#define LSI_SP27XX_AVSC_AVSKEY_AVSKEY_BO			0x00000000
#define LSI_SP27XX_AVSC_AVSKEY_AVSKEY_BM			0xFFFFFFFF

#define LSI_SP27XX_AVSC_AVSKEY_AVSKEY_VAL			0xA5504E94

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef union LSI_SP27XX_AVSC_AVSKEY_U
{
	struct
	{
		lsi_sp27xx_register
			avskey : 32;
	} fields;
	lsi_sp27xx_register reg;
} lsi_sp27xx_avsc_avskey_u;
#endif




/*
* Physical register addresses (for ARM or DSS accessing AVSC)
*/

#define LSI_SP27XX_AVSC_BASE				0x98016000
#define LSI_SP27XX_AVSC_RAM_BASE			0x98014000
#define LSI_SP27XX_AVSC_REG(ro)				(LSI_SP27XX_AVSC_BASE+(ro))

#define LSI_SP27XX_AVSC_CTL0_RA         LSI_SP27XX_AVSC_REG(LSI_SP27XX_AVSC_CTL0_RO)
#define LSI_SP27XX_AVSC_CTL1_RA         LSI_SP27XX_AVSC_REG(LSI_SP27XX_AVSC_CTL1_RO)
#define LSI_SP27XX_AVSC_CTL2_RA         LSI_SP27XX_AVSC_REG(LSI_SP27XX_AVSC_CTL2_RO)
#define LSI_SP27XX_AVSC_CTL3_RA         LSI_SP27XX_AVSC_REG(LSI_SP27XX_AVSC_CTL3_RO)
#define LSI_SP27XX_AVSC_CTL4_RA         LSI_SP27XX_AVSC_REG(LSI_SP27XX_AVSC_CTL4_RO)
#define LSI_SP27XX_AVSC_AVSKEY_RA       LSI_SP27XX_AVSC_REG(LSI_SP27XX_AVSC_AVSKEY_RO)

#ifdef LSI_SP27XX_USE_C_STRUCTURES
typedef struct LSI_SP27XX_AVSC_REGS_S
{
	lsi_sp27xx_avsc_ctl0_u		ctl0;
	lsi_sp27xx_avsc_ctl1_u		ctl1;
	lsi_sp27xx_avsc_ctl2_u		ctl2;
	lsi_sp27xx_avsc_ctl3_u		ctl3;
	lsi_sp27xx_avsc_ctl4_u		ctl4;
	lsi_sp27xx_register			RESERVED_1[3] ;
	lsi_sp27xx_avsc_avskey_u	avskey;
} lsi_sp27xx_avsc_reg_s;
/*
* Recommended C syntax for typical usage :
*	volatile lsi_sp27xx_avsc_reg_s *avsc_regs =
*		(volatile lsi_sp27xx_avsc_reg_s *) LSI_SP27XX_AVSC_BASE;
*/
#endif

#endif
/******** History ********
$Log: lsi_sp27xx_avsc.h,v $
Revision 1.1  2012/09/12 23:45:51  srane
Initial commit (should be added to
/auto/aegir-ios/tools/dsp/lsi/devices/2.9.6/sp27xx/ppb/include)



$Endlog$
*/

