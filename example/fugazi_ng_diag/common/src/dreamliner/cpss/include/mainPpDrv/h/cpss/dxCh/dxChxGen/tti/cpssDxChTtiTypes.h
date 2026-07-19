/* $Id: cpssDxChTtiTypes.h,v 1.1 2015/02/13 11:33:00 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h,v $
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
* cpssDxChTtiTypes.h
*
* DESCRIPTION:
*       CPSS definitions for tunnel.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __cpssDxChTtiTypesh
#define __cpssDxChTtiTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/tunnel/cpssGenTunnelTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

/*
 * typedef: enum CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT
 *
 * Description:
 *      TTI includes src interface fields in MAC2ME table type.
 *
 * Enumerations:
 *
 *   CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E - do not use the source interface
 *                                                   fields as part of the MAC2ME lookup
 *   CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E - use the source interface fields
 *                                              as part of the MAC2ME lookup
 *   CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E - use source
 *              interface fields as part of the MAC2ME lookup, exclude the source Device.
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E               = 0,
    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E                    = 1,
    CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E = 2

} CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT;

/*
 * Typedef: struct CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC
 *
 * Description:
 *          The MAC TO ME lookup is used to identify packets destined to:
 *          1. {MAC address, VLAN} if <includeSrcInterfaceType> = 0
 *          2. {MAC address, VLAN, SRC device, SRC is trunk, SRC trunk/port id}
 *              if <includeSrcInterfaceType> = 1
 *          3. {MAC address, VLAN, SRC is trunk, SRC trunk/port id}
 *              if <includeSrcInterfaceType> = 2.
 *          This indication is used in subsequent pipeline units such as
 *          the TTI lookup and Ingress Policy engine or for TRILL packets.
 *
 * Fields:
 *
 *  includeSrcInterfaceType - whether to include src interface fields
 *                            0: do not use the source interface fields as part
 *                               of the MAC2ME lookup
 *                            1: use the source interface fields as part of the
 *                               MAC2ME lookup
 *                            2: use source interface fields as part of the
 *                               MAC2ME lookup, but exclude the Source Device.
 *  srcDevice               - local source device or DSA tag source device
 *  srcIsTrunk              - whether source is port or trunk (APPLICABLE RANGES: 0..1)
 *                            0: Source is not a trunk (i.e. it is port)
 *                            1: Source is a trunk
 *  srcPortTrunk            - Source port or Source trunk-ID
 *                            if port, range (20 bits)
 *                            if trunk, range (12 bits)
 *
 */
typedef struct
{
    CPSS_DXCH_TTI_MAC_TO_ME_INCLUDE_SRC_INTERFACE_TYPE_ENT  includeSrcInterface;
    GT_HW_DEV_NUM                                           srcDevice;
    GT_BOOL                                                 srcIsTrunk;
    GT_U32                                                  srcPortTrunk;

} CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_MAC_VLAN_STC
 *
 * Description:
 *          Mac To Me and Vlan To Me.
 *
 * Fields:
 *
 *  mac            - Mac to me address
 *  vlanId         - vlan-to-me ID
 *
 */
typedef struct
{
    GT_ETHERADDR        mac;
    GT_U16              vlanId;

} CPSS_DXCH_TTI_MAC_VLAN_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_TRILL_ADJACENCY_STC
 *
 * Description:
 *          The TRILL engine uses a single TCAM lookup to implement
 *          the following TRILL adjacency checks:
 *          1. TRILL IS-IS Adjacency check -
 *             Checks that the single-destination TRILL frame arrives from a
 *             {neighbor, port} for which an IS-IS adjacency exists.
 *          2. TRILL Tree Adjacency Check -
 *             Checks that the multi-destination TRILL frame arrives from a
 *             {neighbor, port} that is a branch on the given TRILL distribution tree.
 *
 *          A TCAM lookup is performed for every TRILL packet processed by the
 *          TRILL engine.
 *
 * Fields:
 *
 *      outerMacSa              - The outer MAC source address
 *      trillMBit               - The TRILL header M-bit (APPLICABLE RANGES: 0..1)
 *      trillEgressRbid         - The TRILL header Egress RBridge Nickname (16 bits)
 *      srcDevice               - local source device or DSA tag source device (12 bits)
 *      srcIsTrunk              - whether source is port or trunk (APPLICABLE RANGES: 0..1)
 *                                0: Source is not a trunk (i.e. it is port)
 *                                1: Source is a trunk
 *      srcPortTrunk            - Source port or Source trunk-ID
 *                                if port, range (20 bits)
 *                                if trunk, range (12 bits)
 *
 */
typedef struct
{
    GT_ETHERADDR                    outerMacSa;
    GT_U32                          trillMBit;
    GT_U32                          trillEgressRbid;
    GT_HW_DEV_NUM                   srcDevice;
    GT_U32                          srcIsTrunk;
    GT_U32                          srcPortTrunk;
} CPSS_DXCH_TTI_TRILL_ADJACENCY_STC;


/*
 * typedef: enum CPSS_DXCH_TTI_KEY_TYPE_ENT
 *
 * Description:
 *      TTI key type.
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_KEY_IPV4_E    - IPv4 TTI key type
 *      CPSS_DXCH_TTI_KEY_MPLS_E    - MPLS TTI key type
 *      CPSS_DXCH_TTI_KEY_ETH_E     - Ethernet TTI key type
 *      CPSS_DXCH_TTI_KEY_MIM_E     - Mac in Mac TTI key type (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *      CPSS_DXCH_TTI_KEY_TRILL_E   - TRILL TTI key type (APPLICABLE DEVICES: Lion3)
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_KEY_IPV4_E  = 0,
    CPSS_DXCH_TTI_KEY_MPLS_E  = 1,
    CPSS_DXCH_TTI_KEY_ETH_E   = 2,
    CPSS_DXCH_TTI_KEY_MIM_E   = 3,
    CPSS_DXCH_TTI_KEY_TRILL_E = 4

} CPSS_DXCH_TTI_KEY_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH_TTI_MAC_MODE_ENT
 *
 * Description:
 *      Determines MAC that will be used to generate lookup TCAM key.
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_MAC_MODE_DA_E     - use destination MAC
 *      CPSS_DXCH_TTI_MAC_MODE_SA_E     - use source MAC
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_MAC_MODE_DA_E   = 0,
    CPSS_DXCH_TTI_MAC_MODE_SA_E   = 1

} CPSS_DXCH_TTI_MAC_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_TTI_QOS_MODE_TYPE_ENT
 *
 * Description: QoS trust mode type
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E       - keep prior QoS  profile assignment
 *  CPSS_DXCH_TTI_QOS_TRUST_PASS_L2_E       - Trust passenger packet L2 QoS,
 *                                            relevant ONLY if the passenger packet is Ethernet
 *  CPSS_DXCH_TTI_QOS_TRUST_PASS_L3_E       - Trust passenger packet L3 QoS
 *  CPSS_DXCH_TTI_QOS_TRUST_PASS_L2_L3_E    - Trust passenger packet L2 and L3 QoS
 *  CPSS_DXCH_TTI_QOS_UNTRUST_E             - Untrust packet QoS
 *  CPSS_DXCH_TTI_QOS_TRUST_MPLS_EXP_E      - Trust outer MPLS label EXP
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E       = 0,
    CPSS_DXCH_TTI_QOS_TRUST_PASS_L2_E       = 1,
    CPSS_DXCH_TTI_QOS_TRUST_PASS_L3_E       = 2,
    CPSS_DXCH_TTI_QOS_TRUST_PASS_L2_L3_E    = 3,
    CPSS_DXCH_TTI_QOS_UNTRUST_E             = 4,
    CPSS_DXCH_TTI_QOS_TRUST_MPLS_EXP_E      = 5

} CPSS_DXCH_TTI_QOS_MODE_TYPE_ENT;

/*
 * Typedef: struct CPSS_DXCH_TTI_TRILL_RBID_TABLE_ENTRY_STC
 *
 * Description:
 *      The RBID engine consists of the RBID Lookup Translation Table and RBID Table.
 *      For each TRILL packet, the RBID table is accessed twice: once with the index
 *      set to the TRILL.I-RBID, and once with the index set to the TRILL.E-RBID.
 *
 * Fields:
 *
 *      srcTrgPort  - Ingress RBridge lookup: Source port associated with Ingress RBridge
 *                    Egress RBridge lookup:  Target port associated with Egress RBridge
 *      trgDevice   - Target device associated with Egress RBridge
 *
 */
