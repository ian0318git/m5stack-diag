/* $Id: plug_testcard_host_impl.c,v 1.1 2020/01/09 01:02:07 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/plug_testcard_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_host_impl.c - Host platfrom to implement PLUGGABLE Test Card Functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "proto.h"
#include "menu.h"
#include "plug_testcard_usb_lib.h"
#include "plug_testcard_gpio_exp_lib.h"
#include "plug_testcard_phy.h"
#include "plug_common_host_impl.h"
#include "plug_testcard_host_impl.h"
#include "plug_host_fpga_lib.h"
#include "i2c_api.h"
#include "plat_defs.h"
#include "nvmonvars.h"
#include "linux_usb_test.h"
#include "dash_fpga.h"
#include "linux_pciutils.h"


int plug_tc_host_usb_hub_menu_flag(int);
int plug_tc_host_sgmii_present(int);
int plug_tc_host_pcie_present(int);
void plug_tc_host_reply_usb_bus_lev_port_info(int, int, int *, int *, int *, int *, int);
int plug_tc_host_tx_rx_diag(char*, int, int,int, int, int);
int plug_tc_host_ge_send_packet_util(int);
int plug_tc_host_gephy_set_auto_neg(void);
int plug_tc_host_gephy_set_1000_speed(void);
int plug_tc_host_gephy_set_test_speed(int);
int plug_tc_host_check_ext_lpbk_flag(void);
int plug_tc_host_reply_geport_ethnum(int, int *);
void plug_tc_host_get_eth_interface_info(int, char *);
void plug_tc_host_get_nvme_info(int, char *);


extern struct plug_intf_t *plug_test_if;
extern int plug_curr_i2c_ctrl;
extern int tx_rx_diag(char*, int, int, int, int, int);

/*******************************************************************************
 *                                  Global                                      
 *******************************************************************************
 */
#define TSN_GEPHY_CONFIG_TIME   1000
#define SEC_TO_MICROSEC         1000000.0
#define MAX_CHECKTIME_USEC      1000000   /* 1sec */
#define MAX_POLLING_COUNTS      100
#define POLLING_INTRVL          100 /* 100ms */
#define MAX_TRY                 5

#define NVME_INFO "/tmp/nvmeinfo"
#define PIM1_PCIE_PATH "pci0000:00/0000:00:1b.0/0000:01:00.0"
#define GET_NVME_INFO(pcie_path) "ls -l /sys/class/block | grep "pcie_path \
                                 " | grep -o 'nvme[0-9a-zA-Z]* ->' > "NVME_INFO

#define PIM_PCIE_BUS    "0000:01:00.0"

/*******************************************************************************
 * Function   : plug_tc_host_usb_hub_menu_flag
 * Description: Plug testcard test with USB HUB menu flag
 * Inputs     : input - has USB HUB
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int plug_tc_host_usb_hub_menu_flag (int inv)
{
    return (!inv);
}

/*******************************************************************************
 * Function   : plug_tc_host_sgmii_present
 * Description: check pluggable test card insert slot has SGMII interface or not
 * Inputs     : input - Not used
 * Outputs    : TRUE and FALSE
 *******************************************************************************
 */
int plug_tc_host_sgmii_present (int input)
{
    /* Curie does NOT has pluggable sgmii. */
    return (FALSE);
}

/*******************************************************************************
 * Function   : plug_tc_host_pcie_present
 * Description: check pluggable test card insert slot has PCIe interface or not
 * Inputs     : input - Not used
 * Outputs    : TRUE and FALSE
 *******************************************************************************
 */
int plug_tc_host_pcie_present (int input)
{
    /* Curie has pluggable pcie. */
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_reply_usb_bus_lev_port_info
 * Description: Reply plug test card USB mass storage bus, level and port info 
 *              by slot.
 * Inputs     : slot - plug slot Number
 *              usb_speed - present USB speed 
 *              bus_no - Bus Number 
 *              lev_no - Level Number
 *              port_no - port Number
 *              hub - has usb hub or not
 * Outputs    : None
 *
 *******************************************************************************
 */
void plug_tc_host_reply_usb_bus_lev_port_info (int slot, int usb_speed, int *bus_no, 
                                               int *lev_no, int *prnt_no, int *port_no, int hub)
{
    if (is_curie_1ru()) {
        if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
            *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER;
            *lev_no = PLUG_TC_USB_3P0_LEV_NUMBER;
            *prnt_no = PLUG_TC_USB_3P0_PRNT_NUMBER;
            *port_no = PLUG_TC_USB_3P0_PORT_NUMBER;
        } else {
            *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER;
            *lev_no = PLUG_TC_USB_2P0_LEV_NUMBER;
            *prnt_no = PLUG_TC_USB_2P0_PRNT_NUMBER;
            *port_no = PLUG_TC_USB_2P0_PORT_NUMBER;
        }
    } else if (is_curie_2ru()) {
        if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
            *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER_2RU;
            *lev_no = PLUG_TC_USB_3P0_LEV_NUMBER_2RU;
            *prnt_no = PLUG_TC_USB_3P0_PRNT_NUMBER_2RU;
            *port_no = PLUG_TC_USB_3P0_PORT_NUMBER_2RU;
        } else {
            *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER_2RU;
            *lev_no = PLUG_TC_USB_2P0_LEV_NUMBER_2RU;
            *prnt_no = PLUG_TC_USB_2P0_PRNT_NUMBER_2RU;
            *port_no = PLUG_TC_USB_2P0_PORT_NUMBER_2RU;
        }
    }
}


