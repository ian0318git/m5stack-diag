/* $Id: platform_eth_pkt_txrx.c,v 1.22 2020/05/22 02:28:34 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_eth_pkt_txrx.c,v $
 *------------------------------------------------------------------
 *
 * platform_eth_pkt_txrx.c - Platform dependent ethernet packet
 *   TX and RX functions. This is used to TX and RX packet on the
 *   host cpu SGMII ports.
 *
 * Oct 2011, Paul Tong
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>          /* getpid */
#include <strings.h>         /* for bzero*/
#include <string.h>
#include <errno.h>
#include <sys/types.h>       /* getpid */
#include <sys/socket.h>
#include <features.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>      /* htons */
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>   /*struct ethtool */
#include <linux/sockios.h>   /* SIOCETHTOOL */
#include <assert.h>   /* SIOCETHTOOL */

#include "types.h"
#include "proto.h"
#include "common.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "plat_defs.h"
#include "queryflags.h"
#include "eth_pkt_utils.h"
#include "platform_eth_pkt_txrx.h"
#include "dash_fpga.h"
#include "linux_api.h"
#include "platform_slot.h" /* get ngio_testing_now */

#undef DEBUG

static void system_eth_mac_addr_get(int, mac_addr_t *);
int create_raw_socket(int);
int bind_socket(char *, int, int);
int set_promisc(char *, int);
int clear_promisc(char *, int);
int eth_pkt_rx (eth_rx_pkt_t *);
int host_send_pkt_util(void);


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
    {0x50, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x60, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x70, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x80, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0x90, 0x40, 0x40, 0x40, 0x40, 0x40},
    {0xa0, 0x40, 0x40, 0x40, 0x40, 0x40},
};

/* These config str are the linux ifconfig command
 * for setting up the ethernet port to a know state.
 */
char *ifcfg_str[] = 
{ 
    "ifconfig ", /* eth0 not used in this test */
    ETH1_IFCONFIG_STR,
    ETH2_IFCONFIG_STR,
    ETH3_IFCONFIG_STR,
    ETH4_IFCONFIG_STR
};

/***********************************************************************
 * Function: system_eth_mac_addr_get
 *   Get the cavecreek ethernet port mac address from the linux file.
 *
 * Input:
 *   port - cavecreek sgmii port number
 *   mac_buf - pointer to buffer to store the mac address
 *
 * Return: void
 ************************************************************************
 */
static void system_eth_mac_addr_get (int port, mac_addr_t *mac_buf)
{
    char file_name[] = "/sys/class/net/eth1/address";
    FILE *stream_p;
    uchar macstr[] ="00:01:02:03:04:05";

    sprintf(file_name, "/sys/class/net/eth%d/address", port);

    stream_p = fopen(file_name, "r");
    if ( stream_p == NULL ) {
        printf("\nThe MAC address may not be programmed properly.");
        cterr('f', 0," The file `/sys/class/net/eth%d/address' doesn't exist.\n ", port);
    } else {
        fscanf(stream_p, "%s", macstr);
        fclose(stream_p);
    }

    macstr2macaddr(macstr, mac_buf);
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
    int sgmii;
    int min_port, max_port;

    if (is_overlord() || is_juno() || is_goldbeach() || is_vg400()) {
        max_port = CPU_SGMII_PORT3;
	min_port = CPU_SGMII_PORT0;
    }
    else if (is_usd_machines()) {
        max_port = min_port = CPU_SGMII_PORT3;
    }
    else if (is_curie_1ru() || is_curie_2ru()) {
        /* curie 1ru/2ru using br0 as bridge to bundle nim/sm
         * eth ports */
        system_mac_addr_get("br0", &local_mac_addr[0]); 
        return; 
    }
    else {
        /* Neptune, Triton, Proteus, Neso x86 kernel has 3 eth ports */
        max_port = CPU_SGMII_PORT2;
	min_port = CPU_SGMII_PORT0;
    }

    for (sgmii= min_port; sgmii <= max_port; sgmii++) {
        /* Get the real mac address from kernel file and set it into
	 * local_mac_addr for diaglinux to use each port's real mac
	 * address.
	 */
        system_eth_mac_addr_get(sgmii, &local_mac_addr[sgmii]);
    }
}

/*-------------------------------------------------------------------
 *
 * Function : get_ngio_testing_now
 * Description: weak function for neptune and previous platforms.
 * INPUT:  dummy -- not used.
 * OUTPUT: return TRUE or FALSE
 * -------------------------------------------------------------------
 */
int get_ngio_testing_now (void)
    __attribute__((weak, alias("__get_ngio_testing_now")));