typedef struct
{
    GT_PORT_NUM                     srcTrgPort;
    GT_HW_DEV_NUM                   trgDevice;
} CPSS_DXCH_TTI_TRILL_RBID_TABLE_ENTRY_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC
 *
 * Description:
 *      The RBID table maps each RBridge to interface representing the nexthop
 *      in the path to this RBridge.
 *      The RBID table is accessed indirectly via the RBID Lookup Translation Table (LTT),
 *      which maps the 16-bit RBID to RBID table index.
 *
 * Fields:
 *
 *      rbidEntryIndex  - Index into RBID table. (APPLICABLE RANGES: 0..4095)
 */
typedef struct
{
    GT_U32                   rbidEntryIndex;
} CPSS_DXCH_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_RULE_COMMON_STC
 *
 * Description:
 *          TTI Common fields in TCAM rule.
 *
 * Fields:
 *
 *  pclId             - PCL id
 *  srcIsTrunk        - whether source is port or trunk (APPLICABLE RANGES: 0..1)
 *                      0: Source is not a trunk (i.e. it is port)
 *                      1: Source is a trunk
 *  srcPortTrunk      - Source port or Source trunk-ID
 *                      if port, range (APPLICABLE RANGES: 0..63)
 *                      if trunk, range (APPLICABLE RANGES: 0..127)
 *  mac               - MAC DA or MAC SA of the tunnel header
 *  vid               - packet VID assignment (APPLICABLE RANGES: 0..4095)
 *  isTagged          - whether the external mac is considered tagged on not
 *  dsaSrcIsTrunk     - whether sourse is Trunk or not
 *  dsaSrcPortTrunk   - DSA tag source port or trunk; relevant only for DSA tagged packets.
 *                      In multi port groups devices if this field is a criteria in rule,
 *                      the field sourcePortGroupId is required to be filled as well :
 *                      exact mach and equals to portGroupId to which dsaSrcPortTrunk
 *                      is related.
 *  dsaSrcDevice      - DSA tag source device; relevant only for DSA tagged packets
 *  sourcePortGroupId - Indicates the port group where the packet entered the device.
 *                     (APPLICABLE DEVICES: Lion; Lion2; Lion3; for multi port groups devices only)
 *
 *
 */
typedef struct
{
    GT_U32                  pclId;
    GT_BOOL                 srcIsTrunk;
    GT_U32                  srcPortTrunk;
    GT_ETHERADDR            mac;
    GT_U16                  vid;
    GT_BOOL                 isTagged;
    GT_BOOL                 dsaSrcIsTrunk;
    GT_U32                  dsaSrcPortTrunk;
    GT_U32                  dsaSrcDevice;
    GT_U32                  sourcePortGroupId;

} CPSS_DXCH_TTI_RULE_COMMON_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_IPV4_RULE_STC
 *
 * Description:
 *          TTI IPv4 TCAM rule.
 *
 * Fields:
 *
 *  common            - TTI Common fields in TCAM rule.
 *  tunneltype        - tunneling protocol of the packet: valid options:
 *                          0: IPv4_OVER_IPv4
 *                          1: IPv6_OVER_IPv4
 *                          2: IPv4_OVER_GRE_IPv4; Tunnel IP protocol is 47 and
 *                                  GRE protocol is 0x0800.
 *                          3: IPv6_OVER_GRE_IPv4; Tunnel IP protocol is 47 and
 *                                  GRE protocol is 0x86DD.
 *                          4: Ethernet_OVER_GRE0_IPv4; Tunnel IP protocol is 47 and
 *                                 GRE protocol is set in function cpssDxChTtiEthernetTypeSet.
 *                          5: Ethernet_OVER_GRE1_IPv4; Tunnel IP protocol is 47 and
 *                                 GRE protocol is set in function cpssDxChTtiEthernetTypeSet.
 *                          6: Unknown - If the packet is identified as IPv4 GRE tunnel and
 *                                       there is no match on the configured GRE protocols
 *                          7: Any Protocol - the packet is not one of the above protocols
 *  srcIp             - tunnel source IP
 *  destIp            - tunnel destination IP
 *  isArp             - whether the packet is ARP or not ARP
 *
 */
typedef struct
{
    CPSS_DXCH_TTI_RULE_COMMON_STC common;
    GT_U32                  tunneltype;
    GT_IPADDR               srcIp;
    GT_IPADDR               destIp;
    GT_BOOL                 isArp;

} CPSS_DXCH_TTI_IPV4_RULE_STC;


/*
 * Typedef: struct CPSS_DXCH_TTI_MPLS_RULE_STC
 *
 * Description:
 *          TTI MPLS TCAM rule.
 *
 * Fields:
 *
 *  common            - TTI Common fields in TCAM rule.
 *  label0            - MPLS label 0; the outer most mpls label (APPLICABLE RANGES: 0..2^20-1)
 *  exp0              - EXP of MPLS label 0 (APPLICABLE RANGES: 0..7)
 *  label1            - MPLS label 1; the inner MPLS label following Label0 (APPLICABLE RANGES: 0..2^20-1)
 *                      Note: must be set to 0 if this label does not exist
 *  exp1              - EXP of MPLS label 1 (APPLICABLE RANGES: 0..7)
 *                      Note: must be set to 0 if this label does not exist
 *  label2            - MPLS label 2; the the inner MPLS label following Label1 (APPLICABLE RANGES: 0..2^20-1)
 *                      Note: must be set to 0 if this label does not exist
 *  exp2              - EXP of MPLS label 2 (APPLICABLE RANGES: 0..7)
 *                      Note: must be set to 0 if this label does not exist
 *  numOfLabels       - The number of MPLS labels in the label stack. valid options:
 *                          0: for One label
 *                          1: for two labels
 *                          2: for three labels
 *                          3: for more than 3 labels
 *  protocolAboveMPLS - The protocol above MPLS. valid options:
 *                          IPv4
 *                          IPv6
 *                      Note: feild not valid if more than 3 labels in the stack
 *  reservedLabelExist - GT_TRUE: if one of the MPLS stack labels is a reserved
 *                                label (0<=label<=15)
 *                       GT_FALSE: no reserved MPLS label is found in MPLS stack
 *                      (APPLICABLE DEVICES: Lion3)
 *  reservedLabelValue - If reservedLabelExist==GT_TRUE: The value of the 4 least
 *                       significant bits of the reserved label
 *                       If reservedLabelExist==GT_FALSE: The value of the 4 least
 *                       significant bits of the MPLS label that is at the bottom
 *                       of the MPLS stack
 *                      (APPLICABLE DEVICES: Lion3)
 *  channelTypeProfile - (APPLICABLE DEVICES: Lion3)
 *
 */
typedef struct
{
    CPSS_DXCH_TTI_RULE_COMMON_STC common;
    GT_U32                  label0;
    GT_U32                  exp0;
    GT_U32                  label1;
    GT_U32                  exp1;
    GT_U32                  label2;
    GT_U32                  exp2;
    GT_U32                  numOfLabels;
    GT_U32                  protocolAboveMPLS;
    GT_BOOL                 reservedLabelExist;
    GT_U32                  reservedLabelValue;
    GT_U32                  channelTypeProfile;

} CPSS_DXCH_TTI_MPLS_RULE_STC;


