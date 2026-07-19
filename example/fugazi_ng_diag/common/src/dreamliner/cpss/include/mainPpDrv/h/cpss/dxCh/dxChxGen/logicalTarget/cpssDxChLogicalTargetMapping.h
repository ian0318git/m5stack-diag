/* $Id: cpssDxChLogicalTargetMapping.h,v 1.1 2015/02/13 11:32:09 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget/cpssDxChLogicalTargetMapping.h,v $
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
* cpssDxChLogicalTargetMapping.h
*
* DESCRIPTION:
*       Logical Target mapping.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssDxChlogicalTargetMappingh
#define __cpssDxChlogicalTargetMappingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

/*******************************************************************************
* cpssDxChLogicalTargetMappingEnableSet
*
* DESCRIPTION:
*       Enable/disable logical port mapping feature on the specified device.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* INPUTS:
*       devNum   - physical device number
*       enable   - GT_TRUE:  enable
*                  GT_FALSE: disable .
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChLogicalTargetMappingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);


/*******************************************************************************
* cpssDxChLogicalTargetMappingEnableGet
*
* DESCRIPTION:
*       Get the Enable/Disable status logical port mapping feature on the
*       specified  device.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum    - physical device number
*
* OUTPUTS:
*       enablePtr - Pointer to the enable/disable state.
*                   GT_TRUE : enable,
*                   GT_FALSE: disable
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChLogicalTargetMappingEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *enablePtr
);



/*******************************************************************************
* cpssDxChLogicalTargetMappingDeviceEnableSet
*
* DESCRIPTION:
*       Enable/disable a target device to be considered as a logical device
*       on the specified device.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* INPUTS:
*       devNum        - physical device number
*       logicalDevNum - logical device number.
*                       (APPLICABLE RANGES: 24..31)
*       enable        - GT_TRUE:  enable
*                       GT_FALSE: disable .
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum, logicalDevNum
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChLogicalTargetMappingDeviceEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     logicalDevNum,
    IN  GT_BOOL   enable
);


/*******************************************************************************
* cpssDxChLogicalTargetMappingDeviceEnableGet
*
* DESCRIPTION:
*       Get Enable/disable status of target device to be considered as a logical device
*       on the specified device.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* INPUTS:
*       devNum        - physical device number
*       logicalDevNum - logical device number.
*                       (APPLICABLE RANGES: 24..31)
*
* OUTPUTS:
*       enablePtr     - Pointer to the  Enable/disable state.
*                       GT_TRUE : enable,
*                       GT_FALSE: disable
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum, logicalDevNum
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChLogicalTargetMappingDeviceEnableGet
(
    IN   GT_U8     devNum,
    IN  GT_U8      logicalDevNum,
    OUT  GT_BOOL   *enablePtr
);

/*******************************************************************************
* cpssDxChLogicalTargetMappingTableEntrySet
*
* DESCRIPTION:
*       Set logical target mapping table entry.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum                       - physical device number.
*       logicalDevNum                - logical device number.
*                                      (APPLICABLE RANGES: 24..31)
*       logicalPortNum               - logical port number.
*                                      (APPLICABLE RANGES: 0..63)
*       logicalPortMappingTablePtr   - points to logical Port Mapping  entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success.
*       GT_BAD_PARAM    - on wrong devNum, logicalDevNum, logicalPortNum
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_OUT_OF_RANGE - on the memebers of virtual port entry struct out of range.
*       GT_HW_ERROR     - on Hardware error.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChLogicalTargetMappingTableEntrySet
(
    IN GT_U8                           devNum,
    IN GT_U8                           logicalDevNum,
    IN GT_U8                           logicalPortNum,
    IN CPSS_DXCH_OUTPUT_INTERFACE_STC  *logicalPortMappingTablePtr
);

/*******************************************************************************
* cpssDxChLogicalTargetMappingTableEntryGet
*
* DESCRIPTION:
*       Get logical target mapping table entry.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum                       - physical device number.
*       logicalDevNum                - logical device number.
*                                      (APPLICABLE RANGES: 24..31)
*       logicalPortNum               - logical port number.
*                                      (APPLICABLE RANGES: 0..63)
*
* OUTPUTS:
*       logicalPortMappingTablePtr   - points to logical Port Mapping  entry
*
* RETURNS:
*       GT_OK           - on success.
*       GT_BAD_PARAM    - on wrong devNum, logicalDevNum, logicalPortNum
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_OUT_OF_RANGE - on the memebers of virtual port entry struct out of range.
*       GT_HW_ERROR     - on Hardware error.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChLogicalTargetMappingTableEntryGet
(
    IN GT_U8                            devNum,
    IN GT_U8                            logicalDevNum,
    IN GT_U8                            logicalPortNum,
    OUT CPSS_DXCH_OUTPUT_INTERFACE_STC  *logicalPortMappingTablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChlogicalTargetMapping */
/*
 *------------------------------------------------------------------
 * $Log: cpssDxChLogicalTargetMapping.h,v $
 * Revision 1.1  2015/02/13 11:32:09  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
