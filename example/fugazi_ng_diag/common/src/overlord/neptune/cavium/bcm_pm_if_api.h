/* $Id: bcm_pm_if_api.h,v 1.2 2018/05/18 09:24:56 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/cavium/bcm_pm_if_api.h,v $
 *-----------------------------------------------------------------------------
 * bcm_pm_if_api.h - Leverage from BCM API
 * Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/bcm_quadra28_app/quadra28/bcm_pm_if_api.h
 *
 * August 2016, meho
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef plp_quadra28_BCM_PM_IF_API_H
#define plp_quadra28_BCM_PM_IF_API_H
#include "bcm_common_defines.h"

#define BCM_LINE_SIDE 0

#define BCM_LEGACY_PHY_SUPPORT
typedef struct bcm_plp_quadra28_value_override_s {
    unsigned int enable;
    unsigned int value;
} bcm_plp_quadra28_value_override_t;

typedef struct bcm_plp_quadra28_rx_s {
    bcm_plp_quadra28_value_override_t vga;
    unsigned int num_of_dfe_taps; /*number of elements in DFE array*/
    bcm_plp_quadra28_value_override_t dfe[BCM_NUM_DFE_TAPS];
    bcm_plp_quadra28_value_override_t peaking_filter;
    bcm_plp_quadra28_value_override_t low_freq_peaking_filter;
} bcm_plp_quadra28_rx_t;

typedef struct bcm_plp_quadra28_tx_s {
    char pre;
    char main;
    char post;
    char post2;
    char post3;
    char amp;
}bcm_plp_quadra28_tx_t;

typedef struct bcm_plp_quadra28_pm_diag_slicer_offset_s {
    unsigned int offset_pe;
    unsigned int offset_ze;
    unsigned int offset_me;
    unsigned int offset_po;
    unsigned int offset_zo;
    unsigned int offset_mo;
} bcm_plp_quadra28_pm_diag_slicer_offset_t;

typedef struct bcm_plp_quadra28_pm_diag_eyescan_s {
    unsigned int heye_left;
    unsigned int heye_right;
    unsigned int veye_upper;
    unsigned int veye_lower;
} bcm_plp_quadra28_pm_diag_eyescan_t;

typedef struct bcm_plp_quadra28_pm_phy_diagnostics_s {
    unsigned int signal_detect;
    unsigned int vga_bias_reduced;
    unsigned int postc_metric;
    bcm_pm_osr_mode_t osr_mode;
    bcm_pm_pmd_mode_t pmd_mode;
    unsigned int rx_lock;
    unsigned int rx_ppm;
    unsigned int tx_ppm;
    unsigned int clk90_offset;
    unsigned int clkp1_offset;
    unsigned int p1_lvl;
    unsigned int m1_lvl;
    unsigned int dfe1_dcd;
    unsigned int dfe2_dcd;
    unsigned int slicer_target;
    bcm_plp_quadra28_pm_diag_slicer_offset_t slicer_offset;
    bcm_plp_quadra28_pm_diag_eyescan_t eyescan;
    unsigned int state_machine_status;
    unsigned int link_time; /* Added as required by Falcon core */
    char pf_main;
    char pf_hiz;
    char pf_bst;
    char pf_low;
    char pf2_ctrl;
    char vga;
    char dc_offset;
    char p1_lvl_ctrl;
    char dfe1;
    char dfe2;
    char dfe3;
    char dfe4;
    char dfe5;
    char dfe6;
    char txfir_pre;
    char txfir_main;
    char txfir_post1;
    char txfir_post2;
    char txfir_post3;
    char tx_amp_ctrl;
    unsigned char br_pd_en; 
} bcm_plp_quadra28_pm_phy_diagnostics_t;
typedef struct bcm_plp_quadra28_pm_phy_reset_s {
    bcm_pm_reset_direction_t rx;
    bcm_pm_reset_direction_t tx;
} bcm_plp_quadra28_pm_phy_reset_t;

typedef struct plp_quadra28_static_config_s {
    unsigned int rptr_mode;
    unsigned int ull_dp;
    unsigned int an_master_lane;
    unsigned int an_mst_lane_p0;
    unsigned int an_mst_lane_p1;
    unsigned int avdd_txdrv;
} plp_quadra28_static_config_t;

typedef struct bcm_plp_quadra28_phy_static_config_s {
    unsigned int phy_id;
    void* bcm_static_config;
} bcm_plp_quadra28_phy_static_config_t;

