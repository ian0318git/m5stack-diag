/* $Id: linux_usb_test.c,v 1.28 2020/01/09 01:01:51 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_usb_test.c,v $
 *------------------------------------------------------------------
 * 
 * Filename: linux_usb_test.c
 *
 * Copyright (c) 2016-2019 by cisco Systems, Inc.
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
#include <malloc.h>

#define TEST_SIZE  1000

static void usb_display(int idx);
static int usb_dump(int n);
static int show_usb_info(void); 

static struct usb_info_t usb[3];

/* 
 * USB info read from C lib
 */
static  submenu_xtable_t usb_menu_table[] = {
    { "Show USB info",    (PFT)show_usb_info,    0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define USB_MENU_TABLE_SZ \
        (sizeof(usb_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static  mitem_t usb_pri_items[USB_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static  mitem_t usb_sec_items[USB_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static struct menuinfo usb_menu = {
    "USB Main Menu",
    0,                  /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,  /* notes missing WICs in combos */
    0,                  /* use generic prompt */
    0,                  /* size (bumped by add_menu_item() */
    usb_pri_items,
};
static struct menuinfo *usb_menup = &usb_menu;


/* 
 * Old legacy USB info read, it is expired on kernel 3.19 and later 
 */

static  submenu_xtable_t        cf_menu_table[] = {
    { "Display USB",    (PFT)usb_dump,    0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define CF_MENU_TABLE_SZ \
        (sizeof(cf_menu_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static  mitem_t cf_pri_items[CF_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static  mitem_t cf_sec_items[CF_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static struct menuinfo    cf_menu = {
    "USB Main Menu",
    0,                  /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,  /* notes missing WICs in combos */
    0,                  /* use generic prompt */
    0,                  /* size (bumped by add_menu_item() */
    cf_pri_items,
};
static struct menuinfo *cf_menup = &cf_menu;

/*************************************************************************
 *
 * Function   : usb_utils_v2
 * Description: show usb utility submenu 
 * Inputs     : show_menu, flag set if submenu is selected
 * Outputs    : PASSED
 *
 *************************************************************************
 */
int
usb_utils_v2 (void)
{
    build_primary_submenu(usb_menu_table, USB_MENU_TABLE_SZ, "USB",
                              &usb_menup);
    build_secondary_submenu(usb_menu_table, USB_MENU_TABLE_SZ, usb_sec_items);
    menu(&usb_menu, usb_sec_items, '\0');

    return(PASSED);
}

/*******************************************************************************
 *
 * Function   :	usb_utils
 * Description:	show usb utility subment
 * Inputs     :	show_menu, flag set if submenu is selected
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int
usb_utils (int show_menu)
{
    /*
     * Set up CF for faster timings:
     * MWDMA mode for Cavium CPU
     * PIO mode 6 for Freescale CPU
     */
    if (show_menu) {
        build_primary_submenu(cf_menu_table, CF_MENU_TABLE_SZ, "CompactFlash",
                              &cf_menup);
        build_secondary_submenu(cf_menu_table, CF_MENU_TABLE_SZ, cf_sec_items);
        menu(&cf_menu, cf_sec_items, '\0');
    } 

    return(PASSED);
}


/***********************************************************************
 *
 * Function   : show_usb_info
 * Description: print out usb info
 * Inputs     : NONE
 * Outputs    : NONE
 *
 ***********************************************************************
 */
int show_usb_info (void)
{
    char buf[256];

    /* using 'lsusb' to get info, since old method is obsoleted on new kernel */
    sprintf(buf, "%s '%s' %s",
            "lsusb -v | grep -E",
            "\\<(Bus|idVendor|idProduct|iProduct|iSerial)",
            "| grep -v 'Powered' 2>/dev/null");

    system(buf); 
    return 0; 
}

/*******************************************************************************
 *
 * Function   :	usb_display
 * Description:	print out usb info
 * Inputs     :	idx, index into the device number
 * Outputs    : NONE
 *
 *******************************************************************************
 */
void
usb_display (int idx)
{
    printf("Vendor: %s; Product Name: %s; \n"
           "Serial#: %s; USB%d: %d:0:0:0; /dev/%s\n",
           usb[idx].mfg[USB_MFG], usb[idx].mfg[USB_PROD], usb[idx].mfg[USB_SER],
           usb[idx].port, usb[idx].bus_no, usb[idx].dev_name);
    printf("-----------------------------------------------------------------\n");
    return;
}


/*******************************************************************************
 *
 * Function   :	usb_find_dev_name
 * Description:	search file system for the deivce
 * Inputs     :	idx, index into the device number
 * Outputs    : linux error status, errno; 0, otherwise.
 *
 *******************************************************************************
 */
static int
usb_find_dev_name (int idx)
{
    DIR *dir;
    struct dirent *dp;
    char name[82];

    sprintf(name, "%s/%d:0:0:0/block", USB_SCSI_DEVICE_FILE, usb[idx].bus_no);
    
    if ((dir = opendir(name)) == NULL) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            perror("cannot open directory /sys/blockbvblock/ .");
        }
        return errno;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        sprintf(usb[idx].dev_name, "%s", dp->d_name);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf(" at /dev/%s\n", dp->d_name);
        }
    }
    closedir(dir);

    return 0;
}

/*******************************************************************************
 *
 * Function   :	usb_match
 * Description:	find match withbetween 2 usb devices that have same str value.
 * Inputs     :	usb_ptr, fist usb device; tmp, second usb devicex
 * Outputs    : 1, if match occurs; 0, otherwise
 *
 *******************************************************************************
 */
static int
usb_match (struct usb_info_t *usb_ptr, struct usb_info_t *tmp)
{

        /* need to match all 3 fiels: name, prodId, and serial number */
    if (strcmp(usb_ptr->mfg[0], tmp->mfg[0])==0 &&
        strcmp(usb_ptr->mfg[1], tmp->mfg[1])==0 &&
        strcmp(usb_ptr->mfg[2], tmp->mfg[2])==0) {
        return 1;
    }

    return 0;
}


/*******************************************************************************
 *
 * Function   :	usb_get_str
 * Description:	get usb string from proc file.
 * Inputs     :	line, entire line from file, key, key string to look for
 * Outputs    : ptr, string value to be saved. return NULL if failed to
 *              find key in string.
 *
 *******************************************************************************
 */
static char *
usb_get_str (char *ptr, char *line, char *key)
{
    if ((ptr = strstr(line, key))) {
        while (*ptr != ':') ptr++;
        ptr += 2;
        ptr[strlen(ptr)-1] = '\0';
    } else {
        return NULL;
    }
    return ptr;
}


/*******************************************************************************
 *
 * Function   :	usb_get_bus_no
 * Description:	get usb number given usb_ptr struct.
 * Inputs     :	usb_ptr, usb structure
 * Outputs    : bus number, if sucessful; linux error code otherwise.
 *
 *******************************************************************************
 */
int
usb_get_bus_no (struct usb_info_t *usb_ptr)
{
    struct usb_info_t usb_tmp;
    FILE *fp;
    DIR *dir;
    struct dirent *dp;
    char line[320],  *ptr;

    if ((dir = opendir(USB_SCSI_USB_STORAGE_FILE)) == NULL) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            perror("cannot open directory USB_SCSI_FILE .");
        }
        return errno;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;

        //check make sure it's a number
        sprintf(line, "%s/%s", USB_SCSI_USB_STORAGE_FILE, dp->d_name);
        if ((fp = fopen(line, "r")) == NULL) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                perror("usb.c : cannot open file.");
            }
            return errno;
        }

        memset(&usb_tmp, 0, sizeof(usb_tmp));

        while (fgets(line, sizeof(line), fp)) {

            if (strstr(line, "usb-storage")) {
                usb_tmp.bus_no = atoi(dp->d_name);
                continue; /* go to the next line */
            } 

            if (usb_tmp.bus_no) {
                ptr = NULL;
                if ((ptr = usb_get_str(ptr, line, "Vendor"))) {
                    sprintf(usb_tmp.mfg[0], "%s", ptr);
                    //printf("Vendor found :%s\n", usb_tmp.mfg[0]);
                }
                if ((ptr = usb_get_str(ptr, line, "Product"))) {
                    sprintf(usb_tmp.mfg[1], "%s", ptr);
                    //printf("Product found :%s\n", usb_tmp.mfg[1]);
                }
                if ((ptr = usb_get_str(ptr, line, "Serial Number"))) {
                    sprintf(usb_tmp.mfg[2], "%s", ptr);
                    //printf("SN found :%s\n", usb_tmp.mfg[2]);
                    if (usb_match(usb_ptr, &usb_tmp)) {
                        usb_ptr->bus_no = usb_tmp.bus_no;
                        break;
                    }
                } /* if bus.no */
            }
        } /* end while scanning file  */
        fclose(fp);
    }  /* end while scanning directory */
    closedir(dir);
    return (usb_tmp.bus_no);
}
    
/*******************************************************************************
 *
 * Function   :	usb_get_info
 * Description:	serach proc file for usb devices and save info into usb struct.
 * Inputs     :	NONE
 * Outputs    : errno, if failed. 0 otherwise.
 *
 *******************************************************************************
 */
int
usb_get_info (void)
{
    struct usb_info_t usb_tmp;
    FILE *fp;
    char line[82], tmp[32], *ptr, *ptr2;
    int str_idx = 0;
    
    system(SCAN_DEVICES);
    
    fp = fopen(USB_DEVICE_FILE, "r");
    if (!fp) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s not found", USB_DEVICE_FILE);
            perror("not found.");
        }
        return errno;
    }
    
    memset(usb, 0, sizeof(usb));
    
    ptr = &line[1];
    while (fgets(line, sizeof(line), fp)) {
        //        printf("type is %c..\n", line[0]);

        if (line[0] == 'T') {
            str_idx = 0;
            memset(&usb_tmp, 0, sizeof(usb_tmp));
            sscanf(ptr, "%[^0-9]%02cx %[^0-9]%02cx %[^0-9]%02cx %[^0-9]%02cx %[^0-9]%02cx %[^0-9]%cd %[^0-9]%d %[^0-9]%d\n",
                   tmp, &usb_tmp.bus, tmp, &usb_tmp.lev, tmp, &usb_tmp.prnt, tmp,
                   &usb_tmp.port, tmp, &usb_tmp.cnt, tmp, &usb_tmp.dev, tmp, &usb_tmp.spd,
                   tmp, &usb_tmp.mxch);


        } /* end if 'T' */

        /* save manufacturing info into temp usb structure
           if dievice exists, there will be 3 lines starging with 'S'. */
        if (line[0] == 'S') {
            if ((ptr2 =strstr(ptr, "="))) {
                ptr2++;
                ptr2[strlen(ptr2)-1] = '\0';
                sprintf(usb_tmp.mfg[str_idx++], "%s", ptr2);
            }

        }

        if (line[0] == 'I') {
            if ((ptr2 =strstr(ptr, "Driver=usb-storage"))) {
                usb_tmp.found  = 1;
                memcpy(&usb[usb_tmp.port], &usb_tmp, sizeof(usb_tmp));
                if (usb_get_bus_no(&usb[usb_tmp.port])) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****BUS FOUND**** %d\n", usb[usb_tmp.port].bus_no);
                    }
                }
                if (usb_find_dev_name(usb_tmp.port)) {
                    if ((NVRAM)->diagflag & D_VERBOSE) {
                        printf("****dev found**** %s\n", usb[usb_tmp.port].dev_name);
                    }
                } else {

                }
                
            }
        }

        
    } /* while */

    fclose(fp);

    return 0;
}

