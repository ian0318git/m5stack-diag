/* $Id: dev_98dxc25x.h,v 1.2 2021/09/24 01:22:18 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_98dxc25x_marvell/dev_98dxc25x.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : dev_98dxc25x.h
 *
 * Description: Marvell 98dxc25x ESW device driver.
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __DEV_98DXC25X_H__
#define __DEV_98DXC25X_H__



#include "dev_object.h"
#include "dev_cpss42.h"


#define PORT_0_IRUPT_CAUSE_REGISTER      0x0a800020
#define PORT_0_IRUPT_MASK_REGISTER       0x0a800024
#define PORT_IRUPT_OFFSET                0x400

#define GPP_INPUT_REG_OFFSET             0x018001bc
#define GPP_IO_CTRL_REG_OUTPUT           0x018001c4
#define GPP_IO_CTRL_REG_OFFSET           0x018001c8

#define XCAT5_1000MS    1000
#define XCAT5_200MS    200


#define GPIO_DATA_OUT_SET_REG    0x18130
#define GPIO_DATA_OUT_CLEAR_REG    0x18134


/* For AC5 Switch */
#define DATAOUT_ENABLE_CONTROL_REG    0x7F018104
#define DATAOUT_ENABLE                0xFDFFFFFF
#define DATAOUT_DISABLE               0xFFFFFFFF

#define DATAOUT_REG   0x7F018100
#define MPP25_HIGH    0x2000000
#define MPP25_LOW     0x0

/* Common */
#define XCAT5_PORT_NUM 24

#define MRV98DXC25X_ERR_BUF_SIZE    (80)


typedef enum mrvl_98dxc25x_link_status_t_ {
    LINK_UP,
    LINK_DOWN
} mrvl_98dxc25x_link_status_t;


typedef struct dev_98dxc25x_callin_fvt_t_ {
    int (*esw_xcat5_init)(dev_object_t *, uint);                  /* xcat5 Init Function */
    int (*esw_clear_all_port_interrupt)(dev_object_t *, uint);    /* Clear all port interrupt */
    int (*esw_sw_gpp_init)(dev_object_t *, uint);                 /* GPP gpio init */
    int (*esw_config_port_pve)(dev_object_t *, uint, uint, uint);  /* Esw config port pve */
    int (*esw_unconfig_port_pve)(dev_object_t *, uint, uint, uint);  /* Esw unconfig port pve */
    int (*esw_config_port_pve_single_direction)(dev_object_t *, uint, uint, uint);  /* Esw config port pve single direction */
    int (*esw_unconfig_port_pve_single_direction)(dev_object_t *, uint, uint, uint);  /* Esw unconfig port pve single direction*/
    int (*esw_print_mac_counter)(dev_object_t *, uint, uint);    /* Esw print mac counter */
    int (*esw_port_force_link_set)(dev_object_t *, uint, mrvl_98dxc25x_link_status_t, int, boolean);    /* Esw port force link set */
    int (*esw_xcat5_all_reg_test)(dev_object_t *, uint);    /* Esw all register test */
    int (*esw_pcie_config_read_util)(dev_object_t *);    /* Esw pcie config read util */
    int (*esw_pcie_config_write_util)(dev_object_t *);    /* Esw pcie config write util */
    int (*esw_xcat5_reg_read_util)(dev_object_t *, uint);    /* Esw xcat5 register read util */
    int (*esw_xcat5_reg_write_util)(dev_object_t *, uint);    /* Esw xcat5 register write util */
    int (*esw_xcat5_internal_reg_read_util)(dev_object_t *, uint);    /* Esw xcat5 internal register read util */
    int (*esw_xcat5_internal_reg_write_util)(dev_object_t *, uint);    /* Esw xcat5 internal register write util */
    int (*esw_xcat5_gen_int)(dev_object_t *, uint);    /* Esw xcat5 generate interrupt */
    int (*esw_xcat5_clear_int)(dev_object_t *, uint);    /* Esw xcat5 clear interrupt */
    int (*esw_xcat5_led_test)(dev_object_t *, uint);    /* Esw xcat5 LED test */
    int (*esw_config_pcs_loopback)(dev_object_t *, uint, uint);  /* Esw config pcs loopback */
    int (*esw_unconfig_pcs_loopback)(dev_object_t *, uint, uint);  /* Esw unconfig pcs loopback */
    int (*esw_xcat5_exit)(dev_object_t *, uint, MAD_DEV *);    /* Esw xcat5 exit */
    int (*esw_xcat5_phy_port_init)(dev_object_t *, uint, uint);    /* Esw xcat5 & phy init port set */
	 
} dev_98dxc25x_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                 platform
 */
