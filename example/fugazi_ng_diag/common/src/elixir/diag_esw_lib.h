/* $Id: diag_esw_lib.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_esw_lib.h,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "ethernet.h"


/* For Elixir AC5 */
#include "dev_98dxc25x.h"
#include "dev_88e1680.h"
#define ELIXIR_AC5_CPSS_DEV    0
#define ELIXIR_XCAT5_USED_PORT    8
#define XCAT5_TO_CPU_PORT        26
#define XCAT5_TO_WIFI_PORT       24
#define LINKDOWN 0
#define LINKUP 1
#define LMS0_MISC_CONFIG_REG_OFF         0x07004200
#define FPGA_DEV_RST_AC3_RST    (1 << 18)
#define FPGA_DEV_RST_88E1680_2_RST    (1 << 17)
#define FPGA_DEV_RST_88E1680_1_RST    (1 << 16)
#define FPGA_DEV_RST_88E1680_0_RST    (1 << 15)
#define AC5_MPP_BIT_9                 (1 << 9)
#define ETH_RM_AC5_NIM_DM_MODULE           "rmmod ac5_driver &> /dev/null"
#define ETH_INSMOD_AC5_NIM_DM_MODULE       "insmod /diag/ac5_driver.ko &> /dev/null"
#define ESW_WAIT_500MS   500
#define ESW_WAIT_200MS   200
#define ESW_WAIT_1000MS  1000
#define ELIXIR_ESW_PORT_NUM    8
#define ELIXIR_ESW_SMI1_PORT_START_NUM    8
#define ELIXIR_1680_GROUP_NUM    1
#define ELIXIR_1680_GROUP_INDEX  0
#define ELIXIR_1680_GROUP_0_START_ADDR    0
#define ELIXIR_1680_NUM_PHY_IN_ONE_GRUOP    8
#define SPD_10MBPS          10
#define SPD_100MBPS         100
#define SPD_1000MBPS        1000
#define GE_XCAT5_PORT        26
#define ELIXIR_XCAT_PCIE_BUS    0x01
#define XCAT5_PCI_PORT_GROUP    0
#define PCIE_REMOVE         "echo 1 > /sys/bus/pci/devices/0000:00:00.0/remove"
#define PCIE_RESCAN         "echo 1 > /sys/bus/pci/rescan"
#define PCIE_ENABLE00       "echo 1 > /sys/bus/pci/devices/0000:00:00.0/enable"
#define PCIE_ENABLE01       "echo 1 > /sys/bus/pci/devices/0000:01:00.0/enable"


typedef enum {
    ESW_SPD_10 = 0,
    ESW_SPD_100,
    ESW_SPD_1000
} dev_esw_speed_t;

MAD_DEV phy_mad_88e1680;

extern int diag_ac5_init (void);
extern int diag_esw_exit (void);
extern int diag_reset_esw_to_default (void);
extern int diag_config_port_speed (uint, uint, uint);
extern int xcat5_reg_pci_read (uint, uint, uint*);
extern int diag_esw_all_phy_green_led_on (void);
extern int diag_esw_all_phy_green_led_off (void);

dev_object_t *diag_get_esw_98dxc25x_obj (void); /* create device object */
dev_object_t *diag_get_phy_88e1680_obj (void); /* create device object */

/* End of Elixir AC5 */

/* Common */
#define ENHANCE_ERROR_MSG_RDY 1
#define PLAT_ESW_MAX_GEPORT    8
#define PLAT_ESW_MAX_PORT      0xA
#define ESW_ACCESS_WAITTIME   20
#define ESW_RETRY_MAX         100
#define ESW_PHY_RST_TIMEOUT   1000
#define SEC_TO_MICROSEC            1000000
#define ESW_MAX_POLLINGTIME_USEC   5000000   /* 5sec */
#define SEC_TO_MICROSEC        1000000
#define MAX_POLLINGTIME_USEC   10000000   /* 10sec */
#define ESW_REG_PAGE_252 0xfc
#define PLAT_SMI_WAIT_CUNTR    200
#define PLAT_ESW_PORTS   8

