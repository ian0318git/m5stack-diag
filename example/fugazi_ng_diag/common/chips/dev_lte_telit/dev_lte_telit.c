/* $Id: dev_lte_telit.c,v 1.9 2020/08/19 09:48:53 markzha Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_telit/dev_lte_telit.c,v $
 *------------------------------------------------------------------
 *
 * Filename:	dev_lte_telit.c
 *
 * Description:	LTE Telit Driver. Supports following modules:
 *              LM940, LM960
 * Copyright (c) 2019 by cisco Systems, Inc.
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
#include "dev_lte_telit.h"
#include "dev_lte_telit_at.h"

void lte_telit_dev_create(dev_object_t *, dev_error_report_t);
static uint32 dev_lte_telit_attach(dev_object_t *);
static uint32 dev_lte_telit_detach(dev_object_t *);
static uint32 dev_lte_telit_restart(dev_object_t *);
static void dev_lte_telit_destroy(dev_object_t **);
static int dev_lte_telit_modem_detection(dev_object_t *);
static int dev_lte_telit_modem_power_down(dev_object_t *);
static int dev_lte_telit_modem_check_gpio5_stat(dev_object_t *, int);
static int dev_lte_telit_modem_disable_audio(dev_object_t *);
static int dev_lte_telit_modem_disable_fast_shutdown(dev_object_t *);
static int dev_lte_telit_modem_enable_fast_shutdown(dev_object_t *);
static int dev_lte_telit_modem_set_shdn_indicator(dev_object_t *);
static int dev_lte_telit_modem_disable_shdn_indicator(dev_object_t *);
static int dev_lte_telit_modem_disable_dying_gasp(dev_object_t *);
static int dev_lte_telit_modem_enable_dying_gasp(dev_object_t *);
static int dev_lte_telit_sim_detect_test(dev_object_t *, int);
static int dev_lte_telit_simin_pin_present(dev_object_t *, int);
static int dev_lte_telit_dump_simin_pin_status(dev_object_t *, int);
static int dev_lte_telit_switch_modem_usb_mode(dev_object_t *, int);
static int dev_lte_telit_modem_rssi_test(dev_object_t *, int, int);
static int dev_lte_telit_modem_gps_test(dev_object_t *);
static int dev_lte_telit_modem_reboot(dev_object_t *);
static int dev_lte_telit_modem_enable_operation_mode(dev_object_t *);
static int dev_lte_telit_modem_enable_operation_mode_without_esc(dev_object_t *);
static int dev_lte_telit_modem_enable_test_mode(dev_object_t *);
static int dev_lte_telit_modem_dump_temp(dev_object_t *);
static int dev_lte_telit_modem_dump_info(dev_object_t *);
static int dev_lte_telit_set_modem_img(dev_object_t *, char *);
static int dev_lte_telit_disable_img_switching(dev_object_t *);
static int dev_lte_telit_modem_lpm_wwan_led_ctrl(dev_object_t *, int);
static int dev_lte_telit_modem_pwrsav_mode_ctrl(dev_object_t *, int);
static boolean dev_lte_telit_modem_is_in_operation_mode(dev_object_t *);
static boolean dev_lte_telit_modem_audio_is_disable(dev_object_t *);
static boolean dev_lte_telit_softshdn_indicator_is_set(dev_object_t *);
static boolean dev_lte_telit_fast_shutdown_is_disable(dev_object_t *);
static boolean dev_lte_telit_dying_gasp_is_disable(dev_object_t *);
static boolean dev_lte_telit_modem_is_in_lpm(dev_object_t *);
static boolean dev_lte_telit_modem_is_online(dev_object_t *);
static boolean dev_lte_telit_modem_img_is_matched(dev_object_t *, char *);


/******************************************************************************
 * Name:        lte_telit_dev_create
 * Description: Create device object with various function point to "do nothing"
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              error_report_fn - error reporting function pointer. 
 * Returns:     none
 *****************************************************************************/
