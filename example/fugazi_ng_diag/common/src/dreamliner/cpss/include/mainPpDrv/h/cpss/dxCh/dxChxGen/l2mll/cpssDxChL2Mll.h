/* $Id: cpssDxChL2Mll.h,v 1.1 2015/02/13 11:32:07 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h,v $
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
* cpssDxChL2Mll.h
*
* DESCRIPTION:
*       The CPSS DXCH L2 Mll definitions
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/
#ifndef __cpssDxChL2Mllh
#define __cpssDxChL2Mllh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/*
 * typedef: enum CPSS_DXCH_MLL_TABLE_PARTITIONING_ENT
 *
 * Description: MLL memory partitioning mode between L2 MLL & L3 MLL.
 *
 * Enumerations:
 *
 *      CPSS_DXCH_MLL_TABLE_PARTITIONING_L3MLL_E - the entire memory is used by 
 *                                                 the L3 MLL table.
 *      CPSS_DXCH_MLL_TABLE_PARTITIONING_L2MLL_E - the entire memory is used by 
 *                                                 the L2 MLL table.
 *      CPSS_DXCH_MLL_TABLE_PARTITIONING_SPLIT_E - the memory is split: half is 
 *                                                 used by L3 MLL, and half by 
 *                                                 the L2 MLL. 
 */
typedef enum
{
    CPSS_DXCH_MLL_TABLE_PARTITIONING_L3MLL_E,
    CPSS_DXCH_MLL_TABLE_PARTITIONING_L2MLL_E,
    CPSS_DXCH_MLL_TABLE_PARTITIONING_SPLIT_E
}CPSS_DXCH_MLL_TABLE_PARTITIONING_ENT;

/*
 * typedef: enum CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_ENT
 *
 * Description: the L2 MLL TC queue scheduling mode
 *
 * Enumerations:
 *  CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_SDWRR_E - L2 MLL TC queue will be
 *                                                     part of the SDWRR 
 *                                                     scheduling.
 *  CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_SP_E - L2 MLL TC queue will be 
 *                                                  part of the SP (strict 
 *                                                  priority) scheduling.
 */
typedef enum
{
    CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_SDWRR_E,
    CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_SP_E
}CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_ENT
 *
 * Description: Multi target/unicast Scheduler MTUs
 *
 * Enumerations:
 *  CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_2K_E   - 2k bytes MTU
 *  CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_8K_E   - 8k bytes MTU
 */
typedef enum
{
    CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_2K_E,
    CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_8K_E
}CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_ENT;

/*
 * Typedef: struct CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC
 *
 * Description: L2 MLL Exception Counters
 *
 * Fields:
 *      skip    - number of L2 MLL entries skipped.
 *      ttl     - number of TTL exceptions in the L2 MLL.
 */
typedef struct
{
    GT_U32 skip;
    GT_U32 ttl;
}CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC;

/*
 * Typedef: struct CPSS_DXCH_L2_MLL_LTT_ENTRY_STC
 *
 * Description: L2 MLL Lookup Translation Table (LTT) Entry
 *
 * Fields:
 *      mllPointer           - pointer to the MLL entry. 
 *                             valid range see in datasheet of specific device.
 *      mllMaskProfileEnable - GT_TRUE: enable MLL entry according to match 
 *                                      with its profile mask.
 *                             GT_FALSE: MLL entry is used while ignoring
 *                                       the profile mask.
 *      mllMaskProfile       - The mask profile. (APPLICABLE RANGES: 0..14)
 *                             Relevant for mllMaskProfileEnable == GT_TRUE
 */
typedef struct
{
   GT_U32   mllPointer;
   GT_BOOL  mllMaskProfileEnable;
   GT_U32   mllMaskProfile;
} CPSS_DXCH_L2_MLL_LTT_ENTRY_STC;

