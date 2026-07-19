/* $Id: mvDdr3SdramIf.h,v 1.1 2015/02/13 11:34:24 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private/mvDdr3SdramIf.h,v $
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
* mvDdr3SdramIf.h
*
* DESCRIPTION: DDR3 SDRAM interface implementation
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvDdr3SdramIf_H
#define __mvDdr3SdramIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <common/siliconIf/mvSiliconIf.h>
#include <ddr3/mvHwsDdr3InitIf.h>
#include <ddr3/private/mvDdr3PrvIf.h>
#include <ddr3/private/mvDdr3Regs.h>

typedef enum
{
    DDR3_READ_LEVELING_PATTERN = 1,
    DDR3_PBS_PATTERN,
    DDR3_DQS_BASIC_PATTERN,
    DDR3_DQS_ADVANCE_PATTERN,

    DDR3_LAST_PATTERN
}HWS_DDR3_SDRAM_PATTERNS;


/*******************************************************************************
* ddr3SdramPbsCompare
*
* DESCRIPTION:
*       Execute SRAM compare per PUP and DQ
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memNum    - memory interface number
*       rxTx      - PBS RX ot PBS TX test
*       uiPbsPatternIdx - PBS pattern number
*
* OUTPUTS:
*       failedArray - store DQ failed bit per PUP, DQ number
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramPbsCompare
(
    GT_U8   devNum,
    GT_U32  portGroup,
    MV_HWS_EMC_TYPE  memType,
    GT_U32  memNum,
    MV_DDR3_DIRECTION   rxTx,
    GT_U32  uiPbsPatternIdx,
    GT_U32          failedArray[MAX_PUP_NUM][PUP_DQ_NUM]
);

/*******************************************************************************
* ddr3SdramPbsDqsCompare
*
* DESCRIPTION:
*       Execute SRAM regular compare
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memNum    - memory interface number
*       pbsPatternNum - PBS pattern number
*       uiSdramOffset - offset of pattern in memory
*
* OUTPUTS:
*       puiNewLockedPup - store failed PUPs bitmap
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_U32 ddr3SdramPbsDqsCompare
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_EMC_TYPE memType,
    GT_U32          memNum,
    GT_U32          *puiNewLockedPup,
    GT_U32          pbsPatternNum,
    GT_U32          uiSdramOffset
);

/*******************************************************************************
* ddr3SdramWriteKillerPattern
*
* DESCRIPTION:
*       Write killer pattern for specified DQ to all memory interfaces from
*       specified type
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       dqPatternAddr  - memory address to write pattern
*       patternEntries - number of entries in pattern
*       dqNum     - DQ number
*
* OUTPUTS:
*       none
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramWriteKillerPattern
(
    GT_U8           devNum,
    GT_U32          portGroup,
    GT_U32          memType,
    GT_U32          dqPatternAddr,
    GT_U32          patternEntries,
    GT_U32          dqNum
);

/*******************************************************************************
* ddr3SdramDqsAdvIICompare
*
* DESCRIPTION:
*       Execute compare per PUP od DQS killer (advanced) pattern
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memNum    - number od memory interface
*       dqNum     - DQ number
*       patternEntries - number of entries in pattern
*       uiSdramOffset - offset of pattern in memory
*
* OUTPUTS:
*       puiNewLockedPup -  bit array of the pups with failed compare
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramDqsAdvIICompare
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_EMC_TYPE memType,
    GT_U32          memNum,
    GT_U32          *puiNewLockedPup,
    GT_U32          dqNum,
    GT_U32          patternEntries,
    GT_U32          uiSdramOffset
);

/*******************************************************************************
* ddr3SdramDqsCompare
*
* DESCRIPTION:
*       Execute compare per PUP od DQS killer (basic) pattern
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memNum    - number od memory interface
*       dqNum     - DQ number
*       patternEntries - number of entries in pattern
*       uiSdramOffset - offset of pattern in memory
*
* OUTPUTS:
*       puiNewLockedPup -  bit array of the pups with failed compare
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramDqsCompare
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_EMC_TYPE memType,
    GT_U32          memNum,
    GT_U32          *puiNewLockedPup,
    GT_U32          dqNum,
    GT_U32          patternEntries,
    GT_U32          uiSdramOffset
);

/*******************************************************************************
* ddr3SdramDqsAdvCompare
*
* DESCRIPTION:
*       Execute compare per PUP od DQS killer (advanced) pattern
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type to init (FWR, LU)
*       memNum    - number od memory interface
*       dqNum     - DQ number
*       patternEntries - number of entries in pattern
*       uiSdramOffset - offset of pattern in memory
*
* OUTPUTS:
*       puiNewLockedPup -  bit array of the pups with failed compare
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramDqsAdvCompare
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_EMC_TYPE memType,
    GT_U32          memNum,
    GT_U32          *puiNewLockedPup,
    GT_U32          dqNum,
    GT_U32          patternEntries,
    GT_U32          uiSdramOffset
);

/*******************************************************************************
* ddr3SdramWriteAllPatterns
*
* DESCRIPTION:
*       Write all patterns to memory interface/s from specified type
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type (FWR, LU)
*       memNumber - memory number
*       isBroadcast - enable / disable broadcast access to memory interface/s
*       pattern     - specified pattern type or all patterns
*
* OUTPUTS:
*       none
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramWriteAllPatterns
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    GT_U32                  memNumber,
    GT_BOOL                 isBroadcast,
    HWS_DDR3_SDRAM_PATTERNS pattern
);

/*******************************************************************************
* ddr3SdramWritePbsPattern
*
* DESCRIPTION:
*       Write PBS pattern to memory interface/s from specified type
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - memory type (FWR, LU)
*       memNumber - memory number
*       isBroadcast - enable / disable broadcast access to memory interface/s
*       pattern     - specified pattern type or all patterns
*
* OUTPUTS:
*       none
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS ddr3SdramWritePbsPattern
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_EMC_TYPE memType
);

/*******************************************************************************
* ddr3SdramPbsInit
*
* DESCRIPTION:
*       Init PBS data structures
*
* INPUTS:
*       uiMaxPup    - max number of PUPs
*
* OUTPUTS:
*       none
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void ddr3SdramPbsInit(GT_U32 uiMaxPup);

#ifdef __cplusplus
}
#endif

#endif /* __mvDdr3SdramIf_H*/
/*
 *------------------------------------------------------------------
 * $Log: mvDdr3SdramIf.h,v $
 * Revision 1.1  2015/02/13 11:34:24  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
