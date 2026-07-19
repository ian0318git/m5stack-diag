/* $Id: diag_lte_lib.h,v 1.6 2020/02/19 03:11:29 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_lte_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_lte_lib.h
 * Description: Header file of LTE Library
 * 
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
  
#ifndef __DIAG_LTE_LIB_H__
#define __DIAG_LTE_LIB_H__

#include "dev_lte_swi.h"

#define LTE_USB_SYS_DEV_PATH            "/sys/bus/usb/drivers/usb/1-2"
#define LTE_USB_AT_DEV_PATH             "1-2:1.3"
#define LTE_USB_UDEV_AT_TTY_DEV         "cwan_ttyUSB2"
#define LTE_MODEL_NAME_PATH             "/sys/bus/usb/drivers/usb/usb1/1-2/product"
#define LTE_PWR_ON_TIME                 155*100
#define LTE_USB_MUX_NOT_ON_DEBUG_PORT   (-2)
#define MAX_IMG_CARRIER_MATCH_TIME      (6)
#define CARRIER_MATCH_POLLING_DELAY     (10)

extern int diag_lte_swi_dev_create(dev_lte_swi_object_t *);
extern int diag_lte_pwr_on(int);
extern int diag_lte_is_usb_found(int, int);
extern int diag_lte_sim_detected_by_fpga(void);
extern void diag_lte_get_ttyusb_name(char *);
extern int diag_lte_dport_host_usb_detect(char *, int);
extern int diag_lte_mux_switch_util(boolean); 
extern int diag_lte_pwr_on(int);
int lte_get_model_name(char *, char *);
extern int diag_lte_dport_enable(int);
extern int diag_lte_chk_modem_carrier_is_match(void);

#endif

/*-------------------------------------------------
$Log: diag_lte_lib.h,v $
Revision 1.6  2020/02/19 03:11:29  harrchan
Add LTE patch for matching modem carrier (CSCvt07550)

Revision 1.5  2019/08/16 10:57:19  alicehua
If the LTE debug port test is failed,
it will keep printing error message,
so modify code to fix this problem.
(CDETS number:CSCvq77630)

Revision 1.4  2019/07/11 12:31:29  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
