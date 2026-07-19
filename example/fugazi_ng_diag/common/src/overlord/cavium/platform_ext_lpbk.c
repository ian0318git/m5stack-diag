/* $Id: platform_ext_lpbk.c,v 1.40 2013/02/26 01:48:42 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 * 
 * platform_ext_lpbk.c  
 * support PHY external loopback 
 * internal loopback: media PHY, bridge PHY and Cavium.
 *
 * Oct 2011 Alan Peng
 * Copyright (c) 2011-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
  
#include<stdio.h>
#include<stdlib.h>
#include <assert.h>
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
#include "eth_pkt_utils.h"
#include "platform_eth.h"
#include "platform_ext_lpbk.h"
#include "dash_fpga.h"

/* global */

static int eth_port_list[] = { SGMII0, SGMII1, SGMII2, SGMII3 }; 
static int eth_speed_list[] = { SPD_10MBPS, SPD_100MBPS , SPD_1000MBPS }; 

int ovld_set_media_phy_testmode(int, uint8_t);
int ovld_softreset_media_phy(int);

/* packet buffer */
char volatile tx_packet[ETH_PKT_MAX_LEN];
char volatile rx_packet[ETH_PKT_MAX_LEN];

/* for ctrl diag flow */
sem_t rx_ready, rx_finish, tx_cmp;


/* Packets to be used in sgmii port loopback tests
 * The array element info is:
 * { first byte valu, packet size, increment flag, number of packets }
 */
static pktdata_info_t pktdata[] = {
  {0xa0, ETH_PKT_MIN_LEN, H_INCFILL, 100},
  {0xa2, (ETH_PKT_MIN_LEN + 1), H_INCFILL, 100},
  {0xa4, ((ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN - 1)), H_INCFILL, 100},
  {0xa6, (ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN), H_INCFILL, 100},
};

mac_addr_t mac_da = {0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc};
mac_addr_t mac_sa = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56};


/* Based on 88E1548P_RevA0_Release-Notes_021313 from Marvell FAE,
 * Steps to enter 1548 PHY to Test Mode 1, 2 or 4 are:
 * 1. Write Page 0, Reg  9 = 0x1F00 (Set PHY to Master mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_phy_setup_t phy_testmode124_steps[] = {
    {OVLD_PHY_PAGE0, OVLD_PHY_REG9,  0x1F00, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0x9140, 0xFB40},
    {OVLD_PHY_PAGE4, OVLD_PHY_REG27, 0x3E80, 0xFFFF},
    {OVLD_PHY_PAGE6, OVLD_PHY_REG26, 0x8000, 0xFFA0}
};

/* Based on 88E1548P_RevA0_Release-Notes_021313 from Marvell FAE,
 * Steps to enter 1548 PHY to Test Mode 3 are:
 * 1. Write Page 0, Reg  9 = 0x1700 (Set PHY to Slave mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_phy_setup_t phy_testmode3_steps[] = {
    {OVLD_PHY_PAGE0, OVLD_PHY_REG9,  0x1700, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0x9140, 0xFB40},
    {OVLD_PHY_PAGE4, OVLD_PHY_REG27, 0x3E80, 0xFFFF},
    {OVLD_PHY_PAGE6, OVLD_PHY_REG26, 0x8000, 0xFFA0}
};

/*-------------------------------------------------------------------
 *
 * Function: byteswap32
 * 
 * wrapper function for dswap32. if we need to swap, this function wil
 * call dswap32.
 *
 * Input: int num; number to be swapped
 *
 * Output: org, swapped value
 *
 *-------------------------------------------------------------------
 */
static int
byteswap32 (int num)
{
    return num;
}

/*-------------------------------------------------------------------
 *
 * Function: reset_platform_ext_dev
 * 
 * Use the reset external device register to 
 * reset ext devices on platform
 *
 * Input: bit; bit mask representig device to be reset
 * bit is defined as
 * FPGA_EXT_GE_QUAD_RESET         0x1
 *
 * OUTPUT: none
 *
 *-------------------------------------------------------------------
 */
void
reset_platform_ext_dev (int bit)
{
    assert(dash_fpga);

    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    bit = byteswap32(bit);
    sys->ext_rst |= bit;
}

/*-------------------------------------------------------------------
 *
 * Function: unreset_platform_ext_dev
 *
 * Unreset the external device
 *
 * Input: bit - bit mask for the desired external device
 *
 * Return: void
 *
 *-------------------------------------------------------------------
 */
void
unreset_platform_ext_dev (int bit)
{
    assert(dash_fpga);
    sys_lvl_t *sys = (sys_lvl_t *)dash_fpga;
    bit = byteswap32(bit);
    sys->ext_rst &= ~bit;
}

/*
 * Function: reset_quad_phy
 *
 * Description: Reset the 1548 and 1340 PHYs
 *
 * Input: void
 *
 * Return: void
 */
void reset_quad_phy(void)
{
    printf("%s\n",__FUNCTION__);
    reset_platform_ext_dev (FPGA_EXT_GE_QUAD_RST);
    msleep(100);
    unreset_platform_ext_dev (FPGA_EXT_GE_QUAD_RST);
    msleep(100);

    /* Must init 1340
     */
    marvell_1340_init();
}

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
    int phy_addr = port + ADDR_BRIDGE_PHY;

    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, OVLD_PHY_PAGE4);
    /* per FAE: Make sure autoneg with the speed set at the 1548
     * (10, 100, 1000 accordingly)
     */
    ovld_phy_reg_wr(phy_addr, PHY_REG(0), 0x9140);
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
        return(FAILED);
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
        return(FAILED);
    }

    return(PASSED);
 
}

/*------------------------------------------------------------------
 *
 * Function: set_promisc
 *    set ethernet port in promisc mode.
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
        return(FAILED);
    }
    
    ifr.ifr_flags|=IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ifr)==-1) {
        perror("ioctl: SIOCSIFFLAGS to set promiscous mode");
        close(sock);
        return(FAILED);
    }

    return(PASSED);
 
}

/*----------------------------------------------------------------------
 *
 * Function:	setup_eth_port()
 *
 * Description:	Setup the Linux ethernet packet socket on the host for
 * either TX or RX
 *
 * Input:	sgmii_port - host system sgmii port to initialize
 *              *socket - pointer for passing the created socket ID
 *                        to the caller.
 * Output:	PASS/FAIL
 *
 *---------------------------------------------------------------------
 */
int setup_eth_port (int sgmii_port, int *sock)
{
    int raw;
    char eth_name[8];

    sprintf (eth_name, "eth%d", sgmii_port);

    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    if (raw == -1) {
        return(FAILED);
    }

    /* Set socket to promiscuous mode
     */
    if (set_promisc(eth_name, raw) == -1) {
        return(FAILED);
    }

    /* Bind raw socket to interface */
    if (bind_socket(eth_name, raw, ETH_P_ALL) == -1) {
        return(FAILED);
    }

    *sock = raw;

    return(PASSED);
}

/*
 * Function: pkt_cmp
 *
 * Description: Compare 2 packets
 *
 * Input: bufa - buffer of first packet
 *        bufb - buffer of 2nd packet
 *        count - number of bytes to compare
 *
 * Return: pass/fail
 */
int pkt_cmp (char volatile *bufa, char volatile *bufb, int count)
{
    int ib = 0, rc = PASSED;
    uchar *p1 = (uchar *)bufa;
    uchar *p2 = (uchar *)bufb;	

    for (ib = 0; ib < count; ib++, p1++, p2++) {
        if (*p1 != *p2) {
	    printf("failed on byte %d, first data = %02x, second data = %02x\n",(ib+1), *p1, *p2);
	    printf("print byte %d, first data = %02x, second data = %02x\n",(ib+2), *(p1+1), *(p2+1));
	    printf("print byte %d, first data = %02x, second data = %02x\n",(ib+3), *(p1+2), *(p2+2));
	    printf("print byte %d, first data = %02x, second data = %02x\n",(ib+4), *(p1+3), *(p2+3));
	    rc = FAILED;
	    break;
        }
    }
    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: is_media_phy_copper_linkup
 *   Check if copper link up
 *
 * Input:
 *   phy_addr - The MII phy id defined in cavium uboot
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int is_media_phy_copper_linkup(int phy_addr)
{
    short rdval;

    /* go to page 0 */
    if(ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
    	return(FAILED);

    rdval = ovld_phy_reg_rd(phy_addr, COP_STATUS_REG17);

    if (rdval & PHY_REG_BIT(10)) {
        return(TRUE);
    }
    else {
        printf("Media PHY copper link down. reg 17_0= %#.4x\n", rdval);
    	return(FALSE);
    }
}

/*------------------------------------------------------------------
 *
 * Function: is_media_phy_qsgmii_linkup
 *   Chenk if QSGMII interface with 1340 link up
 *
 * Input:
 *   phy_addr - The MII phy id defined in cavium uboot
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int is_media_phy_qsgmii_linkup(int phy_addr)
{
    short rdval;

    /* go to page 0 */
    if(ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, OVLD_PHY_PAGE4))
    	return(FAILED);

    rdval = ovld_phy_reg_rd(phy_addr, PHY_REG(17));

    if (rdval & PHY_REG_BIT(10)) {
        return(TRUE);
    }
    else {
        printf("Media PHY QSGMII link down. reg 17_4= %#.4x\n", rdval);
    	return(FALSE);
    }
}

