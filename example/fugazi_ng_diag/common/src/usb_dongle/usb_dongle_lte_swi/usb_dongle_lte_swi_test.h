/* $Id: usb_dongle_lte_swi_test.h,v 1.2 2019/06/14 09:59:36 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_lte_swi/usb_dongle_lte_swi_test.h,v $ 
 *------------------------------------------------------------------
 *
 * usb_dongle_lte_swi_test.h - Header file for USB Dongle LTE 
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __USB_DONGLE_LTE_SWI_TEST_H__
#define __USB_DONGLE_LTE_SWI_TEST_H__



#define SYS_SUPPRESS_PRINTK                         "dmesg -n 1"
#define SYS_RESTORE_PRINTK                          "dmesg -n 7"

#define PROBE_LTE_USB_TOUT                          (60000)


#define INSMOD_SWI_TEST_KO_1  "insmod /diag/sierra.ko"
#define INSMOD_SWI_TEST_KO_2  "insmod /diag/sierra_net.ko"

#define RMMOD_SWI_TEST_KO_1   "rmmod /diag/sierra.ko"
#define RMMOD_SWI_TEST_KO_2   "rmmod /diag/sierra_net.ko"


typedef enum {
    USB_20_MODE,
    USB_30_MODE
} usb_host_mode_t;


extern int usb_dongle_lte_swi_main(void *);


#endif                  /* __USB_DONGLE_LTE_SWI_TEST_H__ */

/*-------------------------------------------------
$Log: usb_dongle_lte_swi_test.h,v $
Revision 1.2  2019/06/14 09:59:36  steja
Supported Cooper usb dongle LTE

*
$Endlog$
*--------------------------------------------------
*/
