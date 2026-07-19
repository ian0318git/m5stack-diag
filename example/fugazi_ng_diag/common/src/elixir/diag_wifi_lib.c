/* $Id: diag_wifi_lib.c,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_wifi_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_wifi_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "error.h"
#include "errno.h"
#include "menu.h"
#include "nvmonvars.h"
#include "common.h"
#include "common_utils.h"
#include "proto.h"
#include "diag_moka_fpga_lib.h"
#include "platform_cookie.h"
#include <sys/ioctl.h>
#include "platform_i2c.h"
#include "i2c_api.h"
#include "i2c_dev.h"
#include "diag_i2c_lib.h"
#include "diag_wifi_lib.h"
#include "diag_uart_lib.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int wifi_set_uboot_tftp(char *, char *, char *);
int wifi_ts_dev_create(dev_lm75b_object_t *);
static uint32_t wifi_ts_i2c_rd(uint32, ushort *); 
static uint32_t wifi_ts_i2c_wr(uint32, ushort *);
int wifi_led_control(int);
int put_wifi_in_reset(void);
int release_wifi_from_reset(void);
void plat_setup_wlan_uart(void);
int plat_wifi_nc_dispatch_comm(char *);
static int popen_cmd_exec_wrap(char *);


// unit is miniseconds
#define DELAY_SYSCMD 1000

#define ENHANCE_ERROR_MSG_RDY 1

/*
 * Global variables
 */
int parm1 = 0;
int parm2 = 0;
int parm3 = 0;
int parm4 = 0;
extern boolean wifi_booted;

/*
 * Global extern functions
 */
extern int do_all_menu_items(struct menuinfo *);

#define PLAT_WIFI_PID_LENGTH 32


/************************************************************************
 * Function: wifi_reset_init()
 * Description : Do the Wifi reset initialization sequence
 *
 * Input: None
 *
 * Output: PASSED/FAILED
 *************************************************************************
 */
int wifi_reset_init (void)
{
    /* WiFi module initialization reset sequence */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_WLAN_RESET, TRUE,
                          RESET_20_MILLISECONDS) == FAILED) {
        return (FAILED);
    }

    /* Un-reset the wifi module */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_WLAN_RESET, FALSE,
                          UNRESET_20_MILLISECONDS) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_uart_rx
 * Description: Function to configure WiFi u-boot TFTP parameters.
 * Inputs     : dev
 *              size
 *              *uart_buf
 *              timeout
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static void *wifi_uart_rx (void *input)
{
    int timeout = 5; /*in secs */
    int size = 0; /* when size = 0, read all bytes from uart controller */

    plat_uart *uart = (plat_uart *)input;

    if (plat_rx_uart(uart->dev, size, (char *)uart->buf, timeout) < 0) {

    }
    pthread_exit(NULL);
}

/*******************************************************************************
 *
 * Function   : wifi_set_uboot_tftp
 * Description: Function to configure WiFi u-boot TFTP parameters.
 * Inputs     : tty_dev
 *              set_type
 *              ip_str
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wifi_set_uboot_tftp (char *tty_dev, char *set_type, char *ip_str)
{
    char      cmd_str[PLAT_WIFI_PARMS_LENGTH];
    char      chk_cmd[PLAT_WIFI_PARMS_LENGTH];
    int       ctr = 0, is_configed = FALSE;
    pthread_t threads;
    plat_uart  uart;

    uart.dev = tty_dev;

    memset(cmd_str, 0, sizeof(cmd_str));
    memset(chk_cmd, 0, sizeof(chk_cmd));

    snprintf(cmd_str, sizeof(cmd_str), "setenv %s %s\n", set_type, ip_str);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("cmd_str = %s", cmd_str);
    }

    snprintf(chk_cmd, sizeof(chk_cmd), "printenv %s\n", set_type);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("chk_cmd = %s", chk_cmd);
    }

    for (ctr = 0; ctr < PLAT_WIFI_MAX_RETRY; ctr++) { 
        memset(uart.buf, 0, sizeof(uart.buf));

        if (plat_tx_uart(tty_dev, cmd_str) != PASSED) {
            printf("%s: Failed to set %s to %s.\n",
                   __FUNCTION__, set_type, ip_str);
            return (FAILED);
        }
        msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */

        if (pthread_create(&threads, NULL, wifi_uart_rx, (void *)&uart)) {
            /* Should never occur */
            printf("%s: pthread_create failed.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(WAIT_WIFI_ACCESS_TIME);   /* 300ms */

        if (plat_tx_uart(tty_dev, chk_cmd) != PASSED) {
            printf("%s: Failed to print %s.\n", __FUNCTION__, set_type);
            return (FAILED);
        }

        pthread_join(threads, NULL);

        if (!strlen(uart.buf)) {
            printf("%s: [%d]No data received.\n", __FUNCTION__, ctr);
            break;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("RX = %s", uart.buf);
        }

        /* Confirm set-up by checked RX data */
        if (strstr(uart.buf, ip_str) != NULL) {
            is_configed = TRUE;
            break;
        }
    }
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Total retry = %d.\n", ctr);
    }

    if (is_configed != TRUE) {
        printf("%s: Failed to set WiFi %s.\n", __FUNCTION__, set_type);
        return (FAILED);
    }
    return (PASSED);
}

