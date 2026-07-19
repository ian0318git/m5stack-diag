/* $Id: usb_dongle_common_lib.c,v 1.2 2019/06/14 09:59:33 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_common/usb_dongle_common_lib.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_common_lib.c - USB dongle common library functions.
 *
 *
 * Copyright (c) 2015-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "proto.h"
#include "nvmonvars.h"
#include "usb_dongle_common_lib.h" 
#include "usb_dongle_common_test.h"
#include "usb_dongle_lte_swi_test.h"
#include "usb_dongle_host_slot_modules.h"


/* Prototype */
int usb_dongle_get_module_entry_ptr(struct udongle_intf_t *);
int usb_dongle_get_vid_pid_product(char *, uint16_t *, uint16_t *, char *);
int usb_dongle_parse_info(void);
int usb_dongle_get_bus_speed(int);
int usb_dongle_dev_is_mass_storage(int);
int usb_dongle_dev_present_by_bus_lev(int, int, int, int *);

/* Global */
static struct udongle_module_info *usb_dongle_get_module(uint16_t, uint16_t,
                                                         char *);
static struct usb_dongle_info_t usb_dongle[USB_DEVICE_MAX_NUM];

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_get_bus_speed
 * Description: print out usb speed info
 * Inputs:      usb_idx - Index to USB device
 * Outputs:     USB_2P0_SPEED (USB 2.0) or USB_3P0_SPEED (USB 3.0)
 *------------------------------------------------------------------------------
 */
int usb_dongle_get_bus_speed (int usb_idx)
{
    return (usb_dongle[usb_idx].spd);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_parse_info
 * Description: Function to search proc file for usb devices and save info 
 *              into usb struct.
 * Inputs:      NONE
 * Outputs:     errno, if failed. 0 otherwise.
 *------------------------------------------------------------------------------
 */
int usb_dongle_parse_info (void)
{
    struct usb_dongle_info_t usb_dongle_tmp;
    FILE *fp;
    char line[82], tmp[32], tmp1[32] = {0, }, *ptr, *ptr2;
    int ix = 0;

    if (access(USB_DEVICE_FILE, F_OK) == -1) {
        /* if file doesn't exist, try to mount it */
        system(MOUNT_DEBUGFS);
    }

    fp = fopen(USB_DEVICE_FILE, "r");
    /* sys/kernel/debug/usb/devies */

    if (!fp) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s not found", USB_DEVICE_FILE);
        }
        printf("%s: '%s' can't be opened\n", __func__, USB_DEVICE_FILE);
        return (FAILED);
    }

    memset(usb_dongle, 0, sizeof(usb_dongle));

    ptr = &line[1];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'T') {
            memset(&usb_dongle_tmp, 0, sizeof(usb_dongle_tmp));
            sscanf(ptr,
                   "%[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d \
                   %[^0-9]%d %[^0-9]%d %[^0-9]%d\n",
                   tmp, (int *) &usb_dongle_tmp.bus,
                   tmp, (int *) &usb_dongle_tmp.lev,
                   tmp, (int *) &usb_dongle_tmp.prnt,
                   tmp, (int *) &usb_dongle_tmp.port,
                   tmp, (int *) &usb_dongle_tmp.cnt,
                   tmp, (int *) &usb_dongle_tmp.dev,
                   tmp, &usb_dongle_tmp.spd, tmp, &usb_dongle_tmp.mxch);
        }
        /*
         * end if 'T'
         */

        /* Store usb device class */
        if (line[0] == 'D') {
            if ((ptr2 = strstr(ptr, USB_CLASS_STR))) {
                ptr2 = ptr2 + strlen(USB_CLASS_STR);
                ptr2[USB_CLS_SIZE] = '\0';
                usb_dongle_tmp.dcls = strtol(ptr2, NULL, HEX);
            }
        }

        /* Store usb interface class */
        if (line[0] == 'I') {
            if ((ptr2 = strstr(ptr, USB_CLASS_STR))) {
                ptr2 = ptr2 + strlen(USB_CLASS_STR);
                strncpy(tmp1, ptr2, USB_CLS_SIZE); 
                usb_dongle_tmp.icls = strtol(tmp1, NULL, HEX);
            }
            memcpy(&usb_dongle[ix], &usb_dongle_tmp, sizeof(usb_dongle_tmp));
            ix++; 
        }
    } /* while */

    fclose(fp);

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_dev_present_by_bus_lev 
 * Description: This function checks whether USB device is detected by bus 
 *              and level number and return usb index if it is detected
 * Inputs:      bus_2p0_no - USB 2.0 Bus Number
 *              bus_3p0_no - USB 3.0 Bus Number
 *              lev_no - USB Level Number
 *              usb_idx - Index to the USB Dongle Internal Data Structure
 * Outputs:     TRUE or FALSE
 *------------------------------------------------------------------------------
 */
int usb_dongle_dev_present_by_bus_lev (int bus_2p0_no, int bus_3p0_no,
                                       int lev_no, int *usb_idx)
{
    int ix;

    /* Traverse the USB internal data structure */
    for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
        if ((usb_dongle[ix].bus == bus_2p0_no ||
             usb_dongle[ix].bus == bus_3p0_no) && 
             usb_dongle[ix].lev == lev_no) {
            *usb_idx = ix;
            return (TRUE);
        }
    }

    *usb_idx = 0;

    return (FALSE);
}


/*------------------------------------------------------------------------------
 * Function:     usb_dongle_dev_is_mass_storage
 * Description : Function to check whether USB device is a mass storage deivce
 * Inputs:       usb_idx - Index to USB device
 * Output:       TRUE/FALSE
 *------------------------------------------------------------------------------
 */
