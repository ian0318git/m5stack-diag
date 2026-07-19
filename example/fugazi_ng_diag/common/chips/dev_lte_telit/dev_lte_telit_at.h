/* $Id: dev_lte_telit_at.h,v 1.5 2019/08/14 02:28:06 shjung Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_telit/dev_lte_telit_at.h,v $
 *------------------------------------------------------------------
 *
 * Filename:    dev_lte_telit_at.h
 *
 * Description: Header File of LTE Telit AT driver
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_LTE_TELIT_AT_H__
#define __DEV_LTE_TELIT_AT_H__


#define VTIME_TIMEOUT                           (30)

#define AT_CMD_BUFFER_SIZE                      (1024)
#define MAX_SELFTEST_RETRY                      (100)
#define AT_SELFTEST_TOUT_IN_SEC                 (1)
#define AT_SELFTEST_DELAY                       (500)

#define AT_CMD_RESP_TOUT_IN_SEC                 (30)
#define AT_CMD_SEND_DELAY                       (20)
#define DELAY_1_SEC                             (1)

#define GPS_AMP_DBM                             (60)
#define GPS_AMP_TORLENCE                        (5)
#define GPS_FRQ_HZ                              (100000)
#define GPS_FRQ_TORLENCE                        (3150)

#define MAIN_DIV_RSSI_AMP_DBM                   (60)
#define MAIN_DIV_RSSI_TORLENCE                  (5)
#define SYS_DEVICE_PATH                         "/dev"
#define LTE_SET_PSAV_CMD_STR                    "AT#PSMWDISACFG="

typedef enum lte_at_test_opt_ {
    LTE_MODEM_DETECTION,
    LTE_PWR_DOWN,
    LTE_DISABLE_AUDIO,
    LTE_AUDIO_IS_DISABLE,
    LTE_DISABLE_FASTSHDN,
    LTE_ENABLE_FASTSHDN,
    LTE_FASTSHDN_IS_DISABLE,
    LTE_DISABLE_DYINGGASP,
    LTE_ENABLE_DYINGGASP,
    LTE_DG_IS_DISABLE,
    LTE_REBOOT,
    LTE_DUMP_TEMP,
    LTE_DUMP_MODEM_INFO,
    LTE_SIM1_DETECT_TEST,
    LTE_SIM2_DETECT_TEST,
    LTE_DUMP_SIMIN1_STAT,
    LTE_DUMP_SIMIN2_STAT,
    LTE_SIMIN1_DETECT_TEST,
    LTE_SIMIN2_DETECT_TEST,
    LTE_SWITCH_USB2P0,
    LTE_SWITCH_USB3P0,
    LTE_RSSI_CONFIG_B2,
    LTE_RSSI_CONFIG_B30,
    LTE_READ_MAIN_RSSI_PWR,
    LTE_READ_DIV_RSSI_PWR,
    LTE_GPS_ANTENNA_TEST,
    LTE_EXIT_TEST_MODE,
    LTE_ENABLE_OP_MODE,
    LTE_ENABLE_TEST_MODE,
    LTE_IN_OP_MODE,
    LTE_IN_LOWPWR_MODE,
    LTE_FULL_FUNC,
    LTE_DISABLE_IMG_SWITCHING,
    LTE_SET_ATT_IMG,
    LTE_SET_VERIZON_IMG,
    LTE_SET_GENERIC_IMG,
    LTE_SET_SPRINT_IMG,
    LTE_IMG_IS_MATCHED,
    LTE_LPM_WWANLED_ON,
    LTE_LPM_WWANLED_OFF,
    LTE_LPM_WWANLED_DEFAULT,
    LTE_GPIO5_IS_HIGH,
    LTE_GPIO5_IS_LOW,
    LTE_SET_SHDN_IND,
    LTE_DISABLE_SHDN_IND,
    LTE_SOFTSHDN_IND_IS_SET,
    LTE_SET_PWRSAV,
    LTE_CHK_PWRSAV_MODE,
} lte_at_test;

/* Structure for sending AT commands to modem */
typedef struct at_cmd_str_ {
    char *str;                         /* AT command */
    int  parse_the_result;             /* Need to parse the result? */
} at_cmd_str;