void lte_telit_dev_create(dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t *dev_fvt;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
                    NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in lte_telit_dev_create()", 0);
        printf("%s: NULL\n", __func__);
        return;
    }

    /* Init the device object structure to default */
    init_default_dev_object(dev, dev_fvt);

    obj_lte_telit->base.dev_object_fvt->dev_attach    = dev_lte_telit_attach;
    obj_lte_telit->base.dev_object_fvt->dev_detach    = dev_lte_telit_detach;
    obj_lte_telit->base.dev_object_fvt->dev_restart   = dev_lte_telit_restart;
    obj_lte_telit->base.dev_object_fvt->dev_error_report = error_report_fn;
    obj_lte_telit->base.dev_object_fvt->dev_destroy   = dev_lte_telit_destroy;
    obj_lte_telit->base.dev_object_fvt->dev_name      = "LTE_TELIT";

    obj_lte_telit->modem_type = TELIT_LM940;

    obj_lte_telit->callin_fvt = (dev_lte_telit_callin_fvt_t *)
                                 malloc(sizeof(dev_lte_telit_callin_fvt_t));
    obj_lte_telit->callout_fvt = (dev_lte_telit_callout_fvt_t *)
                                  malloc(sizeof(dev_lte_telit_callout_fvt_t));

    obj_lte_telit->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 * Name:        dev_lte_telit_attach
 * Description: Attach the Telit LTE device for use. This function will
 *              initialize and setup all necessary pointers and bring the chip
 *              to operation.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static uint32 dev_lte_telit_attach (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (obj_lte_telit->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_lte_telit_attach() callin malloc",
                         DEV_LTE_TELIT_ATTACH);
        return (FAILED);
    }
    if (obj_lte_telit->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_lte_telit_attach() callout malloc",
                         DEV_LTE_TELIT_ATTACH);
        return (FAILED);
    }

    /* Init the call in functions */
    obj_lte_telit->callin_fvt->modem_detection_test =
                               dev_lte_telit_modem_detection;
    obj_lte_telit->callin_fvt->sim_detect_test =
                               dev_lte_telit_sim_detect_test;
    obj_lte_telit->callin_fvt->modem_switch_usb_mode =
                               dev_lte_telit_switch_modem_usb_mode;
    obj_lte_telit->callin_fvt->modem_rssi_test =
                               dev_lte_telit_modem_rssi_test;
    obj_lte_telit->callin_fvt->modem_gps_test =
                               dev_lte_telit_modem_gps_test;
    obj_lte_telit->callin_fvt->modem_simin_pin_present =
                               dev_lte_telit_simin_pin_present;
    obj_lte_telit->callin_fvt->modem_dump_simin_pin_status =
                               dev_lte_telit_dump_simin_pin_status;
    obj_lte_telit->callin_fvt->modem_disable_img_switching =
                               dev_lte_telit_disable_img_switching;
    obj_lte_telit->callin_fvt->modem_set_lte_img =
                               dev_lte_telit_set_modem_img;
    obj_lte_telit->callin_fvt->modem_check_lte_img =
                               dev_lte_telit_modem_img_is_matched;
    obj_lte_telit->callin_fvt->modem_reboot =
                               dev_lte_telit_modem_reboot;
    obj_lte_telit->callin_fvt->modem_in_operation_mode =
                               dev_lte_telit_modem_is_in_operation_mode;
    obj_lte_telit->callin_fvt->modem_enable_op_mode =
                               dev_lte_telit_modem_enable_operation_mode;
    obj_lte_telit->callin_fvt->modem_enable_op_mode_without_esc =
                               dev_lte_telit_modem_enable_operation_mode_without_esc;
    obj_lte_telit->callin_fvt->modem_enable_test_mode =
                               dev_lte_telit_modem_enable_test_mode;
    obj_lte_telit->callin_fvt->modem_dump_temp =
                               dev_lte_telit_modem_dump_temp;
    obj_lte_telit->callin_fvt->modem_lpm_wwan_led_ctrl =
                               dev_lte_telit_modem_lpm_wwan_led_ctrl;
    obj_lte_telit->callin_fvt->modem_in_lpm =
                               dev_lte_telit_modem_is_in_lpm;
    obj_lte_telit->callin_fvt->modem_is_online =
                               dev_lte_telit_modem_is_online;
    obj_lte_telit->callin_fvt->modem_power_down =
                               dev_lte_telit_modem_power_down;
    obj_lte_telit->callin_fvt->modem_check_gpio5_stat =
                               dev_lte_telit_modem_check_gpio5_stat;
    obj_lte_telit->callin_fvt->modem_disable_shdn_indicator =
                               dev_lte_telit_modem_disable_shdn_indicator;
    obj_lte_telit->callin_fvt->modem_set_shdn_indicator =
                               dev_lte_telit_modem_set_shdn_indicator;
    obj_lte_telit->callin_fvt->modem_disable_fast_shutdown =
                               dev_lte_telit_modem_disable_fast_shutdown;
    obj_lte_telit->callin_fvt->modem_fast_shutdown_is_disable =
                               dev_lte_telit_fast_shutdown_is_disable;
    obj_lte_telit->callin_fvt->modem_disable_audio =
                               dev_lte_telit_modem_disable_audio;
    obj_lte_telit->callin_fvt->modem_audio_is_disable =
                               dev_lte_telit_modem_audio_is_disable;
    obj_lte_telit->callin_fvt->modem_softshdn_indic_is_enable =
                               dev_lte_telit_softshdn_indicator_is_set;
    obj_lte_telit->callin_fvt->modem_enable_fast_shutdown =
                               dev_lte_telit_modem_enable_fast_shutdown;
    obj_lte_telit->callin_fvt->modem_enable_dying_gasp =
                               dev_lte_telit_modem_enable_dying_gasp;
    obj_lte_telit->callin_fvt->modem_disable_dying_gasp =
                               dev_lte_telit_modem_disable_dying_gasp;
    obj_lte_telit->callin_fvt->modem_dying_gasp_is_disable =
                               dev_lte_telit_dying_gasp_is_disable;
    obj_lte_telit->callin_fvt->modem_pwrsaving_mode_ctrl =
                               dev_lte_telit_modem_pwrsav_mode_ctrl;
    obj_lte_telit->callin_fvt->modem_dump_info =
                               dev_lte_telit_modem_dump_info;

    obj_lte_telit->base.dev_state = DEV_STATE_ATTACH;
    return (PASSED);
}


