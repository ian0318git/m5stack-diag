/* $Id: ngwic_prince.c,v 1.26 2020/05/25 09:37:09 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/ngwic_prince.c,v $
 *------------------------------------------------------------------
 *
 * ngwic_prince.c - This file contains functions for Prince NGWIC.
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "dash_fpga.h"
#include "cross_platform.h"
#include "adapter_fpga.h"
#ifdef TABEIL
#include "diag_fpga_lib.h"
#endif

#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define OIR_RES    0.02
#define OIR_SENSE_RES    0.151
/*#define PRINCE_P1B    1 */
#define PRINCE_LOCAL_IP_ADDR     "192.123.123.200"
#define PRINCE_REQUEST_PORT      2013
#define PRINCE_STATUS_PORT1       2016
#define DIAG_RTN_PASS_STR        "PASS"
#define DIAG_RTN_STR_LEN         4
#define DIAG_KILL_NC_TMP_FILE    "/tmp/prince_nc_tmp.pid"

extern int tftp_get (unsigned char *dir, unsigned char *file, 
     unsigned char *server_ip, unsigned char *dest, unsigned int check);
extern int ovld_get_ge_sw_port_num (int slot, int tgt_device, int local_port);
extern int set_gesw_line_loopback(int port_num, int onoff);
extern void port_tx_util(void);
extern int ovld_bcm_check_port_init(void);
extern int get_gesw_line_loopback(int port_num);
extern int do_all_menu_items(struct menuinfo *);

static int ltc4215_test(void);
static int ltc4215_register_test (void);
static int prince_pwr_off (void);
static int prince_pwr_on (void);
static int prince_power_off (void);
static int prince_power_on (void);
static int prince_pwr_cycle (void);
static int prince_console_switch(void);
static void disable_bp_ge_lpbk (void);
static void enable_bp_ge_lpbk (void);
static int prince_bp_ge_test(void);
static int pca9557_test(void);
static int pca9557_reg_write(void);
static int pca9557_reg_read(void);
static int set_ngwic_console(void);
static int prince_uart_test (void);
static int prince_reset (void);
static void* read_aux (void *u);
static int prince_utils (void);
static int prince_nim_test(void);
static int nc_cmd_run_prince_diag (int port);
static int prince_check_test_status (void);
static int prince_init_status_file (void);
static void prince_kill_nc (void);
static void prince_get_host_flag(void);
static int prince_send_diag_flag(void);

static void (*prince_saved_diag_exec)(void) = NULL;
static void *oir_if;

static n2g_i2c_if_t *pca_i2c;

static struct ngio_intf_t *prince_wic_iface;

#define PRINCE_CR_STRING            "\015"
#define PRINCE_ESC_CR_STRING        "\033\015"
#define PRINCE_RUN_DIAG             "/home/apps/prince"

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
submenu_xtable_t prince_utils_submenu_table[] = {
    {"Console Redirect",              (PFT)prince_console_switch,0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power on Prince NGWIC",     (PFT)prince_power_on,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power off Prince NGWIC",      (PFT)prince_power_off,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Power cycle Prince NGWIC",    (PFT)prince_pwr_cycle,   0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Prince Backplane GE Utility",  (PFT)prince_bp_ge_test,     0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"UART Test",                     (PFT)prince_uart_test,   0,   
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"LTC4215 Hotswap Utility",       (PFT)ltc4215_test, 0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"PCA9557 GPIO Expander Utility", (PFT)pca9557_test,      0,   0,
     (type_t(*)())0, 0,	   (type_t(*)())0,          0},
    {"Reset Prince NGWIC",            (PFT)prince_reset,         0,    0,
     (type_t(*)())0, 0,    (type_t(*)())0,           0},
};

#define PRINCE_UTILS_SUBMENU_TABLE_SZ (sizeof(prince_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t prince_utils_primary_items[PRINCE_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t prince_utils_secondary_items[PRINCE_UTILS_SUBMENU_TABLE_SZ + MAX_BASE_ITEMS];

char princeutiltitle[50];

menuinfo_t prince_util_submenu = {
    princeutiltitle,
    0,                                /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    prince_utils_primary_items,
};

menuinfo_t *prince_util_submenup = &prince_util_submenu;
/*=========================================
 * Main menu items
 *=========================================
 */