/*******************************************************************************
 *
 * Function   :	device_select
 * Description:	block until something shows in in the read descriptor.
 * Inputs     :	fd, file descriptor.
 * Outputs    : errno, if failed. 0 otherwise.
 *
 *******************************************************************************
 */
int 
device_select (int fd)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    /* Wait up to five seconds. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    retval = select(fd+1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (retval == -1) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            perror("device_selcet: select \n");
        }
        return errno;
    } else if (retval) {
        return 0;
        /* FD_ISSET(0, &rfds) will be true. */
    } else {
        return 0;
    }
    return 0;
}

/*******************************************************************************
 *
 * Function   :	access_device_test
 * Description:	main test for usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
access_device_test (char *src)
{
    char buf[128], *buf_bk, *buf_wr, *buf_rd, *buf_bkrd;
    char *p1;
    char *p2;
    char *p3;
    char *p4;
    int devfd, num, ib;
    int ix, cnt = 0;
    off_t dev_size, test_size;
    int rc = PASSED;
  
    sprintf(buf, "%s", src);
    
    for (ix = 0; ix < 10; ix++) {
        devfd = open(buf, O_RDWR);
        if(devfd < 0) {
            sleep(1);
            continue;
        } else {
            break;
        }
    }

    if (devfd < 0) {
        perror("there is no device file descriptor available. ");
        printf("Can not access device at %s. is slot vacant?", src);
        return FAILED;
    }

    /* compare strings : usbdrv0, usbdrv1, emmc0, mSATA, m2sata, eUSB, cf */
    if ((strcmp(buf, DEV_USB0) == 0) || (strcmp(buf, DEV_USB1) == 0) || (strcmp(buf, DEV_EMMC) == 0) ||
        (strcmp(buf, DEV_MSATA) == 0) || (strcmp(buf, DEV_M2SATA) == 0) || (strcmp(buf, DEV_EUSB) == 0) ||
        (strcmp(buf, DEV_CF) == 0) || (strcmp(buf, DEV_M2EUSB) == 0)) {
        dev_size = lseek(devfd, 0, SEEK_END);
        /* Verifing 512 bytes */
        test_size = SIZE_512B;
    } else {
        dev_size = lseek(devfd, 0, SEEK_END);
        /* Verifing 100M bytes */
        test_size = SIZE_100MB;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\ndev_name = %s, dev_size = %lx, test_size = %lx\n", buf, dev_size, test_size);
    }

    buf_bk = (char *) malloc(test_size);
    buf_wr = (char *) malloc(test_size);
    buf_rd = (char *) malloc(test_size);
    buf_bkrd = (char *) malloc(test_size);

    p1 = buf_wr;
    p2 = buf_rd;
    p3 = buf_bk;
    p4 = buf_bkrd;
    memset(buf_bk, 0, malloc_usable_size(buf_bk));
    memset(buf_wr, 0, malloc_usable_size(buf_wr));
    memset(buf_rd, 0, malloc_usable_size(buf_rd));
    memset(buf_bkrd, 0, malloc_usable_size(buf_bkrd));

    /* back up data */
    if (lseek(devfd, 0, SEEK_SET) <  0) {
        perror("lseek to the beginning of device failed.");
        printf("backup lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = read(devfd, buf_bk, malloc_usable_size(buf_bk))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read data from device failed");
        printf("Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    /* prepare data pattern */
    for (cnt = 0; cnt < malloc_usable_size(buf_wr); cnt++) {
        buf_wr[cnt]= PATTERN + cnt;
    }

    /* write data pattern */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("write lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = write(devfd, buf_wr, malloc_usable_size(buf_wr))) < 0) {
        perror("Write test pattern failed, can not write to drive.\n");
        printf("Unable to write data pattern to device.");
        rc = FAILED;
        goto exit;
    }

    if (num != malloc_usable_size(buf_wr)) {
        perror("not all the bytes are written for data pattern");
        rc = FAILED;
        goto exit;
    }

    if (fsync(devfd)<0) {
        perror("fsync failed.");
        printf("Unable to sync data pattern to device.");
        rc = FAILED;
        goto exit;
    }

    /* read back data for comparing */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = read(devfd, buf_rd, malloc_usable_size(buf_rd))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read back data from device failed");
        printf("Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != malloc_usable_size(buf_rd)) {
        perror("not all the bytes are read for data pattern");
        rc = FAILED;
        goto exit;
    }
    
    /* comparing data */
    for (ib = 0; ib < malloc_usable_size(buf_rd); ib++, p1++, p2++) {
        if (*p1 != *p2) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p1, *p2);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    /* restore data */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("restore lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }
    
    if ((num = write(devfd, buf_bk, malloc_usable_size(buf_bk))) < 0) {
        close(devfd); /* don't need it anymore */
        perror("Write restore data failed, can not write to drive.\n");
        printf("Unable to write restore data to device.");
        rc = FAILED;
        goto exit;
    }

    if (num != malloc_usable_size(buf_bk)) {
        perror("not all the bytes are written for restore");
        rc = FAILED;
        goto exit;
    }

    if (fsync(devfd)<0) {
        perror("fsync failed.");
        printf("Unable to sync data pattern to device.");
        rc = FAILED;
        goto exit;
    }

    /* read back restore data for comparing */
    if (lseek(devfd, 0, SEEK_SET) < 0) {
        perror("lseek to the beginning of device failed.");
        printf("lseek failed; Cannot point to the beginning of device.");
        rc = FAILED;
        goto exit;
    }

    if ((num = read(devfd, buf_bkrd, malloc_usable_size(buf_bkrd))) == -1) {
        close(devfd); /* don't need it anymore */
        perror("Read back data from device failed");
        printf("Unable to read from drive.\n");
        rc = FAILED;
        goto exit;
    }

    if (num != malloc_usable_size(buf_bkrd)) {
        perror("not all the bytes are read for data pattern");
        rc = FAILED;
        goto exit;
    }
    
    /* comparing data */
    for (ib = 0; ib < malloc_usable_size(buf_bkrd); ib++, p3++, p4++) {
        if (*p3 != *p4) {
            printf("failed on byte %d, wrote = %02x, read back = %02x\n",(ib+1), *p3, *p4);
            if (cnt++ > 10) {
                printf("Too many data mismatches. Stop testing\n");
            }
            break;
        }
    }

    goto exit;

exit:
    free(buf_rd);
    free(buf_bkrd);
    free(buf_wr);
    free(buf_bk);
    close(devfd); /* don't need it anymore */
    return rc;
}

/*******************************************************************************
 *
 * Function   :	check_block_size
 * Description:	check for existence of block device and print its size
 * Inputs     :	device name (e.g. /dev/eUSB)
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int check_block_size (char *dev_name)
{
    int ret=PASSED; 
    char *check_status;
    char *get_blk_info;
    char *blk_size;
    FILE *blk_info;
	
    check_status = (char *) malloc(strlen("ls %s >& /dev/null")+strlen(dev_name)+1);
    get_blk_info = (char *) malloc(strlen("lsblk %s | grep 'disk' | awk '{print $3 " " $4}' >> /etc/blk.info")+strlen(dev_name)+1);
    blk_size = (char *) malloc(8);

    sprintf(check_status, "ls %s >& /dev/null", dev_name);
    sprintf(get_blk_info, "lsblk %s | grep 'disk' | awk '{print $3 " " $4}' >> /etc/blk.info", dev_name);
	
    if (!system(check_status)) {
        printf("%s detected, size is:", dev_name);
	system(get_blk_info);
	/*parse lsblk output*/
	blk_info = fopen("/etc/blk.info", "r+");
	fseek(blk_info, 1, SEEK_SET);
	fgets(blk_size, 8, blk_info);
	fclose(blk_info);
			
	printf("%s\n", blk_size);
	system("rm /etc/blk.info");
    } else {
	printf("no %s detected\n\n", dev_name);
	ret = FAILED;
    }

    free(check_status);
    free(get_blk_info);
    free(blk_size);
		
    return (ret);
}
 
