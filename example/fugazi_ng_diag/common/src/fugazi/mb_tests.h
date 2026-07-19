/* $Id: mb_tests.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/mb_tests.h,v $
 *------------------------------------------------------------------
 *
 * mb_tests.h
 *
 * May 2016, Sofian Teja adapted from Xformers
 *
 * Copyright (c) 2008-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _MB_TESTS_H_
#define _MB_TESTS_H_

#define DELAY_USBCMD 5000   /* Wait for 5 sec until sys stable */
/*
 * Global extern functions
 */
extern int do_all_menu_items(struct menuinfo *);
extern int build_boot_flash_menu(boolean);
extern int build_emmc_test_menu(boolean);
extern int build_snsr_menu(boolean);
extern int build_fpga_test_menu(boolean);
extern int build_esw_test_menu(boolean);
extern int linux_memory_tester_with_ecc_check(int);
extern int synce_pll_test_menu (int);

extern menuinfo_t mb_subtest_menu;
extern menuinfo_t *mb_submenup;
extern int mb_tests(int);
extern int do_all_menu_items(struct menuinfo *);
/*
 * Used in sub menu for mb tests.
 */
/* For USB test */
#define USB2                       2 
#define USB3                       3


#define PCIE_FPGA_VID 0x1137
#define PCIE_FPGA_DID 0x01D3
#define PCIE_BCM57412_VID 0x14E4
#define PCIE_BCM57412_DID 0x16D6
#define PCIE_I211_VID 0x8086
#define PCIE_I211_DID 0x1539

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

/* parse PCI bus */
#define PCIE_I211_BUS              0x03
#define PCIE_BCM57412_DEV1_BUS     0x17
#define PCIE_BCM57412_DEV2_BUS     0x18
#define PCIE_BCM57412_DEV3_BUS     0x65
#define PCIE_BCM57412_DEV4_BUS     0x66
#define PCIE_BCM57412_DEV5_BUS     0x67
#define PCIE_BCM57412_DEV6_BUS     0x68
#define PCIE_FPGA_BUS              0x02
#define PCIE_NVME_DID              0x0A54

#define PCI_DEV_0      0
#define PCI_FUN_0      0
#define PCI_CAP_PTR_OFFSET      0x34

typedef enum {
    I211,
    BCM57412,
    FPGA,
} PCIE_DEVICES;

/* Fugazi mother board plug-in items can be skipped by
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
extern ushort get_pci_vendor_id(ushort, uchar, uchar, uchar);

#endif                          /* MB_TESTS_H__ */

/*-------------------------------------------------
 * $Log: mb_tests.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:49  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.6  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.5  2019/03/28 19:00:34  letsai
 * 1. Modify FPGA interrupt test and utility.
 * 2. Modify I2C address of PSU2.
 * 3. Clean up code.
 * 4. Merge M.2 NVME and M.2 USB tests to combo test.
 *
 * Revision 1.1.6.4  2019/03/16 01:56:51  iachang
 * Bring up PCIe lane scan test.
 *
 * Revision 1.1.6.3  2019/03/14 23:00:00  iachang
 * Bring up PCIe lane scan
 *
 * Revision 1.1.6.2  2019/03/14 03:48:26  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