static submenu_xtable_t main_menu_table[] = {
    {"Prince Utilities",           (PFT)prince_utils,       0,   0,
     (type_t(*)())0, 0,    (type_t(*)())prince_utils, 0},
    {"Prince NIM test",            (PFT)prince_nim_test, 0, MF_CONTINUOUS | MF_SHOW_ERRCOUNT | MF_DOALL,
     (type_t(*)())0, 0,    (type_t(*)())0,          0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo maindiag = {
    "Prince Main Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 **********************************************************************
 *
 *  Function: prince_utils
 *
 *  Description: Prince Utitlities menu
 *
 *  Input: None 
 *
 *  Returns: PASSED
 *
 **********************************************************************
 */
static int prince_utils (void)
{
    assert(prince_wic_iface);

    sprintf(princeutiltitle, "Prince Slot %d Utilities Menu",
            prince_wic_iface->slot);
    build_primary_submenu(prince_utils_submenu_table,
                          PRINCE_UTILS_SUBMENU_TABLE_SZ,
                          princeutiltitle, &prince_util_submenup);

    build_secondary_submenu(prince_utils_submenu_table,
                            PRINCE_UTILS_SUBMENU_TABLE_SZ,
                            prince_utils_secondary_items);

    menu(prince_util_submenup, prince_utils_secondary_items, '\0');

    return (PASSED);
}

/*
 ***************************************************************************************
 *
 *  Function: prince_nim_test
 *
 *  Description: run Prince nim test automatically by sending a nc client request to the 
 *               nc server listening on Prince side
 *
 *  Input: None 
 *
 *  Returns: PASSED/FAILED
 *
 ****************************************************************************************
 */
static int prince_nim_test(void)
{
    assert(prince_wic_iface);

    printf("\nStarting Prince diag test with nc...\n");

    if (nc_cmd_run_prince_diag(PRINCE_REQUEST_PORT)) {
        cterr('f', 0, "HOST: NC command failed in run Prince test\n");
        prince_kill_nc();
        return (FAILED);
    }

    prince_kill_nc();
    return (PASSED);
}

/*************************************************************************************************
 * Function: nc_cmd_run_prince_diag
 * Description: Start the local nc server for receiving test status and initial the status file
 *              Send a nc client request to the module side nc server.
 *              Check the test status.
 *
 * Input:    port - ruuning diag request port number
 *
 * Return: PASSED / FAILED
 **************************************************************************************************
 */
static int nc_cmd_run_prince_diag (int port)
{
    char cmdbuf[128];
    int check_flag;
    int i;
    uchar data;

    assert(prince_wic_iface);
    assert(oir_if);

    printf("\nWait for Prince module side to boot up diag menu.\n");

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Prince module side when the diag menu is up. */
    for (i = 0; i < 2000; i++) {
        if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
            util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
            cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
            return (FAILED);
        }

        if (data & 0x08)
            break;

        msleep(200);
    }

    if (i == 2000) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }
    sleep(1);

    prince_get_host_flag();
    prince_send_diag_flag();

    sprintf(cmdbuf, "nc %s %d\n", PRINCE_LOCAL_IP_ADDR, port);
    printf("HOST: nc command: %s\n", cmdbuf);

    if (prince_init_status_file()) {
        printf("Initial status file error.\n");
        return (FAILED);
    }

    for (i = 0; i < 3; i++) {
        if (system(cmdbuf)) {
            printf("Unable to request nc server.\n");
            return (FAILED);
        }

        check_flag = prince_check_test_status();
        if (check_flag != -1) {
            break;
        } else {
            /* Status file is empty */
            printf("Retry...\n");
        }
        msleep(1000);
    }

    if (check_flag == FAILED) {
        cterr('f', 0, "NGWIC PRINCE-%d test fails\n", prince_wic_iface->slot);
    } else if (check_flag == PASSED) {
        prpass(testpass, "NGWIC PRINCE-%d test passes\n", prince_wic_iface->slot);
    } else {
        cterr('f', 0, "NC Connection Error.\n");
        return (FAILED);
    }

    return (PASSED);
}

/********************************************************************************
 * Function: prince_init_status_file
 * Description: This function create the status file if it doesn't exist
 *              and listen to the status port
 *
 * Input:  None
 * Output: PASSED/FAILED
 *
 ********************************************************************************
 */
static int prince_init_status_file (void)
{
    char cmd1[84];
    char status_file[32];

    assert(prince_wic_iface);

    sprintf(status_file, "/tmp/ngwic_prince_%d.status", prince_wic_iface->slot);
    /* create or clear the status file */
    /*sprintf(cmd1, "rm -rf %s", status_file);*/
    sprintf(cmd1, "echo ' ' > %s", status_file);
    system(cmd1);

    /* Listen to the command status */
    sprintf(cmd1, "nc -l -l -p %d > %s &", PRINCE_STATUS_PORT1, status_file);
    /* sprintf(cmd, "nc -l -l -p %d  > /dev/console &", PRINCE_STATUS_PORT);*/
    printf("HOST: nc command: %s\n", cmd1);
    /*if (system(cmd1) || system(cmd2) || system(cmd3)) {
        return (FAILED);
    }*/
    if (system(cmd1)) {
        return (FAILED);
    }
    return (PASSED);
}

/*****************************************************************
 *
 * Function: prince_check_test_status
 *
 * Description: This function checks the content of status file and
 *              determine whether the test passes or fails.
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************
 */
static int prince_check_test_status (void)
{
    FILE *fp;
    char status_file[32];
    char buf[DIAG_RTN_STR_LEN + 1];
    char cmd[32];

    sprintf(status_file, "/tmp/ngwic_prince_%d.status", prince_wic_iface->slot);


    fp = fopen(status_file, "r");
    if (fp == NULL) {
        printf("%s: Unable to open '%s'\n", __FUNCTION__, status_file);
        return (FAILED);
    }

    if (fgets(buf, (DIAG_RTN_STR_LEN + 1), fp) != NULL) {
        sprintf(cmd, "cat %s", status_file);
        system(cmd);
        if (strcmp(buf, DIAG_RTN_PASS_STR)) {
            printf("Fail! Return Status is %s\n", buf);
            fflush(stdout);
            fclose(fp);
            return (FAILED);
         } else {
            fclose(fp);
            return (PASSED);
         }
    }

    printf("Warning: status file is empty.\n");
    fclose(fp);

    return (-1);
}

/***************************************************************************
 *
 * Function: prince_kill_nc
 *
 * Description: This function lists all process and grep nc process,
 *              and dump their pids to a temporary to kill them
 *
 * Input:  None
 *
 * Output: None
 *
 ***************************************************************************
 */
static void prince_kill_nc (void)
{
    char cmd[128];
    char buf[128];
    char pid_file[32];
    char *token;
    int pid;
    FILE *fp;
    struct stat sts;

    fp = fopen(DIAG_KILL_NC_TMP_FILE, "w+");
    if (fp == NULL) {
        printf("%s: Open '%s' Failed\n", __FUNCTION__, DIAG_KILL_NC_TMP_FILE);
        return;
    }

    sprintf(cmd, "ps | grep 'nc 192.123.123 \\| nc -l -l -p' > %s", DIAG_KILL_NC_TMP_FILE);
    system(cmd);
    printf("\nkill cmd: %s\n", cmd);

    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        /* separate string of one line, get 1st substring pointer */
        token = strtok(buf, " ");
        pid = atoi(token);
        /* Check if this process is still alive */
        sprintf(pid_file, "/proc/%d", pid);
        if (stat(pid_file, &sts) == -1) {
            /* Process doesn't exist */
            continue;
        }
        printf("Killing a nc process.\n"); 
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }

    fclose(fp);

}

