/* $Id: cpssDxCh3Policer.h,v 1.1 2015/02/13 11:31:24 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h,v $
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
* cpssDxCh3Policer.h
*
* DESCRIPTION:
*       CPSS DxCh3 Policing Engine API and above.
*       The Policing Engine (or Traffic Conditioner) is responsible for:
*           - Ingress Metering - implemented using SrTCM Token Bucket,
*             TrTCM Token Bucket and Two Color Mode algorithms.
*           - Egress Metering - implemented for xCat and above devices
*           - Ingress Billing - byte/packet counters that keep track of amount
*             of traffic belonging to a flow.
*           - Egress Billing - implemented for xCat and above devices
*             Re-Marking - QoS assigned to the packet according to metering
*             results.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __cpssDxCh3Policerh
#define __cpssDxCh3Policerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>

/*
 * typedef: enum CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT
 *
 * Description: Enumeration for Color Counting mode.
 *
 * Enumerations:
 *  CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E -   Color Counting is done according
 *                                          to the packet's Conformance Level.
 *
 *  CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E -   Color Counting is done according
 *                                          to the packet's Drop Precedence.
 */
typedef enum{
    CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E,
    CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E
} CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT
 *
 * Description: Billing Counters Mode. This enumeration controls the Billing
 *              counters resolution.
 *
 * Enumerations:
 *  CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E   - 1 Byte resolution.
 *
 *  CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E - 16 Byte resolution.
 *                                                In 16 Byte resolution,
 *                                                Billing counter is rounded
 *                                                down, i.e. a 64 Billing
 *                                                counter packet is counted as
 *                                                4 while a 79 Billing counter
 *                                                packet is counted as 4 as
 *                                                well.
 *
 *  CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E   - packet resolution.
 */
typedef enum{
    CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E,
    CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E,
    CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E
} CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT
 *
 * Description: Packet/Byte Based Meter resolution.
 *              This enumeration controls the metering algorithm resolution:
 *              packets per second or Bytes per second.
 *
 *
 * Enumerations:
 *  CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E   - Byte based Meter resolution.
 *
 *  CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E - Packet based Meter
 *                                                  resolution.
 */
typedef  enum{
    CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E,
    CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E
} CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT
 *
 * Description: Enumeration for Management counters resolution.
 *
 * Enumerations:
 *  CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E  -  1 Byte resolution.
 *
 *  CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E -  16 Byte resolution.
 */
typedef enum{
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E
} CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT
 *
 * Description: Enumeration for Management Counters Set.
 *
 * Enumerations:
 *    CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E       - Management Counters Set #0.
 *    CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E       - Management Counters Set #1.
 *    CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E       - Management Counters Set #2.
 *    CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E   - Management Counters are
 *                                               disabled for the Policer
 *                                               Entry.
 *
 * Notes: each Metering Entry may be associated with one of 3
 *        Management Counters Set, or not at all.
 *
 */
typedef enum
{
    CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E
} CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT
 *
 * Description: Enumeration for Management Counters Type.
 *
 * Enumerations:
 *    CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E  - These Management Counters count
 *                                           marked GREEN Packet and DU that
 *                                           passed in this flow.
 *    CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E - These Management Counters count
 *                                           marked YELLOW Packet and DU that
 *                                           passed in this flow.
 *    CPSS_DXCH3_POLICER_MNG_CNTR_RED_E    - These Management Counters count
 *                                           marked RED Packet and DU that
 *                                           passed in this flow and were not
 *                                           dropped.
 *    CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E   - These Management Counters count
 *                                           marked RED Packet and DU that
 *                                           passed in this flow and were
 *                                           dropped.
 */
typedef enum
{
    CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_RED_E,
    CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E
} CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT
 *
 * Description: Enumeration for Policer Entry Type.
 *
 * Enumerations:
 *    CPSS_DXCH3_POLICER_ENTRY_METERING_E  - Metering Policer Entry.
 *    CPSS_DXCH3_POLICER_ENTRY_BILLING_E   - Billing Policer Entry.
 */
typedef enum
{
    CPSS_DXCH3_POLICER_ENTRY_METERING_E,
    CPSS_DXCH3_POLICER_ENTRY_BILLING_E
} CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT
 *
 * Description: Enumeration for Tunnel Termination Packet Size Mode.
 *
 * Enumerations:
 *  CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_ENT   -  Metering and counting
 *                                                      of TT packets is
 *                                                      performed according to
 *                                                      the Ingress Metered
 *                                                      Packet Size Mode.
 *
 *  CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_ENT -  Metering and counting
 *                                                      of TT packets is
 *                                                      performed according to
 *                                                      L3 datagram size only.
 *                                                      This mode doesn't
 *                                                      include the tunnel
 *                                                      header size, the L2
 *                                                      header size, and the
 *                                                      packet CRC in the
 *                                                      metering and counting.
 *
 */
typedef enum{
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_ENT,
    CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_ENT
} CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_METER_MODE_ENT
 *
 * Description: Enumeration of the Meter modes.
 *
 * Enumerations:
 *    CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E - a Single Rate Three Color
 *                                             Marking mode.
 *    CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E - a Two Rate Three Color Marking
 *                                             mode.
 */
typedef enum
{
    CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E,
    CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E
} CPSS_DXCH3_POLICER_METER_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT
 *
 * Description: Structure contains Policer command for Non-Conforming Packets.
 *              Conforming (i.e., In-profile) packets preserve their QoS
 *              parameters setting and their forwarding decision, as set in the
 *              previous engine in the ingress pipe. Non-conforming (i.e.,
 *              Out-ofprofile) packets, are subject to forwarding decision
 *              override and QoS parameter remarking according to the setting
 *              of this field.
 *              A non-conforming packet (marked by Red or Yellow color) can be
 *              dropped by the metering engine, QoS profile remark or can be
 *              forwarded.
 *
 * Enumerations:
 *      CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E  - NO CHANGE.
 *                          Non-conforming packets preserve their QoS
 *                          parameters setting and their forwarding decision,
 *                          as set in the previous engine in the ingress pipe.
 *
 *      CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E       - DROP.
 *                          Non-conforming packets preserve their QoS
 *                          parameters setting as set in the previous engine in
 *                          the ingress pipe, but their forwarding decision
 *                          changes to Soft Drop or Hard Drop according to the
 *                          setting of the Policer Drop Mode.
 *
 *      CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E     - REMARK. QoS Profile
 *                                                        Remarking by table.
 *                          Non-conforming packets preserve their forwarding
 *                          decision as set in the previous engine in the
 *                          ingress pipe, but their QoS parameters setting is
 *                          modified as follows:
 *                          - QoS Profile is extracted from the Policers QoS
 *                            Remarking Table Entry according to CL.
 *                          - ModifyUP is modified by this entry <modifyUP>.
 *                          - ModifyDSCP is modified by this entry <modifyDSCP>.
 *
 *      CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E - REMARK. QoS Profile
 *                                                             Remark by entry.
 *                          Non-conforming packets preserve their forwarding
 *                          decision as set in the previous engine in the ingress
 *                          pipe, but their QoS parameters setting is modified as
 *                          follows:
 *                          - QoS Profile is modified by this entry <QosProfile>.
 *                          - ModifyUP is modified by this entry <ModifyUP>.
 *                          - ModifyDSCP is modified by this entry <ModifyDSCP>.
 *                          No access to remarking table is done.
 *                          Note: Relevant only for xCat and above.
 *
 */
typedef enum
{
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E,
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E,
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E
} CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT;

/*
 * typedef: struct CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC
 *
 * Description: Structure for Policer Management Counters Entry.
 *
 * Fields:
 *  duMngCntr       -   Data Unit Management Counter.
 *                      DU size defines according to the assigned resolution
 *                      of Management counters (1 Byte or 16 Bytes).
 *                      Note: DxCh3 - only 32 LSB are used.
 *                            xCat and above - 42 bits are used.
 *  packetMngCntr   -   Packet 32 bit size Management Counter.
 *
 */
typedef struct{
    GT_U64  duMngCntr;
    GT_U32  packetMngCntr;
} CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC;


/*
 * typedef: struct CPSS_DXCH3_POLICER_BILLING_ENTRY_STC
 *
 * Description: Structure for Policer Billing Entry.
 *
 * Fields:
 *  greenCntr   -   Green Billing counter, counts the number of
 *                  Data Units (1 Byte/16 Bytes) of packets, marked Green
 *                  that passed in this flow.
 *                  Note: DxCh3 - only 32 LSB are used.
 *                        xCat and above - 42 bits are used.
 *  yellowCntr  -   Yellow Billing counter, counts the number of
 *                  Data Units (1 Byte/16 Bytes) of packets, marked Yellow
 *                  that passed in this flow.
 *                  Note: DxCh3 - only 32 LSB are used.
 *                        xCat and above - 42 bits are used.
 *  redCntr     -   Red Billing counter, counts the number of
 *                  Data Units (1 Byte/16 Bytes) of packets, marked Red
 *                  that passed in this flow.
 *                  Note: DxCh3 - only 32 LSB are used.
 *                        xCat and above - 42 bits are used.
 *  billingCntrMode    -   The resolution of billing counters.
 *                  Applies only to the color counters.
 *                  Note: Relevant for xCat and above.
 *
 */
typedef struct{
    GT_U64                                      greenCntr;
    GT_U64                                      yellowCntr;
    GT_U64                                      redCntr;
    CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT    billingCntrMode;
} CPSS_DXCH3_POLICER_BILLING_ENTRY_STC;

/*
 * typedef: union CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT
 *
 * Description: Union for single/dual Token Bucket parameters.
 *              The set of meter parameters depends on the Meter Mode:
 *              - srTcmParams - Single Rate Three Color Marking mode params;
 *              - trTcmParams - Two Rate Three Color Marking mode params.
 *
 * Fields:
 *  srTcmParams:
 *     cir    - Committed Information Rate in Kbps (1K = 1000);
 *     cbs    - Committed Burst Size in bytes;
 *     ebs    - Excess Burst Size in bytes;
 *  trTcmParams:
 *     cir    - Committed Information Rate in Kbps (1K = 1000);
 *     cbs    - Committed Burst Size in bytes;
 *     pir    - Peak Information Rate in Kbps (1K = 1000);
 *     pbs    - Peak Burst Size in bytes;
 *
 */
typedef union
{
    struct
    {
        GT_U32 cir;
        GT_U32 cbs;
        GT_U32 ebs;
    } srTcmParams;
    struct
    {
        GT_U32 cir;
        GT_U32 cbs;
        GT_U32 pir;
        GT_U32 pbs;
    }trTcmParams;

} CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT;


