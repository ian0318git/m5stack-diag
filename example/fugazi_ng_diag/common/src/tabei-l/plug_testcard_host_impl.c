/* $Id: plug_testcard_host_impl.c,v 1.5 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/plug_testcard_host_impl.c,v $
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
#include <stdlib.h>
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
#include "diag_usb_lib.h"
#include "linux_pciutils.h"
#include "diag_fpga.h"

int plug_tc_host_usb_hub_menu_flag(int);
int plug_tc_host_sgmii_present(int);
int plug_tc_host_pcie_present(int);
void plug_tc_host_reply_usb_bus_lev_port_info(int, int, int *, int *, int *, 
                                              int *, int);
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
int check_pim_usb_enum_sts(int);
int plug_tc_host_check_nvme_existence(int);

extern struct plug_intf_t *plug_test_if;
extern int plug_curr_i2c_ctrl;
extern int tx_rx_diag(char*, int, int, int, int, int);
extern int usb_parse_info(void);

/*******************************************************************************
 *                                  Global                                      
 *******************************************************************************
 */

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
    /* Tabei does NOT has pluggable sgmii. */
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
    /* Tabei has pluggable pcie. */
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
 *              prnt_no - parent device Number
 *              port_no - port Number
 *              hub - has usb hub or not
 * Outputs    : None
 *
 *******************************************************************************
 */