/******************************************************************************
 * Name:        dev_lte_telit_detach
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
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static uint32 dev_lte_telit_detach (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_lte_telit->base.dev_object_fvt);

    obj_lte_telit->base.dev_state = DEV_STATE_DETACH;
    return (PASSED);
}


/******************************************************************************
 * Name:        dev_lte_telit_restart
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *****************************************************************************/
static uint32 dev_lte_telit_restart (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    obj_lte_telit->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
} 


/******************************************************************************
 * Name:        dev_lte_telit_destroy
 * Description: Destroy the dev_object structure and free all the resources.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *****************************************************************************/
static void dev_lte_telit_destroy (dev_object_t **dev)
{
    dev_lte_telit_object_t *obj_lte_telit;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_lte_telit = (dev_lte_telit_object_t *)dev;

    /* Free call out/in structure */
    if (obj_lte_telit->callout_fvt) {
        free(obj_lte_telit->callout_fvt);
    }
    if (obj_lte_telit->callin_fvt) {
        free(obj_lte_telit->callin_fvt);
    }

    /* Free dev_object_t */
    free(obj_lte_telit->base.dev_object_fvt);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_reboot
 * Description: Function to power down modem through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_reboot (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_REBOOT) != PASSED) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
    sleep(TELIT_LTE_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_dump_temp
 * Description: Function to dump modem temperature through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_dump_temp (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_DUMP_TEMP));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_dump_info
 * Description: Function to dump modem information through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_dump_info (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_DUMP_MODEM_INFO));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_disable_audio
 * Description: Function to disable the modem audio feature
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_disable_audio (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_DISABLE_AUDIO;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_audio_is_disable
 * Description: Function to check whether the modem audio feature is disable
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_modem_audio_is_disable (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_AUDIO_IS_DISABLE;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_dying_gasp_is_disable
 * Description: Function to check whether the Dying Gasp feature is disable
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_dying_gasp_is_disable (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_DG_IS_DISABLE;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_disable_dying_gasp
 * Description: Function to disable the modem Dying Gasp feature
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_disable_dying_gasp (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_DISABLE_DYINGGASP;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_enable_dying_gasp
 * Description: Function to enable the modem Dying Gasp feature
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_enable_dying_gasp (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_ENABLE_DYINGGASP;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_disable_fast_shutdown
 * Description: Function to disable the modem fast shutdown feature
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_disable_fast_shutdown (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_DISABLE_FASTSHDN;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_enable_fast_shutdown
 * Description: Function to enable fast shutdown feature and shutdown indicator
 *              for monitor fast shutdown through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_enable_fast_shutdown (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_ENABLE_FASTSHDN;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test)) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
    sleep(TELIT_LTE_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_fast_shutdown_is_disable
 * Description: Function to check whether the modem fast shutdown feature is
 *              disable
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_fast_shutdown_is_disable (dev_object_t *dev)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    at_test = LTE_FASTSHDN_IS_DISABLE;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_check_gpio5_stat
 * Description: Function to check whether the value of modem GPIO_05 is expected
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              expected_val - Expected GPIO value
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_check_gpio5_stat (dev_object_t *dev,
                                                 int expected_val)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (expected_val == GPIO_HIGH) {
        at_test = LTE_GPIO5_IS_HIGH;
    } else {
        at_test = LTE_GPIO5_IS_LOW;
    }

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_power_down
 * Description: Function to power down modem through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_power_down (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_PWR_DOWN));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_detection
 * Description: Function to detect modem through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_detection (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_MODEM_DETECTION));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_sim_detect_test
 * Description: Function to detect SIM card through AT command.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              which sim - SIM 0 or SIM 1
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_sim_detect_test (dev_object_t *dev, int which_sim)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    switch (which_sim) {
        case SIM_0:
            at_test = LTE_SIM1_DETECT_TEST;
            break;
        case SIM_1:
            at_test = LTE_SIM2_DETECT_TEST;
            break;
        default:
            printf("%s: Invaid SIM slot number (SIM %d)\n", __func__,
                                                            which_sim); 
            return (FAILED);
    }

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_dump_simin_pin_status
 * Description: Function to dump SIMIN pin status through AT command.
 *              SIMIN pins are used to indicate SIM card is inserted or not
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              which sim - SIM 0 or SIM 1
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_dump_simin_pin_status (dev_object_t *dev,
                                                int which_sim)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    switch (which_sim) {
        case SIM_0:
            at_test = LTE_DUMP_SIMIN1_STAT;
            break;
        case SIM_1:
            at_test = LTE_DUMP_SIMIN2_STAT;
            break;
        default:
            printf("%s: Invalid SIM slot number (SIM %d)\n", __func__,
                                                             which_sim); 
            return (FAILED);
    }
    return (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_simin_pin_present
 * Description: Function to check SIMIN pin status to see whether SIM card is
 *              present or not through AT command.
 *              SIMIN pins are used to indicate SIM card is inserted or not
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              which sim - SIM 0 or SIM 1
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_simin_pin_present (dev_object_t *dev, int which_sim)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    switch (which_sim) {
        case SIM_0:
            at_test = LTE_SIMIN1_DETECT_TEST;
            break;
        case SIM_1:
            at_test = LTE_SIMIN2_DETECT_TEST;
            break;
        default:
            printf("%s: Invaid SIM slot number (SIM %d)\n", __func__,
                                                            which_sim); 
            return (FAILED);
    }
    return (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_switch_modem_usb_mode
 * Description: Function to switch modem USB configuration to super-speed(3.0)
 *              mode or high-speed(2.0) mode.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              usb_mode - which USB configuration(HIGH_SPD_USB/SUPER_SPD_USB)
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_switch_modem_usb_mode (dev_object_t *dev, int usb_mode)
{
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (usb_mode == HIGH_SPD_USB) {
        at_test = LTE_SWITCH_USB2P0;
    } else if (usb_mode == SUPER_SPD_USB) {
        at_test = LTE_SWITCH_USB3P0;
    } else {
        printf("Invalid USB mode\n");
        return (FAILED);
    }

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) != PASSED) {
        return (FAILED);
    }
    
    printf("Wait %d seconds to reset modem ...\n", TELIT_LTE_RST_DELAY);
    sleep(TELIT_LTE_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_rssi_test
 * Description: Function to perform RSSI 4G LTE RX path verification.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              antenna_type - MAIN_RSSI/DIV_RSSI
 *              antenna_no - Antenna number
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_rssi_test (dev_object_t *dev, int antenna_type,
                                          int antenna_no)
{
    int at_test_cmd;
    char test_freq[16] = {0, };
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    /* Configure test band and RX bandwidth */
    switch (obj_lte_telit->modem_type) {
        case TELIT_LM940:
            at_test_cmd = LTE_RSSI_CONFIG_B2;
            break;
        case TELIT_LM960:
            if (antenna_no == LTE_ANTENNA_CON_0) {
                at_test_cmd = LTE_RSSI_CONFIG_B2;
            } else {
                at_test_cmd = LTE_RSSI_CONFIG_B30;
            }
            break;
        default:
            printf("Invaid modem type.\n");
            return (FAILED);
            break;
    }

    /* Prints out the test frequency, offset, and amplitude */
    if (at_test_cmd == LTE_RSSI_CONFIG_B2) {
        sprintf(test_freq, RSSI_B2_FREQ);
    } else {
        sprintf(test_freq, RSSI_B30_FREQ);
    }
    printf("\nFreq = %s MHz, Power = %s dBm\n", test_freq, RSSI_AMP);

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test_cmd) != PASSED) {
        printf("Failed to set RSSI test configuration\n");
        return (FAILED);
    }

    /* Read modem RX power level */
    switch (antenna_type) {
        case MAIN_RSSI:
            at_test_cmd = LTE_READ_MAIN_RSSI_PWR;
            break;
        case DIV_RSSI:
            at_test_cmd = LTE_READ_DIV_RSSI_PWR;
            break;
        default:
            printf("Invaid RSSI test option.\n");
            return (FAILED);
            break;
    }

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test_cmd) != PASSED) {
        return (FAILED);
    }

    /* Exit test mode and switch back to operation mode */
    if (dev_lte_telit_modem_enable_operation_mode(dev) != PASSED) {
        printf("Failed to enable modem operation mode.\n");
        return (FAILED);
    }

    return (PASSED);

}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_gps_test 
 * Description: Function to perform GPS antenna connector RX path verification.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_gps_test (dev_object_t *dev)
{
    int at_test_cmd;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    /* AT#TESTMODE="GNSS", which is used to dump the GNSS test result, is only
     * supported by the FW 24.01.513-B006 and later */
    printf("[WARNING] This test is dependent on Telit FW.\n"
           "Please make sure the current FW version is 24.01.513-B006 or later.\n");
    /* Prints out the test frequency, offset, and amplitude */
    printf("\nFreq = %s MHz, Power = %s dBm\n", GPS_FREQ, GPS_AMP);
    at_test_cmd = LTE_GPS_ANTENNA_TEST;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test_cmd) != PASSED) {
       return (FAILED);
    }

    /* The modem will switch to Test mode during test, need switch modem 
     * back to OP mode after testing */
    if (dev_lte_telit_modem_enable_operation_mode_without_esc(dev) != PASSED) {
        printf("Failed to enable modem operation mode.\n");
        return (FAILED);
    }
        
    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_lpm_wwan_led_ctrl
 * Description: Function to turn on/off WWAN_LED when modem is in LPM.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              led_opt - on or off WWAN_LED
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_lpm_wwan_led_ctrl (dev_object_t *dev,
                                                  int led_opt)
{
    int at_test_cmd;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    /* AT#WWANLED", which is used to control WWAN_LED on/off, is only
     * supported by the FW (CAT11)24.01.5x3-B005/(CAT18)32.00.0x2.1-B003 
     * and later */
    printf("[WARNING] This test is dependent on Telit FW.\n"
           "Please make sure the current FW version is (CAT11)24.01.5x3-B005/"
           "(CAT18)32.00.0x2.1-B003 or later.\n");

    switch (led_opt) {
        case LED_OFF:
            at_test_cmd = LTE_LPM_WWANLED_OFF;
            break;
        case LED_ON:
            at_test_cmd = LTE_LPM_WWANLED_ON;
            break;
        case LED_DEFAULT:
            at_test_cmd = LTE_LPM_WWANLED_DEFAULT;
            break;
        default:
            printf("Invalid LED blinking option\n");
            return (FAILED);
    }


    return (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test_cmd));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_is_in_operation_mode
 * Description: Function to query the current testmode status
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_modem_is_in_operation_mode (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_IN_OP_MODE) != PASSED) {
        return (FALSE);
    }
    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_enable_operation_mode
 * Description: Function to enable modem operation mode
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_enable_operation_mode (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;
    if (dev_lte_telit_modem_is_in_operation_mode(dev) == TRUE) {
        return (PASSED);
    }

    /* Exit current testing mode */
    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_EXIT_TEST_MODE) 
        != PASSED) {
        printf("Failed to exit current testing mode\n");
        return (FAILED);
    }

    /* Enable operation mode */
    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_ENABLE_OP_MODE));
}

