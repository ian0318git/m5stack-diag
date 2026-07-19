/* $Id: plug_testcard_usb_lib.c,v 1.3 2021/09/24 01:27:20 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_usb_lib.c,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_usb_lib.c - PLUGGABLE Test Card USB Functions
 *
 * Copyright (c) 2015 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "proto.h"
#include "menu.h"
#include "plug_slot.h"
#include "plug_common_host_impl.h"
#include "plug_testcard_util.h"
#include "plug_temp_sensor_test.h"
#include "plug_testcard_test.h"
#include "plug_testcard_host_impl.h"
#include "plug_testcard_usb_lib.h"
#include "plug_common_host.h"
#include "plug_host_fpga_lib.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"

int plug_testcard_main(void *);

struct usb_info_t plug_usb[MAX_PLUG_USB_MASS_STORE_ON_SYS];
static int plug_tc_usb_find_dev_name(int, char *);
static char *plug_tc_usb_get_str(char *, char *, char *);
static int plug_tc_usb_match(struct usb_info_t *, struct usb_info_t *);
int plug_tc_usb_get_speed(int);
int plug_tc_usb_parse_info(void);
static int plug_tc_usb_get_scsi_host_no(struct usb_info_t *);

extern struct plug_intf_t *plug_test_if;


/*******************************************************************************
 *
 * Function   :    plug_tc_usb_mass_stor_present_index
 * Description:    This function checks whether USB Mass storage is detected 
 *                 by bus and level number and return usb index if it is detected
 * Inputs     :    slot - plug slot
 *                 usb_idx - Index to the USB Internal Data Structure
 *                 usb_speed - present USB speed
 *                 hub - include hub or not  
 * Outputs    :    TRUE or FALSE
 *
 *******************************************************************************
 */
int plug_tc_usb_mass_stor_present_index (int slot, int *usb_idx, int usb_speed, int hub)
{
    int ix;
    int bus_no=0, lev_no=0, prnt_no=0,port_no=0;

    plug_tc_host_reply_usb_bus_lev_port_info(slot, usb_speed, &bus_no, &lev_no, 
                                             &prnt_no, &port_no, hub);   

    /* Traverse the USB internal data structure */
    for (ix = 0; ix < MAX_PLUG_USB_MASS_STORE_ON_SYS; ix++) {
        if (((plug_usb[ix].bus == bus_no) || (bus_no == PLUG_TC_USB_IGNORE)) && 
            ((plug_usb[ix].lev == lev_no)  || (lev_no == PLUG_TC_USB_IGNORE)) &&
            ((plug_usb[ix].prnt == prnt_no)  || (prnt_no == PLUG_TC_USB_IGNORE)) &&
            ((plug_usb[ix].port == port_no) || (port_no == PLUG_TC_USB_IGNORE))) {
            *usb_idx = ix;
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s: USB preset PASS. index = %d, slot = %d, bus_no = %d, lev = %d, \
                        prnt = %d, port = %d.\n",
                       __FUNCTION__, ix, slot, bus_no, lev_no, prnt_no, port_no);
            }
            return (TRUE);
        }
    } 
    *usb_idx = 0;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s: USB preset fail. slot = %d, bus_no = %d, lev = %d, prnt = %d, port = %d.\n",
               __FUNCTION__, slot, bus_no, lev_no, prnt_no, port_no);
    }
    return (FALSE);
}

/*******************************************************************************
 *
 * Function   :    plug_tc_usb_parse_info
 * Description:    search proc file for usb devices and save info into usb struct.
 * Inputs     :    NONE
 * Outputs    : errno, if failed. 0 otherwise.
 *
 *******************************************************************************
 */
