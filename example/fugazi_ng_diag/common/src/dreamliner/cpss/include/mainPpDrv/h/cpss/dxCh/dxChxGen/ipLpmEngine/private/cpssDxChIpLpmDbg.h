/* $Id: cpssDxChIpLpmDbg.h,v 1.1 2015/02/13 11:32:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h,v $
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
* cpssDxChIpLpmDbg.h
*
* DESCRIPTION:
*       the CPSS DXCH debug .
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssDxChIpLpmDbgh
#define __cpssDxChIpLpmDbgh

#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* dumpRouteTcam
*
* DESCRIPTION:
*     This func makes physical router tcam scanning and prints its contents.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*     dump   -  parameter for debugging purposes
*
* OUTPUTS:
*     None
*
* RETURNS:
*       GT_OK   - on success.
*       GT_BAD_PARAM             - on bad parameter.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_INITIALIZED       - if the driver was not initialized
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL - on failure.
*
* COMMENTS:
*     None
*
*******************************************************************************/
GT_STATUS dumpRouteTcam
(
    IN GT_BOOL dump
);


/*******************************************************************************
* cpssDxChIpPatTrieValidityCheck
*
* DESCRIPTION:
*   This function checks Patricia trie validity.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       lpmDBId     - The LPM DB id.
*       vrId        - The virtual router identifier.
*       protocol    - ip protocol
*       prefixType   - uc/mc prefix type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_STATE             - on bad state in patricia trie
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChIpPatTrieValidityCheck
(
  IN GT_U32                     lpmDBId,
  IN GT_U32                     vrId,
  IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
  IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/*******************************************************************************
* cpssDxChIpPatTriePrint
*
* DESCRIPTION:
*     This function prints Patricia trie contents.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*     lpmDbId     - The LPM DB id.
*     vrId        - The virtual router identifier.
*     protocol    - ip protocol
*     prefixType  - uc/mc prefix type
*
* OUTPUTS:
*     None.
*
* RETURNS:
*     GT_OK                    - on success
*     GT_NOT_FOUND             - if lpmDbId or vrId is not found
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChIpPatTriePrint
(
    IN GT_U32                     lpmDbId,
    IN GT_U32                     vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/*******************************************************************************
* prvCpssDxChIpPatTrieScan
*
* DESCRIPTION:
*     This func scans recursively Patricia Trie and print its contents
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       rootPtr             - pointer to the current trie node
*       protocol            - ip protocol
*       prefixType          - uc/mc prefix type
*
* OUTPUTS:
*       None
* RETURNS:
*       NoNe
* COMMENTS:
*     none
*
*******************************************************************************/

void prvCpssDxChIpPatTrieScan
(
    IN PRV_CPSS_DXCH_IP_PAT_TRIE_NODE_STC *rootPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/*******************************************************************************
* cpssDxChIpLpmIpv4UcPrefixAddMany
*
* DESCRIPTION:
*       This debug function tries to add many sequential IPv4 Unicast prefixes and
*       returns the number of prefixes successfully added.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3. 
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       lpmDbId               - the LPM DB id
*       vrId                  - the virtual router id
*       startIpAddr           - the first address to add
*       routeEntryBaseMemAddr - base memory Address (offset) of the route entry
*       numOfPrefixesToAdd    - the number of prefixes to add
*
* OUTPUTS:
*       lastIpAddrAddedPtr    - points to the last prefix successfully
*                               added (NULL to ignore)
*       numOfPrefixesAddedPtr - points to the nubmer of prefixes that were
*                               successfully added (NULL to ignore)
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong devNum or vrId.
*       GT_HW_ERROR              - on Hardware error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*******************************************************************************/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixAddMany
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/*******************************************************************************
* cpssDxChIpLpmIpv4UcPrefixAddManyRandom
*
* DESCRIPTION:
*       This function tries to add many random IPv4 Unicast prefixes and
*       returns the number of prefixes successfully added.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       lpmDbId               - the LPM DB id
*       vrId                  - the virtual router id
*       startIpAddr           - the first address to add
*       routeEntryBaseMemAddr - base memory Address (offset) of the route entry
*       numOfPrefixesToAdd    - the number of prefixes to add
*       isWholeIpRandom       - GT_TRUE: all IP octets calculated by cpssOsRand
*                               GT_FALSE: only 2 LSB octets calculated by cpssOsRand
*
* OUTPUTS:
*       numOfPrefixesAddedPtr - points to the nubmer of prefixes that were
*                               successfully added (NULL to ignore)
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong devNum or vrId.
*       GT_HW_ERROR              - on Hardware error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*******************************************************************************/
GT_STATUS cpssDxChIpLpmIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpLpmDbgh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChIpLpmDbg.h,v $
 * Revision 1.1  2015/02/13 11:32:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
