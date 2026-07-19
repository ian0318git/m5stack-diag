/* $Id: cpssGenCfg.h,v 1.1 2015/02/13 11:33:12 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/config/cpssGenCfg.h,v $
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
* cpssGenCfg.h
*
* DESCRIPTION:
*       CPSS generic configuration types.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssGenCfgh
#define __cpssGenCfgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/*
 * typedef: CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT
 *
 * Description: defines device's HW device usage
 *
 * Enumerations:
 *  CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E - device uses single HW device number
 *  CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E - device uses dual HW device number
*/

typedef enum
{
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E,
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E
} CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT;


/*
 * typedef: struct CPSS_GEN_CFG_DEV_INFO_STC
 *
 * Description: Generic device info structure
 *
 * Fields:
 *      devType        - device type of the PP.
 *      revision       - the device's revision number.
 *      devFamily      - CPSS's device family that device belongs to.
 *      maxPortNum     - maximal port's number not including CPU one.
 *      numOfVirtPorts - number of virtual ports.
 *                       Relevant only for devices with virtual ports support.
 *      existingPorts  - bitmap of actual exiting ports not including CPU one.
 *      hwDevNumMode   - HW device number mode.
 *      cpuPortMode    - CPU port mode.
 */
typedef struct
{
    CPSS_PP_DEVICE_TYPE              devType;
    GT_U8                            revision;
    CPSS_PP_FAMILY_TYPE_ENT          devFamily;
    GT_U8                            maxPortNum;
    GT_U8                            numOfVirtPorts;
    CPSS_PORTS_BMP_STC               existingPorts;
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevNumMode;
    CPSS_NET_CPU_PORT_MODE_ENT       cpuPortMode;
}CPSS_GEN_CFG_DEV_INFO_STC;

/*******************************************************************************
* cpssPpCfgNextDevGet
*
* DESCRIPTION:
*
*       Return the number of the next existing device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device number to start from. For the first one devNum should be 0xFF.
*
* OUTPUTS:
*       nextDevNumPtr - number of next device after devNum.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
*       GT_BAD_PARAM             - devNum > max device number
*       GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenCfgh */

/*
 *------------------------------------------------------------------
 * $Log: cpssGenCfg.h,v $
 * Revision 1.1  2015/02/13 11:33:12  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
