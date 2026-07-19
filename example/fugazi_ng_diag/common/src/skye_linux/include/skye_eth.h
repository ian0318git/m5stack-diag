/* $Id: skye_eth.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_eth.h,v $
 *------------------------------------------------------------------
 * Header file for platform ethernet code 
 *
 * Oct 2010 ptong
 * 
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2011-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __SHRINKRAY_ETH_H__
#define __SHRINKRAY_ETH_H__

#include <net/if.h>

#define MAX_7_PORT      7
#define MAX_5_PORT      5
#define MAX_4_PORT      4

/* Overlord platform internal IP addresses used in the diag
 */
#define HOST_ETH1_IP_ADDR                        "192.123.123.1"
#define HOST_ETH2_IP_ADDR                        "192.123.123.2"
#define HOST_ETH3_IP_ADDR                        "192.123.123.3"
#define TILERA_XAUI0_IP_ADDR                     "18.18.18.18"
#define TILERA_XAUI0_DUMMY_IP_ADDR               "18.18.18.20"
#define TILERA_XAUI1_IP_ADDR                     "19.19.19.19"
#define TILERA_XAUI1_DUMMY_IP_ADDR               "19.19.19.20"
#define TILERA_NETMASK                           "255.255.255.0"

#define SPD_10MBPS    10
#define SPD_100MBPS   100
#define SPD_1000MBPS   1000

#define ADDR_MEDIA_PHY 4
#define ADDR_BRIDGE_PHY 128

#define SEL_PORT_ETH "gbe"
#define PHY_ID_88E1514  0x00

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

#define RX_READY   0x1
#define RX_FINISH  0x10


/* The cpu port connected to the GE switch */
#define CPU_GESW_PORT         SGMII3
#define PLAT_SGMII_NUM_MAX    SGMII3
#define PLAT_RGMII_NUM_MAX    RGMII1

#define TILERA_GMX0_INF_ID       0
#define TILERA_GMX1_INF_ID       1
#define TILERA_GMX2_INF_ID       2
#define TILERA_GMX3_INF_ID       3
#define TILERA_GMX4_INF_ID       4
/* Woodlawn use Cavium CN68XX which XAUI is attach to GMX3 */
#define TILERA_XAUI_INF_ID       TILERA_GMX3_INF_ID
#define TILERA_BP_XAUI_INF_ID       TILERA_GMX2_INF_ID

#define MAX_TX_BUF          1536    /* Maximum Tx Buf size (0x600) */
#define MAX_RX_BUF          1536    /* Maximum Rx Buf size (0x600) */

#define CRC_SIZE               4

enum loopback_num {
    TILERA_INT_LPBK = 0,
    BRIDGE_PHY_INT_LPBK,
    MEDIA_PHY_INT_LPBK,
    SGMII_EXT_LPBK,
    SFP_EXT_LPBK,
    SGMII_INT_EXT_LPBK
};

enum eth_ge_port_num {
    GE_PORT0 = 0,
    GE_PORT1,
    GE_PORT2,
    GE_PORT3,
    GE_PORT4,
    GE_PORT5,
};

enum eth_sfp_port_num {
    SFP_PORT0 = 0,
    SFP_PORT1,
    SFP_PORT2,
    SFP_PORT3,
    SFP_PORT4,
    SFP_PORT5,
};

enum phy_port_num {
    PHY_PORT0 = 0,
    PHY_PORT1,
    PHY_PORT2,
    PHY_PORT3,
};

typedef struct tsec_bd {
   volatile ushort status;        /* Status Fields  */
   volatile ushort length;        /* Buffer length  */
   volatile uchar  *buf_ptr;      /* Buffer Pointer */
} tsec_bd_t;

typedef struct tsec_info_struct {
    unsigned char *name;
    unsigned int reg_base_addr;     /* ptr to SMI controller base address. */
    unsigned int phyaddr;
    unsigned int flags;
    unsigned char mac_addr[6];
    unsigned int ip_addr;
    unsigned int tsec_num;
    unsigned int tx_bd;             /* ptr to TxBD ring */
    unsigned int tx_buf;            /* ptr to start of tx_buf */
    unsigned int rx_bd;             /* ptr to RxBD ring */
    unsigned int rx_buf;            /* ptr to start of rx_buf */
} tsec_info_struct_t;

typedef struct skye_ge_ip {
    char *ge_name;
    char *ge_ip;     /* string to save IP of GE */
} skye_ge_ip_t;

extern int skye_receive_frames(int *, uchar *, int);
extern int skye_send_frames(int *, uchar *, int);

extern int phy_soft_reset(char *, int);
extern int set_phy_int_lpbk(char *ifname, int eth_num);
extern int set_phy_ext_lpbk(char *ifname, int eth_num);
extern boolean is_eth_phy_linkup (char *ifname, int portnum);
extern void phy_reg_dump(char *ifname, int portnum);
extern void phy_reg_access(void);
extern void skye_phy_reg_access(void);
extern int check_ext_lpbk_flag(void);
extern int phy_reg_wr(int, struct ifreq *, ushort, ushort);
extern int phy_reg_rd(int, struct ifreq *, ushort, ushort *);
extern int cfg_phy(char *ifname, int portnum, int speed, int duplex, int autoneg);


/* XAUI test function prototypes
 */
extern int xaui_internal_lpbk_test(void);
extern int xaui_external_lpbk_test(void);
extern int xaui_ping_test(void);
extern void display_xaui_port_status(void);
extern void dump_xaui_gmx_regs(void);
extern void dump_xaui_pcs_regs(void);
extern void xaui_int_lpbk_util(void);
extern int cleanup_tsec (int);
extern void dismem(unsigned char *, int, unsigned long, int);

#endif /* __SHRINKRAY_ETH_H__ */

/*-------------------------------------------------
 * $Log: skye_eth.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:28  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:39  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * shrinkray_eth.h:
 * Revision 1.2.8.1  2014/07/17 09:14:41  palin2
 * Added skye_ge_ip struct definition to fix compile error.
 *
 * Revision 1.2  2014/02/27 15:01:10  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.4  2013/10/10 00:36:22  steja
 * 1. Add TLK Utility PLL and Polarity TX RX switch
 * 2. Code update
 *
 * Revision 1.1.4.3  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.2  2013/09/13 07:00:00  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.3  2013/08/30 09:05:46  steja
 * Fix the typo define
 *
 * Revision 1.1.2.2  2013/08/15 11:30:32  steja
 * Add code command and respond ( Host <->GE <-> TILE CPU#0) for G2 (PPC & MIPS) platform
 *
 * Revision 1.1.2.1  2013/06/24 09:03:35  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 */
