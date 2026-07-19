/* $Id: platform_ext_lpbk.c,v 1.2 2013/10/08 08:48:30 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * platform_ext_lpbk.c  
 * support PHY external loopback 
 * internal loopback: media PHY, bridge PHY and Cavium.
 *
 * January 2012 Kody Ko
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
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

#include "cvmx.h"
#include "cvmx-gmxx-defs.h"
#include "cvmx-pcsxx-defs.h"

#include "queryflags.h"

#include "router_if.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "diag_ge_phy_88E1340_lib.h"
#include "diag_ge_phy_88E1548L_lib.h"
#include "diag_fpga_lib.h"
#include "platform_xaui.h"
#include "diag_common_drv.h"

void create_rx_buffer();
int phy_check_iface_up_with_speed(char *, int);
int wait_iface_link_stats(char *, int);
int setup_xaui_port(int, int *);
int woodlawn_cavium_is_linkup(char *, int);
int woodlawn_phy_soft_reset (char *, boolean);
int sgmii_set_packet(int, int);
int force_linkup(boolean, int);
int sgmii_adv_full_duplex(boolean, int);
int direct_phy_soft_reset(int);
int set_bridge_phy_speed(int, int);
int woodlawn_err_clean_up(int);

extern int enable_88e1548_ptp_engine(int);
extern int en_88e1548_ptp_per_port(int, int);
/* global */

/*Note: eth0 not tested during development, it is used to connect to dhcp server*/

int print_statistic = 0;
unsigned int woodlawn_packet_count = 0;

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
  {0xa0, ETH_UDP_DATA_MIN_LEN, H_INCFILL, 5},
  {0xa7, (ETH_UDP_DATA_MIN_LEN + 1), H_INCFILL, 5},
  {0xa5, ((ETH_UDP_DATA_MAX_LEN - 1) - 12), H_INCFILL, 5},
  {0xa3, (ETH_UDP_DATA_MAX_LEN - 12), H_INCFILL, 5},
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

mac_addr_t mac_da = {0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc};
mac_addr_t mac_sa = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56};

/* Copper speed table */
static int eth_speed_list[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};

/* PTP speed table */
static int ptp_eth_speed_list[] = {SPD_1000MBPS};

/*
 * Function: bridge_phy_autoneg_on
 *
 * Description: Turn on the 1340 PHY autoneg
 *
 * Input: port - port number 0-3
 *
 * Return: void
 */
void bridge_phy_autoneg_on(int port)
{
    int phy_addr = qlm_0_4_1340_phy_addr[port];
    int bus_id;

    bus_id = get_smi_bus_id(phy_addr);

    woodlawn_phy_reg_wr(bus_id, phy_addr, PHY_REG(22), PHY_REG(4));
    /* per FAE: Make sure autoneg with the speed set at the 1548
     * (10, 100, 1000 accordingly)
     */
    woodlawn_phy_reg_wr(bus_id, phy_addr, PHY_REG(0), 0x9140);
}

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
 * Note: if the set_promisc is failed, the rx will get the haft of 
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
int setup_eth_port (int sgmii_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf (eth_name, "eth%d", sgmii_port);

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (set_promisc(eth_name, raw) == -1) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        return(FAIL);
    }

    *socket = raw;

    return PASS;
}

/***********************************************************************
 *
 * Function:    setup_xaui_port()
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
int setup_xaui_port (int xaui_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf (eth_name, "xaui%d", xaui_port);

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAIL);
    }

    /* Set socket to promiscuous mode
     */
    if (set_promisc(eth_name, raw) == -1) {
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
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


int pkt_cmp (unsigned char *bufa, unsigned char *bufb, int count)
{
    int ib = 0, rc = 0;
    unsigned char *p1 = bufa;
    unsigned char *p2 = bufb;

    for (ib = 0; ib < count; ib++, p1++, p2++) {
      if (*p1 != *p2) {
            if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
                printf("failed on byte %d, first data = %02x, second data = %02x\n",
                       (ib+1), *p1, *p2);
                printf("print byte %d, first data = %02x, second data = %02x\n",
                       (ib+2), *(p1+1), *(p2+1));
                printf("print byte %d, first data = %02x, second data = %02x\n",
                       (ib+3), *(p1+2), *(p2+2));
                printf("print byte %d, first data = %02x, second data = %02x\n",
                       (ib+4), *(p1+3), *(p2+3));
            }

            rc = FAILED;
            break;
        }
    }
      return rc;
}

