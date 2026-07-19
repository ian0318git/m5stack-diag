/* $Id: diag_vtss_phy.h,v 1.1 2015/02/26 07:18:29 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/wallander/diag_vtss_phy.h,v $
 *------------------------------------------------------------------
 *
 * diag_vtss_phy.h - Header file for Vitesse PHY definitions and functions.
 *
 * Xiaoying Zhang -- Mar. 2014
 *
 * Copyright (c) 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __WALLANDER_PHY_H__
#define __WALLANDER_PHY_H__

#include "dev_object.h"
#include "vtss_misc_api.h"
#include "vtss_phy_api.h"
#include "vtss_init_api.h"
#include "vtss_port_api.h"
#include "vtss_phy_10g_api.h"
#include "vtss_phy_ts_api.h"
#include "vtss_api.h"
#include "vtss_wis_api.h"
#include "vtss_state.h"
#include "vtss_phy_ts.h"
#include "vtss_phy_ts_regs_ptp.h"

//#define NULL ((void *) 0)
#define EOK    0
//#define BIT_0  1
#define IEEEBYTES 6

#define VTSS_PHY_1588_PACKET_SIZE  0x1A  // 1588 packet size to FPGA

#define VTSS_PHY_ID_VSC8584         0x000707c0
#define VTSS_PHY_ID_VSC8552         0x000704e0
#define VTSS_PHY_ID_MASK            0xfffffff0

#define VTSS_NUM_PHY_MAX_PORTS     4

typedef struct vsc_phy_regs_t_
{
    const char *pagename;
    uint32_t    pagenum;
    reg_info_t *pageregs;
} vsc_phy_regs_t;

/*
** Device status definitions
**
** All of the functions in dev_object_fvt_t must return a device status
** to the caller for verification of the associated function. In the case
** of function success (ie., no errors), the device status will be
** 'DEV_STATUS_SUCCESS', whereas for any non-successful case, the device
** is expected to provide a device-specific FAILURE field for the caller
** to decipher.
*/
typedef u32 dev_status;
typedef enum dev_status_e_ {
    DEV_STATUS_SUCCESS = 0,
    DEV_STATUS_FAILURE_MALLOC = 1,
    DEV_STATUS_INVALID_ARG = 2,         /* useful for NULL check failures */
    DEV_STATUS_COMMON_END = DEV_STATUS_INVALID_ARG
} dev_status_e;


/*
** Device status isr definitions
**
** When a device has completed the service an interrupt, it will return
** a value to the caller indicating the status of the device interrupt
** handler; These common device isr status definitions are shown below...
*/
typedef u32 dev_status_isr;
typedef enum dev_status_isr_e_ {
    DEV_STATUS_ISR_SUCCESS = 0,
    DEV_STATUS_ISR_INFO = 1,
    DEV_STATUS_ISR_WARNING = 2,
    DEV_STATUS_ISR_NON_FATAL = 3,
    DEV_STATUS_ISR_FATAL = 4,
    DEV_STATUS_ISR_COMMON_END = DEV_STATUS_ISR_FATAL
} dev_status_isr_e;


#define VSC_LED_MOD_SEL        0x001D
#define VSC_LED_OFF            0
#define VSC_LED_GREEN          1
#define VSC_LED_AMBER          2
#define VSC_LED0_OFF           0x000E
#define VSC_LED0_ON            0x000F
#define VSC_LED1_OFF           0x00E0
#define VSC_LED1_ON            0x00F0
#define VSC_LED2_OFF           0x0E00
#define VSC_LED2_ON            0x0F00
#define VSC_LED0_MASK          0x000F
#define VSC_LED1_MASK          0x00F0
#define VSC_LED2_MASK          0x0F00
#define VSC_ALL_LED_OFF        0x0EEE

/* Set media_type on the PHY */
#define VSC85XX_1G_MODE_CONTROL_REGISTER  0x00
#define VSC85XX_1G_AUTONEG_ENABLE         0x1000

#define VSC85XX_1G_PHY_ID_1_REGISTER      0x02
#define VSC85XX_1G_PHY_ID_2_REGISTER      0x03

#define VSC85XX_COMMAND_REGISTER        0x12
#define VSC85XX_GPIO_CONTROL_3          0x13
#define VSC85XX_ENABLE_1000X            0x8FC1
#define VSC85XX_ENABLE_100FX            0x8FD1
#define VSC85XX_MICRO_CMD_DELAY         10
#define VSC85XX_MICRO_CMD_BEGIN         0x8000
#define VSC85XX_EXT_PHY_CONTROL_1       0x17
#define VSC_MEDIA_TYPE_MASK             0x0700
#define VSC_MEDIA_TYPE_1000BASE_X       0x0200
#define VSC_MEDIA_TYPE_100BASE_FX       0x0300
#define VSC_MEDIA_TYPE_PASS_THRU        0x100
#define VSC85XX_MODE_CONTROL            0x0
#define VSC85XX_SW_RESET                0x8000
#define VSC85XX_SW_RESET_DELAY          4
#define VSC85XX_1G_INTR_MASK_REGISTER   0x19

#define VSC85XX_1G_EX_MODE_CTRL_REGISTER  0x13


#define VSC8487_10G_PAGE_1E                 0x1e
#define VSC8487_10G_PAGE_2                  0x2
#define VSC8487_10G_PAGE_3                  0x3
#define VSC8487_10G_WIS_INTR_MASK_REGISTER  0xEE06

#define VSC8487_10G_IP_1588_SPARE_REG       0x002F

