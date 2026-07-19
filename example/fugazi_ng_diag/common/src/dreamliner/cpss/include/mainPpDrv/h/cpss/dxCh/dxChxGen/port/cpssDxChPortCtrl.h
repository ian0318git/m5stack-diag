/* $Id: cpssDxChPortCtrl.h,v 1.1 2015/02/13 11:32:31 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/cpss/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h,v $
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
* cpssDxChPortCtrl.h
*
* DESCRIPTION:
*       CPSS implementation for Port configuration and control facility.
*
* FILE REVISION NUMBER:
*       $Revision: 1.1 $
*******************************************************************************/

#ifndef __cpssDxChPortCtrlh
#define __cpssDxChPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/* definition for the number of PSC lanes per port */
#define CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS 4

#define CPSS_DXCH_PORT_MAX_MRU_CNS 10304  /* jumbo frame + 64  (including 4 bytes CRC) */

#define CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS 32

#define CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS 128

/*
 * typedef: enum CPSS_DXCH_PORT_SERDES_MODE_ENT
 *
 * Description: Enumeration of Serdes modes
 *
 * Enumerations:
 *    CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E - Regular Serdes are active
 *    CPSS_DXCH_PORT_SERDES_MODE_COMBO_E - Combo Serdes are active
 */
typedef enum
{
    CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E,
    CPSS_DXCH_PORT_SERDES_MODE_COMBO_E

}CPSS_DXCH_PORT_SERDES_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT
 *
 * Description: Enumeration of the MAC error types.
 *
 * Enumerations:
 *  CPSS_DXCH_PORT_MAC_ERROR_JABBER_E    - Jabber error type
 *  CPSS_DXCH_PORT_MAC_ERROR_FRAGMENTS_E - Fragment error type
 *  CPSS_DXCH_PORT_MAC_ERROR_UNDERSIZE_E - Undersize error type
 *  CPSS_DXCH_PORT_MAC_ERROR_OVERSIZE_E  - Oversize error type
 *  CPSS_DXCH_PORT_MAC_ERROR_RX_E        - Rx error type
 *  CPSS_DXCH_PORT_MAC_ERROR_CRC_E       - CRC error type
 *  CPSS_DXCH_PORT_MAC_ERROR_OVERRUN_E   - Overrun error type
 *  CPSS_DXCH_PORT_MAC_ERROR_NO_E        - No error
 *
 * Comments:
 *         None.
 */
typedef enum
{
    CPSS_DXCH_PORT_MAC_ERROR_JABBER_E,
    CPSS_DXCH_PORT_MAC_ERROR_FRAGMENTS_E,
    CPSS_DXCH_PORT_MAC_ERROR_UNDERSIZE_E,
    CPSS_DXCH_PORT_MAC_ERROR_OVERSIZE_E,
    CPSS_DXCH_PORT_MAC_ERROR_RX_E,
    CPSS_DXCH_PORT_MAC_ERROR_CRC_E,
    CPSS_DXCH_PORT_MAC_ERROR_OVERRUN_E,
    CPSS_DXCH_PORT_MAC_ERROR_NO_E

}CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_FC_MODE_ENT
 *
 * Description: Enumeration of FC modes.
 *
 * Enumerations:
 *  CPSS_DXCH_PORT_FC_MODE_802_3X_E  - 802.3X
 *  CPSS_DXCH_PORT_FC_MODE_PFC_E     - PFC: Priority Flow Control
 *  CPSS_DXCH_PORT_FC_MODE_LL_FC_E   - LL FC: Link Level Flow control
 *
 * Comments:
 *         None.
 */
typedef enum
{
    CPSS_DXCH_PORT_FC_MODE_802_3X_E,
    CPSS_DXCH_PORT_FC_MODE_PFC_E,
    CPSS_DXCH_PORT_FC_MODE_LL_FC_E
}CPSS_DXCH_PORT_FC_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_SERDES_SPEED_ENT
 *
 * Description: Defines SERDES speed.
 *
 * Enumerations:
 *  CPSS_DXCH_PORT_SERDES_SPEED_1_25_E  - Serdes speed is 1.25G
 *  CPSS_DXCH_PORT_SERDES_SPEED_3_125_E - Serdes speed is 3.125G
 *  CPSS_DXCH_PORT_SERDES_SPEED_3_75_E  - Serdes speed is 3.75G
 *  CPSS_DXCH_PORT_SERDES_SPEED_6_25_E  - Serdes speed is 6.25G
 *  CPSS_DXCH_PORT_SERDES_SPEED_5_E     - Serdes speed is 5G
 *  CPSS_DXCH_PORT_SERDES_SPEED_4_25_E  - Serdes speed is 4.25G
 *  CPSS_DXCH_PORT_SERDES_SPEED_2_5_E   - Serdes speed is 2.5G
 *  CPSS_DXCH_PORT_SERDES_SPEED_5_156_E - Serdes speed is 5.156G (x8 serdes = 40G data speed)
 *  CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E - used by KR
 *  CPSS_DXCH_PORT_SERDES_SPEED_3_333_E - used by HGL 16G
 *  CPSS_DXCH_PORT_SERDES_SPEED_12_5_E  - for 100G
 *  CPSS_DXCH_PORT_SERDES_SPEED_NA_E    - Last member, the speed is not determind
 */
typedef enum
{
    CPSS_DXCH_PORT_SERDES_SPEED_1_25_E,
    CPSS_DXCH_PORT_SERDES_SPEED_3_125_E,
    CPSS_DXCH_PORT_SERDES_SPEED_3_75_E,
    CPSS_DXCH_PORT_SERDES_SPEED_6_25_E,
    CPSS_DXCH_PORT_SERDES_SPEED_5_E,
    CPSS_DXCH_PORT_SERDES_SPEED_4_25_E,
    CPSS_DXCH_PORT_SERDES_SPEED_2_5_E,
    CPSS_DXCH_PORT_SERDES_SPEED_5_156_E,
    CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E,
    CPSS_DXCH_PORT_SERDES_SPEED_3_333_E,
    CPSS_DXCH_PORT_SERDES_SPEED_12_5_E,

    CPSS_DXCH_PORT_SERDES_SPEED_NA_E
} CPSS_DXCH_PORT_SERDES_SPEED_ENT;

/*
 * typedef: struct CPSS_DXCH_PORT_SERDES_CONFIG_STC
 *
 * Description: Port SERDES configuration.
 *
 * Fields:
 *      txAmp       - Tx Driver output amplitude(valid values range 0-31).
 *      txEmphEn    - Pre- or de-emphasis enable signal(GT_TRUE - enable, GT_FALSE - disable).
 *      txEmphType  - GT_FALSE - de-emphasis, GT_TRUE - pre-emphasis.
 *      txEmphAmp   - Emphasis level control bits(valid values range 0-15).
 *      txAmpAdj    - Transmitter Amplitude Adjust(valid values range 0-1).
 *      ffeSignalSwingControl - FFE signal swing control (0..3).
 *      ffeResistorSelect - Feed Forward Equalization (FFE) Resistor Select (0..0x7).
 *      ffeCapacitorSelect - Feed Forward Equalization (FFE) Capacitor Select (0..0xF).
 * Comments:
 *      1. Relevant fields for DxCh3: txAmp, txEmphEn, txEmphType, txEmphAmp.
 *      2. Relevant fields for xCat and above: txAmp, txEmphAmp, txAmpAdj, txEmphEn,
 *         ffeSignalSwingControl, ffeResistorSelect, ffeCapacitorSelect.
 *      3. For further information, contact your Marvell representative.
 */
typedef struct {
    GT_U32 txAmp;
    GT_BOOL txEmphEn;
    GT_BOOL txEmphType;
    GT_U32 txEmphAmp;
    GT_U32 txAmpAdj;
    GT_U32 ffeSignalSwingControl;
    GT_U32 ffeResistorSelect;
    GT_U32 ffeCapacitorSelect;
} CPSS_DXCH_PORT_SERDES_CONFIG_STC;


/* size of DFE values array */
#define CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS    6

/*
 * typedef: struct CPSS_DXCH_PORT_SERDES_TUNE_STC
 *
 * Description: Port SERDES lanes fine tuning values.
 *
 * Fields:
 *      dfe - A digital filter controls the gain according to the previous 
 *              received bit (one tap) and compensates for interconnect ISI 
 *              and ILD (refer to the DFE_F1 field)
 *              (APPLICABLE DEVICES: xCat, xCat2, Lion)
 *              (APPLICABLE RANGES: 0..31)
 *      ffeR - mainly controls the low frequency gain (refer to the 
 *              FFE_res_sel field)(APPLICABLE DEVICES: xCat and above)
 *              (APPLICABLE RANGES: 0..7)
 *      ffeC - mainly controls the high frequency gain (refer to the 
 *              FFE_cap_sel field)(APPLICABLE DEVICES: xCat and above)
 *              (APPLICABLE RANGES: 0..15)
 *      sampler - sampler (refer to the cal_os_ph_rd field)
 *                  (APPLICABLE DEVICES: xCat and above)
 *      sqlch - Threshold that trips the Squelch detector peak-to-peak 
 *              differential amplitude (refer to the SQ_THRESH field)
 *              (APPLICABLE DEVICES: xCat and above)
 *      txEmphAmp - Emphasis level control bits
 *                  (TX_EMPH0 - Transmitter Pre Cursor Emphasis in Lion2)
 *                  (APPLICABLE RANGES: 0..15)
 *      txAmp - Tx Driver output amplitude (APPLICABLE RANGES: 0..31)
 *      txAmpAdj- Transmitter Amplitude Adjust (APPLICABLE DEVICES: xCat and above)
 *                  (APPLICABLE RANGES: 0..15)
 *      ffeS - FFE signal swing control (APPLICABLE DEVICES: xCat, xCat2, Lion)
 *              (APPLICABLE RANGES: 0..3)
 *      txEmphEn - Pre- or de-emphasis enable signal(GT_TRUE - enable, 
 *                                                  GT_FALSE - disable).
 *                 (APPLICABLE DEVICES: xCat and above)
 *      txEmph1 - Controls the emphasis amplitude for Gen1 bit rates
 *                  (APPLICABLE RANGES: 0..7) (APPLICABLE DEVICES: Lion2)
 *      align90 - Align 90 Calibration Phase Offset(This is the external 
 *                  value used in place of the autocalibration value for 
 *                  rxclkalign90). (APPLICABLE RANGES: 0..127)
 *                  (APPLICABLE DEVICES: Lion2)
 *      txEmphEn1 - (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES: Lion2)
 *      txAmpShft - (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES: Lion2)
 *      dfeVals - Adapted DFE Coefficient. This field indicates the DFE 
 *                  auto-calibration and auto-trained results.
 *                  (APPLICABLE DEVICES: Lion2)
 * Comments:
 *      Following fields could be received from serdes optimizer algorithm
 *          by cpssDxChDiagSerdesTuningRxTune: dfe, ffeR, ffeC, sampler, 
 *              sqlch.
 *      Following fields could be configured by application using 
 *          cpssDxChPortSerdesConfigSet: ffeR, ffeC, txEmphAmp, txAmp, 
 *              txAmpAdj, ffeS, txEmphEn.
 *      Pay attention, for now, two fields could be defined by two API's:
 *              ffeR, ffeC.
 */
