/*
 * $Id: dev_NR_5G_telit.c,v 1.2 2021/06/02 02:56:19 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_telit/dev_NR_5G_telit.c,v $
 *
 *------------------------------------------------------------------
 *
 * Filename:	dev_NR_5G_telit.c
 *
 * Description:	NR 5G Telit Driver. Supports following modules:
 *              FN980m 
 * Copyright (c) 2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "common.h"
#include "free.h"
#include "proto.h"
#include "error.h"
#include "dev_NR_5G_telit.h"
#include "dev_NR_5G_telit_at.h"
#include "plug_NR_5G_telit_lib.h"

void dev_NR_5g_telit_dev_create(dev_object_t *, dev_error_report_t);
static uint32 dev_NR_5g_telit_attach(dev_object_t *);
static uint32 dev_NR_5g_telit_detach(dev_object_t *);
static uint32 dev_NR_5g_telit_restart(dev_object_t *);
static void dev_NR_5g_telit_destroy(dev_object_t **);
static int dev_NR_5g_telit_modem_bootup_msg(dev_object_t *);
static int dev_NR_5g_telit_modem_detection(dev_object_t *);
static int dev_NR_5g_modem_power_down(dev_object_t *);
static int dev_NR_5g_telit_modem_set_shdn_indicator(dev_object_t *);
static int dev_NR_5g_telit_modem_disable_shdn_indicator(dev_object_t *);
static int dev_NR_5g_telit_sim_detect_test(dev_object_t *, int);
static int dev_NR_5g_telit_simin_pin_present(dev_object_t *, int);
static int dev_NR_5g_telit_dump_simin_pin_status(dev_object_t *, int);
static int dev_NR_5g_telit_switch_modem_usb_mode(dev_object_t *, int);
static int dev_NR_5g_telit_modem_rssi_test(dev_object_t *, int, int, int);
static int dev_NR_5g_telit_modem_gps_test(dev_object_t *);
static int dev_NR_5g_telit_modem_reboot(dev_object_t *);
static int dev_NR_5g_telit_modem_enable_operation_mode(dev_object_t *);
static int dev_NR_5g_telit_modem_enable_test_mode(dev_object_t *);
static int dev_NR_5g_telit_modem_dump_temp(dev_object_t *);
static int dev_NR_5g_telit_modem_dump_info(dev_object_t *);
static int dev_NR_5g_telit_modem_lpm_wwan_led_ctrl(dev_object_t *, int);
static int dev_NR_5g_telit_modem_pwrsav_mode_ctrl(dev_object_t *, int);
static boolean dev_NR_5g_telit_modem_is_in_operation_mode(dev_object_t *);
static boolean dev_NR_5g_telit_softshdn_indicator_is_set(dev_object_t *);
static boolean dev_NR_5g_telit_modem_is_in_lpm(dev_object_t *);
static boolean dev_NR_5g_telit_modem_is_online(dev_object_t *);
static boolean dev_NR_5g_telit_modem_sim_hotswap_disable(dev_object_t *);
static boolean dev_NR_5g_telit_modem_sim_hotswap_status(dev_object_t *);
static int dev_NR_5g_telit_modem_ant_tx_test (dev_object_t *, int, int );

extern int ant_test_band_config(nr_sub6_band_struct *, int );
extern nr_sub6_band_struct nr_sub6_band_tbl[];
extern int band_tbl_size;


/******************************************************************************
 * Name:        dev_NR_5g_telit_dev_create
 * Description: Create device object with various function point to "do nothing"
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              error_report_fn - error reporting function pointer. 
 * Returns:     none
 *****************************************************************************/
