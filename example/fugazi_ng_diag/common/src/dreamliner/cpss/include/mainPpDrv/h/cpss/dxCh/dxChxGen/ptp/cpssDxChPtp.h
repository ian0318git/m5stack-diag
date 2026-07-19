/* $Id: cpssDxChPtp.h,v 1.1 2015/02/13 11:32:48 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h,v $
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
* cpssDxChPtp.h
*
* DESCRIPTION:
*       CPSS DxCh Precision Time Protocol APIs
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __cpssDxChPtph
#define __cpssDxChPtph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>



/*
 * typedef: CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT
 *
 *  Description:
 *     This enum defines possible TOD (Time Of Day) counter functions.
 *
 * Enumerations:
 *     CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E -    Copy the value from the TOD
 *                              counter shadow to the TOD counter register
 *     CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E - Add the value of the TOD
 *                              counter shadow to the TOD counter register
 *     CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E -   Copy the value of the TOD
 *                              counter to the TOD counter shadow register
 *     CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E -  Generate a pulse on the
 *                              external interface at the configured time,
 *                              determined by the TOD counter shadow
 *
 */
typedef enum {
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E,
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E,
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E
} CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT;


/*
 * typedef: struct CPSS_DXCH_PTP_TOD_COUNT_STC
 *
 * Description: Structure for TOD Counter.
 *
 * Fields:
 *  nanoSeconds - nanosecond value of the timestamping TOD counter
 *  seconds     - second value of the timestamping TOD counter
 *
 */
typedef struct{
    GT_U32      nanoSeconds;
    GT_U64      seconds;
} CPSS_DXCH_PTP_TOD_COUNT_STC;


/*
 * typedef: struct CPSS_DXCH_PTP_TIMESTAMP_ENTRY_STC
 *
 * Description: Structure for Captured Timestamp Entry.
 *
 * Fields:
 *  portNum      -  PTP source/target port number
 *                  Note: In ingress timestamping this field specifies the
 *                        source port of the incoming PTP packet.
 *                        In egress timestamping this field specifies the
 *                        target port of the outgoing PTP packet.
 *  messageType  -  PTP message type from PTP header.
 *  sequenceId   -  The PTP Sequence ID of the PTP packet.
 *  timeStampVal -  The PTP timestamp value, contains the nanosecond field of
 *                  the TOD counter.
 *                  Note: Bits 31:30 are always 0.
 *
 */
typedef struct{
    GT_U8       portNum;
    GT_U32      messageType;
    GT_U32      sequenceId;
    GT_U32      timeStampVal;
} CPSS_DXCH_PTP_TIMESTAMP_ENTRY_STC;


