/* $Id: diag_usb_lib.c,v 1.4 2019/07/11 12:31:30 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_usb_lib.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_lib.c
 *
 * Copyright (c) 2013-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "diag_usb_lib.h"
#include "platform_fru.h"
#include "mb_tests.h"

/*
 * Global extern functions
 */

/*
 * Declare local function
 */

int usb_parse_info(void);
int usb_get_speed(int);
int usb_mass_stor_present_by_bus_lev(int, int, int *);

static int usb_find_dev_name(int, char *);
static int usb_get_scsi_host_no(struct usb_info_t *);
static char *usb_get_str(char *, char *, char *);
static int usb_match(struct usb_info_t *, struct usb_info_t *);
int get_usb_storage_info(int, struct usb_info_t *);
int find_usb_storage_index(int, int *);

struct usb_info_t usb[USB_DEVICE_MAX_NUM];
/* Denverton CPU can not dynamically switch USB 3.0/2.0 speed, so it needs to 
 * parse the USB debug information to distinguish whether USB 3.0/2.0 flashs
 * are at USB hub. */
struct usb_info_t usb3_storage[USB_DEVICE_MAX_NUM];
struct usb_info_t usb2_storage[USB_DEVICE_MAX_NUM];
struct usb_info_t usb3_hub;
struct usb_info_t usb2_hub;

/*******************************************************************************
 *
 * Function   :    get_usb_storage_info
 * Description:    Get usb storage information 
 * Inputs     :    speed - USB storage speed
                   *usb_storage_info - pointer to usb_stroage_info data
 * Outputs    :    PASSED/FAILED
 *
 *******************************************************************************
 */
int get_usb_storage_info (int speed, struct usb_info_t *usb_storage_info) 
{
    int ix, ret_val = FALSE;
    
    if (speed == USB_HOST20_SPEED) {
        for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\nusb2_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                       " port: %#x, dev: %#x, spd: %d, found: %d\n", ix,
                       usb2_storage[ix].bus, usb2_storage[ix].lev,
                       usb2_storage[ix].prnt, usb2_storage[ix].port,
                       usb2_storage[ix].dev, usb2_storage[ix].spd,
                       usb2_storage[ix].found);
            }
            /* The found default value is FALSE for all usb2_storage index
             * usb_parse_info() will be called before this function and will set the found 
             * value to TRUE once usb flash is found. 
             * So "found == TRUE" means the USB2.0 flash storage information is found and 
             * copied. */
            if (usb2_storage[ix].found == TRUE) {
                memcpy(usb_storage_info, &usb2_storage[ix], sizeof(struct usb_info_t));
                ret_val = PASSED;
                break;
            }
        }
    } else if (speed == USB_HOST30_SPEED) {
        for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("\nusb3_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                       "port: %#x, dev: %#x, spd: %d, found: %#x\n", ix,
                       usb3_storage[ix].bus, usb3_storage[ix].lev,
                       usb3_storage[ix].prnt, usb3_storage[ix].port,
                       usb3_storage[ix].dev, usb3_storage[ix].spd,
                       usb3_storage[ix].found);
            }
            /* The found default value is FALSE for all usb3_storage index
             * usb_parse_info() will be called before this function and will set the found 
             * value to TRUE once usb flash is found. 
             * So "found == TRUE" means the USB3.0 flash storage information is found and 
             * copied. */
            if (usb3_storage[ix].found == TRUE) {
                memcpy(usb_storage_info, &usb3_storage[ix], sizeof(struct usb_info_t));
                ret_val = PASSED;
                break;
            }
        }
    } else { 
        cterr('f', 0, "%s() USB speed %#x does not exist.", __FUNCTION__, speed);
        return (FAILED);
    }
   
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("usb_storage_info->bus: %#x, lev: %#x, prnt: %#x, "
               "port: %#x, dev: %#x, spd: %d, found: %#x\n", usb_storage_info->bus,
               usb_storage_info->lev, usb_storage_info->prnt, usb_storage_info->port,
               usb_storage_info->dev, usb_storage_info->spd, usb_storage_info->found);
    }

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   :    usb_parse_info
 * Description:    search proc file for usb devices and save info into usb struct.
 * Inputs     :    NONE
 * Outputs    :    errno, if failed. 0 otherwise.
 *
 *******************************************************************************
 */
