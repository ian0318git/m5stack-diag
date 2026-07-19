/* $Id: skye_ext_lpbk.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * skye_ext_lpbk.c
 * support PHY external loopback 
 * internal loopback: media PHY, bridge PHY .
 *
 * May 2013, Steja
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
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
#include "skye_eth.h"
#include "skye_ext_lpbk.h"
#include "skye_xaui.h"
#include "diag_common_drv.h"
#include "nvmonvars.h"
#include "sgmii_defs.h"
#include "skye_main.h"
#include "platform_fru.h"

extern boolean is_cpu0(void);
extern int dev_88e1514_cleanup_lpbk(void);
extern int dev_88e1514_set_lpbk(int, int);
static int		crc_table_inited;
static unsigned int	crc_table[256];

int setup_xaui_port(int, int *);
int skye_tilera_is_linkup(char *, int);
int cpu0_xaui_bp_lp_test(void);

/* packet buffer */
unsigned char tx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet[ETH_PKT_MAX_LEN];
unsigned char rx_packet_sec[ETH_PKT_MAX_LEN];
unsigned char rx_packet_cmd[ETH_PKT_MAX_LEN];
unsigned char global_pkt_array[ETH_PKT_MAX_LEN*2+1];


/* Global define for different packet length */
int g_lbpacket_len = 0;

sem_t rx_ready, rx_finish, tx_cmp;

mac_addr_t mac_da = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
mac_addr_t mac_sa = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};

typedef struct skye_cmd_table {
    ushort cmd;
    PFI routine;
    int param;
} skye_cmd_table_t;

/*
 *  Table of commands received from host platform and the routines to
 *  be executed for the command.
 */
static skye_cmd_table_t skye_cmd_table[] = {
    {FROM_HOST_CPU_ALIVE_TEST, skye_cpu_alive_test, 0},
    {FROM_HOST_SWITCH_CONSOLE, skye_switch_console, 0},

};
#define NUM_CMDS_FRM_HOST sizeof(skye_cmd_table)/sizeof(skye_cmd_table_t)


/* Ethernet packet 4 byte CRC calculationg
 */