/*
 * Typedef: struct CPSS_DXCH_TTI_ETH_RULE_STC
 *
 * Description:
 *          TTI Ethernet TCAM rule.
 *
 * Fields:
 *
 *  common            - TTI Common fields in TCAM rule.
 *  up0               - user priority as found in the outer VLAN tag header
 *  cfi0              - CFI (Canonical Format Indicator) as found in the outer
 *                      VLAN tag header
 *  isVlan1Exists     - whether the fields: VLAN-ID1, UP1, and CFI1, are valid
 *                      vlan1 is the vlan header matching the Ethertype that
 *                      is different from the Ethertype selected for the port
 *  vid1              - VLAN ID of vlan1
 *  up1               - user priority of vlan1
 *  cfi1              - CFI (Canonical Format Indicator) of vlan1
 *  etherType         - ether type
 *  macToMe           - if there was a match in MAC2ME table
 *  srcId             - When packet has DSA tag this field reflects the source
 *                      device as extracted from the DSA tag.
 *                      When packet does not contain DSA tag this field is set to 0.
 *                     (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *  dsaQosProfile     - DSA tag QoS profile; relevant only for DSA tagged packets
 *                     (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *  tag0TpidIndex     - (APPLICABLE DEVICES: Lion3)
 *  tag1TpidIndex     - (APPLICABLE DEVICES: Lion3)
 *
 */
typedef struct
{
    CPSS_DXCH_TTI_RULE_COMMON_STC common;
    GT_U32                  up0;
    GT_U32                  cfi0;
    GT_BOOL                 isVlan1Exists;
    GT_U16                  vid1;
    GT_U32                  up1;
    GT_U32                  cfi1;
    GT_U32                  etherType;
    GT_BOOL                 macToMe;
    GT_U32                  srcId;
    GT_U32                  dsaQosProfile;
    GT_U32                  tag0TpidIndex;
    GT_U32                  tag1TpidIndex;
} CPSS_DXCH_TTI_ETH_RULE_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_MIM_RULE_STC
 *
 * Description:
 *          TTI MIM TCAM rule.
 *
 * Fields:
 *
 *  common            - TTI Common fields in TCAM rule.
 *  bUp               - backbone user priority assigned from vlan header
 *  bDp               - backbone drop precedence assigned from vlan header
 *  iSid              - inner service instance assigned from tag information
 *  iUp               - inner user priority assigned from tag information
 *  iDp               - inner drop precedence assigned from tag information
 *  iRes1             - 2 reserved bits
 *  iRes2             - 2 reserved bits
 *  macToMe                 - (APPLICABLE DEVICES: Lion3)
 *  innerPacketTag0Exists   - (APPLICABLE DEVICES: Lion3)
 *  innerPacketTag0Vid      - (APPLICABLE DEVICES: Lion3)
 *  innerPacketTag0Up       - (APPLICABLE DEVICES: Lion3)
 *  innerPacketTag0Dei      - (APPLICABLE DEVICES: Lion3)
 */
typedef struct
{
    CPSS_DXCH_TTI_RULE_COMMON_STC   common;
    GT_U32                          bUp;
    GT_U32                          bDp;
    GT_U32                          iSid;
    GT_U32                          iUp;
    GT_U32                          iDp;
    GT_U32                          iRes1;
    GT_U32                          iRes2;
    GT_BOOL                         macToMe;
    GT_BOOL                         innerPacketTag0Exists;
    GT_U32                          innerPacketTag0Vid;
    GT_U32                          innerPacketTag0Up;
    GT_U32                          innerPacketTag0Dei;
} CPSS_DXCH_TTI_MIM_RULE_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_TRILL_RULE_STC
 *
 * Description:
 *          TTI TRILL TCAM rule
 *
 * Fields:
 *
 *  common                      - TTI Common fields in TCAM rule.
 *      common.srcIsTrunk           - If packet arrived DSA-tagged && physical port <Centralized Chassis> ==1
 *                                    this field is always 0.
 *                                    If packet arrived DSA-tagged && physical port <Centralized Chassis> ==0
 *                                    this is the DSA tag <Src Is Trunk>.
 *                                    If the packet arrived non-DSA, this bit is set if the default port
 *                                    <Trunk-ID> != 0; otherwise it is clear.
 *      common.srcPortTrunk         - If the packet arrived DSA-tagged && physical port <Centralized Chassis> == 0
 *                                    this is the DSA tag <source Trunk-ID/Port>.
 *                                    If the packet arrived DSA-tagged && physical port <Centralized Chassis> ==1
 *                                    this is the Centralized Chassis default port mapped from the DSA <SrcTrunk-ID/Port>.
 *                                    If packet arrived non-DSA tagged:
 *                                      If default port <Trunk-ID> != 0, this is the default port <trunk-ID>.
 *                                      If default port <Trunk-ID> == 0, this is the physical port <Default port>.
 *      common.dsaSrcDevice         - If packet arrived DSA-tagged && physical port <Centralized Chassis> ==1
 *                                    this is the local Device.
 *                                    If packet arrived DSA-tagged && physical port <Centralized Chassis> ==0
 *                                    this is the DSA tag <SrcDev>.
 *                                    If not-DSA, this is the local Device.
 *      common.isTagged             - Not Relevant
 *      common.dsaSrcIsTrunk        - Not Relevant
 *      common.dsaSrcPortTrunk      - Not Relevant
 *  trillMBit                   - TRILL header Multi-destination bit
 *  trillEgressRbid             - TRILL Egress RBID (16 bits)
 *  trillIngressRbid            - TRILL Ingress RBID (16 bits)
 *  innerPacketMacDa            - The inner packet MAC DA
 *  innerPacketTag0Exists       - 1: Tag0 found in inner packet
 *                                0: Tag0 not found in inner packet
 *  innerPacketTag0Vid          - The inner packet Tag0 VID classification (12 bits)
 *  innerPacketFieldsAreValid   - innerPacketMacDa, innerPacketTag0Exists and innerPacketTag0Vid
 *                                fields are valid.
 *                                NOTE: the inner packet fields are not valid when
 *                                      the option data exceed 40B
 *  trillMcDescriptorInstance   - All multi-target TRILL packets received are always duplicated.
 *                                The 1st instance has key field assignment
 *                                <TRILL MC Desc Instance> = 0
 *                                The 2nd instance has key field assignment
 *                                <TRILL MC Desc Instance> = 1
 *                                NOTE: this indication is not from the packet
 *
 */
typedef struct
{
    CPSS_DXCH_TTI_RULE_COMMON_STC   common;
    GT_BOOL                         trillMBit;
    GT_U32                          trillEgressRbid;
    GT_U32                          trillIngressRbid;
    GT_ETHERADDR                    innerPacketMacDa;
    GT_BOOL                         innerPacketTag0Exists;
    GT_U32                          innerPacketTag0Vid;
    GT_BOOL                         innerPacketFieldsAreValid;
    GT_BOOL                         trillMcDescriptorInstance;
} CPSS_DXCH_TTI_TRILL_RULE_STC;

/*
 * typedef: union CPSS_DXCH_TTI_RULE_UNT
 *
 * Description:
 *      Union for TTI rule.
 *
 * Fields:
 *
 *  ipv4      - ipv4 rule
 *  mpls      - MPLS rule
 *  eth       - ethernet rule
 *  mim       - Mac in Mac rule (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *  trill     - TRILL rule (APPLICABLE DEVICES: Lion3)
 *
 */
typedef union
{
    CPSS_DXCH_TTI_IPV4_RULE_STC   ipv4;
    CPSS_DXCH_TTI_MPLS_RULE_STC   mpls;
    CPSS_DXCH_TTI_ETH_RULE_STC    eth;
    CPSS_DXCH_TTI_MIM_RULE_STC    mim;
    CPSS_DXCH_TTI_TRILL_RULE_STC  trill;

} CPSS_DXCH_TTI_RULE_UNT;

