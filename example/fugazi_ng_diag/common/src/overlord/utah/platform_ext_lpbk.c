/* $Id: platform_ext_lpbk.c,v 1.31 2018/12/21 00:58:13 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_ext_lpbk.c,v $
 *------------------------------------------------------------------
 * 
 * platform_ext_lpbk.c  
 * support PHY external loopback 
 * internal loopback: media PHY, bridge PHY and Cavium.
 *
 * Oct 2011 Alan Peng
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
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
#include "platform_fru.h"
#include "platform_margin_utils.h"
#include "cli_cmd.h"
#include "nvsysvars.h"
#include "dash_fpga.h"

#define TWO_PHY 0 /* 0 for 1 PHY; 1 for 2 PHYs */


/* global */
extern uchar mb_phy_id[];
extern uchar mb_phy_loc[];
extern int sem_timedwait(sem_t *, const struct timespec *);
extern int create_raw_socket(int);
extern int bind_socket(char *, int, int);
extern int set_promisc(char *, int);
extern int chk_macaddr(uchar *, uchar *);

/* GE port count for Utah */
static int eth_port_list[] = { SGMII0, SGMII1, SGMII2 }; 
/* GE port count for Sword and Dagger */
static int eth_port_list_sd[] = { SGMII0, SGMII1 };   
static int eth_port0_list_gb[] = { SGMII0 };   
static int eth_port1_list_gb[] = { SGMII1 };   
static int eth_speed_list[] = { SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS }; 
static int dagger_eth_speed_list[] = { SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS };
static int vg400_eth_speed_list[] = { SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS };

int ovld_set_media_phy_testmode(int, uint8_t);
int goldbeach_set_media_phy_testmode(int, uint8_t);
int set_mac_speed(int, int);
void show_eth_counter(int); 

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
  {0xa0, ETH_PKT_MIN_LEN, H_INCFILL, 3000},
  {0xa2, (ETH_PKT_MIN_LEN + 1), H_INCFILL, 3000},
  {0xa4, ((ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN - 1)), H_INCFILL, 3000},
  {0xa6, (ETH_PKT_MAX_LEN - ETH_PKT_CRC_LEN), H_INCFILL, 3000},
};

mac_addr_t mac_da = {0x90, 0x2B, 0x34, 0xDA, 0xE2, 0xcc};
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

/* Based on 88E1548P_RevA0_Release-Notes_021313 from Marvell FAE,
 * Steps to enter 1548 PHY to Test Mode 1, 2 or 4 are:
 * 1. Write Page 0, Reg  9 = 0x1F00 (Set PHY to Master mode)
 * 2. Write Page 0, Reg  0 = 0x9140 (Soft-reset)
 * 3. Write Page 4, Reg 27 = 0x3E80 (Disable Clock on the HSDACP/N by set bit8 to 0)
 * 4. Write Page 6, Reg 26 = 0x8000 (Enable TX_TCLK)
 */
