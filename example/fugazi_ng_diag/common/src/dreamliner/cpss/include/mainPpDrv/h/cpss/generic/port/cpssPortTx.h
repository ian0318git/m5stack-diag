/* $Id: cpssPortTx.h,v 1.1 2015/02/13 11:34:13 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/cpssPortTx.h,v $
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
* cpssPortTx.h
*
* DESCRIPTION:
*       CPSS definitions for port Tx Traffic Class Queues.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssPortTxh
#define __cpssPortTxh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

/**********************************************************************
 * typedef: enum CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT
 *
 * Description: Enumeration of Tx Queue scheduler profile set
 *
 * Enumerations:
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_1_E - Tx Queue scheduler profile 1
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_2_E - Tx Queue scheduler profile 2
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_3_E - Tx Queue scheduler profile 3
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_4_E - Tx Queue scheduler profile 4
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_5_E - Tx Queue scheduler profile 5
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_6_E - Tx Queue scheduler profile 6
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_7_E - Tx Queue scheduler profile 7
 *    CPSS_PORT_TX_SCHEDULER_PROFILE_8_E - Tx Queue scheduler profile 8
 * Comments:
 *    Profiles 5-8 are not supported by DxCh1 and DxCh1_Diamond devices
 *********************************************************************/
typedef enum
{
     CPSS_PORT_TX_SCHEDULER_PROFILE_1_E = 0,
     CPSS_PORT_TX_SCHEDULER_PROFILE_2_E,
     CPSS_PORT_TX_SCHEDULER_PROFILE_3_E,
     CPSS_PORT_TX_SCHEDULER_PROFILE_4_E,
     CPSS_PORT_TX_SCHEDULER_PROFILE_5_E,
     CPSS_PORT_TX_SCHEDULER_PROFILE_6_E,
     CPSS_PORT_TX_SCHEDULER_PROFILE_7_E,
     CPSS_PORT_TX_SCHEDULER_PROFILE_8_E
} CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT;

/**********************************************************************
 * typedef: CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS
 *
 * Description: The Tail Drop profile Traffic Class parameters
 *
 * Fields:
 *   dp0MaxBuffNum   - the number of buffers that can be allocated to all of the
 *                     port's TC Tx queues, for a port that uses this profile.
 *                     for GREEN packets
 *   dp1MaxBuffNum   - the number of buffers that can be allocated to all of the
 *                     port's TC Tx queues, for a port that uses this profile.
 *                     for YELLOW packets
 *   dp2MaxBuffNum   - the number of buffers that can be allocated to all of the
 *                     port's TC Tx queues, for a port that uses this profile.
 *                     for RED packets
 *   dp0MaxDescrNum  - the number of descriptors that can be allocated to all of
 *                     the port's TC Tx queues, for a port that uses this
 *                     profile. for GREEN packets
 *   dp1MaxDescrNum  - the number of descriptors that can be allocated to all of
 *                     the port's TC Tx queues, for a port that uses this
 *                     profile. for Yellow packets
 *   dp2MaxDescrNum  - the number of descriptors that can be allocated to all of
 *                     the port's TC Tx queues, for a port that uses this
 *                     profile. for RED packets
 *   tcMaxBuffNum    - the number of buffers that can be allocated to all of the
 *                     port's TC Tx queues, for a port that uses this profile.
 *                     (APPLICABLE DEVICES: Lion)
 *   tcMaxDescrNum   - the number of descriptors that can be allocated to all of
 *                     the port's TC Tx queues, for a port that uses this
 *                     profile. (APPLICABLE DEVICES: Lion)
 *********************************************************************/
typedef struct
{
    GT_U16  dp0MaxBuffNum;
    GT_U16  dp1MaxBuffNum;
    GT_U16  dp2MaxBuffNum;
    GT_U16  dp0MaxDescrNum;
    GT_U16  dp1MaxDescrNum;
    GT_U16  dp2MaxDescrNum;
    GT_U32  tcMaxBuffNum;
    GT_U32  tcMaxDescrNum;
} CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS;

/**********************************************************************
 * typedef: enum CPSS_PORT_TX_WRR_MODE_ENT
 *
 * Description: Enumeration of Transmit Scheduler mode
 *
 * Enumerations:
 *    CPSS_PORT_TX_WRR_BYTE_MODE_E   - byte mode
 *    CPSS_PORT_TX_WRR_PACKET_MODE_E - packet mode
 *********************************************************************/
typedef enum
{
    CPSS_PORT_TX_WRR_BYTE_MODE_E,
    CPSS_PORT_TX_WRR_PACKET_MODE_E
} CPSS_PORT_TX_WRR_MODE_ENT;