/*------------------------------------------------------------------
 *
 * Function: is_bridge_phy_qsgmii_linkup
 *   Chenk if QSGMII interface with 1548 link up
 *
 * Input:
 *   phy_addr - The MII phy id defined in cavium uboot
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int is_bridge_phy_qsgmii_linkup(int phy_addr)
{
    short rdval;

    if(ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, OVLD_PHY_PAGE4))
    	return(FAILED);
    
    rdval = ovld_phy_reg_rd(phy_addr, PHY_REG(17));

    if (rdval & PHY_REG_BIT(10)) {
        return(TRUE);
    }
    else {
        printf("Bridge PHY QSGMII link down. reg 17_4= %#.4x\n", rdval);
    	return(FALSE);
    }
}

/*------------------------------------------------------------------
 *
 * Function: ovld_cavium_is_linkup
 *   Chenk linu up status from Linux information.
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ovld_cavium_is_linkup(int port){

    int timeout_counter = 100, is_link = FALSE;
    struct ifaddrs *if_list, *if_info;
    unsigned short flags;
    char pname[10];

    
    sprintf(pname,"eth%d", port);  
    
    while(1) {
    
        /* Get the interface information */
        if (getifaddrs(&if_list) < 0) {
            printf("Failed to get interface information: %s.\n",
            strerror(errno));
            return(FAILED);
        }
        if (if_list == NULL) {
            printf("No network interfaces were found.\n");
            return(FAILED);
        }

        for (if_info = if_list; if_info; if_info = if_info->ifa_next) {
            
            /* parse the port name */
            if (strncmp(if_info->ifa_name, pname, IFNAMSIZ) != 0)
            	continue;
            	 
             /* printf("%s ", if_info->ifa_name); */
            
             flags = if_info->ifa_flags;
             if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
               /* printf("up\n"); */
               fflush(stdout);
               is_link = TRUE;
               break;
                    
             } else {
               /*  printf("down\n");  */
               msleep(10);
               timeout_counter--;
               if (timeout_counter == 0)
                 return(FAILED);
  	     }

	     fflush(stdout);
        } /*for*/
        
        freeifaddrs(if_list);
        if (is_link == TRUE)
            break;        
    } /*while */

     return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: check_pkt
 *   Compared the packet between the buffer of tx and rx.
 *
 * Input: NONE 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int check_pkt(int pkt_len){

    if ((pkt_cmp(tx_packet, rx_packet, pkt_len)) == PASSED) {
#if DEBUG
        printf("%s() Rx packet matched\n", __func__); 
#endif
	return(PASSED);
    } else {
        printf("%s() Rx packet mismatched\n", __func__);
    	return(FAILED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: chk_macaddr
 *   Compared 2 mac address for matching.
 *
 * Input: NONE 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int chk_macaddr(uchar *macaddr1, uchar *macaddr2)
{
    return(memcmp(macaddr1, macaddr2, 6));
}

/*
 * Function: send_packets
 *
 * Description: Send packets out to a ethernet port
 *
 * Input: socket - socket ID associated with the port
 *        len - the length of the packet
 *        val - the val of the first byte of the payload
 *        port - the ethernet port number 0-3
 *
 * Return: pass/fail
 */
int send_packets(int *socket, int len, char val, int port)
{   
    int raw;
    uint mac_size, fil_len;
    unsigned char *cptr;
    int sent = 0;
    uint crc;
    int frame_len;

    raw = *socket;

    /* clean up the tx_packet buffer */
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    
    cptr = (unsigned char *)tx_packet;
    mac_size = sizeof(mac_addr_t);

    /* put in the destination/source mac address */
    memcpy(cptr, mac_da, sizeof(mac_addr_t));
    cptr += mac_size;
    memcpy(cptr, mac_sa, sizeof(mac_addr_t));
    cptr += mac_size; 
 
    /* fill the packet. the len is include the size of mac address 
     * we need to minus the size of mac address on len for filbyte
     */
    fil_len = (len - (2*mac_size)); 
    filbyte(cptr, fil_len, val);
    cptr += fil_len;

    /* Add crc after the payload
     */
    crc = ~crc32(~0, (unsigned char *)tx_packet, len);
    *cptr++ = (crc >> 24) & 0xff;
    *cptr++ = (crc >> 16) & 0xff;
    *cptr++ = (crc >> 8) & 0xff;
    *cptr++ = crc & 0xff;
    frame_len = len + ETH_PKT_CRC_LEN;

    sent = write(raw, (unsigned char *)tx_packet, frame_len);

    if (sent != frame_len) {
        printf("error on sending packet\n"); 
	return(FAILED);
    }
    return(PASSED);
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
 * Input: onoff - 1 is on. 0 is off
 *        port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int force_linkup(boolean onoff, int port)
{
    short rdval, wrval;

    /* go to page 0 */
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
    	return(FAILED);

    rdval = ovld_phy_reg_rd(port, COP_SPEC_CTRL_REG16);

    /*bit 10 for force link up*/
    if(onoff)
        wrval = rdval | SET_PHY_BIT10;  /* enable force link up*/
    else
        wrval = rdval & ~SET_PHY_BIT10;  /* restore force link up*/
   	
    if(ovld_phy_reg_wr(port, COP_SPEC_CTRL_REG16, wrval))
    	return(FAILED);

    sleep(ETH_DRIVER_DELAY);  

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: ovld_err_clean_up
 *   using this function when diag failed, to prevent the endless
 *   'Trying speed' message from ethtool. when external loopback 
 *   failed, ethtool will keep trying speed.
 *
 * Input:  port - port number
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ovld_err_clean_up(int port){
    
    char pname[10];

    sprintf(pname, "eth%d", port);
#if DEBUG
    if(show_status_info(port + ADDR_MEDIA_PHY)) {   
    	printf("Show status info failed. ");
    	return(FAILED);
    }
#endif 

    /* to prevent the endless message from ethtool setting */
    if(force_linkup(ENABLE, (port + ADDR_MEDIA_PHY))) {   
    	printf("force_linkup failed. ");
    	return(FAILED);
    }
    
    /* cfg port to 10 or 100 spd, or 1000 spd will fail on clean up, 
     * because force link up is not support on 1000 spd. 
     */
    sleep(ETH_DRIVER_DELAY); 
    if ((cfg_phy_setting(pname, SPD_10MBPS, FULL_DUPLEX, AUTONEG_OFF, SIG_COPPER))) {
    	printf("cfg_phy_setting failed \n");
    	return(FAILED); 
    }
    sleep(ETH_DRIVER_DELAY);
    

    /* after link up, disable the register. */
    if(force_linkup(DISABLE, (port + ADDR_MEDIA_PHY))) {   
    	printf("force_linkup failed. ");
    	return(FAILED);
    }
    
    return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: show_status_info
 *   print out critical port link up info from page 0 reg 17.
 *
 * Input: port - port number 0-3
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int show_status_info(int port)
{
    short rdval = 0, result = 0;
    uint speed = 0;

    /* go to page 0 */
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED);

    rdval = ovld_phy_reg_rd(port, COP_AUTONEG_ADV_REG4); /*advertisment register reg4*/

    printf("advertisement register rdval = 0x%x\n", rdval);

    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED);

    rdval = ovld_phy_reg_rd(port, COP_STATUS_REG17); /*status register reg17*/
   	   
    result = ((rdval & OVLD_PHY_SPEED_MSK) >> OVLD_PHY_SPEED_OFFSET);
    printf("status register rdval = 0x%x\n", rdval);

    switch(result) {
    case OVLD_PHY_SPD_1000: 
    	speed = SPD_1000MBPS;
    	break;
    case OVLD_PHY_SPD_100:
    	speed = SPD_100MBPS;
    	break;
    case OVLD_PHY_SPD_10:
    	speed = SPD_10MBPS;
    	break;
    default:
    	printf("Unknown value for speed \n");
    	break;
    }	
    
    printf("Current speed is %d \n", speed);
    
    printf("%s Duplex\n", (rdval & OVLD_PHY_DUPLEX) ? "Full" : "Half");
    printf("Copper Link %s\n", (rdval & OVLD_PHY_COP_LINK) ? "Up" : "Down");
    printf("Global Link Status: %s\n", (rdval & OVLD_PHY_GL_LINK_STA) ? "Up" : "Down");

    return(PASSED);	
}


/*------------------------------------------------------------------
 *
 * Function: cfg_phy_setting
 *   this function is not like set_port_speed() which is based on 
 *   ethtool and may let other reg reset.
 *   cfg_phy_setting will config PHY reg for speed directly 
 *   and switch the page to let driver detect the setting.
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
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return(FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);
    
    /* The following code is per the Marvell 88E1112c PHY */

    /* select page 0 or page 1 from signal */
    regnum = OVLD_PHY_PAGE22;
    phy_reg_wr(sk, &ethreq, regnum, signal); 

    /* Read PHY control reg for current speed*/
    /* set speed, Reg [0_2.6, 0_2.13] = value */
    regnum = OVLD_PHY_PAGE0; 
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
    	printf("register setup failed. wrval = 0x%x rdval = 0x%x\n", wrval, rdval);
        close(sk);
    	return(FAILED);
    }

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    if (speed != 0) {

        regnum = OVLD_PHY_PAGE22;  
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
    
    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: receive_packets
 *   This function receives the packets being looped back to RX
 *   from TX in the loopback test function tx_rx_diag. It is
 *   invoked in the pthread_create call in that function.
 *
 * Input:  get_info - data structure contain the rx socket and
 *                    packet info.
 *
 * Output: PASSED/FAILED via pthread_exit
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
	        printf("%s rx= %d socket %d read timeout. loop(ii)= %d, pkt_cnt = %d otherpkt_cnt= %d\n",
		       __FUNCTION__, rx, get_info->socket, ii, pkt_cnt, otherpkt_cnt);
		break; /* exit do loop */
	    }

            /* drop invalid packet */  
            if (chk_macaddr(&rx_pkt_buf[0], (uchar *)mac_da) != 0) {
	        otherpkt_cnt++;

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
        if (sem_post(&rx_finish)){
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
	    if (errno == ETIMEDOUT)
	        printf("sem_timedwait on tx_cmp timeout. \n");
	    else  
                printf("semaphore wait on tx_cmp failed. \n");
	    pthread_exit ((void *)FAILED);
        }
    }  /* for*/

    pthread_exit((void *)PASSED);
}

/*
 * Function: show_buf_content
 *
 * Description: Display the tx and rx packet buffer content.
 *              Used when test failed.
 *
 * Input: show_pkt_len - number of bytes to dump
 *
 * Return: void
 */
void show_buf_content (int show_pkt_len) {	
    uint ii;
    char volatile *tptr, *rptr;
    
    tptr = tx_packet;
    rptr = rx_packet;
		
    printf("\nstart of pkt print.\n");
    for (ii=0; ii < show_pkt_len; ii++) {
        if ((ii > 0) && (ii % 8) == 0) {
            printf("\n");
        }
        printf("tx:%02x rx:%02x  ", (uchar)tptr[ii], (uchar)rptr[ii]);
    }
    printf("\nend of pkt print.\n");
}

/*------------------------------------------------------------------
 *
 * Function: tx_rx_diag
 *   Using Pthread to create another thread for rx.
 *   tx should wait for rx build. After tx send packet to rx
 *   tx also need to wait for rx get all the packet.
 *   the waiting mechanism is using semaphore. 
 *   the timeout value is set to 10.
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
    int rc = FAILED;
    void  *pthr_rv = NULL;
    
    /* init the semaphore. */
    rc = sem_init(&rx_ready, 0, 0 );
    if (rc != PASSED) {
    	printf("sem_init on rx_ready failed.\n");
        goto exit_tx_rx_diag; 
    }
    
    rc = sem_init(&rx_finish, 0, 0 );
    if (rc != PASSED) {
    	printf("sem_init on rx_finish failed.\n");
        goto exit_tx_rx_diag;
    }
    
    rc = sem_init(&tx_cmp, 0, 0 );
    if (rc != PASSED) {
    	printf("sem_init on tx_cmp failed.\n");
        goto exit_tx_rx_diag;
    }
 
    sprintf(pname,"%s%d", p_type, eth_port);

    /* setup tx and rx socket */
    rc = setup_eth_port(eth_port, &tx_skt);
    if (rc != PASSED) {
    	printf("setup eth port for tx failed.\n");
        goto exit_tx_rx_diag;
    }
    
    rc = setup_eth_port(eth_port, &rx_skt);
    if (rc != PASSED) {
    	printf("setup eth port for rx failed.\n");
        goto exit_tx_rx_diag;
    }
   
    /* set up global value for both rx and tx on struct*/
    strncpy(rx_info.name, pname,IFNAMSIZ);
    rx_info.speed = speed;
    rx_info.pkt_num = pkt_cnt;
    rx_info.pkt_len = pkt_len;
    rx_info.socket = rx_skt;

    /* build another thread for rx, and pass rx_info to rx */
    rc = pthread_create(&threads, NULL, (void *)receive_packets, (diag_info_pthread_t *) &rx_info); 
    if (rc != PASSED) {
        printf("pthread_create failed \n");
        goto exit_tx_rx_diag;
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
       	    show_buf_content(pkt_len);
	    if (errno == ETIMEDOUT)
	        printf("rx_ready semaphore timeout for packet %d \n", ii);
	    else  
	        printf("rx_ready semaphore failed for packet %d \n", ii);
	    goto exit_tx_rx_diag;
        }

	msleep(1); /* ensure rx read is ready before tx */
        
        /* the main thread prepare to sending packet. */
        rc = send_packets(&tx_skt, pkt_len, value, eth_port); 
        if (rc != PASSED) {
            printf("send_packets failed \n");
            goto exit_tx_rx_diag;
        }

  	/* Add some more time to wait for sem rx_finish to be unlocked
	 */
	ts.tv_sec += TX_RX_SYNC_TIME;
        rc = sem_timedwait(&rx_finish, &ts);
        if (rc != PASSED) {
            show_buf_content(pkt_len);
	    if (errno == ETIMEDOUT)
	        printf("rx_finish semamphoe timeout for packet %d.\n", ii);
	    else  
	        printf("rx_finish semamphoe failed for packet %d.\n", ii);
	    goto exit_tx_rx_diag;
        }
        
        /* compare the packet on rx_packet and tx_packet */
        rc = check_pkt(pkt_len);
        if (rc != PASSED) {
	    printf("Packet %d check mismatch\n", ii);
            show_buf_content(pkt_len);
            goto exit_tx_rx_diag;
        }
	else {
	    /* inform rx for read next packet. */
            rc = sem_post(&tx_cmp);
            if (rc != PASSED) {
                if (errno == EINVAL){
		    printf("The sem(tx_cmp) does not refer to a valid semaphore \n");
		} else {
		    printf("The function sem_post() is not supported by this implementation\n");
		}
		goto exit_tx_rx_diag;
	    }
	}
    }  /* for */
    
exit_tx_rx_diag:

    /* if failed, cancel the thread */
    if(rc != PASSED)
        pthread_cancel(threads);

    /* Sync the tx and rx in here and check the rx is pass or fail */
    pthread_join(threads, (void **)&pthr_rv);

    if (pthr_rv != PASSED) {
        rc = FAILED;
        printf("tx_rx_diag receive failed\n");
    } else {
        rc = PASSED;  
    }

    close(tx_skt);
    close(rx_skt);
    sem_destroy(&rx_ready);
    sem_destroy(&rx_finish);
    sem_destroy(&tx_cmp);
    
    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: ovld_phy_soft_reset
 *   Enable PHY reset, add swtich page flow.
 *
 * Input: ifname - port type
 *        signal - 0 select copper page, 1 for fiber page
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ovld_phy_soft_reset(char *ifname, boolean signal)
{
    int sk;
    struct ifreq ethreq;
    ushort rdval, wrval, regnum;
    int repeat = 100;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return(FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY */
    /* Reset reg 0_0.15=1 */
    /* Use signal to select page for copper or fiber */
    regnum = OVLD_PHY_PAGE22; 
    phy_reg_wr(sk, &ethreq, regnum, signal);

    regnum = PHY_REG(0);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    wrval = rdval | SET_PHY_BIT15;
    phy_reg_wr(sk, &ethreq, regnum, wrval);

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    regnum = OVLD_PHY_PAGE22; 
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

    /* Read back to check for reset done */
    do {
      msleep(10);
      phy_reg_rd(sk, &ethreq, PHY_REG(0), &rdval);
    } while((repeat-- > 0) && (rdval & SET_PHY_BIT15));

    close(sk);

    if ((repeat == 0) && (rdval & SET_PHY_BIT15)) {
      return(FAILED);
    }
    else {
      return(PASSED);
    }
}


/*------------------------------------------------------------------
 *
 * Function: set_phy_stub
 *   Enable/disable stub for external loopback test.
 *
 * Input: ifname - port type
 *        enable - enable/disable the Enable stub register 
 *        signal - Copper or Fiber
 *
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
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY */
    /* enable phy stub for external loopback */
    /* set page 6 in reg 18 bit 3 */
    phy_reg_wr(sk, &ethreq, OVLD_PHY_PAGE22, OVLD_PHY_PAGE6);
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
      printf("reg mismatch rdval = 0x%x, wrval = 0x%x\n", rdval, wrval);
      return(FAILED);
    } else {
      return(PASSED);
    }
}

