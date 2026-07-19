/* $Id: dev_ltc4215.c,v 1.4 2013/11/26 08:40:32 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_ltc4215/dev_ltc4215.c,v $
 *------------------------------------------------------------------------------
 *
 * Filename:	dev_ltc4215.c
 *
 * Description:	Device drive for the OIR chip on the SM adapter card
 *
 *
 * Copyright (c) 2008-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "types.h"
#include "defs.h"
#include "common.h"
#include "common_utils.h"
#include "dev_print.h"
#include "dev_object.h"
#include "dev_ltc4215.h"
#include "free.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32 dev_ltc4215_attach(dev_object_t *dev);
static uint32 dev_ltc4215_detach(dev_object_t *dev);
static uint32 dev_ltc4215_init(dev_object_t *dev);
static uint32 dev_ltc4215_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void dev_ltc4215_destroy(dev_object_t **);
static int dev_ltc4215_reg_read(dev_object_t *dev, uint8_t *buf, uint32_t offset);
static int dev_ltc4215_reg_write(dev_object_t *dev, uint8_t *buf, uint32_t offset);
static int dev_ltc4215_reg_test(dev_object_t *dev);
static int dev_ltc4215_pg_led(dev_object_t *dev, boolean en);
static int dev_ltc4215_gpio2_led(dev_object_t *dev, boolean en);
static int dev_ltc4215_pg_led_test(dev_object_t *dev);
static int dev_ltc4215_gpio2_led_test(dev_object_t *dev);
 
/*****************************************************************
 *
 * Name: dev_ltc4215_create()
 *
 * Description: Create object with various device function point to "do nothing"
 *
 * Input: dev_object_t pointer to the LTC4215.
 *	  error reporting function pointer.
 *
 * Returns: PASS/FAIL
 *
 *****************************************************************/
int
dev_ltc4215_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_ltc4215_object_t *ltc4215 = (dev_ltc4215_object_t *)dev;
    char msg[ERR_MSG_SZ];
    char *fn = "dev_ltc4215_create";

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		NULL) {
	/* Unable to allocate memory */
        sprintf(msg, "%s failed malloc for dev_object_fvt_t", fn);
	error_report_fn(dev, msg, (uint)FATAL);
	return(FAIL);
    }

    /* Init the device object structure to default "do nothing" */
    ltc4215->base.dev_state = DEV_STATE_CREATE;
    init_default_dev_object(dev, dev_fvt);

    ltc4215->base.dev_object_fvt->dev_attach = dev_ltc4215_attach;
    ltc4215->base.dev_object_fvt->dev_detach = dev_ltc4215_detach;
    ltc4215->base.dev_object_fvt->dev_init = dev_ltc4215_init;
    ltc4215->base.dev_object_fvt->dev_error_report = error_report_fn;
    ltc4215->base.dev_object_fvt->dev_collect_crashinfo = dev_ltc4215_crsh;
    ltc4215->base.dev_object_fvt->dev_destroy = dev_ltc4215_destroy;
    ltc4215->base.dev_object_fvt->dev_name = "OIR LTC4215 Device";

    ltc4215->callin_fvt = (dev_ltc4215_callin_fvt_t *)
				malloc(sizeof(dev_ltc4215_callin_fvt_t));
    if (ltc4215->callin_fvt == NULL) {
        sprintf(msg, "%s failed malloc for callin_fvt", fn);
	error_report_fn(dev, msg, (uint)FATAL);
	return(FAIL);
    }

    ltc4215->callout_fvt = (dev_ltc4215_callout_fvt_t *)
				malloc(sizeof(dev_ltc4215_callout_fvt_t));
    if (ltc4215->callout_fvt == NULL) {
        sprintf(msg, "%s failed malloc for callout_fvt", fn);
	error_report_fn(dev, msg, (uint)FATAL);
	return(FAIL);
    }
    return (PASS);
}

