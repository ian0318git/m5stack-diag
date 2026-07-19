/* $Id: mvHwsDdr3InitIf.h,v 1.1 2015/02/13 11:34:22 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/mvHwsDdr3InitIf.h,v $
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
* mvHwsDdr3InitIf.h
*
* DESCRIPTION:
*       DDR3 interface
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwDDR3If_H
#define __mvHwDDR3If_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

#define MV_DDR3_INIT_ERROR 0xFFFFFFFF

/*
 * Typedef: enum MV_HWS_MEMORY_TYPE
 *
 * Description: Defines the different memory types.
 *
 */
typedef enum
{
    MV_LOOKUP_EMC,
    MV_FORWARD_EMC

}MV_HWS_EMC_TYPE;

/*
 * Typedef: enum MV_HWS_TARGET_FREQ
 *
 * Description: Defines the different target frequency values.
 *
 */
typedef enum
{
    FREQ_667,
    FREQ_800,

    LAST_FREQ

}MV_HWS_TARGET_FREQ;

/*
 * Typedef: enum MV_HWS_CWL
 *
 * Description: Defines the CAS write latency values.
 *
 */
typedef enum
{
    CWL5,
    CWL6,
    CWL7,
    CWL8,
    CWL9,
    CWL10,
    CWL11,
    CWL12

}MV_HWS_CWL;

/*
 * Typedef: enum MV_HWS_CAS_L
 *
 * Description: Defines the CAS Latency value
 *              CL[3:0] (DDR3_MR0_Reg{[6:4],[2]})
 */
typedef enum
{
    CL_12 = 1,
    CL_5,
    CL_13,
    CL_6,
    CL_14,
    CL_7,
    CL_8,
    CL_9,
    CL_10,
    CL_11

}MV_HWS_CAS_L;

/*
* DRMA memory size supported
*/
typedef enum
{
    DDR3_2Gb = 0,
    DDR3_Reserved,
    DDR3_512Mb,
    DDR3_1Gb,
    DDR3_4Gb,
    DDR3_8Gb
}MV_HWS_DDR3_MEM_SIZE;

/*
 * Typedef: enum MV_HWS_MEMORY_CFG_INFO
 *
 * Description: Defines the vendor memory parameters.
 *
 */
typedef struct _mvDramInfo
{
    GT_U32      rdSmplDly;
    GT_U32      rdRdyDly;
    GT_U32      casL;
    MV_HWS_CWL  casWL;
    MV_HWS_DDR3_MEM_SIZE    size;

} MV_HWS_MEMORY_CFG_INFO;


/*******************************************************************************
* mvHwsDdr3GenCfg
*
* DESCRIPTION:
*       Init DDR3 interface with global configuration.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memType   - DDR3 interface type
*       freq      - memory target frequency
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3GenCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    MV_HWS_TARGET_FREQ      freq,
    MV_HWS_CWL              casWL,
    MV_HWS_CAS_L            casL,
    MV_HWS_DDR3_MEM_SIZE    size
);


/*******************************************************************************
* mvHwsDdr3Puma3Init
*
* DESCRIPTION:
*       Init DDR3 interface.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memoryType - memory type to init (FWR, LU)
*       memCfgParam - memory configuration parameters
*       targetFreq  - target frequency
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3Puma3Init
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memoryType,
    MV_HWS_MEMORY_CFG_INFO  memCfgParam,
    MV_HWS_TARGET_FREQ      targetFreq

);

/*******************************************************************************
* mvHwsDdr3Puma3TuneAlg
*
* DESCRIPTION:
*       Run DDR3 tune algorithm.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       memoryType - memory type to init (FWR, LU)
*       hwsDllOff  - if true, run with DLL Off
*       readLevelingMode  - if 0 - no read leveling run, 1 - HW RL, 2 - SW RL
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3Puma3TuneAlg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memoryType,
    GT_BOOL                 hwsDllOff,
    GT_U32                  readLevelingMode
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwDDR3If_H */


/*
 *------------------------------------------------------------------
 * $Log: mvHwsDdr3InitIf.h,v $
 * Revision 1.1  2015/02/13 11:34:22  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
