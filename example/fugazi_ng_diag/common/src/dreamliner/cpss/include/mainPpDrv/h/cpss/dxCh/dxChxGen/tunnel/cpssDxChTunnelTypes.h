/* $Id: cpssDxChTunnelTypes.h,v 1.1 2015/02/13 11:33:01 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h,v $
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
* cpssDxChTunnelTypes.h
*
* DESCRIPTION:
*       CPSS definitions for tunnel.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __cpssDxChTunnelTypesh
#define __cpssDxChTunnelTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/tunnel/cpssGenTunnelTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>


/*
 * typedef: enum CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT
 *
 * Description: Enumeration of tunnel start ethernet over x vlan tag mode.
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E -
 *      Vlan tag mode is set according to the tag bit in the vlan entry.
 *
 *  CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E -
 *      Vlan tag mode is set according to the <EthernetOverXPassangerTagged> entry
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E   = 0,
    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E  = 1

} CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_TUNNEL_QOS_MODE_ENT
 *
 * Description: Enumeration of QoS modes.
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TUNNEL_QOS_KEEP_PREVIOUS_E  - keep QoS profile assignment from
 *                                          previous mechanisms
 *  CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_UP_E  - trust passenger protocol 802.1p user
 *                                          priority; relevant to Ethernet-over-MPLS
 *                                          packets only
 *  CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_DSCP_E    - trust passenger protocol IP DSCP
 *  CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_UP_DSCP_E - trust passenger protocol 802.1p user
 *                                              priority and IP DSCP
 *  CPSS_DXCH_TUNNEL_QOS_TRUST_MPLS_EXP_E - trust outer label MPLS EXP in the
 *                                          tunneling header
 *  CPSS_DXCH_TUNNEL_QOS_UNTRUST_PKT_E    - untrust packet QoS, assign QoS profile
 *                                          according to tunnel termination entry
 *                                          QoS profile assignment
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_QOS_KEEP_PREVIOUS_E        = 0,
    CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_UP_E        = 1,
    CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_DSCP_E      = 2,
    CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_UP_DSCP_E   = 3,
    CPSS_DXCH_TUNNEL_QOS_TRUST_MPLS_EXP_E       = 4,
    CPSS_DXCH_TUNNEL_QOS_UNTRUST_PKT_E          = 5

} CPSS_DXCH_TUNNEL_QOS_MODE_ENT;


/*
 * typedef: enum CPSS_DXCH_TUNNEL_TERM_EXCEPTION_UNT
 *
 * Description: Enumeration for tunnel termination exceptions.
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TUNNEL_TERM_IPV4_HEADER_ERROR_E -
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
 *  CPSS_DXCH_TUNNEL_TERM_IPV4_OPTION_FRAG_ERROR_E -
 *      IPv4 tunnel termination option/fragment error.
 *      An IPv4 Tunnel Termination Option/Fragment error occurs if the packet
 *      has either IPv4 Options or is fragmented.
 *        - IPv4 Options are detected if the IPv4 header <IP Header Len> > 5.
 *        - The packet is considered fragmented if either the IPv4 header flag
 *          <More Fragments> is set, or if the IPv4 header <Fragment offset> > 0.
 *          A tunnel-terminated fragmented packet must be reassembled prior
 *          to further processing of the passenger packet.
 *
 *  CPSS_DXCH_TUNNEL_TERM_IPV4_UNSUP_GRE_ERROR_E -
 *      IPv4 tunnel termination unsupported GRE header error.
 *      An IPv4 Tunnel Termination GRE Header error occurs if the IPv4 header
 *      <IP Protocol> = 47 (GRE) and the 16 most significant bits of the GRE
 *      header are not all 0.
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_TERM_IPV4_HEADER_ERROR_E       = 0,
    CPSS_DXCH_TUNNEL_TERM_IPV4_OPTION_FRAG_ERROR_E  = 1,
    CPSS_DXCH_TUNNEL_TERM_IPV4_UNSUP_GRE_ERROR_E    = 2

} CPSS_DXCH_TUNNEL_TERM_EXCEPTION_ENT;

/*
 * typedef: enum CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT
 *
 * Description: Enumeration of tunnel start MPLS LSR TTL operation.
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E -
 *      Set TTL to incoming TTL Assignment
 *
 *  CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E -
 *      Set TTL to the TTL of the swapped label
 *
 *  CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E -
 *      Set TTL to the TTL of the swapped label - 1
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E             = 0,
    CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E               = 1,
    CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E     = 2

} CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT
 *
 * Description: Enumeration of tunnel start entry QoS mark modes.
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E -
 *      Mark based on the explicit QoS fields in Tunnel Start entry.
 *
 *  CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E -
 *      Mark based on the packet QoS Profile assignment. The QoS Profile table
 *      entry provides the tunnel header QoS fields.
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E                = 0,
    CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E   = 1

} CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT
 *
 * Description:
 *      Tunnel passenger protocol types.
 *
 * Enumerations:
 *      CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E     - ethernet passenger type
 *      CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E        - other passenger type
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E           = 0,
    CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E              = 1

} CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT
 *
 * Description: Enumeration of tunnel start MIM I-SID assign mode
 *
 * Enumerations:
 *
 *  CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_TABLE_E -
 *      I-SID assigned from the tunnel start entry
 *
 *  CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_HA_EGRESS_TABLE_E -
 *      I-SID assigned from the HA egress attribute table <I-SID>
 *
 */