/*------------------------------------------------------------------
 *
 * Function: set_speed
 *   Set init speed , autoneg , duplex full via ioctl on ethtool 
 *
 * Input: device - device name (ex: "eth1" )
 *        sock - raw socket
 *        speed - select speed 
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
    if(ioctl(sock,SIOCETHTOOL,&ifr)==-1) {
      perror("ioctl: get info via ethtool");
      close(sock);
      return(FAILED);
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
    if(ioctl(sock,SIOCETHTOOL,&ifr)==-1) {
      perror("ioctl: set speed via ethtool");
      close(sock);
      return(FAILED);
    }
    
    return(PASSED);
    
}

/*------------------------------------------------------------------
 *
 * Function: set_port_speed
 *   init port status via ethtool.
 *
 * Input: port_type - string of "eth0", "eth1", etc
 *        speed - SPD_10MBPS to SPD_1000MBPS
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
   
     switch(speed){
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
    	   printf("not support this speed, %s \n", __FUNCTION__);
    	   return FAILED;
    	break;
    }
    
    /* Create the raw socket */
    raw = create_raw_socket(ETH_P_ALL);
    /* Bind raw socket to interface */

    if(bind_socket(pname, raw, ETH_P_ALL)){
      printf("Bind socket failed. ");
      return(FAILED);
    }

    if(set_speed(pname, raw, get_speed)) {
      printf("Set Speed failed. ");
      return(FAILED);
    }

    if(set_promisc(pname, raw)) {
      printf("Set promisc failed. ");
      return(FAILED);
    }

    close(raw);
 
    return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: sig_pwr_ctrl
 *   Enable the power of PHY.
 *
 * Input:  ifname - port type
 *   enable - enable/disable the power up/down register 
 *   signal - select page for copper/fiber
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
        return(FAILED);
    }

    strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1548P PHY
     * signal == 0 is for copper mode, 1 is for fiber mode
     */
    regnum = OVLD_PHY_PAGE22; /* test fiber diable copper, vice versa. */
    phy_reg_wr(sk, &ethreq, regnum, signal);


    /* Set SGMII fiber ouput amplitude
     */
    if ((signal == SIG_FIBER) && enable) {
        regnum = FIB_SPEC_CTRL_REG2; 
	phy_reg_rd(sk, &ethreq, regnum, &rdval);
        wrval = ((rdval & ~FIB_OUTPUT_AMP_MSK) | FIB_OUTPUT_AMP_VAL504);
	phy_reg_wr(sk, &ethreq, regnum, wrval);
    }

    regnum = COP_CTRL_REG0; /* reg 0 */
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    
    if (enable){
        wrval = rdval & ~SET_PHY_BIT11; /*power up*/
    } else {
        wrval = rdval | SET_PHY_BIT11;  /*power down*/
    } 
    
    /* we do not need to setup the same value */
    if (wrval == rdval){
       	close(sk);
        return(PASSED);
    }    	
    
    phy_reg_wr(sk, &ethreq, regnum, wrval);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
   
    if (rdval != wrval) {
        printf("reg mismatch rdval = 0x%x, wrval = 0x%x\n", rdval, wrval);
	close(sk);
        return(FAILED);
    }

    close(sk);
    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: init_sgmii_env
 *   init sgmii port. link up port, ensure power up and turn off other 
 *   power and set speed.
 *
 * Input:  pname - port 
 *         speed: current test speed   
 *         port - port 
 *         lpbk_mode - EXT_LPBK or INT_LPBK
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int init_sgmii_env(char *pname, int speed, int port, int lpbk_mode){
	
    int rc = 0, autoneg = 0;

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
    	printf("set_port_speed failed \n");
    	return(FAILED);
    }

    /* internal loopback using force_linkup to ensure link stable,
     * external loopback can not use force_linkup, so using check link
     * to ensure the link is stable 
     * ovld_cavium_is_linkup will return failed,
     * need to verify this one is necessary or not.
     */
    if (lpbk_mode == EXT_LPBK) {
      if ((rc = ovld_cavium_is_linkup(port)) != PASSED) {
	  printf("after init port, sgmii link up time out after 1 second \n");
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
    	printf("cfg_phy_setting failed \n");
    	return(FAILED); 
    }

    /* using check link to ensure the link is stable */
    if (lpbk_mode == EXT_LPBK) {
      if ((rc = ovld_cavium_is_linkup(port)) != PASSED) {
	printf("after config speed, sgmii link up time out after 1 second \n");
      }
    }

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: ovld_set_packet
 *   Set up packet info for tx_rx_diag function to start the test.
 *
 * Input:  port: current test port   
 *         speed: current test speed   
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ovld_set_packet(int port, int speed) {
	
    int pkt_len, pkt_val, pkt_cnt;
    int typ_curr, pkt_type;
    int rc = 0;
    uchar orig_hkpflag = hkeepflags;
    
    pkt_type = sizeof(pktdata)/sizeof(pktdata_info_t);

    printf("testing.");
    fflush(stdout);

    hkeepflags = orig_hkpflag;
    
    for(typ_curr = 0; typ_curr < pkt_type; typ_curr++) {
    /* each dot '.' means a pattern */
    printf(".");
    fflush(stdout);
         /* set packet */
         pkt_cnt = pktdata[typ_curr].send_count;  
         pkt_len = pktdata[typ_curr].len;
         pkt_val = pktdata[typ_curr].val;
         hkeepflags |= pktdata[typ_curr].hkpflags;
                  
         /* prepare to send packet */
         rc = tx_rx_diag(SEL_PORT_ETH, port, speed, pkt_cnt, pkt_len, pkt_val);

         if (rc != PASSED) 
           break;

    } /* typ_curr */	
 
    if (rc != PASSED){
        printf("tx_rx_diag failed Port: %d Speed: %d, rc = %d\n",port, speed, rc);
        show_status_info(port + ADDR_MEDIA_PHY);
	printf("%s failed\n", __FUNCTION__);       
#if DEBUG
	printf("======= dump 1548 phy\n");
	/* must do page 0 last
	 */
	phy_reg_show(port, ADDR_MEDIA_PHY, 4, DUMP_ONE_PAGE);
	phy_reg_show(port, ADDR_MEDIA_PHY, 18, DUMP_ONE_PAGE);
	phy_reg_show(port, ADDR_MEDIA_PHY, 0, DUMP_ONE_PAGE);

	printf("\n\n======= dump 1340 phy\n");
	phy_reg_show(port, ADDR_BRIDGE_PHY, 1, DUMP_ONE_PAGE);
	phy_reg_show(port, ADDR_BRIDGE_PHY, 4, DUMP_ONE_PAGE);
#endif
    } else {	      
        printf("Pass\n");
    }

    fflush(stdout);
    hkeepflags = orig_hkpflag;

    return rc;
}


/*------------------------------------------------------------------
 *
 * Function: set_media_phy_ext_lpbk
 *   initial and setup loopback type on sgmii for external lpbk 
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

    sprintf(pname,"%s%d", type, port);   

    /* init sgmii environment and set speed for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, EXT_LPBK)) != PASSED){
    	printf("init_sgmii_env failed \n");
    	return(FAILED);
    }


    /* 1GMbps external loopback need to setup*/
    if ((rc = set_phy_stub(pname, EXT_LPBK, SIG_COPPER)) != PASSED){
    	printf("set_phy_stub failed \n"); 
    	return(FAILED);
    }

    /* Per marvell FAE. Make sure bridge phy autoneg is on
     */
    bridge_phy_autoneg_on(port);

    /* ovld_phy_soft_reset will turn off Enable loopback reg */
    if ((rc = ovld_phy_soft_reset(pname, SIG_COPPER)) != PASSED){
    	printf("ovld_phy_soft_reset failed \n");
    	return(FAILED);
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY*3);	

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: media_phy_ext_lpbk_test
 *   This is the entry point for external loopback test only.
 *
 * Input: port - port number
 *        speed - 10, 100, or 1000MB
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int media_phy_ext_lpbk_test(int port, int speed) {
	  
    int rc = 0; 
    
    /* setup loopback information */          
    rc = set_media_phy_ext_lpbk(SEL_PORT_ETH, port, speed);
                   
    if (rc == FAILED) {
        printf("set_media_phy_ext_lpbk failed, port: %d \n", port);   
       	return(FAILED);
    }
            
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = ovld_set_packet(port, speed);
            
    if (rc != PASSED) {
        printf("ovld_set_packet failed %s\n", __FUNCTION__);
       	return(FAILED);
    }

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_macsec
 *   from Marvell Eng: setup internal loopback need to disable macsec 
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
    short rdval, wrval;
	
    /* go to page 18*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE18))
      return(FAILED);

    rdval = ovld_phy_reg_rd(port, GENERAL_CTRL2_REG27);
   
    if(onoff)
        wrval = rdval & ~SET_PHY_BIT13;  /*disable*/
    else
        wrval = rdval | SET_PHY_BIT13;  /*restore*/
   
    if(ovld_phy_reg_wr(port, GENERAL_CTRL2_REG27, wrval))
        return(FAILED);

    return PASSED;	
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
    short rdval, wrval;
    int repeat = 100;

    /* go to page 18*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE18))
        return(FAILED);

    rdval = ovld_phy_reg_rd(port, GENERAL_CTRL1_REG20);
   
    if(onoff)
        wrval = rdval & ~SET_AUTO_MEDIA;  /* QSGMII-to-Copper reg[2:0] = 0x000*/
    else
        wrval = rdval | SET_AUTO_MEDIA;  /* QSGMII-to-automedia reg[2:0] = 0x111*/

    if(ovld_phy_reg_wr(port, GENERAL_CTRL1_REG20, wrval))
        return(FAILED);

    /* Set the reset bit in a separte write
     */
    wrval = wrval | SET_PHY_BIT15; /* Mode Software Reset */
    if(ovld_phy_reg_wr(port, GENERAL_CTRL1_REG20, wrval))
        return(FAILED);

    /* Read back to check for reset done */
    do {
      msleep(10);
      rdval = ovld_phy_reg_rd(port, GENERAL_CTRL1_REG20);
    } while((repeat-- > 0) && (rdval & SET_PHY_BIT15));


    if ((repeat == 0) && (rdval & SET_PHY_BIT15)) {
        return(FAILED);
    } else {
        return(PASSED);	
    }
}

/*------------------------------------------------------------------
 *
 * Function: set_eng_detect
 *   setup internal loopback need to disable energy detect
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
    short rdval, wrval;

    /* go to page 0*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED); 

    rdval = ovld_phy_reg_rd(port, COP_SPEC_CTRL_REG16);
   
    if(onoff)
        wrval = rdval & ~SET_ENG_DETECT;  /*disable energy detect*/
    else
        wrval = rdval | SET_ENG_DETECT;  /* restore energy detect*/
   	
    if(ovld_phy_reg_wr(port, COP_SPEC_CTRL_REG16, wrval))
      return(FAILED);

    sleep(ETH_DRIVER_DELAY); /*cannot mask or test failed */

    return PASSED;	
}