/*******************************************************************************
* cpssDxChPtpEtherTypeSet
*
* DESCRIPTION:
*       Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*       A packet is identified as PTP over Ethernet if its EtherType matches
*       one of the configured values.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       etherTypeIndex  - PTP EtherType index (APPLICABLE RANGES: 0..1)
*       etherType       - PTP EtherType0 or EtherType1 according to the index,
*                         (APPLICABLE RANGES: 0..0xFFFF)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
*       GT_OUT_OF_RANGE          - on wrong etherType
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherTypeIndex,
    IN GT_U32   etherType
);


/*******************************************************************************
* cpssDxChPtpEtherTypeGet
*
* DESCRIPTION:
*       Get ethertypes of PTP over Ethernet packets.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       etherTypeIndex  - PTP EtherType index (APPLICABLE RANGES: 0..1)
*
* OUTPUTS:
*       etherTypePtr    - (pointer to) PTP EtherType0 or EtherType1 according
*                          to the index
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpEtherTypeGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   etherTypeIndex,
    OUT GT_U32   *etherTypePtr
);

/*******************************************************************************
* cpssDxChPtpUdpDestPortsSet
*
* DESCRIPTION:
*       Configure UDP destination port0/port1 of PTP over UDP packets.
*       A packet is identified as PTP over UDP if it is a UDP packet, whose
*       destination port matches one of the configured ports.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       udpPortIndex    - UDP port index (APPLICABLE RANGES: 0..1)
*       udpPortNum      - UDP port1/port0 number according to the index,
*                         (APPLICABLE RANGES: 0..0xFFFF)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or udpPortIndex
*       GT_OUT_OF_RANGE          - on wrong udpPortNum
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpUdpDestPortsSet
(
    IN GT_U8    devNum,
    IN GT_U32   udpPortIndex,
    IN GT_U32   udpPortNum
);

/*******************************************************************************
* cpssDxChPtpUdpDestPortsGet
*
* DESCRIPTION:
*       Get UDP destination port0/port1 of PTP over UDP packets.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       udpPortIndex    - UDP port index (APPLICABLE RANGES: 0..1)
*
* OUTPUTS:
*       udpPortNumPtr     - (pointer to) UDP port0/port1 number, according
*                           to the index
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or udpPortIndex
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpUdpDestPortsGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   udpPortIndex,
    OUT GT_U32   *udpPortNumPtr
);


/*******************************************************************************
* cpssDxChPtpMessageTypeCmdSet
*
* DESCRIPTION:
*       Configure packet command per PTP message type.
*       The message type is extracted from the PTP header.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       messageType     - message type (APPLICABLE RANGES: 0..15)
*       command         - assigned command to the packet.
*                         Acceptable values: FORWARD, MIRROR, TRAP, HARD_DROP and
*                                            SOFT_DROP.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, messageType or command
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpMessageTypeCmdSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   messageType,
    IN CPSS_PACKET_CMD_ENT      command
);

/*******************************************************************************
* cpssDxChPtpMessageTypeCmdGet
*
* DESCRIPTION:
*       Get packet command per PTP message type.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       messageType     - message type (APPLICABLE RANGES: 0..15).
*
* OUTPUTS:
*       commandPtr      - (pointer to) assigned command to the packet.
*                         Acceptable values: FORWARD, MIRROR, TRAP, HARD_DROP and
*                                            SOFT_DROP.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or messageType
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_BAD_STATE             - on bad value found in HW
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpMessageTypeCmdGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   messageType,
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
);

/*******************************************************************************
* cpssDxChPtpCpuCodeBaseSet
*
* DESCRIPTION:
*       Set CPU code base assigned to trapped and mirrored PTP packets.
*       A PTP packet's CPU code is computed as base CPU code + messageType
*       (where message type is taken from PTP header).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       cpuCode         - The base of CPU code assigned to PTP packets mirrored
*                         or trapped to CPU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or cpuCode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpCpuCodeBaseSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
);

/*******************************************************************************
* cpssDxChPtpCpuCodeBaseGet
*
* DESCRIPTION:
*       Get CPU code base assigned to trapped and mirrored PTP packets.
*       A PTP packet's CPU code is computed as base CPU code + messageType
*       (where message type is taken from PTP header).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*
* OUTPUTS:
*       cpuCodePtr      - (pointer to) The base of CPU code assigned to PTP
*                         packets mirrored or trapped to CPU.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpCpuCodeBaseGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
);

/*******************************************************************************
* cpssDxChPtpTodCounterFunctionSet
*
* DESCRIPTION:
*       The function configures type of TOD counter action that will be
*       performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*       API or by pulse from an external interface.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*       function        - One of the four possible TOD counter functions
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*                                  or function
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterFunctionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT       function
);

/*******************************************************************************
* cpssDxChPtpTodCounterFunctionGet
*
* DESCRIPTION:
*       Get type of TOD counter action that will be performed once triggered by
*       cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*       external interface.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*
* OUTPUTS:
*       functionPtr     - (pointer to) One of the four possible TOD counter
*                          functions
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterFunctionGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      *functionPtr
);

/*******************************************************************************
* cpssDxChPtpTodCounterFunctionTriggerSet
*
* DESCRIPTION:
*       Trigger TOD (Time of Day) counter function accorging to the function set
*       by cpssDxChPtpTodCounterFunctionSet().
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_STATE             - when previous TOD triggered action is not finished yet
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterFunctionTriggerSet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction
);

/*******************************************************************************
* cpssDxChPtpTodCounterFunctionTriggerGet
*
* DESCRIPTION:
*       Get status of trigger TOD (Time of Day) counter function.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                               function is not finished.
*                         GT_FALSE - The trigger is off, the TOD function is
*                                   finished.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterFunctionTriggerGet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
);

/*******************************************************************************
* cpssDxChPtpTodCounterShadowSet
*
* DESCRIPTION:
*       Configure TOD timestamping shadow counter values.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*       todCounterPtr   - pointer to TOD counter shadow
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterShadowSet
(
    IN GT_U8                        devNum,
    IN CPSS_PORT_DIRECTION_ENT      direction,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC  *todCounterPtr
);

/*******************************************************************************
* cpssDxChPtpTodCounterShadowGet
*
* DESCRIPTION:
*       Get TOD timestamping shadow counter values.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*
* OUTPUTS:
*       todCounterPtr      - (pointer to) TOD counter shadow
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterShadowGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_DIRECTION_ENT         direction,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC     *todCounterPtr
);

/*******************************************************************************
* cpssDxChPtpTodCounterGet
*
* DESCRIPTION:
*       Get TOD timestamping counter value.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*
* OUTPUTS:
*       todCounterPtr      - (pointer to) TOD counter
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTodCounterGet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_DIRECTION_ENT          direction,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC     *todCounterPtr
);

/*******************************************************************************
* cpssDxChPtpPortTimeStampEnableSet
*
* DESCRIPTION:
*       Enable/Disable port for PTP timestamping.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*       portNum         - port number
*                         Note: For Ingress timestamping the trigger is based
*                               on the local source port.
*                               For Egress timestamping the trigger is based
*                               on the local target port.
*       enable          - GT_TRUE - The port is enabled for ingress/egress
*                                   timestamping.
*                         GT_FALSE - The port is disabled for ingress/egress
*                                   timestamping.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, direction or portNum.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortTimeStampEnableSet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
);

/*******************************************************************************
* cpssDxChPtpPortTimeStampEnableGet
*
* DESCRIPTION:
*       Get port PTP timestamping status (enabled/disabled).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*       portNum         - port number
*                         Note: For Ingress timestamping the trigger is based
*                               on the local source port.
*                               For Egress timestamping the trigger is based
*                               on the local target port.
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - The port is enabled for
*                                               ingress/egress timestamping.
*                         GT_FALSE - The port is disabled for ingress/egress
*                                   timestamping.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, direction or portNum.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortTimeStampEnableGet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
);


/*******************************************************************************
* cpssDxChPtpEthernetTimeStampEnableSet
*
* DESCRIPTION:
*       Enable/Disable PTP Timestamping over Ethernet packets.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*       enable          - GT_TRUE - PTP Timestamping is enabled over Ethernet.
*                         GT_FALSE - PTP Timestamping is disabled over Ethernet.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpEthernetTimeStampEnableSet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_BOOL                  enable
);


/*******************************************************************************
* cpssDxChPtpEthernetTimeStampEnableGet
*
* DESCRIPTION:
*       Get PTP Timestamping status over Ethernet packets (enable/disabled).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - PTP Timestamping is enabled
*                                                over Ethernet.
*                         GT_FALSE - PTP Timestamping is disabled over Ethernet.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpEthernetTimeStampEnableGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_BOOL                  *enablePtr
);

/*******************************************************************************
* cpssDxChPtpUdpTimeStampEnableSet
*
* DESCRIPTION:
*       Enable/Disable PTP Timestamping over UDP packets.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*       protocolStack   - types of IP to set: IPV4 or IPV6.
*       enable          - GT_TRUE - PTP Timestamping is enabled over UDP (IPV4/IPV6).
*                         GT_FALSE - PTP Timestamping is disabled over UDP (IPV4/IPV6).
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, protocolStack or direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpUdpTimeStampEnableSet
(
    IN  GT_U8                        devNum,
    IN  CPSS_PORT_DIRECTION_ENT      direction,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN  GT_BOOL                      enable
);


/*******************************************************************************
* cpssDxChPtpUdpTimeStampEnableGet
*
* DESCRIPTION:
*       Get PTP Timestamping status over UDP packets (enable/disabled).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*       protocolStack   - types of IP to set: IPV4 or IPV6.
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - PTP Timestamping is enabled
*                                               over UDP (IPV4/IPV6).
*                         GT_FALSE - PTP Timestamping is disabled over UDP (IPV4/IPV6).
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, protocolStack or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpUdpTimeStampEnableGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_DIRECTION_ENT         direction,
    IN  CPSS_IP_PROTOCOL_STACK_ENT      protocolStack,
    OUT GT_BOOL                         *enablePtr
);

/*******************************************************************************
* cpssDxChPtpMessageTypeTimeStampEnableSet
*
* DESCRIPTION:
*       Enable/Disable PTP Timestamping for specific message type of the packet.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
*       messageType     - message type (APPLICABLE RANGES: 0..15).
*       enable          - GT_TRUE - PTP Timestamping is enabled for specific message type.
*                         GT_FALSE - PTP Timestamping is disabled for specific message type.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, messageType or direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpMessageTypeTimeStampEnableSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  messageType,
    IN  GT_BOOL                 enable
);

/*******************************************************************************
* cpssDxChPtpMessageTypeTimeStampEnableGet
*
* DESCRIPTION:
*       Get PTP Timestamping status for specific message type of the packet
*       (enable/disabled).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*       messageType     - message type (APPLICABLE RANGES: 0..15).
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - PTP Timestamping is enabled for
*                                                specific message type.
*                         GT_FALSE - PTP Timestamping is disabled for specific
*                                    message type.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, messageType or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpMessageTypeTimeStampEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  messageType,
    OUT GT_BOOL                 *enablePtr
);

/*******************************************************************************
* cpssDxChPtpTransportSpecificCheckEnableSet
*
* DESCRIPTION:
*       Enable/Disable PTP Timestamping check of the packet's transport specific value.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum      - device number
*       direction   - Tx, Rx or both directions
*       enable      - GT_TRUE - PTP Timestamping check of the packet's transport
*                               specific value is enabled.
*                     GT_FALSE - PTP Timestamping check of the packet's transport
*                                specific value is disabled.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTransportSpecificCheckEnableSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_BOOL                 enable
);

/*******************************************************************************
* cpssDxChPtpTransportSpecificCheckEnableGet
*
* DESCRIPTION:
*       Get status of PTP Timestamping check of the packet's transport specific
*       value (the check is enabled/disabled).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum      - device number
*       direction   - Tx or Rx
* OUTPUTS:
*       enablePtr   - (pointer to) GT_TRUE - PTP Timestamping check of the
*                                  packet's transport specific value is enabled.
*                     GT_FALSE - PTP Timestamping check of the packet's transport
*                                specific value is disabled.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTransportSpecificCheckEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
);

/*******************************************************************************
* cpssDxChPtpTransportSpecificTimeStampEnableSet
*
* DESCRIPTION:
*       Enable PTP Timestamping for transport specific value of the packet.
*       Note: Relevant only if global configuration determines whether the
*       transport specific field should be checked by
*       cpssDxChPtpTransportSpecificCheckEnableSet().
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum                  - device number
*       direction               - Tx, Rx or both directions
*       transportSpecificVal    - transport specific value taken from PTP header
*                                 bits [7..4]. (APPLICABLE RANGES: 0..15).
*       enable                  - GT_TRUE - PTP Timestamping is enabled for this
*                                           transport specific value.
*                                GT_FALSE - PTP Timestamping is disabled for this
*                                           transport specific value.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, transportSpecificVal or
*                                  direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTransportSpecificTimeStampEnableSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  transportSpecificVal,
    IN  GT_BOOL                 enable
);

/*******************************************************************************
* cpssDxChPtpTransportSpecificTimeStampEnableGet
*
* DESCRIPTION:
*       Get PTP Timestamping status for specific message type of the packet
*       (enable/disabled).
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx or Rx
*       transportSpecificVal    - transport specific value taken from PTP header
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - PTP Timestamping is enabled for
*                                                this transport specific value.
*                          GT_FALSE - PTP Timestamping is disabled for this
*                                     transport specific value.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, transportSpecificVal or
*                                  direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTransportSpecificTimeStampEnableGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  transportSpecificVal,
    OUT GT_BOOL                 *enablePtr
);

/*******************************************************************************
* cpssDxChPtpTimestampEntryGet
*
* DESCRIPTION:
*       Read current Timestamp entry from the timestamp FIFO queue.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       direction       - Tx, Rx or both directions
* OUTPUTS:
*       entryPtr        - (pointer to) timestamp entry
*
* RETURNS:
*       GT_OK                    - on success
*       GT_NO_MORE               - the action succeeded and there are no more waiting
*                                  Timestamp entries
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpTimestampEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_DIRECTION_ENT             direction,
    OUT CPSS_DXCH_PTP_TIMESTAMP_ENTRY_STC   *entryPtr
);



/******************************************************************************/
/******************************************************************************/
/******* start of functions with portGroupsBmp parameter **********************/
/******************************************************************************/
/******************************************************************************/

