 /* $Id: skye_eth.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_eth.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base ethernet port loopback test
 * 
 * May 2013, Steja
 * 
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "skye_eth.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "queryflags.h" /* for query user functions */ 
#include "skye_ext_lpbk.h" // 12.30


mac_addr_t host_mac_addr;

extern unsigned char tx_packet[ETH_PKT_MAX_LEN];
extern unsigned char rx_packet[ETH_PKT_MAX_LEN];
extern int receive_raw_packet(int, unsigned char *, int);
extern int send_raw_packet(int, unsigned char *, int);
extern int ping_test(char *);
extern boolean cpu_id;

/* Get mac address from cookie */
extern mac_addr_t mac_da;
extern mac_addr_t mac_sa;

/*****************************************************************************
 ***************************  Globals Variables   ****************************
 *****************************************************************************/
static skye_ge_ip_t skye_cpu0_ge_ip[] = {
    {
        .ge_name = "xgbe1",
        .ge_ip   = "192.168.1.101",
    },
    {
        .ge_name = "xgbe2",
        .ge_ip   = "192.168.1.201",
    },
    {
        .ge_name = "gbe3",
        .ge_ip   = "192.168.1.103",
    },
    {
        .ge_name = "gbe4",
        .ge_ip   = "192.123.123.101",
    },
    {
        .ge_name = "gbe5",
        .ge_ip   = "192.168.1.105",
    },
};
static skye_ge_ip_t skye_cpu1_ge_ip[] = {
    {
        .ge_name = "xgbe1",
        .ge_ip   = "192.168.1.102",
    },
    {
        .ge_name = "gbe2",
        .ge_ip   = "192.168.1.106",
    },

};

/***********************************************************************
 * Name: skye_receive_frames
 *
 * Description:
 *      This function receives frames
 *
 * Input: socket        - socket num
 *        rx_pkt_buf    - receive packet buffer
 *        len           - buffer length
 *
 * Output: PASSED or FAILED
 *
 ***********************************************************************
 */
int
skye_receive_frames (int *socket, uchar *rx_pkt_buf, int len)
{
   int raw;
   raw = *socket;

   /* clean up the rx_packet buffer */
   memset((unsigned char *)rx_pkt_buf, 0, ETH_PKT_MAX_LEN);

   if(receive_raw_packet(raw, (unsigned char *)rx_packet, len)){
       cterr('f',0, "error on receiving packet");
       return (FAILED);
   }

   rx_pkt_buf = (unsigned char *)rx_packet;
   memcpy(&host_mac_addr[0], (uchar *)&rx_pkt_buf[6], sizeof(mac_addr_t));

#if DEBUG
   printf("Host MAC address %02x:%02x:%02x:%02x:%02x:%02x",
	   host_mac_addr[0],
	   host_mac_addr[1],
	   host_mac_addr[2],
	   host_mac_addr[3],
	   host_mac_addr[4],
	   host_mac_addr[5]);fflush(0);
/* steja */
#endif

   return (PASSED);

}


/***********************************************************************
 * Name: skye_send_frames
 *
 * Description:
 *      This function send Ethernet frames
 *
 * Input: socket        - socket num
 *        rx_pkt_buf    - receive packet buffer
 *        len           - buffer length
 *
 * Output: PASSED or FAILED.
 *
 ***********************************************************************
 */
int
skye_send_frames (int *socket, uchar *rx_pkt_buf, int len)
{
    int raw, ix, val;
    uint mac_size, fil_len;
    unsigned char volatile *cptr;

    raw = *socket;
    /* clean up the tx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    cptr = (unsigned char *)tx_packet;
    mac_size = sizeof(mac_addr_t);

    val = 1;

    /* make ethernet frame */
    /* put in the destination/source mac address */
    memcpy((char *)cptr, (char *)mac_da, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy((char *)cptr, (char *)mac_sa, sizeof(mac_addr_t));
    cptr += mac_size;

    /* fill the packet. the len is include the size of mac address
      * we need to minus the size of mac address on len for filbyte
      */
     fil_len = (len - (2*mac_size));

     for (ix = 0; ix < fil_len ; ix++ , cptr++ ) {
         *cptr = val + ix;
     }

#if DEBUG
    printf("len = 0x%2x  \n", len);
    for ( ix =0; ix <len; ix++) {
       printf("tx_packet[%d] = 0x%2x  ", ix, tx_packet[ix]);
    }
#endif

    /* Transmit the frame */
    if(!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        cterr('f',0, "error on sending packet");
        return (FAILED);
    }

    return (PASSED);
}
/***********************************************************************
 * Name: skye_ge_alive_test
 *
 * Description: This function test to make sure the Ethernet is alive 
 *
 * Input : opt - Reserved for future use
 *
 * Return: PASS/FAIL
 *
 ***********************************************************************
 */
int
skye_ge_alive_test (int opt)
{

    int retval = 0, ge_num = 0, curr_ge = 0, ping_ret = 0;
    skye_ge_ip_t *test_ge;

    testname("GE Alive Test ");
  
    if (cpu_id == MASTER_CPU) {
        test_ge = &skye_cpu0_ge_ip[0];
        ge_num = (uint32_t)(sizeof(skye_cpu0_ge_ip) /
                               sizeof(skye_ge_ip_t));
        
    } else {
        test_ge = &skye_cpu1_ge_ip[0];
        ge_num = (uint32_t)(sizeof(skye_cpu1_ge_ip) /
                               sizeof(skye_ge_ip_t));
    }
    for (curr_ge = 0; curr_ge < ge_num; curr_ge++, test_ge++) {
        prpass(testpass, "Skye %s ",test_ge->ge_name);
        ping_ret = ping_test(test_ge->ge_ip);
        if (ping_ret != PASSED) {
            cterr('f',0,"Ethernet Ping Test Fail\n");
            retval = FAILED;
        } else {
            retval = PASSED;
        }    
    }

   return (retval);
}


/*-------------------------------------------------
 * $Log: skye_eth.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.4  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.3  2015/04/30 08:33:53  steja
 * Clean up code
 *
 * Revision 1.1.4.2  2015/04/29 11:36:35  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:55  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * skye_eth.c:
 * Revision 1.2.8.1  2014/07/14 08:05:32  iachang
 * Support GE alive test
 *
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.4  2014/02/07 04:48:02  steja
 * code clean up
 *
 * Revision 1.1.4.3  2013/10/10 00:36:22  steja
 * 1. Add TLK Utility PLL and Polarity TX RX switch
 * 2. Code update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:09  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.2  2013/08/15 11:30:33  steja
 * Add code command and respond ( Host <->GE <-> TILE CPU#0) for G2 (PPC & MIPS) platform
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 *-------------------------------------------------
 */