int __get_ngio_testing_now (void)
{
    printf("%s : is not support\n", __FUNCTION__);
    return (0); 

}

/***********************************************************************
 * Function: get_ctrl_plane_sgmii_port
 *   API for module code.
 *   Return the CPU SGMII port used for platform control plane data
 *   GE path conneted to the GESW
 *
 * Input: void
 *
 * Return: one of the value of CPU_SGMII_PORT1 to CPU_SGMII_PORT3
 ************************************************************************
 */
int get_ctrl_plane_sgmii_port (void)
{
    int ctrl_plane_sgmii_port = 0, now_testing;

    if (is_overlord() || is_juno() || is_ntpn_machines() || is_goldbeach() || is_vg450() || is_vg400()) {
       /* o2 juno */
       ctrl_plane_sgmii_port = CPU_SGMII_PORT1;

    } else if (is_curie_1ru()) {
       /* curie 1ru NIM1 GE0=eth7, no GE1; SM1 GE0=eth6, GE1=eth8 */
       now_testing = get_ngio_testing_now(); 
       switch (now_testing) {     
       case NOW_TESTING_NIM1: 
           ctrl_plane_sgmii_port = CPU_SGMII_PORT7;
       break; 
       case NOW_TESTING_SM1: 
           ctrl_plane_sgmii_port = CPU_SGMII_PORT6;
       break; 
       case NOW_TESTING_SM1_NIM1:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT6;
       break;
       case NOW_TESTING_SM1_NIM2:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT8;
       break;
       }
    } else if (is_curie_2ru()) {
        /* curie 2ru NIM1 GE0=eth7, no GE1; NIM2 GE0=eth6, no GE1
         * SM1 GE0=eth11, GE1=eth9, SM2 GE0=eth10, GE1=eth8 */
        now_testing = get_ngio_testing_now();
        switch (now_testing) {
        case NOW_TESTING_NIM1:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT7;
            break;
        case NOW_TESTING_NIM2:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT6;
            break;
        case NOW_TESTING_SM1:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT11;
            break;
        case NOW_TESTING_SM2:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT10;
            break;
        case NOW_TESTING_SM1_NIM1:
                ctrl_plane_sgmii_port = CPU_SGMII_PORT11;
            break;
        case NOW_TESTING_SM1_NIM2:
                ctrl_plane_sgmii_port = CPU_SGMII_PORT9;
            break;
        case NOW_TESTING_SM2_NIM1:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT10;
            break;
        case NOW_TESTING_SM2_NIM2:
            ctrl_plane_sgmii_port = CPU_SGMII_PORT8;
            break;
        }
    } else { 
        /* USD */
       ctrl_plane_sgmii_port = CPU_SGMII_PORT3;
    }

    return(ctrl_plane_sgmii_port);
}

/***********************************************************************
 * Function: get_local_mac_addr
 *   Return the eth port mac addrss stored in local_mac_addr to the caller
 *
 * Input:
 *   port - cavecreek sgmii port numner
 *   mac_buf - pointer to the buffer for output value
 *
 * Return: void
 ************************************************************************
 */
void get_local_mac_addr (int port, mac_addr_t *mac_buf)
{
    memcpy(mac_buf, &local_mac_addr[port], sizeof(mac_addr_t));
}

/***********************************************************************
 *
 * Function:	get_sgmii_port_num()
 *
 * Description:	Legacy code port from ISR-G2. Used by NGIO cards
 *              which also support ISR-G2 systems.
 *              Return the sgmii port number of the device attached
 *		to the sgmii port.
 *
 * Input:	port - 0 based port number of the type 
 *		type - TYPE_SWITCH
 *
 * Output:	-1   if the type is invalid or the port number is out of range
 *		else the sgmii port number for the device of type TYPE, and port
 *
 ************************************************************************
 */
int
get_sgmii_port_num (uint port, uint type)
{
#ifdef OBSOLETE /* Obsolete the ISR-G2 way */
    int retval;

    retval = -1;
    switch (type) {
    case TYPE_SWITCH:
	if (port < NUMBER_OF_CPU_SW_PORTS) {
	    retval = CPU_SGMII_PORT1 + port;
	} else {
	    cterr('f', 0, "%s Invalid Switch port %#x", __FUNCTION__, port);
	}
	break;
    default:
	break;
    }
    return(retval);
#else
    if (is_goldbeach() || is_vg400()) {
        /* CSCuy97616 : Goldbeach NIM1 = eth2, NIM2 = eht3 */
        return(get_ctrl_plane_sgmii_port() + port);
    } else {
        return(get_ctrl_plane_sgmii_port());
    }
#endif
}

