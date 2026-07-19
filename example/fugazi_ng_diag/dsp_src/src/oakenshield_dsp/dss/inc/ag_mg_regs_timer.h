/* $Id: ag_mg_regs_timer.h,v 1.2 2017/07/28 07:58:35 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/inc/ag_mg_regs_timer.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_timer.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_dss_timer.h
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
 * Each DSP Subsystem (DSS) contains two general-purpose timer units.
 * Register offsets and contents defined herein are the same for both timers.
 *
 * Register addresses defined herein are for code running on a DSP accessing
 * timers in its own DSS via its APB bus.

 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *   TIMER_LOAD<0-1>    Timer Load Value Registers
 *   TIMER_VAL<0-1>     Timer Current Value Registers
 *   TIMER_CTRL<0-1>    Timer Control Registers
 *   TIMER_INTCLR<0-1>  Timer Interrupt Clear Registers
 *   TIMER_INTRIS<0-1>  Timer Raw Interrupt Status Registers
 *   TIMER_INTMIS<0-1>  Timer Masked Interrupt Status Registers
 *   TIMER_BGLOAD<0-1>  Timer Background Load Value Registers
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

#ifndef AG_MG_REGS_DSS_TIMER_REGISTERS_H
#define AG_MG_REGS_DSS_TIMER_REGISTERS_H

#include "ag_mg_regs_regops.h"






/*
 * TIMER_LOAD<0-1> (Timer Load Value Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TIMER_LOAD_RO                 0x00000000
#define AG_MG_REGS_TIMER_LOAD_RM                 0xFFFFFFFF

#define AG_MG_REGS_TIMER_LOAD_LOADVAL_BO         0
#define AG_MG_REGS_TIMER_LOAD_LOADVAL_BM         0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_LOAD_U
{
    struct
    {
        ag_mg_regs_register
            loadval;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_load_u;
#endif


/*
 * TIMER_VAL<0-1> (Timer Current Value Registers)
 * Initialization value: 0xFFFFFFFF  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TIMER_VAL_RO                  0x00000004
#define AG_MG_REGS_TIMER_VAL_RM                  0xFFFFFFFF

#define AG_MG_REGS_TIMER_VAL_CURRENTVAL_BO       0
#define AG_MG_REGS_TIMER_VAL_CURRENTVAL_BM       0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_VAL_U
{
    struct
    {
        ag_mg_regs_register
            currentval;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_val_u;
#endif


/*
 * TIMER_CTRL<0-1> (Timer Control Registers)
 * Initialization value: 0x00000020  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TIMER_CTRL_RO                 0x00000008
#define AG_MG_REGS_TIMER_CTRL_RM                 0x000000EF

#define AG_MG_REGS_TIMER_CTRL_ONESH_BO           0
#define AG_MG_REGS_TIMER_CTRL_ONESH_BM           0x00000001

#define AG_MG_REGS_TIMER_CTRL_SIZE_BO            1
#define AG_MG_REGS_TIMER_CTRL_SIZE_BM            0x00000002

#define AG_MG_REGS_TIMER_CTRL_PRE_BO             2
#define AG_MG_REGS_TIMER_CTRL_PRE_BM             0x0000000C

#define AG_MG_REGS_TIMER_CTRL_INTEN_BO           5
#define AG_MG_REGS_TIMER_CTRL_INTEN_BM           0x00000020

#define AG_MG_REGS_TIMER_CTRL_MOD_BO             6
#define AG_MG_REGS_TIMER_CTRL_MOD_BM             0x00000040

#define AG_MG_REGS_TIMER_CTRL_EN_BO              7
#define AG_MG_REGS_TIMER_CTRL_EN_BM              0x00000080

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_CTRL_U
{
    struct
    {
        ag_mg_regs_register
            onesh : 1,
            size : 1,
            pre : 2,
            fill1 : 1,
            inten : 1,
            mod : 1,
            en : 1,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_ctrl_u;
#endif


/*
 * TIMER_INTCLR<0-1> (Timer Interrupt Clear Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Write Only
 */
#define AG_MG_REGS_TIMER_INTCLR_RO               0x0000000C
#define AG_MG_REGS_TIMER_INTCLR_RM               0x00000001