typedef struct {
    GT_U32 dfe;
    GT_U32 ffeR;
    GT_U32 ffeC;
    GT_U32 sampler;
    GT_U32 sqlch;

    GT_U32  txEmphAmp;
    GT_U32  txAmp;
    GT_U32  txAmpAdj;
    GT_U32  ffeS;
    GT_BOOL txEmphEn;

    GT_U32  txEmph1;
    GT_U32  align90;
    GT_BOOL txEmphEn1;
    GT_BOOL txAmpShft;
    GT_32   dfeValsArray[CPSS_DXCH_PORT_DFE_VALUES_ARRAY_SIZE_CNS];

} CPSS_DXCH_PORT_SERDES_TUNE_STC, *CPSS_DXCH_PORT_SERDES_TUNE_STC_PTR;

/*
 * typedef: struct CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC
 *
 * Description: Port SERDES TX configuration parameters.
 * 
 * APPLICABLE DEVICES: Lion2, Lion3
 * 
 * Fields:
 *      txAmp       - Tx Driver output amplitude (APPLICABLE RANGES: 0..31).
 *      txAmpAdj    - Transmitter Amplitude Adjust (GT_TRUE - enable,
 *                                                  GT_FALSE - disable).
 *      emph0       - Controls the emphasis amplitude for Gen0 bit rates
 *                                                  (APPLICABLE RANGES: 0..15).
 *      emph1       - Controls the emphasis amplitude for Gen1 bit rates
 *                                                  (APPLICABLE RANGES: 0..15).
 *      txAmpShft   - Transmitter Amplitude Shift (GT_TRUE - enable,
 *                                                  GT_FALSE - disable).
 * 
 */
typedef struct {
    GT_U32              txAmp;
    GT_BOOL             txAmpAdjEn;
    GT_U32              emph0;
    GT_U32              emph1;
    GT_BOOL             txAmpShft;
} CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC;

/*
 * typedef: struct CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC
 *
 * Description: Port SERDES RX configuration parameters.
 * 
 * APPLICABLE DEVICES: Lion2, Lion3
 * 
 * Fields:
 *      sqlch - Threshold that trips the Squelch detector peak-to-peak 
 *              differential amplitude (refer to the SQ_THRESH field)
 *              (APPLICABLE RANGES: 0..31).
 *      ffeRes - mainly controls the low frequency gain (refer to the 
 *              FFE_res_sel field) (APPLICABLE RANGES: 0..7)
 *      ffeCap - mainly controls the high frequency gain (refer to the 
 *              FFE_cap_sel field)(APPLICABLE RANGES: 0..15)
 *      align90 - Align 90 Calibration Phase Offset(This is the external 
 *                  value used in place of the autocalibration value for 
 *                  rxclkalign90). (APPLICABLE RANGES: 0..127)
 * 
 */
typedef struct {
    GT_U32              sqlch;
    GT_U32              ffeRes;
    GT_U32              ffeCap;
    GT_U32              align90;
} CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC;

/*
 * typedef: struct CPSS_DXCH_PORT_EOM_MATRIX_STC
 *
 * Description: A struct containing two 32x128 matrices representing
 *              the values received from the EOM feature.
 *
 * Fields:
 *      upperMatrix - The upper EOM matrix
 *      lowerMatrix - The lower EOM matrix
 *
 *  Comments:
 *      Due to it's size the struct should be allocated on the heap
 */
typedef struct {
    GT_U32 upperMatrix[CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS]
                        [CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS];
    GT_U32 lowerMatrix[CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS]
                        [CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS];
} CPSS_DXCH_PORT_EOM_MATRIX_STC;

/*
 * typedef: enum CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT
 *
 * Description: Periodic Flow Control Port Type speed enumeration
 *
 * Enumerations:
 *      CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E  - GIG ports
 *      CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E   - XG ports
 *
 */
typedef enum
{
    CPSS_DXCH_PORT_PERIODIC_FC_TYPE_GIG_E,
    CPSS_DXCH_PORT_PERIODIC_FC_TYPE_XG_E
} CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT
 *
 * Description: Enumeration of port's serdes auto tuning modes.
 *
 * Enumerations:
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E - TX training
 *                                                              configure state
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E - start TX training
 *                                                              itself
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E - get TX training
 *                                                              result
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E - run RX training
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E - stop training of any 
 *                                                          type
 */
typedef enum
{
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E,
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E

}CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT
 *
 * Description: Enumeration of port's serdes auto tuning modes.
 *
 * Enumerations:
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E - serdes auto-tuning (rx/tx) succeeded
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E - serdes auto-tuning (rx/tx) failed
 *    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E - serdes auto-tuning (rx/tx) 
 *                                                      still in process
 */
typedef enum
{
  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E,
  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E,
  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E
}CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT
 *
 * Description: Enumeration of port's serdes loopback modes.
 *
 * Enumerations:
 *  CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E        - no any loopback on serdes
 *  CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E   - analog TX to RX loopback
 *  CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E  - digital TX to RX loopback
 *  CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E  - digital RX to TX loopback
 *  CPSS_DXCH_PORT_SERDES_LOOPBACK_MAX_E            - enum type upper boarder
 */
typedef enum
{
    CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E,
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MAX_E

}CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT;

/*
 * typedef: enum CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT
 *
 * Description: Enumeration of port's PCS loopback modes.
 *
 * Enumerations:
 *  CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E   - no any loopback on PCS
 *  CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E     - TX to RX loopback
 *  CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E     - RX to TX loopback
 */
typedef enum
{
    CPSS_DXCH_PORT_PCS_LOOPBACK_DISABLE_E,
    CPSS_DXCH_PORT_PCS_LOOPBACK_TX2RX_E,
    CPSS_DXCH_PORT_PCS_LOOPBACK_RX2TX_E

}CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT;

/*
* typedef: enum CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT
*
* Description: Enumeration of Port Periodic FC type enablers.
*
* Enumerations:
*  CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E   - Both disabled
*  CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E  - Both enabled
*  CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E  - Only XON FC enabled
*  CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E - Only XOFF FC enabled
*
* Comments:
*         None.
*/
typedef enum
{
    CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = GT_FALSE,
    CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E = GT_TRUE,
    CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E,
    CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E
} CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT;


/*
 * typedef: enum CPSS_DXCH_PORT_FEC_MODE_ENT
 *
 * Description: Enumeration of port's Forward Error Correction modes.
 *
 * Enumerations:
 *  CPSS_DXCH_PORT_FEC_MODE_ENABLED_E   - FEC enabled on port
 *  CPSS_DXCH_PORT_FEC_MODE_DISABLED_E  - FEC disabled on port
 * 
 */
typedef enum
{
    CPSS_DXCH_PORT_FEC_MODE_ENABLED_E,
    CPSS_DXCH_PORT_FEC_MODE_DISABLED_E
}CPSS_DXCH_PORT_FEC_MODE_ENT;

/*******************************************************************************
* cpssDxChPortMacSaLsbSet
*
* DESCRIPTION:
*       Set the least significant byte of the MAC SA of specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical port number
*       macSaLsb    - The ls byte of the MAC SA
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacSaLsbSet
(
    IN GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8    macSaLsb
);

/*******************************************************************************
* cpssDxChPortMacSaLsbGet
*
* DESCRIPTION:
*       Get the least significant byte of the MAC SA of specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical port number
*
* OUTPUTS:
*       macSaLsbPtr - (pointer to) The ls byte of the MAC SA
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssDxChPortMacSaBaseSet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8    *macSaLsbPtr
);

/*******************************************************************************
* cpssDxChPortEnableSet
*
* DESCRIPTION:
*       Enable/disable a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number, CPU port number
*       enable   - GT_TRUE:  enable port,
*                  GT_FALSE: disable port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   enable
);

/*******************************************************************************
* cpssDxChPortEnableGet
*
* DESCRIPTION:
*       Get the Enable/Disable status of a specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum  - physical device number
*       portNum - physical port number
*
* OUTPUTS:
*       statePtr - Pointer to the Get Enable/disable state of the port.
*                  GT_TRUE for enabled port, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortEnableGet
(
    IN   GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
);

/*******************************************************************************
* cpssDxChPortDuplexModeSet
*
* DESCRIPTION:
*       Set the port to enable and the port mode to half- or full-duplex mode.
*       When port is disabled it does not transmit pending packets and
*       drops packets received from the line.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       dMode    - 0 = The port operates in half-duplex mode.
*                  1 = The port operates in full-duplex mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or dMode
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported duplex mode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The <SetFullDx> bit is meaningless when <ANDuplex> is set.
*       For CPU port, this field must be set to reflect this port Duplex mode.
*
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
);

/*******************************************************************************
* cpssDxChPortDuplexModeGet
*
* DESCRIPTION:
*       Gets duplex mode for specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       dModePtr - duplex mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
);

/*******************************************************************************
* cpssDxChPortSpeedSet
*
* DESCRIPTION:
*       Sets speed for specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       speed    - port speed
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or speed
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported speed
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_STATE             - Speed of the port group member is different
*                                  from speed for setting.
*                                  (For DxCh3, not XG ports only.)
*
* COMMENTS:
*       1. If the port is enabled then the function disables the port before
*          the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*          configured before port's speed, see cpssDxChPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*         not initialized, proper init will be done
*       4.Added callback bind option used to run PHYMAC configuration functions
*         Callbacks can be run before switch MAC configuration and after
*         First callback can set doPpMacConfig TRUE or FALSE. If the return value
*         is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortSpeedSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/*******************************************************************************
* cpssDxChPortSpeedGet
*
* DESCRIPTION:
*       Gets speed for specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       speedPtr - pointer to actual port speed
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_INITIALIZED       - on no initialized SERDES per port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        1.This API also checks if at least one serdes per port was initialized.
*        In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*        returned.
*
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);


/*******************************************************************************
* cpssDxChPortDuplexAutoNegEnableSet
*
* DESCRIPTION:
*       Enable/disable an Auto-Negotiation for duplex mode on specified port on
*       specified device.
*       When duplex Auto-Negotiation is enabled, the port's duplex mode is
*       resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*       mode is full-duplex.
*       When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*       via cpssDxChPortDuplexModeSet
*       The function does not configure the PHY connected to the port. It
*       configures only the MAC layer of the port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       state    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                  GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported state
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       1.CPU port not supports the duplex auto negotiation
*
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);
/*******************************************************************************
* cpssDxChPortDuplexAutoNegEnableGet
*
* DESCRIPTION:
*       Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*       per port.
*       When duplex Auto-Negotiation is enabled, the port's duplex mode is
*       resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*       mode is full-duplex.
*       When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*       via cpssDxChPortDuplexModeSet
*       The function does not configure the PHY connected to the port. It
*       configures only the MAC layer of the port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*
*
* OUTPUTS:
*       statePtr - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                  GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       1. CPU port not supports the duplex auto negotiation
*
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortDuplexAutoNegEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *statePtr
);

/*******************************************************************************
* cpssDxChPortFlowCntrlAutoNegEnableSet
*
* DESCRIPTION:
*       Enable/disable an Auto-Negotiation for Flow Control on
*       specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum         - physical device number
*       portNum        - physical port number
*       state          - GT_TRUE for enable Auto-Negotiation for Flow Control
*                        GT_FALSE otherwise
*       pauseAdvertise - Advertise symmetric flow control support in
*                        Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported state
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       1.CPU port not supports the flow control auto negotiation
*
*       2. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
);
/*******************************************************************************
* cpssDxChPortFlowCntrlAutoNegEnableGet
*
* DESCRIPTION:
*       Get Auto-Negotiation enable/disable state for Flow Control per port
*
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum         - physical device number
*       portNum        - physical port number
*
* OUTPUTS:
*       statePtr       -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                          GT_FALSE otherwise
*       pauseAdvertisePtr - Advertise symmetric flow control support in
*                           Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       CPU port not supports the flow control auto negotiation
*
*******************************************************************************/
GT_STATUS cpssDxChPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
);