void dev_NR_5g_telit_dev_create(dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t *dev_fvt;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
                    NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in dev_NR_5g_telit_dev_create()", 0);
        printf("%s: NULL\n", __func__);
        return;
    }

    /* Init the device object structure to default */
    init_default_dev_object(dev, dev_fvt);

    obj_telit_modem->base.dev_object_fvt->dev_attach    = dev_NR_5g_telit_attach;
    obj_telit_modem->base.dev_object_fvt->dev_detach    = dev_NR_5g_telit_detach;
    obj_telit_modem->base.dev_object_fvt->dev_restart   = dev_NR_5g_telit_restart;
    obj_telit_modem->base.dev_object_fvt->dev_error_report = error_report_fn;
    obj_telit_modem->base.dev_object_fvt->dev_destroy   = dev_NR_5g_telit_destroy;
    obj_telit_modem->base.dev_object_fvt->dev_name      = "NR_5G_TELIT";

    obj_telit_modem->modem_type = TELIT_FN980;

    obj_telit_modem->callin_fvt = (dev_NR_5G_telit_callin_fvt_t *)
                                 malloc(sizeof(dev_NR_5G_telit_callin_fvt_t));
    obj_telit_modem->callout_fvt = (dev_NR_5G_telit_callout_fvt_t *)
                                  malloc(sizeof(dev_NR_5G_telit_callout_fvt_t));

    obj_telit_modem->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 * Name:        dev_NR_5g_telit_attach
 * Description: Attach the Telit device for use. This function will
 *              initialize and setup all necessary pointers and bring the chip
 *              to operation.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static uint32 dev_NR_5g_telit_attach (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (obj_telit_modem->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_NR_5g_telit_attach() callin malloc",
                         DEV_NR_5G_TELIT_ATTACH);
        return (FAILED);
    }
    if (obj_telit_modem->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_NR_5g_telit_attach() callout malloc",
                         DEV_NR_5G_TELIT_ATTACH);
        return (FAILED);
    }

    /* Init the call in functions */
    obj_telit_modem->callin_fvt->modem_bootup_msg =
                               dev_NR_5g_telit_modem_bootup_msg;
    obj_telit_modem->callin_fvt->modem_detection_test =
                               dev_NR_5g_telit_modem_detection;
    obj_telit_modem->callin_fvt->sim_detect_test =
                               dev_NR_5g_telit_sim_detect_test;
    obj_telit_modem->callin_fvt->modem_switch_usb_mode =
                               dev_NR_5g_telit_switch_modem_usb_mode;
    obj_telit_modem->callin_fvt->modem_rssi_test =
                               dev_NR_5g_telit_modem_rssi_test;
    obj_telit_modem->callin_fvt->modem_ant_tx_test =
                               dev_NR_5g_telit_modem_ant_tx_test;
    obj_telit_modem->callin_fvt->modem_gps_test =
                               dev_NR_5g_telit_modem_gps_test;
    obj_telit_modem->callin_fvt->modem_simin_pin_present =
                               dev_NR_5g_telit_simin_pin_present;
    obj_telit_modem->callin_fvt->modem_dump_simin_pin_status =
                               dev_NR_5g_telit_dump_simin_pin_status;
    obj_telit_modem->callin_fvt->modem_reboot =
                               dev_NR_5g_telit_modem_reboot;
    obj_telit_modem->callin_fvt->modem_in_operation_mode =
                               dev_NR_5g_telit_modem_is_in_operation_mode;
    obj_telit_modem->callin_fvt->modem_enable_op_mode =
                               dev_NR_5g_telit_modem_enable_operation_mode;
    obj_telit_modem->callin_fvt->modem_enable_test_mode =
                               dev_NR_5g_telit_modem_enable_test_mode;
    obj_telit_modem->callin_fvt->modem_dump_temp =
                               dev_NR_5g_telit_modem_dump_temp;
    obj_telit_modem->callin_fvt->modem_lpm_wwan_led_ctrl =
                               dev_NR_5g_telit_modem_lpm_wwan_led_ctrl;
    obj_telit_modem->callin_fvt->modem_in_lpm =
                               dev_NR_5g_telit_modem_is_in_lpm;
    obj_telit_modem->callin_fvt->modem_is_online =
                               dev_NR_5g_telit_modem_is_online;
    obj_telit_modem->callin_fvt->modem_power_down =
                               dev_NR_5g_modem_power_down;
    obj_telit_modem->callin_fvt->modem_disable_shdn_indicator =
                               dev_NR_5g_telit_modem_disable_shdn_indicator;
    obj_telit_modem->callin_fvt->modem_set_shdn_indicator =
                               dev_NR_5g_telit_modem_set_shdn_indicator;
    obj_telit_modem->callin_fvt->modem_softshdn_indic_is_enable =
                               dev_NR_5g_telit_softshdn_indicator_is_set;
    obj_telit_modem->callin_fvt->modem_pwrsaving_mode_ctrl =
                               dev_NR_5g_telit_modem_pwrsav_mode_ctrl;
    obj_telit_modem->callin_fvt->modem_dump_info =
                               dev_NR_5g_telit_modem_dump_info;
    obj_telit_modem->callin_fvt->sim_hotswap_disable =
                               dev_NR_5g_telit_modem_sim_hotswap_disable;
    obj_telit_modem->callin_fvt->sim_hotswap_status =
                               dev_NR_5g_telit_modem_sim_hotswap_status;

    obj_telit_modem->base.dev_state = DEV_STATE_ATTACH;
    return (PASSED);
}