typedef enum
{
    CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_TABLE_E          = 0,
    CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_HA_EGRESS_TABLE_E   = 1

} CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT;


/*
 * typedef: struct CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC
 *
 * Description: Struct of configuration for X-over-IPv4 tunnel start.
 *
 * Fields:
 *
 *  tagEnable         - If set, the packet is sent with VLAN tag
 *  vlanId            - Relevant only if entry <tagEnable> is set. This
 *                      field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
 *  upMarkMode        - Relevant only if entry <tagEnable> is set.
 *                      Options:
 *                      - Set user priority according to entry <802.1p UP>.
 *                      - Set user priority according to packet assigned QoS attributes
 *  up                - The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
 *                      Relevant only if entry <tagEnable> is set and entry
 *                      <upMarkMode> is set according to entry <802.1p UP>.
 *  dscpMarkMode      - Options:
 *                      - Set DSCP according to entry <DSCP>.
 *                      - Set DSCP according to packet QoS Profile assignment.
 *  dscp              - The DSCP in the IPv4 tunnel header (APPLICABLE RANGES: 0..63)
 *                      Relevant only if entry <dscpMarkMode> is according
 *                      to entry <DSCP>.
 *  macDa             - Tunnel next hop MAC DA
 *  dontFragmentFlag  - Don't fragment flag in the tunnel IPv4 header
 *  ttl               - tunnel IPv4 header TTL (APPLICABLE RANGES: 0..255)
 *                          0:     use passenger packet TTL
 *                          1-255: use this field for header TTL
 *  autoTunnel        - Relevant for IPv6-over-IPv4 and IPv6-over-GRE-IPv4 only,
 *                      if GT_TRUE, the IPv4 header DIP is derived from IPv6
 *                      passenger packet and destIp field is ignored
 *  autoTunnelOffset  - Relevant for IPv6-over-IPv4 or IPv6-over-GRE-IPv4 only (APPLICABLE RANGES: 0..12)
 *                      If <autoTunnel> is GT_TRUE, this field is the offset
 *                      of IPv4 destination address inside IPv6 destination
 *                      address. Offset is measured in bytes between LSBs of
 *                      the addresses:
 *                          1. IPv4 compatible & ISATAP = 0
 *                          2. 6to4 = 10 (decimal)
 *  destIp            - Tunnel destination IP
 *  srcIp             - Tunnel source IP
 *  cfi               - CFI bit assigned for TS packets (DxCh3 and above.)
 *                        0: Canonical MAC
 *                        1: non-Canonical MAC
 */
typedef struct
{
    GT_BOOL         tagEnable;
    GT_U16          vlanId;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    upMarkMode;
    GT_U32          up;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    dscpMarkMode;
    GT_U32          dscp;
    GT_ETHERADDR    macDa;
    GT_BOOL         dontFragmentFlag;
    GT_U32          ttl;
    GT_BOOL         autoTunnel;
    GT_U32          autoTunnelOffset;
    GT_IPADDR       destIp;
    GT_IPADDR       srcIp;
    GT_U32          cfi;

} CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC;


