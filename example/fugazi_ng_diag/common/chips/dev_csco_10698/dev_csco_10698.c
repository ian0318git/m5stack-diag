/* $Id: dev_csco_10698.c,v 1.3 2012/03/28 00:38:07 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_csco_10698/dev_csco_10698.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_csco_10698.c
 *
 * Description:	Cisco part number 15-10698-01 (Renesas R5F21262SNFP) is used by
 *		the Xformers Environment MCU and the Xformers Power Sequencer.
 *		This file provides the I2C Common device driver functions used
 *		by both devices..
 *
 *		Neither the Environmental Control Unit nor the Power Sequencer
 *		generate interrupt; therefore, dev_intr_enable,
 *		dev_intr_disable, and dev_isr are not implemented.
 *
 *		The Power Sequencer has a GFYM_HRESET_OUT_L (input to Pwr Seq)
 *		pin. This is the software reset signal. However,
 *		the Environmental Control Unit has a ENV_MCU_RST_L (Debug
 *		reset signal). But there is no mechanism to cause this pin
 *		to be asserted; therefore, dev_oper_enable, and dev_oper_disable
 *		are not implemented. The power sequencer can provide its reset
 *		in its platform specific file.
 *
 *		Only dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_init, dev_show, dev_err_report, dev_collect_crashinfo,
 *		dev_destroy are implemented.
 *
 *		CSCO p/n 15-10698 access involves offset, and data.
 *		Refer to EDCS-534569 for more info.
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include <assert.h>
#include "endians.h"
#include "defs.h"
#include "common.h"
#include "dev_csco_10698.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "byteswap.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32   dev_10698_attach(dev_object_t *);
static uint32	dev_10698_detach(dev_object_t *);
static uint32	dev_10698_reconfig(dev_object_t *, void *, boolean *);
static uint32	dev_10698_restart(dev_object_t *);
static uint32	dev_10698_init(dev_object_t *);
static uint32	dev_10698_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	dev_10698_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dev_10698_destroy(dev_object_t **);

/*****************************************************************
 *
 * Name: csco_10698_dev_create()
 *
 * Description: Create object with various device function
 * point to "do nothing"
 *
 * Input: dev_object_t pointer to the CSCO 10698 device.
 *	  error reporting function pointer.
 *
 * Returns: none
 *
 *****************************************************************/