/*------------------------------------------------------------------
 *
 * Function: woodlawn_cavium_is_linkup
 *   Check linux up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int woodlawn_cavium_is_linkup (char *type, int port)
{

    int timeout_counter = 100, is_link = FALSE;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];

    sprintf(pname,"%s%d", type, port);

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

int chk_macaddr (uchar *macaddr1, uchar *macaddr2)
{
    return (pkt_cmp(macaddr1, macaddr2,6));
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
int send_packets(int *socket, int len, char val, int port, int speed)
{   
    int raw, rc = 0;
    uint mac_size, fil_len;
    unsigned char volatile *cptr;
    char iface_type[32];
    
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
#if 0
    printf("len = 0x%2x  \n", len);
    for ( ix =0; ix <len; ix++)
       printf("tx_packet[%d] = 0x%2x  ", ix, tx_packet[ix]);
#endif 

    if (speed == SPD_10000MBPS) {
        sprintf(iface_type, SEL_PORT_XAUI);
    } else {
        sprintf(iface_type, SEL_PORT_ETH);
    }

    rc = woodlawn_cavium_is_linkup(iface_type, port);
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
int ptp_send_packets(int *socket, int len, char val, int port, int speed)
{   
    int raw, rc = 0, ix;
    uint fil_len;
    uint mac_size;
    unsigned char volatile *cptr;
    char iface_type[32];
    V2MsgHeader *ptp_header;    
    uint16_t ether_type;    
    char init_clock[8] = {0x26, 0x33, 0x45, 0x2e, 0x15, 0x28, 0x72, 0x25};

    /* Enable ptp engine */
    en_88e1548_ptp_per_port(port, speed);

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

    if (speed == SPD_10000MBPS) {
        sprintf(iface_type, SEL_PORT_XAUI);
    } else {
        sprintf(iface_type, SEL_PORT_ETH);
    }

    rc = woodlawn_cavium_is_linkup(iface_type, port);
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
    ushort wrval;
    int bus_id, val, rc;
    
    bus_id = get_smi_bus_id(phy_id);

    /* go to page 0 */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        printf("Write to page 0 failed\n");
        return (rc);
    }
    
    rc = woodlawn_phy_reg_rd(bus_id, phy_id, COP_SPEC_CTRL_REG16, &val);
    if (rc == FAILED) {
        printf("Read register %d failed\n", COP_SPEC_CTRL_REG16);
        return (rc);
    }

    /*bit 10 for force link up*/
    if (onoff) {
        wrval = val | SET_PHY_BIT10;  /* enable force link up*/
    } else {
        wrval = val & ~SET_PHY_BIT10;  /* restore force link up*/
    }

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, COP_SPEC_CTRL_REG16, wrval);
    if (rc == FAILED) {
        printf("Write register %d with value %d failed\n", COP_SPEC_CTRL_REG16, wrval);
        return (rc);
    }
    
    sleep(ETH_DRIVER_DELAY);  

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, COP_SPEC_CTRL_REG16, &val);
    if (rc == FAILED) {
        printf("Read register %d failed\n", COP_SPEC_CTRL_REG16);
        return (rc);
    }

    if (val < 0) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/*------------------------------------------------------------------
 *
 * Function: woodlawn_err_clean_up
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
int woodlawn_err_clean_up (int port)
{
    
    char pname[10];

    sprintf(pname, "eth%d", port);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        if (show_status_info(port + ADDR_MEDIA_PHY)) {
            printf("Show status info failed. ");
            return (FAILED);
        }
    }

    /* to prevent the endless message from ethtool setting */
    if (force_linkup(ENABLE, (port + ADDR_MEDIA_PHY))) {
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
    if (force_linkup(DISABLE, (port + ADDR_MEDIA_PHY))) {
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
    ushort result = 0;
    uint speed = 0;

    int bus_id, rc, val;

    bus_id = get_smi_bus_id(port);

    /* go to page 0 */
    rc = woodlawn_phy_reg_wr(bus_id, port, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        cterr('f', 0, "Write to page %d failed", PHY_REG(0));
        return (rc);
    }

    /*advertisment register reg4*/
    rc = woodlawn_phy_reg_rd(bus_id, port, COP_AUTONEG_ADV_REG4, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read original reg %d val failed", COP_AUTONEG_ADV_REG4);
        return (rc);
    }

   if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("advertisement register val = 0x%x\n", val);
   }

    /* go to page 0 */
    rc = woodlawn_phy_reg_wr(bus_id, port, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        cterr('f', 0, "Write to page %d failed", PHY_REG(0));
        return (rc);
    }

    /*status register reg17*/
    rc = woodlawn_phy_reg_rd(bus_id, port, COP_STATUS_REG17, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read original reg %d val failed", COP_STATUS_REG17);
        return (rc);
    }
    
    result = ((val & WOODLAWN_PHY_SPEED_MSK) >> WOODLAWN_PHY_SPEED_OFFSET);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("status register val = 0x%x\n", val);
    }

    switch(result) {
    case WOODLAWN_PHY_SPD_1000:
        speed = SPD_1000MBPS;
        break;
    case WOODLAWN_PHY_SPD_100:
        speed = SPD_100MBPS;
        break;
    case WOODLAWN_PHY_SPD_10:
        speed = SPD_10MBPS;
        break;
    default:
        printf("Unknown value for speed \n");
        break;
    }   

    prpass(testpass, "Current speed is %d", speed);

    result = (val & WOODLAWN_PHY_DUPLEX);
    if (result) {
        prpass(testpass, "Current Duplex is Full Duplex");
    } else {
        prpass(testpass, "Current Duplex is Half Duplex");
    }
   
    result = (val & WOODLAWN_PHY_COP_LINK);
    if (result) {
        prpass(testpass, "Copper Link Up");
    } else { 
        prpass(testpass, "Copper Link Down");
    }
   
    result = (val & WOODLAWN_PHY_GL_LINK_STA);
    if (result) {
        prpass(testpass, "Global Link Status: Copper Is Link Up");
    } else { 
        prpass(testpass, "Global Link Status: Copper Is Link Down");
    }
   
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
    int sk;
    struct ifreq ethreq;
    ushort rdval, wrval, regnum;
    int spdset = 0;;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        cterr('f',0, "%s() Error Creating RX Socket", __FUNCTION__);
        return(FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */

    /* select page 0 or page 1 from signal */
    regnum = PHY_REG(22);
    phy_reg_wr(sk, &ethreq, regnum, signal); 

    /* Read PHY control reg for current speed*/
    /* set speed, Reg [0_2.6, 0_2.13] = value */
    regnum = PHY_REG(0);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);

    switch(speed) {
    case SPD_10MBPS:
        spdset = 0x0000;
        break;
    case SPD_100MBPS:
        spdset = 0x2000;
        break;
    case SPD_1000MBPS:
        spdset = 0x0040;
        break;
    case 0:  /* use the same speed */
        spdset = rdval & 0x2040;
        break;
    }

    /* Set speed bits */
    wrval = rdval & ~0x2040; /* clear bit 6 and 13 (speed)  */
    wrval |= spdset;
    
    /* Set duplex mode */
    if (duplex) {
        wrval |= 0x100; /* full duplex */
    } else {
        wrval &= ~0x100; /* half duplex */
    }

    /* Set autoneg on or off */
    if (autoneg) {
        wrval |= 0x1000; /* enable autoneg */
    } else {
        wrval &= ~0x1000; /* disable autoneg */
    }

    /* Write to the phy and read back immediate to make sure. */
    phy_reg_wr(sk, &ethreq, regnum, wrval);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);

    if (wrval != rdval) {
        cterr('f', 0, "%s(): register setup failed. wrval = 0x%x rdval = 0x%x",
              __FUNCTION__, wrval, rdval);

        sleep(1);
        phy_reg_rd(sk, &ethreq, regnum, &rdval);
        cterr('f', 0, "%s(): register setup failed. wrval = 0x%x rdval = 0x%x",
              __FUNCTION__, wrval, rdval);
        close(sk);
        return (FAILED);
    }

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    if (speed != 0) {

        regnum = PHY_REG(22);
        if(signal) {
            phy_reg_wr(sk, &ethreq, regnum, SIG_COPPER);
	    sleep(ETH_DRIVER_DELAY); /* link down here */
            phy_reg_wr(sk, &ethreq, regnum, SIG_FIBER);
        } else {
            phy_reg_wr(sk, &ethreq, regnum, SIG_FIBER);
	    sleep(ETH_DRIVER_DELAY); /* link down here */
            phy_reg_wr(sk, &ethreq, regnum, SIG_COPPER);
	}
	sleep(ETH_DRIVER_DELAY); /* link up here */ 
    }

    close(sk);

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

    /* Judge to create XAUI raw socket or create ETH row sockt */
    if (speed == SPD_10000MBPS) {
        /* setup XAUI tx and rx socket */
        if (setup_xaui_port(eth_port, &tx_skt) == FAIL) {
            return(FAILED);
        }
    
        if (setup_xaui_port(eth_port, &rx_skt) == FAIL) {
            return(FAILED);
        }
    } else {
        /* setup ETH tx and rx socket */
        if (setup_eth_port(eth_port, &tx_skt) == FAIL) {
            return(FAILED);
        }
    
        if (setup_eth_port(eth_port, &rx_skt) == FAIL) {
            return(FAILED);
        }
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
    if(pthread_create(&threads, NULL, (void *)receive_packets, (diag_info_pthread_t *) &rx_info)) {
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
            rc = ptp_send_packets(&tx_skt, pkt_len, value, eth_port, speed);
            
            if (rc == FAILED) {
                printf("ptp_send_packets failed\n");
                goto exit_tx_rx_diag;
            }
        } else {
            rc = send_packets(&tx_skt, pkt_len, value, eth_port, speed);

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
 * Function: woodlawn_phy_soft_reset
 *  Enable PHY reset, add swtich page flow.
 *
 * Input:  ifname - port type
 *         portnum - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int woodlawn_phy_soft_reset (char *ifname, boolean signal)
{
    int sk;
    struct ifreq ethreq;
    ushort rdval, wrval, regnum;
    int repeat = 100;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket\n", __FUNCTION__);
        return (FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY */
    /* Reset reg 0_0.15=1 */
    /* Use signal to select page for copper or fiber */
    regnum = PHY_REG(22);
    phy_reg_wr(sk, &ethreq, regnum, signal);

    regnum = PHY_REG(0);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    wrval = rdval | SET_PHY_BIT15;
    phy_reg_wr(sk, &ethreq, regnum, wrval);


    /* switch the page is needed*/
    regnum = PHY_REG(22);
    if (signal) {
        phy_reg_wr(sk, &ethreq, regnum, SIG_COPPER);
        sleep(ETH_DRIVER_DELAY); /* link down here */
        phy_reg_wr(sk, &ethreq, regnum, SIG_FIBER);
    } else {
        phy_reg_wr(sk, &ethreq, regnum, SIG_FIBER);
        sleep(ETH_DRIVER_DELAY); /* link down here */
        phy_reg_wr(sk, &ethreq, regnum, SIG_COPPER);
    }
    sleep(ETH_DRIVER_DELAY); /* link up here */ 

    /* Read back to check for reset done */

    do {
        msleep(10);
        phy_reg_rd(sk, &ethreq, PHY_REG(0), &rdval);
    } while((repeat-- > 0) && (rdval & SET_PHY_BIT15));
    
    close(sk);

    if ((repeat == 0) && (rdval & SET_PHY_BIT15)) {
        return (FAILED);
    } else {
        return (PASSED);
    }
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
    int sk;
    struct ifreq ethreq;
    ushort rdval, wrval;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket\n", __FUNCTION__);
        return (FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY */
    /* enable phy stub for external loopback */
    /* set page 6 in reg 18 bit 3 */
    phy_reg_wr(sk, &ethreq, PHY_REG(22), PHY_REG(6));
    phy_reg_rd(sk, &ethreq, CHECKER_CTRL_REG18, &rdval);
    if ((enable) && (!signal)) {
        wrval = rdval | SET_PHY_BIT3;  /*external and copper*/
    } else {
        wrval = rdval & ~SET_PHY_BIT3; /*internal or fiber*/
    } 
    phy_reg_wr(sk, &ethreq, CHECKER_CTRL_REG18, wrval);
    phy_reg_rd(sk, &ethreq, CHECKER_CTRL_REG18, &rdval);

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);  /* can not mask, effect 1000 external lpbk test */

    close(sk);
   
    if(rdval != wrval) {
        printf("%s: reg mismatch rdval = 0x%x, wrval = 0x%x\n", __FUNCTION__,
               rdval, wrval);
        return (FAILED);
    } else {
        return (PASSED);
    }
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
    int sk;
    struct ifreq ethreq;
    ushort rdval, wrval, regnum;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        cterr('f',0, "%s() Error Creating RX Socket", __FUNCTION__);
        return(FAILED);
    }

    /* Note: Using the ifreq data structure was observed how the
     * ethtool implementation was done in the cavium SDK. The ifr.name
     * field is used by the SDK to get the net_device by the interface
     * name (e.g. eth0). See file linux/net/core/dev.c and ethtool.c
     * for how it is being used.
     */
    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY */

    /* Enable phy internal loopback. Reg 0_0.14 = 1 */
    /* go to page 0 for copper and fiber */ 
    regnum = PHY_REG(22);
    phy_reg_wr(sk, &ethreq, regnum, signal);
    regnum = PHY_REG(0);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    
    if (enable){
        wrval = rdval & ~SET_PHY_BIT14; /*external*/
    } else {
        wrval = rdval | SET_PHY_BIT14;  /*internal*/
    } 
    
    phy_reg_wr(sk, &ethreq, regnum, wrval);

    /* switch the page is needed */ 
    /* the page is polled periodically, 
     * switch the page from page 0, 
     * will cause PHY link up/down,
     * then new setup will be detect.
     */  
    regnum = PHY_REG(22);
    if (signal) {
        phy_reg_wr(sk, &ethreq, regnum, SIG_COPPER);
        sleep(ETH_DRIVER_DELAY); /* link down here */
        phy_reg_wr(sk, &ethreq, regnum, SIG_FIBER);
        sleep(ETH_DRIVER_DELAY); /* link up here */ 
    }
    close(sk);

    /* Give time for Linux driver and HW to settle when loopback is set */
    //sleep(ETH_DRIVER_DELAY);  // Mask is ok
    
    return (PASSED);
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
    int sk;
    struct ifreq ethreq;
    ushort rdval, wrval, regnum;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY */
    regnum = PHY_REG(22); /* test fiber disable copper, vice versa. */
    phy_reg_wr(sk, &ethreq, regnum, signal);

    /* Set SGMII fiber ouput amplitude
     */
    if ((signal == SIG_FIBER) && enable) {
        regnum = FIB_SPEC_CTRL_REG2; 
	phy_reg_rd(sk, &ethreq, regnum, &rdval);
        wrval = ((rdval & ~FIB_OUTPUT_AMP_MSK) | FIB_OUTPUT_AMP_VAL504);
	phy_reg_wr(sk, &ethreq, regnum, wrval);
    }

    regnum = PHY_REG(0); // reg 0
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    
    if (enable) {
        wrval = rdval & ~SET_PHY_BIT11; /*power up*/
    } else {
        wrval = rdval | SET_PHY_BIT11;  /*power down*/
    } 
    
    /* we do not need to setup the same value */
    if (wrval == rdval) {
        close(sk);
        return (PASSED);
    }
    
    phy_reg_wr(sk, &ethreq, regnum, wrval);
    
    /* Read back to make sure the write is complete */
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    
    if (rdval != wrval) {
        printf("%s: reg mismatch rdval = 0x%x, wrval = 0x%x\n", __FUNCTION__,
                rdval, wrval);
        close(sk);
        return(FAILED);
    }

    close(sk);
    
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
int init_sgmii_env(char *pname, int speed, int port, int lpbk_mode) {
    
    int rc = 0;
    int autoneg = 0;

    /* Ensure turn off the fiber and turn on copper before test */	
    if ((rc = sig_pwr_ctrl(pname, DISABLE_SIG, SIG_FIBER)) != PASSED){  
    	printf("sig_pwr_ctrl disable fiber failed \n"); 
    	return(FAILED);
    }
    if ((rc = sig_pwr_ctrl(pname, ENABLE_SIG, SIG_COPPER)) != PASSED){  
    	printf("sig_pwr_ctrl enable copper failed \n"); 
    	return(FAILED);
    }

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
     * woodlawn_cavium_is_linkup will return failed,
     * need to verify this one is necessary or not.
     */
    if (lpbk_mode == EXT_LPBK){
        if ((rc = woodlawn_cavium_is_linkup(SEL_PORT_ETH, port)) != PASSED) {
            printf("%s: after init port, sgmii link up time out after 1 second\n",
                    __FUNCTION__);
        }
    }   

    /* to ensure the test stay on full duplex and set speed 
     */
    if(speed == SPD_1000MBPS ) {
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
 * Function: woodlawn_set_packet
 *  Set up packet info for tx and rx using.
 *
 * Input:  port: current test port
 *         speed: current test speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int woodlawn_set_packet(int port, int speed) {

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

        prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                                    port, speed, pkt_cnt, pkt_len, pkt_val);
        fflush(stdout);                          
                          
        
         /* prepare to send packet */
         rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
         if (rc == FAILED) {
             cterr('f', 0, "%s(): tx_rx_diag failed Port: %d Speed: %d",
                  __FUNCTION__, port, speed);
             hkeepflags = orig_hkpflag;
             show_status_info(port + ADDR_MEDIA_PHY);
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
 * Function: sgmii_set_packet
 *  Set up packet info for tx and rx using.
 *
 *
 * Input:  port: current test port   
 *         speed: current test speed   
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sgmii_set_packet (int port, int speed)
{
    
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0, sku_id, front_panel_port;
    uchar orig_hkpflag = hkeepflags;
    
    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;
    
    sku_id = get_sku_id();
    if (sku_id == WOODLAWN_4GE_1XAUI) {
        front_panel_port = one_phy_eth_mapping_ge_num[port];
    } else {
        front_panel_port = two_phy_eth_mapping_ge_num[port];
    }

    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
        /* set packet */
        pkt_cnt = pktdata[typ_curr].send_count;
        pkt_len = pktdata[typ_curr].len;
        pkt_val = pktdata[typ_curr].val;
        hkeepflags |= pktdata[typ_curr].hkpflags;

        prpass(testpass, "Test port-%d, speed-%d, pkt-cnt(%#x), pkt-len(%#x), pkt-val(%#x)",
                                    front_panel_port, speed, pkt_cnt, pkt_len, pkt_val);
        fflush(stdout);                          
                  
        /* To do the tx/rx loopback test */
        rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
       
        if (rc == FAILED) {
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                  __FUNCTION__, front_panel_port, speed);
            hkeepflags = orig_hkpflag;
            show_status_info(port + ADDR_MEDIA_PHY);
            return (FAILED);
        }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", front_panel_port, speed);
    fflush(stdout);
    
    hkeepflags = orig_hkpflag;

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: ptp_sgmii_set_packet
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
int ptp_sgmii_set_packet (int port, int speed)
{
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0, sku_id, front_panel_port;
    uchar orig_hkpflag = hkeepflags;
    
    pkt_type = sizeof(ptp_pktdata)/sizeof(pktdata_info_t);

    hkeepflags = orig_hkpflag;

    sku_id = get_sku_id();
    if (sku_id == WOODLAWN_4GE_1XAUI) {
        front_panel_port = one_phy_eth_mapping_ge_num[port];
    } else {
        front_panel_port = two_phy_eth_mapping_ge_num[port];
    }
    
    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
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
        rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
       
        if (rc == FAILED) {
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                  __FUNCTION__, front_panel_port, speed);
            hkeepflags = orig_hkpflag;
            show_status_info(port + ADDR_MEDIA_PHY);
            return (FAILED);
        }
    } /* typ_curr */

    prpass(testpass, "Pass port %d speed %d", front_panel_port, speed);
    fflush(stdout);
    
    hkeepflags = orig_hkpflag;

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_media_phy_ext_lpbk
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
int set_media_phy_ext_lpbk(char *type, int port, int speed)
{
    char pname[10];
    int rc = 0;
    char cmd_str[32];

    sprintf(pname,"%s%d", type, port);

    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* init sgmii environment and set speed for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, EXT_LPBK)) != PASSED){
        printf("%s(): %s init_sgmii_env failed.\n", __FUNCTION__, pname);
        return (FAILED);
    }

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* 1GMbps external loopback need to setup*/
    if (speed == SPD_1000MBPS) {
        if ((rc = set_phy_stub(pname, EXT_LPBK, SIG_COPPER)) != PASSED){
            printf("%s(): %s set_phy_stub failed.\n", __FUNCTION__, pname);
            return (FAILED);
        }
    }

    /* Per marvell FAE. Make sure bridge phy autoneg is on
     */
    bridge_phy_autoneg_on(port);

    /* woodlawn_phy_soft_reset will turn off Enable loopback reg */
    if ((rc = woodlawn_phy_soft_reset(pname, SIG_COPPER)) != PASSED){
        printf("%s(): %s woodlawn_phy_soft_reset failed\n", __FUNCTION__, pname);
        return (FAILED);
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY * 3);

    /* internal loopback using force_linkup to ensure link stable,
     * external loopback can not use force_linkup, so using check link
     * to ensure the link is stable
     */
    rc = woodlawn_cavium_is_linkup(SEL_PORT_ETH, port);
    if ((rc == FAILED)) {
        printf("%s(): %s sgmii link up time out\n", __FUNCTION__, pname);                                                        
        return FAILED;
    } 

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: media_phy_ext_lpbk_test
 *  This is the entry point for external loopback test only.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int media_phy_ext_lpbk_test(int port, int speed, int lpbkmode) {
      
    int rc = 0; 
    
    /* setup loopback information */          
    rc = set_media_phy_ext_lpbk(SEL_PORT_ETH, port, speed);
                   
    if (rc == FAILED) {
        printf("%s(): sgmii_set_phy_ext_lpbk failed, port: %d\n",
              __FUNCTION__, port);
        return (FAILED);
    }
            
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    if (lpbkmode == PTP_SGMII_EXT_LPBK) {
        rc = ptp_sgmii_set_packet(port, speed);
        if (rc == FAILED) {
            printf("%s(): ptp_sgmii_set_packet failed\n", __FUNCTION__);
            return (FAILED);
        }
    } else {
        rc = sgmii_set_packet(port, speed);
        if (rc == FAILED) {
            printf("%s(): sgmii_set_packet failed\n", __FUNCTION__);
            return (FAILED);
        }
    }

    return (PASSED);
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
    ushort wrval;
    int bus_id, rc, val;

    bus_id = get_smi_bus_id(port);

    /* go to page 18 */
    rc = woodlawn_phy_reg_wr(bus_id, port, PHY_REG(22), PHY_REG(18));
    if (rc == FAILED) {
        cterr('f', 0, "Write to page %d failed", PHY_REG(18));
        return (rc);
    }

    rc = woodlawn_phy_reg_rd(bus_id, port, GENERAL_CTRL2_REG27, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read original reg %d val failed", GENERAL_CTRL2_REG27);
        return (rc);
    }
    
    if (onoff) {
        wrval = val & ~SET_PHY_BIT13;  /*disable*/
    } else {
        wrval = val | SET_PHY_BIT13;  /* restore*/
    }

    rc = woodlawn_phy_reg_wr(bus_id, port, GENERAL_CTRL2_REG27, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", GENERAL_CTRL2_REG27, wrval);
        return (rc);
    }

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: set_automedia
 *   setup internal loopback need to disable automedia detect.
 *   and set QSGMII-to-Copper mode 
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_automedia(boolean onoff, int port)
{
    ushort  wrval;
    int bus_id, rc, val, timeout;

    bus_id = get_smi_bus_id(port);

    /*go to page 18*/
    rc = woodlawn_phy_reg_wr(bus_id, port, PHY_REG(22), PHY_REG(18));
    if (rc == FAILED) {
        cterr('f', 0, "Write to page %d failed", PHY_REG(18));
        return (rc);
    }

    rc = woodlawn_phy_reg_rd(bus_id, port, GENERAL_CTRL1_REG20, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read original reg %d val failed", GENERAL_CTRL1_REG20);    
        return (rc);
    }

    if (onoff) {
        wrval = val & ~SET_AUTO_MEDIA;  /* QSGMII-to-Copper reg[2:0] = 0x000*/
    } else {
        wrval = val | SET_AUTO_MEDIA;  /* QSGMII-to-automedia reg[2:0] = 0x111*/
    }

    rc = woodlawn_phy_reg_wr(bus_id, port, GENERAL_CTRL1_REG20, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", GENERAL_CTRL1_REG20, wrval);    
        return (rc);
    }

    rc = woodlawn_phy_reg_rd(bus_id, port, GENERAL_CTRL1_REG20, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read reg %d val failed", GENERAL_CTRL1_REG20);    
        return (rc);
    }

    wrval = val | SET_PHY_BIT15; /* reset this bit */

    rc = woodlawn_phy_reg_wr(bus_id, port, GENERAL_CTRL1_REG20, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", GENERAL_CTRL1_REG20, wrval);    
        return (rc);
    }
    
    /* Poll reg 20 to make sure reset is done */
    timeout = ETH_DRIVER_POLL_TIMEOUT;
    do {
        woodlawn_phy_reg_rd(bus_id, port, GENERAL_CTRL1_REG20, &val);

        if ((val & SET_PHY_BIT15) == 0) {
            return (PASSED);
        }
        msleep(1);
    } while (timeout--);

    cterr('f', 0, "%s: Reset is never cleared", __FUNCTION__);

    return (FAILED);
}

/*------------------------------------------------------------------
 *
 * Function: set_eng_detect
 *  setup internal loopback need to disable energy detect
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_eng_detect(boolean onoff, int port)
{
    ushort wrval;
    int bus_id, rc, val;

    bus_id = get_smi_bus_id(port);

    /*go to page 0*/
    rc = woodlawn_phy_reg_wr(bus_id, port, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        cterr('f', 0, "Write to page %d failed", PHY_REG(0));
        return (rc);
    }

    rc = woodlawn_phy_reg_rd(bus_id, port, COP_SPEC_CTRL_REG16, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read original reg %d val failed", COP_SPEC_CTRL_REG16);    
        return (rc);
    }

    if (onoff) {
        wrval = val & ~SET_ENG_DETECT;  /*disable energy detect*/
    } else {
        wrval = val | SET_ENG_DETECT;  /* restore energy detect*/
    }

    rc = woodlawn_phy_reg_wr(bus_id, port, COP_SPEC_CTRL_REG16, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", COP_SPEC_CTRL_REG16, wrval);    
        return (rc);
    }

    sleep(ETH_DRIVER_DELAY); /*cannot mask or test failed */

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
    ushort wrval;
    int bus_id, rc, val;
    
    bus_id = get_smi_bus_id(phy_id);

    /*go to page 2*/
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(2));
    if (rc == FAILED) {
        printf("Write to page %d failed\n", PHY_REG(2));
        return (FAILED);
    }
    
    rc = woodlawn_phy_reg_rd(bus_id, phy_id, MAC_SPEC_CTRL2_REG21, &val);
    if (rc == FAILED) {
        printf("Read reg %d val failed\n", MAC_SPEC_CTRL2_REG21);
        return (FAILED);
    }

    val = val & ~0x0007; /* clean up the speed reg[0:2]*/
    switch(speed) {
        case SPD_10MBPS:
            wrval = val | 0x0004;
            break;
        case SPD_100MBPS:
            wrval = val | 0x0005;
            break;
        case SPD_1000MBPS:
            wrval = val | 0x0006;
            break;
        default:  
            printf("phy_id %d not support speed %d on MAC\n", phy_id, speed);    
            break;
    }

    woodlawn_phy_reg_wr(bus_id, phy_id, MAC_SPEC_CTRL2_REG21, wrval);
    if (rc == FAILED) {
        printf("Write reg %d with val %d failed\n", MAC_SPEC_CTRL2_REG21, wrval);
        return (FAILED);
    }
    
    sleep(2*ETH_DRIVER_DELAY); /*can not be mask bridge PHY will failed */

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, MAC_SPEC_CTRL2_REG21, &val);
    if (rc == FAILED) {
        printf("Read reg %d val failed\n", MAC_SPEC_CTRL2_REG21);
        return (FAILED);
    }

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_media_int_lpbk
 * turn on/off these register, we must restore the value
 * when error occured.  
 *  
 * Input:  onoff - turn on/off
 *         phy_id - phy addr for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int set_media_int_lpbk(boolean onoff, int phy_id)
{     
    if (set_macsec(onoff, phy_id)) {
        cterr('f', 0, "Set MacSec to %d on phy_id-%d failed", onoff, phy_id);        
        return (FAILED);
    }
    
    /* the media PHY turn off auto media detect mode */
     if (set_automedia(onoff, phy_id)) {
         cterr('f', 0, "Set Auto-Media on phy_id %d failed", phy_id); 
         return (FAILED);
    }
    
    /* turn off energy detect, to prevent the lpbk stub is not plug-in. */
    if (set_eng_detect(onoff, phy_id)) {
        cterr('f', 0, "Turn off energy detect failed on phy_id %d", phy_id); 
        return (FAILED);
    }
    
     if (force_linkup(onoff, phy_id)) {
         cterr('f', 0, "Force phy_id %d link to %d failed.", phy_id, onoff); 
         return (FAILED);
    }
      
      return (PASSED);
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
    char pname[10];
    int rc = 0;
    int bus_id, phy_id_1548;
    char *cmd_ptr, cmd_str[100];
    int rdval, wrval;
    
    sprintf(pname,"%s%d", type, port);

    phy_id_1548 = qlm_0_4_1548_phy_addr[port];
    bus_id = get_smi_bus_id(phy_id_1548);

    /* Use ethtool to configure driver speed */
    cmd_ptr = cmd_str;
    cmd_ptr += sprintf(cmd_ptr, "ethtool -s %s speed ", pname);

    switch (speed) {
        case SPD_10MBPS:
            cmd_ptr += sprintf(cmd_ptr, "10 ");
            break;
        case SPD_100MBPS:
            cmd_ptr += sprintf(cmd_ptr, "100 ");
            break;
        case SPD_1000MBPS:
            cmd_ptr += sprintf(cmd_ptr, "1000 ");
            break;
    }
    sprintf(cmd_ptr, "duplex full");
    system(cmd_str);

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, INT_LPBK)) != PASSED){
        printf("init_sgmii_env failed\n");
        return (FAILED);
    }

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* turn off stub loopback */
    if ((rc = set_phy_stub(pname, INT_LPBK, SIG_COPPER)) != PASSED){
        printf("set_phy_stub failed\n");
        return (FAILED);
    }
    
    /* Per marvell FAE. Make sure bridge phy autoneg is on
     */
    bridge_phy_autoneg_on(port);
    
    /* set env for media PHY internal loopback */
    set_media_int_lpbk(ENABLE_SIG, phy_id_1548);
    
    /* Note: marvell FAE suggest the sequence up to the end.
     * Please keep the code this way.
     */
    if (set_mac_speed(phy_id_1548, speed)) {
        printf("set mac speed failed.\n");
        return (FAILED);
    }

    /* Reset the copper control reg 0
     */
    woodlawn_phy_reg_wr(bus_id, phy_id_1548, PHY_REG(22), PHY_REG(0));
    woodlawn_phy_reg_rd(bus_id, phy_id_1548, COP_CTRL_REG0, &rdval);

    wrval = rdval | PHY_REG_BIT(15);
    woodlawn_phy_reg_wr(bus_id, phy_id_1548, COP_CTRL_REG0, wrval);

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current
     * reg. mask this part may cause the driver can not detect
     * current setting.
     */
    woodlawn_phy_reg_wr(bus_id, phy_id_1548, PHY_REG(22), SIG_FIBER);
    sleep(ETH_DRIVER_DELAY);
    woodlawn_phy_reg_wr(bus_id, phy_id_1548, PHY_REG(22), SIG_COPPER);
    sleep(ETH_DRIVER_DELAY);

    /* Make sure media phy copper reset bit is 0
     */
    woodlawn_phy_reg_rd(bus_id, phy_id_1548, COP_CTRL_REG0, &rdval);
    if ((rdval & PHY_REG_BIT(15)) == 1) {
        printf("Media PHY reg 0_0:15 reset not cleared\n");
    }

    /* Per FAE instruction, set the copper internal loopback bit
     * in a second write
     */
    woodlawn_phy_reg_rd(bus_id, phy_id_1548, COP_CTRL_REG0, &rdval);

    wrval = rdval | PHY_REG_BIT(14);
    woodlawn_phy_reg_wr(bus_id, phy_id_1548, COP_CTRL_REG0, wrval);

    /* Make sure loopback bit is set
     */
    woodlawn_phy_reg_rd(bus_id, phy_id_1548, COP_CTRL_REG0, &rdval);
    if ((rdval & PHY_REG_BIT(14)) == 0) {
        printf("Media PHY reg 0_0:14 loopback not set\n");
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY * 3);
    
    if ((rc = woodlawn_cavium_is_linkup(SEL_PORT_ETH, port)) != PASSED) {
        printf("sgmii link up time out after 1 second \n");
        show_status_info(phy_id_1548);
        return(FAILED);
    }

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
 * Function: phy_check_iface_up
 *           Wait and check until interface is up with desired speed
 *
 * Input:  pname - Interface name
 *         speed - desired speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int phy_check_iface_up_with_speed (char *iface, int speed)
{
    int timeout = ETH_DRIVER_POLL_TIMEOUT;
    struct ifreq ifr;
    int sock, link_is_up;
    struct ethtool_cmd ecmd;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cterr('f', 0, "%s: Failed to create ioctl socket\n", __FUNCTION__);
        return (FAILED);
    }

    /* Prepare ifreq data into the ioctl socket */
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);

    do {
        ecmd.cmd = ETHTOOL_GSET;
        ecmd.speed = 0;
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
            close(sock);
            cterr('f', 0, "%s: Get Iface flag fails", __FUNCTION__);
            return (FAILED);
        }

        link_is_up = ifr.ifr_flags & IFF_UP;

        ifr.ifr_data = (caddr_t)&ecmd;
        if (ioctl(sock, SIOCETHTOOL, &ifr) == -1) {
            close(sock);
            cterr('f', 0, "%s: Run Eth Tool fails", __FUNCTION__);
            return (FAILED);
        }

        if ((link_is_up) && (ecmd.speed == speed)) {
            close(sock);
            return (PASSED);
        }
        msleep(1);
    } while (timeout--);

    printf("%s: Link is %d, speed is %d\n", __FUNCTION__, link_is_up, ecmd.speed);

    close(sock);

    exit(0);

    return (FAILED);
}


