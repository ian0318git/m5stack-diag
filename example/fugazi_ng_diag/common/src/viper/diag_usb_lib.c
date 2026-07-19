 /* $Id: diag_usb_lib.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_usb_lib.c,v $
 *------------------------------------------------------------------
 *
 * Filename: diag_usb_lib.c
 *
 * Copyright (c) 2013-2018 by cisco Systems, Inc.
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
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

/*
 * Declare local function
 */

int usb_get_info(void);
int usb_get_speed(int);
void usb_display(int);
static int usb_get_bus_no(struct usb_info_t *);
static int get_storage_size(char*, int, char*);
static int usb_find_dev_name(int);
static char *usb_get_str(char *, char *, char *);
static int usb_match(struct usb_info_t *, struct usb_info_t *);
struct usb_info_t usb[4];


/*******************************************************************************
 *
 * Function   :    usb_get_info
 * Description:    search proc file for usb devices and save info into usb struct.
 * Inputs     :    NONE
 * Outputs    : errno, if failed. 0 otherwise.
 *
 *******************************************************************************
 */
int usb_get_info (void)
{
    struct usb_info_t usb_tmp;
    FILE *fp;
    char line[82], tmp[32], *ptr, *ptr2;
    int str_idx = 0, ix = 0;

    fp = fopen(USB_DEVICE_FILE, "r");

    if (fp) {
        /* file exists */
        fclose(fp);
    } else {
        /* if file doesn't exist, try to mount it */
        system(MOUNT_DEBUGFS);
    }

    fp = fopen(USB_DEVICE_FILE, "r");
    /* sys/kernel/debug/usb/devies */

    if (!fp) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s not found", USB_DEVICE_FILE);
        }
        return (FAILED);
    }

    memset(usb, 0, sizeof(usb));

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
                usb_tmp.found = 1;
                if ( usb_tmp.bus == 1 ) {
                    ix = 0; /* bus 1: internal usb port (2.0) */
                } else if (usb_tmp.bus == 2 ) {
                    ix = 1; /* bus 2: internal usb port (3.0) */
                } 
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
                if (usb_get_bus_no(&usb[ix])) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****BUS FOUND**** no = %d\n",
                               usb[ix].bus_no);
                    }
                }
                if (usb_find_dev_name(ix) != FAILED) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****dev found**** dev = %s\n",
                               usb[ix].dev_name);
                    }
                }
            }
        }
        if (line[0] == 'I') {
            /* port 0 is LTE debug port*/
            if ((ptr2 = strstr(ptr, "Driver=sierra")) && usb_tmp.port == 0) {
                usb_tmp.found = 1;
                if ( usb_tmp.bus == 1 || usb_tmp.bus == 2) {
                    ix = 2; /* bus 1, 2: internal usb port (lte0) */
                }     
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            }
            else  if ((ptr2 = strstr(ptr, "Driver=sierra")) && usb_tmp.lev !=1) {
                usb_tmp.found = 1;
                if ( usb_tmp.bus == 1 || usb_tmp.bus == 2) {
                    ix = 2; /* bus 1, 2: internal usb port (lte0) */
                }
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            } else  if ((ptr2 = strstr(ptr, "Driver=sierra")) && usb_tmp.lev ==1 && usb_tmp.prnt == 1 &&usb_tmp.port == 1) {
                if ( usb_tmp.bus == 1 || usb_tmp.bus == 2) {
                    ix = 3; /* bus 1, 2: internal usb port (lte0) */
                }
                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            }
        }
    } /* while */

    fclose(fp);

    if (usb[0].found == 0 && usb[1].found == 0 &&usb[2].found == 0 &&usb[3].found == 0) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    usb_get_speed
 * Description:    print out usb speed info
 * Inputs     :    idx, index into the device number
 * Outputs    :    NONE
 *
 *******************************************************************************
 */
int usb_get_speed (int slot)
{
    if (strcmp(usb[slot].dev_name, "") == 0) { /* don't find the devname ex./dev/sda */
        cterr('f',0,"Failed to find USB storage.");
        return -1;
    }

    if (usb[slot].spd == 5000 ) {
        return 3;
    } else {
        return 2;
    }
}

/*******************************************************************************
 *
 * Function   :    usb_display
 * Description:    print out usb info
 * Inputs     :    idx, index into the device number
 * Outputs    :    NONE
 *
 *******************************************************************************
 */
