/* $Id: cpssDxChTypes.h,v 1.1 2015/02/13 11:31:27 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssDxChTypes.h,v $
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
* cpssDxChTypes.h
*
* DESCRIPTION:
*       CPSS DXCH Generic types.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssDxChTypesh
#define __cpssDxChTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/generic/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>

/*
 * typedef: struct CPSS_DXCH_OUTPUT_INTERFACE_STC
 *
 * Description: Defines the interface info
 *
 * Fields:
 *   isTunnelStart      - if set to GT_TRUE, packet is to be tunneled.
 *   tunnelStartInfo    - Tunnel Start information (Relevant if isTunnelStart== GT_TRUE)
 *              passengerPacketType - Type of passenger packet going to be
 *                                    encapsulated.
 *              ptr                 - A pointer to a tunnel entry
 *   physicalInterface   - defines the phisical interface info. Reffer to
 *                         CPSS_INTERFACE_INFO_STC. The vidx and vlanId fields
 *                         are not in use in this case.
 *
 * Notes:
 *      1. The structure is used in following cases:
 *          a. Logical Target Mapping
 *
 */

typedef struct
{
    GT_BOOL                 isTunnelStart;

    struct{
        CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType;
        GT_U32                              ptr;
    }tunnelStartInfo;

    CPSS_INTERFACE_INFO_STC physicalInterface;
} CPSS_DXCH_OUTPUT_INTERFACE_STC;

/*
 * typedef: enum CPSS_DXCH_MEMBER_SELECTION_MODE_ENT
 *
 *      Description: enumerator that hold values for the type of how many bits
 *      are used in a member selection function.
 *      Used for trunk member selection and by L2 ECMP member selection.
 *
 * Enumerations:
 *      CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E -
 *                  Use the entire 12 bit hash in the member selection function.
 *                  ((Hash[11:0] * #members)/4096)
 *
 *      CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E -
 *                  Use only the 6 least significant bits in the member selection.
 *                  ((Hash[5:0] * #members)/64)
 *
 *      CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E -
 *                  Use only the 6 most significant bits in the member selection.
 *                  ((Hash[11:6] * #members)/64)
 *
 */
typedef enum {
    CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E = 0,
    CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E,
    CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E
}CPSS_DXCH_MEMBER_SELECTION_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_FDB_LEARN_PRIORITY_ENT
 *
 * Description: Enumeration of FDB Learn Priority
 *
 * Enumerations:
 *      CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E - low priority
 *      CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E - high priority
 */
typedef enum
{
    CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E,
    CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E
} CPSS_DXCH_FDB_LEARN_PRIORITY_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTypesh */
/*
 *------------------------------------------------------------------
 * $Log: cpssDxChTypes.h,v $
 * Revision 1.1  2015/02/13 11:31:27  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