/*------------------------------------------------------------------
 *
 * Function: set_mac_speed
 *   set mac speed to 100. both PHY 1548 and 1340 are use 
 * Reg 21 on Page 2. need follow by soft reset. 
 *  
 * Input:  port - mdio address of the port
 *         speed - setup speed
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_mac_speed(int port, int speed)
{
    short rdval, wrval;

    /*go to page 2*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE2))
      return(FAILED); 

    rdval = ovld_phy_reg_rd(port, MAC_SPEC_CTRL2_REG21);

    rdval = rdval & ~0x0007; /* clean up the speed reg[0:2]*/
    switch(speed) {
      case SPD_10MBPS:
	wrval = rdval | 0x0004;
      break;
      case SPD_100MBPS:
	wrval = rdval | 0x0005;
      break;
      case SPD_1000MBPS:
	wrval = rdval | 0x0006;
      break;
      default:  
        printf("Port %d not support speed %d on MAC\n", port, speed);	  
      break;
    }

    if(ovld_phy_reg_wr(port, MAC_SPEC_CTRL2_REG21, wrval))
      return(FAILED);

    sleep(2*ETH_DRIVER_DELAY); /*can not be mask bridge PHY will failed */

    return(PASSED);	
}

/*------------------------------------------------------------------
 *
 * Function: set_media_int_lpbk
 *   Set up to prepare for media PHY internal loopback
 *  
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_media_int_lpbk(boolean onoff, int port)
{ 
    /* Note: The following sequence is suggested by marvell FAE.
     * Please keep it this way.
     */

    /* Disable macsec
     */
    if(set_macsec(onoff, port+ADDR_MEDIA_PHY)) {
    	printf("set_macsec failed. ");
    	return(FAILED);
    }

    /* The media PHY turn off auto media detect mode */
    if(set_automedia(onoff, port+ADDR_MEDIA_PHY)) {
    	printf("set automedia failed. ");
    	return(FAILED);
    }

    /* turn off energy detect, to prevent the lpbk stub is not plug-in. */
    if(set_eng_detect(onoff, port+ADDR_MEDIA_PHY)) {
    	printf("set eng detect failed. ");
    	return(FAILED);
    }

    /* Force copper linke up. Needed for 1548P part
     */
    if(force_linkup(onoff, port+ADDR_MEDIA_PHY)) {
    	printf("force_linkup failed. ");
    	return(FAILED);
    }

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_media_phy_int_lpbk
 *   initial and setup loopback type on sgmii for internal lpbk 
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
    short rdval, wrval;
    int phy_addr;
   
    sprintf(pname,"%s%d", type, port);   

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, INT_LPBK)) != PASSED){
    	printf("init_sgmii_env failed \n");
    	return(FAILED);
    }

    /* turn off stub loopback */
    if ((rc = set_phy_stub(pname, INT_LPBK, SIG_COPPER)) != PASSED){
    	printf("set_phy_stub failed \n"); 
    	return(FAILED);
    }   

    /* Per marvell FAE. Make sure bridge phy autoneg is on
     */
    bridge_phy_autoneg_on(port);

    /* set env for media PHY internal loopback */
    if ((rc = set_media_int_lpbk(ENABLE_SIG, port)) != PASSED){
    	printf("set_media_int_lpbk failed \n"); 
    	return(FAILED);
    }

    /* Note: marvell FAE suggest the sequence up to the end.
     * Please keep the code this way.
     */
    if(set_mac_speed(port + ADDR_MEDIA_PHY, speed)) {
    	printf("set mac speed failed. ");
    	return(FAILED);
    }
    
    /* Reset the copper control reg 0
     */
    phy_addr = port + ADDR_MEDIA_PHY;
    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0);
    rdval = ovld_phy_reg_rd(phy_addr, COP_CTRL_REG0);

    wrval = rdval | PHY_REG_BIT(15);
    ovld_phy_reg_wr(phy_addr, COP_CTRL_REG0, wrval);

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, SIG_FIBER);
    sleep(ETH_DRIVER_DELAY);	
    ovld_phy_reg_wr(phy_addr, OVLD_PHY_PAGE22, SIG_COPPER);
    sleep(ETH_DRIVER_DELAY);	

    /* Make sure media phy copper reset bit is 0
     */
    rdval = ovld_phy_reg_rd(phy_addr, COP_CTRL_REG0);
    if ((rdval & PHY_REG_BIT(15)) == 1) {
        printf("Media PHY reg 0_0:15 reset not cleared\n");
    }

    /* Per FAE instruction, set the copper internal loopback bit
     * in a second write
     */
    rdval = ovld_phy_reg_rd(phy_addr, COP_CTRL_REG0);
    wrval = rdval | PHY_REG_BIT(14);
    ovld_phy_reg_wr(phy_addr, COP_CTRL_REG0, wrval);

    /* Make sure loopback bit is set
     */
    rdval = ovld_phy_reg_rd(phy_addr, COP_CTRL_REG0);
    if ((rdval & PHY_REG_BIT(14)) == 0) {
      printf("Media PHY reg 0_0:14 loopback not set\n");
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     */
    sleep(ETH_DRIVER_DELAY*3);	
  
    if (!is_media_phy_copper_linkup(port + ADDR_MEDIA_PHY) ||
	!is_media_phy_qsgmii_linkup(port + ADDR_MEDIA_PHY) ||
	!is_bridge_phy_qsgmii_linkup(port + ADDR_BRIDGE_PHY)) {
        return(FAILED);
    }
    
    if ((rc = ovld_cavium_is_linkup(port)) != PASSED) {
        printf("sgmii link up time out after 1 second \n");
	show_status_info(port + ADDR_MEDIA_PHY);
	return(FAILED);
    }

#if DEBUG
    printf("\n1548 & 1340 PHY's links are up before sending packets.\n");
#endif

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_bridge_phy_mode
 *   setup internal PHY mode to SGMII-to-Copper mode and process
 *   mode reset. the setting is for 1340 PHY register.
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_bridge_phy_mode(boolean onoff, int port)
{
    short rdval, wrval;
    int repeat = 100;
 
     /* go to page 6*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE6))
      return(FAILED);

    rdval = ovld_phy_reg_rd(port, GENERAL_CTRL_REG20);

    rdval = rdval & ~0x0007; /*clean up reg[2:0]*/
    if(onoff)
      wrval = rdval | 0x0001;  /*SGMII to Copper mode */
    else
      wrval = rdval | 0x0005;  /*SGMII to QSGMII mode */   

    wrval = wrval | SET_PHY_BIT15;  /* reset reg 20 for mode reset */
    if(ovld_phy_reg_wr(port, GENERAL_CTRL_REG20, wrval))
      return(FAILED);

    /* Read back to check for reset done */
    do {
      msleep(10);
      rdval = ovld_phy_reg_rd(port, GENERAL_CTRL1_REG20);
    } while((repeat-- > 0) && (rdval & SET_PHY_BIT15));


    if ((repeat == 0) && (rdval & SET_PHY_BIT15)) {
        return(FAILED);
    } else {
        return(PASSED);	
    }
}

/*------------------------------------------------------------------
 *
 * Function: direct_phy_soft_reset
 *   soft reset PHY on page0 reg0 and bit 15.
 *
 * Input:  port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int direct_phy_soft_reset(int port)
{
    short rdval, wrval;

    /* go to page 0*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED); 

    rdval = ovld_phy_reg_rd(port, OVLD_PHY_PAGE0); /* reg0 */

    wrval = rdval | SET_PHY_BIT15;  
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE0, wrval))
      return(FAILED);

    sleep(ETH_DRIVER_DELAY);

    return(PASSED);	
}

