/* $Id: platform_ext_lpbk.c,v 1.6 2019/01/18 05:54:46 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_ext_lpbk.c
 * Description: Main file of TSN external loopback Diag.
 *
 * Copyright (c) 2016 ~ 2019 by Cisco Systems, Inc.
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
#include "plat_defs.h"
#include "platform_esw.h"
#include "platform_ge_phy.h"
#include "platform_smi.h"
#include "platform_ext_lpbk.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "platform_cpu.h"
#include "tsn_comm.h"
#include "platform_fpga.h"

void create_rx_buffer();
int phy_check_iface_up_with_speed(char *, int);
int wait_iface_link_stats(char *, int);
int setup_xaui_port(int, int *);
int tsn_check_link_status(char *, int);
int tsn_phy_soft_reset(int, boolean);
int tsn_sgmii_lpbk_test(int, int);
int force_linkup(boolean, int);
int sgmii_adv_full_duplex(boolean, int);
int direct_phy_soft_reset(int);
int set_bridge_phy_speed(int, int);
int woodlawn_err_clean_up(int);

int media_phy_ext_lpbk_test(int, int, int);
int tsn_reset_eth_phy(int);
int show_status_info(int);

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

/* Copper speed table */
static int ge_test_speed_tbl[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};

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
        return(FAILED);
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
    if((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
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
 * Function   : tsn_check_link_status
 * Description: Function to check linux up status from Linux information.
 * Inputs     : type -
 *              port - port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_check_link_status (char *type, int port)
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

    rc = tsn_check_link_status(iface_type, port);
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

/*******************************************************************************
 *
 * Function   : show_status_info
 * Description: Function to show port status.
 * Inputs     : port - setup port
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int show_status_info (int port)
{
    ushort result = 0;
    uint   speed = 0;
    int    reg_page = (int)PHY_PAGE(0);
    int    reg_addr = 0;
    ushort reg_val = 0, w_data = 0;

/* go to page 0 */
    reg_addr = (int)PHY_REG(22);
    w_data = (ushort)PHY_PAGE(0);
    
    /* Get value of Copper Auto-nego Adv. register(4_0) */
    reg_addr = (int)COP_AUTONEG_ADV_REG4;
    if (tsn_esw_phy_reg_rd(port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Page%d, Reg%d.\n",
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }
    printf("Port%d PHY Copper Auto-Nego Adv Reg(%d_%d) = 0x%04X.\n",
           port, reg_addr, reg_page, reg_val);

    /* Get value of Copper Specific Status register 1(17_0) */
    reg_addr = (int)COP_STATUS_REG17;
    reg_val = 0;
    if (tsn_esw_phy_reg_rd(port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Page%d, Reg%d.\n",
               __FUNCTION__, __LINE__, reg_page, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    result = ((reg_val & (ushort)COP_P0R17_SPEED) >> COP_P0R17_SPEED_OFFSET);

    switch (result) {
    case COP_P0R17_SPEED_1000:
        speed = SPD_1000MBPS;
    break;
    case COP_P0R17_SPEED_100:
        speed = SPD_100MBPS;
    break;
    case COP_P0R17_SPEED_10:
        speed = SPD_10MBPS;
    break;
    default:
        printf("Unknown speed value: %d.\n", result);
    break;
    }   
    prpass(testpass, "PHY Speed is %d Mbps", speed);

    if (reg_val & (ushort)COP_P0R17_DUPLEX_FULL) {
        prpass(testpass, "PHY is Full Duplex");
    } else {
        prpass(testpass, "PHY is Half Duplex");
    }
   
    if (reg_val & (ushort)COP_P0R17_COP_LINK_UP) {
        prpass(testpass, "Copper Link Up");
    } else { 
        prpass(testpass, "Copper Link Down");
    }
   
    if (reg_val & (ushort)COP_P0R17_GLOBAL_LINK_UP) {
        prpass(testpass, "Global Link Status is Up");
    } else { 
        prpass(testpass, "Global Link Status is Down");
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : cfg_phy_setting
 * Description: Function to config PHY register for speed directly,
 *              and switch page to let driver detect the setting.
 *              This function is not like set_port_speed() which is based on 
 *              ethtool and may let other Reg reset.
 * Inputs     : ifname - port name.
 *              speed - setup speed 
 *              duplex - turn full/half duplex
 *              autoneg - turn on/off autoneg        
 *              signal - signal
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int cfg_phy_setting (int port, int speed, int duplex,
                     int autoneg, boolean signal)
{
    int    reg_addr = 0;
    ushort wr_data = 0, reg_val = 0, spdset = 0;
    int    phy_smiaddr = 0;

    /* select page 0 or page 1 from signal */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;

        if (port == TSN_GE0_ETHNUM) {
            phy_smiaddr = (int)TSN_GE0_SMIADDR;
        } else if (port == TSN_GE1_ETHNUM) {
            phy_smiaddr = (int)TSN_GE1_SMIADDR;
        } else {
            printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, port);
            return (FAILED);
        }

        /* select page 0 or page 1 from signal */
        reg_addr = (int)PHY_REG(22);
        wr_data = (ushort)signal;
        if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
            printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                   __func__, __LINE__, wr_data);
            return (FAILED);
        }

        /* Read PHY control reg for current speed*/
        /* set speed, Reg [0_2.6, 0_2.13] = value */
        reg_addr = (int)PHY_REG(0);
        if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read Reg%d.\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

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
        spdset = reg_val & (ushort)0x2040;
        break;
    }

    /* Set speed bits */
    wr_data = reg_val & (ushort)(~0x2040); /* clear bit 6 and 13 (speed)  */
    wr_data |= spdset;
    
    /* Set duplex mode */
    if (duplex) {
        wr_data |= (ushort)0x100; /* full duplex */
    } else {
        wr_data &= (ushort)(~0x100); /* half duplex */
    }

    /* Set autoneg on or off */
    if (autoneg) {
        wr_data |= (ushort)0x1000; /* enable autoneg */
    } else {
        wr_data &= (ushort)(~0x1000); /* disable autoneg */
    }

    /* Write to the phy and read back immediate to make sure. */
    reg_addr = (int)PHY_REG(0);
       
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(0);
    reg_val = 0;
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if (wr_data != reg_val) {
        cterr('f', 0, "%s(): register setup failed. wrval = 0x%x rdval = 0x%x",
              __FUNCTION__, wr_data, reg_val);

        sleep(1);
        return (FAILED);
    }

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    if (speed != 0) {

        reg_addr = (int)PHY_REG(22);
        if(signal) {
            if (tsn_smi_write(phy_smiaddr, reg_addr,
                              (ushort)SIG_COPPER) != PASSED) {
                printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                       __func__, __LINE__, wr_data);
                return (FAILED);
            }

	    sleep(ETH_DRIVER_DELAY); /* link down here */

            if (tsn_smi_write(phy_smiaddr, reg_addr,
                              (ushort)SIG_FIBER) != PASSED) {
                printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                       __func__, __LINE__, wr_data);
                return (FAILED);
            }

        } else {
            if (tsn_smi_write(phy_smiaddr, reg_addr,
                              (ushort)SIG_FIBER) != PASSED) {
                printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                       __func__, __LINE__, wr_data);
                return (FAILED);
            }

	    sleep(ETH_DRIVER_DELAY); /* link down here */

            if (tsn_smi_write(phy_smiaddr, reg_addr,
                              (ushort)SIG_COPPER) != PASSED) {
                printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                       __func__, __LINE__, wr_data);
                return (FAILED);
            }

	}
	sleep(ETH_DRIVER_DELAY); /* link up here */ 
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

