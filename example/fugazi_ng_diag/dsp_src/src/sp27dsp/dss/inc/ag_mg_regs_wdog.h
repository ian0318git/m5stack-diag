/* $Id: ag_mg_regs_wdog.h,v 1.1 2012/04/18 18:08:27 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_mg_regs_wdog.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_wdog.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_dss_wdog.h
 *
 * Copyright (c) 2009 LSI Systems Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Systems Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license lsieements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Systems Inc. and treated accordingly.
 *
 * Each DSP Subsystem (DSS) contains a watchdog timer unit, as does
 * the ARM Subsystem (ARMSS). Register offsets and contents defined
 * herein are the same for all watchdog timer units.
 *
 * Register addresses defined herein are for code running on a DSP
 * accessing the Watchdog Timer in its own DSS via its APB bus.
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *   WDOG_LOAD      Watchdog Load Register
 *   WDOG_VAL       Watchdog Counter Value Register
 *   WDOG_CTRL      Watchdog Control Register
 *   WDOG_INTCLR    Watchdog Interrupt Clear Register
 *   WDOG_RIS       Watchdog Interrupt Status Register
 *   WDOG_MIS       Watchdog Masked Interrupt Status Register
 *   WDOG_LOCK      Watchdog Lock Register
 *
 * Prefix naming conventions :
 *   AG_MG_REGS_XXX_RA : register/memory physical address
 *   AG_MG_REGS_XXX_RO : register/memory address offset
 *   AG_MG_REGS_XXX_RM : register mask
 *   AG_MG_REGS_XXX_BO : bit/field offset from LSB
 *   AG_MG_REGS_XXX_BM : bit/field mask
 *   AG_MG_REGS_XXX_U  : bitfields in C union typedef
 *   AG_MG_REGS_XXX_S  : registers in C struct typedef
 *   AG_MG_REGS_XX_RPT : number of identical registers in array
 *   AG_MG_REGS_XX_IVL : interval between registers in array
 *
 * NOTE: user may redefine ag_mg_regs_register
 *       in ag_mg_regs_regops.h if necessary
 * NOTE: access mode of individual bit fields matches that
 *       of containing register unless indicated otherwise
 */

#ifndef AG_MG_REGS_DSS_WDOG_REGISTERS_H
#define AG_MG_REGS_DSS_WDOG_REGISTERS_H

#include "ag_mg_regs_regops.h"

/*
 * WDOG_LOAD (Watchdog Load Register)
 * Initialization value: 0xFFFFFFFF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_WDOG_LOAD_RO             0x00000000
#define AG_MG_REGS_WDOG_LOAD_RM             0xFFFFFFFF

#define AG_MG_REGS_WDOG_LOAD_LD_BO          0
#define AG_MG_REGS_WDOG_LOAD_LD_BM          0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_LOAD_U
{
    struct
    {
        ag_mg_regs_register
            ld;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_load_u;
#endif


/*
 * WDOG_VAL (Watchdog Counter Value Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_WDOG_VAL_RO              0x00000004
#define AG_MG_REGS_WDOG_VAL_RM              0xFFFFFFFF

#define AG_MG_REGS_WDOG_VAL_VALUE_BO        0
#define AG_MG_REGS_WDOG_VAL_VALUE_BM        0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_VAL_U
{
    struct
    {
        ag_mg_regs_register
            value;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_val_u;
#endif


/*
 * WDOG_CTRL (Watchdog Control Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_WDOG_CTRL_RO             0x00000008
#define AG_MG_REGS_WDOG_CTRL_RM             0x00000003

#define AG_MG_REGS_WDOG_CTRL_INTEN_BO       0
#define AG_MG_REGS_WDOG_CTRL_INTEN_BM       0x00000001

#define AG_MG_REGS_WDOG_CTRL_RESEN_BO       1
#define AG_MG_REGS_WDOG_CTRL_RESEN_BM       0x00000002

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            inten : 1,
            resen : 1,
            fill0 : 30;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_ctrl_u;
#endif


/*
 * WDOG_INTCLR (Watchdog Interrupt Clear Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Write Only
 */
