/* $Id: dev_1337.c,v 1.7 2017/07/14 02:51:38 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_1337_maxim/dev_1337.c,v $
 ***********************************************************************
 * File Name:	dev_1337.c
 *
 * Description:	Library support routines to access Dallas/Maxim 1337 
 *		Real Time Clock.
 *
 *		Ported from Steelers with modifications.
 *
 *		DS1337 has 2 interrupt pins (INTA, SQW/INTB). But Xformers
 *		and ISRs are not using them; therefore, dev_intr_enable,
 *		dev_intr_disable, dev_isr are not used.
 *
 *		EOSC in the Control register can be used to enable or disable
 *		the oscillator; therefore, dev_oper_enable and dev_oper_disable
 *		will clear or set EOSC bit.
 *
 *		dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_init, dev_show, dev_err_report, dev_collect_crashinfo, and
 *		dev_destroy are also implemented.
 *
 * Copyright (c)2007-2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ***********************************************************************
 */
#include <stdlib.h>
#include <assert.h>
#include "types.h"
#include "endians.h"
#include "common.h"
#include "defs.h"
#include "dev_1337.h"
#include "dev_print.h"
#include "free.h"
#include "queryflags.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32	dev_1337_attach(dev_object_t *);
static uint32	dev_1337_detach(dev_object_t *);
static uint32	dev_1337_reconfig(dev_object_t *, void *, boolean *);
static uint32	dev_1337_restart(dev_object_t *);
static uint32	dev_1337_init(dev_object_t *);
/*static uint32	dev_1337_oper_en(dev_object_t *);
static uint32	dev_1337_oper_dis(dev_object_t *); */
static uint32	dev_1337_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	dev_1337_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dev_1337_destroy(dev_object_t **);

static int	ds1337_alter_reg(dev_object_t *, print_fn_t);
static int	rtc_read_time(dev_object_t *, ds1337_rtc_data_t *);
static int	rtc_set_time(dev_object_t *, ds1337_rtc_data_t *);
static void	convert_timeval_to_rtc_data(ds1337_rtc_data_t *,
					    ds1337_time_t *);
static void	convert_rtc_data_to_timeval(ds1337_rtc_data_t *,
					    ds1337_time_t *);
static uint32	ds1337_mux_clk_to_dout(dev_object_t *, uchar);
static uint32	ds1337_display_rtc(dev_object_t *);
static uint32	ds1337_set_rtc(dev_object_t *);
static uint32	ds1337_register_test(dev_object_t *);
static uint32	ds1337_time_validity_test(dev_object_t *);

static int	dev_i2c_rd(ulong, int, ulong *, void *);
static int	dev_i2c_wr(ulong, int, ulong, void *);

extern void udelay(unsigned long t);

/* Registers test table */
static reg_info_t_ext reg_ext = {
			sizeof(rtc_t), dev_i2c_rd, dev_i2c_wr, 0};

static reg_info_t ds1337_reg_tbl[] = {
    {"Seconds",		DS1337_SECONDS_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x7F, 0x00 },
    {"Minutes",		DS1337_MINUTES_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x7F, 0x00 },
    {"Hours",		DS1337_HOURS_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x7F, 0x00 },
    {"Day",		DS1337_DAY_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x07, 0x00 },
    {"Date",		DS1337_DATE_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x3F, 0x00 },
    {"Month/Century",	DS1337_MONTH_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x9F, 0x00 },
    {"Year",		DS1337_YEAR_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 1 Seconds",	DS1337_A1_SEC_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 1 Minutes",	DS1337_A1_MIN_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 1 Hours",	DS1337_A1_HR_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 1 Day/Date", DS1337_A1_DAY_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 2 Minutes",	DS1337_A2_MIN_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 2 Hours",	DS1337_A2_HR_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Alarm 2 Day/Date", DS1337_A2_DAY_REG,
     READ_WRITE| SAVE_RESTORE | REG_ACCESS, {(utype_t)&reg_ext}, 0xFF, 0x00 },
    {"Control",		DS1337_CONTROL_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x9F, 0x18 },
    {"Status",		DS1337_STATUS_REG,
     READ_ONLY | REG_ACCESS,		    {(utype_t)&reg_ext}, 0x83, 0x00 },
    { 0, 0, 0, {0}, 0, 0},
};

/*****************************************************************
 * Name:	dev_1337_create
 *
 * Description:	Newly create object with various device function
 *		point to "do nothing" and then initialize all of the
 *		appropriate function.
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device
 *		dev_error_report_t This is a callout function provided
 *		by/for the platform
 *
 * Returns:	none
 *****************************************************************
 */