/*
 * typedef: enum CPSS_DXCH_TTI_PASSENGER_TYPE_ENT
 *
 * Description:
 *      TTI tunnel passenger protocol types.
 *
 * Enumerations:
 *      CPSS_DXCH_TTI_PASSENGER_IPV4_E            - IPv4 passenger type
 *      CPSS_DXCH_TTI_PASSENGER_IPV6_E            - IPv6 passenger type
 *      CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E    - ethernet with crc passenger type
 *      CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E - ethernet with no crc passenger type
 *      CPSS_DXCH_TTI_PASSENGER_IPV4V6_E          - IPv4/IPv6 passenger type
 *      CPSS_DXCH_TTI_PASSENGER_MPLS_E            - MPLS passenger type
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_PASSENGER_IPV4_E              = 0,
    CPSS_DXCH_TTI_PASSENGER_IPV6_E              = 1,
    CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E      = 2,
    CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E   = 3,
    CPSS_DXCH_TTI_PASSENGER_IPV4V6_E            = 4,
    CPSS_DXCH_TTI_PASSENGER_MPLS_E              = 5

} CPSS_DXCH_TTI_PASSENGER_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH_MPLS_CMD_ENT
 *
 * Description: Enumeration of MPLS command assigned to the packet
 *
 * Enumerations:
 *  CPSS_DXCH_TTI_MPLS_NOP_CMD_E          - do nothing
 *  CPSS_DXCH_TTI_MPLS_SWAP_CMD_E         - swap the other label
 *  CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E        - push a new label
 *  CPSS_DXCH_TTI_MPLS_POP1_CMD_E         - pop 1 label
 *  CPSS_DXCH_TTI_MPLS_POP2_CMD_E         - pop 2 labels
 *  CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E - pop one label and swap the second label
 *
 */
typedef enum {
    CPSS_DXCH_TTI_MPLS_NOP_CMD_E            = 0,
    CPSS_DXCH_TTI_MPLS_SWAP_CMD_E           = 1,
    CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E          = 2,
    CPSS_DXCH_TTI_MPLS_POP1_CMD_E           = 3,
    CPSS_DXCH_TTI_MPLS_POP2_CMD_E           = 4,
    CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E   = 5

}CPSS_DXCH_TTI_MPLS_CMD_ENT;

/*
 * typedef: enum CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT
 *
 * Description:
 *      TTI action redirect command.
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_NO_REDIRECT_E               - do not redirect this packet
 *      CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E        - policy switching : Any non-TT can
 *                                                  be redirect to egress port.  But if
 *                                                  TT, then only Ether-over-MPLS
 *      CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E - policy routing    (APPLICABLE DEVICES: xCat; Lion; Lion2; Lion3)
 *      CPSS_DXCH_TTI_VRF_ID_ASSIGN_E             - vrf id assignment (APPLICABLE DEVICES: xCat; Lion; Lion2; Lion3)
 *      CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E       - logical port assignment
 *                                                  (APPLICABLE DEVICES: Lion2)
 *                                                   Lion2 device: applicable for revision B0 and above
 */
typedef enum
{
    CPSS_DXCH_TTI_NO_REDIRECT_E               = 0,
    CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E        = 1,
    CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E = 2,
    CPSS_DXCH_TTI_VRF_ID_ASSIGN_E             = 4,
    CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E       = 5

} CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT;

/*
 * typedef: enum CPSS_DXCH_TTI_VLAN_COMMAND_ENT
 *
 * Description:
 *      TTI action VLAN command.
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E      - do not modify vlan
 *      CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E    - modify vlan only for untagged packets
 *      CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E      - modify vlan only for tagged packets
 *      CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E         - modify vlan to all packets
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E    = 0,
    CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E  = 1,
    CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E    = 2,
    CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E       = 3

} CPSS_DXCH_TTI_VLAN_COMMAND_ENT;


/*
 * typedef: enum CPSS_DXCH_TTI_MODIFY_UP_ENT
 *
 * Description:
 *      TTI modify UP enable
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E     - do not modify the previous UP enable setting
 *      CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E            - set modify UP enable flag to 1
 *      CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E       - set modify UP enable flag to 0
 *      CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E        - reserved
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E   = 0,
    CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E        = 1,
    CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E       = 2,
    CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E      = 3

} CPSS_DXCH_TTI_MODIFY_UP_ENT;

/*
 * typedef: enum CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT
 *
 * Description:
 *      TTI tag1 UP command
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E - If packet does not contain
 *          Tag1 assign according to action entry's <UP1>, else use Tag1<UP>
 *      CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E - If packet contains Tag0
 *          use Tag0<UP0>, else use action entry's <UP1> field
 *      CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E - Assign action entry's <UP1> field
 *          to all packets
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E      = 0,
    CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E      = 1,
    CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E                 = 2

} CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT;

/*
 * typedef: enum CPSS_DXCH_TTI_MODIFY_DSCP_ENT
 *
 * Description:
 *      TTI modify DSCP enable
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E   - do not modify the previous DSCP enable setting
 *      CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E      - set modify DSCP enable flag to 1
 *      CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E         - set modify DSCP enable flag to 0
 *      CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E      - reserved
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E = 0,
    CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E      = 1,
    CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E     = 2,
    CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E    = 3

} CPSS_DXCH_TTI_MODIFY_DSCP_ENT;

/*
 * Typedef: struct CPSS_DXCH_TTI_ACTION_STC
 *
 * Description:
 *          TTI TCAM rule action parameters (standard action).
 *
 * Fields:
 *
 *  Tunnel Terminate Action Fields
 *  ------------------------------
 *      tunnelTerminate       - whether the packet's tunnel header is removed;
 *                              must not be set for non-tunneled packets
 *      passengerPacketType   - passenger packet type (refer to CPSS_DXCH_TTI_PASSENGER_TYPE_ENT).
 *                              valid if <tunnelTerminate> is GT_TRUE
 *                              valid values:
 *                                 CPSS_DXCH_TTI_PASSENGER_IPV4_E
 *                                 CPSS_DXCH_TTI_PASSENGER_IPV6_E
 *                                 CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E
 *                                 CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E
 *      copyTtlFromTunnelHeader  - whether to copy TTL from tunnel header. valid if
 *                                <tunnelTerminate> is GT_TRUE and passenger packet is IPV4/6
 *
 *  Forwarding Attributes
 *  ---------------------
 *      command               - forwarding command; valid values:
 *                                  CPSS_PACKET_CMD_FORWARD_E
 *                                  CPSS_PACKET_CMD_MIRROR_TO_CPU_E
 *                                  CPSS_PACKET_CMD_TRAP_TO_CPU_E
 *                                  CPSS_PACKET_CMD_DROP_HARD_E
 *                                  CPSS_PACKET_CMD_DROP_SOFT_E
 *
 *  Redirect Command and Data
 *  -------------------------
 *      redirectCommand        - where to redirect the packet; valid values:
 *                               (refer to CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)
 *      egressInterface        - egress interface to redirect packet. valid if <redirectCommand> is
 *                               REDIRECT_TO_EGRESS. The packet is forwarded as Unicast to a port or a trunk
 *      tunnelStart            - whether <redirectCommand> is REDIRECT_TO_EGRESS. Relevant only to
 *                               Ethernet-over-MPLS tunnel termination.
 *      tunnelStartPtr         - pointer to the Tunnel Start entry. valid if <tunnelStart>
 *                               is GT_TRUE and <redirectCommand> is REDIRECT_TO_EGRESS
 *      routerLookupPtr        - pointer to the Router Lookup Translation Table entry
 *                               if <redirectCommand> is REDIRECT_TO_ROUTER_LOOKUP
 *                               FEr#2018 - Router Lookup Translation Table (LTT) index can be written
 *                               only to column 0 of the LTT row.
 *                               NOTE:
 *                               - DxCh3 devices support index 0,1,2,3(max IP TCAM row) only
 *                               - xCat devices support index 0,4,8,12(max IP TCAM row * 4) only
 *      vrfId                  - VRF-ID assigned to the packet if <redirectCommand> is VRF_ID_ASSIGN
 *
 *
 *  Source ID assignment
 *  --------------------
 *      sourceIdSetEnable     - if set, then the <sourceId> is assign to the packet
 *      sourceId              - source ID assigned to the packet if <sourceIdSetEnable> is GT_TRUE
 *
 *
 *  VLAN Modification and Assignment
 *  --------------------------------
 *      vlanCmd               - vlan command applied to packets matching the TTI
 *                              (refer to CPSS_DXCH_TTI_VLAN_COMMAND_ENT)
 *      vlanId                - TTI VLAN-ID assignment if <vlanCmd> != 0
 *      vlanPrecedence        - whether the VID assignment can be overridden by
 *                              subsequent VLAN assignment mechanism
 *                              (refer to CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)
 *      nestedVlanEnable      - enable/disable nested vlan; relevant only for
 *                              Ethernet packets
 *
 *  Bind to Policer
 *  ---------------
 *      bindToPolicer         - If set, the packet is bound to <policerIndex>
 *      policerIndex          - Traffic profile to be used if the <bindToPolicer> is set
 *
 *
 *  Qos Fields
 *  ----------
 *      qosPrecedence         - whether QoS profile can be overridden by subsequent pipeline
 *                              engines (refer to CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)
 *      qosTrustMode          - QoS trust mode type (refer to CPSS_DXCH_TTI_QOS_MODE_TYPE_ENT)
 *      qosProfile            - QoS profile to assign to the packet in case <qosTrustMode> is 4
 *      modifyUpEnable        - modify UP enable (refer to CPSS_DXCH_TTI_MODIFY_UP_ENT)
 *      modifyDscpEnable      - modify DSCP enable (refer to CPSS_DXCH_TTI_MODIFY_DSCP_ENT)
 *      up                    - default UP assignment
 *      remapDSCP             - Whether to remap the DSCP. if <tunnelTerminateEnable> is 1, valid
 *                              if <qosTrustMode> is 2/3 and passenger packet is IPV4/6.
 *                              if <tunnelTerminateEnable> is 0, valid if <qosTrustMode> is
 *                              2/3 and the packet is IPV4/6.
 *
 *  Management and Debug
 *  --------------------
 *      mirrorToIngressAnalyzerEnable - If set, the packet is mirrored to the Ingress
 *                                      Analyzer port.
 *      userDefinedCpuCode    - user defined cpu code. relavant only if command is
 *                              trap or mirror
 *      vntl2Echo             - if set, the packet MAC SA/DA are switched on the transmitting
 *      bridgeBypass          - if set, the packet isn't subject to any bridge mechanisms
 *      actionStop            - if set, the packet will not have any searches in the IPCL mechanism
 *
 *  CNC binding
 *  -----------
 *      activateCounter - enables binding of a CNC counter with matching of this TTI action.
 *      counterIndex    - the CNC match counter index.
 */
