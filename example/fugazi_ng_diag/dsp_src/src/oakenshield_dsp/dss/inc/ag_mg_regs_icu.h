/* $Id: ag_mg_regs_icu.h,v 1.2 2017/07/28 07:58:34 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/ag_mg_regs_icu.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_icu.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_dss_icu.h
 *
 * Copyright (c) 2009 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * This file defines the DSP subsystem (DSS) Interrupt Control Unit
 * (ICU) registers. Register addresses defined herein are for code
 * running on the DSS accessing its own ICU.
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *   ICU_MIPL<0-127>  ICU Maskable Interrupt Priority Level Registers
 *
 * Prefix naming conventions :
 *   AG_MG_REGS_XXX_RA : register/memory physical address
 *   AG_MG_REGS_XXX_RO : register/memory address offset
 *   AG_MG_REGS_XXX_RM : register mask
 *   AG_MG_REGS_XXX_BO : bit/field offset from LSB
 *   AG_MG_REGS_XXX_BM : bit/field mask
 *   AG_MG_REGS_XXX_U  : bitfields in C union typedef
 *   AG_MG_REGS_XXX_S  : registers in C struct typedef
 *   AGR_SP25XX_XX_RPT : number of identical registers in array
 *   AGR_SP25XX_XX_IVL : interval between registers in array
 *
 * NOTE: user may redefine ag_mg_regs_register
 *       in ag_mg_regs_regops.h if necessary
 * NOTE: access mode of individual bit fields matches that
 *       of containing register unless indicated otherwise
 */

#ifndef AG_MG_REGS_DSS_ICU_REGISTERS_H
#define AG_MG_REGS_DSS_ICU_REGISTERS_H

#include "ag_mg_regs_regops.h"

/*
 * ICU_MIPL<0-127> (ICU Maskable Interrupt Priority Level Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_ICU_MIPL_RO           0x00000000
#define AG_MG_REGS_ICU_MIPL_RM           0x00000007

#define AG_MG_REGS_ICU_MIPL_IPL_BO       0
#define AG_MG_REGS_ICU_MIPL_IPL_BM       0x00000007

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_ICU_MIPL_U
{
    struct
    {
        ag_mg_regs_register
            ipl : 3,
            fill0 : 29;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_icu_mipl_u;
#endif


/*
 * Physical register addresses (for DSS accessing its Interrupt Control Unit)
 */
#define AG_MG_REGS_ICU_BASE        0x90000000
#define AG_MG_REGS_ICU_REG(ro)     (AG_MG_REGS_ICU_BASE+(ro))

#define AG_MG_REGS_ICU_MIPL_IVL          4
#define AG_MG_REGS_ICU_MIPL_RPT          128

#define AG_MG_REGS_ICU_MIPL_RA(x)  AG_MG_REGS_ICU_REG((x)*AG_MG_REGS_ICU_MIPL_IVL)

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef struct AG_MG_REGS_ICU_REGS_S
{
    ag_mg_regs_icu_mipl_u  mipl[AG_MG_REGS_ICU_MIPL_RPT];
} ag_mg_regs_icu_reg_s;
/*
 * Recommended C syntax for typical usage :
 *   volatile ag_mg_regs_icu_reg_s *icu_regs =
 *       (volatile ag_mg_regs_icu_reg_s *)AG_MG_REGS_ICU_BASE;
 */
#endif

#endif

/******** History ********
$Log: ag_mg_regs_icu.h,v $
Revision 1.2  2017/07/28 07:58:34  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:25  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 18:08:26  srane
Initial checkin


$Endlog$
*/

