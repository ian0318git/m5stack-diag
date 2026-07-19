/* $Id: full_load.c,v 1.2 2018/02/09 09:56:56 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/full_load.c,v $
 *------------------------------------------------------------------
 *
 * full_load.c - Full loading utility
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "error.h"
#include "common_utils.h"
#include "full_load.h"


#define USB_SCRIPT "\
#!/bin/sh \n\
NODE=`find / -name sd* | grep block | grep \"2-1:1.0\\|1-1:1.0\" | awk -F / '{print $15}' | awk 'NR==1'` \n\
for i in 1 2 3 4 5 \n\
do \n\
	if [[ ${#NODE} -eq 0 ]]; then { \n\
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
DEVNODE=`blkid | grep $NODE | awk '{print $1}' | tr -d : | awk -F / '{print $NF}'` \n\
mkdir -p /mnt/$NODE \n\
mount -t vfat /dev/$DEVNODE /mnt/$NODE \n\
while : \n\
do \n\
	taskset 0x1 dd conv=notrunc if=/dev/zero of=/mnt/$NODE/thermal.test bs=400M count=1 oflag=direct \n\
	echo \"[USB] file is written in /mnt/$NODE\" \n\
	res=`taskset 0x1 md5sum /mnt/$NODE/thermal.test` &> /dev/null \n\
	if [[ ${#res} -ne 0 ]]; then { \n\
		echo \"[USB] file is read /mnt/$NODE/thermal.test\" \n\
	} \n\
	fi \n\
done & \n\
" 

#define EMMC_SCRIPT  "\
#!/bin/sh \n\
partition_check=`blkid | grep mmcblk0p1` \n\
if [[ ${#partition_check} -eq 0 ]]; then { \n\
	echo \"This DUT eMMC need to create partition and it will cost some time. Please wait.....\" \n\
	(echo d; echo d; echo n; echo p; echo 1; echo; echo; echo w) | fdisk /dev/mmcblk0 \n\
	sleep 3 \n\
	mkfs.ext4 -F /dev/mmcblk0p1 \n\
} \n\
else { \n\
	fs_check=`fsck -N /dev/mmcblk0p1 | grep ext4` \n\
	if [[ ${#fs_check} -eq 0 ]]; then { \n\
		echo \"This DUT eMMC partition isn't ext4. Please wait.....\" \n\
		(echo d; echo d; echo n; echo p; echo 1; echo; echo; echo w) | fdisk /dev/mmcblk0 \n\
		sleep 3 \n\
		mkfs.ext4 -F /dev/mmcblk0p1 \n\
	} \n\
	fi \n\
} \n\
fi \n\
mkdir -p /mnt/emmc 2>/dev/null \n\
mount -t ext4 /dev/mmcblk0p1 /mnt/emmc 2>/dev/null \n\
while : \n\
do \n\
	taskset 0x8 dd conv=notrunc if=/dev/zero of=/mnt/emmc/thermal.test bs=400M count=1 oflag=direct \n\
	echo \"[eMMC] file is written in /mnt/emmc\" \n\
	res=`taskset 0x8 md5sum /mnt/emmc/thermal.test` 2>/dev/null \n\
	if [[ ${#res} -ne 0 ]]; then { \n\
		echo \"[eMMC] file is read /mnt/emmc/thermal.test\" \n\
	} \n\
	fi \n\
done & \n\
"

static int g_Initial=0;

int excute_bash(char *cmd, char *result_buf, unsigned int result_buf_size)
{
    int rc = 0, len=0;
    char buf[BUF_SIZE], *p = buf;
    FILE *fp;
    char c;
    /*opens a process by creating a pipe, forking, and invoking the shell.*/
    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        printf("popen Fail! \n");
        return -1/*FALSE*/;
    }

    while((c = fgetc(fp)) != EOF && c != '\n')
    {
		if(len >= result_buf_size)
			return -1;
		p[len++] = (char)c;
	}
	strcpy(result_buf, buf);

    /*waits for the associated process to terminate and returns the exit status of the command.*/
    rc = pclose(fp);
 
    if(-1 == rc)
    {
        printf("pclose Fail! \n");
        return -1/*FALSE*/;
    }
    //printf("Return: %s", result_buf);
    return 1/*TRUE*/;
}

void create_scripts(void)
{
	struct stat st;

	if(!g_Initial)
	{
		if(stat("/usr/sbin/usbrw",&st) != 0)
        {
			//add usb script
			system("touch /usr/sbin/usbrw; chmod 777 /usr/sbin/usbrw");
		}
		
		FILE *f = fopen("/usr/sbin/usbrw", "w");
		if (f == NULL)
		{
			printf("Error opening file!\n");
			exit(1);
		}
		fprintf(f, "%s", USB_SCRIPT);
		fclose(f);
		
		if(stat("/usr/sbin/emmcrw",&st) != 0)
		{
			//add emmc script
			system("touch /usr/sbin/emmcrw; chmod 777 /usr/sbin/emmcrw");
		}
		
		f = fopen("/usr/sbin/emmcrw", "w");
		if (f == NULL)
		{
			printf("Error opening file!\n");
			exit(1);
		}
		fprintf(f, "%s", EMMC_SCRIPT);
		fclose(f);
	}
}

void fload_start(void)
{
	char buf[BUF_SIZE];
	int buf_len=512;
	
	memset(buf, 0, sizeof(buf));
	
	printf("If you want to stop full load utility, please input s to stop\n");
	if(getc_answer("Do you want to start full load utility? Please input y to confirm", "yn", 'n') == 'y')
		create_scripts();
	else
		return;

	printf("Prepare to start : \n1.Memory test\n2.USB R/W\n3.eMMC R/W\n4.CPU 100%%\n");

	system("umount /mnt 2>/dev/null; umount /mnt/* 2>/dev/null; usbrw; sleep 2; emmcrw; sleep 2");
	system("taskset 0x2 memtester 2G &>/dev/null &");
	
	memset(buf, 0, sizeof(buf));
	excute_bash("ps | grep  memtester | wc -l", buf, buf_len);
	if(strlen(buf) > 0)
	{
		if(atoi(buf) > 1)
			printf("Memory test executed...\n");
	}
	else
		printf("Memory test execute fail!\n");
		
	system("taskset 0x1 stress --cpu 1 &");
	system("taskset 0x2 stress --cpu 1 &");
	system("taskset 0x4 stress --cpu 1 &");
	system("taskset 0x8 stress --cpu 1 &");
	sleep(1);
	excute_bash("ps | grep  stress | wc -l", buf, buf_len);
	if(strlen(buf) > 0)
	{
		if(atoi(buf) > 1)
			printf("CPU stress test 100%% executed...\n");
	}
	else
		printf("CPU stress 100%% execute fail!\n");
		
	while(getc_answer("Do you want to stop full load utility? Please input s to confirm. c:Continue, s:Stop\n", "cs", 'c') != 's');
	
	system("killall usbrw 2>/dev/null; killall emmcrw 2>/dev/null; killall dd 2>/dev/null; killall stress; killall memtester");
	printf("Full Load Utility is stopped!");
}

/*------------------------------------------------------------------
$Log: full_load.c,v $
Revision 1.2  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.4.2  2018/01/20 05:57:48  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.2.2  2017/11/21 02:37:46  lucywang
Show how to stop full load utility before start the full loading

Revision 1.1.2.1  2017/09/20 08:18:09  lucywang
added full load utility


$Endlog$
*/