/* cut and pasted from vtss_phy_ts_api.c, not in API .h file */
typedef enum {
    VTSS_PHY_TS_ANA_BLK_ID_ING_0, /* Order taken from 1G PHY */
    VTSS_PHY_TS_ANA_BLK_ID_EGR_0,
    VTSS_PHY_TS_ANA_BLK_ID_ING_1,
    VTSS_PHY_TS_ANA_BLK_ID_EGR_1,
    VTSS_PHY_TS_ANA_BLK_ID_ING_2,
    VTSS_PHY_TS_ANA_BLK_ID_EGR_2,
    VTSS_PHY_TS_PROC_BLK_ID_0,
    VTSS_PHY_TS_PROC_BLK_ID_1,
    VTSS_PHY_TS_MAX_BLK_ID
} vtss_phy_ts_blk_id_t;



/* 1G PHY regsiter Addresses */
#define VSC85XX_PHY_EXT_REG_PAGE                    0x1F
#define VSC85XX_PHY_EXT_REG_PAGE_0                  0x0000
#define VSC85XX_PHY_EXT_REG_PAGE_1                  0x0001
#define VSC85XX_PHY_EXT_REG_PAGE_2                  0x0002
#define VSC85XX_PHY_EXT_REG_PAGE_3                  0x0003
#define VSC85XX_PHY_EXT_REG_PAGE_4                  0x0004
#define VSC85XX_PHY_EXT_REG_PAGE_16                 0x0010
#define VSC85XX_PHY_EXT_REG_PAGE_1588               0x1588
#define VSC85XX_PHY_EXT_REG_PAGE_52b5               0x52b5

/* Page 0 */
#define VSC85XX_TEST_MODE_REG                       0x9
#define VSC85XX_TEST_MODE_MASK                      0xe000
#define VSC85XX_NORMAL_MODE                         0x0000
#define VSC85XX_TEST_MODE_1                         0x2000
#define VSC85XX_TEST_MODE_2                         0x4000
#define VSC85XX_TEST_MODE_3                         0x6000
#define VSC85XX_TEST_MODE_4                         0x8000
#define SET_PHY_TO_MASTER_MODE                      0x1F00
#define SET_PHY_TO_SLAVE_MODE                       0x1700

/* Page 3*/
#define VSC85XX_PHY_PCS_CONTROL                     0x10
#define VSC85XX_PHY_PCS_CONTROL_ANEG_ENABLE         0x0080
#define VSC85XX_PHY_PCS_STATUS                      0x11
#define VSC85XX_PHY_PCS_STATUS_LINK                 0x0004
#define VSC85XX_PHY_MAC_SERDES_STATUS               0x14
#define VSC85XX_PHY_MAC_SERDES_SYNC                 0x2000

/* Page 16*/
#define VSC85XX_PHY_GPIO_CTRL2                      0x0e
#define VSC85XX_PHY_RCVD_CLK0_CTRL                  0x17
#define VSC85XX_PHY_RCVD_CLK1_CTRL                  0x18
#define VSC85XX_PHY_RCVD_CLK_CTRL_ENABLE            0x8000
#define VSC85XX_PHY_RCVD_CLK_CTRL_SRC_MASK          0x7800
#define VSC85XX_PHY_RCVD_CLK_CTRL_SRC_PHY0          0x0000
#define VSC85XX_PHY_RCVD_CLK_CTRL_SRC_PHY1          0x0800
#define VSC85XX_PHY_RCVD_CLK_CTRL_SRC_PHY2          0x1000
#define VSC85XX_PHY_RCVD_CLK_CTRL_SRC_PHY3          0x1800
#define VSC85XX_PHY_RCVD_CLK_CTRL_FREQ_MASK         0x0700
#define VSC85XX_PHY_RCVD_CLK_CTRL_FREQ_25MHZ        0x0000
#define VSC85XX_PHY_RCVD_CLK_CTRL_SQUELCH_MASK      0x0030
#define VSC85XX_PHY_RCVD_CLK_CTRL_SQUELCH_DISABLED  0x0030
#define VSC85XX_PHY_RCVD_CLK_CTRL_CLK_SEL_MASK      0x0007
#define VSC85XX_PHY_RCVD_CLK_CTRL_CLK_SEL_RCVD      0x0001

/* 10G PHY Registeris*/
#define VSC8487_PHY_PMA                                 0x01  /* Addr 1 - PMA */
#define VSC8487_PHY_DEV_VS1                             0x1E
#define VSC8487_PHY_RX_CLK_OUT_CTRL                     0xA008
#define VSC8487_PHY_RCVD_CLK_CTRL_ENABLE                0x0009
#define VSC8487_PHY_TX_RATE_CRTL                        0x8017
#define VSC8487_PHY_REF_CLK_SEL                         0x7F10
#define VSC8487_PHY_GPIO4                               0x0108
#define VSC8487_PHY_GPIO5                               0x010A
#define VSC8487_PHY_GPIO7                               0x0126
#define VSC8487_PHY_GPIO8                               0x0128
#define VSC8487_PHY_GPIO9                               0x012A
#define VSC8487_PHY_GPIO10                              0x012C
#define VSC8487_GPIO_INPUT                              0x8000
#define VSC8487_GPIO_TRADIO_SEL                         0x6000
#define VSC8487_GPIO_FUNC                               0x0007
#define VSC8487_GPIO_OUT_HI                             0x1000
#define VSC8487_GPIO4_CFG_STATUS_REG                    0x0108
#define VSC8487_GPIO4_CFG2_REG                          0x0109
#define VSC8487_GPIO5_CFG_STATUS_REG                    0x010a
#define VSC8487_GPIO5_CFG2_REG                          0x010b

#define VSC8487_INT_GPIO                                6

