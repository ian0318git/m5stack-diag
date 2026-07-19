/* $Id: platform_eth.c,v 1.3 2014/02/19 03:23:13 leschen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/platform_eth.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base ethernet port loopback test
 * 
 * January 2012 Kody Ko
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "defs.h"
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "cross_platform.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "cvmx.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "platform_eth.h"
#include "cvmx-mdio.h"
#include "ethernet.h"  /* for SFPx port definition */
#include "queryflags.h" /* for query user functions */ 

#include "diag_fpga_lib.h"
#include "platform_ext_lpbk.h" // 12.30
#include "diag_ge_phy_88E1112C_lib.h"
#include "diag_common_drv.h"

#define DEBUG 0

#define F_GRP         (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL          (F_GRP | MF_DOALL)
#define F_ALL_E      (F_ALL | MF_SHOW_ERRCOUNT)

int reset_quad_phy(void);
int phy_soft_reset(char *, int);

extern void msleep(unsigned long);

/*****************************************************************************
 ***************************  Globals Variables   ****************************
 *****************************************************************************/

/* SKU of 6 ports 1G and 1 port 10G for X2222M and X3120 */
/* Port Mapping of GE0 ~ GE5*/
int old_eth_mapping_ge_num[] = {-1, -1, GE_PORT5, GE_PORT4, GE_PORT1, GE_PORT0,
                                GE_PORT3, GE_PORT2};
int old_eth_mapping_sfp_num[] = {-1, -1, SFP_PORT5, SFP_PORT4, SFP_PORT2,
                                 SFP_PORT3, SFP_PORT0, SFP_PORT1};
int old_ge_mapping_eth_num[] = {ETH5, ETH4, ETH7, ETH6, ETH3, ETH2};
int old_ge_mapping_phy_port[] = {PHY_PORT2, PHY_PORT3, PHY_PORT0, PHY_PORT1,
                                 PHY_PORT0, PHY_PORT1, PHY_PORT2, PHY_PORT3};

/* Port Mapping of SFP0 ~ SFP5*/
int old_sfp_mapping_eth_num[] = {ETH6, ETH7, ETH4, ETH5, ETH2, ETH3};
int old_sfp_mapping_phy_port[] = {PHY_PORT1, PHY_PORT0, PHY_PORT3, PHY_PORT2,
                                  PHY_PORT1, PHY_PORT0};

/* Marvel 1548 eth port and fiber phy address mapping */
int old_eth_fiber_mapping[] = {-1, -1, PHY_ADDR5, PHY_ADDR4, PHY_ADDR3, PHY_ADDR2,
                               PHY_ADDR1, PHY_ADDR0};

/* SKU of 6 ports 1G and 1 port 10G for X2222P
 * SKU of 4 ports 1G, 1 port for 88E1112C and 1 port 10G for X2222P
 */
/* Eth number of copper port mapping for GE0 ~ GE5*/
int two_phy_eth_mapping_ge_num[] = {GE_PORT1, GE_PORT0, -1, -1, GE_PORT5, GE_PORT4,
                                    GE_PORT3, GE_PORT2};
/* Eth number of copper port mapping for GE0 ~ GE3 */
int one_phy_eth_mapping_ge_num[] = {-1, -1, -1, -1, GE_PORT3, GE_PORT2, GE_PORT1, GE_PORT0};

/* Eth number of sfp port mapping for GE0 ~ GE5*/
int two_phy_eth_mapping_sfp_num[] = {SFP_PORT1, SFP_PORT0, -1, -1, SFP_PORT5, SFP_PORT4,
                                     SFP_PORT3, SFP_PORT2};
/* Eth number of sfp port mapping for GE0 ~ GE3*/
int one_phy_eth_mapping_sfp_num[] = {-1, -1, -1, -1, SFP_PORT3, SFP_PORT2, SFP_PORT1, SFP_PORT0};

/* GE port mapping of 6GE port SKU eth number */
int two_phy_ge_mapping_eth_num[] = {ETH1, ETH0, ETH7, ETH6, ETH5, ETH4};
/* GE port mapping of 4GE port SKU eth number */
int one_phy_ge_mapping_eth_num[] = {ETH3, ETH2, ETH1, ETH0};

