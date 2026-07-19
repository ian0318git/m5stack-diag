 /* $Id: diag_m2_test.h,v 1.2 2019/10/17 02:16:22 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_m2_test.h,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_m2_test.h
 * Description: Header file of M2 device
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef __DIAG_M2_TEST_H__
#define __DIAG_M2_TEST_H__


#define CHECK_M2_DEV_SCRIPT "/opt/script/check_m2_device.sh"
#define M2_DEVICE_PLUG "/tabei-diag/plug_m2_device.txt"
#define M2_SATA_DEV    "/dev/m2sata"
#define M2_USB_DEV     "/dev/m2usb"
#define M2_NVME_DEV    "/dev/m2nvme1"


enum {
    TABEI_M2_SATA,
    TABEI_M2_USB,
    TABEI_M2_PCIE,
};


extern int check_m2_device_utility(void);
extern int build_m2_test_menu(boolean);

#endif   /* __DIAG_M2_TEST_H__ */


/*-------------------------------------------------
 * $Log: diag_m2_test.h,v $
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.3  2019/09/27 01:36:25  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.2  2019/05/07 06:08:33  olin2
 * Check M.2 device present through FPGA
 *
 * Revision 1.1.2.1  2018/12/21 07:09:47  olin2
 * Update M.2 device menu
 *
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