void plug_tc_host_reply_usb_bus_lev_port_info (int slot, int usb_speed, 
                                               int *bus_no, int *lev_no, 
                                               int *prnt_no, int *port_no, 
                                               int hub)
{
    int ix;
    struct usb_info_t pim_testcard_usb2_storage, pim_testcard_usb3_storage; 

    memset(&pim_testcard_usb2_storage, 0, sizeof(&pim_testcard_usb2_storage));
    memset(&pim_testcard_usb3_storage, 0, sizeof(&pim_testcard_usb3_storage));

    /* Check if PIM USB enumeation is completed. */
    for (ix = 0; ix < MAX_USB_ENUMERATION_CHK_TIMES; ix++) {
        if (check_pim_usb_enum_sts(usb_speed) == PASSED) {
            break;
        }
        msleep(USB_ENUMERATION_TIME);
    }

    /* Save usb info into usb structure */
    if (usb_parse_info() == FAILED) {
        printf("usb_parse_info() failed\n");
    }

    /* Assgin bus, level, port number, parent number information. */
    if (usb_speed == PLUG_TESTCARD_USB3P0_SPEED) {
        /* Get USB3.0 storage information */
        if (get_usb_storage_info(PIM_USB, USB_HOST30_SPEED, 
                                 &pim_testcard_usb3_storage) == FAILED) {
            printf("%s() can not get USB 3.0 storage information\n", __FUNCTION__);
        }
        *bus_no = pim_testcard_usb3_storage.bus;
        *lev_no = pim_testcard_usb3_storage.lev;
        *port_no = pim_testcard_usb3_storage.port;
        *prnt_no = pim_testcard_usb3_storage.prnt;
    } else {
        /* Get USB2.0 storage information */
        if (get_usb_storage_info(PIM_USB, USB_HOST20_SPEED, 
                                 &pim_testcard_usb2_storage) == FAILED) {
            printf("%s() can not get USB 2.0 storage information\n", __FUNCTION__);
        }
        *bus_no = pim_testcard_usb2_storage.bus;
        *lev_no = pim_testcard_usb2_storage.lev;
        *port_no = pim_testcard_usb2_storage.port;
        *prnt_no = pim_testcard_usb2_storage.prnt;
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
    /* Tabei-L HW requests to test PCIe speed on 5G (CSCvs11330) */
    *dev_vid = PIM_PCIE_NVME_VID;
    *dev_did = PIM_PCIE_NVME_DID;
    *dev_width = PCI_EXP_LINK_STA_WID_1;
    *dev_speed = PCI_EXP_LINK_STA_SPD_5GT;
}

/*******************************************************************************
 * Function   : plug_tc_host_check_nvme_existence
 * Description: Check if NVMe is existing
 * Inputs     : existence - TRUE: Check is existing > PASSED
 *                          FALSE: Check is not existing > PASSED
 * Oupputs    : PASSED / FAILED
 *******************************************************************************
 */
int plug_tc_host_check_nvme_existence (int existence) 
{
    int counter = 0, check_value;
    char *plug_testcard_pci_device = PLUG_PCI_DEVICE; 

    if (existence == TRUE) {
        check_value = 0;
    } else {
        check_value = -1;
    }

    if (is_promethium()) {
        plug_testcard_pci_device = PLUG_PCI_DEVICE_PMTM;
    } else {
        plug_testcard_pci_device = PLUG_PCI_DEVICE;
    }

    while (access(plug_testcard_pci_device, F_OK) != check_value) {
        if (counter >= PLUG_TESTCARD_CHECK_NVME) {
            if (existence == TRUE) {
                cterr('f', 0, "NVMe is not existing");
            } else {
                cterr('f', 0, "NVMe is still existing");
            }
            return (FAILED);
        }
        msleep(PLUG_TESTCARD_WAIT_NVME);
        counter++;
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   :    check_pim_usb_enum_sts 
 * Description:    check if PIM usb enumeration status is completed 
 * Inputs     :    usb_spd - USB 2.0 or 3.0 speed 
 * Outputs    :    PASSED/FAILED
 *
 *******************************************************************************
 */
int check_pim_usb_enum_sts (int usb_spd)
{
    struct usb_info_t usb_tmp;
    FILE *fp;
    char line[82], tmp[32], *ptr, *ptr2;
    int str_idx = 0, ix = 0;
    boolean usb_hub_type = FALSE, usb_storage_type = FALSE;
    struct usb_info_t usb[USB_DEVICE_MAX_NUM];
    struct usb_info_t pim_usb3_storage;
    struct usb_info_t pim_usb2_storage;
    struct usb_info_t front_usb3_hub;
    struct usb_info_t front_usb2_hub;
    struct usb_info_t pim_usb3_hub;
    struct usb_info_t pim_usb2_hub;

    if (access(USB_DEVICE_FILE, F_OK) == -1) {
        /* if file doesn't exist, try to mount it */
        system(MOUNT_DEBUGFS);
    }

    /* Open sys/kernel/debug/usb/devies */
    fp = fopen(USB_DEVICE_FILE, "r");
    if (!fp) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s not found", USB_DEVICE_FILE);
        }
        printf("%s: '%s' can't be opened\n", __func__, USB_DEVICE_FILE);
        return (FAILED);
    }

    /* Init usb structure */
    memset(usb, 0, sizeof(usb));
    memset(&pim_usb3_storage, 0, sizeof(pim_usb3_storage));
    memset(&pim_usb2_storage, 0, sizeof(pim_usb2_storage));
    memset(&front_usb3_hub, 0, sizeof(front_usb3_hub));
    memset(&front_usb2_hub, 0, sizeof(front_usb2_hub));
    memset(&pim_usb3_hub, 0, sizeof(pim_usb3_hub));
    memset(&pim_usb2_hub, 0, sizeof(pim_usb2_hub));

    ptr = &line[1];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'T') {
            str_idx = 0;
            memset(&usb_tmp, 0, sizeof(usb_tmp));
            sscanf(ptr,
                   "%[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d"
                   " %[^0-9]%d %[^0-9]%d\n",
                   tmp, (int *) &usb_tmp.bus, tmp, (int *) &usb_tmp.lev,
                   tmp, (int *) &usb_tmp.prnt, tmp, (int *) &usb_tmp.port,
                   tmp, (int *) &usb_tmp.cnt, tmp, (int *) &usb_tmp.dev,
                   tmp, &usb_tmp.spd, tmp, &usb_tmp.mxch);
        }

        /* Check if the USB device is USB hub or USB storage  */
        if(strstr(line, USB_DEVICE_TYPE_HUB) != NULL) {
            usb_hub_type = TRUE;
            usb_storage_type = FALSE;
        } else if(strstr(line, USB_DEVICE_TYPE_STORAGE) != NULL) {
            usb_storage_type = TRUE;
            usb_hub_type = FALSE;
        } else {
            usb_storage_type = FALSE;
            usb_hub_type = FALSE;
        }

        /* Find the front USB hub or storage and pim usb hub or usb storage 
         * based on following example rule: 
         * 1. Device number is unique nubmer for each usb device. 
         * 2. Prnt number is parent device number for each usb device.
         * 3. Check the Prnt number and usb device type to check whether
         *    this usb device is hub or storage and record its information.
         * Example log: 
         *     CPU root 2.0 hub:
         *         Bus=01 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#=  1 Spd=480  MxCh= 4
         *         I:* If#= 0 Alt= 0 #EPs= 1 Cls=09(hub  ) Sub=00 Prot=00 Driver=hub
         *     PIM usb 2.0 hub:
         *         T:  Bus=01 Lev=01 Prnt=01 Port=01 Cnt=01 Dev#=  3 Spd=480  MxCh= 4     
         *         I:* If#= 0 Alt= 0 #EPs= 1 Cls=09(hub  ) Sub=00 Prot=00 Driver=hub
         *     PIM usb 2.0 usb storage:
         *         T:  Bus=01 Lev=02 Prnt=03 Port=02 Cnt=01 Dev#=  4 Spd=480  MxCh= 0
         *         I:* If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage
         */
        if ((usb_tmp.bus == USB_BUS_2) && (usb_tmp.spd == USB_HOST30_SPEED) && 
            (usb_tmp.prnt == USB_DEV_1) && (usb_tmp.port == USB_PORT_3) &&
            (usb_hub_type == TRUE)) {
            /* Tabei-L front USB 3.0 port number is fixed at 3, prnt number is 
             * inherited from CPU USB 3.0 controller device number 1 */
            memcpy(&front_usb3_hub, &usb_tmp, sizeof(usb_tmp));
        } else if ((usb_tmp.bus == USB_BUS_2) && (usb_tmp.spd == USB_HOST30_SPEED) 
                   && (usb_tmp.prnt == USB_DEV_1) && (usb_tmp.port == USB_PORT_1) 
                   && (usb_hub_type == TRUE)) {
            /* Tabei-L PIM USB 3.0 port number is fixed at 1, prnt number is 
             * inherited from CPU USB 3.0 controller device number 1 */
            memcpy(&pim_usb3_hub, &usb_tmp, sizeof(usb_tmp));
        } else if ((usb_tmp.bus == USB_BUS_1) && (usb_tmp.spd == USB_HOST20_SPEED) 
                   && (usb_tmp.prnt == USB_DEV_1) && (usb_tmp.port == USB_PORT_3)
                   && (usb_hub_type == TRUE)) {
            /* Tabei-L front USB 2.0 port number is fixed at 0, prnt number is 
             * inherited from CPU USB 2.0 controller device number 1 */
            memcpy(&front_usb2_hub, &usb_tmp, sizeof(usb_tmp));
        } else if ((usb_tmp.bus == USB_BUS_1) && (usb_tmp.spd == USB_HOST20_SPEED) 
                   && (usb_tmp.prnt == USB_DEV_1) && (usb_tmp.port == USB_PORT_1)
                   && (usb_hub_type == TRUE)) {
            /* Tabei-L pim USB 2.0 port number is fixed at 1, prnt number is 
             * inherited from CPU USB 2.0 controller device number 1 */
            memcpy(&pim_usb2_hub, &usb_tmp, sizeof(usb_tmp));
        }

        /*
         * end if 'T'
         */
        /*
         * save manufacturing info into temp usb structure
         * if device exists, there will be 3 lines starting with 'S'.
         */
        if (line[0] == 'S') {
            if ((ptr2 = strstr(ptr, "="))) {
                ptr2++;
                ptr2[strlen(ptr2) - 1] = '\0';
                sprintf(usb_tmp.mfg[str_idx++], "%s", ptr2);
            }
        }

        if (line[0] == 'I') {
            if ((ptr2 = strstr(ptr, "Driver=usb-storage"))) {
                usb_tmp.found = TRUE;
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp)); 
                
                if ((usb_tmp.prnt == pim_usb3_hub.dev) && 
                           (usb_storage_type == TRUE)) {
                    /* This USB flash is plugged at PIM USB 3.0 hub */
                    memcpy(&pim_usb3_storage, &usb_tmp, sizeof(usb_tmp));
                    pim_usb3_storage.found = TRUE;
                } else if ((usb_tmp.prnt == pim_usb2_hub.dev) && 
                           (usb_storage_type == TRUE)) {
                    /* This USB flash is plugged at PIM USB 2.0 hub */
                    memcpy(&pim_usb2_storage, &usb_tmp, sizeof(usb_tmp));
                    pim_usb2_storage.found = TRUE;
                }
                
                /* No USB hub was plugged on front USB or pim usb */
                if ((usb_tmp.bus == USB_BUS_2) && (usb_tmp.prnt == USB_DEV_1) 
                    && (usb_tmp.port == USB_PORT_1) 
                    && (usb_storage_type == TRUE)) {
                    /* This USB 3.0 flash is directly plugged at PIM USB port. */
                    memcpy(&pim_usb3_storage, &usb_tmp, sizeof(usb_tmp));
                    pim_usb3_storage.found = TRUE;
                } else if (((usb_tmp.bus == USB_BUS_1)) && (usb_tmp.prnt == USB_DEV_1) 
                           && (usb_tmp.port == USB_PORT_1) 
                           && (usb_storage_type == TRUE)) {
                    /* This USB 2.0 flash is directly plugged at PIM USB port. */
                    memcpy(&pim_usb2_storage, &usb_tmp, sizeof(usb_tmp));
                    pim_usb2_storage.found = TRUE;
                }                
            }
        }
    } /* while */

    fclose(fp);
    system(UMOUNT_DEBUGFS);

    /* Check if PIM USB storage is found. */
    if (usb_spd == PLUG_TESTCARD_USB3P0_SPEED) {
        if (pim_usb3_storage.found == TRUE) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s() line %d: PIM USB 3.0 flash is found\n", 
                       __FUNCTION__, __LINE__);
            }
            return (PASSED);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s() line %d: PIM USB 3.0 flash is not found\n", 
                        __FUNCTION__, __LINE__);
            }
            return (FAILED);
        }
    } else {
        if (pim_usb2_storage.found == TRUE) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s() line %d: PIM USB 2.0 flash is found\n", 
                        __FUNCTION__, __LINE__);
            }
            return (PASSED);
        } else {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s() line %d: PIM USB 2.0 flash is not found\n", 
                       __FUNCTION__, __LINE__);
            }
            return (FAILED);
        }
    }
}

