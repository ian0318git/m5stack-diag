/* $Id: tsn_comm.c,v 1.3 2018/11/23 08:49:53 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/tsn_comm.c,v $ 
 *------------------------------------------------------------------
 * 
 * tsn_comm.c
 *
 * Copyright (c) 2017 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "menu.h"
#include "nvmonvars.h"
#include "proto.h"
#include "queryflags.h"
#include "tsn_comm.h"


/*******************************************************************************
 *                             Functions Declaration                           *
 *******************************************************************************
 */
static void tsn_transmit_nc_request(int);
void        tsn_kill_all_nc(void);
void        tsn_nc_init_parms_file(void);
void        tsn_nc_dispatch_comm(char *, char *);
int         tsn_nc_get_parms(int, char *);
int         tsn_nc_dispatch_comm_is_ok(void);
int         tsn_mem_read32(uint, uint *);
int         tsn_mem_write32(uint, uint);
int         tsn_cpureg_rd_util(int);
int         tsn_cpureg_wr_util(int);
void        tsn_print_spining_wheel(int);
int         check_ext_lpbk_flag(void);

/*******************************************************************************
 *                               Global Variable                               *
 *******************************************************************************
 */
int tsn_module = TSN_WIFI_MODULE;

/*
 * CPU register access Utilities
 */
