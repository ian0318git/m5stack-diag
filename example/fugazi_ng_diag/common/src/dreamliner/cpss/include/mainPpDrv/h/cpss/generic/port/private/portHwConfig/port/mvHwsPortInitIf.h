/* $Id: mvHwsPortInitIf.h,v 1.1 2015/02/13 11:34:26 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mvHwsPortInitIf.h,v $
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
* mvHwsPortInitIf.h
*
* DESCRIPTION:
*       
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwServicesPortIf_H
#define __mvHwServicesPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

/*
 * Typedef: enum MV_HWS_PORT_STANDARD
 *
 * Description: Defines the different port standard metrics.
 *
 */
typedef enum 
{
    _100Base_FX,
    SGMII,
    _1000Base_X,
    SGMII2_5,
    QSGMII,
    _10GBase_KX4,
    _10GBase_KX2,
    _10GBase_KR,
    _20GBase_KR,
    _40GBase_KR,
    _100GBase_KR10,
    HGL,
    RHGL,
    CHGL,           /* CHGL_LR10 */  
    RXAUI,
    _20GBase_KX4,
    _10GBase_SR_LR,
    _20GBase_SR_LR,
    _40GBase_SR_LR,
    _12_5GBase_KR,  /* XLHGL_KR */
    XLHGL_KR4,
    HGL16G,
    HGS,
    HGS4,
    _100GBase_SR10,
    CHGL_LR12,
    TCAM,
    INTLKN_12Lanes_6_25G,
    INTLKN_16Lanes_6_25G,
    INTLKN_24Lanes_6_25G,
    INTLKN_12Lanes_10_3125G,
    INTLKN_16Lanes_10_3125G,
    INTLKN_12Lanes_12_5G,
    INTLKN_16Lanes_12_5G,
    INTLKN_16Lanes_3_125G,
    INTLKN_24Lanes_3_125G,
    CHGL11_LR12,

    NON_SUP_MODE

}MV_HWS_PORT_STANDARD;

/*
 * Typedef: enum MV_HWS_REF_CLOCK_SOURCE
 *
 * Description: Defines the supported reference clock source.
 *
 */
typedef enum
{
  PRIMARY_LINE_SRC,
  SECONDARY_LINE_SRC

}MV_HWS_REF_CLOCK_SOURCE;

/*
 * Typedef: enum MV_HWS_REF_CLOCK_SUP_VAL
 *
 * Description: Defines the supported reference clock.
 *
 */
typedef enum
{ 
  MHz_25,
  MHz_125,
  MHz_156  

}MV_HWS_REF_CLOCK_SUP_VAL;

/*
 * Typedef: enum MV_HWS_PORT_ACTION
 *
 * Description: Defines different actions during port delete.
 *
 */
typedef enum
{
  PORT_POWER_DOWN,
  PORT_RESET

}MV_HWS_PORT_ACTION;

/*******************************************************************************
* mvHwsPortInit
*
* DESCRIPTION:
*       Init physical port. Configures the port mode and all it's elements 
*       accordingly.
*       Does not verify that the selected mode/port number is valid at the
*       core level.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       lbPort     - if true, init port without serdes activity
*       refClock   - Reference clock frequency
*       refClockSrc - Reference clock source line
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortInit
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 lbPort,
    MV_HWS_REF_CLOCK_SUP_VAL refClock,
    MV_HWS_REF_CLOCK_SOURCE  refClockSource

);

/*******************************************************************************
* mvHwsPortReset
*
* DESCRIPTION:
*       Clears the port mode and release all its resources according to selected.
*       Does not verify that the selected mode/port number is valid at the core 
*       level and actual terminated mode.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       action    - Power down or reset
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION      action

);

#ifdef __cplusplus
}
#endif

#endif /* mvHwServicesPortIf_H */

/*
 *------------------------------------------------------------------
 * $Log: mvHwsPortInitIf.h,v $
 * Revision 1.1  2015/02/13 11:34:26  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