/*******************************************************************************
* cpssDxChPortSpeedAutoNegEnableSet
*
* DESCRIPTION:
*       Enable/disable an Auto-Negotiation of interface speed on specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       state    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                  GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported state
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       1.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*       2.CPU port not supports the speed auto negotiation
*
*******************************************************************************/
GT_STATUS cpssDxChPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/*******************************************************************************
* cpssDxChPortSpeedAutoNegEnableGet
*
* DESCRIPTION:
*
*   Get status  of  Auto-Negotiation enable on specified port
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*
*
* OUTPUTS:
*       statePtr - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                  GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*       2. CPU port not supports the speed auto negotiation
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortSpeedAutoNegEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL  *statePtr
);

/*******************************************************************************
* cpssDxChPortFlowControlEnableSet
*
* DESCRIPTION:
*       Enable/disable receiving and transmission of 802.3x Flow Control frames
*       in full duplex on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*       state    - Flow Control state: Both disabled,
*                  Both enabled, Only Rx or Only Tx enabled.
*                  Note: only XG ports can be configured in all 4 options,
*                        Tri-Speed and FE ports may use only first two.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number, state or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
);

/*******************************************************************************
* cpssDxChPortFlowControlEnableGet
*
* DESCRIPTION:
*       Get status of 802.3x Flow Control on specific logical port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       statePtr - Pointer to Flow Control state: Both disabled,
*                  Both enabled, Only Rx or Only Tx enabled.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
);


/*******************************************************************************
* cpssDxChPortPeriodicFcEnableSet
*
* DESCRIPTION:
*       Enable/Disable transmits of periodic 802.3x flow control.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*       enable   - If GT_TRUE, enable transmit.
*                  If GT_FALSE, disable transmit.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         - on unsupported request
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*******************************************************************************/
GT_STATUS cpssDxChPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
);

/*******************************************************************************
* cpssDxChPortPeriodicFcEnableGet
*
* DESCRIPTION:
*       Get status of periodic 802.3x flow control transmition.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       enablePtr  - Pointer to the periodic 802.3x flow control
*                    tramsition state.
*                    If GT_TRUE, enable transmit.
*                    If GT_FALSE, disable transmit.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*       Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*******************************************************************************/
GT_STATUS cpssDxChPortPeriodicFcEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
);

/*******************************************************************************
* cpssDxChPortBackPressureEnableSet
*
* DESCRIPTION:
*       Enable/disable of Back Pressure in half-duplex on specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*       state    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         - if feature not supported by port
*
* COMMENTS:
*       1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value 
*       is TRUE the switch MAC will be configured
*
*       2. Although for XGMII (10 Gbps) ports feature is not supported the 
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*******************************************************************************/
GT_STATUS cpssDxChPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/*******************************************************************************
* cpssDxChPortBackPressureEnableGet
*
* DESCRIPTION:
*       Gets the state of Back Pressure in half-duplex on specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       enablePtr  - pointer to back pressure enable/disable state:
*                       - GT_TRUE to enable Back Pressure
*                       - GT_FALSE to disable Back Pressure.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value 
*       is TRUE the switch MAC will be configured
*
*       2. Although for XGMII (10 Gbps) ports feature is not supported the 
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*******************************************************************************/
GT_STATUS cpssDxChPortBackPressureEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *enablePtr
);

/*******************************************************************************
* cpssDxChPortLinkStatusGet
*
* DESCRIPTION:
*       Gets Link Status of specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       isLinkUpPtr - GT_TRUE for link up, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *isLinkUpPtr
);

/*******************************************************************************
* cpssDxChPortInterfaceModeSet
*
* DESCRIPTION:
*       Sets Interface mode on a specified port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number (or CPU port)
*       ifMode    - Interface mode.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_SUPPORTED  - on not supported interface for given port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        Depends on capability of the specific device. The port's speed
*        should be set after configuring Interface Mode.
*
*******************************************************************************/
GT_STATUS cpssDxChPortInterfaceModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);


/*******************************************************************************
* cpssDxChPortInterfaceModeGet
*
* DESCRIPTION:
*       Gets Interface mode on a specified port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       ifModePtr - interface mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         - wrong media interface mode value received
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortInterfaceModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

/*******************************************************************************
* cpssDxChPortForceLinkPassEnableSet
*
* DESCRIPTION:
*       Enable/disable Force Link Pass on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*       state    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortForceLinkPassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/*******************************************************************************
* cpssDxChPortForceLinkPassEnableGet
*
* DESCRIPTION:
*       Get Force Link Pass on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       statePtr - (ptr to) current force link pass state:
*                           GT_TRUE for force link pass, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - NULL pointer in statePtr
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortForceLinkPassEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *statePtr
);

/*******************************************************************************
* cpssDxChPortForceLinkDownEnableSet
*
* DESCRIPTION:
*       Enable/disable Force Link Down on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*       state    - GT_TRUE for force link down, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/*******************************************************************************
* cpssDxChPortForceLinkDownEnableGet
*
* DESCRIPTION:
*       Get Force Link Down status of specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (or CPU port)
*
* OUTPUTS:
*       statePtr - (ponter to) current force link down status:
*                              GT_TRUE - force link down, GT_FALSE - otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - NULL pointer in statePtr
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortForceLinkDownEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL   *statePtr
);

/*******************************************************************************
* cpssDxChPortMruSet
*
* DESCRIPTION:
*       Sets the Maximal Receive Packet size for specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number (or CPU port)
*       mruSize    - max receive packet size in bytes. (0..10304) -  jumbo frame + 64  (including 4 bytes CRC) 
*                    value must be even
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or
*                          odd value of mruSize
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - mruSize > 10304
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32 mruSize
);


/*******************************************************************************
* cpssDxChPortMruGet
*
* DESCRIPTION:
*       Gets the Maximal Receive Packet size for specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number (or CPU port)
*
* OUTPUTS:
*       mruSizePtr - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                    value must be even
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or
*                          odd value of mruSize
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - mruSize > 10304
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortMruGet
(
    IN   GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32 *mruSizePtr
);


/*******************************************************************************
* cpssDxChPortCrcCheckEnableSet
*
* DESCRIPTION:
*       Enable/Disable 32-bit the CRC checking.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number (or CPU port)
*       enable     - If GT_TRUE, enable CRC checking
*                    If GT_FALSE, disable CRC checking
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/*******************************************************************************
* cpssDxChPortCrcCheckEnableGet
*
* DESCRIPTION:
*       Get CRC checking (Enable/Disable) state for received packets.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number (or CPU port)
*
* OUTPUTS:
*       enablePtr - Pointer to the CRS checking state :
*                       GT_TRUE  - CRC checking is enable,
*                       GT_FALSE - CRC checking is disable.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*******************************************************************************/
GT_STATUS cpssDxChPortCrcCheckEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/*******************************************************************************
* cpssDxChPortXGmiiModeSet
*
* DESCRIPTION:
*       Sets XGMII mode on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       mode  - XGMII mode
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortXGmiiModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
);

/*******************************************************************************
* cpssDxChPortXGmiiModeGet
*
* DESCRIPTION:
*       Gets XGMII mode on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
* OUTPUTS:
*       modePtr  - Pointer to XGMII mode
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortXGmiiModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
);

/*******************************************************************************
* cpssDxChPortIpgBaseSet
*
* DESCRIPTION:
*       Sets IPG base for fixed IPG mode on XG ports.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       ipgBase  - IPG base
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number, device or ipgBase
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*******************************************************************************/
GT_STATUS cpssDxChPortIpgBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
);

/*******************************************************************************
* cpssDxChPortIpgBaseGet
*
* DESCRIPTION:
*       Gets IPG base for fixed IPG mode on XG ports.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
* OUTPUTS:
*       ipgBasePtr  - pointer to IPG base
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*
* COMMENTS:
*       Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*******************************************************************************/
GT_STATUS cpssDxChPortIpgBaseGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
);

