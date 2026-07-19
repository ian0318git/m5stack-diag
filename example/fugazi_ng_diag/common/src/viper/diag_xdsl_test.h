 /* $Id: diag_xdsl_test.h,v 1.4 2018/09/21 02:48:54 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_xdsl_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_xdsl_test.h 
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#ifndef _DIAG_XDSL_TEST_H_
#define _DIAG_XDSL_TEST_H_

#define DSL_PING_TOUT             (10)    /* 10 secs */
#define DSL_BL_PROMPT_TOUT        (30)    /* 30 secs */
#define DSL_DIAG_PROMPT_TOUT      (120)   /* 120 secs */
#define DSL_BOOT_UP_WAIT_30       30
#define DSL_BOOT_UP_WAIT_35       35
#define DSL_BAUDRATE_9600         9600 
#define DSL_DATABIT_8             8
#define DSL_PARITY_1             "1"
#define DSL_FLOW_N               "n"

/* environment variable */
#define DSL_IMAGE_NAME            "DSL_IMG_FILE"       
#define IOS_IMAGE_NAME            "IOSDSL_IMG_FILE"       
#define VIPER_DSL_UART_DEV_STR    "/dev/ttyS1"

#define VIPER_DSL_DHCPD           "/opt/tool/dhcpd &"
#define VIPER_DSL_OPENTFTP        "/opt/tool/opentftpd -i /etc/opentftp.ini &"
#define VIPER_DSL_KILL_DHCPD      "killall dhcpd"
#define VIPER_DSL_KILL_OPENTFTP   "killall opentftpd"

#define VIPER_IOS_HOST_FIRMWARE_FOLDER_STRING "/firmware/"
#define VIPER_IOS_VADSL_FIRMWARE_STRING "vadsl_module_img.bin"

#define DSL_AM_DIAG_IMG           "viper_dsl_am_diag.img"
#define DSL_BJ_DIAG_IMG           "viper_dsl_bj_diag.img"
#define DSL_DEFAULT_IMG_FILE      "vadsl_module_img.bin"
#define DSL_CFE_PRESS_KEY_TIMEOUT        (30000)  /* 30 second */
#define DSL_CFE_TIMEOUT                  (100)
#define DSL_LINUX_TIMEOUT                (100)
#define DSL_CFE_COMMAND_STATUS_TIMEOUT   (1000)
#define DSL_CFE_RETRY_TIMES              (30)
#define DSL_LINUX_WAIT_RETRY_TIMES       (90)
#define DSL_SELF_DISCONN_TIMEOUT         (12000)
#define DSL_SELF_DISCONN_RETRY_TIMES     (3)
#define DSL_CONN_TIMEOUT                 (18000)
#define VIPER_DSL_PRESS_KEY_STRING        "Press any key"
#define VIPER_DSL_CFE_STRING              "CFE>"
#define VIPER_DSL_CHANGE_CFE_PARMS_STRING "c"
#define VIPER_DSL_PRINT_CFE_PARMS_STRING  "p"
#define VIPER_DSL_COMMAND_STATUS_STRING   "command status"
#define VIPER_HOST_FIRMWARE_FOLDER_STRING "/firmware/"
#define VIPER_DSL_CR_STRING               "\r"
#define VIPER_DSL_LINUX_PROMPT            ">"
#define VIPER_DSL_IFCONFIG_BR0_STRING     "ifconfig br0 "
#define VIPER_DSL_SHELL_STRING            "sh"
#define VIPER_DSL_LINUX_PROMPT_RETRY      (10)
#define VIPER_DSL_INT_RETRY               (10)
#define VIPER_DSL_MAX_LENGTH              (128)
#define WAIT_DSL_LINUX_PROMPT            (1000)
#define WAIT_DSL_CFE_PROMPT              (500)
#define WAIT_DSL_UART_TX                 (500)
#define WAIT_DSL_CFE_COMMAND_STATUS      (200)
#define WAIT_DSL_IP_SETUP                (1000)
#define WAIT_FOR_FLASH_WRITE             (3000)
#define WAIT_DSL_SELF_DISCONN            (1000)
#define WAIT_DSL_CONN_START              (2000)
#define WAIT_DSL_IDLE_LISTEN             (25)
#define WAIT_FOR_CPLD_WRITE              (500)
#define WAIT_FOR_CTRL_DSL_INTR           (10)
#define CHECK_FPGA_DSL_PENDING_TIMES     (20)

