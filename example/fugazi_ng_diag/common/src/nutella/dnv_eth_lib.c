/* $Id: dnv_eth_lib.c,v 1.5 2019/08/28 01:22:04 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/dnv_eth_lib.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base ethernet port 
 * 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
#include "proto.h"
#include "common.h"
#include "common_utils.h"
#include "monitor.h"
#include "menu.h"
#include "nvmonvars.h"
#include "error.h"
#include "router_if.h"
#include "sgmii_defs.h"
#include "ethernet.h"
#include "linux_eth.h"
#include "queryflags.h" /* for query user functions */  
#include "dnv_eth_lib.h"
#include "diag_fpga.h"


void dnv_phy_reg_access(void);
int  dnv_get_correct_iface_name(int, char *);
int  dnv_eth_get_iface_name(int, char *);
int  dnv_eth_link_is_up(int);
int  dnv_eth_force_link_set(int, int);
void ifconfig_down_up_eth(char *);


/******************************************************************************
 *
 * Function: dnv_get_correct_iface_name
 *
 * Description: Get correct interface name from LAN and Port number
 *
 * Inputs      : portnum - port number
 *               iface_name - buffer to interface name
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int dnv_get_correct_iface_name (int portnum, char *iface_name)
{
    if (has_sfp_sku()) {
        switch (portnum) {
        case DNV_LAN0_PORT0:
            sprintf(iface_name, NUTELLA_88E1543_P0_PHY_IFACE_NAME);
            break;
        case DNV_LAN0_PORT1:
            sprintf(iface_name, NUTELLA_88E1543_P1_PHY_IFACE_NAME);
            break;
        case DNV_LAN1_PORT0:
            sprintf(iface_name, NUTELLA_88E1543_P2_PHY_IFACE_NAME);
            break;
        case DNV_LAN1_PORT1:
            sprintf(iface_name, NUTELLA_88E1543_P3_PHY_IFACE_NAME);
            break;
        case DNV_I350_PORT1:
            sprintf(iface_name, NUTELLA_I350_SFP_P1_IFACE_NAME);
            break;
        case DNV_I350_PORT2:
            sprintf(iface_name, NUTELLA_I350_SFP_P2_IFACE_NAME);
            break;
        default:
            printf("Wrong Ethernet port\n");
            return (FAILED);
        }
    } else {
        switch (portnum) {
        case DNV_LAN0_PORT0:
            sprintf(iface_name, NUTELLA_100M_88E1543_P0_PHY_IFACE_NAME);
            break;
        case DNV_LAN0_PORT1:
            sprintf(iface_name, NUTELLA_100M_88E1543_P1_PHY_IFACE_NAME);
            break;
        case DNV_LAN1_PORT0:
            sprintf(iface_name, NUTELLA_100M_88E1543_P2_PHY_IFACE_NAME);
            break;
        case DNV_LAN1_PORT1:
            sprintf(iface_name, NUTELLA_100M_88E1543_P3_PHY_IFACE_NAME);
            break;
        default:
            printf("Wrong Ethernet port\n");
            return (FAILED);
        }
    }
    return (PASSED);
}

/*
 * Function: intel_write_phy_reg
 *
 * Description:
 * Intel Write PHY Register
 *
 * Input:
 * portnum - which Intel port
 * regnum - register number
 * regval - val write to the reg
 *
 * Return: pass/fail
 */
int dnv_write_phy_reg (uint portnum, uint phyaddr, uint regnum, uint regval)
{
    struct mii_ioctl_data *miip;
    int sk, phy_addr = 0;
    struct ifreq ethreq;
    char interface_name[20];

    phy_addr = phyaddr;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    dnv_get_correct_iface_name(portnum, interface_name);

    sprintf(ethreq.ifr_name, interface_name);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->phy_id = phy_addr;
    miip->reg_num = regnum;
    miip->val_in = regval;

    /* Note: This ioctl call will get to dev_ioctl() in linux/core/dev.c.
     * The cmd SIOCSMIIREG will eventually get to calling
     * cvm_oct_ioctl in deriver/net/octeon/ethernet-mdio.c and
     * the finally phy_mii_ioctl() in phy.c
     */
    if (ioctl(sk, SIOCSMIIREG, &ethreq) == -1) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }

#ifdef ETH_DEBUG
    printf("Ethernet Interface: %s\n", ethreq.ifr_name);
    printf("\nwrote PHY reg %d = %#.4x\n", miip->reg_num, miip->val_in);
#endif
    close(sk);


    return (PASSED);
}

/*
 * Function: dnv_read_phy_reg
 *
 * Description:
 * Intel read PHY Register
 *
 * Input:
 * portnum - which Intel port
 * regnum - register number
 * buf - pointer to the data buffer to hold the return value
 *
 * Return: pass/fail
 */