/****************************************************************************
* cpssDxChPortIpgSet
*
* DESCRIPTION:
*       Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*       Using this API may be required to enable wire-speed in traffic paths
*       that include cascading ports, where it may not be feasible to reduce
*       the preamble length.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical or CPU port number
*       ipg         - IPG in bytes, acceptable range:
*                     [0..511] for DxCh3 and above devices.
*                     [0..15] for DxCh1 and DxCh2 devices.
*                     Default HW value is 12 bytes.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_HW_ERROR      - on hardware error
*       GT_NOT_SUPPORTED - on wrong port type
*       GT_OUT_OF_RANGE  - ipg value out of range
*       GT_BAD_PARAM     - on bad parameter
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPortIpgSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  ipg
);

/****************************************************************************
* cpssDxChPortIpgGet
*
* DESCRIPTION:
*       Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical or CPU port number
*
* OUTPUTS:
*       ipgPtr      - (pointer to) IPG value in bytes
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_HW_ERROR      - on hardware error
*       GT_NOT_SUPPORTED - on wrong port type
*       GT_BAD_PARAM     - on bad parameter
*       GT_BAD_PTR       - one of the parameters is NULL pointer
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPortIpgGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32  *ipgPtr
);

/*******************************************************************************
* cpssDxChPortExtraIpgSet
*
* DESCRIPTION:
*       Sets the number of 32-bit words to add to the 12-byte IPG.
*       Hence, 12+4*ExtraIPG is the basis for the entire IPG calculation.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       number   -  number of words
*                   for DxCh,DxCh2  : number range 0..3
*                   DxCh3 and above : number range 0..127
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - the number is out of range
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortExtraIpgSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8       number
);

/*******************************************************************************
* cpssDxChPortExtraIpgGet
*
* DESCRIPTION:
*       Gets the number of 32-bit words to add to the 12-byte IPG.
*       Hence, 12+4*ExtraIPG is the basis for the entire IPG calculation.
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
*
* OUTPUTS:
*       numberPtr -  pointer to number of words
*                   for DxCh,DxCh2  : number range 0..3
*                   DxCh3 and above : number range 0..127
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - the number is out of range
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortExtraIpgGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8       *numberPtr
);

/*******************************************************************************
* cpssDxChPortXgmiiLocalFaultGet
*
* DESCRIPTION:
*       Reads bit then indicate if the XGMII RS has detected local
*       fault messages.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum         - physical device number
*       portNum        - physical port number
*
* OUTPUTS:
*       isLocalFaultPtr - GT_TRUE, if bit set or
*                         GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isLocalFaultPtr
);

/*******************************************************************************
* cpssDxChPortXgmiiRemoteFaultGet
*
* DESCRIPTION:
*       Reads bit then indicate if the XGMII RS has detected remote
*       fault messages.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - physical device number
*       portNum         - physical port number
*
* OUTPUTS:
*       isRemoteFaultPtr  - GT_TRUE, if bit set or
*                           GT_FALSE otherwise
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
);

/*******************************************************************************
* cpssDxChPortMacStatusGet
*
* DESCRIPTION:
*       Reads bits that indicate different problems on specified port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*
* OUTPUTS:
*       portMacStatusPtr  - info about port MAC
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacStatusGet
(
    IN  GT_U8                      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
);

/*******************************************************************************
* cpssDxChPortInternalLoopbackEnableSet
*
* DESCRIPTION:
*       Set the internal Loopback state in the packet processor MAC port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       enable    - If GT_TRUE, enable loopback
*                   If GT_FALSE, disable loopback
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       For port 25 in DX269 (XG/HX port) it's the application responsibility to
*       reconfigure the loopback mode after switching XG/HX mode.
*
*******************************************************************************/
GT_STATUS cpssDxChPortInternalLoopbackEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/*******************************************************************************
* cpssDxChPortInternalLoopbackEnableGet
*
* DESCRIPTION:
*       Get Internal Loopback
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*
* OUTPUTS:
*       enablePtr  - Pointer to the Loopback state.
*                    If GT_TRUE, loopback is enabled
*                    If GT_FALSE, loopback is disabled
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not relevant for the CPU port.
*
*******************************************************************************/
GT_STATUS cpssDxChPortInternalLoopbackEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);


/*******************************************************************************
* cpssDxChPortInbandAutoNegEnableSet
*
* DESCRIPTION:
*       Configure Auto-Negotiation mode of MAC for a port.
*       The Tri-Speed port MAC may operate in one of the following two modes:
*       - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*       - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum  - device number
*       portNum - port number
*       enable  - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                 GT_FALSE - Auto-Negotiation works in out-of-band via
*                            the device's Master SMI interface mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - The feature is not supported the port/device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*******************************************************************************/
GT_STATUS cpssDxChPortInbandAutoNegEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChPortInbandAutoNegEnableGet
*
* DESCRIPTION:
*       Gets Auto-Negotiation mode of MAC for a port.
*       The Tri-Speed port MAC may operate in one of the following two modes:
*       - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*       - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum  - device number
*       portNum - port number
*
* OUTPUTS:
*       enablePtr  - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                    GT_FALSE - Auto-Negotiation works in out-of-band via
*                            the device's Master SMI interface mode.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - The feature is not supported the port/device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*******************************************************************************/
GT_STATUS cpssDxChPortInbandAutoNegEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL  *enablePtr
);

/*******************************************************************************
* cpssDxChPortAttributesOnPortGet
*
* DESCRIPTION:
*       Gets port attributes for particular logical port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - device number
*       portNum    - port number (or CPU port)
*
* OUTPUTS:
*       portAttributSetArrayPtr - Pointer to attributes values array.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortAttributesOnPortGet
(
    IN    GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
);

/*******************************************************************************
* cpssDxChPortPreambleLengthSet
*
* DESCRIPTION:
*       Set the port with preamble length for Rx or Tx or both directions.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - device number.
*       portNum      - physical port number (or CPU port)
*       direction    - Rx or Tx or both directions
*                      only XG ports support Rx direction and "both directions"
*                      options (GE ports support only Tx direction)
*       length       - length of preamble in bytes
*                      support only values of 4,8
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM - on wrong device number or wrong physical port number or
*                      wrong direction or wrong length
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortPreambleLengthSet(
    IN GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
);

/*******************************************************************************
* cpssDxChPortPreambleLengthGet
*
* DESCRIPTION:
*       Get the port with preamble length for Rx or Tx or both directions.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - device number.
*       portNum      - physical port number (or CPU port)
*       direction    - Rx or Tx or both directions
*                      only XG ports support Rx direction
*                      GE ports support only Tx direction.
*
*
* OUTPUTS:
*       lengthPtr    - pointer to preamble length in bytes :
*                      supported length values are : 4,8.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_BAD_PARAM - on wrong device number or wrong physical port number or
*                      wrong direction or wrong length
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortPreambleLengthGet(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_U32                   *lengthPtr
);

/*******************************************************************************
* cpssDxChPortMacSaBaseSet
*
* DESCRIPTION:
*       Sets the base part(40 upper bits) of all device's ports MAC addresses.
*       Port MAC addresses are used as the MAC SA for Flow Control Packets
*       transmitted by the device. In addition these addresses can be used as
*       MAC DA for Flow Control packets received by these ports.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device number
*       macPtr - (pointer to)The system Mac address to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
);

/*******************************************************************************
* cpssDxChPortMacSaBaseGet
*
* DESCRIPTION:
*       Gets the base part (40 upper bits) of all device's ports MAC addresses.
*       Port MAC addresses are used as the MAC SA for Flow Control Packets
*       transmitted by the device.In addition these addresses can be used as
*       MAC DA for Flow Control packets received by these ports.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device number
*
* OUTPUTS:
*       macPtr - (pointer to)The system Mac address.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
);

/*******************************************************************************
* cpssDxChPortSerdesComboModeSet
*
* DESCRIPTION:
*       Set Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*       Combo Serdes are active for the port, else regular Serdes are active for
*       the port.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum      - device number
*       portNum  - physical port number, values 20..23.
*       mode     - serdes mode.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum, mode
*       GT_FAIL         - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesComboModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_SERDES_MODE_ENT  mode
);

/*******************************************************************************
* cpssDxChPortSerdesComboModeGet
*
* DESCRIPTION:
*       Get Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*       Combo Serdes are active for the port, else regular Serdes are active for
*       the port.
*
* APPLICABLE DEVICES:
*        DxCh3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number, values 20..23.
*
* OUTPUTS:
*       modePtr   -  pointer to Serdes mode.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesComboModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_SERDES_MODE_ENT    *modePtr
);

/*******************************************************************************
* cpssDxChPortSerdesConfigSet
*
* DESCRIPTION:
*       Set SERDES configuration per port
*
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number
*       portNum         - physical port number.
*       serdesCfgPtr    - pointer to port serdes configuration.
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_OUT_OF_RANGE - on SERDES parameters out of range
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*
*       WARNING: This API is obsolete, kept for backword compatibility 
*       and highly adviced not to use it. The right way is to use 
*       cpssDxChPortSerdesTuningSet!!!
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesConfigSet
(
    IN GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PORT_SERDES_CONFIG_STC  *serdesCfgPtr
);

/*******************************************************************************
* cpssDxChPortSerdesConfigGet
*
* DESCRIPTION:
*       Get SERDES configuration per port.
*
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum          - device number
*       portNum         - physical port number.
*
* OUTPUTS:
*       serdesCfgPtr    - pointer to port serdes configuration.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_INITIALIZED - on no initialized SERDES per port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        This API also checks if at least one serdes per port was initialized.
*        In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*        returned.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesConfigGet
(
    IN GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_SERDES_CONFIG_STC  *serdesCfgPtr
);

/*******************************************************************************
* cpssDxChPortPaddingEnableSet
*
* DESCRIPTION:
*       Enable/Disable padding of transmitted packets shorter than 64B.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number (or CPU port).
*       enable   - GT_TRUE  - Pad short packet in Tx.
*                - GT_FALSE - No padding in short packets.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/*******************************************************************************
* cpssDxChPortPaddingEnableGet
*
* DESCRIPTION:
*       Gets padding status of transmitted packets shorter than 64B.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number (or CPU port).
*
* OUTPUTS:
*       enablePtr   - pointer to packet padding status.
*                     - GT_TRUE  - Pad short packet in Tx.
*                     - GT_FALSE - No padding in short packets.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/*******************************************************************************
* cpssDxChPortExcessiveCollisionDropEnableSet
*
* DESCRIPTION:
*       Enable/Disable excessive collision packets drop.
*       In half duplex mode if a collision occurs the device tries to transmit
*       the packet again. If the number of collisions on the same packet is 16
*       and  excessive collision packets drop is enabled,
*       the packet is dropped.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number (or CPU port).
*       enable   - GT_TRUE  - if the number of collisions on the same packet
*                             is 16 the packet is dropped.
*                - GT_FALSE - A collided packet will be retransmitted by device
*                             until it is transmitted without collisions,
*                             regardless of the number of collisions on the packet.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*
* COMMENTS:
*       The setting is not relevant in full duplex mode
*       Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value 
*       is TRUE the switch MAC will be configured
*
*
*******************************************************************************/
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/*******************************************************************************
* cpssDxChPortExcessiveCollisionDropEnableGet
*
* DESCRIPTION:
*       Gets status of excessive collision packets drop.
*       In half duplex mode if a collision occurs the device tries to transmit
*       the packet again. If the number of collisions on the same packet is 16
*       and  excessive collision packets drop is enabled,
*       the packet is dropped.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - device number
*       portNum   - physical port number (or CPU port).
*
* OUTPUTS:
*       enablePtr - pointer to status of excessive collision packets drop.
*                   - GT_TRUE  - if the number of collisions on the same packet
*                               is 16 the packet is dropped.
*                   - GT_FALSE - A collided packet will be retransmitted by
*                                device until it is transmitted
*                                without collisions, regardless of the number
*                                of collisions on the packet.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*       1. Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*       2. Not relevant in full duplex mode
*
*******************************************************************************/
GT_STATUS cpssDxChPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/*******************************************************************************
* cpssDxChPortXgLanesSwapEnableSet
*
* DESCRIPTION:
*       Enable/Disable swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number.
*       enable   - GT_TRUE  - Swap Lanes by follows:
*                             The lanes are swapped by follows:
*                             Lane 0 PSC Tx data is connected to SERDES Lane 3
*                             Lane 1 PSC Tx data is connected to SERDES Lane 2
*                             Lane 2 PSC Tx data is connected to SERDES Lane 1
*                             Lane 3 PSC Tx data is connected to SERDES Lane 0
*
*                - GT_FALSE - Normal operation (no swapping)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*
* COMMENTS:
*       Supported for XAUI or HyperG.Stack ports only.
*
*******************************************************************************/
GT_STATUS cpssDxChPortXgLanesSwapEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/*******************************************************************************
* cpssDxChPortXgLanesSwapEnableGet
*
* DESCRIPTION:
*       Gets status of swapping XAUI or HyperG.Stack port SERDES Lanes.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number.
*
*
* OUTPUTS:
*       enablePtr - pointer to status of swapping XAUI PHY SERDES Lanes.
*                    - GT_TRUE  - Swap Lanes by follows:
*                             The lanes are swapped by follows:
*                             Lane 0 PSC Tx data is connected to SERDES Lane 3
*                             Lane 1 PSC Tx data is connected to SERDES Lane 2
*                             Lane 2 PSC Tx data is connected to SERDES Lane 1
*                             Lane 3 PSC Tx data is connected to SERDES Lane 0
*                    - GT_FALSE - Normal operation (no swapping)
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*       Supported for XAUI or HyperG.Stack ports only.
*
*******************************************************************************/
GT_STATUS cpssDxChPortXgLanesSwapEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/*******************************************************************************
* cpssDxChPortXgPscLanesSwapSet
*
* DESCRIPTION:
*       Set swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum   - device number.
*       portNum  - physical port number.
*       rxSerdesLaneArr - array for binding port Rx PSC and SERDES lanes:
*                         array index is PSC lane, array value is SERDES lane.
*       txSerdesLaneArr - array for binding port Tx PSC and SERDES lanes:
*                         array index is PSC lane, array value is SERDES lane.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_HW_ERROR     - on hardware error
*       GT_OUT_OF_RANGE - wrong SERDES lane
*       GT_BAD_VALUE    - multiple connection detected
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Supported for XAUI or HyperG.Stack ports only.
*
*******************************************************************************/
GT_STATUS cpssDxChPortXgPscLanesSwapSet
(
    IN GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS]
);

