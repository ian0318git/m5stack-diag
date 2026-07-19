/* $Id: plug_lte_at.h,v 1.13 2020/01/18 07:02:07 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_at.h,v $
 *------------------------------------------------------------------
 *
 * plug_lte_at.h - Header file for Pluggable AT Command Functions
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_LTE_AT__
#define __PLUG_LTE_AT__

#define AT_TRANS_DELAY                          (2)

#define GPS_TEST_FREQ_MAX                       (105000)
#define GPS_TEST_FREQ_MIN                       (95000)

#define GPS_CTON_MAX                            (63)
#define GPS_CTON_MIN                            (53)

#define AT_CMD_RESP_TOUT_IN_SEC                 (30)
#define VTIME_TIMEOUT                           (30)

#define SIM_INIT_DELAY                          (2000)
#define AT_SELFTEST_TOUT_IN_SEC                 (1)
#define MAX_SELFTEST_RETRY                      (1000)
#define AT_SELFTEST_DELAY                       (500)
#define MAX_BUF_FLUSH_TOUT                      (5)
#define AT_CMD_BUFFER_SIZE                      (1024)
#define PLUG_LTE_BUF_FLUSH_DELAY                (10)

#define MAIN_DIV_RSSI_AMP_DBM                   (-70)
#define MAIN_DIV_RSSI_DELTA                     (6)

typedef enum lte_at_test_ {
    RSSI_LTE_B8_MAIN_TEST,
    RSSI_LTE_B8_DIV_TEST,
    RSSI_LTE_B4_MAIN_TEST,
    RSSI_LTE_B4_DIV_TEST,
    RSSI_LTE_B1_MAIN_TEST,
    RSSI_LTE_B1_DIV_TEST,
    RSSI_LTE_GPS_TEST,
    RSSI_LTE_RESET_TEST,
    RSSI_LTE_ATI_TEST,
    LTE_SIM0_DETECT_TEST,
    LTE_SIM1_DETECT_TEST,
    LTE_WP_SIM_PROTECT,
    LTE_GPS_DR_SYNC_TEST,
    LTE_GPS_DR_SYNC_FORCE_HIGH,
    LTE_GPS_DR_SYNC_FORCE_LOW,
    LTE_GPS_ENABLE,
    LTE_GPS_FIXES_STATUS,
    PLUG_LTE_BOOT_MODE,
    LTE_GET_MODEM_STATUS,
    DUMP_LTE_MODEM_TEMP,
    PLUG_LTE_PWR_DOWN,
    PLUG_LTE_CHK_IMG_CARRIER_MATCH,
    PLUG_LTE_SET_IMG_GENC,
    PLUG_LTE_SET_IMG_ATT,
    PLUG_LTE_SET_IMG_VERZ,
    PLUG_LTE_SET_IMG_DOCOMO,
    PLUG_LTE_EN_AUTO_SWITCH_IMG,
    EM74XX_SIMDETECT_H,
    EM74XX_SIMDETECT_L,
    EM74XX_SIMDETECT2_H,
    EM74XX_SIMDETECT2_L,
    WP76XX_UIM1DET_H,
    WP76XX_UIM1DET_L,
    EM74XX_SIMDETECT_STAT,
    EM74XX_SIMDETECT2_STAT,
    WP76XX_UIM1DET_STAT
} lte_at_test;

/* Structure for sending AT commands to modem */
typedef struct at_cmd_str_ {
    char *str;                         /* AT command */
    int  parse_the_result;             /* Need to parse the result? */
} at_cmd_str;

extern int plug_lte_at_run_cmd(int);

#endif

/*-------------------------------------------------
$Log: plug_lte_at.h,v $
Revision 1.13  2020/01/18 07:02:07  sherliu2
Modify WP7605 test carrier firmware from Generic to DOCOMO.

Revision 1.12  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.11  2019/06/14 05:48:10  shjung
Supported WP7605 modules

Revision 1.10  2018/11/23 09:15:07  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.9.14.2  2018/11/21 01:02:50  shjung
Added GPIO expander test register table and modified RF test macro name based on test RF band

Revision 1.9.14.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.9  2018/07/12 09:33:41  shjung
Fixed CSCvk20378:Covered pluggable LTE modem SIM_DETECT pin

Revision 1.8  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.7  2018/05/21 08:11:29  shjung
Merged code from star-branch-c110x

Revision 1.6  2018/04/13 09:34:59  shjung

1. Fix CSCvh79986 and CSCvh79979: Added modem tty device file descriptor
   slef test to ensure communication between host and modem is good
2. Modified code based on Pluggable LTE WP7601/03 ER code review
3. Put all cterr functions to the outer file
4. Modified modem USB device enumeration timeout and GPS pin vaule polling
   timeout

Revision 1.5  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.5  2018/05/17 02:53:41  shjung

1. Added delay for SIM initialization while switching UIM interface
2. Clear SAFE_PWR_REMOVAL signal after powered down WP module
3. Added 15.5 seconds boot-up delay for WP module based on spec.

Revision 1.3.2.3  2018/04/11 09:21:05  shjung

1. Remove modem reset test and add modem soft reset utility
2. Code modified based on pluggable LTE EM7455 ER code review

Revision 1.3.2.2  2018/03/16 07:47:28  shjung
1. Correct the default value of WP SAFE_POWER_REMOVAL and implement WP modem power-off function 2. Check modem status after hard reset

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:45  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/02/01 23:40:59  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.2  2018/01/20 06:56:33  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:08  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.6  2017/12/13 08:33:24  shjung
Added diagnostic test mode for pluggable LTE-WP76xx GPS pin test

Revision 1.1.4.5  2017/12/06 13:45:49  shjung
Modified SIM detection on LTE-WP modem to avoid power glitch

Revision 1.1.4.4  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.3  2017/10/30 14:15:13  shjung
Added GPS pin test for LTE-WP module

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.2  2017/07/26 01:50:52  tirawan
Fix SIM select mux for WP and extend AT Command timeout to 30secs

Revision 1.1.2.1  2017/07/24 22:51:25  tirawan
Add Pluggable AT command functions


*/

