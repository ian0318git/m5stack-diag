/* $Id: prvCpssDxChTrunk.h,v 1.1 2015/02/13 11:32:58 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h,v $
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
* prvCpssDxChTrunk.h
*
* DESCRIPTION:
*       API definitions for 802.3ad Link Aggregation (Trunk) facility
*       private - CPSS - DxCh
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __prvCpssDxChTrunkh
#define __prvCpssDxChTrunkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>

/*******************************************************************************
* prvCpssDxChTrunkFamilyInit
*
* DESCRIPTION:
*       This function sets CPSS private DxCh family info about trunk behavior
*       that relate to the family and not to specific device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devFamily - device family.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success,
*       GT_FAIL                  - on failure.
*       GT_NOT_INITIALIZED       - the family was not initialized properly
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChTrunkFamilyInit(
    IN CPSS_PP_FAMILY_TYPE_ENT  devFamily
);


/*******************************************************************************
* prvCpssDxChTrunkTrafficToCpuWaUpInit
*
* DESCRIPTION:
*       initialize the UP (user priority) to port settings,needed WA for :
*  "wrong trunk id source port information of packet to CPU"
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum              - device number of the device that we set member on.
*       additionalInfoBmp   - bitmap for additional WA info
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaUpInit
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  additionalInfoBmp
);

/*******************************************************************************
* prvCpssDxChTrunkTrafficToCpuWaUpToMemberConvert
*
* DESCRIPTION:
*       convert the UP (user priority) to trunk member {dev,port}
*       Needed for WA of "wrong trunk id source port information of packet to CPU" erratum
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - device number of the device that connected to CPU port/SDMA.
*       dsaTagInfoPtr - (pointer to) the DSA tag info parsed from the DSA tag.
*
* OUTPUTS:
*       dsaTagInfoPtr - (pointer to) the DSA tag info with fixed info regarding
*                       "wrong trunk id source port information of packet to CPU"
*
* RETURNS:
*       GT_OK   - on success , the DSA tag was 'fixed' (updated)
*       GT_NO_CHANGE - there is no need for 'convert'
*       GT_DSA_PARSING_ERROR - No valid mapping between the UP and trunk member.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaUpToMemberConvert
(
    IN  GT_U8   devNum,
    INOUT CPSS_DXCH_NET_DSA_PARAMS_STC  *dsaTagInfoPtr
);

/*******************************************************************************
* prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet
*
* DESCRIPTION:
*       enable/disable the needed for the WA for the CPU/cascade port that may
*       represent the pass to the CPU
*       WA of "wrong trunk id source port information of packet to CPU" erratum
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - device number of the device that we set member on.
*       portNum   - the CPU/cascade port that may represent the pass to the CPU
*       enable    - enable/disable the settings
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success , or device not need the WA
*       GT_BAD_PARAM             - on wrong parameters
*       GT_HW_ERROR              - on hardware error
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChTrunkTrafficToCpuWaCpuPortEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/*******************************************************************************
* prvCpssDxChTrunkHwDevNumSet
*
* DESCRIPTION:
*        Update trunk logic prior change of HW device number.
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       hwDevNum - HW device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on wrong parameters
*       GT_HW_ERROR     - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChTrunkHwDevNumSet
(
    IN GT_U8            devNum,
    IN GT_HW_DEV_NUM    hwDevNum
);

/*******************************************************************************
* prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet
*
* DESCRIPTION:
*
*       the function sets the designated device table with the portsArr[].trunkPort
*       in indexes that match hash (%8 or %64(modulo)) on the ports in portsArr[].srcPort
*       this to allow 'Src port' trunk hash for traffic sent to the specified
*       trunk.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number
*       numOfPorts  - number of pairs in array portsArr[].
*       portsArr[] - (array of) pairs of 'source ports' ,'trunk ports'
*                     for the source port hash.
*       mode - hash mode (%8 or %64 (modulo))
*
* OUTPUTS:
*       None
*
* RETURNS:
*    GT_OK        - on success
*    GT_HW_ERROR  - on hardware error
*    GT_BAD_PARAM - on wrong devNum or trunkId or port in portsArr[].srcPort or
*                   port in portsArr[].trunkPort or mode
*                   or map two Source ports that falls into same Source hash index
*                   into different trunk member ports
*    GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChTrunkDesignatedTableForSrcPortHashMappingSet
(
    IN GT_U8             devNum,
    IN GT_U32            numOfPorts,
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_PORT_HASH_PAIR_STC  portsArr[],
    IN CPSS_DXCH_MULTI_PORT_GROUP_SRC_HASH_MODE_ENT  mode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChTrunkh */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChTrunk.h,v $
 * Revision 1.1  2015/02/13 11:32:58  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
