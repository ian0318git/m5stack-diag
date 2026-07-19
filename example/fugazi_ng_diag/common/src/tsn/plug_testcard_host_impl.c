/* $Id: plug_testcard_host_impl.c,v 1.2 2018/11/23 08:49:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/plug_testcard_host_impl.c,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_host_impl.c - Host platfrom to implement PLUGGABLE Test Card Functions
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
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
#include "plug_slot.h"
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


int plug_tc_host_usb_hub_menu_flag(int);
int plug_tc_host_sgmii_present(int);
void plug_tc_host_reply_usb_bus_lev_port_info(int, int, int *, int *, int *, int *, int);
int plug_tc_host_tx_rx_diag(char*, int, int,int, int, int);
int plug_tc_host_ge_send_packet_util(int);
int plug_tc_host_gephy_set_auto_neg(void);
int plug_tc_host_gephy_set_1000_speed(void);
int plug_tc_host_gephy_set_test_speed(int);
int plug_tc_host_check_ext_lpbk_flag(void);
int plug_tc_host_reply_geport_ethnum(int, int *);
void plug_tc_host_get_eth_interface_info(int, char *);


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
 * Outputs    : ENABLE and DISABLEP
 *******************************************************************************
 */
int plug_tc_host_sgmii_present (int input)
{
    struct plug_intf_t *plug;
    int slot;

    plug = (struct plug_intf_t *)plug_test_if;
    slot = plug->slot;

    if (slot == PLUG_SLOT_1) {
        return (ENABLE);
    } else {
        return (DISABLE);
    }
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
 *              hub - has usb hub or not
 * Outputs    : None
 *
 *******************************************************************************
 */
void plug_tc_host_reply_usb_bus_lev_port_info (int slot, int usb_speed, int *bus_no, 
                                               int *lev_no, int *prnt_no, int *port_no, int hub)
{
    /* Star do not need prnt_no to separate USB interface */
    *prnt_no = PLUG_TC_USB_IGNORE;

    if (this_is_star_c1101p()) {
        if (hub == TRUE) {
            if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
                *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER;
                *lev_no = PLUG_TC_USB_HUB_C1101_LEV_NUMBER;
                *port_no = PLUG_TC_USB_IGNORE;
            } else {
                *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER;
                *lev_no = PLUG_TC_USB_HUB_C1101_LEV_NUMBER;
                *port_no = PLUG_TC_USB_IGNORE;
            }
        } else {
            if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
                *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER;
                *lev_no = PLUG_TC_USB_C1101_LEV_NUMBER;
                *port_no = PLUG_TC_USB_SLOT1_PORT_NUMBER;
            } else {
                *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER;
                *lev_no = PLUG_TC_USB_C1101_LEV_NUMBER;
                *port_no = PLUG_TC_USB_SLOT1_PORT_NUMBER;
            }
        }
    } else {
        if (hub == TRUE) {
            if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
                *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER;
                *lev_no = PLUG_TC_USB_HUB_C1109_LEV_NUMBER;
                *port_no = PLUG_TC_USB_IGNORE;
            } else {
                *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER;
                *lev_no = PLUG_TC_USB_HUB_C1109_LEV_NUMBER;
                *port_no = PLUG_TC_USB_IGNORE;
            }
        } else {
            if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
                if (slot == PLUG_SLOT_1){
                    *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER;
                    *lev_no = PLUG_TC_USB_C1109_LEV_NUMBER;
                    *port_no = PLUG_TC_USB_SLOT1_PORT_NUMBER;
                } else {
                    *bus_no = PLUG_TC_USB_3P0_BUS_NUMBER;
                    *lev_no = PLUG_TC_USB_C1109_LEV_NUMBER;
                    *port_no = PLUG_TC_USB_SLOT2_PORT_NUMBER;
                }
            } else {
                if (slot == PLUG_SLOT_1){
                    *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER;
                    *lev_no = PLUG_TC_USB_C1109_LEV_NUMBER;
                    *port_no = PLUG_TC_USB_SLOT1_PORT_NUMBER;
                } else {
                    *bus_no = PLUG_TC_USB_2P0_BUS_NUMBER;
                    *lev_no = PLUG_TC_USB_C1109_LEV_NUMBER;
                    *port_no = PLUG_TC_USB_SLOT2_PORT_NUMBER;
                }
            }
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
    return (tx_rx_diag(p_type, eth_port, speed, pkt_cnt, pkt_len, value));
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
    return (ge_send_packet_util(eth_num));
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
    uint     cpu_reg_addr = 0, cpu_reg_val = 0; 

    cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);
    cpu_reg_val = (uint)PANCR_FORCE_LINK_DOWN; 
    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s: Failed to config CPU GE reg. 0x%08X",
               __FUNCTION__, cpu_reg_addr);
        return (FAILED);
    }

    cpu_reg_val = (uint)(PANCR_RESERVED |
                         PANCR_AN_DUPLEX_EN |
                         PANCR_AN_FC_EN |
                         PANCR_AN_SPEED_EN |
                         PANCR_INBAND_BYPASS_EN |
                         PANCR_INBAND_AN_EN);
 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to CPU GE reg. 0x%08X: 0x%08X.\n",
               __FUNCTION__, cpu_reg_addr, cpu_reg_val);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s: Failed to set CPU GE registers back to default.\n",
               __FUNCTION__);
        return (FAILED);
    }
    
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
    uint cpu_reg_addr = 0, cpu_reg_val = 0;
    
    cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);
    cpu_reg_val = (uint)PANCR_FORCE_LINK_DOWN; 

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s: Failed to config CPU GE reg. 0x%08X",
               __FUNCTION__, cpu_reg_addr);
        return (FAILED);
    }

    cpu_reg_val = (uint)(PANCR_RESERVED |
                         PANCR_SET_FULL_DUPLEX |
                         PANCR_SET_SGMII_1000 |
                         PANCR_INBAND_BYPASS_EN |
                         PANCR_INBAND_AN_EN);
 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to CPU GE reg. 0x%08X: 0x%08X.\n",
               __FUNCTION__, cpu_reg_addr, cpu_reg_val);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        printf("%s: Failed to set CPU GE registers back to default.\n",
               __FUNCTION__);
        return (FAILED);
    }
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
    uint     cpu_reg_addr = 0, cpu_reg_val = 0;
    
    cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(3);  
    cpu_reg_val = (uint)PANCR_FORCE_LINK_DOWN; 
   
    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        cterr('f', 0, "Failed to config CPU GE reg. 0x%08X",
              cpu_reg_addr);
        return (FAILED);
    }

    cpu_reg_val = (uint)(PANCR_RESERVED |
                         PANCR_SET_FULL_DUPLEX |
                         PANCR_AN_FC_EN |
                         PANCR_INBAND_BYPASS_EN |
                         PANCR_INBAND_AN_EN |
                         PANCR_FORCE_LINK_UP);

    if (test_speed == SPD_100MBPS) {
        cpu_reg_val |= (uint)PANCR_SET_MII_100;
    } else if (test_speed == SPD_1000MBPS) {
        cpu_reg_val |= (uint)PANCR_SET_SGMII_1000;
    } else if (test_speed != SPD_10MBPS) {
        cterr('f', 0, "Failed at GE: Got unsupported Testspeed(%d) ",
              test_speed);
        return (FAILED);
    }
        
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: Write-in data to CPU GE reg. 0x%08X: 0x%08X.\n",
               __FUNCTION__, cpu_reg_addr, cpu_reg_val);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_val) != PASSED) {
        cterr('f', 0, "Failed to config CPU GE reg. 0x%08X",
              cpu_reg_addr);
        return (FAILED);
    }
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
    return (check_ext_lpbk_flag());
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
    *eth_num = TSN_GE1_ETHNUM;
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


/*-------------------------------------------------
$Log: plug_testcard_host_impl.c,v $
Revision 1.2  2018/11/23 08:49:53  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.6  2018/11/02 09:50:46  hondwang
Add USB prnt info for tabei-L

Revision 1.1.2.5  2018/11/01 12:59:34  hondwang
Modify pluggable testcard USB Hub testing with random port

Revision 1.1.2.4  2018/11/01 08:17:45  hondwang
Add USB hub flag for USB menu test item

Revision 1.1.2.3  2018/11/01 06:24:33  hondwang
Add plug testcard USB HUB testing function

Revision 1.1.2.2  2018/10/16 07:08:45  hondwang
plug_tc_host_sgmii_present should be platform code, modified

Revision 1.1.2.1  2018/10/15 06:44:32  hondwang
pluggable common code re-instruct add and remove files



$Endlog$
*/
