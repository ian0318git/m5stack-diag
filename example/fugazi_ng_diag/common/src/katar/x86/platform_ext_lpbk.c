/* $Id: platform_ext_lpbk.c,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_ext_lpbk.c  
 * support PHY external loopback 
 * internal loopback: GE PHY and Cavium.
 *
 * June 2016 Mecca Ho
 * Copyright (c) 2016 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>  /* for bzero*/
#include<string.h>
#include<sys/socket.h>
#include<features.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<linux/ethtool.h> /*struct ethtool */
#include<linux/sockios.h> /* SIOCETHTOOL */
#include<sys/types.h> /* getpid */
#include<unistd.h>  /* getpid */
#include<netinet/in.h>  /* for including the linux_eth.h */
#include<ifaddrs.h> /* for using getifaddrs */
#include <stdint.h>

#include<errno.h>
#include<sys/ioctl.h>
#include<net/if.h>

#include<pthread.h>
#include<semaphore.h>
#include<time.h>

#include "nvsysvars.h"
#include "defs.h"
#include "proto.h"
#include "types.h"
#include "common.h"
#include "error.h"
#include "monitor.h"
#include "queryflags.h"
#include "menu.h"
#include "mb_tests.h"

#include "router_if.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"

#include "eth_pkt_utils.h"

#include "AQ_API.h" 

//#define USE_STATIC_MAC_ADDR

extern AQ_Retcode AQ_API_Set_Speed(AQ_API_Port* port, int Speed_1G);
extern int wait_iface_link_stats (int port_num, int exp_val);

void create_rx_buffer();
int force_linkup(boolean, int);
int direct_phy_soft_reset(int);
int neptune_err_clean_up(int);
int show_status_info(int);
void print_buf_content(unsigned char volatile *tptr, unsigned char volatile *rptr,int show_pkt_len);
int cfg_phy_setting(char *, int, int, int, boolean);
int is_glc_ge_100fx = 0;

/* Port Mapping of GE0 ~ GE3*/
int eth_mapping_ge_num[] = {ETH_GE_PORT0, ETH_GE_PORT1, ETH_10GE_PORT0, ETH_10GE_PORT1, ETH_2P5GE_PORT0, ETH_2P5GE_PORT1, 
                                                 ETH_2P5GE_PORT2, ETH_2P5GE_PORT3};


/* DLM 5/6 eth number from GE port 0 ~ 3*/
//int eth_bgx2_list[] = {ETH3, ETH4, ETH5, ETH6};
int eth_ge_list[] = {ETH_GE_PORT0, ETH_GE_PORT1};
int eth_ge2p5_list[] = {ETH_2P5GE_PORT0, ETH_2P5GE_PORT1, ETH_2P5GE_PORT2, ETH_2P5GE_PORT3};
int eth_ge10_list[] = {ETH_10GE_PORT0, ETH_10GE_PORT1};
/* QLM 2 eth number from TE port 0 ~ 1*/
int eth_bgx0_list[] = {ETH0, ETH1};

/* global */
unsigned int neptune_packet_count = 0;

/* packet buffer */
unsigned char tx_packet[ETH_FRAME_MAX_LEN];
unsigned char rx_packet[ETH_FRAME_MAX_LEN];

/* for ctrl diag flow */
volatile int tx_rx_box = 0;
sem_t rx_ready, rx_finish, tx_cmp;

/* Packets to be used in xaui port loopback tests
 * we leaave 12 byte for put mac address into the packet
 */
static pktdata_info_t pktdata[] = {
  {0xa5, ((ETH_UDP_DATA_MAX_LEN - 1) - 12), H_INCFILL, 1000},
  {0xa3, (ETH_UDP_DATA_MAX_LEN - 12), H_INCFILL, 1000},
  {0xa0, ETH_PKT_MIN_LEN, H_INCFILL, 1000},
  {0xa7, ETH_PKT_MIN_LEN, H_INCFILL, 1000},
};
static mac_addr_t mac_da = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static mac_addr_t mac_sa = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};

static char ifsrc[20] = {0};
static char ifdst[20] = {0};

static int      crc_table_inited;
static unsigned int crc_table[256];

/* Copper speed table */
static int ge2p5_eth_speed_list[] = {SPD_1000MBPS, SPD_2500MBPS};

#define PACKET_DROP_SKIP 1
#define PACKET_DROP_RETRY_COUNT 5

#if PACKET_DROP_SKIP
static int packet_drop_skip_enable = 0;
static int packet_drop=0;
#endif

int check_ge_int_lpbk_flag(void)
{
    return 0;
}
 
int check_ext_lpbk_flag(void)
{
    return 1;
}

unsigned int
swap32(unsigned int i)
{
    i = (i << 16) | (i >> 16);

    return (i & 0xff00ffff) >> 8 | (i & 0xffff00ff) << 8;
}

unsigned int
crc32(unsigned int crc, unsigned char *data, int len)
{
    int         i;

    if (!crc_table_inited) {
    int     j;
    unsigned int        accum;

    for (i = 0; i < 256; i++) {
        accum = i;

        for (j = 0; j < 8; j++) {
        if (accum & 1) {
            accum = accum >> 1 ^ 0xedb88320UL;
        } else {
            accum = accum >> 1;
        }
        }

        crc_table[i] = swap32(accum);
    }

    crc_table_inited = 1;
    }

    for (i = 0; i < len; i++) {
    crc = crc << 8 ^ crc_table[crc >> 24 ^ data[i]];
    }

    return crc;
}

/***********************************************************************
 * Function: create_raw_socket
 *   Create a Linux network socket for raw packet
 *
 * Input:
 *   ptotocol - Linux network socket protocol values such as
 *              AF_PACKET, ETH_P_ALL, etc.
 *
 * Return: socket ID which is an integer
 ***********************************************************************
 */
int create_raw_socket (int protocol)
{ 
    int rawsock;
    if((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol)))== -1) {
        perror("Error creating raw socket: ");
        return(-1);
    }
    return rawsock;
}
 
/***********************************************************************
 * Function: bind_socket
 *   Bind the socket to the ethernet interface specified by the
 *   char string in "device"
 *
 * Input:
 *   device - a string such as eth0, eth1, etc.
 *   rawsock - socket ID of socket to bind with the device
 *   protocol - Linux network protocol value such as AF_PACKET
 *
 * Return: PASSED/FAILED
 *
 ***********************************************************************
 */
int bind_socket (char *device, int rawsock, int protocol) {
    struct sockaddr_ll sll;
    struct ifreq ifr;
 
    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */
 
    sprintf((char *)ifr.ifr_name, device);
    if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        perror("Error getting Interface index !\n");
        return FAILED;
    }
 
    /* Bind our raw socket to this interface */
 
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol); 

    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll))) == -1) {
        perror("Error binding raw socket to interface\n");
        return FAILED;
    }
 
    return PASSED;
}


/***********************************************************************
 * Function: set_promisc
 *   Set the ethernet interface to promisc mode
 *
 * Input:
 *   device - Name of ethernet interface such as "eth0", eth1, etc.
 *   sock - socket ID
 *
 * Return: PASSED/FAILED
 ***********************************************************************
 */
int set_promisc (char *device, int sock) {

    struct ifreq ifr;
 
    bzero(&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */
    /* Set the network card in promiscuos mode */
    sprintf(ifr.ifr_name, device);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS get index interface");
        close(sock);
        return FAILED;
    }
    
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to set promiscous mode");
        close(sock);
        return FAILED;
    }

    return PASSED;
}
/*------------------------------------------------------------------
 *
 * Function:    setup_eth_port()
 *
 * Description: Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:   sgmii_port - host system sgmii port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:  PASS/FAIL
 *
 ************************************************************************
 */
int setup_eth_port (char *eth_name, int *socket)
{
    int raw;

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (set_promisc(eth_name, raw) == FAILED) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == FAILED) {
        return(FAIL);
    }

    *socket = raw;

    return PASS;
}

/*------------------------------------------------------------------
 *
 * Function: send_raw_packet
 *    set packet via socket.
 *
 * Input:  rawsock - socket
 *         pkt - tx buffer
 *         pkt_len - size of packet
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int send_raw_packet(int rawsock, unsigned char *pkt, int pkt_len)
{
    int sent= 0;
 
    /* A simple write on the socket ..thats all it takes ! */
    if((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
        return PASSED;
    }

    return FAILED;

}

