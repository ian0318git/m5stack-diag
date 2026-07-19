/* $Id: dev_lte_telit.h,v 1.7 2020/08/19 09:48:53 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_telit/dev_lte_telit.h,v $
 *------------------------------------------------------------------
 *
 * Filename:    dev_lte_telit.h
 *
 * Description: Header File of LTE Telit driver
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_LTE_TELIT_H__
#define __DEV_LTE_TELIT_H__

#include "dev_object.h"

#define RSSI_B2_FREQ                "1960.5"
#define RSSI_B30_FREQ               "2355.5"
#define RSSI_AMP                    "-60"
#define GPS_FREQ                    "1575.52"
#define GPS_AMP                     "-110"
#define TELIT_LTE_RST_DELAY         (40)
#define TELIT_LTE_HARD_RST_DELAY    (30)

#define ATT                         "ATT"
#define VERIZON                     "Verizon"
#define GENERIC                     "Generic"
#define SPRINT                      "Sprint"

typedef enum {
    DEV_LTE_TELIT_DEV_STATE = 0,
    DEV_LTE_TELIT_ATTACH,
    DEV_LTE_TELIT_DETACH,
    DEV_LTE_TELIT_INIT,
    DEV_LTE_TELIT_SHOW,
    DEV_LTE_TELIT_DESTROY,
    DEV_LTE_TELIT_READ,
    DEV_LTE_TELIT_WRITE,
} dev_lte_telit_report_code_t;

typedef enum {
    TELIT_LM940,
    TELIT_LM960
} dev_lte_telit_modem_model_t;

typedef enum {
    SUPER_SPD_USB,
    HIGH_SPD_USB,
} dev_lte_telit_usb_mode_t;

typedef enum {
    SIM_0,
    SIM_1
} dev_lte_telit_sim_no_t;

typedef enum {
    LTE_ANTENNA_CON_0,
    LTE_ANTENNA_CON_1
} lte_antenna_connector_no_t;

typedef enum {
    MAIN_RSSI,
    DIV_RSSI
} dev_lte_telit_rssi_test_opt_t;

typedef enum {
    LED_OFF,
    LED_ON,
    LED_DEFAULT
} dev_lte_telit_led_opt_t;

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

typedef struct dev_lte_telit_callin_fvt_t_ {
    int (*modem_detection_test)(dev_object_t *);  /* Modem Detection Test */
    int (*sim_detect_test)(dev_object_t *, int);  /* SIM 0/SIM 1 Test */
    int (*modem_switch_usb_mode)(dev_object_t *, int);   /* Switch modem USB mode */
    int (*modem_rssi_test)(dev_object_t *, int, int);  /* Main/Div RSSI Test */
    int (*modem_gps_test)(dev_object_t *);    /* GPS Antenna Test */
    int (*modem_simin_pin_present)(dev_object_t *, int); /* Check SIMIN PIN status */
    int (*modem_dump_simin_pin_status)(dev_object_t *, int); /* Dump SIMIN PIN status */
    int (*modem_disable_img_switching)(dev_object_t *);  /* Disable modem SIM-based carrier image auto-switching feature */
    int (*modem_set_lte_img)(dev_object_t *, char *);  /* Set LTE modem preferred image */
    boolean (*modem_check_lte_img)(dev_object_t *, char *);  /* Check LTE modem image is as expected or not */
    int (*modem_reboot)(dev_object_t *);          /* Modem reboot */
    int (*modem_in_operation_mode)(dev_object_t *);          /* Query current modem testmode status */
    int (*modem_enable_op_mode)(dev_object_t *);          /* Enable OP mode */
    int (*modem_enable_op_mode_without_esc)(dev_object_t *);    /* Enable OP mode without ESC cmd*/
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
    int (*modem_enable_dying_gasp)(dev_object_t *);      /* Enable Modem dying gasp feature */
    int (*modem_disable_dying_gasp)(dev_object_t *);      /* Disable Modem dying gasp feature */
    boolean (*modem_dying_gasp_is_disable)(dev_object_t *);      /* Check if Modem dying gasp feature is disable */
    int (*modem_check_gpio5_stat)(dev_object_t *, int);      /* Check Modem GPIO_05 status */
    boolean (*modem_in_lpm)(dev_object_t *);      /* Check modem is in LPM */
    boolean (*modem_is_online)(dev_object_t *);      /* Check modem is online */
} dev_lte_telit_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                           platform
 */
typedef struct dev_lte_telit_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    void (*get_current_usb_port)(int *); /* modem current USB port */
    int (*get_ttyusb_dev_name)(char *); /* TTY USB, e.g. /dev/ttyUSB2 */
} dev_lte_telit_callout_fvt_t;

/*
 * Define the LTE Telit device object structure
 */
typedef struct dev_lte_telit_object_t {
    dev_object_t        base;
    int                 modem_type;
    char		model[32];
    dev_lte_telit_callin_fvt_t        *callin_fvt;
    dev_lte_telit_callout_fvt_t       *callout_fvt;
} dev_lte_telit_object_t;

extern void lte_telit_dev_create(dev_object_t *, dev_error_report_t);

#endif /* __DEV_LTE_TELIT_H__ */

/*------------------------------------------------------------------
$Log: dev_lte_telit.h,v $
Revision 1.7  2020/08/19 09:48:53  markzha
*** empty log message ***

Revision 1.6  2019/08/14 02:28:06  shjung

1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational
1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational

Revision 1.5  2019/07/01 10:05:25  sherliu2
Supported mdev for Hyperloop

Revision 1.4  2019/06/26 03:52:59  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.3  2019/06/14 05:46:08  shjung
Fixed CSCvq12342, disable modem dying gasp before toggling Modem_Power_ON pin to avoid modem perform fast shutdown and added enabling dying gasp utility

Revision 1.2  2019/05/14 09:34:12  shjung
Support Hyperloop

Revision 1.1.2.18  2019/05/09 07:50:19  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.17  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.16  2019/04/17 10:09:10  sherliu2
remove mdev related

Revision 1.1.2.15  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.14  2019/04/08 09:54:25  sherliu2
Modified tty device name to symbolic name generated by mdev

Revision 1.1.2.13  2019/04/08 06:41:22  shjung
Check the configuration of modem shutdown indicator while powering down the modem

Revision 1.1.2.12  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.11  2019/03/19 09:38:05  shjung
Chaged RSSI test bands based on test carrier

Revision 1.1.2.10  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.9  2019/03/12 02:53:01  shjung

1. Removed OP mode enabling process when RF test failed
2. Added query modem testmode status function
3. Added enable OP mode function
4. Adjusted RF test criteria
5. Code clean up

Revision 1.1.2.8  2019/02/23 06:50:28  shjung
Followed Telit's suggestion to increase modem reboot pause time

Revision 1.1.2.7  2019/02/23 06:36:27  shjung

1. Added message for RSSI test criteria
2. Followed Telit's suggestion to increase modem reboot pause time

Revision 1.1.2.6  2019/02/15 02:59:49  shjung
Added WWAN_LED control utility

Revision 1.1.2.5  2019/02/11 08:06:04  sherliu2
Add GPS Antenna Test

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