/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterFunctionSet
*
* DESCRIPTION:
*       The function configures type of TOD counter action that will be
*       performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*       API or by pulse from an external interface.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx, Rx or both directions
*       function        - One of the four possible TOD counter functions
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*                                  or function
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterFunctionSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT       function
);

/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterFunctionGet
*
* DESCRIPTION:
*       Get type of TOD counter action that will be performed once triggered by
*       cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*       external interface.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx or Rx
*
* OUTPUTS:
*       functionPtr     - (pointer to) One of the four possible TOD counter functions
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterFunctionGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      *functionPtr
);

/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterFunctionTriggerSet
*
* DESCRIPTION:
*       Trigger TOD (Time of Day) counter function accorging to the function set
*       by cpssDxChPtpTodCounterFunctionSet().
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx, Rx or both directions
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_STATE             - when previous TOD triggered action is not finished yet
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterFunctionTriggerSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_GROUPS_BMP       portGroupsBmp,
    IN CPSS_PORT_DIRECTION_ENT  direction
);

/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterFunctionTriggerGet
*
* DESCRIPTION:
*       Get status of trigger TOD (Time of Day) counter function.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx or Rx
*
* OUTPUTS:
*       enablePtr       - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                               function is not finished.
*                         GT_FALSE - The trigger is off, the TOD function is
*                                   finished.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterFunctionTriggerGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_GROUPS_BMP       portGroupsBmp,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
);


