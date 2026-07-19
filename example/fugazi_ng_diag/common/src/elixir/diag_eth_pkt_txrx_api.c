 /* $Id: diag_eth_pkt_txrx_api.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_eth_pkt_txrx_api.c,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx_api.c
 * Description:Application for packets tx/rx
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /* getpid */
#include <strings.h>  /* for bzero*/
#include <string.h>
#include <errno.h>
#include <sys/types.h> /* getpid */
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h> /*struct ethtool */
#include <linux/sockios.h> /* SIOCETHTOOL */
#include <pthread.h>
#include <ifaddrs.h> /* for using getifaddrs */ 
#include <netinet/in.h>
#include "common.h"
#include "nvmonvars.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_utils.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_info.h"
#include "plat_defs.h"

static int create_raw_socket(int);
static int bind_socket(char *, int, int);
static int macstr2macaddr(char *, mac_addr_t *);

/* This MAC address array is for cavecreek sgmii 1-3
 * These values in the array will be replaced by the
 * real value read from the system
 */
mac_addr_t local_mac_addr[] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* eth0 not used internally */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x20, 0x20, 0x20, 0x20, 0x20, 0x20},
    {0x30, 0x30, 0x30, 0x30, 0x30, 0x30},
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
};



/***********************************************************************
 * Function: system_mac_addr_get
 *   based on sys log/intf name to get mac address
 *
 * Input:
 *   *name - interface name 
 *   *mac_buf - pointer to mac buf
 *
 * Return: NONE
 ***********************************************************************
 */
void system_mac_addr_get (char *name, mac_addr_t *mac_buf)
{
    char file_name[70];
    FILE *stream_p;
    char macstr[] ="00:01:02:03:04:05";

    sprintf(file_name, "/sys/class/net/%s/address", name);

    stream_p = fopen(file_name, "r");
    if (stream_p == NULL) {
        printf("can't open %s\n", file_name);
        exit(-1); /* software error so exit */
    }

    fscanf(stream_p, "%s", macstr);
    fclose(stream_p);

    if (macstr2macaddr(macstr, mac_buf) != PASSED) {
        printf("Failed to transfer mac address from string to hex num\n");
    }
}

/***********************************************************************
 * Function: macstr2macaddr
 * 
 * Convert a string of MAC address "xx:xx:xx:xx:xx:xx" to 
 * 6 byte uchar numbers
 *
 * Input:
 *   *macstr - point to mac string 
 *   *mac_buf - pointer to mac buf
 *
 * Return: PASSED/FAILED
 ***********************************************************************
 */
static int macstr2macaddr (char *macstr, mac_addr_t *mac_buf)
{
    char tmp_mac[6];
    char *cptr, tmpstr[4];
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
        return (PASSED);
    } else {
        return (FAILED);
    }
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
static int create_raw_socket (int protocol)
{
    int rawsock;
    if((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol)))== -1) {
        printf("Error creating raw socket: ");
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
 * Return: 0 or -1
 ***********************************************************************
 */
static int bind_socket (char *device, int rawsock, int protocol) {
    struct sockaddr_ll sll;
    struct ifreq ifr;

    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));

    /* First Get the Interface Index  */

    sprintf((char *)ifr.ifr_name, device);
    if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        printf("Error getting Interface index !\n");
        return(-1);
    }

    /* Bind our raw socket to this interface */

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll))) == -1) {
        printf("Error binding raw socket to interface\n");
        return(-1);
    }

    return 0;
}


/***********************************************************************
 * Function: set_promisc
 *   Set the ethernet interface to promisc mode
 *
 * Input:
 *   device - Name of ethernet interface such as "eth0", eth1, etc.
 *   sock - socket ID
 *
 * Return: 0 or -1
 ***********************************************************************
 */
int set_promisc (char *device, int sock) {

    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* First Get the Interface Index  */
    /* Set the network card in promiscuos mode */
    sprintf(ifr.ifr_name, device);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        printf("ioctl: SIOCSIFFLAGS get index interface");
        close(sock);
        return(-1);
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        printf("ioctl: SIOCSIFFLAGS to set promiscous mode");
        close(sock);
        return(-1);
    }

    return 0;
}

/***********************************************************************
 * Function: clear_promisc
 *   Clear the ethernet interface from promisc mode
 *
 * Input:
 *   device - Name of ethernet interface such as "eth0", eth1, etc.
 *   sock - socket ID
 *
 * Return: 0 or -1
 ***********************************************************************
 */
int clear_promisc (char *device, int sock) {

    struct ifreq ifr;

    bzero(&ifr, sizeof(ifr));

    /* First Get the Interface Index  */
    /* Set the network card in promiscuos mode */
    sprintf(ifr.ifr_name, device);
    if (ioctl(sock,SIOCGIFFLAGS,&ifr) == -1) {
        printf("ioctl: SIOCSIFFLAGS get interface index");
        close(sock);
        return(-1);
    }

    ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr) == -1) {
        printf("ioctl: SIOCSIFFLAGS to clear promiscous mode");
        close(sock);
        return(-1);
    }

    return 0;
}

/***********************************************************************
 *
 * Function:    setup_eth_dev()
 *
 * Description: Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:       eth_name - eth interface name.
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:      PASS/FAIL
 *
 ************************************************************************
 */
