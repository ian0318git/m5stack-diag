/* $Id: mvDdr3TrainingIf.h,v 1.1 2015/02/13 11:34:24 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private/mvDdr3TrainingIf.h,v $
 *------------------------------------------------------------------
 *
 * Ian Chang - Nov. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvDdr3TrainingIf.h
*
* DESCRIPTION:
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwDDR3TrainingIf_H
#define __mvHwDDR3TrainingIf_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* mvHwsDdr3SlowFreqSetup
*
* DESCRIPTION:
*       DUNIT slow frequency training setup
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       memoryType  - memory type to init (FWR, LU)
*       dllOff      - enable/disable DLL OFF reature
*       swTuning    - run SW or HW training process
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3SlowFreqSetup
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_BOOL                 dllOff,
    GT_BOOL                 swTuning
);

/*******************************************************************************
* mvHwsDdr3MedFreqSetup
*
* DESCRIPTION:
*       DUNIT medium frequency training setup
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       memoryType  - memory type to init (FWR, LU)
*       dllOff      - enable/disable DLL OFF reature
*       swTuning    - run SW or HW training process
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3MedFreqSetup
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_BOOL                 dllOff,
    GT_BOOL                 swTuning
);

/*******************************************************************************
* mvHwsDdr3HighFreqSetup
*
* DESCRIPTION:
*       DUNIT high frequency training setup
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       memoryType  - memory type to init (FWR, LU)
*       dllOff      - enable/disable DLL OFF reature
*       swTuning    - run SW or HW training process
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3HighFreqSetup
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_BOOL                 dllOff,
    GT_BOOL                 swTuning
);

/*******************************************************************************
* mvHwsDdr3SwReadLeveling
*
* DESCRIPTION:
*       Run SW read Leveling configuration for
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memIfNum  - memory interface number (0-13 for LU, 0-7 for FWD)
*       rdReadyDel - read Ready delay to start from
*       rdSmplDel  - read Sample delay to start from
*       phases     - phase to start from
*       aDll       - ADLL to start from
*
* OUTPUTS:
*       windowSize - DDR3 window size in taps.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3SwReadLeveling
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_U32                  memIfNum,
    GT_U32                  rdReadyDel,
    GT_U32                  rdSmplDel,
    GT_U32                  phases,
    GT_U32                  aDll,
    GT_U32                  *windowSize
);

/*******************************************************************************
* ddr3DqsIfCentralization
*
* DESCRIPTION:
*       Execute the DQS centralization RX or TX phase
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memIfNum  - memory interface number (0-13 for LU, 0-7 for FWD)
*       direct    - checking direction (DDR3 Tx or Rx)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3DqsIfCentralization
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_U32                  memNum,
    MV_DDR3_DIRECTION       direct
);

/*******************************************************************************
* ddr3DqsCentralizationResult
*
* DESCRIPTION:
*       Return last calculated DQS centralization limits
*
* INPUTS:
*       pupNum    - system device number
*
* OUTPUTS:
*       lowLimit  - ADLL low value
*       highLimit - ADLL high value
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3DqsCentralizationResult
(
    GT_U32 pupNum,
    GT_32 *lowLimit,
    GT_32 *highLimit
);

/*******************************************************************************
* ddr3PbsRx
*
* DESCRIPTION:
*       Execute the PBS RX phase
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memIfNum  - memory interface number (0-13 for LU, 0-7 for FWD)
*
* OUTPUTS:
*       skew     - PBS value
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3PbsRx
(
    GT_U8               devNum,
    GT_U32              portGroup,
    MV_HWS_EMC_TYPE     memType,
    GT_U32              memNum
);

/*******************************************************************************
* ddr3PbsTx
*
* DESCRIPTION:
*       Execute the PBS TX phase
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memIfNum  - memory interface number (0-13 for LU, 0-7 for FWD)
*
* OUTPUTS:
*       skew     - PBS value
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3PbsTx
(
    GT_U8               devNum,
    GT_U32              portGroup,
    MV_HWS_EMC_TYPE     memType,
    GT_U32              memNum
);

/*******************************************************************************
* ddr3PbsCalcResult
*
* DESCRIPTION:
*       Return last calculated PBS value
*
* INPUTS:
*       pupNum    - system device number
*       dqNum     - DQ number
*
* OUTPUTS:
*       skew     - PBS value
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3PbsCalcResult
(
    GT_U32 pupNum,
    GT_U32 dqNum,
    GT_32 *skew
);

/*******************************************************************************
* mvResetDunit
*
* DESCRIPTION:
*       Reset DUnit.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*
* OUTPUTS:
*       none
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvResetDunit
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_EMC_TYPE memType
);


#ifdef __cplusplus
}
#endif

#endif /* __mvHwDDR3TrainingIf_H */
/*
 *------------------------------------------------------------------
 * $Log: mvDdr3TrainingIf.h,v $
 * Revision 1.1  2015/02/13 11:34:24  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
