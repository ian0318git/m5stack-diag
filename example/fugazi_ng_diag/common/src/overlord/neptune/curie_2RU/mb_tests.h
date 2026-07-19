/* $Id: mb_tests.h,v 1.1 2020/01/09 01:02:01 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2008, Shih-Nan Huang adapted from Xformers
 *
 * Copyright (c) 2008-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MB_TESTS_H_
#define _MB_TESTS_H_

#define DELAY_USBCMD 5000   /* Wait for 5 sec until sys stable */
/*-----------------------------------------------------------------------
 *  Externs                                                             *
 *----------------------------------------------------------------------*/
extern menuinfo_t     mb_subtest_menu;
extern menuinfo_t    *mb_submenup;
extern title_buf_t    mb_subtest_header;
extern title_buf_t    mb_subtest_title[];

extern int map_mainmem_test(int);
extern int test_memory_ecc();
extern int mvl_ge_switch_main(int);
extern int ethernet_tests(int);
extern int build_ge_phy_menu(int);
extern int usb_test(void);

#define DISABLE_USB3_SS "setpci -s 00:14.0 d8.l=0x0"
#define ENABLE_USB3_SS "setpci -s 00:14.0 d8.l=0x3f"
#define ROUTE_USB2_TO_EHCI "setpci -s 00:14.0 d0.l=0xf0"
#define ROUTE_USB2_TO_XHCI "setpci -s 00:14.0 d0.l=0xf3"
#define UNBIND_XHCI_CONTROLLER "echo '0000:00:14.0' > /sys/bus/pci/drivers/xhci_hcd/unbind"
#define BIND_XHCI_CONTROLLER "echo '0000:00:14.0' > /sys/bus/pci/drivers/xhci_hcd/bind"
#define UNBIND_EHCI_CONTROLLER "echo '0000:00:1d.0' > /sys/bus/pci/drivers/ehci-pci/unbind"
#define BIND_EHCI_CONTROLLER "echo '0000:00:1d.0' > /sys/bus/pci/drivers/ehci-pci/bind"
#define SET_CPU_TO_AUX "pp 0x910 0"
#define SUPPRESS_MESG "echo 0 > /proc/sys/kernel/printk"
#define OPEN_MESG "echo 4 > /proc/sys/kernel/printk"
#define CREATE_USBDRV_FILE "ls /dev/usbdrv* > /curie-2RU-diag/usbdrv.txt"
#define REMOVE_USBSPD_FILE "rm -f /tmp/usb_speed.txt"
#define GET_USB0_SPEED "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv0) | grep speed | sort -u > /tmp/usb_speed.txt"
#define GET_USB1_SPEED "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv1) | grep speed | sort -u > /tmp/usb_speed.txt"
#define UDEVTRIGGER "udevtrigger"

#define PCIE_FPGA_VID 0x1137
#define PCIE_FPGA_DID 0x01D3
#define PCIE_I350_VID 0x8086
#define PCIE_I350_DID 0x1521
#define PCIE_BCM57412_VID 0x14E4
#define PCIE_BCM57412_DID 0x16D6
#define PCIE_NVME_VID 0x8086
#define PCIE_NVME_DID 0x0A54

typedef enum {
    I350,       
    BCM57412,
    FPGA,    
} PCIE_DEVICES;

/* Curie 1RU mother board plug-in items can be skipped by
 * skip_plugin.sh. 
 */
typedef enum {
    EUSB_SK = 0,
    M2_SK,
    USB0_SK,
    USB1_SK,
    MB_SKIP_END
} MB_SKIP_ITEMS;
extern char *mb_skip_item_name[];
extern int check_skip_test(char *); 

#endif /* MB_TESTS_H__ */

/*
 *-----------------------------------------------------------------------------
$Log: mb_tests.h,v $
Revision 1.1  2020/01/09 01:02:01  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