void
dev_1337_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_ds1337_object_t	*pds1337 = (dev_ds1337_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		    NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in dev_1337_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    pds1337->base.dev_object_fvt->dev_attach	= dev_1337_attach;
    pds1337->base.dev_object_fvt->dev_detach	= dev_1337_detach;
    pds1337->base.dev_object_fvt->dev_reconfig_needed = dev_1337_reconfig;
    pds1337->base.dev_object_fvt->dev_restart	= dev_1337_restart;
    pds1337->base.dev_object_fvt->dev_init	= dev_1337_init;
    pds1337->base.dev_object_fvt->dev_show	= dev_1337_show;
    pds1337->base.dev_object_fvt->dev_error_report = error_report_fn;
    pds1337->base.dev_object_fvt->dev_collect_crashinfo = dev_1337_crsh;
    pds1337->base.dev_object_fvt->dev_destroy	= dev_1337_destroy;
    pds1337->base.dev_object_fvt->dev_name	= "Maxim DS1337";

    pds1337->callin_fvt = (dev_ds1337_callin_fvt_t *)
				malloc(sizeof(dev_ds1337_callin_fvt_t));
    pds1337->callout_fvt = (dev_ds1337_callout_fvt_t *)
				malloc(sizeof(dev_ds1337_callout_fvt_t));

    pds1337->base.dev_state = DEV_STATE_CREATE;

}

/*****************************************************************
 * Name:	dev_1337_attach
 *
 * Description:	Attach the Dallas 1337 device for use. This
 *		function will initialize and setup all necessary pointers
 *		and bring the chip to operation.
 *
 * Input:	Pointer to the Dallas 1337 device object
 *
 * Returns:	none
 *****************************************************************
 */
static uint32
dev_1337_attach (dev_object_t *dev)
{
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    uint32 rc;
    char err_buf[ERR_BUF_SIZE];

    if (pds1337->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_1337_attach() callin malloc",
			 DS1337_ATTACH);
	return(FAILED);
    }

    if (pds1337->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_1337_attach() callout malloc",
			 DS1337_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    pds1337->callin_fvt->peek_n_poke = ds1337_alter_reg;
    pds1337->callin_fvt->display_rtc = ds1337_display_rtc;
    pds1337->callin_fvt->set_rtc = ds1337_set_rtc;
    pds1337->callin_fvt->mux_clk_to_dout = ds1337_mux_clk_to_dout;
    pds1337->callin_fvt->register_test = ds1337_register_test;
    pds1337->callin_fvt->time_validity_test = ds1337_time_validity_test;

    /* Lock the I2C device */
    if ((rc = pds1337->callout_fvt->open(pds1337->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_1337_attach() I2C open returns with rc %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_ATTACH);
	return(FAILED);
    }

    pds1337->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name:	dev_1337_detach()
 *
 * Description:	detach the device specific functions from the caller.
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
 * Input:	Pointer to the Dallas 1337 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_1337_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = pds1337->callout_fvt->close(pds1337->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_1337_detach() I2C close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, pds1337->base.dev_object_fvt);

    pds1337->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name:	dev_1337_reconfig_needed
 *
 * Description:	To check whether device re-configuration is needed during
 *		(re)initialization. Based on the provided context information,
 *		the boolean return value, and possibly other factors external
 *		to the device object, the caller shall decide whether to invoke
 *		either dev_restart or dev_init, but not both. In general, the
 *		boolean return value alone is not sufficient to decide whether
 *		the device can safely be restarted or whether it must be fully
 *		initialized from scratch.
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device
 *		void * - a device/platform specific context handle
 *		boolean * - a pointer to a boolean
 *
 * Returns:	PASSED/FAILED, context information and a boolean value.
 *		The boolean value shall be set to TRUE if the device must be
 *		reconfigured from scratch and it shall be set to FALSE
 *		otherwise.
 *
 * Assumptions:	The dev_attach() function has been called and successfully
 *****************************************************************/
static uint32
dev_1337_reconfig(dev_object_t *dev, void *context_handle, boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_1337_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		called and successfully executed.
 *****************************************************************/
static uint32
dev_1337_restart(dev_object_t *dev)
{
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *) dev;

    pds1337->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 *
 * Name:	dev_1337_init()
 *
 * Description:	Initializes the Dallas 1337 chip.
 *		Since Real Time Clock is battery backed, there is nothing
 *		to initialize. However, we can check the status to make sure
 *		that we can access the chip, and see if the oscillator or
 *		the alarm flag is set. Xformers do not use the alarm, However,
 *		a message can be displayed if it is set.
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device.
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_1337_init (dev_object_t *dev)
{
    uint32 rc;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    dev_ds1337_callout_fvt_t *callout_p = pds1337->callout_fvt;
    n2g_i2c_if_t new_i2c_if;
    rtc_t status;
    char err_buf[ERR_BUF_SIZE * 2];

    /* Initialize the new I2C API interface struct */
    new_i2c_if.size = sizeof(rtc_t);
    new_i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;

    /* Read the status */
    new_i2c_if.buf = (char *)&status;
    new_i2c_if.offset = DS1337_STATUS_REG;

    rc = callout_p->rd(&new_i2c_if);

    if (rc != PASSED) {
	sprintf(err_buf, "dev_1337_init() read status rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_INIT);
	return(FAILED);
    }

    if (status & DS1337_STAT_OSF) {
	status = 0;	/* Clear all status bit */
	rc = callout_p->wr(&new_i2c_if);

	sprintf(err_buf, "%s() Oscillator Stopped. Status = %#x.\n"
			 "Set the date and time with I2C Real Time Clock "
			 "utility.\nIf this occurs again, replace the RTC.",
			 __FUNCTION__, status);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_INIT);
	rc = FAILED;
    }

    pds1337->base.dev_state = DEV_STATE_INIT;

    return(rc);

}

/*****************************************************************
 * Name:	dev_10698_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device
 *		A device print function vector
 *		A dev_show_cmd_e command
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		platform which implements the print logging functionality. The
 *		dev_attach() function has been called and successfully executed
 *
 *****************************************************************/
static uint32
dev_1337_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    uint32 rc;
    rtc_t data;	/* data bytes from Dallas 1337 */
    reg_info_t *reg_p;
    n2g_i2c_if_t new_i2c_if;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    dev_ds1337_callout_fvt_t *callout_p = pds1337->callout_fvt;
    char err_buf[ERR_BUF_SIZE];

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = (char *)&data;
    new_i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;

    dev_print("\n     %s Registers:\n", pds1337->base.dev_object_fvt->dev_name);

    /* set the pointer to the beginning of the registers table */
    reg_p = &ds1337_reg_tbl[0];

    while (reg_p->size.size) {
	/* Read the data bytes of Dallas 1337 */
	new_i2c_if.offset = reg_p->offset;
	new_i2c_if.size = sizeof(data);
	rc = (*callout_p->rd)(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "dev_1337_show() read %s @ %#x rc = %#x",
						reg_p->name, reg_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, DS1337_SHOW);
	    return(FAILED);
	}

#ifdef I2C_DEBUG
	printf("data = %#x @ %#x\n", data, &data);
#endif /* I2C_DEBUG */

	switch (cmd) {
	case DEV_SHOW_ALL:
	case DEV_SHOW_CONFIG:
	case DEV_SHOW_REGISTERS:
	    /* Print the read register */
	    dev_print("%s @ offset %#x = 0x%02X\n",
		       reg_p->name, reg_p->offset, data);
	    break;
	case DEV_SHOW_BRIEF:
	    dev_print("@ %#x = 0x%02X ", reg_p->offset, data);
	    break;
	default:
	    assert(!"dev_1337_show");
	    break;
	} /* endof switch */
	reg_p++;
#ifdef LINUX_APP
	udelay(N2G_I2C_BIT_DELAY);	/* Wait for stop bit */
#endif /* LINUX_APP */
    } /* endof for */

    return(PASSED);
}

/*****************************************************************
 * Name:	dev_1337_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device
 *		A crash print function vector.
 *		A verbosity level.
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	A device print function vector has been provided by the host
 *		platform which implements the crash logging functionality. It
 *		could be the mechanism to log info to the Compact Flash before
 *		the device crash and now retrieve them. The dev_attch()
 *		function has been called and successfully executed.
 *
 *****************************************************************/
static uint32
dev_1337_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;

    /* more development in this section */
    dev_print("dev_1337_crsh(): No Crash info available for %s\n",
					pds1337->base.dev_object_fvt->dev_name);
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_1337_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the Dallas 1337 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_1337_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_ds1337_object_t *pds1337;
    char err_buf[80];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    pds1337 = (dev_ds1337_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = pds1337->callout_fvt->close(pds1337->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_1337_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, DS1337_DESTROY);
	return;
    }

    if (pds1337->callout_fvt) {
	free(pds1337->callout_fvt);	/* Free callout struct */
    }

    if (pds1337->callin_fvt) {
	free(pds1337->callin_fvt);	/* Free callin struct */
    }

    free(pds1337->base.dev_object_fvt);	/* Free dev_object_t */
}

