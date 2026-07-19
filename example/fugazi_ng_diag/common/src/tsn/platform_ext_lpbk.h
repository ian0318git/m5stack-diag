/* $Id: platform_ext_lpbk.h,v 1.4 2018/05/09 06:53:12 letsai Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_ext_lpbk.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_ext_lpbk.h
 * Description: Head file of TSN external loopback Diag.
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PLARFORM_EXT_LPBK_H__
#define __PLARFORM_EXT_LPBK_H__

/* setup delay time for driver to read the PHY reg. */
#define ETH_DRIVER_DELAY    1

/* Common */
#define SPD_10MBPS    10
#define SPD_100MBPS   100
#define SPD_1000MBPS   1000

#define I2CPHY_REG_WRITE_DELAY   1000

#define SEL_PORT_ETH "eth"

/* definition of PHY signal */
#define SIG_COPPER 0
#define SIG_FIBER 1

#define DISABLE_SIG   0
#define ENABLE_SIG    1

#define INT_LPBK 0
#define EXT_LPBK 1

#define AUTONEG_OFF  0
#define AUTONEG_ON   1

#define HALF_DUPLEX  0
#define FULL_DUPLEX  1

enum loopback_num {
    SGMII_EXT_LPBK = 0,
    SGMII_INT_EXT_LPBK,
    E_1000BASEX_INT_EXT_LPBK,
};

/* Poll timeout */
#define ETH_DRIVER_POLL_TIMEOUT     (10000) /* 10 secs */

/* Copper specific status register(17) */
#define COP_P0R17_SPEED           (3 << 14)
#define COP_P0R17_SPEED_OFFSET    14
#define COP_P0R17_SPEED_1000      0x2
#define COP_P0R17_SPEED_100       0x1
#define COP_P0R17_SPEED_10        0x0

#define COP_P0R17_DUPLEX_FULL     (1 << 13)
#define WOODLAWN_PHY_PAGE_REC    0x1000
#define WOODLAWN_PHY_RESOLVED    0x0800
#define COP_P0R17_COP_LINK_UP     (1 << 10)
#define WOODLAWN_PHY_TRANS_PAUSE_EN  0x0200
#define WOODLAWN_PHY_RECEV_PAUSE_EN  0x0100
/* reserved 0x0080*/
#define WOODLAWN_PHY_MDI_CO_STA   0x0040
#define WOODLAWN_PHY_DOWN_SHIFT   0x0020
#define WOODLAWN_PHY_COP_ENG_STA   0x0010
#define COP_P0R17_GLOBAL_LINK_UP  (1 << 3)
#define WOODLAWN_PHY_DTE_PWR_STA   0x0004
#define WOODLAWN_PHY_POLARITY   0x0002
#define WOODLAWN_PHY_JABBER   0x0001


/* define for PHY setting */
#define SET_PHY_BIT15     0x8000
#define SET_PHY_BIT14     0x4000
#define SET_PHY_BIT13     0x2000
#define SET_PHY_BIT11     0x0800
#define SET_PHY_BIT10     0x0400
#define SET_PHY_BIT3       0x0008
#define SET_AUTO_MEDIA     0x0007
#define SET_ENG_DETECT     0x0300

#define TX_RX_SYNC_TIME       10

#define COP_CTRL_REG0         0
#define COP_STATUS_REG1       1
#define COP_AUTONEG_ADV_REG4  4
#define COP_SPEC_CTRL_REG16   16
#define COP_STATUS_REG17      17
#define MAC_SPEC_CTRL2_REG21  21
#define COP_SPEC_CTRL_REG2    26

#define FIBER_SPECIFIC_STATUS_REG 17
#define FIBER_STATUS_REG1     1
#define GEN_CONT_REG_1        20

#define MAC_CTRL_REG0  0
#define MAC_SPEC_CTRL_REG2      26

