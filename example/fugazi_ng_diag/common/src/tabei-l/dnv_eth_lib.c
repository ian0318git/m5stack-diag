 /* $Id: dnv_eth_lib.c,v 1.3 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/dnv_eth_lib.c,v $
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
#include "diag_i350_test.h"


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
    if (is_fortnite() == TRUE){
        /* Fortnite */
        switch (portnum) {
            case DNV_LAN0_PORT0:
                sprintf(iface_name, TABEI_88E1543_P0_PHY_IFACE_NAME);
                break;
            case DNV_LAN0_PORT1:
                sprintf(iface_name, TABEI_88E1543_P1_PHY_IFACE_NAME);
                break;
            case DNV_LAN1_PORT0:
                sprintf(iface_name, CPU_LAN1_P0_TO_ESW_88E6390_P9_IF_NAME);
                break;
            case DNV_LAN1_PORT1:
                sprintf(iface_name, CPU_LAN1_P1_TO_ESW_88E6390_P10_IF_NAME);
                break;
            default:
                printf("Wrong Ethernet port\n");
                return (FAILED);
        }
    }else{
       /* Tabei-L */
       switch (portnum) {
            case DNV_LAN0_PORT0:
                sprintf(iface_name, TABEI_88E1514_PHY1_IFACE_NAME);
                break;
            case DNV_LAN0_PORT1:
                sprintf(iface_name, TABEI_88E1514_PHY2_IFACE_NAME);
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
    printf("\nEthernet Interface: %s\n", ethreq.ifr_name);
    printf("wrote PHY reg %d = %#.4x\n", miip->reg_num, miip->val_in);
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
    printf("\nEthernet Interface: %s\n", ethreq.ifr_name);
    printf("read PHY reg %d = %#.4x\n", miip->reg_num, miip->val_out);
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
    if (is_fortnite() == TRUE){
        /* Fortnite */
        switch (phy_no) {
            case TABEI_88E1543_P0_QSGMII_PHY:
                sprintf(iface_name, TABEI_88E1543_P0_PHY_IFACE_NAME);
                break;
            case TABEI_88E1543_P0_AUTO_DETECT_PHY:
                sprintf(iface_name, TABEI_88E1543_P0_PHY_IFACE_NAME);
                break;
            case TABEI_88E1543_P1_QSGMII_PHY:
                sprintf(iface_name, TABEI_88E1543_P1_PHY_IFACE_NAME);
                break;
            case TABEI_88E1543_P1_AUTO_DETECT_PHY:
                sprintf(iface_name, TABEI_88E1543_P1_PHY_IFACE_NAME);
                break;
            default:
                printf("Wrong Ethernet port (%d)\n", phy_no);
                return (FAILED);
        }
    }else{
        /* Tabei-L */
        switch (phy_no) {
            case TABEI_GE0_88E1514_PHY:
                sprintf(iface_name, TABEI_88E1514_PHY1_IFACE_NAME);
                break;
            case TABEI_GE1_88E1514_PHY:
                sprintf(iface_name, TABEI_88E1514_PHY2_IFACE_NAME);
                break;
            case TABEI_I350_SFP_PORT2:
                sprintf(iface_name, TABEI_I350_SFP_P2_IFACE_NAME);
                break;
            case TABEI_I350_SFP_PORT3:
                sprintf(iface_name, TABEI_I350_SFP_P3_IFACE_NAME);
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
 * Function   : igb_read_sfp_vendor_name
 * Description: Read SFP vendor name
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_read_sfp_vendor_name (int which_sfp, char *buf)
{
    struct mii_ioctl_data *miip;
    int sk, data_s = SFP_VENDOR_NAME_20;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P2_IFACE_NAME);
    } else if (which_sfp == I350_PORT3) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P3_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        close(sk);
        return (FAILED);
    }

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = 0;
    if (ioctl(sk, SIOCDEVPRIVATE, &ethreq) == -1) {
        printf("%s() %d Error do IOCTL", __FUNCTION__, __LINE__);
        close(sk);
        return (FAILED);
    }
    if ((miip->val_out != SFP_IDENTIFIER_SFP_3) && (miip->val_out != SFP_IDENTIFIER_SFP_DWDM_b)) {
        printf("I350 Port: %d, Cannot read correct SFP identifier: %#.4x\n", which_sfp, miip->val_out);
        close(sk);
        return (FAILED);
    }

    for (data_s = SFP_VENDOR_NAME_20; data_s <= SFP_VENDOR_NAME_35; data_s++) { 
        miip->reg_num = data_s;
        if (ioctl(sk, SIOCDEVPRIVATE, &ethreq) == -1) {
            printf("%s() %d Error do IOCTL", __FUNCTION__, __LINE__);
            close(sk);
            return (FAILED);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("read EEPROM reg %d = %#.4x\n", data_s, miip->val_out);
        }
        buf[data_s - SFP_VENDOR_NAME_20] = (char)miip->val_out;
    }

    close(sk);

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_read_sfp_eeprom util
 * Description: Read SFP EEPORM util
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_read_sfp_eeprom_util (void)
{
    struct mii_ioctl_data *miip;
    int sk, which_sfp;
    struct ifreq ethreq;
    ushort regnum;

    which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);

    regnum = getdec_answer("\nEnter reg: ", 0, 0, 100);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P2_IFACE_NAME);
    } else if (which_sfp == I350_PORT3) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P3_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        return (FAILED);
    }

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;

    
    if (ioctl(sk, SIOCDEVPRIVATE, &ethreq) == -1) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }

    printf("%s: read EEPROM reg %d = %#.4x\n", ethreq.ifr_name, miip->reg_num, miip->val_out);

    close(sk);


    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_write_sfp_phy
 * Description: Write SFP PHY 
 * Inputs     : which_sfp - which I350 SFP port
 *              regnum - PHY register
 *              write_data - write data
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_write_sfp_phy (int which_sfp, ushort regnum, ushort write_data)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P2_IFACE_NAME);
    } else if (which_sfp == I350_PORT3) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P3_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        return (FAILED);
    }

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;
    miip->val_in = write_data;

    if (ioctl(sk, SIOCDEVPRIVATE+2, &ethreq) != 0) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }
    close(sk);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: write PHY reg %d = %#.4x\n", ethreq.ifr_name, miip->reg_num, miip->val_in);
    }

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_write_sfp_phy_util
 * Description: Read SFP PHY util
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_write_sfp_phy_util (void)
{
    int which_sfp;
    ushort regnum, regval;

    which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);

    regnum = getdec_answer("\nEnter PHY reg: ", 0, 0, 100);

    regval = gethex_answer("\nEnter data: ", 0, 0, 0xffff);

    printf("I350 port %d: write PHY reg %d = %#.4x\n", which_sfp, regnum, regval);
    if (getdec_answer("\nWrite Data? Yes 1; No 0", 0, 0, 1)) {
        igb_write_sfp_phy (which_sfp, regnum, regval);
    }

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_read_sfp_phy
 * Description: Read SFP PHY 
 * Inputs     : which_sfp - which I350 SFP port
 *              regnum - PHY register
 *              read_data - read data
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_read_sfp_phy (int which_sfp, ushort regnum, ushort *read_data)
{
    struct mii_ioctl_data *miip;
    int sk;
    struct ifreq ethreq;

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P2_IFACE_NAME);
    } else if (which_sfp == I350_PORT3) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P3_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        return (FAILED);
    }

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    miip->reg_num = regnum;

    if (ioctl(sk, SIOCDEVPRIVATE+1, &ethreq) != 0) {
        printf("%s() Error do IOCTL", __FUNCTION__);
        close(sk);
        return (FAILED);
    }
    close(sk);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: read PHY reg %d = %#.4x\n", ethreq.ifr_name, miip->reg_num, miip->val_out);
    }

    *read_data = miip->val_out;

    return (PASSED);
}