static submenu_xtable_t cpureg_utils_tbl[] = {
    {"CPU register Read", (type_t(*)())tsn_cpureg_rd_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"CPU register Write", (type_t(*)())tsn_cpureg_wr_util, 0, 0,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define CPUREG_UTILS_TBL_SIZE (sizeof(cpureg_utils_tbl) / sizeof(submenu_xtable_t))

/* CPU register access Utils items (filled in from xtable) */
static mitem_t cpureg_utils_pri_items[CPUREG_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t cpureg_utils_sec_items[CPUREG_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

/* CPU register access Utils submenu */
menuinfo_t cpureg_utils_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    cpureg_utils_pri_items,
};
menuinfo_t *cpureg_utils_menup = &cpureg_utils_menu;


/*******************************************************************************
 *                                    Functions                                *
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function    : tsn_cpureg_rd_util
 * Description : Utility to read TSN CPU register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_cpureg_rd_util (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address (0x0 ~ 0xffffffff): ",
                               0, 0, 0xffffffff);

    if (tsn_mem_read32(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("0x%08X = 0x%08X\n", reg_offset, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_cpureg_wr_util
 * Description : Utility to write TSN CPU register.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_cpureg_wr_util (int opt)
{
    uint reg_offset = 0, orig_val = 0, reg_val = 0;
    
    reg_offset = gethex_answer("Enter register address(0x0 ~ 0xffffffff): ",
                               0, 0, 0xffffffff);

    if (tsn_mem_read32(reg_offset, &orig_val) != PASSED) {
        printf("Failed to read FPGA register 0x%08X.\n", reg_offset);
        return (FAILED);
    }

    reg_val = gethex_answer("Enter write-in data(hex): ",
                            orig_val, 0, 0xffffffff);

    if (tsn_mem_write32(reg_offset, reg_val) != PASSED) {
        printf("Failed to write CPU register 0x%08X.\n", reg_offset);
        return (FAILED);
    } else {
        printf("Done writing 0x%08X to CPU register(0x%08X).\n",
               reg_val, reg_offset);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_cpureg_utils
 * Description : Function to access TSN CPU register utilities submenu.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_cpureg_utils (int opt)
{
    build_primary_submenu(cpureg_utils_tbl, CPUREG_UTILS_TBL_SIZE,
                          "CPU register access Utilities", &cpureg_utils_menup);
    build_secondary_submenu(cpureg_utils_tbl, CPUREG_UTILS_TBL_SIZE,
                            cpureg_utils_sec_items);

    menu(cpureg_utils_menup, cpureg_utils_sec_items, '\0');

    return (PASSED);
}

/*****************************************************************
 *
 * Function: tsn_get_module_ip_addr
 *
 * Description: This function returns IP Address of module.
 *
 * Input:  ip_addr - Buffer to put ip address
 *
 * Output: None
 *
 *****************************************************************
 */
void tsn_get_module_ip_addr (char *ip_addr)
{
    char module_ip[TSN_NC_MAX_STR_SIZE];

    /* Sanity check */
    if (ip_addr == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }
    /* Check if DSL SKUs */
    if (tsn_module == TSN_DSL_MODULE) {
        sprintf(module_ip, "%s.%d", TSN_DIAG_DSL_SUBNET_STR,
                TSN_DIAG_MODULE_IP_ADDR);
    } else {
        sprintf(module_ip, "%s.%d", TSN_DIAG_WIFI_SUBNET_STR,
                TSN_DIAG_MODULE_IP_ADDR);
    }

    sprintf(ip_addr, "%s", module_ip);
}

/*******************************************************************************
 *
 * Function: tsn_nc_init_parms_file
 *
 * Description: This function clears out the content of parameters file
 *              and listen to a specific port for connections.
 *
 * Input:  None
 *
 * Output: None
 *
 *******************************************************************************
 */
void tsn_nc_init_parms_file (void)
{
    char cmd[128];
    char parms_file[64];

    sprintf(parms_file, DIAG_TSN_NC_TMP_PARMS_FILE);
    sprintf(cmd, "echo ' ' > %s", parms_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -p %d > %s &",
            DIAG_TSN_NC_RTN_PARMS_PORT_BASE, parms_file);
    system(cmd);
}


/*****************************************************************
 *
 * Function: tsn_nc_dispatch_comm
 *
 * Description: This function transmits nc client request to module.
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
void tsn_nc_dispatch_comm (char *comm, char *parms_str)
{
    char cmd[128];

    /* Sanity check */
    if (comm == NULL) {
        printf("%s: NULL Pointer\n", __FUNCTION__);
        return;
    }

    /* Prepare command and listen for module to grab */
    sprintf(cmd, "echo %s,%s, > %s",
            comm, parms_str, DIAG_TSN_NC_COMMAND_DISPATCH_FILE);
    system(cmd);

    sprintf(cmd, "nc -l -p %d < %s &",
            DIAG_TSN_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_TSN_NC_COMMAND_DISPATCH_FILE);
    system(cmd);

    tsn_transmit_nc_request(DIAG_TSN_NC_EXECUTE_COMMAND_PORT_BASE);
}

/*******************************************************************************
 *
 * Function: tsn_nc_get_parms
 *
 * Description: This function extract the return parameters that in the
 *              parms file.
 *
 * Input:  parms_num: Which parameter in the parms file shall be extracted.
 *         input: The parameter that be extracted out.
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_nc_get_parms (int parms_num, char *input)
{
    FILE *fp;
    char parms_file[64];
    char buf[128];
    char cmd[128];
    int curr_parm_num = 0;
    char *token = NULL;
    int ix = 0, repeat = 100;

    sprintf(parms_file, DIAG_TSN_NC_TMP_PARMS_FILE);

    for (ix = 0; ix <= repeat; ix++) {
        fp = fopen(parms_file, "r");

        if (fgets(buf, sizeof(buf), fp) == NULL) {
            fclose(fp);
            mdelay(10);  /* delay 10 ms before retries to reopen file */
            if (ix == repeat) { /* Max wait 1 sec */
                printf("counter:%d\n", ix); 
                printf("%s: Open %s fails\n", __FUNCTION__,
                      parms_file);
                printf("Nothing in buffer\n");
                goto __exit;
            }
        }else {
            break;
        }
    }

    /* Proxy the return parameters */
    token = strtok(buf, ",");
    
    while (token != NULL) {
        if (curr_parm_num == parms_num) {
            break;
        }
        curr_parm_num++;
        token = strtok(NULL, ",");
    }

    strcpy(input, token);

    fclose(fp);
    return (PASSED);

    __exit:
    printf("%s: Fgets fails\nContent of '%s':\n", __FUNCTION__, parms_file);
    fflush(stdout);
    
    sprintf(cmd, "cat %s", parms_file);
    system(cmd);

    return (FAILED);
}