/***********************************************************************
 * Name:	get_host_mac_addr
 *
 * Description:	get host mac addr
 *
 * Input:	port number, user array to store the address
 *
 * Output:	failed to get the number -1;
 *		OK : 0.
 *
 ***********************************************************************
 */
int
get_host_mac_addr (uint port, unsigned char *mac)
{
#ifdef OBSOLETE /* This old code does not work with Utah */
    /* Note: for Overlord, the cavecreek sgmii port that
     * is dedicated to support NGIO GE packets is sgmii 1.
     */
    get_local_mac_addr((CPU_SGMII_PORT1 + port), (mac_addr_t *)mac);
    return(0);
#else
    int sgmii_port = get_ctrl_plane_sgmii_port();
    if (is_goldbeach() || is_vg400()) {
        /* CSCuy97616 : Goldbeach NIM1 = eth2, NIM2 = eht3 */
        get_local_mac_addr((sgmii_port + port), (mac_addr_t *)mac);
    } else if (is_curie_1ru() || is_curie_2ru()) {
        /* Curie 1RU/2RU using bridge, br0, is the eth interface for
         * NIM/SM ge ports. Check local_mac_addrs_init() for detail.
         * */
        get_local_mac_addr(0, (mac_addr_t *)mac);
    } else {
        get_local_mac_addr(sgmii_port, (mac_addr_t *)mac);
    }
    return(0);
#endif
}

/***********************************************************************
 * Name:	get_host_port_ip
 *
 * Description:	API for module host code to get the ip address of
 *              the host SGMII port connected to the GE switch
 *
 * Input:	buffer point to hold to the returned ip address
 *
 * Output:	The str of the ip address
 *
 ***********************************************************************
 */
void get_host_port_ip(char *ip_str_buf)
{
    char *ip_str_p = HOST_ETH_IP_ADDR;

    sprintf(ip_str_buf, "%s", ip_str_p);
}

/***********************************************************************
 * Function: do_ifconfig
 *   Use the Linux "system" command to execute the ifconfig command.
 *   This function uses the ifcfg_str array which is only applicable
 *   to Overlord and Juno machine. For USD machines, use the 
 *   utah_do_ifconfig function.
 *
 * Input:
 *   eth_port - cavecreek sgmii port number
 *   option_str - options to add to the ifconfig command
 *
 * Return: void
 ***********************************************************************
 */
void do_ifconfig (int eth_port, char *option_str)
{
    char cmdbuf[128];

    sprintf(cmdbuf, "%s %s",
	    ifcfg_str[eth_port], (option_str == NULL) ? "" : option_str);
    system(cmdbuf);
}

/***********************************************************************
 * Function: utah_do_ifconfig
 *   Use the Linux "system" command to execute the ifconfig command.
 *   This function is only applicable to USD machines.
 *
 * Input:
 *   eth_port - cavecreek sgmii port number
 *   option_str - options to add to the ifconfig command
 *
 * Return: void
 ***********************************************************************
 */
void utah_do_ifconfig (int eth_port, char *option_str)
{
    char cmdbuf[128];
    int len;

    /* USD platformers only use SGMII-3 and the IP is 192.123.123.1
     */
    len = sprintf(cmdbuf, "ifconfig eth%d %s netmask 255.255.255.0 %s",
	    eth_port, HOST_ETH_IP_ADDR, (option_str == NULL) ? "" : option_str);
    if (len >= sizeof(cmdbuf)) {
        assert(!"utah_do_ifconfig. string too big");
    }
    system(cmdbuf);
}

/***********************************************************************
 * Function: cavecreek_sgmii_macsa_declare
 *   Cavecreek send out packets to the gesw to let it learn about
 *   the mac sa of the 3 sgmii port
 *
 * Input: none
 *
 * Return: void
 ***********************************************************************
 */
void cavecreek_sgmii_macsa_declare (void)
{
    char ethname[8], cmd[128];
    int sgmii, start_port, end_port, use_port;

    local_mac_addrs_init();

    if (is_overlord() || is_juno()) {
        start_port = use_port = CPU_SGMII_PORT1;
	end_port = CPU_SGMII_PORT3;
    }
    else {
        /* Neptune, Triton, Proteus, Neso */
        start_port = use_port = CPU_SGMII_PORT1;
	end_port = CPU_SGMII_PORT2;
    }

    for (sgmii= start_port; sgmii <= end_port; sgmii++) {
	do_ifconfig(sgmii, "arp -promisc up");
	msleep(100);
    }

    /* Use arping to send packet out to GE switch so that it has
     * the MAC address of the cpu ports
     */
    for (sgmii= start_port; sgmii <= end_port; sgmii++) {
	sprintf(ethname, "eth%1d", sgmii);
	sprintf(cmd, "arping -q -c 1 -I %s %s",
		ethname, LOCAL_ETH1_IP_ADDR);
	system(cmd);
	msleep(300);

	/* Only keep the use_port up for dhcp and tftp download.
	 * Keep the others in down state
	 */
	if (sgmii != use_port) {
	    do_ifconfig(sgmii, "down");
	}
    }
}

