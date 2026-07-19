/* $Id: mvComPhyHDb.h,v 1.1 2015/02/13 11:35:03 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyH/mvComPhyHDb.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell 
* International Ltd. and/or its affiliates ("Marvell") under the following 
* alternative licensing terms.  
* If you received this File from Marvell, you may opt to use, redistribute 
* and/or modify this File under the following licensing terms. 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer. 
*  -   Redistributions in binary form must reproduce the above copyright 
*       notice, this list of conditions and the following disclaimer in the 
*       documentation and/or other materials provided with the distribution. 
*  -    Neither the name of Marvell nor the names of its contributors may be 
*       used to endorse or promote products derived from this software without 
*       specific prior written permission. 
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
* mvComPhyHDb.h
*
* DESCRIPTION:
*       
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __mvComPhyHDb_H
#define __mvComPhyHDb_H


#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <common/configElementDb/mvCfgElementDb.h>

typedef enum
{ 
  SERDES_SD_RESET_SEQ,
  SERDES_SD_UNRESET_SEQ,
  SERDES_RF_RESET_SEQ,
  SERDES_RF_UNRESET_SEQ,
  SERDES_SYNCE_RESET_SEQ,
  SERDES_SYNCE_UNRESET_SEQ,

  SERDES_SERDES_POWER_UP_CTRL_SEQ,
  SERDES_SERDES_POWER_DOWN_CTRL_SEQ,
  SERDES_SERDES_RXINT_UP_SEQ,
  SERDES_SERDES_RXINT_DOWN_SEQ,
  SERDES_SERDES_WAIT_PLL_SEQ,

    SERDES_SPEED_1_25G_SEQ,
    SERDES_SPEED_3_125G_SEQ,
    SERDES_SPEED_3_75G_SEQ,
    SERDES_SPEED_4_25G_SEQ,
    SERDES_SPEED_5G_SEQ,
    SERDES_SPEED_6_25G_SEQ,
    SERDES_SPEED_7_5G_SEQ,
    SERDES_SPEED_10_3125G_SEQ,
  
  SERDES_SD_LPBK_NORMAL_SEQ,
  SERDES_SD_ANA_TX_2_RX_SEQ,
  SERDES_SD_DIG_TX_2_RX_SEQ,
  SERDES_SD_DIG_RX_2_TX_SEQ,

  SERDES_PT_AFTER_PATTERN_NORMAL_SEQ,
  SERDES_PT_AFTER_PATTERN_TEST_SEQ,
  
  SERDES_RX_TRAINING_ENABLE_SEQ,
  SERDES_RX_TRAINING_DISABLE_SEQ,
  SERDES_TX_TRAINING_ENABLE_SEQ,
  SERDES_TX_TRAINING_DISABLE_SEQ,

  SERDES_SPEED_12_5G_SEQ,
  SERDES_SPEED_3_3G_SEQ,

  SERDES_SPEED_11_5625G_SEQ,
  SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ,
  SERDES_SERDES_PARTIAL_POWER_UP_SEQ,

  MV_SERDES_LAST_SEQ
}MV_HWS_COM_PHY_H_SUB_SEQ;

extern MV_CFG_SEQ hwsSerdesSeqDb[MV_SERDES_LAST_SEQ];

GT_STATUS hwsComPhyHSeqInit(void);
void hwsComPhyHSeqFree(void);
GT_STATUS hwsComPhyHSeqGet(MV_HWS_COM_PHY_H_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum);
GT_STATUS hwsComPhyHSeqSet(GT_BOOL firstLine, MV_HWS_COM_PHY_H_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 numOfOp);

#ifdef __cplusplus
}
#endif

#endif /* __mvComPhyHDb_H */

/*
 *------------------------------------------------------------------
 * $Log: mvComPhyHDb.h,v $
 * Revision 1.1  2015/02/13 11:35:03  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