int dnv_read_phy_reg (uint portnum, uint phyaddr, uint regnum, uint *buf)
{
    struct mii_ioctl_data *miip;
    int sk, phy_addr = 0;
    struct ifreq ethreq;
    char interface_name[20];

    phy_addr = phyaddr;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    dnv_get_correct_iface_name(portnum, interface_name);

    sprintf(ethreq.ifr_name, interface_name);


    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->phy_id = phy_addr;
    miip->reg_num = regnum;

    if (ioctl(sk, SIOCGMIIREG, &ethreq) == -1) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }
    *buf = miip->val_out;

#ifdef ETH_DEBUG
    printf("Ethernet Interface: %s\n", ethreq.ifr_name);
    printf("\nread PHY reg %d = %#.4x\n", miip->reg_num, miip->val_out);
#endif
    close(sk);


    return (PASSED);
}


/**********************************************************
 * Function: dnv_phy_reg_access
 *
 * Description: Utility to do peek and poke to PHY registers
 *
 * Input: none
 *
 * Return: none
 ***********************************************************/
void dnv_phy_reg_access (void)
{
    char c;
    uint rdval, wrval;
    int busnum, bus_max = 3;
    uint portnum, phy_addr = 0;
    uint regnum, regnum_max = 32;

    printf("LAN0-P0: 0, LAN0-P1:1, LAN1-P0: 2, LAN1-P1: 3 \n");
    busnum = getdec_answer("\nEnter PHY bus number", 0, 0, bus_max);

    printf("\n");
    phy_addr = getdec_answer("\nEnter PHY ADDR (0xFF: exit)", 0, 0, 0xFF);

    if (phy_addr == 0xFF) {
        return;
    }

    portnum = busnum;

    do {

        regnum = getdec_answer("\nEnter PHY reg number", 0, 0, regnum_max);
        dnv_read_phy_reg(portnum, phy_addr, regnum, &rdval);
        printf("Current value of reg %d = (%d)%#.4x\n", regnum, rdval ,rdval);

        c = getc_answer("Do you want to change value?", "yn",'n');

        if (c == 'y') {
            wrval = gethex_answer("Enter value:", 0, 0, 0xffff);
            dnv_write_phy_reg(portnum, phy_addr, regnum, wrval);
            dnv_read_phy_reg(portnum, phy_addr, regnum, &rdval);
            printf("Read back reg %d = %#.4x\n", regnum, rdval);
        }
    } while (getc_answer("Continue?", "yn", 'y') == 'y');

    return;
}


/******************************************************************************
 *
 * Function: dnv_eth_get_iface_name
 *
 * Description: Get interface name based on port number
 *
 * Inputs      : phy_no - PHY Number
 *               iface_name - buffer to interface name
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int dnv_eth_get_iface_name (int phy_no, char *iface_name)
{
    if (has_sfp_sku()) {
        switch (phy_no) {
        case NUTELLA_88E1543_P0_PHY:
            sprintf(iface_name, NUTELLA_88E1543_P0_PHY_IFACE_NAME);
            break;
        case NUTELLA_88E1543_P1_PHY:
            sprintf(iface_name, NUTELLA_88E1543_P1_PHY_IFACE_NAME);
            break;
        case NUTELLA_88E1543_P2_PHY:
            sprintf(iface_name, NUTELLA_88E1543_P2_PHY_IFACE_NAME);
            break;
        case NUTELLA_88E1543_P3_PHY:
            sprintf(iface_name, NUTELLA_88E1543_P3_PHY_IFACE_NAME);
            break;
        case NUTELLA_I350_SFP_PORT1:
            sprintf(iface_name, NUTELLA_I350_SFP_P1_IFACE_NAME);
            break;
        case NUTELLA_I350_SFP_PORT2:
            sprintf(iface_name, NUTELLA_I350_SFP_P2_IFACE_NAME);
            break;
        default:
            printf("Wrong Ethernet port (%d)\n", phy_no);
            return (FAILED);
        }
    } else {
        switch (phy_no) {
        case NUTELLA_88E1543_P0_PHY:
            sprintf(iface_name, NUTELLA_100M_88E1543_P0_PHY_IFACE_NAME);
            break;
        case NUTELLA_88E1543_P1_PHY:
            sprintf(iface_name, NUTELLA_100M_88E1543_P1_PHY_IFACE_NAME);
            break;
        case NUTELLA_88E1543_P2_PHY:
            sprintf(iface_name, NUTELLA_100M_88E1543_P2_PHY_IFACE_NAME);
            break;
        case NUTELLA_88E1543_P3_PHY:
            sprintf(iface_name, NUTELLA_100M_88E1543_P3_PHY_IFACE_NAME);
            break;
        case NUTELLA_I350_SFP_PORT1:
            sprintf(iface_name, NUTELLA_I350_SFP_P1_IFACE_NAME);
            break;
        case NUTELLA_I350_SFP_PORT2:
            sprintf(iface_name, NUTELLA_I350_SFP_P2_IFACE_NAME);
            break;
        default:
            printf("Wrong Ethernet port (%d)\n", phy_no);
            return (FAILED);
        }
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: dnv_eth_link_is_up
 *
 * Description: Return True if link is up, otherwise link is down
 *
 * Inputs      : phy_no - PHY Number
 * Outputs     : TRUE/FALSE
 *
 *****************************************************************************/
