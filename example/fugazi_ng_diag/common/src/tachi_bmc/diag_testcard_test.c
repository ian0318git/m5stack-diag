/* $Id: diag_testcard_test.c,v 1.2 2016/04/20 11:25:26 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_testcard_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_testcard_test.c - BTB Test Card Test functions
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "nvmonvars.h"
#include "diag_testcard_test.h"
#include "common_utils.h"
#include "proto.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "diag_i2c_api.h"
#include "diag_i2c_test.h"
#include "diag_nc_common.h"
#include "intel_tests.h"
#include "diag_lewis_gesw_test.h"
#include "linux_api.h"
#include "diag_console_util.h"
#include "platform_fru.h"

int diag_testcard_build_test(int);
int diag_testcard_io_test(void);

static uint32_t fpga_reg_test(void);
static uint32_t btb_x86_test(void);
static uint32_t btb_isp_lewis_test(void);
static uint32_t btb_isp_pci_test(void);
static uint32_t btb_isp_uart_test(void);
static int fpga_i2c_bmc_reg_read(unsigned long, int, unsigned long *, void *);
static int fpga_i2c_bmc_reg_write(unsigned long, int, unsigned long, void *);
static int fpga_i2c_iofpga_reg_read(unsigned long, int, unsigned long *, void *);
static int fpga_i2c_iofpga_reg_write(unsigned long, int, unsigned long, void *);

static reg_info_t_ext fpga_bmc_reg_ext = {1, fpga_i2c_bmc_reg_read, 
                                             fpga_i2c_bmc_reg_write, 0};
static reg_info_t_ext fpga_iofpga_reg_ext = {1, fpga_i2c_iofpga_reg_read, 
                                             fpga_i2c_iofpga_reg_write, 0};

static reg_info_t fpga_bmc_reg_tbl[] = {
    {"Scratchpad Register", FPGA_SCRATCHPAD_REG_OFFSET,
     (READ_WRITE | REG_ACCESS), {(unsigned long)&fpga_bmc_reg_ext}, 0xFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};

static reg_info_t fpga_iofpga_reg_tbl[] = {
    {"Scratchpad Register", FPGA_SCRATCHPAD_REG_OFFSET,
     (READ_WRITE | REG_ACCESS), {(unsigned long)&fpga_iofpga_reg_ext}, 0xFF, 0x0},
    {"END",                       0x00,  0,           {0},   0x0,  0x0},
};

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/* Sub Menu used for Test Card Main tests.
 */