/*
 * typedef: enum CPSS_DXCH_POLICER_REMARK_MODE_ENT
 *
 * Description: IP/MPLS Remark mode.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_REMARK_MODE_L2_E   - Use TC/UP for packets remarking.
 *    CPSS_DXCH_POLICER_REMARK_MODE_L3_E   - Use DSCP for IP packets remarking
 *                          or use EXP for MPLS packets remarking.
 */
typedef enum
{
    CPSS_DXCH_POLICER_REMARK_MODE_L2_E,
    CPSS_DXCH_POLICER_REMARK_MODE_L3_E
} CPSS_DXCH_POLICER_REMARK_MODE_ENT;


/*
 * typedef: struct CPSS_DXCH3_POLICER_ENTRY_METER_CFG_STC
 *
 * Description: Structure for Policer Entry Metering Part.
 *
 * Fields:
 *  countingEntryIndex      -   index of billing or policy counting entry.
 *  mngCounterSet           -   affiliation of the Policer Entry to the
 *                              Management Counter Set.
 *  meterColorMode          -   the meter color mode (blind/aware).
 *  meterMode               -   meter color marking mode: srTCM or trTCM.
 *  tokenBucketParams       -   Token Bucket Parameters.
 *  modifyUp                -   enable/disable modifying Drop Precedence.
 *                              Note: For xCat and above Egress, only Enable/Disable
 *                                    options are valid.
 *  modifyDscp              -   enable/disable modifying IP DCSP field.
 *                              Note: For xCat and above Egress, only Enable/Disable
 *                                    options are valid.
 *  modifyDp                -   enable/disable modifying DP field.
 *                              Note: Supported for xCat and above.
 *                                    Ignored by DxCh3.
 *                              Note: Only Enable/Disable options are valid.
 *                              Note: only for Egress Metering Entry.
 *  yellowPcktCmd           -   Policer commands for Non-Conforming Yellows
 *                              Packets.
 *                              Note: Relevant only for Ingress Metering Entry.
 *  redPcktCmd              -   Policer commands for Non-Conforming Red Packets.
 *  qosProfile              -   Policer Qos Profile. Relevant for non-conforming
 *                              packets when yellowPcktCmd or redPcktCmd is
 *                              "Qos profile mark by entry".
 *                              Note: Supported for xCat and above.
 *                                    Ignored by DxCh3.
 *                              Note: Relevant only for Ingress Metering Entry.
 *  remarkMode              -   Remark mode, controls the QoS remarking table
 *                              access for IP and MPLS packets.
 *                              Note: Supported for xCat and above.
 *                                    Ignored by DxCh3.
 *                              Note: only for Egress Metering Entry.
 *                              Note: Non-IP/MPLS packets always access the
 *                                    QoS remarking table using the L2 mode.
 *                              Note: In L2 mode, TC/UP selection is configured
 *                                    by cpssDxChPolicerEgressL2RemarkModelSet API.
 */
typedef struct
{
    GT_U32                                  countingEntryIndex;
    CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCounterSet;
    CPSS_POLICER_COLOR_MODE_ENT             meterColorMode;
    CPSS_DXCH3_POLICER_METER_MODE_ENT       meterMode;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  tokenBucketParams;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT   modifyUp;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT   modifyDscp;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT   modifyDp;
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT  yellowPcktCmd;
    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT  redPcktCmd;
    GT_U32                                  qosProfile;
    CPSS_DXCH_POLICER_REMARK_MODE_ENT       remarkMode;
} CPSS_DXCH3_POLICER_METERING_ENTRY_STC;


/*
 * typedef: enum CPSS_DXCH_POLICER_STAGE_TYPE_ENT
 *
 * Description: Enumeration for Policer Unit Type.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_STAGE_INGRESS_0_E   - Ingress Policer stage #0.
 *    CPSS_DXCH_POLICER_STAGE_INGRESS_1_E   - Ingress Policer stage #1.
 *    CPSS_DXCH_POLICER_STAGE_EGRESS_E      - Egress Policer stage.
 */
typedef enum
{
    CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
    CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,
    CPSS_DXCH_POLICER_STAGE_EGRESS_E
} CPSS_DXCH_POLICER_STAGE_TYPE_ENT;


/*
 * typedef: enum CPSS_DXCH_POLICER_COUNTING_MODE_ENT
 *
 * Description: Enumeration for Policer Counting Mode.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_COUNTING_DISABLE_E         - Counting is disabled.
 *    CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E   - Billing (per color counting)
 *                                  or IPFIX counting.
 *                                  Counting is triggered by Policy Action Entry
 *                                  or by Metering Entry.
 *                                  IPFIX counting is not relevant for xCat2.
 *    CPSS_DXCH_POLICER_COUNTING_POLICY_E          - Policy activity counting mode:
 *                                  packet counters only. Counting is triggered by
 *                                  Policy Action Entry.
 *    CPSS_DXCH_POLICER_COUNTING_VLAN_E            - VLAN counting mode: packet or
 *                                  byte counters.
 *                                  Counting is triggered for every packet according
 *                                  to VLAN-ID.
 *                                  (APPLICABLE DEVICES: xCat; Lion; Lion2; Lion3)
 */
typedef enum
{
    CPSS_DXCH_POLICER_COUNTING_DISABLE_E,
    CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E,
    CPSS_DXCH_POLICER_COUNTING_POLICY_E,
    CPSS_DXCH_POLICER_COUNTING_VLAN_E
} CPSS_DXCH_POLICER_COUNTING_MODE_ENT;


/*
 * typedef: enum CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT
 *
 * Description:
 *      QoS parameter enumeration
 *
 * Enumerations:
 *      CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E - TC/UP Remark table.
 *      CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E - DSCP Remark table.
 *      CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E - EXP Remark table.
 *
 * COMMENTS:
 */
typedef enum {
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E,
    CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E
}CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT;

/*
 * typedef: CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT
 *
 * Description: Enumeration for selecting what algorithm is
 *              selected to convert the metering SW parameters
 *              to selected HW ones.
 *
 * Enumerations:
 *      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - HW metering
 *          parameters are configured such that highest granularity
 *          for CIR\PIR is selected. CBS, EBS and PBS are not used while
 *          choosing the rate type. This is the default method.
 *      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - HW metering
 *          parameters are configured such that most accurate CBS\PBS
 *          is selected, while the CIR\PIR configured value is in allowed
 *          range derived from the allowed deviation.
 *
 */
typedef enum {
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E,
    CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E
} CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT;

/*
 * Typedef: CPSS_DXCH_POLICER_QOS_PARAM_STC
 *
 * Description: The Quality of Service parameters.
 *
 * Fields:
 *    up    - IEEE 802.1p User Priority (APPLICABLE RANGES: 0..7)
 *    dscp  - IP DCSP field (APPLICABLE RANGES: 0..63)
 *    exp   - MPLS label EXP value (APPLICABLE RANGES: 0..7)
 *    dp    - Drop Precedence value: GREEN, YELLOW or RED.
 */
typedef struct
{
    GT_U32              up;
    GT_U32              dscp;
    GT_U32              exp;
    CPSS_DP_LEVEL_ENT   dp;
}CPSS_DXCH_POLICER_QOS_PARAM_STC;


/*
 * typedef: enum CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT
 *
 * Description: Global stage mode.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E   - Port-based metering is performed if
 *                          the relevant port is enabled for metering.
 *                          Port policing is done per device local source port
 *                          for the Ingress Policer and per device local egress
 *                          port for Egress Policer.
 *    CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E   - Flow-based metering is performed if
 *                          metering is enabled in Policy/TTI Action and
 *                          the pointer is taken from the TTI/Policy Action.

 */
typedef enum
{
    CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E,
    CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E
} CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT;



/*
 * typedef: enum CPSS_DXCH_POLICER_STORM_TYPE_ENT
 *
 * Description: Storm Types.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E        - Knowm Unicast
 *    CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E      - Unknowm Unicast
 *    CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E - Unregistered Multicast
 *    CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E   - Registered Multicast
 *    CPSS_DXCH_POLICER_STORM_TYPE_BC_E              - Broadcast
 *    CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E         - TCP-SYN
 */
typedef enum
{
    CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E,
    CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E,
    CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E,
    CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E,
    CPSS_DXCH_POLICER_STORM_TYPE_BC_E,
    CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E
} CPSS_DXCH_POLICER_STORM_TYPE_ENT;


/*
 * typedef: enum CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT
 *
 * Description: Enumeration for L2 packets remarking model.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_L2_REMARK_MODEL_TC_E  - Traffic Class is index in the
 *                                              QoS table.
 *    CPSS_DXCH_POLICER_L2_REMARK_MODEL_UP_E  - User Priority is index in the
 *                                              QoS table.
 */
typedef enum
{
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_TC_E,
    CPSS_DXCH_POLICER_L2_REMARK_MODEL_UP_E
} CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT;


#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E
#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E   CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E
#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E   CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_2_E
#define CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_3_E


/*
 * typedef: enum CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT
 *
 * Description: Enumeration for Memory control mode.
 *              Lower memory uses 256 entries. Upper memory uses the rest.
 *              In case of xCat2: memory 1 has 172 entries, memory 2 has
 *              84 entries and memory 3 has 28 entries.
 *
 * Enumerations:
 *    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E  - Policer 0
 *                  uses the upper memory and Policer 1 uses the lower memory.
 *                  In case of xCat2: Policer 1 uses two memories (1 and 2),
 *                  Policer 0 uses one memory (3).
 *    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E  - Policer 0
 *                  uses both memories.
 *                  In case of xCat2: Policer 0 uses all three memories.
 *    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_2_E  - Policer 1
 *                  uses both memories.
  *                  In case of xCat2: Policer 1 uses all three memories.
 *    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_3_E - Policer 1
 *                  uses the upper memory and Policer 0 uses the lower memory.
 *                  In case of xCat2: Policer 0 uses two memories (1 and 2),
 *                  Policer 1 uses one memory (3).
 *    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E - Policer 0 uses one memory (1),
 *                  Policer 1 uses two memories (2 and 3).
 *                  Applicable devices: xCat2
 *                  Note: In this mode, when stage mode is Port, a compressed
 *                        address select should be configured:
 *                        CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
 *                        by cpssDxChPolicerPortModeAddressSelectSet().
 *    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E - Policer 1 uses one memory (1),
 *                  Policer 0 uses two memories (2 and 3).
 *                  Applicable devices: xCat2
 *                  Note: In this mode, when stage mode is Port, a compressed
 *                        address select should be configured:
 *                        CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
 *                        by cpssDxChPolicerPortModeAddressSelectSet().
 */
