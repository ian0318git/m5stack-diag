 /*------------------------------------------------------------------
 * 
 * diag_lte_host_impl.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "error.h"
#include "proto.h"
#include "diag_lte_host_impl.h"
//#include "plug_common_host_impl.h"

int diag_lte_host_get_usb_devinfo(char *, char *, char *);
int diag_lte_telit_host_get_usb_devinfo(char *, char *, char *);
int diag_lte_host_get_modem_drv_path(char *);
int diag_lte_telit_host_get_modem_drv_path(char *);


/* ----------------------------------------------------------------------------
 *  Function: diag_lte_host_get_usb_devinfo
 *  Description: Function to get the corresponding USB device info for 
 *               LTE module
 *  Inputs: usb2p0_ptr - pointer to the corresponding USB2.0 device info
 *          usb3p0_ptr - pointer to the corresponding USB3.0 device info
 *          dport_ptr  - pointer to the corresponding debug port device 
 *                            info
 *  Output: PASSED/FAILED
 * ----------------------------------------------------------------------------
 */
int diag_lte_host_get_usb_devinfo (char *usb2p0_ptr, char *usb3p0_ptr, 
                                   char *usb_dport_ptr)
{
    sprintf(usb_dport_ptr, "%s", HOST_USB0_DEVINFO);
    sprintf(usb3p0_ptr, "%s", HOST_USB1_3P0_DEVINFO);
    sprintf(usb2p0_ptr, "%s", HOST_USB1_2P0_DEVINFO);
    return (PASSED);
}    


/*******************************************************************************
 *  Function: diag_lte_telit_host_get_usb_devinfo
 *  Description: Function to get the corresponding USB device info for 
 *               Telit LTE  module
 *  Inputs: usb2p0_ptr - pointer to the corresponding USB2.0 device info
 *          usb3p0_ptr - pointer to the corresponding USB3.0 device info
 *          dport_ptr  - pointer to the corresponding debug port device 
 *                            info
 *  Output: PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_host_get_usb_devinfo (char *usb2p0_ptr, char *usb3p0_ptr,
                                         char *usb_dport_ptr)
{
    return (diag_lte_host_get_usb_devinfo(usb2p0_ptr, usb3p0_ptr,
                                          usb_dport_ptr));
}


/*******************************************************************************
 * Function   : diag_lte_host_get_modem_drv_path
 * Description: To get the path of LTE driver
 * Inputs     : drv_path - pointer to store the path of LTE modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_lte_host_get_modem_drv_path (char *drv_path)
{
    sprintf(drv_path, DIAG_FOLDER);

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_host_get_modem_drv_path
 * Description: To get the path of LTE telit modem driver
 * Inputs     : drv_path - pointer to store the path of LTE telit modem driver
 *              (e.g. /diag, /firmware)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_lte_telit_host_get_modem_drv_path (char *drv_path)
{
    diag_lte_host_get_modem_drv_path(drv_path);
    
    return (PASSED);
}
/*-------------------------------------------------
 * $Log: diag_lte_host_impl.c,v $
 * Revision 1.1  2020/08/19 09:49:34  markzha
 * *** empty log message ***
 *
 *
 *-------------------------------------------------
 */
