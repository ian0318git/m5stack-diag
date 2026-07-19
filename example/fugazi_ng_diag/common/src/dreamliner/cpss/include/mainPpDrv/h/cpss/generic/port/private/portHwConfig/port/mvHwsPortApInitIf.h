/* $Id: mvHwsPortApInitIf.h,v 1.1 2015/02/13 11:34:26 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mvHwsPortApInitIf.h,v $
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
* mvHwsPortApInitIf.h
*
* DESCRIPTION:
*       API to configure and run 802.3ap Serdes AutoNeg engine
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
******************************************************************************/

#ifndef __mvHwServicesPortApIf_H
#define __mvHwServicesPortApIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <mvHwsPortInitIf.h>
#include <private/mvHwsPortTypes.h>

/*
 * IEEE capability vector
 if ($macro_mode eq '1x40G_KR4') {$adv_bitmap=$adv_bitmap+(1<<4);}
    if ($macro_mode eq '2x20G_KR2') {$adv_bitmap=$adv_bitmap+(1<<8);}
    if ($macro_mode eq '4x10G_KR') {$adv_bitmap=$adv_bitmap+(1<<5);}
    if ($macro_mode eq '4x1G') {$adv_bitmap=$adv_bitmap+(1<<7);}
    if ($macro_mode eq '2x10G_KX2') {$adv_bitmap=$adv_bitmap+(1<<9);}
    if ($macro_mode eq '1x10G_KX4') {$adv_bitmap=$adv_bitmap+(1<<6);}
    $master->Reg_Write('address' => 0x088c0000 + ($port*0x1000) + 0x400 + 0x13c,
 */
typedef enum
{
    _1000Base_KX_Bit0 = 1, /* _1000Base_KX */
	_10GBase_KX4_Bit1 = 2, /* _10GBase_KX4 */
    _10GBase_KR_Bit2  = 4, /* _10GBase_KR */
    _40GBase_KR4_Bit3 = 8, /* _40GBase_KR */
    _40GBase_CR4_Bit4 = 16, /* not supported */
    _100GBase_KR10_Bit5 = 32,/* not supported */

    LAST_ADV_MODE

}MV_HWS_AP_CAPABILITY;

/*
 * Reference Clock configuration
 *   refClockFreq   - Reference clock frequency
 *   refClockSource - Reference clock source
 */
typedef struct
{
	MV_HWS_REF_CLOCK_SUP_VAL refClockFreq;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
}MV_HWS_REF_CLOCK_CFG;

/*
 * AP configuration parameters:
 *   apLaneNum   - lane number inside a port (if not 0, pre AP port number isn't guarantee)
 *   modesVector - bitmap of supported port modes (IEEE capabilities):
 *                     _1000Base_KX_Bit0 = 1,
 *                     _10GBase_KX4_Bit1 = 2,
 *                     _10GBase_KR_Bit2  = 4,
 *                     _40GBase_KR4_Bit3 = 8,
 *                     _40GBase_CR4_Bit4 = 16,
 *                     _100GBase_KR10_Bit5 = 32
 *   fcPause     - FC pause (true/false)
 *   fcAsmDir    - FC ASM_DIR (Annex 28B)
 *   fecSup      - FEC ability (true/false)
 *   fecReq      - Request link partner to enable FEC (true/false)
 *   nonceDis    - indicates the port mode for the AP-Init (In loopback
 *                 Nonce detection is disabled)
 *   refClockCfg - Reference clock configuration
*/
typedef struct
{
	GT_U32                apLaneNum;
    GT_U32                modesVector;
    GT_BOOL             fcPause;
    GT_BOOL             fcAsmDir;
    GT_BOOL             fecSup;
    GT_BOOL             fecReq;
    GT_BOOL               nonceDis;
	MV_HWS_REF_CLOCK_CFG  refClockCfg;

}MV_HWS_AP_CFG;

/*
 * Highest common denominator (HCD) results parameters:
 *   hcdFound      - indicating AP resolution completed
 *   hcdLinkStatus - link status
 *   hcdResult     - the auto negotiation resolution
 *                   (bit location in IEEE capabilities vector)
 *   hcdPcsLockStatus - link/lock status form the relevant PCS/MAC
 *   hcdFecEn      - whether FEC negotiation resolution is enabled
 *   hcdFcRxPauseEn   - FC Rx pause resolution (true/false)
 *   hcdFcTxPauseEn   - FC Tx pause resolution (true/false)
*/
typedef struct
{
    GT_BOOL                 hcdFound;
    GT_U32                  hcdLinkStatus;
    GT_U32                  hcdResult;
    GT_U32                  hcdPcsLockStatus;
	GT_BOOL                 hcdFecEn;
	GT_BOOL                 hcdFcRxPauseEn;
    GT_BOOL                 hcdFcTxPauseEn;

}MV_HWS_HCD_INFO;