/* 6 GE ports mapping of phy port number */
int two_phy_ge_mapping_phy_port[] = {PHY_PORT0, PHY_PORT1, PHY_PORT0, PHY_PORT1,
                                     PHY_PORT2, PHY_PORT3};
/* 4 GE port mapping of phy port number */
int one_phy_ge_mapping_phy_port[] = {PHY_PORT0, PHY_PORT1, PHY_PORT2, PHY_PORT3};

/* 6 SFP ports mapping of eth number */
int two_phy_sfp_mapping_eth_num[] = {ETH1, ETH0, ETH7, ETH6, ETH5, ETH4};

/* 4 SFP ports mapping of eth number */
int one_phy_sfp_mapping_eth_num[] = {ETH3, ETH2, ETH1, ETH0};

/* 6 SFP ports mapping of phy port number */
int two_phy_sfp_mapping_phy_port[] = {PHY_PORT0, PHY_PORT1, PHY_PORT0, PHY_PORT1,
                                      PHY_PORT2, PHY_PORT3};
/* 4 SFP ports mapping of phy port number */
int one_phy_sfp_mapping_phy_port[] = {PHY_PORT0, PHY_PORT1, PHY_PORT2, PHY_PORT3};

/* 6 ports Marvel 1548 eth port and fiber phy address mapping */
int two_phy_eth_fiber_mapping[] = {PHY_ADDR5, PHY_ADDR4, -1, -1, PHY_ADDR3,
                                   PHY_ADDR2, PHY_ADDR1, PHY_ADDR0};

/* 4 ports Marvel 1548 eth port and fiber phy address mapping */
int one_phy_eth_fiber_mapping[] = {PHY_ADDR3, PHY_ADDR2, PHY_ADDR1, PHY_ADDR0};

/* QLM 0 eth number from qLM port 0 ~ 3*/
int eth_qlm0_list[] = {ETH0, ETH1, ETH2 , ETH3};
/* QLM 4 eth number from qLM port 0 ~ 3*/
int eth_qlm4_list[] = {ETH4, ETH5, ETH6, ETH7};

/* 1340 Phy Address of QLM0 and QLM4 */
int qlm_0_4_1340_phy_addr[] = {0xE, 0xF, 0xC, 0xD, 0x8, 0x9, 0xA, 0xB};
/* 1548 Phy Address of QLM0 and QLM4 */
int qlm_0_4_1548_phy_addr[] = {0x5, 0x4, 0x6, 0x7, 0x3, 0x2, 0x1, 0x0};


/*------------------------------------------------------------------
 * Function: check_ext_lpbk_flag
 *        the external flag is put on file "diagflag.log", which is
 *        placed on path of root
 *
 * Input:  NONE
 *
 * Output: TRUE/FALSE
 *
 *------------------------------------------------------------------
 */
int check_ext_lpbk_flag(void){
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else { 
        return (TRUE);
    }
}


/* using cavium built-in function to config PHY */
void woodlawn_phy_reg_access (void)
{
    char c;
    int phy_max = 131;
    int phy_min = 4;
    int rdval, wrval;
    int bus_id, phy_id;
    int regnum, regnum_max = 32;

    phy_id = getdec_answer("\nEnter PHY ID ", phy_min, phy_min, phy_max);
    printf("\nPHY ID is : %d\n", phy_id);
    bus_id = getdec_answer("\nEnter Bus No ", 0, 0, 3);
    printf("\nSMI Bus: %d\n", bus_id);

    do {

        regnum = getdec_answer("\nEnter PHY reg number", 0, 0, regnum_max);
        woodlawn_phy_reg_rd(bus_id, phy_id, regnum, &rdval);
        printf("Current value of reg %d = (%d)%#.4x\n", regnum, rdval ,rdval);

        c = getc_answer("Do you want to change value?", "yn",'n');

        if (c == 'y') {
            wrval = gethex_answer("Enter value:", 0, 0, 0xffff);
            woodlawn_phy_reg_wr(bus_id, phy_id, regnum, wrval);
            woodlawn_phy_reg_rd(bus_id, phy_id, regnum, &rdval);
            printf("Read back reg %d = %#.4x\n", regnum, rdval);
        }
    } while(getc_answer("Continue?", "yn", 'y') == 'y');

}