/*******************************************************************************
 *
 * Function   :	usb_slot_tests
 * Description:	entry point to usb device test
 * Inputs     :	file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int 
usb_slot_tests (int slot)
{
    char src[32];
    int retval;
    
    sprintf(src, "/dev/usbdrv%d", slot);

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	cf_slot_tests
 * Description:	main test for cf test.
 * Inputs     :	file path to compact flash device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
cf_slot_tests (int slot)
{
    char src[32];
    int retval;
   
    sprintf(src, "/dev/cf");

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	eusb_slot_tests
 * Description:	main test for eUSB test.
 * Inputs     : dummy	
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
eusb_slot_tests (int dummy)
{
    char src[32];
    int retval;
   
    sprintf(src, "/dev/eUSB");

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	m2usb_slot_tests
 * Description:	main test for M.2 USB module test.
 * Inputs     : dummy	
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
m2usb_slot_tests (int dummy)
{
    char src[32];
    int retval;
   
    sprintf(src, "/dev/m2usb");

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   : msata_slot_tests
 * Description: main test for msata test.
 * Inputs     : dummy
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
msata_slot_tests (int dummy)
{
    char src[32];
    int retval;

    sprintf(src, "/dev/mSATA");

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   : emmc_slot_tests
 * Description: main test for emmc test.
 * Inputs     : dummy
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
emmc_slot_tests (int dummy)
{
    char src[32];
    int retval;

    sprintf(src, "/dev/emmc0");

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	sata_tests
 * Description:	main test for sata test.
 * Inputs     :	slot, 0 or 1.
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int
sata_tests (uchar *sata_name)
{
    char src[32];
    int retval;

    /* get sata name */
    sprintf(src, "%s", sata_name);	

    retval = access_device_test(src);
    return retval;
}

