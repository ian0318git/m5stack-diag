/* $Id: diag_pkt_txrx_lib.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_pkt_txrx_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_pkt_txrx_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
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
#include "diag_smi_lib.h"
#include "diag_pkt_txrx_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_cpu_lib.h"
#include "diag_esw_lib.h"
#include "diag_moka_fpga_lib.h"
#include "dev_mrvl_ge.h"
#include "diag_ge_phy_lib.h"


void create_rx_buffer();
int phy_check_iface_up_with_speed(char *, int);
int wait_iface_link_stats(char *, int);
int setup_xaui_port(int, int *);
int plat_check_link_status(char *, int);
int force_linkup(boolean, int);
int sgmii_adv_full_duplex(boolean, int);
int direct_phy_soft_reset(int);
int set_bridge_phy_speed(int, int);
int woodlawn_err_clean_up(int);


int cfg_phy_setting(int, int, int, int, boolean);
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

mac_addr_t mac_da = {0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc};
mac_addr_t mac_sa = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56};

#define ETH_MAX_RETRY   10
#define ENHANCE_ERROR_MSG_RDY 1


/*******************************************************************************
 *
 * Function   : create_raw_socket
 * Description: Create the raw socket with specific protocol.
 * Inputs     : protocol - seclect protocol
 * Outputs    : rawsock - return created socket num / -1(FAILED)
 *
 *******************************************************************************
 */
int create_raw_socket (int protocol)
{ 
    int rawsock;
    if ((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol)))== -1) {
        perror("Error creating raw socket\n");
        exit(-1);
    }
    return (rawsock);
}

