/* $Id: dev_98dxc323.h,v 1.2 2019/12/11 10:10:22 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_98dxc323_marvell/dev_98dxc323.h,v $
 *------------------------------------------------------------------
 * 
 * Filename   : dev_98dxc323.h
 * Description: Marvell 98DXC323 ESW device driver.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __DEV_98DXC323_H__
#define __DEV_98DXC323_H__



#include "dev_object.h"

#define PORT_0_IRUPT_CAUSE_REGISTER      0x0a800020
#define PORT_0_IRUPT_MASK_REGISTER       0x0a800024
#define PORT_IRUPT_OFFSET                0x400

#define GPP_INPUT_REG_OFFSET             0x018001bc
#define GPP_IO_CTRL_REG_OUTPUT           0x018001c4
#define GPP_IO_CTRL_REG_OFFSET           0x018001c8

#define XCAT3_1000MS    1000
#define XCAT3_200MS    200

#define MPP_CONTROL_8_15_REG    0x18004
#define MPP_CONTROL_MASK_MPP13    0xFF0FFFFF

#define GPIO_DATA_OUT_EN_REG    0x18104
#define GPIO_DATA_OUT_SET_REG    0x18130
#define GPIO_DATA_OUT_CLEAR_REG    0x18134

#define GPIO_DATA_OUT_MPP13_BIT    (1 << 13)




/* Common */
#define XCAT3_PORT_NUM 24

#define MRV98DXC323_ERR_BUF_SIZE    (80)


typedef enum mrvl_98dxc323_link_status_t_ {
    LINK_UP,
    LINK_DOWN
} mrvl_98dxc323_link_status_t;


typedef struct dev_98dxc323_callin_fvt_t_ {
    int (*esw_xcat3_init)(dev_object_t *, uint);                                            /*xcat3 Init Function */
    int (*esw_xcat3_init_post)(dev_object_t *, uint);                                            /*xcat3 Init Function */
    int (*esw_clear_all_port_interrupt)(dev_object_t *, uint);                 /* Clear all port interrupt */
    int (*esw_sw_gpp_init)(dev_object_t *, uint);                 /* GPP gpio init */
    int (*esw_config_port_pve)(dev_object_t *, uint, uint, uint);  /*Esw config port pve */
    int (*esw_unconfig_port_pve)(dev_object_t *, uint, uint, uint);  /*Esw unconfig port pve */
    int (*esw_config_port_pve_single_direction)(dev_object_t *, uint, uint, uint);  /*Esw config port pve single direction */
    int (*esw_unconfig_port_pve_single_direction)(dev_object_t *, uint, uint, uint);  /*Esw unconfig port pve single direction*/
    int (*esw_print_sw_counter)(dev_object_t *, uint, uint);    /*Esw print sw counter */
    int (*esw_clear_sw_counter)(dev_object_t *, uint, uint);    /*Esw clear sw counter */
    int (*esw_port_force_link_set)(dev_object_t *, uint, mrvl_98dxc323_link_status_t, int, boolean);    /*Esw port force link set */
    int (*esw_xcat3_all_reg_test)(dev_object_t *, uint);    /* Esw all register test */
    int (*esw_pcie_config_read_util)(dev_object_t *);    /* Esw pcie config read util */
    int (*esw_pcie_config_write_util)(dev_object_t *);    /* Esw pcie config write util */
    int (*esw_xcat3_reg_read_util)(dev_object_t *, uint);    /* Esw xcat3 register read util */
    int (*esw_xcat3_reg_write_util)(dev_object_t *, uint);    /* Esw xcat3 register write util */
    int (*esw_xcat3_internal_reg_read_util)(dev_object_t *, uint);    /* Esw xcat3 internal register read util */
    int (*esw_xcat3_internal_reg_write_util)(dev_object_t *, uint);    /* Esw xcat3 internal register write util */
    int (*esw_xcat3_gen_int)(dev_object_t *, uint);    /* Esw xcat3 generate interrupt */
    int (*esw_xcat3_clear_int)(dev_object_t *, uint);    /* Esw xcat3 clear interrupt */
    int (*esw_xcat3_led_test)(dev_object_t *, uint);    /* Esw xcat3 LED test */
    int (*esw_xcat3_10g_kr_test_mode)(dev_object_t *, uint, uint, uint, uint);    /* Esw xcat3 10g kr test mode */
    int (*esw_xcat3_serdes_tx_config_read)(dev_object_t *);    /* Esw xcat3 serdes tx config read */
    int (*esw_xcat3_serdes_tx_config_write)(dev_object_t *);    /* Esw xcat3 serdes tx config write */ 
    int (*esw_xcat3_phy_tx_config_read)(dev_object_t *);    /* Esw xcat3 phy tx config read */
    int (*esw_xcat3_phy_tx_config_write)(dev_object_t *);    /* Esw xcat3 phy tx config write */ 
	 
} dev_98dxc323_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                 platform
 */