/*------------------------------------------------------------------
 *
 * Function: set_bridge_phy_mode
 *  setup internal PHY mode to SGMII-to-Copper mode and process
 *  mode reset. the setting is for 1340 PHY register.
 *
 * Input:  onoff - turn on/off
 *         phy_id - phy addr for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_bridge_phy_mode(boolean onoff, int phy_id)
{
    ushort wrval;
    int bus_id, rc, val, timeout;

    bus_id = get_smi_bus_id(phy_id);

    /* go to page 6 */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(6));
    if (rc == FAILED) {
        cterr('f', 0, "Set page to %d failed", PHY_REG(6));
        return (FAILED);
    }

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, GENERAL_CTRL_REG20, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read reg %d val failed", GENERAL_CTRL_REG20);
        return (FAILED);
    }

    val = val & ~0x0007; /*clean up reg[2:0]*/
    if (onoff) {
       wrval = val | 0x0001;  /*SGMII to Copper mode */
   } else {
       wrval = val | 0x0005;  /*SGMII to QSGMII mode */
   }

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, GENERAL_CTRL_REG20, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", GENERAL_CTRL_REG20, wrval);
        return (FAILED);
    }

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, GENERAL_CTRL_REG20, &val);
    if (rc == FAILED) {
        cterr('f', 0, "Read reg %d val failed", GENERAL_CTRL_REG20);
        return (FAILED);
    }
    
    wrval = val | SET_PHY_BIT15;  /* reset reg 20 for mode reset */

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, GENERAL_CTRL_REG20, wrval);
    if (rc == FAILED) {
        cterr('f', 0, "Write reg %d with val %d failed", GENERAL_CTRL_REG20, wrval);
        return (FAILED);
    }
    
    /* Poll reg 20 to make sure reset is done */
    timeout = ETH_DRIVER_POLL_TIMEOUT;
    do {
        woodlawn_phy_reg_rd(bus_id, phy_id, GENERAL_CTRL_REG20, &val);

        if ((val & SET_PHY_BIT15) == 0) {
            return (PASSED);
        }
        msleep(1);
    } while (timeout--);

    cterr('f', 0, "%s: Reset is never cleared", __FUNCTION__);
    
    return (FAILED);
}