/*******************************************************************************
* cpssDxChPortXgPscLanesSwapGet
*
* DESCRIPTION:
*       Get swapping configuration of XAUI or HyperG.Stack port SERDES Lanes.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum   - device number.
*       portNum  - physical port number.
*
* OUTPUTS:
*       rxSerdesLaneArr - array for binding port Rx PSC and SERDES lanes:
*                         array index is PSC lane, array value is SERDES lane.
*       txSerdesLaneArr - array for binding port Tx PSC and SERDES lanes:
*                         array index is PSC lane, array value is SERDES lane.
*
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_HW_ERROR     - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Supported for XAUI or HyperG.Stack ports only.
*
*******************************************************************************/
GT_STATUS cpssDxChPortXgPscLanesSwapGet
(
    IN GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS]
);

/*******************************************************************************
* cpssDxChPortInBandAutoNegBypassEnableSet
*
* DESCRIPTION:
*       Enable/Disable Auto-Negotiation by pass.
*       If the link partner doesn't respond to Auto-Negotiation process,
*       the link is established by bypassing the Auto-Negotiation procedure.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number.
*       enable   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                - GT_FALSE - Auto-Negotiation is bypassed.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK            - on success
*       GT_BAD_PARAM     - wrong devNum, portNum
*       GT_NOT_SUPPORTED - The feature is not supported the port/device
*       GT_FAIL          - on error
*
* COMMENTS:
*       Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*
*       Although for XGMII (10 Gbps) ports feature is not supported the function 
*       lets application to configure it
*
*******************************************************************************/
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL    enable
);

/*******************************************************************************
* cpssDxChPortInBandAutoNegBypassEnableGet
*
* DESCRIPTION:
*       Gets Auto-Negotiation by pass status.
*       If the link partner doesn't respond to Auto-Negotiation process,
*       the link is established by bypassing the Auto-Negotiation procedure.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number.
*
* OUTPUTS:
*       enablePtr - pointer to Auto-Negotiation by pass status.
*                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                   - GT_FALSE - Auto-Negotiation is bypassed.
*
* RETURNS:
*       GT_OK            - on success
*       GT_BAD_PARAM     - wrong devNum, portNum
*       GT_NOT_SUPPORTED - The feature is not supported the port/device
*       GT_FAIL          - on error
*       GT_BAD_PTR       - one of the parameters is NULL pointer
*
* COMMENTS:
*       Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssDxChPortInbandAutoNegEnableSet.)
*
*       Although for XGMII (10 Gbps) ports feature is not supported the function 
*       lets application to configure it
*
*******************************************************************************/
GT_STATUS cpssDxChPortInBandAutoNegBypassEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL    *enablePtr
);

/*******************************************************************************
* cpssDxChPortMacErrorIndicationPortSet
*
* DESCRIPTION:
*       Sets port monitored for MAC errors.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum   - device number
*       portNum  - port monitored for MAC errors.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*
* COMMENTS:
*       MAC source addresses and the error type can be read
*       for the port. See cpssDxChPortMacErrorIndicationGet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacErrorIndicationPortSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/*******************************************************************************
* cpssDxChPortMacErrorIndicationPortGet
*
* DESCRIPTION:
*       Gets port monitored for MAC errors.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum      - device number
*
* OUTPUTS:
*       portNumPtr  - pointer to port monitored for MAC errors.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*       MAC source addresses and the error type can be read
*       for the port. See cpssDxChPortMacErrorIndicationGet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacErrorIndicationPortGet
(
    IN  GT_U8      devNum,
    OUT GT_PHYSICAL_PORT_NUM      *portNumPtr
);

/*******************************************************************************
* cpssDxChPortMacErrorIndicationGet
*
* DESCRIPTION:
*       Gets MAC source addresses and the error type for the monitored port.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* NOT APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* INPUTS:
*       devNum      - device number
*
* OUTPUTS:
*       typePtr     - pointer to MAC error type.
*       macPtr      - pointer to MAC SA of the last packet received
*                     with  a MAC error on MAC error indication port.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*
* COMMENTS:
*      Port monitored for MAC errors should be set.
*      See cpssDxChPortMacErrorIndicationPortSet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacErrorIndicationGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PORT_MAC_ERROR_TYPE_ENT   *typePtr,
    OUT GT_ETHERADDR                        *macPtr
);

/******************************************************************************
* cpssDxChPortSerdesPowerStatusSet
*
* DESCRIPTION:
*       Sets power state of SERDES port lanes according to port capabilities.
*       XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*       HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1_Diamond.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       direction - may be either CPSS_PORT_DIRECTION_RX_E,
*                                 CPSS_PORT_DIRECTION_TX_E, or
*                                 CPSS_PORT_DIRECTION_BOTH_E.
*       lanesBmp  - bitmap of SERDES lanes (bit 0-> lane 0, etc.)
*       powerUp   - GT_TRUE  = power up, GT_FALSE = power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_NOT_SUPPORTED - HW does not support the requested operation
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     DxCh3 and above devices supports only CPSS_PORT_DIRECTION_BOTH_E.
*
******************************************************************************/
GT_STATUS cpssDxChPortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

/******************************************************************************
* cpssDxChPortGroupSerdesPowerStatusSet
*
* DESCRIPTION:
*       Sets power state of SERDES port lanes per group
*       according to port capabilities.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum     - physical device number
*       portSerdesGroup  - port group number
*                 DxCh3 Giga/2.5 G, xCat GE devices:
*                        Ports       |    SERDES Group
*                                 0..3        |      0
*                                 4..7        |      1
*                                 8..11       |      2
*                                 12..15      |      3
*                                 16..19      |      4
*                                 20..23      |      5
*                                 24          |      6
*                                 25          |      7
*                                 26          |      8
*                                 27          |      9
*                DxCh3 XG devices:
*                                 0           |      0
*                                 4           |      1
*                                 10          |      2
*                                 12          |      3
*                                 16          |      4
*                                 22          |      5
*                                 24          |      6
*                                 25          |      7
*                                 26          |      8
*                                 27          |      9
*               xCat FE devices
*                                 24          |      6
*                                 25          |      7
*                                 26          |      8
*                                 27          |      9
*               Lion devices: Port == SERDES Group
*
*       powerUp   - GT_TRUE  = power up, GT_FALSE = power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on wrong devNum, portSerdesGroup
*     GT_FAIL          - on error
*     GT_NOT_SUPPORTED - HW does not support the requested operation
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     Get SERDES port group according to port by calling to
*     cpssDxChPortSerdesGroupGet.
*
******************************************************************************/
GT_STATUS cpssDxChPortGroupSerdesPowerStatusSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     portSerdesGroup,
    IN  GT_BOOL    powerUp
);