typedef enum
{
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E,
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E,
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_2_E,
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_3_E,
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E,
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E
} CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT;


/*
 * typedef: enum CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT
 *
 * Description: Packet/Byte Vlan counter mode.
 *
 *
 * Enumerations:
 *  CPSS_DXCH_POLICER_VLAN_CNTR_MODE_BYTES_E   - Bytes Vlan counter mode.
 *
 *  CPSS_DXCH_POLICER_VLAN_CNTR_MODE_PACKETS_E - Packets Vlan counter mode.
 */
typedef  enum{
    CPSS_DXCH_POLICER_VLAN_CNTR_MODE_BYTES_E,
    CPSS_DXCH_POLICER_VLAN_CNTR_MODE_PACKETS_E
} CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT
 *
 * Description: Port Mode Address Select type.
 *
 *
 * Enumerations:
 *  CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E   - The address is {index,port_num}.
 *
 *  CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E - Compressed;
 *          The address is {port_num,index}.
 */
typedef enum{
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E,
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E
} CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT;


/*
 * Typedef: CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC
 *
 * Description: Defines policer trigger entry.
 *
 * Fields:
 *    meteringEnable        - GT_TRUE: Metering is performed
 *                            GT_FALSE: Metering is not performed
 *    countingEnable        - GT_TRUE: Counting is performed
 *                            GT_FALSE: Counting is not performed
 *    policerIndex          - Policer pointer that is used for metering/counting.
 *    ucKnownEnable         - GT_TRUE: Counting/metering is enabled for unicast 
 *                                     known traffic.
 *                            GT_FALSE: Counting/metering is disabled for unicast 
 *                                     known traffic.
 *    ucKnownOffset         - Offset used for unicast known traffic (APPLICABLE RANGES: 0..6).
 *                            The offset is compared to policerIndex.
 *                            Note: Relevant only when  ucKnownEnable = GT_TRUE.
 *                            Note: In Egress stage, traffic type is ignored, 
 *                                  thus the offset assumed to be 0x0.
 *    ucUnknownEnable       - GT_TRUE: Counting/metering is enabled for unicast 
 *                                     unknown traffic.
 *                            GT_FALSE: Counting/metering is disabled for unicast 
 *                                      unknown traffic.
 *    ucUnknownOffset       - Offset used for unicast unknown traffic (APPLICABLE RANGES: 0..6).
 *                            The offset is compared to policerIndex.
 *                            Note: Relevant only when  ucUnknownEnable = GT_TRUE.
 *                            Note: In Egress stage, traffic type is ignored, 
 *                                  thus the offset assumed to be 0x0.
 *    mcRegisteredEnable    - GT_TRUE: Counting/metering is enabled for  
 *                                     registered multicast traffic.
 *                            GT_FALSE: Counting/metering is disabled for  
 *                                      registered multicast traffic.
 *    mcRegisteredOffset    - Offset used for registered multicast traffic (APPLICABLE RANGES: 0..6).
 *                            The offset is compared to policerIndex.
 *                            Note: Relevant only when  mcRegisteredEnable = GT_TRUE.
 *                            Note: In Egress stage, traffic type is ignored, 
 *                                  thus the offset assumed to be 0x0.
 *    mcUnregisteredEnable  - GT_TRUE: Counting/metering is enabled for  
 *                                     unregistered multicast traffic.
 *                            GT_FALSE: Counting/metering is disabled for  
 *                                      unregistered multicast traffic.
 *    mcUnregisteredOffset  - Offset used for unregistered multicast traffic (APPLICABLE RANGES: 0..6).
 *                            The offset is compared to policerIndex.
 *                            Note: Relevant only when  mcUnregisteredEnable = GT_TRUE.
 *                            Note: In Egress stage, traffic type is ignored, 
 *                                  thus the offset assumed to be 0x0.
 *    bcEnable              - GT_TRUE: Counting/metering is enabled for  
 *                                     broadcast traffic.
 *                            GT_FALSE: Counting/metering is disabled for  
 *                                      broadcast traffic.
 *    bcOffset              - Offset used for broadcast traffic (APPLICABLE RANGES: 0..6).
 *                            The offset is compared to policerIndex.
 *                            Note: Relevant only when  bcEnable = GT_TRUE.
 *                            Note: In Egress stage, traffic type is ignored, 
 *                                  thus the offset assumed to be 0x0.
 *    tcpSynEnable          - GT_TRUE: Counting/metering is enabled for  
 *                                     TCP SYN traffic.
 *                            GT_FALSE: Counting/metering is disabled for  
 *                                      TCP SYN traffic.
 *    tcpSynOffset          - Offset used for TCP SYN traffic (APPLICABLE RANGES: 0..6).
 *                            The offset is compared to policerIndex.
 *                            Note: Relevant only when  tcpSynEnable = GT_TRUE.
 *                            Note: In Egress stage, traffic type is ignored, 
 *                                  thus the offset assumed to be 0x0.
 */
typedef struct
{
    GT_BOOL             meteringEnable;
    GT_BOOL             countingEnable;
    GT_U32              policerIndex;
    GT_BOOL             ucKnownEnable;
    GT_U32              ucKnownOffset;
    GT_BOOL             ucUnknownEnable;
    GT_U32              ucUnknownOffset;
    GT_BOOL             mcRegisteredEnable;
    GT_U32              mcRegisteredOffset;
    GT_BOOL             mcUnregisteredEnable;
    GT_U32              mcUnregisteredOffset;
    GT_BOOL             bcEnable;
    GT_U32              bcOffset;
    GT_BOOL             tcpSynEnable;
    GT_U32              tcpSynOffset;
}CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC;