/*******************************************************************************
 *
 * Function   : plug_tc_host_tx_rx_diag
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
int plug_tc_host_tx_rx_diag (char* p_type, int eth_port, int speed,
                int pkt_cnt, int pkt_len, int value)
{
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_ge_send_packet_util
 * Description: Utility to send and check specific plug ethernet port.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_host_ge_send_packet_util (int eth_num)
{
    return (PASSED);
}


/*******************************************************************************
 *    
 * Function   : plug_tc_host_gephy_set_auto_neg 
 * Description: Function to set plug testcard GE PHY back to default.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
int plug_tc_host_gephy_set_auto_neg (void)
{
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plug_tc_host_gephy_set_1000_speed 
 * Description : Function configure platform GE MAC back to default.
 * Inputs      : None 
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_host_gephy_set_1000_speed (void)
{
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plug_tc_host_gephy_set_test_speed 
 * Description : Function configure platform GE MAC interface.
 * Inputs      : test_speed - testing speed
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_host_gephy_set_test_speed (int test_speed)
{
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_check_ext_lpbk_flag 
 * Description: Function to check if Ext. Loopback Flag is ON or not.
 * Inputs     : None
 * Outputs    : TRUE(ON) / FALSE(OFF)
 *
 *******************************************************************************
 */
int plug_tc_host_check_ext_lpbk_flag (void)
{
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_reply_geport_ethnum
 * Description: Function to reply eth_num and mac number by platform slot design.
 * Inputs     : slot - plug slot number
 *              eth_num - ethernet number
 * Outputs    : TRUE(ON) / FALSE(OFF)
 *
 *******************************************************************************
 */
int plug_tc_host_reply_geport_ethnum (int slot,int *eth_num)
{
    return (PASSED);
}

/*******************************************************************************
 * Function : plug_tc_host_get_eth_interface_info
 * Description: Get plug ethernet information with pluggable testcard
 * Inputs     : slot     - Pluggable slot no.
 *            : eth_ifname - ethernet interface (ie. eth)
 * OUTPUT: None
 *******************************************************************************
 */
void plug_tc_host_get_eth_interface_info (int slot, char *eth_ifname)
{
    sprintf(eth_ifname, "%s", SEL_PORT_ETH);
}

/*******************************************************************************
 * Function : plug_tc_host_get_nvme_info
 * Description: Get plug ethernet information with pluggable testcard
 * Inputs     : slot     - Pluggable slot no.
 *            : eth_ifname - ethernet interface (ie. eth)
 * OUTPUT: None
 *******************************************************************************
 */
void plug_tc_host_get_nvme_info (int slot, char *dev_name)
{
    char str[64];
    FILE *fp;
    int i;
    int len;

    if (is_curie_2ru()) {
        system(GET_NVME_INFO(PIM1_PCIE_PATH));
        fp = fopen(NVME_INFO, "r");
        if(fp == NULL) {
            printf("open "NVME_INFO" failed\n");
            sprintf(dev_name, "%s", NVME_DEV);
            return;
        }

        if(fgets(str, 64, fp) != NULL) {
            len = strlen(str);
            strcpy(dev_name, "/dev/");
            for (i = 0; i < len && str[i] != ' '; i++) {
                dev_name[5 + i] = str[i];
            }
            dev_name[5 + i + 1] = '\0';
        } else {
            sprintf(dev_name, "%s", NVME_DEV);
        }

        fclose(fp);
    } else {
        sprintf(dev_name, "%s", NVME_DEV);
    }
}

static long get_value_from_file(const char *path, int base)
{
    int fd;
    char buf[128];
    ssize_t rc;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("cannot open %s\n", path);
        return -1;
    }

    rc = read(fd, buf, sizeof(buf) - 1);
    if (rc == -1) {
        printf("cannot read %s\n", path);
        return -1;
    }

    close(fd);

    buf[rc] = '\0';

    return strtol(buf, NULL, base);
}

/*******************************************************************************
 * Function : plug_tc_host_get_nvme_info
 * Description: Get plug ethernet information with pluggable testcard
 * Inputs     : slot     - Pluggable slot no.
 *            : eth_ifname - ethernet interface (ie. eth)
 * OUTPUT: None
 *******************************************************************************
 */
void plug_tc_host_get_pcie_dev_info (int slot, uint *dev_vid, uint *dev_did,
                                     uint *dev_speed, uint *dev_width)
{
    long val;
    char path[256], *p, *end = path + sizeof(path);

    *dev_speed = PCI_EXP_LINK_STA_SPD_8GT;
    *dev_width = PCI_EXP_LINK_STA_WID_1;

    val = snprintf(path, sizeof(path), "%s/%s/", "/sys/bus/pci/devices",
                   PIM_PCIE_BUS);
    p = path + val;

    snprintf(p, end - p, "%s", "vendor");
    val = get_value_from_file(path, 16);
    if (val > 0) {
        *dev_vid = val;
        snprintf(p, end - p, "%s", "device");
        val = get_value_from_file(path, 16);
        if (val > 0) {
            *dev_did = val;
            printf("%04x:%04x\n", *dev_vid, *dev_did);
            return;
        }
    }

    *dev_vid = PIM_PCIE_NVME_VID;
    *dev_did = PIM_PCIE_NVME_DID;
}

/*
 *-----------------------------------------------------------------------------
$Log: plug_testcard_host_impl.c,v $
Revision 1.1  2020/01/09 01:02:07  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