typedef struct bcm_plp_quadra28_modctrl_pin_io_value_s {
    unsigned int enable;
    unsigned int value;
} bcm_plp_quadra28_modctrl_pin_io_value_t;

typedef struct bcm_plp_quadra28_modctrl_cfp_io_pins_s {
    bcm_plp_quadra28_modctrl_pin_io_value_t tx_dis;
    bcm_plp_quadra28_modctrl_pin_io_value_t rx_los;
    bcm_plp_quadra28_modctrl_pin_io_value_t mod_lopwr;
    bcm_plp_quadra28_modctrl_pin_io_value_t mod_abs;
    bcm_plp_quadra28_modctrl_pin_io_value_t glb_alrmn;
    bcm_plp_quadra28_modctrl_pin_io_value_t mod_rstn;
} bcm_plp_quadra28_modctrl_cfp_io_pins_t;

typedef struct bcm_plp_quadra28_modctrl_qsfp_io_pins_s {
    bcm_plp_quadra28_modctrl_pin_io_value_t lpmod;
    bcm_plp_quadra28_modctrl_pin_io_value_t resetl;
    bcm_plp_quadra28_modctrl_pin_io_value_t intl;
    bcm_plp_quadra28_modctrl_pin_io_value_t mod_sell;
    bcm_plp_quadra28_modctrl_pin_io_value_t mod_prsl;
} bcm_plp_quadra28_modctrl_qsfp_io_pins_t;

typedef struct bcm_plp_quadra28_device_aux_modes_s {
    unsigned int pass_thru;
} bcm_plp_quadra28_device_aux_modes_t;
typedef struct bcm_plp_quadra28_pm_firmware_lane_config_s {
    bcm_pm_firmware_mode_t firmware_mode;
    unsigned int ena_dis;
} bcm_plp_quadra28_pm_firmware_lane_config_t;

typedef struct bcm_plp_quadra28_firmware_load_type_s{
	bcm_pm_firmware_load_method_t firmware_load_method;
	bcm_pm_firmware_load_force_t  force_load_method;
}bcm_plp_quadra28_firmware_load_type_t;


/*! PHY INFO  
 *
 * \arg void *platform_ctxt \n
 *             Represents user data, platform_ctxt is passed to\n
 *	           register read/write API. It can be NULL if not used in read/write register
 *
 * \arg unsigned int phy_addr \n
 *             Represents Phy-id\n
 *
 * \arg unsigned int if_side \n
 *             Represents the interface side \n 
 *                    0 - Line side of the PHY device\n
 *                    1 - system side of the PHY device\n
 *
 * \arg unsigned int lane_map \n
 *             Represents the Lane mapping of a port,\n
 *             LSB Bit 0 represents lane 0 of the specified PHY-ID.\n
 *             LSB Bit 1 represents lane 1 of the specified PHY-ID\n
 *             Similarly for lane 2 to lane N.\n
 *             where N is maximum number of lanes on a PHY. 
 *             It also support multicast.\n
 *             Eg: 
 *                   0x3 represent lane 0 and 1 \n
 *                   0xF represent lane 0 to lane 3
 */
typedef struct bcm_plp_quadra28_access_s {
    void *platform_ctxt; 
    unsigned int phy_addr;
    unsigned int if_side;
    unsigned int lane_map;
}bcm_plp_quadra28_access_t;
#ifdef BCM_LEGACY_PHY_SUPPORT
/* Macro for BCM_PM API's to support existing users*/
#define bcm_pm_if_static_config_set(P_C, P_ID, bcm_static_config)                                                  bcm_plp_quadra28_static_config_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, bcm_static_config)

#define bcm_pm_if_static_config_get(P_C, P_ID, bcm_static_config)                                                  bcm_plp_quadra28_static_config_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, bcm_static_config)

#define bcm_pm_if_init(P_C, P_ID, READ, WRITE, FW_L_M)                                                             bcm_plp_quadra28_init((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, READ, WRITE, FW_L_M) 

#define bcm_pm_if_cleanup(P_ID)                                                                                    bcm_plp_quadra28_cleanup((bcm_plp_quadra28_access_t){0, P_ID, 0xFF, 0xFF})

#define bcm_pm_if_link_status_get(P_C, P_ID, I_S, L_M, link_status)                                                bcm_plp_quadra28_link_status_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, link_status)

#define bcm_pm_if_mode_config_set(P_C, P_ID, I_S, L_M, speed, if_type, ref_clk, interface_mode, device_aux_modes)  bcm_plp_quadra28_mode_config_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, speed, if_type, ref_clk, interface_mode, device_aux_modes)

