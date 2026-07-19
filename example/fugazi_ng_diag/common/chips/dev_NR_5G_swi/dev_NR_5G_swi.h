/* 
 * $Id: dev_NR_5G_swi.h,v 1.3 2021/06/30 20:04:55 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_swi/dev_NR_5G_swi.h,v $
 *------------------------------------------------------------------
 * Filename:    dev_NR_5G_swi.h
 *
 * Description: Header File of LTE SWI driver
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_SWI_5G_H__
#define __DEV_SWI_5G_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

#define RSSI_N1_FREQ                            "2140"
#define RSSI_N79_FREQ                           "4699.995"
#define RSSI_AMP                                "-80"
#define GPS_L1_FREQ                             "1575.52"
#define GPS_L5_FREQ                             "1177.45"
#define GPS_AMP                                 "-110"

#define MODEM_RESET_DELAY                       (30000)

/* 5G MMWAVE Frequencies */
#define RSSI_MMWV_N260_FREQ                     "37000"
#define RSSI_MMWV_AMP                           "-70"

#define BAND_FREQ_STR_SIZE                       (10)
#define MODEM_MODEL_STR_SIZE                     (32)
#define MODEM_SKU_STR_SIZE                       (32)
#define MODEM_FW_STR_SIZE                        (64)
#define MODEM_FSN_STR_SIZE                       (1024)
#define NUM_ANT                                  (4)
typedef enum {
    DEV_NR_5G_SWI_DEV_STATE = 0,
    DEV_NR_5G_SWI_ATTACH,
    DEV_NR_5G_SWI_DETACH,
    DEV_NR_5G_SWI_INIT,
    DEV_NR_5G_SWI_SHOW,
    DEV_NR_5G_SWI_DESTROY,
    DEV_NR_5G_SWI_READ,
    DEV_NR_5G_SWI_WRITE,
} dev_NR_5G_SWI_report_code_t;

typedef enum {
    MAIN_RSSI  = 1,
    AUX_RSSI   = 2,
    MIMO1_RSSI = 4,
    MIMO2_RSSI = 8,
} dev_SWI_NR_5G_swi_RSSI_t;

typedef enum {
    GNSS_L1_RSSI = 1,
    GNSS_L5_RSSI = 5,
} dev_NR_5G_SWI_GNSS_t;

typedef enum {
    SIM_0,
    SIM_1
} dev_NR_5G_SWI_sim_t;

typedef enum {
    SWI_EM_9190,
    SWI_EM_9191
} dev_NR_5G_SWI_modem_model_t;

typedef enum {
    SIM_DET_PIN_NO_PRE,
    SIM_DET_PIN_PRE
} dev_NR_5G_SWI_sim_det_pin_t;


typedef enum {
    WWAN_LED_OFF,
    WWAN_LED_ON
} dev_lte_wwan_led_stat_t;

typedef enum {
    MMWAVE_PON_DISABLE,
    MMWAVE_PON_ENABLE
} dev_5g_mmwave_pon_stat_t;

typedef enum {
    MMWAVE_ANTENNA_0,
    MMWAVE_ANTENNA_1,
    MMWAVE_ANTENNA_2,
    MMWAVE_ANTENNA_3
} dev_5g_mmwave_antenna_num_t;

typedef enum {
    MMWAVE_ANTENNA0_MASK = 0x01,
    MMWAVE_ANTENNA1_MASK = 0x02,
    MMWAVE_ANTENNA2_MASK = 0x04,
    MMWAVE_ANTENNA3_MASK = 0x08
} dev_5g_mmwave_antenna_mask_t;

#define MMWAVE_ANTENNA_ALL  MMWAVE_ANTENNA0_MASK | \
                            MMWAVE_ANTENNA0_MASK | \
                            MMWAVE_ANTENNA0_MASK | \
                            MMWAVE_ANTENNA0_MASK

