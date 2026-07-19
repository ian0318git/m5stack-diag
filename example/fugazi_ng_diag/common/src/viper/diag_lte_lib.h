 /* $Id: diag_lte_lib.h,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_lte_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_lte_lib.h
 * Description: Header file of LTE Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_LTE_LIB_H__
#define __DIAG_LTE_LIB_H__

#include "dev_lte_swi.h"

#define LTE_USB_SYS_DEV_PATH                     "/sys/bus/usb/drivers/usb/1-2"
#define LTE_USB_AT_DEV_PATH                      "1-2:1.3"
#define LTE_USB_TTY_DEV                          "ttyUSB2"
#define LTE_MODEL_NAME_PATH                      "/sys/bus/usb/drivers/usb/usb1/1-2/product"
#define LTE_PWR_ON_TIME                          155*100

extern int diag_lte_swi_dev_create(dev_lte_swi_object_t *);
extern int diag_lte_pwr_on(int);
extern int diag_lte_is_usb_found(int, int);
extern int diag_lte_sim_detected_by_fpga(void);
extern void diag_lte_get_ttyusb_name(char *);
extern int diag_lte_dport_host_usb_detect(char *, int);
extern int diag_lte_mux_switch_util(boolean); 
extern int diag_lte_pwr_on(int);
int lte_get_model_name (char *, char *);

#endif

/*-------------------------------------------------
 * $Log: diag_lte_lib.h,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.6  2018/06/28 11:20:36  lucywang
 * Added power on/off of "enumerate internal USB port0 (LTE 0)"
 *
 * Revision 1.1.2.5  2018/05/29 03:04:20  harrchan
 * Add LTE power on/off sequence
 *
 * Revision 1.1.2.4  2018/05/10 09:35:32  lucywang
 * Added LTE Mux Switch Utility
 *
 * Revision 1.1.2.3  2018/04/20 02:10:50  lucywang
 * Added to support LTE WP7607/WP7608/WP7609
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