/*******************************************************************************
 *
 * Function: tsn_nc_dispatch_comm_is_ok
 *
 * Description: This function checks the NC response is good or not.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_nc_dispatch_comm_is_ok(void)
{
    char buff[64];
    int retval = FAILED;

    /* First token is to indicate the result of the opcode */
    retval = tsn_nc_get_parms(0, buff);

    if (retval == FAILED) {
        return (FAILED);
    }

    if (strcmp(buff, DIAG_TSN_NC_RTN_PASS_STR)) {
        return (FAILED);
    }

    return (PASSED);
}

/***********************************************************************
 *  Static Functions
 ************************************************************************/

/*****************************************************************
 *
 * Function: tsn_transmit_nc_request
 *
 * Description: This function transmits nc client request to module
 *              on provided port number
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
static void tsn_transmit_nc_request (int port)
{
    char cmd[32];
    char module_ipaddr[32];

    tsn_get_module_ip_addr(module_ipaddr);

    sprintf(cmd, "nc %s %d", module_ipaddr, port);
    system(cmd);
}

/*****************************************************************
 *
 * Function: tsn_kill_all_nc
 *
 * Description: This function lists all process and grep 'nc -l'
 *              keyword, and dump it to temporary directory so we
 *              can kill them afterwards
 *
 * Input:  None
 *
 * Output: None
 *
 *****************************************************************
 */
void tsn_kill_all_nc (void)
{
    char cmd[64];
    char buf[128];
    char pid_file[32];
    char *token;
    int pid;
    FILE *fp;
    struct stat sts;

    /* Clear the content of the file first */
    sprintf(cmd, "echo ' ' > %s", DIAG_TSN_NC_KILL_TMP_FILE);
    system(cmd);
    sprintf(cmd, "ps | grep 'nc 192\\|nc -l' > %s", DIAG_TSN_NC_KILL_TMP_FILE);
    system(cmd);

    fp = fopen(DIAG_TSN_NC_KILL_TMP_FILE, "r");
    if (fp == NULL) {
        printf("%s: Open '%s' Failed\n", __FUNCTION__,
        	   DIAG_TSN_NC_KILL_TMP_FILE);
        return;
    }

    /* Check the result
     */
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        token=strtok(buf, " ");
        pid = atoi(token);
        /* Check if this process is still alive */
        sprintf(pid_file, "/proc/%d", pid);
        if (stat(pid_file, &sts) == -1) {
            /* Process doesn't exist */
            continue;
        }
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }

    fclose(fp);
}