/*
 * Function: phy_reg_wr
 *
 * Description:
 * Write marvell 88E1112C PHY register
 *
 * Input:
 * sk - socket id for ioctl()
 * ethreq_p - ptr to ifreq data structure for Linux MII reg access
 * regnum - register number
 * regval - val write to the reg
 *
 * Return: pass/fail
 */
int
phy_reg_wr(int sk, struct ifreq *ethreq_p, ushort regnum, ushort regval)
{
    struct mii_ioctl_data *miip;

    miip = (struct mii_ioctl_data *)&ethreq_p->ifr_ifru;

    miip->reg_num = regnum;
    miip->val_in = regval;

    /* 2013/02/23 - Because old sku eth7 port GE2 addr is 0x0000, when do 88E1548L 1G 
        external loopback will cause duplex mode to half duplex, speed 10M and loopback fail, 
        so add this phy id to 0xff to make the addr different with port GE2 */
    miip->phy_id = 0xff;
    
    /* Note: This ioctl call will get to dev_ioctl() in linux/core/dev.c.
     * The cmd SIOCSMIIREG will eventually get to calling
     * cvm_oct_ioctl in deriver/net/octeon/ethernet-mdio.c and
     * the finally phy_mii_ioctl() in phy.c
     */
    ioctl(sk, SIOCSMIIREG, ethreq_p);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nwrote PHY reg %d = %#.4x\n", miip->reg_num, miip->val_in);
    }
    return (PASSED);
}

/*
 * Function: phy_reg_rd
 *
 * Description:
 * Read marvell 88E1112C PHY register
 *
 * Input:
 * sk - socket id for ioctl()
 * ethreq_p - ptr to ifreq data structure for Linux MII reg access
 * regnum - register number
 * buf - pointer to the data buffer to hold the return value
 *
 * Return: pass/fail
 */
int
phy_reg_rd(int sk, struct ifreq *ethreq_p, ushort regnum, ushort *buf)
{
    struct mii_ioctl_data *miip;

    miip = (struct mii_ioctl_data *)&ethreq_p->ifr_ifru;

    miip->reg_num = regnum;
    ioctl(sk, SIOCGMIIREG, ethreq_p);
    *buf = miip->val_out;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nread PHY reg %d = %#.4x\n", miip->reg_num, miip->val_out);
    }
    return(PASSED);
}

boolean
is_eth_phy_linkup (char *ifname, int portnum)
{
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval;
    int repeat = 100;

    printf("--  pfix %s()\n",__FUNCTION__);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */
    /* Check copper infterface link up status */
    /* set page 0 in reg 22 */
    phy_reg_wr(sk, &ethreq, 22, 0);

    /* It requires 2 reads to get the real copper link status */
    phy_reg_rd(sk, &ethreq, 1, &rdval); //first read
    
    do {
        msleep(1);
	/* Read register 1 second time to get the real time value */
	phy_reg_rd(sk, &ethreq, 1, &rdval);
    } while((repeat-- > 0) && ((rdval &0x0004) == 0));

    close(sk);

    printf("%s() %s%d link is ", __FUNCTION__, ifname, portnum);
    if (repeat > 0) {
      printf("up\n");
      return(TRUE);
    }
    else {
      printf("down\n");
      return(FALSE);
    }
}

void
phy_reg_dump(char *ifname, int portnum)
{
    int sk, page;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, ii;
    uchar reglist[] = {0, 1, 2, 3, 5, 9, 10, 16, 17, 18, 19};

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);
    printf("\nEthernet Port: %s\n", pname);
    fflush(stdout);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return;
    }

    page = getdec_answer("\nEnter page number", 0, 0, 255);
    fflush(stdout);

    /* The following code is per the Marvell 88E1112c PHY */
    /* set page 0 */
    phy_reg_wr(sk, &ethreq, 22, page);

    for(ii=0; ii < sizeof(reglist); ii++) {
        phy_reg_rd(sk, &ethreq, reglist[ii], &rdval);
        printf("PHY reg %d = %#.4x\n", reglist[ii], rdval);
        fflush(stdout);
    }

    close(sk);
}