#define AG_MG_REGS_WDOG_INTCLR_RO           0x0000000C
#define AG_MG_REGS_WDOG_INTCLR_RM           0x00000001

#define AG_MG_REGS_WDOG_INTCLR_INTCLR_BO    0
#define AG_MG_REGS_WDOG_INTCLR_INTCLR_BM    0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_INTCLR_U
{
    struct
    {
        ag_mg_regs_register
            intclr : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_intclr_u;
#endif


/*
 * WDOG_RIS (Watchdog Interrupt Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_WDOG_RIS_RO              0x00000010
#define AG_MG_REGS_WDOG_RIS_RM              0x00000001

#define AG_MG_REGS_WDOG_RIS_RWI_BO          0
#define AG_MG_REGS_WDOG_RIS_RWI_BM          0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_RIS_U
{
    struct
    {
        ag_mg_regs_register
            rwi : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_ris_u;
#endif


/*
 * WDOG_MIS (Watchdog Masked Interrupt Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_WDOG_MIS_RO              0x00000014
#define AG_MG_REGS_WDOG_MIS_RM              0x00000001

#define AG_MG_REGS_WDOG_MIS_WI_BO           0
#define AG_MG_REGS_WDOG_MIS_WI_BM           0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_MIS_U
{
    struct
    {
        ag_mg_regs_register
            wi : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_mis_u;
#endif


/*
 * WDOG_LOCK (Watchdog Lock Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_WDOG_LOCK_RO             0x00000C00
#define AG_MG_REGS_WDOG_LOCK_RM             0xFFFFFFFF

#define AG_MG_REGS_WDOG_LOCK_RWES_BO        0
#define AG_MG_REGS_WDOG_LOCK_RWES_BM        0x00000001

#define AG_MG_REGS_WDOG_LOCK_ERW_BO         1
#define AG_MG_REGS_WDOG_LOCK_ERW_BM         0xFFFFFFFE

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_WDOG_LOCK_U
{
    struct
    {
        ag_mg_regs_register
            rwes : 1,
            erw : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_wdog_lock_u;
#endif


/*
 * Physical register addresses (for DSP accessing its own DSS Watchdog Timer)
 */
#define AG_MG_REGS_WDOG_BASE       0xCF003000
#define AG_MG_REGS_WDOG_REG(ro)    (AG_MG_REGS_WDOG_BASE+ro)

#define AG_MG_REGS_WDOG_LOAD_RA    AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_LOAD_RO)
#define AG_MG_REGS_WDOG_VAL_RA     AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_VAL_RO)
#define AG_MG_REGS_WDOG_CTRL_RA    AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_CTRL_RO)
#define AG_MG_REGS_WDOG_INTCLR_RA  AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_INTCLR_RO)
#define AG_MG_REGS_WDOG_RIS_RA     AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_RIS_RO)
#define AG_MG_REGS_WDOG_MIS_RA     AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_MIS_RO)
#define AG_MG_REGS_WDOG_LOCK_RA    AG_MG_REGS_WDOG_REG(AG_MG_REGS_WDOG_LOCK_RO)

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef struct AG_MG_REGS_WDOG_REGS_S
{
    ag_mg_regs_wdog_load_u    load;
    ag_mg_regs_wdog_val_u     val;
    ag_mg_regs_wdog_ctrl_u    ctrl;
    ag_mg_regs_wdog_intclr_u  intclr;
    ag_mg_regs_wdog_ris_u     ris;
    ag_mg_regs_wdog_mis_u     mis;
    ag_mg_regs_register       RESERVED[762];
    ag_mg_regs_wdog_lock_u    lock;
} ag_mg_regs_wdog_reg_s;
/*
 * Recommended C syntax for typical usage :
 *   volatile ag_mg_regs_wdog_regs_s *wdog_regs =
 *       (volatile ag_mg_regs_wdog_regs_s *)AG_MG_REGS_WDOG_BASE;
 */
#endif

#endif

/******** History ********
$Log: ag_mg_regs_wdog.h,v $
Revision 1.1  2012/04/18 18:08:27  srane
Initial checkin


$Endlog$
*/