/*******************************************************************************
 *
 * Function    : tsn_mem_read32
 * Description : Function to read TSN memory by byte.
 * Inputs      : offset - memory offset
 *               *buf   - buffer to put read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_mem_read32 (uint offset, uint *buf)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;
 
    fd = open("/dev/mem", (O_RDONLY | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *buf = *(volatile uint32_t*)virt_addr;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_mem_write32
 * Description : Function performs write TSN memory by byte.
 * Inputs      : offset  - offset
 *               wr_data - data for write
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_mem_write32 (uint offset, uint wr_data)
{
    int      fd = -1;
    void     *map_base, *virt_addr;
    off_t    target = 0;
    unsigned map_size, page_size, offset_in_page;

    target = (off_t)offset;
 
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd < 0) {
        printf("%s: Failed to open MEM device.\n", __FUNCTION__);
        return (FAILED);
    }

    map_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);
    if (offset_in_page + (8 * sizeof(int)) > page_size) {
        map_size *= 2;
    }

    map_base = mmap(NULL, map_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                    target & ~(off_t)(page_size -1));
    if (map_base == MAP_FAILED) {
            printf("%s: Failed to map in virtual address space.\n",
                   __FUNCTION__);
            close(fd);
            return (FAILED);
    }

    virt_addr = (char *)map_base + offset_in_page;
    *(volatile uint32_t*)virt_addr = wr_data;

    if (munmap(map_base, map_size) == -1) {
        printf("%s: Failed to munmap.\n", __FUNCTION__);
        close(fd);
        return (FAILED);
    }
    close(fd);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_rx_polling_uart
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 * Inputs     : *tty_dev  - device string (ie /dev/ttyS0, ..., /dev/ttyS2)
 *              *comp_str - compared string
 *              timeout   - timeout value (ms)
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
{
    int            uart_fd = -1, cnt;
    struct timeval read_timeout;
    fd_set         set;
    char           buf[1024];
    char           *search_str;
    int            rc;
    struct timeval start_time, curr_time;
    int            elapsed_time_in_ms;

    /* Sanity check */
    if (tty_dev == NULL || comp_str == NULL) {
        printf("%s: Null pointer.\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);
    if (uart_fd < 0) {
        printf("%s: Failed to open %s.\n", __FUNCTION__, tty_dev);
        fflush(stdout);
        return (FAILED);
    }

    gettimeofday(&start_time, NULL);
    do {
        memset(buf, 0, sizeof(buf));
        /* Set timeout on file descriptor */
        FD_ZERO(&set);
        FD_SET(uart_fd, &set);

        read_timeout.tv_sec  = SKYE_UART_READ_TIMEOUT;
        read_timeout.tv_usec = 0;

        rc = select(uart_fd + 1, &set, NULL, NULL, &read_timeout);

        if (rc == -1) { /* Error occured */
            perror("select");
            fflush(stdout);
            goto exit_poll_uart;
        }

        if (FD_ISSET(uart_fd, &set)) {
            /* Now, we read the buffer */
            cnt = read(uart_fd, buf, 255);
            if (cnt < 0) {
                perror("Read error");
                fflush(stdout);
                goto exit_poll_uart;
            }
            
            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                close(uart_fd);
                return (PASSED);
            }
        }

        /* Now check if elapsed time exceeds timeout value */
        gettimeofday(&curr_time, NULL);

        elapsed_time_in_ms = (curr_time.tv_sec - start_time.tv_sec) * 1000;
        elapsed_time_in_ms += (curr_time.tv_usec - start_time.tv_usec) / 1000;

        if (elapsed_time_in_ms > timeout) {
            goto exit_poll_uart;
        }

        msleep(1);
    } while (1);

exit_poll_uart:
    close(uart_fd);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function   : tsn_tx_uart
 * Description: Function to transmit strings into tty console.
 * Inputs     : *tty_dev - device string(ie /dev/ttyS0, ..., /dev/ttyS2)
 *              *in_str  - input string
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_tx_uart (char *tty_dev, char *in_str)
{
    int uart_fd = -1;
    int ret_val = PASSED;

    /* Sanity check */
    if (tty_dev == NULL || in_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_WRONLY);
    if (uart_fd < 0) {
        printf("%s: Failed to open %s\n", __FUNCTION__, tty_dev);
        fflush(stdout);
        return (FAILED);
    }

    if (write(uart_fd, in_str, strlen(in_str)) < 0) {
        printf("%s: write failed.\n", __FUNCTION__);
        ret_val = FAILED;
    }

    close(uart_fd);
    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : tsn_rx_uart
 * Description: Function to receive strings from tty console.
 * Inputs     : *tty_dev - device string(ie /dev/ttyS0, ..., /dev/ttyS2)
 *              *in_str  - input string
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_rx_uart (char *dev, int size, char *uart_buf, int timeout)
{
    struct timeval tv;
    fd_set rdfd;
    char *ptr;
    char tmp_buf[32];
    int uart_fd;
    int retval, total, cnt, pass;

    cnt = total = 0;

    /* Sanity check */
    if (dev == NULL || uart_buf == NULL) {
        printf("%s: Null pointer.\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(dev, O_RDONLY);
    if (uart_fd < 1) {
        printf("%s: Unable to open tty.\n", __FUNCTION__);
        fflush(stdout);
        return (FAILED);
    }

    ptr = uart_buf;
    pass = 0;
    while (1) {
        memset(&tv, 0, sizeof(tv));
        tv.tv_sec = 1;  /* minimal wait is 1 sec. can be more but not less. */
        tv.tv_usec = 0;
        FD_ZERO(&rdfd);
        FD_SET(uart_fd, &rdfd);
        retval = select(uart_fd + 1, &rdfd, NULL, NULL, &tv);
        if (FD_ISSET(uart_fd, &rdfd)) {
            do {
                cnt = read(uart_fd, ptr, 1);

                if (cnt < 0) {
                    perror("Unable to read from buffer\n");
                    fflush(stdout);
                    close(uart_fd);
                    return cnt;

                } else {
                    if (*ptr == '\n') {
                        break;
                    }

                    total += cnt;

                    /* we are interested only up to 'size' bytes. anything beyound that, we will store into
                       temp buffer and discard it. we need to keep reading to drain the buffer */
                    if ( (size > 0)  && (total >= size)) {
                        ptr = tmp_buf;
                    } else {
                        ptr += cnt;
                    }
                }
            } while (cnt > 0);
        } else {
            /* select() times out, no more characters, we are done. */
            if (total)
                break;
            /* need to exit if we have waited more than 'timeout' secs. */
            if ((timeout > 0) && (pass >= (timeout-1)))
                break;
            tsn_print_spining_wheel(pass++);
        }
        FD_CLR(uart_fd, &rdfd);
        if (total >= TSN_UART_BUF_SIZE) {
            break;
        }
    }

    close(uart_fd);

    return total;
}

/*******************************************************************************
 *
 * Function   : tsn_print_spining_wheel
 * Description: Display the spining wheel during the waiting time.
 * Inputs     : Ring cycle
 * Outputs    : none
 *
 *******************************************************************************
 */
void tsn_print_spining_wheel (int pass)
{
    static int idx = 0;

    if (pass < 0) {
        pass = idx++;
    }

    printf("\b");
    switch (pass%8) {
    case 0:
        printf("|");
        break;
    case 1:
        printf("/");
        break;
    case 2:
        printf("-");
        break;
    case 3:
        printf("\\");
        break;
    case 4:
        printf("|");
        break;
    case 5:
        printf("/");
        break;
    case 6:
        printf("-");
        break;
    case 7:
        printf("\\");
        break;
    default:
        break;
    }
    fflush(stdout);
    printf("\r");
}

/*******************************************************************************
 *
 * Function   : check_ext_lpbk_flag
 * Description: Function to check if Ext. Loopback Flag is ON or not.
 * Inputs     : None
 * Outputs    : TRUE(ON) / FALSE(OFF)
 *
 *******************************************************************************
 */
int check_ext_lpbk_flag (void)
{
    /* according to menu_show_dflags(), D_EXT_LPBK is inverse flag */ 
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        return (FALSE);
    } else { 
        return (TRUE);
    }
}


/*-------------------------------------------------
 * $Log: tsn_comm.c,v $
 * Revision 1.3  2018/11/23 08:49:53  hondwang
 * Re-instruct pluggable common code with CDETs CSCvn17216
 *
 * Revision 1.2.80.1  2018/10/15 06:53:08  hondwang
 * pluggable common code re-instruct modify code
 *
 * Revision 1.2  2017/08/02 14:21:50  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.3  2017/08/01 14:02:04  steja
 * Enhanced Wifi Diag Kernel boot up
 *
 * Revision 1.1.8.2  2017/07/29 03:41:21  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.3  2017/07/24 14:14:11  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:08  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.7  2016/10/02 20:32:27  palin2
 * Enhanced WiFi uart code to fix CSCvb53793.
 *
 * Revision 1.1.4.6  2016/09/09 06:30:15  steja
 * enhanced vdsl nc comm (CSCva67484)
 *
 * Revision 1.1.4.5  2016/08/16 03:08:17  palin2
 * Unified test pass print outs.
 *
 * Revision 1.1.4.4  2016/07/17 11:15:16  palin2
 * Added function to distinguish bwteen TSN-H and TSN-M.
 *
 * Revision 1.1.4.3  2016/06/30 14:06:32  steja
 * Pick up the latest from tsn-branch1
 *
 * Revision 1.1.4.2  2016/06/30 06:22:52  steja
 * tsn-branch2 sync with main trunk
 *
 * Revision 1.1.2.5  2016/06/29 14:14:51  palin2
 * 1. Updated code to support TSN-M.
 * 2. Added utility to set LAN PHY 1000Base-T Test mode.
 *
 * Revision 1.1.2.4  2016/06/17 15:26:25  palin2
 * Added WLAN module diags and utilities.
 *
 * Revision 1.1.2.3  2016/04/19 07:37:59  palin2
 * Added utilities to access TSN CPU register(s).
 *
 * $Endlog$
 *-------------------------------------------------
 */
