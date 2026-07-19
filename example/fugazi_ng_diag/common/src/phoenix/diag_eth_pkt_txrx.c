/* $Id: diag_eth_pkt_txrx.c,v 1.2 2021/04/15 00:52:24 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_eth_pkt_txrx.c,v $
 *------------------------------------------------------------------
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
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
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
#include "diag_eth_pkt_txrx_utils.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_eth_info.h"
#include "types.h"
#include "dnv_eth_lib.h"
#include "plat_defs.h"
#include "nvmonvars.h"
#include "ngio.h"

int eth_pkt_txrx (char *, int, int);
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

/* This MAC address array is for sgmii 1-3
 * These values in the array will be replaced by the
 * real value read from the system
 */
mac_addr_t local_mac_addr[] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* eth0 not used internally */
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x20, 0x20, 0x20, 0x20, 0x20, 0x20},
    {0x30, 0x30, 0x30, 0x30, 0x30, 0x30},
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x50, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x60, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x70, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x80, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x90, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0xa0, 0x40, 0x40, 0x40, 0x40, 0x40},
};

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
        print_debug_msg("%s receive socket timeout.\n", __FUNCTION__);
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
	    	    print_debug_msg(" detected packet not sent by the test. Ignore.\n");
	            continue; /* not targeted packet */
	        }

	        memcpy(pkt[cnt], bufp, rxpkt_s_ptr->pkt_size);
	        plen[cnt] = rxpkt_s_ptr->pkt_size;

	        print_debug_msg("\n\n>>>> %s %d bytes received into pkt[%d]: pkt_num= %d\n",
	    	                __FUNCTION__, rxpkt_s_ptr->pkt_size, cnt, rxpkt_s_ptr->pkt_num);
            
	        print_debug_msg("Destination MAC address: "
	    	                "%02x:%02x:%02x:%02x:%02x:%02x\n",
	    	                pkt[cnt][0],pkt[cnt][1],pkt[cnt][2],
	    	                pkt[cnt][3],pkt[cnt][4],pkt[cnt][5]);
	        print_debug_msg("Source MAC address: "
	    	                "%02x:%02x:%02x:%02x:%02x:%02x\n",
	    	                pkt[cnt][6],pkt[cnt][7],pkt[cnt][8],
	    	                pkt[cnt][9],pkt[cnt][10],pkt[cnt][11]);

	        for (ii = 0; ii < rxpkt_s_ptr->pkt_size; ii++) {
	            if ((ii > 0) && (ii % 16) == 0) {
	    	        print_debug_msg("\n");
	            }
	            print_debug_msg("%02x ", pkt[cnt][ii]);
	        }
	        print_debug_msg("\n");

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
    unsigned short pkt_type = 0x8ff0;
    int payload_size = PKT_PAYLOAD_SIZE;
    int tx_skt;
    eth_tx_pkt_t txpkt_s;
    unsigned char txpkt_buf[ETH_PKT_BUF_LEN];

    /*prepare source mac addr*/
    system_mac_addr_get(if_name, &mac_sa);

    /* 1. prepare the rx data structure
     */
    if (setup_eth_dev(if_name, &rx_skt) == FAILED) {
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
    if (setup_eth_dev(if_name, &tx_skt) == FAILED) {
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
    
    for (ii = 0; ii < pkt_cnt; ii++) {
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
	        printf("WARNING: packet tx had problem\n");
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
	    	    } else {
	    	        printf("WARNING: TX and RX packet mismatch.\n");
	    	        rtn_val = FAILED;
	    	        break;
	    	    }		  

	        } else {
	            printf("WARNING: RX packet size %d not equal to the TX packet size %d\n",
	    	           rxpkt_s.pkt_size, (txpkt_s.payload_size + ETH_HDR_LEN));
	    	    rtn_val = FAILED;
	    	    break;
	        }
	    } else {
	        printf("WARNING: RX packet receive had problem\n");
	        rtn_val = FAILED;
	        break;
	    }
    }

    if (rtn_val == FAIL) {
        printf("-----TX packet is : -----\n");
        display_pkt(txpkt_buf, rxpkt_s.pkt_size);
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
        printf("PASSED\n");
        return PASS;
    } else {
        printf("WARNING!!!!\n");
        return FAIL;
    }

#ifdef TEMPORARY_PASS    
USAGE:
    eth_pkt_txrx_usage(); 
    return 0;
#endif
}