#define FIB_CTRL_REG0  0
#define FIB_STATUS_REG1       1
#define FIB_SPEC_CTRL_REG2      26
#define FIB_OUTPUT_AMP_MSK      0x7
#define FIB_OUTPUT_AMP_VAL504   0x5

#define CHECKER_CTRL_REG18  18
#define GENERAL_CTRL_REG20  20  /*page 6*/
#define GENERAL_CTRL1_REG20  20 /*page 18*/
#define GENERAL_CTRL2_REG27  27 /*page 18*/

/* Marvell phy register number and bit mask
 */
#define PHY_REG(x) (x)
#define PHY_PAGE(x) (x)
#define PHY_REG_BIT(x) (1 << (x))

#define FIB_CTRL_REG0  0

typedef struct {   	 
   char name[10];  /* name of eth*/
	 int speed;   /* test speed */
	 int pkt_num; /* packet number */
	 int pkt_len; /* packet length */
	 boolean signal;  /* test signal */
	 ushort type; /* to avoid set env everytime */
	 int socket;
} diag_info_pthread_t;

/* Linux ethernet interface name */
#define NAME_ETH        "eth"
#define NAME_XAUI       "xaui"
#define NAME_MGMT       "mgmt"


/* lpbk_typ 1:external, 0:internal */
/* signal   1:fiber 0:copper */
/*
int pkt_cmp(unsigned char *, unsigned char *, int);
extern int woodlawn_err_clean_up(int);
extern int set_port_speed(char *, int);
extern int sig_pwr_ctrl(char *, boolean, boolean);
extern int cfg_phy_setting(char *, int, int, int, boolean);
extern int set_phy_stub(char *, boolean, boolean);
extern int phy_lpbk_type(char *, boolean, boolean);
extern int tx_rx_diag(char *, int, int, int, int, int );
extern int sfp_ext_lpbk_test_util(int);
extern int woodlawn_phy_lpbk_test(int, int);
extern int init_sgmii_env(char *, int, int, int);
extern int setup_xaui_port(int, int *);
extern int phy_check_iface_up_with_speed(char *, int);
extern int woodlawn_cavium_is_linkup(char *, int);
extern int woodlawn_phy_soft_reset (char *, boolean);
extern int force_linkup(boolean, int);
extern int sgmii_adv_full_duplex(boolean, int);
extern int direct_phy_soft_reset(int);
extern int set_bridge_phy_speed(int, int);
*/
extern int tsn_ge_lpbk_test(int, int);
extern int sig_pwr_ctrl(int, boolean, boolean);
extern int tx_rx_diag(char*, int, int, int, int, int);
extern int set_phy_stub(int, boolean, boolean);
extern int tsn_check_link_status(char *, int);
extern int sig_set_port_speed(int, int, boolean);
extern int cfg_phy_setting (int, int, int, int, boolean);
extern int tsn_phy_soft_reset(int, boolean);
extern int media_phy_ext_lpbk_test(int, int, int);
extern int tsn_sgmii_lpbk_test(int, int);
extern int tsn_reset_eth_phy(int);
extern int setup_eth_port (int, int *);

#endif  /* __PLARFORM_EXT_LPBK_H__ */


/*
$Log: platform_ext_lpbk.h,v $
Revision 1.4  2018/05/09 06:53:12  letsai
Add TSN GSHDSL portion

Revision 1.3  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.1  2017/11/07 09:44:26  hondwang
Change test card test with 1000base-X

Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/24 14:14:10  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3  2016/09/13 08:14:23  palin2
Added CPU to GE PHY MAC loopback test.

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.4  2016/04/29 10:14:56  palin2
Updated code and added support ext. loopback test after bring up Switch.

Revision 1.1.2.3  2016/04/26 20:48:49  palin2
Updated code after bring up SFP external loopback test.

Revision 1.1.2.2  2016/04/22 12:28:36  palin2
Updated code after bring up GE PHY external loopback test.

Revision 1.1.2.1  2016/03/29 02:50:02  palin2
Added GE PHY Diag.

$Endlog$
*/