typedef struct dev_98dxc25x_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    void (*reg_config_rd)(uint, uint, uint *);
    void (*reg_config_wr)(uint, uint, uint);
    int (*smi_phy_init) (uint);
    int (*global_enable_pve) (uint);
    int (*global_disable_pve) (uint);
    int (*set_port_pve_singel_direction) (uint, uint, uint);
    int (*clear_port_pve_singel_direction) (uint, uint, uint);
    int (*port_enable) (uint, uint);
    int (*force_link_down_en) (uint, uint, boolean);
    int (*force_link_pass_en) (uint, uint, boolean);
    void (*pcie_config_read) (int, uint *);
    void (*pcie_config_write) (int, uint);
    int (*led_class_config) (uint, uint32_t, boolean, boolean, int, boolean, uint32_t);
    int (*pcs_loopback_enable) (uint, uint);
    int (*pcs_loopback_disable) (uint, uint);
    int (*xcat5_exit) (MAD_DEV *);
    int (*cpss_pp_phase1_info_init) (CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *);
    int (*cpss_pp_phase2_info_init) (uint, CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *);
    int (*xcat5_specific_port_init) (uint);
    int (*xcat5_specific_port_enable) (uint);

} dev_98dxc25x_callout_fvt_t;

typedef enum {
    DEV_98DXC25X_DEV_STATE = 0,
    DEV_98DXC25X_ATTACH,
    DEV_98DXC25X_DETACH,
    DEV_98DXC25X_INIT,
    DEV_98DXC25X_SHOW,
    DEV_98DXC25X_DESTROY,
    DEV_98DXC25X_ALTER,
    DEV_98DXC25X_ALERT,
    DEV_98DXC25X_DISPLAY,
} dev_98dxc25x_report_code_t;

typedef enum {
    VLAN_PROFILE_1,
    VLAN_PROFILE_END
} dev_98dxc25x_vlan_profile_t;

typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_98dxc25x_phy_setup_t;

/*
 * Define the 98DXC25X device object structure
 */
typedef struct dev_98dxc25x_object_t {
    dev_object_t        base;
    dev_98dxc25x_callin_fvt_t        *callin_fvt;
    dev_98dxc25x_callout_fvt_t       *callout_fvt;
    int        cpss_dev;
    int        port_group;
} dev_98dxc25x_object_t;


enum {
    DEV_98DXC25X_DISABLE,
    DEV_98DXC25X_ENABLE
};

extern void mrv98dxc25x_dev_create(dev_object_t *, dev_error_report_t);

#endif   /* __DEV_98DXC25X_H__ */


/*------------------------------------------------------------------
 *$Log: dev_98dxc25x.h,v $
 *Revision 1.2  2021/09/24 01:22:18  harrchan
 *Collapse Elixir-branch to Main Trunk.
 *
 *Revision 1.1.2.8  2021/05/31 10:39:33  illiu
 *Remove function esw_clear_sw_counter
 *Remove function phy_tx_config_read/write
 *Remove function esw_xcat5_phy_tx_config_read/write
 *Rename function esw_print_sw_counter to esw_print_mac_counter
 *Change return type of some function to void type
 *
 *Revision 1.1.2.7  2021/04/23 02:50:56  illiu
 *1. Add variable cpss_dev as a member of 98dxc25x object
 *2. Add variable port_group as a member of 98dxc25x object
 *3. Remove redundent function
 *
 *Revision 1.1.2.6  2021/04/12 08:56:23  illiu
 *Move some Marvell library's functions from platform code to device driver
 *
 *Revision 1.1.2.5  2021/03/18 08:25:08  illiu
 *1. Add call-in function to do AC5 switch exit process
 *2. Add call-out function to do AC5 switch exit process
 *3. Add call-in function to do AC5 & PHY init port setting
 *4. Add call-out function to do AC5 & PHY init port setting
 *
 *Revision 1.1.2.4  2021/02/04 03:21:59  illiu
 *Clean up code
 *
 *Revision 1.1.2.3  2020/11/05 07:11:11  harrchan
 *1. Add print_port_mac_counter function to print MAC counters
 *
 *Revision 1.1.2.2  2020/11/05 04:19:18  illiu
 *Add test item: xCat5 Interrupt Test
 *
 *$Endlog$
*/

