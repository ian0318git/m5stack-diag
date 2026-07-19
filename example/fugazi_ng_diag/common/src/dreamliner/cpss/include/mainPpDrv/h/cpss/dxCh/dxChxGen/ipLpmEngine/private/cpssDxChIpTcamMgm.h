/* $Id: cpssDxChIpTcamMgm.h,v 1.1 2015/02/13 11:32:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpTcamMgm.h,v $
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
* cpssDxChIpTcamMgm.h
*
* DESCRIPTION:
*       This file includes Rules alloction manegment for TCAM IP routing
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __gtCoreIpTcamMgmh
#define __gtCoreIpTcamMgmh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>

struct PRV_CPSS_DXCH_IP_PAT_TRIE_NODE_STCT;
/*
 * typedef: struct PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC
 *
 * Description:
 *      IP Tcam Routing Rule Node.
 *
 * Fields:
 *      valid - wheather this rule is a valid one.
 *      offset - the rule offset in the IP TCAM.
 *      trieNode - the trie node that points to this node (the prefix)
 *
 *      redirectOffset - for policy-based routing, free redirect index in routing
 *                       lookup table.
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STCT
{
    GT_BOOL                                     valid;
    GT_U32                                      offset;
    struct PRV_CPSS_DXCH_IP_PAT_TRIE_NODE_STCT  *trieNode;

    GT_U32                                      redirectOffset;
} PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC;



/*
 * typedef: struct PRV_CPSS_DXCH_IP_TCAM_TOKEN_NODE_STCT
 *
 * Description:
 *      IP Tcam Routing Rule Node.
 *
 * Fields:
 *      tcamManagerEntryToken - token repersenting allocated entry.
 *      trieNode              - the trie node that points to this node (the prefix)
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_IP_TCAM_TOKEN_NODE_STCT
{
    GT_UINTPTR                                     tcamManagerEntryToken;
    struct PRV_CPSS_DXCH_IP_PAT_TRIE_NODE_STCT     *trieNode;
} PRV_CPSS_DXCH_IP_TCAM_TOKEN_NODE_STC;
/*
 * typedef: enum PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_METHOD_ENT
 *
 * Description: IP Tcam Managment allocation method.
 *
 * Enumerations:
 *  PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_MIN_E - allocate minimal index.
 *  PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_MID_E - allocate near middle index.
 *  PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_MAX_E - allocate maximal index.
 *
 * Comments:
 *
 */
typedef enum
{
    PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_MIN_E,
    PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_MID_E,
    PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_MAX_E
}PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_METHOD_ENT;

/*******************************************************************************
* PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_INDEX_CONVERT_FUN
*
* DESCRIPTION:
*       converts the pool index to the interconnected pool index
*
* INPUTS:
*       originalIndex    - the original pool's rule index.
* OUTPUTS:
*       None.
*
* RETURNS:
*       the interconnected pool's rule index
*
* COMMENTS:
*      None.
*
*******************************************************************************/
typedef GT_U32 (*PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_INDEX_CONVERT_FUN)
(
    IN GT_U32 originalIndex
);