/******************************************************************************
* cpssDxChPortSerdesGroupGet
*
* DESCRIPTION:
*       Get SERDES port group that may be used in per SERDES group APIs.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*
* OUTPUTS:
*       portSerdesGroupPtr  - Pointer to port group number
*                 DxCh3 Giga/2.5 G, xCat GE devices:
*                        Ports       |    SERDES Group
*                        0..3        |      0
*                        4..7        |      1
*                        8..11       |      2
*                        12..15      |      3
*                        16..19      |      4
*                        20..23      |      5
*                        24          |      6
*                        25          |      7
*                        26          |      8
*                        27          |      9
*                DxCh3 XG devices:
*                        0           |      0
*                        4           |      1
*                        10          |      2
*                        12          |      3
*                        16          |      4
*                        22          |      5
*                        24          |      6
*                        25          |      7
*                        26          |      8
*                        27          |      9
*               xCat FE devices
*                        24          |      6
*                        25          |      7
*                        26          |      8
*                        27          |      9
*               Lion devices: Port == SERDES Group
*
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on wrong devNum, portNum
*     GT_FAIL          - on error
*     GT_BAD_PTR       - on NULL pointer
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     None.
*
*
******************************************************************************/
GT_STATUS cpssDxChPortSerdesGroupGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32   *portSerdesGroupPtr
);


/*******************************************************************************
* cpssDxChPortSerdesCalibrationStartSet
*
* DESCRIPTION:
*       Start calibration on all SERDES of given port.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Lion3.
*
* INPUTS:
*       devNum          - device number
*       portNum         - physical port number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum, portNum
*       GT_FAIL                  - on error
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         - on not supported port type
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesCalibrationStartSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum
);

/*******************************************************************************
* cpssDxChPortFlowControlModeSet
*
* DESCRIPTION:
*       Sets Flow Control mode on given port.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
*
* INPUTS:
*       devNum     - device number.
*       portNum    - port number, CPU port number.
*       fcMode     - flow control mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       For Lion and above:
*       This function also configures insertion of DSA tag for PFC frames.
*
*******************************************************************************/
GT_STATUS cpssDxChPortFlowControlModeSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
);

/*******************************************************************************
* cpssDxChPortFlowControlModeGet
*
* DESCRIPTION:
*       Gets Flow Control mode on given port.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
*
* INPUTS:
*       devNum     - device number.
*       portNum    - port number, CPU port number.
*
* OUTPUTS:
*       fcModePtr  - flow control mode.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortFlowControlModeGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT   *fcModePtr
);

/*******************************************************************************
* cpssDxChPortMacResetStateSet
*
* DESCRIPTION:
*       Set MAC Reset state on specified port on specified device.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - device number
*       portNum  - port number (or CPU port)
*       state    - Reset state
*                  GT_TRUE   - Port MAC is under Reset
*                  GT_FALSE - Port MAC is Not under Reset, normal operation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/*******************************************************************************
* cpssDxChPortSerdesResetStateSet
*
* DESCRIPTION:
*       Set SERDES Reset state on specified port on specified device.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* INPUTS:
*       devNum   - device number
*       portNum  - port number (or CPU port)
*       state  - Reset state
*                GT_TRUE   - Port SERDES is under Reset
*                GT_FALSE - Port SERDES is Not under Reset, normal operation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
);

/*******************************************************************************
* cpssDxChPortModeSpeedAutoDetectAndConfig
*
* DESCRIPTION:
*       Autodetect and configure interface mode and speed of a given port
*       If the process succeeded the port gets configuration ready for link,
*       otherwise (none of provided by application options valid) the port
*       is left in reset mode.
*
* APPLICABLE DEVICES:
*        Lion; xCat; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Lion3.
*
* INPUTS:
*       devNum   - device number
*       portNum  - port number (CPU port not supported, FE ports not supported)
*       portModeSpeedOptionsArrayPtr – array of port mode and speed options
*                                      preferred for application,
*                                   NULL – if any of supported options acceptable
*       optionsArrayLen – length of options array (must be 0 if
*                                     portModeSpeedOptionsArrayPtr == NULL)
*
*
* OUTPUTS:
*       currentModePtr – if succeeded ifMode and speed that were configured
*                        on the port,
*                        otherwise previous ifMode and speed restored
*
* RETURNS:
*       GT_OK   - if process completed without errors (no matter if mode
*                                                       configured or not)
*       GT_BAD_PARAM             - on wrong port number or device,
*                                   optionsArrayLen more then possible options
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_NOT_SUPPORTED         – ifMode/speed pair not supported, 
*                                   wrong port type
*       GT_HW_ERROR              - HW error
*
* COMMENTS:
*       For list of supported modes see:
*           lionDefaultPortModeSpeedOptionsArray,
*           xcatDefaultNetworkPortModeSpeedOptionsArray,
*           xcatDefaultStackPortModeSpeedOptionsArray
*
*       There is no possibility to distinguish if partner has SGMII or
*       1000BaseX, when auto-negotiation disabled. Preference to 1000BaseX given
*       by algorithm.
*
*       Responsibility of application to mask link change interrupt during
*       auto-detection algorithm run.
*
*******************************************************************************/
GT_STATUS cpssDxChPortModeSpeedAutoDetectAndConfig
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   CPSS_PORT_MODE_SPEED_STC *portModeSpeedOptionsArrayPtr,
    IN   GT_U8                    optionsArrayLen,
    OUT  CPSS_PORT_MODE_SPEED_STC *currentModePtr
);

/*******************************************************************************
* cpssDxChPortForward802_3xEnableSet
*
* DESCRIPTION:
*       Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*       pipeline of a specified port. Processing of  802.3x Flow Control frames
*       is done like regular data frames if forwarding enabled.
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (CPU port not supported)
*       enable   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                  GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port’s packet transmission if it is an XOFF packet, or to
*       resume the port’s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 00-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*******************************************************************************/
GT_STATUS cpssDxChPortForward802_3xEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
);

/*******************************************************************************
* cpssDxChPortForward802_3xEnableGet
*
* DESCRIPTION:
*       Get status of 802.3x frames forwarding on a specified port
*
* APPLICABLE DEVICES:
*        DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (CPU port not supported)
*
* OUTPUTS:
*       enablePtr - status of 802.3x frames forwarding
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port’s packet transmission if it is an XOFF packet, or to
*       resume the port’s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 00-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*******************************************************************************/
GT_STATUS cpssDxChPortForward802_3xEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/*******************************************************************************
* cpssDxChPortForwardUnknownMacControlFramesEnableSet
*
* DESCRIPTION:
*       Enable/disable forwarding of unknown MAC control frames to the ingress
*       pipeline of a specified port. Processing of unknown MAC control frames
*       is done like regular data frames if forwarding enabled.
*
* APPLICABLE DEVICES:
*        xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (CPU port not supported)
*       enable   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                  GT_FALSE: do not forward unknown MAC control frames to
*                               the ingress pipe.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is not 00-01 and not 01-01
*           OR
*         Packet’s MAC DA is not 01-80-C2-00-00-01 and not the port’s configured
*           MAC Address
*
*******************************************************************************/
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL enable
);


/*******************************************************************************
* cpssDxChPortForwardUnknownMacControlFramesEnableGet
*
* DESCRIPTION:
*       Get current status of unknown MAC control frames
*           forwarding on a specified port
*
* APPLICABLE DEVICES:
*        xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion.
*
*
* INPUTS:
*       devNum      - physical device number
*       portNum     - physical port number (CPU port not supported)
*
* OUTPUTS:
*       enablePtr   - status of unknown MAC control frames forwarding
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - on NULL pointer
*
* COMMENTS:
*       A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is not 00-01 and not 01-01
*           OR
*         Packet’s MAC DA is not 01-80-C2-00-00-01 and not the port’s configured
*           MAC Address
*
*******************************************************************************/
GT_STATUS cpssDxChPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/*******************************************************************************
* cpssDxChPortSerdesTxEnableSet
*
* DESCRIPTION:
*        Enable / Disable transmission of packets in SERDES layer of a port.
*        Use this API to disable Tx for loopback ports.
*
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number
*       portNum         - physical port number.
*       enable          - GT_TRUE  - Enable transmission of packets in
*                                    SERDES layer of a port
*                       - GT_FALSE - Disable transmission of packets in
*                                    SERDES layer of a port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum, portNum
*       GT_FAIL                  - on error
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesTxEnableSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable
);

/*******************************************************************************
* cpssDxChPortSerdesTxEnableGet
*
* DESCRIPTION:
*       Get Enable / Disable transmission of packets in SERDES layer of a port.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum          - device number
*       portNum         - physical port number.
*
* OUTPUTS:
*       enablePtr       - Pointer to transmission of packets in SERDES
*                         layer of a port.
*                       - GT_TRUE  - Enable transmission of packets in
*                                    SERDES layer of a port.
*                       - GT_FALSE - Enable transmission of packets in
*                                    SERDES layer of a port.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - wrong devNum, portNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - on NULL pointer
*       GT_NOT_SUPPORTED         - on not supported feature for a port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesTxEnableGet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr
);

/******************************************************************************
* cpssDxChPortSerdesTuningSet
*
* DESCRIPTION:
*       SerDes fine tuning values set.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       laneBmp   - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where values
*                   in tuneValuesPtr appliable
*       serdesFrequency - serdes speed for which to save tuning values
*       tuneValuesPtr   - (ptr to) structure with tuned values
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_OUT_OF_RANGE  - one of tune values is out of range
*     GT_FAIL          - on error
*     GT_BAD_PTR       - one of the parameters is NULL pointer
*     GT_NOT_SUPPORTED - HW does not support the requested operation
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     Set same tuning parameters for all lanes defined in laneBmp in SW DB, 
*       then cpssDxChPortSerdesPowerStatusSet will write them to HW.
*
******************************************************************************/
GT_STATUS cpssDxChPortSerdesTuningSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency, 
    IN  CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/******************************************************************************
* cpssDxChPortSerdesTuningGet
*
* DESCRIPTION:
*       SerDes fine tuning values get.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - physical device number
*       portNum - physical port number
*       laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) 
*       serdesFrequency - serdes speed for which to save tuning values
*
* OUTPUTS:
*       tuneValuesPtr   - (ptr to) structure to put tuned values
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_BAD_PTR       - one of the parameters is NULL pointer
*     GT_NOT_SUPPORTED - HW does not support the requested operation
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Reads values saved in SW DB by cpssDxChPortSerdesTuningSet or 
*       if it was not called, from default matrix (...SerdesPowerUpSequence).
*
******************************************************************************/
GT_STATUS cpssDxChPortSerdesTuningGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum,
    IN  CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC *tuneValuesPtr
);

/******************************************************************************
* cpssDxChPortPeriodicFlowControlIntervalSet
*
* DESCRIPTION:
*       Set Periodic Flow Control interval.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum    - physical device number
*       portType  - port type 
*       value     - The interval in microseconds between two successive 
*                   Flow Control frames that are sent periodically by the port.
*                   (APPLICABLE RANGES: 0..171798691)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad devNum, portType
*     GT_OUT_OF_RANGE  - on bad value
*     GT_FAIL          - on error
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     None.
*
******************************************************************************/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalSet 
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    IN  GT_U32                              value
);

