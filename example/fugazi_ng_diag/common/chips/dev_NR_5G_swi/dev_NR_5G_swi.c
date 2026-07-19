/* 
 * $Id: dev_NR_5G_swi.c,v 1.3 2021/06/30 20:04:55 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_NR_5G_swi/dev_NR_5G_swi.c,v $
 *------------------------------------------------------------------
 * Filename: dev_NR_5G_swi.c
 *
 * Description:	5G SWI Driver. Supports following modules
 *              
 * Copyright (c) 2019-2020 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "defs.h"
#include "common.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#ifdef LINUX_APP
#include <assert.h>
#endif


#include "dev_NR_5G_swi.h"
#include "dev_NR_5G_swi_at.h"
#include "dev_NR_5G_band_info.h"


static uint32 dev_swi_5g_attach(dev_object_t *);
static uint32 dev_swi_5g_detach(dev_object_t *);
static uint32 dev_swi_5g_restart(dev_object_t *);
static void dev_swi_5g_destroy(dev_object_t **);



static int dev_swi_5g_modem_info_display(dev_object_t *, int);
static int dev_swi_5g_modem_detection_test(dev_object_t *);
static int dev_swi_5g_sim_detect_test(dev_object_t *, int);
static int dev_swi_5g_gps_ant_test(dev_object_t *, int);
static int dev_swi_5g_modem_sub6_rssi_test(dev_object_t *, int, int, int);
static int dev_swi_start_tx_rssi(dev_object_t *, int , int);
static int dev_swi_stop_tx_rssi(dev_object_t *, int , int);
static int dev_swi_5g_modem_exit_tm(dev_object_t *, int);
static int dev_swi_5g_modem_reset_test(dev_object_t *);
static int dev_swi_5g_sim_detect_pin_present(dev_object_t *, int, int);
static int dev_swi_5g_sim_detect_pin_status(dev_object_t *, int);
static int dev_swi_5g_toggle_mmwv_pon(dev_object_t *, int, int);
static int dev_swi_5g_modem_mmwv_rssi_test (dev_object_t *, int, int, int);
static int dev_swi_5g_modem_mmwv_transmit_test (dev_object_t *, int, int, int);
static int dev_swi_5g_modem_mmwv_transmit_stop(dev_object_t *, int param);
static int dev_swi_5g_modem_temperature_test (dev_object_t *);
static int dev_toggle_wwan_led(dev_object_t *, int);
static int dev_get_custom_param(dev_object_t *, int);
static int dev_config_custom_param(dev_object_t *, int);
static int dev_get_sarstate(dev_object_t *, int param);
static int dev_get_fsn_num (dev_object_t *);
static int dev_get_mmwave_ant_present_status (dev_object_t *); 
static int dev_swi_5g_modem_sub6_ota_rssi_test(dev_object_t *, int, int, int);

#ifdef MMWAVE_INDVIDUAL_ANTENNA_CTRL
static int dev_swi_5g_toggle_mmwv_pon_indv(dev_object_t *, int, int);
#endif

void swi_5g_dev_create(dev_object_t *, dev_error_report_t);

/******************************************************************************
 *
 * Name:	swi_5g_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the SWI device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void swi_5g_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in swi_5g_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_5g_swi->base.dev_object_fvt->dev_attach         = dev_swi_5g_attach;
    obj_5g_swi->base.dev_object_fvt->dev_detach         = dev_swi_5g_detach;
    obj_5g_swi->base.dev_object_fvt->dev_restart        = dev_swi_5g_restart;
    obj_5g_swi->base.dev_object_fvt->dev_error_report   = error_report_fn;
    obj_5g_swi->base.dev_object_fvt->dev_destroy        = dev_swi_5g_destroy;
    obj_5g_swi->base.dev_object_fvt->dev_name           = "SWI 5g EM9190";

    obj_5g_swi->modem_type = SWI_EM_9190;

    obj_5g_swi->callin_fvt = (dev_NR_5G_swi_callin_fvt_t *)
                               malloc(sizeof(dev_NR_5G_swi_callin_fvt_t));
    obj_5g_swi->callout_fvt = (dev_NR_5G_swi_callout_fvt_t *)
                                malloc(sizeof(dev_NR_5G_swi_callout_fvt_t));

    obj_5g_swi->base.dev_state = DEV_STATE_CREATE;
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_attach()
 *
 * Description:	Attach the SWI device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_swi_5g_attach (dev_object_t *dev)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    if (obj_5g_swi->callin_fvt == NULL) {
        printf ("\nobj_5g_swi->callin_fvt == NULL ");
        DEV_ERROR_REPORT(dev, "dev_swi_5g_attach() callin malloc", DEV_NR_5G_SWI_ATTACH);
        return (FAILED);
    }

    if (obj_5g_swi->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_swi_5g_attach() callout malloc", DEV_NR_5G_SWI_ATTACH);
        printf ("\nobj_5g_swi->callout_fvt == NULL ");
        return (FAILED);
    }

    /* init the call in function */
    obj_5g_swi->callin_fvt->show_modem_info = dev_swi_5g_modem_info_display;
    obj_5g_swi->callin_fvt->modem_detection_test = dev_swi_5g_modem_detection_test;
    obj_5g_swi->callin_fvt->modem_temperature_test = dev_swi_5g_modem_temperature_test;
    obj_5g_swi->callin_fvt->modem_reset_test = dev_swi_5g_modem_reset_test;
    obj_5g_swi->callin_fvt->modem_exit_tm = dev_swi_5g_modem_exit_tm;
    obj_5g_swi->callin_fvt->modem_sub6_rssi_test = dev_swi_5g_modem_sub6_rssi_test;
    obj_5g_swi->callin_fvt->modem_sub6_ota_rssi_test = dev_swi_5g_modem_sub6_ota_rssi_test;
    obj_5g_swi->callin_fvt->modem_sub6_tx_test = dev_swi_start_tx_rssi;
    obj_5g_swi->callin_fvt->modem_stop_tx = dev_swi_stop_tx_rssi;
    obj_5g_swi->callin_fvt->modem_gps_ant_test = dev_swi_5g_gps_ant_test;
    obj_5g_swi->callin_fvt->sim_detect_test = dev_swi_5g_sim_detect_test;
    obj_5g_swi->callin_fvt->sim_detect_pin_present = dev_swi_5g_sim_detect_pin_present;
    obj_5g_swi->callin_fvt->display_sim_detect_stat = dev_swi_5g_sim_detect_pin_status;
    obj_5g_swi->callin_fvt->toggle_wwan_led = dev_toggle_wwan_led;
    obj_5g_swi->callin_fvt->toggle_mmwv_pon = dev_swi_5g_toggle_mmwv_pon;
    obj_5g_swi->callin_fvt->modem_mmwv_rssi_test = dev_swi_5g_modem_mmwv_rssi_test;
    obj_5g_swi->callin_fvt->modem_mmwv_transmit_test = dev_swi_5g_modem_mmwv_transmit_test;
    obj_5g_swi->callin_fvt->modem_mmwv_transmit_stop = dev_swi_5g_modem_mmwv_transmit_stop;
    obj_5g_swi->callin_fvt->modem_mmwv_ant_status = dev_get_mmwave_ant_present_status;
    obj_5g_swi->callin_fvt->modem_get_custom_param = dev_get_custom_param;
    obj_5g_swi->callin_fvt->modem_config_custom_param = dev_config_custom_param;
    obj_5g_swi->callin_fvt->modem_get_sarstate = dev_get_sarstate;
    obj_5g_swi->callin_fvt->modem_get_fsn_num = dev_get_fsn_num;

    obj_5g_swi->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_detach()
 *
 * Description:	detach the device specific functions from the caller.
 *	        	All of the device specific function are connected to the
 *        		dev_do_nothing() function, except for the dev_attach()
 *        		function. Also, the dev_state must be assigned the value
 *        		of DEV_STATE_DETACH.
 *
 *        		Since, some platforms may want to detach the device, but not
 *        		release the memory resources (via a free () in the
 *        		dev_destroy()), this function can be executed to accomplish
 *        		this task. However, before a detached device can be used again,
 *        		it must be re-attached (via the dev_attach()).
 *
 * Input:	Pointer to the SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_swi_5g_detach (dev_object_t *dev)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_5g_swi->base.dev_object_fvt);

    obj_5g_swi->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);
}

