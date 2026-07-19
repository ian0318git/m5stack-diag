/* $Id: platform_wifi.c,v 1.3 2018/05/15 09:37:32 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_wifi.c,v $ 
 *------------------------------------------------------------------
 *
 * Filename   : platform_wifi.c
 * Description: File for TSN WiFi related library functions.
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "common.h"
#include "common_utils.h"
#include "proto.h"
#include "wifi_tests.h"
#include "uart_api.h"
#include "tsn_comm.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_wifi.h"


/*******************************************************************************
 *                            Function Prototypes                              *
 *******************************************************************************
 */
int        tsn_release_wifi_from_reset(void);
void       tsn_setup_wlan_uart(void);
int        tsn_wifi_console_switch(void);
int        tsn_wifi_led_control(int);
int        tsn_wifi_nc_dispatch_comm(char *);
int        tsn_put_wifi_in_reset(void);
int        tsn_reset_wifi(void);
int        tsn_wifi_nc_get_test_result(int *, char *);
static int popen_cmd_exec_wrap(char *);

/*******************************************************************************
 *                              Global variables                               *
 *******************************************************************************
 */
extern boolean wifi_booted;

#define TSN_NC_MAX_RETRY 20
/*******************************************************************************
 *                                  Functions                                  *
 *******************************************************************************
 */