/*-------------------------------------------------*
 *  Below are unused functions for future reference*
 *-------------------------------------------------*/

int
set_phy_int_lpbk(char *ifname, int portnum)
{
#define ETH_DRIVER_DELAY    1 // Kernel need time to bring up link
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval, regnum;

    printf("--  pfix %s()\n",__FUNCTION__);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    /* Note: Using the ifreq data structure was observed how the
     * ethtool implementation was done in the cavium SDK. The ifr.name
     * field is used by the SDK to get the net_device by the interface
     * name (e.g. eth0). See file linux/net/core/dev.c and ethtool.c
     * for how it is being used.
     */
    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */

    /* Enable phy internal loopback. Reg 0_0.14=1 */
    regnum = 22; // select page 0
    phy_reg_wr(sk, &ethreq, regnum, 0);

    regnum = 0; // reg 0
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    wrval = rdval | 0x4000;
    phy_reg_wr(sk, &ethreq, regnum, wrval);

    close(sk);

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);
    
    return(PASS);
}

int
cfg_phy_spd(char *ifname, int portnum, int speed)
{
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval, regnum;
    int spdset = 0;;

    printf("--  pfix %s()\n",__FUNCTION__);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    switch(speed) {
    case ETH_10MBS:
	spdset = 0x0000;
        break;
    case ETH_100MBS:
	spdset = 0x2000;
        break;
    case ETH_1GBS:
	spdset = 0x0040;
        break;
    }
    
    /* The following code is per the Marvell 88E1112c PHY */

    /* set speed, Reg [0_2.6, 0_2.13] = value */
    regnum = 22; // select page 2
    phy_reg_wr(sk, &ethreq, regnum, 2); // select page 2

    regnum = 0; // reg 0
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    wrval = rdval & ~0x2040; // clear bit 6 and 13
    wrval |= spdset;

    phy_reg_wr(sk, &ethreq, regnum, wrval);

    close(sk);

    /* Reset to take effect. Reg 0_0.15=1 */
    return (phy_soft_reset(ifname, portnum));
}


int
cfg_phy_autoneg(char *ifname, int portnum, boolean onoff)
{
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval, regnum;

    printf("-- pfix %s()\n",__FUNCTION__);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */

    /* Disable SGMII auto-neg, Reg 0_2.12=0 */
    regnum = 22; // select page 2
    phy_reg_wr(sk, &ethreq, regnum, 2); // select page 2

    regnum = 0; // reg 0
    phy_reg_rd(sk, &ethreq, regnum, &rdval);

    if (onoff) {
        wrval = rdval | 0x1100; // enable autoneg and duplex
    }
    else {
        wrval = rdval & ~0x1000; // disable autoneg
    }
    phy_reg_wr(sk, &ethreq, regnum, wrval);

    close(sk);

    /* Reset to take effect. Reg 0_0.15=1 */
    return(phy_soft_reset(ifname, portnum));
}

int
force_phy_linkup(char *ifname, int portnum, boolean onoff)
{
#define ETH_DRIVER_DELAY    1 // Kernel need time to bring up link
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval, regnum;

    printf("--  pfix %s()\n",__FUNCTION__);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */

    /* Force copper link up. Reg 16_0.10=1 */
    regnum = 22; // select page 0
    phy_reg_wr(sk, &ethreq, regnum, 0);

    regnum = 16; // reg 16
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    if (onoff == TRUE) {
      wrval = rdval | 0x0400;
    }
    else {
      wrval = rdval & ~0x0400;
    }
    phy_reg_wr(sk, &ethreq, regnum, wrval);

    close(sk);

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);
    
    return(PASS);
}


