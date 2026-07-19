/* $Id: cpssDxChIpLpmTypes.h,v 1.1 2015/02/13 11:32:01 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h,v $
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
* cpssDxChIpLpmTypes.h
*
* DESCRIPTION:
*       the CPSS DXCH LPM Engine Type.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssExMxIpLpmTypesh
#define __cpssExMxIpLpmTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>


/*
 * typedef: enum CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT (type)
 *
 * Description: the types of DXCH ip shadows.
 *
 * Enumerations:
 *      CPSS_DXCH_IP_TCAM_CHEETAH_SHADOW_E
 *            - cheetah+ type shadow (routing is policy based routing)
 *              2 columns, column based search order,
 *              IPv4 & IPv6 entries use single TCAM entry
 *
 *      CPSS_DXCH_IP_TCAM_CHEETAH2_SHADOW_E
 *            - cheetah2 type shadow (routing is router TCAM based)
 *              5 columns, column based search order,
 *              MC source entries can reside only on columns 1 & 2
 *              IPv4 entry use single TCAM entry, IPv6 entry use single TCAM line
 *
 *      CPSS_DXCH_IP_TCAM_CHEETAH3_SHADOW_E
 *            - cheetah3 type shadow (routing is router TCAM based)
 *              4 columns, column based search order
 *              IPv4 entry use single TCAM entry, IPv6 entry use single TCAM line
 *
 *      CPSS_DXCH_IP_TCAM_XCAT_SHADOW_E
 *            - xCat type shadow (routing is router TCAM based)
 *              4 columns, row based search order
 *              IPv4 entry use single TCAM entry, IPv6 entry use single TCAM line
 *
 *      CPSS_DXCH_IP_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E
 *            - xCat type shadow (routing is policy based routing)
 *              4 columns, row based search order
 *              IPv4 & IPv6 entries use single TCAM entry
 *
 *      CPSS_DXCH_IP_TCAM_SHADOW_TYPE_LAST_E    - last value in the enum
 */
typedef enum
{
    CPSS_DXCH_IP_TCAM_CHEETAH_SHADOW_E                      = 0,
    CPSS_DXCH_IP_TCAM_CHEETAH2_SHADOW_E                     = 1,
    CPSS_DXCH_IP_TCAM_CHEETAH3_SHADOW_E                     = 2,
    CPSS_DXCH_IP_TCAM_XCAT_SHADOW_E                         = 3,
    CPSS_DXCH_IP_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E    = 4,
    CPSS_DXCH_IP_TCAM_SHADOW_TYPE_LAST_E

}CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT;

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC
 *
 * Description:
 *      This struct hold the capacity configuration required from the Tcam LPM
 *      manager. According to these requirements the TCAM entries will be
 *      fragmented.
 *
 * Fields:
 *  numOfIpv4Prefixes           - The total number of Ipv4 Uc prefixes and
 *                                ipv4 MC group prefixes required.
 *  numOfIpv4McSourcePrefixes   - The number of Ipv4 MC source prefixes required.
 *  numOfIpv6Prefixes           - The total number of Ipv6 Uc prefixes,
 *                                ipv6 MC group prefixes and ipv6 MC sources
 *                                prefixes required.
 */
typedef struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STCT
{
    GT_U32  numOfIpv4Prefixes;
    GT_U32  numOfIpv4McSourcePrefixes;
    GT_U32  numOfIpv6Prefixes;
}CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC;

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC
 *
 * Description:
 *      This struct hold the indexes available for the Tcam LPM manager.
 *      it defines the From-To indexes in the TCAM the lpm manager can use.
 *
 *      Cheetah+ systems: The indexes are the PCL Rule indexes dedicated for the
 *                        IP.
 *                        Pay attention that when used to indicate PCL Tcam
 *                        lines, the line x means pcl tcam indexes x,x+512.
 *      Cheetah2 systems: The indexes are the LINE indexes dedicated to for the
 *                        IP (UC + MC) out of the 1024 lines avilable. pay
 *                        attention that the a line x in the router tcam means
 *                        router tcam indexes x,x+1024,x+2048,x+3072,x+4096.
 *      Cheetah2 with VR support systems:
 *                        The indexes are the LINE indexes dedicated to for the
 *                        IP UC out of the total lines available in PCL/Router.
 *                        Pay attention that when used to indicate Router Tcam
 *                        lines, the line x means router tcam indexes
 *                        x,x+1024,x+2048,x+3072,x+4096.
 *                        Pay attention that when used to indicate PCL Tcam
 *                        lines, the line x means pcl tcam indexes x,x+512.
 *
 * Fields:
 *  firstIndex - this is the first index availble (from)
 *  lastIndex  - this is the last index availble (to - including)
 *
 */
typedef struct CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STCT
{
    GT_U32  firstIndex;
    GT_U32  lastIndex;
}CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC;

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CAPACITY_STC
 *
 * Description:
 *      This struct hold the prefixes capacity required from the Tcam LPM
 *      manager in Cheetah2 devices with VR support. According to these
 *      requirements the TCAM entries will be fragmented.
 *
 * Fields:
 *      numOfIpv4Prefixes   - The total number of Ipv4 Uc prefixes required.
 *      numOfIpv6Prefixes   - The total number of Ipv6 Uc prefixes required.
 *
 */