/*
 * Function: pkt_cmp
 *
 * Description: Compare 2 packet's CRC
 *
 * Input: bufa - buffer of first packet
 *        bufb - buffer of 2nd packet
 *        count - number of bytes to compare
 *
 * Return: pass/fail
 */
int pkt_cmp (unsigned char *bufa, unsigned char *bufb, int count)
{
    int rc = PASSED;
    uint crc_tx, crc_rx;

    crc_tx = ~crc32(~0, (unsigned char *)bufa, count);
    crc_rx = ~crc32(~0, (unsigned char *)bufb, count);
    if (crc_tx != crc_rx) {
        printf("tx crc doesn't match rx crc\n");
        rc = FAILED;
    }
    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: katar_is_linkup
 *   Check linux up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int katar_is_linkup (char *type, int eth_num)
{
    int timeout_counter = 100, is_link = FALSE;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];

    sprintf(pname,"%s%d", type, eth_num);

    while(1) {
        /* Get the interface information */
        if (getifaddrs(&if_list) < 0) {
            printf("%s(): %s Failed to get interface information: %s.\n",
                   __FUNCTION__, pname, strerror(errno));
            return (FAILED);
        }
        if (if_list == NULL) {
            printf("%s(): %s No network interfaces were found.\n",
                    __FUNCTION__, pname);
            return (FAILED);
        }

        for (if_info = if_list; if_info; if_info = if_info->ifa_next) {
            /* parse the port name */
            if (strncmp(if_info->ifa_name, pname, IFNAMSIZ) != 0) {
                continue;
            }

             /* printf("%s ", if_info->ifa_name); */

             flags = if_info->ifa_flags;
             if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
               /* printf("up\n"); */
                 fflush(stdout);
                 is_link = TRUE;
                 break;
             } else {
                 /* printf("down\n");  */
                 msleep(10);
                 timeout_counter--;
                 if (timeout_counter == 0) {
                     return (FAILED);
                 }
             }
             fflush(stdout);
        } /*for*/

        freeifaddrs(if_list);

        if (is_link == TRUE) {
            break;
        }
    } /*while */

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: check_pkt
 *   Compared the packet between the buffer of tx and rx.
 *
 * Input:  NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int check_pkt (int pkt_len){

    if ((pkt_cmp(tx_packet, rx_packet, pkt_len)) == PASSED) {
#if DEBUG
        printf("%s() Rx packet matched\n", __func__);
#endif
//printf("tx_packet %2x %2x %2x %2x, rx_packet %2x %2x %2x %2x, 1count =%d\n",*tx_packet, *(tx_packet+1), *(tx_packet+6), *(tx_packet+7), *rx_packet, *(rx_packet+1), *(rx_packet+6), *(rx_packet+7));

        return PASSED;
    } else {
        printf("%s() Rx packet mismatched\n", __func__);

//printf("tx_packet %2x %2x %2x %2x, rx_packet %2x %2x %2x %2x, 1count =%d\n",*tx_packet, *(tx_packet+1), *(tx_packet+6), *(tx_packet+7), *rx_packet, *(rx_packet+1), *(rx_packet+6), *(rx_packet+7));

        return (FAILED);
    }

}

/*
 * Function: chk_macaddr
 * Compare if 2 mac addresses matches
 *
 * Input:
 * macaddr1 and macaddr2 - 2 mac addresses to be compared
 *
 * Return: 0 when matched
 */
static int chk_macaddr(uchar *macaddr1, uchar *macaddr2)
{
    return(memcmp(macaddr1, macaddr2, 6));
}

/*------------------------------------------------------------------
 *
 * Function: ptp_check_pkt
 *   Compared the ptp packet between the buffer of tx and rx.
 *
 * Input:  NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ptp_check_pkt (int pkt_len)
{
    int ix, timestamp_count = 0;
    int tx_mismatch_buf[10] = {0};
    int rx_mismatch_buf[10] = {0};
    unsigned char *p1 = tx_packet;
    unsigned char *p2 = rx_packet;

    /* Check first 12 bytes, byte 1~ byte 12, tx must equal to rx */
    for (ix = PTP_PKT_CMP_START_LEN_1; ix <= PTP_PKT_CMP_END_LEN_1; ix++) {
        if (p1[ix] != p2[ix]) {
            printf("Packet mismatched on byte %d, tx_packet data = %02x, rx_packet data = %02x",
                   ix, p1[ix], p2[ix]);

            return (FAILED);
        }
    }

    /* Check timestamps 10 bytes, byte 49~byte 58, will give timestamps init val,
        after update the timestamps the value will be changed, but it's possible the value
        will same with the original val */
    for (ix = PTP_PKT_CMP_START_LEN_2; ix <= PTP_PKT_CMP_END_LEN_2; ix++) {
        if (p1[ix] == p2[ix]) {
            tx_mismatch_buf[timestamp_count] = p1[ix];
            rx_mismatch_buf[timestamp_count] = p2[ix];
            timestamp_count ++;
        }
    }

    /* If 10 bytes timestamps values keep the same, then timestamp update fail */
    if (timestamp_count == 10) {
        timestamp_count =0;
        printf("Timestamp is not updated. Content:\n");
        for (ix = PTP_PKT_CMP_START_LEN_2; ix <= PTP_PKT_CMP_END_LEN_2 ; ix++) {
            printf("Timestamp-tx[%d]=%02x, Timestamp-rx[%d]=%02x\n",
                   ix, tx_mismatch_buf[timestamp_count],
                   ix, rx_mismatch_buf[timestamp_count]);
            timestamp_count ++;
        }
        printf("\n");

        return (FAILED);
    }
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: send_packets
 *   for tx send packet to rx. if number of packet is too much,
 *   then the delay is needed.
 *
 * Input:  len - packet length
 *         val - content of packet
 *
 * Output: PASSED/FAILED
 * 
 *------------------------------------------------------------------
 */