/*
 * typedef: struct CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC
 *
 * Description: Struct of configuration for X-over-MPLS tunnel start
 *
 * Fields:
 *
 *  tagEnable         - If set, the packet is sent with VLAN tag
 *  vlanId            - Relevant only if entry <tagEnable> is set. This
 *                      field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
 *  upMarkMode        - Relevant only if entry <tagEnable> is set.
 *                      Options:
 *                      - Set user priority according to entry <802.1p UP>.
 *                      - Set user priority according to packet assigned QoS attributes
 *  up                - The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
 *                      Relevant only if entry <tagEnable> is set and entry
 *                      <upMarkMode> is set according to entry <802.1p UP>.
 *  macDa             - Tunnel next hop MAC DA
 *  numLabels         - number of MPLS labels
 *                          DxCh2,DxCh3: (APPLICABLE RANGES: 1..2)
 *                          xCat and above: (APPLICABLE RANGES: 1..3)
 *  ttl               - tunnel IPv4 header TTL (APPLICABLE RANGES: 0..255)
 *                          0:     use passenger packet TTL
 *                          1-255: use this field for header TTL
 *  ttlMode           - MPLS LSR TTL operations;
 *                      Relevant when <ttl> = 0
 *                      (DxChXcat and above.)
 *  label1            - MPLS label 1; (APPLICABLE RANGES: 0..1048575) if <numLabels> > 1 then this
 *                      is the inner label.
 *  exp1MarkMode      - Options:
 *                      - Set EXP1 according to entry <exp1>.
 *                      - Set EXP1 according to packet QoS Profile assignment
 *                        to EXP mapping table.
 *  exp1              - The EXP1 value (APPLICABLE RANGES: 0..7)
 *                      Relevant if entry <exp1MarkMode> is set according to
 *                      entry <EXP1>.
 *  label2            - MPLS label 2; (APPLICABLE RANGES: 0..1048575) Relevant only when number of
 *                      labels is 2 or more; if <numLabels> = 2 then this is the outer
 *                      label, immediately following the MPLS Ether Type.
 *  exp2MarkMode      - Relevant only when number of labels is 2;
 *                      Options:
 *                      - Set EXP2 according to entry <exp2>.
 *                      - Set EXP2 according to packet QoS Profile assignment
 *                        to EXP mapping table.
 *  exp2              - The EXP2 value (APPLICABLE RANGES: 0..7)
 *                      Relevant only when number of labels is 2 and if entry
 *                      <exp2MarkMode> is set to according to entry <EXP2>.
 *  label3            - MPLS label 3; (APPLICABLE RANGES: 0..1048575) Relevant only when number of
 *                      labels is 3; if <numLabels> = 3 then this is the outer
 *                      label, immediately following the MPLS Ether Type.
 *                      (DxChXcat and above.)
 *  exp3MarkMode      - Relevant only when number of labels is 3;
 *                      Options:
 *                      - Set EXP3 according to entry <exp3>.
 *                      - Set EXP3 according to packet QoS Profile assignment
 *                        to EXP mapping table.
 *                      (DxChXcat and above.)
 *  exp3              - The EXP3 value (APPLICABLE RANGES: 0..7)
 *                      Relevant only when number of labels is 3 and if entry
 *                      <exp3MarkMode> is set to according to entry <EXP3>.
 *                      (DxChXcat and above.)
 *  retainCRC         - Relevant for Ethernet-over-MPLS. If GT_TRUE, retain
 *                      the passenger packet CRC and add additional 4-byte CRC
 *                      based on the tunneling header packet.
 *  setSBit           - <GT_FALSE>: S-bit is cleared
 *                      <GT_TRUE>:  S-bit is set on Label1
 *                      If <Number of MPLS Labels> is two or three, Label1 is
 *                      the inner label. This flag is typically enabled at
 *                      an MPLS LER, and disabled at an LSR.
 *                      (DxChXcat and above.)
 *  cfi               - CFI bit assigned for TS packets (DxCh3 and above.)
 *                        0: Canonical MAC
 *                        1: non-Canonical MAC
 */