/****************************************************************************** 
 * Name:        dev_lte_telit_modem_enable_operation_mode_without_esc
 * Description: Function to enable modem operation mode without AT#TESTMODE="ESC"
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_enable_operation_mode_without_esc (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;
    if (dev_lte_telit_modem_is_in_operation_mode(dev) == TRUE) {
        return (PASSED);
    }

    /* Enable operation mode */
    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_ENABLE_OP_MODE));
}

/****************************************************************************** 
 * Name:        dev_lte_telit_modem_pwrsav_mode_ctrl
 * Description: Function to configure modem power saving mode
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              mode - which mode will modem switch to while power
 *                     saving event is triggered(i.e. W_DISABLE_N pin
 *                     goes to LOW)
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_pwrsav_mode_ctrl (dev_object_t *dev,
                                                 int mode)
{
    int rc = FAILED;
    int at_test;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    dev_lte_set_modem_pwrsav_para(mode);

    /* First check the current configuration of the modem power saving,
     * if the current configuration is what we want, return PASSED */
    at_test = LTE_CHK_PWRSAV_MODE;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test) == PASSED) {
        return (PASSED);
    }

    /* Configure the modem power saving mode */
    at_test = LTE_SET_PWRSAV;

    rc = dev_lte_telit_at_run_cmd(obj_lte_telit, at_test);
    /* Modem will reboot after configured the power saving mode */
    if (rc == PASSED) {
        printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
        sleep(TELIT_LTE_RST_DELAY);
    }

    return (rc);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_enable_test_mode
 * Description: Function to enable modem test mode
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_enable_test_mode (dev_object_t *dev)
{
    int rc = FAILED;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    rc = dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_ENABLE_TEST_MODE);

    /* Modem will reboot while switching modem test mode */
    if (rc == PASSED) {
        printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
        sleep(TELIT_LTE_RST_DELAY);
    }

    return (rc);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_is_in_lpm
 * Description: Function to check whether modem is in LPM(Low Power Mode)
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_modem_is_in_lpm (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_IN_LOWPWR_MODE) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_is_online
 * Description: Function to check whether modem is online.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_modem_is_online (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_FULL_FUNC) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_disable_shdn_indicator
 * Description: Function to disable the LTE modem shutdown indicator
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_disable_shdn_indicator (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_DISABLE_SHDN_IND)
                                 != PASSED) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
    sleep(TELIT_LTE_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_set_shdn_indicator
 * Description: Function to set the LTE modem shutdown indicator
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_modem_set_shdn_indicator (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_SET_SHDN_IND) != PASSED) {
        return (FAILED);
    }

    printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
    sleep(TELIT_LTE_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_softshdn_indicator_is_set
 * Description: Function to check whether modem soft shutdown indicator is set.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_softshdn_indicator_is_set (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_SOFTSHDN_IND_IS_SET)
                                 != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_disable_img_switching
 * Description: Function to disable the LTE modem SIM-based carrier image
                auto-switching feature
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_disable_img_switching (dev_object_t *dev)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    return (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_DISABLE_IMG_SWITCHING));
}