unsigned int
crc32 (unsigned int crc, unsigned char *data, int len)
{
    int			i;

    if (!crc_table_inited) {
	int		j;
	unsigned int		accum;

	for (i = 0; i < 256; i++) {
	    accum = i;

	    for (j = 0; j < 8; j++) {
		if (accum & 1) {
		    accum = accum >> 1 ^ 0xedb88320UL;
		} else {
		    accum = accum >> 1;
		}
	    }

	    crc_table[i] = SWAP32(accum);
	}

	crc_table_inited = 1;
    }

    for (i = 0; i < len; i++) {
	crc = crc << 8 ^ crc_table[crc >> 24 ^ data[i]];
    }

    return crc;
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
int
create_raw_socket (int protocol)
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
int
bind_socket (char *device, int rawsock, int protocol)
{
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
 * Input:  device - pointer to device
 *         sock   - socket structure
 *
 * Output: PASSED/FAILED
 *
 * Note: if the set_promisc is failed, the rx will get the haft of 
 *       packet from tx.
 *------------------------------------------------------------------
 */
int
set_promisc (char *device, int sock)
{
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
 * Input:  p_type    - Ethernet type
 *         sgmii_port - host system sgmii port to initialize
 *         *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:  PASS/FAIL
 *
 ************************************************************************
 */
int
setup_eth_port (char *p_type, int sgmii_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf (eth_name, "%s%d", p_type, sgmii_port);

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
int
setup_xaui_port (int xaui_port, int *socket)
{
    int raw;
    char eth_name[5];

    sprintf (eth_name, "xgbe%d", xaui_port);

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
int
send_raw_packet (int rawsock, unsigned char *pkt, int pkt_len)
{
    int sent= 0;
 
    /* A simple write on the socket ..thats all it takes ! */
    if((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
        return PASSED;
    }

    return FAILED;

}

/*------------------------------------------------------------------
 *
 * Function: receive_raw_packet
 *    read packet via socket.
 *
 * Input:  rawsock - socket
 *         pkt - rx buffer
 *         pkt_len - size of packet
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
receive_raw_packet (int rawsock, unsigned char *pkt, int pkt_len)
{
    int recv = 0;

    recv = read(rawsock, pkt, pkt_len);
    /* A simple read on the socket .. that's all it takes ! */
    if(recv < 0) {
        return FAILED;
    }

    return PASSED;
}

/*------------------------------------------------------------------
 *
 * Function: pkt_cmp
 *    packet compare
 *
 * Input:  bufa - buffer a
 *         bufb - buffer b
 *         count - size of packet
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
pkt_cmp (unsigned char *bufa, unsigned char *bufb, int count)
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
 * Function: skye_tilera_is_linkup
 *   Check linux up status from Linux information.
 *
 * Input: port number.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
skye_tilera_is_linkup (char *type, int port)
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
 * Input:  pkt_len - packet length
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
check_pkt (int pkt_len)
{
    if ((NVRAM)->diagflag & D_VERBOSE) { /* Turns debug on */
        int yy;
        for (yy=0; yy < pkt_len; yy++) {
	    if ((yy > 0) && (yy % 16) == 0) {
	    printf("\n");
        }
	    printf("(tx:%02x rx:%02x)", tx_packet[yy],rx_packet[yy]);
        }
            printf("%s end of pkt(%d) print\n",__FUNCTION__,pkt_len);
    }

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
 * Function: chk_macaddr
 *   check mac address
 *
 * Input:  macaddr1 - macaddress 1
 *         macaddr2 - macaddress 2
 *
 * Output: return result
 *
 *------------------------------------------------------------------
 */
int
chk_macaddr (uchar *macaddr1, uchar *macaddr2)
{
    return (pkt_cmp(macaddr1, macaddr2,6));
}

/*------------------------------------------------------------------
 *
 * Function: send_packets
 *   for tx send packet to rx. if number of packet is too much,
 *   then the delay is needed.
 *
 * Input:  socket - socket structure
 *         len - packet length
 *         val - content of packet
 *         port - socket port
 *         speed - socket speed
 *
 * Output: PASSED/FAILED
 * 
 *------------------------------------------------------------------
 */
int
send_packets (int *socket, int len, char val, int port, int speed)
{   
    int raw, rc = 0, ix;
    uint mac_size, fil_len;
    unsigned char volatile *cptr;
    char iface_type[32];
    raw = *socket;

    /* clean up the tx_packet buffer */
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
    for (ix = 0; ix < fil_len ; ix++ , cptr++ ) {
        *cptr = val + ix;
    }

#ifdef DEBUG
    printf("len = 0x%2x  \n", len);
    for ( ix =0; ix <len; ix++)
       printf("tx_packet[%d] = 0x%2x  ", ix, tx_packet[ix]);
#endif

    if (speed == SPD_10000MBPS) {
        sprintf(iface_type, SEL_PORT_XAUI);
    } else {
        sprintf(iface_type, SEL_PORT_ETH);
    }

    rc = skye_tilera_is_linkup(iface_type, port);
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
 * Function: cpu1_send_packets
 * For CPU1 tx send packet to CPU0 rx.
 *
 * Input:  len - packet length
 *         val - content of packet
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu1_send_packets (int *socket, int len, char val, int port)
{
    int raw;
    raw = *socket;

    if(!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        cterr('f',0, "error on sending packet");
        return (FAILED);
    }
    /* clean up the tx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
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
 * Output: None
 *
 *------------------------------------------------------------------
 */
void *
receive_packets (diag_info_pthread_t *get_info)
{
    int rx= 0, rc = 0;
    struct timespec ts;
    uint ii, pkt_cnt = 0;
    uchar *rx_pkt_buf;
    struct timeval tv;
    int otherpkt_cnt = 0;

#if DEBUG
    int yy;

    printf(" %s ", __FUNCTION__);
    printf(" name %s ", get_info->name);
    printf(" speed %d ", get_info->speed);
    printf(" pkt_num %d ", get_info->pkt_num);
    printf(" pkt_len %d ", get_info->pkt_len);
    printf(" signal %d ", get_info->signal);
#endif
    printf(" socket %d ", get_info->socket);
    
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
            g_lbpacket_len = rx;

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
void *
slave_receive_packets (diag_info_pthread_t *get_info)
{
    int rx= 0, rc = 0;
    struct timespec ts;
    uint ii, pkt_cnt = 0;
    uchar *rx_pkt_buf;


    /* clean up the rx_packet buffer */
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
    rx_pkt_buf = (uchar *)rx_packet;

    /* init timeout value. */
    rc = clock_gettime(CLOCK_REALTIME, &ts);
    if (rc != PASSED) {
    	cterr('f', 0, "clock gettime failed..");
        return (void *)FAILED;
    }
    /* SYNC timer set 100 second */
    ts.tv_sec += TX_RX_SYNC_TIME;
    ts.tv_nsec = 0;

#if DEBUG
    printf(" %s ", __FUNCTION__);
    printf(" name %s ", get_info->name);
    printf(" speed %d ", get_info->speed);
    printf(" pkt_num %d ", get_info->pkt_num);
    printf(" pkt_len %d ", get_info->pkt_len);
    printf(" signal %d ", get_info->signal);
    printf(" socket %d ", get_info->socket);
#endif

#if USER_TIMEOUT /* uncoment this if you want rx to time out */
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = TX_RX_SYNC_TIME;   /* set receive time 10 secs */
    if (setsockopt(get_info->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
        return (void *)FAILED;
    }
#endif
   /* when send packet will got a driver layer loopback. one for original path
    * another one from driver. so * 2.
    */
    ii = 0;
    while (ii < get_info->pkt_num) {
        /* clean up the rx_packet buffer */
        memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

        rx = read(get_info->socket, (unsigned char *)rx_packet, get_info->pkt_len);

        /* If read packet same with last time transmit packet,
         * this packet will driver level loopback packet need drop
         */
#ifdef DEBUG
        if ( !check_pkt(rx) ) {
		   printf("Drop driver level internal loopback packet count=%d, packet size=%d.\n",ii,rx);
		   continue;
        }
#endif
        if (rx <= 0) {
            printf("Did NOT received %d bytes\n",rx);
            return (void *)(FAILED);
        }
        g_lbpacket_len = rx;

#ifdef DEBUG_SHOW
        printf("Packet %d received %d bytes\n", ii, g_lbpacket_len);
#endif

		memcpy((unsigned char *)tx_packet, (unsigned char *)rx_packet, g_lbpacket_len);
#if DROP_ERRPACK
            /* drop invalid packet, MAC address wrong or packet length wrong */
            if ((chk_macaddr(&rx_pkt_buf[0], mac_da) != 0) &&
                (chk_macaddr(&rx_pkt_buf[6], mac_sa) != 0)) {
#if DEBUG
            	show_buf_content(rx_info.pkt_len);
#endif
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); /* clean up buffer */
#if DEBUG
               printf("\n detected non diag packet. Ignore.\n");
#endif
                pkt_cnt = 0; /* restore the packet count */
                continue; /* not firmware download packet */
            }

            if (rx != get_info->pkt_len) {
                printf("received %d bytes but expected %d bytes\n",rx, get_info->pkt_len);
                return (void *)FAILED;
            }
#endif
#if DEBUG
        int yy;
		for (yy=0; yy < rx; yy++) {
		  if ((yy > 0) && (yy % 16) == 0) {
		    printf("\n");
		  }
		  printf("%02x ", rx_packet[yy]);
		}
		printf("%s end of pkt(%d) print\n",__FUNCTION__,pkt_cnt);
#endif
            /* valid packet, increase the packet count */
            pkt_cnt++;

#if DEBUG
            printf("%d bytes received: !!! , pkt_cnt = %d \n", rx, pkt_cnt);
#endif

       /*
        * Inform tx for rx read packet is ready.
        */
       if (sem_post(&rx_ready)) {
           if (errno == EINVAL) {
               printf("The sem(rx_ready) does not refer to a valid semaphore \n");
           } else {
               printf("The function sem_post() is not supported by this implementation\n");
           }
           return (void *)FAILED;
       }

        /* wait for tx complete send packet */
        rc = sem_timedwait(&tx_cmp, &ts);
        if (rc != PASSED) {
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on tx_cmp timeout. \n");
            } else {
                printf("semaphore wait on tx_cmp failed. \n");
            }

            return (void *)FAILED;
        }

        /* clean up the rx_packet buffer ready for next time receive */
        memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
        rx_pkt_buf = rx_packet;
        ii++;
    }  /* Ready for receive next Packet */

    return PASSED;
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
void *
cmd_respond_receive_packets (diag_info_pthread_t *get_info)
{
    int rx= 0, rc = 0;
    struct timespec ts;
    uint ii, pkt_cnt = 0;
    uchar *rx_pkt_buf;

    /* clean up the rx_packet buffer */
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
    rx_pkt_buf = (uchar *)rx_packet;

    /* init timeout value. */
    rc = clock_gettime(CLOCK_REALTIME, &ts);
    if (rc != PASSED) {
    	cterr('f', 0, "clock gettime failed..");
        return (void *)FAILED;
    }
    /* SYNC timer set 100 second */
    ts.tv_sec += TX_RX_SYNC_TIME;
    ts.tv_nsec = 0;

#if DEBUG
    printf(" %s ", __FUNCTION__);
    printf(" name %s ", get_info->name);
    printf(" speed %d ", get_info->speed);
    printf(" pkt_num %d ", get_info->pkt_num);
    printf(" pkt_len %d ", get_info->pkt_len);
    printf(" signal %d ", get_info->signal);
    printf(" socket %d ", get_info->socket);
#endif

#if USER_TIMEOUT /* uncoment this if you want rx to time out */
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = TX_RX_SYNC_TIME;   /* set receive time 10 secs */
    if (setsockopt(get_info->socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,
                   sizeof(struct timeval))==-1) {
        perror("can't set receive time out");
        return (void *)FAILED;
    }
#endif
   /* when send packet will got a driver layer loopback. one for original path
    * another one from driver. so * 2.
    */
    ii = 0;
    while (ii < get_info->pkt_num) {
        /* clean up the rx_packet buffer */
        memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

        rx = read(get_info->socket, (unsigned char *)rx_packet, get_info->pkt_len);

        /* If read packet same with last time transmit packet,
         * this packet will driver level loopback packet need drop
         */
#ifdef DEBUG
        if ( !check_pkt(rx) ) {
		   printf("Drop driver level internal loopback packet count=%d, packet size=%d.\n",ii,rx);
		   continue;
        }
#endif
        if (rx <= 0) {
            printf("Did NOT received %d bytes\n",rx);
            return (void *)(FAILED);
        }
        g_lbpacket_len = rx;

#if DEBUG_SHOW
        printf("Packet %d received %d bytes\n", ii, g_lbpacket_len);
#endif

		memcpy((unsigned char *)rx_packet_cmd, (unsigned char *)rx_packet, g_lbpacket_len);
#if DROP_ERRPACK
            /* drop invalid packet, MAC address wrong or packet length wrong */
            if ((chk_macaddr(&rx_pkt_buf[0], mac_da) != 0) &&
                (chk_macaddr(&rx_pkt_buf[6], mac_sa) != 0)) {
#if DEBUG
            	show_buf_content(rx_info.pkt_len);
#endif
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); /* clean up buffer */
#if DEBUG
               printf("\n detected non diag packet. Ignore.\n");
#endif
                pkt_cnt = 0; /* restore the packet count */
                continue; /* not firmware download packet */
            }

            if (rx != get_info->pkt_len) {
                printf("received %d bytes but expected %d bytes\n",rx, get_info->pkt_len);
                return (void *)FAILED;
            }
#endif
#if DEBUG
        int yy;
		for (yy=0; yy < rx; yy++) {
		  if ((yy > 0) && (yy % 16) == 0) {
		    printf("\n");
		  }
		  printf("%02x ", rx_packet[yy]);
		}
		printf("%s end of pkt(%d) print\n",__FUNCTION__,pkt_cnt);
#endif
            /* valid packet, increase the packet count */
            pkt_cnt++;

#if DEBUG
            printf("%d bytes received: !!! , pkt_cnt = %d \n", rx, pkt_cnt);
#endif

       /*
        * Inform tx for rx read packet is ready.
        */
       if (sem_post(&rx_ready)) {
           if (errno == EINVAL) {
               printf("The sem(rx_ready) does not refer to a valid semaphore \n");
           } else {
               printf("The function sem_post() is not supported by this implementation\n");
           }
           return (void *)FAILED;
       }

        /* wait for tx complete send packet */
        rc = sem_timedwait(&tx_cmp, &ts);
        if (rc != PASSED) {
            if (errno == ETIMEDOUT) {
                printf("sem_timedwait on tx_cmp timeout. \n");
            } else {
                printf("semaphore wait on tx_cmp failed. \n");
            }

            return (void *)FAILED;
        }

        /* clean up the rx_packet buffer ready for next time receive */
        memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
        rx_pkt_buf = rx_packet;
        ii++;
    }  /* Ready for receive next Packet */

    return PASSED;
}

/*------------------------------------------------------------------
 *
 * Function: show_buf_content
 *  show buffer content
 *
 * Input:  show pkt length
 *
 *
 * Output: none
 *
 *------------------------------------------------------------------
 */
void
show_buf_content (int show_pkt_len)
{    
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
 * Function: show_tx_buf_content
 *  show tx buffer content
 *
 * Input:  tx pkt length
 *
 *
 * Output: none
 *
 *------------------------------------------------------------------
 */
void
show_tx_buf_content (int show_pkt_len)
{
    uint ii;
    unsigned char volatile *tptr;

    tptr = tx_packet;

    printf("\nstart of TX pkt print.\n");
    for (ii=0; ii < show_pkt_len; ii++) {
	    if ((ii > 0) && (ii % 8) == 0) {
		         printf("\n");
	    }
        printf("tx:%02x ", tptr[ii]);
    }
    printf("\nend of TX pkt print.\n");
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
int
tx_rx_diag (char* p_type, int eth_port, int speed,
            int pkt_cnt, int pkt_len, int value) 
{
    pthread_t threads;
    struct timespec ts;
    diag_info_pthread_t rx_info;
    char pname[10];
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
    
    sprintf(pname,"%s%d", p_type, eth_port);
    printf("\nEthernet: %s\n",pname);
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
        if (setup_eth_port(p_type, eth_port, &tx_skt) == FAIL) {
            return(FAILED);
        }
    
        if (setup_eth_port(p_type, eth_port, &rx_skt) == FAIL) {
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
        rc = send_packets(&tx_skt, pkt_len, value, eth_port, speed);
        msleep(10); /* ensure tx is slow than rx */

        if (rc == FAILED) {
            printf("send_packets failed\n");
            goto exit_tx_rx_diag;
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
        rc = check_pkt(pkt_len);
        if (rc == PASSED) {
            /* if match, clean up rx buffer for next packet. */
            memset((unsigned char *)tx_packet, 1, ETH_PKT_MAX_LEN);
            memset((unsigned char *)rx_packet, 2, ETH_PKT_MAX_LEN);
            /* printf("match\n"); */
        } else {
            printf("%s: mismatch\n", __FUNCTION__);
            show_buf_content(50);
            goto exit_tx_rx_diag;
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
    printf("\nPacket %d send %d bytes", ii , pkt_len);

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
 * Function: rx_tx_diag
 *  Using Pthread to create another thread for rx.
 *  tx should wait for rx build. When rx get data copy to tx then send out.
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
int
rx_tx_diag (char* p_type, int eth_port, int speed,
            int pkt_cnt, int pkt_len, int value)
{
    pthread_t threads;
    struct timespec ts;
    diag_info_pthread_t rx_info;
    char pname[10];
    int ii;
    int rc;
    void  *pthr_rv;
    int tx_skt, rx_skt;

    /* init the semaphore. */
    rc = sem_init(&rx_ready, 0, 0 );
    if (rc != PASSED) {
        cterr('f',0, "eth_port %d sem_init on rx_ready failed.", eth_port);
        return (FAILED);
    }

    rc = sem_init(&tx_cmp, 0, 0 );
    if (rc != PASSED) {
        cterr('f',0, "eth_port %d sem_init on tx_cmp failed.", eth_port);
        return (FAILED);
    }

    /* init timeout value. */
    rc = clock_gettime(CLOCK_REALTIME, &ts);
    if (rc != PASSED) {
        cterr('f',0, "eth_port %d clock gettime failed..", eth_port);
        return (FAILED);
    }

    sprintf(pname,"%s%d", p_type, eth_port);
    printf("\nEthernet : %s\n",pname);
    if (strncmp(pname, "xgbe1", sizeof(pname))) {
        /* SYNC timer set 100 second */
        ts.tv_sec += TX_RX_SYNC_TIME;        /* TX_RX_SYNC_TIME = 10*/
    } else { /* xgbe1 */
        /* Add more 10 seconds to wait console redirect */
        ts.tv_sec += (TX_RX_SYNC_TIME + 10);
    }
    ts.tv_nsec = 0;

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
        if (setup_eth_port(p_type, eth_port, &tx_skt) == FAIL) {
            return(FAILED);
        }
    
        if (setup_eth_port(p_type, eth_port, &rx_skt) == FAIL) {
            return(FAILED);
        }
    }

    /* extend the space for putting the dest/src mac address */
    /* SCV1 loop back testing did not care mac address */
    pkt_len += (2*sizeof(mac_addr_t));

    /* set up global value for both rx and tx on struct */
    strncpy(rx_info.name, pname,IFNAMSIZ);
    rx_info.speed = speed;
    rx_info.pkt_num = pkt_cnt;
    rx_info.pkt_len = pkt_len;
    rx_info.socket = rx_skt;

    /* build another thread for rx, and pass rx_info to rx */
    if(pthread_create(&threads, NULL, (void *)slave_receive_packets, 
                      (diag_info_pthread_t *) &rx_info)) {
        cterr('f',0, "pthread_create failed");
        return (FAILED);
    }

    for (ii = 0; ii < pkt_cnt; ii++) {

        /* wait for the rx receive the packet */
        rc = sem_timedwait(&rx_ready, &ts);
#ifdef DEBUG_SHOW
        show_buf_content(rx_info.pkt_len);
#endif
        if (rc != PASSED) {
        	  show_buf_content(rx_info.pkt_len);
    	      if (errno == ETIMEDOUT) {
    	          cterr('f',0, "sem_timedwait on rx_ready timeout.");
    	      } else {
    	          cterr('f',0, "semaphore wait on rx ready failed.");
    	      }
            return (FAILED);
        }

#ifdef DEBUG_SHOW
        show_tx_buf_content(g_lbpacket_len);
#endif
        /* the main thread prepare to sending packet. */
        rc = cpu1_send_packets(&tx_skt, g_lbpacket_len , value, eth_port);

        if (strncmp(pname, "gbe3", sizeof(pname))) {
            msleep(10);   /* for XAUI DUAL CPU running loopback test, ensure tx is slow than rx  */
        } else { /* xgbe1 */
            msleep(300);  /* to enusre PSE2 tx is more slow than rx when is loopback test continously */
        }

        if (rc == FAILED) {
            cterr('f',0, "send_packets failed");
            return (FAILED);
        }

      	/* tx already send out then inform rx for read next packet. */
        if (sem_post(&tx_cmp)){
            if (errno == EINVAL){
                cterr('f',0, "The sem(tx_cmp) does not refer to a valid semaphore");
            } else {
                cterr('f',0, "The function sem_post() is not supported by this implementation");
            }
            return (FAILED);
        }

    }  /* for */
    printf("\nPacket %d send %d bytes", ii , g_lbpacket_len);

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
    sem_destroy(&tx_cmp);

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: skye_phy_lpbk_test
 *
 * Description: SGMII port PHY internal or external loopback test
 *              internal lpbk test: tilera -> media PHY
 * 
 * Input: lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 *
 * Return: pass/fail
 *
 *------------------------------------------------------------------
 */
int
skye_phy_lpbk_test (int phy_port, int mode)
{
    uint port, speed, rc;
    uint start_speed, end_speed;
    int phy_id, lpbkmode = 0;
    uint speed_str = SPD_1000MBPS;
    phy_id = PHY_ID_88E1514;
    port = phy_port;
    speed = 0;


    prpass(testpass, "CPU0 <-> 88E1514 Transfer ");

    start_speed = ETH_MODE_FE10;
    end_speed = ETH_MODE_GE;
    /* SKIP FOR INTERNAL LOOPBACK 1G */
    if (mode == INT_LPBK)
        end_speed = ETH_MODE_FE100;

    if (diagflag_xram & D_SET_OPTIONS) {
        int choice;
        /* SPEED MENU */
        printf("\n0. 1000MBPS \n1. 100MBPS \n2. 10MBPS \n");
        printf("3. ALL");
        choice = getdec_answer("Please enter the speed you want: ",
                           0, 0, 3);

        switch (choice) {
        case 0:
            start_speed = ETH_MODE_GE;
            end_speed = ETH_MODE_GE;
            break;
        case 1:
            start_speed = ETH_MODE_FE100;
            end_speed = ETH_MODE_FE100;
            break;
        case 2:
            start_speed = ETH_MODE_FE10;
            end_speed = ETH_MODE_FE10;
            break;
        case 3:
            start_speed = ETH_MODE_FE10;
            end_speed = ETH_MODE_GE;
            break;
        default:
            printf("Unknown speed\n");
            return (FAILED);
        }
    }

    if (is_cpu0() == TRUE) {
        /* CPU 0 GBE port 5 */
        mac_addr_t mac_sa_gbe5 = {0x00, 0x55, 0x55, 0x55, 0x55, 0x55};
        memcpy(mac_sa, mac_sa_gbe5, sizeof(mac_addr_t));
    } else {
    /* CPU 1 GBE port 2 */
        mac_addr_t mac_sa_gbe2 = {0x00, 0x77, 0x77, 0x77, 0x77, 0x77};
        memcpy(mac_sa, mac_sa_gbe2, sizeof(mac_addr_t));
    }

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    for (speed = start_speed; speed <= end_speed; speed++) {
        if (speed == ETH_MODE_GE)
            speed_str = SPD_1000MBPS;
        else if (speed == ETH_MODE_FE100)
            speed_str = SPD_100MBPS;
        else
            speed_str = SPD_10MBPS;

    switch (mode) {
    case INT_LPBK:
        prpass(testpass, "PHY Internal Loopback at port %d "
                         "(speed %dMbps),", port,
                         ((speed == ETH_MODE_GE)? 1000 :
                         ((speed == ETH_MODE_FE100)? 100 : 10)));
        lpbkmode = SGMII_PHY_LPBK_INTERNAL;
        break;
    case EXT_LPBK:
        prpass(testpass, "PHY External Loopback at port %d "
                         "(speed %dMbps),", port,
                         ((speed == ETH_MODE_GE)? 1000 :
                         ((speed == ETH_MODE_FE100)? 100 : 10)));
        lpbkmode = SGMII_LPBK_NONE;
        /*
         * For 10BASE-T and 100BASE-TX modes, the loopback test requires no
         * register writes. For 1000BASE-T mode, register 18_6.3 must be set
         * to 1 to enable the external loopback.
         * All copper modes require an external loopback stub.
         */
        break;
    default:
        printf("Loopback %d not support\n", mode);
        return (FAILED);
    }

    rc = dev_88e1514_set_lpbk(speed, lpbkmode);
    if (rc == FAILED) {
        cterr('f',0,"set loopback failed \n");
        /* phy clean up */
        rc = dev_88e1514_cleanup_lpbk();
        if (rc == FAILED) {
            cterr('f',0,"clean up loopback failed \n");
            return (FAILED);
        }
        return (FAILED);
    }

    rc = tx_rx_diag(SEL_PORT_ETH, port, speed_str, 100,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
        cterr('f',0,"%s %d speed[%d] Packet transmit fail\n", SEL_PORT_ETH, port, speed_str);

        /* phy clean up */
        rc = dev_88e1514_cleanup_lpbk();
        if (rc == FAILED) {
            cterr('f',0,"clean up loopback failed \n");
            return (FAILED);
        }
        return (FAILED);
    } else {
        printf("\nPacket transmit pass\n");
    }

    /* phy clean up */
    rc = dev_88e1514_cleanup_lpbk();
    if (rc == FAILED) {
        cterr('f',0,"clean up loopback failed \n");
        return (FAILED);
    }

    } /*loop speed */

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: cpu0_send_packet_util
 *
 * Description: CPU0 send packets to CPU1 via XAUI interface.
 *              Then receive and compare packets from CPU1.
 * 
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_send_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye XAUI");
    prpass(testpass, "CPU0 Data Transfer ");
   
    port = 1;
    speed = SPD_10000MBPS;
    /* CPU 1 XGBE port 1 */
    mac_addr_t mac_sa_xgbe1 = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
    memcpy(mac_sa, mac_sa_xgbe1, sizeof(mac_addr_t));

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN); 
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); 

    /* For debug purpose could try to send 10 packet if the RX time out */
    rc = tx_rx_diag(SEL_PORT_XAUI, port, speed, 100,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);
    /* To make sure after one cycle TX will hold for 2 seconds until RX Ready */
    msleep(2000);
    if (rc == FAILED) {
      cterr('f',0,"%s %d speed[%d] Packet transmit fail\n", SEL_PORT_XAUI, port, speed);
      return (FAILED);
    } else {
      printf("\nPacket transmit pass\n");
      return (PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: cpu1_send_packet_util
 *
 * Description: CPU1 receive packets from CPU0.
 *              CPU1 copy Rx data to Tx buffer then send to CPU0.
 * 
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu1_send_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye XAUI");
    prpass(testpass, "CPU1 Data Transfer ");
   
    port = 1;
    speed = SPD_10000MBPS;
    /* CPU 0 XGBE port 1 */
    mac_addr_t mac_sa_xgbe1 = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
    memcpy(mac_sa, mac_sa_xgbe1, sizeof(mac_addr_t));
    
    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN); 
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); 

    /* For debug purpose could try to send 10 packet if the RX time out */
    rc = rx_tx_diag(SEL_PORT_XAUI, port, speed, 100,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
        cterr('f',0,"%s %d speed[%d] Packet loopback fail\n", SEL_PORT_XAUI, port, speed);
      return (FAILED);
    } else {
      printf("Packet loopback pass\n");
      return (PASSED);
    }
}

#ifdef SKYE_ENHANCED_ERR_MSG
void dump_cpu0_ge_bp_lp_reg (void)
{
    cterr_db_print("%s:%s:%d please update me.\n",
                   __FILE__, __FUNCTION__, __LINE__);
}
#endif

/*------------------------------------------------------------------
 *
 * Function: cpu0_ge_bp_lp_test
 *
 * Description: CPU0 send packets to platform via GE interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_ge_bp_lp_test (void)
{
    uint port, speed, rc;

    testname("Skye CPU 0 GE Backplane Loopback Test");
    prpass(testpass, "CPU0 Data Transfer ");

#ifdef SKYE_ENHANCED_ERR_MSG
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = SKYE_BP_GE1;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Tilera CPU", "Backplane");
	
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)dump_cpu0_ge_bp_lp_reg);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)skye_dump_volt_margins,
                       (PFV)skye_dump_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check GE Backplane Loopback setup",
                    "Check GE connection was fine.",
                    "Check GE status up or down.");
#endif   /* SKYE_ENHANCED_ERR_MSG */
	
    port = 4; /* Fix this for the port */ 
    speed = SGMII_SPEED_1000;   /* GE Interface */

    mac_addr_t mac_sa_gbe4 = {0x00, 0x44, 0x44, 0x44, 0x44, 0x44};
    memcpy(mac_sa, mac_sa_gbe4, sizeof(mac_addr_t));

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    /* gbe replace with the definition
     */
    rc = tx_rx_diag(SEL_PORT_ETH, port, speed, 100,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
      cterr('f',0,"%s %d speed[%d] Packet transmit fail\n", SEL_PORT_ETH, port, speed);
      return (FAILED);
    } else {
      printf("\nPacket transmit pass\n");
      prcomplete(testpass, errcount, (char *)0);
      return (PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: cpu0_pse2_lp_test
 *
 * Description: CPU0 send packets to platform via PSE2 interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_pse2_lp_test (void)
{
    uint port, speed, rc;
    testname("Skye CPU 0 --> PSE2 Loopback Test");
    prpass(testpass, "CPU0 PSE2 Data Transfer ");

    port = 3; /* Fix this for the port */
    speed = SGMII_SPEED_1000;   /* GE Interface */

    mac_addr_t mac_sa_gbe3 = {0x00, 0x33, 0x33, 0x33, 0x33, 0x33};
    memcpy(mac_sa, mac_sa_gbe3, sizeof(mac_addr_t));

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    /* gbe replace with the definition
     */
    rc = tx_rx_diag(SEL_PORT_ETH, port, speed, 100,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
      cterr('f',0,"Packet transmit fail\n");
      return (FAILED);
    } else {
      printf("\nPacket transmit pass\n");
      return (PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: cpu0_bp_rx_packet_util
 *
 * Description: CPU0 send packets to platform backplane Rx GE interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_bp_rx_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye BackPlan");
    prpass(testpass, "CPU0 Data RX ");
   
    port = 4;
    speed = SGMII_SPEED_1000;   /* GE Interface */
    
    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN); 
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN); 
    
    rc = rx_tx_diag(SEL_PORT_ETH, port, speed, 2, 
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);    
    
    if (rc == FAILED) {
      printf("Packet loopback fail\n");
      return (FAILED);
    } else {
      printf("Packet loopback pass\n");
      return (PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: cpu0_bp_xaui_rx_packet_util
 *
 * Description: CPU0 send packets to platform backplane Rx XAUI interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_bp_xaui_rx_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye BackPlan");
    prpass(testpass, "CPU0 XAUI Data RX ");

    port = 2;
    speed = SPD_10000MBPS;   /* XAUI Interface */

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    rc = rx_tx_diag(SEL_PORT_XAUI, port, speed, 3,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
      printf("Packet loopback fail\n");
      return (FAILED);
    } else {
      printf("Packet loopback pass\n");
      return (PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: cpu0_bp_pse2_rx_packet_util
 *
 * Description: CPU0 send packets to platform backplane Rx PSE2 interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_bp_pse2_rx_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye PSE2 BackPlan");
    prpass(testpass, "CPU0 Data RX ");

    port = 3;
    speed = SGMII_SPEED_1000;   /* GE Interface */

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    rc = rx_tx_diag(SEL_PORT_ETH, port, speed, 10,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
      printf("Packet loopback fail\n");
      return (FAILED);
    } else {
      printf("Packet loopback pass\n");
      return (PASSED);
    }
}

/*********************************************************************
 *
 * Function: decode_cmd
 * Description: Decodes the command from the host executes
 *              the appropriate test.
 * Inputs: None
 * Outputs: PASSED/FAILED
 *********************************************************************
 */
int
decode_cmd (uchar host_cmd)
{
    uint i, cmd_not_found;
    skye_cmd_table_t *skye_cmd_table_p;
    int ret_val = FAILED;

#ifdef DEBUG
	printf("\n ENTERED DECODE_CMD, host_cmd = 0x%08x \n", host_cmd);
#endif
    /*
     *  Init for command search and initialize timer for use in delays.
     */
    skye_cmd_table_p = skye_cmd_table;
    cmd_not_found = TRUE;

    /*
     *  Search command table for the command that was received.
     */
    i = 0;
#ifdef DEBUG
    printf("NUM_CMDS_FRM_HOST = %#x\n", NUM_CMDS_FRM_HOST);
#endif
    while (( i++ < NUM_CMDS_FRM_HOST) && cmd_not_found ) {
#ifdef DEBUG
	printf("skye_cmd_table_p = %#x\n", (unsigned int)skye_cmd_table_p);
#endif
	if ( skye_cmd_table_p->cmd == host_cmd ) {
	    ret_val = (skye_cmd_table_p->routine)();
	    cmd_not_found = FALSE;
	    break;
	}
	skye_cmd_table_p++;
    }

    /*
     *  If the command was not found, return msg to host that command
     *  could not be decoded.
     */
    if (cmd_not_found == TRUE) {
	ret_val = CMD_NOT_FOUND;
    }

    return ret_val;
}

/*------------------------------------------------------------------
 *
 * Function: cmd_respond
 *
 * Description: CPU0 send command to platform backplane Rx XAUI interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cmd_respond (void)
{
    uint port, speed;
    int ret_val, param;
    char pname[10];
    int ii;
    int tx_skt, rx_skt;
    int pkt_cnt, pkt_len;
    unsigned char read_buf[1600];
    unsigned char write_buf[1600];
    fe_packet_t *rx_pak;
    fe_packet_t *tx_pak;
    volatile unsigned char type, cmd;
    unsigned char to_host_param[10];

    pkt_cnt = 2;
    pkt_len = ETH_PKT_MAX_LEN - ETH_HDR_LEN;

    port = 3; // GBE3 for PSE2
    speed = SGMII_SPEED_1000;   /* GE Interface */

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

#ifdef DEBUG
    rc = rx_tx_diag(SEL_PORT_ETH, port, speed, 2,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);
#endif

    sprintf(pname,"%s%d", SEL_PORT_ETH, port);
    /* Judge to create XAUI raw socket or create ETH row sockt */
    if (speed == SPD_10000MBPS) {
        /* setup XAUI tx and rx socket */
        if (setup_xaui_port(port, &tx_skt) == FAIL) {
            return(FAILED);
        }

        if (setup_xaui_port(port, &rx_skt) == FAIL) {
            return(FAILED);
        }
    } else {
        /* setup ETH tx and rx socket */
        if (setup_eth_port(SEL_PORT_ETH, port, &tx_skt) == FAIL) {
            return(FAILED);
        }

        if (setup_eth_port(SEL_PORT_ETH, port, &rx_skt) == FAIL) {
            return(FAILED);
        }
    }

    while (1) {
    if (skye_receive_frames(&rx_skt, (uchar *)rx_packet, pkt_len) == PASSED) {

        memset((uchar *)read_buf, 0, 1600);

        for (ii = 0; ii < pkt_len; ii++) {
            read_buf[ii] = *(rx_packet+ii);
        }

        rx_pak = (fe_packet_t *)&read_buf[0];
	    type = rx_pak->data[0];
	    /* If it's a command, send ACK first */
	    if (type != SHRINKRAY_CMD) {
		printf("\nNot a valid command type = %d\n", type);
		continue;
	    }

	    cmd = rx_pak->data[1];
        param = (rx_pak->data[2] << 24) | (rx_pak->data[3] << 16) |
                (rx_pak->data[4] << 8 ) | (rx_pak->data[5]);
#ifdef DEBUG
	    printf("\ncmd = 0x%02x", cmd);
	    printf("\nparam = 0x%08x", param);
#endif
	    /* Clean up the tx packet */
	    memset((uchar *)write_buf, 0, ETH_PKT_MAX_LEN);

	    tx_pak = (fe_packet_t *)&write_buf[0];
	    tx_pak->data[0] = SHRINKRAY_ACK;
	    tx_pak->data[1] = cmd + TEST_ACK;
#ifdef DEBUG
	    printf("\ntx_pak->data[0] = 0x%02x", tx_pak->data[0]);
	    printf("\ntx_pak->data[1] = 0x%02x\n", tx_pak->data[1]);
#endif
        if (skye_send_frames(&tx_skt, (uchar *)tx_pak, pkt_len)) {
            printf("\nFailed to send frames to host\n");
            continue;
        }

        if (cmd == FROM_HOST_CPU_ALIVE_TEST) {
            continue;
        }

        if (cmd == FROM_HOST_SWITCH_CONSOLE) {
            break;
        }

        ret_val = decode_cmd(cmd);

#ifdef DEBUG
        printf("\nret_val = 0x%02x", ret_val);
#endif
        tx_pak->data[0] = SHRINKRAY_RESULT;
        tx_pak->data[1] = ret_val;
        tx_pak->data[2] = to_host_param[0];
        if (skye_send_frames(&tx_skt, (uchar *)tx_pak, pkt_len)) {
            printf("\nFailed to send frames to host\n");
            continue;
        }
    }
    }

    return (PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: cpu0_xaui_bp_lp_test
 *
 * Description: CPU0 send packets to platform backplane XAUI interface.
 *              Then receive and compare packets.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu0_xaui_bp_lp_test (void)
{
    uint port, speed, rc;

    prpass(testpass, "CPU0 XAUI Data Transfer ");

    port = 2; /* Fix this for the port */
    speed = SPD_10000MBPS; /* XAUI Interface */

    mac_addr_t mac_sa_xgbe2 = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
    memcpy(mac_sa, mac_sa_xgbe2, sizeof(mac_addr_t));

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    /* gbe replace with the definition
    */
    rc = tx_rx_diag(SEL_PORT_XAUI, port, speed, 10,
         ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
        cterr('f',0,"%s %d speed[%d] Packet transmit fail\n", SEL_PORT_XAUI, port, speed);
        return (FAILED);
    } else {
        printf("\nPacket transmit pass\n");
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }
}


/*------------------------------------------------------------------
 *
 * Function: cpu_host_10g_rx_tx_packet_util
 *
 * Description: Skye CPU0 receive packets from Platform CPU.
 *              Skye CPU0 copy Rx data to Tx buffer then send to Platform CPU.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu_host_10g_rx_tx_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye 10G-KR");
    prpass(testpass, "Data Transfer ");

    port = 2;
    speed = SPD_10000MBPS;
    /* Platform CPU ETH3 */
    mac_addr_t mac_sa_xgbe2 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(mac_sa, mac_sa_xgbe2, sizeof(mac_addr_t));

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    /* For debug purpose could try to send 10 packet if the RX time out */
    rc = rx_tx_diag(SEL_PORT_XAUI, port, speed, 10,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
        cterr('f',0,"%s %d speed[%d] Packet loopback fail\n", SEL_PORT_XAUI, port, speed);
      return (FAILED);
    } else {
      printf("Packet loopback pass\n");
      return (PASSED);
    }
}


/*------------------------------------------------------------------
 *
 * Function: cpu_host_1g_rx_tx_packet_util
 *
 * Description: Skye CPU0 receive packets from Platform CPU.
 *              Skye CPU0 copy Rx data to Tx buffer then send to Platform CPU.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
cpu_host_1g_rx_tx_packet_util (void)
{
    uint port, speed, rc;
    testname("Skye GE1");
    prpass(testpass, "Data Transfer ");

    port = 4;
    speed = SPD_1000MBPS;
    /* Platform CPU ETH3 */
    mac_addr_t mac_sa_xgbe2 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(mac_sa, mac_sa_xgbe2, sizeof(mac_addr_t));

    /* clean up buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    /* For debug purpose could try to send 10 packet if the RX time out */
    rc = rx_tx_diag(SEL_PORT_ETH, port, speed, 10,
                    ETH_PKT_MAX_LEN - ETH_HDR_LEN, 1);

    if (rc == FAILED) {
        cterr('f',0,"%s %d speed[%d] Packet loopback fail\n", SEL_PORT_XAUI, port, speed);
      return (FAILED);
    } else {
      printf("Packet loopback pass\n");
      return (PASSED);
    }
}


/*-------------------------------------------------
 * $Log: skye_ext_lpbk.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.5  2015/05/20 09:43:14  steja
 * Fix TLK missing code after code review <CDETS CSCuu01237>
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
 * Revision 1.1.2.13  2014/11/10 09:42:45  steja
 * Update TLK10232 10G KR loopback setup
 *
 * Revision 1.1.2.12  2014/09/18 07:51:47  steja
 * update speed minor issue when printf
 *
 * Revision 1.1.2.11  2014/09/17 11:13:15  palin2
 * Removed unused code.
 *
 * Revision 1.1.2.10  2014/09/15 07:58:59  steja
 * Code Clean up
 *
 * Revision 1.1.2.9  2014/09/03 03:36:22  steja
 * Enhanced cterr output
 *
 * Revision 1.1.2.8  2014/09/02 13:09:49  steja
 * Update Enhance error code for 88E1514
 *
 * Revision 1.1.2.7  2014/08/31 23:01:14  palin2
 * Updated enhanced error message FRU table offset.
 *
 * Revision 1.1.2.6  2014/08/31 15:59:28  steja
 * Add enhanced error messages
 *
 * Revision 1.1.2.5  2014/08/28 08:03:24  palin2
 * Update Skye show all temp. and all voltage margin states utilities to
 * support enhanced error message.
 *
 * Revision 1.1.2.4  2014/08/22 04:58:54  palin2
 * First check-in to enhance Skye error message.
 *
 * Revision 1.1.2.3  2014/08/14 12:24:25  steja
 * Remove debug message and add printf info for internal loopback
 *
 * Revision 1.1.2.2  2014/08/12 12:33:14  steja
 * Update 10GKR loopback test code
 *
 * Revision 1.1.2.1  2014/07/21 01:56:55  palin2
 * Initial check-in Skye module side Diag code.
 *
 *-------------------------------------------------
 * skye_ext_lpbk.c:
 * Revision 1.2.8.2  2014/06/25 13:10:20  steja
 * Add External loopback to test 10/100/1000 Mbps , Internal loopback 1000 Mbps still debugging
 *
 * Revision 1.2.8.1  2014/05/30 11:26:37  steja
 * Adjust timing issue on Xaui loopback test for both CPU (CSCup09786)
 *
 * Revision 1.2  2014/02/27 15:01:45  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.15  2014/02/27 04:13:18  steja
 * 1. move cpu0 tlk to tlk submenu utility
 * 2. modify msleep for pse2 and dual cpu lopback test
 *
 * Revision 1.1.4.14  2014/02/25 09:20:49  steja
 * debug frontpanel ge phy internal loopback intermittent fail.
 *
 * Revision 1.1.4.13  2014/02/07 04:48:02  steja
 * code clean up
 *
 * Revision 1.1.4.12  2014/01/28 13:49:45  steja
 * Update GE Frontpanel test code
 *
 * Revision 1.1.4.11  2014/01/14 07:45:44  steja
 * Add Verbose flag and update Marvel Test loopback
 *
 * Revision 1.1.4.10  2013/12/18 05:03:11  steja
 * 1. support PSE2 backplane loopback test
 * 2. support BIB change MAC address utility
 *
 * Revision 1.1.4.9  2013/11/29 07:08:55  steja
 * 1. Fix the full data path TLK working.
 * 2. add USB test
 * 3. add read BIB MAC utility
 *
 * Revision 1.1.4.8  2013/11/19 14:36:47  steja
 * Provide TLK utility for debugging
 * Update the BTK TLK into coded
 *
 * Revision 1.1.4.7  2013/11/05 09:17:54  steja
 * 1. Fix the MDIO not stable issue
 * 2. debug tlk log
 *
 * Revision 1.1.4.6  2013/10/10 00:36:22  steja
 * 1. Add TLK Utility PLL and Polarity TX RX switch
 * 2. Code update
 *
 * Revision 1.1.4.5  2013/09/29 09:54:27  iachang
 * Bring-up Dual CPU XAUI Interface
 *
 * Revision 1.1.4.4  2013/09/29 04:03:32  iachang
 * CPU0 GE Backplane RX Debug utility
 * Support 88E1514 initial function
 * Support 88E1514 Power Enable/Disable function
 *
 * Revision 1.1.4.3  2013/09/27 07:25:13  steja
 * update code for bringup
 *
 * Revision 1.1.4.2  2013/09/13 07:00:09  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.3  2013/07/15 09:05:43  steja
 * Add menu for CPU0 GE backplane loopback test
 *
 * Revision 1.1.2.2  2013/07/04 12:16:44  iachang
 * Support Dual CPU XAUI interface loopback test
 *
 * Revision 1.1.2.1  2013/06/24 09:03:34  steja
 * Checkin :
 * - Support TLK10323 Loopback test & Utility
 * - Support MV1514 Loopback test
 *
 *-------------------------------------------------
 * $Endlog$
 */
