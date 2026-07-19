/* $Id: cpssGenPhySmi.h,v 1.1 2015/02/13 11:34:05 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/phy/cpssGenPhySmi.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssGenPhySmi.h
*
* DESCRIPTION:
*       API implementation for port Core Serial Management Interface facility.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssGenPhySmih
#define __cpssGenPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * typedef: enum CPSS_PHY_SMI_INTERFACE_ENT
 *
 * Description: Generic Serial Management Interface numbering
 *
 * Enumerations:
 *      CPSS_PHY_SMI_INTERFACE_0_E  - SMI interface\controller 0
 *      CPSS_PHY_SMI_INTERFACE_1_E  - SMI interface\controller 1
 *      CPSS_PHY_SMI_INTERFACE_MAX_E - to simplify validity check
 */
typedef enum
{
    CPSS_PHY_SMI_INTERFACE_0_E,
    CPSS_PHY_SMI_INTERFACE_1_E,
    CPSS_PHY_SMI_INTERFACE_MAX_E = CPSS_PHY_SMI_INTERFACE_1_E

} CPSS_PHY_SMI_INTERFACE_ENT;

/*
 * typedef: enum CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT
 *
 * Description: Generic Serial Management Interface clock (MDC)
 *              division factor
 *
 * Enumerations:
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E    - 8 division: Core clock divided by 8
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E   - 16 division: Core clock divided by 16
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E   - 32 division: Core clock divided by 32
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E   - 64 division: Core clock divided by 64
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E  - 128 division: Core clock divided by 128
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E  - 256 division: Core clock divided by 256
 *      CPSS_PHY_SMI_MDC_DIVISION_FACTOR_MAX_E  - to simplify validity check
 */
typedef enum
{
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E,
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E,
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E,
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E,
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E,
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E,
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_MAX_E = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E

} CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT;

/*
 * typedef: enum CPSS_PHY_ERRATA_WA_ENT
 *
 * Description: PHY errata workarond list
 *
 * Enumerations:
 *      CPSS_PHY_ERRATA_WA_88E1240_SGMII_LOCKUP_E  - Workaround for 88E1240
 *                  SGMII lock-up problem. Should be used only after packet
 *                  processor PHY SMI lib init is finished and SGMII
 *                  autonegotiation between the PHY and the packet processor
 *                  reached synchronization. The PHY SGMII autonegotiation
 *                  should be disabled if the packet processor uses
 *                  out-of-band PHY autonegotiation.
 *
 *      CPSS_PHY_ERRATA_WA_88E1340_BGA_INIT_E - Workaround for 88E1340 BGA
 *                  package init. If used, should be after hard-reset
 *                  de-asserted for PHY initialization.
 *                  Should only be applied for BGA package.
 */
typedef enum
{
    CPSS_PHY_ERRATA_WA_88E1240_SGMII_LOCKUP_E,
    CPSS_PHY_ERRATA_WA_88E1340_BGA_INIT_E

} CPSS_PHY_ERRATA_WA_ENT;

/*******************************************************************************
* cpssPhyErrataWaExecute
*
* DESCRIPTION:
*   This function execute workaround for PHY errata from a selected list.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - Device Number.
*       portNum     - Port Number.
*       errataWa    - the selected workaround to execute
*
* OUTPUTS:
*       None.
*
* RETURNS :
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_INITIALIZED       - on not initialized function pointers for
*                                  read and write phy registers operations.
*       GT_BAD_PARAM             - on wrong device, port or requested WA
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssPhyErrataWaExecute
(
   IN GT_U8                     devNum,
   IN GT_PHYSICAL_PORT_NUM      portNum,
   IN CPSS_PHY_ERRATA_WA_ENT    errataWa
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPhySmih */

/*
 *------------------------------------------------------------------
 * $Log: cpssGenPhySmi.h,v $
 * Revision 1.1  2015/02/13 11:34:05  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
