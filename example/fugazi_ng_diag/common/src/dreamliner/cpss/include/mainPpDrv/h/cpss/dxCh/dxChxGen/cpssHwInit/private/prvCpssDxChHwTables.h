/* $Id: prvCpssDxChHwTables.h,v 1.1 2015/02/13 11:31:46 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h,v $
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
* prvCpssDxChHwTables.h
*
* DESCRIPTION:
*       Private API definition for tables access of the Cheetah and Cheetah 2.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __prvCpssDxChHwTablesh
#define __prvCpssDxChHwTablesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/private/prvCpssHwMultiPortGroups.h>


/* value for field of 'fieldWordNum' that indicates that the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'Global bit offset' and not
   offset in specific word

   see relevant APIs :
   prvCpssDxChReadTableEntryField(...)
   prvCpssDxChWriteTableEntryField(...)
   prvCpssDxChPortGroupReadTableEntryField(...)
   prvCpssDxChPortGroupWriteTableEntryField(...)
*/
#define PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS   0xFFFFFFFF

/*
 * Typedef: enum PRV_CPSS_DXCH_ACCESS_ACTION_ENT
 *
 * Description:
 *      access action for non-direct memory of cheetah
 *
 * Enumerations:
 *      PRV_CPSS_DXCH_ACCESS_ACTION_READ_E - read action
 *      PRV_CPSS_DXCH_ACCESS_ACTION_WRITE_E - write action
 *
 */
typedef enum
{
    PRV_CPSS_DXCH_ACCESS_ACTION_READ_E = 0,
    PRV_CPSS_DXCH_ACCESS_ACTION_WRITE_E
} PRV_CPSS_DXCH_ACCESS_ACTION_ENT;


/* common used types */
/*
 * Typedef: enum PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT
 *
 * Description:
 *      type of access to cheetah's table
 *
 * Enumerations:
 *      PRV_CPSS_DXCH_DIRECT_ACCESS_E   - direct access
 *      PRV_CPSS_DXCH_INDIRECT_ACCESS_E - indirect access
 *
 */

typedef enum
{
    PRV_CPSS_DXCH_DIRECT_ACCESS_E,
    PRV_CPSS_DXCH_INDIRECT_ACCESS_E
} PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT;