/***********************************************************************
 *
 * Function:    get_sgmii_port_num()
 *
 * Description: Legacy code port from ISR-G2. Used by NGIO cards
 *              which also support ISR-G2 systems.
 *              Return the sgmii port number of the device attached
 *              to the sgmii port.
 *
 * Input:       port - 0 based port number of the type 
 *              type - TYPE_SWITCH
 *
 * Output:      -1   if the type is invalid or the port number is out of range
 *              else the sgmii port number for the device of type TYPE, and port
 *
 ************************************************************************
 */ 
int get_sgmii_port_num (uint port, uint type)
{
    return (DSP_SLOT0_ETH_PORT);
}

int sgmii_lpbk_util(int port, int pkt_cnt)
{
    char eth_interface[8];
    char *iface_name = eth_interface;
    int main_result = FAILED;


    printf("Enter ETH interface for loopback (e.g eth0): ");
    scanf("%s", iface_name);
    main_result = eth_pkt_txrx(iface_name, 3, FALSE);

    if (main_result != PASSED) {
        printf("\n%s: Failed To do loopback\n",
                      __FUNCTION__);
        return FAILED;
    } else {
        printf("Passed - Packets loopback \n");
        return PASSED;
    }
}

/***********************************************************************
 *
 * Function:    rbcp_eth_pkt_rx
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
 * Output:      PASSED  if a packet is received without errors
 *              FAILED if a packet is received with errors
 *              rx_pkt_p->pkt_size contains the size of the rx packet
 *              rx_pkt_p->rx_status contains the receive buffer
 *                      descriptor status word
 *
 ************************************************************************
 */
int rbcp_eth_pkt_rx (eth_rx_pkt_t *rxpkt_p)
{
    int rv = 0;
    struct timeval tv;
    uint wt_sec, wt_usec;

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
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s receive socket timeout.\n", __FUNCTION__);
            perror("eth_pkt_rx perror value:");
        }
        return(ETH_NO_PKT_RX);
    }

    if (rv > ETH_MAX_LEN) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s rx packet size error.\n", __FUNCTION__);
            perror("eth_pkt_rx perror value:");
        }
        return(ETH_PKT_RX_ERR);
    }

    /* The called of eth_pkt_rx shoule know about the CRC.
     * Subtract 4 byte from the size
     */
    rxpkt_p->pkt_size = rv - ETH_CRC_LEN;
    rxpkt_p->pkt_num += 1;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s RX packet %d is:\n", __FUNCTION__, rxpkt_p->pkt_num);
        display_pkt(rxpkt_p->bufr_st_addr, rxpkt_p->pkt_size);
    }

    return (ETH_PKT_RX_OK);
}

/***********************************************************************
 * Name:        get_host_mac_addr
 *
 * Description: get host mac addr
 *
 * Input:       port number, use array to store the address
 *
 * Output:      OK : 0.
 *              
 *
 ***********************************************************************
 */
int get_host_mac_addr (uint port, unsigned char *mac_buf)
{
    char file_name[] = "eth1";
    int sgmii_port = get_ctrl_plane_sgmii_port();
    mac_addr_t eth_if_mac_addr[] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    };

    sgmii_port = DSP_SLOT0_ETH_INTF;
    sprintf(file_name, "eth%d", sgmii_port);
    
    system_mac_addr_get(file_name, &eth_if_mac_addr[0]);
    memcpy(mac_buf, &eth_if_mac_addr[0], sizeof(mac_addr_t));
    
    return(0);
}

/***********************************************************************
 * Function: get_ctrl_plane_sgmii_port
 *
 * Dummy function for NIM
 *
 * Input: void
 *
 * Return: 0
 ************************************************************************
 */
int get_ctrl_plane_sgmii_port (void)
{
    return (CPU_SGMII_PORT1);
}

/***********************************************************************
 * Function: local_mac_addrs_init
 *   Initialize the arrary that keeps the mac sa of the 3 cavecreek
 *   sgmii ports
 *
 * Input: none
 *
 * Return: void
 ************************************************************************
 */
void local_mac_addrs_init (void)
{
    char file_name[] = "eth1";
    int sgmii;
    int min_port, max_port;

    max_port = CPU_SGMII_PORT3;
	min_port = CPU_SGMII_PORT0;
    
    for (sgmii= min_port; sgmii <= max_port; sgmii++) {
        /* Get the real mac address from kernel file and set it into
	    * local_mac_addr for diaglinux to use each port's real mac
	    * address.
	    */
        sprintf(file_name, "eth%d", sgmii);
        system_mac_addr_get(file_name, &local_mac_addr[sgmii]);
    }
}