static mrvl_phy_setup_t gb_phy_testmode124_steps[] = {
    {OVLD_PHY_PAGE0, OVLD_PHY_REG9,  0x1F00, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0x9140, 0x7B40},
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
static mrvl_phy_setup_t gb_phy_testmode3_steps[] = {
    {OVLD_PHY_PAGE0, OVLD_PHY_REG9,  0x1700, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0x9140, 0x7B40},
    {OVLD_PHY_PAGE4, OVLD_PHY_REG27, 0x3E80, 0xFFFF},
    {OVLD_PHY_PAGE6, OVLD_PHY_REG26, 0x8000, 0xFFA0}
};
/* 
 * Steps to enter 1548 PHY to 10M Pseudo-Random Test are:
 * 1. Write Page 0, Reg  16 = 0x0400 (Disable Auto-MDIX & Force copper link up)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 * 3. Write Page 6, Reg 16 = 0x0008 (Enable Packet Generator)
 */
static mrvl_phy_setup_t gb_phy_testmode5_steps[] = {
    {OVLD_PHY_PAGE0, COP_SPEC_CTRL_REG16,  0x0400, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0x8100, 0x7B40},
    {OVLD_PHY_PAGE6, COP_SPEC_CTRL_REG16, 0x0008, 0xFFFF}
};
/* 
 * Steps to enter 1548 PHY to 10M data 0/1 Test Mode are:
 * 1. Write Page 0, Reg  16 = 0x0400 (Disable Auto-MDIX & Force copper link up)
 * 2. Write Page 0, Reg  0 = 0x8100 (Disable Auto-Neg. & Force speed to 10M)
 * 3. Write Page 2, Reg 21 = 0x5044 (Enable Loopback of MDI to MDI)
 */
static mrvl_phy_setup_t gb_phy_testmode6_steps[] = {
    {OVLD_PHY_PAGE0, COP_SPEC_CTRL_REG16,  0x0400, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0x8100, 0x7B40},
    {OVLD_PHY_PAGE2, MAC_SPEC_CTRL2_REG21, 0x5044, 0xFFFF}
};
/* 
 * Steps to enter 1548 PHY to 100M Test Mode are:
 * 1. Write Page 0, Reg 16 = 0x0000 (Disable Auto-MDIX )
 * 2. Write Page 0, Reg  0 = 0xA100 (Disable Auto-Neg. & Force speed to 100M)
 */
static mrvl_phy_setup_t gb_phy_testmode7_steps[] = {
    {OVLD_PHY_PAGE0, COP_SPEC_CTRL_REG16,  0x0000, 0xFFFF},
    {OVLD_PHY_PAGE0, OVLD_PHY_REG0,  0xA100, 0x7B40},
};
static void
display_reg (void)
{
    cterr_db_print("Please using utility: Show SGMII PHY registers.\n");
    cterr_db_print("gg: submenu of motherboard tests -> \n");
    cterr_db_print("pp: submenu of GE and SFP ext/internal loopback tests -> \n");
    cterr_db_print("i: Ethernet port utility menu -> \n");
    cterr_db_print("o: Show SGMII PHY registers \n");
}

static void
display_env (void)
{
    show_margins_x(0, CLI_MODE);
    cterr_db_print("using CLI cmd to show margining again: ./utah_lnx voltfreq");
}

static void
int_lpbk_add_err_report (void)
{

    fru_table_offset = MB_PHY;

    platform_fru_table[MB_PHY].pid_string = mb_phy_id;
    platform_fru_table[MB_PHY].location_string = mb_phy_loc;

    if (is_dagger()) {
        cterr_add_component("CPU", "1548 PHY");
    } else {
        cterr_add_component("CPU", "1340 PHY", "1548 PHY");
    }
    cterr_add_reg_dump((PFV)display_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Loopback test: make sure the port is link up,",
                    "type:[dmesg | grep eth*] on linux prompt to check port link status",  
                    "Internal: 1548->run 1340 loopack test; PHY is bad.",
                    "        : 1340-> PHY is bad.",
                    "Note: dagger has 1548 PHY only",
                    "Ethernet utility menu: \n"
                    "            gg: submenu of motherboard tests -> \n"
                    "            pp: submenu of GE and SFP ext/internal loopback tests -> \n"
                    "            i: Ethernet port utility menu ");

}

static void
ext_lpbk_add_err_report (void)
{

    fru_table_offset = MB_PHY;

    platform_fru_table[MB_PHY].pid_string = mb_phy_id;
    platform_fru_table[MB_PHY].location_string = mb_phy_loc;

    if (is_dagger()) {
        cterr_add_component("CPU", "1548 PHY");
    } else { 
        cterr_add_component("CPU", "1340 PHY", "1548 PHY");
    }
    cterr_add_reg_dump((PFV)display_reg);
    cterr_add_env_dump((PFV)display_env);
    cterr_add_debug("Loopback test: make sure the port is link up.",
                    "type:[dmesg | grep eth*] on linux prompt to check port link status",  
                    "External: make sure external plug is good and inserted.",
                    "        : run internal loopback test or PHY is bad.",
                    "Ethernet utility menu: \n" 
                    "            gg: submenu of motherboard tests -> \n"
                    "            pp: submenu of GE and SFP ext/internal loopback tests -> \n"
                    "            i: Ethernet port utility menu ");

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
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s\n",__FUNCTION__);
    }
    reset_platform_ext_dev (FPGA_EXT_GE_QUAD_RST);
    msleep(100);
    unreset_platform_ext_dev (FPGA_EXT_GE_QUAD_RST);
    msleep(100);

    /* Must init 1340, utah_marvell_1340_init() need to 
     * use with igb driver patched. 
     */
    utah_marvell_1340_init();

    /* per HW requeset, enlarge 1548 and 1340 eye */
    utah_marvell_phy_eye_enlarge();

    /* 2013.12.07: dagger need these settings and HW will fix it on eeprom 
     * after HW fix the issues, we can remove this function 
     */
    dagger_sgmii_setting();
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
 * Description: Compare 2 packet's crc
 *
 * Input: bufa - buffer of first packet
 *        bufb - buffer of 2nd packet
 *        count - number of bytes to compare
 *
 * Return: pass/fail
 */
int pkt_cmp (char volatile *bufa, char volatile *bufb, int count)
{
    int  rc = PASSED;
    uint crc_tx, crc_rx;

    crc_tx = ~crc32(~0, (unsigned char *)bufa, count);
    crc_rx = ~crc32(~0, (unsigned char *)bufb, count);
    if (crc_tx != crc_rx) {
        printf("tx crc doesn't match rx crc\n");
        rc = FAILED;
    }

    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: is_media_phy_copper_linkup
 *   Check if copper link up
 *
 * Input: we define the port setting as global
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int is_media_phy_copper_linkup (uint portnum)
{
    ushort rdval;

    /* go to page 0 */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0);
    utah_phy_reg_rd(portnum, COP_STATUS_REG17, &rdval);

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
 *   Chenk if QSGMII interface with 1548 link up
 *
 * Input: we define the port setting as global
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int is_media_phy_qsgmii_linkup (uint portnum)
{
    ushort rdval;

    /* go to page 4 */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE4);
    utah_phy_reg_rd(portnum, PHY_REG(17), &rdval);

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
 *   Chenk if QSGMII interface with 1340 link up
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
    ushort rdval;

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
 * Function: utah_port_is_linkup
 *   Check link up status from Linux information.
 *
 * Input: port number. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int utah_port_is_linkup(int port){

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
            if (strncmp(if_info->ifa_name, pname, strlen(pname)) != 0)
            	continue;
            	 
             /* printf("%s ", if_info->ifa_name); */
            
             flags = if_info->ifa_flags;
             if (( flags & IFF_UP ) && ( flags & IFF_RUNNING )) {
               /* printf("Link up\n");  */
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
    ushort ether_type;
    int sent = 0;
    uint crc;
    int frame_len;

    if (ether_type) {
        /*
         * Variable ether_type is used in this function
         * Add dummy codes here to fix latest compiler's error
         */
    }

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

    ether_type = htons(ETH_P_IP);
    //memcpy(cptr, (const void *)&ether_type, sizeof(ushort));
    //cptr += sizeof(ushort);

 
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
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
void force_linkup (uint portnum, boolean onoff)
{
    ushort rdval, wrval;

    /* go to page 0 */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0);
    utah_phy_reg_rd(portnum, COP_SPEC_CTRL_REG16, &rdval);

    /*bit 10 for force link up*/
    if (onoff) {
        wrval = rdval | SET_PHY_BIT10;  /* enable force link up*/
    } else {
        wrval = rdval & ~SET_PHY_BIT10;  /* restore force link up*/
    }
   	
    utah_phy_reg_wr(portnum, COP_SPEC_CTRL_REG16, wrval);

    sleep(ETH_DRIVER_DELAY);  

    return;
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
int ovld_err_clean_up (int port) {
    
#if DEBUG
    if(show_status_info(port)) {   
    	printf("Show status info failed. ");
    	return(FAILED);
    }
#endif 

    /* to prevent the endless message from ethtool setting */
    force_linkup(port, ENABLE);
    
    /* cfg port to 10 or 100 spd, or 1000 spd will fail on clean up, 
     * because force link up is not support on 1000 spd. 
     */
    sleep(ETH_DRIVER_DELAY); 
    if ((cfg_phy_setting(port, SPD_10MBPS, FULL_DUPLEX, AUTONEG_OFF, SIG_COPPER))) {
    	printf("cfg_phy_setting failed \n");
    	return(FAILED); 
    }
    sleep(ETH_DRIVER_DELAY);
    

    /* after link up, disable the register. */
    force_linkup(port, DISABLE);
    
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
    ushort rdval = 0;
    short result = 0;
    uint speed = 0;

    /* go to page 0 */
    utah_phy_reg_wr(port, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0);
    utah_phy_reg_rd(port, COP_AUTONEG_ADV_REG4, &rdval);
    printf("advertisement register rdval = 0x%x\n", rdval);

    /* status register reg17 */
    utah_phy_reg_rd(port, COP_STATUS_REG17, &rdval);
    printf("status register rdval = 0x%x\n", rdval);
    result = ((rdval & OVLD_PHY_SPEED_MSK) >> OVLD_PHY_SPEED_OFFSET);

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
 * Input:  portnum - port num.
 *         speed - setup speed 
 *         duplex - turn full/half duplex
 *         autoneg - turn on/off autoneg        
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int cfg_phy_setting(int portnum, int speed, int duplex, int autoneg, boolean signal)
{
    ushort rdval, wrval;
    int spdset = 0;

    /* select page 0 or page 1 from signal */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, signal); 

    /* Read PHY control reg for current speed*/
    /* set speed, Reg [0_2.6, 0_2.13] = value */
    utah_phy_reg_rd(portnum, OVLD_PHY_PAGE0, &rdval);

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

    /* ensure chip is not power down */
    wrval &= ~0x0800;   

    /* NEW: follow by reset */
    wrval |= 0x8000;   

    /* Write to the phy and read back immediate to make sure. */
    utah_phy_reg_wr(portnum, PHY_REG(0), wrval);

#if TWO_PHY

    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    if (speed != 0) {
        if(signal) {
            utah_phy_reg_wr(portnum, PHY_REG(22), SIG_COPPER);
	    sleep(ETH_DRIVER_DELAY); /* link down here */
            utah_phy_reg_wr(portnum, PHY_REG(22), SIG_FIBER);
        } else {
            utah_phy_reg_wr(portnum, PHY_REG(22), SIG_FIBER);
	    sleep(ETH_DRIVER_DELAY); /* link down here */
            utah_phy_reg_wr(portnum, PHY_REG(22), SIG_COPPER);
	}
	sleep(ETH_DRIVER_DELAY); /* link up here */ 
    }
#endif 

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
    sprintf(rx_info.name, pname);
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
    if (rc != PASSED)
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
 * Input: portnum - port num
 *        signal - 0 select copper page, 1 for fiber page
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int ovld_phy_soft_reset(int portnum, boolean signal)
{
    ushort rdval, wrval, regnum;
    int repeat = 100;

    /* The following code is per the Marvell 88E1548P PHY */
    /* Reset reg 0_0.15=1 */
    /* Use signal to select page for copper or fiber */
    regnum = OVLD_PHY_PAGE22; 
    utah_phy_reg_wr(portnum, regnum, signal);

    regnum = PHY_REG(0);
    utah_phy_reg_rd(portnum, regnum, &rdval);
    wrval = rdval | SET_PHY_BIT15;
    utah_phy_reg_wr(portnum, regnum, wrval);

    sleep(ETH_DRIVER_DELAY); /* link up here */ 

    /* Read back to check for reset done */
    do {
      msleep(100);
      utah_phy_reg_rd(portnum, PHY_REG(0), &rdval);
    } while((repeat-- > 0) && (rdval & SET_PHY_BIT15));

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
 * Input: portnum - port num
 *        enable - enable/disable the Enable stub register 
 *        signal - Copper or Fiber
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int set_phy_stub(int portnum, boolean enable, boolean signal)
{
    ushort rdval, wrval;

    /* The following code is per the Marvell 88E1548P PHY */
    /* enable phy stub for external loopback */
    /* set page 6 in reg 18 bit 3 */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE6);
    utah_phy_reg_rd(portnum, CHECKER_CTRL_REG18, &rdval);
    if ((enable) && (!signal)) {
        wrval = rdval | SET_PHY_BIT3;  /*external and copper*/
    } else {
        wrval = rdval & ~SET_PHY_BIT3; /*internal or fiber*/
    } 

    utah_phy_reg_wr(portnum, CHECKER_CTRL_REG18, wrval);
    utah_phy_reg_rd(portnum, CHECKER_CTRL_REG18, &rdval);

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);  /* can not mask, effect 1000 external lpbk test */

    /* 2013/12/25: back to page 0, otherwise the port cannot link up on dagger
     * this setting will not hurt utah, sword
     */
    utah_phy_reg_wr(portnum, PHY_REG(22), PHY_REG(0));

    if(rdval != wrval) {
      printf("%s:write failed, reg rdval = 0x%x, wrval = 0x%x\n",
              __FUNCTION__, rdval, wrval);
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

    sprintf(ifr.ifr_name, device);
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
 * Input:  portnum - port num
 *   enable - enable/disable the power up/down register 
 *   signal - select page for copper/fiber
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int sig_pwr_ctrl(int portnum, boolean enable, boolean signal)
{
    ushort rdval, wrval, regnum;

    /* The following code is per the Marvell 88E1548P PHY
     * signal == 0 is for copper mode, 1 is for fiber mode
     */
    regnum = OVLD_PHY_PAGE22; /* test fiber diable copper, vice versa. */
    utah_phy_reg_wr(portnum, regnum, signal);

    /* Set SGMII fiber ouput amplitude
     */
    if ((signal == SIG_FIBER) && enable) {
        regnum = FIB_SPEC_CTRL_REG2; 
        utah_phy_reg_rd(portnum, regnum, &rdval);
        wrval = ((rdval & ~FIB_OUTPUT_AMP_MSK) | FIB_OUTPUT_AMP_VAL504);
        utah_phy_reg_wr(portnum, regnum, wrval);
    }

    regnum = COP_CTRL_REG0; /* reg 0 */
    utah_phy_reg_rd(portnum, regnum, &rdval);
    
    if (enable){
        wrval = rdval & ~SET_PHY_BIT11; /*power up*/
    } else {
        wrval = rdval | SET_PHY_BIT11;  /*power down*/
    } 
    
    /* we do not need to setup the same value */
    if (wrval == rdval){
        return(PASSED);
    }    	
    
    utah_phy_reg_wr(portnum, regnum, wrval);
    utah_phy_reg_rd(portnum, regnum, &rdval);

    if (rdval != wrval) {
        printf("reg mismatch rdval = 0x%x, wrval = 0x%x\n", rdval, wrval);
        return(FAILED);
    }

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
    if ((rc = sig_pwr_ctrl(port, DISABLE_SIG, SIG_FIBER)) != PASSED){  
    	printf("sig_pwr_ctrl disable fiber failed \n"); 
    	return(FAILED);
    }
    if ((rc = sig_pwr_ctrl(port, ENABLE_SIG, SIG_COPPER)) != PASSED){  
    	printf("sig_pwr_ctrl enable copper failed \n"); 
    	return(FAILED);
    }	

#if TWO_PHY
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
     * utah_port_is_linkup will return failed,
     * need to verify this one is necessary or not.
     */
    if (lpbk_mode == EXT_LPBK) {
      if ((rc = utah_port_is_linkup(port)) != PASSED) {
	  printf("after init port, sgmii link up time out after 1 second \n");
	}
    }

#endif

    /* to ensure the test stay on full duplex and set speed 
     */
    if(speed == SPD_1000MBPS ) {
      autoneg = AUTONEG_ON;
    } else {
      autoneg = AUTONEG_OFF;
    } 

    if ((rc = cfg_phy_setting(port, speed, FULL_DUPLEX, autoneg, SIG_COPPER)) != PASSED) {
    	printf("cfg_phy_setting failed \n");
    	return(FAILED); 
    }

    /* using check link to ensure the link is stable */
    if (lpbk_mode == EXT_LPBK) {
      /* dagger has special connection */
      if (is_dagger()) {
          if (port == SGMII3) {
              port = SGMII1;
          } else { /* port == SGMII1 */
              port = SGMII0;
          }
      }

      if ((rc = utah_port_is_linkup(port)) != PASSED) {
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
        show_status_info(port);
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

    if(set_mac_speed(port, speed)) {
    	printf("set mac speed failed. ");
    	return(FAILED);
    }

    /* 1GMbps external loopback need to setup*/
    if ((rc = set_phy_stub(port, EXT_LPBK, SIG_COPPER)) != PASSED){
    	printf("set_phy_stub failed \n"); 
    	return(FAILED);
    }

#if TWO_PHY 
    /* Per marvell FAE. Make sure bridge phy autoneg is on
     */
    bridge_phy_autoneg_on(port);

    /* ovld_phy_soft_reset will turn off Enable loopback reg */
    if ((rc = ovld_phy_soft_reset(port, SIG_COPPER)) != PASSED){
    	printf("ovld_phy_soft_reset failed \n");
    	return(FAILED);
    }
#endif 

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     * Also needed on Utah
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
int set_macsec(boolean onoff, int portnum)
{
    ushort rdval, wrval;
	
    /* go to page 18*/
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE18);
    utah_phy_reg_rd(portnum, GENERAL_CTRL2_REG27, &rdval);
   
    if(onoff)
        wrval = rdval & ~SET_PHY_BIT13;  /*disable*/
    else
        wrval = rdval | SET_PHY_BIT13;  /*restore*/
   
    utah_phy_reg_wr(portnum, GENERAL_CTRL2_REG27, wrval);

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
int set_automedia(boolean onoff, int portnum)
{
    ushort rdval, wrval;
    int repeat = 100;

    /* go to page 18*/
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE18);
    utah_phy_reg_rd(portnum, GENERAL_CTRL1_REG20, &rdval);
   
    if(onoff)
        wrval = rdval & ~SET_AUTO_MEDIA;  /* QSGMII-to-Copper reg[2:0] = 0x000*/
    else
        wrval = rdval | SET_AUTO_MEDIA;  /* QSGMII-to-automedia reg[2:0] = 0x111*/

    utah_phy_reg_wr(portnum, GENERAL_CTRL1_REG20, wrval);

    /* Set the reset bit in a separte write
     */
    wrval = wrval | SET_PHY_BIT15; /* Mode Software Reset */
    utah_phy_reg_wr(portnum, GENERAL_CTRL1_REG20, wrval);

    /* Read back to check for reset done */
    do {
      msleep(10);
      utah_phy_reg_rd(portnum, GENERAL_CTRL1_REG20, &rdval);
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
int set_eng_detect(boolean onoff, int portnum)
{
    ushort rdval, wrval;

    /* go to page 0*/
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0);
    utah_phy_reg_rd(portnum, COP_SPEC_CTRL_REG16, &rdval);
   
    if(onoff)
        wrval = rdval & ~SET_ENG_DETECT;  /*disable energy detect*/
    else
        wrval = rdval | SET_ENG_DETECT;  /* restore energy detect*/
   	
    utah_phy_reg_wr(portnum, COP_SPEC_CTRL_REG16, wrval);

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
int set_mac_speed(int portnum, int speed)
{
    ushort rdval, wrval;

    /*go to page 2*/
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE2);
    utah_phy_reg_rd(portnum, MAC_SPEC_CTRL2_REG21, &rdval);

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
        printf("Port %d not support speed %d on MAC\n", portnum, speed);
      break;
    }

    utah_phy_reg_wr(portnum, MAC_SPEC_CTRL2_REG21, wrval);

    /* need follow by reset 
     */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, OVLD_PHY_PAGE0);
    utah_phy_reg_rd(portnum, COP_CTRL_REG0, &rdval);

    wrval = rdval | PHY_REG_BIT(15);
    utah_phy_reg_wr(portnum, COP_CTRL_REG0, wrval);

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

#if TWO_PHY
    /* Disable macsec
     */
    if (set_macsec(onoff, port)) {  
    	printf("set_macsec failed. ");
    	return(FAILED);
    }

    /* The media PHY turn off auto media detect mode */
    if (set_automedia(onoff, port)) {
    	printf("set automedia failed. ");
    	return(FAILED);
    }

    /* turn off energy detect, to prevent the lpbk stub is not plug-in. */
    if(set_eng_detect(onoff, port)) {
    	printf("set eng detect failed. ");
    	return(FAILED);
    }
#endif

    /* Force copper linke up. Needed for 1548P part
     */
    force_linkup(port, onoff);

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
int set_media_phy_int_lpbk(char *type, int portnum, int speed)
{
    char pname[10];
    int rc = 0;
    ushort rdval, wrval;
   
    sprintf(pname,"%s%d", type, portnum);   

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, portnum, INT_LPBK)) != PASSED){
    	printf("init_sgmii_env failed \n");
    	return(FAILED);
    }

    /* turn off stub loopback */
    if ((rc = set_phy_stub(portnum, INT_LPBK, SIG_COPPER)) != PASSED){
    	printf("set_phy_stub failed \n"); 
    	return(FAILED);
    }   

#if TWO_PHY
    /* Per marvell FAE. Make sure bridge phy autoneg is on
     */
    bridge_phy_autoneg_on(portnum);
#endif

    /* set env for media PHY internal loopback */
    if ((rc = set_media_int_lpbk(ENABLE_SIG, portnum)) != PASSED){
    	printf("set_media_int_lpbk failed \n"); 
    	return(FAILED);
    }

    /* Note: marvell FAE suggest the sequence up to the end.
     * Please keep the code this way.
     */
    if(set_mac_speed(portnum, speed)) {
    	printf("set mac speed failed. ");
    	return(FAILED);
    }
    

#if TWO_PHY
    /* Per Marvell FAE: Switch to another page is needed, the cavium
     * will aware the page is change, and will poll the current 
     * reg. mask this part may cause the driver can not detect 
     * current setting.
     */
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, SIG_FIBER);
    sleep(ETH_DRIVER_DELAY);	
    utah_phy_reg_wr(portnum, OVLD_PHY_PAGE22, SIG_COPPER);
    sleep(ETH_DRIVER_DELAY);	

#endif 
    /* Make sure media phy copper reset bit is 0
     */
    utah_phy_reg_rd(portnum, COP_CTRL_REG0, &rdval);
    if ((rdval & PHY_REG_BIT(15)) == 1) {
        printf("Media PHY reg 0_0:15 reset not cleared\n");
    }

    /* Per FAE instruction, set the copper internal loopback bit
     * in a second write
     */
    utah_phy_reg_rd(portnum, COP_CTRL_REG0, &rdval);
    wrval = rdval | PHY_REG_BIT(14);
    utah_phy_reg_wr(portnum, COP_CTRL_REG0, wrval);

    /* Make sure loopback bit is set
     */
    utah_phy_reg_rd(portnum, COP_CTRL_REG0, &rdval);
    if ((rdval & PHY_REG_BIT(14)) == 0) {
      printf("Media PHY reg 0_0:14 loopback not set\n");
    }

    /* Note: This delay time is critical for the port to become
     * stable.
     * Bug Fix: CSCuc64054, Overlord data plane 1548 PHY loopback test failed
     * Also needed in Utah
     */
    sleep(ETH_DRIVER_DELAY*3);	
  
#if TWO_PHY
    if (!is_media_phy_copper_linkup(portnum) ||
	!is_media_phy_qsgmii_linkup(portnum) ||
	!is_bridge_phy_qsgmii_linkup(portnum + ADDR_BRIDGE_PHY)) {
        return(FAILED);
    }
#endif
    
    if ((rc = utah_port_is_linkup(portnum)) != PASSED) {
        printf("sgmii link up time out after 1 second \n");
	show_status_info(portnum);
	return(FAILED);
    }

#if DEBUG
    printf("\n1548 & 1340 PHY's links are up before sending packets.\n");
#endif

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
    if ((rc = set_phy_stub(port, INT_LPBK, SIG_COPPER)) != PASSED){
    	printf("set_phy_stub failed\n"); 
    	return(FAILED);
    }   

     /* without loopback stub, the PHY should forced link up here
      * to let setting of init_sgmii_env is working properly.
      */
    force_linkup(port, ENABLE);

    /* ensure the advertisement register is not turn on half duplex */
    if(sgmii_adv_full_duplex(ENABLE, (port + ADDR_MEDIA_PHY))) {
    	printf("sgmii_adv_full_duplex failed. ");
    	return(FAILED);
    } 

    /* soft reset makes setting of media PHY is work. */
    if ((rc = ovld_phy_soft_reset(port, SIG_COPPER)) != PASSED){
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
   	
    force_linkup(port, DISABLE);

    if (rc != PASSED) {
        printf("ovld_set_packet failed %s\n",__FUNCTION__);
        return(FAILED);
    }
    
    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: Dagger_set_media_int_lpbk
 *   Set up to prepare for media PHY internal loopback for dagger
 *   sepcific.
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_set_media_int_lpbk(boolean onoff, int port)
{
    int spe_port;

    /* Disable macsec, this step is necessary.
     */
    if (set_macsec(onoff, port)) {
        printf("set_macsec failed. ");
        return(FAILED);
    }

    /* The media PHY turn off auto media detect mode */
    if (set_automedia(onoff, port)) {  
        printf("set automedia failed. ");
        return(FAILED);
    }

    /* turn off energy detect, to prevent the lpbk stub is not plug-in. */
    if(set_eng_detect(onoff, port)) {
        printf("set eng detect failed. ");
        return(FAILED);
    }

    /* NOTE: GE1 is using phy port2 for qsgmii, port3 for sgmii,
     * somehow the system will look into phy port1 for speed,
     * once port1 is not configure the speed, the GE1 will not link up.
     * Therefore, we also need to configure phy port1 speed.
     */
    if (port == SGMII3) {
        spe_port = SGMII1;
        force_linkup(spe_port, onoff);
    }

    /* Force copper linke up. Needed for 1548P part
     */
    force_linkup(port, onoff);

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: dagger_set_media_phy_int_lpbk
 *   initial and setup loopback type on sgmii for internal lpbk
 *   for dagger specific.
 *
 * Input:  onoff - turn on/off
 *         port - setup port
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_set_media_phy_int_lpbk (char *type, int portnum, int speed) {

    char pname[16], spe_pname[16];
    int rc = 0, spe_port;
    ushort rdval, wrval;

    /* NOTE: GE1 is using phy port2 for qsgmii, port3 for sgmii,
     * somehow the system will look into phy port1 for speed,
     * once port1 is not configure the speed, the GE1 will not link up.
     * Therefore, we also need to configure phy port1 speed. 
     */
    if (portnum == SGMII3) {
        spe_port = SGMII1;
        sprintf(spe_pname,"%s%d", type, spe_port);
        if ((rc = init_sgmii_env(spe_pname, speed, spe_port, INT_LPBK)) != PASSED){
            printf("init_sgmii_env failed \n");
            return(FAILED);
        }
    }

    sprintf(pname,"%s%d", type, portnum);

    /* init sgmii environment for loopback */
    if ((rc = init_sgmii_env(pname, speed, portnum, INT_LPBK)) != PASSED){
        printf("init_sgmii_env failed \n");
        return(FAILED);
    }
 

    /* turn off stub loopback */
    if ((rc = set_phy_stub(portnum, INT_LPBK, SIG_COPPER)) != PASSED){
        printf("set_phy_stub failed \n");
        return(FAILED);
    }

    /* set env for media PHY internal loopback */
    if ((rc = dagger_set_media_int_lpbk(ENABLE_SIG, portnum)) != PASSED){
        printf("set_media_int_lpbk failed \n");
        return(FAILED);
    }

    /* Note: marvell FAE suggest the sequence up to the end.
     * Please keep the code this way.
     */
    if(set_mac_speed(portnum, speed)) {
        printf("set mac speed failed. ");
        return(FAILED);
    }
   
    /* Per FAE instruction, set the copper internal loopback bit
     * in a second write
     */
    utah_phy_reg_rd(portnum, COP_CTRL_REG0, &rdval);
    wrval = rdval | PHY_REG_BIT(14);
    utah_phy_reg_wr(portnum, COP_CTRL_REG0, wrval);

    sleep(ETH_DRIVER_DELAY*3);

#if 0
    if ((rc = utah_port_is_linkup(portnum)) != PASSED) {
        printf("sgmii link up time out after 1 second \n");
        show_status_info(portnum);
        return(FAILED);
    }
#endif 

    return(PASSED);
}

/*------------------------------------------------------------------
 *
 * Function: dagger_int_lpbk_test
 *   This is the entry point for dagger internal loopback test only.
 *
 * Input: port - port number
 *        speed - 10, 100, or 1000MB
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_int_lpbk_test (int port, int speed) {
 
    int rc = 0;
    int qsgmii_port = 0, sgmii_port = 0;
    ushort rdval, wrval;

    /* dagger GE0 -> qsgmii = port0, sgmii = port1
     * dagger GE1 -> qsgmii = port2, sgmii = port3
     */
    if (port == SGMII0) {
       sgmii_port = SGMII1;
       qsgmii_port = SGMII0;
    } else if (port == SGMII1) {
       sgmii_port = SGMII3;
       qsgmii_port = SGMII2;
    }

    /* set qsgmii port to page 4, which is used for QSGMII
     * and ensure it is powered on. SGMII port will power on
     * via function sig_pwr_ctrl().
     */
    utah_phy_reg_wr(qsgmii_port, PHY_REG(22), PHY_REG(4));
    utah_phy_reg_rd(qsgmii_port, PHY_REG(0), &rdval);
    wrval = rdval & ~SET_PHY_BIT11; /* power up */
    utah_phy_reg_wr(qsgmii_port, PHY_REG(0), wrval);

    /* setup loopback information */
    rc = dagger_set_media_phy_int_lpbk(SEL_PORT_ETH, sgmii_port, speed);

    if (rc == FAILED) {
       printf("dagger_sgmii_set_phy_int_lpbk failed, port: %d \n", sgmii_port);
       dagger_set_media_int_lpbk(DISABLE_SIG, sgmii_port);
       return(FAILED);
    }

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    rc = ovld_set_packet(port, speed);

    /* Restore the original setting to prevent ext lpbk error occur.*/
    dagger_set_media_int_lpbk(DISABLE_SIG, sgmii_port);

    /* wait for driver get the packet then restore the setting */
    sleep(ETH_DRIVER_DELAY);

    return rc;
}

/*------------------------------------------------------------------
 *
 * Function: dagger_ext_lpbk_test
 *   This is the entry point for dagger external loopback test only.
 *
 * Input: port - port number
 *        speed - 10, 100, or 1000MB
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_ext_lpbk_test (int port, int speed) {

    int qsgmii_port = 0, sgmii_port = 0;
    ushort rdval, wrval;

    /* dagger GE0 -> qsgmii = port0, sgmii = port1 
     * dagger GE1 -> qsgmii = port2, sgmii = port3 
     */
    if (port == SGMII0) {
       sgmii_port = SGMII1;
       qsgmii_port = SGMII0;
    } else if (port == SGMII1) {
       sgmii_port = SGMII3;
       qsgmii_port = SGMII2;
    }

    /* set qsgmii port to page 4, which is used for QSGMII
     * and ensure it is powered on. SGMII port will power on 
     * via function sig_pwr_ctrl().
     */
    utah_phy_reg_wr(qsgmii_port, PHY_REG(22), PHY_REG(4));
    utah_phy_reg_rd(qsgmii_port, PHY_REG(0), &rdval);
    wrval = rdval & ~SET_PHY_BIT11; /* power up */
    utah_phy_reg_wr(qsgmii_port, PHY_REG(0), wrval);

    if (set_media_phy_ext_lpbk(SEL_PORT_ETH, sgmii_port, speed)) {
        printf("set_media_phy_ext_lpbk failed, port: %d \n", sgmii_port);
        return(FAILED);
    }

    /* set packet from the packet table and call tx_rx_diag for to transfer it.  */
    if (ovld_set_packet(port, speed)) {
        printf("ovld_set_packet failed %s\n", __FUNCTION__);
        return(FAILED);
    }

    return(PASSED);
}


/*------------------------------------------------------------------
 *
 * Function: dagger_phy_lpbk_test
 *   
 * Description: SGMII port PHY internal or external loopback test
 *              internal lpbk test: GE0->PHY port0, port1. 
 *         
 * Input: lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 * 
 * Return: pass/fail
 *
 * Note: dagger has special PHY setting for supprot port qsgmii 
 *       crossover. 
 *       GE0 (system) -> qsgmii port0 -> sgmii port1 -> sgmii media
 *       GE1 (system) -> qsgmii port2 -> sgmii port3 -> sgmii media
 *------------------------------------------------------------------
 */ 
int 
dagger_phy_lpbk_test(int lpbkmode)
{       
    int rc = 0;
    int port_cnt, port_curr, port, sgmii_port;
    int speed_cnt, speed_curr, speed;
    int try, retry_limit = 2; 
    
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
    
    /* for Sword, Dagger, Goldbeach, there are only two GE ports  */
    if (is_goldbeach()) { 
        /* Goldbeach eth1 used for management port, without loopback teset.
           Goldbeach loopback test only for eth0 */ 
        port_cnt = sizeof(eth_port0_list_gb) / sizeof(int);
    } else {
        port_cnt = sizeof(eth_port_list_sd) / sizeof(int);
    }
    speed_cnt = sizeof(dagger_eth_speed_list) / sizeof(int);

    for(port_curr = 0; port_curr < port_cnt; port_curr++) {
        if (is_goldbeach()) { 
            port = eth_port0_list_gb[port_curr];
        } else {
            port = eth_port_list_sd[port_curr];
        }

        /* for clean up error */
        if (port == SGMII0) {
           sgmii_port = SGMII1;
        } else {
           sgmii_port = SGMII3;
        }

        for(speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            speed = dagger_eth_speed_list[speed_curr];

            /* skip 1000Mbps on all the internal loopback test*/
            if((speed == SPD_1000MBPS) &&
               ((lpbkmode != SGMII_EXT_LPBK) && (lpbkmode != SGMII_INT_EXT_LPBK)) &&
                (lpbkmode != GOLDBEACH_ETH0_INT_EXT_LPBK)) {
                continue;
            }
            switch(lpbkmode) {
            case DAGGER_PHY_INT_LPBK:
                testname("Dagger PHY internal loopback");
                prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);

                for (try=0; try < retry_limit; try++) {
                    rc = dagger_int_lpbk_test(port, speed);
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
                           "1. PHY is bad or not soldered\n");
                    cterr('f',0,"Dagger PHY int loopback port %d failed\n", port);
                }
                break;

            case DAGGER_SGMII_EXT_LPBK:
                testname("Dagger SMGII external loopback");
                if (!check_ext_lpbk_flag()) {
                  prpass(testpass, "Test skipped. Ext. loopback diag flag is off. ");
                  return(PASSED);
                }

                prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);

                for (try=0; try < retry_limit; try++) {
                    rc = dagger_ext_lpbk_test(port, speed);
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
                           "2. PHY external path bad. Try to run internal loopback test.\n"
                           "3. PHY is bad or not soldered\n");
                    cterr('f',0,"Dagge SGMII PHY ext loopback port %d failed\n", port);
                }
                break;

            case DAGGER_SGMII_INT_EXT_LPBK:
            case GOLDBEACH_ETH0_INT_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */

                if (check_ext_lpbk_flag()) {
                  /* to verify the interface, running 1000MBps is enough. 
                   * skip 10 and 100Mbps on external loopback test
                   */
                  if((speed == SPD_100MBPS) || (speed == SPD_10MBPS)) {
                      continue;
                  }

                  testname("SGMII external loopback");
                  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
                  for (try=0; try < retry_limit; try++) {
                      rc = dagger_ext_lpbk_test(port, speed);
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
                           "2. PHY external path bad. Try to run internal loopback test.\n"
                           "3. PHY is bad or not soldered\n");
                      cterr('f',0,"Dagge SGMII PHY ext loopback port %d failed\n", port);
                      printf("trying internal loopback...");
                      return(dagger_phy_lpbk_test(DAGGER_PHY_INT_LPBK));

                  }
                }

                if ((!check_ext_lpbk_flag()) && (speed != SPD_1000MBPS)) {

                  /* cover the ext one */
                  if (get_enhance_err_flag()) {
                      int_lpbk_add_err_report();
                  }

                  testname("Dagger PHY internal loopback");
                  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
                  for (try=0; try < retry_limit; try++) {
                      rc = dagger_int_lpbk_test(port, speed);
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
                           "1. PHY is bad or not soldered\n");
                      cterr('f',0,"Dagger PHY int loopback port %d failed\n", port);
                  }
                }

                break;

            default:
                rc = FAILED;
                cterr('f',0," Dagger is not support this loopback mode\n");
                break;
            } /*switch*/

            if (rc != PASSED) {
              ovld_err_clean_up(sgmii_port);
              return(FAILED); /* Do not continue on next port */
            }

        } /*speed*/
    }/*port*/


    return(rc);
}

