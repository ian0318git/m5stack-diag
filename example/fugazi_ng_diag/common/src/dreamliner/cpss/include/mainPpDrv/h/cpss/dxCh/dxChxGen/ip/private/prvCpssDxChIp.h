/* $Id: prvCpssDxChIp.h,v 1.1 2015/02/13 11:28:37 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h,v $
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
* prvCpssDxChIp.h
*
* DESCRIPTION:
*       Private CPSS DXCH Ip HW structures
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/
#ifndef __prvCpssDxChIph
#define __prvCpssDxChIph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>

/*******************************************************************************
* prvCpssDxChIpConvertUcEntry2HwFormat
*
* DESCRIPTION:
*       This function converts a given ip uc entry to the HW format.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum            - the device number
*       ipUcRouteEntryPtr - The entry to be converted.
*
* OUTPUTS:
*       hwDataPtr  - The entry in the HW format representation.
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpConvertUcEntry2HwFormat
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *ipUcRouteEntryPtr,
    OUT GT_U32                          *hwDataPtr
 );

/*******************************************************************************
* prvCpssDxChIpConvertHwFormat2UcEntry
*
* DESCRIPTION:
*       This function converts a given ip uc entry to the HW format.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum             - the device number
*       hwDataPtr          - The entry in the HW format representation.
*       ipUcRouteEntryPtr  - Set the route entry type to determine how hw data will be
*                         interpreted
*
* OUTPUTS:
*       ipUcRouteEntryPtr - The entry to be converted.
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpConvertHwFormat2UcEntry
(
    IN  GT_U8                               devNum,
    IN  GT_U32 *hwDataPtr,
    INOUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *ipUcRouteEntryPtr
);

/*******************************************************************************
* prvCpssDxChIpUcRouteEntriesCheck
*
* DESCRIPTION:
*       Check validity of the route entry parametrers, in all entries
*       of routeEntriesArray.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum               - the device number
*       routeEntriesArray    - the uc route entries array
*       numOfRouteEntries    - the number route entries in the array.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on all valid parameters.
*       GT_BAD_PARAM    - on invalid parameter.
*       GT_OUT_OF_RANGE - on out of range parameter.
*       GT_BAD_PTR      - on NULL pointer.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpUcRouteEntriesCheck
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                          numOfRouteEntries
);


/*******************************************************************************
* prvCpssDxChIpConvertMcEntry2HwFormat
*
* DESCRIPTION:
*       This function converts a given ip mc entry to the HW format.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum            - the device number
*       ipMcRouteEntryPtr - The entry to be converted.
*
* OUTPUTS:
*       hwDataPtr  - The entry in the HW format representation.
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpConvertMcEntry2HwFormat
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *ipMcRouteEntryPtr,
    OUT GT_U32                          *hwDataPtr
);

/*******************************************************************************
* prvCpssDxChIpMcRouteEntryCheck
*
* DESCRIPTION:
*       Check validity of the route entry parametrers, in all entries
*       of routeEntriesArray.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       routeEntryPtr    - the Mc route entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on all valid parameters.
*       GT_BAD_PARAM    - on invalid parameter.
*       GT_OUT_OF_RANGE - on out of range parameter.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpMcRouteEntryCheck
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
);


/*******************************************************************************
* prvCpssDxChIpConvertHwFormat2McEntry
*
* DESCRIPTION:
*       This function converts a given ip mc entry to the HW format.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum       - the device number
*       hwDataPtr    - The mc entry in the HW format to be converted..
*
* OUTPUTS:
*       ipMcRouteEntryPtr - The mc entry data.
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpConvertHwFormat2McEntry
(
    IN   GT_U8                           devNum,
    OUT  CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *ipMcRouteEntryPtr,
    IN   GT_U32                          *hwDataPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChIph */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChIp.h,v $
 * Revision 1.1  2015/02/13 11:28:37  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
