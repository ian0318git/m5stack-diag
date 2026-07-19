/* $Id: cpssDxChBrgFdbHash.h,v 1.1 2015/02/13 11:31:30 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h,v $
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
* cpssDxChBrgFdbHash.h
*
* DESCRIPTION:
*       Hash calculate for MAC address table implementation.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDxChBrgFdbHashh
#define __cpssDxChBrgFdbHashh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>

/*******************************************************************************
* cpssDxChBrgFdbHashCalc
*
* DESCRIPTION:
*       This function calculates the hash index for the FDB table.
*       The FDB table hold 2 types of entries :
*       1. for specific mac address and VLAN id.
*       2. for specific src IP , dst IP and VLAN id.
*
*       for more details see CPSS_MAC_ENTRY_EXT_KEY_STC description.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number
*       macEntryKeyPtr  - pointer to key parameters of the mac entry
*                         according to the entry type:
*                         MAC Address entry -> MAC Address + vlan ID.
*                         IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* OUTPUTS:
*       hashPtr - (pointer to) the hash index.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - wrong devNum, bad vlan-Id
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChBrgFdbHashCalc
(
    IN  GT_U8           devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr,
    OUT GT_U32          *hashPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgFdbHashh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChBrgFdbHash.h,v $
 * Revision 1.1  2015/02/13 11:31:30  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
