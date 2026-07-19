 /* $Id: diag_eth_pkt_txrx.c,v 1.1 2020/08/19 09:50:04 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/diag_eth_pkt_txrx.c,v $
 *-----------------------------------------------------------------------------
 * File: diag_eth_pkt_txrx.c
 * Description:
 * As a general c file:
 *     Transfer packet to specific eth port.
 * As a standalong c application:
 * To compile :
 *    #  gcc -o eth_pkt_txrx -Wall -lpthread -DLINUX_APP eth_pkt_txrx.c eth_pkt_txrx_utils.c
 *    #  gcc -o eth_pkt_txrx -Wall -lpthread -DLINUX_APP eth_pkt_txrx.c eth_pkt_txrx_utils.c eth_pkt_txrx_api.c
 *    -o          : output file as following filename.
 *    -Wall       : enables all compiler's warning messages
 *    -lpthread   : include pthread library
 *    -DLINUX_APP : define LINUX_APP to build as standalone app.
 * Execute :
 *    #  ./eth_pkt_txrx -p [eth port num] -c [pkt count]
 * Aug 2015, Alan Peng
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
#include <linux/filter.h>  /* pkt filter */
#include "common.h"
#include "diag_eth_pkt_txrx.h"
#include "types.h"
#include "nvmonvars.h"

int eth_pkt_txrx (char *, int, int);
unsigned int crc32(unsigned int, unsigned char *, int);
unsigned int swap32(unsigned int);
int tx_a_pkt(int, unsigned char *, int);
int rx_a_pkt(int, unsigned char *, int);
void display_pkt(unsigned char *, int);
int chk_macaddr(unsigned char *, unsigned char *);
int setup_eth_dev(char *, int *);
static int bind_socket(char *, int, int);
static int create_raw_socket(int);
void gen_eth_pkt (mac_addr_t, mac_addr_t, ushort,
                 unsigned char, char, int,
                 unsigned char *);
int cleanup_eth_dev(char *, int);
void system_mac_addr_get(char *, mac_addr_t *);
static int macstr2macaddr(char *, mac_addr_t *);

/* default mac address */
static mac_addr_t mac_da = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#ifdef STATIC_MAC
static mac_addr_t mac_sa = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
#else
static mac_addr_t mac_sa;
#endif

static volatile int     rx_ready = 0;

extern char *optarg;

static int set_src_mac_filt(int *, char *); 

/* tcpdump ether proto 0x8ff0 -dd*/
struct sock_filter code[] = {
    { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 1, 0x00008ff0 },
    { 0x6, 0, 0, 0x0000ffff },
    { 0x6, 0, 0, 0x00000000 },
};
struct sock_fprog bpf = {
    .len = 4,
    .filter = code,
};

/*
 ************************************************************************
 * Function: swap32
 * 32 bit value swap.
 *
 * Input: i - for data
 *
 * Return: swapped data
 ************************************************************************
 */
unsigned int swap32 (unsigned int ia)
{
    ia = (ia << 16) | (ia >> 16);

    return (ia & 0xff00ffff) >> 8 | (ia & 0xffff00ff) << 8;
}

/*
 ************************************************************************
 * Function: crc32
 * Ethernet packet 4 byte CRC calculation
 *
 * Input: crc - a value of crc
 *        data - packet data pointer to calculate crc
 *        len - packet length
 *
 * Return: crc - return crc
 ************************************************************************
 */
unsigned int crc32 (unsigned int crc, unsigned char *data, int len)
{
    int ib;
    int              crc_table_inited = 0;
    unsigned int     crc_table[256];

    if (!crc_table_inited) {
        int jb;
        unsigned int accum;

        for (ib = 0; ib < 256; ib++) {
            accum = ib;

            for (jb = 0; jb < 8; jb++) {
                if (accum & 1) {
                    accum = accum >> 1 ^ 0xedb88320UL;
                } else {
                    accum = accum >> 1;
                }
            }

            crc_table[ib] = swap32(accum);
        }

        crc_table_inited = 1;
    }

    for (ib = 0; ib < len; ib++) {
        crc = crc << 8 ^ crc_table[crc >> 24 ^ data[ib]];
    }

    return crc;
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
        printf("Warning read packet incorrect.\n");
    }

    return(rv);
}