/*
 * typedef: struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC
 *
 * Description:
 *      IP Tcam Routing Pool Manager header.
 *
 * Fields:
 *      *coreIpTcamRulesAllocArray  - the Ip Tcam Rules array.
 *      coreIpNumOfIpTcamRules      - the Ip Tcam Rules array size.
 *      *coreIpTcamRulesAllocBitMap - the Ip Tcam Rules Bit map array.
 *      coreIpTcamBitMapSize        - the ip pcl rules bit map array size.
 *      coreIpTcamNumOfAlloc        - the number of allocated rules in the pool.
 *      interConnectedPoolPtr       - another interconnected pool pointer that
 *                                    shared the rules with this pool in some
 *                                    way, the way to convert the indexes is
 *                                    using the interConnectedPoolIndexConFun
 *                                    if = NULL then there is no inter connected
 *                                    pool.
 *      interConnectedPoolIndexConFun - the inter connected pool index convert
 *                                      function.
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STCT
{
    PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC              *coreIpTcamRulesAllocArray;
    GT_U32                                           coreIpNumOfIpTcamRules;
    GT_U32                                           *coreIpTcamRulesAllocBitMap;
    GT_U32                                           coreIpTcamBitMapSize;
    GT_U32                                           coreIpTcamNumOfAlloc;
    struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STCT       **interConnectedPoolPtr;
    PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_INDEX_CONVERT_FUN interConnectedPoolIndexConFun;
}PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC;

#define IP_TCAM_MGM_POOL_NUM_OF_PROTOCOLS_CNS    2

#define IP_TCAM_MGM_POOL_ROUTER_TCAM_INDEX_CNS   0
#define IP_TCAM_MGM_POOL_PCL_TCAM_INDEX_CNS      1
#define IP_TCAM_MGM_POOL_NUM_OF_TCAMS_CNS        2

/*
 * typedef: struct PRV_CPSS_DXCH_IP_TCAM_MGM_MULTIPLE_POOLS_STCT
 *
 * Description:
 *      Ip Tcam Pool Manager header for several tcams, per stack protocol and
 *      per tcam (pcl, router)
 *
 * Fields:
 *      ipTcamPool - IP Tcam Pool
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_IP_TCAM_MGM_MULTIPLE_POOLS_STCT
{
    PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC *tcamMgm[IP_TCAM_MGM_POOL_NUM_OF_TCAMS_CNS][IP_TCAM_MGM_POOL_NUM_OF_PROTOCOLS_CNS];
}PRV_CPSS_DXCH_IP_TCAM_MGM_MULTIPLE_POOLS_STC;


/*
 * typedef: struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC_IDX_SEG_STC
 *
 * Description:
 *      IP Tcam Routing Pool Manager Index Segment.
 *
 * Fields:
 *      startIndex      - the segment's first index.
 *      endIndex        - the segment's last index.
 *      numOfIndexes    - number of indexes in range, since range can be empty
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC_IDX_SEG_STCT
{
    GT_U32                  startIndex;
    GT_U32                  endIndex;
    GT_U32                  numOfIndexes;
} PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC_IDX_SEG_STC;

/*
 * typedef: struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_ROUTER_IDX_SEG_STC
 *
 * Description:
 *      IP Tcam Routing Pool Manager Index Segment for 5 columns in Router Tcam.
 *
 * Fields:
 *      segment - represent continuous block of indexes in column.
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_ROUTER_IDX_SEG_STCT
{
    PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC_IDX_SEG_STC segment[5];
} PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_ROUTER_IDX_SEG_STC;

/*******************************************************************************
* cpssDxChIpTcamMgmGetRuleIdx
*
* DESCRIPTION:
*       returns the rule index in the rules array
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       allocNodePtr     - the rule to get the idx for.
*
* OUTPUTS:
*       ruleIdxPtr       - the rule idx.
*
* RETURNS:
*       GT_OK                    - if success
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmGetRuleIdx
(
    IN  PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr,
    IN  PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC    *allocNodePtr,
    OUT GT_32                                  *ruleIdxPtr
);

/*******************************************************************************
* cpssDxChIpTcamMgmGetRuleByRuleIdx
*
* DESCRIPTION:
*       returns the rule node according to the internal rule index
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       ruleIdx          - the rule idx.
*
*
* OUTPUTS:
*       ruleNodePtrPtr      - the rule node of this idx.
*
* RETURNS:
*       GT_OK                    - if success
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmGetRuleByRuleIdx
(
    IN  PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr,
    IN  GT_32                                  ruleIdx,
    OUT PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC    **ruleNodePtrPtr
);

/*******************************************************************************
* cpssDxChIpTcamMgmCreate
*
* DESCRIPTION:
*       Creates a new IP TCAM rules pool manager
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       numOfIpTcamRules            - the number of ip Tcam rules.
*       segmentsArrayPtr            - the array of index segments that
*                                     represents the physical indexes of the
*                                     rules in this pool.
*       numOfSegments               - the number of segments
*       redirectSegmentsArrayPtr    - If != NULL, redirect info for policy-based
*                                     routing. Must be the same size as segmentsArray.
*       numOfRedirectSegments       - the number of redirect segments.
*       interConnectedPoolPtrPtr    - another interconnected pool pointer that
*                                     shared the rules with this pool in some
*                                     way, the way to convert the indexes is
*                                     using the interConnectedPoolIndexConFun
*                                     if = NULL then there is no inter connected
*                                     pool.
*       interConnectedPoolIndexConFun - the inter connected pool index convert
*                                       function.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       the pool pointer if succeeded,
*       NULL otherwise.
*
* COMMENTS:
*       numOfIpTcamRules != 0!
*
*******************************************************************************/
PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC* cpssDxChIpTcamMgmCreate
(
    IN GT_U32                                               numOfIpTcamRules,
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC_IDX_SEG_STC       *segmentsArrayPtr,
    IN GT_U32                                               numOfSegments,
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC_IDX_SEG_STC       *redirectSegmentsArrayPtr,
    IN GT_U32                                               numOfRedirectSegments,
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC                   **interConnectedPoolPtrPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_INDEX_CONVERT_FUN     interConnectedPoolIndexConFun
);