/*------------------------------------------------------------------
 *
 * Function: vg400_ge_phy_lpbk_test
 *   
 * Description: SGMII port PHY internal or external loopback test
 *              internal lpbk test: GE0->PHY port0, port1. 
 *         
 * Input: lpbkmode - loopback mode (LOOP_INT or LOOP_EXT)
 * 
 * Return: pass/fail
 *
 *------------------------------------------------------------------
 */ 
int 
vg400_ge_phy_lpbk_test(int lpbkmode)
{       
    int rc = 0;
    int port_cnt, port_curr, port, sgmii_port;
    int speed_cnt, speed_curr, speed;
    int try, retry_limit = 2; 
    
    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);
    
    /* for VG400, there are only two GE ports  */
    if (lpbkmode == VG400_ETH0_INT_EXT_LPBK) { 
        port_cnt = sizeof(eth_port0_list_gb) / sizeof(int);
    } else if (lpbkmode == VG400_ETH1_INT_EXT_LPBK) {
        /* Eth1 used for management port, without Loopback Teset. */
        port_cnt = sizeof(eth_port1_list_gb) / sizeof(int);
    }
    speed_cnt = sizeof(vg400_eth_speed_list) / sizeof(int);

    for (port_curr = 0; port_curr < port_cnt; port_curr++) {
        if (lpbkmode == VG400_ETH0_INT_EXT_LPBK) { 
            port = eth_port0_list_gb[port_curr];
        } else if (lpbkmode == VG400_ETH1_INT_EXT_LPBK) {
            port = eth_port1_list_gb[port_curr];
        } else {
            port = eth_port_list_sd[port_curr];
        }

        /* for clean up error */
        if (port == SGMII0) {
           sgmii_port = SGMII1;
        } else {
           sgmii_port = SGMII3;
        }

        for(speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            speed = dagger_eth_speed_list[speed_curr];

            /* skip 1000Mbps on all the internal loopback test*/
            if((speed == SPD_1000MBPS) &&
                ((lpbkmode != VG400_ETH0_INT_EXT_LPBK) && (lpbkmode != VG400_ETH1_INT_EXT_LPBK))) {
                    continue;
            }
            switch(lpbkmode) {
            case DAGGER_SGMII_INT_EXT_LPBK:
            case VG400_ETH1_INT_EXT_LPBK:
            case VG400_ETH0_INT_EXT_LPBK:
                /* 1. if Ext. loopback flag is on, perform Ext. loopback test;
                 *    if failed, perform Internal loopback test.
                 * 2. if Ext. loopback flag is off, perform Int. loopback test directly.
                 */

                if (check_ext_lpbk_flag()) {
                  /* to verify the interface, running 1000MBps is enough. 
                   * skip 10 and 100Mbps on external loopback test
                   */
                  if((speed == SPD_100MBPS) || (speed == SPD_10MBPS)) {
                      continue;
                  }

                  testname("VG400 PHY  external loopback");   
                  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
                  for (try=0; try < retry_limit; try++) {
                      rc = dagger_ext_lpbk_test(port, speed);
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
                              "2. PHY external path bad. Try to run internal loopback test.\n"
                              "3. PHY is bad or not soldered\n");
                      cterr('f',0,"VG400 SGMII PHY ext loopback port %d failed\n", port);
                      printf("trying internal loopback...");
                      return(vg400_ge_phy_lpbk_test(DAGGER_PHY_INT_LPBK));
                  }
                }

                if ((!check_ext_lpbk_flag()) && (speed != SPD_1000MBPS)) {

                  /* cover the ext one */
                  if (get_enhance_err_flag()) {
                      int_lpbk_add_err_report();
                  }

                  testname("VG400 PHY internal loopback");
                  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
                  for (try=0; try < retry_limit; try++) {
                      rc = dagger_int_lpbk_test(port, speed);
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
                           "1. PHY is bad or not soldered\n");
                      cterr('f',0,"VG400 PHY int loopback port %d failed\n", port);
                  }
                }

                break;

            default:
                rc = FAILED;
                cterr('f',0," VG400 is not support this loopback mode\n");
                break;
            } /*switch*/

            if (rc != PASSED) {
              ovld_err_clean_up(sgmii_port);
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
 ***********************************************************************
 */
int
dagger_phy_lpbk_util(void)
{
    int port, low_port = 0, rc = 0;
    int max_port;
    int speed, spdsel;
    int lpbkmode, lpbksel;
    char qrybuf[64];

    low_port = SGMII0;

    printf("\nSelect loopback 1:MEDIA_PHY_INT_LPBK");
    printf("\n                2:SGMII_EXT_LPBK      3:SFP_EXT_LPBK ");
    lpbksel = getdec_answer("\nEnter ", 1, 1, 3);
    
    if ((lpbksel == DAGGER_PHY_INT_LPBK) || (lpbksel == DAGGER_SGMII_EXT_LPBK)) {
        max_port = SGMII1;
    } else {  /* DAGGER_SFP_EXT_LPBK */
        max_port = SGMII0;
        printf("Dagger has only one port and one speed for SFP \n");
        testname("Dagger SFP PHY external loopback");
        prpass(testpass, "Test SFP-%d, ", max_port);
        return(dagger_sfp_phy_ext_lpbk_test());
    }

    sprintf(qrybuf, "\nEnter port number (%d - %d)", low_port, max_port);
    port = getdec_answer(qrybuf, low_port, low_port, max_port);

    if(lpbksel == DAGGER_SGMII_EXT_LPBK) {
        lpbkmode = LOOP_EXT;
    } else { /*internal loopback */
        lpbkmode = LOOP_INT;
    }

    if (lpbkmode == LOOP_INT) {
      sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS)");
      spdsel = getdec_answer(qrybuf, 0, 0, 1);
      speed = (spdsel == 0) ? SPD_10MBPS : SPD_100MBPS;

      testname("Dagger PHY internal loopback");
      prpass(testpass, "Test SGMII-%d with speed-%d, ", port, speed);
      rc = dagger_int_lpbk_test(port, speed);

    } else {
      sprintf(qrybuf, "\nEnter speed (0: 10MBS, 1: 100MBS, 2: 1000MBS)");
      spdsel = getdec_answer(qrybuf, 0, 0, 2);
      switch(spdsel) {
        case 0:
                speed = SPD_10MBPS;
        break;
        case 1:
                speed = SPD_100MBPS;
        break;
        case 2:
                speed = SPD_1000MBPS;
        break;
        default:
                printf("\n not support this speed. ");
        break;
      }

      testname("Dagger PHY external loopback");
      prpass(testpass, "Test SGMII-%d with speed-%d, ", port, speed);
      rc = dagger_ext_lpbk_test(port, speed);
    }

    if (rc == FAILED) {
        printf("Keep the failing state.\n");
        cterr('f',0,"Loopback test failed on port %d speed %d \n", port, speed);
    }

    return(rc);
}