/*******************************************************************************
 *
 * Function   : bind_socket
 * Description: Bind raw socket to interface 
 * Inputs     : device - current port
 *              rawsock - socket
 *              protocol - select protocol
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int bind_socket (char *device, int rawsock, int protocol)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
 
    bzero((void *)&sll, sizeof(sll));
    bzero((void *)&ifr, sizeof(ifr));
 
    /* First Get the Interface Index  */ 
    strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
    if ((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
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
 
    if ((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1) {
        perror("Error binding raw socket to interface\n");
        return FAILED;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_promisc
 * Description: Function to set promisc mode.
 *              when program exit, this interface will still be promisc mode.
 *              we should disable promisc mode when we exit (ie, use atexit)
 * Inputs     : device
 *              sock
 * Outputs    : PASSED/FAILED
 *
 * Note: Will get the haft of packet from TX if failed to set promisc.
 *
 *******************************************************************************
 */
int set_promisc (int eth_num)
{
    char cmd[128];

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "ifconfig eth%d promisc", eth_num);
    system(cmd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : setup_eth_port
 * Description: Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 * Inputs     : sgmii_port - host system sgmii port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int setup_eth_port (int sgmii_port, int *socket)
{
    int  raw = -1;
    char eth_name[5];

    sprintf(eth_name, "eth%d", sgmii_port);

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return (FAILED);
    }

    /* Set socket to promiscuous mode */
    if (set_promisc(sgmii_port) == -1) {
        return (FAILED);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        return (FAILED);
    }
    *socket = raw;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : send_raw_packet
 * Description: set packet via socket.
 * Inputs     : rawsock - socket
 *              pkt - tx buffer
 *              pkt_len - size of packet
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int send_raw_packet (int rawsock, unsigned char *pkt, int pkt_len)
{
    int sent= 0;
 
    /* A simple write on the socket ..thats all it takes ! */
    if ((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
        return (PASSED);
    }
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : pkt_cmp
 * Description: Function to compare packet.
 * Inputs     : bufa -
 *              bufb -
 *              count -
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
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

/*******************************************************************************
 *
 * Function   : plat_check_link_status
 * Description: Function to check linux up status from Linux information.
 * Inputs     : type -
 *              port - port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_check_link_status (char *type, int port)
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

/*******************************************************************************
 *
 * Function   : check_pkt
 * Description: Compared the packet between the buffer of TX and RX.
 * Inputs     : pkt_len - Length of packet
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int check_pkt (int pkt_len) {
    if ((pkt_cmp(tx_packet, rx_packet, pkt_len)) != PASSED) {
        printf("%s: RX packet mismatched.\n", __FUNCTION__);
        return (FAILED);
    }
#if DEBUG
        printf("%s: RX packet matched.\n", __FUNCTION__);
#endif
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : chk_macaddr
 * Description: Function to compare MAC address from packet.
 * Inputs     : macaddr1
 *              macaddr2
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int chk_macaddr (uchar *macaddr1, uchar *macaddr2)
{
    return (pkt_cmp(macaddr1, macaddr2,6));
}

/*******************************************************************************
 *
 * Function   : send_packets
 * Description: Function to send packet from TX to RX.
 *              Some delay is needed if number of packet is too much.
 * Inputs     : len - packet length
 *              val - content of packet
 *              port - 
 *              speed - 
 * Outputs    : PASSED/FAILED
 * 
 *******************************************************************************
 */
int send_packets (int *socket, int len, char val, int port, int speed)
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

    sprintf(iface_type, SEL_PORT_ETH);

    rc = plat_check_link_status(iface_type, port);
    if (rc == FAILED) {
        printf("%s:%d:Port link up time out\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    if (!send_raw_packet(raw, (unsigned char *)tx_packet, len)) {
        printf("%s:%d:Failed to sending packet with Raw Socket\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }    
    
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : receive_packets
 * Description: Create socket, setup port speed for receiving packet.
 * Inputs     : get_info - pass arg for pthread, now is amount of packet.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
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

/*******************************************************************************
 *
 * Function   : show_buf_content
 * Description: Function to.
 * Inputs     : show_pkt_len - 
 * Outputs    : None
 *
 *******************************************************************************
 */
void show_buf_content (int show_pkt_len)
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

/*******************************************************************************
 *
 * Function   : tx_rx_diag
 * Description: Using Pthread to create another thread for rx.
 *              tx should wait for rx build. After tx send packet to rx
 *              tx also need to wait for rx get all the packet.
 *              the waiting mechanism is using semaphore. 
 *              the timeout value is set to 10.
 * Inputs     : p_type - port type
 *              eth_port - port number
 *              speed - test speed
 *              signal - test signal fiber or copper
 *              pkt_cnt - test packet count
 *              value - contain of speed
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tx_rx_diag (char* p_type, int eth_port, int speed,
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
    if (sem_init(&rx_ready, 0, 0) != PASSED) {
        printf("eth_port %d sem_init on rx_ready failed.\n", eth_port);
        return (FAILED);
    }
    
    if (sem_init(&rx_finish, 0, 0) != PASSED) {
        printf("eth_port %d sem_init on rx_finish failed.\n", eth_port);
        return (FAILED);
    }
    
    if (sem_init(&tx_cmp, 0, 0) != PASSED) {
        printf("eth_port %d sem_init on tx_cmp failed.\n", eth_port);
        return (FAILED);
    }
    
    sprintf(pname,"%s%d", p_type, eth_port);

    /* setup ETH tx and rx socket */
    if (setup_eth_port(eth_port, &tx_skt) != PASSED) {
        return (FAILED);
    }
    
    if (setup_eth_port(eth_port, &rx_skt) != PASSED) {
        return (FAILED);
    }

    /* extend the space for putting the dest/src mac address */
    pkt_len += (2 * sizeof(mac_addr_t));

    /* set up global value for both rx and tx on struct*/
    strncpy(rx_info.name, pname,IFNAMSIZ);
    rx_info.speed = speed;
    rx_info.pkt_num = pkt_cnt;
    rx_info.pkt_len = pkt_len;
    rx_info.socket = rx_skt;

    /* build another thread for rx, and pass rx_info to rx */
    if (pthread_create(&threads, NULL, (void *)receive_packets, (diag_info_pthread_t *) &rx_info)) {
        printf("%s:%d:Failed to create P-Thread\n", __FUNCTION__, __LINE__);
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
                memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
                memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
            } else {
                printf("%s: mismatch\n", __FUNCTION__);
                show_buf_content(rx_info.pkt_len);
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

    /* if failed, cancel the thread */
    if (rc != PASSED)
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



/*******************************************************************************
 *
 * Function   : plat_sgmii_lpbk_test
 * Description: Function to do SGMII loopback test.
 * Inputs     : port: current test port   
 *              speed: current test speed   
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plat_sgmii_lpbk_test (int port, int speed)
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

        /* To do the tx/rx loopback test */
        rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
       
        if (rc == FAILED) {
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            return (FAILED);
        }
    } /* typ_curr */
    
    hkeepflags = orig_hkpflag;

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_pkt_txrx_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