typedef struct
{
    GT_BOOL         tagEnable;
    GT_U16          vlanId;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    upMarkMode;
    GT_U32          up;
    GT_ETHERADDR    macDa;
    GT_U32          numLabels;
    GT_U32          ttl;
    CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT         ttlMode;
    GT_U32          label1;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    exp1MarkMode;
    GT_U32          exp1;
    GT_U32          label2;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    exp2MarkMode;
    GT_U32          exp2;
    GT_U32          label3;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    exp3MarkMode;
    GT_U32          exp3;
    GT_BOOL         retainCRC;
    GT_BOOL         setSBit;
    GT_U32          cfi;

} CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC;

/*
 * typedef: struct CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC
 *
 * Description: Struct of configuration for MacInMac tunnel start entry
 *
 * APPLICABLE DEVICES:
 *        xCat; Lion; xCat2; Lion2; Lion3.
 *
 * NOT APPLICABLE DEVICES:
 *        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
 *
 * Fields:
 *
 *  tagEnable         - GT_TRUE: the packet is sent with VLAN tag.
 *                      GT_FALSE: the packet is send without VLAN tag.
 *  vlanId            - Relevant only if entry <tagEnable> is set. This
 *                      field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
 *  upMarkMode        - Relevant only if entry <tagEnable> is set.
 *                      Options:
 *                      - Set user priority according to entry <802.1p UP>.
 *                      - Set user priority according to up assigned by previous
 *                        ingress engines.
 *  up                - The tag 802.1p user priority (APPLICABLE RANGES: 0..7).
 *                      Relevant only if entry <tagEnable> is set and entry.
 *                      <upMarkMode> is set according to entry <802.1p UP>.
 *  macDa             - Tunnel header MAC DA.
 *  retainCrc         - Relevant for Ethernet passenger packet.
 *                          GT_TRUE: retain passenger packet CRC and add
 *                          additional 4-byte CRC based on the tunneling header
 *                          packet.
 *                          GT_FALSE: remove passenger packet CRC.
 *  iSid              - MacInMac inner service identifier (24 bits).
 *  iUp               - inner user priority field (PCP) in the 802.1ah iTag.
 *                      (APPLICABLE RANGES: 0..7).
 *  iUpMarkMode       - Marking mode to determine assignment of iUp. Options:
 *                      - Set iUp according to entry (iUp).
 *                      - Set iUp according to iUp assigned by previous ingress
 *                        engines.
 *  iDp               - drop precedence in the iTag (APPLICABLE RANGES: 0..1).
 *  iDpMarkMode       - I-DEI (drop precedence) mark mode. Options:
 *                      - Set I-DEI according to entry <iDei>
 *                      - Set I-DEI according to DP assigned by previous ingress
 *                        engines, and according to DP to CFI mapping table
 *  iTagReserved      - The reserved bits to be set in the iTag (APPLICABLE RANGES: 0..15).
 *                      Bits 1-2: Res1
 *                      Bit    3: Res2
 *                      Bit    4: UCA (Use Customer Address)
 *  iSidAssignMode    - tunnel start MIM I-SID assign mode
 *                      (APPLICABLE DEVICES: Lion3)
 *
 */
typedef struct
{
    GT_BOOL                                         tagEnable;
    GT_U16                                          vlanId;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT        upMarkMode;
    GT_U32                                          up;
    GT_ETHERADDR                                    macDa;
    GT_BOOL                                         retainCrc;
    GT_U32                                          iSid;
    GT_U32                                          iUp;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT        iUpMarkMode;
    GT_U32                                          iDp;
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT        iDpMarkMode;
    GT_U32                                          iTagReserved;

    CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT   iSidAssignMode;

} CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC;

/*
 * typedef: struct CPSS_DXCH_TUNNEL_START_TRILL_HEADER_STC
 *
 * Description: configuration struct for TRILL header
 *
 * APPLICABLE DEVICES:
 *        Lion3.
 *
 * NOT APPLICABLE DEVICES:
 *        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
 *
 * Fields:
 *
 *  version           - version (2 bits)
 *  mBit              - Multi-destination bit
 *                      0: unicat packet
 *                      1: multicast packet
 *  opLength          - option length (5 bits)
 *                      NOTE: Applicable range 0
 *  hopCount          - hop count (6 bits)
 *  eRbid             - when mBit=0, this is the Egress RBridge
 *                      when mBit=1, this is the TREE-ID (16 bits)
 *  iRbid             - Ingress RBridge (16 bits)
 */