#define bcm_pm_if_mode_config_get(P_C, P_ID, I_S, L_M, speed, if_type, ref_clk, interface_mode, device_aux_modes)  bcm_plp_quadra28_mode_config_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, speed, if_type, ref_clk, interface_mode, device_aux_modes)
#define bcm_pm_if_version_get(chip_ver, api_ver, enahan_ver)                                                       bcm_plp_quadra28_version_get(chip_ver, api_ver, enahan_ver)
#define bcm_pm_if_prbs_set(P_C, P_ID, I_S, L_M, tx_rx,  poly,  invert,  loopback,  ena_dis)                        bcm_plp_quadra28_prbs_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_rx,  poly,  invert,  loopback,  ena_dis)
#define bcm_pm_if_prbs_get(P_C, P_ID, I_S, L_M, tx_rx, poly, invert, loopback, ena_dis)                            bcm_plp_quadra28_prbs_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_rx, poly, invert, loopback, ena_dis)
#define bcm_pm_if_prbs_rx_stat(P_C, P_ID, I_S, L_M,  time)                                                         bcm_plp_quadra28_prbs_rx_stat((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M},  time)
#define bcm_pm_if_prbs_clear(P_C, P_ID, I_S, L_M,  tx_rx)                                                          bcm_plp_quadra28_prbs_clear((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M},  tx_rx)
#define bcm_pm_if_prbs_config_get(P_C, P_ID, I_S, L_M,  tx_rx, poly, invert)                                       bcm_plp_quadra28_prbs_config_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_rx, poly, invert)
#define bcm_pm_if_prbs_status_get(P_C, P_ID, I_S, L_M, prbs_lock, prbs_lock_loss, error_count)                     bcm_plp_quadra28_prbs_status_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, prbs_lock, prbs_lock_loss, error_count)  
#define bcm_pm_if_reg_value_set(P_C, P_ID, devaddr, regaddr,  data)                                                bcm_plp_quadra28_reg_value_set((bcm_plp_quadra28_access_t){P_C, P_ID,0xFF,0xFF}, devaddr, regaddr,  data)
#define bcm_pm_if_reg_value_get(P_C, P_ID, devaddr, regaddr, data)                                                 bcm_plp_quadra28_reg_value_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, devaddr, regaddr, data)
#define bcm_pm_if_polarity_set(P_C, P_ID, I_S, L_M, tx_pol,  rx_pol)                                               bcm_plp_quadra28_polarity_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_pol,  rx_pol)
#define bcm_pm_if_polarity_get(P_C, P_ID, I_S, L_M, tx_pol, rx_pol)                                                bcm_plp_quadra28_polarity_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_pol, rx_pol)
#define bcm_pm_if_power_set(P_C, P_ID, I_S, L_M, power_rx, power_tx)                                               bcm_plp_quadra28_power_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, power_rx, power_tx)
#define bcm_pm_if_power_get(P_C, P_ID, I_S, L_M, power_rx, power_tx)                                               bcm_plp_quadra28_power_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, power_rx, power_tx)
#define bcm_pm_if_rx_pmd_locked_get(P_C, P_ID, I_S, L_M, rx_pmd_lock)                                              bcm_plp_quadra28_rx_pmd_lock_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rx_pmd_lock)
#define bcm_pm_if_rev_id(P_C, P_ID, rev_id)                                                                       bcm_plp_quadra28_rev_id((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, rev_id)
#define bcm_pm_if_loopback_set(P_C, P_ID, I_S, L_M, lb_mode, enable)                                               bcm_plp_quadra28_loopback_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, lb_mode, enable) 
#define bcm_pm_if_loopback_get(P_C, P_ID, I_S, L_M, lb_mode, enable)                                               bcm_plp_quadra28_loopback_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, lb_mode, enable)
#define bcm_pm_if_tx_set(P_C, P_ID, I_S, L_M, tx)                                                                  bcm_plp_quadra28_tx_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx) 
#define bcm_pm_if_tx_get(P_C, P_ID, I_S, L_M, tx)                                                                  bcm_plp_quadra28_tx_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx)
#define bcm_pm_if_rx_set(P_C, P_ID, I_S, L_M, rx)                                                                  bcm_plp_quadra28_rx_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rx)
#define bcm_pm_if_rx_get(P_C, P_ID, I_S, L_M, rx)                                                                  bcm_plp_quadra28_rx_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rx)
#define bcm_pm_if_reset_set(P_C, P_ID, reset_mode, reset_val)                                                      bcm_plp_quadra28_reset_set((bcm_plp_quadra28_access_t){P_C, P_ID,0xFF,0xFF}, reset_mode, reset_val)
#define bcm_pm_if_phy_lane_reset_set(P_C, P_ID, I_S, L_M, reset)                                                   bcm_plp_quadra28_phy_lane_reset_set ((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, reset)
#define bcm_pm_if_phy_lane_reset_get(P_C, P_ID, I_S, L_M, reset)                                                   bcm_plp_quadra28_phy_lane_reset_get ((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, reset)
#define bcm_pm_if_tx_lane_control_set(P_C, P_ID, I_S, L_M, tx_control)                                             bcm_plp_quadra28_tx_lane_control_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_control) 
#define bcm_pm_if_rx_lane_control_set(P_C, P_ID, I_S, L_M, rx_control)                                             bcm_plp_quadra28_rx_lane_control_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rx_control) 
#define bcm_pm_if_tx_lane_control_get(P_C, P_ID, I_S, L_M, tx_control)                                             bcm_plp_quadra28_tx_lane_control_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, tx_control) 
#define bcm_pm_if_rx_lane_control_get(P_C, P_ID, I_S, L_M, rx_control)                                             bcm_plp_quadra28_rx_lane_control_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rx_control) 
#define bcm_pm_if_lane_cross_switch_map_set(P_C, P_ID, I_S, tx_source_array)                                       bcm_plp_quadra28_lane_cross_switch_map_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, 0xFF}, tx_source_array) 
#define bcm_pm_if_lane_cross_switch_map_get(P_C, P_ID, L_M,  mapped_to)                                            bcm_plp_quadra28_lane_cross_switch_map_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M},  mapped_to)
#define bcm_pm_if_force_tx_training_set(P_C, P_ID, I_S, L_M, enable)                                               bcm_plp_quadra28_force_tx_training_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, enable)
#define bcm_pm_if_force_tx_training_get(P_C, P_ID, I_S, L_M, enable)                                               bcm_plp_quadra28_force_tx_training_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, enable)
#define bcm_pm_if_force_tx_training_status_get(P_C, P_ID, I_S, L_M, enabled, training_failure, trained)            bcm_plp_quadra28_force_tx_training_status_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, enabled, training_failure, trained) 
#define bcm_pm_if_cl73_ability_set(P_C, P_ID, L_M, tech_ability, fec_ability, pause_ability)                       bcm_plp_quadra28_cl73_ability_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, tech_ability, fec_ability, pause_ability)
#define bcm_pm_if_cl73_ability_get(P_C, P_ID, L_M, tech_ability, fec_ability, pause_ability)                       bcm_plp_quadra28_cl73_ability_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, tech_ability, fec_ability, pause_ability)
#define bcm_pm_if_cl73_set(P_C, P_ID, L_M, ena_dis)                                                                bcm_plp_quadra28_cl73_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, ena_dis)
#define bcm_pm_if_cl73_get(P_C, P_ID, L_M, an, an_done)                                                            bcm_plp_quadra28_cl73_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, an, an_done) 
#define bcm_pm_if_display_eye_scan(P_C, P_ID, I_S, L_M)                                                            bcm_plp_quadra28_display_eye_scan((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M})
#define bcm_pm_if_firmware_info_get(P_C, P_ID, fw_version, fw_crc)                                                  bcm_plp_quadra28_firmware_info_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, fw_version, fw_crc)
#define bcm_pm_if_pll_sequencer_restart(P_C, P_ID, I_S, flags, operation)                                          bcm_plp_quadra28_pll_sequencer_restart((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, 0xFF}, flags, operation)
#define bcm_pm_if_fec_enable_set(P_C, P_ID, enable)                                                                bcm_plp_quadra28_fec_enable_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, enable) 
#define bcm_pm_if_fec_enable_get(P_C, P_ID, enable)                                                                bcm_plp_quadra28_fec_enable_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF}, enable)
#define bcm_pm_if_phy_status_dump(P_C, P_ID, I_S, L_M)                                                             bcm_plp_quadra28_phy_status_dump((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M})
#define bcm_pm_if_phy_diagnostics_get(P_C, P_ID, I_S, L_M, diag)                                                   bcm_plp_quadra28_phy_diagnostics_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, diag)
#define bcm_pm_if_intr_status_get(P_C, P_ID, intr_type, intr_status)                                               bcm_plp_quadra28_intr_status_get((bcm_plp_quadra28_access_t){P_C, P_ID,0xFF,0xFF}, intr_type, intr_status)
#define bcm_pm_if_intr_enable_set(P_C, P_ID, intr_type, enable)                                                    bcm_plp_quadra28_intr_enable_set((bcm_plp_quadra28_access_t){P_C, P_ID,0xFF,0xFF}, intr_type, enable)
#define bcm_pm_if_intr_enable_get(P_C, P_ID, intr_type, enable)                                                    bcm_plp_quadra28_intr_enable_get((bcm_plp_quadra28_access_t){P_C, P_ID,0xFF,0xFF}, intr_type, enable)
#define bcm_pm_if_intr_status_clear(P_C, P_ID,  intr_type)                                                         bcm_plp_quadra28_intr_status_clear((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, 0xFF},  intr_type) 
#define bcm_pm_if_fc_pcs_chkr_enable_set(P_C, P_ID, I_S, L_M, fcpcs_chkr_mode, enable)                                bcm_plp_quadra28_fc_pcs_chkr_enable_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, fcpcs_chkr_mode, enable) 
#define bcm_pm_if_fc_pcs_chkr_enable_get(P_C, P_ID, I_S, L_M, fcpcs_chkr_mode, enable)                                bcm_plp_quadra28_fc_pcs_chkr_enable_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, fcpcs_chkr_mode, enable)
#define bcm_pm_if_fc_pcs_chkr_status_get(P_C, P_ID, I_S, L_M, lock_status, lock_lost_lh, error_count)                 bcm_plp_quadra28_fc_pcs_chkr_status_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, lock_status, lock_lost_lh, error_count)
#ifdef SERDES_API_FLOATING_POINT 
#define bcm_pm_if_eye_margin_proj(P_C, P_ID, I_S, L_M, rate, ber_scan_mode, timer_control, max_error_control)      bcm_plp_quadra28_eye_margin_proj((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rate, ber_scan_mode, timer_control, max_error_control)  
#else                                                                                                                                                                                                                
#define bcm_pm_if_eye_margin_proj(P_C, P_ID, I_S,  L_M, rate, ber_scan_mode, timer_control, max_error_control)     bcm_plp_quadra28_eye_margin_proj((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, rate, ber_scan_mode, timer_control, max_error_control)
#endif                                                                                                                                                                                                                    
#define bcm_pm_if_repeater_mode_get(P_C, P_ID, L_M,  ena_dis)                                                         bcm_repeater_mode_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, ena_dis) 
#define bcm_pm_if_repeater_mode_set(P_C, P_ID, L_M,  ena_dis)                                                         bcm_plp_quadra28_repeater_mode_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, ena_dis)
#define bcm_pm_if_module_read(P_C, P_ID, L_M,  slv_addr,  start_addr,  no_of_bytes, read_data)                     bcm_plp_quadra28_module_read ((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, slv_addr, start_addr, no_of_bytes, read_data)
#define bcm_pm_if_module_write(P_C, P_ID, L_M,  slv_addr,  start_addr,  no_of_bytes, write_data)                   bcm_plp_quadra28_module_write((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, slv_addr, start_addr, no_of_bytes, write_data)
#define bcm_pm_if_modctrl_cfg_cfp_linecard_set(P_C, P_ID, L_M,  mdcrtl_pins)                                       bcm_modctrl_cfg_cfp_linecard_set ((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, mdcrtl_pins)
#define bcm_pm_if_modctrl_cfg_qsfp_linecard_set(P_C, P_ID, L_M, mdcrtl_pins)                                       bcm_modctrl_cfg_qsfp_linecard_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, mdcrtl_pins)
#define bcm_pm_if_modctrl_cfg_qsfp_linecard_get(P_C, P_ID, L_M, mdcrtl_pins)                                       bcm_modctrl_cfg_qsfp_linecard_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, mdcrtl_pins)
#define bcm_pm_if_modctrl_cfg_cfp_linecard_get(P_C, P_ID, L_M,  mdcrtl_pins)                                       bcm_modctrl_cfg_cfp_linecard_get ((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF, L_M}, mdcrtl_pins)
#define bcm_pm_if_cfg_gpio_pin_set(P_C, P_ID,  gpio_pin_number,  cfg_direction,  cfg_pull,  pin_value)             bcm_plp_quadra28_cfg_gpio_pin_set((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF,0xFF},  gpio_pin_number,  cfg_direction,  cfg_pull,  pin_value)
#define bcm_pm_if_cfg_gpio_pin_get(P_C, P_ID,  gpio_pin_number, cfg_direction, cfg_pull, pin_value)               bcm_plp_quadra28_cfg_gpio_pin_get((bcm_plp_quadra28_access_t){P_C, P_ID, 0xFF,0xFF},  gpio_pin_number, cfg_direction, cfg_pull, pin_value)
#define bcm_pm_if_firmware_lane_config_set(P_C, P_ID, I_S, L_M, firmware_lane_config)                                      bcm_plp_quadra28_firmware_lane_config_set((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, firmware_lane_config)
#define bcm_pm_if_firmware_lane_config_get(P_C, P_ID, I_S, L_M, firmware_lane_config)                                      bcm_plp_quadra28_firmware_lane_config_get((bcm_plp_quadra28_access_t){P_C, P_ID, I_S, L_M}, firmware_lane_config)
#endif
/* New API */
int bcm_plp_quadra28_static_config_set(bcm_plp_quadra28_access_t phy_info, void* bcm_static_config);

