/* $Id: full_load_util.h,v 1.4 2019/07/11 12:31:31 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/full_load_util.h,v $
 *------------------------------------------------------------------
 *
 * full_load_util.h - Full loading utility
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __FULL_LOAD_UTIL_H__
#define __FULL_LOAD_UTIL_H__

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/ioctl.h>  
#include <linux/sockios.h>  
#include <linux/types.h>
#include <sys/mman.h>
#include <unistd.h>

#define BUF_SIZE 512

#define FULL_SCRIPT "\
#!/bin/sh \n\
USB_NODE=`find / -name sd* | grep block | grep \"2-1:1.0\\|1-1:1.0\" | awk -F / '{print $13}' | awk 'NR==1'` \n\
for i in 1 2 3 4 5 \n\
do \n\
	if [[ ${#USB_NODE} -eq 0 ]]; then { \n\
		if [[ $i -eq 5 ]]; then { \n\
			echo \"Can't find USB Storage\" \n\
			exit $? \n\
		} \n\
		fi \n\
		continue \n\
	} \n\
	else \n\
		break \n\
	fi \n\
done \n\
DEVNODE=`blkid | grep $USB_NODE | awk '{print $1}' | tr -d : | awk -F / '{print $NF}'` \n\
mkdir -p /mnt/$USB_NODE \n\
mount -t vfat /dev/$DEVNODE /mnt/$USB_NODE \n\
\n\
partition_check=`blkid | grep mmcblk0p1` \n\
if [[ ${#partition_check} -eq 0 ]]; then { \n\
	echo \"This DUT eMMC need to create partition and it will cost some time. Please wait.....\" \n\
	(echo d; echo d; echo n; echo p; echo 1; echo; echo; echo w) | fdisk /dev/mmcblk0 \n\
	sleep 3 \n\
	(echo y) | mkfs.ext4 /dev/mmcblk0p1 \n\
} \n\
else { \n\
	fs_check=`file -s /dev/mmcblk0p1 | grep ext4` \n\
	if [[ ${#fs_check} -eq 0 ]]; then { \n\
		echo \"This DUT eMMC partition isn't EXT4. Please wait.....\" \n\
		(echo d; echo d; echo n; echo p; echo 1; echo; echo; echo w) | fdisk /dev/mmcblk0 \n\
		sleep 3 \n\
		(echo y) | mkfs.ext4 /dev/mmcblk0p1 \n\
	} \n\
	fi \n\
} \n\
fi \n\
mkdir -p /mnt/emmc &> /dev/null \n\
mount -t ext4 /dev/mmcblk0p1 /mnt/emmc &> /dev/null \n\
\n\
while : \n\
do \n\
	memtester 200K 1 &> /dev/null \n\
	echo \"[Memory] memory test\" \n\
	\n\
	dd conv=notrunc if=/dev/zero of=$mountpoint/thermal.test bs=2M count=1 \n\
	echo \"[USB] file is written in /mnt/$USB_NODE\" \n\
	res=`md5sum /mnt/$USB_NODE/thermal.test` &> /dev/null \n\
	if [[ ${#res} -ne 0 ]]; then { \n\
		echo \"[USB] file is read /mnt/$USB_NODE/thermal.test\" \n\
	} \n\
	fi \n\
	\n\
	dd conv=notrunc if=/dev/zero of=/mnt/emmc/thermal.test bs=2M count=1 \n\
	echo \"[eMMC] file is written in /mnt/emmc\" \n\
	res=`md5sum /mnt/emmc/thermal.test` &> /dev/null \n\
	if [[ ${#res} -ne 0 ]]; then { \n\
		echo \"[eMMC] file is read /mnt/emmc/thermal.test\" \n\
	} \n\
	fi \n\
done & \n\
"

extern int diag_full_load_util(void);

#endif /* __FULL_LOAD_UTIL_H__ */

/******** History ********
$Log: full_load_util.h,v $
Revision 1.4  2019/07/11 12:31:31  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