/********************************************************************
 *
 * Function:	ds1337_alter_reg
 *
 * Description:	Peek-n-poke Dallas 1337 register.
 *
 * Inputs:	dev_object_t pointer to the Dallas 1337 device
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
ds1337_alter_reg(dev_object_t *dev, print_fn_t dev_print)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    dev_ds1337_callout_fvt_t *callout_p = pds1337->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    rtc_t old_data, new_data;
    rtc_o reg;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(rtc_t);
    new_i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;

    /* Get the register to peek-n-poke */
    reg = gethex_answer("Enter the register number:", 0, 0, DS1337_STATUS_REG);

    /* Find the register text in the register table */
    reg_p = &ds1337_reg_tbl[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != reg) {
	/* Not requested register */
	reg_p++;	/* update the register table pointer */
    }

    if (reg_p->name == 0) {
	sprintf(err_buf, "ds1337_alter_reg() register %#x is not writeable",
						      reg);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_ALTER);
	return(FAILED);
    }

    /* Read the register first. */
    new_i2c_if.buf = (char *)&old_data;
    new_i2c_if.offset = reg;

    rc = (*callout_p->rd)(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "ds1337_alter_reg() read %s register rc = %#x",
						  reg_p->name, rc);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_ALTER);
	return(FAILED);
    } /* endof if rc */

    /* Get the new data */
    new_data = gethex_answer("Enter the data:", old_data, 0, 0xFF);

    /* Write the new data */
    new_i2c_if.buf = (char *)&new_data;

#ifdef DS1337_DEBUG
    printf("\nwrite %s with %#x\n", reg_p->name, new_data);
#endif /* DS1337_DEBUG */

    rc = (*callout_p->wr)(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "ds1337_alter_reg() write %s @ %#x rc = %#x",
						   reg_p->name, reg, rc);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_ALTER);
    }

    return(rc);
}