int bcm_plp_quadra28_static_config_get(bcm_plp_quadra28_access_t phy_info, void* bcm_static_config);
int bcm_plp_quadra28_init(bcm_plp_quadra28_access_t phy_info, int (*read)(void* user_acc,
            unsigned int core_addr, unsigned int reg_addr, unsigned int* val), 
            int (*write)(void* user_acc, unsigned int core_addr, unsigned int reg_addr,
             unsigned int val), bcm_pm_firmware_load_method_t firmware_load_method);

int bcm_plp_quadra28_cleanup(bcm_plp_quadra28_access_t phy_info);

int bcm_plp_quadra28_link_status_get(bcm_plp_quadra28_access_t phy_info, unsigned int *link_status);

int bcm_plp_quadra28_mode_config_set(bcm_plp_quadra28_access_t phy_info, int speed, int if_type,
                            int ref_clk, int interface_mode, void* device_aux_modes);

int bcm_plp_quadra28_mode_config_get(bcm_plp_quadra28_access_t phy_info, int *speed, 
                            int *if_type, int *ref_clk, int *interface_mode,
                            void *device_aux_modes);

void bcm_plp_quadra28_version_get(unsigned short *chip_ver, unsigned short *api_ver,
                         unsigned short *enahan_ver);

int bcm_plp_quadra28_prbs_set(bcm_plp_quadra28_access_t phy_info, unsigned int tx_rx, 
                     unsigned int poly, unsigned int invert,
                     unsigned int loopback, unsigned int ena_dis);