/* common used types
 *
 * Typedef: enum PRV_CPSS_DXCH_TABLE_ENT
 *
 * Description:
 *      names of the tables that are non-direct accessed
 *
 *     !!!! Values of enum linked with tables DB. Tables DB must be updated
 *          in any cases of changes in the enum !!!!!!
 *
 * Enumerations:
 *  PRV_CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E  - VLAN_PORT_PROTOCOL table
 *  PRV_CPSS_DXCH_TABLE_PORT_VLAN_QOS_E - PORT_VLAN_QOS table
 *  PRV_CPSS_DXCH_TABLE_TRUNK_MEMBERS_E - TABLE_TRUNK_MEMBERS.
 *  PRV_CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E - STATISTICAL_RATE_LIMIT table.
 *  PRV_CPSS_DXCH_TABLE_CPU_CODE_E -  CPU_CODE table.
 *  PRV_CPSS_DXCH_TABLE_PCL_CONFIG_E - PCL_CONFIG table
 *  PRV_CPSS_DXCH_TABLE_QOS_PROFILE_E - QOS_PROFILE table.
 *  PRV_CPSS_DXCH_TABLE_REMARKING_E - REMARKING table.
 *  PRV_CPSS_DXCH_TABLE_STG_E - STG table.
 *  PRV_CPSS_DXCH_TABLE_VLAN_E -  VLAN table.
 *  PRV_CPSS_DXCH_TABLE_MULTICAST_E - MULTICAST table
 *  PRV_CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E - ROUTE_HA_MAC_SA table.
 *  PRV_CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E - ROUTE_HA_ARP_DA table.
 *  PRV_CPSS_DXCH_TABLE_FDB_E - FDB table.
 *  PRV_CPSS_DXCH_TABLE_POLICER_E - POLICER table
 *  PRV_CPSS_DXCH_TABLE_POLICER_COUNTERS_E - POLICER_COUNTERS table.
 *  PRV_CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E -  EGRESS_PCL_CONFIG table.
 *  PRV_CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E -  TUNNEL_START table.
 *  PRV_CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E - QOS_PROFILE_TO_ROUTE table.
 *  PRV_CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E - ROUTE_ACCESS_MATRIX table.
 *  PRV_CPSS_DXCH2_LTT_TT_ACTION_E - LTT_TT_ACTION table.
 *  PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E - UC_MC_ROUTE_NEXT_HOP table.
 *  PRV_CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E - ROUTE_NEXT_HOP_AGE table.
 *  PRV_CPSS_DXCH3_TABLE_MAC2ME_E - MAC2ME table.
 *  PRV_CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E - INGRESS_VLAN_TRANSLATION table.
 *  PRV_CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E - EGRESS_VLAN_TRANSLATION table.
 *  PRV_CPSS_DXCH3_TABLE_VRF_ID_E - VRF_ID table.
 *  PRV_CPSS_DXCH3_LTT_TT_ACTION_E - LTT_TT_ACTION table.
 *  PRV_CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E - INGRESS_PCL_LOOKUP1 table.
 *  PRV_CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E - INGRESS_PCL_UDB table.
 *  PRV_CPSS_DXCH_XCAT_TABLE_LOGICAL_TARGET_MAPPING_E -LOGICAL_TARGET_MAPPING table.
 *  PRV_CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E -BCN_PROFILES table.
 *  PRV_CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E - Egress Policer Remarking table.
 *  PRV_CPSS_DXCH_XCAT_TABLE_LAST_E - last for XCAT supported tables.
 *  xCat2, Lion and above tables:
 *  PRV_CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E - INGRESS_PCL_LOOKUP01 table.
 *  PRV_CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E -  trunk hash mask crc table
 *  PRV_CPSS_DXCH_XCAT2_TABLE_LAST_E - last for XCAT2 supported tables.
 *  PRV_CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E        - ingress VLAN table
 *  PRV_CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E         - egress VLAN table
 *  PRV_CPSS_DXCH_LION_TABLE_STG_INGRESS_E         - ingress STG table
 *  PRV_CPSS_DXCH_LION_TABLE_STG_EGRESS_E          - egress STG table
 *  PRV_CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E   - port isolation L2 table
 *  PRV_CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E   - port isolation L3 table
 *  PRV_CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E -
 *              txq shaper - per port per TC token bucket configuration
 *  PRV_CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E -
 *              txq shaper - per port token bucket configuration
 *  PRV_CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E -
 *              txq source Id members table
 *  PRV_CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E -
 *              txq non trunk members table
 *  PRV_CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E -
 *              txq designated port table
 *  PRV_CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E -
 *              txq egress STC (Sampling To CPU) table
 *  PRV_CPSS_DXCH_LION_TABLE_ROUTER_VLAN_URPF_STC_E -
 *              unicast RPF STC (Sampling To CPU) table
 *  PRV_CPSS_DXCH_LION_TABLE_LAST_E - last table ID supported by Lion devices
 *
 *  Lion3 and above
 *  PRV_CPSS_DXCH_LION3_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E -
 *              TTI Physical ports attributes table
 *  PRV_CPSS_DXCH_LION3_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E -
 *              TTI pre lookup ingress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E -
 *              TTI post lookup ingress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_BRIDGE_INGRESS_EPORT_E -
 *              Bridge ingress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E -
 *              Bridge source trunk attribute table
 *  PRV_CPSS_DXCH_LION3_TABLE_EQ_INGRESS_EPORT_E -
 *              Pre-egress ingress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_EQ_EGRESS_EPORT_E -
 *              Pre-egress egress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_L2_ECMP_LTT_E -
 *              L2 ECMP LTT table
 *  PRV_CPSS_DXCH_LION3_TABLE_L2_ECMP_E -
 *              L2 ECMP table
 *  PRV_CPSS_DXCH_LION3_TABLE_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E -
 *              ePort to physical port mapping table
 *  PRV_CPSS_DXCH_LION3_TABLE_TXQ_EGRESS_EPORT_E -
 *              TXQ egress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_HA_EGRESS_EPORT_E -
 *              Header Alteration egress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_HA_EGRESS_EPORT_2_E -
 *              Header Alteration egress ePort table2
 *  PRV_CPSS_DXCH_LION3_TABLE_HA_PHYSICAL_PORT_ATTRIBUTES_E -
 *              Header Alteration physical port attributes table
 *  PRV_CPSS_DXCH_LION3_TABLE_HA_VID0_TO_TPID_SELECTOR_E -
 *              Header Alteration VID0 to TPID selection table
 *  PRV_CPSS_DXCH_LION3_TABLE_HA_VID1_TO_TPID_SELECTOR_E -
 *              Header Alteration VID1 to TPID selection table
 *  PRV_CPSS_DXCH_LION3_TABLE_L2_MLL_E -
 *              L2 MLL table
 *  PRV_CPSS_DXCH_LION3_TABLE_L2_MLL_LTT_E -
 *              L2 MLL LTT table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_PCL_EPORT_ATTRIBUTES_E -
 *              Egress PCL ePort attributes table
 *  PRV_CPSS_DXCH_LION3_TABLE_ADJACENCY_E -
 *              TRILL Adjacency table
 *  PRV_CPSS_DXCH_LION3_TABLE_RBID_LTT_E -
 *              rBridge LTT table
 *  PRV_CPSS_DXCH_LION3_TABLE_RBID_E -
 *              rBridge table
 *  PRV_CPSS_DXCH_LION3_TABLE_POLICER_EPORT_TRIGGER_E -
 *              Policer ePort Trigger table
 *  PRV_CPSS_DXCH_LION3_TABLE_POLICER_EVLAN_TRIGGER_E -
 *              Policer eVlan Trigger table
  *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_POLICER_EPORT_TRIGGER_E -
 *              Egress Policer ePort Trigger table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_POLICER_EVLAN_TRIGGER_E -
 *              Egress Policer eVlan Trigger table
 *  PRV_CPSS_DXCH_LION3_TABLE_IPVX_INGRESS_EPORT_E -
 *              IPvX Ingress ePort table
 *  PRV_CPSS_DXCH_LION3_TABLE_IPVX_EVLAN_E -
 *              IPvX eVlan table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_E -
 *              ingress OAM table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E -
 *              ingress OAM Aging table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_MEG_EXCEPTION_E -
 *              ingress OAM MEG Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E -
 *              ingress OAM Source Interface Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E -
 *              ingress OAM Invalid Keepalive Hash table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E -
 *              ingress OAM Excess Keepalive table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E -
 *              ingress OAM Exception Summary table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E -
 *              ingress OAM RDI Status Cheange Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E -
 *              ingress OAM Tx Period Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E -
 *              ingress OAM Opcode Packet Command table
 *  PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_LM_OFFSET_E
 *              ingress OAM Loss Measurement offset table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_E
 *              egress OAM table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E
 *              egress OAM aging table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_MEG_EXCEPTION_E
 *              egress OAM MEG Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E
 *              egress OAM Source Interface Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E
 *              egress OAM Invalid Keepalive Hash table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E
 *              egress OAM Excess Keepalive table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E
 *              egress OAM Exception Summary table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E
 *              egress OAM RDI Status Cheange Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E
 *              egress OAM Tx Period Exception table
 *  PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E
 *              egress OAM Opcode Packet Command table
 *  PRV_CPSS_DXCH_LION3_TABLE_OAM_TIMESTAMP_OFFSET_E
 *              OAM Timestamp Offset Table
 *  PRV_CPSS_DXCH_LION3_TABLE_OAM_LM_OFFSET_E
 *              OAM Loss Measurement Offset Table
 *  PRV_CPSS_DXCH_LION3_TABLE_LAST_E - last table ID supported by Lion2 devices
 *
 *  General:
 *  PRV_CPSS_DXCH_TABLE_LAST_E - must be maximal
 *
 *
 ******************************************************************************
 *  Table Name                  index                          entry
 ******************************************************************************
 *  VLAN_PORT_PROTOCOL        port num                  8 words, word# is protocol,
 *                                                      1 word of data per protocol
 * PORT_VLAN_QOS             port num                        2 words of data
 *  TABLE_TRUNK_MEMBERS     (trunk_id << 3) + member#         1 word   of data
 *  STATISTICAL_RATE_LIMIT  limiter index, 0..32              1 word   of data
 *  CPU_CODE                  cpu code, 0..255                1 word  of data
 *  PCL_CONFIG                table index, 0..1152 Ch         1 word of data
 *                            0 .. 4224 Ch2                   1 word of data
 *  QOS_PROFILE &            profile index, 0..72 Ch          1 word of data
 *  REMARKING                0..128 Ch2                       1 word of data
 *  STG                     STP Group Index, 0..255           2 words of data
 *  VLAN                    vid, 0..4095                      3 words of data, Ch
 *                                                            4 words of data, Ch2
 *  MULTICAST               vidx, 0..4095                     1 words of data
 *  ROUTE_HA_MAC_SA         port or vid, 0..4095              1 word of data
 *  ROUTE_HA_ARP_DA        Ch, MAC DA index, 0...1023         2 words of data
 *                         Ch2, MAC DA/Tunnel row, 0..1023,   6 words of data,4 MAC DA per row.
 *  FDB                    FDB entry #, 0..(16K -1)           4 words of DATA
 *  POLICER                policer index, 0.. 255             2 words of DATA
 *  POLICER_COUNTERS       counter index, 0.. 15              2 words of DATA
 *
 *  Cheetah 2 only tables
 *
 *  EGRESS_PCL_CONFIG      table index, 0..4159               1 word of data
 *  TUNNEL_START           table index, 0..1023               6 words of data
 *  QOS_PROFILE_TO_ROUTE   table index, 0..15                 1 words of data
 *  ROUTE_ACCESS_MATRIX    table index, 0..7                  1 words of data
 *  LTT_TT_ACTION          table index, 0..1023               4 words of data
 *   UC_MC_ROUTE_NEXT_HOP   table index, 0..4095               4 words of data
 *   ROUTE_NEXT_HOP_AGE     table index, 0..125                1 words of data
 *
 *   Cheetah 3 only tables
 *
 *   MAC2ME                 table index, 0..8                  4 words of data
 *   LTT_TT_ACTION          table index, 0..5119               4 words of data
 *
 *   xCat and above
 *   LTT_TT_ACTION          table index, 0..(13K/4 - 1)        4 words of data
 *   VLAN                   vid, 0..4095                       6 words of data, Xcat
 *
 *   XCat and above tables:
 *   INGRESS_PCL_LOOKUP1    table index, 0..4223    1 word of data, 8 bytes aligned
 *  INGRESS_PCL_UDB        table index, 0..7       8 word of data, 32 bytes aligned
 *  LOGICAL_TARGET_MAPPING table index, 0.. 511    1 word of data, 4 bytes alligned
 *  EGRESS_POLICER_REMARKING table index 0..51     2 words of data, 8 bytes alligned
 *
 *  xCat2 tables:
 *  INGRESS_PCL_LOOKUP01 - table index, 0..4223    1 word of data, 8 bytes aligned
 *
 *  Lion and above tables:
 *  ROUTER_VLAN_URPF        table index 0..255    1 word of data, 4 bytes aligned
 *
 *  Lion3 and above tables:
 *  PHYSICAL_PORT_ATTRIBUTE_E - table index 0..255  1 word of data,  1 word aligned
 *  PRE_TTI_LOOKUP_INGRESS_EPORT_E - table index 0..4095   3 words of data, 8 words aligned
 *  POST_TTI_LOOKUP_INGRESS_EPORT_E - table index 0..4095  1 word of data, 1 word aligned
 *  BRIDGE_INGRESS_EPORT_E - table index 0..4095    3 words of data,  4 words aligned
 *  BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E - 0..4095 1 word of data,  1 word aligned
 *  EQ_INGRESS_EPORT_E - table index 0..4095 1 word of data,  1 word aligned
 *  EQ_EGRESS_EPORT_E - table index 0..4095 1 word of data,  1 word aligned
 *  L2_ECMP_LTT_E - table index 0..xxx (need to check) 1 word of data,  1 word aligned
 *  L2_ECMP_E - table index 0..xxx (need to check) 1 word of data,  1 word aligned
 *  EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E - table index 0..4095 2 word of data,  2 word aligned
 *  TXQ_EGRESS_EPORT_E - table index 0..4095 1 word of data,  1 word aligned
 *  HA_EGRESS_EPORT_E - table index 0..4095 2 word of data,  2 word aligned
 *  HA_EGRESS_EPORT_2_E - table index 0..4095 1 word of data,  1 word aligned
 *  HA_EVB_TR101_EGRESS_EPORT_E - table index 0..4095 2 word of data,  2 word aligned
 *  HA_PHYSICAL_PORT_ATTRIBUTES_E - table index 0..255  2 word of data,  2 word aligned
 *  HA_VID0_TO_TPID_SELECTOR_E - table index 0..1642  1 word of data,  1 word aligned
 *  HA_VID1_TO_TPID_SELECTOR_E - table index 0..1642  1 word of data,  1 word aligned
 *  L2_MLL_E - table index 0..xxx (need to check) 3 words of data,  4 word aligned
 *  L2_MLL_LTT_E - table index 0..xxx (need to check) 1 word of data,  1 word aligned
 *  ADJACENCY_E - table index 0..511 4 words of data, 4 words aligned
 *  RBID_LTT_E - table index 0..(64K-1) 1 word of data, 1 word aligned
 *  RBID_E - table index 0..4095 1 word of data, 1 word aligned
 *  EGRESS_PCL_EPORT_ATTRIBUTES_E - table index 0..0xFFF 1 word of data,  1 word aligned
 *  POLICER_EPORT_TRIGGER_E - table index 0..4095, 2 words of data, 2 words alligned
 *  POLICER_EVLAN_TRIGGER_E - table index 0..8191, 2 words of data, 2 words alligned
 *  EGRESS_POLICER_EPORT_TRIGGER_E - table index 0..4095, 2 words of data, 2 words alligned
 *  EGRESS_POLICER_EVLAN_TRIGGER_E - table index 0..8191, 2 words of data, 2 words alligned
 *  IPVX_INGRESS_EPORT_E - table index 0..4095, 1 words of data, 1 words alligned
 *  IPVX_EVLAN_E - table index 0..65535, 3 words of data, 4 words alligned
 *  OAM_E - table index 0..32767, 3 words of data, 4 words alligned
 *  OAM_AGING_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_MEG_EXCEPTION_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_SOURCE_INTERFACE_EXCEPTION_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_INVALID_KEEPALIVE_HASH_E - able index 0..63, 1 words of data, 1 words alligned
 *  OAM_EXCESS_KEEPALIVE_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_EXCEPTION_SUMMARY_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_RDI_STATUS_CHANGE_EXCEPTION_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_TX_PERIOD_EXCEPTION_E - table index 0..63, 1 words of data, 1 words alligned
 *  OAM_OPCODE_PACKET_COMMAND_E - table index 0..254, 2 words of data, 2 words alligned
 *  OAM_TIMESTAMP_OFFSET_E - table index 0..15, 1 word of data, 1 word alligned
 *  OAM_LM_OFFSET_E - table index 0..15, 1 word of data, 1 word alligned
 *  NOTE:
 *
*/