/**********************************************************************
 * typedef: enum CPSS_PORT_TX_WRR_MTU_ENT
 *
 * Description: Enumeration of MTU for DSWRR, resolution of WRR weights for byte
 *              based Transmit Scheduler count mode
 *
 * Enumerations:
 *    CPSS_PORT_TX_WRR_MTU_256_E    - Resolution is 256 bytes
 *    CPSS_PORT_TX_WRR_MTU_2K_E     - Resolution is 2K bytes
 *    CPSS_PORT_TX_WRR_MTU_8K_E     - Resolution is 8K bytes
 *    CPSS_PORT_TX_WRR_MTU_64_E     - Resolution is 64 bytes
 *    CPSS_PORT_TX_WRR_MTU_128_E    - Resolution is 128 bytes
 *    CPSS_PORT_TX_WRR_MTU_512_E    - Resolution is 512 bytes
 *    CPSS_PORT_TX_WRR_MTU_1K_E     - Resolution is 1K bytes
 *    CPSS_PORT_TX_WRR_MTU_4K_E     - Resolution is 4K bytes
 *    CPSS_PORT_TX_WRR_MTU_16K_E    - Resolution is 16K bytes
 *    CPSS_PORT_TX_WRR_MTU_32K_E    - Resolution is 32K bytes
 *    CPSS_PORT_TX_WRR_MTU_64K_E    - Resolution is 64K bytes
 *
 *********************************************************************/
typedef enum
{
    CPSS_PORT_TX_WRR_MTU_256_E,
    CPSS_PORT_TX_WRR_MTU_2K_E,
    CPSS_PORT_TX_WRR_MTU_8K_E,
    CPSS_PORT_TX_WRR_MTU_64_E,
    CPSS_PORT_TX_WRR_MTU_128_E,
    CPSS_PORT_TX_WRR_MTU_512_E,
    CPSS_PORT_TX_WRR_MTU_1K_E,
    CPSS_PORT_TX_WRR_MTU_4K_E,
    CPSS_PORT_TX_WRR_MTU_16K_E,
    CPSS_PORT_TX_WRR_MTU_32K_E,
    CPSS_PORT_TX_WRR_MTU_64K_E
} CPSS_PORT_TX_WRR_MTU_ENT;

/*
 * typedef: CPSS_PORT_TX_Q_ARB_GROUP_ENT
 *
 * Description: The Traffic Class Queue Arbritration Group
 *
 * Enumerations:
 *         CPSS_PORT_TX_WRR_ARB_GROUP_0_E  - Weighted Round Robin Arbitration
 *                                           Group 0
 *         CPSS_PORT_TX_WRR_ARB_GROUP_1_E  - Weighted Round Robin Arbitration
 *                                           Group 1
 *         CPSS_PORT_TX_SP_ARB_GROUP_E     - Strict Priority Arbitration Group
 *
 * Comments:
 *        There are 3 scheduling Arbitration groups per Logical Port:
 *
 *        1) The Strict Priority Arbitration Group is served first, where
 *        the priority goes from the highest queue index to the lowest.
 *
 *        2) When there is no traffic from the SP Arbitration Group, the
 *        WRR Arbitraion Group 1 may transmit packets.
 *
 *        3) When there is no traffic from the WRR Group 1,
 *        WRR Arbitraion Group 0 may transmit packets.
 *
 *        Note - Within each WRR Arbitration Group, each queue is
 *        guaranteed its proportional minimal bandwidth according to its
 *        configured weight.
 */
typedef enum
{
    CPSS_PORT_TX_WRR_ARB_GROUP_0_E = 0,
    CPSS_PORT_TX_WRR_ARB_GROUP_1_E,
    CPSS_PORT_TX_SP_ARB_GROUP_E
}CPSS_PORT_TX_Q_ARB_GROUP_ENT;


/*
 * typedef: CPSS_PORT_TX_Q_DROP_MODE_ENT
 *
 * Description: The queue drop algorithm mode on a Logical Port
 *
 * Enumerations:
 *        CPSS_PORT_TX_GRED_DROP_MODE_E - Enqueue packets using General RED
 *                                        (WRED with 3 levels drop precedence)
 *                                        for alltraffic queues on this logical
 *                                        port
 *
 *        CPSS_PORT_TX_TAIL_DROP_MODE_E - Enqueue packets using 3-level Tail
 *                                        Drop(according to drop precedence) for
 *                                        all traffic queues on this logical
 *                                        port
 */
typedef enum
{
    CPSS_PORT_TX_GRED_DROP_MODE_E = 0,
    CPSS_PORT_TX_TAIL_DROP_MODE_E
}CPSS_PORT_TX_Q_DROP_MODE_ENT;


