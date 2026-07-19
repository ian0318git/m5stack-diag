/* $Id: dev_lte_swi_at.h,v 1.5 2020/02/19 03:11:30 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_swi/dev_lte_swi_at.h,v $
 * Filename:    dev_lte_swi_at.h
 *
 * Description: Header File of LTE SWI AT driver
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_LTE_SWI_AT_H__
#define __DEV_LTE_SWI_AT_H__

#define AT_CMD_RESP_TOUT_IN_SEC                 (30)

#define GPS_TEST_FREQ_MAX                       (105000)
#define GPS_TEST_FREQ_MIN                       (95000)

#define GPS_CTON_MAX                            (63)
#define GPS_CTON_MIN                            (53)

#define VTIME_TIMEOUT                           (30)

#define AT_CMD_BUFFER_SIZE                      (1024)

#define MAIN_DIV_RSSI_AMP_DBM                   (-70)
#define MAIN_DIV_RSSI_DELTA                     (6)

#define MAX_SELFTEST_RETRY                      (1000)
#define AT_SELFTEST_TOUT_IN_SEC                 (1)
#define AT_SELFTEST_DELAY                       (500)


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
    GPS_PIN_HIGH_WP76XX,
    GPS_PIN_LOW_WP76XX,
    LTE_PWR_DOWN,
    SIM_DETECT_PIN_PRESENT,
    SIM_DETECT_PIN_NO_PRESENT,
    SIM_DETECT_PIN_STATUS,
    WWAN_LED_TURN_ON,
    WWAN_LED_TURN_OFF,
    LTE_CHK_IMG_CARRIER_MATCH
} lte_at_test;

/* Structure for sending AT commands to modem */
typedef struct at_cmd_str_ {
    char *str;                         /* AT command */
    int  parse_the_result;             /* Need to parse the result? */
} at_cmd_str;

extern int dev_lte_swi_at_run_cmd(dev_lte_swi_object_t *, int);

#endif /* __DEV_LTE_SWI_AT_H__ */

/*------------------------------------------------------------------
$Log: dev_lte_swi_at.h,v $
Revision 1.5  2020/02/19 03:11:30  harrchan
Add LTE patch for matching modem carrier (CSCvt07550)

Revision 1.4  2019/06/14 09:59:00  steja
Supported Cooper usb dongle LTE

Revision 1.3  2018/11/09 07:33:22  yungchen
Merge viper branch4 to the main trunk (CSCvn11857)

Revision 1.2  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/07/09 08:28:30  olin2
CSCvk17781: Support util to verify SIM Detect pin

Revision 1.1.2.3  2018/05/29 03:03:57  harrchan
Support powerdown AT command and add selftest

Revision 1.1.2.2  2018/02/27 09:10:33  harrchan
Initial viper application code base


$Endlog$
*/
