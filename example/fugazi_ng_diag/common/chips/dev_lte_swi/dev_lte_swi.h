/* $Id: dev_lte_swi.h,v 1.6 2020/02/19 03:11:30 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_swi/dev_lte_swi.h,v $
 * Filename:    dev_lte_swi.h
 *
 * Description: Header File of LTE SWI driver
 *
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __DEV_LTE_SWI_H__
#define __DEV_LTE_SWI_H__

#include "types.h"
#include "common_utils.h"
#include "dev_object.h"

#define RSSI_B8_FREQ                            "944.5"
#define RSSI_B4_FREQ                            "2134.5"
#define RSSI_B1_FREQ                            "2142"
#define RSSI_AMP                                "-70"

typedef enum {
    DEV_LTE_SWI_DEV_STATE = 0,
    DEV_LTE_SWI_ATTACH,
    DEV_LTE_SWI_DETACH,
    DEV_LTE_SWI_INIT,
    DEV_LTE_SWI_SHOW,
    DEV_LTE_SWI_DESTROY,
    DEV_LTE_SWI_READ,
    DEV_LTE_SWI_WRITE,
} dev_lte_swi_report_code_t;

typedef enum {
    MAIN_RSSI,
    DIV_RSSI
} dev_lte_swi_RSSI_t;

typedef enum {
    SIM_0,
    SIM_1
} dev_lte_swi_sim_t;

typedef enum {
    SWI_EM74XX,         /* EM7430/EM7455 */
    SWI_WP7601_03,      /* WP7601/WP7603 */
    SWI_WP7607_08_09,   /* WP7607/WP7608/WP7609 */
    SWI_WP760X_B8,      /* WP760X support Band8 */
    SWI_WP7610          /* WP7610 */
} dev_lte_modem_model_t;

typedef enum {
    GPS_PIN_LOW,
    GPS_PIN_HIGH
} dev_lte_gpi_pin_level_t;

typedef enum {
    SIM_DET_PIN_PRE,
    SIM_DET_PIN_NO_PRE 
} dev_lte_sim_det_pin_t;

typedef enum {
    WWAN_LED_OFF,
    WWAN_LED_ON
} dev_lte_wwan_led_stat_t;

typedef struct dev_lte_swi_callin_fvt_t_ {
    int (*modem_detection_test)(dev_object_t *dev);  /* Modem Detection Test */
    int (*modem_reset_test)(dev_object_t *dev);      /* Modem Reset Test */
    int (*modem_rssi_test)(dev_object_t *dev, int);  /* Main/Div RSSI Test */
    int (*modem_gps_ant_test)(dev_object_t *dev);    /* GPS Antenna Test */
    int (*sim_detect_test)(dev_object_t *dev, int);  /* SIM 0/SIM 1 Test */
    int (*wp76xx_drive_gps_pin)(dev_object_t *dev, int); /* Drive GPS Pin (WP76XX) */
    int (*modem_power_down)(dev_object_t *dev);  /* Modem Power Down */
    int (*sim_detect_pin_present)(dev_object_t *dev, int); /* SIM Detect PIN present (WP76XX) */
    int (*display_sim_detect_stat)(dev_object_t *dev); /* SIM Detect PIN status (WP76XX) */
    int (*toggle_wwan_led)(dev_object_t *dev, int);  /* LED Test */
    int (*modem_carrier_is_match)(dev_object_t *dev);  /* Detect whether modem carrier is matching or not*/
} dev_lte_swi_callin_fvt_t;

/*
 * device callout function - service needed by the device and defined by
 *                 platform
 */
typedef struct dev_lte_swi_callout_fvt_t_ {
    /*
     * Vectors set by the upper level (eg., platform).
     */
    void (*get_ttyusb_dev_name)(char *); /* TTY USB, e.g. /dev/ttyUSB2 */
} dev_lte_swi_callout_fvt_t;

/*
 * Define the LTE SWI device object structure
 */
typedef struct dev_lte_swi_object_t {
    dev_object_t        base;
    int                 modem_type;
    char		model[32];
    dev_lte_swi_callin_fvt_t        *callin_fvt;
    dev_lte_swi_callout_fvt_t       *callout_fvt;
} dev_lte_swi_object_t;

extern void lte_swi_dev_create(dev_object_t *, dev_error_report_t);

#endif /* __DEV_LTE_SWI_H__ */

/*------------------------------------------------------------------
$Log: dev_lte_swi.h,v $
Revision 1.6  2020/02/19 03:11:30  harrchan
Add LTE patch for matching modem carrier (CSCvt07550)

Revision 1.5  2019/07/11 12:34:41  alicehua
Collapse Nutella codes into main trunk

Revision 1.4  2019/06/14 09:59:00  steja
Supported Cooper usb dongle LTE

Revision 1.3.16.1  2019/02/21 01:28:03  harrchan
Support LTE modem WP7610 RSSI test

Revision 1.3  2018/11/09 07:33:22  yungchen
Merge viper branch4 to the main trunk (CSCvn11857)

Revision 1.2  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.5  2018/07/09 08:28:30  olin2
CSCvk17781: Support util to verify SIM Detect pin

Revision 1.1.2.4  2018/05/29 03:03:57  harrchan
Support powerdown AT command and add selftest

Revision 1.1.2.3  2018/04/20 02:10:49  lucywang
Added to support LTE WP7607/WP7608/WP7609

Revision 1.1.2.2  2018/02/27 09:10:32  harrchan
Initial viper application code base


$Endlog$
*/