/**********************************************************************
 * Function: prince_cleanup()
 *
 * Description: This function performs the cleanup task before exiting
 *              the test.
 *
 * Input:  None
 *
 * Output: None
 **********************************************************************
 */
static void
prince_cleanup (void)
{
    assert(prince_wic_iface);

    disable_bp_ge_lpbk();

    if (prince_saved_diag_exec) {
        pre_diag_exec = prince_saved_diag_exec;
        prince_saved_diag_exec = NULL;
    }
}

/*************************************************************************
 * Function: prince_uart_test_tty
 *
 * Test the UART connection from the host to Prince by sending
 * string to uart and recieve expect msg from  from uart interface
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
prince_uart_test_tty ()
{
    const int maxlen = 28;
    char test_if[maxlen];

    struct termios config;
    int uart_fd;
    char sd_str[100], exp_str[100];
    char *sd_pattern = "o\n";
    char *exp_pattern = "Linux";
    pthread_t threads;
    int txcnt = 0;

    s_uart uart;

    /* 'o\n' for trigger Prince side diag main menu item,
     * which will invoke 'uname'.
     */
    assert(prince_wic_iface);

    // When Prince is in Thule adapter card
    snprintf(test_if, maxlen-1, "/dev/ttyDASH%d",
             prince_wic_iface->uart_ctrl);

    prpass(testpass, "Prince UART ");

    uart.dev = (char*)test_if;
    memset(uart.buf, '\0', sizeof(uart.buf));

    /* the expect strings are from diag menu
     * this is for set exit state on rx_uart
     */
    uart.tst_typ = TRIG_DIAG_M;
    uart_fd = open(uart.dev, O_WRONLY);
    if (uart_fd < 0) {
        cterr('f', 0, "\nprince_uart_test_tty(): open tty failed.\n");
        return uart_fd;
    }
    if (tcgetattr(uart_fd, &config) < 0) {
        cterr('f', 0, "\nprince_uart_test_tty(): Failed in tcgetattr()\n");
        return (FAILED);
    }

    config.c_lflag &= ~(ICANON|IEXTEN|ISIG|ECHO);
    config.c_iflag |= IGNCR;
    config.c_oflag &= ~(OPOST);

    if (cfsetispeed(&config,B9600) < 0) {
        cterr('f', 0, "\nprince_uart_test_tty(): Failed in cfsetispeed()\n");
        return (FAILED);
    }
    if (cfsetospeed(&config,B9600) < 0) {
        cterr('f', 0, "\nprince_uart_test_tty(): Failed in cfsetospeed()\n");
        return (FAILED);
    }
    if (tcsetattr(uart_fd, TCSAFLUSH, &config) < 0) {
        cterr('f', 0, "\nprince_uart_test_tty(): Failed in tcsetattr()\n");
        return (FAILED);
    }

    close(uart_fd);
    if(pthread_create(&threads, NULL, read_aux, (void *)&uart)) {
        printf("%s: pthread_create failed.\n", __FUNCTION__);
        cterr('f', 0, "pthread_create failed.\n");
        return FAILED;
    }

    msleep(500);

    txcnt = tx_uart(uart.dev, sd_pattern, 1);
    printf("\n");
    pthread_join(threads, NULL);
    printf("\n%d bytes sent. %s\n", txcnt, uart.buf);
    if (!strlen(uart.buf)) {
        cterr('f', 0, "prince_uart_test_tty(): Failed to receive data.\n");
        return FAILED;
    }

    if (!strstr(uart.buf, exp_pattern)) {
        sprintf(sd_str, sd_pattern);
        sprintf(exp_str, exp_pattern);
        printf("[sd = %s] [exp = %s] [rp = %s].\n", sd_str, exp_str, uart.buf);
        cterr('f', 0, "prince_uart_test(): Failed to receive expected data.\n");
        return FAILED;
    }

    return (PASSED);
}

