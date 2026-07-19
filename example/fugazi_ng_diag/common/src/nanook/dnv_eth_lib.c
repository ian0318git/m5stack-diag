 /* $Id: dnv_eth_lib.c,v 1.3 2020/04/20 02:28:24 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/dnv_eth_lib.c,v $
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
#include "dash_fpga.h"
#include "nanook_comm.h"

void dnv_phy_reg_access(void);
int  dnv_get_correct_iface_name(int, char *);
int  dnv_eth_get_iface_name(int, char *);
int  dnv_eth_link_is_up(int);
int  dnv_eth_force_link_set(int, int);

extern int ExecuteCmdbyPopen(char *, char *, int);


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
    switch (portnum) {
        case DNV_LAN0_PORT0:
            sprintf(iface_name, inface_lan1p0);
            break;
        case DNV_LAN0_PORT1:
            sprintf(iface_name, inface_lan1p1);
            break;
        case DNV_LAN1_PORT0:
            sprintf(iface_name, inface_lan1p0);
            break;
        case DNV_LAN1_PORT1:
            sprintf(iface_name, inface_lan1p1);
            break;
        default:
            printf("Wrong Ethernet port\n");
            return (FAILED);
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
        printf("%s() Error do IOCTL, %s\n", __FUNCTION__, strerror(errno));
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
    switch (phy_no) {
        case NANOOK_88E1543_P0_QSGMII_PHY:
            sprintf(iface_name, inface_lan1p0);
            break;
        case NANOOK_88E1543_P0_AUTO_DETECT_PHY:
            sprintf(iface_name, inface_lan1p0);
            break;
        case NANOOK_88E1543_P1_QSGMII_PHY:
            sprintf(iface_name, inface_lan1p1);
            break;
        case NANOOK_88E1543_P1_AUTO_DETECT_PHY:
            sprintf(iface_name, inface_lan1p1);
            break;
        default:
            printf("Wrong Ethernet port (%d)\n", phy_no);
            return (FAILED);
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
 *
 * Description: Check eth port link up status from Linux information.
 *
 * Inputs      : phy_no - PHY Number
 *               eth_staus - up/down 
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
        cterr('f', 0," The file `/sys/class/net/%s/operstate' can't be opened.\n ", iface_name);
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

/***************************************************************************
 *
 * Function   : chk_linux_eth_linkup
 * Description: check Linux eth port link status 
 * Inputs     : eth_no - Ethernet port index
 *              eth_status - up/down
 * Outputs    : TRUE/FALSE
 *
 ***************************************************************************
 */
int chk_linux_eth_linkup (int eth_no, int eth_status)
{
    int cnt = LINK_RETRY_COUNTER;
    int delay = LINK_DELAY_TIME_500;

    while (cnt) {
       if (eth_is_linkup(eth_no, eth_status)) {
           msleep(delay);
           return (PASSED);
       } else {
           cnt--;
           msleep(delay);
       }
    }
    return (FAILED);
}

/***************************************************************************
 *
 * Function   : dynamic_get_inface
 * Description: get ethernet interface dynamically 
 * Inputs     : eth_no - Ethernet port index
 *              eth_status - up/down
 * Outputs    : TRUE/FALSE
 *
 ***************************************************************************
 */
int dynamic_get_inface (int eth_no, char *inface_name)
{
    char cmd[256], buf[64];
    
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, sizeof(buf));

    sprintf(cmd, "%s%d' | tr -d '\n'", DYNAMIC_ETH_PREFIX, eth_no);
    ExecuteCmdbyPopen(cmd, buf, sizeof(buf));
    strcpy(inface_name, buf);

    return 0;
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
Revision 1.3  2020/04/20 02:28:24  lucywang

1. Fixed unplug/plug NIM module dynamically issue and added NIM cookie
2. Added to support NIM Prince
3. (CSCvn43011) add retry workaround for Deverton issue
4. add debug message and set default value to seneors
5. Reverted Register value of temp/press snsr after test
6. Bumped up version to 1.0.2

Revision 1.2  2019/12/11 10:10:32  lucywang
Merged Nanook to main trunk


$Endlog$
*/
