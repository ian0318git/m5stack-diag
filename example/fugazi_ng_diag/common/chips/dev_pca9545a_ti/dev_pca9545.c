/* $Id: dev_pca9545.c,v 1.4 2013/11/26 08:40:32 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pca9545a_ti/dev_pca9545.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_pca9545.c
 *
 * Description:	TI PCA9545A (4-Channel I2C and SMBus Switch With Interrupt
 *		Logic and Reset Functions).
 *		This file provides the I2C Common device driver functions used
 *		by this device.
 *
 *		This device does not generate interrupt, even though it provides
 *		pass through interrupt pins from other devices; therefore,
 *		dev_intr_enable,  dev_intr_disable, and dev_isr are
 *		not implemented.
 *
 *		dev_init is used to disable or enable channel(s).
 *
 *		dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_show, dev_err_report, dev_collect_crashinfo, dev_destroy
 *		are also implemented.
 *
 *		Refer to Vendor datasheet for more info.
 *
 * Copyright (c) 2008-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include "endians.h"
#include "defs.h"
#include "common.h"
#include "dev_pca9545.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   dev_pca_attach(dev_object_t *dev);
static uint32	dev_pca_detach(dev_object_t *dev);
static uint32	dev_pca_reconfig(dev_object_t *, void *, boolean *);
static uint32	dev_pca_restart(dev_object_t *);
static uint32	dev_pca_init(dev_object_t *);
static uint32	dev_pca_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	dev_pca_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dev_pca_destroy(dev_object_t **);
static int	pca_alter_control(dev_object_t *, print_fn_t);
static int      dev_pca_test(dev_object_t *);

/*****************************************************************
 *
 * Name: dev_pca9545_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the PCA9545 device.
 *	  error reporting function pointer.
 *
 * Returns: none
 *
 *****************************************************************/