/***********************************************************************
 * Function: ctrl_plane_sgmii_macsa_declare
 *   (Same as cavecreek_sgmii_macsa_declare() but is for platforms
 *   after overlord.)
 *   Control plane CPU send out packets to the gesw to let it learn about
 *   the mac sa of the cpu sgmii port connected to the GESW
 *
 * Input: none
 *
 * Return: void
 ***********************************************************************
 */
void ctrl_plane_sgmii_macsa_declare (void)
{
    char ethname[8], cmd[128];
    int sgmii;

    int ctrl_plane_sgmii_port = 0;

    if (is_overlord() || is_juno()) {
       /* o2 juno */
       ctrl_plane_sgmii_port = CPU_SGMII_PORT1;

    } else {
        /* USD */
       ctrl_plane_sgmii_port = CPU_SGMII_PORT3;
    }

    local_mac_addrs_init();

    sgmii = ctrl_plane_sgmii_port;
    utah_do_ifconfig(sgmii, "arp -promisc up");
    msleep(100);
    sprintf(ethname, "eth%1d", sgmii);
    sprintf(cmd, "arping -q -c 1 -I %s %s",
	    ethname, LOCAL_ETH1_IP_ADDR);
    system(cmd);
    msleep(300);
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
 * Return: 0 or -1
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
        return(-1);
    }
 
    /* Bind our raw socket to this interface */
 
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol); 

    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll))) == -1) {
        perror("Error binding raw socket to interface\n");
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
        perror("ioctl: SIOCSIFFLAGS get index interface");
        close(sock);
        return(-1);
    }
    
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to set promiscous mode");
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
    if (ioctl(sock,SIOCGIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS get interface index");
        close(sock);
        return(-1);
    }
    
    ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to clear promiscous mode");
        close(sock);
        return(-1);
    }

    return 0;
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
int tx_a_pkt (int socket, uchar *pkt, int pkt_len)
{
    int rv;

#if DEBUG
    printf("%s The tx packet is:\n", __FUNCTION__);
    display_pkt(pkt, pkt_len);
#endif

    /* Note: the socket buffer is limited to 1514 bytes
     */

    rv = write(socket, pkt, pkt_len);

    if (rv != pkt_len) {
        printf("\nsending %d bytes\n", pkt_len);
	perror("write command failed.");
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
int rx_a_pkt (int socket, uchar *buf_p, int buf_size)
{
    int rv;

    rv = read(socket, buf_p, buf_size);
#if DEBUG
    if (rv < 0) {
        perror("read command failed.");
    }
#endif
    return(rv);
}

/***********************************************************************
 *
 * Function:	setup_eth_dev()
 *
 * Description:	Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:	sgmii_port - host system sgmii port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:	PASS/FAIL
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
        return(FAIL);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
	printf("%s bind_socket() failed at intf %s\n",__FUNCTION__, eth_name);
        return(FAIL);
    }

    *socket = raw;
    return PASS;
}

/***********************************************************************
 *
 * Function:	cleanup_eth_dev
 *
 * Description:	Cleanup the Linux ethernet packet socket to prevent
 * either TX and RX.
 *
 * Input:	sgmii_port - sgmii port number to disable
 *              socket - The socket created in setup_eth_dev.
 *
 * Output:	PASS/FAIL
 *
 ************************************************************************
 */
int cleanup_eth_dev (char *if_name, int socket)
{

    if (clear_promisc(if_name, socket) == -1) {
        return(FAIL);
    }

    if (close(socket) == -1) {
        return(FAIL);
    }

    return(PASS);
}

/***********************************************************************
 *
 * Function:	eth_pkt_tx
 *
 * Description:	This function will setup and transmit an ethernet packet
 *		from CPU which is attached to the backplane GE switch.
 *		Then check for no transmission errors .
 *
 * Input:	eth_tx_pkt_t - pointer to structure holding tx packet info
 *		  dest_addr    - destination MAC address
 *		  src_addr     - source MAC address.  If the contents
 *		                 equal zero, then use the host MAC address.
 *		  pkt_type     - ethernet packet type field
 *		  bufr_st_addr - buffer address to user tx buffer
 *		  payload_size - size of tx data
 *                socket  - Linux socket ID of the TX port
 *
 * Output:	0    if the packet is transmitted without errors
 *              != 0 if the packet is transmitted with errors
 *
 ************************************************************************
 */
int eth_pkt_tx (eth_tx_pkt_t *tx_pkt_p)
{
    uchar *cptr, *buf_p;
    uchar pkt[XAUI_PKT_BUF_LEN];
    int pkt_len;
    uint crc;

    memset(pkt, 0, XAUI_PKT_BUF_LEN);
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
 * Function:	rbcp_eth_pkt_rx
 *
 * Description:	This function will check to see if an ethernet packet
 *		was received by the GEMAC, which is attached to the backplane
 *		GE switch.
 * 		If a receive packets is received, then check for good
 *		receive status and, if it is good, copy the receive
 *		packet to the user supplied buffer.
 *		There is a check on the size of the user supplied
 *		buffer; if the size of the receive packet is larger
 *		than the size of the user supplied buffer, then a
 *		buffer overflow error will be flagged and the receive
 *		data will be truncated to fit the size of the user buffer.
 *		The receive status will be checked to see if there are
 *		any receive errors and is returned to the user in the
 *		eth_rx_pkt_t structure member, rx_status.
 *
 * Input:	eth_rx_pkt_t - pointer to structure holding rx packet info
 *		  bufr_st_addr - buffer address to of where to put rx buffer
 *		  rx_bufr_size - size of user rx buffer
 *		  pkt_size     - 0
 *		  pkt_num      - packet number, optional
 *		  wait_time    - time to wait for rx packet, in usec
 *                socket  - Linux socket of the RX port
 *
 * Output:	PASSED  if a packet is received without errors
 *		FAILED if a packet is received with errors
 *		rx_pkt_p->pkt_size contains the size of the rx packet
 *		rx_pkt_p->rx_status contains the receive buffer
 *			descriptor status word
 *
 ************************************************************************
 */
int
rbcp_eth_pkt_rx (eth_rx_pkt_t *rxpkt_p)
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
#if DEBUG
        printf("%s receive socket timeout.\n", __FUNCTION__);
	perror("eth_pkt_rx perror value:");
#endif
	return(ETH_NO_PKT_RX);
    }

    if (rv > ETH_MAX_LEN) {
#if DEBUG
        printf("%s rx packet size error.\n", __FUNCTION__);
	perror("eth_pkt_rx perror value:");
#endif
        return(ETH_PKT_RX_ERR);
    }

    /* The called of eth_pkt_rx shoule know about the CRC.
     * Subtract 4 byte from the size
     */
    rxpkt_p->pkt_size = rv - ETH_CRC_LEN;
    rxpkt_p->pkt_num += 1;

#if DEBUG
    printf("%s RX packet %d is:\n", __FUNCTION__, rxpkt_p->pkt_num);
    display_pkt(rxpkt_p->bufr_st_addr, rxpkt_p->pkt_size);
#endif

    return (ETH_PKT_RX_OK);
}

/***********************************************************************
 *
 * Function:	eth_pkt_rx
 *
 * Description:	This function will check to see if an ethernet packet
 *		was received by the GEMAC, which is attached to the backplane
 *		GE switch.
 * 		If a receive packets is received, then check for good
 *		receive status and, if it is good, copy the receive
 *		packet to the user supplied buffer.
 *		There is a check on the size of the user supplied
 *		buffer; if the size of the receive packet is larger
 *		than the size of the user supplied buffer, then a
 *		buffer overflow error will be flagged and the receive
 *		data will be truncated to fit the size of the user buffer.
 *		The receive status will be checked to see if there are
 *		any receive errors and is returned to the user in the
 *		eth_rx_pkt_t structure member, rx_status.
 *
 * Input:	eth_rx_pkt_t - pointer to structure holding rx packet info
 *		  bufr_st_addr - buffer address to of where to put rx buffer
 *		  rx_bufr_size - size of user rx buffer
 *		  pkt_size     - 0
 *		  pkt_num      - packet number, optional
 *		  wait_time    - time to wait for rx packet, in usec
 *                socket  - Linux socket of the RX port
 *
 * Output:	PASSED  if a packet is received without errors
 *		FAILED if a packet is received with errors
 *		rx_pkt_p->pkt_size contains the size of the rx packet
 *		rx_pkt_p->rx_status contains the receive buffer
 *			descriptor status word
 *
 ************************************************************************
 */
int
eth_pkt_rx (eth_rx_pkt_t *rxpkt_p)
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
#if DEBUG
        printf("%s receive socket timeout.\n", __FUNCTION__);
	perror("eth_pkt_rx perror value:");
#endif
	return(ETH_NO_PKT_RX);
    }

    if (rxpkt_p->rx_chk) {
        /* The called of eth_pkt_rx shoule know about the CRC.
         * Subtract 4 byte from the size
         */
        rxpkt_p->pkt_size = rv - ETH_CRC_LEN;

        if ((rv <= (ETH_HDR_LEN + ETH_CRC_LEN)) || (rv > ETH_MAX_LEN)) {
            printf("%s received under size packet of %d bytes\n", __FUNCTION__, rv);
            perror("eth_pkt_rx perror value:");
            return(ETH_PKT_RX_ERR);
        }
    } else {
        rxpkt_p->pkt_size = rv;
    }


    rxpkt_p->pkt_num += 1;

