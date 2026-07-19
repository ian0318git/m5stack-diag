/* $Id: platform_ext_lpbk.h,v 1.5 2018/12/21 00:58:16 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_ext_lpbk.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * Oct 2011 Alan Peng
 *
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __PLARFORM_EXT_LPBK_H__
#define __PLARFORM_EXT_LPBK_H__

/* setup delay time for driver to read the PHY reg. */
#define ETH_DRIVER_DELAY    1

/* ovld copper specific status register */
#define OVLD_PHY_SPEED_MSK  0xC000
#define OVLD_PHY_SPEED_OFFSET  14
#define OVLD_PHY_SPD_1000   0x2
#define OVLD_PHY_SPD_100   0x1
#define OVLD_PHY_SPD_10    0x0

#define OVLD_PHY_DUPLEX      0x2000
#define OVLD_PHY_PAGE_REC    0x1000
#define OVLD_PHY_RESOLVED    0x0800
#define OVLD_PHY_COP_LINK    0x0400
#define OVLD_PHY_TRANS_PAUSE_EN  0x0200
#define OVLD_PHY_RECEV_PAUSE_EN  0x0100
/* reserved 0x0080*/
#define OVLD_PHY_MDI_CO_STA   0x0040
#define OVLD_PHY_DOWN_SHIFT   0x0020
#define OVLD_PHY_COP_ENG_STA   0x0010
#define OVLD_PHY_GL_LINK_STA   0x0008
#define OVLD_PHY_DTE_PWR_STA   0x0004
#define OVLD_PHY_POLARITY   0x0002
#define OVLD_PHY_JABBER   0x0001


/* define for PHY setting */
#define SET_PHY_BIT15     0x8000
#define SET_PHY_BIT14     0x4000
#define SET_PHY_BIT13     0x2000
#define SET_PHY_BIT11     0x0800
#define SET_PHY_BIT10     0x0400
#define SET_PHY_BIT3       0x0008
#define SET_AUTO_MEDIA     0x0007
#define SET_ENG_DETECT     0x0300

#define TX_RX_SYNC_TIME       20

#define COP_CTRL_REG0         0
#define COP_STATUS_REG1       1
#define COP_AUTONEG_ADV_REG4  4
#define COP_SPEC_CTRL_REG16   16
#define COP_STATUS_REG17      17
#define MAC_SPEC_CTRL2_REG21  21

#define CHECKER_CTRL_REG18  18
#define GENERAL_CTRL_REG20  20  /*page 6*/
#define GENERAL_CTRL1_REG20  20 /*page 18*/
#define GENERAL_CTRL2_REG27  27 /*page 18*/

#define FIB_CTRL_REG0  0
#define FIB_SPEC_CTRL_REG2      26
#define FIB_OUTPUT_AMP_MSK      0x7
#define FIB_OUTPUT_AMP_VAL504   0x5

#define OVLD_PHY_PAGE0  0
#define OVLD_PHY_PAGE1  1
#define OVLD_PHY_PAGE2  2
#define OVLD_PHY_PAGE3  3
#define OVLD_PHY_PAGE4  4
#define OVLD_PHY_PAGE6  6
#define OVLD_PHY_PAGE8  8
#define OVLD_PHY_PAGE18  18
#define OVLD_PHY_PAGE22  22

/* Common Overlord PHY Reg. offset definitions */
#define OVLD_PHY_REG0    0
#define OVLD_PHY_REG9    9
#define OVLD_PHY_REG22  22
#define OVLD_PHY_REG26  26
#define OVLD_PHY_REG27  27

