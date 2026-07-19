/* $Id: usb_dongle_common_lib.h,v 1.2 2019/06/14 09:59:33 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_common/usb_dongle_common_lib.h,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_lib.h - USB dongle common library functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __USB_DONGLE_COMMON_LIB_H__
#define __USB_DONGLE_COMMON_LIB_H__

#define MOUNT_DEBUGFS               "mount -t debugfs none /sys/kernel/debug"
#define USB_DEVICE_FILE             "/sys/kernel/debug/usb/devices"

#define USB_CLS_SIZE                (2)
#define USB_INTF_CLASS_STORAGE      (8)
#define HEX                         (16)
#define USB_DEVICE_MAX_NUM          (16)
#define USB_2P0_SPEED               (480)
#define USB_3P0_SPEED               (5000)

#define USB_CLASS_STR               "Cls="
#define USB_TYPE_STORAGE_STR        "usb-storage"

#define MID_INFO_CMD                "more /sys/bus/usb/devices"
#define MID_INFO_FILE               "/diag/usb_dongle_info.txt"


typedef struct udongle_intf_t {
    char   name[48];     /* module name */
    char   *port;        /* usb port number on platform */
    char   product[64];  /* usb dongle module product name */
    uint16_t vid;
    uint16_t pid;
    int    menu_display; /* flag set if submenu is invoked */
    int    test_type;    /* full test or HWIC interface test */
    PFT    diag;         /* main test of this module */
    PFT    intf_diag;    /* interface test of this module */
} udongle_if;

struct usb_dongle_info_t {
    unsigned char bus;
    unsigned char lev;
    unsigned char prnt;
    unsigned char port;  /* port=0 for usb0; port=1 for usb1, etc...*/
    unsigned char cnt;
    unsigned char dev;
    unsigned int spd;
    unsigned int mxch;
    unsigned int dcls;  /* device class */
    unsigned int icls;  /* interface class */
};

extern int usb_dongle_parse_info(void);
extern int usb_dongle_get_bus_speed(int);
extern int usb_dongle_dev_is_mass_storage(int);
extern int usb_dongle_dev_present_by_bus_lev(int, int, int, int *);
extern int usb_dongle_get_vid_pid_product(char *, uint16_t *, uint16_t *, char *);
extern int usb_dongle_get_module_entry_ptr(struct udongle_intf_t *);


#endif                  /* __USB_DONGLE_COMMON_LIB_H__ */


/*-------------------------------------------------
$Log: usb_dongle_common_lib.h,v $
Revision 1.2  2019/06/14 09:59:33  steja
Supported Cooper usb dongle LTE



*/