/*******************************************************************************
 * Function:	rtc_read_time
 *
 * Description:	This function reads out the value programmed in the RTC
 *		(Real Time Clock) chip and displays it as English.
 *	
 *		RTC runs at 100 KHz, ie. 10 microseconds per bit. With 9 bits
 *		per register (8 bits + 1 ACK/NACK bit), and the struct size of
 *		7 registers, the burst should complete in 630 microseconds.
 *
 *		If the second is 59 BCD, we can clear it to 0, and increment
 *		the minute, hour, date, month, year, if needed.
 *
 * Inputs:	pointer to the Dallas 1337 device object.
 *		Pointer to the RTC register struct 
 *
 * Output:	PASSED/FAILED 
 *
 *******************************************************************************
 */
static int
rtc_read_time (dev_object_t *dev, ds1337_rtc_data_t *rtc)
{
    int rc;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    n2g_i2c_if_t new_i2c_if;
    char err_buf[ERR_BUF_SIZE];

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(ds1337_rtc_data_t);
    new_i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;
    new_i2c_if.offset = DS1337_SECONDS_REG;
    new_i2c_if.buf = (char *)rtc;

    rc = pds1337->callout_fvt->rd(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "RTC read error. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_READ);
	return (FAILED);
    } 

    if (rtc->year == DS1337_DEFAULT_YEAR) {
	DEV_ERROR_REPORT(dev, "Real-Time-Clock might not be programmed",
			      DS1337_RTC_PROGRAM);
	return (FAILED);
    }

    return (PASSED);
} /* end of rtc_read_time */


/*******************************************************************************
 * Function:	rtc_set_time
 *         
 * Description:	This function will allow user to set the current time
 *		Note that the hour is in 24 hour mode; not 12 hour mode
 *
 * Inputs :	pointer to the Dallas 1337 device object.
 *		pointer to the RTC register struct 
 *
 * Output :	PASSED/FAILED
 *
 *******************************************************************************
 */
static int
rtc_set_time (dev_object_t *dev, ds1337_rtc_data_t *rtc) 
{
    int   ret;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    n2g_i2c_if_t new_i2c_if;
    char err_buf[ERR_BUF_SIZE];

    /* Setup I2C API interface struct */
    /* There are 7 bytes to be sent. This is more than the Cavium HLC limit.
     * Send 6 bytes first, then the year last.
     */
    new_i2c_if.size = sizeof(ds1337_rtc_data_t) - 1;
    new_i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;
    new_i2c_if.offset = DS1337_SECONDS_REG;
    new_i2c_if.buf = (char *)rtc;

#ifdef RTC_DEBUG
    printf("\n\nrtc_set_time() BEFORE write, rtc->second=%#.2x", rtc->second);
#endif

    ret = pds1337->callout_fvt->wr(&new_i2c_if);

    if (ret != PASSED) {
	sprintf(err_buf, "RTC write error, rc = %#x", ret);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_WRITE);
    } 

    new_i2c_if.size = sizeof(rtc_t);
    new_i2c_if.buf = (char *)&rtc->year;
    new_i2c_if.offset = DS1337_YEAR_REG;

    ret = pds1337->callout_fvt->wr(&new_i2c_if);

    if (ret != PASSED) {
	sprintf(err_buf, "RTC write error for year. rc = %#x", ret);
	DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_WRITE);
    }

    return (ret);
} /* end of rtc_set_time */


/*******************************************************************************
 * Function:	convert_timeval_to_rtc_data
 *
 * Description:	Convert Time values to DS1337 inputs
 *
 * Inputs:	rtc : RTC data required by the DS1337 driver.
 *		tv  : structure contain input of the time values (tv)
 *
 * Output:	void
 *******************************************************************************
 */
static void
convert_timeval_to_rtc_data (ds1337_rtc_data_t *rtc, ds1337_time_t *tv)
{
    uchar year, century;

    rtc->second = ((tv->second / 10) << HALF_BYTE) |
		  ((tv->second % 10) & DS1337_SINGLE_SEC_MASK);
    rtc->minute = ((tv->minute / 10) << HALF_BYTE) |
		  ((tv->minute % 10) & DS1337_SINGLE_MIN_MASK);
    rtc->hour   = ((tv->hour / 10) << HALF_BYTE)| /* diag uses 24 hr format */
		  ((tv->hour % 10) & DS1337_SINGLE_HOUR_MASK);
    rtc->date   = ((tv->date / 10) << HALF_BYTE) |
		  ((tv->date % 10) & DS1337_SINGLE_DATE_MASK);
    rtc->month  = ((tv->month / 10) << HALF_BYTE) |
		  ((tv->month % 10) & DS1337_SINGLE_MONTH_MASK);
    rtc->day_of_week = tv->day_of_week & DS1337_WEEK_DAY_MASK;

    century = (tv->year / 100);
    if (century >= 20) {
	rtc->month |= DS1337_CENTURY_MASK;
    }
    year = tv->year - (century * 100);
    rtc->year = ((year / 10) << HALF_BYTE) |
		((year % 10) & DS1337_SINGLE_YEAR_MASK);

#ifdef RTC_DEBUG
    printf("\nRTC Struct Time->RTC...");
    printf("\nsecond = %#.2x  minute = %#.2x  hour = %#.2x", 
	   rtc->second, rtc->minute, rtc->hour);
    printf("\ndate   = %#.2x  month  = %#.2x  year = %#.2x  \n",
	   rtc->date, rtc->month, rtc->year);

    printf("\nTV Struct Time->RTC...");
    printf("\nsecond = %.2d  minute = %.2d  hour = %.2d", 
	   tv->second, tv->minute, tv->hour);
    printf("\ndate   = %.2d  month  = %.2d  year = %.4d\n",
	   tv->date, tv->month, tv->year);
#endif
} /* end of convert_timeval_to_rtc_data */