/*****************************************************************
 *
 * Name: dev_ltc4215_attach()
 *
 * Description: Attach the ltc4215 device for use.
 *
 * Input: Pointer to the cpld device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_ltc4215_attach (dev_object_t *dev)
{

    uint32 rc;
    dev_ltc4215_object_t *ltc4215 = (dev_ltc4215_object_t *) dev;

    if (ltc4215->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_ltc4215_attach() callin malloc",
			 DEV_LTC4215_ATTACH);
	return(FAILED);
    }

    if (ltc4215->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_ltc4215_attach() callout malloc", 
			 DEV_LTC4215_ATTACH);
	return(FAILED);
    }

    /* Lock the I2C device */
    if ((rc = ltc4215->callout_fvt->open(ltc4215->i2c_p)) != PASSED) {
        DEV_ERROR_REPORT(dev, "dev_ltc4215_attach() I2C open", rc);
        return(FAILED);
    }

    /* Connect the device specific function vector table */
    ltc4215->callin_fvt->reg_rd = dev_ltc4215_reg_read;
    ltc4215->callin_fvt->reg_wr = dev_ltc4215_reg_write;
    ltc4215->callin_fvt->reg_test = dev_ltc4215_reg_test;
    ltc4215->callin_fvt->pg_led = dev_ltc4215_pg_led;
    ltc4215->callin_fvt->gpio2_led = dev_ltc4215_gpio2_led;
    ltc4215->callin_fvt->pg_led_test = dev_ltc4215_pg_led_test;
    ltc4215->callin_fvt->gpio2_led_test = dev_ltc4215_gpio2_led_test;

    ltc4215->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name: dev_ltc4215_detach()
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
 * Input: Pointer to the cpld device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_ltc4215_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_ltc4215_object_t *ltc4215 = (dev_ltc4215_object_t *) dev;

    /* Unlock the I2C device */
    if ((rc = ltc4215->callout_fvt->close(ltc4215->i2c_p)) != PASSED) {
	DEV_ERROR_REPORT(dev, "dev_ltc4215_detach() I2C close", rc);
	return(FAILED);
    }

    ltc4215->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 *
 * Name: dev_ltc4215_init()
 *
 * Description: init i2c interface of ltc4215.
 *
 * Input: Pointer to the cpld device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_ltc4215_init (dev_object_t *dev)
{
    dev_ltc4215_object_t *ltc4215 = (dev_ltc4215_object_t *) dev;
    int rc = PASSED;

    rc = ltc4215->callout_fvt->init(ltc4215->i2c_p);
    if (rc != PASSED) {
	DEV_ERROR_REPORT(dev, "dev_ltc4215_init() I2C init", rc);
	return(FAILED);
    }

    return (PASSED);
}

/*****************************************************************
 * Name: dev_ltc4215_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input: dev_object_t pointer to the device
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
dev_ltc4215_crsh(dev_object_t *dev, print_fn_t dev_print, 
	dev_show_cmd verbosity)
{

    /* more development in this section */
    dev_print("dev_ltc4215_crsh(): No Crash info available for LTC4215\n");
    return(PASSED);
}

/*****************************************************************
 * Name: dev_ltc4215_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input: dev_object_t pointer to the device
 *
 * Returns: none
 *
 * Assumptions: The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_ltc4215_destroy(dev_object_t **dev)
{
    dev_ltc4215_object_t *ltc4215;

    if ((dev == NULL) || (*dev == NULL)) {
	return;
    }

    ltc4215 = (dev_ltc4215_object_t *)*dev;

    if (ltc4215->callin_fvt) {
	free((uint8_t *)ltc4215->callin_fvt);
    }

    if (ltc4215->callout_fvt) {
	free((uint8_t *)ltc4215->callout_fvt);
    }

    if (ltc4215->base.dev_object_fvt) {
        free(ltc4215->base.dev_object_fvt);
    }
}

/**********************************************************************
 *
 * Function: dev_ltc4215_reg_read
 *
 * Description:	Read the LTC4215 reg with standard I2C operation.
 *
 * Input : dev_object_t pointer to the device
 *         d_buf - pointer to the data buffer
 *         offset - reg addr offset
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_reg_read(dev_object_t *dev, uint8_t *buf, uint32_t offset)
{
    n2g_i2c_if_t i2c_if;
    dev_ltc4215_object_t *ltc4215 = (dev_ltc4215_object_t *)dev;
    dev_ltc4215_callout_fvt_t *callout_p = ltc4215->callout_fvt;

    /* Setup the interface struct for I2C API read */
    memcpy(&i2c_if, ltc4215->i2c_p, sizeof(n2g_i2c_if_t));
    i2c_if.size = sizeof(uint8_t);
    i2c_if.buf = (char *)buf;
    i2c_if.offset = offset;

    return((*callout_p->rd)(&i2c_if));
}

/**********************************************************************
 *
 * Function: dev_ltc4215_reg_write
 *
 * Description:	Write the LTC4215 with standard I2C operation.
 *
 * Input : dev_object_t pointer to the device
 *         d_buf - pointer to the data buffer
 *         offset - reg addr offset
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_reg_write(dev_object_t *dev, uint8_t *buf, uint32_t offset)
{
    n2g_i2c_if_t i2c_if;
    dev_ltc4215_object_t *ltc4215 = (dev_ltc4215_object_t *)dev;
    dev_ltc4215_callout_fvt_t *callout_p = ltc4215->callout_fvt;

    /* Setup the interface struct for I2C API write */
    memcpy(&i2c_if, ltc4215->i2c_p, sizeof(n2g_i2c_if_t));
    i2c_if.size = sizeof(uint8_t);
    i2c_if.buf = (char *)buf;
    i2c_if.offset = offset;

    return((*callout_p->wr)(&i2c_if));
}

