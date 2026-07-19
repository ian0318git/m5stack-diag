/* $Id: platform_storage.c,v 1.6 2018/06/05 09:54:08 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_storage.c,v $
 *------------------------------------------------------------------
 *
 * Filename: platform_storage.c
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
#include "types.h"
#include "common.h"
#include "monitor.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "linux_usb_test.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "platform_fpga.h"

#define MAX_FILENAME_LENGTH     255
#define MAX_COMMAND_LENGTH      2048

/*
 * Global extern functions
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);

/*
 * Global variables
 */
extern int quiet_launch;

/*
 * Declare local function
 */
static void usb_display(int idx);
static int usb_dump(int n);
static int get_storage_size(char*, int, char*);
static int get_emmc_size(char*, int, char*);
int usb_get_speed(int slot);

static struct usb_info_t usb[3];
static submenu_xtable_t usbdev_menu_table[] = {
    {"enumerate external USB port", (PFT) usb_dump, USB_SLOT0,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"enumerate USB port0 (LTE 0)", (PFT) usb_dump, USB_SLOT1,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"enumerate USB port1 (LTE 1)", (PFT) usb_dump, USB_SLOT2,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())this_is_star_c1109_4p, 0, (type_t(*)())0, 0},
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
 * Function   :    usb_utils
 * Description:    show usb utility subment
 * Inputs     :    show_menu, flag set if submenu is selected
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int usb_utils (int show_menu)
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

    if( (ExecuteCmdbyPopen (cmd, sysfilesize, bufsize)) == 0 ) {
        return FAILED;
    }

    return PASSED;
}

/*******************************************************************************
 *
 * Function   :    get_emmc_size
 * Description:    get emmc size
 * Inputs     :    buffer to return size, buffer size & device name 
 * Outputs    :    PASSED/FAILED
 *
 *******************************************************************************
 */
