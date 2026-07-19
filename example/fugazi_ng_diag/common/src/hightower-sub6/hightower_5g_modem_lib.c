/* $Id: hightower_5g_modem_lib.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/hightower_5g_modem_lib.c,v $
 *********************************************************************
 *
 * hightower_5g_modem_lib.c -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "plat_defs.h"
#include "unistd.h"
#include "gpio.h"
#include "hightower_5g_modem_lib.h"
#include "highrise_cpld_lib.h"
#include "hightower_sub6.h"
#include <dirent.h>


/*SWI VID PID got changed from Alpha4 firmware */
swi_modem_vid_did_info_t swi_modem_vid_did_list [] = \
 { {0x18d7, 0x200, 1},
   {0x17cb, 0x306, 0},//Alpha4 and above firmware
   {0,0}
 };

/*******************************************************************************
 * Name: diag_modem_get_tty_devname
 * Description: This function returns serial TTY device name which the
 *              specified device attaches to.
 * Input: *tty_dev - Pointer to store the TTY device name 
 * Output: PASSED/FAILED
 *******************************************************************************
 */
int diag_modem_get_tty_devname (char *tty_dev)
{
    /* Sanity check */

    if (tty_dev == NULL) {
        printf("%s: NULL pointer\n", __func__);
    }

    sprintf(tty_dev, TTY_DEV_NAME);
    return (PASSED);
}

/*******************************************************************************
 * Function    : diag_5g_swi_dev_create
 * Description : Function to create modem Device Object
 * Inputs      : diag_5g_swi_obj - Pointer of modem device driver object
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int diag_5g_swi_dev_create (dev_5g_swi_object_t *diag_5g_swi_obj)
{
    dev_object_t *dev = (dev_object_t *)diag_5g_swi_obj;

    /* Create common device object */
    swi_5g_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    diag_5g_swi_obj->base.dev_object_fvt->dev_attach(dev);

    /* Assign modem type */
    memset(diag_5g_swi_obj->model, 0, sizeof(diag_5g_swi_obj->model));
    diag_5g_swi_obj->callout_fvt->get_tty_dev_name = diag_modem_get_tty_devname;
    
    return (PASSED); 
}


