/* $Id: mb_tests.h,v 1.2 2018/05/18 09:24:59 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2008, Shih-Nan Huang adapted from Xformers
 *
 * Copyright (c) 2008-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MB_TESTS_H_
#define _MB_TESTS_H_

/*
 * Used in sub menu for mb tests.
 */


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
#define CREATE_USBDRV_FILE "ls /dev/usbdrv* > /nep-diag/usbdrv.txt"
#define REMOVE_USBSPD_FILE "rm /nep-diag/usb_speed.txt"
#define GET_USB0_SPEED "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv0) | grep speed | grep -o 480 > /nep-diag/usb_speed.txt"
#define GET_USB1_SPEED "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv1) | grep speed | grep -o 480 > /nep-diag/usb_speed.txt"
#define UDEVTRIGGER "udevtrigger"

#define NEP_FPGA_VID 0x1137
#define NEP_FPGA_DID 0x0240
#define NEP_CAV_VID  0x177d
#define NEP_CAV_DID  0x9700
#define NEP_PCIE_SW_VID 0x12d8
#define NEP_PCIE_SW_DID 0x8619
#define NEP_I211_VID 0x8086
#define NEP_I211_DID 0x1539
#define NEP_BROADCOM_SW_VID 0x14e4
#define NEP_BROADCOM_SW_DID 0x8403

/* parse PCI speed */
#define PCI_EXP_LINK_STA_SPD_MASK  0x0000000f
#define PCI_EXP_LINK_STA_SPD_2DOT5 0x00000001
#define PCI_EXP_LINK_STA_SPD_5GT   0x00000002
#define PCI_EXP_LINK_STA_SPD_8GT   0x00000003

/* parse PCI width */
#define PCI_EXP_LINK_STA_WID_MASK  0x000003f0
#define PCI_EXP_LINK_STA_WID_1     0x00000001
#define PCI_EXP_LINK_STA_WID_2     0x00000002
#define PCI_EXP_LINK_STA_WID_4     0x00000004
#define PCI_EXP_LINK_STA_WID_8     0x00000008
#define PCI_EXP_LINK_WID_SHIFT     0x00000004

#define PCI_DEV_0      0
#define PCI_FUN_0      0
#define PCI_CAP_PTR_OFFSET      0x34

typedef enum {
    CAVIUM = 0,      
    PERICOM,      
    I211,       
    FPGA,    
    BROADCOM_SW,
} PCIE_DEVICES;

/* Neptune mother board plug-in items can be skipped by
 * skip_plugin.sh. 
 */
typedef enum {
    EUSB_SK = 0,
    M2_SK,
    POECARD_SK,
    USB0_SK,
    USB1_SK,
    AUX_SK,
    MB_SKIP_END
} MB_SKIP_ITEMS;
extern char *mb_skip_item_name[];
extern int check_skip_test(char *); 

#endif /* MB_TESTS_H__ */

/******** History ******** 
$Log: mb_tests.h,v $
Revision 1.2  2018/05/18 09:24:59  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.9  2017/05/15 03:20:28  leschen
Adding counter to check device node is available for usb testing.

Revision 1.1.2.8  2017/03/15 08:18:41  leschen
Add speed key word in to grep to get USB stick speed.

Revision 1.1.2.7  2017/01/26 08:12:32  leschen
Support USB tests.

Revision 1.1.2.6  2017/01/10 23:42:34  ptong
Print item skipped msg in the mb submenu

Revision 1.1.2.5  2016/12/15 08:46:22  leschen
Modify PCIe lane scan test, provide vendor/device ids to get bus number automatically, get pcie cap struct to detect link speed/width.

Revision 1.1.2.4  2016/12/08 01:12:17  leschen
Set the CPU to AUX connection for AUX lpbk testing.

Revision 1.1.2.3  2016/11/04 06:04:57  leschen
Support USB3/USB2 testing.

Revision 1.1.2.2  2016/06/01 23:14:17  jskow
Update Makefile for Neptune, add mb_test structures for PCIe IF test and PCIe register check

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