/**********************************************************************
 * Function: prince_uart_test
 *
 * Description: This function performs the uart interface test for the 
                NGVM
 *
 * Input:  None
 *
 * Output: PASSED/FAILED
 **********************************************************************
 */
static int prince_uart_test (void)
{
    int port;
    char *ptr;
    char *tx_str = "o\n";
    char *exp_pattern = "Linux";
    char rx_str[64];
    int tx_len = strlen(tx_str);
    int rx_sz;
    int i;
    struct adapter_uart_t *padapter_uart;

    prpass(testpass, "Uart Test - ");
    port = prince_wic_iface->uart_ctrl;

    printf("UART port %d\n", port);
    memset(rx_str, 0, sizeof(rx_str));
    rx_sz = 0;

    if (prince_wic_iface->mod_type == DAUGHTER_CARD) {
        /* Switzer-carrier adapter card has its own control FPGA, use adapter Uart */
        /* utilities when NIM is inserted into Switzer-carrier adapter card. */
        padapter_uart = get_current_adapter_uart();
        padapter_uart->adapter_uart_reset(port);
        padapter_uart->adapter_uart_tx(port, 9600, tx_str, tx_len, 0);
        sleep(1);
        padapter_uart->adapter_uart_rx(port, &rx_sz, rx_str);
        padapter_uart->adapter_uart_reset(port);
    } else {
        dash_uart_reset(port);
        dash_uart_tx(port, 9600, tx_str, tx_len, 0);
        sleep(1);
        dash_uart_rx(port, &rx_sz, rx_str);
        dash_uart_reset(port);
    }
    ptr = memchr(rx_str, exp_pattern[0], strlen(rx_str));
    printf("rx_str = %s\n", rx_str);
    if (ptr == NULL) {
        cterr('f', 0, "tx/rx strings do not match: expected %s, got %s",
                exp_pattern, rx_str);
        return (FAILED);
    }
    for (i = 0; i < strlen(exp_pattern); i++) {
        if (ptr[i] != exp_pattern[i]) {
            cterr('f', 0, ".tx/rx strings do not match: expected %s, got %s",
                     exp_pattern, ptr);
            return (FAILED);
        }
    }

    printf("prince UART test passed. \n");
    return (PASSED);
}

/*************************************************************************
 * Function: prince_iface_test
 *
 * Test entry for Prince interface test.
 *      covered: I2C, GE0, UART.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 *************************************************************************
 */
int 
prince_iface_test ()
{
    uchar data;
    int i;

    assert(oir_if);

    /* Testing the I2C interface */
    if (ltc4215_register_test()) {
        return (FAILED);
    }

    printf("\nWait for Prince module side to boot up diag menu.\n");

    /* poll for Primary Interface Ready pin (GPIO pin 3) which is set 
       by Prince module side when the diag menu is up. */
    for (i = 0; i < 2000; i++) {
	if (io_port_8bit_i2c_read(pca_i2c, 0x0, &data, TRUE) == FAILED) {
            util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
	    cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	    return (FAILED);
	}

	if (data & 0x08)
	    break;

	msleep(200);
    }

    if (i == 2000) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
        cterr('f',0,"Timeout waiting for primary interface ready pin asserted");
        return (FAILED);
    }

    sleep(3);

    /* Testing UART and GE0 interfaces */
    if (prince_uart_test()) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
	return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (PASSED);
}


/*------------------------------------------------------------------------------
 *
 * Function: prince_test().
 *
 * Description: This function is the entry point for Prince NGWIC test .
 *
 * Input:  wic - pointer to ngio_intf_t struct
 *
 * Output: PASSED/FAILED.
 *
 *------------------------------------------------------------------------------
 */
int 
prince_test (void *wic)
{ 
    int slot;
    ushort board_id = 0;
    int ret_val = PASSED;
    uint32_t module_type;
    ngio_eth_speed_t new_speed, old_speed;

    assert(wic);

    prince_wic_iface = (struct ngio_intf_t *)wic;

    slot = prince_wic_iface->slot;
    board_id = prince_wic_iface->id;

    prince_wic_iface->uart_on(wic);

    printf("\nprince_test, board_id %#x, slot %d\n", board_id, slot);

    testname("Slot%d Prince NGWIC ", slot);

    oir_if = (void *)(prince_wic_iface->oir);

    pca_i2c = prince_wic_iface->pca;

    if (tftp_get(0, (unsigned char *)"nim_serial_fw.img", 
                 0, (unsigned char *)"/firmware/nim_serial_fw.img", 1) < 0) {
        cterr('f', 0, "Failed to tftp download firmware to local host");
        return(FAILED);
        
    }

    /* Curie 2RU: Force eth port to 1Gb/s for 10G MAC BCM57412 which has
     * not the ability of auto-negotiation between 1G and 10G */
    module_type = prince_wic_iface->mod_type;
    new_speed = NGIO_ETH_SPEED_1G;
    ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);

    prince_wic_iface->unreset(wic);
    msleep(1000);
    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