int plug_tc_usb_parse_info (void)
{
    struct usb_info_t usb_tmp;
    FILE *fp;
    char line[82], tmp[32], *ptr, *ptr2;
    int str_idx = 0, ix = 0;

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

    memset(plug_usb, 0, sizeof(plug_usb));

    ptr = &line[1];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'T') {
            str_idx = 0;
            memset(&usb_tmp, 0, sizeof(usb_tmp));
            sscanf(ptr,
                   "%[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d %[^0-9]%d\n",
                   tmp, (int *) &usb_tmp.bus, tmp, (int *) &usb_tmp.lev,
                   tmp, (int *) &usb_tmp.prnt, tmp, (int *) &usb_tmp.port,
                   tmp, (int *) &usb_tmp.cnt, tmp, (int *) &usb_tmp.dev,
                   tmp, &usb_tmp.spd, tmp, &usb_tmp.mxch);

        }

        /*
         * end if 'T'
         */
        /*
         * save manufacturing info into temp usb structure
         * if device exists, there will be 3 lines starting with 'S'.
         */
        if (line[0] == 'S') {
            if ((ptr2 = strstr(ptr, "="))) {
                ptr2++;
                ptr2[strlen(ptr2) - 1] = '\0';
                snprintf(usb_tmp.mfg[str_idx], sizeof(usb_tmp.mfg[str_idx]), "%s", ptr2);
                str_idx++;
            }
        }

        if (line[0] == 'I') {
            if ((ptr2 = strstr(ptr, "Driver=usb-storage"))) {
                usb_tmp.found = 1;
                memcpy(&plug_usb[ix], &usb_tmp, sizeof(usb_tmp));

                /* Assign None string to avoid error on USB without SN */
                if (!strcmp(plug_usb[ix].mfg[2], "")) {
                    sprintf(plug_usb[ix].mfg[2], "None");
                }

               /* Get SCSI Host number based on Vendor and Serial Number */
               if (plug_tc_usb_get_scsi_host_no(&plug_usb[ix]) == PASSED) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****BUS FOUND**** no = %d\n",
                               plug_usb[ix].dev);
                    }
                }

                /* Find the block device name based on scsi host number */
                if (plug_tc_usb_find_dev_name(plug_usb[ix].dev, plug_usb[ix].dev_name) != FAILED) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****dev found**** dev = %s\n",
                               plug_usb[ix].dev_name);
                    }
                }
                ix++;
            }
        }
    } /* while */

    fclose(fp);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    plug_tc_usb_get_str
 * Description:    get usb string from proc file.
 * Inputs     :    line, entire line from file, key, key string to look for
 * Outputs    : ptr, string value to be saved. return NULL if failed to
 *              find key in string.
 *
 *******************************************************************************
 */
static char *plug_tc_usb_get_str (char *ptr, char *line, char *key)
{
    if ((ptr = strstr(line, key))) {
        while (*ptr != ':')
            ptr++;
        ptr += 2;
        ptr[strlen(ptr) - 1] = '\0';
    } else {
        return NULL;
    }
    return (ptr);
}

/*******************************************************************************
 *
 * Function   :    plug_tc_usb_find_dev_name
 * Description:    search file system for the device
 * Inputs     :    dev_idx - Device number
 *                 dev_name - Device name, such as sda
 * Outputs    :    PASSED/FAILED
 *
 *******************************************************************************
 */
static int plug_tc_usb_find_dev_name (int dev_idx, char *dev_name)
{
    DIR *dir;
    struct dirent *dp;
    char name[82];

    /* Sanity check */
    if (dev_name == NULL) {
        printf("%s: Null pointer\n", __func__);
        return (FAILED);
    }   

    sprintf(name, "%s/%d:0:0:0/block", USB_SCSI_DEVICE_FILE,
            dev_idx);

    if ((dir = opendir(name)) == NULL) {
        cterr('f', 0, "Cannot open directory %s.", name);
        return (FAILED);
    }   
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.') {
            continue;
        }   

        sprintf(dev_name, dp->d_name);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf(" at /dev/%s\n", dev_name);
        }   
    }   
    closedir(dir);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    plug_tc_usb_match
 * Description:    find match withbetween 2 usb devices that have same str value.
 * Inputs     :    usb_ptr, fist usb device; tmp, second usb devicex
 * Outputs    : 1, if match occurs; 0, otherwise
 *
 *******************************************************************************
 */
static int plug_tc_usb_match (struct usb_info_t *usb_ptr, struct usb_info_t *tmp)
{

    /*
     * need to match all 3 fiels: name, prodId, and serial number
     */
    if (strcmp(usb_ptr->mfg[0], tmp->mfg[0]) == 0 &&
        strcmp(usb_ptr->mfg[1], tmp->mfg[1]) == 0 &&
        strcmp(usb_ptr->mfg[2], tmp->mfg[2]) == 0) {
        return 1;
    }

    return 0;
}

