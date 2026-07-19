/* $Id: dev_lte_swi.c,v 1.7 2020/02/19 03:11:30 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_lte_swi/dev_lte_swi.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_lte_swi.c
 *
 * Description:	LTE SWI Driver. Supports following modules:
 *              EM-7430, EM-7455, WP760X
 * Copyright (c) 2018-2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "defs.h"
#include "common.h"
#include "dev_lte_swi.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#ifdef LINUX_APP
#include <assert.h>
#endif

#include "dev_lte_swi.h"
#include "dev_lte_swi_at.h"

static uint32 dev_lte_swi_attach(dev_object_t *);
static uint32 dev_lte_swi_detach(dev_object_t *);
static uint32 dev_lte_swi_restart(dev_object_t *);
static void dev_lte_swi_destroy(dev_object_t **);
static int dev_lte_swi_modem_detection_test(dev_object_t *);
static int dev_lte_swi_modem_power_down(dev_object_t *);
static int dev_lte_sim_detect_test(dev_object_t *, int);
static int dev_lte_gps_ant_test(dev_object_t *);
static int dev_lte_swi_modem_rssi_test(dev_object_t *, int);
static int dev_lte_swi_modem_reset_test(dev_object_t *);
static int dev_lte_wp76xx_drive_gps_pin(dev_object_t *dev, int);
static int dev_lte_sim_detect_pin_present(dev_object_t *dev, int);
static int dev_lte_sim_detect_pin_status(dev_object_t *dev);
static int dev_toggle_wwan_led(dev_object_t *dev, int);
static int dev_lte_swi_modem_carrier_is_match(dev_object_t *dev);

void lte_swi_dev_create(dev_object_t *, dev_error_report_t);

/******************************************************************************
 *
 * Name:	lte_swi_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the LTE SWI device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void lte_swi_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t*)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in lte_swi_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    obj_lte_swi->base.dev_object_fvt->dev_attach	= dev_lte_swi_attach;
    obj_lte_swi->base.dev_object_fvt->dev_detach	= dev_lte_swi_detach;
    obj_lte_swi->base.dev_object_fvt->dev_restart	= dev_lte_swi_restart;
    obj_lte_swi->base.dev_object_fvt->dev_error_report	= error_report_fn;
    obj_lte_swi->base.dev_object_fvt->dev_destroy	= dev_lte_swi_destroy;
    obj_lte_swi->base.dev_object_fvt->dev_name	= "LTE SWI";

    obj_lte_swi->modem_type = SWI_EM74XX;

    obj_lte_swi->callin_fvt = (dev_lte_swi_callin_fvt_t *)
                               malloc(sizeof(dev_lte_swi_callin_fvt_t));
    obj_lte_swi->callout_fvt = (dev_lte_swi_callout_fvt_t *)
                                malloc(sizeof(dev_lte_swi_callout_fvt_t));

    obj_lte_swi->base.dev_state = DEV_STATE_CREATE;
}


/******************************************************************************
 *
 * Name:	dev_lte_swi_attach()
 *
 * Description:	Attach the LTE SWI device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_lte_swi_attach (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    if (obj_lte_swi->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_lte_swi_attach() callin malloc", DEV_LTE_SWI_ATTACH);
        return (FAILED);
    }

    if (obj_lte_swi->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_lte_swi_attach() callout malloc", DEV_LTE_SWI_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    obj_lte_swi->callin_fvt->modem_detection_test = dev_lte_swi_modem_detection_test;
    obj_lte_swi->callin_fvt->modem_reset_test = dev_lte_swi_modem_reset_test;
    obj_lte_swi->callin_fvt->modem_rssi_test= dev_lte_swi_modem_rssi_test;
    obj_lte_swi->callin_fvt->modem_gps_ant_test = dev_lte_gps_ant_test;
    obj_lte_swi->callin_fvt->sim_detect_test = dev_lte_sim_detect_test;
    obj_lte_swi->callin_fvt->wp76xx_drive_gps_pin = dev_lte_wp76xx_drive_gps_pin;
    obj_lte_swi->callin_fvt->modem_power_down = dev_lte_swi_modem_power_down;
    obj_lte_swi->callin_fvt->sim_detect_pin_present = dev_lte_sim_detect_pin_present;
    obj_lte_swi->callin_fvt->display_sim_detect_stat = dev_lte_sim_detect_pin_status;
    obj_lte_swi->callin_fvt->toggle_wwan_led = dev_toggle_wwan_led;
    obj_lte_swi->callin_fvt->modem_carrier_is_match = dev_lte_swi_modem_carrier_is_match;
    

    obj_lte_swi->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}


/******************************************************************************
 *
 * Name:	dev_lte_swi_detach()
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
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_lte_swi_detach (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, obj_lte_swi->base.dev_object_fvt);

    obj_lte_swi->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_lte_swi_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the LTE SWI device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_lte_swi_restart (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi= (dev_lte_swi_object_t *) dev;

    obj_lte_swi->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}



/******************************************************************************
 * Name:	dev_lte_swi_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the LTE SWI object
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_lte_swi_destroy (dev_object_t **dev)
{
    dev_lte_swi_object_t *obj_lte_swi;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    obj_lte_swi = (dev_lte_swi_object_t *)*dev;

    if (obj_lte_swi->callout_fvt) {
        free(obj_lte_swi->callout_fvt);	/* Free callout struct */
    }

    if (obj_lte_swi->callin_fvt) {
        free(obj_lte_swi->callin_fvt);		/* Free callin struct */
    }

    free(obj_lte_swi->base.dev_object_fvt);	/* Free dev_object_t */
}