/* Tx CLk rate 156.25 Mhz*/
#define VSC8487_PHY_TX_RATE                             0
/* SREF as REF clk */
#define VSC8487_PHY_REF_CLK                             0x00C0
/* Config GPIO4 to tx Internal Signals */
#define VSC8487_PHY_TX_INTERNAL_SIG                     0x0003
/* CH0 PCS_RX Status on GPIO 4 Pin */
#define VSC8487_PHY_CH0_PCS_RX_STATUS                   0x006C

#define VSC8488_1588_INTR_STATUS_REG                    0x002D
#define VSC8488_1588_INTR_ANALYZER_BIT                  0x40
#define VSC8488_1588_INTR_PREAMBLE_BIT                  0x20
#define VSC8488_1588_INTR_FCS_ERR_BIT                   0x10
#define VSC8488_1588_INTR_LOADED_BIT                    0x04
#define VSC8488_1588_INTR_UNDERFLOW_BIT                 0x02
#define VSC8488_1588_INTR_OVERFLOW_BIT                  0x01
#define VSC8488_1588_INTR_STATUS_MASK                   0x77

typedef enum dev_vscg_phy_dual_media_mode_ {
    DEV_VSCG_MODE_INVALID,
    DEV_VSCG_MODE_100BASE_FX_ONLY,        
    DEV_VSCG_MODE_COPPER_ONLY,            
    DEV_VSCG_MODE_1000BASE_X_ONLY,
    DEV_VSCG_MODE_SGMII_ONLY,
    DEV_VSCG_MODE_AUTO_COPPER_SGMII,      
    DEV_VSCG_MODE_AUTO_COPPER_1000BASEX,  
    DEV_VSCG_MODE_AUTO_COPPER_100BASE_FX, 
} dev_vscg_phy_dual_media_mode;

typedef enum vts_phy_port_feature_t {
    vts_phy_set_speed_type,
    vts_phy_set_duplex_type,
    vts_phy_set_tx_flowcontrol_type,
    vts_phy_set_rx_flowcontrol_type,
    vts_phy_set_autoneg_enable_type,
    vts_phy_set_automdix_type,
    vts_phy_set_tbi_mode_on_type,
    vts_phy_stbi_mode_on_type,                            
    vts_phy_sfp_information_type,                         
    vts_phy_sfp_enable_power,                             
    vts_phy_sfp_disable_power,                            
    vts_phy_asic_side_autoneg_disable_type,               
    vts_phy_prbs_start,                                   
    vts_phy_prbs_stop,                                    
    vts_phy_clear_ls_alarm_interrupt,                     
    vts_phy_test_set_phy_regs,                            
    vts_phy_set_phy_sfp_mode,                             
    vts_phy_disable_transmit,                             
    vts_phy_enable_transmit,                              
    vts_phy_set_to_default,                               
    vts_phy_ang_next_page_capable,                        
    vts_phy_ang_next_page_content,                        
    vts_phy_set_skylab_ip_bypass,                         
    vts_phy_unset_skylab_ip_bypass,                       
    vts_phy_enable_xaui_tx,                               
    vts_phy_disable_xaui_tx,                              
                                                       
    /* get feature defines */                          
    vts_phy_module_uniqueness_info,                       
    vts_phy_prbs_capability,                              
    vts_phy_prbs_counter,                                 
    vts_phy_test_get_phy_regs,                            
    vts_phy_test_dsp_phy_regs,                            
    vts_phy_get_dmi_data,                                 
    vts_phy_get_dmi_cap,                                  
    vts_phy_is_tdr_supported,                             
    vts_phy_is_tdr_media_invalid,                         
    vts_phy_tdr_test,                                     
    vts_phy_tdr_get_result,                               
    vts_phy_get_pd_det_status,                            
    vts_phy_set_led_color_raw,
    vts_phy_disable_link_raw,
    vts_phy_module_pep_info,
    vts_phy_speed_duplex_reg_set,
    vts_phy_anar_check_supported,
    vts_phy_is_copper_link,
    vts_phy_set_phy_link_mode,
    vts_phy_ang_status,
    vts_phy_ang_aner,
    vts_phy_ang_lp_next_page,
    vts_phy_post_init,
    vts_phy_post_done,
    vts_phy_skip_sgmii_aneg_chk,
    vts_phy_get_enh_link_set,
    vts_phy_lxb_xcvr_write_get,
    vts_phy_disable_sgmii_aneg,
    vts_phy_skip_soft_reset,
    vts_phy_tx_disable,
    vts_phy_get_speed_type,
    vts_phy_get_duplex_type,
    vts_phy_set_timing_feature_type,
    vts_phy_get_timing_feature_type,

    vts_phy_set_1731_feature_type,
    vts_phy_get_1731_feature_type,
    vts_phy_set_media_type,
    vts_phy_get_media_type,
}vts_phy_port_feature_t;

typedef struct dev_vsc8x_port_config_t_ {
    u16 mode;
    u16 speed;
#define DEV_VTSS_SPEED_DEFAULT ((ether_speed) 0)
#define DEV_VTSS_SPEED_AUTO  (1)
#define DEV_VTSS_SPEED_10MB  (2)
#define DEV_VTSS_SPEED_100MB (4)
#define DEV_VTSS_SPEED_1GB   (8)
#define DEV_VTSS_SPEED_10GB  (16)
    u16 duplex;
#define DEV_VTSS_DUPLEX_TYPE_HALF 1
#define DEV_VTSS_DUPLEX_TYPE_FULL 10
#define DEV_VTSS_DUPLEX_TYPE_AUTO 100
    u16 aneg_speed;
/* aneg_speed values are same as speed */
    u16 aneg_duplex;    
/* aneg_duplex values are same as duplex */
    BOOL pause_enable;
    u8 aneg_enable;   
#define DEV_VTSS_PHY_MODE_ANEG 0
#define DEV_VTSS_PHY_MODE_FORCED 1
#define DEV_VTSS_PHY_MODE_UNKNOWN 2
    BOOL auto_mdix;
    dev_vscg_phy_dual_media_mode media;
} dev_vsc8x_port_config_t;

