/* $Id: wifi_tests.h,v 1.4 2018/02/09 09:56:56 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/wifi_tests.h,v $
 *------------------------------------------------------------------
 *
 * wifi_tests.h - This file contains definitions for TSN-Turbo Wifi.
 *
 * by: leslie
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _WIFI_TESTS_H_
#define _WIFI_TESTS_H_

#define TSN_WIFI_PARMS_LENGTH            (64)
#define TSN_WIFI_MAX_LENGTH              (128)
#define TSN_WIFI_MAX_RETRY               5
#define WIFI_ERRMSG_SIZE                 256   /* Error buffer: 256 bytes */

#define POLLING_WIFI_UBOOT_TIME          30   /* 30sec */
#define POLLING_HOST_WIFI_CONNECT_TIME   3    /* 3 times */
#define POLLING_WIFI_KERNEL_PROMPT_TIME  150  /* 5 minutes */
#define WIFI_LINUX_WAIT_RETRY_TIMES      (90)
#define WAIT_WIFI_ACCESS_TIME            (300)
#define WAIT_WIFI_LINUX_PROMPT           (1000)
#define TSN_WIFI_CR_STRING               "\015\015"
#define TSN_UART_SEND_ESC_KEY            "\033\012"
#define TSN_WIFI_PRINTENV_STRING         "printenv\n"
#define WIFI_UBOOT_IPADDR                "ipaddr"
#define WIFI_UBOOT_NETMASK               "netmask"
#define TSN_WIFI_NETMASK                 "255.255.255.0"
#define WIFI_UBOOT_GATEWAYIP             "gatewayip"
#define TSN_WIFI_GATEWAYIP               "192.168.1.100"
#define WIFI_UBOOT_SERVERIP              "serverip"
#define TSN_WIFI_SERVERIP                "192.168.1.100"
#define WIFI_UBOOT_BOOTFILE              "bootfile"
#define TSN_WIFI_BOOTFILE                "tsn_wifi_diag_kernel.img.SSA"
#define TSN_WIFI_PING_SERVER             "ping 192.168.1.100\n"
#define TSN_WIFI_TFTP_SERVER_ALIVE       "is alive"
#define TSN_WIFI_TFTPBOOT_KERNEL         "bootipq tftp\n"
#define TSN_WIFI_IFCONFIG_STRING         "ifconfig\n"
#define TSN_WIFI_RPS_STRING              "RPS"
#define TSN_WIFI_STOP_AUTOBOOT_STRING    "bootipq directly"
#define TSN_WIFI_HIT_ANY_KEY_TIMEOUT     (30000)
#define TSN_WIFI_CATCH_STR_TIMEOUT       (1000)
#define TSN_WIFI_UBOOT_PROMPT_STRING     "(BTLDR) #"
#define TSN_WIFI_LINUX_PROMPT_STRING     "OpenWrt:/#"
#define TSN_WIFI_LINUX_SPECIAL_STRING    "console."
#define RESET_20_MILLISECONDS            20
#define UNRESET_20_MILLISECONDS          20
#define TSN_WLAN_ALIVE_TEST_RESULT       "/diag/tsn_wlan_alive_test_result.txt"

#define TSN_WIFI_TURN_LED_ON_RED         "gpio_test --led --red\n"
#define TSN_WIFI_TURN_LED_ON_GREEN       "gpio_test --led --green\n"
#define TSN_WIFI_TURN_LED_ON_AMBER       "gpio_test --led --amber\n"
#define TSN_WIFI_TURN_ALL_LEDS_OFF       "gpio_test --led --all-off\n"

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

/* Star PIDs definition */
/* Star Host PIDs */
#define STAR_C941_PID_STR   "C1101-" 
#define STAR_C949_PID_STR   "C1109-" 
/* Star WiFi PIDs */
/* Star WiFi PIDs */
#define STAR_WIFI_CTEMP_STR "ISR-AP1101AC-"
#define STAR_WIFI_ITEMP_STR "ISR-AP1101AC-I-"

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

/*extern*/
extern int wifi_tests(int);
extern int wifi_reset_init(void);
extern int wlan_io_test(void);

#endif /*_WIFI_TESTS_H_*/

/*-------------------------------------------------
$Log: wifi_tests.h,v $
Revision 1.4  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.3  2017/10/19 13:41:11  palin2
Fixed CSCvg23616: TSN PoE link down intermittently when connect to iPorter PoE tester.

Revision 1.2  2017/08/02 14:21:50  steja
Support TSN-H/M platform code

Revision 1.1.8.4  2017/08/01 14:02:06  steja
Enhanced Wifi Diag Kernel boot up

Revision 1.1.8.3  2017/07/31 16:35:47  palin2
Updated WiFi Diag kernel boot up process based on Cisco WiFi bootloader.

Revision 1.1.8.2  2017/07/29 03:41:21  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/21 10:46:04  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:08  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.6  2016/11/16 06:20:50  palin2
Added netcat support for TSN WiFi tests.

Revision 1.1.4.5  2016/10/02 20:32:27  palin2
Enhanced WiFi uart code to fix CSCvb53793.

Revision 1.1.4.4  2016/09/13 08:14:23  palin2
Added CPU to GE PHY MAC loopback test.

Revision 1.1.4.3  2016/08/10 12:41:20  palin2
Updated WiFi testing procedure.

Revision 1.1.4.2  2016/06/30 06:22:52  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.2  2016/06/17 15:26:25  palin2
Added WLAN module diags and utilities.

Revision 1.1.2.1  2016/05/20 02:33:07  leschen
Check in wifi codes

*
*/


