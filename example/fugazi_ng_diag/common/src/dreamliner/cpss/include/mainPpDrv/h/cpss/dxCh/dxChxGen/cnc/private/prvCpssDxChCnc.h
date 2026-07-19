/* $Id: prvCpssDxChCnc.h,v 1.1 2015/02/13 11:31:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc/private/prvCpssDxChCnc.h,v $
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
* prvCpssDxChCnc.h
*
* DESCRIPTION:
*       CPSS DxCh Centralized Counters (CNC) private API.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __prvCpssDxChCnch
#define __prvCpssDxChCnch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* general "less check" macro used in other macros below */
#define LESS_CHECK_MAC(_value, _pattern)     \
     if ((_value) >= (_pattern))                      \
     {                                                \
         return GT_BAD_PARAM;                         \
     }

/* checks that the number of CNC block */
#define PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(_dev, _blockIndex) \
    LESS_CHECK_MAC(_blockIndex, PRV_CPSS_DXCH_PP_MAC(_dev)->fineTuning.tableSize.cncBlocks)

/* counter index range check 0..(2K-1) */
#define PRV_CPSS_DXCH_CNC_COUNTER_INDEX_VALID_CHECK_MAC(_dev, counterIndex) \
    LESS_CHECK_MAC(counterIndex, PRV_CPSS_DXCH_PP_MAC(_dev)->fineTuning.tableSize.cncBlockNumEntries)

/* CNC clients configuration */
#define CNC_BLOCK_CLIENT_CFG_ADDR_INCREMENT_CNS  0x00000100

/* register address calculation for CNC client configuration */
#define PRV_CPSS_DXCH_CNC_CLIENT_CFG_ADDR_MAC(_devNum, _blockNum, _clientIndex) \
     (PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->cncRegs.blockClientCfgAddrBase       \
        + (CNC_BLOCK_CLIENT_CFG_ADDR_INCREMENT_CNS * (_blockNum))               \
        + (4 * ((_clientIndex) / 2)))

/*******************************************************************************
* prvCpssDxChCncBlockClientToCfgAddr
*
* DESCRIPTION:
*   The function converts the client to HW base address of configuration register.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum         - device number
*       client         - CNC client
*       blockNum       - CNC block number
*                        valid range see in datasheet of specific device.
* OUTPUTS:
*       cfgRegPtr    - (pointer to) address of the block configuration register.
*       bitOffsetPtr - (pointer to) bit offset of the block configuration.
*
* RETURNS:
*       GT_OK          - on success
*       GT_BAD_PARAM   - on wrong parameters
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChCncBlockClientToCfgAddr
(
    IN   GT_U8                     devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *cfgRegPtr,
    OUT  GT_U32                    *bitOffsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChCnch */


/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChCnc.h,v $
 * Revision 1.1  2015/02/13 11:31:35  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
