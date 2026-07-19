/* $Id: diag_wifi_lib.h,v 1.2 2021/09/24 01:21:07 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_wifi_lib.h,v $
 *------------------------------------------------------------------
 *
 * diag_wifi_lib.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#ifndef __DIAG_WIFI_LIB_H__
#define __DIAG_WIFI_LIB_H__

#include "dev_nxp_lm75b.h"

/* Common */
#define PLAT_WIFI_IPADDR          "192.168.3.1"
#define	WIFI_TEMP_SENSOR_I2C_ADDR   (0x48)  /* WiFi temp. sensor I2C addr. */
#define WIFI_RESET_INTERVAL         (500)   /* 500ms */
#define PLAT_WIFI_NC_ENTRY_EN       "plat_diag_nc_entry &"
#define PLAT_WIFI_NC_RDY_PORT       (2294)
#define PLAT_WIFI_NC_RDY_FILE       "/tmp/plat_wifi_nc_rdy.txt"
#define PLAT_WIFI_NC_TIMEOUT        20000  /* 20sec */
#define PLAT_WIFI_NC_PULL_INTVL     1000   /* 1sec */
#define PLAT_NC_EXEC_TIME           120    /* sec */
#define PLAT_NC_BUF_SIZE            2048  /* Bytes */


#define DIAG_PLAT_NC_KILL_TMP_FILE          "/tmp/plat_nc_rm.pid"
#define DIAG_PLAT_NC_COMMAND_DISPATCH_FILE  "/tmp/plat_nc_comm_dispatch"
#define PLAT_NC_EXEC_LOG_FILE               "/tmp/plat_nc_exec_log.txt"
#define PLAT_NC_DONE_FILE                   "/tmp/plat_nc_exec_done.txt"

#define DIAG_PLAT_NC_RTN_PASS_STR           "PASS"
#define DIAG_PLAT_NC_ACK_STR                "ACK"
#define DIAG_PLAT_NC_NACK_STR               "NACK"

/* WiFi LED control state */
enum wifi_led_control_state {
    WIFI_LED_OFF = 0,
    WIFI_LED_GREEN,
    WIFI_LED_RED,
    WIFI_LED_BLUE,
};

/* WiFi Diag NC command definition */
#define TURN_WIFI_LED_OFF_NC     "turn_wifi_led_off"
#define TURN_WIFI_LED_RED_NC     "turn_wifi_led_red"
#define TURN_WIFI_LED_GREEN_NC   "turn_wifi_led_green"
#define TURN_WIFI_LED_AMBER_NC   "turn_wifi_led_amber"
#define WIFI_DIAG_NOR_TEST_NC    "do_wifi_nor_test"
#define WIFI_DIAG_MEM_TEST_NC    "do_wifi_mem_test"
#define GET_WIFI_PID_NC          "get_wifi_pid"

#define PLAT_WIFI_PARMS_LENGTH            (64)
#define PLAT_WIFI_MAX_LENGTH              (128)
#define PLAT_WIFI_MAX_RETRY               5
#define WIFI_ERRMSG_SIZE                 256   /* Error buffer: 256 bytes */

#define POLLING_WIFI_UBOOT_TIME          30   /* 30sec */
#define POLLING_HOST_WIFI_CONNECT_TIME   50    /* 50 times */
#define POLLING_WIFI_KERNEL_PROMPT_TIME  150  /* 5 minutes */
#define POLLING_WIFI_READY_PIN_TIME      (300)
#define WIFI_LINUX_WAIT_RETRY_TIMES      (90)
#define WAIT_WIFI_ACCESS_TIME            (300)
#define WAIT_WIFI_LINUX_PROMPT           (1000)
#define WAIT_WIFI_PROMPT                 (1000)
#define PLAT_WIFI_CR_STRING               "\015\015"
#define PLAT_WIFI_ONE_CR                  "\015"

