/* $Id: bcm_common_defines.h,v 1.2 2019/08/06 06:56:11 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/bcm_common_defines.h,v $
 *-----------------------------------------------------------------------------
 * bcm_common_defines.h - Leverage from BCM API
 * Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/bcm_quadra28_app/bcm_common_defines.h
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef BCM_COMMON_DEFINES_H
#define BCM_COMMON_DEFINES_H

#define BCM_NUM_DFE_TAPS 5
typedef enum bcm_pm_interface_e {
    bcm_pm_InterfaceBypass = 0,
    bcm_pm_InterfaceSR,
    bcm_pm_InterfaceSR4,
    bcm_pm_InterfaceKX,
    bcm_pm_InterfaceKX4,
    bcm_pm_InterfaceKR,
    bcm_pm_InterfaceKR2,
    bcm_pm_InterfaceKR4,
    bcm_pm_InterfaceCX,
    bcm_pm_InterfaceCX2,
    bcm_pm_InterfaceCX4,
    bcm_pm_InterfaceCR,
    bcm_pm_InterfaceCR2,
    bcm_pm_InterfaceCR4,
    bcm_pm_InterfaceCR10,
    bcm_pm_InterfaceXFI,
    bcm_pm_InterfaceSFI,
    bcm_pm_InterfaceSFPDAC,
    bcm_pm_InterfaceXGMII,
    bcm_pm_Interface1000X,
    bcm_pm_InterfaceSGMII,
    bcm_pm_InterfaceXAUI,
    bcm_pm_InterfaceRXAUI,
    bcm_pm_InterfaceX2,
    bcm_pm_InterfaceXLAUI,
    bcm_pm_InterfaceXLAUI2,
    bcm_pm_InterfaceCAUI,
    bcm_pm_InterfaceQSGMII,
    bcm_pm_InterfaceLR4,
    bcm_pm_InterfaceLR,
    bcm_pm_InterfaceLR2,
    bcm_pm_InterfaceER,
    bcm_pm_InterfaceER2,
    bcm_pm_InterfaceER4,
    bcm_pm_InterfaceSR2,
    bcm_pm_InterfaceSR10,
    bcm_pm_InterfaceCAUI4,
    bcm_pm_InterfaceVSR,
    bcm_pm_InterfaceLR10,
    bcm_pm_InterfaceKR10,
    bcm_pm_InterfaceCAUI4_C2C,
    bcm_pm_InterfaceCAUI4_C2M,
    bcm_pm_InterfaceZR,
    bcm_pm_InterfaceLRM,
    bcm_pm_InterfaceXLPPI,
    bcm_pm_InterfaceCount
} bcm_pm_interface_t;
typedef bcm_pm_interface_t bcm_plp_pm_interface_t;

typedef enum bcm_pm_ref_clk_e {
    bcm_pm_RefClk156Mhz = 0, /**< 156.25MHz */
    bcm_pm_RefClk125Mhz, /**< 125Mhz */
    bcm_pm_RefClk106Mhz, /**< 106.25Mhz */
    bcm_pm_RefClk161Mhz, /**< 161.1328125Mhz */
    bcm_pm_RefClk174Mhz, /**< 174.703125Mhz */
    bcm_pm_RefClk312Mhz, /**< 312Mhz */
    bcm_pm_RefClk322Mhz, /**< 322Mhz */
    bcm_pm_RefClk349Mhz, /**< 349Mhz */
    bcm_pm_RefClk644Mhz, /**< 644Mhz */
    bcm_pm_RefClk698Mhz, /**< 698Mhz */
    bcm_pm_RefClk155Mhz, /**< 155Mhz */
    bcm_pm_RefClk156P6Mhz, /**< 156P6Mhz */
    bcm_pm_RefClk157Mhz, /**< 157Mhz */
    bcm_pm_RefClk158Mhz, /**< 158Mhz */
    bcm_pm_RefClk159Mhz, /**< 159Mhz */
    bcm_pm_RefClk168Mhz, /**< 168Mhz */
    bcm_pm_RefClk172Mhz, /**< 172Mhz */
    bcm_pm_RefClk173Mhz, /**< 173Mhz */
    bcm_pm_RefClkCount
} bcm_pm_ref_clk_t;
typedef bcm_pm_ref_clk_t bcm_plp_pm_ref_clk_t;

