/* $Id: linux_uart.c,v 1.19 2019/08/06 06:56:06 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_uart.c,v $
 *------------------------------------------------------------------
 * 5/2012 mcharon
 *
 * Copyright (c) 2010-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <pthread.h>
#include <sys/time.h>

#include "types.h"
#include "common.h"
#include "proto.h"
#include "error.h"
#include "linux_api.h"
#include "nvmonvars.h"

extern void print_spining_wheel(int);

char last_str[0];

/*
NAME: tx_uart.
Description: this function sends data to uart contrller.
INPUT: char *dev: device string, ie /dev/ttyDASH0.../dev/ttyDASH7
       char *uar_buf: contains data to be sent to uart controller.
       int init: set to true, if we want to init sstty; otherwise, set to faluse.
OUTPUT: if erorr, returns negative value provided by system call that failed.
             ; otherwise returns number bytes sent.
*/

int
tx_uart (char *dev, char *uart_buf, int init)
{
    int uart_fd, cnt;

    uart_fd = open(dev, O_WRONLY);
    if (uart_fd < 0) {
        perror("tx_uart: open tty failed.");
        return uart_fd;
    }
    cnt = write(uart_fd, uart_buf, strlen(uart_buf));
    if (cnt < 0) {
        perror("tx_uart: write failed.");
    }
    close(uart_fd);
    return cnt;

}

