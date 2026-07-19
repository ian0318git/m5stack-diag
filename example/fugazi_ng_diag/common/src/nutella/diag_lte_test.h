/* $Id: diag_lte_test.h,v 1.4 2019/07/11 12:31:29 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_lte_test.h,v $
 *------------------------------------------------------------------
 *
 * diag_lte_test.h - Header File of LTE Test
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
$Log: diag_lte_test.h,v $
Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