typedef enum
{
    PRV_CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E = 0,
    PRV_CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
    PRV_CPSS_DXCH_TABLE_TRUNK_MEMBERS_E,
    PRV_CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E,
    PRV_CPSS_DXCH_TABLE_CPU_CODE_E,
    PRV_CPSS_DXCH_TABLE_PCL_CONFIG_E,
    PRV_CPSS_DXCH_TABLE_QOS_PROFILE_E,
    PRV_CPSS_DXCH_TABLE_REMARKING_E,
    PRV_CPSS_DXCH_TABLE_STG_E,
    PRV_CPSS_DXCH_TABLE_VLAN_E,
    PRV_CPSS_DXCH_TABLE_MULTICAST_E,
    PRV_CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E,
    PRV_CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E,
    PRV_CPSS_DXCH_TABLE_FDB_E,
    PRV_CPSS_DXCH_TABLE_POLICER_E,
    PRV_CPSS_DXCH_TABLE_POLICER_COUNTERS_E,
    PRV_CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
    PRV_CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
    PRV_CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E,
    PRV_CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E,
    PRV_CPSS_DXCH2_LTT_TT_ACTION_E,
    PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E,
    PRV_CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E,
    PRV_CPSS_DXCH3_TABLE_MAC2ME_E,
    PRV_CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E,
    PRV_CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E,
    PRV_CPSS_DXCH3_TABLE_VRF_ID_E,
    PRV_CPSS_DXCH3_LTT_TT_ACTION_E,
    PRV_CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
    PRV_CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E,
    PRV_CPSS_DXCH_XCAT_TABLE_LOGICAL_TARGET_MAPPING_E,
    PRV_CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
    PRV_CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
    /* must be last for XCAT supported tables */
    PRV_CPSS_DXCH_XCAT_TABLE_LAST_E,

    /* based on the end of XCAT values end */
    PRV_CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E =
        PRV_CPSS_DXCH_XCAT_TABLE_LAST_E,

    /* supported in XCAT2, Lion and above devices*/
    PRV_CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,

    /* must be last for XCAT2 supported tables */
    PRV_CPSS_DXCH_XCAT2_TABLE_LAST_E,

    /* based on the end of XCAT2 values end */
    PRV_CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E =
        PRV_CPSS_DXCH_XCAT2_TABLE_LAST_E,
    PRV_CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
    PRV_CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
    PRV_CPSS_DXCH_LION_TABLE_STG_EGRESS_E,

    PRV_CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
    PRV_CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,

    PRV_CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,
    PRV_CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,

    PRV_CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
    PRV_CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
    PRV_CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,

    PRV_CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,

    PRV_CPSS_DXCH_LION_TABLE_ROUTER_VLAN_URPF_STC_E,

    /* must be last for LION supported tables */
    PRV_CPSS_DXCH_LION_TABLE_LAST_E,

    /* must be last for LION2 supported tables */
    PRV_CPSS_DXCH_LION2_TABLE_LAST_E,

    /* based on the end of LION values end */
    PRV_CPSS_DXCH_LION3_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E =
        PRV_CPSS_DXCH_LION2_TABLE_LAST_E,
    PRV_CPSS_DXCH_LION3_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_BRIDGE_INGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E,
    PRV_CPSS_DXCH_LION3_TABLE_EQ_INGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_EQ_EGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_L2_ECMP_LTT_E,
    PRV_CPSS_DXCH_LION3_TABLE_L2_ECMP_E,
    PRV_CPSS_DXCH_LION3_TABLE_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
    PRV_CPSS_DXCH_LION3_TABLE_TXQ_EGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_HA_EGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_HA_EGRESS_EPORT_2_E,
    PRV_CPSS_DXCH_LION3_TABLE_HA_PHYSICAL_PORT_ATTRIBUTES_E,
    PRV_CPSS_DXCH_LION3_TABLE_HA_VID0_TO_TPID_SELECTOR_E,
    PRV_CPSS_DXCH_LION3_TABLE_HA_VID1_TO_TPID_SELECTOR_E,
    PRV_CPSS_DXCH_LION3_TABLE_L2_MLL_E,
    PRV_CPSS_DXCH_LION3_TABLE_L2_MLL_LTT_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_PCL_EPORT_ATTRIBUTES_E,
    PRV_CPSS_DXCH_LION3_TABLE_ADJACENCY_E,
    PRV_CPSS_DXCH_LION3_TABLE_RBID_LTT_E,
    PRV_CPSS_DXCH_LION3_TABLE_RBID_E,
    PRV_CPSS_DXCH_LION3_TABLE_POLICER_EPORT_TRIGGER_E,
    PRV_CPSS_DXCH_LION3_TABLE_POLICER_EVLAN_TRIGGER_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_POLICER_EPORT_TRIGGER_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_POLICER_EVLAN_TRIGGER_E,
    PRV_CPSS_DXCH_LION3_TABLE_IPVX_INGRESS_EPORT_E,
    PRV_CPSS_DXCH_LION3_TABLE_IPVX_EVLAN_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_MEG_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E,
    PRV_CPSS_DXCH_LION3_TABLE_INGRESS_OAM_LM_OFFSET_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_MEG_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E,
    PRV_CPSS_DXCH_LION3_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E,
    PRV_CPSS_DXCH_LION3_TABLE_OAM_TIMESTAMP_OFFSET_E,
    PRV_CPSS_DXCH_LION3_TABLE_OAM_LM_OFFSET_E,

    /* must be last for LION3 supported tables */
    PRV_CPSS_DXCH_LION3_TABLE_LAST_E,

    /* must be last */
    PRV_CPSS_DXCH_TABLE_LAST_E

} PRV_CPSS_DXCH_TABLE_ENT;