/*******************************************************************************
 *
 * Function   : tsn_phy_soft_reset
 * Description: Enable PHY reset, add swtich page flow.
 * Inputs     : ifname - port type
 *              portnum - port number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_phy_soft_reset (int port, boolean signal)
{
    int    reg_addr = 0;
    ushort wr_data = 0, reg_val = 0;
    int    repeat = 100;
    int    phy_smiaddr = 0;

    /* Reset reg 0_0.15=1 */
    /* Use signal to select page for copper or fiber */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    
    if (port == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (port == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, port);
        return (FAILED);
    }

    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(0);
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    wr_data = reg_val | (ushort)PHY_REG_BIT(15);
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    /* switch the page is needed*/
    reg_addr = PHY_REG(22);
    if (signal) {
        wr_data = (ushort)SIG_COPPER;
        if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
            printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                   __func__, __LINE__, wr_data);
            return (FAILED);
        }

        sleep(ETH_DRIVER_DELAY); /* link down here */

        wr_data = (ushort)SIG_FIBER;
        if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
            printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                   __func__, __LINE__, wr_data);
            return (FAILED);
        }

    } else {
        wr_data = (ushort)SIG_FIBER;
        if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
            printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                   __func__, __LINE__, wr_data);
            return (FAILED);
        }

        sleep(ETH_DRIVER_DELAY); /* link down here */
        wr_data = (ushort)SIG_COPPER;
        if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
            printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                   __func__, __LINE__, wr_data);
            return (FAILED);
        }
    }
    sleep(ETH_DRIVER_DELAY); /* link up here */ 

    /* Read back to check for reset done */

    do {
        msleep(10);
        reg_addr = (int)PHY_REG(0);
        if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read Reg%d.\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
        }
    } while((repeat-- > 0) && (reg_val & (ushort)PHY_REG_BIT(15)));
    if ((repeat == 0) && (reg_val &  (ushort)PHY_REG_BIT(15))) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}

