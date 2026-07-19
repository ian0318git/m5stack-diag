/* 
 * $Id: dev_NR_5G_swi_at.h,v 1.3 2021/06/30 20:04:55 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_swi/dev_NR_5G_swi_at.h,v $
 *------------------------------------------------------------------
 * Filename:    dev_NR_5G_swi_at.h
 *
 * Description: Header File of SWI AT driver
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_SWI_5G_AT_H__
#define __DEV_SWI_5G_AT_H__

#define AT_CMD_RESP_TOUT_IN_SEC                 (30)

#define GPS_TEST_FREQ_MAX                       (105000)
#define GPS_TEST_FREQ_MIN                       (95000)

#define GPS_CTON_MAX                            (63)
#define GPS_CTON_MIN                            (53)

#define VTIME_TIMEOUT                           (30)

#define AT_CMD_BUFFER_SIZE                      (2048) //(1024)

#define MAIN_AUX_RSSI_AMP_DBM                   (-80)
#define MAIN_AUX_RSSI_DELTA                     (6)

#define MAX_SELFTEST_RETRY                      (1000)
#define AT_SELFTEST_TOUT_IN_SEC                 (1)
#define AT_SELFTEST_DELAY                       (500)

#define MAX_QTM_ANTS                            (4)
#define INST_PER_ANT                            (4)
#define RFDEVSTAT_STR_SIZE                      (32)

#define AT_CMD_STR_SIZE                         (100)

#define MFG_NAME                                "Sierra Wireless, Incorporated"
#define DEV_TYPE                                "EM9190"
#define SGL_LVL                                 "+CSQ: 99,99"
#define GPS_INFO                                "NO SAT INFO"

/*SUB6 RX cmd*/
#define MODEM_TECH_FAMILY                       (18)
#define MODEM_CARRIER                           (0)
#define MODEM_TECH                              (6)
#define EXPECTED_RSSI_AMP_DBM                   (MAIN_AUX_RSSI_AMP_DBM*10)

/*MMWAVE RX cmd*/
#define MODEM_MIMO_MODE                         (0)
#define MODEM_PATH                              (4)
#define MODEM_ENABLE_TX                         (1)
#define MODEM_DISABLE_TX                        (0)
#define MODEM_MMWAVE_WAVEFORM                   (10)
#define MODEM_MMWAVE_MOD                        (0)
#define MODEM_MMWAVE_NW_SGL_VAL                 (1)
#define MODEM_MMWAVE_START_RB                   (0)
#define MODEM_MMWAVE_NUM_RB                     (66)
#define MODEM_ALL_ANT                           (1)

#define MODEM_ANT_MAIN_PORT                     (0)
#define MODEM_ANT_AUX_PORT                      (3)
#define MODEM_ANT_M1_PORT                       (1)
#define MODEM_ANT_M2_PORT                       (2)

#define MODEM_ANT_MIMO_CFG                      (1)


#define MODEM_ATCMD_MAIN_PORT                   ",0"
#define MODEM_ATCMD_AUX_PORT                    ",3"
#define MODEM_ATCMD_M1_PORT                     ",1"
#define MODEM_ATCMD_M2_PORT                     ",2"

typedef enum nr_5g_at_test_ {
    RSSI_FR1_LEGACY_RX_TEST,
    RSSI_FR1_RX_TEST,
    RSSI_FR1_N1_MAIN_TEST,
    RSSI_FR1_N1_AUX_TEST,
    RSSI_FR1_N1_MIMO1_TEST,
    RSSI_FR1_N1_MIMO2_TEST,
    RSSI_FR1_N79_MAIN_TEST,
    RSSI_FR1_N79_AUX_TEST,
    RSSI_FR1_N79_MIMO1_TEST,
    RSSI_FR1_N79_MIMO2_TEST,
    RSSI_DROP_RADIO_CFG,
    NR_5G_MODEM_EXIT_TM,
    RSSI_FR1_N79_LEGACY_TX_TEST,
    RSSI_FR1_N79_TX_TEST,
    RSSI_FR1_N79_STOP_LEGACY_TX_TEST,
    RSSI_FR1_N79_STOP_TX_TEST,
    RSSI_GPS_L1_TEST,
    RSSI_GPS_L5_TEST,
    MODEM_RESET_TEST,
    MODEM_ATI_TEST,
    NR_5G_SIM0_DETECT_TEST,
    NR_5G_SIM1_DETECT_TEST,
    SIM_DETECT_PIN_PRESENT,
    SIM_DETECT_PIN_NO_PRESENT,
    SIM_DETECT_PIN_STATUS,
    WWAN_LED_TURN_ON,
    WWAN_LED_TURN_OFF,
    MODEM_TEMP_DETECT_TEST,
    SHOW_MODEM_INFO,
    SHOW_MODEM_HWID,
    SHOW_MODEM_SKU,
    MODEM_HOST_IF_EXERCISE,
    MODEM_GET_CUSTOM_GPIOSAR_STATUS,
    MODEM_SET_DIAG_CUSTOM_CONFIG,
    MODEM_SET_DEFAULT_CUSTOM_CONFIG,
    MODEM_IS_SARSTATE_0,
    MODEM_IS_SARSTATE_1,
    RSSI_NR_5G_FSN_TEST,
    MMWAVE_ANTENNA_PON_ENABLE,
    MMWAVE_ANTENNA_PON_DISABLE,
    MMWAVE_ANTENNA0_PON_ENABLE,
    MMWAVE_ANTENNA0_PON_DISABLE,
    MMWAVE_ANTENNA1_PON_ENABLE,
    MMWAVE_ANTENNA1_PON_DISABLE,
    MMWAVE_ANTENNA2_PON_ENABLE,
    MMWAVE_ANTENNA2_PON_DISABLE,
    MMWAVE_ANTENNA3_PON_ENABLE,
    MMWAVE_ANTENNA3_PON_DISABLE,
    MMWAVE_FR2_RSSI_RX_TEST,
    MMWAVE_FR2_TRANSMIT_TEST,
    MMWAVE_FR2_TRANSMIT_STOP,
    MMWAVE_ANT_PRESENT_STATUS,
    RSSI_FR1_OTA_ALL_ANT_RX_TEST,
    RSSI_FR1_OTA_IND_ANT_RX_TEST,
} nr_5g_at_test;

/* Structure for sending AT commands to modem */
typedef struct at_cmd_str_ {
    char *str;                         /* AT command */
    int  parse_the_result;             /* Need to parse the result? */
} at_cmd_str;

extern int dev_swi_5g_at_run_cmd(dev_5g_swi_object_t *, int);

int ant_mmwave_rx_test_radio_config(nr_mmwave_band_struct *, int, int);
int ant_mmwave_tx_test_radio_config(nr_mmwave_band_struct *, int, int);

#endif /* __DEV_SWI_5G_AT_H__ */

/*********************************************************************
 * $Log: dev_NR_5G_swi_at.h,v $
 * Revision 1.3  2021/06/30 20:04:55  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.4  2020/12/31 07:21:51  tshanmug
 * chrysler mmwave antenna detetection test added
 *
 * Revision 1.1.4.3  2020/12/22 22:49:28  tshanmug
 * Empire prrq review comment fix
 *
 * $Endlog$
 */