typedef enum bcm_pm_interface_mode_e {
    bcm_pm_Interface_mode_IEEE = 0,
    bcm_pm_Interface_mode_HIGIG,
    bcm_pm_Interface_mode_OTN
} bcm_pm_interface_mode_t;
typedef bcm_pm_interface_mode_t bcm_plp_pm_interface_mode_t;

typedef enum bcm_pm_prbs_poly_e {
    bcm_pm_PrbsPoly7 = 0,
    bcm_pm_PrbsPoly9,
    bcm_pm_PrbsPoly11,
    bcm_pm_PrbsPoly15,
    bcm_pm_PrbsPoly23,
    bcm_pm_PrbsPoly31,
    bcm_pm_PrbsPoly58,
    bcm_pm_PrbsPolyCount
} bcm_pm_prbs_poly_t;
typedef bcm_pm_prbs_poly_t bcm_plp_pm_prbs_poly_t;
typedef enum bcm_pm_osr_mode_e {
    bcmpmOversampleMode1 = 0,
    bcmpmOversampleMode2,
    bcmpmOversampleMode3,
    bcmpmOversampleMode3P3,
    bcmpmOversampleMode4,
    bcmpmOversampleMode5,
    bcmpmOversampleMode8,
    bcmpmOversampleMode8P25,
    bcmpmOversampleMode10,
    bcmpmOversampleModeCount
} bcm_pm_osr_mode_t;
typedef bcm_pm_osr_mode_t bcm_plp_pm_osr_mode_t;
typedef enum bcm_pm_pmd_mode_e {
    bcmpmPmdModeOs = 0,
    bcmpmPmdModeOsDfe,
    bcmpmPmdModeBrDfe,
    bcmpmPmdModeCount
} bcm_pm_pmd_mode_t;
typedef bcm_pm_pmd_mode_t bcm_plp_pm_pmd_mode_t;
typedef enum bcm_pm_sequencer_operation_e {
    bcmpmSeqOpStop = 0, /**< Stop Sequencer */
    bcmpmSeqOpStart, /**< Start Sequencer */
    bcmpmSeqOpRestart, /**< Toggle Sequencer */
    bcmpmSeqOpCount
} bcm_pm_sequencer_operation_t;
typedef bcm_pm_sequencer_operation_t bcm_plp_pm_sequencer_operation_t;
typedef enum bcm_pm_phy_tx_lane_control_e {
    bcmpmTxTrafficDisable = 0, /**< disable tx traffic */
    bcmpmTxTrafficEnable, /**< enable tx traffic */
    bcmpmTxReset, /**< reset tx data path */
    bcmpmTxSquelchOn, /**< squelch tx */
    bcmpmTxSquelchOff, /**< squelch tx off */
    bcmpmTxCount
} bcm_pm_phy_tx_lane_control_t;
typedef bcm_pm_phy_tx_lane_control_t bcm_plp_pm_phy_tx_lane_control_t;

typedef enum bcm_pm_phy_rx_lane_control_e {
    bcmpmRxReset, /**< reset rx data path */
    bcmpmRxSquelchOn, /**< squelch rx */
    bcmpmRxSquelchOff, /**< squelch rx off */
    bcmpmRxCount
} bcm_pm_phy_rx_lane_control_t;
typedef bcm_pm_phy_rx_lane_control_t bcm_plp_pm_phy_rx_lane_control_t;
typedef enum bcm_pm_firmware_broadcast_method_e {
	bcmpmFirmwareBroadcastNone=0,                 /*    Firmware downloaded as unicast for each phy device in mdio bus\n*/
    bcmpmFirmwareBroadcastCoreReset,             /* Reset the core for all phy id in mdio bus */
    bcmpmFirmwareBroadcastEnable,                    /* Enable the broadcast for all phy id in mdio bus */
    bcmpmFirmwareBroadcastFirmwareExecute,     /* Load the FW for only one phy_id of similar type of phys in mdio bus */
    bcmpmFirmwareBroadcastFirmwareVerify,   /*  FW load verify for all phy id in mdio bus */
    bcmpmFirmwareBroadcastEnd,                            /* Disable the broadcast for all phy id in mdio bus */
    bcmpmFirmwareBroadcastCount
} bcm_pm_firmware_broadcast_method_t;
typedef bcm_pm_firmware_broadcast_method_t bcm_plp_firmware_broadcast_method_t;
typedef enum bcm_pm_firmware_load_method_e {
    bcmpmFirmwareLoadMethodNone = 0, /*Don't load FW*/
    bcmpmFirmwareLoadMethodInternal, /*Load FW internaly*/
    bcmpmFirmwareLoadMethodExternal, /*Load FW by a given function*/
    bcmpmFirmwareLoadMethodProgEEPROM, /*Load FW and flash it on to EEPROM */
    bcmpmFirmwareLoadMethodAuto,     /* Auto download in case of firmware change */
    bcmpmFirmwareLoadMethodCount
} bcm_pm_firmware_load_method_t;
typedef bcm_pm_firmware_load_method_t bcm_plp_pm_firmware_load_method_t;
/*!
 * @enum bcm_pm_firmware_load_force_e
 * @brief Firmware load force 
 */ 