/******************************************************************************
 * Name:        dev_NR_5g_telit_detach
 * Description: Detach the device specific functions from the caller.
 *              All of the device specific function are connected to the
 *              dev_do_nothing() function, except for the dev_attach() function.
 *              Also, the dev_state must be assigned the value of
 *              DEV_STATE_DETACH.
 *
 *        		Since, some platforms may want to detach the device, but not
 *        		release the memory resources (via a free () in the
 *        		dev_destroy()), this function can be executed to accomplish
 *        		this task. However, before a detached device can be used again,
 *        		it must be re-attached (via the dev_attach()).
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static uint32 dev_NR_5g_telit_detach (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_telit_modem->base.dev_object_fvt);

    obj_telit_modem->base.dev_state = DEV_STATE_DETACH;
    return (PASSED);
}


/******************************************************************************
 * Name:        dev_NR_5g_telit_restart
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *****************************************************************************/
static uint32 dev_NR_5g_telit_restart (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    obj_telit_modem->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
} 


/******************************************************************************
 * Name:        dev_NR_5g_telit_destroy
 * Description: Destroy the dev_object structure and free all the resources.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *****************************************************************************/
static void dev_NR_5g_telit_destroy (dev_object_t **dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    /* Free call out/in structure */
    if (obj_telit_modem->callout_fvt) {
        free(obj_telit_modem->callout_fvt);
    }
    if (obj_telit_modem->callin_fvt) {
        free(obj_telit_modem->callin_fvt);
    }

    /* Free dev_object_t */
    free(obj_telit_modem->base.dev_object_fvt);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_reboot
 * Description: Function to power down modem through AT command.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_reboot (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_REBOOT) != PASSED) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
    sleep(TELIT_NR_5G_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_dump_temp
 * Description: Function to dump modem temperature through AT command.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_dump_temp (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_DUMP_TEMP));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_dump_info
 * Description: Function to dump modem information through AT command.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_dump_info (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_DUMP_MODEM_INFO));
}

