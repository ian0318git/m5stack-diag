/* $Id: mvDdr3StaticCfg.h,v 1.1 2015/02/13 11:34:24 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private/mvDdr3StaticCfg.h,v $
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
* mvDdr3StaticCfg.h
*
* DESCRIPTION: DDR3 static configuration data
*       
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef _MV_DDR3_STATIC_CFG_H
#define _MV_DDR3_STATIC_CFG_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct  
{
    GT_U32 ifNum;
    GT_U32 pupNum;
    GT_U32 rdSmplDel; /* data written to 0x1538 */
    GT_U32 rdReadyDel; /* data written to 0x153C */
    GT_U32 readLevelData; /* PUP reg 2 */

} HWS_DRAM_STATIC_CFG;


/*******************************************************************************
* mvHwsDdr3StaticCfg
*
* DESCRIPTION:
*       Run static configuration for specified parameters
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDdr3StaticCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    MV_HWS_EMC_TYPE         memType,
    HWS_DRAM_STATIC_CFG     *cfgArr,
    GT_U32                  cfgArrSize
);

#ifdef __cplusplus
}
#endif

#endif /* _MV_DDR3_STATIC_CFG_H */
/*
 *------------------------------------------------------------------
 * $Log: mvDdr3StaticCfg.h,v $
 * Revision 1.1  2015/02/13 11:34:24  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