int bcm_plp_quadra28_prbs_get(bcm_plp_quadra28_access_t phy_info, unsigned int tx_rx,
                     unsigned int *poly, unsigned int *invert,
                     unsigned int *loopback, unsigned int *ena_dis);

int bcm_plp_quadra28_prbs_rx_stat(bcm_plp_quadra28_access_t phy_info, unsigned int time);

int bcm_plp_quadra28_prbs_clear(bcm_plp_quadra28_access_t phy_info, unsigned int tx_rx);

int bcm_plp_quadra28_prbs_config_get(bcm_plp_quadra28_access_t phy_info, unsigned int tx_rx,
                            unsigned int *poly, unsigned int *invert);

int bcm_plp_quadra28_prbs_status_get(bcm_plp_quadra28_access_t phy_info, unsigned int *prbs_lock,
                            unsigned int *prbs_lock_loss, unsigned int *error_count);  

int bcm_plp_quadra28_reg_value_set(bcm_plp_quadra28_access_t phy_info, unsigned int devaddr,
                          unsigned int regaddr, unsigned int data);

int bcm_plp_quadra28_reg_value_get(bcm_plp_quadra28_access_t phy_info, unsigned int devaddr,
                          unsigned int regaddr, unsigned int *data);

int bcm_plp_quadra28_polarity_set(bcm_plp_quadra28_access_t phy_info, unsigned int tx_pol,
                         unsigned int rx_pol);