typedef enum {
    SIM_NOT_PRESENT,
    SIM_PRESENT,
} simin_stat_t;

typedef enum {
    LOWPWR_MODE,
    OP_MODE,
    TXRX_DISABLE_MODE = 4,
    TM_MODE,
    RESET_MODE,
    OFFLINE_MODE,
} modem_func_level_t;

typedef enum {
    OPERATION_MODE,
    TEST_MODE,
} modem_testmode_status_t;

typedef enum {
    FASTSHDN_DISABLE,
    FASTSHDN_ENABLE,
} fastshdn_stat_t;

typedef enum {
    AUDIO_ENABLE,
    AUDIO_DISABLE,
} audio_diable_stat_t;

typedef enum {
    DYINGGASP_DISABLE,
    DYINGGASP_ENABLE,
} dyinggasp_stat_t;

typedef enum {
    SHDN_IND_DISABLE,
    SOFTSHDN_IND_ENABLE,
    FASTSHDN_IND_ENABLE,
    ALLSHDN_IND_ENABLE,
} shutdown_indicator_stat_t;

extern int dev_lte_telit_at_run_cmd(dev_lte_telit_object_t *, int);
extern void dev_lte_telit_store_expected_img(char *);
extern void dev_lte_set_modem_pwrsav_para(int);

#endif /* __DEV_LTE_TELIT_AT_H__ */

/*------------------------------------------------------------------
$Log: dev_lte_telit_at.h,v $
Revision 1.5  2019/08/14 02:28:06  shjung

1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational
1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational

Revision 1.4  2019/06/26 03:52:59  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.3  2019/06/14 05:46:09  shjung
Fixed CSCvq12342, disable modem dying gasp before toggling Modem_Power_ON pin to avoid modem perform fast shutdown and added enabling dying gasp utility

Revision 1.2  2019/05/14 09:34:12  shjung
Support Hyperloop

Revision 1.1.2.16  2019/05/09 07:50:19  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.15  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.14  2019/04/17 10:09:10  sherliu2
remove mdev related

Revision 1.1.2.13  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.12  2019/04/08 06:41:22  shjung
Check the configuration of modem shutdown indicator while powering down the modem

Revision 1.1.2.11  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.10  2019/03/19 09:38:05  shjung
Chaged RSSI test bands based on test carrier

Revision 1.1.2.9  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.8  2019/03/12 02:53:01  shjung

1. Removed OP mode enabling process when RF test failed
2. Added query modem testmode status function
3. Added enable OP mode function
4. Adjusted RF test criteria
5. Code clean up

Revision 1.1.2.7  2019/02/15 02:59:49  shjung
Added WWAN_LED control utility

Revision 1.1.2.6  2019/02/11 08:06:04  sherliu2
Add GPS Antenna Test

Revision 1.1.2.5  2019/01/18 11:46:24  shjung
CSCvo03379: Temporarily add a workaround for tty device hanging issue on CAT18 modules

Revision 1.1.2.4  2019/01/18 06:17:38  shjung

1. Added W_DISABLE pin test
2. Added modem USB mode switching utility
3. Added delay in modem reboot function based on spec
4. Removed USB mode resotre operation when debug port test failed
5. Code clean up

Revision 1.1.2.3  2019/01/15 10:21:29  shjung
Modified the mechanism to get modem USB device info

Revision 1.1.2.2  2018/12/14 00:11:11  shjung
Modified RF band configuration

Revision 1.1.2.1  2018/12/12 01:46:12  shjung
Initial check-in for Hyperloop: Added device driver for Telit LTE-LM9x0 modem



$Endlog$
*/