/* Marvell 1548 PHY - 1000BaseT Control Reg. (Page0, Register 9) */
#define MRVL1548_1000T_CTRL_REG   0x9
#define PHY_TESTMODE_MSK          0xE000
#define PHY_TESTMODE_OFF          13
#define PHY_TESTMODE_NORMAL       0x0   /* Normal Mode */
#define PHY_TESTMODE_1            0x1   /* TestMode 1-Transmit Waveform Test */
#define PHY_TESTMODE_2            0x2   /* TestMode 2-Transmit Jitter Test (Master) */
#define PHY_TESTMODE_3            0x3   /* TestMode 3-Transmit Jitter Test (Slave) */
#define PHY_TESTMODE_4            0x4   /* TestMode 4-Transmit Distortion Test */
#define PHY_TESTMODE_5            0x5   /* 10M TestMode  */
#define PHY_TESTMODE_6            0x6   /* 10M Data 0/1 TestMode */
#define PHY_TESTMODE_7            0x7   /* 100M TestMode */
#define PHY_TESTMODE_8            0x8   /* 100 TestMode */

/* Marvell phy register number and bit mask
 */
#define PHY_REG(x) (x)
#define PHY_REG_BIT(x) (1 << (x))

typedef struct {   	 
    char name[10];  /* name of eth*/
    int speed;   /* test speed */
    int pkt_num; /* packet number */
    int pkt_len; /* packet length */
    boolean signal;  /* test signal */
    ushort type; /* to avoid set env everytime */
    int socket;
} diag_info_pthread_t;

typedef struct {
    int  reg_page;  /* page of register */
    int  reg_off;   /* offset of register */
    uint16_t  val;  /* value to set */
    uint16_t  mask; /* mask of register r/w capability */
} mrvl_phy_setup_t;

/* Linux ethernet interface name */
#define NAME_ETH        "eth"
#define NAME_XAUI       "xaui"
#define NAME_MGMT       "mgmt"

/* Define for Vg400 */
#define INT_MASK          0x8
#define SPEED_INT         0x4000
#define INTR_TIMEOUT      100
#define SPEED_100M        0x2100
#define SPEED_1000M       0x0140
#define ETH_PORT1         1

/* lpbk_typ 1:external, 0:internal */
/* signal   1:fiber 0:copper */
int pkt_cmp(char volatile *, char volatile *, int);
extern int ovld_err_clean_up(int);
extern int set_port_speed(char *, int);
extern int sig_pwr_ctrl(int, boolean, boolean);
extern int cfg_phy_setting(int, int, int, int, boolean);
extern int set_phy_stub(int, boolean, boolean);
extern int ovld_phy_soft_reset(int, boolean);
extern int tx_rx_diag(char *, int, int, int, int, int );
extern int ovld_set_packet(int, int);
extern int sfp_ext_lpbk_test_util(int);
extern int utah_port_is_linkup(int);
extern void reset_quad_phy(void);

#endif /* __PLARFORM_EXT_LPBK_H__ */
/* end of module */

/*
$Log: platform_ext_lpbk.h,v $
Revision 1.5  2018/12/21 00:58:16  haohsu
CSCvn27142-Fixed 1548 PHY Interrupt test fail

Revision 1.4  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.3  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.2  2013/06/28 04:02:28  alpeng
for P1A check in, add media internal loopback test into menu

Revision 1.1  2013/05/31 11:03:41  alpeng
support front panel GE loopback test

Revision 1.10  2013/02/26 01:48:42  palin2
Fixed the utility to let GE PHY enter TestMode based on Marvell FAE's comments.

Revision 1.9  2013/01/30 23:50:16  palin2
Add utility to set Cavium side GE PHY, Marvell 1548, into Test mode.

Revision 1.8  2012/10/18 06:04:08  ptong
Fix bug: CSCuc64054, Overlord data plane 1548 PHY loopback test failed

Revision 1.7  2012/08/24 23:11:40  ptong
Increase the fiber output amplitude in PHY-1548

Revision 1.6  2012/08/01 14:26:33  alpeng
adding check link up status for SFP and internal loopback

Revision 1.5  2012/06/06 15:00:37  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/06 02:09:42  ptong
Extend semaphore lock time to 20 second

Revision 1.3  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