/*
 ************************************************************************
 * Function: display_pkt
 * Packet display for help debugging
 *
 * Input: b_ptr - buffer porinter
 *        pktlen - packet length
 *
 * Return: 0 when matched
 ************************************************************************
 */
void display_pkt (unsigned char *b_ptr, int pktlen)
{
    int ix, len;
    len = pktlen;

    printf("%s- packet: showing %d bytes\n", __FUNCTION__, len);
    for (ix = 0; ix < len; ix++) {
        if ((ix > 0) && ((ix % 16) == 0)) {
            printf("\n");
        }
        printf("%02x ", *b_ptr++);
    }
    printf("\n");
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

/*
 ************************************************************************
 * Function: chk_macaddr
 * Compare if 2 mac addresses matches
 *
 * Input:
 * macaddr1 and macaddr2 - 2 mac addresses to be compared
 *
 * Return: 0 when matched
 ************************************************************************
 */
int chk_macaddr (unsigned char *macaddr1, unsigned char *macaddr2)
{
    return(memcmp(macaddr1, macaddr2, 6));
}

/***********************************************************************
 *
 * Function:    eth_pkt_tx
 *
 * Description: This function will setup and transmit an ethernet packet
 *              from CPU which is attached to the backplane GE switch.
 *              Then check for no transmission errors .
 *
 * Input:       eth_tx_pkt_t - pointer to structure holding tx packet info
 *                dest_addr    - destination MAC address
 *                src_addr     - source MAC address.  If the contents
 *                               equal zero, then use the host MAC address.
 *                pkt_type     - ethernet packet type field
 *                bufr_st_addr - buffer address to user tx buffer
 *                payload_size - size of tx data
 *                socket  - Linux socket ID of the TX port
 *
 * Output:      0    if the packet is transmitted without errors
 *              != 0 if the packet is transmitted with errors
 *
 ************************************************************************
 */
int eth_pkt_tx (eth_tx_pkt_t *tx_pkt_p)
{
    unsigned char *cptr, *buf_p;
    unsigned char pkt[PKT_BUF_LEN];
    int pkt_len;
    unsigned int crc;

    memset(pkt, 0, PKT_BUF_LEN);
    cptr = pkt;
    buf_p = tx_pkt_p->bufr_st_addr;

    /* Build the packet
     */
    memcpy(cptr, tx_pkt_p->dest_addr, 6);
    cptr += 6;
    memcpy(cptr, tx_pkt_p->src_addr, 6);
    cptr += 6;
    *cptr++ = (tx_pkt_p->pkt_type >> 8) & 0xff;
    *cptr++ = tx_pkt_p->pkt_type & 0xff;


    /* Add the payload after the header
     */
    memcpy(cptr, buf_p, tx_pkt_p->payload_size);
    cptr += tx_pkt_p->payload_size;
    pkt_len = ETH_HDR_LEN + tx_pkt_p->payload_size;

    /* Add crc after the payload
     */
    crc = ~crc32(~0, pkt, pkt_len);
    *cptr++ = (crc >> 24) & 0xff;
    *cptr++ = (crc >> 16) & 0xff;
    *cptr++ = (crc >> 8) & 0xff;
    *cptr++ = crc & 0xff;
    pkt_len += ETH_CRC_LEN;

    if (tx_a_pkt(tx_pkt_p->socket, pkt, pkt_len) != pkt_len) {
        printf("%s Error sending packet on socket %d; length = %d",
               __FUNCTION__, tx_pkt_p->socket, pkt_len);
        return(ETH_PKT_TX_ERR);
    }

    return(ETH_PKT_TX_OK);
}


/***********************************************************************
 *
 * Function:    eth_pkt_rx
 *
 * Description: This function will check to see if an ethernet packet
 *              was received by the GEMAC, which is attached to the backplane
 *              GE switch.
 *              If a receive packets is received, then check for good
 *              receive status and, if it is good, copy the receive
 *              packet to the user supplied buffer.
 *              There is a check on the size of the user supplied
 *              buffer; if the size of the receive packet is larger
 *              than the size of the user supplied buffer, then a
 *              buffer overflow error will be flagged and the receive
 *              data will be truncated to fit the size of the user buffer.
 *              The receive status will be checked to see if there are
 *              any receive errors and is returned to the user in the
 *              eth_rx_pkt_t structure member, rx_status.
 *
 * Input:       eth_rx_pkt_t - pointer to structure holding rx packet info
 *                bufr_st_addr - buffer address to of where to put rx buffer
 *                rx_bufr_size - size of user rx buffer
 *                pkt_size     - 0
 *                pkt_num      - packet number, optional
 *                wait_time    - time to wait for rx packet, in usec
 *                socket  - Linux socket of the RX port
 *
 * Output:      PASSED if a packet is received without errors
 *              FAILED if a packet is received with errors
 *              rx_pkt_p->pkt_size contains the size of the rx packet
 *              rx_pkt_p->rx_status contains the receive buffer
 *                      descriptor status word
 *
 ************************************************************************
 */
int
eth_pkt_rx (eth_rx_pkt_t *rxpkt_p)
{
    int rv = 0;
    struct timeval tv;
    unsigned int wt_sec, wt_usec;

    memset(rxpkt_p->bufr_st_addr, 0, rxpkt_p->rx_bufr_size);

    /* Prepare for RX
     */
    rxpkt_p->pkt_size = 0;
    rxpkt_p->rx_status = 0;

    /* Get the second and microsecond portion of wait time
     * to set the socket time out.
     */
    wt_sec =  rxpkt_p->wait_time / 1000000;
    wt_usec = rxpkt_p->wait_time % 1000000;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = wt_sec;   /* sec portion of wait time */
    tv.tv_usec = wt_usec;  /* microsec portion of wait time */
    if (setsockopt(rxpkt_p->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
        return(ETH_NO_PKT_RX);
    }

    /* rv containe -1 or the number of bytes received
     */
    rv = rx_a_pkt(rxpkt_p->socket,
                  rxpkt_p->bufr_st_addr,
                  rxpkt_p->rx_bufr_size);

    if (rv < 0) {
#if DEBUG
        print("%s receive socket timeout.\n", __FUNCTION__);
        perror("eth_pkt_rx perror value:");
#endif 
        return(ETH_NO_PKT_RX);
    }

    if (rxpkt_p->rx_chk) {
        /* The called of eth_pkt_rx shoule know about the CRC.
         * Subtract 4 byte from the size
         */
        rxpkt_p->pkt_size = rv - ETH_CRC_LEN;

        if ((rv <= (ETH_HDR_LEN + ETH_CRC_LEN)) || (rv > ETH_PKT_LEN_MAX)) {
            printf("%s received under size packet of %d bytes\n", __FUNCTION__, rv);
            perror("eth_pkt_rx perror value:");
            return(ETH_PKT_RX_ERR);
        }
    } else {
        rxpkt_p->pkt_size = rv;
    }


    rxpkt_p->pkt_num += 1;

    if (pkt_deb_flag == TRUE) {
        printf("%s RX packet %d is:\n", __FUNCTION__, rxpkt_p->pkt_num);
        display_pkt(rxpkt_p->bufr_st_addr, rxpkt_p->pkt_size);
    }

    return (ETH_PKT_RX_OK);
}

/*
 ************************************************************************
 * Function: pkt_rx_double
 * This function is the packet receive and compare function for the
 * packet same port loopback test. It is the called back
 * function passed in the pthread_create function in the test.
 *
 * Input:
 * rxpkt_s_ptr - pointer to the rx data structure
 *
 * Return: return specail code via pthread_exit
 ************************************************************************
 */
static void *pkt_rx_double (eth_rx_pkt_t *rxpkt_s_ptr)
{
    unsigned char pkt[2][ETH_PKT_BUF_LEN];
    unsigned int plen[2], pkt_len;
    unsigned char *bufp;    
    int rv, cnt;
    int ii;

    /* Do packet receive
     */
    bufp = rxpkt_s_ptr->bufr_st_addr;
    cnt = 0;
    rx_ready = 1;
    rxpkt_s_ptr->rx_chk = 1;
    do {
        rv = eth_pkt_rx (rxpkt_s_ptr);

	    if (rv == ETH_PKT_RX_OK) {
	        if ((chk_macaddr(&bufp[0], (unsigned char *)mac_da) != 0) ||
	    	(chk_macaddr(&bufp[6], (unsigned char *)mac_sa) != 0)) {
	    	    printf(" detected packet not sent by the test. Ignore.\n");
	            continue; /* not targeted packet */
	        }

	        memcpy(pkt[cnt], bufp, rxpkt_s_ptr->pkt_size);
	        plen[cnt] = rxpkt_s_ptr->pkt_size;

	        printf("\n\n>>>> %s %d bytes received into pkt[%d]: pkt_num= %d\n",
	    	                __FUNCTION__, rxpkt_s_ptr->pkt_size, cnt, rxpkt_s_ptr->pkt_num);
            
	        printf("Destination MAC address: "
	    	                "%02x:%02x:%02x:%02x:%02x:%02x\n",
	    	                pkt[cnt][0],pkt[cnt][1],pkt[cnt][2],
	    	                pkt[cnt][3],pkt[cnt][4],pkt[cnt][5]);
	        printf("Source MAC address: "
	    	                "%02x:%02x:%02x:%02x:%02x:%02x\n",
	    	                pkt[cnt][6],pkt[cnt][7],pkt[cnt][8],
	    	                pkt[cnt][9],pkt[cnt][10],pkt[cnt][11]);

	        for (ii = 0; ii < rxpkt_s_ptr->pkt_size; ii++) {
	            if ((ii > 0) && (ii % 16) == 0) {
	    	        printf("\n");
	            }
	            printf("%02x ", pkt[cnt][ii]);
	        }
	        printf("\n");

	        cnt++;
	    }
    } while ((cnt < 2) && (rv == ETH_PKT_RX_OK));

    if ((cnt == 2) && (rv == ETH_PKT_RX_OK)) {
        pkt_len = (plen[0] > plen[1]) ? plen[0] : plen[1];
        if (memcmp(pkt[0], pkt[1], pkt_len)) {
	        printf("Error: %s Two different packet received.\n", __FUNCTION__);
	        printf("pkt[0]:\n");
	        display_pkt(pkt[0], plen[0]);
	        printf("pkt[1]:\n");
	        display_pkt(pkt[1], plen[1]);

	        pthread_exit((void *)ETH_PKT_RX_ERR);
	    } else {
	      pthread_exit((void *)ETH_PKT_RX_OK);
	    }
    } else {
        pthread_exit((void *)ETH_NO_PKT_RX);
    }
}

/*
 * Function: pkt_lpbk_util
 * This fucntion performs same port packet loopback 
 * on specific interface.
 *
 * Input:
 * if_name - interface name, e.g. eth1, xaui0 ...
 * pkt_cnt - number of packets used in the test
 *
 * Return: PASS/FAIL
 */
int pkt_lpbk_util (char *if_name, int pkt_cnt)
{
    pthread_t threads;
    int rc, rtn_val;
    int ii, ix, retry = 5;
    void  *pthr_rv = NULL;

    /* var for rx
     */
    int rx_skt;
    eth_rx_pkt_t rxpkt_s;
    unsigned char rxpkt_buf[ETH_PKT_BUF_LEN];

    /* var for tx
     */
    //unsigned short pkt_type = 0x0800;
    unsigned short pkt_type = 0x8ff0;
    int payload_size = PKT_PAYLOAD_SIZE;
    int tx_skt;
    eth_tx_pkt_t txpkt_s;
    unsigned char txpkt_buf[ETH_PKT_BUF_LEN];

    /*prepare source mac addr*/
    system_mac_addr_get(if_name, &mac_sa);

    /* 1. prepare the rx data structure
     */
    if (setup_eth_dev(if_name, &rx_skt) == FAIL) {
        return (FAILED);
    }
    
    if((!strcmp(if_name, "eth1_mac1"))||(!strcmp(if_name, "eth1"))) {
        /* 1.5 add socket filter based on mac address 
         */
        if (set_src_mac_filt(&rx_skt, if_name) != PASSED) {
            return (FAILED);
        }
    }
    
    memset(rxpkt_buf, 0, ETH_PKT_BUF_LEN);
    rxpkt_s.socket = rx_skt;
    rxpkt_s.wait_time = RX_PKT_WAIT_TIME; /* 3000000 in usec = 3 sec */

    rxpkt_s.pkt_size = 0;
    rxpkt_s.bufr_st_addr = rxpkt_buf;
    rxpkt_s.rx_bufr_size = ETH_PKT_BUF_LEN;
    rxpkt_s.pkt_num = 0;

    /* 2. prepare the tx data structure
     */
    if (setup_eth_dev(if_name, &tx_skt) == FAIL) {
        return (FAILED);
    }
    memset(txpkt_buf, 0, ETH_PKT_BUF_LEN);

    /* We are using raw socket and this is a loopback test, so
     * using the same mac addr for both da and sa is okay.
     */
    memcpy(txpkt_s.dest_addr, mac_da, 6);
    memcpy(txpkt_s.src_addr, mac_sa, 6);
    txpkt_s.pkt_type = pkt_type;
    txpkt_s.tx_status = 0; 
    txpkt_s.socket = tx_skt;
    txpkt_s.bufr_st_addr = &txpkt_buf[ETH_HDR_LEN]; /* point to payload */

    /* 3. Do the test
     */
    rtn_val = PASS;
    rx_ready = 0;
    
    for (ii=0; ii < pkt_cnt; ii++) {
        /* Prepare the tx packet */
        txpkt_s.payload_size = payload_size;
        gen_eth_pkt(txpkt_s.dest_addr, txpkt_s.src_addr, txpkt_s.pkt_type,
		    (0x00 + ii), 1, txpkt_s.payload_size, txpkt_buf);

        /* 4. Start a rx thread for each packet, 
         * Due to BMC lack of resource we use retry as workaround
         * */

        for (ix = 0; ix < retry; ix++) {
        	rc = pthread_create(&threads, NULL, (void *)pkt_rx_double, (eth_rx_pkt_t *) &rxpkt_s);
	        if (rc == PASSED) {
                break;
            } else if (rc != EAGAIN) {
	            printf("%s pthread_create failed \n", __FUNCTION__);
	            printf("%d %s\n", rc, __FUNCTION__);
	            return (FAILED);
	        } else {
                /*
                 *printf("EAGAIN, retry %s\n", __FUNCTION__);
                 */
            }
        }

	    while (rx_ready == 0) {
	        usleep(10000);
	    }

	    /* Notes;
	     * For the first 3 packets, wait a little longer after the rx
	     * is socket enabled.
	     * The linux driver always send out a few ipv6 multicase 
	     * packets when the port just start up. If the test packets
	     * are sent during this time. the port will stop.
	     */
	    if(ii < 3) {
	      sleep(1);
	    }
	    else {
	      usleep(10000);
	    }

	    /* 5. Tx the packet
	     */
	    rc = eth_pkt_tx(&txpkt_s);
	    if (rc != ETH_PKT_TX_OK) {
	        printf("Error: packet tx failed\n");
	        rtn_val = FAILED;
	        break;
	    }
	    
	    pthread_join(threads, (void **)&pthr_rv);

	    /* 6. Compare the rx packet with the tx packet
	     */
	    if ((ulong)pthr_rv == ETH_PKT_RX_OK) {
	        if (rxpkt_s.pkt_size == (txpkt_s.payload_size + ETH_HDR_LEN)) {
	            rc = memcmp((rxpkt_s.bufr_st_addr + ETH_HDR_LEN), txpkt_s.bufr_st_addr,
	    		    txpkt_s.payload_size);
	    	    if (rc == 0) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
	    	            printf("TX and RX packet match.\n");
                    }
	    	    } else {
	    	        printf("Warning: TX and RX packet mismatch.\n");
	    	        rtn_val = FAILED;
	    	        break;
	    	    }		  

	        } else {
	            printf("Warning: RX packet size %d not equal to the TX packet size %d\n",
	    	           rxpkt_s.pkt_size, (txpkt_s.payload_size + ETH_HDR_LEN));
	    	    rtn_val = FAILED;
	    	    break;
	        }
	    } else {
	        printf("Warning: RX packet receive incorrect\n");
	        rtn_val = FAILED;
	        break;
	    }
    }

    if (rtn_val == FAIL) {
        printf("-----TX packet is : -----\n");
        display_pkt(txpkt_buf, txpkt_s.payload_size + ETH_HDR_LEN);
        printf("-----RX packet is : -----\n");
        display_pkt(rxpkt_s.bufr_st_addr, rxpkt_s.pkt_size);
    }

    cleanup_eth_dev(if_name, tx_skt);
    cleanup_eth_dev(if_name, rx_skt);

    return (rtn_val);
}

