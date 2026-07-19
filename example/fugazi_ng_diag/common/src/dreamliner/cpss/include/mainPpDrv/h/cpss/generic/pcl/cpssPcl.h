/* $Id: cpssPcl.h,v 1.1 2015/02/13 11:34:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/pcl/cpssPcl.h,v $
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
* cpssPcl.h
*
* DESCRIPTION:
*       Common PCL definitions.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssPclh
#define __cpssPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * typedef: CPSS_PCL_DIRECTION_ENT
 *
 * Description:
 *      PCL direction.
 * Fields:
 *      CPSS_PCL_DIRECTION_INGRESS_E - The PCL will work on ingress traffic.
 *      CPSS_PCL_DIRECTION_EGRESS_E  - The PCL will work on egress traffic.
 *
 * Comment:
 */
typedef enum
{
    CPSS_PCL_DIRECTION_INGRESS_E = 0,
    CPSS_PCL_DIRECTION_EGRESS_E
} CPSS_PCL_DIRECTION_ENT;

/*
 * typedef: CPSS_PCL_LOOKUP_NUMBER_ENT
 *
 * Description:
 *      Number of lookup for the PCL
 *
 * Fields:
 *      CPSS_PCL_LOOKUP_0_E - first lookup #0.
 *      CPSS_PCL_LOOKUP_1_E - second lookup #1.
 *      CPSS_PCL_LOOKUP_0_0_E - the 0-th sub lookup of lookup #0
 *                       (APPLICABLE DEVICES: xCat2)
 *      CPSS_PCL_LOOKUP_0_1_E - the 1-th sub lookup of lookup #0
 *                       (APPLICABLE DEVICES: xCat2)
 *
 *
 * Comment:
 */
typedef enum
{
    CPSS_PCL_LOOKUP_0_E = 0,
    CPSS_PCL_LOOKUP_1_E,
    CPSS_PCL_LOOKUP_0_0_E,
    CPSS_PCL_LOOKUP_0_1_E
} CPSS_PCL_LOOKUP_NUMBER_ENT;

/*
 * Typedef: enum CPSS_PCL_RULE_SIZE_ENT
 *
 * Description:
 *     This enum describes sizes of Policy Rules.
 * Fields:
 *     CPSS_PCL_RULE_SIZE_STD_E           - standard size rule
 *     CPSS_PCL_RULE_SIZE_EXT_E           - extended size rule
 *     CPSS_PCL_RULE_SIZE_ULTRA_E         - ultra size rule
 *                (APPLICABLE DEVICES: Lion2)
 *                Used for extended rules with ePort-eVlan
 *                Architecture extension.
 *
 *  Comments:
 */
typedef enum
{
    CPSS_PCL_RULE_SIZE_STD_E,
    CPSS_PCL_RULE_SIZE_EXT_E,
    CPSS_PCL_RULE_SIZE_ULTRA_E
} CPSS_PCL_RULE_SIZE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPclh */

/*
 *------------------------------------------------------------------
 * $Log: cpssPcl.h,v $
 * Revision 1.1  2015/02/13 11:34:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