#define ESW_PORT0       0
#define ESW_PORT1       1
#define ESW_PORT2       2
#define ESW_PORT3       3
#define ESW_PORT4       4
#define ESW_PORT5       5
#define ESW_PORT6       6
#define ESW_PORT7       7
#define ESW_PORT8       8
#define ESW_PORT9       9
#define ESW_PORT10      10

#define ESW_VLAN1       1
#define ESW_VLAN2       2
#define ESW_VLAN3       3
#define ESW_VLAN4       4


#define PLAT_ESW_SMI_CMD_REG    MRVL88E6390_MCA_SMI_CMD_REG
#define PLAT_ESW_SMI_DATA_REG   MRVL88E6390_MCA_SMI_DATA_REG

#define ALL_ESW_LEDS         0xF
#define ESW_LED_F_ON         1
#define ESW_LED_F_OFF        0 

#define ESW_RESET_ONE_SEC    1000   /* 1sec = 1000ms */

/* Multi Chip Addressing mode */
/* SMI command register(0x0) */
#define SMI_CMD_SMIBUSY       (1 << 15)
#define SMI_CMD_SMIMODE_C22   (1 << 12)
#define SMI_CMD_SMIMODE_C45   (0 << 12)
#define SMI_CMD_SMIOP_RD      (1 << 11)   /* 11:10 0x2 Read Data Reg. */
#define SMI_CMD_SMIOP_WR      (1 << 10)   /* 11:10 0x1 Write Data Reg. */
#define SMIOP_C45_WR_ADDR     (0 << 10)   /* [11:10] C45 0x0 Write Addr. Reg. */
#define SMIOP_C45_WR_DATA     (1 << 10)   /* [11:10] C45 0x1 Write Data Reg. */
#define SMIOP_C45_RD_DATA_PI  (2 << 10)   /* [11:10] C45 0x2 Read Data Reg. with post increament on Addr. Reg. */
#define SMIOP_C45_RD_DATA     (3 << 10)   /* [11:10] C45 0x3 Read Data Reg. */

/* ESW SMI Device Register MAP */
/* Device Addr. */
#define ESW_SMIDEV_GLOB1      0x1B
#define ESW_SMIDEV_GLOB2      0x1C

#define ESW_PHYCTR_REG        0x1
#define ESW_PORTCTR_REG       0x4
#define ESW_PORT_VLAN_REG     0x6
#define ESW_PORTVLAN_ID_REG   0x7
#define ESW_PORTCTR2_REG      0x8

#define ESW_PCR_PSTAT_MSK     0x3
#define ESW_PCR_DISABLE       0x0
#define ESW_PCR_BLOCKING      0x1
#define ESW_PCR_LEARNING      0x2
#define ESW_PCR_FORWARD       0x3

/* ESW Port Status Reg.(0x0) */
#define ESW_PSR_ADDR             0x0
#define ESW_PSR_LINKUP           (1 << 11)
#define ESW_PSR_LINK             (1 << 11)
#define ESW_PSR_FULLDPX          (1 << 10)
#define ESW_PSR_DPX              (1 << 10)
#define ESW_PSR_SPD_MSK          0x0300
#define ESW_PSR_10MBPS           0x0000
#define ESW_PSR_100MBPS          0x0100
#define ESW_PSR_1000MBPS         0x0200
#define ESW_PSR_10GBPS           0x0300

/* ESW Physical Control Reg.(0x1) */
#define ESW_PCR_ADDR             0x1
#define ESW_PCR_RGMII_RX_DELAY   (1 << 15)
#define ESW_PCR_RGMII_TX_DELAY   (1 << 14)
#define ESW_PCR_FORCE_SPEED      (1 << 13)
#define ESW_PCR_MII_MAC_MODE     (0 << 11)
#define ESW_PCR_MII_PHY_MODE     (1 << 11)
#define ESW_PCR_F_LINKUP         (1 << 5)
#define ESW_PCR_FORCE_LINK       (1 << 4)
#define ESW_PCR_F_FULLDPX        (1 << 3)
#define ESW_PCR_FORCE_DPX        (1 << 2)
#define ESW_PCR_10MBPS           0x0
#define ESW_PCR_100MBPS          0x1
#define ESW_PCR_1000MBPS         0x2
#define ESW_PCR_10GBPS           0x3
#define ESW_PCR_DEF_VAL          0x3