int
cfg_phy(char *ifname, int portnum, int speed, int duplex, int autoneg)
{
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval, regnum;
    int spdset = 0;;

    printf("pfix %s()\n",__func__);
    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    switch(speed) {
    case ETH_10MBS:
	spdset = 0x0000;
        break;
    case ETH_100MBS:
	spdset = 0x2000;
        break;
    case ETH_1GBS:
	spdset = 0x0040;
        break;
    }
    
    /* The following code is per the Marvell 88E1112c PHY */

    /* set speed, Reg [0_2.6, 0_2.13] = value */
    regnum = 22; // select page 0
    phy_reg_wr(sk, &ethreq, regnum, 0); // select page 2

    /* Set speed bits
     */
    regnum = 0; // reg 0 is phy control reg
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    wrval = rdval & ~0x6040; // clear bit 6 and 13 (speed) and bit 14 (loopback)
    wrval |= spdset;

    /* Set duplex mode
     */
    if (duplex) {
        wrval |= 0x100; // full duplex
    }
    else {
        wrval &= ~0x100; // half duplex
    }

    /* Set autoneg on or off
     */
    if (autoneg) {
        wrval |= 0x1000; // enable autoneg
    }
    else {
        wrval &= ~0x1000; // disable autoneg
    }

    /* All these bits need a soft reset to take reset
     */
    wrval |= 0x8000;

    /* Write to the phy and read back immediate to make sure
     */
    phy_reg_wr(sk, &ethreq, regnum, wrval);
    phy_reg_rd(sk, &ethreq, regnum, &rdval);

    close(sk);

    return (PASSED);
}


/*
 * Function: set_phy_ext_lpbk
 *
 * Description:
 * Program the Ethernet port PHY device via socket and ioctl() to allow
 * Ethernet port external loopback test.
 *
 * Input: ifname - interface name according to Linux (eth, mgmt)
 *        portnum - port number (ETH0, RGMII0, etc)
 *
 * Return: pass/fail
 */
int
set_phy_ext_lpbk(char *ifname, int portnum)
{
#define ETH_DRIVER_DELAY    1 // Kernel need time to bring up link
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */
    /* set page 6 in reg 22 */
    phy_reg_wr(sk, &ethreq, 22, 6);

    /* enable external loopback stub test */
    phy_reg_rd(sk, &ethreq, 16, &rdval);
    wrval = rdval | 0x0020;
    phy_reg_wr(sk, &ethreq, 16, wrval);

    /* Read back to make sure the write is complete */
    phy_reg_rd(sk, &ethreq, 16, &rdval);
    close(sk);

    /* Give time for Linux driver and HW to settle when loopback is set */
    sleep(ETH_DRIVER_DELAY);
    
    return(PASS);
}

int phy_soft_reset(char *ifname, int portnum)
{
    int sk;
    struct ifreq ethreq;
    char pname[10];
    ushort rdval, wrval, regnum;
    int repeat = 100;

    printf("--  pfix %s()\n",__FUNCTION__);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return(FAIL);
    }

    sprintf(pname,"%s%d", ifname, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);

    /* The following code is per the Marvell 88E1112c PHY */

    regnum = 22; // select page 0
    phy_reg_wr(sk, &ethreq, regnum, 0);

    regnum = 0;
    phy_reg_rd(sk, &ethreq, regnum, &rdval);
    wrval = rdval | 0x8000;
    phy_reg_wr(sk, &ethreq, regnum, wrval);

    /* Read back to check for reset done */
    do {
      msleep(10);
      phy_reg_rd(sk, &ethreq, regnum, &rdval);
    } while((repeat-- > 0) && (rdval & 0x8000));

    close(sk);

    if ((repeat == 0) && (rdval & 0x8000)) {
      return(FAIL);
    }
    else {
      return(PASS);
    }
}


/*-------------------------------------------------*
 *  Below are unused functions for future reference*
 *-------------------------------------------------*/

/* using ioctl to config the PHY */