/*
 * Typedef: struct PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC
 *
 * Description: A structure to hold Cheetah's indirect access table entry info.
 *
 * Fields:
 *      controlReg   - address of the control register
 *      dataReg      - address of the data register of the entry
 *      trigBit      - the bit num that trig the action in the control register
 *      indexBit     - bit where to write the index (of entry)
 *      specificTableValue - some tables share the same control registers ,
 *                           this value is the specific table
 *      specificTableBit   - start bit for the specificTableValue
 *      actionBit          - the bit number where to specify the action (read/write)
 */


typedef struct
{
    GT_U32  controlReg;
    GT_U32  dataReg;
    GT_U32  trigBit;
    GT_U32  indexBit;
    GT_U32  specificTableValue;
    GT_U32  specificTableBit;
    GT_U32  actionBit;
} PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC
 *
 * Description: A structure to hold Cheetah's direct access table entry info.
 *
 * Fields:
 *      baseAddress    - base address
 *      step           - step in entry promotion
 *      nextWordOffset - next word offest value in bytes.
 */

typedef struct
{
    GT_U32  baseAddress;
    GT_U32  step;
    GT_U32  nextWordOffset;
} PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC;


/*
 * Typedef: struct PRV_CPSS_DXCH_TABLES_INFO_STC
 *
 * Description: A structure to hold Cheetah's direct access table entry info.
 *
 * Fields:
 *      maxNumOfEntries  - number of entries
 *      entrySize        - size in words
 *      readAccessType   - direct/indirect access for read action
 *      readTablePtr     - address of table for read action
 *      writeAccessType  - direct/indirect access for write action
 *      writeTablePtr    - address of table for write action
 */