/***************************************************************************
 *
 * Function   : igb_read_sfp_phy_util
 * Description: Read SFP PHY util
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_read_sfp_phy_util (void)
{
    int which_sfp;
    ushort regnum, read_data;

    which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);

    regnum = getdec_answer("\nEnter PHY reg: ", 0, 0, 100);

    igb_read_sfp_phy(which_sfp, regnum, &read_data);

    printf("I350 port %d: read PHY reg %d = %#.4x\n", which_sfp, regnum, read_data);


    return (PASSED);
}
/***************************************************************************
 *
 * Function   : igb_dump_sfp_eeprom_util
 * Description: dump SFP EEPORM util
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 ***************************************************************************
 */
int igb_dump_sfp_eeprom_util (void)
{
    struct mii_ioctl_data *miip;
    int sk, which_sfp;
    struct ifreq ethreq;
    ushort regnum;

    which_sfp = getdec_answer("\nEnter I350 Port 2/3 SFP ", 2, 2, 3);

    /* Create socket for ioctl calls
     */
    sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (-1 == sk) {
        printf("%s() Error Creating RX Socket", __FUNCTION__);
        return (FAILED);
    }

    if (which_sfp == I350_PORT2) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P2_IFACE_NAME);
    } else if (which_sfp == I350_PORT3) {
        sprintf(ethreq.ifr_name, TABEI_I350_SFP_P3_IFACE_NAME);
    } else {
        printf("Please select correct interface!!!!\n");
        return (FAILED);
    }

    printf("\nEthernet Interface: %s\n", ethreq.ifr_name);

    miip = (struct mii_ioctl_data *)&ethreq.ifr_ifru;

    for (regnum = EEPROM_DATA_ADDR_0; regnum < EEPROM_DATA_ADDR_64; regnum++) {

        miip->reg_num = regnum;

        if (ioctl(sk, SIOCDEVPRIVATE, &ethreq) == -1) {
            printf("%s() Error do IOCTL", __FUNCTION__);
            close(sk);
            return (FAILED);
        }
        
        if (regnum%8 == 0) {
            printf("\nread EEPROM reg %.2d: ", miip->reg_num);
        }
            printf(" 0x%02x", miip->val_out);
    }

    close(sk);

    return (PASSED);
}


