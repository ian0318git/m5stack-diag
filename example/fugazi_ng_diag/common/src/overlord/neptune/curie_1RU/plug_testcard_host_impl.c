/* $Id: plug_testcard_host_impl.c,v 1.2 2019/08/06 06:56:15 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/plug_testcard_host_impl.c,v $
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
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
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
void plug_tc_host_get_pcie_dev_info(int, uint *, uint *, uint *, uint *);


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
    sprintf(dev_name, "%s", PIM_NVME_DEV);
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
    *dev_vid = PIM_PCIE_NVME_VID;
    *dev_did = PIM_PCIE_NVME_DID;
    *dev_speed = PCI_EXP_LINK_STA_SPD_5GT;
    *dev_width = PCI_EXP_LINK_STA_WID_1;
}

/*-------------------------------------------------
$Log: plug_testcard_host_impl.c,v $
Revision 1.2  2019/08/06 06:56:15  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.5  2019/06/11 02:30:46  meho
Skip access on Micron NVMe sample.

Revision 1.1.2.4  2019/03/11 02:43:13  meho
Fixed PIM NVMe device name

Revision 1.1.2.3  2019/02/11 07:33:37  meho
Support new PIM test-card (PCIe)

Revision 1.1.2.2  2018/11/06 07:16:20  meho
sync pluggable test-card common code

Revision 1.1.2.1  2018/10/16 09:05:40  meho
Pluggable re-structured



$Endlog$
*/
