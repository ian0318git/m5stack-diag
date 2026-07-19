/* $Id: diag_m2_test.h,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_m2_test.h,v $
 *------------------------------------------------------------------
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
#define M2_DEVICE_PLUG "/phoenix-diag/plug_m2_device.txt"
#define M2_USB_DEV     "/dev/m2usb"
#define M2_NVME_DEV    "/dev/m2nvme1"


extern int check_m2_device_utility(void);
extern int build_m2_test_menu(boolean);

#endif   /* __DIAG_M2_TEST_H__ */



