/* $Id: diag_usb_test.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_usb_test.h,v $
 *------------------------------------------------------------------
 * Filename: diag_usb_test.h
 *
 * Description: Diag usb test header file.
 *
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_USB_TEST_H__
#define __DIAG_USB_TEST_H__

#define DISABLE_USB3_SS         "setpci -s 00:14.0 d8.l=0x0"
#define ENABLE_USB3_SS          "setpci -s 00:14.0 d8.l=0x3f"
#define ROUTE_USB2_TO_EHCI      "setpci -s 00:14.0 d0.l=0xf0"
#define ROUTE_USB2_TO_XHCI      "setpci -s 00:14.0 d0.l=0xf3"
#define UNBIND_XHCI_CONTROLLER  "echo '0000:00:14.0' > /sys/bus/pci/drivers/xhci_hcd/unbind"
#define BIND_XHCI_CONTROLLER    "echo '0000:00:14.0' > /sys/bus/pci/drivers/xhci_hcd/bind"
#define UNBIND_EHCI_CONTROLLER  "echo '0000:00:1d.0' > /sys/bus/pci/drivers/ehci-pci/unbind"
#define BIND_EHCI_CONTROLLER    "echo '0000:00:1d.0' > /sys/bus/pci/drivers/ehci-pci/bind"
#define SET_CPU_TO_AUX          "pp 0x910 0"
#define SUPPRESS_MESG           "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG               "echo 4 > /proc/sys/kernel/printk"
#define CREATE_USBDRV_FILE      "ls /dev/usbdrv* > /fugazi-diag/usbdrv.txt"
#define REMOVE_USBSPD_FILE      "rm -f /fugazi-diag/usb_speed.txt"
#define GET_USB0_SPEED          "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv0) | grep speed | sort -u > /fugazi-diag/usb_speed.txt"
#define GET_USB1_SPEED          "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv1) | grep speed | sort -u > /fugazi-diag/usb_speed.txt"
#define UDEVTRIGGER             "udevtrigger"
#define LSUSB_CMD               "echo; lsusb -t"

/* Super Speed Port Enable (SSPE) Offset 80B8h */
#define XHCI_SSPE               0x80B8
#define XHCI_SSPE_PORT_MASK     0x3ff

#define CHECK_USBDRV_FILE_TIME  10
#define WAIT_USBDRV_FILE_TIME   500
#define XHCI_CONFIG_WAITTIME    5000
#define SUPER_SPEED_WAITTIME    500
#define UDEVTRIGGER_WAITTIME    100
enum {
    FUGAZI_USB_PORT_MASK__FRONT_A    = 6,
    FUGAZI_USB_PORT_MASK__FRONT_C    = 7,
    FUGAZI_USB_PORT_MASK__PIM        = 9,
};

enum {
    FUGAZI_USB_PORT_MASK_FRONT_A  = (1 << 6),
    FUGAZI_USB_PORT_MASK_FRONT_C  = (1 << 7),
    FUGAZI_USB_PORT_MASK_PIM      = (1 << 9),
};

extern int usb_tests(int);
extern int usb_exist(int);
extern int fugazi_access_device_test(char *);
extern int check_block_size(char *);
extern int fugazi_usb_slot_tests(int);
extern int fugazi_emmc_slot_tests(int);
extern int fugazi_sata_tests(uchar *);
#endif   /* __DIAG_USB_TEST_H__ */


/*
 *------------------------------------------------------------------
 * $Log: diag_usb_test.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.5  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.4  2020/04/24 07:13:16  iachang
 * The block device access test, used fix size to replace malloc_usable_size()
 *
 * Revision 1.1.6.3  2019/05/13 01:53:41  iachang
 * CSCvp45238 : Fixed USB3.0 default speed issue
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