/*******************************************************************************
 *
 * Function   : tsn_wifi_nc_get_test_result
 * Description: Function to get TSN WiFi NC Diag test result and
 *              error message from WiFi if have.
 * Inputs     : cmd - WIFI_LED_GREEN/WIFI_LED_RED/WIFI_LED_AMBER/WIFI_LED_OFF
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_wifi_nc_get_test_result (int *result, char *err_msg)
{
    FILE *fp;
    char buf[TSN_NC_BUF_SIZE];
    int  ret_val = PASSED;

    memset(buf, 0, sizeof(buf));

    /* 1. Check NC exec result file */
    if ((fp = fopen(TSN_NC_DONE_FILE, "r")) == NULL) {
        printf("%s(%d): Can't open file %s\n",
               __func__, __LINE__, TSN_NC_DONE_FILE);
        return (FAILED);
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "PASSED") == 0) {
            *result = PASSED;
            break;
        } else {
            *result = FAILED;
        }
    }

    if (fclose(fp) != 0) {
        printf("%s(%d): Filed to do fclose - %s.\n",
               __func__, __LINE__, strerror(errno));
        return (FAILED);
    }

    if (*result == PASSED) {
        return (PASSED);
    }

    /* 2. Get error message if needed */
    /* 2.1 Clean up buffer */
    memset(buf, 0, sizeof(buf));

    /* 2.2 Get error message */
    if ((fp = fopen(TSN_NC_EXEC_LOG_FILE, "r")) == NULL) {
        printf("%s(%d): Can't open file %s\n",
               __func__, __LINE__, TSN_NC_DONE_FILE);
        return (FAILED);
    }

    if (fgets(err_msg, sizeof(err_msg), fp) == NULL) {
        printf("%s(%d): Failed to get NC execution error message.\n",
               __func__, __LINE__);
        ret_val = FAILED;
    } else {
        ret_val = PASSED;
    }

    if (fclose(fp) != 0) {
        printf("%s(%d): Filed to do fclose - %s.\n",
               __func__, __LINE__, strerror(errno));
        return (FAILED);
    }

    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : tsn_wifi_nc_dispatch_comm
 * Description: Function dispatches command to TSN WiFi through nc command.
 * Inputs     : *cmd_str - command that will be transfered to WiFi module
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_wifi_nc_dispatch_comm (char *cmd_str)
{
    char exec_cmd[128];
    char tty_dev[32];
    FILE *fp;
    char buf[TSN_NC_BUF_SIZE];
    int  ret_val = FAILED;
    int  w_time = 0;

    memset(buf, 0, sizeof(buf));
    memset(exec_cmd, 0, sizeof(exec_cmd));
    memset(tty_dev, 0, sizeof(tty_dev));
    sprintf(tty_dev, "/dev/ttyS2");

    /* Sanity check */
    if (cmd_str == NULL) {
        printf("%s(%d): *cmd_str is NULL.\n", __func__, __LINE__);
        return (FAILED);
    }

    /* Prepare NC command */
    if ((fp = fopen(DIAG_TSN_NC_COMMAND_DISPATCH_FILE, "w")) == NULL) {
        printf("%s(%d): Can't open/create file %s\n",
               __func__, __LINE__, DIAG_TSN_NC_COMMAND_DISPATCH_FILE);
        return (FAILED);
    }

    fprintf(fp, "%s;", cmd_str);

    if (fclose(fp) != 0) {
        printf("%s(%d): Filed to do fclose - %s.\n",
               __func__, __LINE__, strerror(errno));
        return (FAILED);
    }

    /* Pulling WiFi module side ready signal */
    memset(exec_cmd, 0, sizeof(exec_cmd));
    sprintf(exec_cmd, "echo OK | netcat -c -v -v %s %d >& %s",
                      TSN_WIFI_IPADDR,
                      TSN_WIFI_NC_RDY_PORT,
                      TSN_WIFI_NC_RDY_FILE);
    for (w_time = 0;
         w_time < TSN_WIFI_NC_TIMEOUT;
         w_time+= TSN_WIFI_NC_PULL_INTVL) {

        if (popen_cmd_exec_wrap(exec_cmd) != PASSED) {
            printf("%s(%d): Failed to execute command \"%s\".\n",
                   __func__, __LINE__, exec_cmd);
            return (FAILED);
        }

        memset(buf, 0, sizeof(buf));

        if ((fp = fopen(TSN_WIFI_NC_RDY_FILE, "r")) == NULL) {
            printf("%s(%d): Can't open file %s for read.\n",
                   __func__, __LINE__, TSN_NC_DONE_FILE);
            return (FAILED);
        }

        while (fgets(buf, sizeof(buf), fp) != NULL) {
            if (strstr(buf, "open") != NULL) {
                ret_val = PASSED;
                break;
            }
        }

        if (fclose(fp) != 0) {
            printf("%s(%d): Filed to fclose - %s.\n",
                   __func__, __LINE__, strerror(errno));
            return (FAILED);
        }

        msleep(TSN_WIFI_NC_PULL_INTVL);
    }

    if (ret_val != PASSED) {
        printf("%s(%d): Failed to get WiFi NC ready signal, TIMEOUT!\n",
               __func__, __LINE__);
        return (FAILED);
    }

    memset(exec_cmd, 0, sizeof(exec_cmd));
    sprintf(exec_cmd, "netcat -c %s %d < %s",
            TSN_WIFI_IPADDR,
            DIAG_TSN_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
            DIAG_TSN_NC_COMMAND_DISPATCH_FILE);
    if (popen_cmd_exec_wrap(exec_cmd) != PASSED) {
        printf("%s(%d): Failed to execute command \"%s\".\n",
               __func__, __LINE__, exec_cmd);
        return (FAILED);
    }

    /* Waiting for NC execute finish signal from WiFi module */
    memset(exec_cmd, 0, sizeof(exec_cmd));
    sprintf(exec_cmd, "netcat -w %d -l -p %d >& %s",
                      TSN_NC_EXEC_TIME,
                      DIAG_TSN_NC_RET_EXEC_DONE_PORT,
                      TSN_NC_DONE_FILE);
    if (popen_cmd_exec_wrap(exec_cmd) != PASSED) {
        printf("%s(%d): Failed to execute command \"%s\".\n",
               __func__, __LINE__, exec_cmd);
        return (FAILED);
    }

    /* Check test result */
    memset(buf, 0, sizeof(buf));

    if ((fp = fopen(TSN_NC_DONE_FILE, "r")) == NULL) {
        printf("%s(%d): Can't open file %s for read.\n",
               __func__, __LINE__, TSN_NC_DONE_FILE);
        return (FAILED);
    }

    ret_val = FAILED;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "PASSED") != NULL) {
            ret_val = PASSED;
            break;
        }
    }

    if (fclose(fp) != 0) {
        printf("%s(%d): Filed to fclose - %s.\n",
               __func__, __LINE__, strerror(errno));
        ret_val = FAILED;
    }
    return (ret_val);
}