/****************************************************************************** 
 * Name:        dev_NR_5g_modem_power_down
 * Description: Function to power down modem through AT command.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_modem_power_down (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_PWR_DOWN));
}

/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_bootup_msg
 * Description: Function to print modem info on invoking the modem diag
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_bootup_msg (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_MODEM_MFG_NAME)) return FAILED;
    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_MODEM_TYPE))     return FAILED;
    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_MODEM_SL_NUM))   return FAILED;
    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_MODEM_HOST_FW))  return FAILED;
    return PASSED;
}
/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_detection
 * Description: Function to detect modem through AT command.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_detection (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_MODEM_DETECTION));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_sim_detect_test
 * Description: Function to detect SIM card through AT command.
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              which sim - SIM 0 or SIM 1
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_sim_detect_test (dev_object_t *dev, int which_sim)
{
    int at_test;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    switch (which_sim) {
        case SIM_0:
        case SIM_1:
            at_test = NR_5G_SIM1_DETECT_TEST;
            break;
        default:
            printf("%s: Invaid SIM slot number (SIM %d)\n", __func__,
                                                            which_sim); 
            return (FAILED);
    }

    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_dump_simin_pin_status
 * Description: Function to dump SIMIN pin status through AT command.
 *              SIMIN pins are used to indicate SIM card is inserted or not
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              which sim - SIM 0 or SIM 1
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_dump_simin_pin_status (dev_object_t *dev,
                                                int which_sim)
{
    int at_test;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    switch (which_sim) {
        case SIM_0:
        case SIM_1:
            at_test = NR_5G_DUMP_SIMIN1_STAT;
            break;
        default:
            printf("%s: Invalid SIM slot number (SIM %d)\n", __func__,
                                                             which_sim); 
            return (FAILED);
    }
    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_simin_pin_present
 * Description: Function to check SIMIN pin status to see whether SIM card is
 *              present or not through AT command.
 *              SIMIN pins are used to indicate SIM card is inserted or not
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              which sim - SIM 0 or SIM 1
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_simin_pin_present (dev_object_t *dev, int which_sim)
{
    int at_test;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    switch (which_sim) {
        case SIM_0:
        case SIM_1:
            at_test = NR_5G_SIMIN1_DETECT_TEST;
            break;
        default:
            printf("%s: Invaid SIM slot number (SIM %d)\n", __func__,
                                                            which_sim); 
            return (FAILED);
    }
    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_switch_modem_usb_mode
 * Description: Function to switch modem USB configuration to super-speed(3.0)
 *              mode or high-speed(2.0) mode.
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              usb_mode - which USB configuration(HIGH_SPD_USB/SUPER_SPD_USB)
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_switch_modem_usb_mode (dev_object_t *dev, int usb_mode)
{
    int at_test;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (usb_mode == HIGH_SPD_USB) {
        at_test = NR_5G_SWITCH_USB2P0;
    } else if (usb_mode == SUPER_SPD_USB) {
        at_test = NR_5G_SWITCH_USB3P0;
    } else {
        printf("Invalid USB mode\n");
        return (FAILED);
    }

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test) != PASSED) {
        return (FAILED);
    }
    sleep(DELAY_5_SEC); //As per Telit recommendation
    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_REBOOT) != PASSED) {
        return (FAILED);
    }
    
    printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
    sleep(TELIT_NR_5G_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_rssi_test
 * Description: Function to perform RSSI NR5G sub6  RX path verification.
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              antenna_type - MAIN_RSSI/DIV_RSSI/MIMO1/MIMO2
 *              antenna_no - Antenna number
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_rssi_test (dev_object_t *dev, int antenna_type, int band, int test_seq)
{
    int i, at_ant_sel_cmd;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;
 

    for (i = 0; i < band_tbl_size; i++){
        if (nr_sub6_band_tbl[i].band_num == band) {
            break;
         }
    }
    if ( i >= band_tbl_size){
        printf ("\nTest band :%d is not valid\n", band);
        return FAILED;
    }


    /* Read modem RX power level */
    switch (antenna_type) {
        case NR_5G_ANTENNA_0:
            at_ant_sel_cmd = nr_sub6_band_tbl[i].ant0;
            break;
        case NR_5G_ANTENNA_1:
            at_ant_sel_cmd = nr_sub6_band_tbl[i].ant1;
            break;
        case NR_5G_ANTENNA_2:
            at_ant_sel_cmd = nr_sub6_band_tbl[i].ant2;
            break;
        case NR_5G_ANTENNA_3:
            at_ant_sel_cmd = nr_sub6_band_tbl[i].ant3;
            break;
        default:
            printf("Invaid RSSI test option.\n");
            return (FAILED);
            break;
    }

    
    if (ant_test_band_config (&nr_sub6_band_tbl[i], band) != PASSED) {
        printf ("\nBand configuration failed ");
        return (FAILED);
    }

    if ((test_seq & INIT_AND_GET_PWR) == INIT_AND_GET_PWR) {
        if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, \
                          NR_5G_RSSI_TEST_CONFIG) != PASSED) {
            printf("Failed to set RSSI test configuration\n");
            return (FAILED);
        }
    }
    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_ant_sel_cmd) != PASSED) {
        return (FAILED);
    }

    if ((test_seq & GET_PWR_AND_EXIT) == GET_PWR_AND_EXIT) {
        /* Exit test mode and switch back to operation mode */
        if (dev_NR_5g_telit_modem_enable_operation_mode(dev) != PASSED) {
            printf("Failed to enable modem operation mode.\n");
            return (FAILED);
        }
    }


    return (PASSED);

}