typedef struct
{
    GT_U32                              maxNumOfEntries;
    GT_U32                              entrySize;
    PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT readAccessType;
    GT_VOID                            *readTablePtr;
    PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT writeAccessType;
    GT_VOID                            *writeTablePtr;

} PRV_CPSS_DXCH_TABLES_INFO_STC;

/*******************************************************************************
* prvCpssDxChReadTableEntry
*
* DESCRIPTION:
*       Read a whole entry from the table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       tableType - the specific table name
*       entryIndex   - index in the table
*
* OUTPUTS:
*       entryValuePtr - (pointer to) the data read from the table
*                       may be NULL in the case of indirect table.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*      In the case of entryValuePtr == NULL and indirect table the function
*      just send read entry command to device. And entry is stored in the
*      data registers of a indirect table
*******************************************************************************/
GT_STATUS prvCpssDxChReadTableEntry
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    OUT GT_U32                 *entryValuePtr
);

/*******************************************************************************
* prvCpssDxChWriteTableEntry
*
* DESCRIPTION:
*       Write a whole entry to the table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       tableType - the specific table name
*       entryIndex   - index in the table
*       entryValuePtr - (pointer to) the data that will be written to the table
*                       may be NULL in the case of indirect table.
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*      In the case of entryValuePtr == NULL and indirect table the function
*      just send write entry command to device. And entry is taken from the
*      data registers of a indirect table.
*******************************************************************************/
GT_STATUS prvCpssDxChWriteTableEntry
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

