/* $Id: platform_ext_lpbk.c,v 1.3 2020/01/21 05:49:01 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_ext_lpbk.c  
 * support PHY external loopback 
 * internal loopback: GE PHY and Cavium.
 *
 * Feb 2019 Leschen
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

#include "router_if.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "platform_xfi.h"
#include "bcm82752_api.h"
#include "bcm82752_reg_def.h"
#include "platform_sfp_cookie.h"
#include "eth_pkt_utils.h"
#include "bcm57412_test.h"
#include "dash_fpga.h"
#include "bcm57412_lib.h"

void create_rx_buffer();
int wait_iface_link_stats(char *, int);
int setup_xfi_port(int, int *);
int neptune_cavium_is_linkup(char *, int);
int neptune_phy_soft_reset (char *, boolean);
int force_linkup(boolean, int);
int direct_phy_soft_reset(int);
int neptune_err_clean_up(int);
int show_status_info(int);
void show_eth_counter (char *, int);

int is_glc_ge_100fx = 0;
int eth_mapping_sfp_num[] = {SFP_PORT0, SFP_PORT1};
/* Curie BCM82752 PHY address */
int eth_mapping_addr[] = {0x10, 0x11};

/* Port Mapping of GE0 ~ GE3*/
int eth_mapping_ge_num[] = {0, 0, 0, GE_PORT0, GE_PORT1, GE_PORT2, GE_PORT3};

int eth_mapping_sgmii_num[] = {0, 0, 0, SGMII0, SGMII1, SGMII2, SGMII3};
int eth_mapping_xfi_num[] = {XFI0, XFI2};

/* DLM 5/6 eth number from GE port 0 ~ 3*/
int eth_bgx2_list[] = {ETH3, ETH4, ETH5, ETH6};

/* Curie ETH number */
int eth_bgx0_list[] = {ETH4, ETH5};

/* global */
int print_statistic = 0;
unsigned int neptune_packet_count = 0;

/* packet buffer */
unsigned char tx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet_sec[ETH_PKT_MAX_LEN];
unsigned char global_pkt_array[ETH_PKT_MAX_LEN*2+1]; //12.30
//unsigned char *global_pkt_ptr; //12.30

/* for ctrl diag flow */
volatile int tx_rx_box = 0;
sem_t rx_ready, rx_finish, tx_cmp;

/* Packets to be used in xaui port loopback tests
 * we leaave 12 byte for put mac address into the packet
 */
static pktdata_info_t pktdata[] = {
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 1000},
  {0xa7, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 1000},
  {0xa5, ((ETH_UDP_DATA_MAX_LEN - 1) - 12), H_INCFILL, 1000},
  {0xa3, (ETH_UDP_DATA_MAX_LEN - 12), H_INCFILL, 1000},
};

/* PTP packets to be used in ge port loopback tests
 * we leave 12 byte for put mac address into the packet
 */
static pktdata_info_t ptp_pktdata[] = {
  {0xa0, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
  {0xa7, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
  {0xa5, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
  {0xa3, PTP_MESSAGE_LENGTH, H_INCFILL, 1},
};

//mac_addr_t mac_da = {0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc};
//mac_addr_t mac_sa = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56};
//Using this source/destination address for MACSec loopback test
static mac_addr_t mac_da = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static mac_addr_t mac_sa = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};

extern struct curie_bcm82752 *curie;
extern int curie_bcm82752_mode_config(struct curie_bcm82752 *, int, int);
static int xfi_eth_1g_speed_list[] = {SPEED_1G};
static int xfi_eth_10g_speed_list[] = {SPEED_10G};

/* PTP speed table */
static int ptp_xfi_eth_speed_list[] = {SPEED_10G};

 /*------------------------------------------------------------------
 *
 * Function: curie_create_raw_socket
 *     Create the raw socket with specific protocol.
 *
 * Input:  protocol - seclect protocol
 *
 * Output: rawsock - return created socket num.
 *
 *------------------------------------------------------------------
 */
int curie_create_raw_socket(int protocol)
{ 
    int rawsock;
    if((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol)))== -1) {
        perror("Error creating raw socket\n");
        exit(-1);
    }
    return rawsock;
}

/*------------------------------------------------------------------
 *
 * Function: curie_bind_socket
 *     Bind raw socket to interface 
 *
 * Input:  device - current port
 *         rawsock - socket
 *         protocol - select protocol
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int curie_bind_socket(char *device, int rawsock, int protocol) {
    struct sockaddr_ll sll;
    struct ifreq ifr;
 
    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */
 
    strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
    if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        perror("Error getting Interface index !\n");
        return FAILED;
    }
 
    /* Bind our raw socket to this interface */
 
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol); 

#if DEBUG
    printf("%s interface is %d\n", __FUNCTION__, ifr.ifr_ifindex);
#endif
 
    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1) {
        perror("Error binding raw socket to interface\n");
        return FAILED;
    }

    
    return PASSED;
 
}



/*------------------------------------------------------------------
 *
 * Function: curie_set_promisc
 *    set promisc mode.
 *    when program exit, this interface will still be promisc mode.
 *    we should disable promisc mode when we exit (ie, use atexit)
 *
 * Input:  
 *
 * Output: PASSED/FAILED
 *
 * Note: if the curie_set_promisc is failed, the rx will get the haft of 
 *       packet from tx.
 *------------------------------------------------------------------
 */
int curie_set_promisc(char *device, int sock) {

    struct ifreq ifr;
 
    bzero(&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */
    /* Set the network card in promiscuos mode */
    strncpy(ifr.ifr_name, device, IFNAMSIZ);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS");
        close(sock);
        return FAILED;
    }
    
    ifr.ifr_flags|=IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to set promiscous mode");
        close(sock);
        return FAILED;
    }

    return PASSED;
 
}

/***********************************************************************
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
    raw = curie_create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (curie_set_promisc(eth_name, raw) == -1) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (curie_bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        return(FAIL);
    }

    *socket = raw;

    return PASS;
}

/***********************************************************************
 *
 * Function:    setup_xfi_port()
 *
 * Description: Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:   xaui_port - host system xaui port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:  PASS/FAIL
 *
 ************************************************************************
 */
int setup_xfi_port (int xaui_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf (eth_name, "eth%d", xaui_port);

    /* Create the raw socket */
    raw = curie_create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (curie_set_promisc(eth_name, raw) == -1) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (curie_bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
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
 * Function: neptune_cavium_is_linkup
 *   Check linux up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int neptune_cavium_is_linkup (char *type, int eth_num)
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
        return PASSED;
    } else {
        printf("%s() Rx packet mismatched\n", __func__);

        return (FAILED);
    }

}