void
dev_pca9545_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_pca_object_t *ppca = (dev_pca_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in dev_pca9545_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    ppca->base.dev_object_fvt->dev_attach	= dev_pca_attach;
    ppca->base.dev_object_fvt->dev_detach	= dev_pca_detach;
    ppca->base.dev_object_fvt->dev_reconfig_needed = dev_pca_reconfig;
    ppca->base.dev_object_fvt->dev_restart	= dev_pca_restart;
    ppca->base.dev_object_fvt->dev_init		= dev_pca_init;
    ppca->base.dev_object_fvt->dev_show		= dev_pca_show;
    ppca->base.dev_object_fvt->dev_error_report	= error_report_fn;
    ppca->base.dev_object_fvt->dev_collect_crashinfo = dev_pca_crsh;
    ppca->base.dev_object_fvt->dev_destroy	= dev_pca_destroy;
    ppca->base.dev_object_fvt->dev_name	= "TI PCA9545A";

    ppca->callin_fvt = (pca_callin_fvt_t *)
				malloc(sizeof(pca_callin_fvt_t));
    ppca->callout_fvt = (pca_callout_fvt_t *)
				malloc(sizeof(pca_callout_fvt_t));

    ppca->base.dev_state = DEV_STATE_CREATE;

}
/*****************************************************************
 *
 * Name: dev_pca_attach()
 *
 * Description: Attach the PCA9545A device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the PCA9545A device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_pca_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_pca_object_t *ppca = (dev_pca_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    if (ppca->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_pca_attach() callin malloc", 
			 PCA_ATTACH);
	return(FAILED);
    }

    if (ppca->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_pca_attach() callout malloc", 
			 PCA_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    ppca->callin_fvt->peek_n_poke = pca_alter_control;
    ppca->callin_fvt->pca_test = dev_pca_test;

    /* Lock the I2C device */
    if ((rc = ppca->callout_fvt->open(ppca->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_pca_attach() I2C open return rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, PCA_ATTACH);
        return(FAILED);
    }

    ppca->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: dev_pca_detach()
 *
 * Description: detach the device specific functions from the caller.
 *		All of the device specific function are connected to the
 *		dev_do_nothing() function, except for the dev_attach()
 *		function. Also, the dev_state must be assigned the value
 *		of DEV_STATE_DETACH.
 *
 *		Since, some platforms may want to detach the device, but not
 *		release the memory resources (via a free () in the
 *		dev_destroy()), this function can be executed to accomplish
 *		this task. However, before a detached device can be used again,
 *		it must be re-attached (via the dev_attach()).
 *
 * Input: Pointer to the PCA9545A device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_pca_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_pca_object_t *ppca = (dev_pca_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = ppca->callout_fvt->close(ppca->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_pca_detach() I2C close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, PCA_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, ppca->base.dev_object_fvt);

    ppca->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: dev_pca_reconfig_needed
 *
 * Description: To check whether device re-configuration is needed during
 *		(re)initialization. Based on the provided context information,
 *		the boolean return value, and possibly other factors external
 *		to the device object, the caller shall decide whether to invoke
 *		either dev_restart or dev_init, but not both. In general, the
 *		boolean return value alone is not sufficient to decide whether
 *		the device can safely be restarted or whether it must be fully
 *		initialized from scratch.
 *
 * Input: dev_object_t pointer to the PCA9545A device
 *	  void * - a device/platform specific context handle
 *	  boolean * - a pointer to a boolean
 *
 * Returns: PASSED/FAILED, context information and a boolean value.
 *	    The boolean value shall be set to TRUE if the device must be
 *	    reconfigured from scratch and it shall be set to FALSE otherwise.
 *
 * Assumptions: The dev_attach() function has been called and successfully
 *****************************************************************/
static uint32
dev_pca_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: dev_pca_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *   
 * Input: dev_object_t pointer to the PCA9545A device
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *****************************************************************/
static uint32
dev_pca_restart(dev_object_t *dev)
{
    dev_pca_object_t *ppca = (dev_pca_object_t *) dev;

    ppca->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_pca_init
 *
 * Description:	Initilialize PCA9545A control register.
 *
 * Input:	dev_object_t pointer to the PCA9545A device
 *		Control registeControl register to be initialized to is passed in the object
 *		struct
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_pca_init(dev_object_t *dev)
{
    uint32 rc;
    n2g_i2c_if_t new_i2c_if;
    dev_pca_object_t *ppca = (dev_pca_object_t *)dev;
    pca_callout_fvt_t *callout_p = ppca->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    pca_t ctrl;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(ctrl);
    new_i2c_if.buf = (char *)&ctrl;
    new_i2c_if.i2c_bus_type = ppca->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = ppca->i2c_p->i2c_dev;

    ctrl = ppca->init;
    rc = callout_p->wr(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "dev_pca_init: Write failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, PCA_INIT);
	return(FAILED);
    }

    return(PASSED);
}

/*****************************************************************
 * Name: dev_pca_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the PCA9545A device
 *	  A device print function vector
 *	  A dev_show_cmd_e command
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *****************************************************************/
static uint32
dev_pca_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 rc;
    pca_t ctrl;	/* Control register from PCA9545A */
    n2g_i2c_if_t new_i2c_if;
    dev_pca_object_t *ppca = (dev_pca_object_t *)dev;
    pca_callout_fvt_t *callout_p = ppca->callout_fvt;
    dev_pca_desc_t *pdesc;
    char err_buf[ERR_BUF_SIZE];

    /* Setup I2C API interface struct */
    new_i2c_if.i2c_bus_type = ppca->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = ppca->i2c_p->i2c_dev;
    new_i2c_if.buf = (char *)&ctrl;
    new_i2c_if.size = sizeof(ctrl);

    /* Read the control register of PCA9545A */
    rc = (*callout_p->rd)(&new_i2c_if);

    if (rc != PASSED) {
	sprintf(err_buf, "dev_pca_show() read failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, PCA_SHOW);
	return(FAILED);
    }

    dev_print("\n   %s Control register = %02x:\n", ppca->dev_name, ctrl);

    pdesc = ppca->desc_p;

    while(pdesc && pdesc->name) {
	dev_print(" %s - ", pdesc->name);
	if (ctrl & pdesc->mask) {
	    dev_print("%s\n", pdesc->true);
	} else {
	    dev_print("%s\n", pdesc->false);
	}

	pdesc++;	/* points to the next field */
    } /* endof while */

    return(PASSED);
}

/*****************************************************************
 * Name: dev_pca_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the PCA9545A device
 *        A crash print function vector.
 *        A verbosity level.
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: A device print function vector has been provided by the host
 *		platform which implements the crash logging functionality. It
 *		could be the mechanism to log info to the Compact Flash before
 *		the device crash and now retrieve them. The dev_attch()
 *		function has been called and successfully executed.
 *
 *****************************************************************/
static uint32
dev_pca_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{
    dev_pca_object_t *ppca = (dev_pca_object_t *)dev;

    /* more development in this section */
    dev_print("dev_pca_crsh(): No Crash info available for %s\n",
						ppca->dev_name);
    return(PASSED);
}

/*****************************************************************
 * Name: dev_pca_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the PCA9545A device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_pca_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_pca_object_t *ppca;
    char err_buf[ERR_BUF_SIZE];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    ppca = (dev_pca_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = ppca->callout_fvt->close(ppca->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_pca_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, PCA_DESTROY);
	return;
    }

    if (ppca->callout_fvt) {
	free(ppca->callout_fvt);	/* Free callout struct */
    }

    if (ppca->callin_fvt) {
	free(ppca->callin_fvt);	/* Free callin struct */
    }

    free(ppca->base.dev_object_fvt);	/* Free dev_object_t */
}

/********************************************************************
 *
 * Function:	pca_alter_control
 *
 * Description:	Peek-n-poke PCA9545A control register.
 *
 * Inputs:	dev_object_t pointer to the PCA9545A device
 *		A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		also be called after the exit.
 *
 *********************************************************************
 */
static int
pca_alter_control(dev_object_t *dev, print_fn_t dev_print)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    dev_pca_object_t *ppca = (dev_pca_object_t *)dev;
    pca_callout_fvt_t *callout_p = (pca_callout_fvt_t *)ppca->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    pca_t old_ctrl, new_ctrl;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(pca_t);
    new_i2c_if.i2c_bus_type = ppca->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = ppca->i2c_p->i2c_dev;

    /* Read the register first. */
    new_i2c_if.buf = (char *)&old_ctrl;

    rc = (*callout_p->rd)(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "pca_alter_control() read failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, PCA_ALTER);
	return(FAILED);
    } /* endof if rc */

    new_ctrl = gethex_answer("Enter the new control register", old_ctrl,
					PCA9545_CTRL_MIN, PCA9545_CTRL_MAX);

    /* Write the new data */
    new_i2c_if.buf = (char *)&new_ctrl;

    rc = (*callout_p->wr)(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "at_alter_eeprom() write failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, PCA_ALTER);
	return(FAILED);
    } /* endof if rc */

    return(PASSED);
}