/******************************************************************************
* cpssDxChPortPeriodicFlowControlIntervalGet
*
* DESCRIPTION:
*       Get Periodic Flow Control interval.
*
* APPLICABLE DEVICES:
*        Lion; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2.
*
* INPUTS:
*       devNum    - physical device number
*       portType  - port type 
*
* OUTPUTS:
*       valuePtr  - pointer to interval value in microseconds between two 
*                   successive Flow Control frames that are sent periodically
*                   by the port.
*                   (APPLICABLE RANGES: 0..171798691)
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad devNum, portType
*     GT_FAIL          - on error
*     GT_BAD_PTR       - on NULL pointer
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*     None.
*
******************************************************************************/
GT_STATUS cpssDxChPortPeriodicFlowControlIntervalGet 
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT portType,
    OUT GT_U32                              *valuePtr
);


/*******************************************************************************
* cpssDxChPortModeSpeedSet
*
* DESCRIPTION:
*       Configure Interface mode and speed on a specified port and execute
*        on port's serdeses power up sequence; or configure power down on port's
*        serdeses.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum    - physical device number
*       portsBmp  - physical port number (or CPU port)
*       powerUp   - serdes power:
*                       GT_TRUE - up;
*                       GT_FALSE - down;
*       ifMode    - interface mode (related only for serdes power up [powerUp==GT_TRUE])
*       speed     - port data speed (related only for serdes power up [powerUp==GT_TRUE])
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_SUPPORTED  - on not supported interface for given port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*        Supposed to replace old API's:
*           cpssDxChPortInterfaceModeSet
*           cpssDxChPortSpeedSet
*           cpssDxChPortSerdesPowerStatusSet
*
*******************************************************************************/
GT_STATUS cpssDxChPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/*******************************************************************************
* cpssDxChPortExtendedModeEnableSet
*
* DESCRIPTION:
*       Define which GE and XG MAC ports 9 and 11 of every mini-GOP will use - 
*           from local mini-GOP or extended from other mini-GOP
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (not CPU port)
*       enable   - extended mode:
*                       GT_TRUE - use extended MAC;
*                       GT_FALSE - use local MAC;
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       For now if application interested to implement GE and XG modes of ports 9 
*       and 11 of every mini-GOP over extended MAC's, it can call this function 
*       at init stage once for port 9 and once for port 11 and enough
*******************************************************************************/
GT_STATUS cpssDxChPortExtendedModeEnableSet 
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/*******************************************************************************
* cpssDxChPortExtendedModeEnableGet
*
* DESCRIPTION:
*       Read which GE and XG MAC ports 9 and 11 of every mini-GOP will use - 
*           from local mini-GOP or extended from other mini-GOP
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number (not CPU port)
*
* OUTPUTS:
*       enablePtr - extended mode:
*                       GT_TRUE - use extended MAC;
*                       GT_FALSE - use local MAC;
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_BAD_PTR        - enablePtr is NULL
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       
*******************************************************************************/
GT_STATUS cpssDxChPortExtendedModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/*******************************************************************************
* cpssDxChPortSerdesAutoTune
*
* DESCRIPTION:
*       Run auto tune algorithm on given port's serdes. 
*       Set the port Tx and Rx parameters according to different working 
*       modes/topologies.
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*       portTuningMode - port tuning mode
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success, for TX_TRAINING_STATUS means training
*                           succeeded
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL           - on error, for TX_TRAINING_STATUS means training
*                           failed
*
* COMMENTS: 
*       Directions for TX training execution: 
*       1.	As a pre-condition, ports on both sides of the link must be UP. 
*       2.	For 2 sides of the link call execute TX_TRAINING_CFG;
*           No need to maintain special timing sequence between them. 
*           The CFG phase sets some parameters at the SerDes as a preparation 
*           to the training phase. 
*       3.	After CFG is done, for both sides of the link, call TX_TRAINING_START;
*           No need to maintain special timing sequence between them. 
*       4.	Wait at least 0.5 Sec. (done by API inside).
*       5.	Verify Training status by calling TX_TRAINING_STATUS.
*           No need to maintain special timing sequence between them. 
*           This call provides the training status (OK/Failed) and terminates it. 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesAutoTune 
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode
);

/*******************************************************************************
* cpssDxChPortSerdesAutoTuneStatusGet
*
* DESCRIPTION:
*       Get current status of RX and TX serdes auto-tuning on port.
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number
*
* OUTPUTS:
*       rxTuneStatusPtr - RX tuning status
*       txTuneStatusPtr - TX tuning status
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL           - on error
*       GT_BAD_PTR      - one of the parameters is NULL pointer
*       GT_HW_ERROR     - on hardware error
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesAutoTuneStatusGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
);

/*******************************************************************************
* cpssDxChPortSerdesPolaritySet
*
* DESCRIPTION:
*   Invert the Tx or Rx serdes polarity.
*
* APPLICABLE DEVICES:
*   xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*   laneBmp - bitmap of SERDES lanes (bit 0-> lane 0, etc.) to define
*               polarity on.
*   invertTx –  GT_TRUE - Transmit Polarity Invert.
*               GT_FALSE – no invert
*   invertRx –  GT_TRUE - Receive Polarity Invert.
*               GT_FALSE – no invert
*
* OUTPUTS:
*   None.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*   This API should be egaged by application after serdes power up. Important to
*       wrap both steps i.e. "serdes power up" and "serdes polarity set" by port 
*       disable and link_change interrupt lock and restore port enable and 
*       reenable link change interrupt only after "serdes polarity set" to 
*       prevent interrupt toggling during the process.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesPolaritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               laneBmp,
    IN GT_BOOL              invertTx,
    IN GT_BOOL              invertRx
);

/*******************************************************************************
* cpssDxChPortSerdesPolarityGet
*
* DESCRIPTION:
*   Get status of the Tx or Rx serdes polarity invert.
*
* APPLICABLE DEVICES:
*   xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*   laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.) to
*               define polarity on.
*
* OUTPUTS:
*   invertTxPtr – (ptr to) GT_TRUE - Transmit Polarity Invert.
*                          GT_FALSE – no invert
*   invertRxPtr – (ptr to) GT_TRUE - Receive Polarity Invert.
*                          GT_FALSE – no invert
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_BAD_PTR                  - NULL pointer
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesPolarityGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  laneNum,
    OUT GT_BOOL *invertTxPtr,
    OUT GT_BOOL *invertRxPtr
);

/******************************************************************************
* cpssDxChPortSerdesLoopbackModeSet
*
* DESCRIPTION:
*       Configure loopback of specific type on SerDes.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       laneBmp   - bitmap of SERDES lanes (bit 0-> lane 0, etc.) where to
*                   set loopback (not used for Lion2)
*       mode      - define loopback type or no loopback
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*     GT_NOT_INITIALIZED       - if serdes was not intialized
*
* COMMENTS:
*       Pay attention - when new loopback mode enabled on serdes lane,
*                       previous mode disabled
*
******************************************************************************/
GT_STATUS cpssDxChPortSerdesLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneBmp,
    IN  CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT mode
);

/******************************************************************************
* cpssDxChPortSerdesLoopbackModeGet
*
* DESCRIPTION:
*       Get current mode of loopback on SerDes.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - physical device number
*       portNum - physical port number
*       laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*                   to read loopback status
*
* OUTPUTS:
*       modePtr - current loopback mode or none
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_BAD_PTR       - NULL pointer
*     GT_NOT_SUPPORTED - on not expected mode value
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*     GT_NOT_INITIALIZED       - if serdes was not intialized
*
* COMMENTS:
*
******************************************************************************/
GT_STATUS cpssDxChPortSerdesLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  laneNum,
    OUT CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT *modePtr
);

/*******************************************************************************
* cpssDxChPortEomDfeResGet
*
* DESCRIPTION:
*       Returns the current DFE parameters.
*
* APPLICABLE DEVICES:
*         Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion3.
*
* INPUTS:
*       devNum    - system device number
*       portNum   - port  number
*       serdesNum - local serdes number
*
* OUTPUTS:
*       dfeResPtr - current DFE V in millivolts
*
* RETURNS:
*       GT_OK  - on success
*       else  - on error
*
*******************************************************************************/
GT_STATUS cpssDxChPortEomDfeResGet
(
	IN  GT_U8                   devNum,               
	IN  GT_PHYSICAL_PORT_NUM    portNum,
	IN  GT_U32                  serdesNum,
	OUT GT_U32                 *dfeResPtr
);

/*******************************************************************************
* cpssDxChPortEomMatrixGet
*
* DESCRIPTION:
*       Returns the eye mapping matrix.
*
* APPLICABLE DEVICES:
*         Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion3.
*
* INPUTS:
*       devNum    - system device number
*       portNum   - port  number
*       serdesNum - local serdes number
*       samplingTime   - sampling time in usec, must be a positive value.
*
* OUTPUTS:
*       rowSizePtr - number of rows in matrix
*       matrixPtr - horizontal/vertical Rx eye matrix
*
* RETURNS:
*       GT_OK  - on success
*       else  - on error
*
* COMMENTS:
*       The function initializes the EOM mechanism for the serdes, 
*       gets the matrix and then closes the mechanism.
*
*       Due to it's size the CPSS_DXCH_PORT_EOM_MATRIX_STC struct should
*       be allocated on the heap.
*
*******************************************************************************/
GT_STATUS cpssDxChPortEomMatrixGet
(
	IN  GT_U8                            devNum,               
	IN  GT_PHYSICAL_PORT_NUM             portNum,
	IN  GT_U32                           serdesNum,
	IN  GT_U32                           samplingTime,
	OUT GT_U32                          *rowSizePtr,
	OUT CPSS_DXCH_PORT_EOM_MATRIX_STC   *matrixPtr
);

/*******************************************************************************
* cpssDxChPortEomBaudRateGet
*
* DESCRIPTION:
*       Returns the current system baud rate.
*
* APPLICABLE DEVICES:
*         Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion3.
*
* INPUTS:
*       devNum    - system device number
*       portNum   - port  number
*       serdesNum - local serdes number
*
* OUTPUTS:
*       baudRatePtr - current system baud rate in pico-seconds.
*
* RETURNS:
*       GT_OK  - on success
*       else  - on error
*
*******************************************************************************/
GT_STATUS cpssDxChPortEomBaudRateGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
	OUT GT_U32                   *baudRatePtr
);

