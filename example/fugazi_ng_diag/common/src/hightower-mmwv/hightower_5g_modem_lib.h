/* $Id: hightower_5g_modem_lib.h,v 1.4 2021/06/30 20:04:56 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/hightower_5g_modem_lib.h,v $
 *********************************************************************
 *
 * hightower_5g_modem_lib.h - 5g SWI modem Library Functions
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include "dev_NR_5G_swi.h"
#include "dev_NR_5G_swi_at.h"

#ifndef HIGHTOWER_5G_MODEM_LIB_H
#define HIGHTOWER_5G_MODEM_LIB_H

#define TTY_PATH                                 "/dev/"
#define INSMOD_CMD                               "insmod"
#define RMMOD_CMD                                "rmmod"
#define USB_SYS_SPEED_FILE                       "speed"

#define GOBISERIAL_KO                            "GobiSerial.ko"
#define DEFAULT_LTE_USB_TTY_DEV                  "ttyUSB1"
#define USB_SYS_DRV_PATH                         "/sys/bus/usb/drivers/usb"
#define USB_AT_CMD_PORT                          "1.2"
#define USB2P0_SPEED                             (480)
#define USB_SYS_VID_FILE                         "idVendor"
#define USB_SYS_DID_FILE                         "idProduct"

#define PCI_SYS_DEV_PATH                         "/sys/bus/pci/devices"
#define PCI_SYS_DRV_PATH                         "/sys/bus/pci/drivers/mhictrl"
#define MODEM_SWI_USB_VID                        (0x5c6)
#define MODEM_SWI_PCI_VID                        (0x17cb) //(0x18d7)<--Version10
#define MODEM_SWI_PCI_DID                        (0X306) //(0x200)<--Version10
#define MODEM_SWI_PCI_BUS_NUM                    "0000:01:00.0"
#define PCI_GEN2_SPEED                           (8)
#define PCI_SYS_VID_FILE                         "vendor"
#define PCI_SYS_DID_FILE                         "device"
#define TTY_DEV_NAME                             "mhitty1"
#define MODEM_MODEL_9191                         "EM9191"
#define MODEM_MODEL_9190                         "EM9190"
#define MODEM_9190_MMWAVE_SUB6_SKU               "1104704"
#define DV3X                                     "AE"   //FSN ends with AE is DV3.X and should not used in mfg

#define DELAY_ONE_SEC                            (1)
#define DELAY_TEN_SEC                            (10)
#define MODEM_LM9X0_PWR_ON_DELAY                 (30)
#define SIM_MUX_SWITCH_DELAY                     (100)
#define DPR_WAIT_DELAY                           (100)
#define AT_COMMAND_UTIL_DELAY                    (1000)
#define MODEM_PWR_OFF_OR_ON_PLA_WAIT_TIME        (60)
#define TESTMSG_BUFSZ                            (128)

#define MODEM_CPLD_PWR_ON                        (0x20)     
#define MODEM_CPLD_PWR_OFF                       (0xD0)     
#define MODEM_RADIO_ON                           (1)
#define MODEM_GNSS_ON                            (2)
#define MODEM_USB_MODE                           (0x80)

#define DEFAUT_CUSTOM_CFG                        (0)
#define DIAG_CUSTOM_CFG                          (1)

#define MODEM_PWR_OFF_OR_ON_WAIT_TIME            (10)
#define MODEM_MODEL_NUM_LEN                      (32)
#define MODEM_SKU_NUM_LEN                        (32)

#define MODEM_OTA_TX_EXPECTED_POWER              (23)
#define MODEM_OTA_TX_PWR_HI_TOLERANCE            (5)
#define MODEM_OTA_TX_PWR_LO_TOLERANCE            (23)
#define MODEM_TX_POWER_LOW                       (MODEM_OTA_TX_EXPECTED_POWER-MODEM_OTA_TX_PWR_LO_TOLERANCE)
#define MODEM_TX_POWER_HIGH                      (MODEM_OTA_TX_EXPECTED_POWER+MODEM_OTA_TX_PWR_HI_TOLERANCE)

#define MMWAVE_RX_POWER_LEVEL                    (-30)

#define OTA_SUB6_N79_EXP_PWR                     (-60)
#define OTA_SUB6_MIN                             (-75)
#define OTA_SUB6_MAX                             (-15)

#define MMWAVE_RX_POWER_LEVEL                     (-30)

#define MMWAVE_TX_POWER_LEVEL                     (23)
#define MMWAVE_TX_POWER_TOLERANCE                 (5)
#define MMWAVE_TX_POWER_LOW                       (MMWAVE_TX_POWER_LEVEL - MMWAVE_TX_POWER_TOLERANCE)
#define MMWAVE_TX_POWER_HIGH                      (MMWAVE_TX_POWER_LEVEL + MMWAVE_TX_POWER_TOLERANCE)

#define SWI_SYS_SUPPRESS_PRINTK                  "dmesg -n 1"

#define MODEM_PWR_ON                             (1)
#define MODEM_PWR_OFF                            (0)
#define MODEM_PLA_HI                             (1)
#define MODEM_PLA_LO                             (0)

#define MODEM_RSSI_LEGACY_ATCMD                  (1)
#define MODEM_RSSI_LATEST_ATCMD                  (0)

#define UTIL_TEST_MAIN                           (0)
#define UTIL_TEST_AUX                            (1)
#define UTIL_TEST_M1                             (2)
#define UTIL_TEST_M2                             (3)

#ifdef ENABLE_USB_PORT
enum usb_port_t {
    DEBUG_USB,
    USB2P0,
    USB3P0,
    MAX_USB_PORT
} usb_mode;
#endif
enum sim_num {
    SIM0,
    SIM1
};

enum sim_stat {
  SIM_NOT_PRESENT = 0,
  SIM_PRESENT,
};

typedef enum {
    OPT_DISABLE,
    OPT_ENABLE
} debug_usb_util_opt_t;

typedef struct modem_usb_config_t {
    char usb_devinfo[128];
    char at_usb_devinfo[128];
} swi_5g_modem_usb_config_t;

typedef struct modem_vid_did_info_t{
    unsigned short int vid;
    unsigned short int did;
    unsigned short int warn_msg;
} swi_modem_vid_did_info_t;

extern int diag_modem_pwr_ctrl (int pwr_opt);
extern int diag_swi_5g_insmod (int input);
extern int diag_5g_swi_dev_create (dev_5g_swi_object_t *diag_5g_swi_obj);
extern int diag_modem_reset_pin_ctrl (uchar value);
extern int diag_swi_5g_modem_pci_detect (char *pcie_devinfo, int vid, int did);
extern int diag_swi_5g_usb_deb_enable (int input);
extern int diag_swi_5g_sim_selection (int input);

#ifdef ENABLE_USB_PORT
extern int diag_swi_5g_modem_usb_detect (char *usb_devinfo, int vid, int speed);
#endif


#endif

/*********************************************************************
 * $Log: hightower_5g_modem_lib.h,v $
 * Revision 1.4  2021/06/30 20:04:56  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.3  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.6  2021/05/12 17:54:23  tshanmug
 * Chrysler infra for SIM1 test
 *
 * Revision 1.1.4.5  2021/01/22 07:01:21  tshanmug
 * chrysler modem power OFF ON sequence and modem access through external usb access
 *
 * Revision 1.1.4.4  2020/10/12 15:48:35  tshanmug
 * Chrysler menu change, mmwave ant test added and Empire modem code cleanup
 *
 * Revision 1.1.4.3  2020/09/18 17:53:53  ksabzwar
 * Add Empire & Chrysler 5G modem SKU check
 *
 * Revision 1.1.4.2  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