/*------------------------------------------------------------------
 *
 * Function: media_phy_int_lpbk_test
 *   This is the entry point for internal loopback test only.
 *
 * Input: port - port number
 *        speed - 10, 1000, 1000MB
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int media_phy_int_lpbk_test(int port, int speed) {
	  
    int rc = 0; 

    /* setup loopback information */          
    rc = set_media_phy_int_lpbk(SEL_PORT_ETH, port, speed);
                   
    if (rc == FAILED) {
       printf("sgmii_set_phy_int_lpbk failed, port: %d \n", port);
       set_media_int_lpbk(DISABLE_SIG, port);
       return(FAILED);
    }

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = ovld_set_packet(port, speed);
        
    /* Restore the original setting to prevent ext lpbk error occur.*/
    set_media_int_lpbk(DISABLE_SIG, port);

    /* wait for driver get the packet then restore the setting */
    sleep(ETH_DRIVER_DELAY);

    return (rc);
}


/*------------------------------------------------------------------
 *
 * Function: sgmii_adv_full_duplex
 *   setup advertised full duplex register for 10 and 100.
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sgmii_adv_full_duplex(boolean onoff, int port)
{
    short rdval, wrval;

    /* go to page 0*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED); 

    rdval = ovld_phy_reg_rd(port, COP_AUTONEG_ADV_REG4); /* reg4 */

    /* clean up reg7 and reg5 which are advertised half duplex */
    rdval = rdval & ~0x00A0;  
    if(onoff)
      /* Set reg6 and reg8 for adv full duplex */
      wrval = rdval | 0x0140;  
    else
      wrval = rdval & ~0x0140;  /* clean up value */   

    if(ovld_phy_reg_wr(port, COP_AUTONEG_ADV_REG4, wrval))
      return(FAILED);
   
    /* reset PHY to let settung is work */
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED); 

    rdval = ovld_phy_reg_rd(port, COP_CTRL_REG0);
   
    wrval = rdval | SET_PHY_BIT15;
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE0, wrval))
      return(FAILED);

    sleep(ETH_DRIVER_DELAY);

    return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: set_bridge_phy_speed
 *   setup bridge phy speed
 *
 * Input: port - setup port
 *        speed - 10, 1000, 1000MB
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_bridge_phy_speed(int port, int speed)
{
    short rdval, wrval;

    /* go to page 0*/
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0))
      return(FAILED); 
   
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
        printf(" Port %d not support speed %d on Bridge PHY\n", port, speed);	  
        break;
    }
 
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE0, wrval))
      return(FAILED);

    rdval = ovld_phy_reg_rd(port, 0);

    wrval = rdval | 0x4000;  /*  turn on loopback */
    if(ovld_phy_reg_wr(port, OVLD_PHY_PAGE0, wrval))
      return(FAILED);

    sleep(ETH_DRIVER_DELAY);  /* cannot remove, will effect bridge PHY */

    return(PASSED);	
}