/*******************************************************************************
 * Function:	convert_rtc_data_to_timeval
 *
 * Description:	Convert the RTC data returned by the DS1337 to the timeval
 *		format as needed by diag.
 *
 * Inputs:	rtc : RTC data returned by the DS1337 driver.
 *		te  : structure constains the converted time value (tv)
 *
 * Output: void
 *******************************************************************************
 */
static void 
convert_rtc_data_to_timeval (ds1337_rtc_data_t *rtc, ds1337_time_t *tv)
{
    tv->second = BITS_4_TO_6(rtc->second) * 10 + BITS_0_TO_3(rtc->second);
    tv->minute = BITS_4_TO_6(rtc->minute) * 10 + BITS_0_TO_3(rtc->minute);

    if (rtc->hour & DS1337_12HR_24HR_MASK) {  /* AM/PM format */
	if (rtc->hour & DS1337_AM_PM_MASK) {   /* PM */
	    if (rtc->hour & DS1337_AM_PM_10_HOUR_MASK) {
		tv->hour = 12 + 10 + BITS_0_TO_3(rtc->hour);
	    } else {
		tv->hour = 12 + BITS_0_TO_3(rtc->hour);
	    }
	} else { /* AM */
	    if (rtc->hour & DS1337_AM_PM_10_HOUR_MASK) {
		tv->hour = 10 + BITS_0_TO_3(rtc->hour);
            } else {
		tv->hour = BITS_0_TO_3(rtc->hour);
	    }
	}
    } else { /* 24 hours format */
	tv->hour = BITS_4_TO_5(rtc->hour) * 10 + BITS_0_TO_3(rtc->hour);
    }

    tv->day_of_week = BITS_0_TO_2(rtc->day_of_week);
    tv->date  = BITS_4_TO_5(rtc->date) * 10 + BITS_0_TO_3(rtc->date);
    tv->month = GET_BIT_4(rtc->month)*10 + BITS_0_TO_3(rtc->month);
    /*
     * We can safely assume that century for this router
     * will always be 2000, with this we can save one i2c
     * read cycle.
     */
    tv->year  = BITS_0_TO_3(rtc->year) + 
		(BITS_4_TO_7(rtc->year) * 10) + CURRENT_CENTURY;

#ifdef RTC_DEBUG
    printf("\nRTC Struct RTC->Time...");
    printf("\nsecond = %#.2x  minute = %#.2x  hour = %#.2x", 
	   rtc->second, rtc->minute, rtc->hour);
    printf("\ndate   = %#.2x  month  = %#.2x  year = %#.2x  \n",
	   rtc->date, rtc->month, rtc->year);

    printf("\nTV Struct RTC->Time...");
    printf("\nsecond = %.2d  minute = %.2d  hour = %.2d", 
	   tv->second, tv->minute, tv->hour);
    printf("\ndate   = %.2d  month  = %.2d  year = %.4d\n",
	   tv->date, tv->month, tv->year);
#endif
}/* end of convert_rtc_data_to_timeval */

/*******************************************************************************
 * Function:	ds1337_mux_clk_to_dout
 *
 * Description:	This function is a debug funstion to help debug RTC.
 *		This will output the value of the specified clock to the 
 *		SDA pin of the I2C bus.    
 *
 * Inputs:	dev : pointer to the Dallas 1337 Device object struct
 *		freq : Square-wave output frequency. (1 Hz, 4 kHz, 8 kHz, or
 *		       32 kHz)
 *
 * Output: void
 *******************************************************************************
 */