typedef struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CAPACITY_STCT
{
    GT_U32  numOfIpv4Prefixes;
    GT_U32  numOfIpv6Prefixes;
}CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CAPACITY_STC;

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CFG_STC
 *
 * Description:
 *      This struct hold the configuration required from the Tcam LPM
 *      manager in Cheetah2 devices with VR support. According to these
 *      requirements the TCAM entries will be fragmented.
 *
 * Fields:
 *      prefixesCfg  - prefixes configuration required to manage.
 *      tcamRange    - tcam indexes dedicated to LPM manager.
 *
 */
typedef struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CFG_STCT
{
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CAPACITY_STC  prefixesCfg;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC                   tcamRange;
}CPSS_DXCH_IP_TCAM_LPM_MANGER_CHEETAH2_VR_SUPPORT_CFG_STC;

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
 *
 * Description:
 *      This union holds the two possible accosiactions a prefix can have
 *      for Cheetha+ devices it will be a PCL action.
 *      for Cheetha2 devices it will be a LTT entry.
 *
 * Fields:
 *  pclIpUcActionPtr - a pointer to PCL action representing the UC route entry
 *                     used for Cheetha+ devices. (see CPSS_DXCH_PCL_ACTION_STC)
 *  ipLttEntryPtr    - a pointer to a LTT entry which holds the route entry
 *                     pointer and more (see CPSS_DXCH_IP_LTT_ENTRY_STC)
 */
typedef union CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNTT
{
    CPSS_DXCH_PCL_ACTION_STC        pclIpUcAction;
    CPSS_DXCH_IP_LTT_ENTRY_STC      ipLttEntry;
}CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT;


/*
 * Typedef: struct CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC
 *
 * Description: IPv4 UC prefix insert data
 *
 * Fields:
 *
 *    vrId              - The virtual router identifier.
 *    ipAddr            - The destination IP address of this prefix.
 *    prefixLen         - The prefix length of ipAddr
 *    nextHopInfo       - the route entry info accosiated with this UC prefix.
 *    numOfEcmpWaRouteEntries - Number of ECMP Work Around Route Entires.
 *                        relevant to cheetah+ only.
 *    override          - GT_TRUE, override an existing prefix.
 *                        GT_FALSE, don't override an existing prefix,and return
 *                        an error.
 *    returnStatus      - the return status for this prefix.
 *
 */
typedef struct CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STCT
{
    GT_U32                                 vrId;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                                 numOfEcmpWaRouteEntries;
    GT_BOOL                                override;
    GT_STATUS                              returnStatus;
} CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC
 *
 * Description: IPv6 UC prefix insert data
 *
 * Fields:
 *
 *    vrId              - The virtual router identifier.
 *    ipAddr            - The destination IP address of this prefix.
 *    prefixLen         - The prefix length of ipAddr
 *    nextHopInfo       - the route entry info accosiated with this UC prefix.
 *    numOfEcmpWaRouteEntries - Number of ECMP Work Around Route Entires.
 *                        relevant to cheetah+ only.
 *    override          - GT_TRUE, override an existing prefix.
 *                        GT_FALSE, don't override an existing prefix,and return
 *                        an error.
 *    returnStatus      - the return status for this prefix.
 *
 */
typedef struct CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STCT
{
    GT_U32                                 vrId;
    GT_IPV6ADDR                            ipAddr;
    GT_U32                                 prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                                 numOfEcmpWaRouteEntries;
    GT_BOOL                                override;
    GT_STATUS                              returnStatus;
} CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC;


/*
 * Typedef: struct CPSS_DXCH_IP_LPM_VR_CONFIG_STCT
 *
 * Description: IP virtual router configuration parameters.
 *
 * Fields:
 *
 *      supportIpv4Uc           - whether this VR support UC Ipv4.
 *      defIpv4UcNextHopInfo    - the route entry info associated with the default
 *                                Ipv4 UC route entry.
 *      supportIpv4Mc           - whether this VR support MC Ipv4.
 *      defIpv4McRouteLttEntry  - the LTT entry pointing to the default ipv4 MC route
 *                                entry.
 *      supportIpv6Uc           - whether this VR support UC Ipv6.
 *      defIpv6UcNextHopInfo    - the route entry info associated with the default
 *                                Ipv6 UC route entry.
 *      supportIpv6Mc           - whether this VR support MC Ipv6
 *      defIpv6McRouteLttEntry  - the LTT entry pointing to the default Ipv6 MC route
 *                                entry;
 * Comments:
 *       supportIpv4Uc, supportIpv4Mc, supportIpv6Uc, supportIpv6Mc can't be all
 *       GT_FALSE.
 */

typedef struct CPSS_DXCH_IP_LPM_VR_CONFIG_STCT
{
    GT_BOOL                                 supportIpv4Uc;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  defIpv4UcNextHopInfo;
    GT_BOOL                                 supportIpv4Mc;
    CPSS_DXCH_IP_LTT_ENTRY_STC              defIpv4McRouteLttEntry;
    GT_BOOL                                 supportIpv6Uc;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  defIpv6UcNextHopInfo;
    GT_BOOL                                 supportIpv6Mc;
    CPSS_DXCH_IP_LTT_ENTRY_STC              defIpv6McRouteLttEntry;
} CPSS_DXCH_IP_LPM_VR_CONFIG_STC;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssExMxIpLpmTypesh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChIpLpmTypes.h,v $
 * Revision 1.1  2015/02/13 11:32:01  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