int usb_parse_info (void)
{
    struct usb_info_t usb_tmp;
    FILE *fp;
    char line[82], tmp[32], *ptr, *ptr2;
    int str_idx = 0, ix = 0, jx = 0, nx = 0;

    if (access(USB_DEVICE_FILE, F_OK) == -1) {
        /* if file doesn't exist, try to mount it */
        system(MOUNT_DEBUGFS);
    }

    fp = fopen(USB_DEVICE_FILE, "r");
    /* sys/kernel/debug/usb/devices */

    if (!fp) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s not found", USB_DEVICE_FILE);
        }
        printf("%s: '%s' can't be opened\n", __func__, USB_DEVICE_FILE);
        return (FAILED);
    }

    memset(usb, 0, sizeof(usb));
    memset(usb3_storage, 0, sizeof(usb3_storage));
    memset(usb2_storage, 0, sizeof(usb2_storage));
    memset(&usb3_hub, 0, sizeof(usb3_hub));
    memset(&usb2_hub, 0, sizeof(usb2_hub));

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

        if ((usb_tmp.bus == USB_BUS_2) && (usb_tmp.spd == USB_HOST30_SPEED) && 
            (usb_tmp.prnt == USB_DEV_1) && (usb_tmp.port == USB_PORT_0)) {
            /* Nutella USB 3.0 port number is fixed at 0, prnt number is 
             * inherited from CPU USB 3.0 controller device number 1 */
            memcpy(&usb3_hub, &usb_tmp, sizeof(usb_tmp));
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("usb3_hub.bus: %#x, usb3_hub.lev: %#x, "
                       "usb3_hub.prnt: %#x, usb3_hub.port: %#x, "
                       "usb3_hub.dev: %#x, usb3_hub.spd: %#x\n", 
                       usb3_hub.bus, usb3_hub.lev, usb3_hub.prnt,
                       usb3_hub.port, usb3_hub.dev, usb3_hub.spd);
            }
        } else if ((usb_tmp.bus == USB_BUS_1) && (usb_tmp.spd == USB_HOST20_SPEED) &&
                (usb_tmp.prnt == USB_DEV_1) && (usb_tmp.port == USB_PORT_0)) {
            /* Nutella USB 2.0 port number is fixed at 0, prnt number is 
             * inherited from CPU USB 2.0 controller device number 1 */
            memcpy(&usb2_hub, &usb_tmp, sizeof(usb_tmp));
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("usb2_hub.bus: %#x, usb2_hub.lev: %#x, "
                       "usb2_hub.prnt: %#x, usb2_hub.port: %#x, "
                       "usb2_hub.dev: %#x, usb2_hub.spd: %#x\n",
                       usb2_hub.bus, usb2_hub.lev, usb2_hub.prnt,
                       usb2_hub.port, usb2_hub.dev, usb2_hub.spd);
            }
        }
        /*
         * end if 'T'
         */

        /*
         * save manufacturing info into temp usb structure
         * if dievice exists, there will be 3 lines starging with 'S'.
         */
        if (line[0] == 'S') {
            if ((ptr2 = strstr(ptr, "="))) {
                ptr2++;
                ptr2[strlen(ptr2) - 1] = '\0';
                sprintf(usb_tmp.mfg[str_idx++], "%s", ptr2);
            }

        }

        if (line[0] == 'I') {
            if ((ptr2 = strstr(ptr, "Driver=usb-storage"))) {
                usb_tmp.found = TRUE;
                memcpy(&usb[nx], &usb_tmp, sizeof(usb_tmp));
                
                /* USB plugs with USB3.0 hub. */
                if ((usb_tmp.prnt == usb3_hub.dev)) {
                    /* This USB flash is plugged at USB 3.0 hub */
                    memcpy(&usb3_storage[ix], &usb_tmp, sizeof(usb_tmp));
                    usb3_storage[ix].found = TRUE;
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("\nusb3_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                               "port: %#x, dev: %#x, spd: %d, found: %#x\n", ix, 
                               usb3_storage[ix].bus, usb3_storage[ix].lev,
                               usb3_storage[ix].prnt, usb3_storage[ix].port,
                               usb3_storage[ix].dev, usb3_storage[ix].spd,
                               usb3_storage[ix].found);
                    }
                    ix++;
                } else if (usb_tmp.prnt == usb2_hub.dev) {
                    /* This USB flash is plugged at USB 2.0 hub */
                    memcpy(&usb2_storage[jx], &usb_tmp, sizeof(usb_tmp));
                    usb2_storage[jx].found = TRUE;
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("\nusb2_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                                "port: %#x, dev: %#x, spd: %d, found: %d\n", jx,
                                usb2_storage[jx].bus, usb2_storage[jx].lev,
                                usb2_storage[jx].prnt, usb2_storage[jx].port,
                                usb2_storage[jx].dev, usb2_storage[jx].spd,
                                usb2_storage[jx].found);
                    }
                    jx++;
                } 
                
                /* No USB hub was plugged on USB */
                if ((usb_tmp.bus == USB_BUS_2) && (usb_tmp.prnt == USB_DEV_1) &&
                    (usb_tmp.port == USB_PORT_0)) {
                    /* This USB 3.0 flash is directly plugged with USB port. */
                    memcpy(&usb3_storage[ix], &usb_tmp, sizeof(usb_tmp));
                    usb3_storage[ix].found = TRUE;
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("\nusb3_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                               "port: %#x, dev: %#x, spd: %d, found: %#x\n", ix,
                               usb3_storage[ix].bus, usb3_storage[ix].lev,
                               usb3_storage[ix].prnt, usb3_storage[ix].port,
                               usb3_storage[ix].dev, usb3_storage[ix].spd,
                               usb3_storage[ix].found);
                    }
                    ix++;
                } else if ((usb_tmp.bus == USB_BUS_1) && (usb_tmp.prnt == USB_DEV_1) && 
                           (usb_tmp.port == USB_PORT_0)) {
                    /* This USB 2.0 flash is directly plugged at USB port. */
                    memcpy(&usb2_storage[jx], &usb_tmp, sizeof(usb_tmp));
                    usb2_storage[jx].found = TRUE;
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("\nusb2_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                               "port: %#x, dev: %#x, spd: %d, found: %d\n", jx,
                               usb2_storage[jx].bus, usb2_storage[jx].lev,
                               usb2_storage[jx].prnt, usb2_storage[jx].port,
                               usb2_storage[jx].dev, usb2_storage[jx].spd,
                               usb2_storage[jx].found);
                    }
                    jx++;
                }                

                /* Get SCSI Host number based on Vendor and Serial Number */
                if (usb_get_scsi_host_no(&usb[nx]) == PASSED) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                         printf("****BUS FOUND**** no = %d\n", usb[nx].dev);
                    }
                }

                /* Find the block device name based on scsi host number */
                if (usb_find_dev_name(usb[nx].dev, usb[nx].dev_name) != FAILED) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****dev found**** dev = %s\n", usb[nx].dev_name);
                    }
                }
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("usb[%d].bus: %#x, lev: %#x, prnt: %#x, port: %#x, dev: %#x, spd: %d\n", 
                            nx, usb[nx].bus, usb[nx].lev, usb[nx].prnt,
                            usb[nx].port, usb[nx].dev, usb[nx].spd);
                }
                nx++;
            }
        }

        if (line[0] == 'I') {
            /* port 0 is LTE debug port*/
            if ((ptr2 = strstr(ptr, "Driver=sierra")) && usb_tmp.port == USB_PORT_0) {
                usb_tmp.found = 1;
                /* bus 1 is for 2.0 bus 2 is for 3.0*/
                if (usb_tmp.bus == USB_BUS_1 || usb_tmp.bus == USB_BUS_2) {
                    ix = 2; 
                }     
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            } else  if ((ptr2 = strstr(ptr, "Driver=sierra")) && usb_tmp.lev != USB_LEV_1) {
                usb_tmp.found = 1;
                /* bus 1 is for 2.0 bus 2 is for 3.0*/
                if ( usb_tmp.bus == USB_BUS_1 || usb_tmp.bus == USB_BUS_2) {
                    ix = 2;
                }
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            } else  if ((ptr2 = strstr(ptr, "Driver=sierra")) && usb_tmp.lev == USB_LEV_1 &&
                        usb_tmp.prnt == 1 &&usb_tmp.port == USB_PORT_1) {
                /* bus 1 is for 2.0 bus 2 is for 3.0*/
                usb_tmp.found = 1;
                if (usb_tmp.bus == USB_BUS_1 || usb_tmp.bus == USB_BUS_2) {
                    ix = 3;
                }
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            }
        }
    } /* while */

    fclose(fp);

    if (usb[0].found == 0 && usb[1].found == 0 && usb[2].found == 0 && usb[3].found == 0) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    usb_mass_stor_present_by_bus 
 * Description:    This function checks whether USB Mass storage is detected 
 *                 by bus and level number and return usb index if it is detected
 * Inputs     :    bus_no - Bus Number
 *                 lev_no - Level Number
 *                 usb_idx - Index to the USB Internal Data Structure
 * Outputs    :    TRUE or FALSE
 *
 *******************************************************************************
 */