int curie_chk_macaddr (uchar *macaddr1, uchar *macaddr2)
{
    return (pkt_cmp(macaddr1, macaddr2,6));
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
    int raw;

    uint mac_size, fil_len, crc;
    unsigned char volatile *cptr;

    raw = *socket;

    /* clean up the rx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    
    cptr = (unsigned char *)tx_packet;
    mac_size = sizeof(mac_addr_t);

    /* put in the destination/source mac address */
    memcpy((char *)cptr, (char *)mac_da, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy((char *)cptr, (char *)mac_sa, sizeof(mac_addr_t));
    cptr += mac_size; 
 
    /* fill the packet. the len is include the size of mac address 
     * we need to minus the size of mac address on len for filbyte
     */
    fil_len = (len - (2*mac_size));

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
#if 0
    int ix = 0;
    printf("len = %d  \n", len);
    for ( ix = 0; ix < len; ix++)
       printf("tx_packet[%d] = %#.2x  ", ix, tx_packet[ix]);
#endif 

    if(!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        cterr('f',0, "error on sending packet");
        return (FAILED);
    }    
    
    return (PASSED);

}

/*------------------------------------------------------------------
 *
 * Function: ptp_send_packets
 *   for 1588 tx send packet to rx. if number of packet is too much,
 *   then the delay is needed.
 *
 * Input:  len - packet length
 *         val - content of packet
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ptp_send_packets(int *socket, char *iface_type, int len, char val, int port, int speed)
{
    int raw, rc = 0, ix;
    uint fil_len;
    uint mac_size;
    unsigned char volatile *cptr;
    V2MsgHeader *ptp_header;
    uint16_t ether_type;
    char init_clock[8] = {0x26, 0x33, 0x45, 0x2e, 0x15, 0x28, 0x72, 0x25};

    /* Enable ptp engine */
    //en_bcm54194_ptp_per_port(port, speed);

    ptp_header = (V2MsgHeader *)malloc(sizeof(V2MsgHeader));
    ether_type = PTP_ETHERNET_TYPE;
    ptp_header->transportSpecificAndMessageType = PTP_MESSAGE_TYPE;
    ptp_header->reserved1AndVersionPTP = PTP_VERSION;
    ptp_header->messageLength = PTP_MESSAGE_LENGTH;

    ptp_header->domainNumber = 0x0;
    ptp_header->reserved2 = 0x0;
    ptp_header->flags[0] = 0x4;
    ptp_header->flags[1] = 0x0;
    ptp_header->correctionField = 0x0;
    ptp_header->reserved3 = 0x0;

    /* Init clock identity array with non-zero values, if not init this array the timestamp
        will not be updated */
    for (ix = 0; ix < 8; ix++) {
        ptp_header->sourcePortId.clockIdentity[ix] = init_clock[ix];
    }

    ptp_header->sourcePortId.portNumber = 0x0;
    ptp_header->sequenceId = 0x0;
    ptp_header->control = 0x0;
    ptp_header->logMeanMessageInterval = 0x0;

    raw = *socket;

    /* clean up the rx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);

    cptr = (unsigned char *)tx_packet;
    mac_size = sizeof(mac_addr_t);

    /* put in the destination/source mac address */
    memcpy((char *)cptr, (char *)mac_da, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy((char *)cptr, (char *)mac_sa, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy((char *)cptr, (char *)&ether_type, sizeof(uint16_t));
    cptr += sizeof(uint16_t);
    memcpy((char *)cptr, (char *)ptp_header, sizeof(V2MsgHeader) - mac_size);
    cptr += sizeof(V2MsgHeader) - mac_size;

    /* fill the packet. the len is include the size of mac address
     * we need to minus the size of mac address on len for filbyte
     */
    fil_len = (len - (2*mac_size) - sizeof(uint16_t) - sizeof(V2MsgHeader) + mac_size);
    filbyte((uchar *)cptr, fil_len, val);

    rc = neptune_cavium_is_linkup(iface_type, port);
    if (rc == FAILED) {
        printf("port link up time out\n");
        return (FAILED);
    }

    if (!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
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
 * Function: neptune_err_clean_up
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
int neptune_err_clean_up (int port)
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
            memset((char *)rx_packet, 0, ETH_PKT_MAX_LEN);

            rx = read(get_info->socket, (unsigned char *)rx_pkt_buf, get_info->pkt_len);

            if (rx < 0) {
                printf("\n%s rx= %d socket %d read timeout. loop(ii)= %d, pkt_cnt = %d otherpkt_cnt= %d\n",
                       __FUNCTION__, rx, get_info->socket, ii, pkt_cnt, otherpkt_cnt);
                break; /* exit do loop */
            }

            /* drop invalid packet*/
            if (curie_chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) {
                otherpkt_cnt++;

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
            /* valid packet, increase the packet count */
            pkt_cnt++;

#if DEBUG
            printf("%d bytes received: !!! , pkt_cnt = %d \n", rx, pkt_cnt);
#endif
        } while (pkt_cnt < 2);  
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
        
        printf("\nstart of pkt print.\n");
        for (ii=0; ii < show_pkt_len; ii++) {
      if ((ii > 0) && (ii % 8) == 0) {
                 printf("\n");
          }
          printf("tx:%02x rx:%02x  ", tptr[ii], rptr[ii]);
      }
    printf("\nend of pkt print.\n");

}

/*------------------------------------------------------------------
 *
 * Function: tx_rx_diag
 *  Using Pthread to create another thread for rx.
 *  tx should wait for rx build. After tx send packet to rx
 *  tx also need to wait for rx get all the packet.
 *  the waiting mechanism is using semaphore. 
 *  the timeout value is set to 10.
 *
 * Input:  p_type - port type
 *         eth_port - port number
 *         speed - test speed
 *         signal - test signal fiber or copper
 *         pkt_cnt - test packet count
 *         value - contain of speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int tx_rx_diag(char* p_type, int eth_port, int speed, int pkt_cnt, int pkt_len, int value) 
{
    pthread_t threads;
    struct timespec ts;
    diag_info_pthread_t rx_info;
    char pname[10];
    int ii;
    int tx_skt, rx_skt;
    int rc, is_ptp_flag;
    void  *pthr_rv;
    unsigned int link_up = 0;
    int sfp_port;

    /* sfp_port is using to check BCM82752 PHY link status */
    if (eth_port == MB_57412_PORT2) {
        sfp_port = eth_mapping_sfp_num[1];
    } else {
        sfp_port = eth_mapping_sfp_num[0];
    }

    if (pkt_len & PTP_PKT_LEN_BIT_31_MASK) {
        is_ptp_flag = TRUE;
        pkt_len &= (~PTP_PKT_LEN_BIT_31_MASK);
    } else {
        is_ptp_flag = FALSE;
    }

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
    
    sprintf(pname,"%s%d", p_type, eth_port);

    /* setup ETH tx and rx socket */
    if (setup_eth_port(pname, &tx_skt) == FAIL) {
        return(FAILED);
    }

    if (setup_eth_port(pname, &rx_skt) == FAIL) {
        return(FAILED);
    }

    /* 
     * To prevent port link not up promptly in high temperature.
     * Due to Broadcom 57412 and 82752 may need longer time to 
     * finish initialization, checking PHY and Linux ETH link
     * status before sending packets.
     */
    /* Checking PHY link */
    rc = bcm82752_is_link_up(sfp_port, &link_up);
    if ((rc != PASSED) || (!link_up)) {
        cterr('f', 0, "PHY link up time out.");
        return (rc);
    } else {
        printf("After setup socket, PHY link is up.\n");
    }

    /* Checking ETH Linux link status */
    if (curie1ru_port_is_linkup(eth_port) == FAILED) {
        return (FAILED);
    }

    /* extend the space for putting the dest/src mac address */
    pkt_len += (2*sizeof(mac_addr_t));

    /* set up global value for both rx and tx on struct*/
    strncpy(rx_info.name, pname,IFNAMSIZ);
    rx_info.speed = speed;
    rx_info.pkt_num = pkt_cnt;
    rx_info.pkt_len = pkt_len;
    rx_info.socket = rx_skt;

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
            goto exit_tx_rx_diag;
        }
        ts.tv_sec += TX_RX_SYNC_TIME;

        /* wait for the setting of rx side */
        rc = sem_timedwait(&rx_ready, &ts);
        if (rc != PASSED) {
              show_buf_content(rx_info.pkt_len);
              if (errno == ETIMEDOUT) {
                  printf("sem_timedwait on rx_ready timeout.\n");
              } else {
                  printf("semaphore wait on rx ready failed.\n");
              }
            goto exit_tx_rx_diag;
        }

        msleep(1); /* ensure rx read is ready before tx */
        
        /* the main thread prepare to sending packet. */
        if (is_ptp_flag == TRUE) {
            rc = ptp_send_packets(&tx_skt, p_type, pkt_len, value, eth_port, speed);

            if (rc == FAILED) {
                printf("ptp_send_packets failed\n");
                goto exit_tx_rx_diag;
            }
        } else {
            rc = send_packets(&tx_skt, p_type, pkt_len, value, eth_port, speed);

            if (rc == FAILED) {
                printf("send_packets failed\n");
                goto exit_tx_rx_diag;
            }
        }

        /* Add some more time to wait for sem rx_finish to be unlocked
         */
        ts.tv_sec += TX_RX_SYNC_TIME;
        /* use semaphore to detect timeout on rx side */
        rc = sem_timedwait(&rx_finish, &ts);
        if (rc != PASSED) {
            show_buf_content(rx_info.pkt_len);
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on rx_finish timeout. (Packet-%d)\n", ii);
            } else {
                printf("semaphore wait on rx finish failed. (Packet-%d)\n", ii);
            }
            goto exit_tx_rx_diag;
        }
        
        /* compare the packet on rx_packet and tx_packet */
        if (is_ptp_flag == TRUE) {
            rc = ptp_check_pkt(pkt_len);
            if (rc == PASSED) {
                /* if match, clean up rx buffer for next packet. */
                memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
            } else {
                printf("%s: mismatch\n", __FUNCTION__);
                show_buf_content(rx_info.pkt_len);
                goto exit_tx_rx_diag;
            }
        } else {
            rc = check_pkt(pkt_len);
            if (rc == PASSED) {
                /* if match, clean up rx buffer for next packet. */
                memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
            } else {
                printf("%s: mismatch\n", __FUNCTION__);
                show_buf_content(rx_info.pkt_len);
                goto exit_tx_rx_diag;
            }
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

exit_tx_rx_diag:

    /* if failed, cancel the thread */
    if(rc != PASSED)
        pthread_cancel(threads);
        
    /* Sync the tx and rx in here and check the rx is pass or fail */
    pthread_join(threads, (void **)&pthr_rv);
    if (pthr_rv != PASSED) {
        printf("tx_rx_diag receive failed\n");
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
 * Function: neptune_phy_soft_reset
 *  Enable PHY reset, add swtich page flow.
 *
 * Input:  ifname - port type
 *         portnum - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int neptune_phy_soft_reset (char *ifname, boolean signal)
{
	return 0;
}


/*------------------------------------------------------------------
 *
 * Function: set_phy_stub
 *  Enable stub for external loopback test.
 *
 * Input:  ifname - port type
 *         enable - enable/disable the Enable stub register 
 *         signal - Copper or Fiber
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_phy_stub(char *ifname, boolean enable, boolean signal)
{
    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: phy_lpbk_type
 *  Select loopback type via setup loopback register.
 *
 * Input:  ifname - port type
 *         portnum - port number
 *         enable - enable/disable the Enable loopback register 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int phy_lpbk_type(char *ifname, boolean enable, boolean signal) 
{
    return 0;
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
        default:
            printf("%s(): %s not support this speed %d\n",
                    __FUNCTION__, pname, speed);
           return (FAILED);
           break;
    }
    
    /* Create the raw socket */
    raw = curie_create_raw_socket(ETH_P_ALL);

    /* Bind raw socket to interface */
    if (curie_bind_socket(pname, raw, ETH_P_ALL)) {
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

    if (curie_set_promisc(pname, raw)) {
        printf("%s(): %s Set promisc failed.\n", __FUNCTION__, pname);
        close(raw);
        return (FAILED);
    }

    close(raw);
    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: sig_pwr_ctrl
 *  Enable the power of PHY.
 *
 * Input:  ifname - port type
 *         enable - enable/disable the power up/down register 
 *         signal - select page for copper/fiber
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sig_pwr_ctrl(char *ifname, boolean enable, boolean signal)
{
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: init_sgmii_env
 *  init sgmii port. link up port, ensure power up and turn off other 
 *  power and set speed.
 *
 * Input:  pname - port 
 *         speed: current test speed   
 *         port - port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int init_sgmii_env(char *pname, int speed, int eth_num, int lpbk_mode)
{    
    int rc = 0;
    int autoneg = 0;
#if 0
    /* Ensure turn off the fiber and turn on copper before test */	
    if ((rc = sig_pwr_ctrl(pname, DISABLE_SIG, SIG_FIBER)) != PASSED){  
    	printf("sig_pwr_ctrl disable fiber failed \n"); 
    	return(FAILED);
    }
    if ((rc = sig_pwr_ctrl(pname, ENABLE_SIG, SIG_COPPER)) != PASSED){  
    	printf("sig_pwr_ctrl enable copper failed \n"); 
    	return(FAILED);
    }
#endif
    /* Always init as 10Mbps, duplex full, autoneg off
     * to make it more stable for 1000MPB initialation.
     */
    if ((rc = set_port_speed(pname, SPD_10MBPS)) != PASSED){
        printf("%s(): set_port_speed %d failed\n", __FUNCTION__, speed);
        return (FAILED);
    }

    /* internal loopback using force_linkup to ensure link stable,
     * external loopback can not use force_linkup, so using check link
     * to ensure the link is stable
     * neptune_cavium_is_linkup will return failed,
     * need to verify this one is necessary or not.
     */
    if (lpbk_mode == EXT_LPBK) {
        if ((rc = neptune_cavium_is_linkup(SEL_PORT_ETH, eth_num)) != PASSED) {
            printf("%s: after init port, sgmii link up time out after 1 second\n",
                   __FUNCTION__);
        }
    }   

    /* To ensure the test stay on full duplex and set speed
     */
    if (speed == SPD_1000MBPS ) {
        autoneg = AUTONEG_ON;
    } else {
        autoneg = AUTONEG_OFF;
    }

    if ((rc = cfg_phy_setting(pname, speed, FULL_DUPLEX, autoneg, SIG_COPPER)) != PASSED) {
        printf("%s(): %s cfg_phy_setting failed speed is %d\n",
               __FUNCTION__, pname, speed);
        return (FAILED);
    }

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: neptune_set_packet
 *  Set up packet info for tx and rx using.
 *
 * Input:  port: current test port
 *         speed: current test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int neptune_set_packet(char *type, int port, int speed)
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
            printf("\nTest port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)\n",
                    front_panel_port, speed, pkt_cnt, pkt_len, pkt_val);
        }
        fflush(stdout);

        /* prepare to send packet */
        rc = tx_rx_diag(type, port, speed, pkt_cnt, pkt_len, pkt_val);
        if (rc == FAILED) {
            cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            show_status_info(port + ADDR_MEDIA_PHY);
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nshow packet counter\n");
            msleep(1000);
            show_eth_counter(SEL_PORT_XFI, port);
        }
    } /* typ_curr */

    printf("Passed\n");
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: neptune_ptp_set_packet
 *  Set up PTP packet info for tx and rx using.
 *
 *
 * Input:  port: current test port
 *            speed: current test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int neptune_ptp_set_packet (char *type, int port, int speed)
{
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0, front_panel_port;
    uchar orig_hkpflag = hkeepflags;

    pkt_type = sizeof(ptp_pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;

    front_panel_port = eth_mapping_ge_num[port];

    for (typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = ptp_pktdata[typ_curr].send_count;
        pkt_len = ptp_pktdata[typ_curr].len;
        pkt_val = ptp_pktdata[typ_curr].val;
        hkeepflags |= ptp_pktdata[typ_curr].hkpflags;

        prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                                    front_panel_port, speed, pkt_cnt, pkt_len, pkt_val);
        fflush(stdout);

        /* Write value 1 to pkt_len bit 31 for distinguish ptp lpbk packet or normal
          lpbk packet */
        pkt_len |= PTP_PKT_LEN_BIT_31_MASK;

        /* To do the tx/rx loopback test */
        rc = tx_rx_diag(type, port, speed, pkt_cnt, pkt_len, pkt_val);
        if (rc == FAILED) {
            cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                  __FUNCTION__, front_panel_port, speed);
            hkeepflags = orig_hkpflag;
            show_status_info(port + ADDR_MEDIA_PHY);
            return (FAILED);
        }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", front_panel_port, speed);
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: set_macsec
 *  from Marvell Eng: setup internal loopback need to disable macsec 
 *  
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_macsec(boolean onoff, int port)
{
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_mac_speed
 * set mac speed to 100. both PHY 1548 and 1340 are use 
 * Reg 21 on Page 2. need follow by soft reset. 
 *  
 * Input:  phy_id - phy addr for setup port
 *         speed - setup speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_mac_speed(int phy_id, int speed)
{
    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: set_media_phy_int_lpbk
 *  initial and setup loopback type on sgmii for internal lpbk 
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_media_phy_int_lpbk(char *type, int port, int speed)
{
    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: wait_iface_link_stats
 *           Wait and check until link status is down/up
 *
 * Input:  pname - Interface name
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int wait_iface_link_stats (char *iface, int exp_val)
{
    int timeout = ETH_DRIVER_POLL_TIMEOUT;
    struct ifreq ifr;
    int sock;
    struct ethtool_value ecmd;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "%s: Failed to create ioctl socket\n", __FUNCTION__);
        return (FAILED);
    }

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);

    do {
        ecmd.cmd = ETHTOOL_GLINK;
        ecmd.data = !exp_val;
        ifr.ifr_data = (caddr_t)&ecmd;

        if (ioctl(sock, SIOCETHTOOL, &ifr) == -1) {
            close(sock);
            cterr('f', 0, "%s: Run Eth Tool fails", __FUNCTION__);
            return (FAILED);
        }

        if (ecmd.data == exp_val) {
            close(sock);
            return (PASSED);
        }
        msleep(1);
    } while (timeout--);

    close(sock);

    return (FAILED);
}

/*------------------------------------------------------------------
 *
 * Function: set_ten_g_phy_int_lpbk
 *  we setup 10G PHY first which will let cavium know the current
 *  setting of diag. Then to initial and setup loopback type on
 *  bridge PHY.
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_ten_g_phy_int_lpbk(char *type, int port, int speed, int lpbk_mode)
{
    char pname[10];
    int rc = PASSED;
    char cmd_str[32];
    unsigned int link_up = 0;
    int sfp_port;
    int port_addr;

    sprintf(pname,"%s%d", type, port);

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    msleep(500);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);
    msleep(500);

    if (port == MB_57412_PORT2) {
        sprintf(cmd_str, "ifconfig eth%d up", port - 1);
        system(cmd_str);
        msleep(500);
        sfp_port = eth_mapping_sfp_num[1];
        port_addr = eth_mapping_addr[1];
    } else {
        sfp_port = eth_mapping_sfp_num[0];
        port_addr = eth_mapping_addr[0];
    }

    if (is_bcm82752()) {
        bcm82752_xfi_sfi_access(port_addr, BCM82752_SFI_INTF);
        rc = bcm82752_config_loopback(port_addr, lpbk_mode);
        if (rc != PASSED) {
            printf("GE PHY config loopback failed.\n");
            return (rc);
        }

        if (is_thallium()) {
            bcm82752_set_port_speed(port_addr, speed);
        } else {
            bcm82752_set_port_speed(port_addr, speed);
        }
        sleep(ETH_DRIVER_DELAY);

        /* Check PHY SFI interface link up status */
        if ((lpbk_mode != BCM82752_LOOPBACK_NONE) &&
            !check_ge_int_lpbk_flag()) {
            rc = bcm82752_is_link_up(sfp_port, &link_up);
            if ((rc != PASSED) || (!link_up)) {
                printf("%s(): PHY link up time out\n", __FUNCTION__);
                return (rc);
            } else {
                printf("%s(): PHY link is up\n", __FUNCTION__);
            }
        }
    } 

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: set_ten_g_ext_int_lpbk
 *  we setup 10G PHY first which will let cavium know the current
 *  setting of diag. Then to initial and setup loopback type on
 *  bridge PHY.
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_ten_g_phy_ext_lpbk(char *type, int port, int speed, int lpbk_mode)
{
    char pname[10];
    int rc = PASSED;
    char cmd_str[32];
    unsigned int link_up = 0;
    int sfp_port;
    int port_addr;

    if (port == MB_57412_PORT2) {
        sfp_port = eth_mapping_sfp_num[1];
        port_addr = eth_mapping_addr[1];
    } else {
        sfp_port = eth_mapping_sfp_num[0];
        port_addr = eth_mapping_addr[0];
    }

    sprintf(pname,"%s%d", type, port);

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    msleep(500);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);
    msleep(500);

    bcm82752_xfi_sfi_access(port_addr, BCM82752_SFI_INTF);
    rc = bcm82752_config_loopback(port_addr, lpbk_mode);
    if (rc != PASSED) {
        printf("GE PHY config loopback failed.\n");
        return (rc);
    }

    if (is_thallium()) {
        bcm82752_set_port_speed(port_addr, speed);
    } else {
        bcm82752_set_port_speed(port_addr, speed);
    }
    sleep(ETH_DRIVER_DELAY);

    /* Check PHY SFI interface link up status */
    if (lpbk_mode != BCM82752_LOOPBACK_NONE) {
        rc = bcm82752_is_link_up(sfp_port, &link_up);
        if ((rc != PASSED) || (!link_up)) {
            printf("%s(): PHY link up time out\n", __FUNCTION__);
            return (rc);
        } else {
            printf("%s(): PHY link is up\n", __FUNCTION__);
        }
    }
    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: ten_g_phy_int_lpbk_test
 *  This is the entry point for 10GE PHY internal loopback test.
 *
 * Input:  port - port number
 *         speed - test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ten_g_phy_int_lpbk_test(int eth_num, int speed)
{
    int rc = PASSED;
    char cmd_str[32];

    /* setup loopback information */
    rc = set_ten_g_phy_int_lpbk(SEL_PORT_XFI, eth_num, speed, BCM82752_LOOPBACK_PCS);
    if (rc != PASSED) {
        cterr('f', 0, "set_ten_g_phy_int_lpbk failed, port: %d", eth_num);
        goto ten_g_phy_int_lpbk_exit;
    }

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = neptune_set_packet(SEL_PORT_XFI, eth_num, speed);
    if (rc != PASSED) {
        cterr('f', 0, "neptune_set_packet failed, port: %d", eth_num);
        goto ten_g_phy_int_lpbk_exit;
    }

    printf("\nshow packet counter\n");
    msleep(1000);
    show_eth_counter(SEL_PORT_XFI, eth_num);

ten_g_phy_int_lpbk_exit:
    /* restore the setting */
    if (is_bcm82752()) {
        if ((set_ten_g_phy_int_lpbk(SEL_PORT_XFI, eth_num, speed, BCM82752_LOOPBACK_NONE)) != PASSED) {
           cterr('f', 0, "disable GE PHY loopback failed");
           return (rc);
        }

    } 
    
    sprintf(cmd_str, "ifconfig %s%d down", SEL_PORT_XFI, eth_num);
    system(cmd_str);
    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: ten_g_phy_int_lpbk_test
 *  This is the entry point for 10GE PHY internal loopback test.
 *
 * Input:  port - port number
 *         speed - test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ten_g_phy_ext_lpbk_test(int eth_num, int speed, int lpbkmode)
{
    int rc = PASSED;
    char cmd_str[32];
    int sfp_port;
    int port_addr;
    unsigned int link_up = 0;

    /* CUrie - Up eth4 in order to checking bcm82752 ID */
    if (eth_num == MB_57412_PORT2) {
        sprintf(cmd_str, "ifconfig %s%d up", SEL_PORT_XFI, eth_num - 1);
        system(cmd_str);
        msleep(500);
        sfp_port = eth_mapping_sfp_num[1];
        port_addr = eth_mapping_addr[1];
    } else {
        sfp_port = eth_mapping_sfp_num[0];
        port_addr = eth_mapping_addr[0];
    }

    sprintf(cmd_str, "ifconfig %s%d up", SEL_PORT_XFI, eth_num);
    system(cmd_str);
    msleep(500);

    if (is_bcm82752()) {
        if (speed == 0x1) {
            if (curie_bcm82752_mode_config(curie, sfp_port, CURIE_BCM82752_PORT_SPEED_1G)) {
                printf("Failed to configure bcm82752 port1 1G speed\n");
            }
        } else {
            bcm82752_set_port_speed(port_addr, speed);
        }
    }

    rc = bcm82752_is_link_up(sfp_port, &link_up);
    if ((rc != PASSED) || (!link_up)) {
        cterr('f', 0, "PHY link up time out");
        return (rc);
    } else {
        printf("PHY link is up\n");
    }

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    if (lpbkmode == PTP_XFI_SFP_EXT_LPBK) {
        rc = neptune_ptp_set_packet(SEL_PORT_XFI, eth_num, speed);
    } else {
        rc = neptune_set_packet(SEL_PORT_XFI, eth_num, speed);
    }
    if (rc != PASSED) {
        cterr('f', 0, "neptune_set_packet failed, port: %d", eth_num);
        goto ten_g_phy_ext_lpbk_exit;
    }

    printf("\nshow packet counter\n");
    msleep(1000);
    show_eth_counter(SEL_PORT_XFI, eth_num);

ten_g_phy_ext_lpbk_exit:
    if (is_bcm82752()) {
        /*restore the setting */
        if ((set_ten_g_phy_ext_lpbk(SEL_PORT_XFI, eth_num, speed, BCM82752_LOOPBACK_NONE)) != PASSED) {
            cterr('f', 0, "disable GE PHY loopback failed");
            return (rc);
        }
    }

    sprintf(cmd_str, "ifconfig %s%d down", SEL_PORT_XFI, eth_num);
    system(cmd_str);
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

/*------------------------------------------------------------------
 *
 * Function: neptune_ten_g_phy_lpbk_test
 *
 * Description: XFI port 10GE PHY internal or external loopback test
 *              internal lpbk test: cavium->bridge PHY->media PHY
 *
 * Input: lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 *
 * Return: pass/fail
 *------------------------------------------------------------------
 */
int neptune_ten_g_phy_lpbk_test (int lpbkmode)
{
    int rc = 0, ix = 0;
    int retval = PASSED;
    int try, retry_limit = 2;
    int retry_occur = 0;
    int port_cnt, port_curr, port;
    int speed_cnt, speed_curr, speed;
    int *eth_mapping_front_panel_port = eth_mapping_sfp_num;
    int *eth_speed_list;
    int *eth_port_list = eth_bgx0_list;
    char cmd_str[32];

    if (is_thallium()) {
        eth_speed_list = xfi_eth_1g_speed_list;
    } else {
        eth_speed_list = xfi_eth_10g_speed_list;
    }

    /* get test envrionment variable */
    port_cnt = sizeof(eth_bgx0_list) / sizeof(int);
    if (lpbkmode == PTP_XFI_SFP_EXT_LPBK) {
        speed_cnt = sizeof(ptp_xfi_eth_speed_list) / sizeof(int);
    } else {
        if (is_thallium()) {
            speed_cnt = sizeof(xfi_eth_1g_speed_list) / sizeof(int);
        } else {
            speed_cnt = sizeof(xfi_eth_10g_speed_list) / sizeof(int);
        }
    }

    if (lpbkmode == PTP_XFI_SFP_EXT_LPBK) {
        for (ix = 0; ix <= port_cnt; ix++) {
            enable_bcm82752_ptp_engine(eth_mapping_front_panel_port[ix]);
        }
    }

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        port = eth_port_list[port_curr];

        for (speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            speed = eth_speed_list[speed_curr];
            switch(lpbkmode) {
            case TEN_GE_PHY_INT_LPBK:
                testname("BCM82752 PHY Internal Loopback");
                prpass(testpass, "Test TE%d speed-%d ", 
                       ((port == eth_port_list[0]) ? eth_mapping_front_panel_port[0] 
                        : eth_mapping_front_panel_port[1]),
                       ((speed == SPEED_1G) ? SPD_1000MBPS : SPD_10000MBPS));
                rc = ten_g_phy_int_lpbk_test(port, speed);
                if (rc == FAILED) {
                    cterr('f',0,"XFI 10GE PHY int loopback port %d failed",
                          eth_mapping_front_panel_port[port]);
                    retval = FAILED;
                }
            break;

            case TEN_GE_PHY_SFP_EXT_LPBK:
                testname("BCM82752 External Loopback");
                prpass(testpass, "Test TE%d speed-%d ", eth_mapping_front_panel_port[port],
                       ((speed == SPEED_1G) ? SPD_1000MBPS : SPD_10000MBPS));
                if (check_ge_int_lpbk_flag() || !check_ext_lpbk_flag()) {
                    prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
                    return (PASSED); /* external loopback is not set, skipped. */
                }
                rc = ten_g_phy_ext_lpbk_test(port, speed, lpbkmode);
                if (rc == FAILED) {
                    cterr('f',0,"XFI 10GE PHY ext loopback port %d failed",
                          eth_mapping_front_panel_port[port]);
                    retval = FAILED;
                }
            break;

            case XFI_INT_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */
                sprintf(cmd_str, "ifconfig eth%d up", port);
                system(cmd_str);
                msleep(200);

                if (check_ext_lpbk_flag()) {
                    testname("External Loopback");
                    prpass(testpass, "Test TE%d speed-%d ", 
                           ((port == eth_port_list[0]) ? eth_mapping_front_panel_port[0] 
                            : eth_mapping_front_panel_port[1]),
                           ((speed == SPEED_1G) ? SPD_1000MBPS : SPD_10000MBPS));

                    for (try = 0; try < retry_limit; try++) {
                        printf("\n\nAt the beginning of the test - Display Linux "
                               "Ethernet counters\n");
                        show_eth_counter(SEL_PORT_XFI, port);
                        rc = ten_g_phy_ext_lpbk_test(port, speed, TEN_GE_PHY_SFP_EXT_LPBK);

                        if ((rc == PASSED) || (try == (retry_limit - 1))) {
                            break;
                        } else {
                            printf("\n\nBefore retry the test - Display Linux "
                                   "Ethernet counters\n");
                            show_eth_counter(SEL_PORT_XFI, port);
                            printf("####### retry the test #########\n");
                            retry_occur = 1;
                            //bcm82752_soft_reset(port, BCM82752_DEV_PMA);
                        }
                    }

                    if (rc != PASSED) {
                        cterr('f', 0, "XFI PHY ext loopback TE port %d failed",
                              eth_mapping_front_panel_port[port]);
                        retval = FAILED;
                    }
                } else {
                    testname("Internal Loopback");
                    prpass(testpass, "Test TE%d speed-%d ", 
                           ((port == eth_port_list[0]) ? eth_mapping_front_panel_port[0] 
                            : eth_mapping_front_panel_port[1]),
                           ((speed == SPEED_1G) ? SPD_1000MBPS : SPD_10000MBPS));

                    for (try = 0; try < retry_limit; try++) {
                        printf("\n\nAt the beginning of the test - Display Linux "
                               "Ethernet counters\n");
                        show_eth_counter(SEL_PORT_XFI, port);
                        rc = ten_g_phy_int_lpbk_test(port, speed);

                        if ((rc == PASSED) || (try == (retry_limit - 1))) {
                            break;
                        } else {
                            printf("\n\nBefore retry the test - Display Linux "
                                   "Ethernet counters");
                            show_eth_counter(SEL_PORT_XFI, port);
                            printf("####### retry the test #########\n");
                            retry_occur = 1;
                            //bcm82752_soft_reset(port, BCM82752_DEV_PMA);
                        }
                    }

                    if (rc != PASSED) {
                        cterr('f', 0, "XFI PHY int loopback TE port %d failed",
                              eth_mapping_front_panel_port[port]);
                        retval = FAILED;
                    }
                }

            break;

            case PTP_XFI_SFP_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */

                if (check_ext_lpbk_flag()) {
                    testname("TE%d External Loopback", eth_mapping_front_panel_port[port]);
                    prpass(testpass, "Test TE%d speed-%d ", eth_mapping_front_panel_port[port],
                           ((speed == SPEED_1G) ? SPD_1000MBPS : SPD_10000MBPS));

                    for (try = 0; try < retry_limit; try++) {
                        printf("\n\nAt the beginning of the test - Display Linux "
                               "Ethernet counters - speed = %d\n", speed);
                        show_eth_counter(SEL_PORT_XFI, port);
                        rc = ten_g_phy_ext_lpbk_test(port, speed, lpbkmode);

                        if ((rc == PASSED) || (try == (retry_limit - 1))) {
                            break;
                        } else {
                            printf("\n\nBefore retry the test - Display Linux "
                                   "Ethernet counters - speed = %d\n", speed);
                            show_eth_counter(SEL_PORT_XFI, port);
                            printf("####### PTP lpbk retry the test #########\n");
                            retry_occur = 1;
                            //bcm82752_soft_reset(port, BCM82752_DEV_PMA);
                            enable_bcm82752_ptp_engine(port);
                        }
                    }

                    if (rc != PASSED) {
                        cterr('f', 0, "PTP loopback TE port %d failed",
                              eth_mapping_front_panel_port[port]);
                        retval = FAILED;
                    }
                } else {
                    cterr('f', 0, "External loopback flag is not turn on");
                }

            break;

            default:
                retval = FAILED;
                cterr('f',0," Neptune not support this loopback mode");
                break;
            }

            if (retval != PASSED) {
                break;
            }
        } /*speed*/
    }/*port*/

    if (retry_occur == 1) {
        cterr('f',0,"XFI loopback test retry occurred, \
              please investigate the problem - TE%d\n", eth_mapping_front_panel_port[port]);
        retval = FAILED;
    }
#if DEBUG
    printf("*******End*******\n");
    system("date"); /* real time counter */
    printf("*****************\n");
#endif

    return (retval);
}

/**********************************************************************
 *
 * Function: neptune_ten_g_phy_lpbk_util
 *
 * Description:
 * Utility to execute XFI single port internal or external loopback
 *
 * Input: void
 *
 * Return: pass/fail
 */
int neptune_ten_g_phy_lpbk_util(void)
{
    int port, low_port, rc = 0;
    int speed, spdsel;
    int lpbksel;
    char qrybuf[64];

    low_port = XFI0;

    printf("\nSelect loopback 0:CAVIUM_INT_LPBK    1:TEN_GE_PHY_INT_LPBK");
    printf("\n                2:TEN_GE_PHY_SFP_EXT_LPBK");
    lpbksel = getdec_answer("\nEnter ", 0, 0, 2);
    switch(lpbksel) {
  	    case 1:
  		    lpbksel = TEN_GE_PHY_INT_LPBK; break;
  	    case 2:
  		    lpbksel = TEN_GE_PHY_SFP_EXT_LPBK; break;
        default:
        	lpbksel = CAVIUM_INT_LPBK; break;
    	break;
    }

    sprintf(qrybuf, "\nEnter port number (%d - %d)", low_port, PLAT_XFI_NUM_MAX);
    port = getdec_answer(qrybuf, low_port, low_port, PLAT_XFI_NUM_MAX);

	sprintf(qrybuf, "\nEnter speed (0: 1000MBS, 1: 10000MBS)");
    spdsel = getdec_answer(qrybuf, 0, 0, 1);
    switch(spdsel) {
  	    case 0:
  		    speed = SPD_1000MBPS; break;
  	    case 1:
  		    speed = SPD_10000MBPS; break;
        default:
    	    printf("\n not support this speed. ");
    	break;
    }

    switch (lpbksel) {
        case TEN_GE_PHY_INT_LPBK:
            testname("PHY internal loopback");
            prpass(testpass, "Test XFI-%d with speed-%d, ", port, speed);
            rc = ten_g_phy_int_lpbk_test(port, speed);
        break;
        case TEN_GE_PHY_SFP_EXT_LPBK:
            testname("SFP PHY external loopback");
            prpass(testpass, "Test SFP-%d with speed-%d, ", port, speed);
            rc = ten_g_phy_ext_lpbk_test(port, speed, lpbksel);
        break;
        default:
            printf("\n not support this loopback test. ");
        break;
    }
#if 0
    if (rc == FAIL) {
        neptune_err_clean_up(port);
        cterr('f',0,"Loopback test failed on port %d speed %d \n", port, speed);
    }
#endif
    return(rc);
}

int send_packet_util(void)
{
      uint port, speed, rc;
    
      port = getdec_answer("\nEnter port ", 0, 0, 3);
      speed = getdec_answer("\nEnter speed 0:10 1:100", 0, 0, 1);
      
      /* clean up buffer */
      memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN); 
      memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); 
      
      switch(speed) {
        case 0: 
            speed = SPD_10MBPS; 
            break;
        case 1:
            speed = SPD_100MBPS;
            break;
        default:
            printf("Speed %d not support\n", speed);
            break;
      }
    
    rc = tx_rx_diag(SEL_PORT_ETH, port, speed, 2, 6, 1);    
    
      if (rc == FAILED) {
        printf("Packet transmit faid\n");
        return (FAILED);
      } else {
        printf("Packet transmit pass\n");
        return (PASSED);
      }
}

void write_txrx_pkt(void){
    
    FILE *fd_record1, *fd_record2, *fd_record3; 
    int ix; 
  
    fd_record1 = fopen("packet_tx.log", "w");
    fd_record2 = fopen("packet_rx_1st.log", "w");
    fd_record3 = fopen("packet_rx_2nd.log", "w");
    if (fd_record1 == NULL) {
        cterr('f',0,"open packet_tx.log failed. \n");
    }
    if (fd_record2 == NULL) {
        cterr('f',0,"open packet_rx_1st.log failed. \n");
    }
    if (fd_record3 == NULL) {
        cterr('f',0,"open packet_rx_2nd.log failed. \n");
    }
        
    for( ix = 0; ix < ETH_PKT_MAX_LEN; ix++) {
        fprintf(fd_record1, "%d", tx_packet[ix]);  
        fprintf(fd_record2, "%d", rx_packet[ix]);  
        fprintf(fd_record3, "%d", rx_packet_sec[ix]); 
    }
    fclose(fd_record1); 
    fclose(fd_record2);
    fclose(fd_record3);
    
}

void write_statistic(void){

     system("ifconfig eth2 > statisitcTX.log"); 
}


void print_statistic_tofile(int count, int packet_count, int receive_byte){

    FILE *fd_record; 
    int ix = 0;
    
    switch(count) { 
    case 0:
     system("ifconfig eth2 > statisitc0.log");
     fd_record = fopen("statisitc0.log", "a");
         if (fd_record == NULL) {
        cterr('f',0,"open statisitc0.log failed. \n");
    //    return FAILED;
    }
    break;
    case 1:
     system("ifconfig eth2 > statisitc1.log");
     fd_record = fopen("statisitc1.log", "a");
         if (fd_record == NULL) {
        cterr('f',0,"open statisitc1.log failed. \n");
    //    return FAILED;
    }
    break;
    case 2:
     system("ifconfig eth2 > statisitc2.log");
     fd_record = fopen("statisitc2.log", "a");
         if (fd_record == NULL) {
        cterr('f',0,"open statisitc2.log failed. \n");
    //    return FAILED;
    }
    break;
     case 3:
     system("ifconfig eth2 > statisitc3.log");
     fd_record = fopen("statisitc3.log", "a");
         if (fd_record == NULL) {
        cterr('f',0,"open statisitc3.log failed. \n");
    //    return FAILED;
    }
    break;
    default:
     printf(" print_statistic_tofile failed\n" );
    break;
    }
    
      fprintf(fd_record, "\n packet_count = %d \n receive_byte = %d  \n print rx_buffer \n", packet_count, receive_byte);

    for( ix = 0; ix < (ETH_PKT_MAX_LEN); ix++) {

        fprintf(fd_record, "%d", rx_packet[ix]);  /* we expect two packets in here zzzz. */
        if ((ix % 100) == 0 )
           fprintf(fd_record, "\n");
    }
    
    fprintf(fd_record, " \n print rx_buffer_sec \n");
    for( ix = 0; ix < (ETH_PKT_MAX_LEN); ix++) {

        fprintf(fd_record, "%d", rx_packet_sec[ix]);  /* we expect two packets in here zzzz. */
        if ((ix % 100) == 0 )
           fprintf(fd_record, "\n");
    }
#if 0
    fprintf(fd_record, " \n print global_buffer \n");
    for( ix = 0; ix < (ETH_PKT_MAX_LEN*2 + 1); ix++) {
        fprintf(fd_record, "%d", global_pkt_array[ix]);  /* we expect two packets in here zzzz. */
         if ((ix % 100) == 0 )
           fprintf(fd_record, "\n");
    }
#endif     
          
      fclose(fd_record);    
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


void print_packet_info(void) {

    FILE *fd_record; 
    int ix = 0;
    
    fd_record = fopen("packet_record.log", "w");
    if (fd_record == NULL) {
        cterr('f',0,"open packet_record.log failed. \n");
    //    return FAILED;
    }
    
    for( ix = 0; ix < (ETH_PKT_MAX_LEN*2 + 1); ix++) {
        fprintf(fd_record, "%d", global_pkt_array[ix]);  /* we expect two packets in here zzzz. */
        if ((ix % 100) == 0 )
           fprintf(fd_record, "\n");
    }
    fclose(fd_record);

}

static void *thread_rx_buffer(void *argument)
{
    int rx= 0, rx_sec = 0, pkt_cnt = 0, temp = 0;
    unsigned char *rx_pkt_buf, *rx_pkt_sec;
    int raw;
    char pname[10];

    /*fixed for eth2*/
    sprintf(pname,"eth%d", 2);  

    /* socket setting */
    if (setup_eth_port(pname, &raw) == FAIL) {
        return (void *)FAILED;
    }

    /* clean up buffer */
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet_sec, 0, ETH_PKT_MAX_LEN);
        
    rx_pkt_buf = rx_packet;
    rx_pkt_sec = rx_packet_sec;
    
    /* keep on reading socket. */
    while(1) {
        if((rx = read(raw, (unsigned char *)rx_pkt_buf, ETH_UDP_DATA_MIN_LEN)) != ETH_UDP_DATA_MIN_LEN) {
             if (rx < 0) {
                 printf("receive socket timeout. %d packets received\n", pkt_cnt);
                 return (void *)FAILED;
             } else {
                 printf("received %d bytes but expected %d bytes\n",rx, ETH_UDP_DATA_MIN_LEN);
             }
             return (void *)FAILED;
        }

        /* drop invalid packet*/  
        if ((curie_chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) &&
            (curie_chk_macaddr(&rx_pkt_buf[6], (uchar *)mac_sa) != 0)) {
            memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); /* clean up buffer */
            printf("\n detected non diag packet. Ignore.\n");
            pkt_cnt = 0; /* restore the packet count */
            continue; /* not firmware download packet */
        }

        pkt_cnt++;
        temp = (pkt_cnt %4);
        print_statistic_tofile(temp, pkt_cnt, rx );



        if((rx_sec = read(raw, (unsigned char *)rx_pkt_sec, ETH_UDP_DATA_MIN_LEN)) != ETH_UDP_DATA_MIN_LEN) {
             if (rx_sec < 0) {
                 printf("receive socket timeout. %d packets received on duplicate packet \n", pkt_cnt);
                 return (void *)FAILED;
             } else {
                 printf("received %d bytes but expected %d bytes on duplicate packet \n",rx_sec, ETH_UDP_DATA_MIN_LEN);
             }
             return (void *)FAILED;
        }

        /* drop invalid packet*/  
        if ((curie_chk_macaddr(&rx_pkt_sec[0], (uchar *)mac_da) != 0) &&
            (curie_chk_macaddr(&rx_pkt_sec[6], (uchar *)mac_sa) != 0)) {
            memset((unsigned char *)rx_packet_sec, 0, ETH_PKT_MAX_LEN); /* clean up buffer */
            printf("\n detected non diag packet. Ignore.\n");
            pkt_cnt = 0; /* restore the packet count */
            continue; /* not firmware download packet */
        }

        pkt_cnt++;
        temp = (pkt_cnt %4);
        print_statistic_tofile(temp, pkt_cnt, rx_sec );

        neptune_packet_count++;
        /*inform tx that rx receive interrupt. */
        //write_pktcnt(pkt_cnt);
    }

    return (PASSED);
}
    
    
/*create rx buffer before entering menu*/
void create_rx_buffer(void) {

    pthread_t threads;

    if(pthread_create(&threads, NULL, thread_rx_buffer, (void *)NULL)) {
        printf("pthread_create failed \n");
        exit(-1);
    }
    
}




/*
$Log: platform_ext_lpbk.c,v $
Revision 1.3  2020/01/21 05:49:01  leschen
Fixing Ethernet port 4(10G) high temperature lbpk issue.

Revision 1.2  2019/08/06 06:56:13  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.4  2019/06/04 03:07:59  leschen
Check link status before executing ext lpbk test

Revision 1.1.2.3  2019/04/09 08:51:06  leschen
Fix Thallium 1G internal loopback issue when external module is not present

Revision 1.1.2.2  2019/04/09 01:31:32  leschen
Remove unnecessary flag checking for BCM82752 lpbk

Revision 1.1.2.1  2019/03/12 07:41:52  leschen
Initial check in to support BCM82752


$Endlog$
*/