/*------------------------------------------------------------------
 *
 * Function: direct_phy_soft_reset
 *  soft reset PHY on page0 reg0 and bit 15.
 *
 * Input:  phy_id - phy id for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int direct_phy_soft_reset (int phy_id)
{
    ushort wrval;
    int bus_id, rc, val;
    int timeout;

    bus_id = get_smi_bus_id(phy_id);

    /* go to page 0 */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        printf("Write to page 0 failed\n");
        return (rc);
    }

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, COP_CTRL_REG0, &val);
    if (rc == FAILED) {
        printf("Read original reg %d val failed\n", COP_CTRL_REG0);
        return (rc);
    }
    
    wrval = val | SET_PHY_BIT15;

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, COP_CTRL_REG0, wrval);
    if (rc == FAILED) {
        printf("Write reg %d with val %d failed\n", COP_CTRL_REG0, wrval);
        return (rc);
    }
    
    timeout = ETH_DRIVER_POLL_TIMEOUT;

    do {
        woodlawn_phy_reg_rd(bus_id, phy_id, COP_CTRL_REG0, &val);

        if (!(val & SET_PHY_BIT15)) {
            return (PASSED);
        }
        msleep(1);
    } while (timeout--);

    printf("%s: Reset is never cleared\n", __FUNCTION__);

    sleep(ETH_DRIVER_DELAY);

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: media_phy_int_lpbk_test
 *  This is the entry point for internal loopback test only.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int media_phy_int_lpbk_test(int port, int speed) {
      
    int rc = 0; 
    int phy_id_1548;
    
    phy_id_1548 = qlm_0_4_1548_phy_addr[port];
    
    /* setup loopback information */          
    rc = set_media_phy_int_lpbk(SEL_PORT_ETH, port, speed);
                   
    if (rc == FAILED) {
        printf("Sgmii_set_phy_int_lpbk failed, port - %d, speed - %d\n", port, speed);
        set_media_int_lpbk(DISABLE_SIG, port);
        return (FAILED);
    }
            
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = sgmii_set_packet(port, speed);

    /* Restore the original setting to prevent ext lpbk error occur.*/
    set_media_int_lpbk(DISABLE_SIG, phy_id_1548);
    
    /* wait for driver get the packet then restore the setting */
    sleep(ETH_DRIVER_DELAY);
            
    if (rc == FAILED) {     
       return (FAILED);
    }

    return (PASSED);
}




