/* 
 * $Id: dev_NR_5G_telit.h,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_telit/dev_NR_5G_telit.h,v $
 *
 *------------------------------------------------------------------
 *
 * Filename:    dev_NR_5G_telit.h
 *
 * Description: Header File of Telit driver
 *
 * Copyright (c) 2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_NR_5G_TELIT_H__
#define __DEV_NR_5G_TELIT_H__

#include "dev_object.h"
#define RSSI_AMP                    "-60"
#define GPS_FREQ                    "1575.52"
#define GPS_AMP                     "-110"
#define TELIT_NR_5G_RST_DELAY         (60)
#define TELIT_NR_5G_HARD_RST_DELAY    (60)

typedef enum {
    DEV_NR_5G_TELIT_DEV_STATE = 0,
    DEV_NR_5G_TELIT_ATTACH,
    DEV_NR_5G_TELIT_DETACH,
    DEV_NR_5G_TELIT_INIT,
    DEV_NR_5G_TELIT_SHOW,
    DEV_NR_5G_TELIT_DESTROY,
    DEV_NR_5G_TELIT_READ,
    DEV_NR_5G_TELIT_WRITE,
} dev_NR_5G_telit_report_code_t;

typedef enum {
    TELIT_FN980
} dev_NR_5G_telit_modem_model_t;

typedef enum {
    SUPER_SPD_USB,
    HIGH_SPD_USB,
} dev_NR_5G_telit_usb_mode_t;

typedef enum {
    SIM_0,
    SIM_1
} dev_NR_5G_telit_sim_no_t;

typedef enum {
    NR_5G_ANTENNA_0,
    NR_5G_ANTENNA_1,
    NR_5G_ANTENNA_2,
    NR_5G_ANTENNA_3,
    NR_5G_ANTENNA_4,

} NR_5G_antenna_connector_no_t;

typedef enum {
    MAIN_RSSI,
    DIV_RSSI
} dev_NR_5G_telit_rssi_test_opt_t;

typedef enum {
    LED_OFF,
    LED_ON,
    LED_DEFAULT
} dev_NR_5G_telit_led_opt_t;

typedef enum {
    GPIO_LOW,
    GPIO_HIGH,
} modem_gpio_status_t;

typedef enum {
    MODEM_LPM,
    MODEM_PWR_SAV_MODE,
    MODEM_IGNORE,
    MODEM_DYING_GASP = 10,
} modem_power_saving_mode_config_t;


typedef enum {
    GET_ONLY_PWR,    
    INIT_AND_GET_PWR,
    GET_PWR_AND_EXIT,
    INIT_GETPWR_EXIT,
} rssi_test_seq;

typedef struct dev_NR_5G_telit_callin_fvt_t_ {
    int (*modem_detection_test)(dev_object_t *);  /* Modem Detection Test */
    int (*sim_detect_test)(dev_object_t *, int);  /* SIM 0/SIM 1 Test */
    int (*modem_switch_usb_mode)(dev_object_t *, int);   /* Switch modem USB mode */
    int (*modem_rssi_test)(dev_object_t *, int, int, int);  /* Main/Div RSSI Test */
    int (*modem_ant_tx_test)(dev_object_t *, int, int);  /* Main/Div/mimo1 antenna tx Test */
    int (*modem_gps_test)(dev_object_t *);    /* GPS Antenna Test */
    int (*modem_simin_pin_present)(dev_object_t *, int); /* Check SIMIN PIN status */
    int (*modem_dump_simin_pin_status)(dev_object_t *, int); /* Dump SIMIN PIN status */
    int (*modem_disable_img_switching)(dev_object_t *);  /* Disable modem SIM-based carrier image auto-switching feature */
    int (*modem_set_lte_img)(dev_object_t *, char *);  /* Set modem preferred image */
    boolean (*modem_check_lte_img)(dev_object_t *, char *);  /* Check modem image is as expected or not */
    int (*modem_reboot)(dev_object_t *);          /* Modem reboot */
    int (*modem_in_operation_mode)(dev_object_t *);          /* Query current modem testmode status */
    int (*modem_enable_op_mode)(dev_object_t *);          /* Enable OP mode */
    int (*modem_enable_test_mode)(dev_object_t *);          /* Enable test mode */
    int (*modem_dump_temp)(dev_object_t *);       /* Dump modem temperature */
    int (*modem_dump_info)(dev_object_t *);       /* Dump modem information */
    int (*modem_pwrsaving_mode_ctrl)(dev_object_t *, int);       /* Modem Power Saving Mode control */
    int (*modem_lpm_wwan_led_ctrl)(dev_object_t *, int);       /* Modem LPM WWAN_LED blinking control */
    int (*modem_power_down)(dev_object_t *);      /* Modem Power Down */
    boolean (*modem_audio_is_disable)(dev_object_t *);      /* Modem check audio feature is disable or not */
    int (*modem_disable_audio)(dev_object_t *);      /* Disable Modem audio feature */
    boolean (*modem_fast_shutdown_is_disable)(dev_object_t *);      /* Check fast shutdown feature is disable or not */
    int (*modem_enable_fast_shutdown)(dev_object_t *);      /* Modem enable Fast Shutdown */
    boolean (*modem_softshdn_indic_is_enable)(dev_object_t *);      /* Check soft shutdown indicator feature is enable or not */
    int (*modem_disable_fast_shutdown)(dev_object_t *);      /* Disable Modem fast shutdown feature */
    int (*modem_set_shdn_indicator)(dev_object_t *);      /* Set Modem shutdown indicator */
    int (*modem_disable_shdn_indicator)(dev_object_t *);      /* Disable Modem shutdown indicator */
    boolean (*modem_dying_gasp_is_disable)(dev_object_t *);      /* Check if Modem dying gasp feature is disable */
    int (*modem_check_gpio5_stat)(dev_object_t *, int);      /* Check Modem GPIO_05 status */
    boolean (*modem_in_lpm)(dev_object_t *);      /* Check modem is in LPM */
    boolean (*modem_is_online)(dev_object_t *);      /* Check modem is online */
    int (*modem_bootup_msg)(dev_object_t *);      /* boot up message */
    int (*sim_hotswap_disable)(dev_object_t *);      /* Disable SIM hotswap */
    int (*sim_hotswap_status)(dev_object_t *);      /* Check SIM hotswap status */
} dev_NR_5G_telit_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                           platform
 */
