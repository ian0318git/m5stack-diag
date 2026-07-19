/* $Id: linux_usb_test.h,v 1.19 2021/06/02 07:42:56 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_usb_test.h,v $
 *------------------------------------------------------------------
 * 
 * Filename: linux_usb_test.h
 *
 * Copyright (c) 2016-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 * 
 *------------------------------------------------------------------
 */

#ifndef __USB_HEADER__
#define __USB_HEADER__

enum {
    USB_MFG = 0,
    USB_PROD,
    USB_SER
};

enum {
    USB0 = 0,
    USB1,
    COMPACT_FLASH,
};

struct usb_info_t {
    unsigned char bus;
    unsigned char lev;
    unsigned char prnt;
    unsigned char port;  /* port=0 for usb0; port=1 for usb1; port=2 for flash, etc...*/
    unsigned char cnt;
    unsigned char dev;
    unsigned int spd;
    unsigned int mxch;
    unsigned int vendor;
    unsigned int prodID;
    char mfg[3][64]; /* hold manufacturing info, (ie 0:company name, 1:product name, 2: serial num */
    //    unsigned char mft[64];
    //    unsigned char prod[64];
    //    unsigned char ser[64];
    unsigned int bus_no;
    char dev_name[32]; /* holds device node assoicated with this usb, ie /dev/sda/, dev/sda1/, dev/sda2 */
    unsigned char found;
    
};

#define SCAN_DEVICES   "mdev -s"
#if defined (TSN) || (CURIE_1RU) || defined (CURIE_2RU) || defined(VIPER) \
    || defined(NUTELLA) || defined(TABEIL) || defined(NANOOK) || defined (HIGHRISE) || defined(PHOENIX)
/*
 * /proc/bus/usb/devices
 *    Location where the usb/devices file can normally be found for
 *    Linux kernels before 2.6.31, if usbfs is mounted.
 *
 * /sys/kernel/debug/usb/devices
 *    Location where the usb/devices file can normally be found for
 *    Linux kernel 2.6.31 and later, if debugfs is mounted.
 *
 */
#define MOUNT_DEBUGFS "mount -t debugfs none /sys/kernel/debug"
#define USB_DEVICE_FILE  "/sys/kernel/debug/usb/devices"
#else
//mount -t usbfs none /proc/bus/usb
#define USB_DEVICE_FILE  "/proc/bus/usb/devices"
#endif /* TSN VIPER*/
#define USB_SCSI_USB_STORAGE_FILE  "/proc/scsi/usb-storage"
#define USB_SCSI_DEVICE_FILE  "/sys/bus/scsi/devices"
#define MAX_USB   3 /* including usb and compact flash */
#define DEV_USB0 "/dev/usbdrv0"
#define DEV_USB1 "/dev/usbdrv1"
#define DEV_EMMC "/dev/emmc0"
#define DEV_MSATA "/dev/mSATA"
#define DEV_M2SATA "/dev/m2sata"
#define DEV_M2EUSB "/dev/m2eusb"
#define DEV_M2NVME "/dev/m2nvme01"
#define DEV_M2USB "/dev/m2usb"
#define DEV_EUSB "/dev/eUSB"
#define DEV_CF "/dev/cf"
#define SIZE_512B 0x200
#define SIZE_100MB 0x5F5E100

extern int usb_slot_tests(int);
extern int cf_slot_tests(int);
extern int eusb_slot_tests (int);
extern int m2usb_slot_tests (int);
extern int msata_slot_tests(int);
extern int emmc_slot_tests(int);
extern int sata_tests(uchar *);
extern int compactflash_tests(int);
extern int usb_get_dev_name();
extern int usb_get_info();
extern int usb_utils();
extern int usb_utils_v2(); /* for 3.19 kernel and later, using on neptune */
extern int check_block_size(char *);

#endif
/******** History ********
$Log: linux_usb_test.h,v $
Revision 1.19  2021/06/02 07:42:56  iachang
CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk

Revision 1.18  2021/04/14 09:10:13  achiu2
[PRRQ:CSCvx56970-2] Phoenix code review for ER

Revision 1.17  2020/08/19 09:48:59  markzha
*** empty log message ***

Revision 1.16  2020/01/09 01:01:49  jiajliu
Merge Curie 2RU to main trunk

Revision 1.15  2019/12/11 10:10:22  lucywang
Merged Nanook to main trunk

Revision 1.14  2019/10/17 02:16:14  kehuang2
Collapse Tabei-L into main trunk

Revision 1.13  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.12  2019/07/11 12:34:40  alicehua
Collapse Nutella codes into main trunk

Revision 1.11.40.1  2019/01/25 02:11:06  harrchan
Add definition of NUTELLA

Revision 1.11  2018/08/06 02:31:00  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.10  2018/05/18 09:24:47  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.9  2017/08/02 14:21:28  steja
Support TSN-H/M platform code

Revision 1.8.22.1  2017/07/29 03:40:43  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.8.2.3  2018/05/17 10:50:19  alpeng
 sync with trunk <trunk-051618>

Revision 1.8.2.2  2017/08/31 06:19:01  leschen
Add definition of storage device node.

Revision 1.8.2.1  2016/12/28 09:46:59  alpeng
update usb util, it is obsolete on new kernel

Revision 1.9  2017/08/02 14:21:28  steja
Support TSN-H/M platform code

Revision 1.8.22.1  2017/07/29 03:40:43  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.8  2016/04/19 00:15:37  jskow
Add function to check for eUSB and emmc and display size in victory platforms.

Revision 1.7  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.6  2013/11/13 11:07:21  danchung
Add eUSB test for Utah.

Revision 1.5  2013/11/07 00:53:16  danchung
Add emmc0 tset.

Revision 1.4  2013/09/05 01:58:27  alpeng
support mSATA test on Utah

Revision 1.3  2012/10/23 08:02:37  alpeng
supported HDD test on overdrive

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/