/*******************************************************************************
* cpssDxChIpTcamMgmDestroy
*
* DESCRIPTION:
*       Destroys a IP Tcam rules pool manager
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - if susccessed
*       GT_FAIL                  - otherwise
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmDestroy
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr
);

/*******************************************************************************
* cpssDxChIpTcamMgmAlloc
*
* DESCRIPTION:
*       allocates a rule of the pool by the method given
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       lowerBoundPtr    - the lower bound rule to allocate between.
*                        if = NULL the use the lowest index.
*       upperBoundPtr     - the upper bound rule to allocate between.
*                        if = NULL the use the max index.
*       allocMethod      - the allocation method in which to allocate
*
*
* OUTPUTS:
*       allocNodePtrPtr - the allocated rule node.
*
* RETURNS:
*       GT_OK if allocation succedded.
*       GT_OUT_OF_PP_MEM - if there is no space left between the upper and lower
*                          bounds.
*       GT_OUT_OF_RANGE - if the indexes were out of range of the pool.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmAlloc
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC         *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC        *lowerBoundPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC        *upperBoundPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_ALLOC_METHOD_ENT allocMethod,
    OUT PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC       **allocNodePtrPtr
);

/*******************************************************************************
* cpssDxChIpTcamMgmAllocByGivenRule
*
* DESCRIPTION:
*       allocates a given rule.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       allocRulePtr     - the rule to allocate .
*
* OUTPUTS:
*       allocNodePtrPtr  - the allocated rule node.
*
* RETURNS:
*       GT_OK if allocation succedded.
*       GT_ALREADY_EXIST         - if the rule is already allocated.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmAllocByGivenRule
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC    *allocRulePtr,
    OUT PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC   **allocNodePtrPtr
);

/*******************************************************************************
* cpssDxChIpTcamMgmFree
*
* DESCRIPTION:
*       frees a rule.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       allocNodePtr     - the rule to free.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK if allocation succedded.
*       GT_NOT_FOUND             - if the rule wasn't found.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmFree
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC    *allocNodePtr
);

/*******************************************************************************
* cpssDxChIpTcamMgmSetRealIdxAsAlloc
*
* DESCRIPTION:
*       sets a real hw index as allocated.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       hwIndex          - the hwRule index.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       NULL - if the rule is already allocated or it's not in the pool
*       or the rule pointer, if we managed to allocate it.
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC *cpssDxChIpTcamMgmSetRealIdxAsAlloc
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC     *pIpTcamPoolPtr,
    IN GT_U32                                 hwIndex
);

/*******************************************************************************
* cpssDxChIpTcamMgmCheckAvailbleMem
*
* DESCRIPTION:
*       Checks weather this poll has the availble memory
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       numOfRulesNeeded - number of tcam rules needed
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK if there is room
*       GT_OUT_OF_PP_MEM otherwise
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
* COMMENTS:
*       numOfIpTcamRules != 0!
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamMgmCheckAvailbleMem
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC   *pIpTcamPoolPtr,
    IN GT_U32                               numOfRulesNeeded
);

/*******************************************************************************
* cpssDxChIpTcamSetRuleData
*
* DESCRIPTION:
*       set a rule data by rule index, only for valid rules!
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       pIpTcamPoolPtr  - the pool pointer
*       ruleIdx         - the rule index (pce num)
*       ruleData        - the data to set for that rule
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK if allocation succedded.
*       GT_ERROR                 - for trying to set an invalid rule.
*       GT_OUT_OF_RANGE          - if the index is out of range of the pool.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChIpTcamSetRuleData
(
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC        *pIpTcamPoolPtr,
    IN GT_U32                                    ruleIdx,
    IN GT_PTR                                    ruleData
);

/*******************************************************************************
* cpssDxChIpTcamCompRulesPos
*
* DESCRIPTION:
*       compares two rules in terms of position
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       allocNode1Ptr   - the first rule to compare.
*       allocNode2Ptr   - the secone rule to compare.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       comapre result GT_EQUAL , GT_GREATER , or GT_SMALLER.
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_COMP_RES cpssDxChIpTcamCompRulesPos
(
    IN PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC    *allocNode1Ptr,
    IN PRV_CPSS_DXCH_IP_TCAM_RULE_NODE_STC    *allocNode2Ptr
);

/*******************************************************************************
* cpssDxChIpTcamMgmGetMemPoolPtr
*
* DESCRIPTION:
*       Converts VR id to Tcam manager pool pointer according to shadow type.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       vrId            - index of VR.
*       protocolStack   - stack protocol (IPv4/6).
*       shadowType      - LPM DB shadow type - determines work configuration
*                         (Ch+/Ch2/Ch2 multiple Vrs)
*       vrEntryPtr      - pointer to vr management information
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to tcam memory pool.
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_IP_TCAM_MGM_POOL_STC* cpssDxChIpTcamMgmGetMemPoolPtr
(
    IN GT_U32                                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT                       protocolStack,
    IN CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT                shadowType,
    IN PRV_CPSS_DXCH_IP_TCAM_MGM_MULTIPLE_POOLS_STC     vrEntryPool
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtCoreIpTcamMgmh */
/*
 *------------------------------------------------------------------
 * $Log: cpssDxChIpTcamMgm.h,v $
 * Revision 1.1  2015/02/13 11:32:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