/******************************************************************************
 * Name:	dev_swi_5g_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the SWI device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_swi_5g_restart (dev_object_t *dev)
{
    dev_5g_swi_object_t *obj_5g_swi= (dev_5g_swi_object_t *) dev;

    obj_5g_swi->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}

/******************************************************************************
 * Name:	dev_swi_5g_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the SWI object
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_swi_5g_destroy (dev_object_t **dev)
{
    dev_5g_swi_object_t *obj_5g_swi;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }
    obj_5g_swi = (dev_5g_swi_object_t *)dev;
    if (obj_5g_swi->callout_fvt) {
        free(obj_5g_swi->callout_fvt);	/* Free callout struct */
    }

    if (obj_5g_swi->callin_fvt) {
        free(obj_5g_swi->callin_fvt);		/* Free callin struct */
    }

    free(obj_5g_swi->base.dev_object_fvt);	/* Free dev_object_t */
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_modem_detection_test
 *
 * Description:	Function to detect modem through AT command
 *
 * Input:	Pointer to the SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_detection_test (dev_object_t *dev)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

#if 0
    //To excercise the modem host interface
    if (dev_swi_5g_at_run_cmd(obj_5g_swi, MODEM_HOST_IF_EXERCISE) == FAILED) {
        printf ("\nFailed extra commands");
        return FAILED;
    }else {
        printf ("\nextra modem commands passed");
    }