/*******************************************************************************                                                                                                                                                     
 *
 * Function   :    plug_tc_usb_get_speed
 * Description:    print out usb speed info
 * Inputs     :    usb_idx, index into the device number
 * Outputs    :    PLUG_TESTCARD_USB2P0_SPEED (USB 2.0) or PLUG_TESTCARD_USB3P0_SPEED (USB 3.0)
 *
 *******************************************************************************
 */
int plug_tc_usb_get_speed (int usb_idx)
{       
    return (plug_usb[usb_idx].spd);
}   

/*******************************************************************************
 *
 * Function   :    plug_tc_usb_get_scsi_host_no
 * Description:    get scsi_host_number given usb_ptr struct.
 * Inputs     :    usb_ptr, usb structure
 * Outputs    :    PASSED or FAILED
 *
 *******************************************************************************
 */
static int plug_tc_usb_get_scsi_host_no (struct usb_info_t *usb_ptr)
{
    int scsi_host_no;
    struct usb_info_t usb_tmp;
    FILE *fp;
    DIR *dir;
    struct dirent *dp;
    char line[82], *ptr;

    if ((dir = opendir(USB_SCSI_USB_STORAGE_FILE)) == NULL) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr('f', 0, "Cannot open directory %s\n",
                  USB_SCSI_USB_STORAGE_FILE);
        }
        return (FAILED);
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.') {
            continue;
        }

        /*
         * check make sure it's a number
         */
        sprintf(line, "%s/%s", USB_SCSI_USB_STORAGE_FILE, dp->d_name);
        if ((NVRAM)->diagflag & D_VERBOSE) { 
            printf("%s/%s\n", USB_SCSI_USB_STORAGE_FILE, dp->d_name);
        }

        if ((fp = fopen(line, "r")) == NULL) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                cterr('f', 0, "Cannot open file %s\n", line);
            }
            fclose(fp);
            return (FAILED);
        }

        memset(&usb_tmp, 0, sizeof(usb_tmp));

        while (fgets(line, sizeof(line), fp)) {

            if (strstr(line, "usb-storage")) {
                scsi_host_no = atoi(dp->d_name);
                continue;       /* go to the next line */
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("d_name :%s\n", dp->d_name);
                printf("scsi_host_number :%d\n", scsi_host_no);
            }

           ptr = NULL;
           if ((ptr = plug_tc_usb_get_str(ptr, line, "Vendor"))) {
               sprintf(usb_tmp.mfg[0], "%s", ptr);
               if ((NVRAM)->diagflag & D_VERBOSE) {
                   printf("Vendor found :%s\n", usb_tmp.mfg[0]);
               }
           }
           if ((ptr = plug_tc_usb_get_str(ptr, line, "Product"))) {
               sprintf(usb_tmp.mfg[1], "%s", ptr);
               if ((NVRAM)->diagflag & D_VERBOSE) {
                   printf("Product found :%s\n", usb_tmp.mfg[1]);
               }
           }
           if ((ptr = plug_tc_usb_get_str(ptr, line, "Serial Number"))) {
               snprintf(usb_tmp.mfg[2], sizeof(usb_tmp.mfg[2]), "%s", ptr);
               if ((NVRAM)->diagflag & D_VERBOSE) {
                   printf("SN found :%s\n", usb_tmp.mfg[2]);
               }
               if (plug_tc_usb_match(usb_ptr, &usb_tmp)) {
                   usb_ptr->dev = scsi_host_no;
                   break;
               }
            }
        }                       /* end while scanning file  */
        fclose(fp);
    }                           /* end while scanning directory */
    closedir(dir);
    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_testcard_usb_lib.c,v $
Revision 1.3  2021/09/24 01:27:20  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.2  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.1.2.6  2018/11/16 06:40:49  hondwang
modify PRRQ suggest with CSCvn17216 pluggable re-instruct

Revision 1.1.2.5  2018/11/14 07:40:11  hondwang
Modify code for no serial number fix

Revision 1.1.2.4  2018/11/02 09:50:46  hondwang
Add USB prnt info for tabei-L

Revision 1.1.2.3  2018/11/01 12:59:33  hondwang
Modify pluggable testcard USB Hub testing with random port

Revision 1.1.2.2  2018/11/01 06:24:33  hondwang
Add plug testcard USB HUB testing function

Revision 1.1.2.1  2018/10/15 06:44:31  hondwang
pluggable common code re-instruct add and remove files


 
*/