int bcm_plp_quadra28_polarity_get(bcm_plp_quadra28_access_t phy_info, unsigned int *tx_pol,
                         unsigned int *rx_pol);

int bcm_plp_quadra28_rx_pmd_lock_get(bcm_plp_quadra28_access_t phy_info, unsigned int* rx_pmd_lock);

int bcm_plp_quadra28_rev_id(bcm_plp_quadra28_access_t phy_info, unsigned int* rev_id);


int bcm_plp_quadra28_loopback_set(bcm_plp_quadra28_access_t phy_info, unsigned int lb_mode,
                         unsigned int enable);

int bcm_plp_quadra28_loopback_get(bcm_plp_quadra28_access_t phy_info, unsigned int lb_mode,
                         unsigned int *enable);

int bcm_plp_quadra28_tx_set(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_tx_t* tx);

int bcm_plp_quadra28_tx_get(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_tx_t* tx);

int bcm_plp_quadra28_rx_set(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_rx_t* rx);

int bcm_plp_quadra28_rx_get(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_rx_t* rx);

int bcm_plp_quadra28_reset_set(bcm_plp_quadra28_access_t phy_info, unsigned int reset_mode,
                      unsigned int reset_val);

int bcm_plp_quadra28_phy_lane_reset_set(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_pm_phy_reset_t* reset);