/*******************************************************************************
 *
 * Function   : set_phy_stub
 * Description: Function to enable stub for external loopback test.
 * Inputs     : ifname - port type
 *              enable - enable/disable the Enable stub register 
 *              signal - Copper or Fiber
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_phy_stub (int port, boolean enable, boolean signal)
{
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    int    phy_smiaddr = 0;

    /* Based on Marvell88E1112 spec,
     * for 1000BASE-T mode, 16_6:5 must be set to 1
     * to enable the external loopback.
     */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)PHY_PAGE(6);

    if (port == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (port == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, port);
        return (FAILED);
    }

    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Get current value of 16_6 */
    reg_addr = (int)PHY_REG(16);
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if ((enable == EXT_LPBK) && (signal == SIG_COPPER)) {
        wr_data = (reg_val | (ushort)(PHY_REG_BIT(5)));
    } else {
        wr_data = (reg_val & (ushort)(~PHY_REG_BIT(5)));
    }
 
    reg_addr = (int)PHY_REG(16);
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);  /* can not mask, effect 1000 external lpbk test */

    reg_val = 0;
    reg_addr = (int)PHY_REG(16);
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if(reg_val != wr_data) {
        printf("%s: Failed because of data mismatch.\n"
               "read back = 0x%04X; but write in = 0x%04X\n",
               __FUNCTION__, reg_val, wr_data);
        return (FAILED);
    } 
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_speed
 * Description: Function to set init speed, autoneg, full duplex full
 *              via ioctl on ethtool.
 * Inputs     : device - device name (ex: "eth1" )
 *              sock - raw socket
 *              speed - select speed
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sig_set_speed (int port, int speed, boolean signal)
{
    int    reg_addr = 0;
    ushort wr_data = 0;
    int    phy_smiaddr = 0;

    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    
    if (port == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (port == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, port);
        return (FAILED);
    }

    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    wr_data = (((ushort)speed) |
               ((ushort)COP_CTRL_DUPLEX_FULL) |
               ((ushort)COP_CTRL_RESET));

    reg_addr = (int)PHY_REG(0);
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }
    msleep(1000);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_port_speed
 * Description: Function to set port speed.
 * Inputs     : port_type - port type
 *              speed - port speed
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sig_set_port_speed (int port, int speed, boolean signal)
{
    int get_speed;
   
     switch(speed) {
         case SPD_10MBPS:
             get_speed = 0x0;
         break;
         case SPD_100MBPS:
             get_speed = 0x2000;
         break;
         case SPD_1000MBPS:
             get_speed = 0x0040;
         break;   
         default:
             printf("%s: eth%d not support this speed %d\n",
                    __FUNCTION__, port, speed);
             return (FAILED);
         break;
    }
    
    if (sig_set_speed(port, get_speed, signal)) {
        printf("%s: eth%d set Speed %d failed.\n",
                __FUNCTION__, port, get_speed);
        return (FAILED);
    }

    if (set_promisc(port) != PASSED) {
        printf("%s: Failed to set eth%d promisc mode.\n", __FUNCTION__, port);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : sig_pwr_ctrl
 * Description: Enable the power of PHY.
 * Inputs     : ifname - port type
 *              enable - enable/disable the power up/down register 
 *              signal - select page for copper/fiber
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int sig_pwr_ctrl (int port, boolean enable, boolean signal)
{
    int    reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    int    phy_smiaddr = 0;

    /* Change page based on the signal(Copper/Fiber) */
    reg_addr = (int)PHY_REG(22);
    wr_data = (ushort)signal;
    
    if (port == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (port == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, port);
        return (FAILED);
    }

    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Set SGMII fiber ouput amplitude */
    if ((signal == SIG_FIBER) && (enable == ENABLE_SIG)) {
        reg_addr = (int)FIB_SPEC_CTRL_REG2;
        reg_val = 0;
        if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read Reg%d.\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
        }
    
        wr_data = ((reg_val & (ushort)(~FIB_OUTPUT_AMP_MSK)) |
                   (ushort)FIB_OUTPUT_AMP_VAL504);   /* TSN_DIAG */
        reg_addr = (int)FIB_SPEC_CTRL_REG2;
        if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
            printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
                   __func__, __LINE__, wr_data);
            return (FAILED);
        }
    }

    reg_addr = (int)PHY_REG(0);   /* reg 0 */
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }
    
    if (enable) {
        wr_data = (reg_val & (ushort)(~PHY_REG_BIT(11))); /* power up */
    } else {
        wr_data = (reg_val | (ushort)PHY_REG_BIT(11));  /* power down */
    } 
    
    /* we do not need to setup the same value */
    if (wr_data == reg_val) {
        return (PASSED);
    }
    
    reg_addr = (int)PHY_REG(0);
    if (tsn_smi_write(phy_smiaddr, reg_addr, wr_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, wr_data);
        return (FAILED);
    }

    /* Read back to make sure the write is complete */
    reg_addr = (int)PHY_REG(0);   /* reg 0 */
    reg_val = 0;
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    if (reg_val != wr_data) {
        printf("%s: Failed because of data mismatch.\n"
               "read back = 0x%04X; but write in = 0x%04X.\n",
               __FUNCTION__, reg_val, wr_data);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Functioni  : init_sgmii_env
 * Description: Function to init sgmii port.
 *              Link up port, ensure power up,
 *              and turn off other power then set speed.
 * Inputs     : pname - port 
 *              speed: current test speed   
 *              port - port
 *              lpbk_mode - loopback mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int init_sgmii_env (char *pname, int speed, int port, int lpbk_mode)
{    
    int autoneg = 0;

    /* Ensure turn off the fiber and turn on copper before test */	
    if (sig_pwr_ctrl(port, DISABLE_SIG, SIG_FIBER) != PASSED) {  
    	printf("Failed to disable FIBER.\n"); 
    	return (FAILED);
    }
    if (sig_pwr_ctrl(port, ENABLE_SIG, SIG_COPPER) != PASSED) {
    	printf("Failed to enable COPPER.\n"); 
    	return (FAILED);
    }

    /* Always init as 10Mbps, duplex full, autoneg off
     * to make it more stable for 1000MPB initialation.
     */
    if (sig_set_port_speed(port, SPD_10MBPS, SIG_COPPER) != PASSED) {
        printf("%s: Failed to set port speed %d.\n",
               __FUNCTION__, speed);
        return (FAILED);
    }

    /* internal loopback using force_linkup to ensure link stable,
     * external loopback can not use force_linkup, so using check link
     * to ensure the link is stable
     * woodlawn_cavium_is_linkup will return failed,
     * need to verify this one is necessary or not.
     */
    if (lpbk_mode == EXT_LPBK){
        if (tsn_check_link_status(SEL_PORT_ETH, port) != PASSED) {
            printf("%s: After init port, sgmii link up time out after 1 second\n",
                    __FUNCTION__);
        }
    }   

    /* To ensure the test stay on full duplex and set speed */
    if(speed == SPD_1000MBPS ) {
        autoneg = AUTONEG_ON;
    } else {
        autoneg = AUTONEG_OFF;
    }

    if (cfg_phy_setting(port, speed, FULL_DUPLEX,
                        autoneg, SIG_COPPER) != PASSED) {
        printf("%s: %s cfg_phy_setting failed speed is %d\n",
               __FUNCTION__, pname, speed);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : tsn_sgmii_lpbk_test
 * Description: Function to do TSN SGMII loopback test.
 * Inputs     : port: current test port   
 *              speed: current test speed   
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_sgmii_lpbk_test (int port, int speed)
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
            printf("Test eth%d, speed-%d, pkt-cnt(%#x),"
                   " pkt-len(%#x), pkt-val(%#x)\n",
                                    port, speed, pkt_cnt, pkt_len, pkt_val);
        fflush(stdout);                          
        }
                  
        /* To do the tx/rx loopback test */
        rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);
       
        if (rc == FAILED) {
            printf("%s(): tx_rx_diag failed Port: %d Speed: %d\n",
                  __FUNCTION__, port, speed);
            hkeepflags = orig_hkpflag;
            return (FAILED);
        }
    } /* typ_curr */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("eth%d speed %d PASSED.\n", port, speed);
    fflush(stdout);
    }
    
    hkeepflags = orig_hkpflag;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : set_media_phy_ext_lpbk
 * Description: initial and setup loopback type on sgmii for external lpbk 
 * Inputs     : type - port type
 *              port - port number
 *              lpbk_typ - internal or external
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_media_phy_ext_lpbk (char *type, int port, int speed)
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
        if (set_phy_stub(port, EXT_LPBK, SIG_COPPER) != PASSED){
            printf("%s(): %s set_phy_stub failed.\n", __FUNCTION__, pname);
            return (FAILED);
        }
    }

    if ((rc = tsn_phy_soft_reset(port, SIG_COPPER)) != PASSED){
        printf("%s(): %s tsn_phy_soft_reset failed\n", __FUNCTION__, pname);
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
    rc = tsn_check_link_status(SEL_PORT_ETH, port);
    if ((rc == FAILED)) {
        printf("%s(): %s sgmii link up time out\n", __FUNCTION__, pname);                                                        
        return FAILED;
    } 

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : media_phy_ext_lpbk_test
 * Description: This is the entry point for external loopback test only.
 * Inputs     : Port = test ethernet port
 *              speed = test speed 
 *              lpbkmode = SGMII mode/E_1000BASAE mode 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int media_phy_ext_lpbk_test (int port, int speed, int lpbkmode)
{
    switch(lpbkmode) {
        case SGMII_EXT_LPBK:
        case SGMII_INT_EXT_LPBK:

            /* setup loopback information */          
            if (set_media_phy_ext_lpbk(SEL_PORT_ETH, port, speed) != PASSED) {
                printf("%s: Failed to set loopback mode of port%d.\n",
                        __FUNCTION__, port);
                return (FAILED);
            }

            /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
            if (tsn_sgmii_lpbk_test(port, speed) != PASSED) {
                printf("%s: tsn_sgmii_lpbk_test failed\n", __FUNCTION__);
                return (FAILED);
            }
            return (PASSED);
        default:
            cterr('f',0," Invalid Loopback mode(0x%08X).\n", lpbkmode);
            return (FAILED);
    }
}

/*******************************************************************************
 *
 * Function   : tsn_ge_lpbk_test
 * Description: SGMII port PHY internal or external loopback test
 *              internal lpbk test: cavium->bridge PHY->media PHY
 * Inputs     : lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 * Outputs    : pass/fail
 *
 *******************************************************************************
 */
int tsn_ge_lpbk_test (int eth_num, int lpbk_mode)
{
    if (this_is_not_supernova()) {
        uchar mb_get_loc[FRU_SIZE] = {0};
        uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
        /*
         * 1. Subtests of the test function will reuse all variables
         * 2. All variables will be cleared automatically when
         *    entering and leaving each menu item.
         */
        /* Segment 1: PID | Unique_string : slot_info */
        fru_table_offset = MB;
        /* fru_table_offset should be set, otherwise, it will not */
        /* go to enhanced error message format in cterr() */
        /* set fru_table_offset to get the predefine value */
        /* or change mb_pid & mb_loc below */
        platform_get_pid((char *)mb_get_pid);
        strcpy((char *)mb_get_loc, "MB");
        platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
        platform_fru_table[fru_table_offset].location_string = mb_get_loc;

        /* Segment 2: Test step captured from prpass */
        /* Segment 3: Failure message captured from cterr */

        /* Segment 4: Components used */
        cterr_add_component("APM", "BCM63168", "DDR RAM");

        /* Segment 5: register and memory dump */
        //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

        /* Segment 6: Platform Environment initialized here*/
        //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

        /* Segment 7: Top 3 Debugging Steps */
        cterr_add_debug("Switch console to BCM side, and make sure BCM can reach "
                        "Host by typing \"ping 192.123.123.1\". "
                        "If ping fails, check two paths. "
                        "One is the interface between host and Marvell1512, the "
                        "other one is RGMII between the Marvell1512 and BCM63168.",
                        "Check the interface between the BCM63168 and the DDR3.",
                        "If there is no problem on these interfaces, replace one "
                        "DDR3 and redo the test.");
#endif
    }

    int rc = 0; 
    int retval = PASSED; 
    int try, retry_limit = 2;
    int total_port_ctr = 1, port_ctr, test_port;
    int total_speed_ctr, speed_ctr, test_speed;
    int ge_num = 0;

    /* get test envrionment variable */
    total_speed_ctr = sizeof(ge_test_speed_tbl) / sizeof(int);

    if (eth_num == TSN_GE0_ETHNUM) {
        ge_num = TSN_GE0;                
    } else if (eth_num == TSN_GE1_ETHNUM) {
        ge_num = TSN_GE1;                
    } else {
        cterr('f', 0, "%s(%d): TSN doesn't have GE%d.",
                      __func__, __LINE__, ge_num);
        return (FAILED);
    }

    for (port_ctr = 0; port_ctr < total_port_ctr; port_ctr++) {
            test_port = eth_num;

        for (speed_ctr = 0; speed_ctr < total_speed_ctr; speed_ctr++) {
            test_speed = ge_test_speed_tbl[speed_ctr];
        
            /* skip 1000Mbps on all the internal loopback test*/
            if(((lpbk_mode != SGMII_EXT_LPBK) && (lpbk_mode != SGMII_INT_EXT_LPBK))
                && (test_speed == SPD_1000MBPS)) {
                continue;
            }
    
            switch(lpbk_mode) {
            case SGMII_INT_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */

                testname("External Loopback");

                prpass(testpass, "Test GE%d(eth%d) speed-%d ", ge_num, eth_num, test_speed);

                for (try = 0; try < retry_limit; try++) {
                    rc = media_phy_ext_lpbk_test(test_port, test_speed, SGMII_INT_EXT_LPBK);

                    if ((rc == PASSED) || (try == (retry_limit - 1))) {
                        break;
                    } else {
                        printf("####### retry the test #########\n");
                        break;
                    }
                }

                if (rc != PASSED) {
                    cterr('f', 0, "Failed at GE%d ", ge_num);
                    retval = FAILED;
                }
            break;
            default:
                retval = FAILED;
                cterr('f',0," Invalid Loopback mode(0x%08X).\n", lpbk_mode);
                break;
            }
        } /*test_speed*/
    }/*test_port*/

    /* Reset PHY */
    if (tsn_reset_eth_phy(eth_num) != PASSED) {
        cterr('f', 0, "Failed to reset eth%d.", eth_num);
        retval = FAILED;
    }

#if DEBUG
    printf("*******End*******\n");
    system("date"); /* real time counter */
    printf("*****************\n");
#endif

    prcomplete(testpass, errcount, (char *)0);
    return (retval);
}


int tsn_reset_eth_phy (int eth_num)
{
    int    reg_addr = 0;
    ushort w_data = 0, reg_val = 0;
    int    ctr = 0;
    int    phy_smiaddr = 0;

    /* Confirm PHY back to normal mode: */
    /* 1. Disable stub mode (16_6:5 = 0) */
    reg_addr = (int)PHY_REG(22);
    w_data = (ushort)PHY_PAGE(6);
    
    if (eth_num == TSN_GE0_ETHNUM) {
        phy_smiaddr = (int)TSN_GE0_SMIADDR;
    } else if (eth_num == TSN_GE1_ETHNUM) {
        phy_smiaddr = (int)TSN_GE1_SMIADDR;
    } else {
        printf("%s(%d): TSN doesn't have eth%d.", __func__, __LINE__, eth_num);
        return (FAILED);
    }

    if (tsn_smi_write(phy_smiaddr, reg_addr, w_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, w_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(16);   /* 16_6 */
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    /* Disable STUB */
    w_data = (reg_val & (ushort)(~PG_P6R16_STUB_EN));

    reg_addr = (int)PHY_REG(16);
    if (tsn_smi_write(phy_smiaddr, reg_addr, w_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, w_data);
        return (FAILED);
    }

    msleep(1000);

    /* 2. Reset PHY */
    reg_addr = (int)PHY_REG(22);
    w_data = (ushort)PHY_PAGE(0);
    if (tsn_smi_write(phy_smiaddr, reg_addr, w_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, w_data);
        return (FAILED);
    }

    reg_addr = (int)PHY_REG(0);   /* 0_0 */
    if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read Reg%d.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Reg.%d = 0x%04X.\n", reg_addr, reg_val);
    }

    /* RESET PHY */
    w_data = (reg_val | (ushort)COP_CTRL_RESET);

    reg_addr = (int)PHY_REG(0);
    if (tsn_smi_write(phy_smiaddr, reg_addr, w_data) != PASSED) {
        printf("%s(%d): Failed to write PHY Page Reg.(%d).\n",
               __func__, __LINE__, w_data);
        return (FAILED);
    }

    for (ctr = 0; ctr < ETH_MAX_RETRY; ctr++) {
        reg_val = 0x8000;
        if (tsn_smi_read(phy_smiaddr, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read Reg%d.\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("[%d]Reg.%d = 0x%04X.\n", ctr, reg_addr, reg_val);
        }

        if ((reg_val & (ushort)COP_CTRL_RESET) == 0) {
            break;
        } else {
            if (ctr == (ETH_MAX_RETRY - 1)) {
                printf("%s: Failed to reset eth%d PHY.\n",
                       __FUNCTION__, eth_num);
                return (FAILED);
            } 
        }
        msleep(1000);
    }
    return (PASSED);
}

/*
$Log: platform_ext_lpbk.c,v $
Revision 1.6  2019/01/18 05:54:46  yungchen
Merge Supernova branch to the main trunk (CSCvn79871)

Revision 1.5  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.4.38.2  2018/10/31 10:20:04  hondwang
fix MB GE0 external loopback testing fail

Revision 1.4.38.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.4  2018/04/15 22:03:30  palin2
Merged Vulcan back to maintrunk.

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.2  2017/11/07 09:44:26  hondwang
Change test card test with 1000base-X

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:48  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/24 14:14:10  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.5.6.1  2017/06/18 07:52:27  hondwang
Modify all phy SMI bus function to support I2C bus

Revision 1.1.4.5.2.1  2017/07/17 13:54:44  palin2
Code cleanup.

Revision 1.1.4.5  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.4  2016/09/13 08:14:23  palin2
Added CPU to GE PHY MAC loopback test.

Revision 1.1.4.3  2016/06/30 14:06:32  steja
Pick up the latest from tsn-branch1

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.3  2016/06/29 14:14:51  palin2
1. Updated code to support TSN-M.
2. Added utility to set LAN PHY 1000Base-T Test mode.

Revision 1.1.2.2  2016/04/26 20:48:49  palin2
Updated code after bring up SFP external loopback test.

Revision 1.1.2.1  2016/04/22 12:28:36  palin2
Updated code after bring up GE PHY external loopback test.

$Endlog$
*/
 
