/* $Id: platform_plug_serial_test.h,v 1.7 2018/11/23 09:28:46 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_serial/platform_plug_serial_test.h,v $
 *------------------------------------------------------------------
 *
 * plug_serial_test.h - Header file for plug_serial_test.c
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLATFORM_PLUG_SERIAL_TEST__
#define __PLATFORM_PLUG_SERIAL_TEST__

#define PLUG_SERIAL_REQUEST_PORT            (2013)
#define PLUG_SERIAL_STATUS_PORT1            (2016)
#define PLUG_SERIAL_SERDES_PORT             (2023)
#define PLUG_SERIAL_HOST_FLAG_PORT          (3013)
#define PLUG_SERIAL_SERDES_TYPE_PORT        (3023)
#define PLUG_SERIAL_RESET_WAIT_IN_MS        (150)
#define PLUG_SERIAL_TIMEOUT                 (100)
#define PLUG_SERIAL_UBOOT_TIME              (10)
#define PLUG_SERIAL_WAIT_TIME               (200)
#define PLUG_SERIAL_BOOT_TIME               (1000)
#define PLUG_SERIAL_UNRESET_WAIT            (1000)
#define PLUG_SERIAL_WAIT_ONE_SEC            (1000)
#define PLUG_SERIAL_CMD_WAIT_TIME           (3000)
#define PLUG_SERIAL_LOCAL_IP_ADDR           "192.168.2.200"
#define PLUG_SERIAL_SRC_IMG                 "/var/lib/tftpboot/p_1t_fw.img"
#define PLUG_SERIAL_DEST_IMG                "p_1t_fw.img"
#define PLUG_SERIAL_CR_STRING               "\015"
#define PLUG_SERIAL_CR_C_STRING             "\003"
#define PLUG_SERIAL_BOOT_CMD                "bootp 0x00400000 "
#define PLUG_SERIAL_BOOT_CMD_TAIL           ":p_1t_fw.img; bootm 0x00400000"
#define PLUG_SERIAL_UBOOT_STRING            "cisco-uboot>"
#define PLUG_SERIAL_AUTOBOOT_PROMPT         "## Warning: gatewayip"
#define PLUG_SERIAL_LINUX_PROMPT            "cisco>"
#define PLUG_SERIAL_ESC_CR_STRING           "\033\015"
#define PLUG_SERIAL_UBOOT_SAVE              "saveenv"
#define PLUG_SERIAL_DISPLAY_UBOOT           "printenv"
#define PLUG_SERIAL_SERDES_TYPE_TEST        "o\n" /* o: SerDes Type GPIO Test */
#define DIAG_KILL_NC_TMP_FILE               "/tmp/pluggable_serial_nc_tmp.pid"
#define DIAG_RTN_PASS_STR                   "PASS"
#define DIAG_RTN_STR_LEN                    (4)
#define PRIMARY_READY                       (4)
#define CHECK_EXP_DATA                      (4)
#define FOUR_SERDES_TYPE                    (4)
#define CHECK_IMAGE_BOOTUP                  (4)
#define BOOT_TIMEOUT                        (300)
#define SYS_PROC_PRINTK_FILE                "/proc/sys/kernel/printk"
#define SYS_CHANGE_PRINTK_LEVEL             "dmesg -n"
#define SYS_SUPPRESS_PRINTK_LEVEL           (3)
#define SYS_RESTORE_PRINTK_CMD              "dmesg -n"
#define ARP_PING_CMD                        "arping -I "
#define ARP_PING_CMD_TAIL                   " -c 3 -w 3 192.168.2.200"
#define HOST_IP                             "192.168.2"
#define HOST_FLAG                           "/tmp/host_flags"
#define ARP_PING_CHK_STRING                 "Unicast reply from 192.168.2.200"
#define NC_LENGTH                           (1024)
#define EMPTY                               (-1)
/* For UART Driver */

typedef enum {
    INPUT,
    OUTPUT
} gpio_dirc;

typedef enum {
    LOW,
    HIGH
} gpio_driv;

typedef enum {
    LED_OFF,
    LED_AMBER,
    LED_GREEN
} gpio_led_action;