/*
NAME: rx_uart.
Description: this function reads data from uart controller.
INPUT: char *dev: device string, (ie /dev/ttyDASH0, .../dev/ttyDASH7)
       int size: how many bytes of data the calling function wants to receive.
                 if size is < 0, the function will store everyting that it reads from uart and
                 save it in 'uart_buf' variable; otherwise, function will store only 'size' bytes it
                 reads from uart FIFO.
       char *uar_buf: storage for data read from uart controller.
       int timeout: how long in secs to wait for data. if timeout < 0, this function will wait
                 forever, and  will not return to calling function (maybe useful for debugging.)
       uint exit_mode: for user to specify the exit character of receive strings. 
OUTPUT: if erorr, returns negative value provided by system call that failed.
             ; otherwise returns number bytes sent.
*/
int
rx_uart (char *dev, int size, char *uart_buf, int timeout, uint exit_mode)
{
    struct timeval tv;
    fd_set rdfd;
    char *ptr;
    char tmp_buf[32];
    int uart_fd;
    int total, cnt, pass;

    cnt = total = 0;

    memset(uart_buf, 0, sizeof(*uart_buf));

    uart_fd = open(dev, O_RDONLY);

    if (uart_fd < 1) {
        perror("unable to open tty");
        fflush(stdout);
        return uart_fd;
    }

    ptr = uart_buf;
    pass = 0;
    while (1) {
        memset(&tv, 0, sizeof(tv));
        tv.tv_sec = 1;  /* minimal wait is 1 sec. can be more but not less. */
        tv.tv_usec = 0;
        FD_ZERO(&rdfd);
        FD_SET(uart_fd, &rdfd);
        select(uart_fd + 1, &rdfd, NULL, NULL, &tv);
        if (FD_ISSET(uart_fd, &rdfd)) {
            do {
                cnt = read(uart_fd, ptr, 1);

                if (cnt < 0) {
                    perror("unable to read fro buffer\n");
                    fflush(stdout);
                    close(uart_fd);
                    return cnt;

                } else {
                    /* if we dont' check for carriage return, we are going to block at read(),
                       when there're no more bytes to be read.
                       by keying on '\n', we force the code to call 'select()' for each line of data
                       that uart receives. if select() times out, we know there are no more characters,
                       so the code can skip calling read().
                    */
                    if (SPECIAL_PAT == exit_mode) {
                        if (*ptr == last_str[0])
                            break;
                    } else if (TRIG_DIAG_M == exit_mode) {
                        /* the end of character on diag menu is SPACE, not arrow sign */
                        if (*ptr == ' ') {
                            break;
                        }
                    } else { /* DEFAULT_CASE */
                        if (*ptr == '\n')
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
            print_spining_wheel(pass++);
        }
        FD_CLR(uart_fd, &rdfd);
        if (total >= UART_BUF_SIZE) {
            break;
        }
    }

    close(uart_fd);

    //    printf("buff %s !!!\n", uart_buf);

    return total;

}

static void
* read_aux (void *u)
{
    int timeout = 5; /*in secs */
    int size = 0; /* when size= 0, read all bytes from uart controller */

    s_uart *uart = (s_uart *)u;

    if (rx_uart(uart->dev, size, (char *)uart->buf, timeout, uart->tst_typ) < 0) {

    }
    pthread_exit(NULL);
}

/* Function: uart_intf_test:
   Description: send string to uart and receive from uart interface.
   compare tx and rx string.  (similiar to loopback test.)
               INPUT: char * dev, name of device (ie, "/dev/ttyDASH0)
               OUTPUT: passed if successful; failed otherwise

*/

int
uart_intf_test (char *dev, const char *test_str, speed_t test_speed)
{
    struct termios config, ori_conf;
    int uart_fd, ret_val = PASSED, result = 0;
    char tx_str[100], tmp_str[100];
    char *pattern = "1234567890ABCDEF\n";
    pthread_t threads;

    s_uart uart;

    uart.dev =  dev;

    memset(uart.buf, '\0', sizeof(uart.buf));
    uart.tst_typ = DEFAULT_CASE;

    /* using specified pattern */
    if (test_str != NULL) {
        uart.tst_typ = SPECIAL_PAT;
        pattern = (char *)test_str;

        /* get last character for rx_uart() */
        sprintf(tmp_str, pattern);
        last_str[0] = tmp_str[strlen(tmp_str)- 1];
    }

    uart_fd = open(dev, O_WRONLY);
    if (uart_fd < 0) {
        cterr('f', 0, "failed to open %s", dev);
        return FAILED;
    }

    if (tcgetattr(uart_fd, &config) < 0) {
        close(uart_fd);
        cterr('f', 0, "uart_intf_test(): Failed in tcgetattr() %d", __LINE__);
        return (FAILED);
    }

    /* Backup default config for recover after test */
    memcpy(&ori_conf, &config, sizeof(config));

    if ((config.c_cflag & CBAUD) != test_speed) {
        if (cfsetospeed(&config, test_speed) < 0) {
            tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
            close(uart_fd);
            cterr('f', 0, "uart_intf_test(): Failed to set output speed.");
            return (FAILED);
        }

        if (cfsetispeed(&config, test_speed) < 0) {
            tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
            close(uart_fd);
            cterr('f', 0, "uart_intf_test(): Failed to set intput speed.");
            return (FAILED);
        }
    }

    config.c_lflag &= ~(ICANON|IEXTEN|ISIG|ECHO);
    config.c_iflag |= IGNCR;
    config.c_oflag &= ~(OPOST);

    if (tcsetattr(uart_fd, TCSAFLUSH, &config) < 0) {
        tcsetattr(uart_fd, TCSAFLUSH, &ori_conf); /*try to reset to orig value */
        close(uart_fd);
        cterr('f', 0, "\nuart_intf_test(): Failed in set new config values tcsetattr()");
        return (FAILED);
    }
    
    if(pthread_create(&threads, NULL, read_aux, (void *)&uart)) {
        perror("pthread_create failed.");  /* softeware bug. should never occur */
        /* Recover to original settings */
        result = tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
        if (result < 0) {
            cterr('f', 0, "uart_intf_test(): Failed in tcsetattr() %d", __LINE__);
        }
        close(uart_fd);
        return FAILED;
    }

    msleep(500);

    tx_uart(dev, pattern, 1);

    pthread_join(threads, NULL);

    if (!strlen(uart.buf)) {
        /* Recover to original settings */
        cterr('f', 0, "uart_intf_test(): No data received() %d", __LINE__);
        result = tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
        close(uart_fd);
        if (result < 0) {
            cterr('f', 0, "uart_intf_test(): Failed in tcsetattr() %d", __LINE__);
        }
        return FAILED;
    }

    /* don't compare carriage return */
    if (!strstr(uart.buf, pattern)) {
        /* strip carraige return not needed by cterr */
        sprintf(tx_str, pattern);
        tx_str[strlen(pattern)-1] = '\0';
        ret_val = FAILED;
        tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
        close(uart_fd);
        cterr('f', 0, "rx/tx string differ [rx = %s] [tx = %s].",
              uart.buf, tx_str);
        return FAILED;

    }

    ret_val = PASSED;
    /* Recover to original settings */
    result = tcsetattr(uart_fd, TCSAFLUSH, &ori_conf);
    if (result < 0) {
        /*software erro should coem here */
        perror("\nuart_intf_test(): data rx ok but Failed in tcsetattr()"); 
        ret_val = FAILED;
    }
    close(uart_fd);

    return (ret_val);

}

/* Function: uart_msg_exh_test
   Description: send string to uart and recieve expect msg from 
   from uart interface.
           INPUT: char * dev, name of device (ie, "/dev/ttyDASH0)
                  send_str, sent string (ie. "j\n" for trigger menu) 
                  exp_msg, expect string.
                  trig_typ, type of uart sting for rx_uart
           OUTPUT: passed if successful; failed otherwise
*/

int
uart_msg_exh_test (char *dev, const char *send_str, const char *exp_msg, uint trig_typ)
{
    struct termios config;
    int uart_fd;
    char sd_str[100], exp_str[100];
    char *sd_pattern, *exp_pattern; 
    pthread_t threads;

    s_uart uart;

    uart.dev =  dev;

    memset(uart.buf, '\0', sizeof(uart.buf));

    /* the expect strings are from diag menu
     * this is for set exit state on rx_uart
     */
    uart.tst_typ = trig_typ;


    sd_pattern = (char *)send_str;
    exp_pattern = (char *)exp_msg;

    uart_fd = open(dev, O_WRONLY);
    if (uart_fd < 0) {
        perror("\nuart_msg_exh_test(): open tty failed.");
        return uart_fd;
    }
    if (tcgetattr(uart_fd, &config) < 0) {
        perror("\nuart_msg_exh_test(): Failed in tcgetattr()\n");
        return (FAILED);
    }
    config.c_lflag &= ~(ICANON|IEXTEN|ISIG|ECHO);
    config.c_iflag |= IGNCR;
    config.c_oflag &= ~(OPOST);
    if (tcsetattr(uart_fd, TCSAFLUSH, &config) < 0) {
        perror("\nuart_msg_exh_test(): Failed in tcsetattr()\n");
        return (FAILED);
    }
    close(uart_fd);

    if(pthread_create(&threads, NULL, read_aux, (void *)&uart)) {
        perror("pthread_create failed.");
        printf("%s: pthread_create failed.\n", __FUNCTION__);
        return FAILED;
    }

    msleep(500);

    tx_uart(dev, sd_pattern, 1);

    pthread_join(threads, NULL);

    if (!strlen(uart.buf)) {
        printf("%s: Failed to receive data.\n", __FUNCTION__);
        return FAILED;

    }
    /* don't compare carriage return */
    if (!strstr(uart.buf, exp_pattern)) {
        sprintf(sd_str, sd_pattern);
        sprintf(exp_str, exp_pattern);

        printf("%s: failed. send/expect/reply string ", __FUNCTION__);
        printf("[sd = %s] [exp = %s] [rp = %s].\n",
               sd_str, exp_str, uart.buf);
        return FAILED;
    }

    return (PASSED);
}
/*****************************************************************
 *
 * Function: dash_tx_uart
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

int dash_tx_uart (char *tty_dev, char *out_str)
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
 * Function: dash_rx_polling_uart
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
int dash_rx_polling_uart (char *tty_dev, char *comp_str, int timeout)
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

        read_timeout.tv_sec  = 1;
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
            if (diagflag_xram & D_DEBUG_OPTIONS) {
                printf("[%s]",buf);
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
        msleep(10);
    } while (1);

exit_poll_uart:
    close(uart_fd);

    return (FAILED);
}

/*****************************************************************
 *
 * Function: dash_uart_setup
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

int dash_uart_setup (char *tty_dev)
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
$Log: linux_uart.c,v $
Revision 1.19  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.18.46.1  2019/05/27 03:21:49  yozou
CSCvp78555: Curie - Switzer10G integration

Revision 1.18  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.17  2016/10/16 12:28:12  iachang
Supported Goldbeach Platform.

Revision 1.16  2014/05/19 23:27:00  mcharon
if rx byte is greater than uart_buf_size, get out of loop

Revision 1.15  2013/02/15 10:31:47  palin2
Update UART test by add a new parameter to allow using specific baud rate.

Revision 1.14  2012/11/27 01:30:15  alpeng
show send/expect/reply string when uart_msg_exh_test failed

Revision 1.13  2012/10/04 23:53:14  mcharon
dont detach thread in uart_msg_exh_test; otherwise pthread_join won't work

Revision 1.12  2012/10/02 22:25:31  mcharon
do not detach receive thread

Revision 1.11  2012/10/01 22:06:19  ptong
Correct function name used in perror

Revision 1.10  2012/09/24 05:58:25  alpeng
add argument for rx_uart(), for getting last character on rx

Revision 1.9  2012/09/17 08:28:56  alpeng
revert uart_intf_test(), add uart_msg_exh_test()

Revision 1.8  2012/09/12 02:34:08  alpeng
do cavium uart loopback test with trigger cavium diag item

Revision 1.7  2012/08/22 21:27:57  srane
system call to "stty ..." after pthread causes seg fault in uart loopback
test after initial O2 bootup.

Revision 1.6  2012/08/22 09:32:40  alpeng
supporting uart test for cavium

Revision 1.5  2012/08/08 22:19:00  palin2
Update error reporting format of UART test, uart_intf_test.

Revision 1.4  2012/07/27 17:10:15  mcharon
move uart test to common code

Revision 1.3  2012/07/25 00:43:07  mcharon
add size and timeout argument for uart_rx

Revision 1.2  2012/07/25 00:12:15  huanngo
Fix uart test with new linux kernel

Revision 1.1  2012/06/01 22:28:25  mcharon
add api for uart interface test


$Endlog$
*/