/*******************************************************************************
* cpssDxCh3PolicerMeteringEnableSet
*
* DESCRIPTION:
*       Enables or disables metering per device.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       enable          - Enable/disable metering:
*                         GT_TRUE  - metering is enabled on the device.
*                         GT_FALSE - metering is disabled on the device.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringEnableGet
*
* DESCRIPTION:
*       Gets device metering status (Enable/Disable).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       enablePtr       - pointer to Enable/Disable metering:
*                         GT_TRUE  - metering is enabled on the device.
*                         GT_FALSE - metering is disabled on the device.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxCh3PolicerBillingCountingEnableSet
*
* DESCRIPTION:
*       Enables or disables Billing Counting.
*       Billing is the process of counting the amount of traffic that
*       belongs to a flow and can be activated only by metering.
*       When metering is activated per source port, Billing counting
*       cannot be enabled.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*       enable          - enable/disable Billing Counting:
*                         GT_TRUE  - Billing Counting is enabled on the device.
*                         GT_FALSE - Billing Counting is disabled on the device.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerBillingCountingEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/*******************************************************************************
* cpssDxCh3PolicerBillingCountingEnableGet
*
* DESCRIPTION:
*       Gets device Billing Counting status (Enable/Disable).
*       Billing is the process of counting the amount of traffic that
*       belongs to a flow and can be activated only by metering.
*       When metering is activated per source port, Billing counting
*       cannot be enabled.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*
* OUTPUTS:
*       enablePtr       - pointer to Enable/Disable Billing Counting:
*                         GT_TRUE  - Billing Counting is enabled on the device.
*                         GT_FALSE - Billing Counting is disabled on the device.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerBillingCountingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/*******************************************************************************
* cpssDxChPolicerCountingModeSet
*
* DESCRIPTION:
*       Configures counting mode.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above
*                         devices and ignored by DxCh3.
*       mode           -  counting mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or mode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_COUNTING_MODE_ENT      mode
);

/*******************************************************************************
* cpssDxChPolicerCountingModeGet
*
* DESCRIPTION:
*       Gets the couning mode.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above
*                         devices and ignored by DxCh3.
*
* OUTPUTS:
*       modePtr         - pointer to Counting mode.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerCountingModeGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_COUNTING_MODE_ENT     *modePtr
);

/*******************************************************************************
* cpssDxCh3PolicerBillingCountersModeSet
*
* DESCRIPTION:
*      Sets the Billing Counters resolution.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*       cntrMode        - Billing Counters resolution: 1 Byte,
*                         16 Byte or packet.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or cntrMode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerBillingCountersModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT     cntrMode
);


/*******************************************************************************
* cpssDxCh3PolicerBillingCountersModeGet
*
* DESCRIPTION:
*      Gets the Billing Counters resolution.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*
* OUTPUTS:
*       cntrModePtr     - pointer to the Billing Counters resolution:
*                         1 Byte, 16 Byte or packet.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerBillingCountersModeGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT    *cntrModePtr
);

/*******************************************************************************
* cpssDxCh3PolicerPacketSizeModeSet
*
* DESCRIPTION:
*       Sets metered Packet Size Mode that metering and billing is done
*       according to.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum             - physical device number.
*       stage              - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                            Stage type is significant for xCat and above devices
*                            and ignored by DxCh3.
*       packetSize         - Type of packet size:
*                               - L3 datagram size only (this does not include
*                                 the L2 header size and packets CRC).
*                               - L2 packet length including.
*                               - L1 packet length including
*                                 (preamble + IFG + FCS).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or packetSize.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerPacketSizeModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT    packetSize
);

/*******************************************************************************
* cpssDxCh3PolicerPacketSizeModeGet
*
* DESCRIPTION:
*       Gets metered Packet Size Mode that metering and billing is done
*       according to.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       packetSizePtr   - pointer to the Type of packet size:
*                           - L3 datagram size only (this does not include
*                             the L2 header size and packets CRC).
*                           - L2 packet length including.
*                           - L1 packet length including (preamble + IFG + FCS).
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerPacketSizeModeGet
(
    IN  GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    OUT CPSS_POLICER_PACKET_SIZE_MODE_ENT    *packetSizePtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeterResolutionSet
*
* DESCRIPTION:
*       Sets metering algorithm resolution: packets per
*       second or bytes per second.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       resolution      - packet/Byte based Meter resolution.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or resolution.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeterResolutionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT  resolution
);

/*******************************************************************************
* cpssDxCh3PolicerMeterResolutionGet
*
* DESCRIPTION:
*       Gets metering algorithm resolution: packets per
*       second or bytes per second.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       resolutionPtr   - pointer to the Meter resolution: packet or Byte based.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeterResolutionGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT *resolutionPtr
);

/*******************************************************************************
* cpssDxCh3PolicerDropTypeSet
*
* DESCRIPTION:
*       Sets the Policer out-of-profile drop command type.
*       This setting controls if non-conforming dropped packets
*       (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0 or Ingress #1.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       dropType        - Policer Drop Type: Soft or Hard.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, dropType or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerDropTypeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_DROP_MODE_TYPE_ENT              dropType
);

/*******************************************************************************
* cpssDxCh3PolicerDropTypeGet
*
* DESCRIPTION:
*       Gets the Policer out-of-profile drop command type.
*       This setting controls if non-conforming dropped packets
*       (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0 or Ingress #1.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       dropTypePtr     - pointer to the Policer Drop Type: Soft or Hard.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerDropTypeGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropTypePtr
);

/*******************************************************************************
* cpssDxCh3PolicerCountingColorModeSet
*
* DESCRIPTION:
*       Sets the Policer color counting mode.
*       The Color Counting can be done according to the packet's
*       Drop Precedence or Conformance Level.
*       This affects both the Billing and Management counters.
*       If the packet was subject to remarking, the drop precedence used
*       here is AFTER remarking.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       mode            - Color counting mode: Drop Precedence or
*                         Conformance Level.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or mode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerCountingColorModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT  mode
);

/*******************************************************************************
* cpssDxCh3PolicerCountingColorModeGet
*
* DESCRIPTION:
*       Gets the Policer color counting mode.
*       The Color Counting can be done according to the packet's
*       Drop Precedence or Conformance Level.
*       This affects both the Billing and Management counters.
*       If the packet was subject to remarking, the drop precedence used
*       here is AFTER remarking.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       modePtr         - pointer to the color counting mode:
*                         Drop Precedence or Conformance Level.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerCountingColorModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT     *modePtr
);

/*******************************************************************************
* cpssDxCh3PolicerManagementCntrsResolutionSet
*
* DESCRIPTION:
*       Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       cntrSet         - Management Counters Set (APPLICABLE RANGES: 0..2).
*       cntrResolution  - Management Counters resolution: 1 or 16 Bytes.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or
*                                     Management Counters Set or cntrResolution.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerManagementCntrsResolutionSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT          cntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT   cntrResolution
);

/*******************************************************************************
* cpssDxCh3PolicerManagementCntrsResolutionGet
*
* DESCRIPTION:
*       Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       cntrSet         - Management Counters Set (APPLICABLE RANGES: 0..2).
*
* OUTPUTS:
*       cntrResolutionPtr   - pointer to the Management Counters
*                             resolution: 1 or 16 Bytes.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or  Mng Counters Set.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerManagementCntrsResolutionGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT         cntrSet,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT  *cntrResolutionPtr
);

/*******************************************************************************
* cpssDxCh3PolicerPacketSizeModeForTunnelTermSet
*
* DESCRIPTION:
*      Sets size mode for metering and counting of tunnel terminated packets.
*      The Policer provides the following modes to define packet size:
*       - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*       - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* INPUTS:
*       devNum              - physical device number.
*       stage               - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                             Stage type is significant for xCat and above devices
*                             and ignored by DxCh3.
*       ttPacketSizeMode    - Tunnel Termination Packet Size Mode.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or ttPacketSizeMode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  ttPacketSizeMode
);

/*******************************************************************************
* cpssDxCh3PolicerPacketSizeModeForTunnelTermGet
*
* DESCRIPTION:
*      Gets size mode for metering and counting of tunnel terminated packets.
*      The Policer provides the following modes to define packet size:
*       - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*       - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* INPUTS:
*       devNum              - physical device number.
*       stage               - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                             Stage type is significant for xCat and above devices
*                             and ignored by DxCh3.
*
* OUTPUTS:
*       ttPacketSizeModePtr     - pointer to the Tunnel Termination
*                                 Packet Size Mode.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  *ttPacketSizeModePtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet
*
* DESCRIPTION:
*       Enables or disables the metering Auto Refresh Scan mechanism.
*
*       If no traffic passes through a meter (long periods of "silence") the
*       meter state variables do not get refreshed. This can cause a problem of
*       state misinterpreted once the flow traffic is restarted, because the
*       timers can have already wrapped around.
*       To avoid this problem the software must initiate a refresh transaction
*       on the meter every ten minutes.
*
*       This CPU intensive metering refresh maintanance can be offloaded by
*       Auto Refresh Scan mechanism.
*       The range of Auto Refresh Scan is configurable by the
*       cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*       The Auto Refresh Scan Rate can be controlled by setting time interval
*       between two refresh access to metering table. The time interval is
*       configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*       enable          - Enable/disable Auto Refresh Scan mechanism:
*                         GT_TRUE  - enable Auto Refresh scan mechanism on
*                                    the device.
*                         GT_FALSE - disable Auto Refresh scan mechanism on
*                                    the device.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet
(
    IN GT_U8                                devNum,
    IN GT_BOOL                              enable
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet
*
* DESCRIPTION:
*       Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*
*       If no traffic passes through a meter (long periods of "silence") the
*       meter state variables do not get refreshed. This can cause a problem of
*       state misinterpreted once the flow traffic is restarted, because the
*       timers can have already wrapped around.
*       To avoid this problem the software must initiate a refresh transaction
*       on the meter every ten minutes.
*
*       This CPU intensive metering refresh maintanance can be offloaded by
*       Auto Refresh Scan mechanism.
*       The range of Auto Refresh Scan is configurable by the
*       cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*       The Auto Refresh Scan Rate can be controlled by setting time interval
*       between two refresh access to metering table. The time interval is
*       configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*
* OUTPUTS:
*       enablePtr       - pointer on Auto Refresh Scan mechanism status
*                         (Enable/Disable) :
*                         GT_TRUE  - Auto Refresh scan mechanism is enabled.
*                         GT_FALSE - Auto Refresh scan mechanism is disabled.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringAutoRefreshRangeSet
*
* DESCRIPTION:
*       Sets Metering Refresh Scan address range (Start and Stop addresses).
*       The Auto Refresh Scan mechanism is enabled by the
*       cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*       The Auto Refresh Scan Rate can be controlled by setting time interval
*       between two refresh access to metering table. The time interval is
*       configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum              - physical device number.
*       startAddress        - beginning of the address range to be scanned
*                             by the Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
*       stopAddress         - end of the address range to be scanned by the
*                             Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or illegal values
*                                     of startAddress and stopAddress.
*       GT_OUT_OF_RANGE             - on out of range of startAddress
*                                     or stopAddress.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshRangeSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               startAddress,
    IN GT_U32                               stopAddress
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringAutoRefreshRangeGet
*
* DESCRIPTION:
*       Gets Metering Refresh Scan address range (Start and Stop addresses).
*       The Auto Refresh Scan mechanism is enabled by the
*       cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*       The Auto Refresh Scan Rate can be controlled by setting time interval
*       between two refresh access to metering table. The time interval is
*       configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum              - physical device number.
*
* OUTPUTS:
*       startAddressPtr     - pointer to the beginning address of Refresh
*                             Scan address range.
*       stopAddressPtr      - pointer to the end address of Refresh Scan
*                             address range.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointers.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshRangeGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *startAddressPtr,
    OUT GT_U32                              *stopAddressPtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringAutoRefreshIntervalSet
*
* DESCRIPTION:
*       Sets the time interval between two refresh access to metering table.
*       The Auto Refresh Scan mechanism is enabled by the
*       cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*       The range of Auto Refresh Scan is configurable by the
*       cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*       interval        - time interval between refresh of two entries
*                         in micro seconds. Upon 270 Mhz core clock value
*                         of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_OUT_OF_RANGE             - on out of range of interval value.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshIntervalSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               interval
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringAutoRefreshIntervalGet
*
* DESCRIPTION:
*       Gets the time interval between two refresh access to metering table.
*       The Auto Refresh Scan mechanism is enabled by the
*       cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*       The range of Auto Refresh Scan is configurable by the
*       cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - physical device number.
*
* OUTPUTS:
*       intervalPtr     - pointer to the time interval between refresh of two
*                         entries in micro seconds. Upon 270 Mhz core clock
*                         value of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshIntervalGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *intervalPtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringEntryRefresh
*
* DESCRIPTION:
*       Refresh the Policer Metering Entry.
*       Meter Bucket's state parameters refreshing are needed in order to
*       prevent mis-behavior due to wrap around of timers.
*
*       The wrap around problem can occur when there are long periods of
*       'silence' on the flow and the bucket's state parameters do not get
*       refreshed (meter is in the idle state). This causes a problem once the
*       flow traffic is re-started since the timers might have already wrapped
*       around which can cause a situation in which the bucket's state is
*       mis-interpreted and the incoming packet is marked as non-conforming
*       even though the bucket was actually supposed to be full.
*       To prevent this from happening the CPU needs to trigger a meter
*       refresh transaction at least once every 10 minutes per meter.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above
*                         devices and ignored by DxCh3.
*       entryIndex      - index of Policer Metering Entry.
*                         Range: see datasheet for specific device.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_TIMEOUT                  - on time out of IPLR Table indirect access.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or entryIndex.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              entryIndex
);

/*******************************************************************************
* cpssDxCh3PolicerPortMeteringEnableSet
*
* DESCRIPTION:
*       Enables or disables a port metering trigger for packets
*       arriving on this port.
*       When feature is enabled the meter entry index is a port number.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       portNum         - port number (including the CPU port).
*       enable          - Enable/Disable per-port metering for packets arriving
*                         on this port:
*                         GT_TRUE  - Metering is triggered on specified port.
*                         GT_FALSE - Metering isn't triggered on specified port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or portNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerPortMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
);

/*******************************************************************************
* cpssDxCh3PolicerPortMeteringEnableGet
*
* DESCRIPTION:
*       Gets port status (Enable/Disable) of metering for packets
*       arriving on this port.
*       When feature is enabled the meter entry index is a port number.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       portNum         - port number (including the CPU port).
*
* OUTPUTS:
*       enablePtr       - pointer on per-port metering status (Enable/Disable)
*                         for packets arriving on specified port:
*                         GT_TRUE  - Metering is triggered on specified port.
*                         GT_FALSE - Metering isn't triggered on specified port.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or portNum.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerPortMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxCh3PolicerMruSet
*
* DESCRIPTION:
*       Sets the Policer Maximum Receive Unit size.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       mruSize         - Policer MRU value in bytes, (APPLICABLE RANGES: 0..0xFFFF).
*                         This value is used in the metering algorithm.
*                         When the number of bytes in the bucket is lower than
*                         this value a packet is marked as non conforming.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_OUT_OF_RANGE             - on mruSize out of range.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMruSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               mruSize
);

/*******************************************************************************
* cpssDxCh3PolicerMruGet
*
* DESCRIPTION:
*       Gets the Policer Maximum Receive Unit size.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       mruSizePtr      - pointer to the Policer MRU value in bytes.
*                         This value is used in the metering algorithm.
*                         When the number of bytes in the bucket is lower than
*                         this value a packet is marked as non conforming.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMruGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *mruSizePtr
);

/*******************************************************************************
* cpssDxCh3PolicerErrorGet
*
* DESCRIPTION:
*       Gets address and type of Policer Entry that had an ECC error.
*       This information available if error is happened and not read till now.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       entryTypePtr    - pointer to the Type of Entry (Metering or Counting)
*                         that had an error.
*       entryAddrPtr    - pointer to the Policer Entry that had an error.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_EMPTY                    - on missing error information.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerErrorGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
);

/*******************************************************************************
* cpssDxCh3PolicerErrorCounterGet
*
* DESCRIPTION:
*       Gets the value of the Policer ECC Error Counter.
*       The Error Counter is a free-running non-sticky 8-bit read-only
*       counter.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       cntrValuePtr    - pointer to the Policer ECC Error counter value.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
);

/*******************************************************************************
* cpssDxCh3PolicerManagementCountersSet
*
* DESCRIPTION:
*       Sets the value of specified Management Counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       mngCntrSet      - Management Counters Set (APPLICABLE RANGES: 0..2).
*       mngCntrType     - Management Counters Type (GREEN, YELLOW, RED, DROP).
*       mngCntrPtr      - pointer to the Management Counters Entry must be set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage or mngCntrType
*                                     or Management Counters Set number.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
);

/*******************************************************************************
* cpssDxCh3PolicerManagementCountersGet
*
* DESCRIPTION:
*       Gets the value of specified Management Counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       mngCntrSet      - Management Counters Set (APPLICABLE RANGES: 0..2).
*       mngCntrType     - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* OUTPUTS:
*       mngCntrPtr      - pointer to the requested Management Counters Entry.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage or mngCntrType
*                                     or Management Counters Set number.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringEntrySet
*
* DESCRIPTION:
*      Sets Metering Policer Entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Metering Entry.
*                         Range: see datasheet for specific device.
*       entryPtr        - pointer to the metering policer entry going to be set.
*
* OUTPUTS:
*       tbParamsPtr     - pointer to actual policer token bucket parameters.
*                         The token bucket parameters are returned as output
*                         values. This is due to the hardware rate resolution,
*                         the exact rate or burst size requested may not be
*                         honored. The returned value gives the user the
*                         actual parameters configured in the hardware.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on hardware error.
*       GT_TIMEOUT                  - on time out of Policer Tables
*                                     indirect access.
*       GT_BAD_PARAM                - wrong devNum, stage or entryIndex or
*                                     entry parameters.
*       GT_OUT_OF_RANGE             - on Billing Entry Index out of range.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
);

/*******************************************************************************
* cpssDxCh3PolicerMeteringEntryGet
*
* DESCRIPTION:
*      Gets Metering Policer Entry configuration.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Metering Entry.
*                         Range: see datasheet for specific device.
*
* OUTPUTS:
*       entryPtr        - pointer to the requested metering policer entry.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - wrong devNum or stage or entryIndex.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*       GT_BAD_STATE                - on bad value in a entry.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
);

/*******************************************************************************
* cpssDxCh3PolicerEntryMeterParamsCalculate
*
* DESCRIPTION:
*      Calculates Token Bucket parameters in the Application format without
*      HW update.
*      The token bucket parameters are returned as output values. This is
*      due to the hardware rate resolution, the exact rate or burst size
*      requested may not be honored. The returned value gives the user the
*      actual parameters configured in the hardware.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       tbInParamsPtr   - pointer to Token bucket input parameters.
*       meterMode       - Meter mode (SrTCM or TrTCM).
*
* OUTPUTS:
*       tbOutParamsPtr  - pointer to Token bucket output paramters.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_BAD_PARAM                - on wrong devNum or meterMode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  CPSS_DXCH3_POLICER_METER_MODE_ENT       meterMode,
    IN  CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbInParamsPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbOutParamsPtr
);

/*******************************************************************************
* cpssDxCh3PolicerBillingEntrySet
*
* DESCRIPTION:
*       Sets Policer Billing Counters.
*       Billing is the process of counting the amount of traffic that belongs
*       to a flow. This process can be activated only by metering and is used
*       either for statistical analysis of a flow's traffic or for actual
*       billing.
*       The billing counters are free-running no-sticky counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Billing Counters Entry.
*                         Range: see datasheet for specific device.
*       billingCntrPtr  - pointer to the Billing Counters Entry.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_TIMEOUT                  - on time out upon counter reset by
*                                     indirect access.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or entryIndex.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*           - disconnect entryes from traffic
*           - call cpssDxChPolicerCountingWriteBackCacheFlush
*           - set new values
*           - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/*******************************************************************************
* cpssDxCh3PolicerBillingEntryGet
*
* DESCRIPTION:
*       Gets Policer Billing Counters.
*       Billing is the process of counting the amount of traffic that belongs
*       to a flow. This process can be activated only by metering and is used
*       either for statistical analysis of a flow's traffic or for actual
*       billing.
*       The billing counters are free-running no-sticky counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Billing Counters Entry.
*                         Range: see datasheet for specific device.
*       reset           - reset flag:
*                         GT_TRUE  - performing read and reset atomic operation.
*                         GT_FALSE - performing read counters operation only.
*
* OUTPUTS:
*       billingCntrPtr  - pointer to the Billing Counters Entry.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_TIMEOUT                  - on time out of IPLR Table indirect access.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or entryIndex.
*       GT_BAD_STATE                - on counter entry type mismatch.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/*******************************************************************************
* cpssDxCh3PolicerQosRemarkingEntrySet
*
* DESCRIPTION:
*       Sets Policer Relative Qos Remarking Entry.
*       The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*       assigned to Out of profile packets with the Yellow and Red colors.
*       According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*       are extracted from the Qos Profile Table Entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum                      - device number.
*       stage                       - Policer Stage type: Ingress #0 or Ingress #1.
*                                     Stage type is significant for xCat and above devices
*                                     and ignored by DxCh3.
*       qosProfileIndex             - index of Qos Remarking Entry will be set.
*       yellowQosTableRemarkIndex   - QoS profile (index in the Qos Table)
*                                     assigned to Out-Of-Profile packets with
*                                     the Yellow color. (APPLICABLE RANGES: 0..127).
*       redQosTableRemarkIndex      - QoS profile (index in the Qos Table)
*                                     assigned to Out-Of-Profile packets with
*                                     the Red color. (APPLICABLE RANGES: 0..127).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or qosProfileIndex.
*       GT_OUT_OF_RANGE             - on yellowQosTableRemarkIndex and
*                                     redQosTableRemarkIndex out of range.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       Supported only for Ingress Policer.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerQosRemarkingEntrySet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    IN GT_U32                               yellowQosTableRemarkIndex,
    IN GT_U32                               redQosTableRemarkIndex
);

/*******************************************************************************
* cpssDxCh3PolicerQosRemarkingEntryGet
*
* DESCRIPTION:
*       Gets Policer Relative Qos Remarking Entry.
*       The Qos Remarking Entry contains two indexes [0..127] of Qos Profile
*       assigned to Out of profile packets with the Yellow and Red colours.
*       According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*       are extracted from the Qos Profile Table Entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum              - device number.
*       stage               - Policer Stage type: Ingress #0 or Ingress #1.
*                             Stage type is significant for xCat and above devices
*                             and ignored by DxCh3.
*       qosProfileIndex     - index of requested Qos Remarking Entry.
*
* OUTPUTS:
*       yellowQosTableRemarkIndexPtr    - pointer to the QoS profile (index in
*                                         the Qos Table) assigned to
*                                         Out-Of-Profile packets with the
*                                         Yellow color. (APPLICABLE RANGES: 0..127).
*       redQosTableRemarkIndexPtr       - pointer to the QoS profile (index in
*                                         the Qos Table) assigned to
*                                         Out-Of-Profile packets with the Red
*                                         color. (APPLICABLE RANGES: 0..127).
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or qosProfileIndex.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       Supported only for Ingress Policer.
*
*******************************************************************************/
GT_STATUS cpssDxCh3PolicerQosRemarkingEntryGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    OUT GT_U32                              *yellowQosTableRemarkIndexPtr,
    OUT GT_U32                              *redQosTableRemarkIndexPtr
);

