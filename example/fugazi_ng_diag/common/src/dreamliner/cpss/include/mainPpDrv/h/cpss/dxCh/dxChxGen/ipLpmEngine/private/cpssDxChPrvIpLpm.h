/* $Id: cpssDxChPrvIpLpm.h,v 1.1 2015/02/13 11:32:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h,v $
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
* cpssDxChPrvIpLpm.h
*
* DESCRIPTION:
*       This file includes internal typedefs declarations for controlling the
*       IP tables and structures, and structures definitions for shadow
*       management.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDxChPrvIpLpmh
#define __cpssDxChPrvIpLpmh

#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManagerTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Pointers to dummy trie nodes to support policy based routing MC defaults */
extern GT_U32                              pbrDefaultMcTrieNodePtrArraySize;
extern PRV_CPSS_DXCH_IP_PAT_TRIE_NODE_STC  **pbrDefaultMcTrieNodePtrArray;

/*******************************************************************************
* prvCpssDxChMoveToLocation
*
* DESCRIPTION:
*       Copy single entry from one TCAM location to another TCAM location
*       and to invalidate the entry in the original location
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       fromPtr           - points to the original hardware TCAM location
*       toPtr             - points to the new hardware TCAM location
*       clientCookiePtr   - points to the client cookie associated with the entry
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS prvCpssDxChMoveToLocation
(
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *fromPtr,
    IN  CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC    *toPtr,
    IN  GT_VOID                                     *clientCookiePtr
);

/*******************************************************************************
* prvCpssDxChMoveToAnywhere
*
* DESCRIPTION:
*       Move single entry from TCAM location and to invalidate the entry in
*       the original location.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       clientCookiePtr   - points to the client cookie associated with the entry
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS prvCpssDxChMoveToAnywhere
(
    IN  GT_VOID         *clientCookiePtr
);

/*******************************************************************************
* prvCpssDxChCheckCapacityForTcamShadow
*
* DESCRIPTION:
*       This function checks the capacity given in lpb db create or capacity update.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       shadowType              - the type of shadow we're building
*       indexesRangePtr         - the range of TCAM indexes availble for this
*                                 LPM DB (see explanation in
*                                 CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC)
*       partitionEnable         - weather to enable partitioning of the router
*                                 TCAM according to the
*                                 tcamLpmManagerCapcaityCfgPtr.
*       capacityCfgPtr          - The capcity configuration struct.
*
* OUTPUTS:
*       totalLinesPtr           - total lines needed
*       totalSingleEntriesPtr   - total single entries needed
*       totalQuadEntriesPtr     - total quad entries needed
*
* RETURNS:
*       GT_OK on success, or
*       GT_BAD_PARAM on bad capacityCfg.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChCheckCapacityForTcamShadow
(
    IN CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT                shadowType,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC     *indexesRangePtr,
    IN GT_BOOL                                          partitionEnable,
    IN  CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    *capacityCfgPtr,
    OUT GT_U32                                          *totalLinesPtr,
    OUT GT_U32                                          *totalSingleEntriesPtr,
    OUT GT_U32                                          *totalQuadEntriesPtr
);

/*******************************************************************************
* prvCpssDxChPrepareTcamReservation
*
* DESCRIPTION:
*       This function returns a pointer to a new IP shadow struct.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       shadowType                  - the type of shadow we're building (cheetah+ or cheetah2)
*       partitionEnable             - weather to enable partitioning of the router
*                                     TCAM according to the
*                                     tcamLpmManagerCapacityCfgPtr.
*       capacityCfgPtr              - The capcity configuration struct.
*       totalPlacesNeeded           - total places needed
*       totalCapacity               - total number of lines possible
*       sizeOfArrayPtr              - points to allocated size of the
*                                     requestedEntriesArrayPtr array
*
* OUTPUTS:
*       reservationTypePtr          - type of reservation needed
*       requestedEntriesArrayPtr    - array of amounts needed per entry type
*       sizeOfArrayPtr              - number of elements filled in the
*                                     requestedEntriesArrayPtr array
*       numberOfUnusedTcamEntriesPtr  - points to unused Tcam entires
*
* RETURNS:
*       GT_OK on success, or
*       GT_BAD_PARAM on bad capacityCfg.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChPrepareTcamReservation
(
    IN    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT                   shadowType,
    IN    GT_BOOL                                             partitionEnable,
    IN    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC        *capacityCfgPtr,
    IN    GT_U32                                              totalPlacesNeeded,
    IN    GT_U32                                              totalCapacity,
    OUT   CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT *reservationTypePtr,
    OUT   CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC             *requestedEntriesArrayPtr,
    INOUT GT_U32                                              *sizeOfArrayPtr,
    OUT   GT_U32                                              *numberOfUnusedTcamEntriesPtr
);


/*******************************************************************************
* prvCpssDxChIpTcamFreeSpaceCountersCheck
*
* DESCRIPTION:
*       This function check whether TCAM has enough free space for
*       added IPv4/Ipv6 prefixes.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       numberOfPrefixes      - number of added prefixes
*       ipMode                - ip uc/mc selection
*       mcEntryType           - mc entry type: relevant only if ipMode is mc.
*       protocolStack         - the protocol stack to support
*       ipTcamShadowPtr       - ip shadow we are working on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                 - on success.
*       GT_BAD_PARAM          - on wrong number.
*       GT_FULL               - if TCAM is full.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpTcamFreeSpaceCountersCheck
(
    IN GT_U32                                          numberOfPrefixes,
    IN CPSS_IP_UNICAST_MULTICAST_ENT                   ipMode,
    IN PRV_CPSS_DXCH_IP_LPM_MC_ENTRY_TYPE_ENT          mcEntryType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack,
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC                *ipTcamShadowPtr
);

/*******************************************************************************
* cpssDxChIpTcamInit
*
* DESCRIPTION:
*       Init the Ip unit
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       shadowType              - the type of shadow we're building (cheetah+ or
*                                 cheetah2)
*       rangeIndexsPtr          - the range of TCAM indexes availble for this
*                                 LPM DB (see explanation in
*                                 CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC)
*       partitionEnable         - weather to enable partitioning of the router
*                                 TCAM according to the
*                                 tcamLpmManagercapacityCfgPtr.
*       capacityCfgPtr          - The capcity configuration struct.
*       protocolStack           - the protocol stack to support
*       tcamManagerHandlerPtr   - the TCAM manager handler
*
* OUTPUTS:
*       ipTcamShadowPtrPtr - the ip shadow we created.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamInit
(
    IN  CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT                shadowType,
    IN  CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC     *rangeIndexsPtr,
    IN  GT_BOOL                                          partitionEnable,
    IN  CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC     *capacityCfgPtr,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                       protocolStack,
    IN  GT_VOID                                          *tcamManagerHandlerPtr,
    OUT PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC                 **ipTcamShadowPtrPtr
);

/*******************************************************************************
* deleteIpTcamShadowStruct
*
* DESCRIPTION:
*       This function deletes IP TCAM shadow struct.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       ipTcamShadowPtr       - A pointer to the ip shadow struct.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                 - on success
*       GT_FAIL               - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS deleteIpTcamShadowStruct
(
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC    *ipTcamShadowPtr
);

/*******************************************************************************
* cpssDxChIpTcamCheetah2VrSupportInit
*
* DESCRIPTION:
*       Init the Ip unit for Cheetah2 devices with virtual router support.
*
* APPLICABLE DEVICES:
*        DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       protocolStack    - the type of protocol stack this LPM DB support.
*       pclTcamCfgPtr    - the pcl tcam configuration
*       routerTcamCfgPtr - the router tcam configuration
*
* OUTPUTS:
*       ipTcamShadowPtrPtr  - the ip shadow we created.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamCheetah2VrSupportInit
(
    IN  CPSS_IP_PROTOCOL_STACK_ENT                              protocolStack,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CFG_STC *pclTcamCfgPtr,
    IN CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CFG_STC *routerTcamCfgPtr,
    OUT PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC                        **ipTcamShadowPtrPtr
);

/*******************************************************************************
* cpssDxChIpTcamAddVirtualRouter
*
* DESCRIPTION:
*       This function adds a new virtual router to the valid virtual routers
*       in system for specific shared shadow devices.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       vrId                        - The virtual's router ID (in cheetah+ this
*                                     will be the pcl id).
*       defIpv4UcNextHopInfoPtr     - The next hop info representing the default IPv4-Uc
*                                     route entry.
*       defIpv6UcNextHopInfoPtr     - The next hop info representing the default IPv6-Uc
*                                     route entry.

*       defUcNextHopRuleIdxArr      - if not NULL, then this is the internal rule idx
*                                     of the defualt NextHop prefix. (array of 2
*                                     for each of the protocol Stacks)
*       defIpv4McRouteLttEntryPtr   - The default IPv4_Mc LTT entry
*       defIpv6McRouteLttEntryPtr   - The default IPv6_Mc LTT entry
*       defMcRouteRuleIdxArr        - if not null then this is the internal Rule idx
*                                     of the default mc route (prefix) (array of 2
*                                     for each of the protocol Stacks)
*       protocolStack               - types of IP stack used in this virtual router.
*       ipTcamShadowPtr             - the ip shadow we are working on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - if success
*       GT_NOT_FOUND             - if the given next hop Id is not valid.
*       GT_ALREADY_EXIST         - if the virtual router already exists
*       GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
*       GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
*       GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamAddVirtualRouter
(
    IN GT_U32                                 vrId,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN GT_U32                                 defUcNextHopRuleIdxArr[],
    IN CPSS_DXCH_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN CPSS_DXCH_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN GT_U32                                 defMcRouteRuleIdxArr[],
    IN CPSS_IP_PROTOCOL_STACK_ENT             protocolStack,
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC       *ipTcamShadowPtr
);

/*******************************************************************************
* cpssDxChIpTcamGetVirtualRouter
*
* DESCRIPTION:
*       This function gets the virtual router configuration
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       vrId                    - The virtual's router ID (in cheetah+ this
*                                 will be the pcl id).
*       ipTcamShadowPtr         - the ip shadow we are working on.
*
* OUTPUTS:
*       supportIpv4UcPtr           - Whether the virtual router supports IPv4 UC
*       defIpv4UcNextHopInfoPtr    - The next hop info representing the default Ipv4 Uc
*                                    route entry.
*                                    Relevant only when <supportIpv4UcPtr> == GT_TRUE
*       supportIpv6UcPtr           - Whether the virtual router supports IPv6 UC
*       defIpv6UcNextHopInfoPtr    - The next hop info representing the default Ipv6 Uc
*                                    route entry.
*                                    Relevant only when <supportIpv6UcPtr> == GT_TRUE

*       defUcNextHopRuleIdxArr     - if not NULL, then this is the internal rule idx
*                                    of the defualt NextHop prefix. (array of 2
*                                    for each of the protocol Stacks)
*                                    Relevant only when <supportIpv4UcPtr> == GT_TRUE
*                                    or <supportIpv6UcPtr> == GT_TRUE
*       supportIpv4McPtr           - Whether the virtual router supports Ipv4 MC
*       defIpv4McRouteLttEntryPtr  - The default Ipv4 MC LTT entry
*                                    Relevant only when <supportIpv4McPtr> == GT_TRUE
*       supportIpv6McPtr           - Whether the virtual router supports Ipv6 MC
*       defIpv6McRouteLttEntryPtr  - The default Ipv6 MC LTT entry
*                                    Relevant only when <supportIpv6McPtr> == GT_TRUE
*       defMcRouteRuleIdxArr       - if not null then this is the internal Rule idx
*                                    of the default mc route (prefix) (array of 2
*                                    for each of the protocol Stacks)
*                                    Relevant only when <supportIpv4McPtr> == GT_TRUE
*                                    or <supportIpv6McPtr> == GT_TRUE
*
* RETURNS:
*       GT_OK on success, or
*       GT_NOT_FOUND             - if the given next hop Id is not valid.
*       GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
*       GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamGetVirtualRouter
(
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC        *ipTcamShadowPtr,
    OUT GT_BOOL                                 *supportIpv4UcPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *defIpv4UcNextHopInfoPtr,
    OUT GT_BOOL                                 *supportIpv6UcPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *defIpv6UcNextHopInfoPtr,
    OUT GT_U32                                  defUcNextHopRuleIdxArr[],
    OUT GT_BOOL                                 *supportIpv4McPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC              *defIpv4McRouteLttEntryPtr,
    OUT GT_BOOL                                 *supportIpv6McPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC              *defIpv6McRouteLttEntryPtr,
    OUT GT_U32                                  defMcRouteRuleIdxArr[]
);

/*******************************************************************************
* cpssDxChIpTcamDeleteVirtualRouter
*
* DESCRIPTION:
*       This function deletes an existing virtual router from the vr table, The
*       IP tables (Uc & Mc) are flushed as a result of deletion.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       vrId          - The virtual's router ID to be deleted.
*       ipTcamShadowPtr - the ip shadow we are working on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL if the given vrId does not exist.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamDeleteVirtualRouter
(
    IN GT_U32                              vrId,
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC    *ipTcamShadowPtr
);

/*******************************************************************************
* cpssDxChIpTcamAddDevToShadow
*
* DESCRIPTION:
*       add the device to the ip shadow
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devListPtr  - The device list to add.
*       ipTcamShadowPtr - the ip shadow we are working on.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success,
*       GT_FAIL                  - otherwise.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamAddDevToShadow
(
    IN  PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC *devListPtr,
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC      *ipTcamShadowPtr
);


/*******************************************************************************
* cpssDxChIpTcamRemoveDevsFromShadow
*
* DESCRIPTION:
*       removes the devices from belonging to the shadow.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devListPtr  - The device list to remove
*       ipTcamShadowPtr - the ip shadow we are working on.
*
* OUTPUTS:
*       None.
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamRemoveDevsFromShadow
(
    IN PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC *devListPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC      *ipTcamShadowPtr
);

/*******************************************************************************
* cpssDxChIpTcamGetDevsFromShadow
*
* DESCRIPTION:
*       Get the devices from belonging to the shadow.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       ipTcamShadowPtr   - the ip shadow we are working on.
*       numOfDevsPtr      - points to the size of devListArray
*
* OUTPUTS:
*       numOfDevsPtr      - points to the number of devices retreived
*       devListArray      - array of device ids in the LPM DB
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_BAD_SIZE              - in case not enough memory was allocated to device list
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The application should allocate memory for the device list array.
*       If the function returns GT_OK, then number of devices holds
*       the number of devices the function filled.
*       If the function returns GT_BAD_SIZE, then the memory allocated by the
*       application to the device list is not enough. In this case the
*       number of devices will hold the size of array needed.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamGetDevsFromShadow
(
    IN    PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC           *ipTcamShadowPtr,
    OUT   GT_U8                                      devListArray[],
    INOUT GT_U32                                     *numOfDevsPtr
);

/*******************************************************************************
* cpssDxChIpTcamTrieSetPrefixIpCheetah2
*
* DESCRIPTION:
*       utility function to Add IP UC prefix.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       vrId            - The virtual's router ID.
*       ipAddrPtr       - IP Uc address
*       prefixLen       - Ip Uc prefix length
*       ecmpRouteNum    - the ecmp route number to write
*       isMcSrc         - if this entry is MC source
*       gIndex          - the G-index to write (relevant if isMcSrc == GT_TRUE)
*       tcamRulePtr     - the tcam rule to set it in
*       ipTcamShadowPtr - ip shadow we are working on.
*       pData           - the data to attach to this prefix.
*       protocolStack   - the type of the prefix.
*       devListPtr      - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       1.  In order to add a default IP group, use prefixLen = 0.
*       2.  Bit 15 in the vrId indicates whether this is shared virtual router
*           or not (if the bit is 1 this is shared virtual router).
*           For shared virtual router:
*             - <match all> entries (meaning perfix len is 0) are not written to
*               the hardware
*             - all other prefixes are written with <don't care> masking for
*               the virtual router field.
*******************************************************************************/
GT_STATUS cpssDxChIpTcamTrieSetPrefixIpCheetah2
(
    IN  GT_U32                                    vrId,
    IN  GT_U8                                     *ipAddrPtr,
    IN  GT_U32                                    prefixLen,
    IN  GT_U8                                     ecmpRouteNum,
    IN  GT_BOOL                                   isMcSrc,
    IN  GT_U16                                    gIndex,
    IN  GT_PTR                                    tcamRulePtr,
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC           *ipTcamShadowPtr,
    IN  GT_PTR                                    pData,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                protocolStack,
    IN  PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC     *devListPtr
);

