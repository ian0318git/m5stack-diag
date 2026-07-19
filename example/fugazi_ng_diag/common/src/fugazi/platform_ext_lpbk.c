/* $Id: platform_ext_lpbk.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_ext_lpbk.c  
 * support PHY external loopback 
 * internal loopback: GE PHY and Cavium.
 *
 * June 2016 Mecca Ho
 * Jan 2019, Letsai modified for Fugazi.
 *
 * Copyright (c) 2016 - 2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
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
#include<stdint.h>

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
#include "eth_pkt_utils.h"
#include "router_if.h"

#include "queryflags.h"

#include "router_if.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "diag_bcm54194_api.h"

void create_rx_buffer();
int wait_iface_link_stats(char *, int);
int fugazi_cavium_is_linkup(char *, int);
int fugazi_phy_soft_reset (char *, boolean);
int force_linkup(boolean, int);
int direct_phy_soft_reset(int);
int fugazi_err_clean_up(int);

int is_glc_ge_100fx = 0;

/* Fugazi: eth number from GE port 0 ~ 7 */
int eth_bgx2_list[] = {ETH4, ETH5, ETH6, ETH7, ETH8, ETH9, ETH10, ETH11};

/* global */
int print_statistic = 0;
unsigned int fugazi_packet_count = 0;

/* packet buffer */
unsigned char tx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet_sec[ETH_PKT_MAX_LEN];
unsigned char global_pkt_array[ETH_PKT_MAX_LEN*2+1]; //12.30

/* for ctrl diag flow */
volatile int tx_rx_box = 0;
sem_t rx_ready, rx_finish, tx_cmp;

/* Packets to be used in port loopback tests
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
static mac_addr_t mac_da = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static mac_addr_t mac_sa = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};

 /*------------------------------------------------------------------
 *
 * Function: create_raw_socket
 *     Create the raw socket with specific protocol.
 *
 * Input:  protocol - seclect protocol
 *
 * Output: rawsock - return created socket num.
 *
 *------------------------------------------------------------------
 */