typedef struct
{
    GT_BOOL                                 tunnelTerminate;
    CPSS_DXCH_TTI_PASSENGER_TYPE_ENT        passengerPacketType;
    GT_BOOL                                 copyTtlFromTunnelHeader;

    CPSS_PACKET_CMD_ENT                     command;

    CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT      redirectCommand;
    CPSS_INTERFACE_INFO_STC                 egressInterface;
    GT_BOOL                                 tunnelStart;
    GT_U32                                  tunnelStartPtr;
    GT_U32                                  routerLookupPtr;
    GT_U32                                  vrfId;

    GT_BOOL                                 sourceIdSetEnable;
    GT_U32                                  sourceId;

    CPSS_DXCH_TTI_VLAN_COMMAND_ENT          vlanCmd;
    GT_U16                                  vlanId;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT vlanPrecedence;
    GT_BOOL                                 nestedVlanEnable;

    GT_BOOL                                 bindToPolicer;
    GT_U32                                  policerIndex;

    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    CPSS_DXCH_TTI_QOS_MODE_TYPE_ENT         qosTrustMode;
    GT_U32                                  qosProfile;
    CPSS_DXCH_TTI_MODIFY_UP_ENT             modifyUpEnable;
    CPSS_DXCH_TTI_MODIFY_DSCP_ENT           modifyDscpEnable;
    GT_U32                                  up;
    GT_BOOL                                 remapDSCP;

    GT_BOOL                                 mirrorToIngressAnalyzerEnable;
    CPSS_NET_RX_CPU_CODE_ENT                userDefinedCpuCode;
    GT_BOOL                                 vntl2Echo;
    GT_BOOL                                 bridgeBypass;
    GT_BOOL                                 actionStop;

    GT_BOOL                                 activateCounter;
    GT_U32                                  counterIndex;

} CPSS_DXCH_TTI_ACTION_STC;