int usb_mass_stor_present_by_bus_lev (int bus_no, int lev_no, int *usb_idx)
{
    int ix;

    /* Traverse the USB internal data structure */
    for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
        if (usb[ix].bus == bus_no && usb[ix].lev == lev_no && usb[ix].found == TRUE) {
            *usb_idx = ix;
            return (TRUE);
        }
    }

    *usb_idx = 0;

    return (FALSE);
}

/*******************************************************************************
 *
 * Function   :    usb_get_speed
 * Description:    print out usb speed info
 * Inputs     :    idx, index into the device number
 * Outputs    :    USB_HOST20_SPEED (USB 2.0) or USB_HOST30_SPEED (USB 3.0)
 *
 *******************************************************************************
 */
int usb_get_speed (int usb_idx)
{
    return (usb[usb_idx].spd);
}

/*******************************************************************************
 *
 * Function   :    usb_get_scsi_host_no
 * Description:    get scsi_host_number given usb_ptr struct.
 * Inputs     :    usb_ptr, usb structure
 * Outputs    :    PASSED or FAILED
 *
 *******************************************************************************
 */
static int usb_get_scsi_host_no (struct usb_info_t *usb_ptr)
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
           if ((ptr = usb_get_str(ptr, line, "Vendor"))) {
               sprintf(usb_tmp.mfg[0], "%s", ptr);
               if ((NVRAM)->diagflag & D_VERBOSE) {
                   printf("Vendor found :%s\n", usb_tmp.mfg[0]);
               }
           }
           if ((ptr = usb_get_str(ptr, line, "Product"))) {
               sprintf(usb_tmp.mfg[1], "%s", ptr);
               if ((NVRAM)->diagflag & D_VERBOSE) {
                   printf("Product found :%s\n", usb_tmp.mfg[1]);
               }
           }
           if ((ptr = usb_get_str(ptr, line, "Serial Number"))) {
               sprintf(usb_tmp.mfg[2], "%s", ptr);
               if ((NVRAM)->diagflag & D_VERBOSE) {
                   printf("SN found :%s\n", usb_tmp.mfg[2]);
               }
               if (usb_match(usb_ptr, &usb_tmp)) {
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

/*******************************************************************************
 *
 * Function   :    usb_find_dev_name
 * Description:    search file system for the device
 * Inputs     :    dev_idx - Device number
 *                 dev_name - Device name, such as sda
 * Outputs    :    PASSED/FAILED
 *
 *******************************************************************************
 */
static int usb_find_dev_name (int dev_idx, char *dev_name)
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
 * Function   :    usb_get_str
 * Description:    get usb string from proc file.
 * Inputs     :    line, entire line from file, key, key string to look for
 * Outputs    :    ptr, string value to be saved. return NULL if failed to
 *                 find key in string.
 *
 *******************************************************************************
 */
static char *usb_get_str (char *ptr, char *line, char *key)
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
 * Function   :    usb_match
 * Description:    find match withbetween 2 usb devices that have same str value.
 * Inputs     :    usb_ptr, fist usb device; tmp, second usb devicex
 * Outputs    :    1, if match occurs; 0, otherwise
 *
 *******************************************************************************
 */
static int usb_match (struct usb_info_t *usb_ptr, struct usb_info_t *tmp)
{

    /*
     * need to match all 3 fiels: name, prodId, and serial number
     */
    if (strcmp(usb_ptr->mfg[0], tmp->mfg[0]) == 0 &&
        strcmp(usb_ptr->mfg[1], tmp->mfg[1]) == 0 &&
        strcmp(usb_ptr->mfg[2], tmp->mfg[2]) == 0) {
        return (1);
    }

    return (0);
}

/*******************************************************************************
 *
 * Function   :    find_storage_index
 * Description:    find USB storage index
 * Inputs     :    speed - USB 2.0 or USB 3.0
 *                 index_no - USB storage array index number
 * Outputs    :    TRUE, if match occurs; FALSE, otherwise
 *
 *******************************************************************************
 */
int find_usb_storage_index (int speed, int *index_no)
{
    int ix, jx, ret = FALSE;

    for (ix = 0; ix < USB_DEVICE_MAX_NUM; ix++) {
        if (speed == USB_HOST20_SPEED) {
            for (jx = 0; jx < USB_DEVICE_MAX_NUM; jx++) {
                if ((NVRAM)->diagflag & D_VERBOSE) {
                    printf("\nusb2_storage[%d].bus: %#x, lev: %#x, prnt: %#x, "
                            "port: %#x, dev: %#x, spd: %d, found: %d\n", 
                            jx, usb2_storage[jx].bus, usb2_storage[jx].lev, 
                            usb2_storage[jx].prnt, usb2_storage[jx].port,
                            usb2_storage[jx].dev, usb2_storage[jx].spd, 
                            usb2_storage[jx].found);
                    printf("usb[%d].bus: %#x, lev: %#x, prnt: %#x, port: %#x, dev: %#x, spd: %d\n", 
                            ix, usb[ix].bus, usb[ix].lev, usb[ix].prnt, usb[ix].port,
                            usb[ix].dev, usb[ix].spd);
                }
                if ((usb[ix].bus == usb2_storage[jx].bus) &&
                    (usb[ix].lev == usb2_storage[jx].lev) &&
                    (usb[ix].prnt == usb2_storage[jx].prnt) &&
                    (usb[ix].port == usb2_storage[jx].port) &&
                    (usb[ix].spd == usb2_storage[jx].spd)) {
                    *index_no = ix;
                    ret = TRUE;
                    break;
                }
            }
        } else {
            for (jx = 0; jx < USB_DEVICE_MAX_NUM; jx++) {
                if ((usb[ix].bus == usb3_storage[jx].bus) &&
                    (usb[ix].lev == usb3_storage[jx].lev) &&
                    (usb[ix].prnt == usb3_storage[jx].prnt) &&
                    (usb[ix].port == usb3_storage[jx].port) &&
                    (usb[ix].spd == usb3_storage[jx].spd)) {
                    *index_no = ix;
                    ret = TRUE;
                    break;
                }
            }
        }
        if (ret == TRUE) {
            break;
        }
    }

    return (ret);
}

/******** History ********
$Log: diag_usb_lib.c,v $
Revision 1.4  2019/07/11 12:31:30  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