static uint32 
ds1337_mux_clk_to_dout(dev_object_t *dev, uchar freq)
{
    rtc_t status_reg, control_reg, temp;
    int ret = PASSED;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)dev;
    n2g_i2c_if_t new_i2c_if;
    char err_buf[ERR_BUF_SIZE];

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(rtc_t);
    new_i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    new_i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;

    printf("\n WARNING: System will not work correctly after");
    printf(" running this test.\n");

    if (getc_answer(" Continue (Y/N)", "yn", 'y') == 'y') {
	/* Clear Write Protect Bit in Control Register */

	new_i2c_if.offset = DS1337_STATUS_REG;
	new_i2c_if.buf = (char *)&status_reg;
	ret = pds1337->callout_fvt->rd(&new_i2c_if);

	if (ret != PASSED) {
	    sprintf(err_buf, "ds1337_mux_clk_to_dout() Status read failed %#x",
							ret);
	    DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_READ);
	    return(FAILED);
	}

	new_i2c_if.offset = DS1337_CONTROL_REG;
	new_i2c_if.buf = (char *)&control_reg;

	ret = pds1337->callout_fvt->rd(&new_i2c_if);
 
	if (ret != PASSED) {
	    sprintf(err_buf, "ds1337_mux_clk_to_dout() Control read failed %#x",
							ret);
	    DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_READ);
	    return(FAILED);
	}

	/* Save the OSF bit, but clear the Alarms flags */
        temp = status_reg & DS1337_STAT_OSF;

	new_i2c_if.offset = DS1337_STATUS_REG;
	new_i2c_if.buf = (char *)&temp;

        ret = pds1337->callout_fvt->wr(&new_i2c_if);

	if (ret != PASSED) {
	    sprintf(err_buf, "ds1337_mux_clk_to_dout() Status write failed %#x",
							ret);
	    DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_WRITE);
	    return(FAILED);
	}

	new_i2c_if.offset = DS1337_CONTROL_REG;
	new_i2c_if.buf = (char *)&freq;

	ret = pds1337->callout_fvt->wr(&new_i2c_if);

	if (ret != PASSED) {
	    sprintf(err_buf, "ds1337_mux_clk_to_dout() Freq write failed %#x", ret);
	    DEV_ERROR_REPORT(dev, err_buf, DS1337_RTC_WRITE);
	    return(FAILED);
	}

	if (getc_answer("Restore to the original setting? (y/n)", "yn", 'y') ==
	    'y') {
	    new_i2c_if.buf = (char *)&control_reg;

	    ret = pds1337->callout_fvt->wr(&new_i2c_if);

	} else {
	    printf("\n To continue normal operation."
		   " Please power cycle the system.");
	    printf("\n ### The battery will need to be shorted to ground. ###");
	}
    }
    return (ret);
} /* end of ds1337_mux_clk_to_dout */

/*******************************************************************************
 * Function:	ds1337_display_rtc
 *
 * Description:	RTC read utility
 *
 * Input:	Pointer to the Dallas 1337 device object
 *
 * Output:	PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32
ds1337_display_rtc (dev_object_t *dev)
{
    ds1337_rtc_data_t rtc_data;
    ds1337_time_t tv;
    dev_ds1337_object_t *rtc_obj = (dev_ds1337_object_t *)dev;

    /* Maxim DS1337C I2C RTC */
    if (rtc_read_time(dev, &rtc_data) == FAILED) {
        return (FAILED);
    }		
   
    convert_rtc_data_to_timeval(&rtc_data, &tv);
    if (rtc_obj->dt == 0) {
	printf("\n Date: %.2d/%.2d/%.4d  Time: %.2d:%.2d:%.2d\n", 
		tv.month, tv.date, tv.year, tv.hour, tv.minute, tv.second);
    } else {
	*rtc_obj->dt = tv;
    }
    return (PASSED);

} /* end of ds1337_display_rtc */

/*******************************************************************************
 * Function:	ds1337_set_rtc
 *
 * Description:	Dallas 1337 date and time write utility 
 *
 * Input:	pointer to the Dallas 1337 device object struct
 *
 * Outpu:	PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32
ds1337_set_rtc (dev_object_t *dev)
{
    dev_ds1337_object_t *rtc_obj = (dev_ds1337_object_t *)dev;
    ds1337_rtc_data_t rtc_data, *rtc = &rtc_data;
    ds1337_time_t tv, tv_old;
    int rc;
	
    /* Maxim DS1337C I2C RTC */

    /*
     * if there is a failure from i2c_read_byte(), the i2c_read_byte()
     * will log error, so it's not necessary to check again here
     */
    rc = rtc_read_time(dev, &rtc_data);
    if (rc) {
	printf("RTC read error\n");
	return (FAILED);
    } 

#ifdef RTC_DEBUG
    printf("\nRTC year=%#.2x minute=%#.2x second=%#.2x\n", 
	   rtc->year, rtc->minute, rtc->second);
#endif

    if (rtc_obj->dt == 0) {
	convert_rtc_data_to_timeval(rtc, &tv_old);

	tv.year   = getdec_answer("Enter Year", tv_old.year, 2000, 2050);
	tv.month  = getdec_answer("Enter Month", tv_old.month, 1, 12);
	tv.date   = getdec_answer("Enter Date", tv_old.date, 1, 31);
	tv.hour   = getdec_answer("Enter Hour (24 hour)", tv_old.hour, 0, 23);
	tv.minute = getdec_answer("Enter Minute", tv_old.minute, 0, 59);
	tv.second = getdec_answer("Enter Second", tv_old.second, 0, 59);
    } else {
	tv = *rtc_obj->dt;
    }

#ifdef RTC_DEBUG
    printf("\nTV minute=%.2d second=%.2d\n", tv.minute, tv.second);
#endif

    convert_timeval_to_rtc_data(rtc, &tv);
    rtc_set_time(dev, rtc);
    rtc_read_time(dev, rtc);
    convert_rtc_data_to_timeval(rtc, &tv);

    printf("\n Date: %.2d/%.2d/%.4d  Time: %.2d:%.2d:%.2d\n", 
	   tv.month, tv.date, tv.year, tv.hour, tv.minute, tv.second);

    return (PASSED);
} /* end of ds1337_set_rtc */

