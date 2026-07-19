 /* $Id: diag_lte_test.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_lte_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_lte_test.h - Header File of LTE Test
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DIAG_LTE_TEST__
#define __DIAG_LTE_TEST__

#define LTE_USB_DETECT_TOUT                     (3000)  /* 30 secs */

#define LTE_MODEM_DISAPPEAR_TOUT                (4 * 6000)  /* 4 mins */
#define LTE_MODEM_BACK_ALIVE_TOUT               (4 * 6000)  /* 4 mins */
#define AT_COMMAND_UTIL_DELAY                   (1000)

#define LTE_USB_VENDOR_ID "Sierra Wireless, Incorporated"
extern int diag_lte_test(boolean);

#endif

/*-------------------------------------------------
 * $Log: diag_lte_test.h,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.2  2018/03/26 09:21:22  harrchan
 * Support usb debug port detection test
 *
 * Revision 1.1.2.1  2018/02/27 08:06:45  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