/**********************************************************************
 *
 * Function: dev_ltc4215_reg_test
 *
 * Description:	1. Read reg 0x5 to check the voltage is between 9V ~ 14V.
 *                         2. Toggle GPIO1 bit and read status reg to check the status of 
 *                             GPIO1 is changed or not.
 *
 * Input : dev_object_t pointer to the device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_reg_test(dev_object_t *dev)
{
    uint8_t temp, data;
    uint32_t rc;

    rc = dev_ltc4215_reg_read(dev, &temp, LTC4215_SOURCE_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "source reg read failed", (uint)FATAL);
        return (FAILED);
    }

    if ((temp < MIN_SOURCE_VOLTAGE) || (temp > MAX_SOURCE_VOLTAGE)) {
        DEV_ERROR_REPORT(dev, "Source voltage is out of range (9V ~ 14V)", 
			(uint)FATAL);
        return (FAILED);
    }
	
    temp = 0;
    rc = dev_ltc4215_reg_write(dev, &temp, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    rc = dev_ltc4215_reg_read(dev, &data, LTC4215_STATUS_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "status reg read failed", (uint)FATAL);
        return (FAILED);
    }

    if (data & LTC4215_GPIO1_INPUT_MASK) {
        DEV_ERROR_REPORT(dev, "GPIO1 status is not low", (uint)FATAL);
        return (FAILED);
    }
	
    temp = LTC4215_GPIO1_INPUT_MASK;
    rc = dev_ltc4215_reg_write(dev, &temp, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    rc = dev_ltc4215_reg_read(dev, &data, LTC4215_STATUS_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "status reg read failed", (uint)FATAL);
        return (FAILED);
    }

    if (!(data & LTC4215_GPIO1_INPUT_MASK)) {
        DEV_ERROR_REPORT(dev, "GPIO1 status is not high", (uint)FATAL);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_ltc4215_pg_led_test
 *
 * Description: Power Good Led test.
 *
 * Input : dev_object_t pointer to the device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_pg_led_test(dev_object_t *dev)
{
    uint8_t ctrl_reg = 0, data;
    int rc = PASSED;

    rc = dev_ltc4215_reg_read(dev, &ctrl_reg, LTC4215_CONTROL_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "source reg read failed", (uint)FATAL);
        return (FAILED);
    }

    data = 0x5b;
    rc = dev_ltc4215_reg_write(dev, &data, LTC4215_CONTROL_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    data = 0x0;
	rc = dev_ltc4215_reg_read(dev, &data, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "source reg read failed", (uint)FATAL);
        return (FAILED);
    }

    data &= ~LTC4215_GPIO1_OUTPUT_MASK;
    rc = dev_ltc4215_reg_write(dev, &data, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    msleep(1000);

    data |= LTC4215_GPIO1_OUTPUT_MASK;
    rc = dev_ltc4215_reg_write(dev, &data, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    rc = dev_ltc4215_reg_write(dev, &ctrl_reg, LTC4215_CONTROL_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_ltc4215_pg_led
 *
 * Description: Power Good Led.
 *
 * Input : dev_object_t pointer to the device
 *            enable/disable led
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_pg_led(dev_object_t *dev, boolean en)
{
    uint8_t data;
    int rc = PASSED;

    data = 0x0;
	rc = dev_ltc4215_reg_read(dev, &data, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "source reg read failed", (uint)FATAL);
        return (FAILED);
    }

    if (en) {
        data &= ~LTC4215_GPIO1_OUTPUT_MASK;
    } else {
        data |= LTC4215_GPIO1_OUTPUT_MASK;
    }

    rc = dev_ltc4215_reg_write(dev, &data, LTC4215_ALERT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_ltc4215_gpio2_led
 *
 * Description: GPIO2 Led.
 *
 * Input : dev_object_t pointer to the device
 *            enable/disable led
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_gpio2_led(dev_object_t *dev, boolean en)
{
    uint8_t data;
    int rc = PASSED;

    data = 0x0;
	rc = dev_ltc4215_reg_read(dev, &data, LTC4215_FAULT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "source reg read failed", (uint)FATAL);
        return (FAILED);
    }

    if (en) {
        data |= LTC4215_GPIO2_OUTPUT_MASK;
    } else {
        data &= ~LTC4215_GPIO2_OUTPUT_MASK;
    }

	rc = dev_ltc4215_reg_write(dev, &data, LTC4215_FAULT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: dev_ltc4215_gpio2_led_test
 *
 * Description: GPIO2 Led test.
 *
 * Input : dev_object_t pointer to the device
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_ltc4215_gpio2_led_test(dev_object_t *dev)
{
    uint8_t data;
    int rc = PASSED;

    data = 0x0;
	rc = dev_ltc4215_reg_read(dev, &data, LTC4215_FAULT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "source reg read failed", (uint)FATAL);
        return (FAILED);
    }

    data |= LTC4215_GPIO2_OUTPUT_MASK;
    rc = dev_ltc4215_reg_write(dev, &data, LTC4215_FAULT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    msleep(1000);

    data &= ~LTC4215_GPIO2_OUTPUT_MASK;
    rc = dev_ltc4215_reg_write(dev, &data, LTC4215_FAULT_REG);
    if (rc) {
        DEV_ERROR_REPORT(dev, "alert reg write failed", (uint)FATAL);
        return (FAILED);
    }

    return (PASSED);
}

/******** History ******** 
$Log: dev_ltc4215.c,v $
Revision 1.4  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.3  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:58:09  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