/*******************************************************************************
 *
 * Function   :	usb_dump
 * Description:	main entry to display usb info.
 * Inputs     :	n, not used
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
static int
usb_dump (int n)
{
    usb_get_info();
    usb_display(USB0);
    usb_display(USB1);
    usb_display(COMPACT_FLASH);
    return PASSED;
}


/*************************below left for ref ******************/
/* we assume that udev rule assignes /dev/usb0x to the device name of usb slot 0 (ie, /dev/sdb, etc..)
   if we change the name in the udev rule, then we need to change the nanme in this code as well.
*/
/******** History ********
$Log: linux_usb_test.c,v $
Revision 1.28  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

Revision 1.27  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.26.2.1  2019/03/27 01:13:50  ptong
Release curie 1ru V1.3.1 : Separate motheboard PCIe and M.2 NVMe care PCIe scan tests. Use /dev/m2usb for M.2 USB module according to kernel change. Check FPGA M.2 module bits correctly to support M.2 slot test

Revision 1.26  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.25.2.3  2018/04/24 02:07:17  leschen
To get storage dev size precisely, show correct dev name when unable to detect it for check_block_size function.

Revision 1.25.2.2  2017/08/31 06:16:14  leschen
Enhance storage device testing.

Revision 1.25.2.1  2016/12/28 09:47:04  alpeng
update usb util, it is obsolete on new kernel

Revision 1.25  2016/04/19 00:15:37  jskow
Add function to check for eUSB and emmc and display size in victory platforms.

Revision 1.24  2014/08/15 11:08:05  danchung
add the comparison of restored data in usb test

Revision 1.23  2014/06/09 05:44:46  alpeng
add fsync to make sure the data is written into storage dev

Revision 1.22  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.21  2013/11/13 11:06:34  danchung
Change "cf test" to "eUSB test" for Utah

Revision 1.20  2013/11/07 00:53:41  danchung
Add emmc0 test.

Revision 1.19  2013/09/05 01:58:27  alpeng
support mSATA test on Utah

Revision 1.18  2013/06/24 08:39:24  alpeng
support Ext. loopback flag checking before USB/CF diag

Revision 1.17  2013/04/08 19:32:23  mcharon
allow more time for linux to discover device in access_device_test

Revision 1.16  2013/03/01 22:41:18  mcharon
when running usb test, when probing device allow up to 5 secs for linux to see device

Revision 1.15  2013/02/27 22:17:13  mcharon
use open/read/write instead of 'dd' command for usb test

Revision 1.14  2012/12/06 22:10:58  mcharon
change name of usb from /dev/usb/ to dev/usbdrv

Revision 1.13  2012/11/02 16:09:40  alpeng
support HDD test on overdirve, add some diag item for manufacturing using

Revision 1.12  2012/11/01 09:38:40  alpeng
support SATA r/w on both modes of overdirve

Revision 1.11  2012/10/24 08:58:08  alpeng
release reset bit after reset. using warning instead of fatal when device is vacant

Revision 1.10  2012/10/23 08:02:37  alpeng
supported HDD test on overdrive

Revision 1.9  2012/09/19 02:49:53  alpeng
remove prcomplete on usb, cf and sata test, using menu flag to show errcount

Revision 1.8  2012/09/18 19:19:54  mcharon
support poll slot-fix io intr test; support fpga upgrade; support serial irq intr tst

Revision 1.7  2012/09/12 09:21:13  alpeng
remove SATA test from mbtest and integrate SATA test into ngwic3 test

Revision 1.6  2012/06/06 09:48:03  aarwang
- Clean up compiler warnings.

Revision 1.5  2012/05/07 08:10:10  alpeng
fixed message, remove prcomplete

Revision 1.4  2012/05/03 10:16:28  alpeng
report slot vacant instead of failed.

Revision 1.3  2012/03/28 00:38:14  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:22  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:05  ptong
Initial archive of ng_diag module


$Endlog$
*/