/*******************************************************************************
* prvCpssDxChReadTableEntryField
*
* DESCRIPTION:
*       Read a field from the table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       tableType - the specific table name
*       entryIndex   - index in the table
*       fieldWordNum - field word number
*                   use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                   if need global offset in the field of fieldOffset
*       fieldOffset  - field offset
*       fieldLength - field length
*
* OUTPUTS:
*       fieldValuePtr - (pointer to) the data read from the table
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS prvCpssDxChReadTableEntryField
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                   fieldWordNum,
    IN GT_U32                   fieldOffset,
    IN GT_U32                   fieldLength,
    OUT GT_U32                 *fieldValuePtr
);

/*******************************************************************************
* prvCpssDxChWriteTableEntryField
*
* DESCRIPTION:
*       Write a field to the table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       tableType - the specific table name
*       entryIndex   - index in the table
*       fieldWordNum - field word number
*                   use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                   if need global offset in the field of fieldOffset
*       fieldOffset  - field offset
*       fieldLength - field length
*       fieldValue - the data write to the table
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS prvCpssDxChWriteTableEntryField
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                   fieldWordNum,
    IN GT_U32                   fieldOffset,
    IN GT_U32                   fieldLength,
    IN GT_U32                   fieldValue
);


/*******************************************************************************
* prvCpssDxChTableNumEntriesGet
*
* DESCRIPTION:
*       get the number of entries in a table
*       needed for debug purpose
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       tableType - the specific table name
*
* OUTPUTS:
*       numEntriesPtr - (pointer to) number of entries in the table
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChTableNumEntriesGet
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_TABLE_ENT  tableType,
    OUT GT_U32                  *numEntriesPtr
);


/*******************************************************************************
* prvCpssDxChTablesAccessInit
*
* DESCRIPTION:
*       Initializes all structures for table access.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*
* OUTPUTS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  prvCpssDxChTablesAccessInit
(
    IN GT_U8    devNum
);

/*******************************************************************************
* prvCpssDxChPortGroupWriteTableEntryFieldList
*
* DESCRIPTION:
*       Write a list of fields to the table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum            - device number
*       portGroupId - the port group Id , to support multi-port-group devices that need to access
*                specific port group
*       tableType         - HW table Id
*       entryIndex        - entry Index
*       entryMemoBufArr   - the work memory for read, update and write the entry
*       fieldsAmount      - amount of updated fields in the entry
*       fieldOffsetArr    - (array) the offset of the field in bits
*                           from the entry origin
*       fieldLengthArr    - (array) the length of the field in bits,
*                           0 - means to skip this subfield
*       fieldValueArr     - (array) the value of the field
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK              - on success.
*       GT_BAD_PARAM       - one of the input parameters is not valid.
*       GT_TIMEOUT         - after max number of retries checking if PP ready
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryFieldList
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN PRV_CPSS_DXCH_TABLE_ENT                tableType,
    IN GT_U32                                 entryIndex,
    IN GT_U32                                 entryMemoBufArr[],
    IN GT_U32                                 fieldsAmount,
    IN GT_U32                                 fieldOffsetArr[],
    IN GT_U32                                 fieldLengthArr[],
    IN GT_U32                                 fieldValueArr[]
);

/*******************************************************************************
* prvCpssDxChReadTableEntryFieldList
*
* DESCRIPTION:
*       Read a list of fields from the table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum            - device number
*       portGroupId - the port group Id , to support multi-port-group devices that need to access
*                specific port group
*       tableId           - HW table Id
*       entryIndex        - entry Index
*       entryMemoBufArr   - the work memory for read, update and write the entry
*       fieldsAmount      - amount of updated fields in the entry
*       fieldOffsetArr    - (array) the offset of the field in bits
*                           from the entry origin
*       fieldLengthArr    - (array) the length of the field in bits,
*                           0 - means to skip this subfield

* OUTPUTS:
*       fieldValueArr     - (array) the value of the field
*
*
* RETURNS:
*       GT_OK              - on success.
*       GT_BAD_PARAM       - one of the input parameters is not valid.
*       GT_TIMEOUT         - after max number of retries checking if PP ready
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChPortGroupReadTableEntryFieldList
(
    IN  GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN  PRV_CPSS_DXCH_TABLE_ENT                tableId,
    IN  GT_U32                                 entryIndex,
    IN  GT_U32                                 entryMemoBufArr[],
    IN  GT_U32                                 fieldsAmount,
    IN  GT_U32                                 fieldOffsetArr[],
    IN  GT_U32                                 fieldLengthArr[],
    OUT GT_U32                                 fieldValueArr[]
);


/*******************************************************************************
* prvCpssDxChPortGroupWriteTableEntry
*
* DESCRIPTION:
*       Write a whole entry to the table. - for specific portGroupId
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       portGroupId - the port group Id , to support multi-port-group devices that need to access
*                specific port group
*       tableType - the specific table name
*       entryIndex   - index in the table
*       entryValuePtr - (pointer to) the data that will be written to the table
*                       may be NULL in the case of indirect table.
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*      In the case of entryValuePtr == NULL and indirect table the function
*      just send write entry command to device. And entry is taken from the
*      data registers of a indirect table.
*******************************************************************************/
GT_STATUS prvCpssDxChPortGroupWriteTableEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN PRV_CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

