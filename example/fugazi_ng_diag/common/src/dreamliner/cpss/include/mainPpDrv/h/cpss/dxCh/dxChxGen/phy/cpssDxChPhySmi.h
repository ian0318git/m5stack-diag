/* $Id: cpssDxChPhySmi.h,v 1.1 2015/02/13 11:32:25 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h,v $
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
* cpssDxChPhySmi.h
*
* DESCRIPTION:
*        API implementation for port Core Serial Management Interface facility.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#ifndef __cpssDxChPhySmih
#define __cpssDxChPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/phy/cpssGenPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>


/*******************************************************************************
* cpssDxChPhyPortSmiInit
*
* DESCRIPTION:
*       Initialiaze the SMI control register port, Check all GE ports and
*       activate the errata initialization fix.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyPortSmiInit
(
    IN GT_U8 devNum
);

/*******************************************************************************
* cpssDxChPhyPortSmiRegisterRead
*
* DESCRIPTION:
*       Read specified SMI Register on a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       phyReg    - SMI register (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       dataPtr   - (pointer to) the read data.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_OUT_OF_RANGE          - phyAddr bigger then 31
*       GT_NOT_SUPPORTED         - for XG ports
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_INITIALIZED       - Flex port SMI or PHY address were not set
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyPortSmiRegisterRead
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     phyReg,
    OUT GT_U16    *dataPtr
);

/*******************************************************************************
* cpssDxChPhyPortSmiRegisterWrite
*
* DESCRIPTION:
*       Write value to specified SMI Register on a specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number.
*       portNum     - physical port number.
*       phyReg      - The new phy address, (APPLICABLE RANGES: 0...31).
*       data        - Data to write.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_NOT_SUPPORTED         - for XG ports
*       GT_OUT_OF_RANGE          - phyAddr bigger then 31
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_INITIALIZED       - Flex port SMI or PHY address were not set
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyPortSmiRegisterWrite
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     phyReg,
    IN  GT_U16    data
);

/*******************************************************************************
* cpssDxChPhyPort10GSmiRegisterRead
*
* DESCRIPTION:
*       Read specified SMI Register and PHY device of specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - physical device number
*       portNum - physical port number
*       phyId   - ID of external PHY (APPLICABLE RANGES: 0..31).
*       useExternalPhy - boolean variable, defines if to use external PHY
*                       NOTE: for DxCh3 and above only GT_TRUE supported !
*       phyReg  - SMI register, the register of PHY to read from
*       phyDev  - the PHY device to read from (APPLICABLE RANGES: 0..31).
*
* OUTPUTS:
*       dataPtr - (Pointer to) the read data.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_OUT_OF_RANGE          - phyAddr bigger then 31
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       If useExternalPhy flag is GT_FALSE, the phyId is being ignored
*
*******************************************************************************/
GT_STATUS cpssDxChPhyPort10GSmiRegisterRead
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U8                    phyId,
    IN  GT_BOOL                  useExternalPhy,
    IN  GT_U16                   phyReg,
    IN  GT_U8                    phyDev,
    OUT GT_U16                   *dataPtr
);

/*******************************************************************************
* cpssDxChPhyPort10GSmiRegisterWrite
*
* DESCRIPTION:
*       Write value to a specified SMI Register and PHY device of
*       specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number.
*       portNum     - physical port number.
*       phyId       - ID of external PHY (APPLICABLE RANGES: 0...31).
*       useExternalPhy - Boolean variable, defines if to use external PHY
*                       NOTE: for DxCh3 and above only GT_TRUE supported !
*       phyReg      - SMI register, the register of PHY to read from
*       phyDev      - the PHY device to read from (APPLICABLE RANGES: 0..31).
*       data        - Data to write.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_OUT_OF_RANGE          - phyAddr bigger then 31
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       If useExternalPhy flag is GT_FALSE, the phyId is being ignored
*
*******************************************************************************/
GT_STATUS cpssDxChPhyPort10GSmiRegisterWrite
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U8                    phyId,
    IN  GT_BOOL                  useExternalPhy,
    IN  GT_U16                   phyReg,
    IN  GT_U8                    phyDev,
    IN  GT_U16                   data
);

