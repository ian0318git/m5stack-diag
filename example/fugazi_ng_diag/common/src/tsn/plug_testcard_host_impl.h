/* $Id: plug_testcard_host_impl.h,v 1.2 2018/11/23 08:49:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_testcard_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_host_impl.h - Header file for Host implement Pluggable Test card function
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_TESTCARD_HOST_IMPL__
#define __PLUG_TESTCARD_HOST_IMPL__

/* Common definition for I2C controller */
#define I2C_CTRL_ZERO        0
#define I2C_CTRL_ONE         1
#define I2C_CTRL_TWO         2
#define I2C_CTRL_THREE       3


/* Pluggable I2C Device Address */
#define PLUG_I2C_ADDR_TEMP             (0x9C >> 1)    
#define PLUG_I2C_ADDR_ACT2             (0xE6 >> 1)
#define PLUG_TC_I2C_ADDR_GPIO_EXP      (0x38 >> 1)   /* Pluggable Test Card GPIO Expander */
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
#define TSN_GE0               0
#define TSN_GE1               1

#define TSN_GE0_ETHNUM        ETH0
#define TSN_GE1_ETHNUM        ETH2
#define TSN_ESW_ETHNUM        ETH1
#define PLUG_GE0              TSN_GE0
#define PLUG_GE1              TSN_GE1
#define PLUG_GE0_ETHNUM       TSN_GE0_ETHNUM
#define PLUG_GE1_ETHNUM       TSN_GE1_ETHNUM

#define PLUG_TC_FPGA_OFFSET                (0x10000)
#define PLUG_TC_I2C_CTRL_OFFSET            (PLUG_TC_FPGA_OFFSET + 0x2000)
#define PLUG_TC_FPGA_I2C_OFFSET            (0x100)
#define PLUG_TC_USB_2P0_BUS_NUMBER         (3)
#define PLUG_TC_USB_3P0_BUS_NUMBER         (4)
#define PLUG_TC_USB_C1101_LEV_NUMBER       (1)
#define PLUG_TC_USB_HUB_C1101_LEV_NUMBER   (2)
#define PLUG_TC_USB_C1109_LEV_NUMBER       (2)
#define PLUG_TC_USB_HUB_C1109_LEV_NUMBER   (3)
#define PLUG_TC_USB_SLOT1_PORT_NUMBER      (0)
#define PLUG_TC_USB_SLOT2_PORT_NUMBER      (1)

#define SEL_PORT_ETH "eth"

/* Common defines */
typedef enum {
    CPUMAC_LINKDOWN = 0,
    CPUMAC_LINKUP,
} cpu_mac_linkopt;

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


extern int plug_tc_host_usb_hub_menu_flag(int);
extern int plug_tc_host_sgmii_present(int);
extern void plug_tc_host_reply_usb_bus_lev_port_info(int, int, int *, int *, int *, int *, int);
extern int plug_tc_host_tx_rx_diag(char*, int, int,int, int, int);
extern int plug_tc_host_ge_send_packet_util(int);
extern int plug_tc_host_gephy_set_auto_neg(void);
extern int plug_tc_host_gephy_set_1000_speed(void);
extern int plug_tc_host_gephy_set_test_speed(int);
extern int plug_tc_host_check_ext_lpbk_flag(void);
extern int plug_tc_host_reply_geport_ethnum(int, int *);
extern void plug_tc_host_get_eth_interface_info(int, char *);

/* Platfrom local function */                                                                        
extern int tsn_cpu_mac_config(int, uint);
extern int tsn_cpu_mac_check_linkstat(int, boolean);
extern int ge_send_packet_util(int);
extern int gephy_set_default(int);
extern int tsn_mem_write32(uint, uint);
extern int check_ext_lpbk_flag(void);

#endif

/*-------------------------------------------------
$Log: plug_testcard_host_impl.h,v $
Revision 1.2  2018/11/23 08:49:53  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.6  2018/11/02 09:50:46  hondwang
Add USB prnt info for tabei-L

Revision 1.1.2.5  2018/11/01 12:59:34  hondwang
Modify pluggable testcard USB Hub testing with random port

Revision 1.1.2.4  2018/11/01 08:17:45  hondwang
Add USB hub flag for USB menu test item

Revision 1.1.2.3  2018/11/01 06:24:33  hondwang
Add plug testcard USB HUB testing function

Revision 1.1.2.2  2018/10/16 07:08:45  hondwang
plug_tc_host_sgmii_present should be platform code, modified

Revision 1.1.2.1  2018/10/15 06:44:32  hondwang
pluggable common code re-instruct add and remove files



$Endlog$
*/
