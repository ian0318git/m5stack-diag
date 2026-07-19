/* $Id: diag_esw_test.h,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_esw_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_esw_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int diag_ac5_test (int);
extern int get_pcie_link_cap_with_domain(uint32_t, uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status_with_domain(uint32_t, uint32_t, uint16_t, int, uint);
extern int get_pcie_cap_struct_ptr_with_domain(uint32_t, uint32_t, uint16_t, int, uint);

#define ELIXIR_PHY_INTR_TEST_PORT_NUM  1
#define ELIXIR_PHY_INTR_TEST_PORT_0    0
#define AC5_INTR_POLLING_PERIOD        50 /* 50ms */
#define AC5_INTR_POLLING_ROUND         1000
#define PCIE_DRV_PATH "/dev/ac5_driver"
#define PCIE_DRV_POLLING_TIME 5
#define LPBK_ETH0     "eth0"
#define LPBK_PKG      3
#define AC5_DEV_VID 0x11AB
#define AC5_DEV_PID 0xB403

/* Map panel port number (Cisco defined) 
 * to actual port number (Foxconn HW defined) */
#define FRONT_PANEL_PORT_ZERO          1
#define FRONT_PANEL_PORT_ONE           0
#define FRONT_PANEL_PORT_TWO           3
#define FRONT_PANEL_PORT_THREE         2
#define FRONT_PANEL_PORT_FOUR          5
#define FRONT_PANEL_PORT_FIVE          4
#define FRONT_PANEL_PORT_SIX           7
#define FRONT_PANEL_PORT_SEVEN         6

#define FRONT_PANEL_PORT_ZERO_1GBPS          11
#define FRONT_PANEL_PORT_ONE_1GBPS           10
#define FRONT_PANEL_PORT_TWO_1GBPS           13
#define FRONT_PANEL_PORT_THREE_1GBPS         12
#define FRONT_PANEL_PORT_FOUR_1GBPS          15
#define FRONT_PANEL_PORT_FIVE_1GBPS          14
#define FRONT_PANEL_PORT_SIX_1GBPS           17
#define FRONT_PANEL_PORT_SEVEN_1GBPS         16

#define SPEED_1GBPS_TAG               10
#define DATAIN_REG   0x7F018150
#define GEESW    2 /* use for ESW utility */
/*-------------------------------------------------
 * $Log: diag_esw_test.h,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.14  2021/05/31 10:43:42  illiu
 * Add macro
 *
 * Revision 1.1.2.13  2021/04/12 08:44:07  illiu
 * Add macro
 *
 * Revision 1.1.2.12  2021/03/22 03:24:16  harrchan
 * Add PCIE speed check test in ESW menu
 *
 * Revision 1.1.2.11  2021/03/15 10:07:31  illiu
 * Add macro string
 *
 * Revision 1.1.2.10  2021/03/03 08:06:10  illiu
 * Rename AC5 driver from nim_dm.ko to ac5_driver.ko
 *
 * Revision 1.1.2.9  2020/11/05 03:01:57  illiu
 * Add test item: xCat5 Interrupt Test, PHY Interrupt Test
 *
 * Revision 1.1.2.8  2020/10/15 12:04:56  illiu
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
 * Revision 1.1.2.7  2020/10/07 11:20:48  illiu
 * Clean up code
 *
 * Revision 1.1.2.6  2020/10/07 09:19:36  illiu
 * Clean up code
 *
 * Revision 1.1.2.5  2020/09/25 07:06:47  illiu
 * Add macro for test item(xCat3 Interrupt Test)
 *
 * Revision 1.1.2.4  2020/09/24 09:37:01  illiu
 * Add macro for test item(PHY Interrupt Test)
 *
 * Revision 1.1.2.3  2020/09/21 09:21:29  illiu
 * Add macro for test item(Internal Loopback Test)
 *
 * Revision 1.1.2.2  2020/09/10 09:52:22  illiu
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
