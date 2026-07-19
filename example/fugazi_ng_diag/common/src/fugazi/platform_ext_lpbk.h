/* $Id: platform_ext_lpbk.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_ext_lpbk.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * June 2016 Mecca Ho
 * Jan 2019, Letsai modified for Fugazi.
 *
 * Copyright (c) 2016-2020 by Cisco Systems, Inc.
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

typedef struct
{
  char            clockIdentity[8];
  unsigned short       portNumber;
} volatile PortIdentity;

/** IEEE 1588 and IEEE 802.1AS PTP Version 2 common Message header structure */
typedef struct
{
  unsigned char transportSpecificAndMessageType;        // 00       1 (2 4-bit fields)
  unsigned char reserved1AndVersionPTP;                 // 01       1 (2 4-bit fields)
  unsigned short messageLength;                         // 02       2
  unsigned char    domainNumber;                        // 04       1
  unsigned char    reserved2;                           // 05       1
  char        flags[2];                                 // 06       2
  signed long long    correctionField;                  // 08       8
  unsigned int   reserved3;                             // 16       4
  PortIdentity sourcePortId;                            // 20      10
  unsigned short   sequenceId;                          // 30       2
  unsigned char    control;                             // 32       1
  unsigned char    logMeanMessageInterval;              // 33       1
} volatile V2MsgHeader;

#define PTP_MESSAGE_TYPE (0x0)
#define PTP_VERSION    (0x2)
#define PTP_PACKET_LENGTH (0x40)
#define PTP_MESSAGE_LENGTH (0x34)
#define PTP_ETHERNET_TYPE (0x88f7)
#define PTP_PKT_CMP_START_LEN_1 (0x0)
#define PTP_PKT_CMP_END_LEN_1 (0xb)
#define PTP_PKT_CMP_START_LEN_2 (0x30)
#define PTP_PKT_CMP_END_LEN_2 (0x39)
#define PTP_PKT_LEN_BIT_31_MASK (0x80000000)

/* define for PHY setting */
#define SET_PHY_BIT15       0x8000
#define SET_PHY_BIT14       0x4000
#define SET_PHY_BIT13       0x2000
#define SET_PHY_BIT11       0x0800
#define SET_PHY_BIT10       0x0400
#define SET_PHY_BIT9        0x0200
#define SET_PHY_BIT8        0x0100
#define SET_PHY_BIT7        0x0080
#define SET_PHY_BIT6        0x0040
#define SET_PHY_BIT5        0x0020
#define SET_PHY_BIT4        0x0010
#define SET_PHY_BIT3        0x0008
#define SET_PHY_BIT2        0x0004
#define SET_PHY_BIT1        0x0002
#define SET_PHY_BIT0        0x0001
#define SET_AUTO_MEDIA      0x0007
#define SET_ENG_DETECT      0x0300

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
#define NAME_ETH       "eth"
#define NAME_XFI       "xfi"
#define NAME_MGMT      "mgmt"


/* lpbk_typ 1:external, 0:internal */
/* signal   1:fiber 0:copper */
int pkt_cmp(unsigned char *, unsigned char *, int);

extern int eth_bgx2_list[];
extern int eth_mapping_sfp_num[];

extern int set_port_speed(char *, int);
extern int tx_rx_diag(char *, int, int, int, int, int );
extern int sfp_ext_lpbk_test_util(int, int);
extern int init_sgmii_env(char *, int, int, int);
extern int setup_xaui_port(int, int *);
extern int force_linkup(boolean, int);
extern int set_bridge_phy_speed(int, int);

extern int fugazi_err_clean_up(int);
extern int fugazi_cavium_is_linkup(char *, int);
extern int fugazi_phy_lpbk_test(int);
extern int fugazi_phy_lpbk_util(void);
extern int set_ge_phy_lpbk(char *, int, int, int, int, boolean);
extern int fugazi_set_packet(char *, int, int);
extern void show_eth_counter(char *, int);

#endif /* __PLARFORM_EXT_LPBK_H__ */
/* end of module */



/*
$Log: platform_ext_lpbk.h,v $
Revision 1.2  2021/06/02 08:22:35  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.1.8.2  2020/08/26 02:37:51  iachang
Merge Fugazi code into main trunk

Revision 1.1.6.9  2020/08/06 04:30:33  pdoong
clean code for BCM54194 1G PHY

Revision 1.1.6.8  2020/08/04 08:37:06  iachang
Update Copyright to 2020

Revision 1.1.6.7  2020/03/18 06:51:45  iachang
Create independent file for LASI test

Revision 1.1.6.6  2019/07/19 02:29:36  iachang
Sync loopback funtion with Curie-2RU
Changed Loopback funciton from Curie-2RU to ISR common function tx_rx_diag()
Changed BCM82757 print message "lane" to "port"

Revision 1.1.6.5  2019/04/18 01:21:30  letsai
1. Clean up code
2. Modify 1G phy address mapping
3. Modify print message of MCU FW opgrade

Revision 1.1.6.4  2019/04/10 21:26:59  letsai
1. Support BCM54194 PHY SGMII Internal Loopback test.
2. Return FAILED when M.2 module not present.
3. Clean up code.

Revision 1.1.6.3  2019/04/10 16:29:30  letsai
1. Fix ethernet mapping.
2. Support all BCM54194 phy in utilities.
3. Remove unused functions.

Revision 1.1.6.2  2019/03/14 03:48:27  letsai
Initial check in.



$Endlog$
*/

