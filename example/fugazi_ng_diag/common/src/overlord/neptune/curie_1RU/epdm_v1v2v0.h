/* $Id: epdm_v1v2v0.h,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/epdm_v1v2v0.h,v $
 *-----------------------------------------------------------------------------
 * epdm.h - Leverage from BCM API
 * Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/bcm_quadra28_app/epdm.h
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef EPDM_H
#define EPDM_H

#include "bcm_common_defines.h"

#define MAX_CHIP_NAME_SIZE  20
#define CHIP_IS(_name, _id)    ( 0 == strcmp((_name), chip_id[(_id)]) )
enum {
    plp_quadra28,
    plp_chip_count
} plpDispatch_e;


/*! PHY Access Information 
 *
 * \arg void *platform_ctxt \n
 *             Represents user data. This member is passed to\n
 *             register read/write APIs. It can be NULL if not used.
 *
 * \arg unsigned int phy_addr \n
 *             Represents PHY-ID\n
 *
 * \arg unsigned int if_side \n
 *             Represents the interface side \n
 *                    0 - line side of the PHY device\n
 *                    1 - system side of the PHY device\n
 *
 * \arg unsigned int lane_map \n
 *             Represents the Lane mapping of a port\n
 *             LSB Bit 0 represents lane 0 of the specified PHY-ID.\n
 *             LSB Bit 1 represents lane 1 of the specified PHY-ID\n
 *             and similarly for lane 2 to lane N,\n
 *             where N is the maximum number of lanes on a PHY.
 *             It also supports multicast\n
 *             Eg:
 *                   0x3 represents lane 0 and 1 \n
 *                   0xF represents lane 0 to lane 3
 */
typedef struct bcm_plp_access_s {
    void *platform_ctxt;
    unsigned int phy_addr;
    unsigned int if_side;
    unsigned int lane_map;
}bcm_plp_access_t;

typedef struct bcm_plp_value_override_s {
    unsigned int enable;
    unsigned int value;
} bcm_plp_value_override_t;

typedef struct bcm_plp_rx_s {
    bcm_plp_value_override_t vga;
    unsigned int num_of_dfe_taps; /*number of elements in DFE array*/
    bcm_plp_value_override_t dfe[BCM_NUM_DFE_TAPS];
    bcm_plp_value_override_t peaking_filter;
    bcm_plp_value_override_t low_freq_peaking_filter;
} bcm_plp_rx_t;

typedef struct bcm_plp_device_aux_modes_s {
    unsigned int pass_thru;
} bcm_plp_device_aux_modes_t;

typedef struct bcm_plp_tx_s {
    char pre;
    char main;
    char post;
    char post2;
    char post3;
    char amp;
}bcm_plp_tx_t;

typedef struct bcm_plp_pm_diag_slicer_offset_s {
    unsigned int offset_pe;
    unsigned int offset_ze;
    unsigned int offset_me;
    unsigned int offset_po;
    unsigned int offset_zo;
    unsigned int offset_mo;
} bcm_plp_pm_diag_slicer_offset_t;

typedef struct bcm_plp_pm_diag_eyescan_s {
    unsigned int heye_left;
    unsigned int heye_right;
    unsigned int veye_upper;
    unsigned int veye_lower;
} bcm_plp_pm_diag_eyescan_t;

typedef struct bcm_plp_pm_phy_diagnostics_s {
    unsigned int signal_detect;
    unsigned int vga_bias_reduced;
    unsigned int postc_metric;
    bcm_plp_pm_osr_mode_t osr_mode;
    bcm_plp_pm_pmd_mode_t pmd_mode;
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
    bcm_plp_pm_diag_slicer_offset_t slicer_offset;
    bcm_plp_pm_diag_eyescan_t eyescan;
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
} bcm_plp_pm_phy_diagnostics_t;

#define MAX_LANES_PER_CORE  16
typedef struct bcm_plp_laneswap_map_s {
    /* Number of elements in lane_map_rx/tx arrays */
    unsigned int num_of_lanes;
    /* lane_map_rx[x]=y means that rx lane x is mapped to rx lane y */
    unsigned int lane_map_rx[MAX_LANES_PER_CORE];
    /* lane_map_tx[x]=y means that tx lane x is mapped to tx lane y */
    unsigned int lane_map_tx[MAX_LANES_PER_CORE];
} bcm_plp_laneswap_map_t;