int dnv_eth_link_is_up (int phy_no)
{
    char iface_name[32];
    struct ifreq ifr;
    struct ethtool_value edata;
    int fd, ret;

    if (dnv_eth_get_iface_name(phy_no, iface_name) == FAILED) {
        return (FALSE);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (fd < 0) {
        printf("%s: Open Socket failed\n", __func__);
        return (FALSE);
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, iface_name);

    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;

    ret = ioctl(fd, SIOCETHTOOL, &ifr);

    if (ret != 0) {
        printf("%s: Can't get device settings\n", __func__);
        close(fd);
        return (FALSE);
    }

    close(fd);

    if (edata.data) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}


/******************************************************************************
 *
 * Function: dnv_eth_force_link_set
 *
 * Description: Make socket IOCTL to IXGBE and set/unset MAC force link up
 *
 * Inputs      : phy_no - PHY Number
 *               enable - TRUE or FALSE
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int dnv_eth_force_link_set (int phy_no, int enable)
{
    char iface_name[32];
    struct ifreq ifr;
    int fd, ret;
    int cmd;

    if (dnv_eth_get_iface_name(phy_no, iface_name) == FAILED) {
        return (FAILED);
    }

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (fd < 0) {
        printf("%s: Open Socket failed\n", __func__);
        return (FAILED);
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, iface_name);

    if (enable) {
        cmd = TRUE; 
    } else {
        cmd = FALSE;
    }

    ifr.ifr_data = (caddr_t)&cmd;

    ret = ioctl(fd, SIOCDEVPRIVATE, &ifr);

    if (ret != 0) {
        printf("%s: IOCTL Return Fails (%d)n", __func__, ret);
        fflush(stdout);
        perror("IOCTL");
        close(fd);
        return (FAILED);
    }

    close(fd);

    return (PASSED);
}

/******************************************************************************
 *
 * Function: eth_is_linkup
 * Description: Check eth port link up status from Linux information. 
 *
 * Inputs      : eth_number - Ethernet number
 *               eth_status - up/down
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static boolean eth_is_linkup (int eth_number, int eth_status)
{
    char file_name[64] = {0};
    FILE *stream_p;
    char linkstate[16] = {0};
    char iface_name[16] = {0};

    if (dnv_eth_get_iface_name(eth_number, iface_name) == FAILED) {
        return (FAILED);
    }

    sprintf(file_name, "/sys/class/net/%s/operstate", iface_name);

    stream_p = fopen(file_name, "r");
    if (stream_p == NULL) {
        cterr('f', '0', "The file '/sys/class/net/%s/operstate' can't be opened.\n", iface_name);
    } else {
        fscanf(stream_p, "%s", linkstate);
        fclose(stream_p);
    }

    if (eth_status == TRUE) {
        if (strcmp(linkstate, "up") == 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s link is %s\n", iface_name, linkstate);
            }
            return (TRUE);
        }
    } else {
        if (strcmp(linkstate, "down") == 0) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s link is %s\n", iface_name, linkstate);
            }
            return (TRUE);
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s link is %s\n", iface_name, linkstate);
    }
    return (FALSE);
}

/******************************************************************************
 *
 * Function: chk_linux_eth_linkup
 * Description: Check Linux eth port link status 
 *
 * Inputs      : eth_no - Ethernet port index
 *               eth_status - up/down
 * Outputs     : TRUE/FALSE
 *
 *****************************************************************************/
int chk_linux_eth_linkup (int eth_no, int eth_status)
{
    int cnt = LINK_RETRY_COUNTER;
    int delay = LINK_DELAY_TIME_500;

    while (cnt) {
        if (eth_is_linkup(eth_no, eth_status)){
            msleep(delay);
            return (PASSED);
        } else{
            cnt--;
            msleep(delay);
        }
    }
    return (FAILED);
}

/******************************************************************************
 *
 * Function: ifconfig_down_up_eth
 * Description: ifconfig down and up the interface along with the delay
 *              to avoid race condition.
 * Inputs      : iface_name - Interface name
 * Outputs     : NONE
 *
 *****************************************************************************/
void ifconfig_down_up_eth (char *iface_name)
{
    char cmd_up[50], cmd_down[50];
    
    sprintf(cmd_up, "ifconfig %s up > /dev/null", iface_name);
    sprintf(cmd_down, "ifconfig %s down > /dev/null", iface_name);
    
    system(cmd_down);
    system(cmd_up);
    
    /* Add delay to avoid recing condition on SMI interface
     * between driver and apps (CSCvq58855)*/
    msleep(AVOID_RACING_CONDITION_DELAY);

}

/*-------------------------------------------------
$Log: dnv_eth_lib.c,v $
Revision 1.5  2019/08/28 01:22:04  alicehua
1.CSCvr03904: Add retry to workaround Denverton loopback issue.
2.CSCvr03919: Fix console will hang when testing Marvell 88e1543 Interrupt test.

Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