int send_packets(int *socket, char *iface_type, int len, char val, int port, int speed)
{   
    int raw, rc = 0;
    //int raw;
	unsigned short pkt_type = 0x8ff0;
    uint mac_size, fil_len, crc;
    unsigned char volatile *cptr;

    raw = *socket;

    /* clean up the rx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_FRAME_MAX_LEN);
    
    cptr = (unsigned char *)tx_packet;
    mac_size = sizeof(mac_addr_t);

    /* put in the destination/source mac address */
    memcpy((char *)cptr, (char *)mac_da, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy((char *)cptr, (char *)mac_sa, sizeof(mac_addr_t));
    cptr += mac_size; 
	*cptr++ = (pkt_type >> 8) & 0xff;
    *cptr++ = pkt_type & 0xff;
 
    /* fill the packet. the len is include the size of mac address 
     * we need to minus the size of mac address on len for filbyte
     */
    fil_len = (len - ETH_HDR_LEN);

    filbyte((uchar *)cptr, fil_len, val);
    cptr += fil_len;

    /* Add crc after the payload
     */
    crc = ~crc32(~0, (unsigned char *)tx_packet, len);
    *cptr++ = (crc >> 24) & 0xff;
    *cptr++ = (crc >> 16) & 0xff;
    *cptr++ = (crc >> 8) & 0xff;
    *cptr++ = crc & 0xff;
    len += ETH_PKT_CRC_LEN;

    rc = katar_is_linkup(iface_type, port);
    if (rc == FAILED) {
        cterr('f',0, "port link up time out");
        return (FAILED);
    }

    if(!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        cterr('f',0, "error on sending packet");
        return (FAILED);
    }    
    
    return (PASSED);

}

/*------------------------------------------------------------------
 *
 * Function: force_linkup
 *   for internal loopback force PHY link up to
 *   prevent there is no external stub connect to the port
 *   then the linux will let port link down.
 *   all of the internal lpbk test will need to turn
 *   on this function.
 *
 *
 * Input:  onoff - turn on/off
 *         phy_id - phy addr for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int force_linkup (boolean onoff, int phy_id)
{
    return 0;
}


/*------------------------------------------------------------------
 *
 * Function: katar_err_clean_up
 *   using this function when diag failed, to prevent the endless
 *   'Trying speed' message from ethtool. when external loopback
 *   failed, ethtool will keep trying speed.
 *
 * Input:  num_eth - port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int katar_err_clean_up (int port)
{
    char pname[10];

    sprintf(pname, "eth%d", port);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_status_info(port)) {
            printf("Show status info failed. ");
            return (FAILED);
        }
    }

    /* to prevent the endless message from ethtool setting */
    if (force_linkup(ENABLE, port)) {
        cterr('f', 0, "%s force_linkup failed.", pname);
        return (FAILED);
    }
    
    /* cfg port to 10 or 100 spd, or 1000 spd will fail on clean up,
     * because force link up is not support on 1000 spd.
     */
    sleep(ETH_DRIVER_DELAY); 
    if ((cfg_phy_setting(pname, SPD_10MBPS, FULL_DUPLEX, AUTONEG_OFF, SIG_COPPER))) {
        cterr('f', 0, "%s cfg_phy_setting failed", pname);
        return (FAILED);
    }
    sleep(ETH_DRIVER_DELAY);

    /* after link up, disable the register. */
    if (force_linkup(DISABLE, port)) {
        cterr('f', 0, "%s force_linkup failed.", pname);
        return (FAILED);
    }
    
    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: show_status_info
 *   print out the reg info on page 0 reg 17.
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int show_status_info(int port)
{
    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: cfg_phy_setting
 *  this function is not like set_port_speed() which is based on 
 *  ethtool and may let other reg reset.
 *  cfg_phy_setting will config PHY reg for speed directly 
 *  and switch the page to let driver detect the setting.
 *
 * Input:  ifname - port name.
 *         speed - setup speed 
 *         duplex - turn full/half duplex
 *         autoneg - turn on/off autoneg        
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int cfg_phy_setting(char *ifname, int speed, int duplex, int autoneg, boolean signal)
{
    return 0;
}
extern unsigned int AQ_reg_read(unsigned int port, unsigned int reg_mmd, unsigned int reg_num);

void dump_AQ_statistic(void)
{
    int i;
    unsigned int CRC_E=0, LDPC_E=0, LS_TG=0, LS_TB=0, LS_RG=0, LS_RB=0, SS_TG=0, SS_TB=0, SS_RG=0, SS_RB=0;
    //unsigned int XSS_TG=0, XSS_TB=0, XSS_RG=0, XSS_RB=0;
    unsigned int CRC_E_L=0, LDPC_E_L=0, LS_TG_L=0, LS_TB_L=0, LS_RG_L=0, LS_RB_L=0, SS_TG_L=0, SS_TB_L=0, SS_RG_L=0, SS_RB_L=0;
    //unsigned int XSS_TG_L=0, XSS_TB_L=0, XSS_RG_L=0, XSS_RB_L=0;

    for ( i = 0 ; i < 4 ; i++)
    {
        CRC_E = AQ_reg_read( i, 3, 0xE811);
        CRC_E_L = AQ_reg_read( i, 3, 0xE810);

        LDPC_E = AQ_reg_read( i, 3, 0xE821);
        LDPC_E_L = AQ_reg_read( i, 3, 0xE820);

        LS_TG = AQ_reg_read( i, 3, 0xC821);
        LS_TG_L = AQ_reg_read( i, 3, 0xC820);

        LS_TB = AQ_reg_read( i, 3, 0xC823);
        LS_TB_L = AQ_reg_read( i, 3, 0xC822);

        LS_RG = AQ_reg_read( i, 3, 0xE813);
        LS_RG_L = AQ_reg_read( i, 3, 0xE812);

        LS_RB = AQ_reg_read( i, 3, 0xE815);
        LS_RB_L = AQ_reg_read( i, 3, 0xE814);

        SS_TG = AQ_reg_read( i, 3, 0xC861);
        SS_TG_L = AQ_reg_read( i, 3, 0xC860);

        SS_TB = AQ_reg_read( i, 3, 0xC863);
        SS_TB_L = AQ_reg_read( i, 3, 0xC862);

        SS_RG = AQ_reg_read( i, 3, 0xE861);
        SS_RG_L = AQ_reg_read( i, 3, 0xE860);

        SS_RB = AQ_reg_read( i, 3, 0xE863);
        SS_RB_L = AQ_reg_read( i, 3, 0xE862);
        printf("ID%d : CRC_E=%x, CRC_E_L=%x, LDPC_E=%x, LDPC_E_L=%x, LS_TG=%x, LS_TG_L=%x, LS_TB=%x, LS_TB_L=%x, LS_RG=%x, LS_RG_L=%x, \n",
                   i, CRC_E, CRC_E_L, LDPC_E, LDPC_E_L, LS_TG, LS_TG_L, LS_TB, LS_TB_L, LS_RG, LS_RG_L);
        printf("ID%d : LS_RB=%x, LS_RB_L=%x,  SS_TG=%x,  SS_TG_L=%x, SS_TB=%x, SS_TB_L=%x, SS_RG=%x, SS_RG_L=%x, SS_RB=%x, SS_RB_L=%x, \n",
                   i, LS_RB, LS_RB_L, SS_TG, SS_TG_L, SS_TB, SS_TB_L, SS_RG, SS_RG_L, SS_RB, SS_RB_L);
    }
}

/*------------------------------------------------------------------
 *
 * Function: receive_packets
 *  Create socket, setup port speed for receiving packet.
 *
 * Input:  argument - pass arg for pthread, now is amount of packet.
 *         
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
void *receive_packets (diag_info_pthread_t *get_info)
{
    int rx= 0, rc = 0;
    struct timespec ts;
    uint ii, pkt_cnt = 0;
    uchar *rx_pkt_buf;
    struct timeval tv;
    int otherpkt_cnt = 0;
	unsigned char err_packet[ETH_FRAME_MAX_LEN]={0};

#if DEBUG
    int yy;

    printf(" name %s ", get_info->name);
    printf(" speed %d ", get_info->speed);
    printf(" pkt_num %d ", get_info->pkt_num);
    printf(" pkt_len %d ", get_info->pkt_len);
    printf(" socket %d ", get_info->socket);
#endif
    
    rx_pkt_buf = (uchar *)rx_packet;

    /* Set up tv for socket time out.
     * Get the second and microsecond portion of wait time
     * to set the socket time out.
     */
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = TX_RX_SYNC_TIME;   /* sec portion of wait time */
    tv.tv_usec = 0;  /* microsec portion of wait time */
    if (setsockopt(get_info->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
        pthread_exit ((void *)FAILED);
    }

    for (ii = 0; ii < get_info->pkt_num; ii++) {

        pkt_cnt = 0;

        /* inform tx for rx is ready to receive packet. */
        if (sem_post(&rx_ready) != PASSED) {
            if (errno == EINVAL)
                printf("The sem(rx_ready) does not refer to a valid semaphore \n");
            else
                printf("The function sem_post() is not supported by this implementation\n");
            pthread_exit ((void *)FAILED);
        }
        
        do {
            /* clear the rx_packet buffer */
            memset((char *)rx_packet, 0, ETH_FRAME_MAX_LEN);

            rx = read(get_info->socket, (unsigned char *)rx_pkt_buf, get_info->pkt_len);

            if (rx < 0) {
				int skterror = 0;
				socklen_t sktlen = sizeof (skterror);
				int retval = 0;
				char cmd[1024];
				
				printf("\nread error : %s.", strerror(errno));
				retval = getsockopt (get_info->socket, SOL_SOCKET, SO_ERROR, &skterror, &sktlen);
				printf("get socket rc : %s; socket error : %s\n", strerror(retval),strerror(skterror));
				printf("%s rx= %d socket %d read timeout. loop(ii)= %d, pkt_cnt = %d otherpkt_cnt= %d\n",
                           __FUNCTION__, rx, get_info->socket, ii, pkt_cnt, otherpkt_cnt);

/*if (ii == 0)*/
{
    /*printf("\n 1st packet drop issue happen !!\n");*/
#if PACKET_DROP_SKIP
    if (ii == 0)
        packet_drop =1;
    else
        packet_drop =2;
#endif
}

				if(otherpkt_cnt)
					print_buf_content(tx_packet,err_packet,get_info->pkt_len);

				system("ifconfig");
#if PACKET_DROP_SKIP
                if (packet_drop_skip_enable == 0)
                {
                    printf("Tx(%s) NIC statistics:\n",ifsrc);
                    sprintf(cmd,"ethtool -S %s ",ifsrc);
                    system(cmd);
                    printf("Rx(%s) NIC statistics:\n",ifdst);
                    sprintf(cmd,"ethtool -S %s ",ifdst);
                    system(cmd);
                }
#else
                printf("Tx(%s) NIC statistics:\n",ifsrc);
                sprintf(cmd,"ethtool -S %s ",ifsrc);
                system(cmd);
                printf("Rx(%s) NIC statistics:\n",ifdst);
                sprintf(cmd,"ethtool -S %s ",ifdst);
                system(cmd);
#endif

				printf("2.5G phy static:\n");
				dump_AQ_statistic();
                break; /* exit do loop */
            }

            /* drop invalid packet*/
            if (chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) {
                otherpkt_cnt++;
				
				memcpy(err_packet, rx_pkt_buf, get_info->pkt_len);

                if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
                    printf("\n detected non diag packet at pkt %d. Ignore.\n", ii);
                    printf("\n\n>>>> %s %s %d bytes received: !!! pkt_cnt= %d\n",
                           __FUNCTION__, get_info->name, rx, pkt_cnt);

                    printf("Destination MAC address: "
                           "%02x:%02x:%02x:%02x:%02x:%02x\n",
                           rx_pkt_buf[0],rx_pkt_buf[1],rx_pkt_buf[2],
                           rx_pkt_buf[3],rx_pkt_buf[4],rx_pkt_buf[5]);
                    printf("Source MAC address: "
                           "%02x:%02x:%02x:%02x:%02x:%02x\n",
                           rx_pkt_buf[6],rx_pkt_buf[7],rx_pkt_buf[8],
                           rx_pkt_buf[9],rx_pkt_buf[10],rx_pkt_buf[11]);
                }

#if DEBUG
                for (yy=0; yy < rx; yy++) {
                  if ((yy > 0) && (yy % 16) == 0) {
                    printf("\n");
                  }
                  printf("%02x ", rx_pkt_buf[yy]);
                }
                printf("end of pkt print\n");
#endif

                        continue;
            }
            pkt_cnt++;

#if DEBUG
            printf("%d bytes received: !!! , pkt_cnt = %d \n", rx, pkt_cnt);
#endif
        } while (pkt_cnt < (get_info->rcv_count));  
        /* loopback should get double of packets, one from original path,
         * another one from driver. 
         */

        /* the receive packets pkt_cnt are double of pkt_num
         * one from hw, another from driver. Inform tx for
         * rx read packet is finish.
         */
        if (sem_post(&rx_finish)) {
            if (errno == EINVAL){
                printf("The sem(rx_finish) does not refer to a valid semaphore \n");
            } else {
                printf("The function sem_post() is not supported by this implementation\n");
            }
            pthread_exit ((void *)FAILED);
        }

        if (rx < 0) {
            pthread_exit ((void *)FAILED);
        }

        /* Add some more time to wait for sem tx_cmp to be unlocked
         */
        /* init timeout value. */
        rc = clock_gettime(CLOCK_REALTIME, &ts);
        if (rc != PASSED) {
            printf("clock gettime failed..\n");
            pthread_exit ((void *)FAILED);
        }
        ts.tv_sec += TX_RX_SYNC_TIME; /* TX_RX_SYNC_TIME 10*/
        
        /* wait for tx compare the tx_packet and rx_packet.  */
        rc = sem_timedwait(&tx_cmp, &ts);
        if (rc != PASSED) {
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on tx_cmp timeout. \n");
            } else {
                printf("semaphore wait on tx_cmp failed. \n");
            }
            pthread_exit ((void *)FAILED);
        }
    }  /* for*/

    pthread_exit((void *)PASSED);
}


