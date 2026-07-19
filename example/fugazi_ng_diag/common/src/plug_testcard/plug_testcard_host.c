/* $Id: plug_testcard_host.c,v 1.4 2019/11/25 08:55:51 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_host.c,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_host.c - PLUGGABLE Test card Host Function
 *                   (Needs to be implemented by host side)
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "plug_testcard_host.h"

#define PLUG_WARNING_MSG(func)         printf("'%s' is not implemented !!!\n", func);


/*******************************************************************************
 * Function   : plug_tc_host_usb_hub_menu_flag
 * Description: Plug testcard test with USB HUB menu flag
 * Inputs     : input - has USB HUB
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int plug_tc_host_usb_hub_menu_flag (int inv)
__attribute__((weak, alias("__plug_tc_host_usb_hub_menu_flag")));
int __plug_tc_host_usb_hub_menu_flag (int inv)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 * Function   : plug_tc_host_sgmii_present
 * Description: check pluggable test card insert slot has SGMII interface or not
 * Inputs     : input - Not used
 * Outputs    : ENABLE and DISABLEP
 *******************************************************************************
 */
int plug_tc_host_sgmii_present (int input)
__attribute__((weak, alias("__plug_tc_host_sgmii_present")));
int __plug_tc_host_sgmii_present (int input)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 * Function   : plug_tc_host_pcie_present
 * Description: check pluggable test card insert slot has pcie interface or not
 * Inputs     : input - Not used
 * Outputs    : ENABLE and DISABLEP
 *******************************************************************************
 */
int plug_tc_host_pcie_present (int input)
__attribute__((weak, alias("__plug_tc_host_pcie_present")));
int __plug_tc_host_pcie_present (int input)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
 *              prnt_no - parent device Number
 *              port_no - port Number
 *              hub - has USB hub or not
 * Outputs    : None
 *
 *******************************************************************************
 */
void plug_tc_host_reply_usb_bus_lev_port_info (int slot,int usb_speed, int *bus_no, 
                                               int *lev_no, int *prnt_no, int *port_no, int hub)
__attribute__((weak, alias("__plug_tc_host_reply_usb_bus_lev_port_info")));
void __plug_tc_host_reply_usb_bus_lev_port_info (int slot,int usb_speed, int *bus_no, 
                                                 int *lev_no, int *prnt_no, int *port_no, int hub) 
{
    PLUG_WARNING_MSG(__func__)
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
__attribute__((weak, alias("__plug_tc_host_get_eth_interface_info")));
void __plug_tc_host_get_eth_interface_info (int slot, char *eth_ifname) 
{
    PLUG_WARNING_MSG(__func__)
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
int plug_tc_host_reply_geport_ethnum (int slot, int *eth_num)
__attribute__((weak, alias("__plug_tc_host_reply_geport_ethnum")));
int __plug_tc_host_reply_geport_ethnum (int slot, int *eth_num)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_gephy_set_txtype_util
 * Description: Utility to set testcard GE WAN PHY Transmitter Type.
 * Inputs     : eth_num - ethernet number
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int plug_tc_host_gephy_set_txtype_util (int eth_num)
__attribute__((weak, alias("__plug_tc_host_gephy_set_txtype_util")));
int __plug_tc_host_gephy_set_txtype_util (int eth_num)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_tc_host_ge_send_packet_util")));
int __plug_tc_host_ge_send_packet_util (int eth_num)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_tc_host_gephy_set_1000_speed")));
int __plug_tc_host_gephy_set_1000_speed (void)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_tc_host_gephy_set_auto_neg")));
int __plug_tc_host_gephy_set_auto_neg (void)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_tc_host_gephy_set_test_speed")));
int __plug_tc_host_gephy_set_test_speed (int test_speed)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
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
__attribute__((weak, alias("__plug_tc_host_check_ext_lpbk_flag")));
int __plug_tc_host_check_ext_lpbk_flag (void)
{
    PLUG_WARNING_MSG(__func__)
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_get_nvme_info
 * Description: Function to nvme device node name from host.
 * Inputs     : slot
 *              nvme_name - nvme device name
 * Outputs    : None
 *
 *******************************************************************************
 */
void plug_tc_host_get_nvme_info (int slot, char *nvme_name)
__attribute__((weak, alias("__plug_tc_host_get_nvme_info")));
void __plug_tc_host_get_nvme_info(int slot, char *nvme_name)
{
    PLUG_WARNING_MSG(__func__)
}

/*******************************************************************************
 *
 * Function   : plug_tc_host_get_pcie_dev_info
 * Description: Get the PCIe device speed and width info from Host.
 * Inputs     : slot
 *              dev_vid - vendor id
 *              dev_did - device id
 *              dev_speed - PCI_EXP_LINK_STA_SPD_8GT/PCI_EXP_LINK_STA_SPD_5GT/
 *                          PCI_EXP_LINK_STA_SPD_2DOT5
 *              dev_width - PCI_EXP_LINK_STA_WID_8/PCI_EXP_LINK_STA_WID_4/
 *                          PCI_EXP_LINK_STA_WID_2/PCI_EXP_LINK_STA_WID_1
 * Outputs    : None
 *
 *******************************************************************************
 */
void plug_tc_host_get_pcie_dev_info (int slot, uint *dev_vid, uint *dev_did,
                                     uint *dev_speed, uint *dev_width)
__attribute__((weak, alias("__plug_tc_host_get_pcie_dev_info")));
void __plug_tc_host_get_pcie_dev_info(int slot, uint *dev_vid, uint *dev_did,
                                      uint *dev_speed, uint *dev_width)
{
    PLUG_WARNING_MSG(__func__)
}

/*******************************************************************************
 * Function : plug_tc_host_check_nvme_existence
 * Description: This function is to check the status of nvme.
 *              TRUE: Check is existing > PASSED
 *              FALSE: Check is not existing > PASSED
 * Inputs     : existence - TRUE/FALSE
 * OUTPUT: PASSED/FAILED
 *******************************************************************************
 */
int plug_tc_host_check_nvme_existence (int existence)
__attribute__((weak, alias("__plug_tc_host_check_nvme_existence")));
int __plug_tc_host_check_nvme_existence (int existence)
{
    return (PASSED);
}

/*-------------------------------------------------
$Log: plug_testcard_host.c,v $
Revision 1.4  2019/11/25 08:55:51  kehuang2
Collapse Tabei-L into main trunk

Revision 1.3  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.2  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.6  2018/11/13 07:10:28  hondwang
Base on PRRQ comment add gephy_set_test_mode back

Revision 1.1.2.5  2018/11/02 09:50:46  hondwang
Add USB prnt info for tabei-L

Revision 1.1.2.4  2018/11/01 08:17:45  hondwang
Add USB hub flag for USB menu test item

Revision 1.1.2.3  2018/11/01 06:24:33  hondwang
Add plug testcard USB HUB testing function

Revision 1.1.2.2  2018/10/16 07:08:45  hondwang
plug_tc_host_sgmii_present should be platform code, modified

Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
pluggable common code re-instruct add and remove files



*/