static int get_emmc_size (char* sysfilesize, int bufsize, char* dev_name)
{
    char cmd[MAX_COMMAND_LENGTH]={0};

    sprintf(cmd, "fdisk -l 2>/dev/null | grep Disk | grep -i -w %s | awk '{print $5}'", dev_name);
    
    if( (ExecuteCmdbyPopen (cmd, sysfilesize, bufsize)) == 0 ) {
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
int access_device_test (char *src)
{
    char buf[128], buf_bk[512], buf_wr[512], buf_rd[512];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;

    if(!quiet_launch) {
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
    if(!quiet_launch) {
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
    if(!quiet_launch) {
        prpass(testpass, "Prepare data pattern , ");
    }
    for (cnt = 0; cnt < sizeof(buf_wr); cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }

    /*
     * write data pattern
     */
    if(!quiet_launch) {
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
    if(!quiet_launch) {
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
    if(!quiet_launch) {
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
    if(!quiet_launch) {
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

static boolean emmc_force_stop = FALSE;
/*******************************************************************************
 *
 * Function   :    access_emmc_test
 * Description:    main test for emmc test
 * Inputs     :    file path to emmc
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int access_emmc_test (int full_test)
{
    char buf[EMMC_TEST_PATTERN_SIZE], buf_bk[EMMC_TEST_BUFFER_SIZE], buf_wr[EMMC_TEST_BUFFER_SIZE], buf_rd[EMMC_TEST_BUFFER_SIZE];
    int buf_bk_len = 0, buf_wr_len = 0, buf_rd_len = 0;
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;
    unsigned long pos = 0, size = EMMC_TEST_BUFFER_SIZE;
    char filesize[64]={0};
    int loop_cnt = 0, loop_max = 0;

	buf_bk_len = sizeof(buf_bk);
	buf_wr_len = sizeof(buf_wr);
	buf_rd_len = sizeof(buf_rd);
	
    memset(buf_bk, 0, buf_bk_len);
    memset(buf_wr, 0, buf_wr_len);
    memset(buf_rd, 0, buf_rd_len);
    
    if (full_test) {
		if (((get_emmc_size (filesize, sizeof(filesize), EMMC_BLK))== FAILED)) {
			cterr('f',0,"Failed to get storage size.");
			return (FAILED);
		}
		if (sscanf(filesize, "%lu\n", &size) != 1) {
			cterr('f',0,"Failed to get storage size.");
			return (FAILED);
		}
		loop_max = (size-buf_bk_len)/buf_bk_len;
		printf("test size %lu bytes\n", size);
	}	

    if(!quiet_launch && !full_test) {
        prpass(testpass, "Access device '%s' , ", EMMC_BLK);
    }
    sprintf(buf, "%s", EMMC_BLK);

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
     * prepare data pattern
     */
    if(!quiet_launch && !full_test) {
        prpass(testpass, "Prepare data pattern , ");
    }
    for (cnt = 0; cnt < buf_wr_len; cnt++) {
        buf_wr[cnt] = PATTERN + cnt;
    }
	
	prpass(testpass, "test position ");
	while (loop_cnt <= loop_max && emmc_force_stop == FALSE) {
	
		if (loop_cnt % 9 == 0)
			prpass(testpass, "%lu ", pos);
		
		/*
		 * back up data
		 */
		if(!quiet_launch && !full_test) {
			prpass(testpass, "Backup data , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			printf
				("backup lseek failed; Cannot point to the beginning of device.");
			return (FAILED);
		}
		if ((num = read(devfd, buf_bk, buf_bk_len)) == -1) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "Read data from device failed");
			printf("Unable to read from drive.\n");
			return (FAILED);
		}

		/*
		 * write data pattern
		 */
		if(!quiet_launch && !full_test) {
			prpass(testpass, "Write data pattern , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			printf
				("write lseek failed; Cannot point to the beginning of device.");
			return (FAILED);
		}

		if ((num = write(devfd, buf_wr, buf_wr_len)) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0,
				  "Write test pattern failed, can not write to drive.");
			printf("Unable to write data pattern to device.");
			return (FAILED);
		}
		if (num != buf_bk_len) {
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
		if(!quiet_launch && !full_test) {
			prpass(testpass, "Read back data for comparing , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			printf("lseek failed; Cannot point to the beginning of device.");
			return (FAILED);
		}

		if ((num = read(devfd, buf_rd, buf_rd_len)) == -1) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "Read back data from device failed");
			printf("Unable to read from drive.\n");
			return (FAILED);
		}
		if (num != buf_rd_len) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "not all the bytes are read for data pattern");
			return (FAILED);
		}

		/*
		 * comparing data
		 */
		if(!quiet_launch && !full_test) {
			prpass(testpass, "Comparing data , ");
		}
		p1 = buf_wr;
        p2 = buf_rd;
		for (ib = 0; ib < buf_rd_len; ib++, p1++, p2++) {
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
		if(!quiet_launch && !full_test) {
			prpass(testpass, "Restore data , ");
		}
		if (lseek(devfd, pos, SEEK_SET) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "lseek to the beginning of device failed.");
			return (FAILED);
		}

		if ((num = write(devfd, buf_bk, buf_bk_len)) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0,
				  "Write restore data failed, can not write to drive.\n");
			return (FAILED);
		}

		if (num != buf_bk_len) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "not all the bytes are written for restore");
			return (FAILED);
		}

		if (fsync(devfd) < 0) {
			close(devfd);           /* don't need it anymore */
			cterr('f', 0, "fsync failed.");
			return (FAILED);
		}
        
        pos += buf_bk_len;
        loop_cnt++;
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
    char *tname = "";

    if (strcmp(usb[slot].dev_name, "") == 0) { /* don't find the devname ex./dev/sda */
        cterr('f',0,"Failed to find USB storage.");
        return (FAILED);
    }

    if (usb[slot].spd == 5000 ) {
        sprintf(usb_spd, "%s", "[USB 3.0] ");
    } else {
        sprintf(usb_spd, "%s", "[USB 2.0] ");
    }

    if( slot == USB_SLOT0 ) {
        tname = "External USB";
    } else if ( slot == USB_SLOT1 ) {
        tname = "Internal USB (LTE 0)";
    } else if ( slot == USB_SLOT2 ) {
        tname = "Internal USB (LTE 1)";
    } else {
        cterr('f',0,"Unsupported Slot!");
        return (FAILED);
    }

    sprintf(src, "/dev/%s", usb[slot].dev_name);
    retval = access_device_test(src);

    if(!quiet_launch) {
        prpass(testpass, "%s%s, ", usb_spd, tname);
    }
    return (retval);
}

/*******************************************************************************
 *
 * Function   :    eusb_slot_tests
 * Description:    main test for eUSB test.
 * Inputs     : dummy
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int eusb_slot_tests (int dummy)
{
    char src[32];
    int retval;

    sprintf(src, "/dev/eUSB");

    retval = access_device_test(src);
    return (retval);
}

/***************************************************************************** *
 * Function   : usb_dump
 * Description: main entry to display usb info.
 * Inputs     : slot - usb slot
 * Outputs    : PASSED
 *
 ******************************************************************************/
static int usb_dump (int slot)
{
    if (usb_get_info() == FAILED) {
        cterr('f', 0, "usb_get_info() failed");
        return (FAILED);
    }
    usb_display(slot);
    return (PASSED);
}

/***************************************************************************** *
 * Function   : usb_dump_x
 * Description: extend function to display usb info.
 * Inputs     : slot - usb slot
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int usb_dump_x (int slot)
{
    return (usb_dump(slot));
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

/*******************************************************************************
 *
 * Function   : emmc_slot_tests
 * Description: main test for emmc test.
 * Inputs     : option for future use
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int emmc_slot_tests (int option)
{
    int retval;

	emmc_force_stop = FALSE;

    retval = access_emmc_test(0);
    return (retval);
}

/*******************************************************************************
 *
 * Function   : emmc_full_test
 * Description: main test for emmc full test.
 * Inputs     : option for future use
 * Outputs    : N/A
 *
 *******************************************************************************
 */
int emmc_full_test(int option)
{
    int retval = 1;
    pthread_t emmc_thread;
    void *ret;
    int flag = 1;
	
    emmc_force_stop = FALSE;
    if(pthread_create(&emmc_thread, NULL, (void *)access_emmc_test, (void *)&flag)) {
        cterr('f',0, "pthread_create failed");
        return (FAILED);
    }
    
    while(getc_answer("It may take several hours to test full eMMC, you can stop at any time.\nDo you want to stop eMMC test? Please input y to confirm.", "yn", 'n') != 'y');

    emmc_force_stop = TRUE;
    pthread_join(emmc_thread, &ret);

    return (retval);
}

/*******************************************************************************
 *
 * Function   :    spi_slot_tests
 * Description:    main test for spi test.
 * Inputs     :    option for future use
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int spi_slot_tests (int option)
{
    char src[32];
    int retval;

    sprintf(src, "/dev/mtdblock2");

    retval = access_device_test(src);
    return (retval);
}

/*************************below left for ref ******************/
/* we assume that udev rule assignes /dev/usb0x to the device name of usb slot 0 (ie, /dev/sdb, etc..)
   if we change the name in the udev rule, then we need to change the nanme in this code as well.
*/
/******** History ********
$Log: platform_storage.c,v $
Revision 1.6  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.5  2018/03/27 12:46:38  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4.2.1  2018/03/07 03:31:42  hondwang
Fix TSN wifi ACT2 program utility issue

Revision 1.4  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3  2018/01/23 11:38:19  steja
Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)

Revision 1.2.20.2  2018/02/07 10:23:01  lucywang
Followed coding rule

Revision 1.2.20.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.18.1  2018/01/18 13:13:51  steja
Add LTE USB 2.0 Detection Test

Revision 1.2.4.4  2017/11/22 01:12:12  lucywang
Added the option to stop full eMMC test gracefully

Revision 1.2.4.3  2017/11/20 07:54:32  lucywang
Changed PID to C1101/C1109-2P/C1109-4P

Revision 1.2.4.2  2017/10/20 07:55:27  lucywang
Added utility to test full eMMC

Revision 1.2.4.1  2017/09/09 00:47:48  hondwang
Add C949-4P support with MB,Wifi,LTE EM

Revision 1.2  2017/08/02 14:21:49  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/25 08:31:56  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.3.2.1  2017/06/12 11:23:20  steja
Enhanced LTE mini-usb test

Revision 1.1.4.3  2017/02/10 15:10:44  petteng
Modify USB 3.0 & 2.0 test

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.7  2016/04/25 07:40:57  steja
1. Enable LTE and USB enumerate
2. Enable MODEM detect

Revision 1.1.2.6  2016/04/22 11:34:00  steja
check-in for first release

Revision 1.1.2.5  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.4  2016/03/24 08:14:12  steja
Add Modem Enumeration

Revision 1.1.2.3  2016/03/16 13:36:56  steja
Add bootflash test, need FPGA function

Revision 1.1.2.2  2016/03/16 10:24:50  steja
Add EMMC test

Revision 1.1.2.1  2016/03/16 08:57:54  steja
add usb test


$Endlog$
*/