typedef struct dev_lte_swi_callin_fvt_t_ {
    int (*show_modem_info)(dev_object_t *, int);  /* Modem information */
    int (*modem_detection_test)(dev_object_t *);  /* Modem Detection Test */
    int (*modem_temperature_test)(dev_object_t *);  /* Modem Detection Test */
    int (*modem_reset_test)(dev_object_t *);      /* Modem Reset Test */
    int (*modem_exit_tm)(dev_object_t *, int);  /* Exit test mode */
    int (*modem_sub6_rssi_test)(dev_object_t *, int, int, int);  /* Main/Div RSSI Test */
    int (*modem_sub6_ota_rssi_test)(dev_object_t *dev, int, int, int);  /* Main/Div RSSI Test */
    int (*modem_sub6_tx_test)(dev_object_t *, int, int);  /* Tx RSSI Test */
    int (*modem_stop_tx)(dev_object_t *, int, int);  /* Stop Tx Test */
    int (*modem_gps_ant_test)(dev_object_t *, int);    /* GPS Antenna Test */
    int (*sim_detect_test)(dev_object_t *, int);  /* SIM 0/SIM 1 Test */
    int (*gpio_sim_detect_test)(dev_object_t *, int);  /* SIM 0/SIM 1 detect using Modem GPIO */
    int (*sim_detect_pin_present)(dev_object_t *, int, int); /* SIM Detect PIN present (WP76XX) */
    int (*display_sim_detect_stat)(dev_object_t *, int); /* SIM Detect PIN status (WP76XX) */
    int (*modem_get_custom_param)(dev_object_t *, int);  /* Modem get the custom value */
    int (*modem_config_custom_param)(dev_object_t *, int);  /* Modem config the custom value */
    int (*modem_get_fsn_num)(dev_object_t *);  /* Modem FSN number to check hardware rev */
    int (*modem_get_sarstate)(dev_object_t *, int);  /* Get the SAR state */
    int (*toggle_wwan_led)(dev_object_t *, int);  /* LED Test */
    int (*toggle_mmwv_pon)(dev_object_t *, int, int);  /* Antenna PON Test */
    int (*modem_mmwv_rssi_test)(dev_object_t *, int, int, int);  /* Modem 5G MMWAVE RSSI Test */
    int (*modem_mmwv_transmit_test)(dev_object_t *, int, int, int);  /* Modem 5G MMWAVE Tx Test */
    int (*modem_mmwv_transmit_stop)(dev_object_t *, int);  /* Stop MMWAVE Tx Test */
    int (*modem_mmwv_ant_status)(dev_object_t *); /*Get mmwave Antenna present status */

} dev_NR_5G_swi_callin_fvt_t;


typedef struct dev_NR_5G_swi_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
//    void (*get_current_usb_port)(int *); /* modem current USB port */
    int (*get_tty_dev_name)(char *); /* TTY USB, e.g. /dev/ttyUSB2 */
} dev_NR_5G_swi_callout_fvt_t;

/*
 * Define the SWI device object structure
 */
typedef struct dev_5g_swi_object_t {
    dev_object_t        base;
    int                 modem_type;
	int                 ant_rx_value[NUM_ANT]; 
    char	        model[MODEM_MODEL_STR_SIZE];
    char                sku[MODEM_SKU_STR_SIZE];
    char                fsn[MODEM_FSN_STR_SIZE];
    char                modem_firmware[MODEM_FW_STR_SIZE];
    dev_NR_5G_swi_callin_fvt_t        *callin_fvt;
    dev_NR_5G_swi_callout_fvt_t       *callout_fvt;
} dev_5g_swi_object_t;

extern void swi_5g_dev_create(dev_object_t *, dev_error_report_t);


/*
 * Sub6
 */
typedef enum NR_SUB6_BAND_ {
    BAND_N1=1,
    BAND_N2=2,
    BAND_N3=3,
    BAND_N5=5,
    BAND_N28=28,
    BAND_N41=41,
    BAND_N66=66,
    BAND_N71=71,
    BAND_N77=77,
    BAND_N78=78,
    BAND_N79=79,
}NR_SUB6_BAND;

typedef struct nr_sub6_band_struct_ {
    NR_SUB6_BAND band_num;
    int band_width;
    int tx_channel;
    int rx_channel;
    char rx_center_freq[BAND_FREQ_STR_SIZE];
    char tx_center_freq[BAND_FREQ_STR_SIZE];
    int test_supported_ant;
} nr_sub6_band_struct;

/*
 * MMWAVE
 */
typedef enum NR_MMWAVE_BAND_ {
    BAND_N257 = 257,
    BAND_N258 = 258,
    BAND_N260 = 260,
    BAND_N261 = 261,
} NR_MMWAVE_BAND;

typedef struct nr_mmwave_band_struct_ {
    NR_MMWAVE_BAND band_num;
    int band_width_idx;
    int band_width;
    int tx_channel;
    int rx_channel;
    char if_fr_mhz[BAND_FREQ_STR_SIZE];
} nr_mmwave_band_struct;

typedef enum NR_MMWAVE_BEAMID_ {
    QTM0V_IFV4 = 0,
    QTM0H_IFH1 = 128,
    QTM1V_IFV3 = 1,
    QTM1H_IFH2 = 129,
    QTM2V_IFV2 = 2,
    QTM2H_IFH3 = 130,
    QTM3V_IFV1 = 3,
    QTM3H_IFH4 = 131,
} enum_nr_mmwave_beamid;


#endif /* __DEV_SWI_5G_H__ */

/*********************************************************************
 * $Log: dev_NR_5G_swi.h,v $
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
