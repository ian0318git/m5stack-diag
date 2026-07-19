 /* $Id: dnv_eth_lib.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/dnv_eth_lib.c,v $
 *------------------------------------------------------------------
 * Platform specific code for Linux base ethernet port 
 * 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
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

    if(this_is_viper_j()) {
        switch (portnum) {
            case DNV_LAN0_PORT0:
                sprintf(iface_name, "enp3s0f0");
                break;
            case DNV_LAN0_PORT1:
                sprintf(iface_name, "enp3s0f1");
                break;
            case DNV_LAN1_PORT0:
                sprintf(iface_name, "enp5s0f0");
                break;
            default:
                printf("Wrong Ethernet port\n");
                return (FAILED);
        }
    } else {
        switch (portnum) {
            case DNV_LAN0_PORT0:
                sprintf(iface_name, VIPER_88E1514_PHY_IFACE_NAME);
                break;
            case DNV_LAN0_PORT1:
                sprintf(iface_name, SKY_88E1514_PHY_IFACE_NAME);
                break;
            case DNV_LAN1_PORT0:
                sprintf(iface_name, VIPER_88E6176_IFACE_NAME);
                break;
            case DNV_LAN1_PORT1:
                sprintf(iface_name, VIPER_88E1512_PHY_IFACE_NAME);
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
int dnv_write_phy_reg (uint portnum, uchar phyaddr, ushort regnum, ushort regval)
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
int dnv_read_phy_reg (uint portnum, uchar phyaddr, ushort regnum, ushort *buf)
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
    ushort rdval, wrval;
    int busnum, bus_max = 3;
    int portnum, phy_addr = 0;
    int regnum, regnum_max = 32;

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
    if(this_is_viper_j()) {
        switch (phy_no) {
            case VIPER_88E1514_PHY:
                sprintf(iface_name, VIPERJ_88E1514_PHY0_IFACE_NAME);
                break;
            case VIPER_GE1_88E1514_PHY:
                sprintf(iface_name, VIPERJ_88E1514_PHY1_IFACE_NAME);
                break;
            case VIPER_88E6176:
                sprintf(iface_name, VIPERJ_88E6176_IFACE_NAME);
                break;
            default:
                printf("Wrong Ethernet port (%d)\n", phy_no);
                return (FAILED);
        }
    } else {
        switch (phy_no) {
            case VIPER_88E1514_PHY:
                sprintf(iface_name, VIPER_88E1514_PHY_IFACE_NAME);
                break;
            case VIPER_GE1_88E1514_PHY:
                sprintf(iface_name, SKY_88E1514_PHY_IFACE_NAME);
                break;
            case VIPER_88E6176:
                sprintf(iface_name, VIPER_88E6176_IFACE_NAME);
                break;
            case VIPER_88E1512_PHY:
                sprintf(iface_name, VIPER_88E1512_PHY_IFACE_NAME);
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

/*-------------------------------------------------
$Log: dnv_eth_lib.c,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.6  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.5  2018/05/11 02:22:11  harrchan
Changed interface name by using Cisco BIOS

Revision 1.1.2.4  2018/05/04 03:44:44  lucywang
Changed interface name by using Cisco BIOS c900-rommon.05022018.bin

Revision 1.1.2.3  2018/03/29 10:25:52  lucywang
Changed interface name by using Cisco BIOS

Revision 1.1.2.2  2018/03/28 07:03:51  lucywang
Added API to check SKU ViperJ and changed interface name for ViperJ

Revision 1.1.2.1  2018/02/27 08:06:49  harrchan
Initial viper application code base





$Endlog$
*/