/*-------------------------------------------------
$Log: dnv_eth_lib.c,v $
Revision 1.3  2020/08/06 07:54:55  kehuang2
Collapse Promethium into main trunk

Revision 1.2  2019/10/17 02:16:24  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.19  2019/09/10 06:10:33  olin2
Support read/write SFP PHY function

Revision 1.1.2.18  2019/08/29 07:29:37  olin2
Support read spcific SFP PHY util

Revision 1.1.2.17  2019/08/29 03:49:27  kehuang2
Clean up code by the comment of code review

Revision 1.1.2.16  2019/08/26 08:13:04  olin2
Support read SFP EEPROM

Revision 1.1.2.15  2019/05/29 03:16:17  kehuang2

1.Merge image according to official board type.
2.Reform the structure of diag menu

Revision 1.1.2.14  2019/03/19 09:26:26  kehuang2
Merge Sku1 and Sku2 into same image

Revision 1.1.2.13  2019/02/22 08:20:06  harrchan
Support 88E1543 utility

Revision 1.1.2.12  2019/02/21 03:14:16  harrchan
Support 88e1543 External Loopback Test

Revision 1.1.2.11  2019/02/01 03:46:41  wilbhuan
Defined new macro for shutting down LAN1 P0/P1 interface.

Revision 1.1.2.10  2019/01/31 01:44:43  harrchan
Support Register test and Interrupt test

Revision 1.1.2.9  2019/01/25 08:48:26  harrchan
Merge sku1 and sku2 function

Revision 1.1.2.8  2019/01/25 03:54:27  wilbhuan
1. Removed re-load ixgbe.ko procedure in below files:
   (1)diag_gephy_1543_lib.c
   (2)diag_gephy_lib.c
2. Updated "dnv_get_correct_iface_name" function to support ESW configuration.

Revision 1.1.2.7  2019/01/25 03:21:06  wilbhuan
1. Added ESW(Ethernet Switch) test with 88E6390 PHY device.
2. The scope of ESW test as following:
   (1) Register test
   (2) MAC loopback test
   (3) External loopback test
   (4) Interrupt test

Revision 1.1.2.6  2019/01/19 02:37:56  harrchan
Update Phy 1543 Structure

Revision 1.1.2.5  2019/01/18 02:31:46  harrchan
Update code after code review

Revision 1.1.2.4  2019/01/16 04:03:45  harrchan
Init phy1543 test

Revision 1.1.2.3  2018/12/04 08:12:45  olin2
Update check link

Revision 1.1.2.2  2018/10/24 02:47:27  harrchan
88E1514 GEPHY test

Revision 1.1.2.1  2018/10/02 01:50:02  harrchan
Initial commit for Tabei-L P1A bring up.

$Endlog$
*/