/*
 ************************************************************************
 *
 * Function: set_src_mac_filt
 *
 * Description : setup filter to drop packet with invalid src. 
 *
 * Input: point to socket num
 *
 * Output: PASSED/FAILED
 ************************************************************************
 */
int set_src_mac_filt (int *sock, char *if_name) {

    int ret, socket; 

    socket = *sock; 
    /* sort out the dynamic src mac addr? */

    printf("%s filter\n", if_name);
    ret = setsockopt(socket, SOL_SOCKET, SO_ATTACH_FILTER,
                     &bpf, sizeof(bpf));

    if (ret != PASSED) { 
        printf("%s: failed to attach filter \n", __FUNCTION__);
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/*
 ************************************************************************
 *
 * Function: main 
 *
 * Description : the entry of packet transfer 
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 ************************************************************************
 */
#ifdef TEMPORARY_PASS
int main (int argc, char *argv[]) { 

    int opt_ch, pkt_cnt = 100, ret;
    char if_name[10]; 
    
    pkt_deb_flag = FALSE;

    /* need argument otherwise display usage */
    if (argc == 1) {
        goto USAGE;
    }

    while ((opt_ch = getopt(argc, (char **)argv, "i:c:d")) >= 0) {
        switch(opt_ch) {
            case 'i':
                sprintf(if_name, optarg);
            break;
            case 'c':
                pkt_cnt = atoi(optarg);
            break;
            case 'd':
                pkt_deb_flag = TRUE; 
            break;
            case '?': 
                goto USAGE;
            break; 
         }
    }
#endif /* LINUX_APP */

int eth_pkt_txrx (char *if_name, int pkt_cnt, int debug_flag) { 

    int ret; 

    pkt_deb_flag = FALSE;

    if (debug_flag == TRUE) {
        pkt_deb_flag = TRUE;
    }

    printf("%s loopback %d packets ", if_name, pkt_cnt);
    ret = pkt_lpbk_util(if_name, pkt_cnt);

    if (ret == PASS) {
        return PASS;
    } else {
        return FAIL;
    }

#ifdef TEMPORARY_PASS    
USAGE:
    eth_pkt_txrx_usage(); 
    return 0;
#endif
}
/*---------------------------------------------------------------
$Log: diag_eth_pkt_txrx.c,v $
Revision 1.1  2020/08/19 09:50:04  markzha
*** empty log message ***

$Endlog$
*/