/* ESW Port Control Reg.(0x4) */
#define ESW_PCR_PS_MSK           0x3
#define ESW_PCR_PORT_DIS         0x0
#define ESW_PCR_PORT_FORWARD     0x3

/* ESW Port Based VLAN Map Reg.(0x6) */
#define ESW_PBVM_VLAN_TBL_MSK    0x7F
#define ESW_PBVM_VLAN_TBL(x)     (1 << x)

/* ESW Default Port VLAN ID & Priority */
#define ESW_PVID_FORCE_DVID      (1 << 12)
#define ESW_PVID_DVID_MSK        0xFFF

/* ESW Port Control 2 Reg.(0x08) */
#define ESW_PCR2_8021Q_MODE_MSK  (0x3 << 10)
#define ESW_PCR2_8021Q_SECURE    (0x3 << 10)

/* Global 2(0x1C) SMI register addr. */
#define ESW_GLOB2_PC          0x18   /* SMI PHY Command */
#define ESW_GLOB2_PD          0x19   /* SMI PHY Data */

/* ESW GE PHY Register Map */
#define ESW_GEPHY_PAGE_ADDR   22

/* ESW Copper Control register Map */
#define ESW_CCR_PWRDWN        (1 << 11)

/* ESW LED Control(0x16) register */
#define ESW_LED_CONTR_REG     0x16
#define ESW_LCR_UPDATE        (1 << 15)
#define ESW_LCR_LED1_F_ON     (0xF << 4)
#define ESW_LCR_LED0_F_ON     0xF
#define ESW_LCR_LED1_F_OFF    (0xE << 4)
#define ESW_LCR_LED0_F_OFF    0xE

/* ESW SGMII register Map */
#define ESW_SGMII_DEVNUM      4
#define ESW_SGMII_CONTR_REG   0x2000
#define ESW_SGMII_PWRDWN      (1 << 11)

/* ESW Global 1 Reg. Map */
#define ESW_G1_VTUFID_REG            0x2
#define ESW_G1_VTUOP_REG             0x5
#define ESW_G1_VTUVID_REG            0x6
#define ESW_G1_VTUDATA_0TO7_REG      0x7
#define ESW_G1_VTUDATA_8TO10_REG     0x8

#define ESW_G1_VID_ENTRY_VALID       (1 << 12)

#define ESW_G1_DATA_FRAME_UNTAGGED   0x1
#define ESW_G1_MEMBER_STATE_MSK      0x3

#define ESW_G1_OP_VTUBUSY            (1 << 15)
#define ESW_G1_OP_VTULOAD            (3 << 12)

/* ESW PHY */
#define ESW_ALL_PHY_PORTS            0xA

#define ESW_SET_PORT_10M             0x1
#define ESW_SET_PORT_100M            0x2
#define ESW_SET_PORT_1G              0x3

#define ESW_SET_PORT_HD              0x0
#define ESW_SET_PORT_FD              0x1

/* Copper Control Reg. (0_0) */
#define ESWPHY_CCR_ADDR              0x0
#define ESWPHY_CCR_COP_RST           (1 << 15)
#define ESWPHY_CCR_LPBK              (1 << 14)
#define ESWPHY_CCR_AN_EN             (1 << 12)

/* Copper Auto-Negotitation Advertisement Reg. (4_0) */
#define ESWPHY_CANAR_ADDR            0x4
#define ESWPHY_COP_ANAR_100FD        (1 << 8)
#define ESWPHY_COP_ANAR_100HD        (1 << 7)
#define ESWPHY_COP_ANAR_10FD         (1 << 6)
#define ESWPHY_COP_ANAR_10HD         (1 << 5)

/* 1000BASE-T Control Reg. (9_0) */
#define ESWPHY_1000TCR_ADDR          0x9
#define ESWPHY_1G_CNTR_1000FD        (1 << 9)
#define ESWPHY_1G_CNTR_1000HD        (1 << 8)