/*******************************************************************************
* cpssDxChPolicerEgressQosRemarkingEntrySet
*
* DESCRIPTION:
*       Sets Egress Policer Re-Marking table Entry.
*       Re-marking is the process of assigning new QoS parameters to the
*       packet, at the end of the metering process, based on the
*       Conformance Level the packet is assigned by metering process.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum              - device number.
*       remarkTableType     - Remark table type: DSCP, EXP or TC/UP.
*       remarkParamValue    - QoS parameter value.
*       confLevel           - Conformance Level: Green, Yellow, Red.
*       qosParamPtr         - pointer to the Re-Marking Entry going to be set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success.
*       GT_HW_ERROR         - on hardware error.
*       GT_BAD_PARAM        - on wrong devNum, remarkTableType,
*                                remarkParamValue or confLevel.
*                              -------------------
*                             |QoS param | Range  |
*                             |----------|--------|
*                             |  DSCP    | [0:63] |
*                             |  TC/UP   | [0:7]  |
*                             |  EXP     | [0:7]  |
*                              -------------------
*       GT_BAD_PTR                               - on NULL pointer.
*       GT_OUT_OF_RANGE     - on QoS parameter out of range
*       GT_NOT_APPLICABLE_DEVICE                 - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerEgressQosRemarkingEntrySet
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN GT_U32                                       remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                            confLevel,
    IN CPSS_DXCH_POLICER_QOS_PARAM_STC              *qosParamPtr
);

/*******************************************************************************
* cpssDxChPolicerEgressQosRemarkingEntryGet
*
* DESCRIPTION:
*       Gets Egress Policer Re-Marking table Entry.
*       Re-marking is the process of assigning new QoS parameters to the
*       packet, at the end of the metering process, based on the
*       Conformance Level the packet is assigned by metering process.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum              - device number.
*       remarkTableType     - Remark table type: DSCP, EXP or TC/UP.
*       remarkParamValue    - QoS parameter value.
*       confLevel           - Conformance Level: Green, Yellow, Red.
*
* OUTPUTS:
*       qosParamPtr         - pointer to the requested Re-Marking Entry.
*
* RETURNS:
*       GT_OK               - on success.
*       GT_HW_ERROR         - on hardware error.
*       GT_BAD_PARAM        - on wrong devNum, remarkTableType,
*                                remarkParamValue or confLevel.
*                              -------------------
*                             |QoS param | Range  |
*                             |----------|--------|
*                             |  DSCP    | [0:63] |
*                             |  TC/UP   | [0:7]  |
*                             |  EXP     | [0:7]  |
*                              -------------------
*       GT_BAD_PTR                               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE                 - on not applicable device
*       GT_BAD_STATE                             - on bad value in a entry.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerEgressQosRemarkingEntryGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN  GT_U32                                      remarkParamValue,
    IN  CPSS_DP_LEVEL_ENT                           confLevel,
    OUT CPSS_DXCH_POLICER_QOS_PARAM_STC             *qosParamPtr
);

/*******************************************************************************
* cpssDxChPolicerStageMeterModeSet
*
* DESCRIPTION:
*       Sets Policer Global stage mode.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       mode            - Policer meter mode: FLOW or PORT.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or mode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerStageMeterModeSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      mode
);

/*******************************************************************************
* cpssDxChPolicerStageMeterModeGet
*
* DESCRIPTION:
*       Gets Policer Global stage mode.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* OUTPUTS:
*       modePtr         - (pointer to) Policer meter mode: FLOW or PORT.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerStageMeterModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      *modePtr
);

/*******************************************************************************
* cpssDxChPolicerMeteringOnTrappedPktsEnableSet
*
* DESCRIPTION:
*       Enable/Disable Ingress metering for Trapped packets.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0 or Ingress #1.
*       enable          - GT_TRUE - policing is executed on all packets (data
*                                   and control).
*                         GT_FALSE - policing is executed only on packets that
*                                    were asigned Forward (mirrored to analyzer
*                                    is inclusive) or Mirror to CPU commands by
*                                    previous processing stages.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or mode.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*******************************************************************************/
GT_STATUS cpssDxChPolicerMeteringOnTrappedPktsEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
);