typedef struct dev_NR_5G_telit_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    void (*get_current_usb_port)(int *); /* modem current USB port */
    int (*get_ttyusb_dev_name)(char *); /* TTY USB, e.g. /dev/ttyUSB2 */
} dev_NR_5G_telit_callout_fvt_t;

/*
 * Define the Telit device object structure
 */
typedef struct dev_NR_5g_telit_object_t {
    dev_object_t        base;
    int                 modem_type;
    char		model[32];
    dev_NR_5G_telit_callin_fvt_t        *callin_fvt;
    dev_NR_5G_telit_callout_fvt_t       *callout_fvt;
} dev_NR_5g_telit_object_t;

/*
 * Sub6 band
 */
typedef enum NR_SUB6_BAND_ {
    BAND_N1=1,
    BAND_N2=2,
    BAND_N3=3,
    BAND_N5=5,
    BAND_N7=7,
    BAND_N8=8,
    BAND_N12=12,
    BAND_N20=20,
    BAND_N25=25,
    BAND_N28=28,
    BAND_N38=38,
    BAND_N40=40,
    BAND_N41=41,
    BAND_N48=48,
    BAND_N66=66,
    BAND_N71=71,
    BAND_N77=77,
    BAND_N78=78,
    BAND_N79=79,
}NR_SUB6_BAND;


typedef enum {
    ANT0  = 1,
    ANT1  = 2,
    ANT2  = 4,
    ANT3  = 8,
} dev_NR_5G_telit_ant_t;

typedef struct nr_sub6_band_struct_ {
    NR_SUB6_BAND band_num;
    unsigned long int band_freq;
    int ant0;
    int ant1;
    int ant2;
    int ant3;
    int test_supported_ant;
} nr_sub6_band_struct;

extern void dev_NR_5g_telit_dev_create(dev_object_t *, dev_error_report_t);

#endif /* __DEV_NR_5G_TELIT_H__ */
/*********************************************************************
 * $Log: dev_NR_5G_telit.h,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.4  2021/02/27 00:43:07  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.3  2021/02/12 01:08:18  tshanmug
 * Sears multi band test support
 *
 * Revision 1.1.2.2  2020/12/01 06:38:46  tshanmug
 * Sears antenna test modification to test all antenna in a single menu
 *
 * $Endlog$
 */


