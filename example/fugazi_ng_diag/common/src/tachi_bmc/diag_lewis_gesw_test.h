/* $Id: diag_lewis_gesw_test.h,v 1.4 2017/03/30 08:30:53 hondwang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_lewis_gesw_test.h,v $ 
 *------------------------------------------------------------------
 *
 * diag_lewis_gesw_test.h - Header file for lewis_gesw Tests
 *
 * November 2015, Josh Skow
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/* NC command macro common definition */

#define NC_MVL_TACHI_L         "tachi-l\r"
#define NC_MVL_EXAMPLES        "end\rexamples\r"
#define NC_MVL_END             "end\r"
#define NC_MVL_RELOAD          "reload\r"

#define CONSOLE_FLAG           0
#define TACHI_L_FLAG           1
#define NC_MVL_BMC_CONN               "bmc_conn 1"


/* NC command macro test definitions */

#define NC_MVL_MEM_TEST        "mem test device 0 bmc_conn 1\r"
#define NC_MVL_NAND_TEST       "nand test device 0 bmc_conn 1\r"
#define NC_MVL_GPIO_TEST       "gpio test device 0 bmc_conn 1\r"
#define NC_MVL_POE_TEST        "poe test power device 0 bmc_conn 1\r"
#define NC_MVL_POE_REG_TEST    "poe test register device 0 bmc_conn 1\r"
#define NC_MVL_BTB_TEST        "btb test device 0"
#define NC_MVL_PHY_REG_TEST    "phy test register device 0 bmc_conn 1\r"
#define NC_MVL_PHY_IRQ_TEST    "phy test irq device 0 bmc_conn 1\r"
#define NC_MVL_PHY_INLB_TEST   "phy test internal device 0 bmc_conn 1\r"
#define NC_MVL_PHY_EXLB_TEST   "phy test external device 0 bmc_conn 1\r"
#define NC_MVL_SW_REG_TEST     "switchcore test register device 0 bmc_conn 1\r"
#define NC_MVL_SW_LB_TEST      "switchcore loopback device 0 bmc_conn 1\r"
#define NC_MVL_SW_BIST_TEST    "switchcore test mem device 0\r"

/* NC command macro utility definitions */

#define NC_MVL_SHOW_DIAG_VER_UTILS     "show version bmc_conn 1\r"
#define NC_MVL_PHY_REG_RD_UTILS     "phy utility register read device 0"
#define NC_MVL_PHY_REG_WR_UTILS     "phy utility register write device 0"
#define NC_MVL_PHY_STUB_EN_UTILS    "phy utility stub enable device 0"
#define NC_MVL_PHY_STUB_DIS_UTILS   "phy utility stub disable device 0"
#define NC_MVL_PHY_LED_GRN_UTILS    "phy utility led green device 0\r"
#define NC_MVL_PHY_LED_AMB_UTILS    "phy utility led amber device 0\r"
#define NC_MVL_PHY_LED_CLR_UTILS    "phy utility led clear device 0\r"
#define NC_MVL_SW_LB_EN_UTILS       "serdes utility internal-lpbk enable device 0"
#define NC_MVL_SW_LB_DIS_UTILS      "serdes utility internal-lpbk disable device 0"
#define NC_MVL_SHOW_IF_STA_UTILS    "do show interfaces status ethernet 0/"
#define NC_MVL_SHOW_MAC_CNT_UTILS   "do show interfaces mac counters ethernet 0/"
#define NC_MVL_POE_INIT_UTILS       "poe utility init device 0 bmc_conn 1\r"
#define NC_MVL_POE_SHOW_SYS_UTILS   "poe utility show sys device 0 bmc_conn 1\r"
#define NC_MVL_POE_SHOW_PWR_UTILS   "poe utility show power device 0 bmc_conn 1\r"
#define NC_MVL_POE_PORT_EN_UTILS    "poe utility enable port device 0"
#define NC_MVL_POE_PORT_DIS_UTILS   "poe utility disable port device 0"

#define NC_MVL_POE_LED_AMB_UTILS    "gpio utility led amber device 0\r"
#define NC_MVL_POE_LED_GRN_UTILS    "gpio utility led green device 0\r"
#define NC_MVL_POE_LED_OFF_UTILS    "gpio utility led off device 0\r"