#ifdef ENABLE_USB_PORT
/*******************************************************************************
 * Function   : diag_swi_5g_modem_usb_get_vid_did_speed
 * Description: This function reads from system USB file and return Vendor ID,
 *              Device ID and speed
 * Inputs     : usb_path - USB Path, e.g. 1-1, 3-1, 4-1
 *              *vid, *did, *speed
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_swi_5g_modem_usb_get_vid_did_speed (char *usb_path, int *vid,
                                           int *did, int *speed)
{
    FILE *file;
    char fname[64];


    /* Check if the file exists */
    sprintf(fname, "%s/%s", USB_SYS_DRV_PATH, usb_path);
    if (access(fname, F_OK) == -1) {
        printf ("\n%s[%d]failed", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    /* Get the Vendor ID */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH, usb_path, USB_SYS_VID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        printf ("\n%s[%d] vid failed", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    fscanf(file, "%x", vid);

    fclose(file);

    /* Get the Product ID */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH, usb_path, USB_SYS_DID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        printf ("\n%s[%d] did failed", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    fscanf(file, "%x", did);

    fclose(file);

    /* Get the Speed */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH, usb_path, USB_SYS_SPEED_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        printf ("\n%s[%d] speed failed", __FUNCTION__, __LINE__);
        return (FAILED);
    }

    fscanf(file, "%d", speed);

    fclose(file);
    return(PASSED);
}

/*******************************************************************************
 * Function   : diag_swi_5g_modem_usb_detect
 * Description: Enumerates USB and detects USB device by given vendor and device
 *              ID and speed
 * Inputs     : usb_devinfo - USB device info(e.g. 3-1, 4-1)
 *              vid - Vendor ID
 *              speed - 480 (USB 2.0) or 5000 (USB 3.0)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_swi_5g_modem_usb_detect (char *usb_devinfo, int vid, int speed)
{
    int dev_vid, dev_did, dev_speed;

    if (diag_swi_5g_modem_usb_get_vid_did_speed(usb_devinfo, &dev_vid, &dev_did,
                                       &dev_speed) == FAILED) {
        printf ("\nFailed to read the vid did");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Vendor ID=%#x, Device ID=%#x, Speed=%#x\n", dev_vid, dev_did,
                                                            dev_speed);
   }

    if ((vid == dev_vid) && (speed == dev_speed)) {
        return (PASSED);
    }

    printf ("\nvid did failed");
    return (FAILED);
}
#endif

/*******************************************************************************
 * Function   : plug_NR_5g_pci_get_vid_did_speed
 * Description: This function reads from system PCI file and return Vendor ID,
 *              Device ID and speed
 * Inputs     : pci path
 *              *vid, *did, *speed
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int diag_swi_5g_modem_pci_get_vid_did_speed (char *pci_path, int *vid,
                                           int *did, int *speed)
{
    FILE *file;
    char fname[64];


    /* Check if the file exists */
    sprintf(fname, "%s/%s", PCI_SYS_DRV_PATH, pci_path);
    if (access(fname, F_OK) == -1) {
        printf ("\n%s[%d]failed : %s", __FUNCTION__, __LINE__, fname);
        return (FAILED);
    }

    /* Get the Vendor ID */
    sprintf(fname, "%s/%s/%s", PCI_SYS_DRV_PATH, pci_path, PCI_SYS_VID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        printf ("\n%s[%d] vid failed : %s", __FUNCTION__, __LINE__, fname);
        return (FAILED);
    }

    fscanf(file, "%x", vid);

    fclose(file);

    /* Get the Product ID */
    sprintf(fname, "%s/%s/%s", PCI_SYS_DRV_PATH, pci_path, PCI_SYS_DID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        printf ("\n%s[%d] did failed : %s", __FUNCTION__, __LINE__, fname);
        return (FAILED);
    }

    fscanf(file, "%x", did);

    fclose(file);

    return(PASSED);
}

/*******************************************************************************
 * Function   : plug_NR_5g_pci_detect
 * Description: Enumerates PCI and detects PCI device by given vendor and device
 *              ID and speed
 * Inputs     : pci_devinfo - device info
 *              vid - Vendor ID
 *              did - Device ID
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_swi_5g_modem_pci_detect (char *pcie_devinfo, int vid, int did)
{
    int dev_vid, dev_did, dev_speed;
    swi_modem_vid_did_info_t *travers_dev_info = swi_modem_vid_did_list;

    if (diag_swi_5g_modem_pci_get_vid_did_speed(pcie_devinfo, &dev_vid, &dev_did,
                                       &dev_speed) == FAILED) {
        printf ("\nFailed to read the vid did");
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Vendor ID=%#x, Device ID=%#x, Speed=%#x\n", dev_vid, dev_did,
                                                            dev_speed);
    }

    while (travers_dev_info->vid != 0) {
        if ((travers_dev_info->vid == dev_vid) && \
            (travers_dev_info->did == dev_did)) {
            if (travers_dev_info->warn_msg == 1) {
                printf ("\n\n!!!! Warning : " \
                        "Modem fw is old, please upgrade to Alpha4 !!!!\n\n");
            }
            return (PASSED);
        }
        travers_dev_info++;
    }
    printf ("\nvid did failed VID : 0x%x expected : 0x%x", dev_vid, vid);
    return (FAILED);
}

/*******************************************************************************
 * Function   : diag_swi_5g_usb_deb_enable
 * Description: Function to route USB2 signals from the modem
 *              to the onboard USB connector
 * Inputs     : input - 0 for enable, 1 for disable
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_swi_5g_usb_deb_enable (int input)
{
    int rc;
    if (input == OPT_ENABLE) {
        input = 0;
    } else {
        input = 1;
    }
    rc = gpio_write(USB_MUX_DEBUG_EN, input);
    if (rc == -1) {
        return (FAILED);
    }

    /* TBD  enable debug port*/
    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_swi_5g_sim_selection
 * Description: Function to route sim signals from the modem
 *              to the SIM connector
 * Inputs     : input - 0 for SIM0, 1 for SIM1
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_swi_5g_sim_selection (int input)
{
    int rc;
    if (input == SIM0) {
        input = 0;
    } else {
        input = 1;
    }

    rc = gpio_write(SIM_SELECT, input);
    if (rc == -1) {
        printf ("\nFailed to write GPIO for sim switching");
        return (FAILED);
    }
    printf ("\nSim%d Selected ", input);
    return (PASSED);
}


/*********************************************************************
 * $Log: hightower_5g_modem_lib.c,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.3  2020/12/22 22:49:28  tshanmug
 * Empire prrq review comment fix
 *
 * Revision 1.1.4.2  2020/10/12 15:48:35  tshanmug
 * Chrysler menu change, mmwave ant test added and Empire modem code cleanup
 *
 * Revision 1.1.4.1  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