#define PLAT_UART_SEND_ESC_KEY            "\033\012"
#define PLAT_UART_SEND_SPACE_KEY          "\040\012"
#define PLAT_WIFI_PRINTENV_STRING         "printenv\n"
#define WIFI_UBOOT_IPADDR                 "ipaddr"
#define WIFI_UBOOT_NETMASK                "netmask"
#define PLAT_WIFI_NETMASK                 "255.255.255.0"
#define WIFI_UBOOT_GATEWAYIP              "gatewayip"
#define PLAT_WIFI_GATEWAYIP               "192.168.3.100"
#define WIFI_UBOOT_SERVERIP               "serverip"
#define PLAT_WIFI_SERVERIP                "192.168.3.100"
#define WIFI_UBOOT_BOOTFILE               "bootfile"
#define PLAT_WIFI_BOOTFILE                "tsn_wifi_diag_kernel.img.SSA"
#define PLAT_WIFI_PING_SERVER             "ping 192.168.3.100\n"
#define PLAT_WIFI_TFTP_SERVER_ALIVE       "is alive"
#define PLAT_WIFI_TFTPBOOT_KERNEL         "tftpboot\n"
#define PLAT_WIFI_IFCONFIG_STRING         "ifconfig\n"
#define PLAT_WIFI_RPS_STRING              "RPS"
#define PLAT_WIFI_STOP_AUTOBOOT_STRING    "bootipq directly"
#define PLAT_WIFI_HIT_ANY_KEY_TIMEOUT     (30000)
#define PLAT_WIFI_CATCH_STR_TIMEOUT       (1000)
#define PLAT_WIFI_UBOOT_PROMPT_STRING     "u-boot>"
#define PLAT_WIFI_LINUX_PROMPT_STRING     "#"
#define PLAT_WIFI_LINUX_SPECIAL_STRING    "console."
#define RESET_20_MILLISECONDS             (20)
#define UNRESET_20_MILLISECONDS           (20)
#define PLAT_WLAN_ALIVE_TEST_RESULT       "/diag/plat_wlan_alive_test_result.txt"

#define PLAT_WIFI_TFTPDIR                 "setenv tftpdir\n"
#define PLAT_WIFI_TFTP_DOWNLOAD           "netboot diag-wifi-kernel-image.bin\n"

#define PLAT_WIFI_ERASE_NAND              "nand erase.chip"
#define PLAT_WIFI_WRITE_NAND              "nand write 0x10000000 0 ${filesize}"
#define PLAT_WIFI_RESET                   "reset"

#define PLAT_WIFI_TURN_LED_ON_RED         "fdiag_led_test red-on\n"
#define PLAT_WIFI_TURN_LED_ON_GREEN       "fdiag_led_test green-on\n"
#define PLAT_WIFI_TURN_LED_ON_BLUE        "fdiag_led_test amber-on\n"
#define PLAT_WIFI_TURN_ALL_LEDS_OFF       "fdiag_led_test all-off\n"

/* NC Command Dispatch */
#define DIAG_IPQ4019_DRAM_TEST         "ipq4019_dram_test"
#define DIAG_IPQ4019_NOR_FLASH_TEST    "ipq4019_nor_flash_test"
#define DIAG_IPQ4019_NAND_FLASH_TEST   "ipq4019_nand_flash_test"
#define DIAG_IPQ4019_LED_TEST          "ipq4019_led_test"
#define DIAG_PING_IPQ4019              "ping_ipq4019"
#define WIFI_NC_RETDATA_DELIMITER      ", \r\n"

/* Fail Type definitions */
#define EXIT_FAIL_NONSTARTER                            0x01
#define EXIT_FAIL_ADDRESSLINES                          0x02
#define EXIT_FAIL_OTHERTEST                             0x04