/*******************************************************************************
 *
 * Function: ds1337_register_test
 *
 * This test writes and reads the registers of the DS1337 and then
 * verifies the read value against the written value.
 *
 * Input: pointer to the register struct
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32
ds1337_register_test (dev_object_t *dev)
{
    reg_info_t *reg_ptr;

    /* Init pointer to beginning of register table */
    reg_ptr = &ds1337_reg_tbl[0];
    reg_ptr->size.ext->param = (void *)dev;

    return (register_tests(0, reg_ptr));
}

/*******************************************************************************
 *
 * Function:	ds1337_time_validity_test
 *
 * This function is a debug function to help debug RTC
 *
 * Input:	pointer to the Dallas 1337 Device object struct 
 *
 * Output:	PASSED/FAILED 
 *
 *******************************************************************************
 */
static uint32
ds1337_time_validity_test (dev_object_t *dev)
{
    ds1337_rtc_data_t rtc_data1, rtc_data2;
    ds1337_rtc_data_t rtc_test_start = {
	DS1337_SEC_MAX - V_TEST_START_DELTA, DS1337_MIN_MAX,
	DS1337_HR_24MAX, DS1337_DAY_MAX,
	DS1337_DATE_MAX, DS1337_MN_MAX,
	DS1337_YR_MAX};
    ds1337_rtc_data_t rtc_test_end = {
	TEST_TIME_DELAY - V_TEST_START_DELTA - 1, DS1337_MIN_MIN,
	DS1337_HR_24MIN, DS1337_DAY_MIN,
	DS1337_DATE_MIN, DS1337_CENTURY_MASK | DS1337_MN_MIN,
	DS1337_YR_MIN};
    ds1337_time_t tv;
    int ix, test_status, rc;
    char buf[ERR_BUF_SIZE];

    for (ix = 2; ix > 0; ix--) {

	/* Read the time */
	if (rtc_read_time(dev, &rtc_data1) == FAILED) {
	    sprintf(buf, "%s Unable to read RTC initially", __FUNCTION__);
	    DEV_ERROR_REPORT(dev, buf, DS1337_TIME_STOP);
	    return (FAILED);
	}		
	convert_rtc_data_to_timeval(&rtc_data1, &tv);
       
	/* 
	 * Read again to prevent rollover condition 
	 * Delay to rollover for second read
	 */
	if (tv.second >= (SECONDS_PER_MIN - TEST_TIME_DELAY)) {
	    /* Delay to rollover */
	    mdelay((SECONDS_PER_MIN - tv.second) * 1000);
	} else {
	    /* Updates the seconds field to be restored later */
	    /* Slave address + offset + 7 registers = 9 bytes. 8 data bits +
	     * Ack bit = 9 bits per byte. 9 * 9 + Start + Stop = 83 bits.
	     * At 100KHz (10 us), 830us need for each I2C transactions.
	     * 2 read and 2 write will take 3320 us. Since RTC is granularity
	     * is in second, we can ignore the test transactions.
	     */
	    tv.second += TEST_TIME_DELAY;
	    convert_timeval_to_rtc_data(&rtc_data1, &tv);
	    break;
	}       
    }

    test_status = rtc_set_time(dev, &rtc_test_start);
    if (test_status != PASSED) {
	sprintf(buf, "%s Unable to set start time.", __FUNCTION__);
	test_status = FAILED;
    } else {
	/* Wait for the time to rollover */
	mdelay(TEST_TIME_DELAY * 1000);
	if (rtc_read_time(dev, &rtc_data2) == FAILED) {
	    sprintf(buf, "%s Unable to read the new time", __FUNCTION__);
	    test_status = FAILED;
	} else {
	    /* Got the new time. Check if matches what we expect */
	    /* Check Second */
	    if ((rtc_data2.second < (rtc_test_end.second -
				     RTC_TEST_TOLERANCE)) ||
		(rtc_data2.second > (rtc_test_end.second +
				     RTC_TEST_TOLERANCE))) {
		/* Out of the tolerance bound */
		sprintf(buf, "%s Seconds not within tolerance. Read %02x",
			     __FUNCTION__, rtc_data2.second);
		test_status = FAILED;
	    }
	    /* Check Minute */
	    if ((test_status == PASSED) && (rtc_data2.minute !=
					    rtc_test_end.minute)) {
		/* Minute does not match */
		sprintf(buf, "%s Minute does not match. Expect %02x. Read %02x",
			     __FUNCTION__, rtc_test_end.minute,
			     rtc_data2.minute);
		test_status = FAILED;
	    }
	    /* Check Hour */
	    if ((test_status == PASSED) && (rtc_data2.hour !=
					    rtc_test_end.hour)) {
		/* Hour does not match */
		sprintf(buf, "%s Hour does not match. Expect %02x. Read %02x",
			     __FUNCTION__, rtc_test_end.hour,
			     rtc_data2.hour);
		test_status = FAILED;
	    }
	    /* Check Day of the week */
	    if ((test_status == PASSED) && (rtc_data2.day_of_week !=
					    rtc_test_end.day_of_week)) {
		/* Day of the week does not match */
		sprintf(buf, "%s Day of the week does not match. Expect %02x. "
			     "Read %02x",
			     __FUNCTION__, rtc_test_end.day_of_week,
			     rtc_data2.day_of_week);
		test_status = FAILED;
	    }
	    /* Check Date */
	    if ((test_status == PASSED) && (rtc_data2.date !=
					    rtc_test_end.date)) {
		/* Date does not match */
		sprintf(buf, "%s Date does not match. Expect %02x. Read %02x",
			     __FUNCTION__, rtc_test_end.date,
			     rtc_data2.date);
		test_status = FAILED;
	    }
	    /* Check Month and Century */
	    if ((test_status == PASSED) && (rtc_data2.month !=
					    rtc_test_end.month)) {
		/* Month or Century does not match */
		sprintf(buf, "%s Month or Century does not match. "
			     "Expect %02x. Read %02x",
			     __FUNCTION__, rtc_test_end.month,
			     rtc_data2.month);
		test_status = FAILED;
	    }
	    /* Check Year */
	    if ((test_status == PASSED) && (rtc_data2.year !=
					    rtc_test_end.year)) {
		/* Year does not match */
		sprintf(buf, "%s Year does not match. Expect %02x. Read %02x",
			     __FUNCTION__, rtc_test_end.year,
			     rtc_data2.year);
		test_status = FAILED;
	    }
	}
    }		

    /* Regardless the test result, always try to revover */
    rc = rtc_set_time(dev, &rtc_data1);

    if ((rc != PASSED) && (test_status == PASSED)) {
	sprintf(buf, "%s Unable to restore the time", __FUNCTION__);
	test_status = FAILED;
    }

    if (test_status != PASSED) {
	DEV_ERROR_REPORT(dev, buf, DS1337_TIME_STOP);
    }

    return (test_status);
}