#define NC_MVL_X710_SW_LB_EN       "serdes utility internal-lpbk enable device 0 port 64\r"
#define NC_MVL_X710_SW_LB_DIS      "serdes utility internal-lpbk disable device 0 port 64\r"
#define NC_MVL_SHOW_INFO		   "show version bmc_conn 1\r"

#define NC_MVL_PORT                 "port"
#define NC_MVL_REG                  "register"
#define NC_MVL_DATA                 "data"
#define NC_MVL_POE_PORT             "poe_port"
#define NC_MVL_BTB_SRC_IP           "src_ip"
#define NC_MVL_BTB_DST_IP           "dest_ip"

/* Other NC command definitions */
#define NC_MVL_NIM_VLAN_SET         "load nim_vlan\r"
#define NC_MVL_NIM_VLAN_UNSET       "load nim_vlan_defconfig\r"
#define NC_MVL_NIM_IFACE_1000BASEX  "load nim_basex\r"
#define NC_MVL_NIM_IFACE_SGMII      "load nim_sgmii\r"
#define NC_MVL_NIM_IFACE_KR         "load nim_kr\r"
#define NC_MVL_NIM_FORCE_LINK_UP    "load nim_linkup\r"
#define NC_MVL_X710_EN_LPBK         "load x710_test\r"
#define NC_MVL_X710_DIS_LPBK        "load x710_test_deconfig\r"

extern int diag_lewis_gesw_util(int);
extern int diag_lewis_gesw_phy_util(void);
extern int diag_lewis_gesw_poe_util(void);
extern int diag_lewis_gesw_test(int);
extern int lewis_gesw_pass_file (void);
extern int run_lewis_gesw_test (char *test_comm, int tachi_flag);
extern int gesw_image_info(void); 


#define TACHI_SPECIFIC 1
#define GENERIC_TEST 0

/* Enumeration */
typedef enum {
    NIM_1000BASE_X = 0,
    NIM_SGMII,
    NIM_KR,
    NIM_IFACE_INVALID
} NIM_IFACE_SETTING;

extern int diag_lewis_gesw_set_nim_vlan_util(int);
extern int diag_lewis_gesw_nim_iface_setting(int,int);
extern int diag_lewis_gesw_poe_port_endis_util(int);
extern int diag_lewis_gesw_phy_led_amber_util(void);
extern int diag_lewis_gesw_phy_led_green_util(void);
extern int diag_lewis_gesw_phy_led_clear_util(void);
extern int diag_lewis_gesw_poe_led_amber_util(void);
extern int diag_lewis_gesw_poe_led_green_util(void);
extern int diag_lewis_gesw_poe_led_off_util(void);
extern int diag_lewis_gesw_x710_endis_serdes_lpbk(int);

/*----------------------------------------------------------------
$Log: diag_lewis_gesw_test.h,v $
Revision 1.4  2017/03/30 08:30:53  hondwang
Tachi-L brach merge

Revision 1.3.12.1  2017/02/08 03:42:55  hondwang
Add POE register test

Revision 1.3  2016/06/21 03:03:44  jimmyya
Add lewis NIM serdes setting function

Revision 1.2  2016/04/20 11:25:27  benchen2
add tachi fru portion

Revision 1.1.2.10  2016/02/16 23:41:15  jskow
Add enhanced error messaging to Lewis GESW

Revision 1.1.2.9  2016/02/04 20:56:57  jskow
Update GESW show_version utility

Revision 1.1.2.8  2016/01/30 00:26:18  jskow
Modify GESW tests to only do loopback tests when uncertain about failure status, reduce overall test time.

Revision 1.1.2.7  2016/01/14 02:06:38  jskow
Add verbose mode and time stamps to GESW tests.  Add functions to transmit mid-test information from GESW to BMC.  Modify nc_client_lib to accept INFO status remotely from Intel/GESW to BMC.  Modify GESW test names for clarity.

Revision 1.1.2.6  2016/01/13 10:01:40  jimmyya
Add show Lewis diag image version utility

Revision 1.1.2.5  2016/01/12 07:35:02  jimmyya
Add cvs logs

$Endlog$
*/

