/* $Id: uart_api.c,v 1.2 2017/08/02 14:21:50 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/uart_api.c,v $
 *------------------------------------------------------------------
 * uart_api.c
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
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
#include "uart_api.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/


/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int tsn_console_switch(struct uart_parm *);
int tsn_uart_rx_polling (int, char *, int);
int tsn_uart_tx(int, char *);
int tsn_uart_setup(char *);

/***********************************************************************
 *  Externs
 ************************************************************************/


/***********************************************************************
 *  Global Variable
 ************************************************************************/


/***********************************************************************
 *  Functions
 ************************************************************************/

/*****************************************************************
 *
 * Function   : tsn_console_switch
 *
 * Description: This function provides console redirect
 *
 * Inputs     : u_parm: UART command parameters
 *
 * Outputs    : PASSED/FAILED
 *
 *****************************************************************
 */
int tsn_console_switch(struct uart_parm *u_parm)
{
    const int maxlen = 128;
    char cmd[maxlen];

    printf("\n\n ### NOTE: Type CTRL-a followed by CTRL-x "
                          "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(WAIT_SCREEN_PRINT); // pause a second for the NOTE:

    snprintf(cmd, maxlen-1, "picocom -b%d -d%d -p%s -f%s %s",
    		 u_parm->baudrate, u_parm->databit, u_parm->parity,
    		 u_parm->flow, u_parm->tty_dev);

    system(cmd);

    return(PASSED);
}

/*****************************************************************
 *
 * Function: tsn_uart_rx_polling
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
int tsn_uart_rx_polling (int fd, char *comp_str, int timeout)
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

        read_timeout.tv_sec  = TSN_UART_READ_TIMEOUT;
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
 * Function: tsn_uart_tx
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
int tsn_uart_tx (int fd, char *out_str)
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
 * Function: tsn_uart_setup
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
int tsn_uart_setup (char *tty_dev)
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

/*-------------------------------------------------
$Log: uart_api.c,v $
Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.2  2016/06/30 06:22:52  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.1  2016/03/21 02:56:06  steja
Add debug card test items



*/
