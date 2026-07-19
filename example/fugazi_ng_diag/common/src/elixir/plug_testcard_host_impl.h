/* $Id: plug_testcard_host_impl.h,v 1.2 2021/09/24 01:21:08 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/plug_testcard_host_impl.h,v $
 *------------------------------------------------------------------
 * 
 * plug_testcard_host_impl.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_TESTCARD_HOST_IMPL__
#define __PLUG_TESTCARD_HOST_IMPL__

#define SEC_TO_MICROSEC         1000000.0
#define MAX_CHECKTIME_USEC      1000000   /* 1sec */
#define MAX_POLLING_COUNTS      100
#define POLLING_INTRVL          100 /* 100ms */
#define MAX_TRY                 5

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO        0
#define I2C_CTRL_ONE         1
#define I2C_CTRL_TWO         2
#define I2C_CTRL_THREE       3


/* Pluggable I2C Device Address */
#define PLUG_I2C_ADDR_TEMP             (0x9C >> 1)    
#define PLUG_I2C_ADDR_ACT2             (0xE6 >> 1)
#define PLUG_TC_I2C_ADDR_GPIO_EXP      (0x3E >> 1)   /* Pluggable Test Card GPIO Expander */
#define PLUG_TC_I2C_ADDR_PHY           (0xB8 >> 1)   /* Pluggable Test Card 88E1112 PHY */
#define PLUG_MAN_I2C_ADDR_GPIO_EXP     (0x4E >> 1)   /* Pluggable LTE Mandatory GPIO Expander */
#define PLUG_OPT_I2C_ADDR_GPIO_EXP     (0x4C >> 1)   /* Pluggable LTE Optional GPIO Expander */


/* Common definition for MUX */
#define I2C_MUX_ZERO         0
#define I2C_MUX_ONE          1
#define I2C_MUX_TWO          2
#define I2C_MUX_THREE        3
#define USB_SLOT0       0
#define USB_SLOT1       1
#define USB_SLOT2       2


/* Common */
#define GEWAN_TXTYPE_A   1
#define GEWAN_TXTYPE_B   0


/* Based on Star Test card mapping,
 * GE0 PHY: eth2
 */
/* Ethernet definition */
#define GE0               0
#define GE1               1
#define GE0_ETHNUM        ETH0
#define GE1_ETHNUM        ETH2
#define PLUG_GE0              GE0
#define PLUG_GE1              GE1
#define PLUG_GE0_ETHNUM       GE0_ETHNUM
#define PLUG_GE1_ETHNUM       GE1_ETHNUM

#define PLUG_TC_FPGA_OFFSET                (0x10000)
#define PLUG_TC_I2C_CTRL_OFFSET            (PLUG_TC_FPGA_OFFSET + 0x2000)
#define PLUG_TC_FPGA_I2C_OFFSET            (0x100)
#define PLUG_TC_USB_2P0_BUS_NUMBER         (3)
#define PLUG_TC_USB_3P0_BUS_NUMBER         (4)
#define PLUG_TC_USB_C1101_LEV_NUMBER       (1)
#define PLUG_TC_USB_C1109_LEV_NUMBER       (2)
#define PLUG_TC_USB_SLOT1_PORT_NUMBER      (0)
#define PLUG_TC_USB_SLOT2_PORT_NUMBER      (1)

#define SEL_PORT_ETH "eth"
#define PIM_NVME_DEV                    "/dev/nvme0n1"

/* Port Auto-Negotiation Config Reg(m = 0-3) */
#define CPU_PORT_AN_CONF_REG(m)        (uint)(0xF2130E0C + (m*0x1000))
#define CPU_PACR_F_LINKUP         (uint)(1 << 1)
#define CPU_PACR_F_LINKDOWN       (uint)(1)

/* Digital Loopback Enable Register(n = 0-5) */
#define CPU_DIGITAL_LOOPBACK_ENABLE_REG(n)	(0xF212088C + (n*0x1000))
#define CPU_LOCAL_DIG_RX2TX_LPBK_EN			0x8000	/* (1 << 15) */
#define CPU_RXPHER_TO_TX_EN					0x1000	/* (1 << 12) */

#define PANCR_RESERVED            0x8000   /* 1 << 15 */
#define PANCR_AUTOMEDIA_SEL_EN    0x4000   /* 1 << 14 */
#define PANCR_AN_DUPLEX_EN        0x2000   /* 1 << 13 */
#define PANCR_SET_FULL_DUPLEX     0x1000   /* 1 << 12 */
#define PANCR_AN_FC_EN            0x0800   /* 1 << 11 */
#define PANCR_SUPPORT_FC          0x0200   /* 1 <<  9 */
#define PANCR_AN_SPEED_EN         0x0080   /* 1 <<  7 */
#define PANCR_SET_SGMII_1000      0x0040   /* 1 <<  6 */
#define PANCR_SET_MII_100         0x0020   /* 1 <<  5 */
#define PANCR_INBAND_RESTART_AN   0x0010   /* 1 <<  4 */
#define PANCR_INBAND_BYPASS_EN    0x0008   /* 1 <<  3 */
#define PANCR_INBAND_AN_EN        0x0004   /* 1 <<  2 */
#define PANCR_FORCE_LINK_UP       0x0002   /* 1 <<  1 */
#define PANCR_FORCE_LINK_DOWN     0x0001   /* 1 <<  0 */

#define PANCR_FORCE_LINK_MSK      0x0003   /* bit 0 and bit 1 */

#define PIM_PCIE_NVME_VID         0x1344
#define PIM_PCIE_NVME_DID         0x6001

#define PLUG_TESTCARD_WAIT_NVME   1000
#define PLUG_TESTCARD_CHECK_NVME  50
#define PLUG_PCI_DEVICE          "/sys/bus/pci/devices/0001:01:00.0/vendor"


extern void plug_tc_host_reply_usb_bus_lev_port_info(int, int, int *, int *, int *, int *, int);
extern int plug_tc_host_tx_rx_diag(char*, int, int,int, int, int);
extern int plug_tc_host_ge_send_packet_util(int);
extern int plug_tc_host_gephy_set_auto_neg(void);
extern int plug_tc_host_gephy_set_1000_speed(void);
extern int plug_tc_host_gephy_set_test_speed(int);
extern int plug_tc_host_check_ext_lpbk_flag(void);
extern int plug_tc_host_reply_geport_ethnum(int, int *);
extern void plug_tc_host_get_eth_interface_info(int, char *);
extern int plug_tc_host_usb_hub_menu_flag(int);
extern int plug_tc_host_pcie_present(int);

/* Platfrom local function */                                                                        
extern int ge_send_packet_util(int);
extern int gephy_set_default(int);
extern int check_ext_lpbk_flag(void);
extern int plug_tc_host_sgmii_present(int);

#endif

/*-------------------------------------------------
 * $Log: plug_testcard_host_impl.h,v $
 * Revision 1.2  2021/09/24 01:21:08  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2020/11/10 06:43:20  harrchan
 * Support PCIe test card on elixir
 *
 * Revision 1.1.2.1  2020/09/09 09:09:54  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:29  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
