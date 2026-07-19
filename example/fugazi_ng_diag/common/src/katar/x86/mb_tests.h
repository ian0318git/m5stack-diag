/* $Id: mb_tests.h,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/mb_tests.h,v $
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
extern int build_boot_flash_menu(boolean);

#define DISABLE_USB3_SS "setpci -s 00:14.0 d8.l=0x0"
#define ENABLE_USB3_SS "setpci -s 00:14.0 d8.l=0x3f"
#define ROUTE_USB2_TO_EHCI "setpci -s 00:14.0 d0.l=0xf0"
#define ROUTE_USB2_TO_XHCI "setpci -s 00:14.0 d0.l=0xf3"
#define UNBIND_XHCI_CONTROLLER "echo '0000:00:14.0' > /sys/bus/pci/drivers/xhci_hcd/unbind"
#define BIND_XHCI_CONTROLLER "echo '0000:00:14.0' > /sys/bus/pci/drivers/xhci_hcd/bind"
#define UNBIND_EHCI_CONTROLLER "echo '0000:00:1d.0' > /sys/bus/pci/drivers/ehci-pci/unbind"
#define BIND_EHCI_CONTROLLER "echo '0000:00:1d.0' > /sys/bus/pci/drivers/ehci-pci/bind"
#define SET_CPU_TO_AUX "pp 0x910 0"
#define SUPPRESS_MESG "echo '0 0 0 0'> /proc/sys/kernel/printk"
#define OPEN_MESG "echo '7 4 1 7'> /proc/sys/kernel/printk"
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
#define PCI_EXP_CAP_ID_MASK		0x000000ff
#define PCI_EXP_CAP_ID			0x10
#define PCI_EXP_CAP_NEXTPTR_OFFSET	0x01

#define KATAR_RJ45_SKU	0x01
#define KATAR_SFP_SKU	0x02
#define KATAR_SFP1_SKU  0x03
#define UNKNOWN_SKU		0xFF

int katar_get_plat_sku(void);

typedef enum {
    CAVIUM = 0,      
    PERICOM,      
    I211,       
    FPGA,    
    BROADCOM_SW,
} PCIE_DEVICES;


#endif /* MB_TESTS_H__ */

/*
 *------------------------------------------------------------------
 * $Log: mb_tests.h,v $
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.3  2019/06/10 02:26:44  mikech2
 * Remove skip test function base on PRRQ#4685780 Comment#4
 *
 * Revision 1.1.2.2  2018/11/08 02:21:52  peteteng
 * Remove names in comment
 *
 * Revision 1.1.2.1  2018/10/22 08:02:33  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.5  2018/10/08 03:36:17  mikech2
 * Modify pcie scan for different AQC100 FW
 *
 * Revision 1.1.2.4  2018/09/21 08:52:12  mikech2
 * Add cross-port & internal lpbk test util
 *
 * Revision 1.1.2.3  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.2  2018/06/11 07:05:53  peteteng
 * add bootflash test from viper
 *
 * Revision 1.1.2.1  2018/06/07 01:19:22  peteteng
 * add project katar - based on neptune
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