/*------------------------------------------------------------------
 *
 * Function: sgmii_adv_full_duplex
 *  setup advertised full duplex register for 10 and 100.
 *
 * Input:  onoff - turn on/off
 *         phy_id - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sgmii_adv_full_duplex (boolean onoff, int phy_id)
{
    ushort wrval;
    int bus_id, rc, val;

    bus_id = get_smi_bus_id(phy_id);

    /* go to page 0*/
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        printf("Set page to %d failed\n", PHY_REG(0));
        return (FAILED);
    }

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, COP_AUTONEG_ADV_REG4, &val);
    if (rc == FAILED) {
        printf("Read original reg %d val failed\n", COP_AUTONEG_ADV_REG4);
        return (FAILED);
    }
    
    /* clean up bit7 and bit5 which are advertised half duplex */
    val = val & ~0x00A0;  
    if (onoff) {
        /* Set bit6 and bit8 for adv full duplex */
        wrval = val | 0x0140;  
    } else {
        wrval = val & ~0x0140;  /* clean up value */   
    }

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, COP_AUTONEG_ADV_REG4, wrval);
    if (rc == FAILED) {
        printf("Write reg %d with val %d failed\n", COP_AUTONEG_ADV_REG4, wrval);
        return (FAILED);
    }
    

    rc = woodlawn_phy_reg_rd(bus_id, phy_id, COP_AUTONEG_ADV_REG4, &val);
    if (rc == FAILED) {
        printf("Read reg %d new val failed\n", COP_AUTONEG_ADV_REG4);
        return (FAILED);
    }
   
    /* go to page 0*/
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        cterr('f', 0, "Set page to %d failed", PHY_REG(0));
        return (FAILED);
    }

    
    /* reset PHY to let setting is work */
    rc = woodlawn_phy_reg_rd(bus_id, phy_id, COP_CTRL_REG0, &val);
    if (rc == FAILED) {
        printf("Read original reg %d val failed\n", COP_CTRL_REG0);
        return (FAILED);
    }
   
    wrval = val | SET_PHY_BIT15;

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, COP_CTRL_REG0, wrval);
    if (rc == FAILED) {
        printf("Write reg %d with val %d failed\n", COP_CTRL_REG0, wrval);
        return (FAILED);
    }
    
    sleep(ETH_DRIVER_DELAY);
    
    return (PASSED);
}