#define AG_MG_REGS_TIMER_INTCLR_INTCLR_BO        0
#define AG_MG_REGS_TIMER_INTCLR_INTCLR_BM        0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_INTCLR_U
{
    struct
    {
        ag_mg_regs_register
            intclr : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_intclr_u;
#endif


/*
 * TIMER_INTRIS<0-1> (Timer Raw Interrupt Status Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TIMER_INTRIS_RO               0x00000010
#define AG_MG_REGS_TIMER_INTRIS_RM               0x00000001

#define AG_MG_REGS_TIMER_INTRIS_RIS_BO           0
#define AG_MG_REGS_TIMER_INTRIS_RIS_BM           0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_INTRIS_U
{
    struct
    {
        ag_mg_regs_register
            ris : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_intris_u;
#endif


/*
 * TIMER_INTMIS<0-1> (Timer Masked Interrupt Status Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_TIMER_INTMIS_RO               0x00000014
#define AG_MG_REGS_TIMER_INTMIS_RM               0x00000001

#define AG_MG_REGS_TIMER_INTMIS_MIS_BO           0
#define AG_MG_REGS_TIMER_INTMIS_MIS_BM           0x00000001

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_INTMIS_U
{
    struct
    {
        ag_mg_regs_register
            mis : 1,
            fill0 : 31;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_intmis_u;
#endif


/*
 * TIMER_BGLOAD<0-1> (Timer Background Load Value Registers)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_TIMER_BGLOAD_RO               0x00000018
#define AG_MG_REGS_TIMER_BGLOAD_RM               0xFFFFFFFF

#define AG_MG_REGS_TIMER_BGLOAD_ALTLOADVAL_BO    0
#define AG_MG_REGS_TIMER_BGLOAD_ALTLOADVAL_BM    0xFFFFFFFF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_TIMER_BGLOAD_U
{
    struct
    {
        ag_mg_regs_register
            altloadval;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_timer_bgload_u;
#endif


/*
 * Physical register addresses (for DSS accessing its own Timers)
 */
#define AG_MG_REGS_TIMER_BASE          0xCF002000
#define AG_MG_REGS_TIMER_REG(ro)       (AG_MG_REGS_TIMER_BASE+(ro))

#define AG_MG_REGS_TIMER_RPT                     2
#define AG_MG_REGS_TIMER_IVL                     0x20
// NOTE: t can range from 0 to (AG_MG_REGS_TIMER_RPT - 1)
#define AG_MG_REGS_TIMER_T_REG(t,ro)   AG_MG_REGS_TIMER_REG((t*AG_MG_REGS_TIMER_IVL)+(ro))

#define AG_MG_REGS_TIMER_LOAD_RA(t)    AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_LOAD_RO)
#define AG_MG_REGS_TIMER_VAL_RA(t)     AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_VAL_RO)
#define AG_MG_REGS_TIMER_CTRL_RA(t)    AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_CTRL_RO)
#define AG_MG_REGS_TIMER_INTCLR_RA(t)  AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_INTCLR_RO)
#define AG_MG_REGS_TIMER_INTRIS_RA(t)  AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_INTRIS_RO)
#define AG_MG_REGS_TIMER_INTMIS_RA(t)  AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_INTMIS_RO)
#define AG_MG_REGS_TIMER_BGLOAD_RA(t)  AG_MG_REGS_TIMER_T_REG(t,AG_MG_REGS_TIMER_BGLOAD_RO)

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef struct AG_MG_REGS_TIMER_S
{
    ag_mg_regs_timer_load_u    load;
    ag_mg_regs_timer_val_u     val;
    ag_mg_regs_timer_ctrl_u    ctrl;
    ag_mg_regs_timer_intclr_u  intclr;
    ag_mg_regs_timer_intris_u  intris;
    ag_mg_regs_timer_intmis_u  intmis;
    ag_mg_regs_timer_bgload_u  bgload;
    ag_mg_regs_register        RESERVED;
} ag_mg_regs_timer_s;

typedef struct AG_MG_REGS_TIMER_REGS_S
{
    ag_mg_regs_timer_s timer[AG_MG_REGS_TIMER_RPT];
} ag_mg_regs_timer_reg_s;
/*
 * Recommended C syntax for typical usage :
 *   volatile ag_mg_regs_timer_reg_s *timer_regs =
 *       (volatile ag_mg_regs_timer_reg_s *)AG_MG_REGS_TIMER_BASE;
 */
#endif

#endif

/******** History ********
$Log: ag_mg_regs_timer.h,v $
Revision 1.2  2017/07/28 07:58:35  harrchan
Collapse Oakenshield-branch to Main Trunk.

Revision 1.1.2.1  2017/06/29 08:14:29  harrchan
Initial commit code for Oakenshield

Revision 1.1  2012/04/18 18:08:26  srane
Initial checkin


$Endlog$
*/