int setup_eth_dev (char *eth_name, int *socket)
{
    int raw;

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        printf("%s create_raw_socket failed\n",__FUNCTION__);
        return (FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        printf("%s bind_socket() failed at intf %s\n",__FUNCTION__, eth_name);
        return (FAIL);
    }

    *socket = raw;
    return PASS;
}

/***********************************************************************
 *
 * Function:    cleanup_eth_dev
 *
 * Description: Cleanup the Linux ethernet packet socket to prevent
 * either TX and RX.
 *
 * Input:       if_name - eth interface name.
 *              socket - The socket created in setup_eth_dev.
 *
 * Output:      PASS/FAIL
 *
 ************************************************************************
 */
int cleanup_eth_dev (char *if_name, int socket)
{

    if (close(socket) == -1) {
        return (FAIL);
    }

    return (PASS);
}

/***********************************************************************
 * Function: gen_eth_pkt
 *   Generate an ethernet packet with the provided parameters
 *
 * Input:
 *   macda - mac da of the packet
 *   macsa - mac sa of the packet
 *   pkt_type - packet type field of the packet. e.g. 0x0800, 0x8100
 *   seed - the first byte of the payload
 *   inc_dec - the increment or decremnet value to the seed
 *   payload_len - the length of the payload
 *   buf_p - the pointer to buffer storing the packet
 *
 * Return: void
 ***********************************************************************
 */
void gen_eth_pkt (mac_addr_t macda, mac_addr_t macsa, ushort pkt_type,
                 unsigned char seed, char inc_dec, int payload_len,
                 unsigned char *buf_p)
{
    int ii;
    unsigned char pat = seed;

    /* Build the packet
     */
    memcpy(buf_p, macda, 6);
    buf_p += 6;
    memcpy(buf_p, macsa, 6);
    buf_p += 6;
    *buf_p++ = (pkt_type >> 8) & 0xff;
    *buf_p++ = pkt_type & 0xff;

    for (ii=0; ii < payload_len; ii++) {
        *buf_p++ = pat;
        pat += inc_dec;
    }
}

/***********************************************************************
 * Function: tx_a_pkt
 *   Transmit a packet through the socket. The size is limited to 1514 bytes.
 *
 * Input:
 *   socket - socket ID
 *   pkt - the packet string
 *   pkt_len - The length of the packet
 *
 * Return: bytes being sent or -1 for error
 ***********************************************************************
 */
int tx_a_pkt (int socket, unsigned char *pkt, int pkt_len)
{
    int rv;

    if (pkt_deb_flag) {
        printf("%s The tx packet is:\n", __FUNCTION__);
        display_pkt(pkt, pkt_len);
    }

    /* Note: the socket buffer is limited to 1514 bytes
     */

    rv = write(socket, pkt, pkt_len);

    if (rv != pkt_len) {
        printf("\nsending %d bytes\n", pkt_len);
        printf("write command failed.");
    }
    return(rv);
}

/***********************************************************************
 * Function: rx_a_pkt
 *   Receive a packet through the socket.
 *
 * Input:
 *   socket - socket -ID
 *   buf_p - receive buffer
 *   buf_size - receive buffer size
 *
 * Return: number of bytes being read or -1 for error
 ***********************************************************************
 */
int rx_a_pkt (int socket, unsigned char *buf_p, int buf_size)
{
    int rv;

    rv = read(socket, buf_p, buf_size);
    if (rv < 0) {
        printf("read packet failed.");
    }

    return(rv);
}


#if 0 /* for future use */
/*
 ************************************************************************
 *
 * Function: eth_is_linkup
 *
 * Description : Chenk link up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 ************************************************************************
 */
int eth_is_linkup(int port){

    int timeout_counter = 100, is_link = FALSE, wait_time = 20;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];

    sprintf(pname,"eth%d", port);

    while(TRUE) {

        /* Get the interface information */
        if (getifaddrs(&if_list) < 0) {
            printf("Failed to get interface information: %s.\n",
            strerror(errno));
            freeifaddrs(if_list);
            return(FAIL);
        }
        if (if_list == NULL) {
            printf("No network interfaces were found.\n");
            freeifaddrs(if_list);
            return(FAIL);
        }

        for (if_info = if_list; if_info; if_info = if_info->ifa_next) {

            /* parse the port name */
            if (strncmp(if_info->ifa_name, pname, IFNAMSIZ) != 0)
                continue;

            flags = if_info->ifa_flags;
            if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
                fflush(stdout);
                is_link = TRUE;
                break;
            } else {
                usleep(wait_time*1000);
                timeout_counter--;
                if (timeout_counter == 0) {
                    printf("Network interface link up time out! \n");
                    freeifaddrs(if_list);
                    return(FAIL);
                }
            }
            fflush(stdout);
        } /* End of for loop */

        freeifaddrs(if_list);
        if (is_link == TRUE)
            break;
    } /* End of while loop */

    return(PASS);
}
#endif  /* for future use */
/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx_api.c,v $
Revision 1.2  2021/09/24 01:21:06  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.1.2.1  2020/11/05 06:34:55  harrchan
1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
2.Remove some debug message on AC5 init process

Revision 1.2  2019/12/11 10:10:29  lucywang
Merged Nanook to main trunk


$Endlog$
*/