typedef enum bcm_pm_firmware_load_force_e {
    bcmpmFirmwareLoadSkip = 0, /**< Skip load FW */
    bcmpmFirmwareLoadForce, /**< Force load FW */
    bcmpmFirmwareLoadAuto, /**< Auto load FW in case of firmware change */
    bcmpmFirmwareLoadCount
} bcm_pm_firmware_load_force_t;
typedef bcm_pm_firmware_load_force_t bcm_plp_firmware_load_force_t;
typedef enum bcm_pm_reset_direction_e {
    bcmpmResetDirectionIn = 0, /**< In Reset */
    bcmpmResetDirectionOut, /**< Out of Reset */
    bcmpmResetDirectionInOut, /**< Toggle Reset */
    bcmpmResetDirectionCount
} bcm_pm_reset_direction_t;
typedef bcm_pm_reset_direction_t bcm_plp_pm_reset_direction_t;
typedef enum bcm_pm_firmware_mode_e {
    bcm_pm_fw_default = 0,
    bcm_pm_fw_dfe,
    bcm_pm_fw_osdfe,
    bcm_pm_fw_br_dfe,
    bcm_pm_fw_lp_dfe,
    bcm_pm_fw_sfp_dac,
    bcm_pm_fw_xlaui,
    bcm_pm_fw_sfp_opt_sr4,
    bcm_pm_firmware_mode_Count
} bcm_pm_firmware_mode_t;
typedef bcm_pm_firmware_mode_t bcm_plp_pm_firmware_mode_t;
/*!
 * @enum bcm_pm_failover_mode_e
 * @brief Failover configuration 
 */ 
 typedef enum bcm_pm_failover_mode_e {
    bcmpmFailovermodeNone,
    bcmpmFailovermodeEnable, /**< enable Failover mode */
    bcmpmFailovermodeCount
} bcm_pm_failover_mode_t;
typedef bcm_pm_failover_mode_t bcm_plp_failover_mode_t;
/*!
 * @enum bcm_edc_config_method_e
 * @brief Configuration method for Electronic Dispersion Compensation (EDC) 
 */ 
typedef enum bcm_edc_config_method_e {
    bcmEdcConfigMethodNone,
    bcmEdcConfigMethodHardware, /**< EDC mode is set automatically by hardware */
    bcmEdcConfigMethodSoftware, /**< EDC mode is selected by driver software */
    bcmEdcConfigMethodCount
} bcm_edc_config_method_t;
typedef bcm_edc_config_method_t bcm_plp_edc_config_method_t;

typedef enum bcm_mac_lb_type_e {
    bcmplpMacOuterloopback = 0,
    bcmplpMaxloopbackCnt
}bcm_mac_lb_type_t;

typedef enum bcm_mac_flow_control_e {
    bcmplpFlowcontrolTerminateGenerate = 0,
    bcmplpFlowcontrolPassthrough
} bcm_mac_flow_control_t;

typedef enum bcm_mac_fault_option_e {
    bcmplpFaultoptionTerminateGenerate = 0,
    bcmplpFaultoptionPassthrough
} bcm_mac_fault_option_t;

#endif /*<BCM_COMMON_DEFINES_H>*/