typedef struct bcm_plp_pm_phy_reset_s {
    bcm_plp_pm_reset_direction_t rx;
    bcm_plp_pm_reset_direction_t tx;
} bcm_plp_pm_phy_reset_t;

typedef struct plp_static_config_s {
    unsigned int rptr_mode;
    unsigned int ull_dp;
    unsigned int an_master_lane;
    unsigned int an_mst_lane_p0;
    unsigned int an_mst_lane_p1;
    unsigned int avdd_txdrv;
} plp_static_config_t;

typedef struct bcm_plp_phy_static_config_s {
    unsigned int phy_id;
    void* bcm_static_config;
} bcm_plp_phy_static_config_t;

typedef struct bcm_plp_modctrl_pin_io_value_s {
    unsigned int enable;
    unsigned int value;
} bcm_plp_modctrl_pin_io_value_t;

typedef struct bcm_plp_modctrl_cfp_io_pins_s {
    bcm_plp_modctrl_pin_io_value_t tx_dis;
    bcm_plp_modctrl_pin_io_value_t rx_los;
    bcm_plp_modctrl_pin_io_value_t mod_lopwr;
    bcm_plp_modctrl_pin_io_value_t mod_abs;
    bcm_plp_modctrl_pin_io_value_t glb_alrmn;
    bcm_plp_modctrl_pin_io_value_t mod_rstn;
} bcm_plp_modctrl_cfp_io_pins_t;

typedef struct bcm_plp_modctrl_qsfp_io_pins_s {
    bcm_plp_modctrl_pin_io_value_t lpmod;
    bcm_plp_modctrl_pin_io_value_t resetl;
    bcm_plp_modctrl_pin_io_value_t intl;
    bcm_plp_modctrl_pin_io_value_t mod_sell;
    bcm_plp_modctrl_pin_io_value_t mod_prsl;
} bcm_plp_modctrl_qsfp_io_pins_t;

typedef struct bcm_plp_pm_firmware_lane_config_s {
    bcm_plp_pm_firmware_mode_t firmware_mode;
    unsigned int ena_dis;
} bcm_plp_pm_firmware_lane_config_t;

typedef struct bcm_plp_firmware_load_type_s{
	bcm_pm_firmware_load_method_t firmware_load_method;
	bcm_pm_firmware_load_force_t  force_load_method;
}bcm_plp_firmware_load_type_t;

int bcm_plp_static_config_set(char* chip_name, bcm_plp_access_t phy_info,  void* bcm_static_config);

int bcm_plp_static_config_get(char* chip_name, bcm_plp_access_t phy_info,  void* bcm_static_config);
int bcm_plp_init(char* chip_name, bcm_plp_access_t phy_info,  int (*read)(void* user_acc,
            unsigned int core_addr, unsigned int reg_addr, unsigned int* val), 
            int (*write)(void* user_acc, unsigned int core_addr, unsigned int reg_addr,
             unsigned int val), bcm_pm_firmware_load_method_t firmware_load_method);

int bcm_plp_init_fw_bcast(char* chip_name, bcm_plp_access_t phy_info,
                          int (*read)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int* val),
                          int (*write)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int val),
                          bcm_plp_firmware_load_type_t *firmware_load_type,
                          bcm_plp_firmware_broadcast_method_t broadcast_method);
int bcm_plp_cleanup(char* chip_name, bcm_plp_access_t phy_info);

int bcm_plp_link_status_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *link_status);

int bcm_plp_mode_config_set(char* chip_name, bcm_plp_access_t phy_info,  int speed, int if_type,
                            int ref_clk, int interface_mode, void* device_aux_modes);

int bcm_plp_mode_config_get(char* chip_name, bcm_plp_access_t phy_info,  int *speed, 
                            int *if_type, int *ref_clk, int *interface_mode,
                            void *device_aux_modes);

