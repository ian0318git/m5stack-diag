/* $Id: mvHwsSerdesIf.h,v 1.1 2015/02/13 11:35:01 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/mvHwsSerdesIf.h,v $
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
* mvHwsSerdesIf.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/

#ifndef __mvHwsSerdesIf_H
#define __mvHwsSerdesIf_H

#ifdef __cplusplus
extern "C" {
#endif


/* General H Files */
#include <port/private/mvHwsPortTypes.h>
#include <port/private/mvHwsPortMiscIf.h>

/*
 * Typedef: enum MV_HWS_SERDES_TX_PATTERN
 *
 * Description: Defines different serdes transmit patterns.
 *
 * Comment: Other - as output value means any other mode not mentioned
 *                  in this enum;
 *                  as input means HW default for PHY Test Data 5
 *                  register;
 *
 */
typedef enum
{
  _1T,
  _2T,
  _5T,
  _10T,
  PRBS7,
  PRBS9,
  PRBS15,
  PRBS23,
  PRBS31,
  DFETraining,
  Other

}MV_HWS_SERDES_TX_PATTERN;

/*
 * Typedef: enum MV_HWS_SERDES_TEST_GEN_MODE
 *
 * Description: Defines different serdes test generator modes.
 *
 */
typedef enum
{
  SERDES_NORMAL,
  SERDES_TEST

}MV_HWS_SERDES_TEST_GEN_MODE;

/*
 * Typedef: struct MV_HWS_SERDES_TEST_GEN_STATUS
 *
 * Description: Defines serdes test generator results.
 *
 */
typedef struct
{
  GT_U32  errorsCntr;
  GT_U64  txFramesCntr;
  GT_U32  lockStatus;

}MV_HWS_SERDES_TEST_GEN_STATUS;

/*
 * Typedef: struct MV_HWS_SERDES_EOM_RES
 *
 * Description: Defines serdes EOM results.
 *
 */
typedef struct
{
  GT_U32  horizontalEye;
  GT_U32  verticalEye;
  GT_U32  eomMechanism;

}MV_HWS_SERDES_EOM_RES;

typedef struct
{
    GT_U32 startAlign90;
    GT_U32 rxTrainingCfg;
    GT_U32 osDeltaMax;

}MV_HWS_ALIGN90_PARAMS;


/*******************************************************************************
* mvHwsSerdesTestGen
*
* DESCRIPTION:
*       Activates the Serdes test generator/checker.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       txPattern - pattern to transmit
*       mode      - test mode or normal
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesTestGen
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_SERDES_TEST_GEN_MODE mode
);

/*******************************************************************************
* mvHwsSerdesTestGenGet
*
* DESCRIPTION:
*       Get configuration of the Serdes test generator/checker.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - type of SerDes
*
* OUTPUTS:
*       txPattern - pattern to transmit
*       mode      - test mode or normal
*
* RETURNS:
*       GT_OK  - on success
*       GT_NOT_SUPPORTED - on wrong serdes type
*       GT_FAIL - HW error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesTestGenGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_SERDES_TX_PATTERN  *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/*******************************************************************************
* mvHwsSerdesTestGenStatus
*
* DESCRIPTION:
*       Read the tested pattern receive error counters and status.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       txPattern - pattern to transmit
*
* OUTPUTS:
*       Counters and status.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/*******************************************************************************
* mvHwsSerdesEOMGet
*
* DESCRIPTION:
*       Returns the horizontal/vertical Rx eye margin.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       timeout   - wait time in msec
*
* OUTPUTS:
*       horizontal/vertical Rx eye margin and EOM status.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesEOMGet
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TYPE        serdesType,
    GT_U32                    timeout,
    MV_HWS_SERDES_EOM_RES     *results
);

/*******************************************************************************
* mvHwsSerdesPolarityConfig
*
* DESCRIPTION:
*       Per SERDES invert the Tx or Rx.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesPolarityConfig
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           invertTx,
    GT_BOOL           invertRx
);

/*******************************************************************************
* mvHwsSerdesPolarityConfigGet
*
* DESCRIPTION:
*       Per SERDES invert the Tx or Rx.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       invertTx  - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
*       invertRx  - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    MV_HWS_SERDES_TYPE serdesType,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
);

/*******************************************************************************
* mvHwsSerdesFixAlign90Start
*
* DESCRIPTION:
*       Start fix Align90 process on current SERDES.
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - serdes type
*       params    - pointer to structure to store SERDES configuration parameters that must be restore
*                   during process stop operation
*
* OUTPUTS:
*       params    - SERDES configuration parameters that must be restore
*                   during process stop operation
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesFixAlign90Start
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_ALIGN90_PARAMS   *params
);

/*******************************************************************************
* mvHwsSerdesFixAlign90Status
*
* DESCRIPTION:
*       Return fix Align90 process current status on current SERDES.
*       Can be run after create port and start Align90.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - serdes type
*
* OUTPUTS:
*       trainingPass - true, if process finished and passed;
*                    - false, otherwise (doesn't finished or passed)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesFixAlign90Status
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_AUTO_TUNE_STATUS *trainingStatus
);

/*******************************************************************************
* mvHwsSerdesFixAlign90Stop
*
* DESCRIPTION:
*       Stop fix Align90 process on current SERDES.
*       Can be run after create port and start Align90.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - serdes type
*       params    - SERDES parameters that must be restored (return by mvHwsComHRev2SerdesFixAlign90Start)
*       fixAlignPass - true, if fix Align90 process passed; false otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesFixAlign90Stop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_SERDES_TYPE      serdesType,
    MV_HWS_ALIGN90_PARAMS   *params,
    GT_BOOL                 fixAlignPass
);

/*******************************************************************************
* mvHwsSerdesManualRxConfigGet
*
* DESCRIPTION:
*       Return SERDES RX parameters configured manually .
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       serdesType - serdes type
*
* OUTPUTS:
*       sqlchPtr - Squelch threshold.
*       ffeResPtr - FFE R
*       ffeCapPtr - FFE C
*       aligPtr   - align 90 value
*
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsSerdesManualRxConfigGet
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN GT_U32              serdesNum,
    IN MV_HWS_SERDES_TYPE  serdesType,
    OUT GT_U32             *sqlchPtr,
    OUT GT_U32             *ffeResPtr,
    OUT GT_U32             *ffeCapPtr,
    OUT GT_U32             *aligPtr
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsSerdesIf_H */

/*
 *------------------------------------------------------------------
 * $Log: mvHwsSerdesIf.h,v $
 * Revision 1.1  2015/02/13 11:35:01  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