void show_buf_content(int show_pkt_len) {

	uint ii;
	unsigned char volatile *tptr, *rptr;
    
	tptr = tx_packet;
	rptr = rx_packet;
    
	printf("\nstart of pkt print(length:%d).\n",show_pkt_len);   
	for (ii=0; ii < show_pkt_len; ii++) {
		if ((ii > 0) && (ii % 8) == 0) {
			printf("\n");
		}
		printf("tx:%02x rx:%02x  ", tptr[ii], rptr[ii]);
	}
	printf("\nend of pkt print.\n");
}

void print_buf_content(unsigned char volatile *tptr, unsigned char volatile *rptr,int show_pkt_len) {
    uint i,j;

#if PACKET_DROP_SKIP
    if (packet_drop_skip_enable == 0)
    {
#endif
        printf("\nstart of pkt print(length:%d).\n",show_pkt_len);
        for (i=0; i < show_pkt_len; i++) {
            if((i%16)==0)
                printf("tx %04x: ",i);
            printf("%02x ",tptr[i]);
            if(((i%16)==15)||(i==(show_pkt_len-1)))
            {
                printf("\n");
                for(j=((i/16)*16); j<=i; j++) {
                    if((j%16)==0)
                        printf("rx %04x: ",j);
                    printf("%02x ",rptr[j]);
                    if((j%16)==15)
                        printf("\n");
                }
            }
        }
        printf("\nend of pkt print.\n");
#if PACKET_DROP_SKIP
    }
#endif
}

#ifndef USE_STATIC_MAC_ADDR
/* Convert a string of MAC address "xx:xx:xx:xx:xx:xx" to
 * 6 byte uchar numbers
 */
int macstr2macaddr(uchar *macstr, mac_addr_t *mac_buf)
{
    char tmp_mac[6];
    uchar *cptr, tmpstr[4];
    int ii, tmp_hex, count;

    ii = 0;
    count = 0;
    cptr = macstr;
    do {
      memset(tmpstr, 0, sizeof(tmpstr));
      memcpy(tmpstr, cptr, 2);
      count += sscanf((char *)tmpstr, "%x", &tmp_hex);
      tmp_mac[ii] = (uchar)tmp_hex;
      ii++;
      cptr += 3; /* point to next mac byte */
    } while(ii < 6);

    if (count == 6) {
        memcpy(mac_buf, tmp_mac, sizeof(mac_addr_t));
    return(0);
    }
    else {
        return(-1);
    }
}

void system_mac_addr_get
(char *name, mac_addr_t *mac_buf)
{
    char file_name[70];
    FILE *stream_p;
    uchar macstr[] ="00:01:02:03:04:05";

    sprintf(file_name, "/sys/class/net/%s/address", name);

    stream_p = fopen(file_name, "r");
    if (stream_p == NULL) {
        printf("can't open %s\n", file_name);
        exit(-1); /* software error so exit */
    }
    fscanf(stream_p, "%s", macstr);
    fclose(stream_p);

    macstr2macaddr(macstr, mac_buf);
}
#endif

