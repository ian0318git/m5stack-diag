/* $Id: sm_woodlawn_uart.c,v 1.3 2014/02/18 09:11:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn/sm_woodlawn_uart.c,v $
 *------------------------------------------------------------------
 * Filename: sm_woodlawn_uart.c
 *
 * Description: SM Woodlawn UART Library
 * Author: Times Huang
 *
 * Copyright (c) 2013-2014 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "types.h"
#include "defs.h"
#include "signals.h"
#include "error.h"
#include "nvmonvars.h"
#include "nvsysvars.h"
#include "pm_utils.h"
#include "pci.h"
#include "slot.h"
#include "mon_plat_defs.h"
#include "common.h"
#include "common_utils.h"
#include "proto.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>

#include "sm_woodlawn_uart.h"

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/


/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
int woodlawn_rx_polling_uart(char *, char *, int);
int woodlawn_tx_uart(char *, char *);
int woodlawn_uart_setup(char *);

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
 * Function: woodlawn_rx_polling_uart
 *
 * Description: This function reads data from uart controller, and return
 *              pass if the input string is found. If the string can't be
 *              found after timeout, then return failure.
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         comp_str: compared string
 *         timeout: timeout value (ms)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */
int woodlawn_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
{
    int uart_fd, cnt;
    struct timeval read_timeout;
    fd_set set;
    char buf[1024];
    char *search_str;
    int rc;
    struct timeval start_time, curr_time;
    int elapsed_time_in_ms;

    /* Sanity check */
    if (tty_dev == NULL || comp_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_RDWR|O_NOCTTY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    gettimeofday(&start_time, NULL);

    do {
        /* Set timeout on file descriptor */
        FD_ZERO(&set);
        FD_SET(uart_fd, &set);

        read_timeout.tv_sec  = WOODLAWN_UART_READ_TIMEOUT;
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

/*****************************************************************
 *
 * Function: woodlawn_tx_uart
 *
 * Description: This function transmits strings into tty
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *         out_str: compared string
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */

int woodlawn_tx_uart (char *tty_dev, char *out_str)
{
    int uart_fd, cnt;
    int rc = PASSED;

    /* Sanity check */
    if (tty_dev == NULL || out_str == NULL) {
        printf("%s: Null pointer\n", __FUNCTION__);
        return (FAILED);
    }

    uart_fd = open(tty_dev, O_WRONLY);

    if (uart_fd < 0) {
        perror("polling uart: open tty failed");
        fflush(stdout);
        return (FAILED);
    }

    cnt = write(uart_fd, out_str, strlen(out_str));

    if (cnt < 0) {
        perror("tx_uart: write failed\n");
        rc = FAILED;
    }

    close(uart_fd);
    return (rc);
}

/*****************************************************************
 *
 * Function: woodlawn_uart_setup
 *
 * Description: This function setups UART parameter
 *
 * Input:  tty_dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
 *
 * Output: PASSED
 *         FAILED
 *
 *****************************************************************
 */

int woodlawn_uart_setup (char *tty_dev)
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

/***********************************************************************
 *  Static Functions
 ************************************************************************/


/*------------------------------------------------------------------
 * $Log: sm_woodlawn_uart.c,v $
 * Revision 1.3  2014/02/18 09:11:12  alpeng
 * CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h
 *
 * Revision 1.2  2013/10/08 08:48:26  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:49  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.2  2013/04/24 07:27:38  tirawan
 * Fix intermittent boot up issue
 *
 * Revision 1.1.2.1  2013/04/03 05:46:40  tirawan
 * Add auto boot by UART function, and auto run by nc utility
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