void bcm_plp_version_get(char* chip_name, unsigned short *chip_ver, unsigned short *api_ver,
                         unsigned short *enahan_ver);

int bcm_plp_prbs_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int tx_rx, 
                     unsigned int poly, unsigned int invert,
                     unsigned int loopback, unsigned int ena_dis);

int bcm_plp_prbs_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int tx_rx,
                     unsigned int *poly, unsigned int *invert,
                     unsigned int *loopback, unsigned int *ena_dis);

int bcm_plp_prbs_rx_stat(char* chip_name, bcm_plp_access_t phy_info,  unsigned int time);

int bcm_plp_prbs_clear(char* chip_name, bcm_plp_access_t phy_info,  unsigned int tx_rx);

int bcm_plp_prbs_config_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int tx_rx,
                            unsigned int *poly, unsigned int *invert);

int bcm_plp_prbs_status_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *prbs_lock,
                            unsigned int *prbs_lock_loss, unsigned int *error_count);  

int bcm_plp_reg_value_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int devaddr,
                          unsigned int regaddr, unsigned int data);

int bcm_plp_reg_value_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int devaddr,
                          unsigned int regaddr, unsigned int *data);

int bcm_plp_polarity_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int tx_pol,
                         unsigned int rx_pol);

int bcm_plp_polarity_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *tx_pol,
                         unsigned int *rx_pol);

int bcm_plp_rx_pmd_lock_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int* rx_pmd_lock);

int bcm_plp_rev_id(char* chip_name, bcm_plp_access_t phy_info,  unsigned int* rev_id);


int bcm_plp_loopback_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int lb_mode,
                         unsigned int enable);

int bcm_plp_loopback_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int lb_mode,
                         unsigned int *enable);

int bcm_plp_tx_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_tx_t* tx);

int bcm_plp_tx_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_tx_t* tx);

int bcm_plp_rx_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_rx_t* rx);

int bcm_plp_rx_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_rx_t* rx);

int bcm_plp_reset_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int reset_mode,
                      unsigned int reset_val);

int bcm_plp_phy_lane_reset_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_pm_phy_reset_t* reset);

int bcm_plp_phy_lane_reset_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_pm_phy_reset_t* reset);

int bcm_plp_tx_lane_control_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_pm_phy_tx_lane_control_t tx_control);

int bcm_plp_rx_lane_control_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_pm_phy_rx_lane_control_t rx_control);

int bcm_plp_tx_lane_control_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_pm_phy_tx_lane_control_t *tx_control);

int bcm_plp_rx_lane_control_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_pm_phy_rx_lane_control_t *rx_control);

int bcm_plp_lane_cross_switch_map_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int* tx_source_array);

int bcm_plp_lane_cross_switch_map_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *mapped_to);

int bcm_plp_force_tx_training_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int enable);

int bcm_plp_force_tx_training_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *enable);

int bcm_plp_force_tx_training_status_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *enabled,
                                         unsigned int *training_failure, unsigned int *trained);

int bcm_plp_cl73_ability_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned short tech_ability,
                             unsigned short fec_ability, unsigned short pause_ability);

int bcm_plp_cl73_ability_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned short *tech_ability,
                             unsigned short *fec_ability, unsigned short *pause_ability);
int bcm_plp_cl73_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned short ena_dis);

int bcm_plp_cl73_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *an,
                     unsigned int *an_done);

int bcm_plp_display_eye_scan(char* chip_name, bcm_plp_access_t phy_info);

int bcm_plp_firmware_info_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *fw_version,
                              unsigned int *fw_crc);

int bcm_plp_firmware_set(char* chip_name, bcm_plp_access_t phy_info,  const int offset,
                     const unsigned char *data, const int len);

int bcm_plp_rxtx_laneswap_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_laneswap_map_t* laneswap_map);

int bcm_plp_rxtx_laneswap_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_laneswap_map_t* laneswap_map);

int bcm_plp_pll_sequencer_restart(char* chip_name, bcm_plp_access_t phy_info,  unsigned char flags,
                                  bcm_pm_sequencer_operation_t operation);

int bcm_plp_fec_enable_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int enable);

