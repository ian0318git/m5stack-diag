/* $Id: skye_ext_lpbk.h,v 1.2 2015/05/25 03:59:11 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/skye_ext_lpbk.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2011-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef __SHRINKRAY_EXT_LPBK_H__
#define __SHRINKRAY_EXT_LPBK_H__

#define TX_RX_SYNC_TIME       10

enum
{
    ETH_MODE_FE10,
    ETH_MODE_FE100,
    ETH_MODE_GE,
};

/* define loopback mode */
enum
{
    SGMII_LPBK_NONE,        /* no loopback */
    SGMII_PHY_LPBK_INTERNAL,   /* internal loopback at marvell GE PHY */
};

typedef struct {   	 
    char name[10];  /* name of eth*/
    int speed;   /* test speed */
    int pkt_num; /* packet number */
    int pkt_len; /* packet length */
    boolean signal;  /* test signal */
    ushort type; /* to avoid set env everytime */
    int socket;
} diag_info_pthread_t;

int pkt_cmp(unsigned char *, unsigned char *, int);
extern int tx_rx_diag(char *, int, int, int, int, int );
extern int skye_phy_lpbk_test(int, int);
extern int setup_xaui_port(int, int *);
extern int skye_tilera_is_linkup(char *, int);
extern int enable_phy_lpbk(int);
extern int enable_phy_ext_lpbk(int);
extern int phy_88E1514_initial(void);

#endif /* __SHRINKRAY_EXT_LPBK_H__ */
/* end of module */

/*
--------------------------------------------------
$Log: skye_ext_lpbk.h,v $
Revision 1.2  2015/05/25 03:59:11  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:28  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.2  2014/08/14 12:24:25  steja
Remove debug message and add printf info for internal loopback

Revision 1.1.2.1  2014/07/21 01:56:39  palin2
Initial check-in Skye module side Diag code.

--------------------------------------------------
shrinkray_ext_lpbk.h:
Revision 1.2.8.1  2014/06/25 13:10:20  steja
Add External loopback to test 10/100/1000 Mbps , Internal loopback 1000 Mbps still debugging

Revision 1.2  2014/02/27 15:01:09  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.4  2013/09/29 04:03:29  iachang
CPU0 GE Backplane RX Debug utility
Support 88E1514 initial function
Support 88E1514 Power Enable/Disable function

Revision 1.1.4.3  2013/09/27 07:25:13  steja
update code for bringup

Revision 1.1.4.2  2013/09/13 07:00:00  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.1  2013/06/24 09:03:36  steja
Checkin :
- Support TLK10323 Loopback test & Utility
- Support MV1514 Loopback test

--------------------------------------------------
$Endlog$
*/