typedef struct dev_vscg_led_signals_ {
    u8 led_dest_signals[4];
} dev_vscg_led_signals;

typedef enum dev_vsc8x_workaround_t_ {
   DEV_VSC8X_WORKAROUND_NONE,
} dev_vsc8x_workaround_t;

typedef enum dev_vsc8x_link_status_t_ {
   DEV_VSC_LINK_STATUS_DN,
   DEV_VSC_LINK_STATUS_UP,
   DEV_VSC_LINK_STATUS_UNAVAIL
} vsc8x_link_status_t;

typedef struct dev_vts_phy_loopback_type_t_ {
//     vtss_phy_10g_loopback_t loopback_10g;
//     vtss_phy_1g_loopback_t loopback_1g;
    u8 dummy;
} dev_vts_phy_loopback_type_t;

typedef struct dev_vsc8x_object_t_ dev_vsc8x_object_t;

typedef BOOL (*dev_vsc8x_callin_enable_disable_link_fn) 
         (dev_vsc8x_object_t *phy, u16 port_num, 
          BOOL enable);

typedef BOOL (*dev_vsc8x_callin_set_one_feature_fn)
         (dev_vsc8x_object_t *phy, u16 port_num,
          vts_phy_port_feature_t feature,
          dev_vsc8x_port_config_t phy_config);

typedef BOOL (*dev_vsc8x_callin_get_one_feature_fn)
         (dev_vsc8x_object_t *phy, u16 port_num,
          vts_phy_port_feature_t feature,
          dev_vsc8x_port_config_t *phy_config);

typedef BOOL (*dev_vsc8x_callin_get_oper_state_fn)
         (dev_vsc8x_object_t *phy, u16 port_num,
          dev_vsc8x_port_config_t *phy_config);

typedef BOOL (*dev_vsc8x_callin_loopback_fn) 
         (dev_vsc8x_object_t *phy, u16 port_num,
          dev_vts_phy_loopback_type_t *loopback_type);

typedef BOOL (*dev_vsc8x_callin_set_led_color_fn) 
         (dev_vsc8x_object_t *phy, u16 port_num,
          dev_vscg_led_signals *led_signals);

typedef BOOL (*dev_vsc8x_is_workaround_reqd_fn) 
         (dev_vsc8x_object_t *phy, u16 port_num,
         dev_vsc8x_workaround_t workaround);

typedef BOOL (*dev_vsc8x_workaround_apply_fn) 
         (dev_vsc8x_object_t *phy, u16 port_num,
         dev_vsc8x_workaround_t workaround);

typedef vsc8x_link_status_t (*dev_vsc8x_callin_get_link_status_fn)
         (dev_vsc8x_object_t *phy, u16 port_num);

typedef BOOL (*dev_vsc8x_callin_set_one_feature_raw_fn)
         (dev_vsc8x_object_t *phy, u16 port_num,
          vts_phy_port_feature_t feature,
          void *param);

typedef BOOL (*dev_vsc8x_callin_set_recover_clk)
         (dev_vsc8x_object_t *phy, u16 port_num,
          BOOL primary, BOOL enable);

typedef BOOL (*dev_vsc8x_callin_set_sync_mode)
         (dev_vsc8x_object_t *phy, u16 port_num,
          BOOL enable);


typedef vtss_rc (*phy_miin_read_fn) (const vtss_port_no_t port_no,
                const u8 addr, u16 *const val);

typedef vtss_rc (*phy_miin_write_fn) (const vtss_port_no_t port_no,
                const u8  addr, const u16 val);

/*
 * Callout Function for 10G MMD operations
 */
typedef vtss_rc (*phy_mmd_read_fn) (const vtss_port_no_t port_no,
             const u8 mmd, const u16 addr, u16 *const val);

typedef vtss_rc (*phy_mmd_write_fn) (const vtss_port_no_t port_no,
             const u8 mmd, const u16 addr, const u16 val);

typedef vtss_rc (*phy_mmd_read_inc_fn) (const vtss_port_no_t port_no,
             const u8 mmd, const u16 addr, u16 *const buf, u8 count);

/*
 * WAN interface sublayer alarms datastructure
 *
 */
typedef struct ether_wis_alerts_ {
    BOOL    ser; /* Severly error Frame  */
    BOOL    plm_p_far; /* Far end Path Label Mismatch */
    BOOL    ais_p_far; /* Far end Alarm Indication Signal */
    BOOL    lof; /* Loss of Frame */
    BOOL    los; /* Loss of Signal raised */
    BOOL    rdi; /* Remote defect indication line flag raised */
    BOOL    ais_l; /* Ais for line flag */
    BOOL    lcd_p; /* Path loss of code-group delineation */
    BOOL    plm_p; /* Path label mismatch */
    BOOL    ais_p ; /* AIS for path */
    BOOL    lop; /* Loss of Pointer */
} ether_wis_alerts;

/*
 * WAN BIP stats
 */
typedef struct ether_wis_bip_block_error_stats_ {
    u16 far_end_path_block_error_count;
    u32 far_end_wis_line_bip_error;
    u32 wis_line_bip_error;
    u16 wis_path_block_error_count;
    u16 wis_section_bip_error_count;
} ether_wis_bip_block_error_stats;

typedef struct ether_wis_rx_ptb_ {
    u8 ptb_rx[16];
} ether_wis_rx_ptb;

typedef struct ether_wis_tx_ptb_ {
    u8 ptb_tx[16];
} ether_wis_tx_ptb;

