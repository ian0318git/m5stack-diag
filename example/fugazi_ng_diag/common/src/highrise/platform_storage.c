/* $Id: 
 * $Source: 
 *------------------------------------------------------------------
 *
 * Filename: platform_storage.c
 * shuyyu leveraged from tsn
 *
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
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

#define MAX_FILENAME_LENGTH     255
#define MAX_COMMAND_LENGTH      2048

/*
 * Global extern functions
 */
extern int ExecuteCmdbyPopen(char *cmd, char *retBuf, int sizeOfBuf);
extern int getc_answer(char *msg, char *cmpstr, char curval);


/*
 * Global variables
 */
extern int quiet_launch;


/*  local variables */



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
 * Function   :    access_device_test
 * Description:    main test for usb device test
 * Inputs     :    file path to usb device
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int access_device_test (const char *src)
{
    char buf[128], buf_bk[512], buf_wr[512], buf_rd[512];
    char *p1 = buf_wr;
    char *p2 = buf_rd;
    int devfd, num, ib;
    int ix, cnt = 0;
    int rc = PASSED;

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
            cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x\n",
                   (ib + 1), *p1, *p2);
            if (cnt++ > 10) {
                cterr('f', 0, "Too many data mismatches, cnt:%d. Stop testing\n", cnt);
                break;
            }
            rc = FAILED;
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
    return (rc);
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
    int rc = PASSED;

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
        cnt = 0;
		for (ib = 0; ib < buf_rd_len; ib++, p1++, p2++) {
			if (*p1 != *p2) {
				cterr('f', 0, "failed on byte %d, wrote = %02x, read back = %02x\n",
					   (ib + 1), *p1, *p2);
				if (cnt++ > 10) {
					cterr('f', 0, "Too many data mismatches, cnt:%d. Stop testing\n", cnt);
                    break;
				}
                rc = FAILED;
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
    return (rc);
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
 * Function   :    spi_slot_tests
 * Description:    main test for spi test.
 * Inputs     :    option for future use
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int spi_slot_tests (int option, const char *mtdblk)
{
    return access_device_test(mtdblk);
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