/*******************************************************************************
 *
 * Function   : tsn_wifi_led_control
 * Description: Function to control TSN WiFi LED
 * Inputs     : cmd - WIFI_LED_GREEN/WIFI_LED_RED/WIFI_LED_AMBER/WIFI_LED_OFF
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_wifi_led_control (int cmd)
{
    char tty_dev[32];

    memset(tty_dev, 0, sizeof(tty_dev));
    sprintf(tty_dev, "/dev/ttyS2");
    
    if (wifi_booted != TRUE) {
        printf("%s(%d): Please boot TSN WiFi module.\n",
               __func__, __LINE__);
        return (FAILED);
    } 
    switch (cmd) {
    case WIFI_LED_OFF:
        tsn_tx_uart(tty_dev, TSN_WIFI_TURN_ALL_LEDS_OFF);
        break;
    case WIFI_LED_RED:
        tsn_tx_uart(tty_dev, TSN_WIFI_TURN_LED_ON_RED);
        break;
    case WIFI_LED_GREEN:
        tsn_tx_uart(tty_dev, TSN_WIFI_TURN_LED_ON_GREEN);
        break;
    case WIFI_LED_AMBER:
        tsn_tx_uart(tty_dev, TSN_WIFI_TURN_LED_ON_AMBER);
        break;
    default:
        printf("%s(%d): Unsupported TSN WiFi LED control command(%d).\n",
               __func__, __LINE__, cmd);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_put_wifi_in_reset
 * Description: Function to put Wifi module in RESET.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_put_wifi_in_reset (void)
{
    uint reg_addr = (uint)FPGA_EXTER_DEV_RST_REG;
    uint reg_val = 0;

    /* 1. Confirm is WiFi module is present */
    if (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT) != TRUE) {
        printf("%s(%d): WiFi is not present.\n", __func__, __LINE__);
        return (PASSED);
    }

    /* 2. Check if WiFi module is in reset. */
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read WiFi state from FPGA reg.(0x%04X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    /* 3. Put WiFi module in reset if needed. */
    if ((reg_val & (uint)EXT_WLAN_RESET) != (uint)EXT_WLAN_RESET) {
        /* 3.1 Put WiFi module in reset. */
        reg_val |= (uint)(EXT_WLAN_RESET);

        if (fpga_write_32_reg(reg_addr, reg_val) != PASSED) {
            printf("%s(%d): Failed to set FPGA reg.(0x%04X).\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }
    }

    /* 4. Confirm WiFi module in reset */
    reg_val = 0;
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read FPGA reg.(0x%04X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    if ((reg_val & (uint)EXT_WLAN_RESET) != (uint)EXT_WLAN_RESET) {
        printf("%s(%d): Failed to put WiFi module in reset.\n",
               __func__, __LINE__);
        return (FAILED);
    }
    printf("WiFi module is in RESET.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_reset_wifi
 * Description: Reset TSN WiFi module by FPGA
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_reset_wifi (void)
{
    if (tsn_put_wifi_in_reset() != PASSED) {
        return (FAILED);
    }

    msleep(WIFI_RESET_INTERVAL);

    if (tsn_release_wifi_from_reset() != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_release_wifi_from_reset
 * Description: Do the Wifi module reset
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_release_wifi_from_reset (void)
{
    uint reg_addr = (uint)FPGA_EXTER_DEV_RST_REG;
    uint reg_val = 0;

    /* 1. Confirm is WiFi module is present */
    if (tsn_fpga_check_dev_present(FPGA_CPP_WLAN_PRESENT) != TRUE) {
        printf("%s(%d): WiFi is not present.\n", __func__, __LINE__);
        return (PASSED);
    }

    /* 2. Check if WiFi module is in reset. */
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read WiFi state from FPGA reg.(0x%04X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    /* 3. Release WiFi module from reset if needed. */
    if ((reg_val & (uint)EXT_WLAN_RESET) == (uint)EXT_WLAN_RESET) {
        /* 3.1 Release WiFi module from reset. */
        reg_val &= (uint)(~EXT_WLAN_RESET);

        if (fpga_write_32_reg(reg_addr, reg_val) != PASSED) {
            printf("%s(%d): Failed to release WiFi from reset by "
                   "setting FPGA reg.(0x%04X).\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }

        /* 3.2 Confirm WiFi module in reset */
        reg_val = 0;
        if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
            printf("%s(%d): Failed to read FPGA reg.(0x%04X).\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }

        if ((reg_val & (uint)EXT_WLAN_RESET) == (uint)EXT_WLAN_RESET) {
            printf("%s(%d): Failed to release WiFi module from reset.\n",
                   __func__, __LINE__);
            return (FAILED);
        }

        /* 3.3 Set up parameters to communicate with WiFi UART. */
        tsn_setup_wlan_uart();
    }
    printf("WiFi module is out of RESET.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_setup_wlan_uart
 * Description: Function to setup TSN UART interface parameter.
 * Inputs     : NONE
 * Outputs    : NONE 
 *
 *******************************************************************************
 */
void tsn_setup_wlan_uart (void)
{
    const int maxlen = 128;
    char tty[maxlen];
    int fd = -1;
    struct termios oldtio, newtio;

    snprintf(tty, maxlen-1, "/dev/ttyS2");
    fd = open(tty, O_RDWR|O_NOCTTY);
    if (fd < 0) {
      perror(tty);
      exit(1);
    }

    tcgetattr(fd, &oldtio);
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = B9600|CS8|CLOCAL|CREAD;
    /* IGNPAR : Ignore framing errors and parity errors*/
    /* ICRNL  : Translate carriage return to newline on input (unless IGNCR is set). */
    /* ICANON : Enable canonical input (else raw) */
    newtio.c_iflag = IGNPAR | ICRNL; 
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    close(fd);
    return;
}

/*******************************************************************************
 *
 * Function   : tsn_wifi_console_switch
 * Description: Function for TSN to switch to WiFi console by Picocom.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_wifi_console_switch (void)
{
    struct uart_parm picocom;
    picocom.tty_dev = TSN_WIFI_UART_DEV_STR;
    picocom.baudrate = 9600;
    picocom.databit = 8;
    picocom.parity = "1";
    picocom.flow = "n";

    /* Release WLAN module from Reset if needed */
    if (tsn_release_wifi_from_reset() != PASSED) {
        return (FAILED);
    }

    /* Console Switch to WiFi module */
    if (tsn_console_switch(&picocom) != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : popen_cmd_exec_wrap
 * Description: Wrapped function to execute command by using popen().
 * Inputs     : *cmd - command string
 * Outputs    : PASSED / FAILED
 *
 ******************************************************************************* 
 */
static int popen_cmd_exec_wrap (char *popen_cmd)
{
    FILE *fp;
    char buf[TSN_NC_BUF_SIZE];

    memset(buf, 0, sizeof(buf));

    if ((fp = popen(popen_cmd, "r")) == NULL) {
        printf("%s(%d): Failed to popen %s\n", __func__, __LINE__, popen_cmd);
        return (FAILED);
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
    }

    if (pclose(fp) == -1) {
        printf("%s(%d): Failed to pclose %s\n", __func__, __LINE__, popen_cmd);
        return (FAILED);
    }
    return (PASSED);
}


/*-------------------------------------------------
$Log: platform_wifi.c,v $
Revision 1.3  2018/05/15 09:37:32  steja
CSCvj38863: Enhanced LED single test utility

Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.4.4  2017/08/01 08:32:35  palin2
Enhanced TSN WiFi NC mechanism.

Revision 1.1.4.3  2017/07/31 16:35:47  palin2
Updated WiFi Diag kernel boot up process based on Cisco WiFi bootloader.

Revision 1.1.4.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.2.1  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

*/