int bcm_plp_quadra28_phy_lane_reset_get(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_pm_phy_reset_t* reset);

int bcm_plp_quadra28_tx_lane_control_set(bcm_plp_quadra28_access_t phy_info, bcm_pm_phy_tx_lane_control_t tx_control);

int bcm_plp_quadra28_rx_lane_control_set(bcm_plp_quadra28_access_t phy_info, bcm_pm_phy_rx_lane_control_t rx_control);

int bcm_plp_quadra28_tx_lane_control_get(bcm_plp_quadra28_access_t phy_info, bcm_pm_phy_tx_lane_control_t *tx_control);

int bcm_plp_quadra28_rx_lane_control_get(bcm_plp_quadra28_access_t phy_info, bcm_pm_phy_rx_lane_control_t *rx_control);

int bcm_plp_quadra28_lane_cross_switch_map_set(bcm_plp_quadra28_access_t phy_info, unsigned int* tx_source_array);

int bcm_plp_quadra28_lane_cross_switch_map_get(bcm_plp_quadra28_access_t phy_info, unsigned int *mapped_to);

int bcm_plp_quadra28_force_tx_training_set(bcm_plp_quadra28_access_t phy_info, unsigned int enable);

int bcm_plp_quadra28_force_tx_training_get(bcm_plp_quadra28_access_t phy_info, unsigned int *enable);

int bcm_plp_quadra28_force_tx_training_status_get(bcm_plp_quadra28_access_t phy_info, unsigned int *enabled,
                                         unsigned int *training_failure, unsigned int *trained);

int bcm_plp_quadra28_cl73_ability_set(bcm_plp_quadra28_access_t phy_info, unsigned short tech_ability,
                             unsigned short fec_ability, unsigned short pause_ability);

int bcm_plp_quadra28_cl73_ability_get(bcm_plp_quadra28_access_t phy_info, unsigned short *tech_ability,
                             unsigned short *fec_ability, unsigned short *pause_ability);
int bcm_plp_quadra28_cl73_set(bcm_plp_quadra28_access_t phy_info, unsigned short ena_dis);

int bcm_plp_quadra28_cl73_get(bcm_plp_quadra28_access_t phy_info, unsigned int *an,
                     unsigned int *an_done);

int bcm_plp_quadra28_display_eye_scan(bcm_plp_quadra28_access_t phy_info);

int bcm_plp_quadra28_firmware_info_get(bcm_plp_quadra28_access_t phy_info, unsigned int *fw_version,
                              unsigned int *fw_crc);

int bcm_plp_quadra28_pll_sequencer_restart(bcm_plp_quadra28_access_t phy_info, unsigned char flags,
                                  bcm_pm_sequencer_operation_t operation);

int bcm_plp_quadra28_fec_enable_set(bcm_plp_quadra28_access_t phy_info, unsigned int enable);

int bcm_plp_quadra28_fec_enable_get(bcm_plp_quadra28_access_t phy_info, unsigned int* enable);

int bcm_plp_quadra28_phy_status_dump(bcm_plp_quadra28_access_t phy_info);

int bcm_plp_quadra28_phy_diagnostics_get(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_pm_phy_diagnostics_t* diag);

int bcm_plp_quadra28_intr_status_get(bcm_plp_quadra28_access_t phy_info, unsigned int intr_type,
                            unsigned int* intr_status);

int bcm_plp_quadra28_intr_enable_set(bcm_plp_quadra28_access_t phy_info, unsigned int intr_type,
                            unsigned int enable);

int bcm_plp_quadra28_intr_enable_get(bcm_plp_quadra28_access_t phy_info, unsigned int intr_type,
                            unsigned int* enable);

int bcm_plp_quadra28_intr_status_clear(bcm_plp_quadra28_access_t phy_info, unsigned int intr_type);

int bcm_plp_quadra28_fc_pcs_chkr_enable_set(bcm_plp_quadra28_access_t phy_info, unsigned int fcpcs_chkr_mode,
                                   unsigned int enable);

