/* $Id: prvCpssDxChHwInit.h,v 1.1 2015/02/13 11:31:46 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h,v $
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
* prvCpssDxChHwInit.h
*
* DESCRIPTION:
*       Includes Core level basic Hw initialization functions, and data
*       structures.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __prvCpssDxChHwInith
#define __prvCpssDxChHwInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#define PRV_CPSS_DXCH_PROFILES_NUM_CNS       4
#define PRV_CPSS_DXCH2_PROFILES_NUM_CNS      8

#define PRV_CPSS_XCAT_NETWORK_PORTS_SERDES_NUM_CNS  6
#define PRV_CPSS_XCAT_SERDES_NUM_CNS        22
#define PRV_CPSS_LION_SERDES_NUM_CNS        24
#define PRV_CPSS_LION2_SERDES_NUM_CNS       24

#define PRV_CPSS_XCAT2_SERDES_NUM_CNS       10

/* For init system with HW access and register defaults */
extern GT_BOOL dxChInitRegDefaults;


/*******************************************************************************
* prvCpssDxChHwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for cheetah devices.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChHwRegAddrInit
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxCh2HwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for cheetah2 devices.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxCh2HwRegAddrInit
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxCh3HwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for cheetah-3 devices.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxCh3HwRegAddrInit
(
    IN GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChXcatHwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for DxCh xCat devices.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChXcatHwRegAddrInit
(
    IN GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChLionHwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for DxCh Lion devices.
*
* APPLICABLE DEVICES:
*        Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChLionHwRegAddrInit
(
    IN GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChLion2HwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for DxCh Lion2 devices.
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChLion2HwRegAddrInit
(
    IN GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChLion3HwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for DxCh Lion2 devices.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChLion3HwRegAddrInit
(
    IN GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChXcat2HwRegAddrInit
*
* DESCRIPTION:
*       This function initializes the registers struct for DxCh xCat2 devices.
*
* APPLICABLE DEVICES:
*        xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* INPUTS:
*       devNum  - The PP's device number to init the struct for.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_OUT_OF_CPU_MEM        - on malloc failed
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChXcat2HwRegAddrInit
(
    IN GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChHwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxChHwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxCh2HwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxCh2HwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxCh2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxCh3HwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxCh3HwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxCh3HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChXcatHwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxChXcatHwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChXcatHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChXcat2HwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxChXcat2HwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChXcat2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChLionHwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxChLionHwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChLionHwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChLion2HwRegAddrRemove
*
* DESCRIPTION:
*       This function release the memory that was allocated by the function
*       prvCpssDxChLionHwRegAddrInit(...)
*
* APPLICABLE DEVICES:
*        Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* INPUTS:
*       devNum  - The PP's device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChLion2HwRegAddrRemove
(
    IN  GT_U32 devNum
);

/*******************************************************************************
* prvCpssDxChHwRegAddrPortMacUpdate
*
* DESCRIPTION:
*       This function updates mac registers addresses for given port accordingly
*           to currently used MAC Unit
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       ifMode   - new interface mode used with this MAC
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChHwRegAddrPortMacUpdate
(
    IN GT_U32 devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);

/*******************************************************************************
* prvCpssDxCh3HwPpPllUnlockWorkaround
*
* DESCRIPTION:
*       Workaround to eliminate the PLL unlocking issue.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum                  - device number
*       networkSerdesPowerUpBmp - bitmap of network SERDES to be power UP
*       xgDevice                - GT_TRUE for XG device, GT_FALSE for GE device
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on other error.
*
* COMMENTS:
*       PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC usage has actual influence
*       only for XG devices, where it is used to get the port number from
*       the power up SERDES bitmap (networkSerdesPowerUpBmp).
*       For GE devices the macro does nothing usefull since previouse one
*       (PRV_CPSS_DXCH_SKIP_NOT_EXIST_SERDES_MAC) make all necessary checks.
*
*******************************************************************************/
GT_STATUS prvCpssDxCh3HwPpPllUnlockWorkaround
(
    IN GT_U8    devNum,
    IN GT_U32   networkSerdesPowerUpBmp,
    IN GT_BOOL  xgDevice
);

/*******************************************************************************
* prvCpssDxChHwEarchRegAddrSet
*
* DESCRIPTION:
*       Enable/Disble eArch features bits.
*
* APPLICABLE DEVICES:
*        Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - device number
*       earchEnable     - GT_TRUE  : enable eArch bits
*                         GT_FALSE : disable eArch bits
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success, or
*       GT_BAD_PARAM             - wrong device type to operate
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS   prvCpssDxChHwEarchRegAddrSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  earchEnable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #define __prvCpssDxChHwInith
 */

/*
 *------------------------------------------------------------------
 * $Log: prvCpssDxChHwInit.h,v $
 * Revision 1.1  2015/02/13 11:31:46  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