/*****************************************************************
 * Name:	dev_pca_test
 *
 * Description:	Test PCA9545A control register.
 *
 * Input:	dev_object_t pointer to the PCA9545A device
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static int
dev_pca_test(dev_object_t *dev)
{
    uint32 rc;
    n2g_i2c_if_t new_i2c_if;
    dev_pca_object_t *ppca = (dev_pca_object_t *)dev;
    pca_callout_fvt_t *callout_p = ppca->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    pca_t ctrl;
    uint8_t chan;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(ctrl);
    new_i2c_if.buf = (char *)&ctrl;
    new_i2c_if.i2c_bus_type = ppca->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = ppca->i2c_p->i2c_dev;

    for (chan=PCA9545_B0; chan <= PCA9545_B3; chan=chan<<1) {
        ctrl = chan;
	rc = callout_p->wr(&new_i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "dev_pca_test: Write failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, PCA_TEST);
	    return(FAILED);
	}

	ctrl = 0; // clear the buffer before read
	rc = callout_p->rd(&new_i2c_if);
	if (rc == PASSED) {
	    if (ctrl != chan)  {
	        sprintf(err_buf, "dev_pca_test: Read data mismatch. Exp %#x Act %#x", chan, ctrl);
		DEV_ERROR_REPORT(dev, err_buf, PCA_TEST);
		return(FAILED);
	    }
	}
	else {
	    sprintf(err_buf, "dev_pca_test: Read failed. rc = %#x", rc);
	    DEV_ERROR_REPORT(dev, err_buf, PCA_TEST);
	    return(FAILED);
	}
    }

    return(PASSED);
}

/******** History ******** 
$Log: dev_pca9545.c,v $
Revision 1.4  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.3  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:14  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