/*------------------------------------------------------------------
 *
 * Function: set_bridge_phy_speed
 *  setup internal PHY speed to 100Mbps
 *
 * Input:  onoff - turn on/off
 *         phy_id - phy id for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_bridge_phy_speed (int phy_id, int speed)
{
    ushort wrval;
    int bus_id, rc, val;

    bus_id = get_smi_bus_id(phy_id);

    /* go to page 0*/
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(22), PHY_REG(0));
    if (rc == FAILED) {
        printf("Set page to %d failed\n", PHY_REG(0));
        return (FAILED);
    }
        
    switch(speed) {
        case SPD_10MBPS:
            wrval = 0x8100;  /* set 10 Mbps */ 
            break;
        case SPD_100MBPS:
            wrval = 0xA100;  /* set 100 Mbps */  
            break;
        case SPD_1000MBPS:
            wrval = 0x9140;  /* set 1000 Mbps */ 
            break;
        default:
            printf("phy_id %d not support speed %d on Bridge PHY\n", phy_id, speed);   
            break;
    }

    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(0), wrval);
    if (rc == FAILED) {
        printf("Write reg %d with val %d failed\n", PHY_REG(0), wrval);
        return (FAILED);
    }
        
    rc = woodlawn_phy_reg_rd(bus_id, phy_id, PHY_REG(0), &val);
    if (rc == FAILED) {
        printf("Read reg %d failed\n", PHY_REG(0));
        return (FAILED);
    }

    wrval = val | 0x4000;  /*  turn on loopback */
    rc = woodlawn_phy_reg_wr(bus_id, phy_id, PHY_REG(0), wrval);
    if (rc == FAILED) {
        printf(" Write reg %d wit val %d failed\n", PHY_REG(0), wrval);
        return (FAILED);
    }
           
    sleep(ETH_DRIVER_DELAY);  /* cannot remove, will effect bridge PHY */

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: set_bridge_int_lpbk
 * set internal PHY to internal loopback 
 *
 * Input:  onoff - turn on/off
 *         phy_id - phy_addr for setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int set_bridge_int_lpbk(boolean onoff, int phy_id)
{
    
    /* ensure the advertisement register of bridge PHY 
     * is not turn on half duplex 
     */
    if (sgmii_adv_full_duplex(onoff, phy_id)) {
        printf("sgmii_adv_full_duplex failed. ");
        return (FAILED);
    } 

    /* ensure the bridge PHY is in SGMII to Copper mode. 
     * also reset the bridge PHY to turn off the loopback reg.
     */ 
    if (set_bridge_phy_mode(onoff, phy_id)) {
        printf("set_bridge_phy_mode failed. ");
        return (FAILED);
    }
    
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_bridge_phy_int_lpbk
 *  we setup media PHY first which will let cavium know the current 
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
int set_bridge_phy_int_lpbk(char *type, int port, int speed)
{
    char pname[10];
    int rc = PASSED;
    int phy_id_1340, phy_id_1548;
    int timeout;
    char cmd_str[16];
    
    sprintf(pname,"%s%d", type, port);   

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, INT_LPBK)) != PASSED){
       printf("%s : init_sgmii_env failed\n", pname);
       return (FAILED);
    }

    sprintf(cmd_str, "ifconfig %s down", pname);
    system(cmd_str);
    sprintf(cmd_str, "ifconfig %s up", pname);
    system(cmd_str);

    /* turn off stub loopback to prevent the packet is loop lpbk stub*/
    if ((rc = set_phy_stub(pname, INT_LPBK, SIG_COPPER)) != PASSED){
        printf("%s :  set_phy_stub failed\n", pname);
        return (FAILED);
    }
    
    phy_id_1340 = qlm_0_4_1340_phy_addr[port];
    phy_id_1548 = qlm_0_4_1548_phy_addr[port];
    
     /* without loopback stub, the PHY should forced link up here
      * to let setting of init_sgmii_env is working properly.
      */
    if (force_linkup(ENABLE, phy_id_1548) != PASSED) {   
        printf("%s : force_linkup failed.\n", pname);
        return (FAILED);
    }

    /* ensure the advertisement register is not turn on half duplex */
    if (sgmii_adv_full_duplex(ENABLE, phy_id_1548) != PASSED) {
        printf("%s : sgmii_adv_full_duplex failed\n", pname);
        return FAILED;
    } 
 
    /* set advertisment reg and PHY mode  for bridge PHY 
     */
    if ((rc = set_bridge_int_lpbk(ENABLE_SIG, phy_id_1340)) != PASSED) {
        printf("%s set_bridge_int_lpbk failed\n", pname);
        return (FAILED);
    }   

    if (set_mac_speed(phy_id_1340, speed) != PASSED) {
        printf("%s set mac speed bridge PHY failed.\n", pname);
        return (FAILED);
    }
    
    /* soft reset bridge PHY makes setting of mac speed is work. */
    if (direct_phy_soft_reset(phy_id_1340) != PASSED) {
        printf("%s reset bridge PHY failed.\n", pname);
        return (FAILED);
    }
    
    /* set bridge PHY and also turn on the loopback bit. */
    if (set_bridge_phy_speed(phy_id_1340, speed) != PASSED) {
        printf("%s set bridge PHY speed failed.\n", pname);
        return (FAILED);
    }   
    
    sleep(ETH_DRIVER_DELAY);

    timeout = ETH_DRIVER_POLL_TIMEOUT;

    return (PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: bridge_phy_int_lpbk_test
 *  This is the entry point for bridge PHY internal loopback test.
 *
 * Input:  port - port number 
 *         speed - test speed 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int bridge_phy_int_lpbk_test(int port, int speed) {
      
    int rc = PASSED; 
    int phy_id_1340, phy_id_1548;
    
    /* ensure the cavium is not in loopback mode. */
    set_sgmii_int_lpbk(port, FALSE);
    
    /* setup loopback information */          
    rc = set_bridge_phy_int_lpbk(SEL_PORT_ETH, port, speed);
                   
    if (rc == FAILED) {
        printf("set_bridge_phy_int_lpbk failed, port: %d\n", port);
        set_bridge_int_lpbk(DISABLE_SIG, port);
        return (FAILED);
    }
            
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = sgmii_set_packet(port, speed);

    phy_id_1340 = qlm_0_4_1340_phy_addr[port];
    phy_id_1548 = qlm_0_4_1548_phy_addr[port];
    
    /* Restore the original setting to prevent ext lpbk error occur.*/
    set_bridge_int_lpbk(DISABLE_SIG, phy_id_1340);
    
    if(force_linkup(DISABLE, phy_id_1548)) {
        printf("force_linkup failed.\n");
        return (FAILED);
    }   

    if (rc == FAILED) {
        printf("sgmii_set_packet failed %s\n",__FUNCTION__);
        return (FAILED);
    }
    
    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: setup_cavium_int_lpbk
 * turn on/off for Cavium internal loopback test
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int setup_cavium_int_lpbk(int port, boolean onoff)
{
    int phy_id_1548;

    phy_id_1548 = qlm_0_4_1548_phy_addr[port];
    
    /* force link up can fool cavium that it is link up, 
     * even without plug-in cable or loopback port.
     */

    if(force_linkup(onoff, phy_id_1548)) {
        cterr('f', 0, "port %d force_linkup failed.", port);
        return (FAILED);
    }

    /* setup cavium into loopback mode */
    set_sgmii_int_lpbk(port, onoff);
      
    /* disable GMX enable reg to keep cavium GMX stay in correct status*/
    set_gmxeno(port, onoff);

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_cavium_int_lpbk
 *  initial and setup loopback type on Cavium.
 *
 * Input:  type - port type
 *         port - port number
 *         lpbk_typ - internal or external
 *
 * Output: PASSED/FAILED
 * 
 *------------------------------------------------------------------
 */
int set_cavium_int_lpbk(char *type, int port, int speed)
{
    char pname[10];
    int rc = 0;
    
    sprintf(pname,"%s%d", type, port); 
    
    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, INT_LPBK)) != PASSED){
        cterr('f', 0, "init_sgmii_env failed");
        return (FAILED);
    }

    /* setup cavium status and speed */
    if ((rc = sgmii_port_cfg(port, speed, AUTONEG_ON)) != PASSED){
       cterr('f', 0, "sgmii port cfg failed");
       return(FAILED);
    }

    if ((rc = setup_cavium_int_lpbk(port, TRUE)) != PASSED){
       cterr('f', 0, "setup cavium internal loopback failed");
       return(FAILED);
    }

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: cavium_int_lpbk_test
 *  This is the entry point for cavium internal loopback test.
 *
 * Input:  port - port number 
 *         speed - test speed 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int cavium_int_lpbk_test(int port, int speed)
{
    int rc = PASSED, retval = 0;
          
    rc = set_cavium_int_lpbk(SEL_PORT_ETH, port, speed);
    if (rc == FAILED) {
       cterr('f', 0, "set_cavium_int_lpbk failed, port: %d", port);
        goto cavium_int_lpbk_exit;
    }
    
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = woodlawn_set_packet(port, speed);
    if (rc == FAILED) {
       cterr('f', 0, "Cavium port %d int loopback speed %d failed", port, speed);
        goto cavium_int_lpbk_exit;
    }

cavium_int_lpbk_exit:
    /*restore the setting */
    if ((retval = setup_cavium_int_lpbk(port, FALSE)) != PASSED){
       cterr('f', 0, "setup cavium internal loopback failed");
       return (FAILED);
    }

   return (rc);

}


/*------------------------------------------------------------------
 *
 * Function: woodlawn_phy_lpbk_test
 *
 * Description: SGMII port PHY internal or external loopback test
 *              internal lpbk test: cavium->bridge PHY->media PHY
 * 
 * Input: lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 *
 * Return: pass/fail
 *
 * Note: Accroding the TE from Marvell:
 * "The PHY internal loopback is available on all speeds. 
 * It's just that Cavium polling PHY status registers
 * (I don't know which ones) is causing Cavium to not configure properly.
 * That's why for all internal loopbacks, I tried to use Forced 100 to fool Cavium,
 * as I cannot force 1000 on PHY as required by IEEE spec.
 * For external loopback, 1000 is doable, because once PHY stub
 * loopback register bit is enabled, Cavium is fooled for some reason 
 * and so I don't need to force 100."
 * Thus, we skip the 1000Mpbs on internal loopback test.
 *------------------------------------------------------------------
 */
int woodlawn_phy_lpbk_test (int phy, int lpbkmode)
{
    int rc = 0, ix; 
    int retval = PASSED; 
    int try, retry_limit = 2;
    int port_cnt, port_curr, port;
    int speed_cnt, speed_curr, speed, id;
    int *sku_eth_mapping_ge_num;
    int four_ge_eth_port[] = {0x4, 0x5, 0x6, 0x7};
    int six_ge_eth_port[] = {0x0, 0x1, 0x4, 0x5, 0x6, 0x7};

    /* Get the SKU id */
    id = get_sku_id();
    
    if (phy == MRVL_1548_PHY0) {
        port_cnt = 4;
    } else {
        /* New SKU just have 4 GE ports */
        if (id == WOODLAWN_4GE_1XAUI) {
            return (PASSED);
        } else {
            port_cnt = 2;  
        }
    }

    /* Not official SKU has different eth number copper port mapping compared
     * with new official SKU
     */
    if (id == WOODLAWN_6GE_1XAUI) {
        sku_eth_mapping_ge_num = old_eth_mapping_ge_num;
    } else if (id == WOODLAWN_6GE){
        sku_eth_mapping_ge_num = two_phy_eth_mapping_ge_num;
    } else if (id == WOODLAWN_4GE_1XAUI) {
        sku_eth_mapping_ge_num = one_phy_eth_mapping_ge_num;
    }

    /* get test envrionment variable */
    if (lpbkmode == PTP_SGMII_EXT_LPBK) {
        speed_cnt = sizeof(ptp_eth_speed_list) / sizeof(int);
    } else {
        speed_cnt = sizeof(eth_speed_list) / sizeof(int);
    }    

    if (lpbkmode == PTP_SGMII_EXT_LPBK) {
        if (id == WOODLAWN_6GE) {
            for (ix = 0; ix <= 5; ix++) {
                enable_88e1548_ptp_engine(six_ge_eth_port[ix]);
            }
        } else {
            for (ix = 0; ix <= 3; ix++) {
                enable_88e1548_ptp_engine(four_ge_eth_port[ix]);
            }
        }
    }

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        if (phy == MRVL_1548_PHY0) {
            port = eth_qlm4_list[port_curr];
        } else {
            port = eth_qlm0_list[port_curr];
        }


        for (speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            if (lpbkmode == PTP_SGMII_EXT_LPBK) {
                speed = ptp_eth_speed_list[speed_curr];
            } else {
                speed = eth_speed_list[speed_curr];
            }
        
            /* skip 1000Mbps on all the internal loopback test*/
            if(((lpbkmode != SGMII_EXT_LPBK) && (lpbkmode != SGMII_INT_EXT_LPBK))
                && (lpbkmode != PTP_SGMII_EXT_LPBK) && (speed == SPD_1000MBPS)) {
                continue;
            }
    
            switch(lpbkmode) {
            case CAVIUM_INT_LPBK:
                testname("Cavium internal loopback");
                prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                       speed);
                rc = cavium_int_lpbk_test(port, speed);
                if (rc == FAILED) {
                    woodlawn_err_clean_up(port);
                    cterr('f',0,"Cavium GE%d int loopback speed %d failed",
                          sku_eth_mapping_ge_num[port], speed);
                    retval = FAILED;
                }
            break;

            case BRIDGE_PHY_INT_LPBK:
                testname("88E1340 PHY-%d Loopback", phy);
                prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                       speed);
                rc = bridge_phy_int_lpbk_test(port, speed);
                if (rc == FAILED) {
                    woodlawn_err_clean_up(port);
                    cterr('f',0,"SGMII bridge PHY int loopback GE port %d failed",
                          sku_eth_mapping_ge_num[port]);
                    retval = FAILED;
                }
            break;

            case MEDIA_PHY_INT_LPBK:
                testname("88E1548 PHY-%d Loopback", phy);
                prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                       speed);
                rc = media_phy_int_lpbk_test(port, speed);
                if (rc == FAILED) {
                    woodlawn_err_clean_up(port);
                    cterr('f',0,"SGMII bridge PHY int loopback GE port %d failed",
                          sku_eth_mapping_ge_num[port]);
                }
            break;

            case SGMII_EXT_LPBK:
                testname("GE%d External Loopback", sku_eth_mapping_ge_num[port]);
                prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                       speed);
                rc = media_phy_ext_lpbk_test(port, speed, SGMII_EXT_LPBK);
                woodlawn_err_clean_up(port);
                if (rc == FAILED) {
                    cterr('f', 0, "SGMII external PHY ext loopback GE port %d failed",
                          sku_eth_mapping_ge_num[port]);
                    return (FAILED);
                }
            break;

            case SGMII_INT_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */

                if (check_ext_lpbk_flag()) {
                    testname("GE%d External Loopback", sku_eth_mapping_ge_num[port]);
                    prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                           speed);

                    for (try = 0; try < retry_limit; try++) {
                        rc = media_phy_ext_lpbk_test(port, speed, SGMII_INT_EXT_LPBK);

                        if ((rc == PASSED) || (try == (retry_limit - 1))) {
                            break;
                        } else {
                            printf("####### retry the test #########\n");
                            reset_quad_phy();
                            diag_88e1340_init();
                            diag_88e1548_init();
                        }
                    }

                    if (rc != PASSED) {
                        cterr('f', 0, "SGMII external PHY ext loopback GE port %d failed",
                              sku_eth_mapping_ge_num[port]);
                        retval = FAILED;
                    }
                }
                if (((!check_ext_lpbk_flag()) || (rc != PASSED)) &&
                     (speed != SPD_1000MBPS)) {
                    testname("88E1548 PHY-%d Loopback", phy);
                    prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                           speed);
                    rc = media_phy_int_lpbk_test(port, speed);

                    for (try = 0; try < retry_limit; try++) {
                        rc = media_phy_int_lpbk_test(port, speed);

                        if ((rc == PASSED) || (try == (retry_limit - 1))) {
                            break;
                        } else {
                            printf("####### retry the test #########\n");
                            reset_quad_phy();
                            diag_88e1340_init();
                        }
                    }
                }

            break;

            case PTP_SGMII_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */

                if (check_ext_lpbk_flag()) {
                    testname("GE%d External Loopback", sku_eth_mapping_ge_num[port]);
                    prpass(testpass, "Test GE%d speed-%d", sku_eth_mapping_ge_num[port],
                           speed);

                    for (try = 0; try < retry_limit; try++) {
                        rc = media_phy_ext_lpbk_test(port, speed, PTP_SGMII_EXT_LPBK);

                        if ((rc == PASSED) || (try == (retry_limit - 1))) {
                            break;
                        } else {
                            printf("####### PTP lpbk retry the test #########\n");
                            reset_quad_phy();
                            diag_88e1340_init();
                            diag_88e1548_init();
                            if (lpbkmode == PTP_SGMII_EXT_LPBK) {
                                if (id == WOODLAWN_6GE) {
                                    for (ix = 0; ix <= 5; ix++) {
                                        enable_88e1548_ptp_engine(six_ge_eth_port[ix]);
                                    }
                                } else {
                                    for (ix = 0; ix <= 3; ix++) {
                                        enable_88e1548_ptp_engine(four_ge_eth_port[ix]);
                                    }
                                }
                           }
                        }
                    }

                    if (rc != PASSED) {
                        cterr('f', 0, "PTP loopback GE port %d failed",
                              sku_eth_mapping_ge_num[port]);
                        retval = FAILED;
                    }
                } else { 
                    cterr('f', 0, "External loopback flag is not turn on");
                }

            break;

            default:
                retval = FAILED;
                cterr('f',0," Woodlawn not support this loopback mode");
                break;
            }
        } /*speed*/
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