/*
 * Typedef: struct CPSS_DXCH_TTI_ACTION_2_STC
 *
 * Description:
 *          TTI TCAM rule action parameters (standard action).
 *
 * Fields:
 *
 *  Tunnel Terminate Action Fields
 *  ------------------------------
 *      tunnelTerminate       - whether the packet's tunnel header is removed;
 *                               must not be set for non-tunneled packets
 *      ttPassengerPacketType - passenger packet type (refer to CPSS_DXCH_TTI_PASSENGER_TYPE_ENT).
 *                              When <tunnelTerminate> is GT_FALSE but <MPLS Command> = POP<n>
 *                              or PUSH, this field must be assigned a value of MPLS.
 *                              Valid values:
 *                                  CPSS_DXCH_TTI_PASSENGER_IPV4V6_E
 *                                  CPSS_DXCH_TTI_PASSENGER_MPLS_E
 *                                  CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E
 *                                  CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E
 *      tsPassengerPacketType - Type of passenger packet for packet redirected
 *                              to Tunnel-Start.
 *                              Valid only when <Redirect Command>= Redirect to
 *                              egress interface and <TunnelStart> = Enabled.
 *                              When packet is not TT but <MPLS Command> = POP<n>
 *                              or PUSH, the <Tunnel Type> bit must be set to OTHER.
 *
 *  TTL
 *  ---
 *      copyTtlFromTunnelHeader   - If <GT_TRUE> and the packet is MPLS
 *                                  (<MPLS Command> = POP and <MPLS TTL> = 0, or
 *                                  <Tunnel Terminate>=Enabled),
 *                                  the TTL is copied from the last popped label.
 *                                  If <GT_TRUE> and the packet is X-over-IPv4
 *                                  tunnel-termination, and the passenger is IPv4/6,
 *                                  the TTL is copied from the IPv4 tunnel header
 *                                  rather than from passenger packet.
 *
 *  MPLS
 *  ----
 *      mplsCommand           - MPLS Action applied to the packet
 *      mplsTtl               - 0:     TTL is taken from the tunnel or inner MPLS label
 *                              1-255: TTL is set according to this field
 *                              Relevant for MPLS packets that have a match in
 *                              the TTI lookup, and with <MPLS Command>!=NOP
 *                                                   (APPLICABLE RANGES: 0..255)
 *  Forwarding Attributes
 *  ---------------------
 *      command               - forwarding command; valid values:
 *                                  CPSS_PACKET_CMD_FORWARD_E
 *                                  CPSS_PACKET_CMD_MIRROR_TO_CPU_E
 *                                  CPSS_PACKET_CMD_TRAP_TO_CPU_E
 *                                  CPSS_PACKET_CMD_DROP_HARD_E
 *                                  CPSS_PACKET_CMD_DROP_SOFT_E
 *                              for TRILL packets: If Command is TRAP, HARD-DROP, or
 *                              SOFT-DROP and the packet is TRILL Multi-Target instance 0,
 *                              the second instance is implicitly assigned HARD-DROP
 *      enableDecrementTtl    - for MPLS packets that are not tunnel terminated
 *                              <GT_TRUE>:  TTL is decremented by one
 *                              <GT_FALSE>: TTL is not decremented
 *                              for TRILL packets:
 *                              <GT_TRUE>:  TRILL header hop count is decremented by one
 *                              <GT_FALSE>: TRILL header hop count is not decremented
 *
 *
 *  Redirect Command and Data
 *  -------------------------
 *      redirectCommand        - where to redirect the packet
 *                               (refer to CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT)
 *      egressInterface        - egress interface to redirect packet. valid if <redirectCommand> is
 *                               REDIRECT_TO_EGRESS. The packet is forwarded as Unicast to a port or a trunk
 *                               For Lion2 B0 this is used as virtual source port when
 *                              <redirectCommand> is LOGICAL_PORT.
 *      arpPtr                 - pointer to the ARP entry for MPLS routed packets
 *                               Valid only when <Redirect Command> = Redirect to egress interface
 *                               and <TunnelStart> = Disabled
 *      tunnelStart            - <GT_TRUE>:  redirected to Tunnel Egress Interface
 *                               <GT_FALSE>: redirected to non-Tunnel Egress Interface
 *                               (supported for Ethernet-over-MPLS Tunnel Start only).
 *                               Relevant only when <Redirect Command> = Redirect to egress interface
 *      tunnelStartPtr         - pointer to the Tunnel Start entry. valid if <tunnelStart>
 *                               is GT_TRUE and <redirectCommand> is REDIRECT_TO_EGRESS
 *      routerLttPtr           - pointer to the Router Lookup Translation Table entry
 *                               if <redirectCommand> is REDIRECT_TO_ROUTER_LOOKUP
 *                               FEr#2018 - Router Lookup Translation Table (LTT) index can be written
 *                               only to column 0 of the LTT row.
 *                               NOTE:
 *                               - DxCh3 devices support index 0,1,2,3…(max IP TCAM row) only
 *                               - xCat devices support index 0,4,8,12…(max IP TCAM row * 4) only
 *                              (APPLICABLE DEVICES: xCat; Lion; Lion2; Lion3)
 *      vrfId                  - VRF-ID assigned to the packet if <redirectCommand> is VRF_ID_ASSIGN
 *                              (APPLICABLE DEVICES: xCat; Lion; Lion2; Lion3)
 *
 *
 *  Source ID assignment
 *  --------------------
 *      sourceIdSetEnable     - if set, then the <sourceId> is assign to the packet
 *      sourceId              - source ID assigned to the packet if <sourceIdSetEnable> is GT_TRUE
 *
 *
 *  VLAN Modification and Assignment
 *  --------------------------------
 *      tag0VlanCmd           - tag0 vlan command; valid options:
 *                                  CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E
 *                                  CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E
 *                                  CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E
 *                                  CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E
 *      tag0VlanId            - tag0 VLAN-ID assignment
 *      tag1VlanCmd           - tag1 vlan command; valid options:
 *                                  CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E
 *                                  CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E
 *                              Relevant if <mplsCommand> == CPSS_DXCH_TTI_MPLS_NOP_CMD_E
 *      tag1VlanId            - tag1 VLAN-ID assignment
 *                              Relevant if <mplsCommand> == CPSS_DXCH_TTI_MPLS_NOP_CMD_E
 *      tag0VlanPrecedence    - whether the VID assignment can be overridden by
 *                              subsequent VLAN assignment mechanism
 *                              (refer to CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)
 *      nestedVlanEnable      - enable/disable nested vlan; relevant only for
 *                              Ethernet packets
 *
 *  Bind to Policer
 *  ---------------
 *      bindToPolicerMeter    - If set, the packet is bound to the policer meter
 *                              specified in <Policer Index>
 *      bindToPolicer         - If set, the packet is bound to the policer/billing/IPFIX
 *                              counter specified in <Policer Index>
 *      policerIndex          - Traffic profile to be used if the <bindToPolicer> is set
 *
 *
 *  Qos Fields
 *  ----------
 *      qosPrecedence         - whether QoS profile can be overridden by subsequent pipeline
 *                              engines (refer to CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)
 *      keepPreviousQoS       - <GT_FALSE>: Assign QoS profile based on QoS fields
 *                              <GT_TRUE>:  Keep prior QoS Profile assignment
 *      trustUp               - <GT_FALSE>: Do not trust packet’s L2 QoS
 *                              <GT_TRUE>:  If packet is tunnel-terminated,
 *                              trust passenger packet L2 QoS. If packet is
 *                              not tunnel-terminated, trust packet L2 QoS
 *                              Relevant only if <keepPreviousQoS>=GT_FALSE.
 *      trustDscp             - <GT_FALSE>: Do not trust packet’s L3 QoS
 *                              <GT_TRUE>:  If packet is not tunnel-terminated,
 *                              trust packet’s outer, remaining, MPLS label’s EXP
 *                              Relevant only if <keepPreviousQoS>=GT_FALSE.
 *      trustExp              - <GT_FALSE>: Do not trust packet’s EXP
 *                              <GT_TRUE>:  If packet is tunnel-terminated, trust
 *                              passenger packet L3 QoS. If packet is not
 *                              tunnel-terminated, trust packet L3 QoS
 *                              Relevant only if <keepPreviousQoS>=GT_FALSE.
 *      qosProfile            - QoS profile to assign to the packet
 *                              Relevant only if <keepPreviousQoS>=GT_FALSE.
 *      modifyTag0Up          - modify tag0 UP (refer to CPSS_DXCH_TTI_MODIFY_UP_ENT)
 *      tag1UpCommand         - tag1 UP command (refer to CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT)
 *      modifyDscp            - modify DSCP mode (refer to CPSS_DXCH_TTI_MODIFY_DSCP_ENT)
 *      tag0Up                - tag0 UP assignment
 *      tag1Up                - tag1 UP assignment
 *      remapDSCP             - <GT_FALSE>: Packet’s DSCP is not remapped.
 *                              <GT_TRUE>:  Packet’s DSCP is remapped.
 *                              If <Tunnel Terminate> = Enabled, the field is
 *                              relevant only if passenger packet is IPv4/6.
 *                              If <Tunnel Terminate> = Disabled, the field is
 *                              relevant only if <Trust DSCP> = Enabled and the
 *                              packet IPv4 or IPv6
 *
 *  IPCL Lookup Configurations
 *  --------------------------
 *      pcl0OverrideConfigIndex   - Controls the index used for IPCL0 lookup
 *                                  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *      pcl0_1OverrideConfigIndex - Controls the index used for IPCL0-1 lookup
 *                                  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *      pcl1OverrideConfigIndex   - Controls the index used for IPCL1 lookup
 *                                  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *      iPclConfigIndex           - Pointer to IPCL configuration entry to be used
 *                                  when fetching IPCL parameter. This overrides
 *                                  the Port/VLAN based configuration entry selection.
 *                                  Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *
 *  Management and Debug
 *  --------------------
 *      mirrorToIngressAnalyzerEnable - If set, the packet is mirrored to the Ingress
 *                                      Analyzer port. in eArch devices this enables writing mirrorToIngressAnalyzerIndex.
 *      mirrorToIngressAnalyzerIndex - Enables mirroring the packet to an Ingress Analyzer interface.
 *                                     If the port configuration assigns a different analyzer.
 *                                     index, the higher index wins. (APPLICABLE DEVICES: Lion3) , (APPLICABLE RANGES: 0..7).
 *      userDefinedCpuCode    - user defined cpu code. relavant only if command is
 *                              trap or mirror
 *      bindToCentralCounter  - <GT_FALSE>: central counter binding is disabled.
 *                              <GT_TRUE>:  central counter binding is enabled,
 *      centralCounterIndex   - central counter bound to this entry
 *                              Relevant only when <bindToCentralCounter> == GT_TRUE
 *      vntl2Echo             - if set, the packet MAC SA/DA are switched on the transmitting
 *                              Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *      bridgeBypass          - if set, the packet isn't subject to any bridge mechanisms
 *      ingressPipeBypass     - If set, the ingress engines are all bypassed for this packet
 *      actionStop            - if set, the packet will not have any searches in the IPCL mechanism
 *      hashMaskIndex         - Determines which mask is used in the CRC based hash.
 *                              0: do not override hash mask index.
 *                              Other values: override the hash mask index value.
 *                             (APPLICABLE DEVICES: Lion; xCat2; Lion2; Lion3)
 *      modifyMacSa           - If set, the packet is marked for MAC SA modification
 *                              Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *                             (APPLICABLE DEVICES: Lion; xCat2; Lion2; Lion3)
 *      modifyMacDa           - If set, the packet is marked for MAC DA modification,
 *                              and the <ARP Index> specifies the new MAC DA.
 *                              Relevant only when <redirectCommand> == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *                             (APPLICABLE DEVICES: Lion; xCat2; Lion2; Lion3)
 *      ResetSrcPortGroupId   - If set, the source port group ID of the packet is set to the current port group.
 *                              Relevant only when <redirectCommand> != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E
 *                             (APPLICABLE DEVICES: Lion; Lion2; Lion3 for multi port groups devices only)
 *      multiPortGroupTtiEnable    - If set, the packet is forwarded to a ring port for another TTI lookup.
 *                                   Relevant only when <redirectCommand> == CPSS_DXCH_TTI_NO_REDIRECT_E
 *                                  (APPLICABLE DEVICES: Lion; Lion2; Lion3 for multi port groups devices only)
 *
 *      sourceEPortAssignmentEnable - Enable assignment of Source ePort number from this entry
 *                                    GT_FALSE- Disabled
 *                                    GT_TRUE - Enabled - New source ePort number
 *                                    is taken from TTI Action Entry<Source ePort>.
 *                                    (APPLICABLE DEVICES: Lion3)
 *
 *      sourceEPort                 - Source ePort number assigned by TTI entry
 *                                    when <Source ePort Assignment Enable> = Enabled
 *                                    NOTE: Overrides any previous assignment of
 *                                          source ePort number.
 *                                    (APPLICABLE DEVICES: Lion3)
 *  OAM related fields
 *  --------------------
 *
 *      oamTimeStampEnable          - Indicates that a timestamp should be inserted into the packet:
 *                                    GT_TRUE  - enable timestamp insertion.
 *                                    GT_FALSE - disable timestamp insertion.
 *      oamOffsetIndex              - When <Timestamp Enable> is enabled, indicates the offset
 *                                    index for the timestamp offset table.
 *                                    (APPLICABLE RANGES: 0..15).
 *      oamProcessEnable            - Binds the packet to an entry in the OAM Table.
 *                                    GT_TRUE  - enable OAM packet processing.
 *                                    GT_FALSE - disable OAM packet processing.
 *      oamProfile                  - Determines the set of UDBs where the key attributes
 *                                    (opcode, MEG level, RDI, MEG level) of the OAM message
 *                                    are taken from.  (APPLICABLE RANGES: 0..1).
 *
 */