int tx_rx_speed_diag(char* p_type, int eth_port, int port2, int speed, int pkt_cnt, int pkt_len, int value) 
{
    pthread_t threads;
    struct timespec ts;
    diag_info_pthread_t rx_info;
    int ii;
    int tx_skt, rx_skt;
    int rc;
    void  *pthr_rv;

    /* init the semaphore. */
    rc = sem_init(&rx_ready, 0, 0 );
    if (rc != PASSED) {
        printf("eth_port %d sem_init on rx_ready failed.\n", eth_port);
        return (FAILED);
    }
    
    rc = sem_init(&rx_finish, 0, 0 );
    if (rc != PASSED) {
        printf("eth_port %d sem_init on rx_finish failed.\n", eth_port);
        return (FAILED);
    }
    
    rc = sem_init(&tx_cmp, 0, 0 );
    if (rc != PASSED) {
        printf("eth_port %d sem_init on tx_cmp failed.\n", eth_port);
        return (FAILED);
    }
    
    sprintf(ifsrc,"%s%d", p_type, eth_port);
    sprintf(ifdst,"%s%d", p_type, port2);

#ifndef USE_STATIC_MAC_ADDR
    system_mac_addr_get(ifdst, &mac_da);
    system_mac_addr_get(ifsrc, &mac_sa);
#endif

    /* setup ETH tx and rx socket */
    if (setup_eth_port(ifsrc, &tx_skt) == FAIL) {
        return(FAILED);
    }

    if (setup_eth_port(ifdst, &rx_skt) == FAIL) {
        return(FAILED);
    }

    /* extend the space for putting the dest/src mac address */
    pkt_len += (2*sizeof(mac_addr_t));

    /* set up global value for both rx and tx on struct*/
    strncpy(rx_info.name,ifdst ,IFNAMSIZ);
    rx_info.speed = speed;
    rx_info.pkt_num = pkt_cnt;
    rx_info.pkt_len = pkt_len;
    rx_info.socket = rx_skt;
    if ( eth_port == port2 ) /* Loopback test */
        rx_info.rcv_count = 2;
    else  /* speed test */
        rx_info.rcv_count = 1; 

    /* build another thread for rx, and pass rx_info to rx */
    if (pthread_create(&threads, NULL, (void *)receive_packets, (diag_info_pthread_t *) &rx_info)) {
        cterr('f',0, "pthread_create failed");
        return (FAILED);
    }

    for (ii = 0; ii < pkt_cnt; ii++) {
        
        /* init timeout value. */
        rc = clock_gettime(CLOCK_REALTIME, &ts);
        if (rc != PASSED) {
            printf("clock gettime failed..\n");
            goto exit_tx_rx_speed_diag;
        }
        ts.tv_sec += TX_RX_SYNC_TIME;

        /* wait for the setting of rx side */
        rc = sem_timedwait(&rx_ready, &ts);
        if (rc != PASSED) {
	      print_buf_content(tx_packet,rx_packet,rx_info.pkt_len);
              //show_buf_content(rx_info.pkt_len);
              if (errno == ETIMEDOUT) {
                  printf("sem_timedwait on rx_ready timeout.\n");
              } else {
                  printf("semaphore wait on rx ready failed.\n");
              }
            goto exit_tx_rx_speed_diag;
        }

        msleep(1); /* ensure rx read is ready before tx */
        
        /* the main thread prepare to sending packet. */
        rc = send_packets(&tx_skt, p_type, pkt_len, (value+ii), eth_port, speed);

        if (rc == FAILED) {
            printf("send_packets failed\n");
            goto exit_tx_rx_speed_diag;
        }

        /* Add some more time to wait for sem rx_finish to be unlocked
         */
        ts.tv_sec += TX_RX_SYNC_TIME;
        /* use semaphore to detect timeout on rx side */
        rc = sem_timedwait(&rx_finish, &ts);
        if (rc != PASSED) {
			print_buf_content(tx_packet,rx_packet,rx_info.pkt_len);
            //show_buf_content(rx_info.pkt_len);
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on rx_finish timeout. (Packet-%d)\n", ii);
            } else {
                printf("semaphore wait on rx finish failed. (Packet-%d)\n", ii);
            }
            goto exit_tx_rx_speed_diag;
        }
	    /* compare the packet on rx_packet and tx_packet */
    	rc = check_pkt(pkt_len);
	    if (rc == PASSED) {
    		/* if match, clean up rx buffer for next packet. */
        	memset((unsigned char *)tx_packet, 0, ETH_FRAME_MAX_LEN);
            memset((unsigned char *)rx_packet, 0, ETH_FRAME_MAX_LEN);
	    } else {
	        printf("%s: mismatch\n", __FUNCTION__);
			print_buf_content(tx_packet,rx_packet,rx_info.pkt_len);
    	    //show_buf_content(rx_info.pkt_len);
        	goto exit_tx_rx_speed_diag;
	    }
	
        /* inform rx for read next packet. */
        if (sem_post(&tx_cmp)){
            if (errno == EINVAL){
                printf("The sem(tx_cmp) does not refer to a valid semaphore\n");
            } else {
                printf("The function sem_post() is not supported by this implementation\n");
            }
            return (FAILED);
        }

    }  /* for */

exit_tx_rx_speed_diag:

    /* if failed, cancel the thread */
    if(rc != PASSED)
        pthread_cancel(threads);
        
    /* Sync the tx and rx in here and check the rx is pass or fail */
    pthread_join(threads, (void **)&pthr_rv);
    if (pthr_rv != PASSED) {
        printf("tx_rx_speed_diag receive failed\n");
        rc = FAILED;
    } else {
        rc = PASSED;
    }

    /* close socket. */
    close(tx_skt);
    close(rx_skt);
    sem_destroy(&rx_ready);
    sem_destroy(&rx_finish);
    sem_destroy(&tx_cmp);

    return (rc);
}



/*------------------------------------------------------------------
 *
 * Function: set_speed
 *  Set init speed , autoneg , duplex full via ioctl on ethtool 
 *
 * Input:  device - device name (ex: "eth1" )
 *         sock - raw socket
 *         speed - select speed 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_speed(char *device, int sock, int speed) {
    struct ethtool_cmd edata;
    struct ifreq ifr;
    bzero(&ifr, sizeof(ifr));
    bzero(&edata, sizeof(edata));

    strncpy(ifr.ifr_name, device, IFNAMSIZ);
    ifr.ifr_data = (char *)&edata;
    edata.cmd = ETHTOOL_GSET;
    if (ioctl(sock,SIOCETHTOOL,&ifr)==-1) {
        perror("ioctl: get info via ethtool");
        close(sock);
        return FAILED;
    }

    edata.cmd = ETHTOOL_SSET;
    edata.speed = speed;
    edata.autoneg = AUTONEG_DISABLE;

    /* according to the ethtool, the AUTONEG_ENABLE 
     * will cause the speed set into 10 Mbps
     */
    /*if (speed == 1000)   
        edata.autoneg = AUTONEG_ENABLE;
    */    
    edata.duplex = DUPLEX_FULL;
    if (ioctl(sock,SIOCETHTOOL,&ifr)==-1) {
        perror("ioctl: set speed via ethtool");
        close(sock);
        return FAILED;
    }
    
    return PASSED;
    
}

/*------------------------------------------------------------------
 *
 * Function: set_port_speed
 *  init port status via ethtool.
 *
 * Input:  ifname - port type
 *         portnum - port number
 *         enable - enable/disable the Enable stub register 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_port_speed(char *port_type, int speed)
{
    int raw;
    int get_speed;
    char pname[10];

#if DEBUG
    printf("port_type %s,  speed %d , %s\n", port_type, speed, __FUNCTION__);
#endif

     sprintf(pname,"%s", port_type);
   
     switch(speed) {
         case SPD_10MBPS:
             get_speed = 10;
             break;
         case SPD_100MBPS:
             get_speed = 100;
             break;
         case SPD_1000MBPS:
             get_speed = 1000;
             break;
         case SPD_2500MBPS:
             get_speed = 2500;
             break;   
         case SPD_5000MBPS:
             get_speed = 5000;
             break;   
         case SPD_10000MBPS:
             get_speed = 10000;
             break;   
        default:
            printf("%s(): %s not support this speed %d\n",
                    __FUNCTION__, pname, speed);
           return (FAILED);
           break;
    }
    
    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);

    /* Bind raw socket to interface */
    if (bind_socket(pname, raw, ETH_P_ALL)) {
        printf("%s(): %s Bind socket failed.\n", __FUNCTION__, pname);
        close(raw);
        return (FAILED);
    }

    if (set_speed(pname, raw, get_speed)) {
        printf("%s(): %s Set Speed  %d failed.\n",
                __FUNCTION__, pname, get_speed);
        close(raw);
        return (FAILED);
    }

    if (set_promisc(pname, raw)) {
        printf("%s(): %s Set promisc failed.\n", __FUNCTION__, pname);
        close(raw);
        return (FAILED);
    }

    close(raw);
    return (PASSED);
}


int katar_speed_set_packet(char *type, int port, int port2, int speed)
{
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0, front_panel_port;
    uchar orig_hkpflag = hkeepflags;

    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;

    front_panel_port = eth_mapping_ge_num[port];

    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = pktdata[typ_curr].send_count;
        pkt_len = pktdata[typ_curr].len;
        pkt_val = pktdata[typ_curr].val;
        hkeepflags |= pktdata[typ_curr].hkpflags;
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)\n",
                    front_panel_port, speed, pkt_cnt, pkt_len, pkt_val);
        }
        fflush(stdout);

        /* prepare to send packet */
        rc = tx_rx_speed_diag(type, port, port2, speed, pkt_cnt, pkt_len, pkt_val);
        if (rc == FAILED) {
#if PACKET_DROP_SKIP
if ( packet_drop_skip_enable == 0 )
#endif
            cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            show_status_info(port + ADDR_MEDIA_PHY);
            return (FAILED);
        }
    } /* typ_curr */

    printf("Passed\n");
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    return (rc);
}
int ge_phy_speed_test(int port1, int port2, int speed)
{
    int rc = PASSED;

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = katar_speed_set_packet(SEL_PORT_ETH, port1, port2, speed);
    return (rc);
}