typedef struct
{
    GT_U32                                          version;
    GT_BOOL                                         mBit;
    GT_U32                                          opLength;
    GT_U32                                          hopCount;
    GT_U32                                          eRbid;
    GT_U32                                          iRbid;
} CPSS_DXCH_TUNNEL_START_TRILL_HEADER_STC;

/*
 * typedef: struct CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC
 *
 * Description: Struct of configuration for TRILL tunnel start
 *
 * APPLICABLE DEVICES:
 *        Lion3.
 *
 * NOT APPLICABLE DEVICES:
 *        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
 *
 * Fields:
 *
 *  upMarkMode        - Relevant only if entry <tagEnable> is set.
 *                      Options:
 *                      - Set user priority according to entry <802.1p UP>.
 *                      - Set user priority according to packet assigned QoS attributes
 *  up                - The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
 *                      Relevant only if entry <tagEnable> is set and entry
 *                      <upMarkMode> is set according to entry <802.1p UP>.
 *  tagEnable         - If set, the packet is sent with VLAN tag
 *  vlanId            - Relevant only if entry <tagEnable> is set. This
 *                      field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
 *  nextHopMacDa      - Tunnel header next hop MAC DA
 *  trillHeader       - TRILL header, relevant only if incoming packet is native Ethernet
 */
typedef struct
{
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT    upMarkMode;
    GT_U32                                      up;
    GT_BOOL                                     tagEnable;
    GT_U16                                      vlanId;
    GT_ETHERADDR                                nextHopMacDa;
    CPSS_DXCH_TUNNEL_START_TRILL_HEADER_STC     trillHeader;
} CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC;

/*
 * typedef: union CPSS_DXCH_TUNNEL_START_CONFIG_UNT
 *
 * Description: Union for configuration for tunnel start
 *
 * Fields:
 *
 *  ipv4Cfg           - configuration for X-over-IPv4 tunnel start
 *  mplsCfg           - configuration for X-over-MPLS tunnel start
 *  mimCfg            - configuration for MIM tunnel start
 *  trillCfg          - configuration for TRILL tunnel start
 *
 */
typedef union
{
    CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC  ipv4Cfg;
    CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC  mplsCfg;
    CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC   mimCfg;
    CPSS_DXCH_TUNNEL_START_TRILL_CONFIG_STC trillCfg;

} CPSS_DXCH_TUNNEL_START_CONFIG_UNT;


/*
 * typedef: struct CPSS_DXCH_TUNNEL_TERM_IPV4_CONFIG_STC
 *
 * Description: Struct for configuration for ipv4 tunnel termination.
 *
 * Fields:
 *
 *  srcPortTrunk  - Source port or Source trunk-ID
 *                  if port, range (APPLICABLE RANGES: 0..63)
 *                  if trunk, range (APPLICABLE RANGES: 0..127)
 *  srcIsTrunk    - whether source is port or trunk (APPLICABLE RANGES: 0..1)
 *                  0: Source is not a trunk (i.e. it is port)
 *                  1: Source is a trunk
 *  srcDev        - Source device (APPLICABLE RANGES: 0..31)
 *  vid           - packet VID assignment (APPLICABLE RANGES: 0..4095)
 *  macDa         - MAC DA of the tunnel header
 *  srcIp         - tunnel source IP
 *  destIp        - tunnel destination IP
 *
 */
typedef struct
{
    GT_U32                  srcPortTrunk;
    GT_U32                  srcIsTrunk;
    GT_U8                   srcDev;
    GT_U16                  vid;
    GT_ETHERADDR            macDa;
    GT_IPADDR               srcIp;
    GT_IPADDR               destIp;

} CPSS_DXCH_TUNNEL_TERM_IPV4_CONFIG_STC;