/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_tx_test
 * Description: Function to perform NR5G sub6 Antenna TX path verification.
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              antenna_type - MAIN_RSSI/DIV_RSSI/MIMO1/MIMO2
 *              antenna_no - Antenna number
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_ant_tx_test (dev_object_t *dev, int antenna_type, int band)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;


    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, band) != PASSED) {
        printf("Failed to set RSSI test configuration\n");
        return (FAILED);
    }

    return (PASSED);

}

/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_gps_test 
 * Description: Function to perform GPS antenna connector RX path verification.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_gps_test (dev_object_t *dev)
{
    int at_test_cmd;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    at_test_cmd = NR_5G_GPS_ANTENNA_TEST;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test_cmd) != PASSED) {
       dev_NR_5g_telit_at_run_cmd(obj_telit_modem,
                                  NR_5G_SET_GPSANTPORT_ACTIVE);
       return (FAILED);
    }

    /* The modem will switch to Test mode during test, need switch modem 
     * back to OP mode after testing */
    if (dev_NR_5g_telit_modem_enable_operation_mode(dev) != PASSED) {
        printf("Failed to enable modem operation mode.\n");
        return (FAILED);
    }
        
    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_lpm_wwan_led_ctrl
 * Description: Function to turn on/off WWAN_LED when modem is in LPM.
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              led_opt - on or off WWAN_LED
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_lpm_wwan_led_ctrl (dev_object_t *dev,
                                                  int led_opt)
{
    int at_test_cmd;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    switch (led_opt) {
        case LED_OFF:
            at_test_cmd = NR_5G_LPM_WWANLED_OFF;
            break;
        case LED_ON:
            at_test_cmd = NR_5G_LPM_WWANLED_ON;
            break;
        case LED_DEFAULT:
            at_test_cmd = NR_5G_LPM_WWANLED_DEFAULT;
            break;
        default:
            printf("Invalid LED blinking option\n");
            return (FAILED);
    }


    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test_cmd));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_is_in_operation_mode
 * Description: Function to query the current testmode status
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_NR_5g_telit_modem_is_in_operation_mode (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_IN_OP_MODE) != PASSED) {
        return (FALSE);
    }
    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_enable_operation_mode
 * Description: Function to enable modem operation mode
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_enable_operation_mode (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;
    if (dev_NR_5g_telit_modem_is_in_operation_mode(dev) == TRUE) {
        return (PASSED);
    }

    /* Exit current testing mode */
    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_EXIT_TEST_MODE) 
        != PASSED) {
        printf("Failed to exit current testing mode\n");
        return (FAILED);
    }

    /* Enable operation mode */
    return (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_ENABLE_OP_MODE));
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_pwrsav_mode_ctrl
 * Description: Function to configure modem power saving mode
 * Input:       dev - dev_object_t pointer to the Telit device.
 *              mode - which mode will modem switch to while power
 *                     saving event is triggered(i.e. W_DISABLE_N pin
 *                     goes to LOW)
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_pwrsav_mode_ctrl (dev_object_t *dev,
                                                 int mode)
{
    int rc = FAILED;
    int at_test;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    dev_NR_5g_set_modem_pwrsav_para(mode);

    /* First check the current configuration of the modem power saving,
     * if the current configuration is what we want, return PASSED */
    at_test = NR_5G_CHK_PWRSAV_MODE;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test) == PASSED) {
        return (PASSED);
    }

    /* Configure the modem power saving mode */
    at_test = NR_5G_SET_PWRSAV;

    rc = dev_NR_5g_telit_at_run_cmd(obj_telit_modem, at_test);
    /* Modem will reboot after configured the power saving mode */
    if (rc == PASSED) {
        printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
        sleep(TELIT_NR_5G_RST_DELAY);
    }

    return (rc);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_enable_test_mode
 * Description: Function to enable modem test mode
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_enable_test_mode (dev_object_t *dev)
{
    int rc = FAILED;
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    rc = dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_ENABLE_TEST_MODE);

    /* Modem will reboot while switching modem test mode */
    if (rc == PASSED) {
        printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
        sleep(TELIT_NR_5G_RST_DELAY);
    }

    return (rc);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_is_in_lpm
 * Description: Function to check whether modem is in LPM(Low Power Mode)
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_NR_5g_telit_modem_is_in_lpm (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_IN_LOWPWR_MODE) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_is_online
 * Description: Function to check whether modem is online.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_NR_5g_telit_modem_is_online (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_FULL_FUNC) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_disable_shdn_indicator
 * Description: Function to disable the modem shutdown indicator
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_disable_shdn_indicator (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_DISABLE_SHDN_IND)
                                 != PASSED) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
    sleep(TELIT_NR_5G_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_set_shdn_indicator
 * Description: Function to set the modem shutdown indicator
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_NR_5g_telit_modem_set_shdn_indicator (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_SET_SHDN_IND) != PASSED) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_NR_5G_RST_DELAY);
    sleep(TELIT_NR_5G_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_NR_5g_telit_softshdn_indicator_is_set
 * Description: Function to check whether modem soft shutdown indicator is set.
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_NR_5g_telit_softshdn_indicator_is_set (dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, NR_5G_SOFTSHDN_IND_IS_SET)
                                 != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}

