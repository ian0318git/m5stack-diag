/* $Id: dreamliner_ge_switch.h,v 1.2 2019/12/11 10:10:25 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/dreamliner_ge_switch.h,v $
 *------------------------------------------------------------------
 *
 * dreamliner_ge_switch.h - This file contains all the definitions for
 *                          Marvell GE switch.
 *
 * Christine Wen -- Nov. 2013
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __DREAMLINER_GE_SWITCH_H__ 
#define __DREAMLINER_GE_SWITCH_H__

#include <stdint.h>
#define NUM_MARVELL_CORES     1

#define XCAT2_DEVICE_ID       0xE61E
#define XCAT2_VENDOR_ID       0x11AB

#define XCAT2_PORT_NUM        28
#define XCAT2_USED_PORT       8

#define GE0_XCAT2_PORT        24
#define GE1_XCAT2_PORT        25

#define MRU_802_1Q            1522

#define XCAT2_DEVICE_ID_REG   0x4C
#define XCAT2_VENDOR_ID_REG   0x50

#define VLAN_1                1
#define VLAN_2                2  
#define VLAN_3                3
#define VLAN_4                4
#define VLAN_5                5

#define DEVICE_ID_REGISTER    0x0000004C
#define DEVICE_ID_4P          0xE75A
#define DEVICE_ID_8P          0xE61E

/* define for port interrupt cause register */
#define PORT_0_IRUPT_CAUSE_REGISTER      0x0a800020
#define PORT_0_IRUPT_MASK_REGISTER       0x0a800024
#define PORT_IRUPT_OFFSET                0x400

#define PORT_MAC_CTRL_REG                0x0a800004
#define PORT_MAC_OFFSET                  0x400

#define PORT_PCS_LPBK_EN                 0x0040
                
/* define for GPIO */
#define GPIO_DATA_OUT_REG                0x00010100
#define GPIO_DATA_OUT_ENA_REG            0x00010104
#define GPIO_DATA_IN_POL_REG             0x0001010c
#define GPIO_DATA_IN_REG                 0x00010110
#define GPIO_INTR_CAUSE_REG              0x00010114
#define GPIO_INTR_LEVEL_MASK_REG         0x0001011c
#define GPIO_FPGA_RESET                  24
#define GPIO_FPGA_INT                    25
#define GPIO_PHY_INT                     26
#define GPIO_PHY_RESET                   27
#define GPIO_MODULE_RDY                  30
#define GPIO_DB_PRESENT                  20
#define GPIO_HIGH_DATA_OUT_REG           0x00010140
#define GPIO_HIGH_DATA_OUT_ENA_REG       0x00010144
#define GPIO_LED_CTRL                    32
#define GPIO_VOLTAGE_HIGH                23
#define GPIO_VOLTAGE_LOW                 33


/* bit define for the GPP bits */
#define FPGA_RESET                       0x1
#define GPP_FPGA_INT                     0x2
#define GPP_PHY_INT                      0x4
#define PHY_RESET                        0x8
#define MODULE_READY                     0x10
#define DB_PRESENT                       0x40
#define VOLTAGE_HIGH                     0x200
#define VOLTAGE_LOW                      0x400
#define LED_CTRL                         0x800

#define GPP_INPUT_REG_OFFSET             0x018001bc
#define GPP_IO_CTRL_REG_OUTPUT           0x018001c4
#define GPP_IO_CTRL_REG_OFFSET           0x018001c8

/* define for SMI configuration */
#define PHY_ADDR_REG0_OFF                0x04004030
#define PHY_AUTO_NEG_REG0_OFF            0x04004034
#define SMI0_MANAGE_REG_OFF              0x04004054
#define LMS0_MISC_CONFIG_REG_OFF         0x04004200

#define PHY_ADDR_REG1_OFF                0x04804030
#define PHY_AUTO_NEG_REG1_OFF            0x04804034

#define PHY_ADDR_REG2_OFF                0x05004030
#define PHY_AUTO_NEG_REG2_OFF            0x05004034
#define SMI1_MANAGE_REG_OFF              0x05004054
#define LMS1_MISC_CONFIG_REG             0x05004200

#define PHY_ADDR_REG3_OFF                0x05804030
#define PHY_AUTO_NEG_REG3_OFF            0x05804034

/* bit define for auto-negotiation configuration register */
#define AUTOMEDIA_SEL_EN_P0              0x1
#define AUTOMEDIA_SEL_EN_P1              0x2
#define AUTOMEDIA_SEL_EN_P2              0x4
#define AUTOMEDIA_SEL_EN_P3              0x8
#define AUTOMEDIA_SEL_EN_P4              0x10
#define AUTOMEDIA_SEL_EN_P5              0x20
#define STOP_AUTO_NEG                    0x80

