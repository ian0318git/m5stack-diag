/* $Id: mvHwsPortCfgIf.h,v 1.1 2015/02/13 11:34:26 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mvHwsPortCfgIf.h,v $
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
* mvHwsPortCfgIf.h
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwsPortCfgIf_H
#define __mvHwsPortCfgIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <mvHwsPortInitIf.h>
#include <private/mvHwsPortMiscIf.h>

/*
 * Typedef: enum MV_HWS_PORT_STANDARD
 *
 * Description: Defines the different port CRC modes.
 *              The XG port is capable of working at four CRC modes,
 *              the standard four-bytes mode and the proprietary
 *              one-byte/two-bytes/three-bytes CRC mode.
 *
 */
typedef enum
{
    HWS_1Byte_CRC,
    HWS_2Bytes_CRC,
    HWS_3Bytes_CRC,
    HWS_4Bytes_CRC

}MV_HWS_PORT_CRC_MODE;


/*******************************************************************************
* hwsPortExtendedModeCfg
*
* DESCRIPTION:
*       Enable / disable extended mode on port specified.
*       Extended ports supported only in Lion2 device.
*       1G, 10GBase-R, 20GBase-R2, RXAUI - can be normal or extended
*       XAUI, DXAUI, 40GBase-R - only extended
*
* INPUTS:
*       devNum       - system device number
*       portGroup    - port group (core) number
*       phyPortNum   - physical port number
*       portMode     - port standard metric
*       extendedMode - enable / disable
*
* OUTPUTS:
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsPortExtendedModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 extendedMode
);

/*******************************************************************************
* mvHwsPortAutoTuneStop
*
* DESCRIPTION:
*       Stop Tx and Rx training.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       portTuningMode - port tuning mode
*
* OUTPUTS:
*       Tuning results for recommended settings.(TBD)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortAutoTuneStop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 stopRx,
    GT_BOOL                 stopTx
);

/*******************************************************************************
* mvHwsPortAutoTuneStateCheck
*
* DESCRIPTION:
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       portTuningMode - port TX related tuning mode
*
* OUTPUTS:
*       Tuning results for recommended settings.(TBD)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortAutoTuneStateCheck
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *rxTune,
    MV_HWS_AUTO_TUNE_STATUS *txTune
);

/*******************************************************************************
* mvHwsPortFecCofig
*
* DESCRIPTION:
*       Configure FEC disable/enable on port.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       portFecEn  - GT_TRUE for FEC enable, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortFecCofig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 portFecEn
);

/*******************************************************************************
* mvHwsPortFecCofigGet
*
* DESCRIPTION:
*       Return FEC status disable/enable on port.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*
* OUTPUTS:
*       portFecEn  - GT_TRUE for FEC enable, GT_FALSE otherwise
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortFecCofigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *portFecEn
);

/*******************************************************************************
* mvHwsPortInterfaceGet
*
* DESCRIPTION:
*       Gets Interface mode and speed of a specified port.
*
*
* INPUTS:
*       devNum   - physical device number
*       portGroup - core number
*       phyPortNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       portModePtr - interface mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS mvHwsPortInterfaceGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    *portModePtr
);

/*******************************************************************************
* mvHwsPortClearChannelCfg
*
* DESCRIPTION:
*       Configures MAC advanced feature  accordingly.
*       Can be run before create port or after delete port.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       txIpg      - TX_IPG
*       txPreamble - TX Preamble
*       rxPreamble - RX Preamble
*       txCrc      - TX CRC
*       rxCrc      - RX CRC
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortClearChannelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  txIpg,
    GT_U32                  txPreamble,
    GT_U32                  rxPreamble,
    MV_HWS_PORT_CRC_MODE    txCrc,
    MV_HWS_PORT_CRC_MODE    rxCrc
);

/*******************************************************************************
* mvHwsPortAcTerminationCfg
*
* DESCRIPTION:
*       Configures AC termination in all port serdes lanes according to mode.
*       Can be run after create port only.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*       portAcTermEn - enable or disable AC termination
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortAcTerminationCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 portAcTermEn
);

/*******************************************************************************
* mvHwsPortCheckGearBox
*
* DESCRIPTION:
*       Check Gear Box Status on related lanes.
*       Can be run after create port only.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       phyPortNum - physical port number
*       portMode   - port standard metric
*
* OUTPUTS:
*       laneLock  - true or false.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsPortCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortMiscIf_H */


/*
 *------------------------------------------------------------------
 * $Log: mvHwsPortCfgIf.h,v $
 * Revision 1.1  2015/02/13 11:34:26  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