/**********************************************************************
 *
 * Function: woodlawn_phy_lpbk_util
 *
 * Description:
 * Utility to execute SGMII single port internal or external loopback
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int
woodlawn_phy_lpbk_util(void)
{
    int port, low_port, rc = 0;
    int speed, spdsel;
    int lpbkmode, lpbksel;
    char qrybuf[64];
    
    low_port = SGMII0;

    printf("\nSelect loopback 0:CAVIUM_INT_LPBK    1:BRIDGE_PHY_INT_LPBK");
    printf("\n                2:MEDIA_PHY_INT_LPBK 3:SGMII_EXT_LPBK");
    printf("\n                4:SFP_EXT_LPBK");
    lpbksel = getdec_answer("\nEnter ", 0, 0, 4);

    sprintf(qrybuf, "\nEnter port number (%d - %d)", low_port, PLAT_SGMII_NUM_MAX);
    port = getdec_answer(qrybuf, low_port, low_port, PLAT_SGMII_NUM_MAX);
 
    if(lpbksel == SGMII_EXT_LPBK){
        lpbkmode = LOOP_EXT;
    } else if (lpbksel ==  SFP_EXT_LPBK) {
        testname("SFP PHY external loopback");
        prpass(testpass, "Test SFP-%d, ", port);
        return(sfp_ext_lpbk_test_util(port));
    } else { /*internal loopback */
        lpbkmode = LOOP_INT;
    }

    if (lpbkmode == LOOP_INT) {
      sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS)");
      spdsel = getdec_answer(qrybuf, 0, 0, 1);
      speed = (spdsel == 0) ? SPD_10MBPS : SPD_100MBPS;
        
      switch(lpbksel) {
    case CAVIUM_INT_LPBK:
          testname("Cavium internal loopback");
           prpass(testpass, "Test SGMII-%d with speed-%d, ", port, speed);
        rc = cavium_int_lpbk_test(port, speed);
      break;    
    case BRIDGE_PHY_INT_LPBK:
        testname("Bridge PHY internal loopback");
        prpass(testpass, "Test SGMII-%d with speed-%d, ", port, speed);
        rc = bridge_phy_int_lpbk_test(port, speed);
      break;
    case MEDIA_PHY_INT_LPBK:
        testname("Media PHY internal loopback");
        prpass(testpass, "Test SGMII-%d with speed-%d, ", port, speed);
        rc = media_phy_int_lpbk_test(port, speed);
      break;
    default:
          printf("\n not support this loopback test. ");
        break;
      }
    } else {
        sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS, 2: 1000MBS)");
      spdsel = getdec_answer(qrybuf, 0, 0, 2);
      switch(spdsel) {
        case 0:
            speed = SPD_10MBPS; break;
        case 1:
            speed = SPD_100MBPS; break;
        case 2: 
            speed = SPD_1000MBPS; break;
        default:
            printf("\n not support this speed. ");
            break;
      }
      testname("Media PHY external loopback");
      prpass(testpass, "Test SGMII-%d with speed-%d, ", port, speed);
      rc = media_phy_ext_lpbk_test(port, speed, SGMII_INT_EXT_LPBK);
    }
      
      if (rc == FAILED) {
          woodlawn_err_clean_up(port);
          cterr('f',0,"Loopback test failed on port %d speed %d \n", port, speed);        
      }


    return (rc);
}