/*-------------------------------------------------
$Log: plug_testcard_host_impl.c,v $
Revision 1.5  2020/08/06 07:54:55  kehuang2
Collapse Promethium into main trunk

Revision 1.4  2019/12/30 06:05:38  kehuang2
CSCvs55860: Support PIM testcard

Revision 1.3  2019/11/25 08:55:53  kehuang2
Collapse Tabei-L into main trunk

Revision 1.2  2019/10/17 02:16:27  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.12  2019/08/02 08:39:05  kodko
Replace hard code delay with polling way to check if PIM USB enumeration is done.

Revision 1.1.2.11  2019/07/29 06:13:52  kodko
Clean up code based on off-line code review

Revision 1.1.2.10  2019/06/06 08:30:09  kodko
Fix PIM USB 2.0 test will fail at first power on issue.

Revision 1.1.2.9  2019/06/05 07:44:14  meho
code clean up

Revision 1.1.2.8  2019/03/22 08:20:24  meho
Added pcie speed/width detection in NVMe test.

Revision 1.1.2.7  2019/03/13 06:00:14  meho
Skip PIM NVMe test from default.

Revision 1.1.2.6  2019/03/11 09:31:08  meho
Support PIM NVMe on reworked Tabei-L

Revision 1.1.2.5  2019/02/25 07:11:50  meho
Support new PIM test-card (PCIe).

Revision 1.1.2.4  2018/12/22 07:20:13  olin2
Clean up code

Revision 1.1.2.3  2018/12/20 08:09:21  kodko
Add extra delay after PIM power on for USB hub and storages enumeration.

Revision 1.1.2.2  2018/11/16 13:42:30  kodko
Support front USB hub and PIM USB hub connect with USB3.0 and USB2.0 storage read/write test.

Revision 1.1.2.1  2018/10/26 08:40:51  kodko
Add support for PIM LTE and test card modules.

$Endlog$
*/
