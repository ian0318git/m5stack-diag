/*
 * $Id: dev_NR_5G_telit_at.h,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_telit/dev_NR_5G_telit_at.h,v $
 *
 *------------------------------------------------------------------
 *
 * Filename:    dev_NR_5G_telit_at.h
 *
 * Description: Header File of Telit AT driver
 *
 * Copyright (c) 2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_NR_5G_TELIT_AT_H__
#define __DEV_NR_5G_TELIT_AT_H__


#define VTIME_TIMEOUT                           (30)
#define VTIME_TIMEOUT_2                         (60)

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
#define NR_5G_SUB6_SET_PSAV_CMD_STR             "AT#PSMWDISACFG="

typedef enum NR_5G_at_test_opt_ {
    NR_5G_MODEM_DETECTION,
    NR_5G_PWR_DOWN,
    NR_5G_DISABLE_AUDIO,
    NR_5G_AUDIO_IS_DISABLE,
    NR_5G_DISABLE_FASTSHDN,
    NR_5G_ENABLE_FASTSHDN,
    NR_5G_FASTSHDN_IS_DISABLE,
    NR_5G_DISABLE_DYINGGASP,
    NR_5G_ENABLE_DYINGGASP,
    NR_5G_DG_IS_DISABLE,
    NR_5G_REBOOT,
    NR_5G_DUMP_TEMP,
    NR_5G_DUMP_MODEM_INFO,
    NR_5G_SIM1_DETECT_TEST,
    NR_5G_SIM2_DETECT_TEST,
    NR_5G_DUMP_SIMIN1_STAT,
    NR_5G_DUMP_SIMIN2_STAT,
    NR_5G_SIMIN1_DETECT_TEST,
    NR_5G_SIMIN2_DETECT_TEST,
    NR_5G_SWITCH_USB2P0,
    NR_5G_SWITCH_USB3P0,
    NR_5G_TX_CONFIG_N1,
    NR_5G_TX_CONFIG_N25,
    NR_5G_TX_CONFIG_N79,
    NR_5G_RSSI_TEST_CONFIG,
    NR_5G_READ_MAIN_RSSI_PWR,
    NR_5G_READ_DIV_RSSI_PWR,
    NR_5G_READ_MIMO1_RSSI_PWR,
    NR_5G_READ_MIMO2_RSSI_PWR,
    NR_5G_GPS_ANTENNA_TEST,
    NR_5G_EXIT_TEST_MODE,
    NR_5G_ENABLE_OP_MODE,
    NR_5G_ENABLE_TEST_MODE,
    NR_5G_IN_OP_MODE,
    NR_5G_IN_LOWPWR_MODE,
    NR_5G_FULL_FUNC,
    NR_5G_DISABLE_IMG_SWITCHING,
    NR_5G_SET_ATT_IMG,
    NR_5G_SET_VERIZON_IMG,
    NR_5G_SET_GENERIC_IMG,
    NR_5G_SET_SPRINT_IMG,
    NR_5G_IMG_IS_MATCHED,
    NR_5G_LPM_WWANLED_ON,
    NR_5G_LPM_WWANLED_OFF,
    NR_5G_LPM_WWANLED_DEFAULT,
    NR_5G_GPIO5_IS_HIGH,
    NR_5G_GPIO5_IS_LOW,
    NR_5G_SET_SHDN_IND,
    NR_5G_DISABLE_SHDN_IND,
    NR_5G_SOFTSHDN_IND_IS_SET,
    NR_5G_SET_PWRSAV,
    NR_5G_CHK_PWRSAV_MODE,
    NR_5G_MODEM_MFG_NAME,
    NR_5G_MODEM_TYPE,
    NR_5G_MODEM_SL_NUM,
    NR_5G_MODEM_HOST_FW,
    NR_5G_SIM_MODE_CHANGE_FOR_DIAG,
    NR_5G_SIM_HOTSWAP_STATUS,
    NR_5G_SET_GPSANTPORT_ACTIVE,
} NR_5G_at_test;

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

extern int dev_NR_5g_telit_at_run_cmd(dev_NR_5g_telit_object_t *, int);
extern void dev_NR_5g_telit_store_expected_img(char *);
extern void dev_NR_5g_set_modem_pwrsav_para(int);

#endif /* __DEV_NR_5G_TELIT_AT_H__ */

/*********************************************************************
 * $Log: dev_NR_5G_telit_at.h,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.5  2021/02/27 00:43:07  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.4  2021/02/12 01:08:19  tshanmug
 * Sears multi band test support
 *
 * Revision 1.1.2.3  2020/12/01 06:38:46  tshanmug
 * Sears antenna test modification to test all antenna in a single menu
 *
 * $Endlog$
 */

