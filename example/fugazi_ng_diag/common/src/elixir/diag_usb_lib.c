/* $Id: diag_usb_lib.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_usb_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_usb_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
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
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "diag_moka_fpga_lib.h"
#include "diag_usb_lib.h"


/*
 * Declare local function
 */
static int access_device_test(char *src);
static void usb_display(int idx);
static int diag_usb_dump_util(int n);
static int get_storage_size(char*, int, char*);

static struct usb_info_t usb[3];
static submenu_xtable_t usbdev_menu_table[] = {
    {"enumerate external USB port", (PFT) diag_usb_dump_util, USB_SLOT0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define USB_MENU_TABLE_SZ \
        (sizeof(usbdev_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t usbdev_pri_items[USB_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t usbdev_sec_items[USB_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static struct menuinfo usbdev_menu = {
    "USB Main Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) menu_show_dflags,     /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    usbdev_pri_items,
};

static struct menuinfo *usb_menup = &usbdev_menu;

/*******************************************************************************
 *
 * Function   :    diag_usb_util
 * Description:    show usb utility subment
 * Inputs     :    show_menu, flag set if submenu is selected
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int diag_usb_util (int show_menu)
{
    char *tname = "Enumerate all USB";

    testname(tname);

    build_primary_submenu(usbdev_menu_table, USB_MENU_TABLE_SZ,
                          "USB", &usb_menup);
    build_secondary_submenu(usbdev_menu_table, USB_MENU_TABLE_SZ,
                          usbdev_sec_items);

    if (show_menu) {
        menu(&usbdev_menu, usbdev_sec_items, '\0');
    } else {
        exec_doall_menu_items(&usbdev_menu);
    }

    return (PASSED);
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

    if ( (ExecuteCmdbyPopen (cmd, sysfilesize, bufsize)) == 0 ) {
        return FAILED;
    }

    return PASSED;
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
 * Function   :    usb_match
 * Description:    find match withbetween 2 usb devices that have same str value.
 * Inputs     :    usb_ptr, fist usb device; tmp, second usb devicex
 * Outputs    : 1, if match occurs; 0, otherwise
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
        return 1;
    }

    return 0;
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
 * Function   :    usb_get_bus_no
 * Description:    get usb number given usb_ptr struct.
 * Inputs     :    usb_ptr, usb structure
 * Outputs    :    bus number, if sucessful; linux error code otherwise.
 *
 *******************************************************************************
 */
int usb_get_bus_no (struct usb_info_t *usb_ptr)
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
        if (dp->d_name[0] == '.')
            continue;

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
                if ( usb_tmp.bus == 1 || usb_tmp.bus == 2) {
                    ix = 0; /* bus 1, 2: internal usb port (lte0) */
                } else if (usb_tmp.bus == 3 || usb_tmp.bus == 4) {
                    if (usb_tmp.port == 0) {
                        ix = 1; /* bus 3, 4: external usb port with slot 1 */
                    } else {
                        ix = 2; /* bus 3, 4: external usb port with slot 2*/
                    }
                } else {
                    ix = 2; /* bus 5, 6: internal usb port (lte1) */
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
            if ((ptr2 = strstr(ptr, "Driver=sierra"))) {
                usb_tmp.found = 1;
                if ( usb_tmp.bus == 1 || usb_tmp.bus == 2) {
                    ix = 0; /* bus 1, 2: internal usb port (lte0) */
                } else if (usb_tmp.bus == 3 || usb_tmp.bus == 4) {
                    if (usb_tmp.port == 0) {
                        ix = 1; /* bus 3, 4: external usb port with slot 1 */
                    } else {
                        ix = 2; /* bus 3, 4: external usb port with slot 2*/
                    }
                } else {
                    ix = 2; /* bus 5, 6: internal usb port (lte1) */
                }

                memcpy(&usb[ix], &usb_tmp, sizeof(usb_tmp));
            }
        }
    }                           /* while */

    fclose(fp);

    if (usb[0].found == 0 && usb[1].found == 0 &&usb[2].found == 0) {
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :    access_device_test
 * Description:    main test for usb device test
 * Inputs     :    file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
static int access_device_test (char *src)
{
    char buf[128], buf_bk[512], buf_wr[512], buf_rd[512];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;

    if (!quiet_launch) {
        prpass(testpass, "Access device '%s' , ", src);
    }
    sprintf(buf, "%s", src);

    memset(buf_bk, 0, sizeof(buf_bk));
    memset(buf_wr, 0, sizeof(buf_wr));
    memset(buf_rd, 0, sizeof(buf_rd));

    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR);
        if (devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }

    }
    if (devfd < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "there is no device file descriptor available.");
        return (FAILED);
    }

    /*
     * back up data
     */
    if (!quiet_launch) {
        prpass(testpass, "Backup data , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf
            ("backup lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }
    if ((num = read(devfd, buf_bk, sizeof(buf_bk))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }

    /*
     * prepare data pattern
     */
    if (!quiet_launch) {
        prpass(testpass, "Prepare data pattern , ");
    }
    for (cnt = 0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }

    /*
     * write data pattern
     */
    if (!quiet_launch) {
        prpass(testpass, "Write data pattern , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf
            ("write lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_wr, sizeof(buf_wr))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write test pattern failed, can not write to drive.");
        printf("Unable to write data pattern to device.");
        return (FAILED);
    }
    if (num != sizeof(buf_bk)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for data pattern");
        return (FAILED);
    }

    if (fsync(devfd) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "fsync failed.");
        printf("Unable to sync data pattern to device.");
        return (FAILED);
    }

    /*
     * read back data for comparing
     */
    if (!quiet_launch) {
        prpass(testpass, "Read back data for comparing , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        return (FAILED);
    }

    if ((num = read(devfd, buf_rd, sizeof(buf_rd))) == -1) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "Read back data from device failed");
        printf("Unable to read from drive.\n");
        return (FAILED);
    }
    if (num != sizeof(buf_rd)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are read for data pattern");
        return (FAILED);
    }

    /*
     * comparing data
     */
    if (!quiet_launch) {
        prpass(testpass, "Comparing data , ");
    }
    cnt = 0;
    for (ib = 0; ib < sizeof(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",
                   (ib + 1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    /*
     * restore data
     */
    if (!quiet_launch) {
        prpass(testpass, "Restore data , ");
    }
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "lseek to the beginning of device failed.");
        return (FAILED);
    }

    if ((num = write(devfd, buf_bk, sizeof(buf_bk))) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0,
              "Write restore data failed, can not write to drive.\n");
        return (FAILED);
    }

    if (num != sizeof(buf_bk)) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "not all the bytes are written for restore");
        return (FAILED);
    }

    if (fsync(devfd) < 0) {
        close(devfd);           /* don't need it anymore */
        cterr('f', 0, "fsync failed.");
        return (FAILED);
    }

    close(devfd);               /* don't need it anymore */
    return (PASSED);

}

/*******************************************************************************
 *
 * Function   :    usb_slot_tests
 * Description:    entry point to usb device test
 * Inputs     :    usb slot
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int usb_slot_tests (int slot)
{
    char src[32], usb_spd[15];
    int retval;
    char *tname = "External USB";

    if (strcmp(usb[slot].dev_name, "") == 0) { /* don't find the devname ex./dev/sda */
        cterr('f',0,"Failed to find USB storage.");
        return (FAILED);
    }

    if (usb[slot].spd == 5000 ) {
        sprintf(usb_spd, "%s", "[USB 3.0] ");
    } else {
        sprintf(usb_spd, "%s", "[USB 2.0] ");
    }

    sprintf(src, "/dev/%s", usb[slot].dev_name);
    retval = access_device_test(src);

    if (!quiet_launch) {
        prpass(testpass, "%s%s, ", usb_spd, tname);
    }
    return (retval);
}

/***************************************************************************** *
 * Function   : diag_usb_dump_util
 * Description: main entry to display usb info.
 * Inputs     : slot - usb slot
 * Outputs    : PASSED
 *
 ******************************************************************************/
static int diag_usb_dump_util (int slot)
{
    if (usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }
    usb_display(slot);
    return (PASSED);
}

/***************************************************************************** *
 * Function   : usb_debugport_test
 * Description: extend function to check usb lte info in from debug port.
 * Inputs     : slot - usb slot
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int usb_debugport_test (int slot)
{
    if (usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }
    usb_display(slot);

    if (strstr(usb[slot].mfg[USB_MFG], "Sierra") == 0) {
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
    if (strcmp(usb[slot].mfg[USB_MFG], "") == 0) { /* don't find the devname ex./dev/sda */
        cterr('f',0,"Failed to find USB device.");
        return -1;
    }

    if (usb[slot].spd == 5000 ) {
        return 3;
    } else {
        return 2;
    }
}

/*-------------------------------------------------
 * $Log: diag_usb_lib.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2020/09/09 09:08:07  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