int bcm_plp_quadra28_fc_pcs_chkr_enable_get(bcm_plp_quadra28_access_t phy_info, unsigned int fcpcs_chkr_mode,
                                   unsigned int* enable);

int bcm_plp_quadra28_fc_pcs_chkr_status_get(bcm_plp_quadra28_access_t phy_info, unsigned int *lock_status,
                                   unsigned int* lock_lost_lh, unsigned int* error_count);
#ifdef SERDES_API_FLOATING_POINT
int bcm_plp_quadra28_eye_margin_proj(bcm_plp_quadra28_access_t phy_info, double rate,
                            unsigned char ber_scan_mode, unsigned char timer_control,
                            unsigned char max_error_control);
#else 
int bcm_plp_quadra28_eye_margin_proj(bcm_plp_quadra28_access_t phy_info, int rate,
                            unsigned char ber_scan_mode, unsigned char timer_control,
                            unsigned char max_error_control);
#endif

int bcm_repeater_mode_get(bcm_plp_quadra28_access_t phy_info, unsigned int *ena_dis);

int bcm_plp_quadra28_repeater_mode_set(bcm_plp_quadra28_access_t phy_info, unsigned int ena_dis);

int bcm_plp_quadra28_module_read(bcm_plp_quadra28_access_t phy_info, unsigned int slv_addr,
                        unsigned int start_addr, unsigned int no_of_bytes,
                        unsigned char *read_data);

int bcm_plp_quadra28_module_write(bcm_plp_quadra28_access_t phy_info, unsigned int slv_addr,
                         unsigned int start_addr, unsigned int no_of_bytes,
                         unsigned char *write_data);

int bcm_modctrl_cfg_cfp_linecard_set(bcm_plp_quadra28_access_t phy_info, 
                                         bcm_plp_quadra28_modctrl_cfp_io_pins_t *mdcrtl_pins);

int bcm_modctrl_cfg_qsfp_linecard_set(bcm_plp_quadra28_access_t phy_info,
                                         bcm_plp_quadra28_modctrl_qsfp_io_pins_t  *mdcrtl_pins);

int bcm_modctrl_cfg_qsfp_linecard_get(bcm_plp_quadra28_access_t phy_info,
                                         bcm_plp_quadra28_modctrl_qsfp_io_pins_t *mdcrtl_pins);

int bcm_modctrl_cfg_cfp_linecard_get(bcm_plp_quadra28_access_t phy_info,
                                         bcm_plp_quadra28_modctrl_cfp_io_pins_t *mdcrtl_pins);

int bcm_plp_quadra28_cfg_gpio_pin_set(bcm_plp_quadra28_access_t phy_info, unsigned int gpio_pin_number,
                             unsigned int cfg_direction, unsigned int cfg_pull,
                             unsigned int pin_value);

int bcm_plp_quadra28_cfg_gpio_pin_get(bcm_plp_quadra28_access_t phy_info, unsigned int gpio_pin_number,
                             unsigned int *cfg_direction, unsigned int *cfg_pull,
                             unsigned int *pin_value);

int bcm_plp_quadra28_power_set(bcm_plp_quadra28_access_t phy_info,  unsigned int power_rx, unsigned int power_tx);

int bcm_plp_quadra28_power_get(bcm_plp_quadra28_access_t phy_info,  unsigned int *power_rx, unsigned int *power_tx);

int bcm_plp_quadra28_firmware_lane_config_set(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_pm_firmware_lane_config_t* firmware_lane_config);

int bcm_plp_quadra28_firmware_lane_config_get(bcm_plp_quadra28_access_t phy_info, bcm_plp_quadra28_pm_firmware_lane_config_t* firmware_lane_config);
int
bcm_plp_quadra28_init_fw_bcast(bcm_plp_quadra28_access_t phy_info,
							   int (*read)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int* val),
							   int (*write)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int val),
							   bcm_plp_quadra28_firmware_load_type_t *firmware_load_type,
							   bcm_pm_firmware_broadcast_method_t broadcast_method);
int bcm_plp_quadra28_failover_mode_set(bcm_plp_quadra28_access_t phy_info,unsigned int failover_mode);
int bcm_plp_quadra28_failover_mode_get(bcm_plp_quadra28_access_t phy_info,unsigned int *failover_mode);

int bcm_plp_quadra28_edc_config_set(bcm_plp_quadra28_access_t phy_info, unsigned int edc_method, unsigned int edc_value );
int bcm_plp_quadra28_edc_config_get(bcm_plp_quadra28_access_t phy_info, unsigned int *edc_method, unsigned int *edc_value );
#endif