/*------------------------------------------------------------------
 *
 * Function: set_bridge_int_lpbk
 *   set internal PHY to internal loopback 
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_bridge_int_lpbk(boolean onoff, int port)
{
    
    /* ensure the advertisement register of bridge PHY 
     * is not turn on half duplex 
     */
    if(sgmii_adv_full_duplex(onoff, (port + ADDR_BRIDGE_PHY))) {
    	printf("sgmii_adv_full_duplex failed. ");
    	return(FAILED);
    } 

    /* ensure the bridge PHY is in SGMII to Copper mode. 
     * also reset the bridge PHY to turn off the loopback reg.
     */	
    if(set_bridge_phy_mode(onoff, (port + ADDR_BRIDGE_PHY))) {
    	printf("set_bridge_phy_mode failed. ");
    	return(FAILED);
    }
	
    return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: set_bridge_phy_int_lpbk
 *   we setup media PHY first which will let cavium know the current 
 *   setting of diag. Then to initial and setup loopback type on 
 *   bridge PHY.
 *
 * Input:  type - port type
 *         port - port number
 *         speed - 10, 100, 1000MB
 *
 * Output: PASSED/FAILED
 * 
 *------------------------------------------------------------------
 */
int set_bridge_phy_int_lpbk(char *type, int port, int speed)
{
    char pname[10];
    int rc = 0;
   
    sprintf(pname,"%s%d", type, port);   

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, port, INT_LPBK)) != PASSED){
    	printf("init_sgmii_env failed \n");
    	return(FAILED);
    }

    /* turn off stub loopback to prevent the packet is loop lpbk stub*/
    if ((rc = set_phy_stub(pname, INT_LPBK, SIG_COPPER)) != PASSED){
    	printf("set_phy_stub failed\n"); 
    	return(FAILED);
    }   

     /* without loopback stub, the PHY should forced link up here
      * to let setting of init_sgmii_env is working properly.
      */
    if(force_linkup(ENABLE, (port + ADDR_MEDIA_PHY))) {   
    	printf("force_linkup failed. ");
    	return(FAILED);
    }

    /* ensure the advertisement register is not turn on half duplex */
    if(sgmii_adv_full_duplex(ENABLE, (port + ADDR_MEDIA_PHY))) {
    	printf("sgmii_adv_full_duplex failed. ");
    	return(FAILED);
    } 

    /* soft reset makes setting of media PHY is work. */
    if ((rc = ovld_phy_soft_reset(pname, SIG_COPPER)) != PASSED){
    	printf("ovld_phy_soft_reset failed \n");
    	return(FAILED);
    }
 
    /* set advertisment reg and PHY mode  for bridge PHY 
     */
    if ((rc = set_bridge_int_lpbk(ENABLE_SIG, port)) != PASSED){
    	printf("set_bridge_int_lpbk failed \n"); 
    	return(FAILED);
    }   
 
    if(set_mac_speed((port + ADDR_BRIDGE_PHY), speed)) {
    	printf("set mac speed bridge PHY failed. \n");
    	return(FAILED);
    }

    /* soft reset bridge PHY makes setting of mac speed is work. */
    if(direct_phy_soft_reset((port + ADDR_BRIDGE_PHY))) {
    	printf("reset bridge PHY failed. \n");
    	return(FAILED);
    }
 
    /* set birdge PHY and also turn on the loopback bit. */
    if(set_bridge_phy_speed((port + ADDR_BRIDGE_PHY), speed)) {
    	printf("set bridge PHY speed failed. \n");
    	return(FAILED);
    }   
    

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: bridge_phy_int_lpbk_test
 *	This is the entry point for bridge PHY internal loopback test.
 *
 * Input:  port - port number 
 *         speed - test speed 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int bridge_phy_int_lpbk_test(int port, int speed) {
	  
    int rc = 0; 

    /* ensure the cavium is not in loopback mode. */
    set_sgmii_int_lpbk(port, FALSE);
    
    /* setup loopback information */          
    rc = set_bridge_phy_int_lpbk(SEL_PORT_ETH, port, speed);
                   
    if (rc == FAILED) {
        printf("set_bridge_phy_int_lpbk failed, port: %d \n", port);
        set_bridge_int_lpbk(DISABLE_SIG, port);
       	return(FAILED);
    }
            
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = ovld_set_packet(port, speed);
            
    /* Restore the original setting to prevent ext lpbk error occur.*/
    set_bridge_int_lpbk(DISABLE_SIG, port);
   	
    if(force_linkup(DISABLE, (port + ADDR_MEDIA_PHY))) {   
    	printf("force_linkup failed. ");
    	return(FAILED);
    }   

    if (rc != PASSED) {
        printf("ovld_set_packet failed %s\n",__FUNCTION__);
        return(FAILED);
    }
    
    return(PASSED);
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
    /* force link up can fool cavium that it is link up, 
     * even without plug-in cable or loopback port.
     */
    if(force_linkup(onoff, port+ADDR_MEDIA_PHY)) {
    	printf("force_linkup failed. ");
    	return FAILED;
    }

    /* setup cavium into loopback mode */
    set_sgmii_int_lpbk(port, onoff);
	  
    /* disable GMX enable reg to keep cavium GMX stay in correct status*/
    set_gmxeno(port, onoff);

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: set_cavium_int_lpbk
 *   initial and setup loopback type on Cavium.
 *
 * Input:  type - port type
 *         port - port number
 *         speed - 10, 100, 1000MB
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
    	printf("init_sgmii_env failed \n");
    	return(FAILED);
    }

    if ((rc = ovld_phy_soft_reset(pname, SIG_COPPER)) != PASSED){
       printf("ovld_phy_soft_reset failed \n");
       return(FAILED);  
    }

    /* setup cavium status and speed */
    if ((rc = sgmii_port_cfg(port, speed, AUTONEG_ON)) != PASSED){
       printf("sgmii port cfg failed\n");
       return(FAILED);  
    }

    if ((rc = setup_cavium_int_lpbk(port, TRUE)) != PASSED){
       printf("setup cavium internal loopback failed\n");
       return(FAILED);  
    }

    if ((rc = ovld_phy_soft_reset(pname, SIG_COPPER)) != PASSED){
       printf("ovld_phy_soft_reset failed\n");
       return(FAILED);  
    }

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: cavium_int_lpbk_test
 *   This is the entry point for cavium internal loopback test.
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
    int rc = 0, retval = 0; 
    	  
    rc = set_cavium_int_lpbk(SEL_PORT_ETH, port, speed);
    if (rc != PASSED) {
        printf("set_cavium_int_lpbk failed, port: %d \n", port);
        goto cavium_int_lpbk_exit;
    }
    
    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = ovld_set_packet(port, speed);
    if (rc != PASSED) {
	printf("Cavium port %d int loopback speed %d failed\n", port, speed);
        goto cavium_int_lpbk_exit;
    }

cavium_int_lpbk_exit:
    /*restore the setting */
    if ((retval = setup_cavium_int_lpbk(port, FALSE)) != PASSED){
       printf("setup cavium internal loopback failed\n");
       return(FAILED);  
    }

   return rc; 

}

/*------------------------------------------------------------------
 *
 * Function: ovld_phy_lpbk_test
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
 * It・s just that Cavium polling PHY・s status registers  
 * (I don・t know which ones) is causing Cavium to not configure properly.
 * That・s why for all internal loopbacks, I tried to use Forced 100 to fool Cavium, 
 * as I cannot force 1000 on PHY as required by IEEE spec.
 * For external loopback, 1000 is doable, because once PHY・s stub 
 * loopback register bit is enabled, Cavium is fooled for some reason 
 * and so I don・t need to force 100."
 * Thus, we skip the 1000Mpbs on internal loopback test.
 *------------------------------------------------------------------
 */