/* Copper Specific Status Reg1 (17_0) */
#define ESWPHY_CSSR1_SPEED           (3 << 14)
#define ESWPHY_CSSR1_DUPLEX          (1 << 13)
#define ESWPHY_CSSR1_RESOLVED        (1 << 11)
#define ESWPHY_CSSR1_COP_LINK        (1 << 10)
#define ESWPHY_CSSR1_LINK_STAT       (1 << 3)

#define ESWPHY_CSSR1_SPD_1000MBPS    0x8000
#define ESWPHY_CSSR1_SPD_100MBPS     0x4000
#define ESWPHY_CSSR1_SPD_10MBPS      0x0000
#define ESWPHY_CSSR1_FULLDUP         0x2000
#define ESWPHY_CSSR1_RT_LINK_UP      0x0400
#define ESWPHY_CSSR1_COP_LINK_UP     0x0008

/* MAC Specific Control Reg2 (21_2) */
#define ESWPHY_MSCR2_MACSPD_MSK        0x7   /* 21_2.2:0 */
#define ESWPHY_MSCR2_MACSPD_10MBPS     0x4   /* 10 MBPS  (21_2.2:0 = 100) */
#define ESWPHY_MSCR2_MACSPD_100MBPS    0x5   /* 100 MBPS (21_2.2:0 = 101) */
#define ESWPHY_MSCR2_MACSPD_1000MBPS   0x6   /* 1000 MBPS(21_2.2:0 = 110) */

/* Checker Control Reg. (18_6) */
#define ESWPHY_CHKREG_STUB_EN_BIT      3
#define ESWPHY_CHKREG_STUB_EN          0x0008
#define ESWPHY_CHKREG_STUB_EN_MSK      0x0008

#define DATA_REG      (5)   /* Port 5 Reg 0x1A is the corresponding data reg */
#define CMD_REG       (4)   /* Port 4 Reg 0x1A is a command reg */
#define REG_RSVD      (0x1A) /* Reserved Register, based Marvell Errata  */
/* Port 5 Reg 0x1A (Block 0xF Index 0x0) */
#define ERRATA_REG_0X1C0 (0x01c0)   /* Must be 0x1C0 */
/* Port 4 Reg 0x1A Debug Control */
#define ERRATA_DBG_CTRL_0XFC00 (0xFC00)   /* Must be 0xFC00 */
#define ERRATA_DBG_CTRL_0XFC20 (0xFC20)   /* Must be 0xFC20 */
#define ERRATA_DBG_CTRL_0XFC40 (0xFC40)   /* Must be 0xFC40 */
#define ERRATA_DBG_CTRL_0XFC60 (0xFC60)   /* Must be 0xFC60 */
#define ERRATA_DBG_CTRL_0XFC80 (0xFC80)   /* Must be 0xFC80 */
#define ERRATA_DBG_CTRL_0XFCA0 (0xFCA0)   /* Must be 0xFCA0 */
#define ERRATA_DBG_CTRL_0XFCC0 (0xFCC0)   /* Must be 0xFCC0 */
#define ERRATA_DBG_CTRL_0XFCE0 (0xFCE0)   /* Must be 0xFCE0 */
#define ERRATA_DBG_CTRL_0XFD00 (0xFD00)   /* Must be 0xFD00 */
#define ERRATA_DBG_CTRL_0XFD20 (0xFD20)   /* Must be 0xFD20 */
#define ERRATA_DBG_CTRL_0XFD40 (0xFD40)   /* Must be 0xFD40 */

#define PLAT_ESW_CPU_MACNUM 2