/*
 * Typedef: struct CPSS_DXCH_L2_MLL_ENTRY_STC
 *
 * Description: L2 MLL Entry
 *
 * Fields:
 *      nextMllPointer          - pointer to the next MLL entry.
 *                                valid range see in datasheet of specific 
 *                                device.
 *      last                    - GT_TRUE: the last MLL entry in this list.
 *                                GT_FALSE: there are more MLL entries in this 
 *                                          list.
 *      unknownUcFilterEnable   - GT_TRUE: unknown UC traffic is not replicated 
 *                                         by this MLL entry.
 *                                GT_FALSE: unknown UC traffic is replicated by
 *                                          this MLL entry.
 *      unregMcFilterEnable     - GT_TRUE: unregistered MC traffic is not 
 *                                         replicated by this MLL entry.
 *                                GT_FALSE: unregistered MC traffic is  
 *                                          replicated by this MLL entry.
 *      bcFilterEnable          - GT_TRUE: Broadcast traffic is not replicated 
 *                                         by this MLL entry.
 *                                GT_FALSE: Broadcast traffic is replicated by
 *                                          this MLL entry.
 *      peFilterEnable          - GT_TRUE: PE (Provider Edge) traffic is not 
 *                                         replicated by this MLL entry.
 *                                GT_FALSE: PE traffic is replicated by this 
 *                                          MLL entry.
 *      mcLocalSwitchingEnable - GT_TRUE: allow replication to the source 
 *                                        ePort of multicast traffic.
 *                               GT_FALSE: avoid replication to the source 
 *                                         ePort of multicast traffic.
 *      maxHopCountEnable      - GT_TRUE: hop count of a replicated packet is 
 *                                        no more than maxOutgoingHopCount.
 *                               GT_FALSE: no hop count limit.
 *                                         (relevant for TRILL).
 *      maxOutgoingHopCount    - The hop count of a replicated packet is no 
 *                               more than this value.
 *                               Relevant only if maxHopCountEn set to GT_TRUE.
 *                               (APPLICABLE RANGES: 0..63)
 *      egressInterface        - target Egress Interface. Valid types are:
 *                               CPSS_INTERFACE_PORT_E
 *                               CPSS_INTERFACE_TRUNK_E
 *                               CPSS_INTERFACE_VIDX_E
 *                               CPSS_INTERFACE_VID_E 
 *      maskBitmap             - A per-profile mask indicator. In each MLL entry
 *                               the relevant bit in this bitmap indicates 
 *                               whether the current MLL entry is masked or not. 
 *                               In each bit, a value of 0 indicates that the 
 *                               entry is skipped (masked), while a value of 1 
 *                               indicates that the current entry is active.
 *                               This bitmap is indexed by the L2 MLL LTT entry
 *                               field <mllMaskProfile>.
 *      ttlThreshold           - if the packet's TTl is less than this field,
 *                               the packet is not replicated. (APPLICABLE RANGES: 0..255)
 *      bindToMllCounterEnable - GT_TRUE: bind entry to MLL counter indexed by 
 *                                        <mllCounterIndex> field.
 *                               GT_FALSE: entry is not binded to one of the 
 *                                         MLL counters.
 *      mllCounterIndex        - The MLL counter to bind the entry to. 
 *                               (APPLICABLE RANGES: 0..2) .Relevant only if
 *                               <bindToMllCounterEnable> == GT_TRUE.
 */
typedef struct
{
   GT_U32   nextMllPointer;
   GT_BOOL  last;
   GT_BOOL  unknownUcFilterEnable;
   GT_BOOL  unregMcFilterEnable;
   GT_BOOL  bcFilterEnable;
   GT_BOOL  peFilterEnable;
   GT_BOOL  mcLocalSwitchingEnable;
   GT_BOOL  maxHopCountEnable;
   GT_U32   maxOutgoingHopCount;
   CPSS_INTERFACE_INFO_STC egressInterface;
   GT_U32   maskBitmap;
   GT_U32   ttlThreshold;
   GT_BOOL  bindToMllCounterEnable;
   GT_U32   mllCounterIndex;
} CPSS_DXCH_L2_MLL_ENTRY_STC;

