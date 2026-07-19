/* $Id: diag_usb_lib.c,v 1.1 2019/10/16 02:27:15 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_usb_lib.c,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2009-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "nvmonvars.h"
#include "plug_host_fpga_lib.h"
#include "diag_usb_lib.h"

int diag_usb_get_auto_suspend_val(int *);
void diag_usb_enable_auto_suspend_feature(int);
int diag_usb_get_hub_reset_status(int *);


/*******************************************************************************
  * Function   : diag_usb_get_auto_suspend_val
  * Description: Function to get current usb auto-suspend value
  * Inputs     : val-current auto-suspend value
  * Outputs    : PASSED/FAILED
  ******************************************************************************/
int diag_usb_get_auto_suspend_val (int *val)
{
   FILE *fp;
   char buf[256];
   char sys_cmd[256];
   
   
   sprintf(sys_cmd, "cat %s", USB_AUTO_SUSPEND_DIR);
   if ((fp = popen(sys_cmd, "r")) == NULL) {
       return (FAILED);
   } 
   while (fgets(buf, 255, fp) != NULL)
   if (pclose(fp) == -1) {
       return (FAILED);
   }

   *val = atoi(buf);

   return (PASSED);
}


/*******************************************************************************
  * Function   : diag_usb_enable_auto_suspend_feature
  * Description: Disable/Enable usb auto-suspend feature
  * Inputs     : DISABLE/ENABLE
  * Outputs    : None
  ******************************************************************************/
void diag_usb_enable_auto_suspend_feature (int opt)
{
    char sys_cmd[256];

    if (opt == EN_USB_AUTO_SUSPEND_FEATURE) {
        sprintf(sys_cmd, "echo %d > %s", EN_USB_AUTO_SUSPEND_FEATURE_VAL,
                USB_AUTO_SUSPEND_DIR);
    } else {
        sprintf(sys_cmd, "echo %d > %s", DIS_USB_AUTO_SUSPEND_FEATURE_VAL,
                USB_AUTO_SUSPEND_DIR);
    }
    
    system(sys_cmd);
}


/*******************************************************************************
  * Function   : diag_usb_get_hub_reset_status
  * Description: Get current hub status
  * Inputs     : stat - current hub status is in reset or out of reset
  * Outputs    : PASSED/FAILED
  ******************************************************************************/
int diag_usb_get_hub_reset_status (int *stat)
{
    uint data;

    if (plug_fpga_reg_read(FPGA_EXTER_DEV_RST_REG, &data) != PASSED) {
        return (FAILED);
    }
    
    *stat = (data &= FPGA_USB_HUB_RESET) >> FPGA_USB_HUB_RESET_BIT; 
    return (PASSED);
}



/*-------------------------------------------------
$Log: diag_usb_lib.c,v $
Revision 1.1  2019/10/16 02:27:15  sherliu2
Fix CSCvq98193, disable auto-suspend feature for Star C1109-4P


*/