void usb_display (int idx)
{
    char sysfilesize[MAX_FILENAME_LENGTH]={0};

    if (strcmp(usb[idx].mfg[USB_MFG], "") == 0) {
        cterr('f',0,"Failed to find device.");
        return;
    }

    printf
        ("-----------------------------------------------------------------\n");

    /* don't find the devname ex./dev/sda */
    if (strcmp(usb[idx].dev_name, "") == 0) {
        printf(" Device Type: %s \n", "LTE");
        printf(" Vendor: %s \n Product Name: %s \n"
               " Serial#: %s \n Speed: %d Mbps\n ",
        usb[idx].mfg[USB_MFG], usb[idx].mfg[USB_PROD],
        usb[idx].mfg[USB_SER], usb[idx].spd);
    }
    else {
        if (((get_storage_size (sysfilesize, MAX_FILENAME_LENGTH,
            usb[idx].dev_name))== FAILED)) {
            cterr('f',0,"Failed to get storage size.");
            return;
        }

        printf(" Device Type: %s \n", "Usb storage");
        printf(" Vendor: %s \n Product Name: %s \n"
               " Serial#: %s \n Speed: %d Mbps\n"
               " Device Name: /dev/%s\n"
               " Disk Size: %s \n ",
        usb[idx].mfg[USB_MFG], usb[idx].mfg[USB_PROD],
        usb[idx].mfg[USB_SER], usb[idx].spd,
        usb[idx].dev_name, sysfilesize);
    }

    printf
        ("-----------------------------------------------------------------\n");
    return;
}
/*******************************************************************************
 *
 * Function   :    usb_get_bus_no
 * Description:    get usb number given usb_ptr struct.
 * Inputs     :    usb_ptr, usb structure
 * Outputs    :    bus number, if sucessful; linux error code otherwise.
 *
 *******************************************************************************
 */
static int usb_get_bus_no (struct usb_info_t *usb_ptr)
{
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
                usb_tmp.bus_no = atoi(dp->d_name);
                continue;       /* go to the next line */
            }
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("d_name :%s\n", dp->d_name);
                printf("Bus_no :%d\n", usb_tmp.bus_no);
            }

            if (usb_tmp.bus_no) {
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
                        usb_ptr->bus_no = usb_tmp.bus_no;
                        break;
                    }
                }               /* if bus.no */
            }
        }                       /* end while scanning file  */
        fclose(fp);
    }                           /* end while scanning directory */
    closedir(dir);
    return (usb_tmp.bus_no);
}

/*******************************************************************************
 *
 * Function   :    get_storage_size
 * Description:    print out usb disk size
 * Inputs     :    cmd size & device name
 * Outputs    :    disk size
 *
 *******************************************************************************
 */
static int get_storage_size (char* sysfilesize, int bufsize, char* dev_name)
{
    char cmd[MAX_COMMAND_LENGTH]={0};

    sprintf(cmd, "fdisk -l 2>/dev/null | grep Disk | grep /dev/%s | cut -d : -f 2", dev_name);

    if( (ExecuteCmdbyPopen (cmd, sysfilesize, bufsize)) == 0 ) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :    usb_find_dev_name
 * Description:    search file system for the device
 * Inputs     :    idx, index into the device number
 * Outputs    : linux error status, errno; 0, otherwise.
 *
 *******************************************************************************
 */
static int usb_find_dev_name (int idx)
{
    DIR *dir;
    struct dirent *dp;
    char name[82];

    sprintf(name, "%s/%d:0:0:0/block", USB_SCSI_DEVICE_FILE,
            usb[idx].bus_no);

    if ((dir = opendir(name)) == NULL) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            cterr('f', 0, "Cannot open directory /sys/blockbvblock/ .");
        }
        return (FAILED);
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        sprintf(usb[idx].dev_name, "%s", dp->d_name);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf(" at /dev/%s\n", usb[idx].dev_name);
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
 * Outputs    : ptr, string value to be saved. return NULL if failed to
 *              find key in string.
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

/******** History ********
$Log: diag_usb_lib.c,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.6  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.5  2018/05/04 07:25:19  lucywang
Show message when FPGA MUX switch failed in LTE debug port test

Revision 1.1.2.4  2018/05/03 07:42:13  lucywang
Added to support test with USB hub

Revision 1.1.2.3  2018/04/10 06:20:34  harrchan
Modify usb bus mapping in function usb_get_info

Revision 1.1.2.2  2018/03/27 07:12:10  lucywang
Modified USB test for 2.0 and 3.0

Revision 1.1.2.1  2018/02/27 08:06:47  harrchan
Initial viper application code base



$Endlog$
*/