int create_raw_socket(int protocol)
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
 * Function: bind_socket
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
int bind_socket(char *device, int rawsock, int protocol) {
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
 * Function: set_promisc
 *    set promisc mode.
 *    when program exit, this interface will still be promisc mode.
 *    we should disable promisc mode when we exit (ie, use atexit)
 *
 * Input:  
 *
 * Output: PASSED/FAILED
 *
 * Note: if the set_promisc is failed, the rx will get the half of
 *       packet from tx.
 *------------------------------------------------------------------
 */
int set_promisc(char *device, int sock) {

    struct ifreq ifr;
 
    bzero(&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */
    /* Set the network card in promiscuos mode */
    strncpy(ifr.ifr_name, device, IFNAMSIZ);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS");
        // close(sock); /* don't close sock here, let caller handle it */
        return FAILED;
    }
    
    ifr.ifr_flags|=IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to set promiscous mode");
        // close(sock); /* don't close sock here, let caller handle it */
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
 *          *socket - pointer for passing the created socket ID
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
    if (set_promisc(eth_name, raw) == -1) {
        close(raw);    /* closed socket */
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        close(raw);    /* closed socket */
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
 * Description: Compare 2 packet's (tx/rx) CRC
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
 * Function: fugazi_is_linkup
 *   Check linux up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int fugazi_cavium_is_linkup (char *type, int eth_num)
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
             /* 1 second timeout to check link up */
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
 * Input:  pkt_len - packet length to check.
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

/*
 * Function: chk_macaddr
 *
 * Description: Compare 2 input MAC address CRC
 *
 * Input: macaddr1 - received MAC address from received packet buffer
 *        macaddr2 - expect MAC address
 *
 * Return: pass/fail
 */
int chk_macaddr (uchar *macaddr1, uchar *macaddr2)
{
    int rc = PASSED;
    uint crc_tx, crc_rx;
 
    crc_tx = ~crc32(~0, macaddr1, 6);
    crc_rx = ~crc32(~0, macaddr2, 6);
    if (crc_tx != crc_rx) {
        if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
            printf("rx MAC address crc doesn't match expect MAC address crc\n");
        }
        rc = FAILED;
    }
    return (rc);

}

/*------------------------------------------------------------------
 *
 * Function: ptp_check_pkt
 *   Compared the ptp packet between the buffer of tx and rx.
 *
 * Input:  pkt_len - packet length to check.
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
    if (diagflag_xram & D_DEBUG_OPTIONS) {
        int ix = 0;
        printf("len = %d  \n", len);
        for ( ix = 0; ix < len; ix++) {
           printf("tx_packet[%d] = %#.2x  ", ix, tx_packet[ix]);
        }
    }
    rc = fugazi_cavium_is_linkup(iface_type, port);
    if (rc == FAILED) {
        cterr('f',0, "port %d link up time out", port);
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

    rc = fugazi_cavium_is_linkup(iface_type, port);
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
            if (chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) {
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
                rintf("end of pkt print\n");
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
        printf("setup_eth_port() %s for tx socket failed.\n", pname);
        return(FAILED);
    }

    if (setup_eth_port(pname, &rx_skt) == FAIL) {
        printf("setup_eth_port() %s for rx socket failed.\n", pname);
        return(FAILED);
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
                printf("ptp_send_packets failed at eth port %d!\n", eth_port);
                goto exit_tx_rx_diag;
            }
        } else {
            rc = send_packets(&tx_skt, p_type, pkt_len, value, eth_port, speed);

            if (rc == FAILED) {
                printf("send_packets failedat eth port %d!\n", eth_port);
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
        // close(sock); /* don't close sock here, let caller handle it */
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
        // close(sock); /* don't close sock here, let caller handle it */
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


/*------------------------------------------------------------------
 *
 * Function: fugazi_set_packet
 *  Set up packet info for tx and rx using.
 *
 * Input:  port: current test port
 *         speed: current test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int fugazi_set_packet(char *type, int port, int speed)
{
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0;
    uchar orig_hkpflag = hkeepflags;

    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;

    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = pktdata[typ_curr].send_count;
        pkt_len = pktdata[typ_curr].len;
        pkt_val = pktdata[typ_curr].val;
        hkeepflags |= pktdata[typ_curr].hkpflags;
        if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)\n",
                port, speed, pkt_cnt, pkt_len, pkt_val);
        }
        fflush(stdout);

        /* prepare to send packet */
        rc = tx_rx_diag(type, port, speed, pkt_cnt, pkt_len, pkt_val);
        if (rc == FAILED) {
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d",__FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            return (FAILED);
        }
    } /* typ_curr */

    printf("Passed\n");
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: fugazi_ptp_set_packet
 *  Set up PTP packet info for tx and rx using.
 *  Note: not used on Fugazi. Fugazi doesn't support PTP.
 *
 * Input:  port: current test port
 *            speed: current test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int fugazi_ptp_set_packet (char *type, int port, int speed)
{
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0;
    uchar orig_hkpflag = hkeepflags;

    pkt_type = sizeof(ptp_pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;

    for (typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = ptp_pktdata[typ_curr].send_count;
        pkt_len = ptp_pktdata[typ_curr].len;
        pkt_val = ptp_pktdata[typ_curr].val;
        hkeepflags |= ptp_pktdata[typ_curr].hkpflags;

        prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                                    port, speed, pkt_cnt, pkt_len, pkt_val);
        fflush(stdout);

        /* Write value 1 to pkt_len bit 31 for distinguish ptp lpbk packet or normal
          lpbk packet */
        pkt_len |= PTP_PKT_LEN_BIT_31_MASK;

        /* To do the tx/rx loopback test */
        rc = tx_rx_diag(type, port, speed, pkt_cnt, pkt_len, pkt_val);
        if (rc == FAILED) {
            cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            return (FAILED);
        }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", port, speed);
    fflush(stdout);

    hkeepflags = orig_hkpflag;

    return (rc);
}

/*------------------------------------------------------------------
 *
 * Function: set_ge_phy_lpbk
 *  initial and setup loopback type on sgmii for external lpbk 
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_ge_phy_lpbk(char *type, int eth_num, int speed, int intf,
                    int lpbk_mode, boolean enable)
{
    char pname[10];
    int rc = PASSED;
    int phy_addr = ge_port_mapping_phy_addr_down[eth_num];
    int phy_num = (int) (eth_num/2);

    sprintf(pname,"%s%d", type, eth_num);

    
    rc = bcm54194_config_loopback(phy_num, phy_addr, speed, intf, lpbk_mode, enable);
    if (rc != PASSED) {
        printf("GE PHY %d config loopback failed.\n", phy_num-2);
    }
    
    if (enable) {
        /* Check GE PHY SERDES link status */
        if (!bcm54194_is_linkup(phy_num, phy_addr, intf)) {
            printf("%s(): GE PHY %d addr %#.2x intf:%d link up time out\n",
                   __FUNCTION__, phy_num-2, phy_addr, intf);
            rc = FAILED;
        } else {
            bcm54194_suspend_lnx_link_polling (type, eth_num, FALSE);
            return rc;
        }
    }
    return (rc);
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
 * Function: ge_phy_check_ext_sfp_lpbk
 *  Check if there is ext loopback on this eth port by read PHY's link status.
 *
 * Input:  eth_num - eth port number
 *
 * Output: TRUE - yes.
 *         FALSE- No
 *
 *------------------------------------------------------------------
 */

int ge_phy_check_ext_sfp_lpbk(int eth_num)
{
    uint16_t rdb_rdval;
    int phy_num = (int) (eth_num/2);
    int phy_addr = ge_port_mapping_phy_addr_down[eth_num];

    /* Read Network side link status */
    bcm54194_rdb_read(phy_num, phy_addr, 0x21, &rdb_rdval);
    msleep(1);
    bcm54194_rdb_read(phy_num, phy_addr, 0x21, &rdb_rdval);
    if (rdb_rdval & 0x40)
    {
        return (TRUE);  /* Link up */
    } 

    return (FALSE);
}

/*------------------------------------------------------------------
 *
 * Function: ge_phy_int_lpbk_test
 *  This is the entry point for PHY internal loopback test.
 *
 * Input:  port - port number
 *         speed - test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ge_phy_int_lpbk_test(int eth_num, int speed, int lpbkmode)
{
    int rc = PASSED;

    /* setup loopback information */          
    rc = set_ge_phy_lpbk(SEL_PORT_ETH, eth_num, speed, BCM54194_FIBER_INTF,
                         lpbkmode, TRUE);

    if (rc == FAILED) {
        printf("set_ge_phy_lpbk failed, port: eth%d\n", eth_num);
        return (rc);
    }

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = fugazi_set_packet(SEL_PORT_ETH, eth_num, speed);

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
 * Function: fugazi_phy_lpbk_test
 *
 * Description: SGMII port PHY internal or external loopback test
 *              internal lpbk test: cavium->bridge PHY->media PHY
 * 
 * Input: lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 *
 * Return: pass/fail
 *------------------------------------------------------------------
 */
int fugazi_phy_lpbk_test (int lpbkmode)
{
    int rc = 0;
    int retval = PASSED; 
    int port_cnt, port_curr, port;
    int speed, f_ext_lpbk;
    int *eth_port_list = eth_bgx2_list;


    /* get test envrionment variable */
    port_cnt = sizeof(eth_bgx2_list) / sizeof(int);

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
            port = eth_port_list[port_curr];
            speed = SPD_1000MBPS;
            /* Get ext lpbk info */
            f_ext_lpbk = ge_phy_check_ext_sfp_lpbk(port);

            switch(lpbkmode) {

            case GE_PHY_INT_LPBK:
                testname("BCM54194 PHY Internal Loopback");
                prpass(testpass, "\n- Test GE PHY eth %d speed-%d -\n", port, speed);
                printf("\n!!! BCM PHY54194 internal loopback require no external plug connected !!!\n");
                
                /* If SFP connected, skip internal loopback test */
                if (!f_ext_lpbk ) {
                    printf("\n\nAt the beginning of the test - Display Linux "
                           "Ethernet counters - speed = %d\n", speed);
                    show_eth_counter(SEL_PORT_ETH, port);

                    rc = ge_phy_int_lpbk_test(port, speed, lpbkmode);
                    if (rc == FAILED) {
                        cterr('f',0,"SGMII GE PHY int loopback eth %d failed", port);
                        retval = FAILED;
                    }
                }
                else {
                    printf("\n+++ eth%d SFP external loopback detected, skip internal loopabck test. +++\n",
                    		port);
                }
            break;

            default:
                retval = FAILED;
                cterr('f',0," Fugazi not support this loopback mode");
                break;
            }

            if (!f_ext_lpbk ) {
                printf("\n\nIn the end of the test - Display Linux "
                        "Ethernet counters - speed = %d\n", speed);
                show_eth_counter(SEL_PORT_ETH, port);

            }
    }/*port*/

#if DEBUG
    printf("*******End*******\n");
    system("date"); /* real time counter */
    printf("*****************\n");
#endif

    return (retval);
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
    
    if (fd_record1)
        fclose(fd_record1);

    if (fd_record2)
        fclose(fd_record2);

    if (fd_record3)
        fclose(fd_record3);

}