/**********************************************************************
 * Function: show_eth_counter
 *
 * Description:
 * Display command "ifconfig" packet related counter
 *
 * Input: port - port number
 *
 * Return: none
 **********************************************************************
 */
void show_eth_counter (char *type, int port)
{
    char tx_pkt[64];
    char rx_pkt[64];
    char tx_err[64];
    char rx_err[64];
    char tx_drop[64];
    char rx_drop[64];

    sprintf(tx_pkt, "cat /sys/class/net/%s%d/statistics/tx_packets\n", type, port);
    sprintf(rx_pkt, "cat /sys/class/net/%s%d/statistics/rx_packets\n", type, port);
    sprintf(tx_err, "cat /sys/class/net/%s%d/statistics/tx_errors\n", type, port);
    sprintf(rx_err, "cat /sys/class/net/%s%d/statistics/rx_errors\n", type, port);
    sprintf(tx_drop, "cat /sys/class/net/%s%d/statistics/tx_dropped\n", type, port);
    sprintf(rx_drop, "cat /sys/class/net/%s%d/statistics/rx_dropped\n", type, port);

    printf("%s%d", type, port);

    printf("\tTX packets:");
    fflush(stdout);
    system(tx_pkt);

    printf("\tRX packets:");
    fflush(stdout);
    system(rx_pkt);

    printf("\tTX errors:");
    fflush(stdout);
    system(tx_err);

    printf("\tRX errors:");
    fflush(stdout);
    system(rx_err);

    printf("\tTX dropped:");
    fflush(stdout);
    system(tx_drop);

    printf("\tRX dropped:");
    fflush(stdout);
    system(rx_drop);
    printf("\n");
}

int katar_2p5_phy_cross_port_tx_rx_setting(int port1_PHY_ID, int port2_PHY_ID, int speed_cur)
{
    AQ_API_Port*  aq_port=NULL;
    AQ_API_AutonegotiationControl autoneg_control;
    //AQ_API_100M_1G_ConnectionStatus connectionStatus_1G;
    AQ_API_ConnectionStatus connect_status;
    int i=0;
        
    //speed_cur = ge2p5_eth_speed_list[i];
    aq_port = malloc(sizeof(AQ_API_Port));
    memset(aq_port, 0x0, sizeof(aq_port));
    aq_port->device = AQ_DEVICE_CAL;
    AQ_API_GetAutonegotiationControl(aq_port, &autoneg_control);

    if ( speed_cur == SPD_1000MBPS )
    {
        //autoneg_control = malloc(sizeof(AQ_API_AutonegotiationControl));
        //memset(autoneg_control, 0x0, sizeof(autoneg_control));
        autoneg_control.advertise5G = 0;
        autoneg_control.advertise10GBASE_T = 0;
        autoneg_control.advertise2_5G = 0;
        autoneg_control.advertise1000BASE_T_FullDuplex = 1;
        //autoneg_control.portType = AQ_API_AN_MultiPort;
        aq_port->PHY_ID = port1_PHY_ID;
        AQ_API_SetAutonegotiationControl(aq_port, &autoneg_control);
        aq_port->PHY_ID = port2_PHY_ID;
        AQ_API_SetAutonegotiationControl(aq_port, &autoneg_control);
        //printf("\n###################set speed and auto-neg\n");
        aq_port->PHY_ID = port1_PHY_ID;
        AQ_API_Set_Speed(aq_port, 1);
        aq_port->PHY_ID = port2_PHY_ID;
        AQ_API_Set_Speed(aq_port, 1);
        //free(aq_port);
        //free(autoneg_control);
        //AQ_API_Get100M_1G_ConnectionStatus(aq_port, &connectionStatus_1G);
        sleep(8);
        for ( i = 0; i < 2 ; i++)
        {
            AQ_API_GetConnectionStatus(aq_port, &connect_status);
            if (  connect_status.state != 17 )
            {
                if (i==1)
                {
                    printf("\nWarning: Is cross-port loopback connector installed? AQ_API_GetConnectionStatus PHY_ID = %d, state = %d\n",aq_port->PHY_ID, connect_status.state);
                    free(aq_port);
                    return (FAILED);
                }
                else
                    sleep(6); 
            }
            else
                break;
        }
    }
    else if ( speed_cur == SPD_2500MBPS )
    {
        autoneg_control.advertise5G = 0;
        autoneg_control.advertise10GBASE_T = 0;
        autoneg_control.advertise2_5G = 1;
        autoneg_control.advertise1000BASE_T_FullDuplex = 0;
        //autoneg_control.portType = AQ_API_AN_MultiPort;
        aq_port->PHY_ID = port1_PHY_ID;
        AQ_API_SetAutonegotiationControl(aq_port, &autoneg_control);
        aq_port->PHY_ID = port2_PHY_ID;
        AQ_API_SetAutonegotiationControl(aq_port, &autoneg_control);
        //printf("\n###################set speed and auto-neg\n");
        aq_port->PHY_ID = port1_PHY_ID;
        AQ_API_Set_Speed(aq_port, 0);
        aq_port->PHY_ID = port2_PHY_ID;
        AQ_API_Set_Speed(aq_port, 0);
        sleep(8);
        for ( i = 0; i < 2 ; i++)
        {
            AQ_API_GetConnectionStatus(aq_port, &connect_status);
            if (  connect_status.state != 16 )
            {
                if (i==1)
                {
                    printf("\nWarning: Is cross-port loopback connector installed? AQ_API_GetConnectionStatus PHY_ID = %d, state = %d\n",aq_port->PHY_ID, connect_status.state);
                    free(aq_port);
                    return (FAILED);
                }
                else
                    sleep(6);
            }
            else
                break;
        }
    }
    else
    {
        printf("katar_2p5_phy_cross_port_tx_rx_setting speed %d is wrong\n", speed_cur);
        free(aq_port);
        return FAILED;
    }
    free(aq_port);
    return PASSED;
} 

extern void AQ_reg_write(unsigned int port, unsigned int reg_mmd, unsigned int reg_num, unsigned int reg_val);
int set_eth_25G_serdes_reset_PHY_ID(int phy_id)
{
    unsigned int PHY_ID=0;
    
    PHY_ID = phy_id;
    AQ_reg_write(PHY_ID, 4, 0xC440, 0x1);
    sleep(6);
    return(PASSED);
}

void katar_autoneg_config_restore(void)
{
    int i=0;
    AQ_API_Port*  aq_port=NULL;
    AQ_API_AutonegotiationControl autoneg_control;
    
    aq_port = malloc(sizeof(AQ_API_Port));
    memset(aq_port, 0x0, sizeof(aq_port));
    aq_port->device = AQ_DEVICE_CAL;
    AQ_API_GetAutonegotiationControl(aq_port, &autoneg_control);
    
    for ( i=0 ; i < 4 ;i++)
    {
        autoneg_control.advertise2_5G = 1;
        autoneg_control.advertise1000BASE_T_FullDuplex = 1;
        aq_port->PHY_ID = i;
        AQ_API_SetAutonegotiationControl(aq_port, &autoneg_control);
    }
    free(aq_port);
    return;
}

void katar_2p5_phy_check_sys_status(int phy0, int phy1)
{
    int sys_status_p0, sys_status_p1;
    int m;
    for (m = 0 ; m < 10 ; m++)
    {
        sys_status_p0 = AQ_reg_read(phy0, 4, 0xe812);
        sys_status_p1 = AQ_reg_read(phy1, 4, 0xe812);
        if (((sys_status_p0 == 0xb411)&&(sys_status_p1 == 0xb411)) || ((sys_status_p0 == 0xb211)&&(sys_status_p1 == 0xb211)))
        {
            sleep(1);
            break;
        }
        if ( m == 9)
            printf(" Error! Check%d AQR KR link status phy%d = %x, phy%d = %x\n", m, phy0, sys_status_p0, phy1, sys_status_p1);
        sleep(1);     
    }
}