/****************************************************************************** 
 * Name:        dev_lte_telit_set_modem_img
 * Description: Function to set the LTE modem image based on carrier
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              which_carrier - carrier name to select modem image
 * Returns:     PASSED/FAILED
 *****************************************************************************/
static int dev_lte_telit_set_modem_img (dev_object_t *dev, char *which_carrier)
{
    int at_test_cmd;
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    if (strcmp(which_carrier, ATT) == 0) {
        at_test_cmd = LTE_SET_ATT_IMG;
    } else if (strcmp(which_carrier, VERIZON) == 0) {
        at_test_cmd = LTE_SET_VERIZON_IMG;
    } else if (strcmp(which_carrier, GENERIC) == 0) {
        at_test_cmd = LTE_SET_GENERIC_IMG;
    } else if (strcmp(which_carrier, SPRINT) == 0) {
        at_test_cmd = LTE_SET_SPRINT_IMG;
    } else {
        printf("Invalid carrier name\n");
        return (FAILED);
    }

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, at_test_cmd) != PASSED) {
        return (FAILED);
    }

    /* Modem will reboot automatically after switching to different image */
    printf("Wait %d seconds to reset modem...\n", TELIT_LTE_RST_DELAY);
    sleep(TELIT_LTE_RST_DELAY);

    return (PASSED);
}