/*-------------------------------------------------
 * $Log: diag_esw_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.23  2021/05/31 10:46:11  illiu
 * Rename function diag_esw_all_phy_led_off/on to diag_esw_all_phy_green_led_off/on
 *
 * Revision 1.1.2.22  2021/04/23 02:42:09  illiu
 * 1. Add macro
 * 2. Clean up code
 *
 * Revision 1.1.2.21  2021/04/12 08:54:21  illiu
 * 1. Replace object-create method as object-get method (Device driver object)
 * 2. Rename DEV_ESW_SPD_10/100/1000 to ESW_SPD_10/100/1000
 *
 * Revision 1.1.2.20  2021/03/18 07:56:22  illiu
 * Add variable: MAD_DEV phy_mad_88e1680
 *
 * Revision 1.1.2.19  2021/03/15 09:51:54  illiu
 * Add macro string
 *
 * Revision 1.1.2.18  2021/03/03 06:55:32  illiu
 * Rename AC5 driver from nim_dm.ko to ac5_driver.ko
 *
 * Revision 1.1.2.17  2021/02/03 02:50:19  illiu
 * Clean up code
 *
 * Revision 1.1.2.16  2020/11/12 06:38:48  illiu
 * 1. Add Elixir 1680 phy led features to MB LED test/utility item
 * 2. Add ESW PHY LED Utility
 *
 * Revision 1.1.2.15  2020/11/05 06:34:55  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.14  2020/11/05 03:01:42  illiu
 * Add test item: xCat5 Interrupt Test, PHY Interrupt Test
 *
 * Revision 1.1.2.13  2020/10/15 12:04:42  illiu
 * 1. Move AC5 switch init and exit process to linux_main.c(It means do init once diag application is actived and do exit once diag application is exit)
 * 2. Add port configuration process for wifi6 module(XCAT5_TO_WIFI_PORT=26) which is connected to AC5 switch
 * 3. Add nim_dm driver polling, to check if driver is ready
 * 4. Add nim_dm driver polling, to check if driver exist before doing insmod or rmmod commend
 * 5. Modify the accessed path of pcie device in diag_esw_remove_pcie_device function
 * 6. Modify marvell_cpssPpInit_xcat5 and phy_dev_88e1680_group_start_addr to be static type variable
 * 7. Move array: phy_dev_88e1680 to header file
 * 8. Remove marvell_ac5_cpss_dev_num_elixir variable, and use ELIXIR_AC5_CPSS_DEV macro directly
 * 9. Modify AC5 switch test item name: External Loopback Test ==> PHY External Loopback Test
 * 10.Remove unneeded variable: port_group, port_group_phy_num
 * 11.Modify code alignment
 *
 * Revision 1.1.2.12  2020/10/07 11:20:39  illiu
 * Clean up code
 *
 * Revision 1.1.2.11  2020/10/07 09:19:28  illiu
 * Clean up code
 *
 * Revision 1.1.2.10  2020/10/06 02:06:00  illiu
 * Transform calling objects from AC3 file/function to AC5 file/finction (dev_98dxc323.c -> dev_98dxc25x.c)
 *
 * Revision 1.1.2.9  2020/09/26 03:34:12  illiu
 * Add below Utilities items:
 *     ESW PCI Config Read Utility
 *     ESW PCI Config Write Utility
 *     ESW xCat3 Internal Register Write Utility
 *     ESW xCat3 PP Register Read Utility
 *     ESW xCat3 PP Register Write Utility
 *     Print All PHY Counter Utility
 *     Clear All PHY Counter Utility
 *     Print xCat3 Counter Utility
 *     Clear xCat3 Counter Utility
 *     ESW Reset Default Utility
 *
 * Revision 1.1.2.8  2020/09/25 07:12:11  illiu
 * Modify macro GE_XCAT3_PORT from 24 to 26 which is used in loopback test item
 *
 * Revision 1.1.2.7  2020/09/24 09:31:36  illiu
 * Add macro for insert PCIE driver(nim_dm.ko)
 *
 * Revision 1.1.2.6  2020/09/22 03:26:36  illiu
 * Add enum for test item(External Loopback Test)
 *
 * Revision 1.1.2.5  2020/09/21 09:20:45  illiu
 * Add function and macro for test item(Internal Loopback Test)
 *
 * Revision 1.1.2.4  2020/09/17 10:30:08  illiu
 * Move macro from diag_esw_lib.c to here
 *
 * Revision 1.1.2.3  2020/09/17 10:03:32  illiu
 * Add function and macro for PHY(88E1680)'s init and PHY(88E1680)'s register test
 *
 * Revision 1.1.2.2  2020/09/10 09:52:12  illiu
 * Delete 88E6390/88E6176 Switch related code
 *
 * Revision 1.1.2.1  2020/09/09 09:09:51  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