/* NC Command Dispatch */
#define DIAG_BCM63268_FLASH_TEST           "bcm63268_flash_test"
#define DIAG_BCM63268_SHOW_SPI_FLASH_REG   "bcm63268_show_spi_flash_reg"
#define DIAG_BCM63268_DRAM_TEST            "bcm63268_dram_test"
#define DIAG_BCM63268_LED_TEST             "bcm63268_led_test"
#define DIAG_BCM63268_INTR_ENABLE          "bcm63268_interrupt_enable"
#define DIAG_BCM63268_INTR_DISABLE         "bcm63268_interrupt_disable"
#define DIAG_BCM63268_RESET                "bcm63268_reset"
#define DIAG_BCM63268_SHOW_PROFILE         "bcm63268_show_profile"
#define DIAG_INIT_BCM63268                 "init_bcm63268"
#define DIAG_UNINIT_BCM63268               "uninit_bcm63268"
#define DIAG_CONFIG_BCM63268               "config_bcm63268"
#define DIAG_BCM63268_GET_VERSION          "bcm63268_get_version"
#define DIAG_BCM63268_CONN_START           "bcm63268_conn_start"
#define DIAG_BCM63268_CONN_STOP            "bcm63268_conn_stop"
#define DIAG_BCM63268_GET_CONN_INFO        "bcm63268_get_conn_info"
#define DIAG_BCM63268_GET_SKU_TYPE         "bcm63268_get_sku_type"
#define DIAG_BCM63268_GET_BONDING_STATE    "bcm63268_get_bonding_state"
#define DIAG_BCM63268_SET_LINE_MODE        "bcm63268_set_line_mode"
#define DIAG_BCM63268_GET_LINE_MODE        "bcm63268_get_line_mode"
#define DIAG_BCM63268_SET_TEST_MODE        "bcm63268_set_test_mode"
#define DIAG_BCM63268_SEND_ALL_TONE        "bcm63268_send_all_tone"
#define DIAG_BCM63268_SET_IDLE_LISTEN      "bcm63268_set_idle_listen"
#define DIAG_BCM63268_GET_SW_INFO          "bcm63268_get_sw_info"
#define DIAG_BCM63268_SHOWTIME_CONT        "bcm63268_showtime_cont"
#define DIAG_BCM63268_SET_TONES            "bcm63268_set_tones"
#define DIAG_BCM63268_PRINT_IDLE_LISTEN    "bcm63268_print_idle_listen"
#define DIAG_BCM63268_GET_CONFIG_INFO      "bcm63268_get_config_info"
#define DIAG_BCM63268_GET_DRAM_INFO        "bcm63268_get_dram_info"
#define DIAG_BCM63268_GET_ADSLMIB_INFO     "bcm63268_get_adslmib_info"
#define DIAG_BCM63268_GET_XTM_BONDING_INFO "bcm63268_get_xtm_bonding_info"
#define DIAG_BCM63268_GET_XDSL_INFO        "bcm63268_get_xdsl_info"
#define DIAG_BCM63268_RESET_STATCOUNTERS   "bcm63268_reset_statcounters"
#define DIAG_PING_BCM63268                 "ping_bcm63268"
#define DIAG_BCM63268_RELAY_PIN_HIGH       "bcm63268_relay_pin_high"
#define DIAG_BCM63268_RELAY_PIN_LOW        "bcm63268_relay_pin_low"
#define DIAG_BCM63268_SPI_PROTECT          "bcm63268_spi_protect"
#define DIAG_BCM63268_SPI_UNPROTECT        "bcm63268_spi_unprotect"
#define DIAG_COMMAND_MRVL1512_REG_TEST     "mrvl1512_reg_test"

extern void xdsl_get_wic_ip_addr(char *);
extern int diag_xdsl_tests(boolean);
extern void viper_dsl_env_setup(void);
extern int wrap_pri_intf_rdy_chk(void);

#endif /* DIAG_XDSL_TEST_H */

/*-------------------------------------------------
 * $Log: diag_xdsl_test.h,v $
 * Revision 1.4  2018/09/21 02:48:54  harrchan
 * Merge viper DSL to the main trunk (CSCvm57542)
 *
 * Revision 1.3  2018/08/31 03:59:30  chieyang
 * Add SPI flash utility, show memory size and xdsl test modification. Merge from viper-branch2
 *
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.5  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.4  2018/05/21 08:42:36  olin2
 * Support DSL LED on/off utility
 *
 * Revision 1.1.2.3  2018/05/09 07:11:26  olin2
 * 1. Move GE and DSL init to the beginning. 2. Add has GE1. 3. Show cookie info
 *
 * Revision 1.1.2.2  2018/04/16 08:41:44  olin2
 * Support DSL test
 *
 * Revision 1.1.2.1  2018/02/27 08:06:48  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