/*******************************************************************************
* cpssDxChPolicerMeteringOnTrappedPktsEnableGet
*
* DESCRIPTION:
*       Get Ingress metering status (Enabled/Disabled) for Trapped packets.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0 or Ingress #1.
*
* OUTPUTS:
*       enablePtr       - GT_TRUE - policing is executed on all packets (data
*                                   and control).
*                         GT_FALSE - policing is executed only on packets that
*                                    were asigned Forward (mirrored to analyzer
*                                    is inclusive) or Mirror to CPU commands by
*                                    previous processing stages.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*******************************************************************************/
GT_STATUS cpssDxChPolicerMeteringOnTrappedPktsEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChPolicerPortStormTypeIndexSet
*
* DESCRIPTION:
*       Associates policing profile with source/target port and storm rate type.
*       There are up to four policers per port and every storm type can be
*       flexibly associated with any of the four policers.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       portNum         - port number (including the CPU port).
*       stormType       - storm type
*       index           - policer index (APPLICABLE RANGES: 0..3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage, stormType ot port.
*       GT_OUT_OF_RANGE             - on out of range of index.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortStormTypeIndexSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DXCH_POLICER_STORM_TYPE_ENT     stormType,
    IN GT_U32                               index
);


/*******************************************************************************
* cpssDxChPolicerPortStormTypeIndexGet
*
* DESCRIPTION:
*       Get policing profile for given source/target port and storm rate type.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       portNum         - port number (including the CPU port).
*       stormType       - storm type
*
* OUTPUTS:
*       indexPtr        - pointer to policer index.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage, stormType or portNum.
*       GT_OUT_OF_RANGE             - on out of range of port.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortStormTypeIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_POLICER_STORM_TYPE_ENT    stormType,
    OUT GT_U32                              *indexPtr
);

/*******************************************************************************
* cpssDxChPolicerEgressL2RemarkModelSet
*
* DESCRIPTION:
*       Sets Egress Policer L2 packets remarking model.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       model           - L2 packets remarking model. It defines QoS parameter,
*                         which used as index in the Qos table:
*                         User Priority or Traffic Class.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerEgressL2RemarkModelSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    model
);

/*******************************************************************************
* cpssDxChPolicerEgressL2RemarkModelGet
*
* DESCRIPTION:
*       Gets Egress Policer L2 packets remarking model.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*
* OUTPUTS:
*       modelPtr        - L2 packets remarking model. It defines QoS parameter,
*                         which used as index in the Qos table:
*                         User Priority or Traffic Class.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerEgressL2RemarkModelGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    *modelPtr
);

/*******************************************************************************
* cpssDxChPolicerEgressQosUpdateEnableSet
*
* DESCRIPTION:
*       The function enables or disables QoS remarking of conforming packets.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - physical device number.
*       enable  - Enable/disable Qos update for conforming packets:
*                 GT_TRUE  - Remark Qos parameters of conforming packets.
*                 GT_FALSE - Keep incoming Qos parameters of conforming packets.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        None
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerEgressQosUpdateEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChPolicerEgressQosUpdateEnableGet
*
* DESCRIPTION:
*       The function get QoS remarking status of conforming packets
*       (enable/disable).
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - physical device number.
*
* OUTPUTS:
*       enablePtr  - Enable/disable Qos update for conforming packets:
*                   GT_TRUE  - Remark Qos parameters of conforming packets.
*                   GT_FALSE - Keep incoming Qos parameters of conforming
*                              packets.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        None
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerEgressQosUpdateEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/*******************************************************************************
* cpssDxChPolicerVlanCntrSet
*
* DESCRIPTION:
*      Sets Policer VLAN Counters located in the Policer Counting Entry upon
*      enabled VLAN counting mode.
*      When working in VLAN Counting Mode each counter counts the number of
*      packets or bytes according to the VLAN counters mode set
*      by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* APPLICABLE DEVICES:
*        xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       vid             - VLAN ID. Bits [1:0] of VID are used to select the
*                         relevant 32 bit counter inside the Counting Entry.
*       cntrValue       - Packet/Byte counter indexed by the packet's VID.
*                         For counter reset this parameter should be nullified.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on disabled VLAN Counting.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM    - on wrong devNum or vid (VID range is limited by max
*                         number of Policer Counting Entries) or stage.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The VLAN counters are free-running no-sticky counters.
*
*       In order to set Vlan counting entry under traffic perform the following
*       algorithm:
*           - disconnect entryes from traffic
*           - call cpssDxChPolicerCountingWriteBackCacheFlush
*           - set new values
*           - connect entries back to traffic
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanCntrSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    IN  GT_U32                              cntrValue
);

/*******************************************************************************
* cpssDxChPolicerVlanCntrGet
*
* DESCRIPTION:
*      Gets Policer VLAN Counters located in the Policer Counting Entry upon
*      enabled VLAN counting mode.
*      When working in VLAN Counting Mode each counter counts the number of
*      packets or bytes according to the VLAN counters mode set
*      by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* APPLICABLE DEVICES:
*        xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       vid             - VLAN ID. Bits [1:0] of VID are used to select the
*                         relevant 32 bit counter inside the Counting Entry.
*
* OUTPUTS:
*       cntrValuePtr    - Packet/Byte counter indexed by the packet's VID.
*                         For counter reset this parameter should be nullified.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on disabled VLAN Counting.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM    - on wrong devNum or vid (VID range is limited by max
*                         number of Policer Counting Entries) or stage.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The VLAN counters are free-running no-sticky counters.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanCntrGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    OUT GT_U32                              *cntrValuePtr
);

/*******************************************************************************
* cpssDxChPolicerVlanCountingPacketCmdTriggerSet
*
* DESCRIPTION:
*       Enables or disables VLAN counting triggering according to the
*       specified packet command.
*
* APPLICABLE DEVICES:
*        xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0 or Ingress #1.
*       cmdTrigger      - The packet command trigger.
*                         The following packet command are supported:
*                           Forward (CPSS_PACKET_CMD_FORWARD_E),
*                           Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                           Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                           Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                           Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
*       enable          - Enable/Disable VLAN Counting according to the packet
*                         command trigger:
*                         GT_TRUE  - Enable VLAN counting on packet with
*                                    packet command trigger.
*                         GT_FALSE - Disable VLAN counting on packet with
*                                    packet command trigger.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         - on not supported egress direction
*
* COMMENTS:
*       All packet commands are enabled in the HW by default.
*       For egress policer the default trigger values shouldn't be changed.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanCountingPacketCmdTriggerSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_PACKET_CMD_ENT                  cmdTrigger,
    IN GT_BOOL                              enable
);

/*******************************************************************************
* cpssDxChPolicerVlanCountingPacketCmdTriggerGet
*
* DESCRIPTION:
*       Gets VLAN counting triggering status (Enable/Disable) according to the
*       specified packet command.
*
* APPLICABLE DEVICES:
*        xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0 or Ingress #1.
*       cmdTrigger      - The packet command trigger.
*                         The following packetcommand are supported:
*                           Forward (CPSS_PACKET_CMD_FORWARD_E),
*                           Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                           Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                           Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                           Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
*
* OUTPUTS:
*       enablePtr       - Pointer to Enable/Disable VLAN Counting according
*                         to the packet command trigger:
*                         GT_TRUE  - Enable VLAN counting on packet with
*                                    triggered packet command.
*                         GT_FALSE - Disable VLAN counting on packet with
*                                    triggered packet command.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       All packet commands are enabled in the HW by default.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  CPSS_PACKET_CMD_ENT                 cmdTrigger,
    OUT GT_BOOL                             *enablePtr
);

/*******************************************************************************
* cpssDxChPolicerVlanCountingModeSet
*
* DESCRIPTION:
*       Sets the Policer VLAN counters mode as byte or packet based.
*
* APPLICABLE DEVICES:
*        xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       mode            - VLAN counters mode: Byte or Packet based.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     mode
);

/*******************************************************************************
* cpssDxChPolicerVlanCountingModeGet
*
* DESCRIPTION:
*       Gets the Policer VLAN counters mode as byte or packet based.
*
* APPLICABLE DEVICES:
*        xCat; Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* OUTPUTS:
*       modePtr         - (pointer to) VLAN counters mode: Byte or Packet based.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_HW_ERROR              - on Hardware error.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanCountingModeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     *modePtr
);

/*******************************************************************************
* cpssDxChPolicerPolicyCntrSet
*
* DESCRIPTION:
*      Sets Policer Policy Counters located in the Policer Counting Entry upon
*      enabled Policy Counting mode.
*      When working in Policy Counting Mode each counter counts the number of
*      packets matching on a Policy rules.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       index           - index set by Policy Action Entry or Metering entry
*                         and belongs to the range from 0 up to maximal number
*                         of Policer Policy counters.
*                         Maximal number is number of policer entries for
*                         counting is number of counting entries
*                         multiplied by 8.
*                         Some indexes in the range are not valid. Not valid
*                         indexes are:
*                           - (index % 8) == 6
*                           - (index % 8) == 7
*                         e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*       cntrValue       - packets counter.
*                         For counter reset this parameter should be nullified.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on disabled Policy Counting.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM    - on wrong devNum or index (index range is limited by
*                         max number of Policer Policy counters).
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The Policy counters are free-running no-sticky counters.
*
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*           - disconnect entryes from traffic
*           - call cpssDxChPolicerCountingWriteBackCacheFlush
*           - set new values
*           - connect entries back to traffic
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
);

/*******************************************************************************
* cpssDxChPolicerPolicyCntrGet
*
* DESCRIPTION:
*      Gets Policer Policy Counters located in the Policer Counting Entry upon
*      enabled Policy Counting mode.
*      When working in Policy Counting Mode each counter counts the number of
*      packets matching on a a Policy rules.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       index           - index set by Policy Action Entry or Metering entry
*                         and belongs to the range from 0 up to maximal number
*                         of Policer Policy counters.
*                         Maximal number is number of policer entries for
*                         counting is number of counting entries
*                         multiplied by 8.
*                         Some indexes in the range are not valid. Not valid
*                         indexes are:
*                           - (index % 8) == 6
*                           - (index % 8) == 7
*                         e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* OUTPUTS:
*       cntrValuePtr    - Pointer to the packet counter.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on disabled Policy Counting.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum or index.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_TIMEOUT               - on time out.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The Policy counters are free-running no-sticky counters.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
);

/*******************************************************************************
* cpssDxChPolicerMemorySizeModeSet
*
* DESCRIPTION:
*      Sets internal table sizes and the way they are shared between the Ingress
*      policers.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       mode            - The mode in which internal tables are shared.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Mode change should not be done when meter or counter entries are in use.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerMemorySizeModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      mode
);

/*******************************************************************************
* cpssDxChPolicerMemorySizeModeGet
*
* DESCRIPTION:
*      Gets internal table sizes and the way they are shared between the Ingress
*      policers.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*
* OUTPUTS:
*       modePtr         - The mode in which internal tables are shared.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong input parameters.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerMemorySizeModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT     *modePtr
);

/*******************************************************************************
* cpssDxChPolicerCountingWriteBackCacheFlush
*
* DESCRIPTION:
*       Flush internal Write Back Cache (WBC) of counting entries.
*       The Policer implements internal Write Back Cache for frequent and rapid
*       update of counting entries.
*       Since the WBC holds the policer entries' data, the CPU must clear its
*       content when performing direct write access of the counting entries by
*       one of following functions:
*        - cpssDxCh3PolicerBillingEntrySet
*        - cpssDxChPolicerVlanCntrSet
*        - cpssDxChPolicerPolicyCntrSet
*       The policer counting should be disabled before flush of WBC if direct
*       write access is performed under traffic. And policer counting should be
*       enabled again after finish of write access.
*       The cpssDxChPolicerCountingModeSet may be used to disable/enable
*       the policer counting.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong devNum or stage.
*       GT_HW_ERROR              - on Hardware error.
*       GT_TIMEOUT               - on time out.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       To use the listed above APIs (cpssDxCh3PolicerBillingEntrySet,
*       cpssDxChPolicerVlanCntrSet and cpssDxChPolicerPolicyCntrSet) under
*       traffic, perform the following algorithm:
*           - disconnect entryes from traffic
*           - call cpssDxChPolicerCountingWriteBackCacheFlush
*           - set new values
*           - connect entries back to traffic

*******************************************************************************/
GT_STATUS cpssDxChPolicerCountingWriteBackCacheFlush
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage
);

