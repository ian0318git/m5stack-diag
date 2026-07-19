/* $Id: prvCpssDxChBrgFdbHash.h,v 1.1 2015/02/13 11:31:32 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHash.h,v $
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
*
* prvCpssDxChBrgFdbHash.h
*
* DESCRIPTION:
*       MAC hash struct implementation.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChBrgFdbHashh
#define __prvCpssDxChBrgFdbHashh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>

/*
 * typedef: enum PRV_CPSS_DXCH_MAC_TBL_SIZE_ENT
 *
 * Description: Enumeration of MAC Table size in entries
 *
 * Enumerations:
 *   PRV_CPSS_DXCH_MAC_TBL_SIZE_8K_E  - 8K Mac Address table entries.
 *   PRV_CPSS_DXCH_MAC_TBL_SIZE_16K_E - 16K Mac Address table entries.
 *   PRV_CPSS_DXCH_MAC_TBL_SIZE_32K_E - 32K Mac Address table entries.
 *   PRV_CPSS_DXCH_MAC_TBL_SIZE_64K_E - 64K Mac Address table entries.
 */
typedef enum
{
    PRV_CPSS_DXCH_MAC_TBL_SIZE_8K_E = 0,
    PRV_CPSS_DXCH_MAC_TBL_SIZE_16K_E,
    PRV_CPSS_DXCH_MAC_TBL_SIZE_32K_E,
    PRV_CPSS_DXCH_MAC_TBL_SIZE_64K_E
} PRV_CPSS_DXCH_MAC_TBL_SIZE_ENT;

/*
 * Typedef: PRV_CPSS_DXCH_MAC_HASH_STC
 *
 * Description: struct contains the hardware parameters for hash mac Address
 *              calculation.
 *
 * Fields:
 *       vlanMode    - the VLAN lookup mode.
 *       hashMode    - the FDB hash function mode.
 *       size        - the entries number in the hash table.
 */
typedef struct
{
    CPSS_MAC_VL_ENT                 vlanMode;
    CPSS_MAC_HASH_FUNC_MODE_ENT     hashMode;
    PRV_CPSS_DXCH_MAC_TBL_SIZE_ENT  size;
}PRV_CPSS_DXCH_MAC_HASH_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbHashh */
/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChBrgFdbHash.h,v $
 * Revision 1.1  2015/02/13 11:31:32  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