/*******************************************************************************
* cpssDxChPortInbandAutoNegRestart
*
* DESCRIPTION:
*       Restart inband auto-negotiation. Relevant only when inband auto-neg. 
*        enabled.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum  - device number
*       portNum - port number (not-CPU)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR         - on hardware error
*       GT_BAD_PARAM         - The feature is not supported the port/device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortInbandAutoNegRestart
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
);

/*******************************************************************************
* cpssDxChPortGePrbsIntReplaceEnableSet
*
* DESCRIPTION:
*       Enable/disable replace <QSGMII PRBS error> interrupt by
*           <Signal Detect triggered> interrupt.
*
* APPLICABLE DEVICES:
*        xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* INPUTS:
*       devNum  - device number
*       portNum - port number (not-CPU)
*                   On XCAT2 supported only on: 0,4,8,12,16,20,24,25,26,27
*       enable  - GT_TRUE - bits of <QSGMII PRBS error> in Port Interrupt Cause
*                               and Port Interrupt Mask registers used for 
*                               <Signal Detect triggered>
*                 GT_FALSE - <QSGMII PRBS error> bits used for himself
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - The feature is not supported the port/device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortGePrbsIntReplaceEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL  enable
);

/*******************************************************************************
* cpssDxChPortGePrbsIntReplaceEnableGet
*
* DESCRIPTION:
*       Get status of replace <QSGMII PRBS error> interrupt by
*           <Signal Detect triggered> interrupt.
*
* APPLICABLE DEVICES:
*        xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; Lion3.
*
* INPUTS:
*       devNum  - device number
*       portNum - port number (not-CPU)
*                   On XCAT2 supported only on: 0,4,8,12,16,20,24,25,26,27
*
* OUTPUTS:
*       enablePtr - GT_TRUE - bits of <QSGMII PRBS error> in Port Interrupt Cause
*                               and Port Interrupt Mask registers used for 
*                               <Signal Detect triggered>
*                 GT_FALSE - <QSGMII PRBS error> bits used for himself
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - The feature is not supported the port/device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_BAD_PTR               - enablePtr is NULL
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortGePrbsIntReplaceEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL  *enablePtr
);

/*******************************************************************************
* cpssDxChPortInterfaceSpeedSupportGet
*
* DESCRIPTION:
*       Check if given pair ifMode and speed supported by given port on 
*        given device
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        None. 
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number (not CPU port) 
*       ifMode    - interface mode
*       speed     - port data speed
*
* OUTPUTS:
*       supportedPtr – GT_TRUE – (ifMode; speed) supported
*                      GT_FALSE – (ifMode; speed) not supported
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - if one of input parameters wrong
*       GT_BAD_PTR        - if supportedPtr is NULL
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssDxChPortInterfaceSpeedSupportGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
);

/*******************************************************************************
* cpssDxChPortSerdesPpmSet
*
* DESCRIPTION:
*       Increase/decrease Tx clock on port (added/sub ppm).
*       Can be run only after port configured, not under traffic.
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
* 
* INPUTS:
*       devNum      - system device number
*       portNum     - number of physical port all serdeses occupied by it, in 
*                       currently configured interface mode will be configured
*                       to required PPM
*       ppmValue    - signed value - positive means speed up, negative means slow
*                       down, 0 - means disable PPM.
*                     (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* OUTPUTS:
*       None.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters 
*   GT_OUT_OF_RANGE             - ppmValue out of range
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*   GT_BAD_STATE                - PPM value calculation wrong
*
* COMMENTS: 
*   In Lion2 granularity of register is 30.5ppm, CPSS will translate ppmValue
*   to most close value to provided by application.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesPpmSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                ppmValue
);

/*******************************************************************************
* cpssDxChPortSerdesPpmGet
*
* DESCRIPTION:
*       Get Tx clock increase/decrease status on port (added/sub ppm).
*       Can be run only after port configured.
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
* 
* INPUTS:
*       devNum      - system device number
*       portNum     - number of physical port all serdeses occupied by it, in 
*                       currently configured interface mode will be configured
*                       to required PPM
*
* OUTPUTS:
*       ppmValuePtr - PPM value currently used by given port.
*                     0 - means PPM disabled.
*                     (APPLICABLE RANGES: -100..100) PPM accoringly to protocol
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_BAD_PTR                  - NULL pointer
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*   In Lion2 granularity of register is 30.5ppm.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesPpmGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_32                *ppmValuePtr
);

/*******************************************************************************
* cpssDxChPortSerdesManualTxConfigSet
*
* DESCRIPTION:
*   Configure specific parameters of serdes TX in HW.
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*   laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*   serdesTxCfgPtr  - serdes Tx parameters
*
* OUTPUTS:
*   None.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*   GT_OUT_OF_RANGE             - parameter out of range
*
* COMMENTS: 
*   Pay attention: every serdes power up restore CPSS default configurations,
*   so this API should be egaged by application after every serdes power up
*   or use cpssDxChPortSerdesTuningSet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
);

/*******************************************************************************
* cpssDxChPortSerdesManualTxConfigGet
*
* DESCRIPTION:
*   Read specific parameters of serdes TX.
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*   laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* OUTPUTS:
*   serdesTxCfgPtr  - serdes Tx parameters
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_BAD_PTR                  - serdesTxCfgPtr == NULL
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
);


/*******************************************************************************
* cpssDxChPortSerdesManualRxConfigSet
*
* DESCRIPTION:
*   Configure specific parameters of serdes RX in HW.
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*   laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*   serdesRxCfgPtr  - serdes Rx parameters
*
* OUTPUTS:
*   None.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*   GT_OUT_OF_RANGE             - parameter out of range
*
* COMMENTS: 
*   Pay attention: every serdes power up restore CPSS default configurations,
*   so this API should be egaged by application after every serdes power up
*   or use cpssDxChPortSerdesTuningSet.
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesManualRxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
);

/*******************************************************************************
* cpssDxChPortSerdesManualRxConfigGet
*
* DESCRIPTION:
*   Read specific parameters of serdes RX.
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*   laneNum - number of SERDES lane of port (0-> lane 0,...,3 -> lane 3 etc.)
*
* OUTPUTS:
*   serdesRxCfgPtr  - serdes Rx parameters
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_BAD_PTR                  - serdesTxCfgPtr == NULL
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
);


/******************************************************************************
* cpssDxChPortFecModeSet
*
* DESCRIPTION:
*       Configure Forward error correction (FEC) mode of port
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       mode      - Forward error correction mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_NOT_INITIALIZED    - if port not configured
*     GT_NOT_SUPPORTED - if FEC not supported on interface mode defined now on port
*     GT_FAIL          - on error
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
******************************************************************************/
GT_STATUS cpssDxChPortFecModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT mode
);

/******************************************************************************
* cpssDxChPortFecModeGet
*
* DESCRIPTION:
*       Read current Forward error correction (FEC) mode of port
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*
* OUTPUTS:
*       modePtr   - current Forward error correction mode
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_NOT_INITIALIZED    - if port not configured
*     GT_BAD_PTR       - modePtr == NULL
*     GT_NOT_SUPPORTED - if FEC not supported on interface mode defined now on port
*     GT_FAIL          - on error
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
******************************************************************************/
GT_STATUS cpssDxChPortFecModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FEC_MODE_ENT *modePtr
);

/******************************************************************************
* cpssDxChPortPcsLoopbackModeSet
*
* DESCRIPTION:
*       Configure loopback of specific type on PCS.
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       mode      - define loopback type or no loopback
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
******************************************************************************/
GT_STATUS cpssDxChPortPcsLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode
);

/******************************************************************************
* cpssDxChPortPcsLoopbackModeGet
*
* DESCRIPTION:
*       Configure loopback of specific type on PCS.
*
* APPLICABLE DEVICES:
*        Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*
* OUTPUTS:
*       modePtr - current loopback type or no loopback
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_BAD_PTR       - modePtr == NULL
*     GT_FAIL          - on error
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
******************************************************************************/
GT_STATUS cpssDxChPortPcsLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    *modePtr
);


/*******************************************************************************
* cpssDxChPortsFastInit
*
* DESCRIPTION:
*       Fast ports configuration (interface, speed, serdes power up) during
*       system initialization.
*
* APPLICABLE DEVICES:
*        xCat; Lion; xCat2; Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum    - physical device number
*       portsBmp  - physical port number (not CPU port)
*       ifMode    - interface mode
*       speed     - port data speed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_BAD_PARAM      - on wrong port number or device
*       GT_HW_ERROR       - on hardware error
*       GT_NOT_SUPPORTED  - on not supported interface for given port
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       Same like cpssDxChPortModeSpeedSet(powerUp=true) but faster.
*       Purposed for use on system init, so assumes that ports are in HW default
*       i.e. all port's units in reset and serdes in power down.
* 
*******************************************************************************/
GT_STATUS cpssDxChPortsFastInit
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/*******************************************************************************
* cpssDxChPortSerdesSignalDetectGet
*
* DESCRIPTION:
*   Return Signal Detect state on SerDes (true/false).
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*
* OUTPUTS:
*   signalStatePtr - signal state on serdes:
*                       GT_TRUE  - signal detected;
*                       GT_FALSE - no signal.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_BAD_PTR                  - signalStatePtr == NULL
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesSignalDetectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *signalStatePtr
);

/*******************************************************************************
* cpssDxChPortSerdesCDRLockStatusGet
*
* DESCRIPTION:
*   Return SERDES CDR lock status (true - locked /false - not locked).
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*
* OUTPUTS:
*   cdrLockPtr - CRD lock state on serdes:
*                       GT_TRUE  - CDR locked;
*                       GT_FALSE - CDR not locked.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_BAD_PTR                  - cdrLockPtr == NULL
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*   GT_NOT_INITIALIZED          - if serdes was not intialized
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortSerdesCDRLockStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *cdrLockPtr
);

/*******************************************************************************
* cpssDxChPortPcsGearBoxStatusGet
*
* DESCRIPTION:
*   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* APPLICABLE DEVICES:
*   Lion2; Lion3.
*
* NOT APPLICABLE DEVICES:
*   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*   devNum - physical device number
*   portNum - physical port number
*
* OUTPUTS:
*   gbLockPtr - Gear Box lock state on serdes:
*                       GT_TRUE  - locked;
*                       GT_FALSE - not locked.
*
* RETURNS:
*   GT_OK                       - on success.
*   GT_BAD_PARAM                - on bad parameters
*   GT_BAD_PTR                  - gbLockPtr == NULL
*   GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS cpssDxChPortPcsGearBoxStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *gbLockPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortCtrlh */

/*
 *------------------------------------------------------------------
 * $Log: cpssDxChPortCtrl.h,v $
 * Revision 1.1  2015/02/13 11:32:31  iachang
 * Check in Marvell GE switch driver header file.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
