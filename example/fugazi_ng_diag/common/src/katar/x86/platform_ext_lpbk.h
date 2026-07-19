/* $Id: platform_ext_lpbk.h,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_ext_lpbk.h,v $
 *------------------------------------------------------------------
 * Header file for linux base ethernet port tests
 * 
 * June 2016 Mecca Ho
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
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
  unsigned char transportSpecificAndMessageType;       // 00       1 (2 4-bit fields)
  unsigned char reserved1AndVersionPTP;                // 01       1 (2 4-bit fields)
  unsigned short messageLength;                         // 02       2
  unsigned char    domainNumber;                          // 04       1
  unsigned char    reserved2;                             // 05       1
  char        flags[2];                              // 06       2
  signed long long    correctionField;                       // 08       8
  unsigned int   reserved3;                             // 16       4
  PortIdentity sourcePortId;                          // 20      10
  unsigned short   sequenceId;                            // 30       2
  unsigned char    control;                               // 32       1
  unsigned char    logMeanMessageInterval;                // 33       1
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

#define TX_RX_SYNC_TIME       10

typedef struct {   	 
   char name[10];  /* name of eth*/
	 int speed;   /* test speed */
	 int pkt_num; /* packet number */
	 int pkt_len; /* packet length */
	 boolean signal;  /* test signal */
	 ushort type; /* to avoid set env everytime */
	 int socket;
         int rcv_count; /* for loopback should be 2 , for speed test should be 1 */
} diag_info_pthread_t;

/* Linux ethernet interface name */
#define NAME_ETH       "eth"

int pkt_cmp(unsigned char *, unsigned char *, int);

#endif /* __PLARFORM_EXT_LPBK_H__ */
/* end of module */

/*
 *------------------------------------------------------------------
 * $Log: platform_ext_lpbk.h,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.2  2019/05/29 05:59:18  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.1  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.1  2018/07/10 09:45:03  benlu
 * phy internal/external loopback
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