/****************************************************************************** 
 * Name:        dev_lte_telit_modem_img_is_matched
 * Description: Function to check whether the avtice image is as expected.
 * Input:       dev - dev_object_t pointer to the Telit LTE device.
 *              which_carrier - expected carrier image
 * Returns:     TRUE/FALSE
 *****************************************************************************/
static boolean dev_lte_telit_modem_img_is_matched (dev_object_t *dev,
                                                   char *which_carrier)
{
    dev_lte_telit_object_t *obj_lte_telit = (dev_lte_telit_object_t *)dev;

    dev_lte_telit_store_expected_img(which_carrier);

    if (dev_lte_telit_at_run_cmd(obj_lte_telit, LTE_IMG_IS_MATCHED) != PASSED) {
        return (FALSE);
    }

    return (TRUE);
}


/*------------------------------------------------------------------
$Log: dev_lte_telit.c,v $
Revision 1.9  2020/08/19 09:48:53  markzha
*** empty log message ***

Revision 1.8  2019/08/14 02:28:05  shjung

1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational
1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational

Revision 1.7  2019/07/10 08:28:34  sherliu2
Supported Hyperloop-PIM

Revision 1.6  2019/07/05 07:33:49  sherliu2
Modified enable modem operation mode function

Revision 1.5  2019/06/26 03:52:59  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.4  2019/06/14 05:46:08  shjung
Fixed CSCvq12342, disable modem dying gasp before toggling Modem_Power_ON pin to avoid modem perform fast shutdown and added enabling dying gasp utility

Revision 1.3  2019/05/20 07:28:14  shjung
1. Replace USB serial driver option.ko/usb_wwan.ko with GobiSerial.ko
2. Changes based on last code review comments.
3. Use poll mechanism to query modem functionality level in W_DISABLE pin test
4. Add 1 second delay after close tty device, which is following Telit's test script process.

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

Revision 1.1.2.14  2019/04/08 06:41:22  shjung
Check the configuration of modem shutdown indicator while powering down the modem

Revision 1.1.2.13  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.12  2019/03/19 09:38:05  shjung
Chaged RSSI test bands based on test carrier

Revision 1.1.2.11  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.10  2019/03/12 02:53:01  shjung

1. Removed OP mode enabling process when RF test failed
2. Added query modem testmode status function
3. Added enable OP mode function
4. Adjusted RF test criteria
5. Code clean up

Revision 1.1.2.9  2019/03/04 06:06:59  sherliu2
Added warning message for WWAN LED test

Revision 1.1.2.8  2019/02/25 06:00:17  shjung
Added warning message for GPS antenna test

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