typedef enum {
    PORT0,
    PORT1
} gpio_port;

typedef struct gpio_exp_init {
    int dev;
    int dir;
    int port;
    int bit;
    int dev_fun;
    int def_val;
} serial_gpio_exp;

typedef enum {
    ENABLE_LED_GREEN = 0,
    ENABLE_LED_YELLOW,
    HOST_SERDES_TYPE_0,
    HOST_SERDES_TYPE_1,
    PRIMARY_INTERFACE_READY,
    DYING_GASP_OK,
    USB_DEBUG_ENABLE,
    WDISABLE_1,
    WDISABLE_2,
    RESET,
    MODEM_POWER_OFF, 
    LED_SIM0_OK,
    LED_SIM0_NOT_OK,
    LED_SIM1_OK,
    LED_SIM1_NOT_OK,
    LED_GPS_OK,
    LED_GPS_NOT_OK,
    LED_RSSI0,
    LED_RSSI1,
    LED_RSSI2,
    LED_RSSI3,
    LED_4G_3G,
    WWAN_LED_ENABLE,
    WWAN_LED_SIM_SEL,
    SIM_SELECT,
    SIM0_DETECT,
    SIM1_DETECT
} gpio_func;
typedef struct uboot_info {
    char           *name;
    char           *value;
} uboot_info_t;


extern int plug_serial_main(void *);
extern int system(const char *); 
extern int plug_serial_uart_setup(char *);
extern int pluggable_serial_bootup_image(void);
extern int ExecuteCmdbyPopen(char *, char *, int);
extern int plug_serial_enable_led(int);
extern struct plug_intf_t *plug_serial_iface;
extern int plug_serial_rx_polling_uart(char *, char *, int);
extern int plug_serial_tx_uart(char *, char *);
extern int tftp_get(unsigned char *, unsigned char *, 
                    unsigned char *, unsigned char *, unsigned int);
extern int plug_fpga_i2c_ack_check(int, uint8_t, uint32_t,
                                   int32_t, uint32_t, uint32_t, uchar *);
extern void	disable_bp_ge_lpbk(void);
extern char uart_device_name[64];
extern char uart_driver_path[64];



#endif

/******** History ********
$Log: platform_plug_serial_test.h,v $
Revision 1.7  2018/11/23 09:28:46  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.6  2018/09/25 08:28:55  iachang
CSCvm33713: Fixed Serial_1T_bootup_faild by 60C in EEDVT

Revision 1.5  2018/09/21 03:01:16  iachang
CSCvm45577: Fixed SerDes Type GPIO test issue

Revision 1.4.10.2  2018/11/21 09:37:22  iachang
Sync up with main trunk.

Revision 1.4.10.1  2018/10/15 06:51:13  hondwang
pluggable common code re-instruct modify code

Revision 1.4  2018/08/02 09:35:01  iachang
Merge Pluggable Serial from branch star-branch-c9xx to main trunk

Revision 1.3  2018/02/09 09:17:33  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/01/24 08:53:05  iachang
CSCvh67800: Fixed NC command fail intermittent issue

Revision 1.2.2.2  2018/01/20 06:54:53  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 04:58:56  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.9  2017/12/13 11:45:44  iachang
Add Pluggable Serial Bootup utility.

Revision 1.1.4.8  2017/11/09 09:37:18  iachang
Suppress printk to get UART driver message
Restore IOS U-BOOT parameters utility

Revision 1.1.4.7  2017/10/24 11:16:05  iachang
Supported SerDes Type GPIO Test.

Revision 1.1.4.6  2017/10/13 02:51:16  iachang
Modify the UART Test.

Revision 1.1.4.5  2017/09/27 00:06:28  iachang
Moved Listen Host diag flags NC command to module kernel initial script
Added insert Sirius FPGA UART driver

Revision 1.1.4.4  2017/09/26 03:28:27  iachang
Changed Reset Pin test from Module reset to I2C reset Pin

Revision 1.1.4.3  2017/08/22 03:29:58  lucywang
set 1000Base-X for pluggable serial and set sgmii for pluggable test card


$Endlog$
*/
