/* $Id: platform_ext_lpbk.h,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_ext_lpbk.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * Oct 2011 Alan Peng
 *
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PLARFORM_EXT_LPBK_H__
#define __PLARFORM_EXT_LPBK_H__

/* setup delay time for driver to read the PHY reg. */
#define ETH_DRIVER_DELAY    1

/* Poll timeout */
#define ETH_DRIVER_POLL_TIMEOUT     (10000) /* 10 secs */

/* Woodlawn copper specific status register */
#define WOODLAWN_PHY_SPEED_MSK  0xC000
#define WOODLAWN_PHY_SPEED_OFFSET  14
#define WOODLAWN_PHY_SPD_1000   0x2
#define WOODLAWN_PHY_SPD_100   0x1
#define WOODLAWN_PHY_SPD_10    0x0

#define WOODLAWN_PHY_DUPLEX      0x2000
#define WOODLAWN_PHY_PAGE_REC    0x1000
#define WOODLAWN_PHY_RESOLVED    0x0800
#define WOODLAWN_PHY_COP_LINK    0x0400
#define WOODLAWN_PHY_TRANS_PAUSE_EN  0x0200
#define WOODLAWN_PHY_RECEV_PAUSE_EN  0x0100
/* reserved 0x0080*/
#define WOODLAWN_PHY_MDI_CO_STA   0x0040
#define WOODLAWN_PHY_DOWN_SHIFT   0x0020
#define WOODLAWN_PHY_COP_ENG_STA   0x0010
#define WOODLAWN_PHY_GL_LINK_STA   0x0008
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

#define FIBER_SPECIFIC_STATUS_REG 17
#define FIBER_STATUS_REG1     1
#define GEN_CONT_REG_1        20

#define FIB_CTRL_REG0  0
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
extern int sgmii_set_packet(int, int);
extern int force_linkup(boolean, int);
extern int sgmii_adv_full_duplex(boolean, int);
extern int direct_phy_soft_reset(int);
extern int set_bridge_phy_speed(int, int);

#endif /* __PLARFORM_EXT_LPBK_H__ */
/* end of module */

/*
$Log: platform_ext_lpbk.h,v $
Revision 1.2  2013/10/08 08:48:30  tirawan
Woodlawn collapsed to main trunk

Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
Branch into woodlawn-branch2 and port woodlawn code

Revision 1.1.2.1  2013/04/24 10:37:24  tirawan
Initial check-in for woodlawn linux code

Revision 1.5  2013/03/27 08:45:05  kuangik
Code cleanup

Revision 1.11  2013/03/07 13:54:57  leslie
Modify for XAUI loopback test.

Revision 1.10  2013/03/01 13:51:56  kuangik
Update Loopback Test, SFP Present, and SFP EEPROM display

Revision 1.9  2013/02/26 09:52:39  leslie
Fix tx rx sync time.

Revision 1.8  2013/02/18 08:19:36  leslie
Add declaration of init_sgmii_env function

Revision 1.7  2012/11/22 05:32:30  leslie
Extend tx rx sync time.

Revision 1.6  2012/11/20 01:35:46  leslie
Add macro definition.

Revision 1.5  2012/09/21 11:53:01  kody
Add fiber definition.

Revision 1.4  2012/08/03 10:16:56  evanli
Mapping to latest O2 source code on 20120726

Revision 1.2  2012/04/06 06:07:45  kuangik
Update for GE PHY Test

Revision 1.1.1.1  2012/02/10 05:59:50  kody
Initial imports Woodlawn project code base.

Revision 1.1.2.9  2011/12/22 00:55:57  alpeng
update register definition

Revision 1.1.2.7  2011/12/09 09:48:19  alpeng
add mac address detect mechanism

Revision 1.1.2.6  2011/12/05 15:04:27  alpeng
update bridge PHY internal loopback
fix menu item name
fix sfp return

Revision 1.1.2.5  2011/11/22 08:55:43  alpeng
clean up code and remove useless delay

Revision 1.1.2.3  2011/11/08 02:07:17  alpeng
modify the packet mount, type, size

Revision 1.1.2.2  2011/11/02 00:50:00  alpeng
update

$Endlog$
*/

