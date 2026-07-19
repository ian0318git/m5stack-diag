/* $Id: mb_tests.h,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/mb_tests.h,v $
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
#define CREATE_USBDRV_FILE "ls /dev/usbdrv* > /curie-1RU-diag/usbdrv.txt"
#define REMOVE_USBSPD_FILE "rm -f /curie-1RU-diag/usb_speed.txt"
#define GET_USB0_SPEED "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv0) | grep speed | sort -u > /curie-1RU-diag/usb_speed.txt"
#define GET_USB1_SPEED "udevadm info -a -p $(udevadm info -q path -n /dev/usbdrv1) | grep speed | sort -u > /curie-1RU-diag/usb_speed.txt"
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
    M2_SK,
    USB0_SK,
    USB1_SK,
    MB_SKIP_END
} MB_SKIP_ITEMS;
extern char *mb_skip_item_name[];
extern int check_skip_test(char *); 

#endif /* MB_TESTS_H__ */

/******** History ******** 
$Log: mb_tests.h,v $
Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.15  2019/03/27 01:13:51  ptong
Release curie 1ru V1.3.1 : Separate motheboard PCIe and M.2 NVMe care PCIe scan tests. Use /dev/m2usb for M.2 USB module according to kernel change. Check FPGA M.2 module bits correctly to support M.2 slot test

Revision 1.1.2.14  2019/02/20 03:05:27  meho
Support NVMe in PCIe lane scan.

Revision 1.1.2.13  2018/11/01 23:23:05  ptong
Fix USB test. Use FPGA device reset register to reset USB stick to retry.

Revision 1.1.2.12  2018/10/18 01:41:40  meho
code clean up

Revision 1.1.2.11  2018/10/08 21:54:36  ptong
Combine NVME and M.2 SATA device test in one menu item

Revision 1.1.2.10  2018/09/07 00:09:36  ptong
Make skip plugin work on Cuire-1RU. Remove AUX port test

Revision 1.1.2.9  2018/09/06 01:30:39  ptong
Fixed hard coded /nep-diag to /curie-1RU-diag

Revision 1.1.2.8  2018/08/24 19:07:04  meho
Fixed plug i2c address

Revision 1.1.2.7  2018/08/20 18:26:39  alpeng
upgrade testcard plx pcie link up test for curie

Revision 1.1.2.6  2018/08/08 09:02:26  alpeng
fixed typo for scan FPGA, update list from NVME on header file

Revision 1.1.2.5  2018/08/08 07:46:25  alpeng
update pcie devices for pcie scan test on mb_test

Revision 1.1.2.4  2018/07/30 08:15:39  alpeng
remove nim3, sm2,3,4 entry; update pcie scan test, except nvme (vid/did/ need to verify with HW

Revision 1.1.2.3  2018/07/27 08:23:53  meho
Added pluggable LTE/Testcard test item.

Revision 1.1.2.2  2018/07/09 10:00:14  alpeng
remove pcie switch bus num init from linux_main.c and add nvme menu on mb_test.c

Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
porting neptune x86 to curie

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