int katar_2p5_phy_cross_test(void)
{
    int rc = 0;
    //int try, retry_limit = 2;
    int port0, port1, port2, port3;
    char systemcmd0[32], systemcmd1[32];
    int i,j;
    int speed_cnt, speed_cur;
    int KR_link_down=0;
#if PACKET_DROP_SKIP
    int k;
#endif
    /* get test envrionment variable */

	testname("2.5G PHY cross port test");

    printf("Start 2.5G PHY cross port test");

    system(SUPPRESS_MESG);
    port0 = ETH_2P5GE_PORT2;
    port1 = ETH_2P5GE_PORT3;
    port2 = ETH_2P5GE_PORT0;
    port3 = ETH_2P5GE_PORT1;

    //sprintf(port_name, "eth%d", port);

    sprintf(systemcmd0,"ifconfig %s%d up", SEL_PORT_ETH, port0);
    sprintf(systemcmd1,"ifconfig %s%d up", SEL_PORT_ETH, port1);
    system(systemcmd0);
    system(systemcmd1);
    sprintf(systemcmd0,"ifconfig %s%d up", SEL_PORT_ETH, port2);
    sprintf(systemcmd1,"ifconfig %s%d up", SEL_PORT_ETH, port3);
    system(systemcmd0);
    system(systemcmd1);
    sleep(5);

    speed_cnt = sizeof(ge2p5_eth_speed_list) / sizeof(int);
#if PACKET_DROP_SKIP
    packet_drop_skip_enable = 1;
#endif
    for ( i = speed_cnt-1; i >= 0 ; i--)
    {
        speed_cur = ge2p5_eth_speed_list[i];
        for ( j = 0 ; j < 3 ; j++ ) /* check system site link status. If link down, retry */
        {
            if( FAILED == (rc = katar_2p5_phy_cross_port_tx_rx_setting(0,1,speed_cur)))
            {
                printf("\n Error: PHY0 -> PHY1 cross port PHY setting failed in speed = %d\n",speed_cur);
            }
            sleep(5);

            KR_link_down = 0; 
            if(wait_iface_link_stats(port0,IFSTATUS_UP)!=PASSED)
            {
                KR_link_down = 1;
            }
            if(wait_iface_link_stats(port1,IFSTATUS_UP)!=PASSED)
            {
                KR_link_down = 2;
            }
            if (KR_link_down == 0)
                break;

            if ((KR_link_down != 0) && (j == 2))
            {
                if ( KR_link_down == 1 )
                    printf("check port0 KR connection link failed.\n");
                else
                    printf("check port1 KR connection link failed.\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag;
            }
        }
        katar_2p5_phy_check_sys_status(0, 1);
        printf("\nSpeed %d test port0 -> port1\n", speed_cur);
        rc = ge_phy_speed_test(port0, port1, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port1 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port1 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(1);
                if(wait_iface_link_stats(port1,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port1 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port0, port1, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 1 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag;
            }
        }
#endif
        sleep(5);
        printf("\nSpeed %d test port1 -> port0\n", speed_cur);
        rc = ge_phy_speed_test(port1, port0, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port0 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port0 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(0);
                if(wait_iface_link_stats(port0,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port0 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port1, port0, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 0 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag;
            }
        }
#endif
        for ( j = 0 ; j < 3 ; j++ ) /* check system site link status. If link down, retry */
        {
            if( FAILED == (rc = katar_2p5_phy_cross_port_tx_rx_setting(2,3,speed_cur)))
            {
                printf("\n Error: PHY2 -> PHY3 cross port PHY setting failed in speed = %d\n",speed_cur);
            }
            sleep(5);

            KR_link_down = 0; 
            if(wait_iface_link_stats(port2,IFSTATUS_UP)!=PASSED)
            {
                KR_link_down = 1;
            }
            if(wait_iface_link_stats(port3,IFSTATUS_UP)!=PASSED)
            {
                KR_link_down = 2;
            }
            if (KR_link_down == 0)
                break;

            if ((KR_link_down != 0) && (j == 2))
            {
                if ( KR_link_down == 1 )
                    printf("check port2 KR connection link failed.\n");
                else
                    printf("check port3 KR connection link failed.\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag;
            }
        }
        katar_2p5_phy_check_sys_status(2, 3);
        printf("\nSpeed %d test port2 -> port3\n", speed_cur);
        rc = ge_phy_speed_test(port2, port3, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port3 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port3 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(3);
                if(wait_iface_link_stats(port3,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port3 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port2, port3, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 3 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag;
            }
        }
#endif
        sleep(5);
        printf("\nSpeed %d test port3 -> port2\n", speed_cur);
        rc = ge_phy_speed_test(port3, port2, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port2 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port2 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(2);
                if(wait_iface_link_stats(port2,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port3 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port3, port2, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 2 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag;
            }
        }
#endif
    }

exit_2p5_cross_port_diag:
    katar_err_clean_up(port0);
    katar_err_clean_up(port1);
    katar_err_clean_up(port2);
    katar_err_clean_up(port3);
#if PACKET_DROP_SKIP
    packet_drop_skip_enable = 0;
#endif
    if (rc == FAILED) {
        cterr('f', 0, "2.5G PHY cross port test failed");
        katar_autoneg_config_restore();
        system(OPEN_MESG);
        return (FAILED);
    }

#if DEBUG
    show_eth_counter(SEL_PORT_ETH, port0);
    show_eth_counter(SEL_PORT_ETH, port2);
    printf("*******End*******\n");
    system("date"); /* real time counter */
    printf("*****************\n");
#endif
    system(OPEN_MESG);
    katar_autoneg_config_restore();
        if(rc ==PASSED)
                prpass(testpass, NULL);
    return (PASSED);
}


int katar_2p5_phy_cross_test_rdt(void);
int katar_2p5_phy_cross_test_rdt(void)
{
    int rc = 0;
    int port0, port1, port2, port3;
    char systemcmd0[32], systemcmd1[32];
    int i,j;
    int speed_cnt, speed_cur;
    int KR_link_down=0;
    /* get test envrionment variable */
#if PACKET_DROP_SKIP
    int k;
	testname("2.5G PHY cross port test with packet drop workaround for RDT.");
    printf("Start 2.5G PHY cross port test with packet drop workaround for RDT.\n");
#else
	testname("2.5G PHY cross port test for RDT.");
    printf("Start 2.5G PHY cross port test for RDT.\n");
#endif

    system(SUPPRESS_MESG);
    port0 = ETH_2P5GE_PORT2;
    port1 = ETH_2P5GE_PORT3;
    port2 = ETH_2P5GE_PORT0;
    port3 = ETH_2P5GE_PORT1;

    sprintf(systemcmd0,"ifconfig %s%d up", SEL_PORT_ETH, port0);
    sprintf(systemcmd1,"ifconfig %s%d up", SEL_PORT_ETH, port1);
    system(systemcmd0);
    system(systemcmd1);
    sprintf(systemcmd0,"ifconfig %s%d up", SEL_PORT_ETH, port2);
    sprintf(systemcmd1,"ifconfig %s%d up", SEL_PORT_ETH, port3);
    system(systemcmd0);
    system(systemcmd1);
    sleep(5);

    speed_cnt = sizeof(ge2p5_eth_speed_list) / sizeof(int);
#if PACKET_DROP_SKIP
    packet_drop_skip_enable = 1;
#endif
    for ( i = speed_cnt-1; i >= 1 ; i--)  /* only for 2.5G */
    {
        speed_cur = ge2p5_eth_speed_list[i];

        sleep(5);
        if ((wait_iface_link_stats(port0,IFSTATUS_UP)!=PASSED) || (wait_iface_link_stats(port1,IFSTATUS_UP)!=PASSED))
        {
            for ( j = 0 ; j < 3 ; j++ ) /* check system site link status. If link down, retry */
            {
                if( FAILED == (rc = katar_2p5_phy_cross_port_tx_rx_setting(0,1,speed_cur)))
                {
                    printf("\n Error: PHY0 -> PHY1 cross port PHY setting failed in speed = %d\n",speed_cur);
                }
                sleep(5);
    
                KR_link_down = 0; 
                if(wait_iface_link_stats(port0,IFSTATUS_UP)!=PASSED)
                {
                    KR_link_down = 1;
                }
                if(wait_iface_link_stats(port1,IFSTATUS_UP)!=PASSED)
                {
                    KR_link_down = 2;
                }
                if (KR_link_down == 0)
                    break;
    
                if ((KR_link_down != 0) && (j == 2))
                {
                    if ( KR_link_down == 1 )
                        printf("check port0 KR connection link failed.\n");
                    else
                        printf("check port1 KR connection link failed.\n");
                    rc = FAILED;
                    goto exit_2p5_cross_port_diag_rdt;
                }
            }
        }
#if PACKET_DROP_SKIP
        packet_drop = 0;
#endif
        katar_2p5_phy_check_sys_status(0, 1);
        printf("\nSpeed %d test port0 -> port1\n", speed_cur);
        rc = ge_phy_speed_test(port0, port1, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port1 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port1 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(1);
                if(wait_iface_link_stats(port1,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port1 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port0, port1, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 1 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag_rdt;
            }
        }
#endif
        sleep(5);
        printf("\nSpeed %d test port1 -> port0\n", speed_cur);
#if PACKET_DROP_SKIP
        packet_drop = 0;
#endif
        rc = ge_phy_speed_test(port1, port0, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port0 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port0 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(0);
                if(wait_iface_link_stats(port0,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port0 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port1, port0, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 0 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag_rdt;
            }
        }
#endif

        if ((wait_iface_link_stats(port2,IFSTATUS_UP)!=PASSED) || (wait_iface_link_stats(port3,IFSTATUS_UP)!=PASSED))
        {
            for ( j = 0 ; j < 3 ; j++ ) /* check system site link status. If link down, retry */
            {
                if( FAILED == (rc = katar_2p5_phy_cross_port_tx_rx_setting(2,3,speed_cur)))
                {
                    printf("\n Error: PHY2 -> PHY3 cross port PHY setting failed in speed = %d\n",speed_cur);
                }
                sleep(5);
    
                KR_link_down = 0; 
                if(wait_iface_link_stats(port2,IFSTATUS_UP)!=PASSED)
                {
                    KR_link_down = 1;
                }
                if(wait_iface_link_stats(port3,IFSTATUS_UP)!=PASSED)
                {
                    KR_link_down = 2;
                }
                if (KR_link_down == 0)
                    break;
    
                if ((KR_link_down != 0) && (j == 2))
                {
                    if ( KR_link_down == 1 )
                        printf("check port2 KR connection link failed.\n");
                    else
                        printf("check port3 KR connection link failed.\n");
                    rc = FAILED;
                    goto exit_2p5_cross_port_diag_rdt;
                }
            }
        }

#if PACKET_DROP_SKIP
        packet_drop = 0;
#endif
        katar_2p5_phy_check_sys_status(2, 3);
        printf("\nSpeed %d test port2 -> port3\n", speed_cur);
        rc = ge_phy_speed_test(port2, port3, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port3 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port3 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(3);
                if(wait_iface_link_stats(port3,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port3 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port2, port3, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 3 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag_rdt;
            }
        }
#endif
        sleep(5);
#if PACKET_DROP_SKIP
        packet_drop = 0;
#endif
        printf("\nSpeed %d test port3 -> port2\n", speed_cur);
        rc = ge_phy_speed_test(port3, port2, speed_cur);
#if PACKET_DROP_SKIP
        if ( rc == FAILED )
        {
            for ( k = 0 ; k < PACKET_DROP_RETRY_COUNT ; k++ )
            {
                if ( packet_drop == 1)
                    printf("\n 1ST Packet drop issue happen !! Port2 retry %d\n", k+1);
                else
                    printf("\n CRC Packet drop issue happen !! Port2 retry %d\n", k+1);

                packet_drop = 0;
                set_eth_25G_serdes_reset_PHY_ID(2);
                if(wait_iface_link_stats(port2,IFSTATUS_UP)!=PASSED)
                {
                    printf("check port3 KR connection link failed.\n");
                    rc = FAILED;
                    continue;
                }
                else
                    rc = ge_phy_speed_test(port3, port2, speed_cur);
                if (rc != FAILED)
                    break;
            }
            if ( (k == PACKET_DROP_RETRY_COUNT) && (rc == FAILED) )
            {
                printf("2.5G PHY ID = 2 packet drop retry error !!\n");
                rc = FAILED;
                goto exit_2p5_cross_port_diag_rdt;
            }
        }
#endif
    }
exit_2p5_cross_port_diag_rdt:
    katar_err_clean_up(port0);
    katar_err_clean_up(port1);
    katar_err_clean_up(port2);
    katar_err_clean_up(port3);
#if PACKET_DROP_SKIP
    packet_drop_skip_enable = 0;
#endif
    if (rc == FAILED) {
        cterr('f', 0, "2.5G PHY RDT cross port test failed");
        katar_autoneg_config_restore();
        system(OPEN_MESG);
        return (FAILED);
    }

#if DEBUG
    show_eth_counter(SEL_PORT_ETH, port0);
    show_eth_counter(SEL_PORT_ETH, port2);
    printf("*******End*******\n");
    system("date"); /* real time counter */
    printf("*****************\n");
#endif
    system(OPEN_MESG);
    katar_autoneg_config_restore();
        if(rc ==PASSED)
                prpass(testpass, NULL);
    return (PASSED);
}

void write_statistic(void){

     system("ifconfig eth2 > statisitcTX.log"); 
}

void write_pktcnt(int pkt_cnt){
    
    FILE *fd_record; 
  
    fd_record = fopen("packet_count.log", "w");
    if (fd_record == NULL) {
        cterr('f',0,"open packet_count.log failed. \n");
    //    return FAILED;
    }
    
     fprintf(fd_record, "%d", pkt_cnt);  /* we expect two packets in here zzzz. */
    fclose(fd_record);  
    
}

/*
 *------------------------------------------------------------------
 * $Log: platform_ext_lpbk.c,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.11  2019/06/10 11:58:28  benlu
 * Modify platform_ext_lpbk.c base on PRRQ#4685780 Comment#6
 *
 * Revision 1.1.2.10  2019/06/10 02:27:03  mikech2
 * Add test name for 2.5G cross-port test
 *
 * Revision 1.1.2.9  2019/05/31 04:06:46  benlu
 * Add workaround for AQR412c CRC and 1st packet error
 *
 * Revision 1.1.2.8  2019/05/29 05:59:18  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.7  2019/05/27 12:18:29  benlu
 * Add delay to wait KR link-up for RDT test
 *
 * Revision 1.1.2.6  2019/05/24 13:45:52  benlu
 * Add 2.5G cross-port test for RDT in MB diag
 *
 * Revision 1.1.2.5  2019/05/24 08:50:51  benlu
 * Add AQR412c 1st packet workaround for RDT test
 *
 * Revision 1.1.2.4  2019/04/26 01:17:34  mikech2
 * clean up Makefile
 *
 * Revision 1.1.2.3  2019/04/12 01:35:55  peteteng
 * Code cleanup
 *
 * Revision 1.1.2.2  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.6  2019/01/08 05:50:10  mikech2
 * chnage cross-port lpbk test order and add debug message
 *
 * Revision 1.1.2.5  2018/12/27 03:48:26  mikech2
 * Fix cross-port test issue
 *
 * Revision 1.1.2.4  2018/11/28 01:35:42  benlu
 * AQR412c config restore after corss test, link down retry, modify message
 *
 * Revision 1.1.2.3  2018/10/26 09:21:03  benlu
 * Modify the port mapping for AQR lpbk test and console log
 *
 * Revision 1.1.2.2  2018/10/26 02:39:34  mikech2
 * Fix typo
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.6  2018/10/22 06:27:25  benlu
 * Remove debug message
 *
 * Revision 1.1.2.5  2018/10/16 08:50:35  benlu
 * Add AQR412c cross-port test
 *
 * Revision 1.1.2.4  2018/09/04 13:34:31  benlu
 * add aqc107 internal loopback test
 *
 * Revision 1.1.2.3  2018/08/28 14:04:14  benlu
 * Add AQC107 cross test
 *
 * Revision 1.1.2.2  2018/07/10 10:28:24  benlu
 * fix the multiple definitions error
 *
 * Revision 1.1.2.1  2018/07/10 09:45:32  benlu
 * phy internal/external loopback
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