/****************************************************************************** 
 * Name:        dev_NR_5g_telit_modem_sim_hotswap_disable
 * Description: Function to disable SIM hot swap
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static boolean dev_NR_5g_telit_modem_sim_hotswap_disable(dev_object_t *dev) 
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem, 
                     NR_5G_SIM_MODE_CHANGE_FOR_DIAG) != PASSED) {
        return (FAILED);
    }
    return (PASSED);

}

/******************************************************************************
 * Name:        dev_NR_5g_telit_modem_sim_hotswap_status
 * Description: Function to get the SIM hotswap status
 * Input:       dev - dev_object_t pointer to the Telit device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_NR_5g_telit_modem_sim_hotswap_status(dev_object_t *dev)
{
    dev_NR_5g_telit_object_t *obj_telit_modem = (dev_NR_5g_telit_object_t *)dev;

    if (dev_NR_5g_telit_at_run_cmd(obj_telit_modem,
                     NR_5G_SIM_HOTSWAP_STATUS) != PASSED) {
        return (FALSE);
    }
    return (TRUE);

}

/*********************************************************************
 * $Log: dev_NR_5G_telit.c,v $
 * Revision 1.2  2021/06/02 02:56:19  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.5  2021/02/27 00:43:07  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.4  2021/02/12 01:08:18  tshanmug
 * Sears multi band test support
 *
 * Revision 1.1.2.3  2020/12/01 06:38:46  tshanmug
 * Sears antenna test modification to test all antenna in a single menu
 *
 */


