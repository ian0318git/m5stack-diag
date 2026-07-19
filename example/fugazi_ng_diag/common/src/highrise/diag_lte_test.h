/* $Id: diag_lte_test.h,v 1.1 2020/08/19 09:49:35 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/diag_lte_test.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_test.h - Header File for Pluggable LTE Telit
 *                         Main Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __LTE_TELIT_TEST__
#define __LTE_TELIT_TEST__


#define TELIT_SYS_SUPPRESS_PRINTK         "dmesg -n 1"
#define PROBE_LTE_TELIT_USB_TOUT          (500)
#define LTE_TELIT_USB_ENUM_TOUT           (10)
#define MODEM_SERDES_SWITCH_PROBE_TOUT    (500)
#define LTE_RESET_RETRY    3

extern int diag_lte_telit_main (boolean );

#endif

/*-----------------------------------------------
 *$Log:     
 */