static submenu_xtable_t testcard_main_tests_submenu_table[] = {
    {"FPGA I2C Register Test", (type_t(*)())fpga_reg_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BTB x86 Test", (type_t(*)())btb_x86_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BTB ISP Lewis Test", (type_t(*)())btb_isp_lewis_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BTB ISP PCI bus Test", (type_t(*)())btb_isp_pci_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"BTB ISP UART Test", (type_t(*)())btb_isp_uart_test,   0,
    MF_3, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define TESTCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE (sizeof(testcard_main_tests_submenu_table) / \
                       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t testcard_main_tests_primary_items[TESTCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                       MAX_BASE_ITEMS];
static mitem_t testcard_main_tests_secondary_items[TESTCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE +
                     MAX_BASE_ITEMS];

menuinfo_t testcard_main_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    testcard_main_tests_primary_items,
};
menuinfo_t *testcard_main_submenup = &testcard_main_subtest_menu;

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
int diag_testcard_build_test (int run_all_tests)
{
    build_primary_submenu(testcard_main_tests_submenu_table,
                          TESTCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                          "TEST CARD", &testcard_main_submenup);
    build_secondary_submenu(testcard_main_tests_submenu_table,
                            TESTCARD_MAIN_TESTS_SUBMENU_TABLE_SIZE,
                            testcard_main_tests_secondary_items);

    if (run_all_tests) {
        exec_doall_menu_items(testcard_main_submenup);
    } else {
        menu(testcard_main_submenup, testcard_main_tests_secondary_items, '\0');
    }
    return (PASSED);
}

int diag_testcard_io_test (void)
{
    int ret;

    /* FPGA REG test */
    ret = fpga_reg_test();

    /* ISP UAR Test */
    ret = btb_isp_uart_test();

    return (ret);
}

static uint32_t btb_x86_test (void)
{
    int rc;
    sighandler_t old_handler;

    testname("Check INTEL linux Ready");
    prpass(testpass, "Check INTEL linux Ready");
    /* Backup SIGNAL before system call  */
    old_handler = signal(SIGCHLD, SIG_DFL);
    if (check_intel_linux_ready()) {
        /* recover system call SIGNAL  */
        signal(SIGCHLD, old_handler);
        cterr('f', 0, "Check INTEL linux ready Failed");
        return (FAILED);
    }
    /* recover system call SIGNAL  */
    signal(SIGCHLD, old_handler);
    prcomplete(testpass, errcount, 0);

    rc = diag_nc_intel_btb_test();
    return (rc);
}

static uint32_t btb_isp_pci_test (void)
{
    int rc;
    sighandler_t old_handler;

    testname("Check INTEL linux Ready");
    prpass(testpass, "Check INTEL linux Ready");
    /* Backup SIGNAL before system call  */
    old_handler = signal(SIGCHLD, SIG_DFL);
    if (check_intel_linux_ready()) {
        /* recover system call SIGNAL  */
        signal(SIGCHLD, old_handler);
        cterr('f', 0, "Check INTEL linux ready Failed");
        return (FAILED);
    }
    /* recover system call SIGNAL  */
    signal(SIGCHLD, old_handler);
    prcomplete(testpass, errcount, 0);

    rc = diag_intel_isp_test_pci_if_test();

    return (rc);
}

static void
add_isp_ping_test_err_report(void)
{
    fru_table_offset = ISP_TEST_SGMII;
    platform_fru_table[ISP_TEST_SGMII].pid_string = isp_test_sgmii;
    platform_fru_table[ISP_TEST_SGMII].location_string = isp_test_sgmii_loc;
}


static uint32_t btb_isp_lewis_test (void)
{
    int ret = PASSED;
    int len;
    char test_comm[128];
    char src_ip[32];
    char *dest_ip;

    if (get_enhance_err_flag()) {
        add_isp_ping_test_err_report();
        cterr_add_component("ISP Testcard", "ISP Testcard SGMII");
        cterr_add_debug("Check Lewis system is power up.",
            "Check ISP Testcard connection.",
            "Check ISP Testcard connector",
            "Check data path trace from Motherboard to ISP Testcard.");
    }

    testname("ISP Testcard ping Test ");
    prpass(testpass, "ISP Testcard ping Test");

    /* we use gatewayip or tftp server ip as the destination ip*/
    if (getenv("DEFAULT_GATEWAY")) {
        dest_ip = getenv("DEFAULT_GATEWAY");
    } else {
        dest_ip = getenv("TFTP_SERVER");
    }

    bzero(src_ip,32);
    if((getenv("IP_ADDRESS") == NULL) || (dest_ip == NULL)) {
        printf("src_ip or dest_ip is not null\n");
        printf("src_ip is %s\n",src_ip);
        printf("dest_ip is %s\n",dest_ip);
        cterr('f', 0, "BTB ping test Failed");
        return (FAILED);
    }
    /* we use IP_ADDRSS +1 to be the BTB test ip address */
    len = strlen(getenv("IP_ADDRESS"));
    strncpy(src_ip, getenv("IP_ADDRESS"), len);
    (*(src_ip + len -1))++;
    printf("\nsrc_ip is %s\n",src_ip);
    printf("dest_ip is %s\n",dest_ip);

    sprintf(test_comm, "%s %s %s %s %s %s%s", NC_MVL_BTB_TEST, NC_MVL_BTB_SRC_IP,
            src_ip, NC_MVL_BTB_DST_IP, dest_ip, NC_MVL_BMC_CONN, "\r");

    if (run_lewis_gesw_test(test_comm, 1)) {
        cterr('f', 0, "ISP testcard Lewis ping test Failed");
        ret = FAILED;
    }

    prcomplete(testpass, errcount, 0);
    return (ret);
}


static uint32_t fpga_reg_test (void)
{
    int retval = PASSED;
    testname("FPGA Register");
    prpass(testpass, "FPGA I2C-1 Register Test");

    if (register_tests(0, &fpga_bmc_reg_tbl[0]) == FAILED) {
        return (FAILED);
    }

    prpass(testpass, "FPGA I2C-2 Register Test");
    if (register_tests(0, &fpga_iofpga_reg_tbl[0]) == FAILED) {
        return (FAILED);
    }
    prcomplete(testpass, errcount, 0);
    return (retval);
}


static int fpga_i2c_bmc_reg_read (unsigned long addr, int size,
                                  unsigned long *buf, void *param)
{
    n2g_i2c_if_t i2c_if;
    int rc;

    i2c_if.i2c_bus_type = CPU_I2C5;
    i2c_if.i2c_dev = (TEST_CARD_I2C_ADDR << 1);
    i2c_if.buf = (char *)buf;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = addr;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to read. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    return (PASSED);
}

static int fpga_i2c_bmc_reg_write (unsigned long addr, int size,
                                   unsigned long buf, void *param)
{
    n2g_i2c_if_t i2c_if;
    int rc;

    i2c_if.i2c_bus_type = CPU_I2C5;
    i2c_if.i2c_dev= (TEST_CARD_I2C_ADDR << 1);
    i2c_if.buf = (char *)&buf;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = addr;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to write. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    return (PASSED);
}

static int fpga_i2c_iofpga_reg_read (unsigned long addr, int size,
                                     unsigned long *buf, void *param)
{
    n2g_i2c_if_t i2c_if;
    int rc;

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_dev= TEST_CARD_I2C_ADDR;
    i2c_if.i2c_ctrl = I2C_CTRL_FIFTEEN;
    i2c_if.buf = (char *)buf;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.offset = addr;

    rc = n2g_i2c_read(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to read. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    return (PASSED);
}

static int fpga_i2c_iofpga_reg_write (unsigned long addr, int size,
                                      unsigned long buf, void *param)
{
    n2g_i2c_if_t i2c_if;
    int rc;

    i2c_if.i2c_bus_type = IOFPGA_I2C;
    i2c_if.i2c_dev= TEST_CARD_I2C_ADDR;
    i2c_if.i2c_ctrl = I2C_CTRL_FIFTEEN;
    i2c_if.size = sizeof(uint8_t);
    i2c_if.buf = (char *)&buf;
    i2c_if.offset = addr;

    rc = n2g_i2c_write(&i2c_if);
    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to write. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    return (PASSED);
}

static void
* isp_read_aux (void *u)
{
    int timeout = 5; /*in secs */
    int size = 0; /* when size= 0, read all bytes from uart controller */

    s_uart *uart = (s_uart *)u;

    if (rx_uart(uart->dev, size, (char *)uart->buf, timeout, uart->tst_typ) < 0) {

    }
    pthread_exit(NULL);
}

static int
isp_uart_intf_test (char *dev, char *test_str, speed_t test_speed)
{   
    struct termios config, ori_conf;
    int uart_fd, ret_val = PASSED, result = 0;
    char tx_str[100];
    char *pattern = "Dereck";
    pthread_t threads;

    s_uart uart;

    uart.dev =  dev;

    memset(uart.buf, '\0', sizeof(uart.buf));
    uart.tst_typ = DEFAULT_CASE;

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

    if(pthread_create(&threads, NULL, isp_read_aux, (void *)&uart)) {
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

    tx_uart(dev, test_str, 1);

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
        perror("\nisp_uart_intf_test(): data rx ok but Failed in tcsetattr()");
        ret_val = FAILED;
    }
    close(uart_fd);
    return (ret_val);

}

static void
add_isp_uart_test_err_report(void)
{
    fru_table_offset = ISP_TEST_UART;
    platform_fru_table[ISP_TEST_UART].pid_string = isp_test_uart;
    platform_fru_table[ISP_TEST_UART].location_string = isp_test_uart_loc;
}

static uint32_t btb_isp_uart_test(void)
{   
    int retval=PASSED;

    if (get_enhance_err_flag()) {
        add_isp_uart_test_err_report();
        cterr_add_component("ISP Testcard", "ISP Testcard UART");
        cterr_add_debug("Check ISP Testcard connection.",
            "Check ISP Testcard connector",
            "Check data path trace from Motherboard to ISP Testcard.");
    }

    testname("ISP Testcard UART");
    prpass(testpass, "ISP Testcard UART Test");

    /* Set FPGA UART MUX to ISP card */
    diag_uart_to_isp_cnnt();

    retval = isp_uart_intf_test(UART_TTYS2_DEV,"0\n0\n",B9600);

    if (retval != PASSED) {
        cterr('f', 0, "ISP Testcard UART Test failed");
        return (FAILED);
    }

    prcomplete(testpass, errcount, 0);
    return (retval);

}


/*---------------------------------------------------------------
$Log: diag_testcard_test.c,v $
Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.8  2016/03/10 05:39:05  uid421098
Add ISP test card io test

Revision 1.1.2.7  2016/03/08 03:07:07  jimmyya
Add ISP testcard uart test

Revision 1.1.2.6  2016/02/26 09:00:22  hondwang
add intel enhance error message, pci bus scan

Revision 1.1.2.5  2016/02/20 16:20:18  hondwang
Add CPU and PCI bus testing

Revision 1.1.2.4  2016/01/12 08:32:20  jimmyya
add bzero to clean the src_ip buffer

Revision 1.1.2.3  2016/01/12 07:38:41  jimmyya
add BTB lewis test

Revision 1.1.2.2  2016/01/12 00:29:01  uid259484
modify to add INTEL NC utility show HDD, DIMM and linux version.
And add RAID card and BTB testing to daughter card item.

Revision 1.1.2.1  2016/01/11 10:50:59  tirawan
Add for the first time


$Endlog$
*/