typedef struct ether_wis_defects_info_t_ {
    ether_wis_tx_ptb  tx;
    ether_wis_rx_ptb  rx;
    ether_wis_bip_block_error_stats bip_stats;
    ether_wis_alerts alarms_stats;
    BOOL  ptb_tx_stable;
} ether_wis_defects_info_t;



/*
 * Callin Functions for 10G WAN operations
 */
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_mode_set_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num, BOOL *wan_mode);
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_mode_get_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num, BOOL *wan_mode);
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_reset_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num);
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_defects_get_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num,
         ether_wis_alerts *alarms);
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_status_get_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num);
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_perf_get_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num,
         ether_wis_bip_block_error_stats *errors);
typedef BOOL (*dev_vsc8x_10gphy_callin_wis_counter_get_fn)
         (dev_vsc8x_object_t *phy, uint16_t port_num,
         ether_wis_bip_block_error_stats *errors);

/*
 * Callout for Link status notification
 */
typedef vtss_rc (*phy_port_link_status_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_fast_link_fail_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_speed_state_change_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_full_duplex_state_change_fn)
        ( dev_vsc8x_object_t* phy,   const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_auto_neg_err_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_auto_neg_comp_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_tx_fifo_ovrflw_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_port_rx_fifo_ovrflw_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_10g_lopc_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_10g_rxlol_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_10g_pcs_rcv_fault_fn)( dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no );

typedef vtss_rc (*phy_10g_ewis_event_notify_fn)(dev_vsc8x_object_t *phy,
        const vtss_port_no_t port_no, ether_wis_alerts alarms);

typedef struct dev_vts8x_phy_callin_t_ {
    dev_vsc8x_callin_enable_disable_link_fn enable_disable_link;
    dev_vsc8x_callin_set_one_feature_fn     set_one_feature;
    dev_vsc8x_callin_get_one_feature_fn     get_one_feature;
    dev_vsc8x_callin_get_oper_state_fn      get_oper_state;
    dev_vsc8x_callin_loopback_fn            set_loopback;
    dev_vsc8x_callin_loopback_fn            get_loopback;
    dev_vsc8x_callin_set_led_color_fn       set_led_color;
    dev_vsc8x_is_workaround_reqd_fn         is_workaround_reqd;
    dev_vsc8x_workaround_apply_fn           workaround_apply;
    dev_vsc8x_callin_get_link_status_fn     get_link_status;
    dev_vsc8x_callin_set_one_feature_raw_fn set_one_feature_raw;
    dev_vsc8x_callin_set_recover_clk        set_recover_clk;
    dev_vsc8x_callin_set_sync_mode          set_sync_mode;
} dev_vts8x_phy_callin_t;

typedef struct dev_vts8x_phy_callout_t_ {
    phy_port_link_status_fn                 phy_link_notify;
    phy_port_fast_link_fail_fn              phy_fast_link_fail_notify;
    phy_port_speed_state_change_fn          phy_speed_state_change_notify;
    phy_port_full_duplex_state_change_fn    phy_full_duplex_state_change_notify;
    phy_port_auto_neg_err_fn                phy_auto_neg_err_notify;
    phy_port_auto_neg_comp_fn               phy_auto_neg_comp_notify;
    phy_port_tx_fifo_ovrflw_fn              phy_tx_fifo_overflw_notify;
    phy_port_rx_fifo_ovrflw_fn              phy_rx_fifo_overflw_notify;
    phy_10g_lopc_fn                         phy_10g_lopc_status_notify;
    phy_10g_rxlol_fn                        phy_10g_rxlol_status_notify;
    phy_10g_pcs_rcv_fault_fn                phy_10g_pcs_rcv_fault_notify;
    phy_10g_ewis_event_notify_fn            phy_10g_ewis_event_notify;
} dev_vts8x_phy_callout_t;

typedef struct dev_vts8x_1gphy_callout_t_ {
    phy_miin_read_fn     phy_miim_read;
    phy_miin_write_fn    phy_miim_write;
} dev_vts8x_1gphy_callout_t;

typedef struct dev_vts8x_10gphy_callout_t_ {
    phy_mmd_read_fn     phy_mmd_read;
    phy_mmd_write_fn    phy_mmd_write;
    phy_mmd_read_inc_fn phy_mmd_read_inc;
} dev_vts8x_10gphy_callout_t;

typedef struct dev_vts8x_10gphy_callin_t_ {
    dev_vsc8x_10gphy_callin_wis_mode_set_fn    wis_mode_set;
    dev_vsc8x_10gphy_callin_wis_mode_get_fn    wis_mode_get;
    dev_vsc8x_10gphy_callin_wis_reset_fn       wis_reset;
    dev_vsc8x_10gphy_callin_wis_defects_get_fn wis_defects_get;
    dev_vsc8x_10gphy_callin_wis_status_get_fn  wis_status_get;
    dev_vsc8x_10gphy_callin_wis_perf_get_fn    wis_perf_get;
    dev_vsc8x_10gphy_callin_wis_counter_get_fn wis_counter_get;
} dev_vts8x_10gphy_callin_t;

typedef struct vsc_api_controller_t_ {
    vtss_target_type_t  platform_target;
    vtss_inst_create_t  vsc_create_inst; /* Context kepy by API */
    vtss_inst_t         vsc_instance;    /* Cookie for Instance */
    vtss_init_conf_t    vsc_init_config;/* Init Configuration */
    vtss_phy_reset_conf_t *phy_reset_config; /* Reset Config, per PHY */
    vtss_phy_conf_t     *phy_config_set;  /* PHY config after Reset */
    vtss_phy_clock_conf_t *phy_clock_conf; /* Initial clock config */
    vtss_phy_recov_clk_t  *phy_recov_clk;
//     vtss_phy_10g_mode_t   *phy_10g_init_params;
    u8               ucode_done:8; /* Tracks ucode download, 1 for each PHY */
} vsc_api_controller_t;