/*******************************************************************************
* cpssDxChPolicerMeteringCalcMethodSet
*
* DESCRIPTION:
*      Sets Metering Policer parameters calculation orientation method.
*      Due to the fact that selecting algorithm has effect on how HW metering
*      parameters are set, this API influence on
*      cpssDxCh3PolicerMeteringEntrySet and
*      cpssDxCh3PolicerEntryMeterParamsCalculate APIs.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum                  - device number.
*       meteringCalcMethod      - selecting calculation of HW metering
*                                 parameters between CIR\PIR or CBS\PBS
*                                 orientation.
*       cirPirAllowedDeviation  - the allowed deviation in percentage from the
*                                 requested CIR\PIR. Relevant only for
*                         CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*       cbsPbsCalcOnFail        - GT_TRUE: If CBS\PBS constraints cannot be
*                                 matched return to CIR\PIR oriented
*                                 calculation.
*                                 GT_FALSE: If CBS\PBS constraints cannot be
*                                 matched return error. Relevant only for
*                         CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PARAM                - wrong devNum or method.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerMeteringCalcMethodSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT   meteringCalcMethod,
    IN GT_U32                                       cirPirAllowedDeviation,
    IN GT_BOOL                                      cbsPbsCalcOnFail
);

/*******************************************************************************
* cpssDxChPolicerMeteringCalcMethodGet
*
* DESCRIPTION:
*      Gets Metering Policer parameters calculation orientation method.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum              - device number.
*
* OUTPUTS:
*       meteringCalcMethodPtr       - (pointer to) selected calculation of HW
*                                     metering parameters between CIR\PIR or
*                                     CBS\PBS orientation.
*       cirPirAllowedDeviationPtr   - (pointer to) the allowed deviation in
*                                     percentage from the requested CIR\PIR.
*                                     Relevant only for
*                         CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*       cbsPbsCalcOnFailPtr         - (pointer to)
*                                     GT_TRUE: If CBS\PBS constraints cannot be
*                                     matched return to CIR\PIR oriented
*                                     calculation.
*                                     GT_FALSE: If CBS\PBS constraints cannot
*                                     be matched return error.
*                                     Relevant only for
*                         CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_BAD_PARAM                - wrong devNum or method.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerMeteringCalcMethodGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT  *meteringCalcMethodPtr,
    OUT GT_U32                                      *cirPirAllowedDeviationPtr,
    OUT GT_BOOL                                     *cbsPbsCalcOnFailPtr
);

/*******************************************************************************
* cpssDxChPolicerCountingTriggerByPortEnableSet
*
* DESCRIPTION:
*       Enable/Disable Counting when port based metering is triggered for
*       the packet.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       enable      - GT_TRUE - If counting mode is Billing, counting is performed
*                               for every packet subject to port based metering.
*                               If counting mode is not Billing, counting is
*                               performed for these packets only if the Policy
*                               engine enabled for Counting.
*                     GT_FALSE - Counting is performed for these packets only
*                                if the Policy engine enabled for Counting.
.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerCountingTriggerByPortEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
);

/*******************************************************************************
* cpssDxChPolicerCountingTriggerByPortEnableGet
*
* DESCRIPTION:
*       Get Counting trigger by port status.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* INPUTS:
*       devNum          - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* OUTPUTS:
*       enablePtr   - GT_TRUE - If counting mode is Billing, counting is performed
*                               for every packet subject to port based metering.
*                               If counting mode is not Billing, counting is
*                               performed for these packets only if the Policy
*                               engine enabled for Counting.
*                     GT_FALSE - Counting is performed for these packets only
*                                if the Policy engine enabled for Counting.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerCountingTriggerByPortEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    OUT GT_BOOL                                 *enablePtr
);


/*******************************************************************************
* cpssDxChPolicerPortModeAddressSelectSet
*
* DESCRIPTION:
*       Configure Metering Address calculation type.
*       Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* APPLICABLE DEVICES:
*        xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       type        - Address select type: Full or Compressed.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or type.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortModeAddressSelectSet
(
    IN  GT_U8                                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT                     stage,
    IN CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT     type
);


/*******************************************************************************
* cpssDxChPolicerPortModeAddressSelectGet
*
* DESCRIPTION:
*       Get Metering Address calculation type.
*       Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* APPLICABLE DEVICES:
*        xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* OUTPUTS:
*       typePtr     - (pointer to) Address select type: Full or Compressed.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortModeAddressSelectGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                    stage,
    OUT CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT    *typePtr
);

/*******************************************************************************
* cpssDxChPolicerMeterModeEnableSet
*
* DESCRIPTION:
*       Enable/Disable ePort/eVlan based metering/counting.
*       If stage is enabled for ePort/eVlan based metering/counting, 
*       it has also an access to ePort/eVlan triggered table.  
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       ePortEnable     - GT_TRUE: enable ePort based metering/couning on given stage.
*                         GT_FALSE: disable ePort based metering/couning on given stage.
*       eVlanEnable     - GT_TRUE: enable eVlan based metering/couning on given stage.
*                         GT_FALSE: disable eVlan based metering/couning on given stage.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, direction or mode.
*       GT_BAD_STATE                - on wrong state. When trying to enable 
*                                     one Ingress stage while other Ingress stage
*                                     is still enabled.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In Dual Ingress stage: 
*       Disable previously ePort/eVlan enabled Ingress stage, before enabling
*       another one.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerMeterModeEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage, 
    IN GT_BOOL                              ePortEnable,
    IN GT_BOOL                              eVlanEnable
);

/*******************************************************************************
* cpssDxChPolicerMeterModeEnableGet
*
* DESCRIPTION:
*       Get the status of ePort/eVlan based metering/counting (Enabled/Disabled).
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - device number.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* OUTPUTS:
*       ePortEnablePtr  - (pointer to) 
*                         GT_TRUE: enable ePort based metering/couning on given stage.
*                         GT_FALSE: disable ePort based metering/couning on given stage.
*       eVlanEnablePtr  - (pointer to) 
*                         GT_TRUE: enable eVlan based metering/couning on given stage.
*                         GT_FALSE: disable eVlan based metering/couning on given stage.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, direction or mode.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerMeterModeEnableGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage, 
    OUT GT_BOOL                              *ePortEnablePtr,
    OUT GT_BOOL                              *eVlanEnablePtr
);


/*******************************************************************************
* cpssDxChPolicerPortTriggerEntrySet
*
* DESCRIPTION:
*       Set Policer port trigger entry. 
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       portNum     - Port number
*       entryPtr    - (pointer to) Policer port trigger entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or portNum.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In Ingress Dual stage both IPLR_0 and IPLR_1 configure same memory.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_PORT_NUM                             portNum,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);

/*******************************************************************************
* cpssDxChPolicerPortTriggerEntryGet
*
* DESCRIPTION:
*       Get Policer port trigger entry. 
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       portNum     - Port number
*
* OUTPUTS:
*       entryPtr    - (pointer to) Policer port trigger entry
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or portNum.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_PORT_NUM                             portNum,
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);

/*******************************************************************************
* cpssDxChPolicerVlanTriggerEntrySet
*
* DESCRIPTION:
*       Set Policer vlan trigger entry. 
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       vlan        - vlan number
*       entryPtr    - (pointer to) Policer vlan trigger entry
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or vlan.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In Ingress Dual stage both IPLR_0 and IPLR_1 configure same memory.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  vlan,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);


/*******************************************************************************
* cpssDxChPolicerVlanTriggerEntryGet
*
* DESCRIPTION:
*       Get Policer vlan trigger entry. 
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum      - device number.
*       stage       - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       vlan        - vlan number
*
* OUTPUTS:
*       entryPtr    - (pointer to) Policer vlan trigger entry
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or vlan.
*       GT_BAD_PTR                  - on NULL pointer
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerVlanTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  vlan,
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);


/******************************************************************************/
/******************************************************************************/
/******* start of functions with portGroupsBmp parameter **********************/
/******************************************************************************/
/******************************************************************************/


