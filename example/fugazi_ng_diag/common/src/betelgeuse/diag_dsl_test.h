/* $Id: diag_dsl_test.h,v 1.3 2019/05/21 07:44:19 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_dsl_test.h,v $
 *------------------------------------------------------------------
 * 
 * diag_dsl_test.h
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _DSL_TESTS_H_
#define _DSL_TESTS_H_

#define DSL_PING_TOUT             (10)    /* 10 secs */
#define DSL_BL_PROMPT_TOUT        (30)    /* 30 secs */
#define DSL_DIAG_PROMPT_TOUT      (120)   /* 120 secs */

/* environment variable */
#define DSL_IMAGE_NAME            "DSL_IMG_FILE"       
#define IOS_IMAGE_NAME            "IOSDSL_IMG_FILE"       

#define PLAT_IOS_HOST_FIRMWARE_FOLDER_STRING "firmware/"
#define PLAT_IOS_VADSL_FIRMWARE_STRING "c1100_vadsl_fw.img"
#define PLAT_IOS_GFAST_FIRMWARE_STRING "c1100_gfast_fw.img"
#define DSL_AM_DIAG_IMG           "tsn_dsl_am_diag.img"
#define DSL_BJ_DIAG_IMG           "tsn_dsl_bj_diag.img"
#define DSL_CFE_PRESS_KEY_TIMEOUT        (30000)  /* 30 second */
#define DSL_BOARD_ID_TIMEOUT             (5000)
#define DSL_CFE_TIMEOUT                  (100)
#define DSL_LINUX_TIMEOUT                (100)
#define DSL_CFE_COMMAND_STATUS_TIMEOUT   (1000)
#define DSL_CFE_RETRY_TIMES              (30)
#define DSL_CFE_CR_TIMES                 (100)
#define DSL_CFE_IP_NUM                   (3)
#define DSL_LINUX_WAIT_RETRY_TIMES       (500)
#define DSL_SELF_DISCONN_TIMEOUT         (12000)
#define DSL_SELF_DISCONN_RETRY_TIMES     (3)
#define DSL_CONN_TIMEOUT                 (18000)
#define UNCOMPRESS_CMD                  "tar -xvf "
#define PLAT_DSL_PRESS_KEY_STRING        "Press any key"
#define PLAT_DSL_BOARD_ID                "Board Id"
#define PLAT_DSL_BOARD_IP_STRING         "Board IP address"
#define PLAT_DSL_HOST_IP_STRING          "Host IP address"
#define PLAT_DSL_GATEWAY_IP_STRING       "Gateway IP address"
#define PLAT_DSL_IMG_RUN_STRING          "Run from flash/host/tftp (f/h/c)"
#define PLAT_DSL_BOARD_ID_NUM            "2"
#define PLAT_DSL_MAC_NUM                 "10"
#define PLAT_DSL_MAC_ADDR                "00:10:18:00:00:00"
#define PLAT_DSL_PSI_SIZ                 "24"
#define PLAT_DSL_RESET_STRING            "reset"
#define PLAT_DSL_CFE_STRING              "CFE>"
#define PLAT_DSL_CHANGE_CFE_PARMS_STRING "c"
#define PLAT_DSL_RUN_IMAGE_LOCATION      "c"
#define PLAT_DSL_RUN_IMAGE_LOCATION_H    "h"
#define GFAST_HOST_RUN_FILE             "vmlinux_rd_boot"
#define GFAST_HOST_RAMDISK_FILE         "ramdisk"
#define BCM963XX_FS_KERNEL              "bcm963xx_fs_kernel"
#define GFAST_RAMDISK_STORE_ADDR        "0x03000000"
#define CLEAR_PARAM                     " "
#define PLAT_DSL_PRINT_CFE_PARMS_STRING  "p"
#define PLAT_DSL_IMG_RUN_FROM_HOST_STR   "h"
#define PLAT_DSL_COMMAND_STATUS_STRING   "command status"
//leslie - tftpd server default will use directory /var/lib/tftpboot
//#define PLAT_HOST_FIRMWARE_FOLDER_STRING "/firmware/"
#define PLAT_HOST_FIRMWARE_FOLDER_STRING "/var/lib/tftpboot/"
#define PLAT_DSL_CR_STRING               "\r"
#define PLAT_DSL_LINUX_PROMPT            ">"
#define PLAT_DSL_IFCONFIG_BR0_STRING     "ifconfig br0 "
#define PLAT_DSL_SHELL_STRING            "sh"
#define PLAT_DSL_LINUX_PROMPT_RETRY      (10)
#define PLAT_DSL_MAX_LENGTH              (128)
#define WAIT_DSL_LINUX_PROMPT            (1000)
#define WAIT_DSL_CFE_PROMPT              (500)
#define WAIT_DSL_UART_TX                 (500)
#define WAIT_DSL_CFE_COMMAND_STATUS      (200)
#define WAIT_DSL_IP_SETUP                (1000)
#define WAIT_FOR_FLASH_WRITE             (2000)
#define WAIT_DSL_SELF_DISCONN            (1000)
#define WAIT_DSL_CONN_START              (2000)
#define WAIT_DSL_IDLE_LISTEN             (25)
#define WAIT_FOR_CPLD_WRITE              (500)
#define WAIT_FOR_CTRL_DSL_INTR           (10)
#define CHECK_FPGA_DSL_PENDING_TIMES     (20)
#define IOS_CFE_PARAM                    TRUE
#define DIAG_CFE_PARAM                   FALSE 
/* TRUE = CFE boot two images; FALSE CFE boot one image */
#define FLAG_CFE_BOOT_TWO_IMG            FALSE  

/* NC Command Dispatch */
#define DIAG_BCM63268_FLASH_TEST           "bcm63268_flash_test"
#define DIAG_BCM63268_SHOW_SPI_FLASH_REG   "bcm63268_show_spi_flash_reg"
#define DIAG_BCM63268_DRAM_TEST            "bcm63268_dram_test"
#define DIAG_BCM63268_LED_TEST             "bcm63268_led_test"
#define DIAG_BCM63138_LED_TEST             "bcm963138_led_test"
#define DIAG_BCM63268_LED_OFF              "bcm63268_led_off"
#define DIAG_BCM63138_LED_OFF              "bcm963138_led_off"
#define DIAG_BCM63268_LED_CD_ON            "bcm63268_led_cd_on"
#define DIAG_BCM63138_LED_CD_ON            "bcm963138_led_cd_on"
#define DIAG_BCM63268_LED_DATA_ON          "bcm63268_led_data_on"
#define DIAG_BCM63138_LED_DATA_ON          "bcm963138_led_data_on"
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
#define DIAG_SET_PROFILE_35B                "bcm63138_set_profile35b"
#define DIAG_BCM63268_SPI_PROTECT          "bcm63268_spi_protect"
#define DIAG_BCM63268_SPI_UNPROTECT        "bcm63268_spi_unprotect"
#define DIAG_BCM63268_CHK_SPI_FLASH_PROTECT     "bcm63268_chk_spi_flash_protect"

#define LED_OFF     0
#define LED_CD_ON   1
#define LED_DATA_ON 2

extern int diag_dsl_test(int);

#endif /* XDSL_TEST_H */

/*-------------------------------------------------
 * $Log: diag_dsl_test.h,v $
 * Revision 1.3  2019/05/21 07:44:19  wilbhuan
 * Add a new xDSL utility to check the SPI Flash protection.
 *
 * Revision 1.2  2019/01/10 06:36:26  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