/*
 * AP results information:
 *    preApPortNum - requested AP port number (not garante, if apLaneNum != 0)
 *    apLaneNum    - relative lane number
 *    postApPortNum - AP resolved port number (lane swap result)
 *    postApPortMode - port mode - result of auto negotiation
 *    hcdResult     - (HCD) results parameters
*/
typedef struct
{
    GT_U32                  preApPortNum;
    GT_U32                  apLaneNum;
    GT_U32                  postApPortNum;
    MV_HWS_PORT_STANDARD    postApPortMode;
    MV_HWS_HCD_INFO         hcdResult;

}MV_HWS_AP_PORT_STATUS;



/*******************************************************************************
* mvHwsApEngineInit
*
* DESCRIPTION:
*       Initialize AP engine.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApEngineInit
(
    GT_U8                    devNum,
    GT_U32                   portGroup
);

/*******************************************************************************
* mvHwsApEngineStop
*
* DESCRIPTION:
*       Disable whole AP engine.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApEngineStop
(
    GT_U8                    devNum,
    GT_U32                   portGroup
);

/*******************************************************************************
* mvHwsApEngineInitGet
*
* DESCRIPTION:
*       Checks if AP engine is enabled whole AP engine.
*
* INPUTS:
*       devNum        - system device number
*       portGroup     - port group (core) number
*       engineEnabled - pointer to boolean that indicated whether the engine
*                       is enabled
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApEngineInitGet
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
	GT_BOOL					 *engineEnabled
);

/*******************************************************************************
* mvHwsApPortStart
*
* DESCRIPTION:
*       Init AP port capability.
*       Runs AP protocol(802.3ap Serdes AutoNeg) and configures the best port
*       mode and all it's elements accordingly.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       phyPortNum  - physical port number
*       apCfg       - AP configuration parameters
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApPortStart
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_U32                   phyPortNum,
    MV_HWS_AP_CFG            *apCfg
);

/*******************************************************************************
* mvHwsApPortStop
*
* DESCRIPTION:
*       Disable the AP engine on port and release all its resources.
*       Clears the port mode and release all its resources according to selected.
*
* INPUTS:
*       devNum     - system device number
*       portGroup  - port group (core) number
*       phyPortNum - physical port number
*       action     - Power down or reset
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApPortStop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_ACTION      action

);

/*******************************************************************************
* mvHwsApPortConfigGet
*
* DESCRIPTION:
*       Returns the AP port configuration.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       phyPortNum  - physical port number
*
* OUTPUTS:
*		apPortEn    - whether AP is enabled on this port
*       apCfg       - if not NULL, AP configuration parameters
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApPortConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
	GT_BOOL                 *apPortEnabled,
    MV_HWS_AP_CFG           *apCfg

);

/*******************************************************************************
* mvHwsApPortSetActiveLanes
*
* DESCRIPTION:
*       Disable the AP engine on port and release all its resources.
*       Clears the port mode and release all its resources according to selected.
*
* INPUTS:
*       devNum       - system device number
*       portGroup    - port group (core) number
*       phyPortNum   - physical port number
*       portMode     - port mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApPortSetActiveLanes
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
	MV_HWS_PORT_STANDARD    portMode

);

/*******************************************************************************
* mvHwsApPortResolutionMaskGet
*
* DESCRIPTION:
*       Returns the port's resolution bit mask
*
* INPUTS:
*       devNum       - system device number
*       portGroup    - port group (core) number
*
* OUTPUTS:
*       portBitmask  - port's resolution bit mask
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApPortResolutionMaskGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
	GT_U32                  *portBitmask
);

/*******************************************************************************
* mvHwsApPortStatusGet
*
* DESCRIPTION:
*       Returns the AP port resolution info.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       phyPortNum - physical port number
*
* OUTPUTS:
*       apResult - AP/HCD results
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApPortStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_STATUS   *apResult

);

/*******************************************************************************
* mvHwsApEngineStatusGet
*
* DESCRIPTION:
*       Returns the AP status for all actives AP ports.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       apPortActNum - size of result array
*       apResult     - pointer to array of AP ports results
*
* OUTPUTS:
*       apResult - AP results
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsApEngineStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  apPortActNum,
    MV_HWS_AP_PORT_STATUS   *apResult

);
/*******************************************************************************
* mvApCheckCounterGet
*
* DESCRIPTION:
*       Read checkCounter value display AP engine activity.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*
* OUTPUTS:
*       'checkCounter' value
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvApCheckCounterGet
(
  GT_U8   devNum,
  GT_U32  portGroup,
  GT_U32 *counter
);

/*******************************************************************************
* mvApPortStatusShow
*
* DESCRIPTION:
*       Print AP port status information stored in system.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       apPortNum - AP port number
*
* OUTPUTS:
*       None
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvApPortStatusShow
(
  GT_U8   devNum,
  GT_U32  portGroup,
  GT_U32  apPortNum
);

#ifdef __cplusplus
}
#endif

#endif /* mvHwServicesPortApIf_H */


/*
 *------------------------------------------------------------------
 * $Log: mvHwsPortApInitIf.h,v $
 * Revision 1.1  2015/02/13 11:34:26  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