/*******************************************************************************
* cpssDxChPolicerPortGroupMeteringEntryRefresh
*
* DESCRIPTION:
*       Refresh the Policer Metering Entry.
*       Meter Bucket's state parameters refreshing are needed in order to
*       prevent mis-behavior due to wrap around of timers.
*
*       The wrap around problem can occur when there are long periods of
*       'silence' on the flow and the bucket's state parameters do not get
*       refreshed (meter is in the idle state). This causes a problem once the
*       flow traffic is re-started since the timers might have already wrapped
*       around which can cause a situation in which the bucket's state is
*       mis-interpreted and the incoming packet is marked as non-conforming
*       even though the bucket was actually supposed to be full.
*       To prevent this from happening the CPU needs to trigger a meter
*       refresh transaction at least once every 10 minutes per meter.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above
*                         devices and ignored by DxCh3.
*       entryIndex      - index of Policer Metering Entry.
*                         Range: see datasheet for specific device.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_TIMEOUT                  - on time out of IPLR Table indirect access.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage, entryIndex or
*                                     portGroupsBmp
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex
);

/*******************************************************************************
* cpssDxChPolicerPortGroupErrorGet
*
* DESCRIPTION:
*       Gets address and type of Policer Entry that had an ECC error.
*       This information available if error is happened and not read till now.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       entryTypePtr    - pointer to the Type of Entry (Metering or Counting)
*                         that had an error.
*       entryAddrPtr    - pointer to the Policer Entry that had an error.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_EMPTY                    - on missing error information.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupErrorGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupErrorCounterGet
*
* DESCRIPTION:
*       Gets the value of the Policer ECC Error Counter.
*       The Error Counter is a free-running non-sticky 8-bit read-only
*       counter.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*
* OUTPUTS:
*       cntrValuePtr    - pointer to the Policer ECC Error counter value.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage or portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupManagementCountersSet
*
* DESCRIPTION:
*       Sets the value of specified Management Counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       mngCntrSet      - Management Counters Set (APPLICABLE RANGES: 0..2).
*       mngCntrType     - Management Counters Type (GREEN, YELLOW, RED, DROP).
*       mngCntrPtr      - pointer to the Management Counters Entry must be set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum or stage or mngCntrType,
*                                     Management Counters Set number or
*                                     portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupManagementCountersGet
*
* DESCRIPTION:
*       Gets the value of specified Management Counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - physical device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       mngCntrSet      - Management Counters Set (APPLICABLE RANGES: 0..2).
*       mngCntrType     - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* OUTPUTS:
*       mngCntrPtr      - pointer to the requested Management Counters Entry.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on Hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage, mngCntrType,
*                                     Management Counters Set number or
*                                     portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupMeteringEntrySet
*
* DESCRIPTION:
*      Sets Metering Policer Entry.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Metering Entry.
*                         Range: see datasheet for specific device.
*       entryPtr        - pointer to the metering policer entry going to be set.
*
* OUTPUTS:
*       tbParamsPtr     - pointer to actual policer token bucket parameters.
*                         The token bucket parameters are returned as output
*                         values. This is due to the hardware rate resolution,
*                         the exact rate or burst size requested may not be
*                         honored. The returned value gives the user the
*                         actual parameters configured in the hardware.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on hardware error.
*       GT_TIMEOUT                  - on time out of Policer Tables
*                                     indirect access.
*       GT_BAD_PARAM                - wrong devNum, stage, entryIndex,
*                                     entry parameters or portGroupsBmp.
*       GT_OUT_OF_RANGE             - on Billing Entry Index out of range.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
);


/*******************************************************************************
* cpssDxChPolicerPortGroupMeteringEntryGet
*
* DESCRIPTION:
*      Gets Metering Policer Entry configuration.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Metering Entry.
*                         Range: see datasheet for specific device.
*
* OUTPUTS:
*       entryPtr        - pointer to the requested metering policer entry.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - wrong devNum, stage, entryIndex or
*                                     portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*       GT_BAD_STATE                - on bad value in a entry.
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupBillingEntrySet
*
* DESCRIPTION:
*       Sets Policer Billing Counters.
*       Billing is the process of counting the amount of traffic that belongs
*       to a flow. This process can be activated only by metering and is used
*       either for statistical analysis of a flow's traffic or for actual
*       billing.
*       The billing counters are free-running no-sticky counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Billing Counters Entry.
*                         Range: see datasheet for specific device.
*       billingCntrPtr  - pointer to the Billing Counters Entry.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_TIMEOUT                  - on time out upon counter reset by
*                                     indirect access.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage, entryIndex or
*                                     portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*           - disconnect entryes from traffic
*           - call cpssDxChPolicerCountingWriteBackCacheFlush
*           - set new values
*           - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupBillingEntryGet
*
* DESCRIPTION:
*       Gets Policer Billing Counters.
*       Billing is the process of counting the amount of traffic that belongs
*       to a flow. This process can be activated only by metering and is used
*       either for statistical analysis of a flow's traffic or for actual
*       billing.
*       The billing counters are free-running no-sticky counters.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                         Stage type is significant for xCat and above devices
*                         and ignored by DxCh3.
*       entryIndex      - index of Policer Billing Counters Entry.
*                         Range: see datasheet for specific device.
*       reset           - reset flag:
*                         GT_TRUE  - performing read and reset atomic operation.
*                         GT_FALSE - performing read counters operation only.
*
* OUTPUTS:
*       billingCntrPtr  - pointer to the Billing Counters Entry.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_BAD_PTR                  - on NULL pointer.
*       GT_TIMEOUT                  - on time out of IPLR Table indirect access.
*       GT_HW_ERROR                 - on hardware error.
*       GT_BAD_PARAM                - on wrong devNum, stage, entryIndex or
*                                     portGroupsBmp.
*       GT_BAD_STATE                - on counter entry type mismatch.
*       GT_NOT_APPLICABLE_DEVICE    - on devNum of not applicable device.
*
* COMMENTS:
*       The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
);

/*******************************************************************************
* cpssDxChPolicerPortGroupPolicyCntrSet
*
* DESCRIPTION:
*      Sets Policer Policy Counters located in the Policer Counting Entry upon
*      enabled Policy Counting mode.
*      When working in Policy Counting Mode each counter counts the number of
*      packets matching on a Policy rules.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       index           - index set by Policy Action Entry or Metering entry
*                         and belongs to the range from 0 up to maximal number
*                         of Policer Policy counters.
*                         Maximal number is number of policer entries for
*                         counting is number of counting entries
*                         multiplied by 8.
*                         Some indexes in the range are not valid. Not valid
*                         indexes are:
*                           - (index % 8) == 6
*                           - (index % 8) == 7
*                         e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*       cntrValue       - packets counter.
*                         For counter reset this parameter should be nullified.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on disabled Policy Counting.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM    - on wrong devNum or index (index range is limited by
*                         max number of Policer Policy counters) or portGroupsBmp.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The Policy counters are free-running no-sticky counters.
*
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*           - disconnect entryes from traffic
*           - call cpssDxChPolicerCountingWriteBackCacheFlush
*           - set new values
*           - connect entries back to traffic
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
);

/*******************************************************************************
* cpssDxChPolicerPortGroupPolicyCntrGet
*
* DESCRIPTION:
*      Gets Policer Policy Counters located in the Policer Counting Entry upon
*      enabled Policy Counting mode.
*      When working in Policy Counting Mode each counter counts the number of
*      packets matching on a a Policy rules.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number.
*       portGroupsBmp   - bitmap of Port Groups.
*                       NOTEs:
*                       1. for non multi-port groups device this parameter is IGNORED.
*                       2. for multi-port groups device :
*                       bitmap must be set with at least one bit representing
*                       valid port group(s). If a bit of non valid port group
*                       is set then function returns GT_BAD_PARAM.
*                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       stage           - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*       index           - index set by Policy Action Entry or Metering entry
*                         and belongs to the range from 0 up to maximal number
*                         of Policer Policy counters.
*                         Maximal number is number of policer entries for
*                         counting is number of counting entries
*                         multiplied by 8.
*                         Some indexes in the range are not valid. Not valid
*                         indexes are:
*                           - (index % 8) == 6
*                           - (index % 8) == 7
*                         e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* OUTPUTS:
*       cntrValuePtr    - Pointer to the packet counter.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on disabled Policy Counting.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum, index or portGroupsBmp.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_TIMEOUT               - on time out.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The Policy counters are free-running no-sticky counters.
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerPortGroupPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
);

/*******************************************************************************
* cpssDxChPolicerSecondStageIndexMaskSet
*
* DESCRIPTION:
*       Sets second stage policer index mask.
*
* APPLICABLE DEVICES:
*       Lion.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - device number.
*       plrIndexMask    - mask for policer index (APPLICABLE RANGES: 0..0xFFF).  
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_OUT_OF_RANGE          - on illegal plrIndexMask
*
* COMMENTS:
*       Note: Lion device applicable starting from revision B2 
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerSecondStageIndexMaskSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  plrIndexMask
);


/*******************************************************************************
* cpssDxChPolicerSecondStageIndexMaskGet
*
* DESCRIPTION:
*       Gets second stage policer index mask.
*
* APPLICABLE DEVICES:
*       Lion.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum          - device number.
*
* OUTPUTS:
*       plrIndexMaskPtr - (pointer to) mask for policer index.  
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_HW_ERROR              - on hardware error.
*       GT_BAD_PARAM             - on wrong devNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       Note: Lion device applicable starting from revision B2 
*
*******************************************************************************/
GT_STATUS cpssDxChPolicerSecondStageIndexMaskGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *plrIndexMaskPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxCh3Policerh */


/*
 *------------------------------------------------------------------
 * $Log: cpssDxCh3Policer.h,v $
 * Revision 1.1  2015/02/13 11:31:24  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