/*
 * typedef: CPSS_PORT_TX_Q_DROP_PROFILE_PARAMS_STC
 *
 * Description: The profile parameters used by 3-level WRED (GRED) or
 *              3-level Tail Drop mechanism for a
 *              Logical Port Traffic Class Queue
 *
 * Fields:
 *   dp0MinThreshold  - The Average_Queue_Length minimal threshold
 *                      for GREEN packets in units of 256 bytes
 *   dp1MinThreshold  - The Average_Queue_Length minimal threshold
 *                      for YELLOW packets in units of 256 bytes
 *   dp2MinThreshold  - The Average_Queue_Length minimal threshold
 *                      for RED packets in units of 256 bytes
 *   dp0MaxThreshold  - The Average_Queue_Length maximum threshold
 *                      for GREEN packets in units of 256 bytes
 *   dp1MaxThreshold  - The Average_Queue_Length maximum threshold
 *                      for Yellow packets in units of 256 bytes
 *   dp2MaxThreshold  - The Average_Queue_Length maximum threshold
 *                      for RED packets in units of 256 bytes
 *   dp0MaxProb       - The GREEN traffic mark probability exponential factor.
 *                      The mark probability is 1/(2**DP1_Max_Prob). Range 1-15.
 *   dp1MaxProb       - The Yellow traffic mark probability exponential factor.
 *                      The mark probability is 1/(2**DP1_Max_Prob). Range 1-15.
 *   dp2MaxProb       - The RED traffic mark probability exponential factor.
 *                      The mark probability is 1/(2**DP1_Max_Prob).Range 1-15.
 *   RedQWeightFactor - The average queue length weight factor. Range 1-15.
 *                      Recommended value: 9
 *
 * Comments:
 *    When operating in 3-level Tail Drop mode (not GRED), only the
 *    xxxMaxThreshold parameters are relevant.
 *
 *    When operation in GRED mode, all the parameters are relevant.
 */
typedef struct
{
    GT_U16  dp0MinThreshold;
    GT_U16  dp1MinThreshold;
    GT_U16  dp2MinThreshold;
    GT_U16  dp0MaxThreshold;
    GT_U16  dp1MaxThreshold;
    GT_U16  dp2MaxThreshold;
    GT_U8   dp0MaxProb;
    GT_U8   dp1MaxProb;
    GT_U8   dp2MaxProb;
    GT_U8   RedQWeightFactor;
} CPSS_PORT_TX_Q_DROP_PROFILE_PARAMS_STC;


/*
 * typedef: CPSS_PORT_TX_DROP_PROFILE_SET_ENT
 *
 * Description:  There are Drop Profile sets of parameters.
 *               This enumeration specifies which Drop Profile set
 *               a given Logical Port is associated with.
 *
 * Enumerations:
 *         CPSS_PORT_TX_DROP_PROFILE_1_E - Drop Profile Set #1
 *         CPSS_PORT_TX_DROP_PROFILE_2_E - Drop Profile Set #2
 *         CPSS_PORT_TX_DROP_PROFILE_3_E - Drop Profile Set #3
 *         CPSS_PORT_TX_DROP_PROFILE_4_E - Drop Profile Set #4
 *         CPSS_PORT_TX_DROP_PROFILE_5_E - Drop Profile Set #5
 *         CPSS_PORT_TX_DROP_PROFILE_6_E - Drop Profile Set #6
 *         CPSS_PORT_TX_DROP_PROFILE_7_E - Drop Profile Set #7
 *         CPSS_PORT_TX_DROP_PROFILE_8_E - Drop Profile Set #8
 * Comments:
 *          Profiles 5 - 8 are relevant for DXCH2 and above devices.
 */
typedef enum
{
    CPSS_PORT_TX_DROP_PROFILE_1_E = 0,
    CPSS_PORT_TX_DROP_PROFILE_2_E,
    CPSS_PORT_TX_DROP_PROFILE_3_E,
    CPSS_PORT_TX_DROP_PROFILE_4_E,
    CPSS_PORT_TX_DROP_PROFILE_5_E,
    CPSS_PORT_TX_DROP_PROFILE_6_E,
    CPSS_PORT_TX_DROP_PROFILE_7_E,
    CPSS_PORT_TX_DROP_PROFILE_8_E
}CPSS_PORT_TX_DROP_PROFILE_SET_ENT;

/*
 * typedef: CPSS_PORT_TX_DROP_SHAPER_MODE_ENT
 *
 * Description: Token Bucket Shaper mode
 *
 * Enumerations:
 *  CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E   - shaper rate is based on the packet's
 *                                           Byte Count
 *  CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E - shaper rate is based on the number of
 *                                           packets
*/

typedef enum
{
    CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E,
    CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E
} CPSS_PORT_TX_DROP_SHAPER_MODE_ENT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortTxh */

/*
 *------------------------------------------------------------------
 * $Log: cpssPortTx.h,v $
 * Revision 1.1  2015/02/13 11:34:13  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