/*
 * typedef: struct CPSS_DXCH_TUNNEL_TERM_MPLS_CONFIG_STC
 *
 * Description: Struct for configuration for MPLS tunnel termination.
 *
 * Fields:
 *
 *  srcPortTrunk  - Source port or Source trunk-ID
 *                  if port, range (APPLICABLE RANGES: 0..63)
 *                  if trunk, range (APPLICABLE RANGES: 0..127)
 *  srcIsTrunk    - whether source is port or trunk (APPLICABLE RANGES: 0..1)
 *                  0: Source is not a trunk (i.e. it is port)
 *                  1: Source is a trunk
 *  srcDev        - Source device (APPLICABLE RANGES: 0..31)
 *  vid           - packet VID assignment (APPLICABLE RANGES: 0..4095)
 *  macDa         - MAC DA of the tunnel header
 *  label1        - MPLS label 1; in case packet arrives with 2 MPLS
 *                  labels, this is the inner label (APPLICABLE RANGES: 0..1048575)
 *  sBit1         - stop bit for MPLS label 1 (APPLICABLE RANGES: 0..1)
 *  exp1          - EXP of MPLS label 1 (APPLICABLE RANGES: 0..7)
 *  label2        - MPLS label 2; this is the outer label, immediately
 *                  following the MPLS Ether Type (APPLICABLE RANGES: 0..1048575)
 *  sBit2         - stop bit for MPLS label 2 (APPLICABLE RANGES: 0..1)
 *  exp2          - EXP of MPLS label 2 (APPLICABLE RANGES: 0..7)
 *
 * Comments:
 *      MPLS S-Bit1 and S-Bit2 are derived from the number of labels.
 *
 */
typedef struct
{
    GT_U32                  srcPortTrunk;
    GT_U32                  srcIsTrunk;
    GT_U8                   srcDev;
    GT_U16                  vid;
    GT_ETHERADDR            macDa;
    GT_U32                  label1;
    GT_U32                  sBit1;
    GT_U32                  exp1;
    GT_U32                  label2;
    GT_U32                  sBit2;
    GT_U32                  exp2;

} CPSS_DXCH_TUNNEL_TERM_MPLS_CONFIG_STC;


/*
 * typedef: union CPSS_DXCH_TUNNEL_TERM_CONFIG_UNT
 *
 * Description: Union for configuration for tunnel termination (X-over-MPLS
 *              tunnel termination or X-over-IPv4 tunnel termination).
 *
 * Fields:
 *
 *  ipv4Cfg           - configuration for X-over-IPv4 tunnel termination
 *  mplsCfg           - configuration for X-over-MPLS tunnel termination
 *
 */
typedef union
{
    CPSS_DXCH_TUNNEL_TERM_IPV4_CONFIG_STC   ipv4Cfg;
    CPSS_DXCH_TUNNEL_TERM_MPLS_CONFIG_STC   mplsCfg;

} CPSS_DXCH_TUNNEL_TERM_CONFIG_UNT;