void
phy_reg_access(void)
{
    int sk;
    struct ifreq ethreq;
    char pname[10], c;
    ushort rdval, wrval;
    int portnum;
    int regnum, regnum_max = 32;

    portnum = getdec_answer("\nEnter SGMII port number", 0, 0, PLAT_SGMII_NUM_MAX);

    sprintf(pname,"%s%d", NAME_ETH, portnum);
    strncpy(ethreq.ifr_name, pname, IFNAMSIZ);
    printf("\nEthernet Port: %s\n", pname);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
	return;
    }
    do {

	regnum = getdec_answer("\nEnter PHY reg number", 0, 0, regnum_max);
	phy_reg_rd(sk, &ethreq, regnum, &rdval);
	printf("Current value of reg %d = %#.4x\n", regnum, rdval);

	c = getc_answer("Do you want to change value?", "yn",'n');

	if (c == 'y') {
	    wrval = gethex_answer("Enter value:", 0, 0, 0xffff);
	    phy_reg_wr(sk, &ethreq, regnum, wrval);
	    phy_reg_rd(sk, &ethreq, regnum, &rdval);
	    printf("Read back reg %d = %#.4x\n", regnum, rdval);
	}
    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    close(sk);
}

/*
 * Function: reset_quad_phy
 *
 * Description: Reset the 1548 and 1340 PHYs
 *
 * Input: void
 *
 * Return: PASSED/FAILED
 */
int reset_quad_phy (void)
{
    char reg_addr, reg_val;
    reg_addr = FPGA_RST_SIG_REG;

    /* Read reg 0x5(SM_RESET_L Reset Devices Enable Register) value */
    if (fpga_reg_read((int)reg_addr, &reg_val) == FAILED) {
        printf("Read FPGA register %#.8x failed\n", reg_addr);
        return (FAILED);
    }

    reg_val &= ~(FPGA_GE_PHY_RST_L);

    if (fpga_reg_write((int)reg_addr, reg_val) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", reg_val, reg_addr);
        return (FAILED);
    }

    msleep(100);

    reg_val |= (FPGA_GE_PHY_RST_L);

    if (fpga_reg_write((int)reg_addr, reg_val) == FAILED) {
        printf("Write data %#.8x to register %#.8x failed\n", reg_val, reg_addr);
        return (FAILED);
    }

    msleep(100);

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: platform_eth.c,v $
 * Revision 1.3  2014/02/19 03:23:13  leschen
 * Using the same nvram struct as menu use to fix lpbk check flag issue.
 *
 * Revision 1.2  2013/10/08 08:48:30  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:59:10  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.3  2013/06/18 00:33:10  leschen
 * Add fflush
 *
 * Revision 1.1.2.2  2013/06/17 11:11:46  leschen
 * Add fflush
 *
 * Revision 1.1.2.1  2013/04/24 10:37:23  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.7  2013/03/27 08:45:05  kuangik
 * Code cleanup
 *
 * Revision 1.5  2013/03/20 03:09:21  kuangik
 * Do not initialize ethernet interface as it will cause ping not working
 *
 * Revision 1.12  2013/02/26 01:30:30  leslie
 * Fix old eth mapping sfp num and 88E1548L external PHY addr 0x0 1G speed lpbk fail issue.
 *
 * Revision 1.11  2013/02/19 08:48:10  leslie
 * Fix PHY port mapping
 *
 * Revision 1.10  2013/02/18 06:47:11  kody
 * Modify for the port mapping changed according to the new SKUs.
 *
 * Revision 1.9  2013/01/18 06:37:43  leslie
 * Fix and clean up code.
 *
 * Revision 1.8  2012/12/11 01:02:44  leslie
 * Add config eth0 and eth1.
 *
 * Revision 1.7  2012/09/05 22:55:15  kody
 * Add enable eth2 ~ 7 network interfaces.
 *
 * Revision 1.6  2012/08/27 06:38:25  evanli
 * Let bus is selected correctly
 *
 * Revision 1.5  2012/08/03 10:16:56  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.3  2012/07/25 01:35:58  leslie
 * Recover to previous revision 1.1.1.1
 *
 * Revision 1.1.1.1  2012/02/10 05:59:50  kody
 * Initial imports Woodlawn project code base.
 *
 * $Endlog$
 *-------------------------------------------------
 */