/*******************************************************************************
* cpssDxChIpTcamTrieDelPrefixCheetah2
*
* DESCRIPTION:
*       utility function to Delete IP prefix rule from IP table.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       vrId                - the virtual router id.
*       tcamRulePtr         - the tcam rule to delete it from.
*       ipTcamShadowPtr     - ip shadow we are working on.
*       pData               - the data entry that was attached to this prefix.
*       protocolStack       - the protocol stack of the prefix.
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChIpTcamTrieDelPrefixCheetah2
(
    IN  GT_U32                                        vrId,
    IN  GT_PTR                                        tcamRulePtr,
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC              *ipTcamShadowPtr,
    IN  GT_PTR                                        pData,
    IN  CPSS_IP_PROTOCOL_STACK_ENT                    protocolStack,
    IN  PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC         *devListPtr
);

/*******************************************************************************
* cpssDxChIpTcamTrieDelIpv4PrefixCheetah2
*
* DESCRIPTION:
*       utility function to Delete IP prefix rule from IP table.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       vrId                - the virtual router id.
*       tcamRulePtr         - the tcam rule to delete it from.
*       ipTcamShadowPtr     - ip shadow we are working on.
*       pData               - the data entry that was attached to this prefix.
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChIpTcamTrieDelIpv4PrefixCheetah2
(
    IN  GT_U32                                        vrId,
    IN  GT_PTR                                        tcamRulePtr,
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC              *ipTcamShadowPtr,
    IN  GT_PTR                                        pData,
    IN  PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC         *devListPtr
);


/*******************************************************************************
* cpssDxChIpTcamTrieDelIpv6PrefixCheetah2
*
* DESCRIPTION:
*       utility function to Delete IP prefix rule from IP table.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       vrId                - the virtual router id.
*       tcamRulePtr         - the tcam rule to delete it from.
*       ipTcamShadowPtr     - ip shadow we are working on.
*       pData               - the data entry that was attached to this prefix.
*       devListPtr          - pointer to the device list to preform the actions on.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChIpTcamTrieDelIpv6PrefixCheetah2
(
    IN  GT_U32                                        vrId,
    IN  GT_PTR                                        tcamRulePtr,
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC              *ipTcamShadowPtr,
    IN  GT_PTR                                        pData,
    IN  PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC         *devListPtr
);


/*******************************************************************************
* cpssDxChIpTcamUpdateHW
*
* DESCRIPTION:
*       Perform the actual HW update at the end of a hot sync operation
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       ipTcamShadowPtr - the shadow to write the HW.
*       updateMode      - the update mode.
*       protocolStack   - which protocol stack to update.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamUpdateHW
(
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC *ipTcamShadowPtr,
    IN PRV_DXCH_IP_UPDATE_TYPE_ENT       updateMode,
    IN CPSS_IP_PROTOCOL_STACK_ENT        protocolStack
);


/*******************************************************************************
* prvCpssDxChIpLpmPbrDefaultMcRuleSet
*
* DESCRIPTION:
*       Sets "do-nothing" PCL rule to capture all MC traffic for specific
*       protocol stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       ipTcamShadowPtr   - the ip shadow we are working on
*       devListPtr        - list of devices to set the rule at
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success.
*       GT_BAD_PARAM    - Illegal parameter in function called
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpLpmPbrDefaultMcRuleSet
(
    IN  PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC        *ipTcamShadowPtr,
    IN  PRV_CPSS_DXCH_IP_SHADOW_DEVS_LIST_STC   *devListPtr
);

/*******************************************************************************
* cpssDxChIpTcamLpmDbGetL3
*
* DESCRIPTION:
*       Retrieves a specific shadow's ip Table memory Size needed and info
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       lpmDBId            - The LPM DB id.
*       iterationSizePtr   - (pointer to) data size in bytes supposed to be processed
*                            in current iteration.
*       iterPtr            - the iterator, to start - set to 0.
*
* OUTPUTS:
*       tableSizePtr  - (pointer to) the table size calculated (in bytes)
*       tablePtr      - (pointer to) the table size info block
*       iterationSizePtr   - (pointer to) data size in bytes left after iteration .
*       iterPtr       - the iterator, if = 0 then the operation is done.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       In case the LPM DB uses TCAM manager and creates the TCAM manager
*       internally, then the LPM DB is also responsible for exporting and
*       importing the TCAM manager data. The TCAM manager must be imported
*       before VR, UC and MC entries.
*       If the LPM uses TCAM manager (internally or externally) then the LPM DB
*       must update the TCAM manager client callback functions.
*
*       Data is arranged in entries of different types in the following order:
*        - TCAM manager entries, if needed. Each TCAM manager entry is 1k
*          of the TCAM manager HSU data (last entry is up to 1k).
*        - VR entry per virtual router in the LPM DB. If the LPM DB contains
*          no VR, then this section is empty.
*        - Dummy VR entry. Note that the dummy VR entry will always exists,
*          even if there are no VR in the LPM DB
*        - If the LPM DB supports IPv4 protocol then for each virtual router in
*          the LPM DB the following section will appear. The order of the
*          virtual routers is according to the virtual router ID. In case
*          there are no virtual routers, this section will be empty.
*            - UC entry for each IPv4 UC entry other than the default.
*            - Dummy UC entry.
*            - MC entry for each IPv4 MC entry other than the default.
*            - Dummy MC entry
*        - If the LPM DB supports IPv6 protocol then for each virtual router in
*          the LPM DB the following section will appear. The order of the
*          virtual routers is according to the virtual router ID. In case
*          there are no virtual routers, this section will be empty.
*            - UC entry for each IPv6 UC entry other than the default.
*            - Dummy UC entry.
*            - MC entry for each IPv6 MC entry other than the default.
*            - Dummy MC entry.
*       Note that if the LPM DB doesn't support a protocol stack then the
*       sections related to this protocol stack will be empty (will not include
*       the dummy entries as well).
*       If a VR doesn't support one of the prefix types then the section for
*       this prefix type will be empty but will include dummy.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamLpmDbGetL3
(
    IN     GT_U32                       lpmDBId,
    OUT    GT_U32                       *tableSizePtr,
    OUT    GT_VOID                      *tablePtr,
    INOUT  GT_U32                       *iterationSizePtr,
    INOUT  GT_UINTPTR                   *iterPtr
);

/*******************************************************************************
* cpssDxChIpTcamLpmDbSetL3
*
* DESCRIPTION:
*       Set the data needed for core IP shadow reconstruction used after HSU.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       lpmDBId             - The LPM DB id.
*       tablePtr            - (pointer to)the table size info block.
*       iterationSizePtr    - (pointer to) data size in bytes supposed to be processed.
*                             in current iteration.
*       iterPtr             - the iterator, to start - set to 0.
*
* OUTPUTS:
*       iterationSizePtr    - (pointer to) data size in bytes left after iteration .
*       iterPtr - the iterator, if = 0 then the operation is done.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*       GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* COMMENTS:
*       Refer to cpssDxChIpTcamLpmDbGetL3.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamLpmDbSetL3
(
    IN     GT_U32                       lpmDBId,
    IN     GT_VOID                      *tablePtr,
    INOUT  GT_U32                       *iterationSizePtr,
    INOUT  GT_UINTPTR                   *iterPtr
);

/*******************************************************************************
* prvCpssPpConfigDevDbRelease
*
* DESCRIPTION:
*       private (internal) function to release the DB of LPM.
*       NOTE: function not 'free' the allocated memory. only detach from it ,
*             and restore DB to 'pre-init' state
*
*             The assumption is that the 'cpssOsMalloc' allocations will be not
*             valid any more by the application , so no need to 'free' each and
*             every allocation !
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpLpmDbRelease
(
    void
);


/*******************************************************************************
* prvCpssDxChGetIpTcamOffset
*
* DESCRIPTION:
*       Get  IP TCAM hardware offset
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       ipTcamShadowPtr     - ip shadow we are working on.
*       tcamRulePtr         - pointer to tcam rule
*       isToken             - whether the tcamRulePtr represent TCAM location
*                             or token (relevant only in TCAM manager mode)
*
* OUTPUTS:
*       offsetPtr           - pointer to TCAM rule hardware offset.
*
* RETURNS:
*      GT_OK        - on OK
*      GT_FAIL      - on FAIL
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChGetIpTcamOffset
(
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC           *ipTcamShadowPtr,
    IN  GT_PTR                                    tcamRulePtr,
    IN  GT_BOOL                                   isToken,
    OUT GT_U32                                    *offsetPtr
);

/*******************************************************************************
* prvCpssDxChGetIpTcamRowColumn
*
* DESCRIPTION:
*       Get  IP TCAM hardware location :row and column
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       ipTcamShadowPtr     - ip shadow we are working on.
*       tcamRulePtr         - pointer to tcam rule
*
* OUTPUTS:
*       tcamRowPtr          - pointer to TCAM row.
*       tcamColumnPtr       - pointer to TCAM column.
*
* RETURNS:
*      GT_OK        - on OK
*      GT_FAIL      - on fail
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDxChGetIpTcamRowColumn
(
    IN PRV_CPSS_DXCH_IP_TCAM_SHADOW_STC           *ipTcamShadowPtr,
    IN  GT_PTR                                    tcamRulePtr,
    OUT GT_U32                                    *tcamRowPtr,
    OUT GT_U32                                    *tcamColumnPtr
);

/*******************************************************************************
* prvCpssDxChIpLpmDbIdGetNext
*
* DESCRIPTION:
*       This function retrieve next LPM DB ID from LPM DBs Skip List
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       slIteratorPtr - The iterator Id that was returned from the last call to
*                       this function.
* OUTPUTS:
*       lpmDbIdPtr    - retrieved LPM DB ID
*       slIteratorPtr - The iteration Id to be sent in the next call to this
*                       function to get the next data.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PTR               - on NULL pointer
*       GT_NO_MORE               - on absence of elements in skip list
*
* COMMENTS:
*       none.
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpLpmDbIdGetNext
(
    OUT     GT_U32      *lpmDbIdPtr,
    INOUT   GT_UINTPTR  *slIteratorPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPrvIpLpmh */


/*
 *------------------------------------------------------------------
 * $Log: cpssDxChPrvIpLpm.h,v $
 * Revision 1.1  2015/02/13 11:32:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
