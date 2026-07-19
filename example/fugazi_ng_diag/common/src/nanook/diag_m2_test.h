 /* $Id: diag_m2_test.h,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_m2_test.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_m2_test.h
 * Description: Header file of M2 device
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_M2_TEST_H__
#define __DIAG_M2_TEST_H__


#define CHECK_M2_DEV_SCRIPT "/opt/script/check_m2_device.sh"
#define M2_DEVICE_PLUG "/nanook-diag/plug_m2_device.txt"
#define M2_SATA_DEV  "m2sata"
#define M2_USB_DEV   "m2usb"
#define M2_PCIE_DEV  "nvme0n1"


enum {
    NANOOK_M2_SATA,
    NANOOK_M2_USB,
    NANOOK_M2_PCIE,
};


extern int check_m2_device_utility (void);
extern int build_m2_test_menu(boolean);

#endif   /* __DIAG_M2_TEST_H__ */


/*-------------------------------------------------
 * $Log: diag_m2_test.h,v $
 * Revision 1.2  2019/12/11 10:10:30  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