/******************************************************************************
 *
 * Name:	dev_lte_swi_modem_detection_test
 *
 * Description:	Function to detect modem through AT command
 *
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_swi_modem_detection_test (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, RSSI_LTE_ATI_TEST));
}

/******************************************************************************
 *
 * Name:	dev_lte_swi_modem_power_down
 *
 * Description:	Function to power down modem through AT command
 *
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_swi_modem_power_down (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, LTE_PWR_DOWN));
}

/******************************************************************************
 *
 * Name:	dev_lte_swi_modem_reset_test
 *
 * Description:	Reset modem through AT command
 *
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_swi_modem_reset_test (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, RSSI_LTE_RESET_TEST));
}


/******************************************************************************
 *
 * Name:	dev_lte_swi_modem_rssi_test
 *
 * Description:	Main/Div RSSI Test, requires external equipment
 *
 * Input:	Pointer to the LTE SWI device object
 *          which_rssi - Main/Div RSSI
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_swi_modem_rssi_test (dev_object_t *dev, int which_rssi)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev; 
    int at_test_cmd;
    int rc = FAILED;

    switch (obj_lte_swi->modem_type) {
    case SWI_EM74XX:
    case SWI_WP760X_B8:
        if (which_rssi == MAIN_RSSI) {
            at_test_cmd = RSSI_LTE_B8_MAIN_TEST;
        } else {
            at_test_cmd = RSSI_LTE_B8_DIV_TEST;
        }
        printf("\nFreq = %s MHz, Power = %s dBm\n", RSSI_B8_FREQ, RSSI_AMP);
        break;
    case SWI_WP7601_03:
    case SWI_WP7610:
        if (which_rssi == MAIN_RSSI) {
            at_test_cmd = RSSI_LTE_B4_MAIN_TEST;
        } else {
            at_test_cmd = RSSI_LTE_B4_DIV_TEST;
        }
        printf("\nFreq = %s MHz, Power = %s dBm\n", RSSI_B4_FREQ, RSSI_AMP);
        break;
    case SWI_WP7607_08_09:
        if (which_rssi == MAIN_RSSI) {
            at_test_cmd = RSSI_LTE_B1_MAIN_TEST;
        } else {
            at_test_cmd = RSSI_LTE_B1_DIV_TEST;
        }
        printf("\nFreq = %s MHz, Power = %s dBm\n", RSSI_B1_FREQ, RSSI_AMP);
        break;
    default:
        return (FAILED);
        break;
    }
    
    rc = dev_lte_swi_at_run_cmd(obj_lte_swi, at_test_cmd);
    if (rc == FAILED){
        cterr('f', 0, "%s:\nPlease check whether modem is in Low Power Mode,\n"
              "you can go to 'f: AT Command Utility' in LTE Test Menu,\n"
              "use (AT!GSTATUS?) AT command to check status in mode column,\n"
              "correct status should be 'ONLINE' not 'LOW POWER MODE'.\n", __FUNCTION__);
        return (FAILED);
    }
    
    return (rc);
}


/******************************************************************************
 *
 * Name:	dev_lte_gps_ant_test
 *
 * Description:	GPS Antenna Test, requires external equipment
 *
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_gps_ant_test (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev; 

    printf("\nSignal Generator Freq = 1575.52 MHz, Power = -110dBm.\n");
    return (dev_lte_swi_at_run_cmd(obj_lte_swi, RSSI_LTE_GPS_TEST)); 
}


/******************************************************************************
 *
 * Name:	dev_lte_sim_detect_test
 *
 * Description:	SIM Detection Test (WP modem only supports 1 SIM)
 *
 * Input:	Pointer to the LTE SWI device object
 *          which_sim - SIM 0 or SIM 1
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_sim_detect_test (dev_object_t *dev, int which_sim)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev; 
    int at_test_cmd;

    if (which_sim == SIM_0) {
        at_test_cmd = LTE_SIM0_DETECT_TEST;
    } else {
        if (which_sim == SIM_1) {
            printf("%s: SWI_WP760X doesn't support SIM 1\n", __func__);
            return (FAILED);
        }

        at_test_cmd = LTE_SIM1_DETECT_TEST;
    }

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, at_test_cmd)); 
}


/******************************************************************************
 *
 * Name:	dev_lte_wp76xx_drive_gps_pin
 *
 * Description:	Drive GPS Pin (WP76XX ONLY)
 *
 * Input:	Pointer to the LTE SWI device object
 *          level - 0 for Low, 1 for High
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_wp76xx_drive_gps_pin (dev_object_t *dev, int level)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev; 
    int at_test_cmd;

    if (level == GPS_PIN_HIGH) {
        at_test_cmd = GPS_PIN_HIGH_WP76XX;
    } else {
        at_test_cmd = GPS_PIN_LOW_WP76XX;
    }

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, at_test_cmd));
}

/******************************************************************************
 *
 * Name:	dev_lte_sim_detect_pin_present
 *
 * Description:	LTE SIM Dectect PIN present
 *
 * Input:	Pointer to the LTE SWI device object
 *          
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_sim_detect_pin_present (dev_object_t *dev, int present)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;
    int at_test_cmd;

    if (present == SIM_DET_PIN_PRE) {
        at_test_cmd = SIM_DETECT_PIN_PRESENT; 
    } else {
        at_test_cmd = SIM_DETECT_PIN_NO_PRESENT;
    }

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, at_test_cmd));
}

/******************************************************************************
 *
 * Name:        dev_lte_sim_detect_pin_status
 *
 * Description: LTE SIM Detect PIN status
 *
 * Input:       Pointer to the LTE SWI device object
 *
 * Returns:     PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_sim_detect_pin_status (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, SIM_DETECT_PIN_STATUS));
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
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;
    int at_test_cmd;
    
    if (stat == WWAN_LED_ON) {
        at_test_cmd = WWAN_LED_TURN_ON; 
    } else {
        at_test_cmd = WWAN_LED_TURN_OFF;
    }

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, at_test_cmd));
}

/******************************************************************************
 *
 * Name:	dev_lte_swi_modem_carrier_is_match
 *
 * Description:	Function to detect whether modem carrier is matching or not
 *
 * Input:	Pointer to the LTE SWI device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static int dev_lte_swi_modem_carrier_is_match (dev_object_t *dev)
{
    dev_lte_swi_object_t *obj_lte_swi = (dev_lte_swi_object_t *) dev;

    return (dev_lte_swi_at_run_cmd(obj_lte_swi, LTE_CHK_IMG_CARRIER_MATCH));
}
/*------------------------------------------------------------------
$Log: dev_lte_swi.c,v $
Revision 1.7  2020/02/19 03:11:30  harrchan
Add LTE patch for matching modem carrier (CSCvt07550)

Revision 1.6  2019/11/21 00:01:20  alicehua
CSCvs04419: Print a clue message for users to check whether modem is in LPM
when LTE RSSI test failed.

Revision 1.5  2019/07/11 12:34:41  alicehua
Collapse Nutella codes into main trunk

Revision 1.4  2019/06/14 09:59:00  steja
Supported Cooper usb dongle LTE

Revision 1.3.16.1  2019/02/21 01:28:02  harrchan
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