#if DEBUG
    printf("%s RX packet %d is:\n", __FUNCTION__, rxpkt_p->pkt_num);
    display_pkt(rxpkt_p->bufr_st_addr, rxpkt_p->pkt_size);
#endif

    return (ETH_PKT_RX_OK);
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
		 uchar seed, char inc_dec, int payload_len,
		 uchar *buf_p)
{
    int ii;
    uchar pat = seed;

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
 * Function: host_send_packet
 *   Send out a packet from the specified host sgmii port 
 *   to the gesw. This func
 *   This function serves as an example of how to use the APIs
 *   setup_eth_dev(), gen_eth_pkt(), and cleanup_eth() for NGIO
 *   diag to send a packet to the GESW which can be forward to
 *   the dedicated NGIO port.
 * 
 *
 * Input:
 *   sgmii_port - Cavecreek sgmii port number
 *   mac_da - mac da of the packet
 *   mac_sa - mac sa of the packet
 *   pkt_type - packet type field of the packet. e.g. 0x0800, 0x8100
 *   payload_size - the length of the payload
 *   seed - the first byte of the payload
 *   inc_dec - the increment or decremnet value to the seed
 *
 * Return: PASS/FAIL
 ***********************************************************************
 */
int host_send_packet (char *if_name, mac_addr_t mac_da,
                      mac_addr_t mac_sa, ushort pkt_type,
                      int payload_size, uchar seed, char inc_dec,
                      unsigned char *txpkt_buf)
{

    int tx_skt;
    eth_tx_pkt_t txpkt_s;
    int rv;

    if (setup_eth_dev(if_name, &tx_skt) == FAIL) {
        return(FAIL);
    }

    memcpy(txpkt_s.dest_addr, mac_da, 6);
    memcpy(txpkt_s.src_addr, mac_sa, 6);
    txpkt_s.pkt_type = pkt_type;
    txpkt_s.tx_status = 0; 
    txpkt_s.socket = tx_skt;
    txpkt_s.bufr_st_addr = &txpkt_buf[ETH_HDR_LEN]; /* point to payload */
    txpkt_s.payload_size = payload_size;

    gen_eth_pkt(txpkt_s.dest_addr, txpkt_s.src_addr, txpkt_s.pkt_type,
		seed, inc_dec, txpkt_s.payload_size, txpkt_buf);

    rv = eth_pkt_tx(&txpkt_s);

    if (rv != ETH_PKT_TX_OK) {
        printf("Error: declare macsa pkt TX failed\n");
        return(FAIL);
    }

    cleanup_eth_dev(if_name, tx_skt);

    if (rv != ETH_PKT_TX_OK) {
      return(FAIL);
    }

    return(PASS);
}

/***********************************************************************
 * Function: host_send_pkt_util
 *   An utility to allow user to send packet from the host sgmii port
 *   with specific macda and macsa. It can be a very useful tool to
 *   check packet forwarding from the host to a NGIO port during
 *   NGIO diag bringup.
 *
 * Input: none
 *
 * Return: PASS/FAIL
 ***********************************************************************
 */
int host_send_pkt_util (void)
{
    int port;
    mac_addr_t mac_da = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    mac_addr_t mac_sa;
    ushort pkt_type;
    int pkt_len, payload_size;
    uchar seed;
    char inc_dec;
    char msg_buf[128];
    char if_name[10];
    /* support xaui jumbo frame size */
    unsigned char txpkt_buf[10300];

    if (is_overlord() || is_juno()) {
        port = getdec_answer("Enter sgmii port (1-3)", 1, 1, 3);
    }
    else if (is_usd_machines()) {
        port = getdec_answer("Enter sgmii port (3)", 3, 3, 3);
    }
    else {
        port = getdec_answer("Enter host cpu ge port (1-2)", 1, 1, 2);
    }

    /* Just a reminder */
    if ((port != 1) && !is_usd_machines()) {
        printf("Note: Port eth%d need to be up to send traffic.\n", port);
    }

    sprintf(if_name, "eth%d", port);
    
    mac_addr_query("Default MAC_DA is (ff:ff:ff:ff:ff:ff)\n",
		   &mac_da);

    get_local_mac_addr(port, &mac_sa);
    sprintf(msg_buf, "Default MAS_SA is "
	    "(%02x:%02x:%02x:%02x:%02x:%02x)\n",
	    mac_sa[0], mac_sa[1], mac_sa[2], 
	    mac_sa[3], mac_sa[4], mac_sa[5]);
    mac_addr_query(msg_buf, &mac_sa);
    
    /* pkt_type is the 2 bytes after the macda+macsa
     */
    pkt_type = gethex_answer("Enter packet type field (nnnn)",
			     0x0800, 0, 0xffff);
    /* pkt_len is ethernet header + payload + crc
     */
    pkt_len = getdec_answer("Enter entire packet len (64-1514)",
			    ETH_MIN_LEN, ETH_MIN_LEN, ETH_MAX_LEN-4);

    seed = gethex_answer("Use byte pattern (0-ff)", 0, 0, 0xff);
    inc_dec = getdec_answer("pattern increment value (0-127)",
			    1, 0, 127);
    payload_size = pkt_len - ETH_HDR_LEN - ETH_CRC_LEN;

    host_send_packet(if_name, mac_da, mac_sa, pkt_type, 
		     payload_size, seed, inc_dec, txpkt_buf);

    return (PASS);
}

/***********************************************************************
 * Function: set_ctrl_plane_sgmii_for_ge_test
 *   The GESW vendor diag is affected if the three SGMII port of
 *   cavecreek is up. Need this function to set the ports down
 *   before running the GE switch test.
 *
 * Input:
 *   up - a flag to turn the ports on or off
 *
 * Return: void
 ***********************************************************************
 */
void set_ctrl_plane_sgmii_for_ge_test (int up)
{
    int sgmii, use_port;

    if (is_overlord() || is_juno()) {
        use_port = CPU_SGMII_PORT1;
        if (up) {
	    do_ifconfig(use_port, "up");
	    msleep(100);
	}
	else {
	    do_ifconfig(use_port, "down");
	}
	do_ifconfig(CPU_SGMII_PORT2, "down");
	do_ifconfig(CPU_SGMII_PORT3, "down");
    }
    else if (is_usd_machines()) {
        sgmii = CPU_SGMII_PORT3;

	if (up) {
	    utah_do_ifconfig(sgmii, "up");
	    msleep(100);
	}
	else {
	    utah_do_ifconfig(sgmii, "down");
	}
    }
    else {
        /* Neptune, Triton, Proteus, Neso */
        use_port = CPU_SGMII_PORT1;
        if (up) {
	    do_ifconfig(use_port, "up");
	    msleep(100);
	}
	else {
	    do_ifconfig(use_port, "down");
	}
	do_ifconfig(CPU_SGMII_PORT2, "down");
    }
}

int
test_txrx (char *passname, char *ifdest, char *ifsrc,
           int pkt_len, int max_pkt_len, char *filename)
{
    ushort pkt_type;
    eth_rx_pkt_t rxpkt_s;
    int rawsock;
    mac_addr_t mac_da;
    mac_addr_t mac_sa;    
    unsigned char seed;
    int status = PASS;
    char inc_dec;
    unsigned char rx_data[XAUI_PKT_BUF_LEN];
    unsigned char tx_data[XAUI_PKT_BUF_LEN];

    /* pkt_len is ethernet header + payload + crc
     */
    
    /* mac doesn't really matter */
    system_mac_addr_get(ifdest, &mac_da);
    system_mac_addr_get(ifsrc, &mac_sa);

    if ((status = setup_eth_dev(ifdest, &rawsock)) == FAILED) {
        /* this is setting/software error so should never happen. */
        return(FAIL);
    }

    /*
    if (set_promisc(ifdest, rawsock) == -1) {
        return(FAILED);
    }
    */
    rxpkt_s.wait_time = 3000000;  /* 3 sec wait */
    rxpkt_s.bufr_st_addr = &rx_data[0];
    rxpkt_s.pkt_num = 0;
    rxpkt_s.rx_bufr_size = pkt_len;// - ETH_CRC_LEN;
    rxpkt_s.socket = rawsock;
    rxpkt_s.rx_chk = 0;

    for (; pkt_len <= max_pkt_len; pkt_len++) {
        
        rxpkt_s.rx_bufr_size = pkt_len;// - ETH_CRC_LEN;
        
        memset(rx_data, 0, sizeof(rx_data));
        memset(tx_data, 0, sizeof(tx_data));
        seed = 0; 
        inc_dec = 1;
        pkt_type = 0x0800;
    
        if ((status = host_send_packet(ifsrc, mac_da, mac_sa, pkt_type, 
                             pkt_len, seed, inc_dec, tx_data))==FAIL) {
            goto out;
        }
        if (eth_pkt_rx((eth_rx_pkt_t *)&rxpkt_s) != ETH_PKT_RX_OK) {
            status=FAIL;
            goto out;
        }

        if ((!rxpkt_s.pkt_size) || (rxpkt_s.pkt_size != pkt_len) ) {
            printf("received length is %d; expected length is %d\n",
                   rxpkt_s.pkt_size, pkt_len);
            status = FAIL;
            goto out;
        } else {
            if (passname != NULL) {
                prpass(testpass, passname, rxpkt_s.pkt_size);
            }
            if (cmpbyte(rx_data, tx_data, pkt_len)==FAIL) {
                logfile(filename, (char *)tx_data, pkt_len);
                logfile(filename, (char *)rx_data, rxpkt_s.pkt_size);
                status = FAIL;
                break;
            } else {

            }
                       
        }
    }
    
out:    
    cleanup_eth_dev(ifdest, rawsock);
    return(status);

}

/******** History ******** 
$Log: platform_eth_pkt_txrx.c,v $
Revision 1.22  2020/05/22 02:28:34  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.21  2020/01/09 01:02:19  jiajliu
Merge Curie 2RU to main trunk

Revision 1.20  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.19  2018/08/30 06:59:55  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.18  2018/05/18 09:24:51  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.17  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.16.36.6  2017/11/27 06:08:41  leschen
Initial check in to support VG450.

Revision 1.16.36.5  2017/04/05 06:45:03  leschen
Sync with <ng_diag-tag-032917>

Revision 1.16.36.4  2017/03/13 07:43:31  leschen
Support Triton system.

Revision 1.16.36.3  2016/12/13 00:23:42  ptong
Added GESW port list util, host port send pkt to GESW test support for Neptune

Revision 1.16.36.2  2016/10/11 01:15:30  alpeng
update ctrl plane ge port number to gesw

Revision 1.16.36.1  2016/10/06 01:33:25  ptong
Init Greyhound switch properly for Neptune

Revision 1.16  2014/07/15 23:01:10  mcharon
in test_txrx: check passname. if null, don't call prpass

Revision 1.15  2014/06/10 23:40:03  mcharon
remove redundant call to close()

Revision 1.14  2014/05/03 14:52:49  mcharon
use IFNAMSIZE; cache uio dir name in uio_iiiio_reautils

Revision 1.13  2014/05/02 18:24:15  mcharon
replace strcpy with sprint when copying interface name to struct ifr

Revision 1.12  2014/05/01 13:43:16  mcharon
fix memory coruption which causes ngvm to fail during bind_socket

Revision 1.11  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.10  2014/04/11 02:15:14  danchung
Add error report when MAC address is setting to all 0xff and avoid the
segmentation fault.

Revision 1.9  2014/01/28 02:40:35  ptong
Host SGMII port to GE switch use a fix IP address of 192.123.123.1 to support NGIO module code

Revision 1.8  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.7  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.6  2013/10/07 21:12:12  ptong
Add get_ctrl_plane_sgmii_port() to support platform specific control plane SGMII port connection to GESW

Revision 1.5  2013/09/09 05:58:02  ptong
Replace set_cavecreek_sgmii_for_ge_test with set_ctrl_plane_sgmii_for_ge_test. Replace #ifdef UTAH with is_overlord() for platform dependent code

Revision 1.4  2013/09/06 22:56:19  ptong
Support Utah with ctrl_plane_sgmii_macsa_declare

Revision 1.3  2013/08/13 00:07:00  hroni
support Rangeley control plane SGMII to GE same port loopback test

Revision 1.2  2013/08/09 00:33:00  hroni
Utah uses SGMII port#4 for eth and xaui loopback test

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.11  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.10  2012/10/15 21:26:46  huanngo
Adding function set_promisc()

Revision 1.9  2012/10/02 23:18:27  ptong
Change options in the arping command

Revision 1.8  2012/09/14 01:12:38  ptong
Code clean up and add comments

Revision 1.7  2012/06/12 00:44:28  ptong
Accept packet smaller than 64 byte in eth_pkt_rx()

Revision 1.6  2012/06/05 11:44:37  palin2
Clean up compiler warnings.

Revision 1.5  2012/06/04 02:11:26  ptong
Only keep x86 SGMII-1 for DHCP and TFTP operations

Revision 1.4  2012/04/27 01:03:58  ptong
Minor changes

Revision 1.3  2012/04/24 08:30:14  hondwang
Add RBCP for Canis

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