int usb_dongle_dev_is_mass_storage (int usb_idx)
{
    if (usb_dongle[usb_idx].icls == USB_INTF_CLASS_STORAGE) {
        return (TRUE);
    }
    return (FALSE);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_get_module_entry_ptr
 * Description: This function essentially to get module test entry
 * Inputs:      udongle - ptr to struct udongle_intf_t
 * Outputs:     PASSED/FAILED
 *-----------------------------------------------------------------------------
 */
int usb_dongle_get_module_entry_ptr (struct udongle_intf_t *udongle)
{
    char u_port[16], product[64];
    uint16_t vendor_id, product_id;    
    struct udongle_module_info * udongle_module_ptr;

    /* sanity check */
    if (udongle == NULL) {
        printf("%s: Null pointer\n", __func__);
        return (FAILED);
    }

    /* To leave a blank line between network modules polling print
     * and this polling print */
    printf("\n");

    sprintf(u_port, "%s", udongle->port);

    /* Get module information(vendor ID, product ID, product) */
    if (usb_dongle_get_vid_pid_product(u_port, &vendor_id, &product_id,
                                       product) == FAILED) {
        return (FAILED);
    }
    udongle->vid = vendor_id;
    udongle->pid = product_id;
    sprintf(udongle->product, "%s", product);
          
    /* Retrieve module full info based on vendor ID, product ID, product*/
    udongle_module_ptr = (struct udongle_module_info *)
                          usb_dongle_get_module((uint16_t)udongle->vid, 
                                                (uint16_t)udongle->pid, 
                                                (char *)udongle->product);
    if (udongle_module_ptr == NULL) {
        cterr('f', 0, "USB port %s. Not supported. product = %s.\n",
              u_port, udongle->product);
        return (FAILED);
    }

    udongle->diag = udongle_module_ptr->diag;
    udongle->intf_diag = udongle_module_ptr->intf_diag;
    sprintf((char *)udongle->name, (char *)udongle_module_ptr->name);

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_get_module
 * Description: This function essentially goes through dongle_module_tb1 
 *              table and return module data based on module info(vendor id,
 *              product id, product)
 * Inputs:      vendor_id - module idVendor
 *              product_id - module idProduct
 *              product - module product
 * Outputs:     pointer of USB Module Entry or Null if Product ID is 
 *              unrecognized
 *------------------------------------------------------------------------------
 */
static struct udongle_module_info *usb_dongle_get_module (uint16_t vendor_id, 
                                                          uint16_t product_id, 
                                                          char *product)
{
    int ix;

    for (ix = 0; ix < MAX_DONGLE_IDS; ix++) {
        if (udongle_module_tb1[ix].vid == vendor_id && 
            udongle_module_tb1[ix].pid == product_id) {
        
            if (strcmp(udongle_module_tb1[ix].product, product) == 0) {
                return (struct udongle_module_info *)(&udongle_module_tb1[ix]);
            }
        }
    }

    return (struct udongle_module_info *)(NULL);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_get_vid_pid_product
 * Description: This function returns module's vendor id, product, id,
 *              product.
 * Inputs:      *u_port, *vendor_id, *product_id, *product
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_get_vid_pid_product (char *u_port,  uint16_t *vendor_id, 
                                    uint16_t *product_id, char *product)
{
    FILE *fp_vid_pid, *fp_product; 
    char line[64], get_vid_pid_cmd[80], str_product[64] = {0,};
    char vid, pid;
    char get_product_cmd[80] = {0,};
    int num_vid, num_pid = 0;

    /* Store vendor id and product id */
    sprintf(get_vid_pid_cmd, "%s/%s/uevent > %s", MID_INFO_CMD, u_port, 
            MID_INFO_FILE);
    system(get_vid_pid_cmd);
    fp_vid_pid = fopen(MID_INFO_FILE, "r");

    if (fp_vid_pid == NULL) {
        printf("Couldn't open file %s\n", MID_INFO_FILE);
    } else {
        while (fgets(line, sizeof(line), fp_vid_pid) != NULL) {
            /* Get the vendor id and the product id from PRODUCT */
            if (strstr(line, "PRODUCT=")) {

                /* Get vendor id */
                sscanf(line, "%*[^=]=%[^/]", &vid);
                num_vid = (int)strtol(&vid, NULL, 16);
                *vendor_id = num_vid;

                /* Get product id */
                sscanf(line, "%*[^/]/%[^/]", &pid);
                num_pid = (int)strtol(&pid, NULL, 16);
                *product_id = num_pid;

                /* Close file */
                fclose(fp_vid_pid);

                break;
            } else {
                continue;
            }
        }

        if ((vendor_id || product_id) != 0) {
            /* Store product info in MID_INFO_FILE */
            sprintf(get_product_cmd, "%s/%s/product > %s", MID_INFO_CMD,
                    u_port, MID_INFO_FILE);
            system(get_product_cmd);
            fp_product = fopen(MID_INFO_FILE, "r");

            if (fgets(str_product, sizeof(str_product), fp_product) != NULL) {
                /* Get product */
                str_product[strcspn(str_product, "\n")] = '\0';
                strcpy(product, str_product);

                /* Close file */
                fclose(fp_product);
                return (PASSED);
            }
        }
    } 
    fclose(fp_vid_pid);
    fclose(fp_product);
    return (FAILED);  
}


/*-------------------------------------------------
$Log: usb_dongle_common_lib.c,v $
Revision 1.2  2019/06/14 09:59:33  steja
Supported Cooper usb dongle LTE



*/