#endif

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, MODEM_ATI_TEST));
}

/******************************************************************************
 *
 * Name:        dev_swi_5g_modem_temperature_test
 *
 * Description: Function to read modem temp through AT command
 *
 * Input:       Pointer to the SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_temperature_test (dev_object_t *dev)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, MODEM_TEMP_DETECT_TEST));
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_modem_reset_test
 *
 * Description:	Reset modem through AT command
 *
 * Input:	Pointer to the SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_reset_test (dev_object_t *dev)
{
    int rc = PASSED;
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    rc = dev_swi_5g_at_run_cmd(obj_5g_swi, MODEM_RESET_TEST);
    if (rc != PASSED) {
        return rc;
    }

    printf ("\nModem resetting please wait...");
    fflush(stdout);
    msleep(MODEM_RESET_DELAY);
    return rc;
}

/******************************************************************************
 *
 * Name:        dev_swi_5g_modem_sub6_ota_rssi_test
 *
 * Description: Main/AUX/MIMO RSSI Test, requires external equipment
 *
 * Input:       Pointer to the SWI device object
 *          which_rssi - Main/Div/mimo RSSI
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/

static int dev_swi_5g_modem_sub6_ota_rssi_test (dev_object_t *dev, int which_ant, 
                                       int band, int exp_power)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev; 
    int rc = PASSED;
    int idx;
    nr_sub6_band_struct *band_struct;
    int tbl_size;

    band_struct = nr_sub6_band_tbl;
    tbl_size = band_tbl_size;


    for (idx = 0; idx < tbl_size; idx++, band_struct++) {
        if (band_struct->band_num == band){
            break;
        }
    }
    if (idx >= tbl_size) {
        printf ("\nBand info not available in the table");
        return FAILED;
    }

    if ((band_struct->test_supported_ant & which_ant) != which_ant){
        printf ("\nThis band is not supported!!!!");
        return FAILED;
    }

    rc = sub6_ant_test_band_config(band_struct, which_ant, exp_power);
    if (rc == FAILED) {
        printf ("\nFailed to update param in the table !!!!");
        return FAILED;

    }


    if (which_ant == (MAIN_RSSI | AUX_RSSI | MIMO1_RSSI | MIMO2_RSSI)) {
        rc = dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_FR1_OTA_ALL_ANT_RX_TEST);
    }
    else {
        rc = dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_FR1_OTA_IND_ANT_RX_TEST);
    }

    return rc;
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_modem_sub6_rssi_test
 *
 * Description:	Main/Div/MIMO RSSI Test, requires external equipment
 *
 * Input:	Pointer to the SWI device object
 *          which_rssi - Main/Div/mimo RSSI
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_sub6_rssi_test (dev_object_t *dev, int which_ant, 
                                       int band, int power)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev; 
    int rc = PASSED;
    int idx;
    nr_sub6_band_struct *band_struct;
    int tbl_size;

    band_struct = nr_sub6_band_tbl;
    tbl_size = band_tbl_size;

    for (idx = 0; idx < tbl_size; idx++, band_struct++) {
        if (band_struct->band_num == band){
            break;
        }
    }
    if (idx >= tbl_size) {
        printf ("\nBand info not available in the table");
        return FAILED;
    }

    if ((band_struct->test_supported_ant & which_ant) != which_ant){
        printf ("\nThis band is not supported!!!!");
        return FAILED;
    }

    rc = sub6_ant_test_band_config(band_struct, which_ant, power);
    if (rc == FAILED) {
        printf ("\nFailed to update param in the table !!!!");
        return FAILED;

    }

    printf("\nFreq = %s MHz, Power = %d dBm\n", band_struct->rx_center_freq, power);

    //Compare for the all ant ro individual antenna
    rc = dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_FR1_RX_TEST);
    if (rc != PASSED) {
        dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_DROP_RADIO_CFG);
        dev_swi_5g_at_run_cmd(obj_5g_swi, NR_5G_MODEM_EXIT_TM);
    }
    printf ("\nmodem resetting please wait...");
    fflush(stdout);
    msleep(MODEM_RESET_DELAY);
    return rc;
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_gps_ant_test
 *
 * Description:	GPS Antenna Test, requires external equipment
 *
 * Input:	Pointer to the SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_gps_ant_test (dev_object_t *dev, int port)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev; 
    int rc = PASSED;

    if (port == GNSS_L1_RSSI){
        printf("\nSignal Generator Freq = %s MHz, Power = %s dBm.\n",
                                               GPS_L1_FREQ, GPS_AMP);
        rc = dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_GPS_L1_TEST); 
    } else if (port == GNSS_L5_RSSI) {
        printf("\nSignal Generator Freq = %s MHz, Power = %s dBm.\n",
                                               GPS_L5_FREQ, GPS_AMP);
        rc = dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_GPS_L5_TEST); 

    }

    if (rc != PASSED) {
        dev_swi_5g_at_run_cmd(obj_5g_swi, NR_5G_MODEM_EXIT_TM);
    }

    printf ("\nmodem resetting please wait...");
    fflush(stdout);
    msleep(MODEM_RESET_DELAY);
    return rc;
}

/******************************************************************************
 *
 * Name:    dev_swi_5g_modem_mmwv_rssi_test
 *
 * Description: Main/Div RSSI Test, requires external equipment
 *
 * Input:   dev          - Pointer to the SWI device object
 *          antenna_mask - Which QTM527 antenna to activate
 *          band         - Which mmwave band number
 *          power        - What power level to measure
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_mmwv_rssi_test (dev_object_t *dev, int antenna_mask,
                            int band, int power)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    int rc = PASSED;
    int idx;

    for (idx = 0; idx < band_mmwave_tbl_size; idx++) {
        if (nr_mmwave_band_tbl[idx].band_num == band){
            break;
        }
    }

    if (idx >= band_mmwave_tbl_size) {
        printf ("\nBand info not available in the table");
        return FAILED;
    }

    rc = ant_mmwave_rx_test_radio_config(&nr_mmwave_band_tbl[idx], antenna_mask, power);
    if (rc == FAILED) {
        printf ("\nFailed to update param in the table !!!!");
        return FAILED;
    }

    printf("\nTx/Rx Channel = %d/%d, IF Freq = %s MHz, Power = %d dBm\n",
            nr_mmwave_band_tbl[idx].tx_channel, nr_mmwave_band_tbl[idx].rx_channel,
            nr_mmwave_band_tbl[idx].if_fr_mhz, power);

    rc = dev_swi_5g_at_run_cmd(obj_5g_swi, MMWAVE_FR2_RSSI_RX_TEST);
    if (rc != PASSED) {
        if (dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_DROP_RADIO_CFG))
            printf ("Failed: RSSI_DROP_RADIO_CFG\n");
        if (dev_swi_5g_at_run_cmd(obj_5g_swi, MMWAVE_ANTENNA_PON_DISABLE))
            printf ("Failed: MMWAVE_ANTENNA_PON_DISABLE\n");
    }
    return rc;

}

/******************************************************************************
 *
 * Name:    dev_swi_5g_modem_mmwv_transmit_test
 *
 * Description: Main/Div RSSI Test, requires external equipment
 *
 * Input:   Pointer to the LTE SWI device object
 *          which_rssi - Main/Div RSSI
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_mmwv_transmit_test (dev_object_t *dev, int antenna_mask,
        int band, int power)
{

    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    int rc = PASSED;
    int idx;

    for (idx = 0; idx < band_mmwave_tbl_size; idx++) {
        if (nr_mmwave_band_tbl[idx].band_num == band){
            break;
        }
    }

    if (idx >= band_mmwave_tbl_size) {
        printf ("\nBand info not available in the table");
        return FAILED;
    }

    rc = ant_mmwave_tx_test_radio_config(&nr_mmwave_band_tbl[idx], antenna_mask, power);
    if (rc == FAILED) {
        printf ("\nFailed to update param in the table !!!!");
        return FAILED;

    }

    printf("\nTx/Rx Channel = %d/%d, IF Freq = %s MHz, Power = %d dBm\n",
            nr_mmwave_band_tbl[idx].tx_channel, nr_mmwave_band_tbl[idx].rx_channel,
            nr_mmwave_band_tbl[idx].if_fr_mhz, power);

    rc = dev_swi_5g_at_run_cmd(obj_5g_swi, MMWAVE_FR2_TRANSMIT_TEST);
    if (rc != PASSED) {
        if (dev_swi_5g_at_run_cmd(obj_5g_swi, MMWAVE_FR2_TRANSMIT_STOP))
            printf ("Failed: MMWAVE_FR2_TRANSMIT_STOP\n");
        if (dev_swi_5g_at_run_cmd(obj_5g_swi, RSSI_DROP_RADIO_CFG))
            printf ("Failed: RSSI_DROP_RADIO_CFG\n");
        if (dev_swi_5g_at_run_cmd(obj_5g_swi, MMWAVE_ANTENNA_PON_DISABLE))
            printf ("Failed: MMWAVE_ANTENNA_PON_DISABLE\n");
    }
    fflush(stdout);
    return rc;
}

/******************************************************************************
 *
 * Name:        dev_swi_5g_modem_mmwv_transmit_stop
 *
 * Description: Function to stop the radio tranmit
 *
 * Input:       Pointer to the NR_5G SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_mmwv_transmit_stop (dev_object_t *dev, int param)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, param));
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_sim_detect_test
 *
 * Description:	SIM Detection Test 
 *
 * Input:	Pointer to the SWI device object
 *          which_sim - SIM 0 or SIM 1
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_sim_detect_test (dev_object_t *dev, int which_sim)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev; 
    int at_test_cmd;

    if (which_sim == SIM_0) {
        at_test_cmd = NR_5G_SIM0_DETECT_TEST;
    } else {
        //EM9190 has only one external sim
        if (which_sim == SIM_1) {
            printf("%s: SWI doesn't support SIM 1\n", __func__);
            return (FAILED);
        }

    }

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, at_test_cmd)); 
}

/******************************************************************************
 *
 * Name:	dev_swi_5g_sim_detect_pin_present
 *
 * Description:	SIM Dectect PIN present
 *
 * Input:	Pointer to the SWI device object, which sim to check and status
 *          
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_sim_detect_pin_present (dev_object_t *dev, 
                                              int present, int which_sim)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    int at_test_cmd;
    
    //EM9190 has only one external sim.. no check on which SIM.

    if (present == SIM_DET_PIN_PRE) {
        at_test_cmd = SIM_DETECT_PIN_PRESENT; 
    } else {
        at_test_cmd = SIM_DETECT_PIN_NO_PRESENT;
    }

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, at_test_cmd));
}

/******************************************************************************
 *
 * Name:        dev_swi_5g_sim_detect_pin_status
 *
 * Description: SIM Detect PIN status
 *
 * Input:       Pointer to the SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_sim_detect_pin_status (dev_object_t *dev, int sim_num)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, SIM_DETECT_PIN_STATUS));
}
/******************************************************************************
 *
 * Name:        dev_toggle_wwan_led
 *
 * Description: LTE LED status
 *
 * Input:       Pointer to the LTE SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_toggle_wwan_led (dev_object_t *dev, int stat)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    int at_test_cmd;
   
    if (stat == WWAN_LED_ON) {
        at_test_cmd = WWAN_LED_TURN_ON;
    } else {
        at_test_cmd = WWAN_LED_TURN_OFF;
    }

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, at_test_cmd));
}

/******************************************************************************
 *
 * Name:        dev_swi_5g_toggle_mmwv_pon
 *
 * Description: 5G MMWAVE PON signal control for all antennas simultaneously
 *
 * Input:       Pointer to the LTE SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_toggle_mmwv_pon (dev_object_t *dev, int antenna_mask, int state)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    int at_test_cmd;

    at_test_cmd = state==MMWAVE_PON_ENABLE ? MMWAVE_ANTENNA_PON_ENABLE : MMWAVE_ANTENNA_PON_DISABLE;

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, at_test_cmd));
}

#ifdef  MMWAVE_INDVIDUAL_ANTENNA_CTRL
/******************************************************************************
 *
 * Name:        dev_swi_5g_toggle_mmwv_pon_indv
 *
 * Description: 5G MMWAVE PON signal control for Individual Antennas
 *
 * Input:       Pointer to the LTE SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_toggle_mmwv_pon_indv (dev_object_t *dev, int antenna_no,
                int state)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    int at_test_cmd;

    switch (antenna_no) {

    case MMWAVE_ANTENNA_0:
        at_test_cmd = state==MMWAVE_PON_ENABLE ?  MMWAVE_ANTENNA0_PON_ENABLE : MMWAVE_ANTENNA0_PON_DISABLE;
        break;
    case MMWAVE_ANTENNA_1:
        at_test_cmd = state==MMWAVE_PON_ENABLE ?  MMWAVE_ANTENNA1_PON_ENABLE : MMWAVE_ANTENNA1_PON_DISABLE;
        break;
    case MMWAVE_ANTENNA_2:
        at_test_cmd = state==MMWAVE_PON_ENABLE ?  MMWAVE_ANTENNA2_PON_ENABLE : MMWAVE_ANTENNA2_PON_DISABLE;
        break;
    case MMWAVE_ANTENNA_3:
        at_test_cmd = state==MMWAVE_PON_ENABLE ?  MMWAVE_ANTENNA3_PON_ENABLE : MMWAVE_ANTENNA3_PON_DISABLE;
        break;
    default:
        return(FAILED);
    }

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, at_test_cmd));
}
#endif

/******************************************************************************
 *
 * Name:        dev_swi_5g_modem_info_display
 *
 * Description: Function to display modem information like mfg name, model number,
 *              serial number, firmware info, modem temperature.
 *
 * Input:       dev     - Pointer to the NR_5G SWI device object
 *              info_id - info to query
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_info_display(dev_object_t *dev, int info_id)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, info_id));
}

/******************************************************************************
 *
 * Name:        dev_swi_5g_modem_exit_tm
 *
 * Description: Function to revert back from test mode to operational mode
 *
 * Input:       Pointer to the NR_5G SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_5g_modem_exit_tm(dev_object_t *dev, int param)
{
    int rc = PASSED;
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    rc = dev_swi_5g_at_run_cmd(obj_5g_swi, NR_5G_MODEM_EXIT_TM);
    printf ("\nmodem resetting please wait...");
    fflush(stdout);
    msleep(MODEM_RESET_DELAY);
    return rc;
}

/******************************************************************************
 *
 * Name:        dev_swi_start_tx_rssi
 *
 * Description: Function to start the radio tranmit 
 *
 * Input:       Pointer to the NR_5G SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_start_tx_rssi(dev_object_t *dev, int param, int is_legacy) {
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    return (dev_swi_5g_at_run_cmd(obj_5g_swi, (is_legacy)?
                        RSSI_FR1_N79_LEGACY_TX_TEST : RSSI_FR1_N79_TX_TEST));
}

/******************************************************************************
 *
 * Name:        dev_swi_stop_tx_rssi
 *
 * Description: Function to stop the radio tranmit 
 *
 * Input:       Pointer to the NR_5G SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_swi_stop_tx_rssi(dev_object_t *dev, int param,int is_legacy)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;

    return (dev_swi_5g_at_run_cmd(obj_5g_swi, (is_legacy)? 
            RSSI_FR1_N79_STOP_LEGACY_TX_TEST : RSSI_FR1_N79_STOP_TX_TEST));
}

/******************************************************************************
 *
 * Name:        dev_get_custom_param
 *
 * Description: Function to get the custom config from the modem "at!custom?"
 *
 * Input:       Pointer to the NR_5G SWI device object , 
 *              param - which parameter to check
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_get_custom_param(dev_object_t *dev, int param)
{
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    return (dev_swi_5g_at_run_cmd(obj_5g_swi, MODEM_GET_CUSTOM_GPIOSAR_STATUS));
}

/******************************************************************************
 *
 * Name:        dev_config_custom_param
 *
 * Description: Function to add custum config 
 *
 * Input:       Pointer to the NR_5G SWI device object
 *              param - which parameter to config
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_config_custom_param(dev_object_t *dev, int param){
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    return (dev_swi_5g_at_run_cmd(obj_5g_swi, param));
}

/******************************************************************************
 *
 * Name:        dev_get_sarstate
 *
 * Description: Function to get the SAR status
 *
 * Input:       Pointer to the NR_5G SWI device object
 *              param - Check SAR backof or SAR value
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_get_sarstate(dev_object_t *dev, int param){
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    return (dev_swi_5g_at_run_cmd (obj_5g_swi, param));
}

/******************************************************************************
 *
 * Name:        dev_get_fsn_num
 *
 * Description: Function to get the FNS number to check modem hardware ver
 *
 * Input:       Pointer to the NR_5G SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_get_fsn_num (dev_object_t *dev) {
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    return (dev_swi_5g_at_run_cmd (obj_5g_swi,RSSI_NR_5G_FSN_TEST));
}

/******************************************************************************
 *
 * Name:        dev_get_mmwave_ant_present_status
 *
 * Description: Function to get the mmwave antenna present status 
 *
 * Input:       Pointer to the NR_5G SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_get_mmwave_ant_present_status (dev_object_t *dev) {
    dev_5g_swi_object_t *obj_5g_swi = (dev_5g_swi_object_t *) dev;
    return (dev_swi_5g_at_run_cmd (obj_5g_swi, MMWAVE_ANT_PRESENT_STATUS));
}

/*********************************************************************
 * $Log: dev_NR_5G_swi.c,v $
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