/* Fail Subtype definitions */
#define EXIT_FAIL_OTHERTEST_RANDOM_VALUE                0x01
#define EXIT_FAIL_OTHERTEST_XOR_COMPARISON              0x02
#define EXIT_FAIL_OTHERTEST_SUB_COMPARISON              0x03
#define EXIT_FAIL_OTHERTEST_MUL_COMPARISON              0x04
#define EXIT_FAIL_OTHERTEST_DIV_COMPARISON              0x05
#define EXIT_FAIL_OTHERTEST_OR_COMPARISON               0x06
#define EXIT_FAIL_OTHERTEST_AND_COMPARISON              0x07
#define EXIT_FAIL_OTHERTEST_SEQINC_COMPARISON           0x08
#define EXIT_FAIL_OTHERTEST_SOLIDBITS_COMPARISON        0x09
#define EXIT_FAIL_OTHERTEST_BLOCKSEQ_COMPARISON         0x0a
#define EXIT_FAIL_OTHERTEST_CHECKBOARD_COMPARISON       0x0b
#define EXIT_FAIL_OTHERTEST_BITSPREAD_COMPARISON        0x0c
#define EXIT_FAIL_OTHERTEST_BITFLIP_COMPARISON          0x0d
#define EXIT_FAIL_OTHERTEST_WALKBITS1_COMPARISON        0x0e
#define EXIT_FAIL_OTHERTEST_WALKBITS0_COMPARISON        0x0f

#define INVALID_TYPE                                    0x00
#define WIFI_GPIO18   18
#define WIFI_GPIO39   39
#define WIFI_GPIO40   40
#define WIFI_GPIO45   45
#define WIFI_GPIO48   48

#define TLMM_GPIO_CONF_REG_OFFSET(x)   (0x1000000 + 0x1000 * (x))

typedef struct wifi_type_string_t
{
    unsigned int    code;
    unsigned char   *name;
} wifi_type_string_t;

struct nc_args {
    char arg[64]; 
    struct nc_args *next;
};

typedef struct wifi_gpio
{
    char*    name;
    int      gpio_num;
    uint16_t value;
} wifi_gpio_t;

typedef struct host_wifi_pid_map_t {
    const char *wifi_pid_str;
    const char *host_pid_str;
} host_wifi_pid_map;

/* Extern */
extern int wifi_ts_dev_create(dev_lm75b_object_t *);
extern int wifi_ts_dev_create(dev_lm75b_object_t *);
extern int wifi_led_control(int);
extern int put_wifi_in_reset(void);
extern int release_wifi_from_reset(void);
extern int wifi_set_uboot_tftp(char *, char *, char *);
extern int plat_wifi_nc_dispatch_comm(char *cmd_str);


#endif   /* __DIAG_WIFI_LIB_H__ */

/*-------------------------------------------------
 * $Log: diag_wifi_lib.h,v $
 * Revision 1.2  2021/09/24 01:21:07  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.10  2021/05/05 06:17:13  illiu
 * Remove redundant function: wifi_enable_platform_wifi_eth() and wifi_confirm_gpio()
 *
 * Revision 1.1.2.9  2021/03/05 07:13:36  illiu
 * Rename wifi kernel image name
 *
 * Revision 1.1.2.8  2021/03/03 01:33:29  illiu
 * Modify wifi bootup kernel from foxconn u-boot version to cisco u-boot version
 *
 * Revision 1.1.2.7  2020/12/03 01:37:55  illiu
 * Modify the procedure of wifi tftp boot test
 *
 * Revision 1.1.2.6  2020/11/19 09:36:21  illiu
 * Fix wifi tftp boot test
 *
 * Revision 1.1.2.5  2020/11/12 06:36:51  illiu
 * 1. Add WiFi module Bootup Test item
 * 2. Add WiFi module Memory Test item
 * 3. Add WiFi module NOR flash Test item
 * 4. Fix WiFi LED control Util item
 *
 * Revision 1.1.2.4  2020/10/13 06:02:39  harrchan
 * Modify nc commnad string for elixir wifi test
 *
 * Revision 1.1.2.3  2020/09/22 05:52:02  harrchan
 * Modify nc commnad string for elixir wifi test
 *
 * Revision 1.1.2.2  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.1  2020/09/09 09:09:53  illiu
 * First version which has been ported with Dreamliner and Marvell CPSS
 *
 * Revision 1.2  2019/01/10 06:36:28  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