typedef struct dev_98dxc323_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    int (*rd)(int, ushort *);
    int (*wr)(int, ushort);
    int (*sgmii_lpbk_test)(void);
    int (*chk_intr_assert) (void);
    int (*chk_intr_deassert) (void);
    int (*cpss_driver_init) (uint);
    int (*cpss_device_init) (uint);
    int (*reg_pci_rd)(uint, uint, uint *);
    int (*reg_pci_wr)(uint, uint, uint);
    int (*reg_config_rd)(uint, uint, uint *);
    int (*reg_config_wr)(uint, uint, uint);
    int (*led_init) (uint);
    int (*smi_phy_init) (uint);
    int (*global_enable_pve) (uint);
    int (*global_disable_pve) (uint);
    int (*set_port_pve) (uint, uint, uint);
    int (*clear_port_pve) (uint, uint, uint);
    int (*set_port_pve_singel_direction) (uint, uint, uint);
    int (*clear_port_pve_singel_direction) (uint, uint, uint);
    int (*port_enable) (uint, uint);
    int (*port_disable) (uint, uint);
    int (*port_mac_loopback_enable) (uint, uint);
    int (*port_mac_loopback_disable) (uint, uint);
    int (*soft_reset) (uint);
    int (*vlan_add) (uint, uint, uint);
    int (*vlan_port_add) (uint, uint, uint);
    int (*vlan_port_del) (uint, uint, uint);
    int (*vlan_port_show) (uint, uint);
    int (*force_link_down_en) (uint, uint, boolean);
    int (*force_link_pass_en) (uint, uint, boolean);
    int (*xcat3_all_reg_test) (uint, uint *, uint *, uint *, uint *);
    int (*pcie_config_read) (int, uint *);
    int (*pcie_config_write) (int, uint);
    int (*led_class_config) (uint, uint32_t, boolean, boolean, int, boolean, uint32_t);
    int (*set_10g_kr_test_mode) (uint, uint, uint, uint);
    int (*serdes_tx_config_read) (void);
    int (*serdes_tx_config_write) (void);
    int (*phy_tx_config_read) (void);
    int (*phy_tx_config_write) (void);
	
	
} dev_98dxc323_callout_fvt_t;

typedef enum {
    DEV_98DXC323_DEV_STATE = 0,
    DEV_98DXC323_ATTACH,
    DEV_98DXC323_DETACH,
    DEV_98DXC323_INIT,
    DEV_98DXC323_SHOW,
    DEV_98DXC323_DESTROY,
    DEV_98DXC323_ALTER,
    DEV_98DXC323_ALERT,
    DEV_98DXC323_DISPLAY,
} dev_98dxc323_report_code_t;

typedef enum {
    VLAN_PROFILE_1,
    VLAN_PROFILE_END
} dev_98dxc323_vlan_profile_t;

typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_98dxc323_phy_setup_t;

/*
 * Define the 98DXC323 device object structure
 */
typedef struct dev_98dxc323_object_t {
    dev_object_t        base;
    dev_98dxc323_callin_fvt_t        *callin_fvt;
    dev_98dxc323_callout_fvt_t       *callout_fvt;
} dev_98dxc323_object_t;


enum {
    DEV_98DXC323_DISABLE,
    DEV_98DXC323_ENABLE
};

extern void mrv98dxc323_dev_create(dev_object_t *, dev_error_report_t);

#endif   /* __DEV_98DXC323_H__ */


/*------------------------------------------------------------------
 *$Log: dev_98dxc323.h,v $
 *Revision 1.2  2019/12/11 10:10:22  lucywang
 *Merged Nanook to main trunk
 *
 *
 *$Endlog$
*/

