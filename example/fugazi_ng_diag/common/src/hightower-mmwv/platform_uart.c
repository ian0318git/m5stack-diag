/* $Id: platform_uart.c,v 1.2 2021/06/02 02:56:22 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_uart.c,v $
 *********************************************************************
 *
 * platform_uart.c -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "hr_commn_util.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "proto.h"


#define UART0_MPP_CONF_REG_1    0XF2440014
#define UART0_MPP_CONF_REG_2    0XF2440018

#define UART0_MPP_CONF_MSK_1    0X00FF0000
#define UART0_MPP_CONF_MSK_2    0X0000FF00

#define UART0_MPP_CONF_FUN_1    0X00770000
#define UART0_MPP_CONF_FUN_2    0X00007700

#define UART0_IOMEM_BASE        0XF2702100

static int set_mpp_uart0_func(void)
{
    const unsigned int paddr = UART0_MPP_CONF_REG_1;
    const unsigned int size  = UART0_MPP_CONF_REG_2 - UART0_MPP_CONF_REG_1 + 16;
    uint32_t *vaddr          = NULL;
    uint32_t  value          = 0;

    ERR_RET_COND(MAP_FAILED == (vaddr = mmap_reg_space(paddr, size)), -(__LINE__), "Map 0xf2440014 failed.\n");

    value  = *vaddr;
    value &= ~UART0_MPP_CONF_MSK_1;
    value |=  UART0_MPP_CONF_FUN_1;
    *vaddr =  value;

    value  = *(vaddr + 1);
    value &= ~UART0_MPP_CONF_MSK_2;
    value |=  UART0_MPP_CONF_FUN_2;
    *(vaddr + 1) =  value;

    printf("Uart0 mpp config:\n");
    printf("  0x%08X : 0x%08x\n", UART0_MPP_CONF_REG_1, *vaddr);
    printf("  0x%08X : 0x%08x\n", UART0_MPP_CONF_REG_2, *(vaddr + 1));
    munmap_reg_space(vaddr, paddr, size);
    return 0;
}

#define UART0_CHK_STR   "Bonjour, this is Hightower Chrysler"
#define UART0_DEV       "/dev/ttyS1"

static int uart0_set_attr(int fd)
{
    struct termios tty;

    tcgetattr(fd, &tty);
    cfmakeraw(&tty);
    tcsetattr(fd, TCSANOW, &tty);
    return 0;
}

static void *uart0_chk(void *arg)
{
    int fd  = 0;
    int idx = 0;
    int jdx = 0;
    int len = 0;
    int ret = 0;
    char buf[4096] = {[0 ... sizeof(buf)-1] = 0};
    char *p = NULL;

    fd_set set;
    struct timeval timeout;

    ERR_RET_COND(0 > (fd = open(UART0_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC)),
                (void *)__LINE__, "Open %s failed.\n", UART0_DEV);

    uart0_set_attr(fd);

    while(idx < 20) {
        p = buf + len;
        for(jdx = 0; jdx < strlen(UART0_CHK_STR); jdx++) {

            FD_ZERO(&set);
            FD_SET(fd, &set);
            timeout.tv_sec  = 1;
            timeout.tv_usec = 0;

            write(fd, UART0_CHK_STR + jdx, 1);

            ret = select(fd + 1, &set, NULL, NULL, &timeout);
            if (ret == -1) {
                perror("Select erro\n");
            }
            else if (ret == 0) {
                printf("Timeout wait data\n");
                break;
            }
            else {
                len += read(fd, buf + len, sizeof(buf) - len - 1);
                if (len >= sizeof(buf) - 1) {
                    printf("buffer full\n");
                    goto _DONE;
                    break;
                }
            }
        }
        printf("%s\n", p);
        idx++;
    }
_DONE:

    if (strstr(buf, UART0_CHK_STR))
        *((int *)arg) = PASSED;
    else
        *((int *)arg) = FAILED;

    close(fd);
    return (void *)0;
}

long highrise_uart_test(void)
{
    int chk = 0;
    char *tname = "AUX loopback";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the external loopback test\n");
        return (PASSED);
    }

    printf("!!!Note: Please loopback the uart to do this test.\n");

    ERR_RET_COND(0 > set_mpp_uart0_func(), FAILED, "Failed to set uart0 mpp sel.\n");

    uart0_chk(&chk);

    if (chk == PASSED) { 
        printf("PASSED\n");
    } else { 
        cterr('f', 0, "FAILED\n");
    } 

    return chk;
}

/*********************************************************************
 * $Log: platform_uart.c,v $
 * Revision 1.2  2021/06/02 02:56:22  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

