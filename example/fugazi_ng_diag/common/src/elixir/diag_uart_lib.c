/* $Id: diag_uart_lib.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_uart_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_uart_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <fcntl.h>
#include "proto.h"
#include "common.h"
#include "diag_uart_lib.h"
#include "common_utils.h"

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int plat_console_switch(struct uart_parm *);
int plat_uart_rx_polling (int, char *, int);
int plat_uart_tx(int, char *);
int plat_uart_setup(char *);
static void plat_print_spining_wheel(int);

/*****************************************************************
 *
 * Function   : plat_console_switch
 *
 * Description: This function provides console redirect
 *
 * Inputs     : u_parm: UART command parameters
 *
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************
 */
int plat_console_switch (struct uart_parm *u_parm)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
                          "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(WAIT_SCREEN_PRINT);

    snprintf(cmd, maxlen-1, "picocom -b%d -d%d -p%s -f%s %s",
    		 u_parm->baudrate, u_parm->databit, u_parm->parity,
    		 u_parm->flow, u_parm->tty_dev);

    system(cmd);

    return (PASSED);
}

/*****************************************************************
 *
 * Function: plat_uart_rx_polling
 *
 * Description: This function polls data from UART interface,
 *              and return true if the compare string can be found.
 *
 * Input:  fd: UART file descriptor
 *         comp_str: compared string
 *         timeout: timeout value (ms)
 *
 * Output: TRUE: The string is found.
 *         FALSE: The input string can not be found before timeout.
 *
 *****************************************************************
 */
int plat_uart_rx_polling (int fd, char *comp_str, int timeout)
{
    int uart_fd = fd;
    int cnt;
    struct timeval read_timeout;
    fd_set set;
    char buf[1024];
    char *search_str;
    int rc;
    struct timeval start_time, curr_time;
    int elapsed_time_in_ms;

    /* Sanity check */
    if (comp_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FALSE);
    }

    gettimeofday(&start_time, NULL);

    do {
        /* Set timeout on file descriptor */
        FD_ZERO(&set);
        FD_SET(uart_fd, &set);

        read_timeout.tv_sec  = PLAT_UART_READ_TIMEOUT;
        read_timeout.tv_usec = 0;

        rc = select(uart_fd + 1, &set, NULL, NULL, &read_timeout);

        if (rc == -1) { /* Error occured */
            perror("select");
            fflush(stdout);
            return (FALSE);
        }

        if (FD_ISSET(uart_fd, &set)) {
        	memset(buf, 0, sizeof(buf));
            /* Now, we read the buffer */
            cnt = read(uart_fd, buf, 255);
            if (cnt < 0) {
                perror("Read error");
                fflush(stdout);
                return (FALSE);
            }

            /* Check if compared string can be found in the incoming string */
            search_str = strstr(buf, comp_str);
            if (search_str != NULL) { /* Found the string */
                return (TRUE);
            }
        }

        /* Now check if elapsed time exceeds timeout value */
        gettimeofday(&curr_time, NULL);

        elapsed_time_in_ms = (curr_time.tv_sec - start_time.tv_sec) * 1000;
        elapsed_time_in_ms += (curr_time.tv_usec - start_time.tv_usec) / 1000;

        if (elapsed_time_in_ms > timeout) {
        	return (FALSE);
        }

        msleep(1);
    } while (1);
}

/*****************************************************************
 *
 * Function: plat_uart_tx
 *
 * Description: This function transmits characters into UART interface.
 *
 * Input:  fd: UART file descriptor
 *         out_str: Characters to be transmitted into UART interface
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int plat_uart_tx (int fd, char *out_str)
{
    int uart_fd = fd;
    int cnt;

    /* Sanity check */
    if (out_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    cnt = write(uart_fd, out_str, strlen(out_str));

    if (cnt < 0) {
        perror("tx_uart: write failed\n");
        return (FAILED);
    }

    return (PASSED);
}

/*****************************************************************
 *
 * Function: plat_uart_setup
 *
 * Description: This function setups UART parameter.
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *
 * Output: PASSED/FAILED
 *
 *
 *****************************************************************
 */
int plat_uart_setup (char *tty_dev)
{
    int uart_fd;
    struct termios tio_setting;

    /* Sanity check */
    if (tty_dev == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    tcgetattr(uart_fd, &tio_setting);

    tio_setting.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    tio_setting.c_iflag = IGNPAR | ICRNL;
    tio_setting.c_oflag = 0;
    tio_setting.c_lflag = ICANON;

    tcflush(uart_fd, TCIFLUSH);
    tcflush(uart_fd, TCOFLUSH);
    tcsetattr(uart_fd, TCSANOW, &tio_setting);
    close(uart_fd);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_tx_uart
 * Description: Function to transmit strings into tty console.
 * Inputs     : *tty_dev - device string(ie /dev/ttyS0, ..., /dev/ttyS2)
 *              *in_str  - input string
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_tx_uart (char *tty_dev, char *in_str)
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
 * Function   : plat_rx_uart
 * Description: Function to receive strings from tty console.
 * Inputs     : *tty_dev - device string(ie /dev/ttyS0, ..., /dev/ttyS2)
 *              *in_str  - input string
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_rx_uart (char *dev, int size, char *uart_buf, int timeout)
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
            plat_print_spining_wheel(pass++);
        }
        FD_CLR(uart_fd, &rdfd);
        if (total >= PLAT_UART_BUF_SIZE) {
            break;
        }
    }

    close(uart_fd);

    return total;
}

/*******************************************************************************
 *
 * Function   : plat_print_spining_wheel
 * Description: Display the spining wheel during the waiting time.
 * Inputs     : Ring cycle
 * Outputs    : none
 *
 *******************************************************************************
 */
void plat_print_spining_wheel (int pass)
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
 * Function   : plat_rx_polling_uart
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
int plat_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
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

        read_timeout.tv_sec  = PLAT_UART_READ_TIMEOUT;
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
                fflush(stdin);
                fflush(stdout);
                tcflush(uart_fd, TCIFLUSH);
                tcflush(uart_fd, TCOFLUSH);
                msleep(2000);
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
    fflush(stdin);
    fflush(stdout);
    tcflush(uart_fd, TCIFLUSH);
    tcflush(uart_fd, TCOFLUSH);
    msleep(2000);
    close(uart_fd);
    return (FAILED);
}

/*******************************************************************************
 *
 * Function: plat_nc_get_parms
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
int plat_nc_get_parms (int parms_num, char *input)
{
    FILE *fp;
    char parms_file[64];
    char buf[128];
    char cmd[128];
    int curr_parm_num = 0;
    char *token = NULL;
    int ix = 0, repeat = 100;

    sprintf(parms_file, DIAG_PLAT_NC_TMP_PARMS_FILE);

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
 * Function: plat_nc_init_parms_file
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
void plat_nc_init_parms_file (void)
{
    char cmd[128];
    char parms_file[64];

    sprintf(parms_file, DIAG_PLAT_NC_TMP_PARMS_FILE);
    sprintf(cmd, "echo ' ' > %s", parms_file);
    system(cmd);

    /* Listen to the command status */
    sprintf(cmd, "nc -l -p %d > %s &",
            DIAG_PLAT_NC_RTN_PARMS_PORT_BASE, parms_file);
    system(cmd);
}

/*-------------------------------------------------
 * $Log: diag_uart_lib.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2020/11/17 10:33:00  illiu
 * Fix wifi tftp boot test item
 *
 * Revision 1.1.2.1  2020/09/09 09:08:07  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
