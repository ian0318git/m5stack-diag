/* $Id: ag_mg_regs_gpio.h,v 1.1 2012/04/18 18:08:25 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/dss/inc/ag_mg_regs_gpio.h,v $
 *------------------------------------------------------------------
 * ag_mg_regs_gpio.h
 *
 * Mar 2012, Smita Rane
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
 * ag_mg_regs_gpio.h
 *
 * Copyright (c) 2010 LSI Inc.  All Rights Reserved
 *
 * This is unpublished proprietary information of LSI Inc.
 * This copyright notice does not evidence publication.
 *
 * The use of the software, documentation, methodologies, and other information
 * contained herein is governed solely by the associated license agreements.
 * Any inconsistent use shall be deemed to be a misappropriation of the
 * intellectual property of LSI Inc. and treated accordingly.
 *
 * This file defines registers for the Packet Processor Block (PPB)
 * 8 bit general-purpose input/output (GPIO) block.
 *
 * This file also defines registers for each DSP Subsystem (DSS) to access
 * it's 2 bit general-purpose input/output (GPIO) block.
 *
 * Register addresses defined herein are for code running
 * on the ARM accessing the PPB GPIO block via the APB bus,
 * the DSS accessing the PPB GPIO block via the DBM, and for
 * the DSS accessing their own GPIO block over the APB
 *
 * See "Physical register addresses" below for macros
 * used to generate addresses for specific registers.
 *
 * Registers defined in this file :
 *   GPIO_DATA   GPIO Data Register
 *   GPIO_DIR    GPIO Data Direction Register
 *   GPIO_IS     GPIO Interrupt Sense Register
 *   GPIO_IBE    GPIO Interrupt Both Edges Register
 *   GPIO_IEV    GPIO Interrupt Event Register
 *   GPIO_IE     GPIO Interrupt Mask Register
 *   GPIO_RIS    GPIO Raw Interrupt Register
 *   GPIO_MIS    GPIO Masked Status Register
 *   GPIO_IC     GPIO Interrupt Clear Register
 *   GPIO_AFSEL  GPIO Mode Control Select Register
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

#ifndef AG_MG_REGS_GPIO_REGISTERS_H
#define AG_MG_REGS_GPIO_REGISTERS_H

#include "ag_mg_regs_regops.h"

/*
 * GPIO_DATA (GPIO Data Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_DATA_RO           0x00000000
#define AG_MG_REGS_GPIO_DATA_RM           0x000000FF

#define AG_MG_REGS_GPIO_DATA_DATA_BO      0
#define AG_MG_REGS_GPIO_DATA_DATA_BM      0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_DATA_U
{
    struct
    {
        ag_mg_regs_register
            data : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_data_u;
#endif


/*
 * GPIO_DIR (GPIO Data Direction Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_DIR_RO            0x00000400
#define AG_MG_REGS_GPIO_DIR_RM            0x000000FF

#define AG_MG_REGS_GPIO_DIR_DIR_BO        0
#define AG_MG_REGS_GPIO_DIR_DIR_BM        0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_DIR_U
{
    struct
    {
        ag_mg_regs_register
            dir : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_dir_u;
#endif


/*
 * GPIO_IS (GPIO Interrupt Sense Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_IS_RO             0x00000404
#define AG_MG_REGS_GPIO_IS_RM             0x000000FF

#define AG_MG_REGS_GPIO_IS_IS_BO          0
#define AG_MG_REGS_GPIO_IS_IS_BM          0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_IS_U
{
    struct
    {
        ag_mg_regs_register
            is : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_is_u;
#endif


/*
 * GPIO_IBE (GPIO Interrupt Both Edges Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_IBE_RO            0x00000408
#define AG_MG_REGS_GPIO_IBE_RM            0x000000FF

#define AG_MG_REGS_GPIO_IBE_IBE_BO        0
#define AG_MG_REGS_GPIO_IBE_IBE_BM        0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_IBE_U
{
    struct
    {
        ag_mg_regs_register
            ibe : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_ibe_u;
#endif


/*
 * GPIO_IEV (GPIO Interrupt Event Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_IEV_RO            0x0000040C
#define AG_MG_REGS_GPIO_IEV_RM            0x000000FF

#define AG_MG_REGS_GPIO_IEV_IEV_BO        0
#define AG_MG_REGS_GPIO_IEV_IEV_BM        0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_IEV_U
{
    struct
    {
        ag_mg_regs_register
            iev : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_iev_u;
#endif


/*
 * GPIO_IE (GPIO Interrupt Mask Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_IE_RO             0x00000410
#define AG_MG_REGS_GPIO_IE_RM             0x000000FF

#define AG_MG_REGS_GPIO_IE_IE_BO          0
#define AG_MG_REGS_GPIO_IE_IE_BM          0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_IE_U
{
    struct
    {
        ag_mg_regs_register
            ie : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_ie_u;
#endif


/*
 * GPIO_RIS (GPIO Raw Interrupt Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_GPIO_RIS_RO            0x00000414
#define AG_MG_REGS_GPIO_RIS_RM            0x000000FF

#define AG_MG_REGS_GPIO_RIS_RIS_BO        0
#define AG_MG_REGS_GPIO_RIS_RIS_BM        0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_RIS_U
{
    struct
    {
        ag_mg_regs_register
            ris : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_ris_u;
#endif


/*
 * GPIO_MIS (GPIO Masked Status Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read Only
 */