typedef struct
{
    GT_BOOL                                 tunnelTerminate;
    CPSS_DXCH_TTI_PASSENGER_TYPE_ENT        ttPassengerPacketType;
    CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT     tsPassengerPacketType;

    GT_BOOL                                 copyTtlFromTunnelHeader;

    CPSS_DXCH_TTI_MPLS_CMD_ENT              mplsCommand;
    GT_U32                                  mplsTtl;
    GT_BOOL                                 enableDecrementTtl;

    CPSS_PACKET_CMD_ENT                     command;

    CPSS_DXCH_TTI_REDIRECT_COMMAND_ENT      redirectCommand;
    CPSS_INTERFACE_INFO_STC                 egressInterface;
    GT_U32                                  arpPtr;
    GT_BOOL                                 tunnelStart;
    GT_U32                                  tunnelStartPtr;
    GT_U32                                  routerLttPtr;
    GT_U32                                  vrfId;

    GT_BOOL                                 sourceIdSetEnable;
    GT_U32                                  sourceId;

    CPSS_DXCH_TTI_VLAN_COMMAND_ENT          tag0VlanCmd;
    GT_U16                                  tag0VlanId;
    CPSS_DXCH_TTI_VLAN_COMMAND_ENT          tag1VlanCmd;
    GT_U16                                  tag1VlanId;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT tag0VlanPrecedence;
    GT_BOOL                                 nestedVlanEnable;

    GT_BOOL                                 bindToPolicerMeter;
    GT_BOOL                                 bindToPolicer;
    GT_U32                                  policerIndex;

    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    GT_BOOL                                 keepPreviousQoS;
    GT_BOOL                                 trustUp;
    GT_BOOL                                 trustDscp;
    GT_BOOL                                 trustExp;
    GT_U32                                  qosProfile;
    CPSS_DXCH_TTI_MODIFY_UP_ENT             modifyTag0Up;
    CPSS_DXCH_TTI_TAG1_UP_COMMAND_ENT       tag1UpCommand;
    CPSS_DXCH_TTI_MODIFY_DSCP_ENT           modifyDscp;
    GT_U32                                  tag0Up;
    GT_U32                                  tag1Up;
    GT_BOOL                                 remapDSCP;

    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT   pcl0OverrideConfigIndex;
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT   pcl0_1OverrideConfigIndex;
    CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT   pcl1OverrideConfigIndex;
    GT_U32                                  iPclConfigIndex;

    GT_BOOL                                 mirrorToIngressAnalyzerEnable;
    GT_U32                                  mirrorToIngressAnalyzerIndex;
    CPSS_NET_RX_CPU_CODE_ENT                userDefinedCpuCode;
    GT_BOOL                                 bindToCentralCounter;
    GT_U32                                  centralCounterIndex;
    GT_BOOL                                 vntl2Echo;
    GT_BOOL                                 bridgeBypass;
    GT_BOOL                                 ingressPipeBypass;
    GT_BOOL                                 actionStop;
    GT_U32                                  hashMaskIndex;
    GT_BOOL                                 modifyMacSa;
    GT_BOOL                                 modifyMacDa;
    GT_BOOL                                 ResetSrcPortGroupId;
    GT_BOOL                                 multiPortGroupTtiEnable;

    GT_BOOL                                 sourceEPortAssignmentEnable;
    GT_PORT_NUM                             sourceEPort;

    GT_BOOL                                 oamTimeStampEnable;
    GT_U32                                  oamOffsetIndex;
    GT_BOOL                                 oamProcessEnable;
    GT_U32                                  oamProfile;
} CPSS_DXCH_TTI_ACTION_2_STC;

/*
 * typedef: union CPSS_DXCH_TTI_ACTION_UNT
 *
 * Description:
 *      Union for TTI action.
 *
 * Fields:
 *
 *  type1      - type1 action (APPLICABLE DEVICES: DxCh3)
 *  type2      - type2 action (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *
 */
typedef union
{
    CPSS_DXCH_TTI_ACTION_STC    type1;
    CPSS_DXCH_TTI_ACTION_2_STC  type2;

} CPSS_DXCH_TTI_ACTION_UNT;

/*
 * typedef: enum CPSS_DXCH_TTI_ACTION_TYPE_ENT
 *
 * Description:
 *      TTI action type enum
 *
 * Enumerations:
 *
 *      CPSS_DXCH_TTI_ACTION_TYPE1_ENT    - type1 action (APPLICABLE DEVICES: DxCh3)
 *      CPSS_DXCH_TTI_ACTION_TYPE2_ENT    - type2 action (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *
 */
typedef enum
{
    CPSS_DXCH_TTI_ACTION_TYPE1_ENT   = 0,
    CPSS_DXCH_TTI_ACTION_TYPE2_ENT   = 1

} CPSS_DXCH_TTI_ACTION_TYPE_ENT;