/* bit define for SMI management register */
#define SMI_BUSY                         0x10000000
#define READ_VALID                       0x08000000
#define SMI_READ                         0x04000000
#define REG_ADDR_SHIFT                   21
#define PHY_ADDR_SHIFT                   16
#define SMI_CHECK_STATUS_RETRY           4000
#define SMI_READ_RETRY                   5000
  
#define SMI_ADDR_SHIFT                   5

#define XCAT2_PCI_VEN_DEV_ID_OFFSET          0x00
#define XCAT2_PCI_COMMAND_OFFSET             0x04
#define XCAT2_PCI_CLASS_REV_OFFSET           0x08
#define XCAT2_PCI_BIST_CACHELINE_OFFSET      0x0C
#define XCAT2_PCI_BAR0_LOW_OFFSET            0x10
#define XCAT2_PCI_BAR0_HIGH_OFFSET           0x14
#define XCAT2_PCI_BAR1_LOW_OFFSET            0x18
#define XCAT2_PCI_BAR1_HIGH_OFFSET           0x1C
#define XCAT2_PCI_BAR2_LOW_OFFSET            0x20
#define XCAT2_PCI_BAR2_HIGH_OFFSET           0x24

typedef enum {
    LINK_DOWN = 0,
    LINK_UP,
} port_link_status_t;
    
typedef struct _xcat2_info {
    uint32_t dev_found;
    uint32_t bist_cache_line;
    uint32_t bar0_addrlow;
    uint32_t bar0_addr_high;
    uint32_t bar1_addr_low;
    uint32_t bar1_addr_high;
    uint64_t virtual_bar0_addr;
} xcat2_info;

/* Voltage Margin enums */
typedef enum {
    VTG_MRGN_SET_3_3V_NORM,  /* Set 3.3V Margin to normal */
    VTG_MRGN_SET_3_3V_HI,    /* Set 3.3V Margin to high */
    VTG_MRGN_SET_3_3V_LO,    /* Set 3.3V Margin to low */
} dl_voltage_margin_t;


extern void dl_pcie_config_read(int offset, uint32_t *reg_ptr);
extern void dl_pcie_config_write(int offset, uint32_t reg_data);
extern int xcat2_reg_pci_read(uint32_t offset, uint32_t *data);
extern int xcat2_reg_pci_write(uint32_t offset, uint32_t data);
extern int smi0_read_reg(unsigned int port_num, unsigned int reg_addr, unsigned short *reg_data);
extern int smi0_write_reg(unsigned int port_num, unsigned int reg_addr, unsigned short reg_data);
extern int smi1_read_reg(unsigned int reg_addr, unsigned short *reg_data);
extern int smi1_write_reg(unsigned int reg_addr, unsigned short reg_data);
extern uint32_t xcat2_clear_all_port_interrupt(void);
extern uint32_t xcat2_port_enable(uint32_t dev_num, uint32_t port_num);
extern uint32_t xcat2_config_port_pve(uint8_t dev_num, uint8_t src_port, uint8_t dst_port);
extern uint32_t xcat2_unconfig_port_pve(uint8_t dev_num, uint8_t src_port, uint8_t dst_port);
extern uint32_t xcat2_port_mac_loopback_enable(uint32_t dev_num, uint32_t port_num);
extern uint32_t xcat2_port_mac_loopback_disable(uint32_t dev_num, uint32_t port_num);
extern uint32_t xcat2_soft_reset(uint32_t dev_num);
extern uint32_t xcat2_vlan_add(uint32_t dev_num, uint32_t vlan_id);
extern uint32_t xcat2_vlan_port_add(uint32_t dev_num, uint32_t vlan_id, uint32_t port_num);
extern uint32_t xcat2_vlan_port_del(uint32_t dev_num, uint32_t vlan_id, uint32_t port_num);
extern uint32_t xcat2_vlan_port_show(uint32_t dev_num, uint32_t port_num);

extern int get_phy_intr_pin (boolean *phy_intr);
extern int get_poe_intr_pin (boolean *poe_intr);
extern int sw_set_poe_int (boolean ena);
extern int sw_set_phy_int (boolean ena, uint64_t *handler);
extern int sw_wait_phy_int (uint64_t handler);
extern int sw_check_poe_int (boolean *intr);
extern int sw_check_phy_int (boolean *intr);
extern int port_force_link_set(port_link_status_t link, int port, boolean set);
extern void print_sw_counter();
extern void clear_sw_counter();

extern int phy_in_reset();
extern int phy_out_of_reset();
extern int xcat2_reset();

extern unsigned long dl_get_pci_base_addr();

#endif /* __DREAMLINER_GE_SWITCH_H__ */
/*
 *------------------------------------------------------------------
 * $Log: dreamliner_ge_switch.h,v $
 * Revision 1.2  2019/12/11 10:10:25  lucywang
 * Merged Nanook to main trunk
 *
 * 
 *------------------------------------------------------------------
 * $Endlog$
 */