#ifdef PRINCE_P1B
    enable_bp_ge_lpbk();
#endif
    /*
     * pm_subtest_menu now built.  Display and interact with user until
     * <ESC><RET> back to main menu.
     *
     * To prevent freeing up allocated memory prematurely,
     * save the pre_diag_exec function and set it to NULL.
     * This will prevent menu() marking the needed memory freed.
     */
    prince_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
			  &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
			    main_menu_secondary_items);

    if (prince_wic_iface->test_type == IFACE_TEST) {
	ret_val = prince_iface_test();
    } else {
        if (prince_wic_iface->menu_display == TRUE) {
            menu(maindiagp, main_menu_secondary_items, '\0');
        } else {
            do_all_menu_items(maindiagp);
        }
    }

    prince_cleanup();

    /* Curie 2RU: restore eth configuration */
    ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);

    return (ret_val);
}
/**********************************************************************
 *
 * Function: ltc4215_register_test
 *
 * Description: A wrapper function for LTC4215 register test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_register_test (void)
{
    int ret;

    prpass(testpass, "LTC4215 OIR Register test ");

    ret = oir_ltc4215_register_test(oir_if);
    if (ret == FAILED)
	cterr('f',0,"LTC4215 register test failed.");

    return (ret);
}

/**********************************************************************
 *
 * Function: ltc4215_test
 *
 * Description: LTC4215 Register Test/Read/Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
ltc4215_test (void)
{
    uchar type = 'd';
    int stop = 0;
    uint8_t  data = 0;
    float current = 0.0;

    assert(oir_if);
    printf("\nLTC4215 Hotswap Utility\n"); 

    while (1) {
        printf("\na: LTC4215 Register Test\n");
        printf("b: LTC4215 Register Read\n");
        printf("c: LTC4215 Register Write\n");
        printf("d: show 12V current consumption\n");
        printf("e: exit\n");
        type = getc_answer("Select an option", "abcde", 'e');
        switch(type) {
        case 'a':
            prpass(testpass, "LTC4215 OIR Register test ");
            if (oir_ltc4215_register_test(oir_if)) {
	        cterr('f',0,"LTC4215 register test failed.");
                stop = 1;
            }
            break;
        case 'b':
            util_oir_ltc4215_reg_read(oir_if);
            break;
        case 'c':
            util_oir_ltc4215_reg_write(oir_if);
            break;
        case 'd':
            oir_ltc4215_reg_read(oir_if, LTC4215_SENSE_REG, &data);
            current = OIR_SENSE_RES * data / OIR_RES;
            printf("Sense Voltage Register divided by R28: %f mA\n", current);
            break;
        case 'e':
            stop = 1;
            break;
        default:
            break;
        }
        if (stop) {
            break;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: prince_pwr_off
 *
 * Description: This function does all necessary configuration to power off.
 *              reset module, power off, i2c reset.
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int
prince_pwr_off (void)
{
    uint8_t data = 0;
    int slot;

    assert(oir_if);
    assert(prince_wic_iface);

    slot = prince_wic_iface->slot;
    printf("\nPower Off the Prince NGWIC.\n");

    /* disable power interrupt */
    if (prince_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ngiosm_disable_intr (slot, NGIO_FLT_INTR);
    else
        ngiowic_disable_intr (slot, NGIO_FLT_INTR);

    if (util_oir_ltc4215_led(oir_if, OIR_LED_OFF)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power off NGWIC module */
    data &= ~LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    prince_wic_iface->i2c_reset(prince_wic_iface);
    prince_wic_iface->off(prince_wic_iface);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: prince_power_off
 *
 * Description: This function is called for power-off utility.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
prince_power_off (void)
{
    uint8_t ans;

    assert(prince_wic_iface);

    printf("\n\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Off ABORT! Prince NGWIC Still Power On.\n\n");
        return (PASSED);
    }

    if (prince_pwr_off()) {
        return (FAILED);
    }

    return PASSED;

}


/***************************************************************************
 *
 * Function: prince_pwr_on
 *
 * Description: This function does all necessary configuration to power on.
 *              enable Prince NGWIC, power it on, take it out of reset.
 *
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 ***************************************************************************
 */
static int
prince_pwr_on (void)
{
    uint8_t  data = 0;

    printf("\nPower On the Prince NGWIC.\n");

    assert(oir_if);
    assert(prince_wic_iface);

    /* enable ngwic and take I2C out of reset */
    slot_i2c_unreset(prince_wic_iface, prince_wic_iface->slot, "WIC");

    if (util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY)) {
        return (FAILED);
    }

    if (oir_ltc4215_reg_read(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }

    /* power on NGWIC module */
    data |= LTC4215_FET_ON_CONTROL;
    if (oir_ltc4215_reg_write(oir_if, LTC4215_CONTROL_REG, &data)) {
        return(FAILED);
    }
    msleep(200);

    /* make sure the power is output good */
    if (oir_ltc4215_reg_read(oir_if, LTC4215_STATUS_REG, &data)) {
        return(FAILED);
    }
    if (!(data & LTC4215_FET_ON_STATUS)) {
        printf("FET CANNOT be Turned On.\n");
        return(FAILED);
    }
    if (data & LTC4215_POWER_BAD_STATUS) {
        printf("Power CANNOT be Turned On.\n");
        return(FAILED);
    }

    printf("Waiting for Prince NGWIC to Power-Up.\n");
    msleep(2000);

    /* take Prince NGWIC out of reset */
    prince_wic_iface->unreset(prince_wic_iface);

    /* turn on the green light */
    if (util_oir_ltc4215_led(oir_if, OIR_LED_GREEN_ONLY)) {
        return (FAILED);
    }
    prince_wic_iface->uart_on(prince_wic_iface);
#ifdef PRINCE_P1B
    enable_bp_ge_lpbk();
#endif
    return (PASSED);
}

/**********************************************************************
 *
 * Function: prince_power_on
 *
 * Description: This function is called for power-on utiliy.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
prince_power_on (void)
{
    assert(prince_wic_iface);

    if(prince_pwr_on()) {
        return(FAILED);
    }

    printf("Prince NGWIC is powered up.\n");

   return (PASSED);
}

/**********************************************************************
 *
 * Function: prince_pwr_cycle
 *
 * Description: A wrapper function for LTC4215 Power Cycle test.
 *
 * Input : None 
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
prince_pwr_cycle (void)
{
    uint8_t i, ans;

    printf("\n");
    printf("Power Cycle the Prince NGWIC");

    printf("\n\nProceed with Power Cycle? (y/n) ");
    ans = getchar();
    putchar(ans);
    printf("\n\n");
    if (ans != 'y' && ans != 'Y') {
        printf("\nPower Cycle ABORT! "
                "Prince is not Power Cycled.\n\n");
        return (PASSED);
    }

    if (prince_pwr_off()) {
        cterr('f', 0, "Failed to Power Off the Prince NGWIC");
        return(FAILED);
    }

    /* msleep for 10 seconds. */
    for (i = 0; i < 10; i++) {
        printf(".");
        msleep(1000);
    }

    if (prince_pwr_on()) {
        cterr('f', 0, "Failed to Power On the Prince NGWIC");
        return(FAILED);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: pca9557_reg_read
 *
 * Description: PCA9557 (GPIO expander) Register Read utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9557_reg_read (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to read: ", 0, 0, 0x3);

    if (io_port_8bit_i2c_read(pca, offset, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    printf("\nRegister @ %#x = %#x\n", offset, data);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: pca9557_reg_write
 *
 * Description: PCA9557 (GPIO expander) Register Write utility.
 *
 * Input : None.
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
pca9557_reg_write (void)
{
    n2g_i2c_if_t *pca = pca_i2c;
    uchar data = 0;
    int offset;

    assert(pca);

    offset = gethex_answer("Reg offset to write: ", 1, 1, 0x3);
    data = gethex_answer("Data to write", data, 0, 0xff);

    if (io_port_8bit_i2c_write(pca, offset, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ %#x\n", offset);
	return (FAILED);
    }
    return (PASSED);
}

static int
pca9557_test (void)
{
    uint8_t ans;

    assert(prince_wic_iface); 

    printf("\nRead or Write pca9557 register? (r/w): ");
    ans = getchar();
    getchar();
    if (ans == 'r' || ans == 'R') {
        if (pca9557_reg_read()){
            return (FAILED);
        }
    } else if (ans == 'w' || ans == 'W') {
         if (pca9557_reg_write()){
            return (FAILED);
         }
    } else {
        printf("ABORT!\n");
    }

    return (PASSED);
}

static int 
set_ngwic_console ()
{
    uchar data;

    if (io_port_8bit_i2c_read(pca_i2c, 0x03, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    /* set GPIO pin 4 as output pin */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x03, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x03\n");
	return (FAILED);
    }

    if (io_port_8bit_i2c_read(pca_i2c, 0x01, &data, TRUE) == FAILED) {
        cterr('f', 0, "Unable to read PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    /* set GPIO pin 4 to 0 for NGWIC console redirect */
    data &= ~0x10;
    if (io_port_8bit_i2c_write(pca_i2c, 0x01, &data) == FAILED) {
        cterr('f', 0, "Unable to write PCA9557 register @ 0x01\n");
	return (FAILED);
    }

    return (PASSED);
}



static int
prince_console_switch ()
{
    const int maxlen = 128;
    char cmd[maxlen];

    assert(prince_wic_iface);
    assert(oir_if);

    if (set_ngwic_console()) {
        util_oir_ltc4215_led(oir_if, OIR_LED_AMBER_ONLY);
    }

    printf("\n\n Type <ctrl-a> <ctrl-x> to return to host console\n");
    fflush(stdout);
    fflush(stderr);
    msleep(1000);

    snprintf(cmd, maxlen-1, "picocom -b9600 -d8 -pn -fn /dev/ttyDASH%d",
             prince_wic_iface->uart_ctrl); 

#if DEBUG_UARTCOM
    printf("cmd=%s\n", cmd);
#endif

    fflush(stdout);
    fflush(stderr);
    msleep(1000);
    system(cmd);

    return (PASSED);
}

static void
disable_bp_ge_lpbk ()
{
    int ge_port;

    assert(prince_wic_iface);
    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        /* Goldbeach platform didn't have GESW*/
        printf("\nGoldbeach and Curie 1RU/2RU Didn't Support GESW\n");
        return;
    }

    /* When wic card is in sm adapter card */
    if (prince_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port =  ovld_get_ge_sw_port_num(prince_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(prince_wic_iface->slot, TGT_DEV_NGWIC, 0);
    
    set_gesw_line_loopback(ge_port, 0);
}

static void
enable_bp_ge_lpbk ()
{
    int ge_port;

    assert(prince_wic_iface);

    if (is_goldbeach() || is_curie_1ru() || is_curie_2ru()) {
        /* Goldbeach platform didn't have GESW*/
        printf("\nGoldbeach and Curie 1RU/2RU Didn't Support GESW\n");
        return;
    }
    // When wic card is in sm adapter card

    if (prince_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port =  ovld_get_ge_sw_port_num(prince_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(prince_wic_iface->slot, TGT_DEV_NGWIC, 0);
    
    set_gesw_line_loopback(ge_port, 1);
}
/**********************************************************************
 *
 * Function: prince_bp_ge_test
 * Description: This function provides tests for Prince port of Backplane GESW
 *
 **********************************************************************
 */
static int
prince_bp_ge_test ()
{
    int ge_port;
    uchar type = 'e';
    int stop = 0;
    int state = -1;

    assert(prince_wic_iface);

    // When wic card is in sm adapter card
    if (prince_wic_iface->mod_type == SM_DAUGHTER_CARD)
        ge_port =  ovld_get_ge_sw_port_num(prince_wic_iface->slot, TGT_DEV_NGSM, 0);
    else
        ge_port = ovld_get_ge_sw_port_num(prince_wic_iface->slot, TGT_DEV_NGWIC, 0);
    
    printf("\nPrince Backplane GE Utility\n"); 

    while (1) {
        printf("\na: enable motherboard line loopback at Prince GESW port\n");
        printf("b: disable motherboard line loopback at Prince GESW port\n");
        printf("c: get Prince GESW port loopback setting\n");
        printf("d: Prince GESW port send package\n");
        printf("e: exit\n");
        type = getc_answer("Select an option", "abcde", 'e');
        switch(type) {
        case 'a':
            enable_bp_ge_lpbk();
            break;
        case 'b':
            disable_bp_ge_lpbk();
            break;
        case 'c':
            state = get_gesw_line_loopback(ge_port);
            if (state) {
                printf("line loopback has been enabled.\n");
            } else {
                printf("line loopback has been disabled.\n");
            }
            break;
        case 'd':
            printf("Prince GE port is %d\n", ge_port);
            port_tx_util();
            break;
        case 'e':
            stop = 1;
            break;
        default:
            break;
        }
        if (stop) {
            break;
        }
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: prince_reset
 *
 * Description: This function query for reset or unreset  Prince NGWIC module.
 *              It doesn't reset or unreset i2c.
 * Input :	None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
prince_reset (void)
{
    uint8_t ans;

    assert(prince_wic_iface);

    printf("\nReset or Unreset Prince module? (r/u): ");
    ans = getchar();
    putchar(ans);
    printf("\n");
    if (ans == 'r' || ans == 'R') {
        if (prince_wic_iface->reset(prince_wic_iface)){
            printf("Unable to reset Prince Module\n");
            return (FAILED);
        }
        msleep(1000);
    } else if (ans == 'u' || ans == 'U') {
         if (prince_wic_iface->unreset(prince_wic_iface)){
            printf("Unable to unreset Prince Module\n");
            return (FAILED);
         }
        msleep(1000);
    } else {
        printf("ABORT!\n");
    }

    return (PASSED);
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

/*****************************************************************
 *
 * Function: prince_tx_uart
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

static int prince_tx_uart (char *tty_dev, char *out_str)
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

/**********************************************************************
*
* Function: prince_get_host_flag
*
* Get current Host diag flags
*
* Input : none
*
* Output: none
*
**********************************************************************
*/
static void prince_get_host_flag (void)
{
    char flag_file[32];
    char flags[256];
    char cmd[256];

    /* Write flags to local file */
    sprintf(flag_file, "/tmp/host_flags");
    sprintf(flags, "diagflag=%x\tdiagflag_xram=%x",
        (unsigned int)(NVRAM)->diagflag, (unsigned int)diagflag_xram);
    sprintf(cmd, "echo %s > %s", flags, flag_file);

    system(cmd);
}


/**********************************************************************
*
* Function: prince_pass_diag_flag
*
* Send the Host diag flags to module side via nc command
*
* Input : none
*
* Output: none
*
**********************************************************************
*/
static int prince_send_diag_flag (void)
{
    int ix;
    char tty_dev[32];
    char flag_file[32];
    char nc_cmd[84];

    assert(prince_wic_iface);

    /* Setup UART toward Prince board */
    sprintf(tty_dev, "/dev/ttyDASH%d", 
            prince_wic_iface->uart_ctrl);

    sprintf(flag_file, "/tmp/host_flags");

    /* NIM: Listen to the command status */
    sprintf(nc_cmd, "nc -l -l -p %d > %s &", 3013, flag_file);

    /* NIM: Escape to default shell by typing 'ESC' and 'Enter' */
    for (ix = 0; ix < 3; ix++) {
        prince_tx_uart(tty_dev, PRINCE_ESC_CR_STRING);
    }

    msleep(500);

    /* NIM: Listen to the command status */
    prince_tx_uart(tty_dev, nc_cmd);
    printf("Prince: nc command: %s\n", nc_cmd);

    /* NIM: Execute Module diag again */
    prince_tx_uart(tty_dev, PRINCE_RUN_DIAG);
    printf("Prince: nc command: %s\n", nc_cmd);
    prince_tx_uart(tty_dev, PRINCE_CR_STRING);

    msleep(2000);

    /* HOST: send the flag */
    sprintf(nc_cmd, "nc %s 3013 < %s\n", PRINCE_LOCAL_IP_ADDR, flag_file);
    printf("HOST: nc command: %s\n", nc_cmd);

    if (system(nc_cmd)) {
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/******** History ********
$Log: ngwic_prince.c,v $
Revision 1.26  2020/05/25 09:37:09  qingcwan
CSCvu31200:Fix bug introduced by prev commit to merge in switzer-carrier code.

Revision 1.25  2020/05/22 02:28:23  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.24  2020/01/09 01:01:51  jiajliu
Merge Curie 2RU to main trunk

Revision 1.23  2019/10/17 02:16:15  kehuang2
Collapse Tabei-L into main trunk

Revision 1.22  2019/08/06 06:56:06  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.21.2.1  2018/07/16 09:28:04  alpeng
skip ge switch portions for prince, reva, arkenstone and dreamliner

Revision 1.21  2018/05/22 02:31:10  alpeng
fixed compiler warning, CSCvj57934

Revision 1.20  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.19  2017/08/10 10:29:25  iachang
Fixed compile issue

Revision 1.18  2017/08/10 10:10:34  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.17  2016/10/16 12:28:12  iachang
Supported Goldbeach Platform.

Revision 1.16.30.4  2018/05/17 10:50:20  alpeng
 sync with trunk <trunk-051618>

Revision 1.16.30.3  2017/04/05 06:40:23  leschen
Sync with <ng_diag-tag-032917>

Revision 1.16.30.2  2016/12/21 10:06:04  alpeng
 update prince uart test for get uart ctrl num from api

Revision 1.16.30.1  2016/12/05 06:36:59  alpeng
fixed the uart ctrl num for ngio; change is approved on prrq

Revision 1.19  2017/08/10 10:29:25  iachang
Fixed compile issue

Revision 1.18  2017/08/10 10:10:34  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.17  2016/10/16 12:28:12  iachang
Supported Goldbeach Platform.

Revision 1.16  2014/10/13 06:31:24  bowang3
Make uart test pass when NIM card is in NGSM carrier card

Revision 1.15  2014/08/26 08:48:42  bowang3
Make NIM support NGSM carrier card Thule

Revision 1.14  2014/07/10 22:23:49  mcharon
if tftp fails, call 'cterr' instead of 'print'

Revision 1.13  2014/06/18 02:56:18  xiaoyizh
Remove the call of obsolete routine ovld_bcm_check_port_init().

Revision 1.12  2014/05/30 04:18:18  xiaoyizh
Modify uart test to bypass tty driver.
Add routine to get host diag flags and pass to module side.

Revision 1.11  2014/05/18 07:14:47  mcharon
printf needs to come before cterr to display proper with stop on error flag

Revision 1.10  2014/03/28 07:36:20  xiaoyizh
Start the NIM test on single character command.

Revision 1.9  2013/12/31 03:27:09  xiaoyizh
Fix compiling errors.

Revision 1.8  2013/12/27 03:32:51  xiaoyizh
Retry if status file is empty.
Modify the module firmware name for TFTP download.

Revision 1.7  2013/11/13 07:17:42  xiaoyizh
Remove nc_cmd_run_prince_diag_loop().

Revision 1.6  2013/11/12 04:14:44  xiaoyizh
Modify the baud rate from 115200 to 9600 for console redirect.
Change the character sent to Prince module for uart test.

Revision 1.5  2013/09/13 06:53:52  liwwang
Add polling for Prince module side diag menu up in Prince NIM test before
setting up nc request. Extend the maximum delay time of the polling to 400s
in both NIM and interface test. Support downloading Prince FW automatically
to Overlord in diag via TFTP.

Revision 1.4  2013/08/29 05:03:10  liwwang
Modify prince local ip address. Support diag flags for Prince NIM Test. Change UART test send pattern 'q' to 'p'

Revision 1.3  2013/07/01 05:45:32  liwwang
Add nc command support to run prince side test,Update uart test trigger item

Revision 1.2  2013/04/28 05:20:11  liwwang
update menu format per ngwic standard

Revision 1.1  2013/02/26 02:35:18  liwwang
check in support file for Prince ngwic on Overlord x86 platform
$Endlog$
*/

