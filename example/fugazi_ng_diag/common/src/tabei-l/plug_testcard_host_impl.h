/* $Id: plug_testcard_host_impl.h,v 1.4 2019/12/30 06:05:38 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_testcard_host_impl.h,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_host_impl.h - Header file for Host implement Pluggable Test card function
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_TESTCARD_HOST_IMPL__
#define __PLUG_TESTCARD_HOST_IMPL__

#define MAX_USB_ENUMERATION_CHK_TIMES   6
#define USB_ENUMERATION_TIME            1000
#define UMOUNT_DEBUGFS                  "umount /sys/kernel/debug"

/* Pluggable I2C Device Address */
extern boolean tc_has_pcie;
#define PLUG_I2C_ADDR_TEMP              (0x9C >> 1)    
#define PLUG_I2C_ADDR_ACT2              (0xE6 >> 1)
#define PLUG_SGMII_TC_I2C_ADDR_GPIO_EXP (0x38 >> 1)
#define PLUG_PCIE_TC_I2C_ADDR_GPIO_EXP  (0x3E >> 1)
#define PLUG_TC_I2C_ADDR_GPIO_EXP       (tc_has_pcie?(0x3E >> 1):(0x38 >> 1))
/* Pluggable Test Card 88E1112 PHY */
#define PLUG_TC_I2C_ADDR_PHY            (0xB8 >> 1)
/* Pluggable LTE Mandatory GPIO Expander */
#define PLUG_MAN_I2C_ADDR_GPIO_EXP      (0x4E >> 1)  
/* Pluggable LTE Optional GPIO Expander */
#define PLUG_OPT_I2C_ADDR_GPIO_EXP      (0x4C >> 1)

#define USB_SLOT0                       0
#define USB_SLOT1                       1
#define USB_SLOT2                       2

/* Common */
#define GEWAN_TXTYPE_A                  1
#define GEWAN_TXTYPE_B                  0


/* Based on Star Test card mapping,
 * GE0 PHY: eth2
 */
/* Ethernet definition */
#define PLUG_GE0                        0
#define PLUG_GE1                        1
#define PLUG_GE0_ETHNUM                 0
#define PLUG_GE1_ETHNUM                 1

#define PLUG_TC_FPGA_OFFSET             (0x10000)
#define PLUG_TC_I2C_CTRL_OFFSET         (PLUG_TC_FPGA_OFFSET + 0x2000)
#define PLUG_TC_FPGA_I2C_OFFSET         (0x100)
#define PLUG_TC_USB_BUS_3_NUMBER        (3)
#define PLUG_TC_USB_BUS_4_NUMBER        (4)

#define SEL_PORT_ETH                    "eth"
#define PIM_NVME_DEV                    "/dev/pimnvme1"


/* Port Auto-Negotiation Config Reg(m = 0-3) */
#define CPU_PORT_AN_CONF_REG(m)                 (uint)(0xF2130E0C + (m*0x1000))
#define CPU_PACR_F_LINKUP                       (uint)(1 << 1)
#define CPU_PACR_F_LINKDOWN                     (uint)(1)

/* Digital Loopback Enable Register(n = 0-5) */
#define CPU_DIGITAL_LOOPBACK_ENABLE_REG(n)	(0xF212088C + (n*0x1000))
#define CPU_LOCAL_DIG_RX2TX_LPBK_EN             0x8000  /* (1 << 15) */
#define CPU_RXPHER_TO_TX_EN                     0x1000  /* (1 << 12) */

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
#define PLUG_PCI_DEVICE          "/sys/bus/pci/devices/0000:07:00.0/vendor"
#define PLUG_PCI_DEVICE_PMTM     "/sys/bus/pci/devices/0000:1b:00.0/vendor"


/* Common defines */
typedef enum {
    CPUMAC_LINKDOWN = 0,
    CPUMAC_LINKUP,
} cpu_mac_linkopt;

extern int plug_tc_host_usb_hub_menu_flag(int);
extern int plug_tc_host_sgmii_present(int);
extern int plug_tc_host_pcie_present(int);
extern void plug_tc_host_reply_usb_bus_lev_port_info(int, int, int *, int *, 
                                                     int *, int *, int);
extern int plug_tc_host_tx_rx_diag(char*, int, int,int, int, int);
extern int plug_tc_host_ge_send_packet_util(int);
extern int plug_tc_host_gephy_set_auto_neg(void);
extern int plug_tc_host_gephy_set_1000_speed(void);
extern int plug_tc_host_gephy_set_test_speed(int);
extern int plug_tc_host_check_ext_lpbk_flag(void);
extern int plug_tc_host_reply_geport_ethnum(int, int *);
extern void plug_tc_host_get_eth_interface_info(int, char *);
extern void plug_tc_host_get_nvme_info(int, char *);
extern void plug_tc_host_get_pcie_dev_info(int, uint *, uint *, uint *, uint *);

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
Revision 1.4  2019/12/30 06:05:38  kehuang2
CSCvs55860: Support PIM testcard

Revision 1.3  2019/11/25 08:55:53  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2019/10/17 02:16:27  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.11  2019/08/02 08:39:05  kodko
Replace hard code delay with polling way to check if PIM USB enumeration is done.

Revision 1.1.2.10  2019/07/29 06:13:52  kodko
Clean up code based on off-line code review

Revision 1.1.2.9  2019/06/05 07:44:14  meho
code clean up

Revision 1.1.2.8  2019/04/17 03:26:33  olin2
Modify nvme device name

Revision 1.1.2.7  2019/03/22 08:20:24  meho
Added pcie speed/width detection in NVMe test.

Revision 1.1.2.6  2019/03/13 06:00:14  meho
Skip PIM NVMe test from default.

Revision 1.1.2.5  2019/03/11 09:31:08  meho
Support PIM NVMe on reworked Tabei-L

Revision 1.1.2.4  2019/02/25 07:11:50  meho
Support new PIM test-card (PCIe).

Revision 1.1.2.3  2018/12/20 08:09:21  kodko
Add extra delay after PIM power on for USB hub and storages enumeration.

Revision 1.1.2.2  2018/11/16 13:42:30  kodko
Support front USB hub and PIM USB hub connect with USB3.0 and USB2.0 storage read/write test.

Revision 1.1.2.1  2018/10/26 08:40:51  kodko
Add support for PIM LTE and test card modules.

$Endlog$
*/