/*******************************************************************************
* cpssDxChL2MllTablePartitioningModeSet
*
* DESCRIPTION:
*       Set the memory partitioning between L2 & L3 MLLs.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*       mode    - the partitioning mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device or mode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllTablePartitioningModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_MLL_TABLE_PARTITIONING_ENT mode
);


/*******************************************************************************
* cpssDxChL2MllTablePartitioningModeGet
*
* DESCRIPTION:
*       Get the memory partitioning between L2 & L3 MLLs.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       modePtr - (pointer to) the partitioning mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device
*       GT_BAD_STATE             - on bad reported mode
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllTablePartitioningModeGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_MLL_TABLE_PARTITIONING_ENT *modePtr
);

/*******************************************************************************
* cpssDxChL2MllLookupForAllEvidxEnableSet
*
* DESCRIPTION:
*       Enable or disable MLL lookup for all multi-target packets.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*       enable  - GT_TRUE: MLL lookup is performed for all multi-target packets.
*                 GT_FALSE: MLL lookup is performed only for multi-target 
*                           packets with eVIDX >= 4K. For packets with 
*                           eVidx < 4K L2 MLL is not accessed.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/*******************************************************************************
* cpssDxChL2MllLookupForAllEvidxEnableGet
*
* DESCRIPTION:
*       Get enabling status of MLL lookup for all multi-target packets.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       enablePtr  - (pointer to)
*                    GT_TRUE: MLL lookup is performed for all multi-target 
*                             packets.
*                    GT_FALSE: MLL lookup is performed only for multi-target 
*                              packets with eVIDX >= 4K. For packets with 
*                              eVidx < 4K L2 MLL is not accessed.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
);

/*******************************************************************************
* cpssDxChL2MllTtlExceptionConfigurationSet
*
* DESCRIPTION:
*       Set configuration for L2 MLL TTL Exceptions.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*       trapEnable  - GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                              if packet's TTL is less than MLL entry field
*                              <TTL Threshold>
*                     GT_FALSE: no packet trap to CPU due to a TTL exception
*                               in the MLL.
*       cpuCode     - CPU code of packets that are trapped to CPU due to a
*                     TTL exception in the MLL.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  trapEnable,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
);

/*******************************************************************************
* cpssDxChL2MllTtlExceptionConfigurationGet
*
* DESCRIPTION:
*       Get configuration of L2 MLL TTL Exceptions.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*
* OUTPUTS:
*       trapEnablePtr   - (pointer to)
*                         GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                  if packet's TTL is less than MLL entry field
*                                  <TTL Threshold>
*                         GT_FALSE: no packet trap to CPU due to a TTL 
*                                   exception in the MLL.
*       cpuCodePtr      - (pointer to) CPU code of packets that are trapped to
*                         CPU due to a TTL exception in the MLL.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *trapEnablePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
);

/*******************************************************************************
* cpssDxChL2MllExceptionCountersGet
*
* DESCRIPTION:
*       Get L2 MLL exception counters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*
* OUTPUTS:
*       countersPtr   - (pointer to) L2 MLL exception counters. 
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
);

/*******************************************************************************
* cpssDxChL2MllPortGroupExceptionCountersGet
*
* DESCRIPTION:
*       Get L2 MLL exception counters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       countersPtr   - (pointer to) L2 MLL exception counters. 
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllPortGroupExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
);

/*******************************************************************************
* cpssDxChL2MllCounterGet
*
* DESCRIPTION:
*       Get L2 MLL counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - device number
*       index   - counter number (APPLICABLE RANGES: 0..2)
*
* OUTPUTS:
*       counterPtr   - (pointer to) L2 MLL counter value. 
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  index,
    OUT GT_U32  *counterPtr
);

/*******************************************************************************
* cpssDxChL2MllPortGroupCounterGet
*
* DESCRIPTION:
*       Get L2 MLL counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       index          - counter number (APPLICABLE RANGES: 0..2)
*
* OUTPUTS:
*       counterPtr   - (pointer to) L2 MLL counter value. 
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllPortGroupCounterGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              index,
    OUT GT_U32              *counterPtr
);

/*******************************************************************************
* cpssDxChL2MllLttEntrySet
*
* DESCRIPTION:
*       Set L2 MLL Lookup Translation Table (LTT) entry.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*       index       - the LTT index. eVidx range.
*       lttEntryPtr - (pointer to) L2 MLL LTT entry.
*      
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllLttEntrySet
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
);

/*******************************************************************************
* cpssDxChL2MllLttEntryGet
*
* DESCRIPTION:
*       Get L2 MLL Lookup Translation Table (LTT) entry.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*       index       - the LTT index. eVidx range.
*
* OUTPUTS:
*       lttEntryPtr - (pointer to) L2 MLL LTT entry.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllLttEntryGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           index,
    OUT CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
);

/*******************************************************************************
* cpssDxChL2MllEntrySet
*
* DESCRIPTION:
*       Set L2 MLL entry.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*       index       - the MLL index, a.k.a. MLL pointer.
*                     valid range see in datasheet of specific device.
*       mllEntryPtr - (pointer to) L2 MLL entry.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN CPSS_DXCH_L2_MLL_ENTRY_STC    *mllEntryPtr
);

/*******************************************************************************
* cpssDxChL2MllEntryGet
*
* DESCRIPTION:
*       Get L2 MLL entry.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number
*       index       - the MLL index, a.k.a. MLL pointer.
*                     valid range see in datasheet of specific device.
*
* OUTPUTS:
*       mllEntryPtr - (pointer to) L2 MLL entry.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - wrong parameters
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllEntryGet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    OUT CPSS_DXCH_L2_MLL_ENTRY_STC    *mllEntryPtr
);

/*******************************************************************************
* cpssDxChL2MllCtrlTrafficMultiTargetTcQueueSet
*
* DESCRIPTION:
*      Sets the multi-target TC queue assigned to multi-target Control
*      traffic.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum              - the device number
*       multiTargetTcQueue  - the multi-target TC queue for multi-target
*                             Control traffic. (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_OUT_OF_RANGE          - multiTargetTcQueue is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Multi-target control traffic is defined as multi-target packets with
*       DSA tag FROM_CPU or multi-target packets that are marked to be mirrored
*       to the CPU.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllCtrlTrafficMultiTargetTcQueueSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  multiTargetTcQueue
);

/*******************************************************************************
* cpssDxChL2MllCtrlTrafficMultiTargetTcQueueGet
*
* DESCRIPTION:
*      Gets the multi-target TC queue assigned to multi-target Control
*      traffic.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum                - the device number
*
* OUTPUTS:
*       multiTargetTcQueuePtr - (pointer to)the multi-target TC queue for
*                               multi-target Control traffic
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Multi-target control traffic is defined as multi-target packets with
*       DSA tag FROM_CPU or multi-target packets that are marked to be mirrored
*       to the CPU.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllCtrlTrafficMultiTargetTcQueueGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *multiTargetTcQueuePtr
);

/*******************************************************************************
* cpssDxChL2MllQosProfileToMultiTargetTcQueueMapSet
*
* DESCRIPTION:
*     Sets the Qos Profile to multi-target TC queue mapping.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum              - the device number
*       qosProfile          - QOS Profile index (APPLICABLE RANGES: 0..127)
*       multiTargetTcQueue  - multi-target TC queue (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_OUT_OF_RANGE          - multiTargetTcQueue is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllQosProfileToMultiTargetTcQueueMapSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    IN  GT_U32  multiTargetTcQueue
);

/*******************************************************************************
* cpssDxChL2MllQosProfileToMultiTargetTcQueueMapGet
*
* DESCRIPTION:
*     Gets the Qos Profile to multi-target TC queue mapping.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum                  - the device number
*       qosProfile              - QOS Profile index. (APPLICABLE RANGES: 0..127)
*
* OUTPUTS:
*       multiTargetTcQueuePtr   - (pointer to) multi-target TC queue. 
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllQosProfileToMultiTargetTcQueueMapGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    OUT GT_U32  *multiTargetTcQueuePtr
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetQueueFullDropCounterSet
*
* DESCRIPTION:
*      Sets the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - the device number
*       counter - value to set counter.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetQueueFullDropCounterSet
(
    IN GT_U8    devNum,
    IN GT_U32   counter
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetQueueFullDropCounterGet
*
* DESCRIPTION:
*      Gets the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - the device number
*
* OUTPUTS:
*       counterPtr - (pointer to) counter value.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetQueueFullDropCounterGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *counterPtr
);

/*******************************************************************************
* cpssDxChL2MllPortGroupMultiTargetQueueFullDropCounterSet
*
* DESCRIPTION:
*      Sets the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       counter        - counter value to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllPortGroupMultiTargetQueueFullDropCounterSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               counter
);

/*******************************************************************************
* cpssDxChL2MllPortGroupMultiTargetQueueFullDropCounterGet
*
* DESCRIPTION:
*      Gets the multi target queue full drop packet counter.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       counterPtr     - (pointer to) counter value.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllPortGroupMultiTargetQueueFullDropCounterGet
(
    IN  GT_U8               devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    OUT GT_U32              *counterPtr
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetTcQueueSchedulerModeSet
*
* DESCRIPTION:
*      Sets the multi-target TC queue scheduling mode and weight.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum              - the device number
*       multiTargetTcQueue  - multi-target TC queue. (APPLICABLE RANGES: 0..3)
*       schedulingMode      - the scheduling mode.
*       queueWeight         - the queue weight for SDWRR scheduler (APPLICABLE RANGES: 0..255)
*                             (relevant only if schedulingMode =
*                             CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_SDWRR_E).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_OUT_OF_RANGE          - queueWeight is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetTcQueueSchedulerModeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          multiTargetTcQueue,
    IN  CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_ENT    schedulingMode,
    IN  GT_U32                                          queueWeight
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetTcQueueSchedulerModeGet
*
* DESCRIPTION:
*      Gets the multi-target TC queue scheduling mode and weight.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum              - the device number
*       multiTargetTcQueue  - multi-target TC queue. (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       schedulingModePtr   - (pointer to) the scheduling mode.
*       queueWeightPtr      - (pointer to) the queue weight for SDWRR scheduler
*                             (relevant only if schedulingMode =
*                             CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_SDWRR_E).
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetTcQueueSchedulerModeGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          multiTargetTcQueue,
    OUT CPSS_DXCH_L2_MLL_TC_QUEUE_SCHEDULER_MODE_ENT    *schedulingModePtr,
    OUT GT_U32                                          *queueWeightPtr
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetRateShaperSet
*
* DESCRIPTION:
*      Sets L2 MLL Rate shaper parameters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum     - the device number
*       enable  - GT_TRUE: L2 MLL rate shaper enabled.
*                 GT_FALSE: L2 MLL rate shaper disabled.
*       windowSize - Shaper window size (APPLICABLE RANGES: 0..0xFFFF).
*                    Relevant only if rate shaper is enabled.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_OUT_OF_RANGE          - windowSize is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The rate shaper window size is the IDLE window, in Core clock cycles, 
*       between two subsequent multi-target packets.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetRateShaperSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable,
    IN  GT_U32  windowSize
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetRateShaperGet
*
* DESCRIPTION:
*      Gets L2 MLL Rate shaper parameters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum     - the device number
*
* OUTPUTS:
*       enablePtr  - (pointer to) GT_TRUE: L2 MLL rate shaper enabled.
*                                 GT_FALSE: L2 MLL rate shaper disabled.
*       windowSizePtr - (pointer to) Shaper window size.
*                       Relevant only if rate shaper is enabled.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The rate shaper window size is the IDLE window, in Core clock cycles, 
*       between two subsequent multi-target packets.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetRateShaperGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *windowSizePtr
);

/*******************************************************************************
* cpssDxChL2MllPortGroupMultiTargetRateShaperSet
*
* DESCRIPTION:
*      Sets L2 MLL Rate shaper parameters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       enable         - GT_TRUE: L2 MLL rate shaper enabled.
*                        GT_FALSE: L2 MLL rate shaper disabled.
*       windowSize     - Shaper window size (APPLICABLE RANGES: 0..0xFFFF).
*                        Relevant only if rate shaper is enabled.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_OUT_OF_RANGE          - windowSize is out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The rate shaper window size is the IDLE window, in Core clock cycles, 
*       between two subsequent multi-target packets.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllPortGroupMultiTargetRateShaperSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_BOOL             enable,
    IN  GT_U32              windowSize
);

/*******************************************************************************
* cpssDxChL2MllPortGroupMultiTargetRateShaperGet
*
* DESCRIPTION:
*      Gets L2 MLL Rate shaper parameters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum         - the device number
*       portGroupsBmp  - bitmap of Port Groups.
*              NOTEs:
*               1. for non multi-port groups device this parameter is IGNORED.
*               2. for multi-port groups device :
*                  bitmap must be set with at least one bit representing
*                  valid port group(s). If a bit of non valid port group
*                  is set then function returns GT_BAD_PARAM.
*                  value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* OUTPUTS:
*       enablePtr  - (pointer to) GT_TRUE: L2 MLL rate shaper enabled.
*                                 GT_FALSE: L2 MLL rate shaper disabled.
*       windowSizePtr - (pointer to) Shaper window size.
*                       Relevant only if rate shaper is enabled.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The rate shaper window size is the IDLE window, in Core clock cycles, 
*       between two subsequent multi-target packets.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllPortGroupMultiTargetRateShaperGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_BOOL             *enablePtr,
    OUT GT_U32              *windowSizePtr
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetUcSchedulerModeSet
*
* DESCRIPTION:
*      Sets the Multi target/unicast scheduler mode and parameters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum       - the device number
*       ucSpEnable   - GT_TRUE: UC receives strict priority over MC.
*                      GT_FALSE: SDWRR scheduling.
*       ucWeight     - UC weight in <schedulerMtu> units. (APPLICABLE RANGES: 0..255)
*                      Relevant only if SDWRR scheduling is enabled.
*       mcWeight     - MC weight in <schedulerMtu> units. (APPLICABLE RANGES: 0..255)
*                      Relevant only if SDWRR scheduling is enabled.
*       schedulerMtu - The MTU used by the scheduler.
*                      Relevant only if SDWRR scheduling is enabled.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_OUT_OF_RANGE          - if ucWeight or mcWeight are out of range
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetUcSchedulerModeSet
(
    IN GT_U8                                    devNum,
    IN GT_BOOL                                  ucSpEnable,
    IN GT_U32                                   ucWeight,
    IN GT_U32                                   mcWeight,
    IN CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_ENT schedulerMtu
);

/*******************************************************************************
* cpssDxChL2MllMultiTargetUcSchedulerModeGet
*
* DESCRIPTION:
*      Gets the Multi target/unicast scheduler mode and parameters.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number
*
* OUTPUTS:
*       ucSpEnablePtr   - (pointer to)
*                         GT_TRUE: UC receives strict priority over MC.
*                         GT_FALSE: SDWRR scheduling.
*       ucWeightPtr     - (pointer to)
*                         UC weight.
*                         Relevant only if SDWRR scheduling is enabled.
*       mcWeightPtr     - (pointer to)
*                         MC weight.
*                         Relevant only if SDWRR scheduling is enabled.
*       schedulerMtuPtr - (pointer to) The MTU used by the scheduler
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong input parameters
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChL2MllMultiTargetUcSchedulerModeGet
(
    IN  GT_U8                                       devNum,
    OUT GT_BOOL                                     *ucSpEnablePtr,
    OUT GT_U32                                      *ucWeightPtr,
    OUT GT_U32                                      *mcWeightPtr,
    OUT CPSS_DXCH_L2_MLL_MC_UC_SCHEDULER_MTU_ENT    *schedulerMtuPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChL2Mllh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChL2Mll.h,v $
 * Revision 1.1  2015/02/13 11:32:07  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