#define AG_MG_REGS_GPIO_MIS_RO            0x00000418
#define AG_MG_REGS_GPIO_MIS_RM            0x000000FF

#define AG_MG_REGS_GPIO_MIS_MIS_BO        0
#define AG_MG_REGS_GPIO_MIS_MIS_BM        0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_MIS_U
{
    struct
    {
        ag_mg_regs_register
            mis : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_mis_u;
#endif


/*
 * GPIO_IC (GPIO Interrupt Clear Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Write Only
 */
#define AG_MG_REGS_GPIO_IC_RO             0x0000041C
#define AG_MG_REGS_GPIO_IC_RM             0x000000FF

#define AG_MG_REGS_GPIO_IC_IC_BO          0
#define AG_MG_REGS_GPIO_IC_IC_BM          0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_IC_U
{
    struct
    {
        ag_mg_regs_register
            ic : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_ic_u;
#endif


/*
 * GPIO_AFSEL (GPIO Mode Control Select Register)
 * Initialization value: 0x00000000  Initialization mask: 0xFFFFFFFF
 * Access mode: Read/Write
 */
#define AG_MG_REGS_GPIO_AFSEL_RO          0x00000420
#define AG_MG_REGS_GPIO_AFSEL_RM          0x000000FF

#define AG_MG_REGS_GPIO_AFSEL_AFSEL_BO    0
#define AG_MG_REGS_GPIO_AFSEL_AFSEL_BM    0x000000FF

#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef union AG_MG_REGS_GPIO_AFSEL_U
{
    struct
    {
        ag_mg_regs_register
            afsel : 8,
            fill0 : 24;
    } fields;
    ag_mg_regs_register reg;
} ag_mg_regs_gpio_afsel_u;
#endif


/*
 * Physical register addresses (for ARM accessing the PPB GPIO)
 */
#ifdef AG_MG_REGS_BUILT_FOR_DSS
#define AG_MG_REGS_GPIO_BASE        0xCF004000
#define AG_MG_REGS_PPB_GPIO_BASE    0xC3046000
#else
#define AG_MG_REGS_GPIO_BASE        0x30046000
#define AG_MG_REGS_PPB_GPIO_BASE    0x30046000
#endif
#define AG_MG_REGS_GPIO_REG(ro)     (AG_MG_REGS_GPIO_BASE+(ro))

#define AG_MG_REGS_GPIO_DATA_RA     AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_DATA_RO)
#define AG_MG_REGS_GPIO_DIR_RA      AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_DIR_RO)
#define AG_MG_REGS_GPIO_IS_RA       AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_IS_RO)
#define AG_MG_REGS_GPIO_IBE_RA      AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_IBE_RO)
#define AG_MG_REGS_GPIO_IEV_RA      AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_IEV_RO)
#define AG_MG_REGS_GPIO_IE_RA       AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_IE_RO)
#define AG_MG_REGS_GPIO_RIS_RA      AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_RIS_RO)
#define AG_MG_REGS_GPIO_MIS_RA      AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_MIS_RO)
#define AG_MG_REGS_GPIO_IC_RA       AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_IC_RO)
#define AG_MG_REGS_GPIO_AFSEL_RA    AG_MG_REGS_GPIO_REG(AG_MG_REGS_GPIO_AFSEL_RO)

#ifdef AG_MG_REGS_BUILT_FOR_DSS
/* In addtion to it's own GPIO block, the DSS can also access GPIO block in the PPB
 * but we need different names for these addresess.
 */
#define AG_MG_REGS_PPB_GPIO_REG(ro)     (AG_MG_REGS_PPB_GPIO_BASE+(ro))

#define AG_MG_REGS_PPB_GPIO_DATA_RA AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_DATA_RO)
#define AG_MG_REGS_PPB_GPIO_DIR_RA  AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_DIR_RO)
#define AG_MG_REGS_PPB_GPIO_IS_RA   AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_IS_RO)
#define AG_MG_REGS_PPB_GPIO_IBE_RA  AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_IBE_RO)
#define AG_MG_REGS_PPB_GPIO_IEV_RA  AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_IEV_RO)
#define AG_MG_REGS_PPB_GPIO_IE_RA   AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_IE_RO)
#define AG_MG_REGS_PPB_GPIO_RIS_RA  AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_RIS_RO)
#define AG_MG_REGS_PPB_GPIO_MIS_RA  AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_MIS_RO)
#define AG_MG_REGS_PPB_GPIO_IC_RA   AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_IC_RO)
#define AG_MG_REGS_PPB_GPIO_AFSEL_RA AG_MG_REGS_PPB_GPIO_REG(AG_MG_REGS_GPIO_AFSEL_RO)

#endif


#ifdef AG_MG_REGS_USE_C_STRUCTURES
typedef struct AG_MG_REGS_GPIO_REGS_S
{
    // see the StarPro2603 DSP Data Sheet for information on how to
    // read and write the 8 bidirectional input/output bits using data[]
    ag_mg_regs_gpio_data_u   data[256];
    ag_mg_regs_gpio_dir_u    dir;
    ag_mg_regs_gpio_is_u     is;
    ag_mg_regs_gpio_ibe_u    ibe;
    ag_mg_regs_gpio_iev_u    iev;
    ag_mg_regs_gpio_ie_u     ie;
    ag_mg_regs_gpio_ris_u    ris;
    ag_mg_regs_gpio_mis_u    mis;
    ag_mg_regs_gpio_ic_u     ic;
    ag_mg_regs_gpio_afsel_u  afsel;
} ag_mg_regs_gpio_reg_s;
/*
 * Recommended C syntax for typical usage :
 *   volatile ag_mg_regs_gpio_reg_s *gpio_regs =
 *       (volatile ag_mg_regs_gpio_reg_s *)AG_MG_REGS_GPIO_BASE;
 *   volatile ag_mg_regs_gpio_reg_s *ppb_gpio_regs =
 *       (volatile ag_mg_regs_gpio_reg_s *)AG_MG_REGS_PPB_GPIO_BASE;
 */
#endif

#endif

/******** History ********
$Log: ag_mg_regs_gpio.h,v $
Revision 1.1  2012/04/18 18:08:25  srane
Initial checkin


$Endlog$
*/

