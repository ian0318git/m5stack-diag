/*------------------------------------------------------------------
 * $Id: usb_dongle_lte_swi_lib.h,v 1.2 2019/06/14 09:59:36 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_lte_swi/usb_dongle_lte_swi_lib.h,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_lte_swi_lib.h - Header for USB dongle Common library
 *                            functions
 *
 *
 * Copyright (c) 2015-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __USB_DONGLE_LTE_SWI_LIB_H__
#define __USB_DONGLE_LTE_SWI_LIB_H__

#include "dev_lte_swi.h"

#define USB_DONGLE_LTE_POLLING_DELAY         (10)

#define MODEM_SWI_USB_VID                    (0X1199)
#define UDONGLE_USB2P0_SPEED                 (480)

#define WP_PWR_ON_DELAY                      (15500)
#define WP_MAX_PWR_OFF_DELAY                 (6000)
#define AT_COMMAND_UTIL_DELAY                (1000)

#define LTE_USB_AT_CMD_PORT                  "1.3"

#define LTE_USB_SYS_DRV_PATH                 "/sys/bus/usb/drivers/usb"
#define LTE_USB_SYS_DEV_PATH                 "/sys/bus/usb/devices"
#define USB_SYS_SPEED_FILE                   "speed"
#define USB_SYS_VID_FILE                     "idVendor"
#define USB_SYS_PID_FILE                     "idProduct"
#define DEV_PATH                             "dev"

#define LTE_USB_SWI_DRV_PATH                 "/sys/bus/usb/drivers/sierra"
/* Using MDEV config to assign this name "sierra_at_cmd" to symbolic link to "ttyUSB2" */
#define UDONGLE_LTE_SWI_TTY_DEV2             "sierra_at_cmd"
#define LTE_TESTMSG_BUFSZ                    (128)

#define AT_POLL_SEC                             (60) 
#define DELAY_1_SEC                             (1) 

#define AT_CMD_BUFFER_SIZE                      (1024)
#define MAX_SELFTEST_RETRY                      (1000)
#define AT_SELFTEST_TOUT_IN_SEC                 (1)
#define AT_SELFTEST_DELAY                       (500)

enum sim_num {
    SIM0,
    SIM1
};

enum sim_stat {
    SIM_PRESENT,
    SIM_NOT_PRESENT,
};


extern void usb_dongle_lte_swi_insmod(int);
extern int usb_dongle_lte_swi_modem_pwr_ctrl(int);
extern boolean usb_dongle_lte_swi_usb_detect(char *, int, int);
extern int usb_dongle_lte_swi_dev_create(dev_lte_swi_object_t *);
extern void usb_dongle_lte_swi_set_devname(char *);
extern void usb_dongle_lte_swi_get_devname(char *);
extern void usb_dongle_lte_swi_get_ttyusb_name(char *);
extern void usb_dongle_lte_swi_set_at_devinfo(char *);
extern boolean is_usb_dongle_lte_swi_wp(void);
extern int usb_dongle_lte_swi_run_at_cmd(int);
extern int usb_dongle_lte_swi_poll_tty_symlink(void);
#endif                  /* __USB_DONGLE_LTE_SWI_LIB_H__ */

/*-------------------------------------------------
 * $Log: usb_dongle_lte_swi_lib.h,v $
 * Revision 1.2  2019/06/14 09:59:36  steja
 * Supported Cooper usb dongle LTE
 *
 *
 * $Endlog$
 *--------------------------------------------------
 */