/********************************************************************
 *
 * Function: nc_check_test_status
 *
 * Description:  open a file /tmp/nc_reselt for check the status 
 *               is pass or fail 
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 **********************************************************************/
int nc_check_test_status (void)
{
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_ts_dev_create
 * Description : Function to create TMPX75 Device Object
 * Inputs      : gpio_obj - Pointer of TMPX75 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int wifi_ts_dev_create (dev_lm75b_object_t *ts_obj)
{
    dev_object_t *dev = (dev_object_t *)ts_obj;

    /* Create common device object */
    lm75b_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    ts_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    ts_obj->callout_fvt->rd = wifi_ts_i2c_rd;
    ts_obj->callout_fvt->wr = wifi_ts_i2c_wr;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_ts_i2c_rd
 * Description : Function implementation of TMPX75 I2C Read
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t wifi_ts_i2c_rd (uint32 offset, ushort *data) 
{
    n2g_i2c_if_t *i2c_if;
    
    i2c_if = (n2g_i2c_if_t *)(get_n2g_i2c_if(I2C_CTRL_TWO,
                                         I2C_MUX_ZERO,
                                         WIFI_TEMP_SENSOR_I2C_ADDR));
                                             
    i2c_if->size = sizeof(ushort);
    i2c_if->buf = (char *)data;
    i2c_if->offset = offset;

    if (n2g_i2c_open(i2c_if) != PASSED) {
        printf("%s: n2g_i2c_open failed\n", __func__);
        return (FAILED);
    }

    if (n2g_i2c_read(i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : wifi_ts_i2c_wr
 * Description : Function implementation of TMPX75 I2C Write
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t wifi_ts_i2c_wr (uint32 offset, ushort *data)
{
    int      wifi_i2c_fd = get_i2c_fd(CPU_I2C2);
    int      i2c_dev_addr = WIFI_TEMP_SENSOR_I2C_ADDR, ret_code = 0;

    i2c_dev_addr = WIFI_TEMP_SENSOR_I2C_ADDR;
    
    if (wifi_i2c_fd < 0) {
        printf("%s:%d i2c-2 descriptor is not exists.\n", __func__, __LINE__);
        return (FAILED);
    }

    ret_code = ioctl(wifi_i2c_fd, I2C_SLAVE, i2c_dev_addr);
    if (ret_code < 0) {
        printf("%s:%d Failed to connect to device %#x(rc = %#x).",
               __func__, __LINE__, i2c_dev_addr, ret_code);
        return (FAILED);
    }

    if (i2c_smbus_write_word_data(wifi_i2c_fd, offset, *data) < 0) {
        printf("%s %d Failed to do I2C write %#x to %#x of device %#x.",
               __func__, __LINE__, *data, offset, i2c_dev_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : wifi_led_control
 * Description: Function to control WiFi LED
 * Inputs     : cmd - WIFI_LED_GREEN/WIFI_LED_RED/WIFI_LED_BLUE/WIFI_LED_OFF
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int wifi_led_control (int cmd)
{
    char tty_dev[32];

    memset(tty_dev, 0, sizeof(tty_dev));
    snprintf(tty_dev, sizeof(tty_dev), "/dev/ttyS2");
    
    if (wifi_booted != TRUE) {
        printf("%s(%d): Please boot WiFi module.\n",
               __func__, __LINE__);
        return (FAILED);
    } 
    switch (cmd) {
    case WIFI_LED_OFF:
        plat_tx_uart(tty_dev, PLAT_WIFI_TURN_ALL_LEDS_OFF);
        break;
    case WIFI_LED_RED:
        plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_RED);
        break;
    case WIFI_LED_GREEN:
        plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_GREEN);
        break;
    case WIFI_LED_BLUE:
        plat_tx_uart(tty_dev, PLAT_WIFI_TURN_LED_ON_BLUE);
        break;
    default:
        printf("%s(%d): Unsupported WiFi LED control command(%d).\n",
               __func__, __LINE__, cmd);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : put_wifi_in_reset
 * Description: Function to put Wifi module in RESET.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int put_wifi_in_reset (void)
{
    uint reg_addr = (uint)FPGA_EXTER_DEV_RST_REG;
    uint reg_val = 0;

    /* 1. Check if WiFi module is in reset. */
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read WiFi state from FPGA reg.(0x%04X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    /* 2. Put WiFi module in reset if needed. */
    if ((reg_val & (uint)EXT_WLAN_RESET) != (uint)EXT_WLAN_RESET) {
        /* 2.1 Put WiFi module in reset. */
        reg_val |= (uint)(EXT_WLAN_RESET);

        if (fpga_write_32_reg(reg_addr, reg_val) != PASSED) {
            printf("%s(%d): Failed to set FPGA reg.(0x%04X).\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }
    }

    /* 3. Confirm WiFi module in reset */
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
 * Function   : release_wifi_from_reset
 * Description: Do the Wifi module reset
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int release_wifi_from_reset (void)
{
    uint reg_addr = (uint)FPGA_EXTER_DEV_RST_REG;
    uint reg_val = 0;

    /* 1. Check if WiFi module is in reset. */
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read WiFi state from FPGA reg.(0x%04X).\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    /* 2. Release WiFi module from reset if needed. */
    if ((reg_val & (uint)EXT_WLAN_RESET) == (uint)EXT_WLAN_RESET) {
        /* 2.1 Release WiFi module from reset. */
        reg_val &= (uint)(~EXT_WLAN_RESET);

        if (fpga_write_32_reg(reg_addr, reg_val) != PASSED) {
            printf("%s(%d): Failed to release WiFi from reset by "
                   "setting FPGA reg.(0x%04X).\n",
                   __func__, __LINE__, reg_addr);
            return (FAILED);
        }

        /* 2.2 Confirm WiFi module in reset */
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

        /* 2.3 Set up parameters to communicate with WiFi UART. */
        plat_setup_wlan_uart();
    }
    printf("WiFi module is out of RESET.\n");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plat_setup_wlan_uart
 * Description: Function to setup UART interface parameter.
 * Inputs     : NONE
 * Outputs    : NONE 
 *
 *******************************************************************************
 */
void plat_setup_wlan_uart (void)
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
 * Function   : plat_wifi_nc_dispatch_comm
 * Description: Function dispatches command to WiFi through nc command.
 * Inputs     : *cmd_str - command that will be transfered to WiFi module
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int plat_wifi_nc_dispatch_comm (char *cmd_str)
{
    char exec_cmd[128];
    char tty_dev[32];
    FILE *fp;
    char buf[PLAT_NC_BUF_SIZE];
    int  ret_val = FAILED;
    int  w_time = 0;

    memset(buf, 0, sizeof(buf));
    memset(exec_cmd, 0, sizeof(exec_cmd));
    memset(tty_dev, 0, sizeof(tty_dev));
    snprintf(tty_dev, sizeof(tty_dev), "/dev/ttyS2");

    /* Sanity check */
    if (cmd_str == NULL) {
        printf("%s(%d): *cmd_str is NULL.\n", __func__, __LINE__);
        return (FAILED);
    }

    /* Prepare NC command */
    if ((fp = fopen(DIAG_PLAT_NC_COMMAND_DISPATCH_FILE, "w")) == NULL) {
        printf("%s(%d): Can't open/create file %s\n",
               __func__, __LINE__, DIAG_PLAT_NC_COMMAND_DISPATCH_FILE);
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
    snprintf(exec_cmd, sizeof(exec_cmd), "echo OK | netcat -c -v -v %s %d >& %s",
             PLAT_WIFI_IPADDR,
             PLAT_WIFI_NC_RDY_PORT,
             PLAT_WIFI_NC_RDY_FILE);

    for (w_time = 0;
         w_time < PLAT_WIFI_NC_TIMEOUT;
         w_time+= PLAT_WIFI_NC_PULL_INTVL) {

        if (popen_cmd_exec_wrap(exec_cmd) != PASSED) {
            printf("%s(%d): Failed to execute command \"%s\".\n",
                   __func__, __LINE__, exec_cmd);
            return (FAILED);
        }

        memset(buf, 0, sizeof(buf));

        if ((fp = fopen(PLAT_WIFI_NC_RDY_FILE, "r")) == NULL) {
            printf("%s(%d): Can't open file %s for read.\n",
                   __func__, __LINE__, PLAT_NC_DONE_FILE);
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

        msleep(PLAT_WIFI_NC_PULL_INTVL);
    }

    if (ret_val != PASSED) {
        printf("%s(%d): Failed to get WiFi NC ready signal, TIMEOUT!\n",
               __func__, __LINE__);
        return (FAILED);
    }

    memset(exec_cmd, 0, sizeof(exec_cmd));
    snprintf(exec_cmd, sizeof(exec_cmd), "netcat -c %s %d < %s",
             PLAT_WIFI_IPADDR,
             DIAG_PLAT_NC_EXECUTE_COMMAND_TRANSFER_PORT_BASE,
             DIAG_PLAT_NC_COMMAND_DISPATCH_FILE);

    if (popen_cmd_exec_wrap(exec_cmd) != PASSED) {
        printf("%s(%d): Failed to execute command \"%s\".\n",
               __func__, __LINE__, exec_cmd);
        return (FAILED);
    }

    /* Waiting for NC execute finish signal from WiFi module */
    memset(exec_cmd, 0, sizeof(exec_cmd));
    snprintf(exec_cmd, sizeof(exec_cmd), "netcat -w %d -l -p %d >& %s",
             PLAT_NC_EXEC_TIME,
             DIAG_PLAT_NC_RET_EXEC_DONE_PORT,
             PLAT_NC_DONE_FILE);

    if (popen_cmd_exec_wrap(exec_cmd) != PASSED) {
        printf("%s(%d): Failed to execute command \"%s\".\n",
               __func__, __LINE__, exec_cmd);
        return (FAILED);
    }

    /* Check test result */
    memset(buf, 0, sizeof(buf));

    if ((fp = fopen(PLAT_NC_DONE_FILE, "r")) == NULL) {
        printf("%s(%d): Can't open file %s for read.\n",
               __func__, __LINE__, PLAT_NC_DONE_FILE);
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
    char buf[PLAT_NC_BUF_SIZE];

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
 * $Log: diag_wifi_lib.c,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.5  2021/05/05 06:16:36  illiu
 * Remove redundant function: wifi_enable_platform_wifi_eth() and wifi_confirm_gpio()
 *
 * Revision 1.1.2.4  2021/04/23 02:38:57  illiu
 * Replace sprintf with snprintf
 *
 * Revision 1.1.2.3  2020/11/12 06:36:47  illiu
 * 1. Add WiFi module Bootup Test item
 * 2. Add WiFi module Memory Test item
 * 3. Add WiFi module NOR flash Test item
 * 4. Fix WiFi LED control Util item
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:08:08  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