/*******************************************************************************
* prvCpssDxChPortGroupReadTableEntry
*
* DESCRIPTION:
*       Read a whole entry from the table. - for specific portGroupId
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       portGroupId - the port group Id , to support multi-port-group devices that need to access
*                specific port group
*       tableType - the specific table name
*       entryIndex   - index in the table
*
* OUTPUTS:
*       entryValuePtr - (pointer to) the data read from the table
*                       may be NULL in the case of indirect table.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*      In the case of entryValuePtr == NULL and indirect table the function
*      just send read entry command to device. And entry is stored in the
*      data registers of a indirect table
*******************************************************************************/
GT_STATUS prvCpssDxChPortGroupReadTableEntry
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroupId,
    IN PRV_CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
);

/*******************************************************************************
* prvCpssDxChPortGroupReadTableEntryField
*
* DESCRIPTION:
*       Read a field from the table. - for specific portGroupId
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       portGroupId - the port group Id , to support multi-port-group devices that need to access
*                specific port group
*       tableType - the specific table name
*       entryIndex   - index in the table
*       fieldWordNum - field word number
*                   use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                   if need global offset in the field of fieldOffset
*       fieldOffset  - field offset
*       fieldLength - field length
*
* OUTPUTS:
*       fieldValuePtr - (pointer to) the data read from the table
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_TIMEOUT  - after max number of retries checking if PP ready
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS prvCpssDxChPortGroupReadTableEntryField
(
    IN GT_U8                  devNum,
    IN GT_U32                  portGroupId,
    IN PRV_CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    OUT GT_U32                *fieldValuePtr
);

/*******************************************************************************
* prvCpssDxChPortGroupWriteTableEntryField
*
* DESCRIPTION:
*       Write a field to the table.  - for specific portGroupId
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       portGroupId - the port group Id , to support multi-port-group devices that need to access
*                specific port group
*       tableType - the specific table name
*       entryIndex   - index in the table
*       fieldWordNum - field word number
*                   use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                   if need global offset in the field of fieldOffset
*       fieldOffset  - field offset
*       fieldLength - field length
*       fieldValue - the data write to the table
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on failure.
*       GT_OUT_OF_RANGE          - parameter not in valid range
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       In xCat A1 and above devices the data is updated only when the last
*       word in the entry was written.
*
*******************************************************************************/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryField
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroupId,
    IN PRV_CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwTablesh */
/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChHwTables.h,v $
 * Revision 1.1  2015/02/13 11:31:46  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