/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterShadowSet
*
* DESCRIPTION:
*       Configure TOD timestamping shadow counter values.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx, Rx or both directions
*       todCounterPtr   - pointer to TOD counter shadow
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterShadowSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN CPSS_PORT_DIRECTION_ENT      direction,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC  *todCounterPtr
);

/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterShadowGet
*
* DESCRIPTION:
*       Get TOD timestamping shadow counter values.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx or Rx
*
* OUTPUTS:
*       todCounterPtr      - (pointer to) TOD counter shadow
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterShadowGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_PORT_DIRECTION_ENT         direction,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC     *todCounterPtr
);

/*******************************************************************************
* cpssDxChPtpPortGroupTodCounterGet
*
* DESCRIPTION:
*       Get TOD timestamping counter value.
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portGroupsBmp   - bitmap of Port Groups.
*                         NOTEs:
*                         1. for non multi-port groups device this parameter is IGNORED.
*                         2. for multi-port groups device :
*                            bitmap must be set with at least one bit representing
*                            valid port group(s). If a bit of non valid port group
*                            is set then function returns GT_BAD_PARAM.
*                            value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*       direction       - Tx or Rx
*
* OUTPUTS:
*       todCounterPtr      - (pointer to) TOD counter
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - on wrong devNum, portGroupsBmp or direction
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPtpPortGroupTodCounterGet
(
    IN GT_U8                            devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN CPSS_PORT_DIRECTION_ENT          direction,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC     *todCounterPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPtp */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChPtp.h,v $
 * Revision 1.1  2015/02/13 11:32:48  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