int
ovld_phy_lpbk_test(int lpbkmode)
{
    int rc = 0;
    int port_cnt, port_curr, port;
    int speed_cnt, speed_curr, speed;
    char pname[10];
    int try, retry_limit = 2;

    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
 
    /* get test envrionment variable */
    port_cnt = sizeof(eth_port_list) / sizeof(int);
    speed_cnt = sizeof(eth_speed_list) / sizeof(int);

    for(port_curr = 0; port_curr < port_cnt; port_curr++) {
        port = eth_port_list[port_curr];

        for(speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            speed = eth_speed_list[speed_curr];
	    
            sprintf(pname, "eth%d", port);   

	    /* skip 1000Mbps on all the internal loopback test*/
	    if((speed == SPD_1000MBPS) && 
	       ((lpbkmode != SGMII_EXT_LPBK) && (lpbkmode != SGMII_INT_EXT_LPBK))) {
	        continue;
	    }

	    switch(lpbkmode) {
	    case CAVIUM_INT_LPBK:
	        testname("Cavium internal loopback");
		prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
		rc = cavium_int_lpbk_test(port, speed);
		if (rc != PASSED) {
		    printf("Possible causes of problem:\n"
			   "1. The Cavium SGMII port may be bad/ not soldered.\n"
			   "2. Bridge PHY may be bad or not soldered to cause link issueto Cavium\n"
			   "3. Suggest replace the bridge phy and media phy before replace Cavium\n");
		    cterr('f',0,"Cavium port %d int loopback speed %d failed\n", port, speed);
		}
		break;
        
	    case BRIDGE_PHY_INT_LPBK:
	        testname("Bridge PHY internal loopback");  
		prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
		rc = bridge_phy_int_lpbk_test(port, speed);
		if (rc != PASSED) {
		    printf("Possible causes of problem:\n"
			   "1. Bridge PHY is bad or not soldered\n"
			   "2. The Cavium SGMII port may be bad. Please run Cavium loopback test utility.\n");
		    cterr('f',0,"SGMII bridge PHY int loopback port %d failed\n", port);
		}	      
		break;
        
	    case MEDIA_PHY_INT_LPBK:
	        testname("Media PHY internal loopback");   
		prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
 
		for (try=0; try < retry_limit; try++) {
		    rc = media_phy_int_lpbk_test(port, speed);
		    if ((rc == PASSED) || (try == (retry_limit - 1))) {
		        break;
		    }
		    else {
		        printf("####### retry the test #########\n");
			reset_quad_phy();
		    }
		}

		if (rc != PASSED) {
		    printf("Possible causes of problem:\n"
			   "1. Media PHY is bad or not soldered\n"
			   "2. The bridge PHY may be bad. Please run bridge PHY test utility.\n");
		    cterr('f',0,"SGMII media PHY int loopback port %d failed\n", port);
		}
		break;
        
	    case SGMII_EXT_LPBK:
	        testname("SGMII external loopback");   	
		if (!check_ext_lpbk_flag()) {
		  prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
		  return(PASSED);
		}

		prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);

		for (try=0; try < retry_limit; try++) {
		    rc = media_phy_ext_lpbk_test(port, speed);
		    if ((rc == PASSED) || (try == (retry_limit - 1))) {
		        break;
		    }
		    else {
		        printf("####### retry the test #########\n");
			reset_quad_phy();
		    }
		}

		if (rc != PASSED) {
		    printf("Possible causes of problem:\n"
			   "1. External loopback plug missing/bad.\n"
			   "2. Media PHY external path bad. Try to run internal loopback test.\n"
			   "3. Media PHY is bad or not soldered\n");
		    cterr('f',0,"SGMII media PHY ext loopback port %d failed\n", port);
		}
		break;
    
	    case SGMII_INT_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test; 
                 *    if failed, perform Internal loopback test. 
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */
                 
		if (check_ext_lpbk_flag()) {
	          testname("SGMII external loopback");   	
		  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
		  for (try=0; try < retry_limit; try++) {
		      rc = media_phy_ext_lpbk_test(port, speed);
		      if ((rc == PASSED) || (try == (retry_limit - 1))) {
		          break;
		      }
		      else {
		          printf("####### retry the test #########\n");
			  reset_quad_phy();
		      }
		  }

		  if (rc != PASSED) {
		      printf("Possible causes of problem:\n"
			     "1. External loopback plug missing/bad.\n"
			     "2. Media PHY external path bad. Please run internal loopback test.\n"
			     "3. Media PHY is bad or not soldered\n");
		      cterr('f',0,"SGMII media PHY ext loopback port %d failed\n", port);
		  }
		}

                if (((!check_ext_lpbk_flag()) || (rc != PASSED)) &&
                     (speed != SPD_1000MBPS)) {
	          testname("Media PHY internal loopback");   	
		  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
		  for (try=0; try < retry_limit; try++) {
		      rc = media_phy_int_lpbk_test(port, speed);
		      if ((rc == PASSED) || (try == (retry_limit - 1))) {
		          break;
		      }
		      else {
		          printf("####### retry the test #########\n");
			  reset_quad_phy();
		      }
		  }

		  if (rc != PASSED) {
		      printf("Possible causes of problem:\n"
			     "1. Media PHY is bad or not soldered\n"
			     "2. The bridge PHY may be bad. Please run bridge PHY test utility.\n");
		      cterr('f',0,"SGMII media PHY int loopback port %d failed\n", port);
                  }
                }

		break;

	    default:
	        rc = FAILED;
		cterr('f',0," Ovld not support this loopback mode\n");	  
		break;
	    } /*switch*/

	    if (rc != PASSED) {
	      ovld_err_clean_up(port);
	      return(FAILED); /* Do not continue on next port */
	    }

	} /*speed*/
    }/*port*/

    return(rc);
}

/**********************************************************************
 *
 * Function: ovld_phy_lpbk_util
 *
 * Description:
 * Utility to execute SGMII single port internal or external loopback
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int
ovld_phy_lpbk_util(void)
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
      rc = media_phy_ext_lpbk_test(port, speed);
    }
	  
    if (rc == FAIL) {
      ovld_err_clean_up(port);
      cterr('f',0,"Loopback test failed on port %d speed %d \n", port, speed);	      
    }

    return(rc);
}


/**********************************************************************
 *
 * Function: ovld_cavium_int_lpbk_test
 *
 * Description: Ovld Cavium internal loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int ovld_cavium_int_lpbk_test(void)
{
    return(ovld_phy_lpbk_test(CAVIUM_INT_LPBK));
}

 
/**********************************************************************
 *
 * Function: ovld_phy_int_lpbk_test
 *
 * Description: Ovld bridge PHY internal loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int ovld_bridge_phy_int_lpbk_test(void)
{
    return(ovld_phy_lpbk_test(BRIDGE_PHY_INT_LPBK));
}


/**********************************************************************
 *
 * Function: ovld_media_phy_int_lpbk_test
 *
 * Description: Ovld media PHY internal loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int ovld_media_phy_int_lpbk_test(void)
{
    return(ovld_phy_lpbk_test(MEDIA_PHY_INT_LPBK));
}


/**********************************************************************
 *
 * Function: ovld_phy_ext_lpbk_test
 *
 * Description: Ovld PHY external loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int ovld_phy_ext_lpbk_test(void) {
    return(ovld_phy_lpbk_test(SGMII_EXT_LPBK));
}

/**********************************************************************
 *
 * Function: ovld_sgmii_int_ext_lpbk_test
 *
 * Description: Ovld PHY internal/external loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int ovld_sgmii_int_ext_lpbk_test(void) {
    return(ovld_phy_lpbk_test(SGMII_INT_EXT_LPBK));
}


/*******************************************************************************
 *
 * Function   : ovld_media_phy_testmode_util
 * Description: Utility to set Overlord PHY(Marvell 1548) into Test mode.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
ovld_media_phy_testmode_util (void)
{
    int     port = 0, test_mode = 0;

    port = getdec_answer("Enter port number (0-3)", 0, 0, 3);

    printf("PHY(Marvell 1548) Supported TestMode:\n");
    printf("[0] Normal Mode.\n");
    printf("[1] Transmit Waveform Test.\n");
    printf("[2] Transmit Jitter Test (Master).\n");
    printf("[3] Transmit Jitter Test (Slave).\n");
    printf("[4] Transmit Distortion Test.\n");
    test_mode = getdec_answer("Enter Test mode (0-4)", 0, 0, 4);

    if (ovld_set_media_phy_testmode(port, (uint8_t)test_mode) != PASSED) {
        printf("%s: FAILED to set Eth%d PHY(1548) into TestMode%d.\n",
               __FUNCTION__, port, test_mode);
        return (FAILED);
    }

    printf("\nNow Eth%d PHY(1548) enter TestMode%d, and press \'q\' to exit: ",
           port, test_mode);

    while (1) {
        if(getchar() == 'q') {
            ovld_softreset_media_phy(port);
            break;
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ovld_set_media_phy_reg_by_value
 * Description: Function to set PHY(Marvell 1548) register.
 * Inputs     : port    - Port number of PHY that want to set
 *              reg_off - Offset of the register
 *              val     - Value to set the register
 *              mask    - Mask of register R/W capability
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
ovld_set_media_phy_by_value (int port, int reg_off, uint16_t val, uint16_t mask)
{
    int phy_id = 0, ret_val = 0;
    uint16_t reg_val = 0;

    /* Set related PHY ID */
    phy_id = ADDR_MEDIA_PHY + port;

    if (ovld_phy_reg_wr(phy_id, reg_off, val) != PASSED) {
        printf("\n%s: Failed to write Eth%d PHY(1548) Reg%.2d to 0x%08X.\n",
               __FUNCTION__, port, reg_off, val);
        return (FAILED);
    }

    /* Use the read back value to confirm the setting */
    ret_val = ovld_phy_reg_rd(phy_id, reg_off);
    if (ret_val < 0) {
        printf("\n%s: Failed to read Eth%d PHY(1548) Reg%.2d (ret = %#X).\n",
               __FUNCTION__, port, reg_off, ret_val);
        return (FAILED);
    }

    reg_val = (uint16_t)ret_val;
    if ((reg_val & mask) != (val & mask)) {
        printf("\n%s: Failed because read back data (0x%04X) and"
               " the date set-in (0x%04X) are not matched.\n",
               __FUNCTION__, reg_val, val);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ovld_set_media_phy_testmode
 * Description: Main function to set PHY(Marvell 1548) into Test mode.
 * Inputs     : port - Port number that want to set into Test mode
 *              test_mode - Type of Test mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
ovld_set_media_phy_testmode (int port, uint8_t test_mode)
{
    int ctr = 0, total_steps = 0;
    uint16_t testmode_val = 0;
    mrvl_phy_setup_t *step_ptr;
    
    if ((test_mode == PHY_TESTMODE_1) || (test_mode == PHY_TESTMODE_2) ||
        (test_mode == PHY_TESTMODE_4)) {
        step_ptr = &phy_testmode124_steps[0];
        total_steps = sizeof(phy_testmode124_steps) / sizeof(mrvl_phy_setup_t);

        /* 1. Enable Test mode 1: 0x3F00
         * 2. Enable Test mode 2: 0x5F00
         * 3. Enable Test mode 4: 0x9F00
         */
        if (test_mode == PHY_TESTMODE_1) {
            testmode_val = 0x3F00;
        } else if (test_mode == PHY_TESTMODE_2) {
            testmode_val = 0x5F00;
        } else if (test_mode == PHY_TESTMODE_4) {
            testmode_val = 0x9F00;
        }
    } else if (test_mode == PHY_TESTMODE_3) {
        step_ptr = &phy_testmode3_steps[0];
        total_steps = sizeof(phy_testmode3_steps) / sizeof(mrvl_phy_setup_t);

        /* Enable Test mode 3: 0x7700 */
        testmode_val = 0x7700;
    } else if (test_mode == PHY_TESTMODE_NORMAL) {
        return (ovld_softreset_media_phy(port));
    } else {
        printf("%s: Not support TestMode%d.\n", __FUNCTION__, test_mode);
        return (FAILED);
    } 

    for (ctr = 0; ctr < total_steps; ctr++, step_ptr++) {
        if ((step_ptr->reg_page == OVLD_PHY_PAGE0) &&
            (step_ptr->reg_off == OVLD_PHY_REG0) &&
            ((step_ptr->val) & SET_PHY_BIT15)) {
            prpass(testpass, "Set Eth%d to TestMode%d: Soft reset PHY(1548)",
                   port, test_mode);
            if (ovld_softreset_media_phy(port) != PASSED) {
                printf("\n%s: Failed to soft reset Eth%d PHY(1548).\n",
                       __FUNCTION__, port);
                return (FAILED);
            } else {
                continue;
            }
        }

        /* Jump to page of register that want to set */
        prpass(testpass, "Set Eth%d to TestMode%d: Jump to page%d",
                   port, test_mode, step_ptr->reg_page);
        if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG22, step_ptr->reg_page,
                                        step_ptr->mask) != PASSED) {
            printf("\n%s: Failed to jump to Eth%d PHY(1548) page%d.\n",
                   __FUNCTION__, port, step_ptr->reg_page);
            return (FAILED);
        }

        /* Set register */
        prpass(testpass, "Set Eth%d to TestMode%d: Set page%d Reg%.2d to 0x%04X",
               port, test_mode, step_ptr->reg_page, step_ptr->reg_off, step_ptr->val);
        if (ovld_set_media_phy_by_value(port, step_ptr->reg_off, step_ptr->val,
                                        step_ptr->mask) != PASSED) {
            printf("\n%s: Failed to set Eth%d PHY(1548) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, port, step_ptr->reg_page,
                   step_ptr->reg_off, step_ptr->val);
            return (FAILED);
        }
    }

    /* Set Test mode by write page0 Reg 9 */
    /* Jump to page of register that want to set */
    prpass(testpass, "Set Eth%d to TestMode%d: Jump to page%d",
           port, test_mode, OVLD_PHY_PAGE0);
    if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG22, OVLD_PHY_PAGE0, 0xC0FF)
        != PASSED) {
        printf("\n%s: Failed to jump to Eth%d PHY(1548) page%d.\n",
               __FUNCTION__, port, OVLD_PHY_PAGE0);
        return (FAILED);
    }

    /* Set register */
    prpass(testpass, "Set Eth%d to TestMode%d: Set page%d Reg%.2d to 0x%04X",
           port, test_mode, OVLD_PHY_PAGE0, OVLD_PHY_REG9, testmode_val);
    if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG9, testmode_val, 0xFFFF)
        != PASSED) {
        printf("\n%s: Failed to set Eth%d PHY(1548) page%d Reg%.2d to 0x%04X.\n",
               __FUNCTION__, port, OVLD_PHY_PAGE0, OVLD_PHY_REG9, testmode_val);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : ovld_softreset_media_phy
 * Description: Function to soft reset PHY(Marvell 1548).
 * Inputs     : port - number of the accessed port
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
ovld_softreset_media_phy (int port)
{
    int phy_id = 0, ret_val = 0;
    uint16_t reg_val = 0;

    /* Set related PHY ID */
    phy_id = ADDR_MEDIA_PHY + port;

    if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG22, OVLD_PHY_PAGE0, 0xC0FF)
        != PASSED) {
        printf("\n%s: Failed to jump to Eth%d PHY(1548) page%d.\n",
               __FUNCTION__, port, OVLD_PHY_PAGE0);
        return (FAILED);
    }

    if (ovld_phy_reg_wr(phy_id, OVLD_PHY_PAGE0, 0x9140) != PASSED) {
        printf("\n%s: Failed to write Eth%d PHY(1548) Reg%.2d to 0x%08X.\n",
               __FUNCTION__, port, OVLD_PHY_REG0, 0x9140);
        return (FAILED);
    }

    /* Use the read back value to confirm soft reset is done */
    ret_val = ovld_phy_reg_rd(phy_id, OVLD_PHY_REG0);
    if (ret_val < 0) {
        printf("\n%s: Failed to read Eth%d PHY(1548) Reg%.2d (ret = %#X).\n",
               __FUNCTION__, port, OVLD_PHY_REG0, ret_val);
        return (FAILED);
    }

    reg_val = (uint16_t)ret_val;
    if (reg_val & SET_PHY_BIT15) {
        printf("\n%s: Soft reset Eth%d PHY(1548) is not finished.\n",
               __FUNCTION__, port);
        return (FAILED);
    }

    return (PASSED);
}