int bcm_plp_fec_enable_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int* enable);

int bcm_plp_phy_status_dump(char* chip_name, bcm_plp_access_t phy_info);

int bcm_plp_phy_diagnostics_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_pm_phy_diagnostics_t* diag);

int bcm_plp_intr_status_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int intr_type,
                            unsigned int* intr_status);

int bcm_plp_intr_enable_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int intr_type,
                            unsigned int enable);

int bcm_plp_intr_enable_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int intr_type,
                            unsigned int* enable);

int bcm_plp_intr_status_clear(char* chip_name, bcm_plp_access_t phy_info,  unsigned int intr_type);

int bcm_plp_fc_pcs_chkr_enable_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int fcpcs_chkr_mode,
                                   unsigned int enable);

int bcm_plp_fc_pcs_chkr_enable_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int fcpcs_chkr_mode,
                                   unsigned int* enable);

int bcm_plp_fc_pcs_chkr_status_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *lock_status,
                                   unsigned int* lock_lost_lh, unsigned int* error_count);
#ifdef SERDES_API_FLOATING_POINT
int bcm_plp_eye_margin_proj(char* chip_name, bcm_plp_access_t phy_info,  double rate,
                            unsigned char ber_scan_mode, unsigned char timer_control,
                            unsigned char max_error_control);
#else 
int bcm_plp_eye_margin_proj(char* chip_name, bcm_plp_access_t phy_info,  int rate,
                            unsigned char ber_scan_mode, unsigned char timer_control,
                            unsigned char max_error_control);
#endif

int bcm_plp_repeater_mode_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *ena_dis);

int bcm_plp_repeater_mode_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int ena_dis);

int bcm_plp_module_read(char* chip_name, bcm_plp_access_t phy_info,  unsigned int slv_addr,
                        unsigned int start_addr, unsigned int no_of_bytes,
                        unsigned char *read_data);

int bcm_plp_module_write(char* chip_name, bcm_plp_access_t phy_info,  unsigned int slv_addr,
                         unsigned int start_addr, unsigned int no_of_bytes,
                         unsigned char *write_data);

int bcm_plp_modctrl_cfg_cfp_linecard_set(char* chip_name, bcm_plp_access_t phy_info,  
                                         bcm_plp_modctrl_cfp_io_pins_t *mdcrtl_pins);

int bcm_plp_modctrl_cfg_qsfp_linecard_set(char* chip_name, bcm_plp_access_t phy_info, 
                                         bcm_plp_modctrl_qsfp_io_pins_t  *mdcrtl_pins);

int bcm_plp_modctrl_cfg_qsfp_linecard_get(char* chip_name, bcm_plp_access_t phy_info, 
                                         bcm_plp_modctrl_qsfp_io_pins_t *mdcrtl_pins);

int bcm_plp_modctrl_cfg_cfp_linecard_get(char* chip_name, bcm_plp_access_t phy_info, 
                                         bcm_plp_modctrl_cfp_io_pins_t *mdcrtl_pins);

int bcm_plp_cfg_gpio_pin_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int gpio_pin_number,
                             unsigned int cfg_direction, unsigned int cfg_pull,
                             unsigned int pin_value);

int bcm_plp_cfg_gpio_pin_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int gpio_pin_number,
                             unsigned int *cfg_direction, unsigned int *cfg_pull,
                             unsigned int *pin_value);

int bcm_plp_power_set(char* chip_name, bcm_plp_access_t phy_info,   unsigned int power_rx, unsigned int power_tx);

int bcm_plp_power_get(char* chip_name, bcm_plp_access_t phy_info,   unsigned int *power_rx, unsigned int *power_tx);

int bcm_plp_firmware_lane_config_set(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_pm_firmware_lane_config_t* firmware_lane_config);

int bcm_plp_firmware_lane_config_get(char* chip_name, bcm_plp_access_t phy_info,  bcm_plp_pm_firmware_lane_config_t* firmware_lane_config);

int bcm_plp_short_channel_mode_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int ena_dis);

int bcm_plp_short_channel_mode_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *ena_dis, unsigned int *status); 

#endif /* EPDM_H */