void write_statistic(void){

     system("ifconfig eth2 > statisitcTX.log"); 
}


void print_statistic_tofile(int count, int packet_count, int receive_byte){

    FILE *fd_record=NULL;
    int ix = 0;
    
    switch(count) { 
    case 0:
        system("ifconfig eth2 > statisitc0.log");
        fd_record = fopen("statisitc0.log", "a");
            if (fd_record == NULL) {
                cterr('f',0,"open statisitc0.log failed. \n");
            }
        break;
    case 1:
        system("ifconfig eth2 > statisitc1.log");
        fd_record = fopen("statisitc1.log", "a");
        if (fd_record == NULL) {
            cterr('f',0,"open statisitc1.log failed. \n");
        }
        break;
    case 2:
        system("ifconfig eth2 > statisitc2.log");
        fd_record = fopen("statisitc2.log", "a");
        if (fd_record == NULL) {
            cterr('f',0,"open statisitc2.log failed. \n");
        }
        break;
    case 3:
        system("ifconfig eth2 > statisitc3.log");
        fd_record = fopen("statisitc3.log", "a");
        if (fd_record == NULL) {
            cterr('f',0,"open statisitc3.log failed. \n");
        }
        break;
    default:
        printf(" print_statistic_tofile failed\n" );
        break;
    }

    if (fd_record==NULL) {
        /* return if open file failed */
        return;
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
          
    fclose(fd_record);
}



/*-------------------------------------------------
 * $Log: platform_ext_lpbk.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.3  2021/05/21 18:46:25  pdoong
 * Let BCM54195 internal loopback test complete on every port even there is a failure happen.
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.24  2020/08/24 00:03:16  pdoong
 * Clean code for ER.
 *
 * Revision 1.1.6.23  2020/08/06 04:37:27  pdoong
 * clean code for BCM54194 1G PHY
 *
 * Revision 1.1.6.22  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.21  2020/06/18 06:11:58  iachang
 * Enhance error message include port number.
 *
 * Revision 1.1.6.20  2019/10/16 06:12:31  letsai
 * Modify file name
 *
 * Revision 1.1.6.19  2019/08/09 02:18:32  letsai
 * Enhance error message when test failed.
 *
 * Revision 1.1.6.18  2019/08/02 07:16:50  letsai
 * 1.Add debug messgage. 2.Fix initial process for BCM 54194 phy
 *
 * Revision 1.1.6.17  2019/06/27 03:17:11  letsai
 * Print message to remove loopback cable before doing BCM54194 internal loopback.
 *
 * Revision 1.1.6.16  2019/06/15 03:48:42  letsai
 * 1.Fix Rx mismatch error messgage showed in loopback test. 2.Removed Copper registers in BCM54194 phy register test. 3.Add print messgge when reset 1G phy.
 *
 * Revision 1.1.6.15  2019/06/12 03:10:15  letsai
 * Add message of print info.
 *
 * Revision 1.1.6.14  2019/06/06 02:19:35  letsai
 * Modify test message. Reset and init 54194 phy before internal test.
 *
 * Revision 1.1.6.13  2019/05/13 06:51:13  letsai
 * Fix link status change
 *
 * Revision 1.1.6.12  2019/05/13 01:32:36  letsai
 * Check link status before send packets.
 *
 * Revision 1.1.6.11  2019/05/09 08:10:36  letsai
 * Modify BCM 54194 phy test.If SFP connected, skip internal loopback test.
 *
 * Revision 1.1.6.10  2019/04/25 17:46:57  letsai
 * For BCM54194 phy loopback test, if config loopback failed, don't send package.
 *
 * Revision 1.1.6.9  2019/04/18 23:11:58  letsai
 * Add loopback mode config uyility and clean up code.
 *
 * Revision 1.1.6.8  2019/04/18 01:21:30  letsai
 * 1. Clean up code
 * 2. Modify 1G phy address mapping
 * 3. Modify print message of MCU FW opgrade
 *
 * Revision 1.1.6.7  2019/04/10 21:26:59  letsai
 * 1. Support BCM54194 PHY SGMII Internal Loopback test.
 * 2. Return FAILED when M.2 module not present.
 * 3. Clean up code.
 *
 * Revision 1.1.6.6  2019/04/10 16:29:30  letsai
 * 1. Fix ethernet mapping.
 * 2. Support all BCM54194 phy in utilities.
 * 3. Remove unused functions.
 *
 * Revision 1.1.6.5  2019/04/09 16:10:40  letsai
 * 1. Support all BCM54194 PHY (0~3) Register Test.
 * 2. Let utilities can dump each phy registers.
 * 3. Check link status for each phy and each port(upstream and downstream).
 *
 * Revision 1.1.6.4  2019/04/06 01:36:14  letsai
 * 1. Remove unused functions and files.
 * 2. Fix BCM54194 SFP External loopback test.
 * 3. Fix BCM54194 Register test.
 * 4. Fix Voltage Margin Utility.
 * 5. Add function to show system information.
 *
 * Revision 1.1.6.3  2019/03/25 18:37:36  letsai
 * Modified eth and port number
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