/*
$Log: platform_ext_lpbk.c,v $
Revision 1.40  2013/02/26 01:48:42  palin2
Fixed the utility to let GE PHY enter TestMode based on Marvell FAE's comments.

Revision 1.39  2013/02/19 19:01:25  ptong
Add more info in error message

Revision 1.38  2013/01/30 23:50:16  palin2
Add utility to set Cavium side GE PHY, Marvell 1548, into Test mode.

Revision 1.37  2013/01/25 10:47:02  alpeng
support macsec util

Revision 1.36  2012/11/03 01:28:36  ptong
Document and clean up

Revision 1.35  2012/10/26 02:00:08  ptong
Add delay to allow PHY to be stable before sending packet

Revision 1.34  2012/10/25 22:43:17  ptong
Fix PHY power up init issue

Revision 1.33  2012/10/20 06:37:34  ptong
Remove a printf

Revision 1.32  2012/10/20 05:24:16  ptong
Change retry limit to 2

Revision 1.31  2012/10/20 01:27:00  ptong
Bug fix: CSCuc79132 SGMII and SFP ext loopback failing randomly on different ports

Revision 1.30  2012/10/18 06:04:08  ptong
Fix bug: CSCuc64054, Overlord data plane 1548 PHY loopback test failed

Revision 1.29  2012/10/03 00:59:18  ptong
Minor fix

Revision 1.28  2012/09/20 08:33:13  alpeng
adding wait after reset PHY

Revision 1.27  2012/09/17 15:55:31  alpeng
1. add is_linkup for sfp
2. combine soft reset on set_automedia and bridge_phy_mode for speed up
3. fixed definition order of SGMII_INT_EXT_LPBK for util.
4. clean up code.

Revision 1.26  2012/09/04 09:09:42  alpeng
add clean up function after test failed.

Revision 1.25  2012/09/04 08:46:28  alpeng
skipped speed 1000 on internal loopback test

Revision 1.24  2012/08/24 23:11:40  ptong
Increase the fiber output amplitude in PHY-1548

Revision 1.23  2012/08/24 14:27:10  alpeng
fixed menu item name and display msg.

Revision 1.22  2012/08/22 10:04:23  alpeng
Using Ext. loopback flag to decide loopback test is internal or external loopback test. Moving media PHY diag item into debug utility menu

Revision 1.21  2012/08/11 00:00:18  ptong
Remove complile flag RELEASE_CVMX_DIAG

Revision 1.20  2012/08/06 03:35:16  alpeng
recover the check-in history

Revision 1.19  2012/08/06 03:30:02  alpeng
fixed r/w PHY reg error message and cleanup code

Revision 1.18  2012/08/01 14:29:03  alpeng
fixed compile warning

Revision 1.17  2012/08/01 14:26:32  alpeng
adding check link up status for SFP and internal loopback

Revision 1.16  2012/07/24 06:41:05  alpeng
clean up compile warning

Revision 1.15  2012/07/24 06:09:39  alpeng
support 2nd retry on loopback test

Revision 1.14  2012/07/19 20:10:30  ptong
Improve test progress message

Revision 1.13  2012/07/18 22:59:29  ptong
Fix a problem so that (NVRAM)->diagflag is used correctly on Cavium data plane menu

Revision 1.12  2012/06/27 01:55:58  ptong
Exit the test when a port failed instead of continue to next port

Revision 1.11  2012/06/21 20:09:37  ptong
Better synchronize the semaphore between the tx and rx thread

Revision 1.10  2012/06/19 17:32:02  ptong
Fix initermittent rx_finish sem timeout problem

Revision 1.9  2012/06/15 01:59:27  ptong
Correct cterr printing

Revision 1.8  2012/06/06 15:00:37  palin2
Clean up compiler warnings.

Revision 1.7  2012/06/05 06:21:03  alpeng
clean up compiler warnings.

Revision 1.6  2012/05/08 00:05:15  ptong
Improve test printing

Revision 1.5  2012/05/02 09:16:37  alpeng
skip next speed and switch to next port when test failed.

Revision 1.4  2012/04/27 10:42:42  alpeng
fixed minor bugs and support set external loopback flag for controlling test flow

Revision 1.3  2012/03/28 00:38:19  mcharon
remove forward slash from second line

Revision 1.2  2012/03/27 16:18:21  alpeng
cavium side code clean up

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module

$Endlog$
*/