/*------------------------------------------------------------------
 *
 * Function: dagger_phy_int_lpbk_test
 *   dagger phy internal loopback test.
 *
 * Input: NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_phy_int_lpbk_test (void)
{
    if (get_enhance_err_flag()) {
        int_lpbk_add_err_report();
    }

    return(dagger_phy_lpbk_test(DAGGER_PHY_INT_LPBK));
}

/*------------------------------------------------------------------
 *
 * Function: dagger_phy_ext_lpbk_test
 *   dagger phy external loopback test.
 *
 * Input : NONE
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_phy_ext_lpbk_test (void)
{
    if (get_enhance_err_flag()) {
        ext_lpbk_add_err_report();
    }

    return(dagger_phy_lpbk_test(DAGGER_SGMII_EXT_LPBK));
}

/*------------------------------------------------------------------
 *
 * Function: dagger_phy_int_ext_lpbk_test
 *   dagger phy int/external loopback test.
 *
 * Input : NONE
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int dagger_phy_int_ext_lpbk_test (void)
{
    if (get_enhance_err_flag()) {
        ext_lpbk_add_err_report();
    }
    if (is_goldbeach()) { 
        return(dagger_phy_lpbk_test(GOLDBEACH_ETH0_INT_EXT_LPBK));
    } else {
        return(dagger_phy_lpbk_test(DAGGER_SGMII_INT_EXT_LPBK));
    }
}

/*------------------------------------------------------------------
 *
 * Function:  vg400_ge_phy_int_ext_lpbk_test
 *   VG400 GE phy int/external loopback test.
 *
 * Input : NONE
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int vg400_ge_phy_int_ext_lpbk_test (uint32_t eth_no) {

    if (get_enhance_err_flag()) {
         ext_lpbk_add_err_report();
    }
    if (eth_no == ETH_PORT1) {
        return(vg400_ge_phy_lpbk_test(VG400_ETH1_INT_EXT_LPBK));
    } else {
        return(vg400_ge_phy_lpbk_test(VG400_ETH0_INT_EXT_LPBK));
    }
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
    int try, retry_limit = 2;
    int *port_num;

    memset((unsigned char *)tx_packet, 0, ETH_PKT_MAX_LEN);
    memset((unsigned char *)rx_packet, 0, ETH_PKT_MAX_LEN);

    /* get test envrionment variable */
    if(is_utah()) {
        port_cnt = sizeof(eth_port_list) / sizeof(int);
        port_num = eth_port_list;
    } else {
        /* for Sword or Dagger, there are only two GE ports  */
        port_cnt = sizeof(eth_port_list_sd) / sizeof(int);
        port_num = eth_port_list_sd;
    }
    speed_cnt = sizeof(eth_speed_list) / sizeof(int);

    for(port_curr = 0; port_curr < port_cnt; port_curr++) {
        port = *(port_num+port_curr);

        for(speed_curr = 0; speed_curr < speed_cnt; speed_curr++) {
            speed = eth_speed_list[speed_curr];
	    
	    /* skip 1000Mbps on all the internal loopback test*/
	    if((speed == SPD_1000MBPS) && 
	       ((lpbkmode != SGMII_EXT_LPBK) && (lpbkmode != SGMII_INT_EXT_LPBK))) {
	        continue;
	    }

	    switch(lpbkmode) {
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
                    printf("\n\nAt the beginning of the test - Display Linux Ethernet counters - speed = %d\n", speed);
                    show_eth_counter(port);
		    rc = media_phy_ext_lpbk_test(port, speed);
		    if ((rc == PASSED) || (try == (retry_limit - 1))) {
		        break;
		    }
		    else {
                        printf("\n\nBefore retry the test - Display Linux Ethernet counters - speed = %d\n", speed);
                        show_eth_counter(port);
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
                  /* to verify the interface, running 1000MBps is enough. 
                   * skip 10 and 100Mbps on external loopback test
                   */
                  if((speed == SPD_100MBPS) || (speed == SPD_10MBPS)) {
                      continue;
                  }

	          testname("SGMII external loopback");   	
		  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
		  for (try=0; try < retry_limit; try++) {
                      printf("\n\nAt the beginning of the test - Display Linux Ethernet counters - speed = %d\n", speed);
                      show_eth_counter(port);
		      rc = media_phy_ext_lpbk_test(port, speed);
		      if ((rc == PASSED) || (try == (retry_limit - 1))) {
		          break;
		      }
		      else {
                          printf("\n\nBefore retry the test - Display Linux Ethernet counters - speed = %d\n", speed);
                          show_eth_counter(port);
		          printf("####### retry the test #########\n");
			  reset_quad_phy();
		      }
		  }

		  if (rc != PASSED) {
                      printf("\n\nIn the end of the test - Display Linux Ethernet counters - speed = %d\n", speed);
                      show_eth_counter(port);
		      printf("Possible causes of problem:\n"
			     "1. External loopback plug missing/bad.\n"
			     "2. Media PHY external path bad. Please run internal loopback test.\n"
			     "3. Media PHY is bad or not soldered\n");
		      cterr('f',0,"SGMII media PHY ext loopback port %d failed\n", port);
                      printf("trying internal loopback...");
                      return(ovld_phy_lpbk_test(MEDIA_PHY_INT_LPBK));
		  }
		}

                if ((!check_ext_lpbk_flag()) && (speed != SPD_1000MBPS)) {

                  /* cover the ext one */
                  if (get_enhance_err_flag()) {
                      int_lpbk_add_err_report();
                  }

	          testname("Media PHY internal loopback");   	
		  prpass(testpass, "Test SGMII-%d speed-%d, ", port, speed);
		  for (try=0; try < retry_limit; try++) {
                      printf("\n\nAt the beginning of the test - Display Linux Ethernet counters - speed = %d\n", speed);
                      show_eth_counter(port);
		      rc = media_phy_int_lpbk_test(port, speed);
		      if ((rc == PASSED) || (try == (retry_limit - 1))) {
		          break;
		      }
		      else {
                          printf("\n\nBefore retry the test - Display Linux Ethernet counters - speed = %d\n", speed);
                          show_eth_counter(port);
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
                printf("\n\nIn the end of the test - Display Linux Ethernet counters - speed = %d\n", speed);
                show_eth_counter(port);
	        ovld_err_clean_up(port);
	        return(FAILED); /* Do not continue on next port */
	    }

            /* Print counters when test passed */
            printf("\n\nIn the end of the test - Display Linux Ethernet counters - speed = %d\n", speed);
            show_eth_counter(port);
	} /*speed*/
    }/*port*/

    
    return(rc);
}

/**********************************************************************
 *
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
void show_eth_counter (int port) 
{
    char tx_pkt[64];
    char rx_pkt[64];
    char tx_err[64];
    char rx_err[64];
    char tx_drop[64];
    char rx_drop[64];

    sprintf(tx_pkt, "cat /sys/class/net/eth%d/statistics/tx_packets\n", port);
    sprintf(rx_pkt, "cat /sys/class/net/eth%d/statistics/rx_packets\n", port);
    sprintf(tx_err, "cat /sys/class/net/eth%d/statistics/tx_errors\n", port);
    sprintf(rx_err, "cat /sys/class/net/eth%d/statistics/rx_errors\n", port);
    sprintf(tx_drop, "cat /sys/class/net/eth%d/statistics/tx_dropped\n", port);
    sprintf(rx_drop, "cat /sys/class/net/eth%d/statistics/rx_dropped\n", port);

    printf("eth%d", port);

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


/**********************************************************************
 *
 * Function: set_sgmii_int_lpbk
 *
 * Description:
 * used by o2 for bridge phy loopback
 *
 * Input: port - port num, dummy  
 *
 * Return: none
 **********************************************************************
 */
void set_sgmii_int_lpbk(int port, boolean dummy) {

    printf("fixed me !!! %s\n", __FUNCTION__);
    return;
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
    int max_port;
    int speed, spdsel;
    int lpbkmode, lpbksel;
    char qrybuf[64];   
 
    low_port = SGMII0;

    printf("\nSelect loopback 0:BRIDGE_PHY_INT_LPBK 1:MEDIA_PHY_INT_LPBK");
    printf("\n                2:SGMII_EXT_LPBK      3:SFP_EXT_LPBK ");
    lpbksel = getdec_answer("\nEnter ", 0, 0, 3);

    if ((is_sword() || is_dagger()) && (lpbksel == SGMII_EXT_LPBK)) {
        max_port = SGMII1;
    } else if (is_dagger() && (lpbksel == SFP_EXT_LPBK)) {
        max_port = SGMII0;
    } else if ((lpbksel == SGMII_EXT_LPBK) || (lpbksel == SFP_EXT_LPBK)) {
        max_port = SGMII2;
    } else {
        max_port = PLAT_SGMII_NUM_MAX;
    }

    if (is_sword() && (lpbksel == SFP_EXT_LPBK)) {
        sprintf(qrybuf, "\nEnter port number (%d or %d)" 
                ",SFP port 1 is not supported for Sword.", low_port, max_port);
    } else {
        sprintf(qrybuf, "\nEnter port number (%d - %d)", low_port, max_port);
    }
    port = getdec_answer(qrybuf, low_port, low_port, max_port);
 
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
 * Function: ovld_bridge_phy_int_lpbk_test
 *
 * Description: Ovld bridge PHY internal loopback test
 * 
 * Input: void
 *
 * Return: pass/fail
 */
int ovld_bridge_phy_int_lpbk_test(void)
{
    if (get_enhance_err_flag()) {
        int_lpbk_add_err_report();
    }

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
    if (get_enhance_err_flag()) {
        int_lpbk_add_err_report();
    }

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
int ovld_phy_ext_lpbk_test(void)
{
    if (get_enhance_err_flag()) {
        ext_lpbk_add_err_report();
    }

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
int ovld_sgmii_int_ext_lpbk_test(void)
{
    if (get_enhance_err_flag()) {
        ext_lpbk_add_err_report();
    }

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
    int port = 0, test_mode = 0;
    int qsgmii_port = 0, sgmii_port = 0, rc = 0;
    ushort rdval, wrval; 

    if (is_dagger()) {
        port = getdec_answer("Enter ge port number (0-1)", 0, 0, 1);
    } else { 
        port = getdec_answer("Enter ge port number (0-3)", 0, 0, 3);
    }
    
    if (is_dg_machines() || is_vg400()) {
       if (port == 0) {
           sgmii_port = SGMII1;
           qsgmii_port = SGMII0;
       } else {
           sgmii_port = SGMII3;
           qsgmii_port = SGMII2;
       }

       utah_phy_reg_wr(qsgmii_port, PHY_REG(22), PHY_REG(4));
       utah_phy_reg_rd(qsgmii_port, PHY_REG(0), &rdval);
       wrval = rdval & ~SET_PHY_BIT11; /* power up */
       utah_phy_reg_wr(qsgmii_port, PHY_REG(0), wrval);
    }

    printf("PHY(Marvell 1548) Supported TestMode:\n");
    printf("[0] Normal Mode.\n");
    printf("[1] Transmit Waveform Test.\n");
    printf("[2] Transmit Jitter Test (Master).\n");
    printf("[3] Transmit Jitter Test (Slave).\n");
    printf("[4] Transmit Distortion Test.\n");
    if (is_goldbeach() || is_vg400()) { 
        printf("[5] 10M Pseudo-Random Test.\n");
        printf("[6] 10M Data 0/1 Test.\n");
        printf("[7] 100M Waveform Test. .\n");
        test_mode = getdec_answer("Enter Test mode (0-7)", 0, 0, 7);
    } else {
        test_mode = getdec_answer("Enter Test mode (0-4)", 0, 0, 4);
    }


    if (is_dagger()) {
        rc = ovld_set_media_phy_testmode(sgmii_port, (uint8_t)test_mode);
    } else if (is_goldbeach() || is_vg400()) {
        rc = goldbeach_set_media_phy_testmode(sgmii_port, (uint8_t)test_mode);
    } else {
        rc = ovld_set_media_phy_testmode(port, (uint8_t)test_mode);
    }

    if (rc == FAILED) {
        printf("%s: FAILED to set Eth%d PHY(1548) into TestMode%d.\n",
               __FUNCTION__, port, test_mode);
        return (FAILED);
    }

    printf("\nNow Eth%d PHY(1548) enter TestMode%d, and press \'q\' to exit: ",
           port, test_mode);

    while (1) {
        if(getchar() == 'q') {
            if (is_goldbeach() || is_vg400()) {
                ovld_phy_soft_reset(sgmii_port, SIG_COPPER);
                ovld_phy_soft_reset(qsgmii_port, SIG_COPPER);
            } else {
                ovld_phy_soft_reset(port, SIG_COPPER);
            }
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
    uint16_t ret_val = 0;
    int  reg_val = 0;

    if (utah_phy_reg_wr(port, reg_off, val) != PASSED) {
        printf("\n%s: Failed to write Eth%d PHY(1548) Reg%.2d to 0x%08X.\n",
               __FUNCTION__, port, reg_off, val);
        return (FAILED);
    }

    /* Use the read back value to confirm the setting */
    if(utah_phy_reg_rd(port, reg_off, &ret_val)) {
        printf("\n%s: Failed to read Eth%d PHY(1548) Reg%.2d (ret = %#X).\n",
               __FUNCTION__, port, reg_off, ret_val);
        return (FAILED);
    }

    reg_val = ret_val;
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
        return (ovld_phy_soft_reset(port, SIG_COPPER));
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
            if (ovld_phy_soft_reset(port, SIG_COPPER) != PASSED) {
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
 * Function   : goldbeach_set_media_phy_testmode
 * Description: Main function to set Goldbeach PHY(Marvell 1548) into Test mode.
 * Inputs     : port - Port number that want to set into Test mode
 *              test_mode - Type of Test mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
/*******************************************************************************
From FAE : 
Please refer 1G test mode 1 setting as below
Test Mode 1, 2 or 4:
Write Reg 22 = 0x0000
Write Reg 9  = 0x1F00
Write Reg 0  = 0x9140
Write Reg 22 = 0x0004
Write Reg 27 = 0x3E80
Write Reg 22 = 0x0006
Write Reg 26 = 0x8000 
Write Reg 22 = 0x0000 
For Test mode 1: Write Reg 9 = 0x3F00
For Test mode 2: Write Reg 9 = 0x5F00
For Test mode 4: Write Reg 9 = 0x9F00
(Take desired measurements. Restore normal values of the registers 
and exit the test mode by clearing bits 9.15:13)
Write Reg 0 = 0x9140 Soft-reset
*******************************************************************************
 */
int
goldbeach_set_media_phy_testmode (int port, uint8_t test_mode)
{
    int ctr = 0, total_steps = 0;
    uint16_t testmode_val = 0;
    mrvl_phy_setup_t *step_ptr;
    int repeat = 100;
    ushort rdval;
    
    if ((test_mode == PHY_TESTMODE_1) || (test_mode == PHY_TESTMODE_2) ||
        (test_mode == PHY_TESTMODE_4)) {
        step_ptr = &gb_phy_testmode124_steps[0];
        total_steps = sizeof(gb_phy_testmode124_steps) / sizeof(mrvl_phy_setup_t);

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
        step_ptr = &gb_phy_testmode3_steps[0];
        total_steps = sizeof(gb_phy_testmode3_steps) / sizeof(mrvl_phy_setup_t);

        /* Enable Test mode 3: 0x7700 */
        testmode_val = 0x7700;
    } else if (test_mode == PHY_TESTMODE_5) {
        step_ptr = &gb_phy_testmode5_steps[0];
        total_steps = sizeof(gb_phy_testmode5_steps) / sizeof(mrvl_phy_setup_t);
    } else if (test_mode == PHY_TESTMODE_6) {
        step_ptr = &gb_phy_testmode6_steps[0];
        total_steps = sizeof(gb_phy_testmode6_steps) / sizeof(mrvl_phy_setup_t);
    }  else if (test_mode == PHY_TESTMODE_7) {
        step_ptr = &gb_phy_testmode7_steps[0];
        total_steps = sizeof(gb_phy_testmode7_steps) / sizeof(mrvl_phy_setup_t);
    } else if (test_mode == PHY_TESTMODE_NORMAL) {
        return (ovld_phy_soft_reset(port, SIG_COPPER));
    } else {
        printf("%s: Not support TestMode%d.\n", __FUNCTION__, test_mode);
        return (FAILED);
    } 

    for (ctr = 0; ctr < total_steps; ctr++, step_ptr++) {
        /* Jump to page of register that want to set */
        prpass(testpass, "Set SGMII%d to TestMode%d: Jump to page%d",
                   port, test_mode, step_ptr->reg_page);
        if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG22, step_ptr->reg_page,
                                        step_ptr->mask) != PASSED) {
            printf("\n%s: Failed to jump to SGMII%d PHY(1548) page%d.\n",
                   __FUNCTION__, port, step_ptr->reg_page);
            return (FAILED);
        }

        /* Set register */
        prpass(testpass, "Set SGMII%d to TestMode%d: Set page%d Reg%.2d to 0x%04X",
               port, test_mode, step_ptr->reg_page, step_ptr->reg_off, step_ptr->val);
        printf("\n");
        if (ovld_set_media_phy_by_value(port, step_ptr->reg_off, step_ptr->val,
                                        step_ptr->mask) != PASSED) {
            printf("\n%s: Failed to set Eth%d PHY(1548) page%d Reg%.2d to 0x%04X.\n",
                   __FUNCTION__, port, step_ptr->reg_page,
                   step_ptr->reg_off, step_ptr->val);
            return (FAILED);
        }

        /* The following code is per the Marvell 88E1548P PHY */
        /* Reset reg 0_0.15=1 */
        /* Use signal to select page for copper or fiber */
        /* Read back to check for reset done */
    	if ((step_ptr->reg_page == OVLD_PHY_PAGE0) &&
            (step_ptr->reg_off == OVLD_PHY_REG0) &&
            ((step_ptr->val) & SET_PHY_BIT15)) {
	    do {
	        msleep(100);
	        utah_phy_reg_rd(port, PHY_REG(0), &rdval);
	    } while((repeat-- > 0) && (rdval & SET_PHY_BIT15));
    }

    }
    if ((test_mode == PHY_TESTMODE_5) || (test_mode == PHY_TESTMODE_6) ||
        (test_mode == PHY_TESTMODE_7)) {
        return (PASSED);
    }
    /* Set Test mode by write page0 Reg 9 */
    /* Jump to page of register that want to set */
    prpass(testpass, "Set SGMII%d to TestMode%d: Jump to page%d",
           port, test_mode, OVLD_PHY_PAGE0);
    if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG22, OVLD_PHY_PAGE0, 0xC0FF)
        != PASSED) {
        printf("\n%s: Failed to jump to SGMII%d PHY(1548) page%d.\n",
               __FUNCTION__, port, OVLD_PHY_PAGE0);
        return (FAILED);
    }
    /* Set register */
    prpass(testpass, "Set SGMII%d to TestMode%d: Set page%d Reg%.2d to 0x%04X",
           port, test_mode, OVLD_PHY_PAGE0, OVLD_PHY_REG9, testmode_val);
    printf("\n");
    if (ovld_set_media_phy_by_value(port, OVLD_PHY_REG9, testmode_val, step_ptr->mask)
        != PASSED) {
        printf("\n%s: Failed to set SGMII%d PHY(1548) page%d Reg%.2d to 0x%04X.\n",
               __FUNCTION__, port, OVLD_PHY_PAGE0, OVLD_PHY_REG9, testmode_val);
        return (FAILED);
    }
    return (PASSED);
}
/*******************************************************************************
 *
 * Function   : tmp_ovld_set_packet
 * Description: send packet to specific port 
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tmp_ovld_set_packet(void) {
 
    int port, speed;

    printf("This util send packets with specific speed.\n");
    printf("It does not config ports, but checks packets are loopback.\n"); 

    port = getdec_answer("\nEnter  port", 0, 0, 3);
    speed = getdec_answer("\nEnter  speed 0:10 100:1 1000:2", 0, 0, 2);
    if( speed == 0) 
       speed = 10;
    else if (speed ==1)
       speed = 100;
    else 
       speed = 1000;
    return (ovld_set_packet(port, speed));

}

/*******************************************************************************
 * 
 * Function   : phy_1548_interrupt_test
 * Description: Do PHY 1548 interrupt test 
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int phy_1548_interrupt_test (void) {    
    int int_sts = 0;  /* The interrupt status get form FPGA */
    int intr_timeout = INTR_TIMEOUT;
    char *tname = "PHY 1548 Interrupt";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname); 


    /* Reset PHY  Register */
    reset_quad_phy();

    /* Read Interrupt Register Status From FPGA */
    get_platform_misc_sts(); 

    /* Enable Speed change Interrupt on PHY 1548 */
    /* Change to Page 4 */
    utah_phy_reg_wr(SGMII0, PHY_REG(22), OVLD_PHY_PAGE4);
    /* Set Reg 18 (Intr Reg) */
    utah_phy_reg_wr(SGMII0, PHY_REG(18), SPEED_INT);
    /* Change to Page 0 */
    utah_phy_reg_wr(SGMII0, PHY_REG(22), OVLD_PHY_PAGE0); 
    /* Set Speed 1000Mbps */
    utah_phy_reg_wr(SGMII0, PHY_REG(0), SPEED_1000M);
    if (ovld_phy_soft_reset(SGMII0, SIG_COPPER) == FAILED) {
        cterr('f',0,"Soft reset failed \n");
        return (FAILED);
    }
    /* Set Speed 100Mbps */
    utah_phy_reg_wr(SGMII0, PHY_REG(0), SPEED_100M);
    if (ovld_phy_soft_reset(SGMII0, SIG_COPPER) == FAILED) {
        cterr('f',0,"Soft reset failed \n");
        return (FAILED);
    }

    /* Read Interrupt Register Status from FPGA */
    do {
        int_sts = ((get_platform_misc_sts() & INT_MASK));
        if (int_sts == 1) {
            break;
        }
        msleep(10);
    } while (intr_timeout--);

    if (intr_timeout == 0 && int_sts == 0) {
        cterr('f',0,"Interrupt has not been sent \n");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("int_sts = %d \n",int_sts);
    }

    /* Reset PHY Register for other tests */
    reset_quad_phy();

    if ((NVRAM)->diagflag & D_VERBOSE) {
        get_platform_misc_sts(); 
    }
    return (PASSED);
}


/*
$Log: platform_ext_lpbk.c,v $
Revision 1.31  2018/12/21 00:58:13  haohsu
CSCvn27142-Fixed 1548 PHY Interrupt test fail

Revision 1.30  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.29  2018/07/03 09:25:04  alpeng
revert to old set speed method which is able to pass ORT on MFG

Revision 1.28  2018/05/10 03:21:54  leschen
Enhance loopback test to help MFG to cover solder bridge high impedance short defection.

Revision 1.27  2018/03/26 09:21:27  iachang
Fixed Goldbeach PHY test mode port mapping.

Revision 1.26  2017/07/14 02:51:39  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.25  2016/10/16 12:28:22  iachang
Supported Goldbeach Platform.

Revision 1.24  2014/09/18 02:10:11  alpeng
test 1000 speed; if ext. lpbk failed, test int. lpbk

Revision 1.23  2014/06/20 08:12:58  alpeng
support GE testmode on dagger

Revision 1.22  2014/05/02 18:24:16  mcharon
replace strcpy with sprint when copying interface name to struct ifr

Revision 1.21  2014/04/25 06:45:04  danchung
CSCul88171-3: remove useless functions

Revision 1.20  2014/02/19 09:11:34  alpeng
suport enhanced error code on loobpack tests

Revision 1.19  2014/01/17 03:23:14  alpeng
support internal loopback test 10/100, and permutation test for int/ext/reset PHY passed

Revision 1.18  2014/01/14 08:54:59  alpeng
support SFP loopback test on dagger

Revision 1.17  2014/01/10 10:46:32  alpeng
working for ext lpbk spd 10,100,1000

Revision 1.16  2013/12/25 07:12:16  alpeng
support dagger eth external loopback test with speed 100Mbps

Revision 1.15  2013/12/21 01:38:05  ptong
Change typo in function name

Revision 1.14  2013/12/18 06:32:58  hroni
use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64

Revision 1.13  2013/12/17 08:11:03  alpeng
per HW request, support special setting on dagger

Revision 1.12  2013/12/06 11:58:58  danchung
Fix SGMII/SFP PHY loopback util menu for USD

Revision 1.11  2013/12/03 08:22:08  alpeng
support 1548 eye enlarge. power on ports before setting speed

Revision 1.10  2013/11/26 08:40:37  hroni
fix compiler warning

Revision 1.9  2013/11/22 00:14:46  alpeng
support eth ports testmode on utah/sword/dagger

Revision 1.8  2013/11/15 10:18:31  danchung
Correct the GE port number assignment of GE lpbk test for Sword and Dagger

Revision 1.7  2013/11/07 07:25:23  alpeng
support 1340 init, eye enlarge, and reset_quad_phy

Revision 1.6  2013/07/09 00:33:37  alpeng
fixed the compiling issue

Revision 1.5  2013/07/04 03:04:03  alpeng
clean up code, modify the menu structure and rearrange test ports.

Revision 1.4  2013/06/28 04:02:28  alpeng
for P1A check in, add media internal loopback test into menu

Revision 1.3  2013/06/14 10:22:23  alpeng
follow O2 menu structure

Revision 1.2  2013/06/13 08:34:56  hroni
add support for SFP mux

Revision 1.1  2013/05/31 11:03:41  alpeng
support front panel GE loopback test

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