/**********************************************************************
 *
 * Function:	dev_i2c_rd
 *
 * This function: reads Dallas 1337 registers
 *
 * Input :	addr - offset of register to be read.
 *		size - number of bytes to be read.
 *		buf  - points to the data buffer to be read.
 *		param - Pointer to the Dallas 1337 device object
 *
 * Output:	PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_i2c_rd(ulong addr, int size, ulong *buf, void *param)
{
    uint32 rc;
    char err_buf[ERR_BUF_SIZE];
    n2g_i2c_if_t i2c_if;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)param;
    dev_ds1337_callout_fvt_t *callout_p = pds1337->callout_fvt;
    rtc_t rtc_data;

    /* Size cannot exceed the buffer size */
    if (size >= (int)sizeof(ulong) - (int)sizeof(uchar)) {
	sprintf(err_buf, "dev_i2c_rd() invalid size %#x", size);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, DS1337_I2C_READ);
	return(FAILED);
    }

    /* Setup the interface struct for I2C API read */
    i2c_if.offset = addr;
    i2c_if.buf = (char *)&rtc_data;
    i2c_if.size = size;
    i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;

    /* Call the I2C Read API */
    rc = callout_p->rd(&i2c_if);
    if (rc != PASSED) {
	/* Read failed */
	sprintf(err_buf, "dev_i2c_rd() read return code %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, DS1337_I2C_READ);
	return(FAILED);
    } else {
	/* Got the data */
	*buf = rtc_data;
	return(PASSED);
    }
}

/**********************************************************************
 *
 * Function:	dev_i2c_wr
 *
 * This function: writes Dallas 1337 registers
 *
 * Input :	addr - offset of register to be written.
 *		size - number of bytes to write.
 *		data  - write data.
 *		param - Pointer to the Dallas 1337 device object
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int
dev_i2c_wr(ulong addr, int size, ulong data, void *param)
{
    uint32 rc;
    n2g_i2c_if_t i2c_if;
    dev_ds1337_object_t *pds1337 = (dev_ds1337_object_t *)param;
    dev_ds1337_callout_fvt_t *callout_p = pds1337->callout_fvt;
    rtc_t rtc_data;
    char err_buf[ERR_BUF_SIZE];

    /* Size cannot exceed the buffer size */
    if (size >= (int)sizeof(ulong) - (int)sizeof(uchar)) {
	sprintf(err_buf, "dev_i2c_wr() invalid size %#x", size);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, DS1337_I2C_WRITE);
	return(FAILED);
    }

    rtc_data = (rtc_t)data;

    /* Setup the interface struct for I2C API write */
    i2c_if.offset = addr;
    i2c_if.buf = (char *)&rtc_data;
    i2c_if.size = size;
    i2c_if.i2c_bus_type = pds1337->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = pds1337->i2c_p->i2c_dev;

    /* Call the I2C Write API */
    rc = callout_p->wr(&i2c_if);
    if (rc != PASSED) {
	/* Write failed */
	sprintf(err_buf, "dev_i2c_wr() write return code %#x", rc);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, DS1337_I2C_WRITE);
	return(FAILED);
    } else {
	/* Data written */
	return(PASSED);
    }
}

/* end of file */
/***********************************************************************
$Log: dev_1337.c,v $
Revision 1.7  2017/07/14 02:51:38  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.6  2013/11/26 08:45:41  hroni
fix compiler warning

Revision 1.5  2013/11/26 08:40:18  hroni
fix compiler warning

Revision 1.4  2012/06/04 10:05:52  aarwang
- Clean up compiler warnings.

Revision 1.3  2012/03/28 00:38:06  mcharon
remove forward slash from second line

Revision 1.2  2012/03/25 03:57:28  palin2
Clean up Overlord Intel side (x86) compile warnings.

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