void
csco_10698_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_ren_object_t *ren = (dev_ren_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in csco_10698_dev_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    ren->base.dev_object_fvt->dev_attach	= dev_10698_attach;
    ren->base.dev_object_fvt->dev_detach	= dev_10698_detach;
    ren->base.dev_object_fvt->dev_reconfig_needed = dev_10698_reconfig;
    ren->base.dev_object_fvt->dev_restart	= dev_10698_restart;
    ren->base.dev_object_fvt->dev_init		= dev_10698_init;
    ren->base.dev_object_fvt->dev_show		= dev_10698_show;
    ren->base.dev_object_fvt->dev_error_report	= error_report_fn;
    ren->base.dev_object_fvt->dev_collect_crashinfo = dev_10698_crsh;
    ren->base.dev_object_fvt->dev_destroy	= dev_10698_destroy;
    ren->base.dev_object_fvt->dev_name	= "Environmental Control Unit";

    ren->callin_fvt = (ren_callin_fvt_t *)
				malloc(sizeof(ren_callin_fvt_t));
    ren->callout_fvt = (ren_callout_fvt_t *)
				malloc(sizeof(ren_callout_fvt_t));

    ren->base.dev_state = DEV_STATE_CREATE;

}
/*****************************************************************
 *
 * Name: dev_10698_attach()
 *
 * Description: Attach the CSCO 10698 device for use. This
 *   function will initialize and setup all necessary pointers
 *   and bring the chip to operation.
 *
 * Input: Pointer to the CSCO 10698 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_10698_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_ren_object_t *ren = (dev_ren_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    if (ren->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_10698_attach() callin malloc", 
			 REN_ATTACH);
	return(FAILED);
    }

    if (ren->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_10698_attach() callout malloc", 
			 REN_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
/*    ren->callin_fvt->register_test = csco_10698_reg_test; */

    /* Lock the I2C device */
    if ((rc = ren->callout_fvt->open(ren->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_10698_attach() I2C open return rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, REN_ATTACH);
        return(FAILED);
    }

    ren->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: dev_10698_detach()
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
 * Input: Pointer to the CSCO 10698 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_10698_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_ren_object_t *ren = (dev_ren_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = ren->callout_fvt->close(ren->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_10698_detach() I2C close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, REN_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, ren->base.dev_object_fvt);

    ren->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name: dev_10698_reconfig_needed
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
 * Input: dev_object_t pointer to the CSCO 10698 device
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
dev_10698_reconfig(dev_object_t *dev, void *context_handle,
					 boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name: dev_10698_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *   
 * Input: dev_object_t pointer to the FEMTOCLOCk DEVICE
 *
 * Returns: PASSED/FAILED
 *
 * Assumptions: The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *****************************************************************/
static uint32
dev_10698_restart(dev_object_t *dev)
{
    dev_ren_object_t *ren = (dev_ren_object_t *) dev;

    ren->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 *
 * Name: dev_10698_init()
 *
 * Description: Initializes the CSCO 10698 chip 
 *              
 *
 * Input: dev_object_t pointer to the CSCO 10698 device.
 *	  Caller has to setup the i2c_p parameters with init values.
 *
 * Returns: PASSED/FAILED
 *
 * Note: Make sure base.dev_addr has been initialized to chip_base_addr
 *       before calling this function.
 *
 *****************************************************************/
static uint32
dev_10698_init (dev_object_t *dev)
{
    uint32 rc;
    dev_ren_object_t *ren = (dev_ren_object_t *)dev;
    ren_callout_fvt_t *callout_p = ren->callout_fvt;
    n2g_i2c_if_t new_i2c_if;
    dev_ren_reg_init_t *init_p;
    ren_t wr_data;
    char err_buf[ERR_BUF_SIZE];

    /* Initialize the new I2C API interface struct */
    new_i2c_if.size = sizeof(ren_t);
    new_i2c_if.i2c_bus_type = ren->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = ren->i2c_p->i2c_dev;

    init_p = ren->init_p;	/* points to the first init table entry */

    /* First register at offset 0 is a Read Only register, and cannot be
     * initialized (or written to). We use it as the stopper of the init
     * table. If this is not true, then we need another mechanism to stop
     * the init table parsing.
     */
    while (init_p->offset) {
	new_i2c_if.buf = (char *)&wr_data;
	new_i2c_if.offset = init_p->offset;
	wr_data = DSWAP2(init_p->data);

#ifdef MCU_DEBUG
	printf("writing %#x to offset %#x\n", init_p->data, init_p->offset);
#endif /* MCU_DEBUG */

	/* Write to the CSCO 10698 with new init value */
	rc = callout_p->wr(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "dev_10698_init() write %#x to addr %#x rc %#x\n",
				init_p->data, init_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, REN_INIT);
	    return(FAILED);
	}
	init_p++;
    }

    ren->base.dev_state = DEV_STATE_INIT;

    return(PASSED);

}

/*****************************************************************
 * Name: dev_10698_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input: dev_object_t pointer to the CSCO 10698 device
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
dev_10698_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 rc;
    ren_t data;	/* data bytes from CSCO 10698 */
    int temp;
    reg_info_t *reg_p;
    n2g_i2c_if_t new_i2c_if;
    dev_ren_object_t *ren = (dev_ren_object_t *)dev;
    ren_callout_fvt_t *callout_p = ren->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = (char *)&data;
    new_i2c_if.i2c_bus_type = ren->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = ren->i2c_p->i2c_dev;

    dev_print("\n	%s Registers:\n", ren->dev_name);

    reg_p = ren->reg_p;	/* Points to the beginning of the registers table */

    while (reg_p->size.size) {
	/* Read the data bytes of CSCO 10698 */
	/* Since neither Environmental control unit, nor Power Sequencer can
	 * manage ReStart on the I2C. A write of the offset, then a read is
	 * required. For the write, use the offset with the same I2C struct.
	 * For this device, the write can use the offset. But for read, the
	 * offset is bypassed.
	 */
	new_i2c_if.buf = (char *)&data;
	new_i2c_if.offset = reg_p->offset;
	new_i2c_if.size = 0;

	rc = (*callout_p->wr)(&new_i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "dev_10698_show() write %s offset failed rc = %#x",
						     reg_p->name, rc);
	    DEV_ERROR_REPORT(dev, err_buf, REN_SHOW);
	    return(FAILED);
	}

#ifdef I2C_DEBUG
	printf("dev_10698_show(): wrote %#x offset\n", offset);
#endif /* I2C_DEBUG */

	new_i2c_if.size = sizeof(data);
	rc = (*callout_p->rd)(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "dev_10698_show() read %s @ %#x rc = %#x",
						reg_p->name, reg_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, REN_SHOW);
	    return(FAILED);
	}

	data = DSWAP2(data);

#ifdef I2C_DEBUG
	printf("data = %#x @ %#x\n", data, &data);
#endif /* I2C_DEBUG */

	switch (cmd) {
	case DEV_SHOW_ALL:
	case DEV_SHOW_CONFIG:
	case DEV_SHOW_REGISTERS:
	    /* Print the read register */
	    dev_print("%s @ offset %#x = ", reg_p->name, reg_p->offset);
	    if (reg_p->type & REG_DEV) {
		/* Print decimal and hex */
		temp = (int)data;
		dev_print("%d (0x%04X)\n", temp, data);
	    } else {
		dev_print("0x%04X\n", data);
	    }
	    break;
	case DEV_SHOW_BRIEF:
	    dev_print("@ %#x = 0x%04X ", reg_p->offset, data);
	    break;
	default:
	    assert(!"dev_10698_show");
	    break;
	} /* endof switch */
	reg_p++;
#ifdef LINUX
	udelay(N2G_I2C_BIT_DELAY);	/* Wait for stop bit */
#else /* Diagmon */
/*	wastetime(N2G_I2C_BIT_DELAY);	* Wait for stop bit */
/*	msleep(1); * */
#endif /* LINUX */
    } /* endof for */

    return(PASSED);
}

/*****************************************************************
 * Name: dev_10698_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the CSCO 10698 device
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
dev_10698_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{
    dev_ren_object_t *ren = (dev_ren_object_t *)dev;

    /* more development in this section */
    dev_print("dev_10698_crsh(): No Crash info available for %s\n",
						ren->dev_name);
    return(PASSED);
}

/*****************************************************************
 * Name: dev_10698_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the CSCO 10698 device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_10698_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_ren_object_t *ren;
    char err_buf[80];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    ren = (dev_ren_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = ren->callout_fvt->close(ren->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev__10698_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, REN_DESTROY);
	return;
    }

    if (ren->callout_fvt) {
	free(ren->callout_fvt);	/* Free callout struct */
    }

    if (ren->callin_fvt) {
	free(ren->callin_fvt);	/* Free callin struct */
    }

    free(ren->base.dev_object_fvt);	/* Free dev_object_t */
}

/******** History ******** 
$Log: dev_csco_10698.c,v $
Revision 1.3  2012/03/28 00:38:07  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:57:41  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