struct dev_vsc8x_object_t_ {
    dev_object_t               base;
    dev_vts8x_phy_callin_t     *callins;
    dev_vts8x_phy_callout_t    *callout;
    dev_vts8x_1gphy_callout_t  *callout_1g;
    dev_vts8x_10gphy_callin_t  *callin_10g;
    dev_vts8x_10gphy_callout_t *callout_10g;
    u8                          port;
    u8                          slot;
    vsc_api_controller_t        *vsc_api_control;
};

typedef struct dev_vts_gphy_create_info_t_ {
    u8                        port;
    u8                        slot;
    vtss_target_type_t        phy_target;
    dev_vts8x_phy_callout_t *callout;
    dev_vts8x_1gphy_callout_t *callout_1g;
    dev_vts8x_10gphy_callout_t *callout_10g;
    vtss_phy_reset_conf_t      *port_1g_phy_reset_init_conf;
    vtss_phy_clock_conf_t      *port_1g_phy_clock_conf;
    vtss_phy_recov_clk_t       *port_1g_phy_recov_clk_conf;
//     vtss_phy_10g_mode_t        *port_10g_init_mode_conf;
    BOOL vsc_inst_created;
} dev_vts_gphy_create_info_t;

    
typedef enum
{
    DEV_VTSS_SPEED_UNDEFINED,   /**< Undefined */
    DEV_VTSS_SPEED_10M,         /**< 10 M */
    DEV_VTSS_SPEED_100M,        /**< 100 M */
    DEV_VTSS_SPEED_1G,          /**< 1 G */
    DEV_VTSS_SPEED_2500M,       /**< 2.5G */
    DEV_VTSS_SPEED_5G,          /**< 5G or 2x2.5G */
    DEV_VTSS_SPEED_10G,         /**< 10 G */
    DEV_VTSS_SPEED_12G          /**< 12G */
} dev_vtss_port_speed_t;


dev_object_t *dev_vsc8x_phy_create(dev_vts_gphy_create_info_t *info);
void dev_vts_gphy_destroy(dev_object_t *dev);

