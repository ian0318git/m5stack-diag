/* $Id: platform_wifi.h,v 1.3 2018/02/09 09:56:55 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_wifi.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_wifi.h
 * Description: Header file of TSN WiFi related library functions.
 *
 * Copyright (c) 2017 - 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_WIFI_H__
#define __PLATFORM_WIFI_H__

/* Common */
#define TSN_WIFI_IPADDR          "192.168.1.1"
#define WIFI_RESET_INTERVAL      500   /* ms */
#define TSN_NC_BUF_SIZE          2048  /* Bytes */
#define TSN_NC_SETUP_TIME        1000  /* 1sec */
#define TSN_NC_OP_TIME           20    /* sec */
#define TSN_NC_EXEC_TIME         120    /* sec */
#define TSN_WIFI_NC_ENTRY_EN     "tsn_diag_nc_entry &"
#define TSN_WIFI_NC_RDY_PORT     (2294)
#define TSN_WIFI_NC_RDY_FILE     "/tmp/tsn_wifi_nc_rdy.txt"
#define TSN_WIFI_NC_TIMEOUT      20000  /* 20sec */
#define TSN_WIFI_NC_PULL_INTVL   1000    /* 1sec */

/* WiFi LED control state */
enum wifi_led_control_state {
    WIFI_LED_OFF = 0,
    WIFI_LED_GREEN,
    WIFI_LED_RED,
    WIFI_LED_AMBER,
};

/* TSN WiFi Diag NC command definition */
#define TURN_WIFI_LED_OFF_NC     "turn_wifi_led_off"
#define TURN_WIFI_LED_RED_NC     "turn_wifi_led_red"
#define TURN_WIFI_LED_GREEN_NC   "turn_wifi_led_green"
#define TURN_WIFI_LED_AMBER_NC   "turn_wifi_led_amber"
#define WIFI_DIAG_NOR_TEST_NC    "do_wifi_nor_test"
#define WIFI_DIAG_MEM_TEST_NC    "do_wifi_mem_test"
#define GET_WIFI_PID_NC          "get_wifi_pid"

/* Extern */
extern int  tsn_release_wifi_from_reset(void);
extern void tsn_setup_wlan_uart(void);
extern int  tsn_wifi_console_switch(void);
extern int  tsn_wifi_led_control(int);
extern int  tsn_reset_wifi(void);
extern int  tsn_wifi_nc_dispatch_comm(char *);
extern int  tsn_wifi_nc_get_test_result(int *, char *);

#endif   /* __PLATFORM_WIFI_H__ */


/*-------------------------------------------------
$Log: platform_wifi.h,v $
Revision 1.3  2018/02/09 09:56:55  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.22.1  2018/01/20 06:27:24  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.1  2017/12/13 11:28:09  palin2
Added to confirm Host and WiFi module PID pair for Star.

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