/*
 * typedef: enum CPSS_DXCH_TTI_EXCEPTION_ENT
 *
 * Description: Enumeration for tunnel termination exceptions.
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E -
 *      IPv4 tunnel termination header error.
 *      IPv4 Tunnel Termination Header Error exception is detected if ANY of
 *      the following criteria are NOT met:
 *        - IPv4 header <Checksum> is correct
 *        - IPv4 header <Version> = 4
 *        - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
 *        - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
 *        - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
 *        - IPv4 header <SIP> != IPv4 header <DIP>
 *
 *  CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E -
 *      IPv4 tunnel termination option/fragment error.
 *      An IPv4 Tunnel Termination Option/Fragment error occurs if the packet
 *      has either IPv4 Options or is fragmented.
 *        - IPv4 Options are detected if the IPv4 header <IP Header Len> > 5.
 *        - The packet is considered fragmented if either the IPv4 header flag
 *          <More Fragments> is set, or if the IPv4 header <Fragment offset> > 0.
 *          A tunnel-terminated fragmented packet must be reassembled prior
 *          to further processing of the passenger packet.
 *
 *  CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E -
 *      IPv4 tunnel termination unsupported GRE header error.
 *      An IPv4 Tunnel Termination GRE Header error occurs if the IPv4 header
 *      <IP Protocol> = 47 (GRE) and the 16 most significant bits of the GRE
 *      header are not all 0.
 *
 *  CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E -
 *      An MPLS TTL error is detected if the MPLS TTI rule has a match and
 *      any of the MPLS labels popped by the TTI have reached the value 0.
 *      (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E -
 *      MPLS unsupported error occurs if one of the following:
 *        - The <Tunnel Terminate> field is enabled for an MPLS packet, and the
 *          packet has more than three MPLS labels
 *        - The <MPLS Command> cannot be assigned to the packet, for one of
 *          the following reasons:
 *            - <MPLS Command> requires to Pop more labels than the packet has
 *              (this check is performed only when <Tunnel Terminate> is disabled).
 *            - <MPLS Command> != NOP/Push and packet is not MPLS.
 *      (APPLICABLE DEVICES: xCat; Lion; xCat2; Lion2; Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E -
 *      TRILL IS IS adjacency exception is detected if there is TCAM miss for
 *      unicast TRILL packet, indicating the packet was received from an invalid adjacency;
 *      the global configurable <TRILL IS IS Adjacency exception Cmd> is applied, and the
 *      respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 0.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E -
 *      TRILL tree adjacency exception is detected if there is TCAM miss for
 *      multi-target TRILL packet, indicating the packet was received from an invalid adjacency;
 *      the global configurable <TRILL Tree Adjacency exception Cmd> is applied, and the
 *      respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 1.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E -
 *      TRILL bad version exception is detected if the packet TRILL header version is greater
 *      than the global configurable <Max TRILL version>, the global configurable
 *      <TRILL bad Version exception Cmd> is applied, and the respective exception counter is icremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 2.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E -
 *      TRILL hop count exception is detected if the packet TRILL header Hop Count is ZERO,
 *      the global configurable <TRILL Hopcount is Zero exception Cmd> is assigned to the packet,
 *      and its respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 3.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E -
 *      TRILL options exception is detected if the packet TRILL header includes options
 *     (.i.e., the TRILL header Op-length field is greater than zero), the global configurable
 *      <TRILL Options exception Cmd> is assigned to the packet, and its respective exception
 *      counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 4.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E -
 *      TRILL critical hop-by-hop exception is detected if the packet TRILL header
 *      includes options and the CHBH bit is set, the global configurable
 *      <TRILL CHBH exception Cmd> is assigned to the packet, and its respective exception
 *      counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 5.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E -
 *      The TRILL standard specifies that all TRILL packets on a link, if tagged, must use
 *      the VLAN-ID as specified by the designated RBridge.
 *      TRILL bad outer VID0 exception is detected if the packet TRILL outer tag VLAN-ID
 *      (Tag0 VLAN-ID) does not equal the global configurable <TRILL Outer Tag0 VID>,
 *      the global configurable <TRILL Bad Outer VID exception Cmd> is assigned to the packet,
 *      and its respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 6.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E -
 *      TRILL transit Unicast traffic, for which this RBridge is the nexthop, must have
 *      the MAC DA of this RBridge. Otherwise, the packet should be dropped by this RBridge.
 *      This is a normal occurrence if the link is attached to a bridged LAN and there are
 *      more than two peer RBridges on this link.
 *      It is required that the MAC DA’s of this RBridge are configured in the MAC2ME table.
 *      TRILL UC not to me exception is detected if the packet is Unicast TRILL and its
 *      outer MAC DA does not match one of the of the MAC addresses configured in the MAC2ME table,
 *      the global configurable <TRILL UC Not MAC2ME exception Cmd> is assigned to the packet,
 *      and its respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 7.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E -
 *      TRILL MC with bad outer DA exception is detected if the TRILL outer MAC DA
 *      is Multicast, and the TRILL header M-bit is clear, the global configurable
 *      <TRILL MC M-bit exception Cmd> is assigned to the packet, and its respective
 *      exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 8.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E -
 *      TRILL UC with bad outer DA exception is detected if the TRILL outer MAC DA is UC,
 *      and the TRILL header M-bit is set, the global configurable <TRILL UC bad outer exception Cmd>
 *      is assigned to the packet, and its respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 9.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E -
 *      TRILL outer UC inner MC exception is detected if the TRILL outer MAC DA is UC,
 *      and the inner DA is MC, the global configurable <TRILL outer UC inner MC exception Cmd>
 *      is assigned to the packet, and its respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 10.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E -
 *      TRILL MC with bad outer DA is detected if the TRILL header M-bit is set, and the outer
 *      MAC DA is not the global configurable <All-RBridges Multicast address>, the global configurable
 *      <TRILL Bad MC DA exception Cmd> is assigned to the packet, and its respective exception
 *      counter is incremented.
 *      If the command is TRAP or MIRROR, the packet CPU code is TRILL_CPU_Code_Base + 11.
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E -
 *      TRILL invalid I-RBID exception is detected if the I-RBID lookup results in
 *      an entry <port> == 0xFFFFF; the global configurable <Trill Invalid I-RBID exception Cmd>
 *      is applied, and the respective exception counter is incremented.
 *       If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +12
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E -
 *      TRILL invalid E-RBID exception is detected if the E-RBID lookup results in
 *      an entry <port> == 0xFFFFF; the global configurable <Trill Invalid E-RBID exception Cmd>
 *      is applied, and the respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +13
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E -
 *      TRILL UC Critical Ingress to Egress exception is detected if the E-RBID lookup
 *      results in an entry <port> == 0xFFFFE and TRILL.M == 0 (i.e. the packet is
 *      TRILL Unicast-to-ME), and the TRILL.OptionData.CItE == 1 (i.e. packet has the Critical
 *      Ingress-to-Egress bit set); the global configurable <TRILL UC CItE Option exception Cmd>
 *      is applied, and the respective exception counter is incremented.
 *      If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +14
 *      (APPLICABLE DEVICES: Lion3)
 *
 *  CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E -
 *      TRILL MC Critical Ingress to Egress exception is detected if the TRILL.M == 0
 *      (i.e. the packet is multi-destination TRILL), and the TRILL.OptionData.CItE == 1
 *      (i.e. packet has the Critical Ingress-to-Egress bit set); the global configurable
 *      <TRILL Multi-target CItE Option exception Cmd> is applied, and the respective exception
 *      counter is icremented.
 *      If the command is TRAP or MIRROR, the CPU code assignment is TRILL_CPU_Code_Base +15.
 *      (APPLICABLE DEVICES: Lion3)
 */
typedef enum
{
    CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E       = 0,
    CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E  = 1,
    CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E    = 2,
    CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E  = 3,
    CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E  = 4,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E = 5,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E = 6,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E = 7,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E = 8,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E = 9,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E = 10,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E = 11,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E = 12,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E = 13,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E = 14,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E = 15,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E = 16,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_I_RBID_E = 17,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_INVALID_E_RBID_E = 18,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_CLTE_OPTION_E = 19,
    CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_CLTE_OPTION_E = 20

} CPSS_DXCH_TTI_EXCEPTION_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTTiTypesh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChTtiTypes.h,v $
 * Revision 1.1  2015/02/13 11:33:00  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