#define VTSS_BTRACE_ERR(format, ...) {      \
      programDebug(format, ##__VA_ARGS__); \

void
vts_phy_btrace_init(void);


typedef enum dev_vsc_ts_dir_ {
    DEV_VSC_TS_DIR_INGRESS = 0x1,
    DEV_VSC_TS_DIR_EGRESS = 0x2,
    DEV_VSC_TS_DIR_BOTH = (DEV_VSC_TS_DIR_INGRESS | DEV_VSC_TS_DIR_EGRESS)
} dev_vsc_ts_dir_t;


/*
 * Which of the 2 IP comparators
 */
typedef enum dev_vsc_ts_ip_comp_ {
    DEV_VSC_TS_IP_COMP1,
    DEV_VSC_TS_IP_COMP2,
} dev_vsc_ts_ip_comp_t;

typedef struct dev_vtss_ts_ptp_ipv4_flow_cfg_ {
    BOOL flow_en;
    vtss_phy_ts_engine_t eng_id;
    dev_vsc_ts_ip_comp_t comp_id;
    dev_vsc_ts_dir_t dir;
    u32 addr;
    u32 mask;
} dev_vtss_ts_ptp_ipv4_flow_cfg_t;

typedef struct dev_vtss_ts_ptp_ipv4_action_cfg_ {
    BOOL flow_en;
    vtss_phy_ts_engine_t eng_id;
    dev_vsc_ts_dir_t dir;
    u8 clk_domain;
} dev_vtss_ts_ptp_ipv4_action_cfg_t;


typedef struct vsc_1g_register_t_ {
    u32 page;
    u32 offset;
    u16 data;
} vsc_1g_register_t;

#define PTP_MESSAGE_TYPE (0x0) 
#define PTP_VERSION    (0x2)
#define PTP_PACKET_LENGTH (0x40)
// #define PTP_MESSAGE_LENGTH (0x34)
#define PTP_MESSAGE_LENGTH (0x54)
#define PTP_ETHERNET_TYPE (0x88f7)
#define PTP_PKT_CMP_START_LEN_1 (0x0)
#define PTP_PKT_CMP_END_LEN_1 (0xb)
/*#define PTP_PKT_CMP_START_LEN_2 (0x30)
#define PTP_PKT_CMP_END_LEN_2 (0x39)*/
#define PTP_PKT_CMP_START_LEN_2 (0x38)
#define PTP_PKT_CMP_END_LEN_2 (0x41)
#define PTP_PKT_LEN_BIT_31_MASK (0x80000000)

#define PTP_CFG_GEN_INRESS  (0x3400)
#define PTP_CFG_GEN_EGRESS  (0x3000)
#define PTP_READ_ADDRESS      (0x0)
#define PTP_WRITE_ADDRESS     (0x1)
#define PTP_DATA_LO           (0x2)
#define PTP_DATA_HI           (0x3)

typedef unsigned char Enumeration8; /**< Local definition of 8  bit enumerated value */

typedef struct 
{
    unsigned short epoch_number;
    unsigned int seconds;
    signed int  nanoseconds;  
} V2TimeRepresentation;

typedef struct 
{
    unsigned char        clockClass;
    Enumeration8     clockAccuracy;
    unsigned short       offsetScaledLogVariance;
} ClockQuality;

typedef struct 
{
    char            clockIdentity[8];
    unsigned short       portNumber;
} volatile PortIdentity;

/** PTP Version 2 Announce message structure */
typedef struct 
{
    V2TimeRepresentation originTimestamp;           
    signed short            currentUTCOffset;            
    unsigned char            reserved;                    
    unsigned char            grandmasterPriority1;          
    ClockQuality         grandmasterClockQuality;     
    unsigned char            grandmasterPriority2;        
    char                grandmasterIdentity[8];      
    unsigned short           stepsRemoved;                 
    Enumeration8         timeSource;                   

/* *Note: stepsRemoved is a 16 bit field, but it is 
 * not 16 bit aligned in the announce message
 */
} MsgAnnounce;

/** IEEE 1588 and IEEE 802.1AS PTP Version 2 common Message header structure */
typedef struct 
{
    unsigned char transportSpecificAndMessageType;       // 00       1 (2 4-bit fields)
    unsigned char reserved1AndVersionPTP;                // 01       1 (2 4-bit fields)
    unsigned short messageLength;                         // 02       2
    unsigned char    domainNumber;                          // 04       1
    unsigned char    reserved2;                             // 05       1
    char        flags[2];                              // 06       2
    signed long long    correctionField;                       // 08       8
    unsigned int   reserved3;                             // 16       4
    PortIdentity sourcePortId;                          // 20      10
    unsigned short   sequenceId;                            // 30       2
    unsigned char    control;                               // 32       1
    unsigned char    logMeanMessageInterval;                // 33       1
} volatile V2MsgHeader;

#define CLOCK_IDENTITY_LENGTH  8
#define PTP_CLK_VERSION_NO     2
#define PTP_CLK_DOMAIN_NO      0 
#define PTP_CLK_IDENTITY       0
#define CLOCK_IDENTITY_LENGTH  8
#define PTP_MSG_DELAY_REQ      1

#define PTP_DELAY_REQ_LENGTH   44

#define CCNGEL_PUNT_TSEC 2

struct ptphdr {
    uint8_t msgtype;
    uint8_t version;
    uint16_t length;
    uint8_t domain_number;
    uint8_t reserved0;
    uint16_t flag;
    uint64_t correction;
    uint32_t reserved1;
    uint64_t source_port_id;
    uint16_t port_number;
    uint16_t sequence_id;
    uint8_t control;
    uint8_t log_msgInterval;
} __attribute__ ((__packed__));


struct ptpdata {
    uint16_t second_mSB;
    uint32_t second_lSB;
    uint32_t nanosecond;
} __attribute__ ((__packed__));

#define DEFAULT_PTP_ENCAP_10G (VTSS_PHY_TS_ENCAP_ETH_IP_PTP)
#define DEFAULT_PTP_MAX_FLOWS_10G (2)
#define PTP_EVENT_MSG_PORT (319) /* Ox13F UDP Dest Port */

#define VTSS_PHY_TS_PTP_IPv4_MAX_FLOWS (2)
#define VTSS_PHY_TS_PTP_IPv4_MAX_ACTIONS (1)

BOOL vsc_1gphy_ts_upper_shared_port(u32 port_num);


#define DEFAULT_Y1731_ETH_OAM_IN_ENGINE_ID (VTSS_PHY_TS_OAM_ENGINE_ID_2B)
#define DEFAULT_Y1731_ETH_OAM_EG_ENGINE_ID (VTSS_PHY_TS_OAM_ENGINE_ID_2B)
#define DEFAULT_Y1731_ETH_OAM_ENCAP (VTSS_PHY_TS_ENCAP_ETH_OAM)
#define DEFAULT_Y1731_ETH_OAM_ENGINE_ID (VTSS_PHY_TS_OAM_ENGINE_ID_2B)
#define DEFAULT_Y1731_ETH_OAM_FLOW_START_IDX (0)
#define DEFAULT_Y1731_ETH_OAM_FLOW_END_IDX (2)
#define DEFAULT_Y1731_ETH_OAM_ETH_TYPE (0x8902)


dev_status 
vsc_1gphy_ts_ptp_init(dev_object_t *dev);

dev_status 
vsc_10gphy_ts_ptp_init(dev_object_t *dev);

dev_status
vsc_10gphy_ts_ptp_engine_alloc(dev_object_t *dev,
                               vtss_phy_ts_engine_t in_eng_id,
                               vtss_phy_ts_engine_t eg_eng_id);

dev_status 
vsc_10gphy_ts_ptp_engine_conf(dev_object_t *dev,
                              vtss_phy_ts_engine_t in_eng_id,
                              vtss_phy_ts_engine_t eg_eng_id);

vtss_phy_ts_fifo_sig_mask_t
init_10g_default_sig_mask_conf(void);

dev_status
vsc_1gphy_ts_ptp_engine_alloc(dev_object_t *dev,
                              vtss_phy_ts_engine_t in_eng_id,
                              vtss_phy_ts_engine_t eg_eng_id);

dev_status 
vsc_1gphy_ts_ptp_engine_conf(dev_object_t *dev,
                             vtss_phy_ts_engine_t in_eng_id,
                             vtss_phy_ts_engine_t eg_eng_id);

vtss_phy_ts_fifo_sig_mask_t
init_1g_default_sig_mask_conf(void);

dev_status 
vsc_phy_ts_1g_load_tod (dev_object_t *dev, u32 sec_hi,
                        u32 sec_lo, u32 nsecs);
dev_status 
vsc_phy_ts_10g_load_tod (dev_object_t *dev, u32 sec_hi,
                         u32 sec_lo, u32 nsecs);

dev_status
vsc_phy_ts_init(dev_object_t *dev, 
                BOOL mode,
                vtss_phy_ts_init_conf_t *conf);

dev_status
vsc_phy_ts_eng_init(dev_object_t *dev, 
                    vtss_phy_ts_engine_t in_eng_id,
                    vtss_phy_ts_engine_t eg_eng_id,
                    u8 in_flow_st,
                    u8 in_flow_end,
                    u8 eg_flow_st,
                    u8 eg_flow_end,
                    vtss_phy_ts_engine_flow_match_t match_mode,
                    vtss_phy_ts_encap_t encap,
                    dev_vsc_ts_dir_t dir);
dev_status
vsc_phy_ts_eng_clear(dev_object_t *dev, 
                     vtss_phy_ts_engine_t in_eng_id,
                     vtss_phy_ts_engine_t eg_eng_id,
                     dev_vsc_ts_dir_t dir);
dev_status 
vsc_phy_ts_fifo_sig_mask(dev_object_t *dev,
                         vtss_phy_ts_fifo_sig_mask_t mask);
dev_status
vsc_phy_ts_conf_set(dev_object_t *dev, 
                    vtss_phy_ts_engine_t in_eng_id,
                    vtss_phy_ts_engine_t eg_eng_id,
                    vtss_phy_ts_engine_flow_conf_t *in_eng_conf,
                    vtss_phy_ts_engine_flow_conf_t *eg_eng_conf,
                    dev_vsc_ts_dir_t dir);

dev_status
vsc_phy_ts_action_set(dev_object_t *dev, 
                    vtss_phy_ts_engine_t in_eng_id,
                    vtss_phy_ts_engine_t eg_eng_id,
                    vtss_phy_ts_engine_action_t *in_eng_conf,
                    vtss_phy_ts_engine_action_t *eg_eng_conf,
                    dev_vsc_ts_dir_t dir);

dev_status 
vsc_phy_ts_1g_read_tod(dev_object_t *dev, u16 *sec_hi,
                       u32 *sec_lo, u32 *nsecs);
dev_status 
vsc_phy_ts_10g_read_tod(dev_object_t *dev, u16 *sec_hi,
                        u32 *sec_lo, u32 *nsecs);
typedef void (*ts_fifo_read_cb_fn) (const vtss_inst_t inst,
                                 const vtss_port_no_t port_no,
                                 const vtss_phy_timestamp_t *const ts,
                                 const vtss_phy_ts_fifo_sig_t *const sig,
                                 void  *cntxt,
                                 const vtss_phy_ts_fifo_status_t status);

dev_status
vsc_phy_ts_fifo_read_install(dev_object_t *dev,
                             ts_fifo_read_cb_fn read_cb_fn);

dev_status 
vsc_phy_ts_fifo_read_from_phy(dev_object_t *dev);

dev_status
vsc_phy_ts_ptp_ipv4_flow_modify(dev_object_t *dev,
                                BOOL en, vtss_phy_ts_engine_t eng_id,
                                dev_vsc_ts_ip_comp_t comp_id,
                                u8 flow_idx, dev_vsc_ts_dir_t dir,
                                u32 addr, u32 mask);

dev_status
vsc_phy_ts_ptp_action_modify(dev_object_t *dev,
                             BOOL en, vtss_phy_ts_engine_t eng_id,
                             u8 flow_idx, dev_vsc_ts_dir_t dir,
                             u8 domain);

dev_status 
vsc_phy_ts_reg_get(dev_object_t *dev, u32 blk_id, u16 addr, u32 *value);

dev_status
vsc_10gphy_ts_y1731_eth_oam_init_wrapper(dev_object_t *dev);

dev_status
vsc_1gphy_ts_y1731_eth_oam_init_wrapper(dev_object_t *dev);

BOOL vsc_1gphy_ts_upper_shared_port(u32 port_num);

dev_status
vsc_phy_ts_oam_y1731_def_eth_oam_flow_modify(dev_object_t *dev,
                         BOOL en,
                         vtss_phy_ts_engine_t eng_id,
                         u8 flow_idx,
                         dev_vsc_ts_dir_t dir,
                         u8 intf_mac[],
                         u8 bb_mac[]);

dev_status
vsc_phy_ts_init_new_spi_mode(dev_object_t *dev);

dev_status 
vsc_phy_ts_load_tod_done(dev_object_t *dev, u32 port_num);

int vsc_1g_quietly_read_all_regs(unsigned int port, int verbose);

// int vsc_1g_qsgmii_set_aneg(unsigned int port, int enable);
// int vsc_1g_qsgmii_set_aneg_on_all_ports (int enable);
// int vsc_1g_qsgmii_aneg_setup(unsigned int port, int lpbk);
// int ccngel_phy_port_squelch_recovered_clock(int port_number, int squelch);

extern int mdio_c22_read (unsigned int bus_no, unsigned int phy_adr, int DevId,
                       unsigned short regAddress, unsigned short *value);

extern int mdio_c22_write (unsigned int bus_no, unsigned int phy_adr, int DevId,
                        unsigned short regAddress, unsigned short value);

extern int mdio_c45_read (unsigned int bus_no, unsigned int phy_adr, int DevId,
                       unsigned short regAddress, unsigned short *value);

extern int mdio_c45_write (unsigned int bus_no, unsigned int phy_adr, int DevId,
                        unsigned short regAddress, unsigned short value);

extern int mdio_c45_read_inc (unsigned int bus_no, unsigned int phy_adr, int DevId, 
                           unsigned short phy_reg, unsigned short *buf, 
                  unsigned char count, int mode);


extern time_t time(time_t *tloc);

extern int vsc_1588_reg_access(u32, u32, u32, u32, u32*);
extern int vsc_1588_reg_dump(u32);
extern int vsc_1588_tod_access(int, int, u16, u32, u32);
extern int vsc_phy_1588_stats(int);
extern int vsc_load_timestamp(void);

extern int wallander_init_all_phy_ports(boolean, boolean, int);
extern int wallander_set_phy_loopback(int, BOOL, BOOL);

#endif /* __WALLANDER_PHY_H__ */    
