/* $Id: diag_usb_lib.h,v 1.1 2019/10/16 02:27:15 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_usb_lib.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2009-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

extern int diag_usb_get_auto_suspend_val(int *);
extern void diag_usb_enable_auto_suspend_feature(int);
extern int diag_usb_get_hub_reset_status(int *);

#define USB_AUTO_SUSPEND_DIR   "/sys/module/usbcore/parameters/autosuspend"

#define EN_USB_AUTO_SUSPEND_FEATURE_VAL     2
#define DIS_USB_AUTO_SUSPEND_FEATURE_VAL   -1

#define USB_HUB_OUT_OF_RESET_STAT      0
#define USB_HUB_RESET_STAT             1

#define DIS_USB_AUTO_SUSPEND_FEATURE   0
#define EN_USB_AUTO_SUSPEND_FEATURE    1


/*-------------------------------------------------
$Log: diag_usb_lib.h,v $
Revision 1.1  2019/10/16 02:27:15  sherliu2
Fix CSCvq98193, disable auto-suspend feature for Star C1109-4P


*/