void write_txrx_pkt(void){
    
    FILE *fd_record1, *fd_record2, *fd_record3; 
    int ix; 
  
    fd_record1 = fopen("packet_tx.log", "w");
    fd_record2 = fopen("packet_rx_1st.log", "w");
    fd_record3 = fopen("packet_rx_2nd.log", "w");
    if (fd_record1 == NULL) {
        cterr('f',0,"open packet_tx.log failed. \n");
    //    return FAILED;
    }
    if (fd_record2 == NULL) {
        cterr('f',0,"open packet_rx_1st.log failed. \n");
    //    return FAILED;
    }
    if (fd_record3 == NULL) {
        cterr('f',0,"open packet_rx_2nd.log failed. \n");
    //    return FAILED;
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
    if (setup_eth_port(2, &raw) == FAIL) {
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
        if ((chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) &&
            (chk_macaddr(&rx_pkt_buf[6], (uchar *)mac_sa) != 0)) {
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
        if ((chk_macaddr(&rx_pkt_sec[0], (uchar *)mac_da) != 0) &&
            (chk_macaddr(&rx_pkt_sec[6], (uchar *)mac_sa) != 0)) {
            memset((unsigned char *)rx_packet_sec, 0, ETH_PKT_MAX_LEN); /* clean up buffer */
            printf("\n detected non diag packet. Ignore.\n");
            pkt_cnt = 0; /* restore the packet count */
            continue; /* not firmware download packet */
        }

        pkt_cnt++;
        temp = (pkt_cnt %4);
        print_statistic_tofile(temp, pkt_cnt, rx_sec );

        woodlawn_packet_count++;
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
Revision 1.2  2013/10/08 08:48:30  tirawan
Woodlawn collapsed to main trunk

Revision 1.1.4.3  2013/09/05 06:19:25  leschen
Modify PTP check packet function and fix lpbk return failed mechanism

Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
Branch into woodlawn-branch2 and port woodlawn code

Revision 1.1.2.2  2013/08/06 09:25:35  leschen
Add PTP related functions and display front panel port number.

Revision 1.1.2.1  2013/04/24 10:37:24  tirawan
Initial check-in for woodlawn linux code

Revision 1.11  2013/03/27 08:45:05  kuangik
Code cleanup

Revision 1.9  2013/03/19 09:51:24  kuangik
Add retry mechanism (ported from O2) and reset quad phy if the test fails

Revision 1.3  2013/03/14 10:59:03  kuangik
Optimize loopback test

Revision 1.2  2013/03/13 12:31:03  kuangik
Improve loopback test to shorten the time

Revision 1.18  2013/03/07 13:57:29  leslie
Judge to create XAUI raw socket or ETH raw socket.

Revision 1.17  2013/03/07 02:24:04  kuangik
Add Show error count

Revision 1.14  2013/02/26 07:11:25  leslie
Fix and clean up lpbk code.

Revision 1.13  2013/02/18 08:17:27  leslie
Fix code to use woodlawn phy reg r/w lib.

Revision 1.12  2013/02/18 06:47:11  kody
Modify for the port mapping changed according to the new SKUs.

Revision 1.11  2013/01/18 07:20:24  leslie
Fix and clean up code.

Revision 1.10  2013/01/16 00:59:46  leslie
Add

Revision 1.9  2012/10/08 09:58:43  leslie
Fix the loopback test to use GE port.

Revision 1.8  2012/09/21 11:52:11  kody
Clean up the code.

Revision 1.7  2012/09/19 09:57:52  leslie
Fix the internal loopback test for woodlawn.

Revision 1.6  2012/09/05 22:57:40  kody
Fix the external loopback test for woodlawn.

Revision 1.5  2012/08/03 10:16:56  evanli
Mapping to latest O2 source code on 20120726

Revision 1.3  2012/05/18 10:25:08  kody
Fix the type warning during compile.

Revision 1.2  2012/04/06 06:07:45  kuangik
Update for GE PHY Test

Revision 1.1.1.1  2012/02/10 05:59:50  kody
Initial imports Woodlawn project code base.

$Endlog$
*/
