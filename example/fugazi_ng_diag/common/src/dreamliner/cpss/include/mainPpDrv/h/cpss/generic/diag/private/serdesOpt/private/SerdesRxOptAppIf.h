/* $Id: SerdesRxOptAppIf.h,v 1.1 2015/02/13 11:33:39 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/private/SerdesRxOptAppIf.h,v $
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
* SersesRxOptAppIf.h
*
* DESCRIPTION:
*       Application IF defintion.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __serdesRxOptAppIf_H
#define __serdesRxOptAppIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

typedef enum
{
    Unknown,
    LionB,
    Xcat,
    BullsEye,
    Milos4

}MV_SERDES_TEST_DEV_TYPE;

/*******************************************************************************
* mvDeviceInfoGetDevType
*
* DESCRIPTION:
*       Return device type for device number.
*       Supported device types are: LionB, Xcat.
*
* INPUTS:
*       devNum - device number in the system.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Current device type or Unknown.
*
*******************************************************************************/
MV_SERDES_TEST_DEV_TYPE mvDeviceInfoGetDevType
(
    GT_U32    devNum
);

#ifndef __siliconIf_H

/*******************************************************************************
* genRegisterSet
*
* DESCRIPTION:
*       Implement write access to device registers.
*
* INPUTS:
*       devNum    - Device Number
*       portGroup - port group (core) number
*       address   - address to access
*       data      - data to write
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
int genRegisterSet
(
    GT_U8  devNum, 
    GT_U32 portGroup,
    GT_U32 address, 
    GT_U32 data, 
    GT_U32 mask
);

/*******************************************************************************
* genRegisterGet
*
* DESCRIPTION:
*       Read access to device registers.
*
* INPUTS:
*       devNum    - Device Number
*       portGroup - port group (core) number
*       address   - address to access
*
* OUTPUTS:
*       data      - read data
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
int genRegisterGet 
(
    GT_U8 devNum, 
    GT_U32 portGroup,
    GT_U32 address, 
    GT_U32 *data, 
    GT_U32 mask
);

#endif 

#ifdef __cplusplus
}
#endif

#endif /* __serdesRxOptDevIf_H */


/*
 *------------------------------------------------------------------
 * $Log: SerdesRxOptAppIf.h,v $
 * Revision 1.1  2015/02/13 11:33:39  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