/*
 * typedef: struct CPSS_DXCH_TUNNEL_TERM_ACTION_STC
 *
 * Description: Struct for tunnel termination action.
 *
 * Fields:
 *
 *  command           - tunnel termination entry forwarding command
 *  userDefinedCpuCode    - user defined cpu code. relavant only if <cmd> is
 *                          trap or mirror (CPSS_NET_FIRST_USER_DEFINED_E ..
 *                          CPSS_NET_LAST_USER_DEFINED_E)
 *  passengerPacketType   - type of passenger packet
 *  egressInterface   - egress interface to redirect packet. Relevant only
 *                      to Ethernet-over-MPLS tunnel ternimation. IPv4/6-over-X
 *                      are processed by the ingree pipeline. The egress
 *                      interface can be {Device, Port}, Trunk-ID or VLAN.
 *  isTunnelStart     - whether <outInterface> is tunnel start. Relevant only
 *                      to Ethernet-over-MPLS tunnel termination.
 *  tunnelStartIdx    - tunnel start entry index in case <isTunnelStart>
 *                      is GT_TRUE (APPLICABLE RANGES: 0..1023)
 *  vlanId            - vlan Id assigned to the tunnel termination packet. Note
 *                      that the VID is set by the TT action regardless to the
 *                      incoming packet <VID Precedence> (APPLICABLE RANGES: 0..4095)
 *  vlanPrecedence    - whether the VID assignment can be overridden by
 *                      subsequent VLAN assignment mechanism (the Policy engine)
 *  nestedVlanEnable  - Relevant only for Ethernet-over-MPLS tunnel termination.
 *                      If GT_TRUE then, in terms of egress tagging, the
 *                      passenger packet is always considered to received as
 *                      untagged.
 *  copyTtlFromTunnelHeader   - Relevant only for IPv4/6-over-X tunnel termination.
 *                              GT_TRUE:  the Router engine uses the IPv4 tunnel
 *                              header TTL as the incoming TTL.
 *                              GT_FALSE: the Router engine ignores the IPv4
 *                              tunnel header TTL and uses the passenger
 *                              packet TTL as the incoming TTL.
 *  qosMode           - QoS mode type (refer to GT_TUNNEL_QOS_MODE_TYPE)
 *  qosPrecedence     - whether QoS profile can be overridden by subsequent
 *                      pipeline engines
 *  qosProfile        - QoS profile to assign to packet in case <qosMode>
 *                      is GT_TUNNEL_QOS_UNTRUST_PKT (APPLICABLE RANGES: 0..127)
 *  defaultUP         - default user priority (APPLICABLE RANGES: 0..7)
 *  modifyUP          - Relevant only if the packet is Ethernet-over-MPLS.
 *                      Whether to modify user priority and how to modify it.
 *                      NOTE: The Tunnel Termination Action Assigns may
 *                      override the packet <modifyUP> assignment regardless
 *                      of its QoS Precedence.
 *  remapDSCP         - Whether to remap the DSCP. Relevant only if the
 *                      passenger packet is IPv4/6 and the entry <qosMode> is
 *                      configured to trust the passenger DSCP.
 *  modifyDSCP        - Whether to modify DSCP and how to modify it.
 *                      Relevant for IPv4/6-over-X, or, if the packet is
 *                      Ethernet-over-MPLS and the Ethernet passenger packet
 *                      is IPv4/6.
 *                      NOTE: The Tunnel Termination Action Assigns may
 *                      override the packet <modifyDSCP> assignment regardless
 *                      of its QoS Precedence.
 *  mirrorToIngressAnalyzerEnable - If set, the packet is mirrored to the Ingress
 *                                  Analyzer port.
 *  policerEnable     - If set, the packet is policed according to the traffic
 *                      profile defined in the specified <policerIndex>.
 *  policerIndex      - the policer index to use in case <enPolicer> is GT_TRUE
 *                      (APPLICABLE RANGES: 0..255)
 *  matchCounterEnable    - Whether to increment counter upon matching tunnel
 *                          termination key.
 *                          Options:
 *                          - Do not increment a Policy engine match counter for
 *                            this packet.
 *                          - Increment the Policy engine match counter defined
 *                            by the entry <matchCounterIndex>.
 *  matchCounterIndex - For packets matching this Tunnel Termination rule,
 *                      the respective Policy engine counter is incremented.
 *
 */
typedef struct
{
    CPSS_PACKET_CMD_ENT                         command;
    CPSS_NET_RX_CPU_CODE_ENT                    userDefinedCpuCode;
    CPSS_TUNNEL_PASSENGER_PACKET_ENT            passengerPacketType;
    CPSS_INTERFACE_INFO_STC                     egressInterface;
    GT_BOOL                                     isTunnelStart;
    GT_U32                                      tunnelStartIdx;
    GT_U16                                      vlanId;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT vlanPrecedence;
    GT_BOOL                                     nestedVlanEnable;
    GT_BOOL                                     copyTtlFromTunnelHeader;
    CPSS_DXCH_TUNNEL_QOS_MODE_ENT               qosMode;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    GT_U32                                      qosProfile;
    GT_U32                                      defaultUP;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyUP;
    GT_BOOL                                     remapDSCP;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyDSCP;
    GT_BOOL                                     mirrorToIngressAnalyzerEnable;
    GT_BOOL                                     policerEnable;
    GT_U32                                      policerIndex;
    GT_BOOL                                     matchCounterEnable;
    GT_U32                                      matchCounterIndex;

} CPSS_DXCH_TUNNEL_TERM_ACTION_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTunnelTypesh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChTunnelTypes.h,v $
 * Revision 1.1  2015/02/13 11:33:01  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