/*******************************************************************************
* cpssDxChPhyAutonegSmiGet
*
* DESCRIPTION:
*       This function gets the auto negotiation status between the PP and PHY.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - physical device number.
*       portNum      - port number: applicable ports from 0 till 23
*
* OUTPUTS:
*       enabledPtr   - (Pointer to) the auto negotiation process state between
*                      PP and Phy:
*                      0 = Auto-Negotiation process is preformed.
*                      1 = Auto-Negotiation process is not preforme
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Although the device ignores the information read from the PHY
*       registers, it keeps polling those registers.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyAutonegSmiGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enabledPtr
);

/*******************************************************************************
* cpssDxChPhyAutonegSmiSet
*
* DESCRIPTION:
*       This function sets the auto negotiation process, between
*       the PP and PHY, to enable/disable.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number.
*       portNum     - port number: applicable ports from 0 till 23
*       enable      - 0 = Auto-Negotiation process is preformed.
*                     1 = Auto-Negotiation process is not preforme
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Although the device ignores the information read from the PHY
*       registers, it keeps polling those registers.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyAutonegSmiSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChPhySmiAutoMediaSelectSet
*
* DESCRIPTION:
*       This function gets a bit per port bitmap (of 6 ports)
*       indicating whether this port is connected to a dual-media PHY,
*       such as Marvell 88E1112, that is able to perform Auto-media select
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - physical device number.
*       portNum         - port number: applicable ports from 0 till 23
*       autoMediaSelect - Sets 6 bits.
*                         0 = Port is not connected to dual-media PHY
*                         1 = Port is connected to dual-media PHY
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_OUT_OF_RANGE          - phyAddr bigger then 31
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhySmiAutoMediaSelectSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32   autoMediaSelect

);

/*******************************************************************************
* cpssDxChPhySmiAutoMediaSelectGet
*
* DESCRIPTION:
*       This function gets a bit per port bitmap (of 6 ports)
*       indicating whether this port is connected to a dual-media PHY,
*       such as Marvell 88E1112, that is able to perform Auto-media select
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - physical device number.
*       portNum         - port number:applicable ports from 0 till 23
*
* OUTPUTS:
*       autoMediaSelectPtr - (Pointer to) port state bitMap of 6 bits:
*                            0 = Port is not connected to dual-media PHY
*                            1 = Port is connected to dual-media PHY
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or portNum.
*       GT_BAD_PTR               - on NULL pointer.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhySmiAutoMediaSelectGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32   *autoMediaSelectPtr
);

/*******************************************************************************
* cpssDxChPhyXsmiMdcDivisionFactorSet
*
* DESCRIPTION:
*       This function sets MDC frequency for Master XSMI Interface
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       divisionFactor  - Division factor of the core clock to get the MDC
*                                           (Serial Management Interface Clock).
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or divisionFactor
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyXsmiMdcDivisionFactorSet
(
    IN GT_U8 devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

/*******************************************************************************
* cpssDxChPhyXsmiMdcDivisionFactorGet
*
* DESCRIPTION:
*       This function gets MDC frequency for Master XSMI Interface
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*
* OUTPUTS:
*       divisionFactorPtr  - (Pointer to) Division factor of the core clock
*                           to get the MDC (Serial Management Interface Clock).
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or divisionFactor
*       GT_BAD_PTR               - on NULL pointer.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhyXsmiMdcDivisionFactorGet
(
    IN  GT_U8 devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);

/*******************************************************************************
* cpssDxChPhySmiMdcDivisionFactorSet
*
* DESCRIPTION:
*       This function sets Fast MDC Division Selector
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       portNum         - port number.
*       divisionFactor  - Division factor of the core clock to get the MDC
*                                           (Serial Management Interface Clock).
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or divisionFactor
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhySmiMdcDivisionFactorSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

/*******************************************************************************
* cpssDxChPhySmiMdcDivisionFactorGet
*
* DESCRIPTION:
*       This function gets Fast MDC Division Selector
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - physical device number.
*       portNum         - port number.
*
* OUTPUTS:
*       divisionFactorPtr  - (Pointer to) Division factor of the core clock
*                           to get the MDC (Serial Management Interface Clock).
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong devNum or divisionFactor
*       GT_BAD_PTR               - on NULL pointer.
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPhySmiMdcDivisionFactorGet
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPhySmih */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChPhySmi.h,v $
 * Revision 1.1  2015/02/13 11:32:25  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
