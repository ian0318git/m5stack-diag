/* $Id: uio_utils.c,v 1.7 2014/05/05 21:37:31 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/uio_utils.c,v $
 *------------------------------------------------------------------
 * Filename:  uio_utils.c
 *            code related to uio actions.
 *
 *
 * Copyright (c) 2014 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "uio_utils.h"

static char name[320];
static volatile void *regs;
static int uio_find_name (const char *drv_name, char *uio_name);
#define DRV_NAME "uio_fpga_dash"

static int uiofd;

extern ssize_t pread(int fd, void *buf, size_t count, off_t offset);
extern ssize_t pwrite(int fd, const void * buf, size_t count, off_t offset);

volatile void *
uio_get_regs ()
{
    return regs;
}

int
uio_enable_intr ()
{
    int err;
    int configfd;
    char config[128];
    char uio_name[82];
                  
    unsigned char command_high;

    if (uio_find_name(DRV_NAME, uio_name) >= 0) {
        sprintf(config, "/sys/class/uio/%s/device/config", uio_name);
    } else {
        printf("enable_intr: uanble to find /sys/class/uio/");
        return(-1);
        
    }
    
    configfd = open(config, O_RDWR);
    if (configfd < 0) {
        printf("can't open %s\n", config);
        perror("config open:");
        return errno;
    }

    /* Read and cache command value */
    err = pread(configfd, &command_high, 1, 5);
    if (err != 1) {
        perror("uio_enable_intr: command config read:");
        return errno;
    }
    command_high &= ~0x4;

    /* Re-henable interrupts. */
    err = pwrite(configfd, &command_high, 1, 5);
    if (err != 1) {
        perror("uio_enable_intr: config write:");
        return errno;
    }

    close(configfd);
    return 0;
}

int
uio_select (unsigned int *icount, unsigned int secs, unsigned int msecs)
{
    struct timeval tv;
    int err;
    fd_set rdfd;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&rdfd);
    FD_SET(uiofd, &rdfd);
    
    if (uio_enable_intr() != 0)
        return errno;
    err = select(uiofd+1, &rdfd, NULL, NULL, &tv);
    if (err == -1) {
        perror("uio_utils: select \n");
        return errno;
    }
    if (FD_ISSET(uiofd, &rdfd)) {
        /* data available now, so read */
        err = read(uiofd, icount, 4);
        if (err != 4) {
            perror("uio select: can't read:");
            return errno;
        }
        printf("uio  count is %d\n", *icount);
    } else {
        printf("NO data count is %d\n", *icount);
    }

    return 0;
}

/* returns direcotry name (ie, "uio0") */
static int
uio_find_name (const char *drv_name, char *uio_name)
{
    FILE *fp;
    DIR *dir;
    struct dirent *dp;
    char *sys = "/sys/class/uio";

    if (strlen(name)) {
        sprintf(uio_name, name);
        return(1);
    }
    
    if ((dir = opendir(sys)) == NULL) {
        perror("cannot open directory /sys/class/uio");
        return -1;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_name[0] == '.')
            continue;
        sprintf(name, "%s/%s/name", sys, dp->d_name);
        if ((fp = fopen(name, "r")) == NULL) {
            continue;
        }

        fgets(name, sizeof(name), fp);
        if (strstr(name, DRV_NAME)) {
            sprintf(name, dp->d_name);
            sprintf(uio_name, name);
            fclose(fp);
            closedir(dir);
            return 1;
        } else {

        }
        fclose(fp);
    }

    printf("uio driver %s not found in %s directory \n", drv_name,
           sys);
    name[0] = '\0';
    closedir(dir);
    return -1;
}

int
uio_open ()
{
    int resourcefd;
    size_t size = 1024 * 1024 * 16; /* should get this from config space */
    char resource[80];
    char config[128];
    char uio_name[32];
    
    char str[80];

    if (uio_find_name(DRV_NAME, uio_name) >= 0) {
        sprintf(config, "/sys/class/uio/%s/device/config", uio_name);
        sprintf(resource, "/sys/class/uio/%s/device/resource0", uio_name);
    } else {
        printf("unable to open /sys/class/uio/uioX directory.\n");
        exit(0);
    }
    
    sprintf(str, "/dev/%s", uio_name);
    uiofd = open(str, O_RDONLY);
    if (uiofd < 0) {
        perror("uio open:");
        return errno;
    }

    resourcefd = open(resource, O_RDWR);
    if (resourcefd < 0) {
        perror("config space resource0: open failed:");
        return errno;
    }

    regs = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, resourcefd, 0);
    if (MAP_FAILED == regs) {
        perror("mmap failed");
        return errno;
    }
    return 0;
}

int
uio_read (unsigned int *icount)
{
    int err;
    
    if (uio_enable_intr()!=0)
        return errno;    

    /* Wait for next interrupt. */
    err = read(uiofd, icount, 4);
    if (err != 4) {
        perror("uio read:");
        return errno;
    }
    return 0;
}


/*------------------------------------------------------------------
$Log: uio_utils.c,v $
Revision 1.7  2014/05/05 21:37:31  mcharon
check return error from uio_enable_intr

Revision 1.6  2014/05/03 14:52:48  mcharon
use IFNAMSIZE; cache uio dir name in uio_utils

Revision 1.5  2013/11/26 08:40:34  hroni
fix compiler warning

Revision 1.4  2012/06/21 00:17:47  mcharon
remove extraneous print mesages

Revision 1.3  2012/06/06 09:48:03  aarwang
- Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:15  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:06  ptong
Initial archive of ng_diag module


$Endlog$
*/
