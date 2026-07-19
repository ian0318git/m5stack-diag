/* $Id: dev_max1617a.c,v 1.2 2012/03/28 00:38:08 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_max1617a/dev_max1617a.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_max1617a.c
 *
 * Description:	Diode Sensor (Maxim 1617A) I2C Common device driver.
 *
 *		Max1617A has an ALERT# pin that can be used as an interrupt.
 *		Xformers have this pin connected to the Environmental Control
 *		Unit. Therefore, dev_intr_enable, dev_intr_disable, and
 *		dev_isr are not implemented.
 *
 *		Max1617A does not have reset pin. But it has SPOR (software
 *		PowerOnReset) command that can be issued.
 *
 *		Max1617A cannot be enabled or disabled; therefore,
 *		dev_oper_enable, and dev_oper_disable are not impletmented.
 *
 *		Only dev_attach, dev_detach, dev_reconfig_needed, dev_restart,
 *		dev_init, dev_show, dev_err_report, dev_collect_crashinfo,
 *		dev_destroy are implemented.
 *
 * Copyright (c) 2007-2012 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <assert.h>
#include "endians.h"
#include "defs.h"
#include "common.h"
#include "dev_max1617a.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"

/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
static uint32	dev_1617_attach(dev_object_t *dev);
static uint32	dev_1617_detach(dev_object_t *dev);
static uint32	dev_1617_reconfig(dev_object_t *, void *, boolean *);
static uint32	dev_1617_restart(dev_object_t *);
static uint32	dev_1617_init(dev_object_t *);
static uint32	dev_1617_show(dev_object_t *, print_fn_t, dev_show_cmd);
static uint32	dev_1617_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void	dev_1617_destroy(dev_object_t **);

static int	max1617a_alter_reg(dev_object_t *, print_fn_t);
static int	max1617a_clear_alert(dev_object_t *, print_fn_t);
static int	max1617a_show_temp(dev_object_t *, print_fn_t, int);

/*static int sensor_diode_open(void);
static void sensor_int_hndlr(void); */

/* Global variables */
/* Max1617A registers table. This device is command based. Register offset is
 * the command written to the device.
 */
static reg_info_t sensor_reg_table[] =
{
    {"Current local temperature",  MAX1617_CMD_RLTS,	READ_ONLY,
	{0}, 0xFF, 0x00},
    {"Local THIGH limit",	MAX1617_CMD_RLHN,	READ_ONLY,
	{0}, 0xFF, 0x7F},
    {"Local TLOW limit",	MAX1617_CMD_RLLI,	READ_ONLY,
	{0}, 0xFF, 0xC9},
    {"Current remote temperature", MAX1617_CMD_RRTE,	READ_ONLY,
	{0}, 0xFF, 0x00},
    {"Remote THIGH limit",	MAX1617_CMD_RRHI,	READ_ONLY,
	{0}, 0xFF, 0x7F},
    {"Remote TLOW limit",	MAX1617_CMD_RRLS,	READ_ONLY,
	{0}, 0xFF, 0xC9},
    {"Status byte",		MAX1617_CMD_RSL,	READ_ONLY,
	{0}, 0xFF, 0x00},
    {"Configuration byte",	MAX1617_CMD_RCL,	READ_ONLY,
	{0}, 0xFF, 0x00},
    {"Conversion rate byte",	MAX1617_CMD_RCRA,	READ_ONLY,
	{0}, 0xFF, 0x02},
    {"Configuration byte",	MAX1617_CMD_WCA,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Conversion rate byte",	MAX1617_CMD_WCRW,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Local THIGH limit",	MAX1617_CMD_WLHO,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Local TLOW limit",	MAX1617_CMD_WLLM,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Remote THIGH limit",	MAX1617_CMD_WRHA,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Remote TLOW limit",	MAX1617_CMD_WRLN,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"One-shot command",	MAX1617_CMD_OSHT,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Software POR",		MAX1617_CMD_SPOR,	WRITE_ONLY,
	{0}, 0xFF, 0x00},
    {"Manufacturer ID code",	MAX1617_CMD_MFGID,	READ_ONLY,
	{0}, 0xFF, 0x4D},
    {"Device ID code",		MAX1617_CMD_DEVID,	READ_ONLY,
	{0}, 0xFF, 0x01},
    {0, 0, 0, {0}, 0, 0},
};

/* Peek-n-poke registers read/write command conversion */
static max1617a_rd_wr_t rd_wr_table[] =
{
    {MAX1617_CMD_RCL, MAX1617_CMD_WCA},
    {MAX1617_CMD_RCRA, MAX1617_CMD_WCRW},
    {MAX1617_CMD_RLHN, MAX1617_CMD_WLHO},
    {MAX1617_CMD_RLLI, MAX1617_CMD_WLLM},
    {MAX1617_CMD_RRHI, MAX1617_CMD_WRHA},
    {MAX1617_CMD_RRLS, MAX1617_CMD_WRLN},
    {MAX1617_CMD_OSHT, MAX1617_CMD_OSHT},
    {MAX1617_CMD_SPOR, MAX1617_CMD_SPOR},
    {MAX1617_CMD_DEVID, MAX1617_CMD_DEVID},	/* read/read as terminator */
};

/* Conversion-Rate Control Byte text string */
static conv_rate_t conversion_rate_table[MAX1617_CRA_8HZ + 1] =
{
    {"0.0625"},
    {"0.125"},
    {"0.25"},
    {"0.5"},
    {"1"},
    {"2"},
    {"4"},
    {"8"},
};

/*****************************************************************
 *
 * Name:	max1617a_dev_create()
 *
 * Description:	Create object with various device function
 *		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the Max1617A device.
 *		error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************/
void
max1617a_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		    NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in max1617a_dev_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    snsr->base.dev_object_fvt->dev_attach	= dev_1617_attach;
    snsr->base.dev_object_fvt->dev_detach	= dev_1617_detach;
    snsr->base.dev_object_fvt->dev_reconfig_needed = dev_1617_reconfig;
    snsr->base.dev_object_fvt->dev_restart	= dev_1617_restart;
    snsr->base.dev_object_fvt->dev_init		= dev_1617_init;
    snsr->base.dev_object_fvt->dev_show		= dev_1617_show;
    snsr->base.dev_object_fvt->dev_error_report	= error_report_fn;
    snsr->base.dev_object_fvt->dev_collect_crashinfo = dev_1617_crsh;
    snsr->base.dev_object_fvt->dev_destroy	= dev_1617_destroy;
    snsr->base.dev_object_fvt->dev_name	= "Max1617A Diode Sensor";

    snsr->callin_fvt = (snsr_callin_fvt_t *)
                                malloc(sizeof(snsr_callin_fvt_t));
    snsr->callout_fvt = (snsr_callout_fvt_t *)
                                malloc(sizeof(snsr_callout_fvt_t));

    snsr->base.dev_state = DEV_STATE_CREATE;

}

/*****************************************************************
 *
 * Name:	dev_1617_attach()
 *
 * Description:	Attach the Max1617A device for use. This function will
 *		initialize and setup all necessary pointers and bring the
 *		chip to operation.
 *
 * Input:	Pointer to the Max1617A device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_1617_attach (dev_object_t *dev)
{
    uint32 rc;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    if (snsr->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_1617_attach() callin malloc",
			 SNSR_ATTACH);
	return(FAILED);
    }

    if (snsr->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_1617_attach() callout malloc",
			 SNSR_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    snsr->callin_fvt->peek_n_poke = max1617a_alter_reg;
    snsr->callin_fvt->clear_alert = max1617a_clear_alert;
    snsr->callin_fvt->show_temp   = max1617a_show_temp;

    /* Lock the I2C device */
    if ((rc = snsr->callout_fvt->open(snsr->i2c_p)) != PASSED) {
        sprintf(err_buf, "dev_1617_attach() I2C open return rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, SNSR_ATTACH);
        return(FAILED);
    }

    snsr->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*****************************************************************
 *
 * Name:	dev_1617_detach()
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
 * Input:	Pointer to the Max1617A device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_1617_detach (dev_object_t *dev)
{
    uint32 rc;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *) dev;
    char err_buf[ERR_BUF_SIZE];

    /* Unlock the I2C device */
    if ((rc = snsr->callout_fvt->close(snsr->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_1617_detach() I2C close rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, SNSR_DETACH);
	return(FAILED);
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, snsr->base.dev_object_fvt);

    snsr->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/*****************************************************************
 * Name:	dev_1617_reconfig_needed
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
 * Input:	dev_object_t pointer to the Max1617A device
 *		void * - a device/platform specific context handle
 *		boolean * - a pointer to a boolean
 *
 * Returns:	PASSED/FAILED, context information and a boolean value.
 *		The boolean value shall be set to TRUE if the device must be
 *		reconfigured from scratch and it shall be set to FALSE otherwise
 *
 * Assumptions:	The dev_attach() function has been called and successfully
 *****************************************************************/
static uint32
dev_1617_reconfig(dev_object_t *dev, void *context_handle, boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_1617_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		the device or changing its configuration.
 *		For example, during a failover event.
 *
 *		Change the state of the device from its current state
 *		to an initial state. Also, dev_state must be assigned the
 *		value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the Max1617A device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		called and successfully executed.
 *****************************************************************/
static uint32
dev_1617_restart(dev_object_t *dev)
{
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *) dev;

    snsr->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/*****************************************************************
 *
 * Name:	dev_1617_init()
 *
 * Description:	Initializes the Max1617A chip
 *
 * Input:	dev_object_t pointer to the Max1617A device.
 *		Caller has to setup the i2c_p parameters with init values.
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************/
static uint32
dev_1617_init (dev_object_t *dev)
{
    uint32 rc;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *)dev;
    snsr_callout_fvt_t *callout_p = snsr->callout_fvt;
    n2g_i2c_if_t new_i2c_if;
    reg_info_t *init_p;
    char err_buf[ERR_BUF_SIZE];

    /* Initialize the new I2C API interface struct */
    new_i2c_if.size = sizeof(sn_d);
    new_i2c_if.i2c_bus_no = snsr->i2c_p->i2c_bus_no;
    new_i2c_if.i2c_dev = snsr->i2c_p->i2c_dev;

    init_p = snsr->init_p;	/* points to the first init table entry */

    /* If name string pointer is NULL, it is the end of the init table.
     */
    while (init_p->name) {
	new_i2c_if.buf = (char *)&init_p->reset_val;
	new_i2c_if.offset = init_p->offset;
	/* Write to the device with new init value */
	rc = callout_p->wr(&new_i2c_if);

	if (rc != PASSED) {
	    sprintf(err_buf, "dev_1617_init() write %#x to addr %#x rc %#x\n",
				init_p->reset_val, init_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, SNSR_INIT);
	    return(FAILED);
	}
	init_p++;
    }

    snsr->base.dev_state = DEV_STATE_INIT;

    return(PASSED);

}

/*****************************************************************
 * Name:	dev_1617_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		device information via the device print function argument.
 *
 * Input:	dev_object_t pointer to the Max1617A device
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
dev_1617_show(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd cmd)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *)dev;
    snsr_callout_fvt_t *callout_p = snsr->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    sn_d data;
    char temp;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = (char *)&data;
    new_i2c_if.i2c_bus_no = snsr->i2c_p->i2c_bus_no;
    new_i2c_if.i2c_dev = snsr->i2c_p->i2c_dev;

    dev_print("\nMaxim1617A Registers:\n");

    /* Points to the beginning of the registers table */
    reg_p = &sensor_reg_table[0];

    /* Read registers */
    while (reg_p->name) {
	if (reg_p->type & WRITE_ONLY) {
	    /* Write only register cannot be displayed */
	} else {
	    /* Readable register */
	    new_i2c_if.offset = reg_p->offset;
	    new_i2c_if.size = sizeof(data);

	    rc = (*callout_p->rd)(&new_i2c_if);

	    if (rc != PASSED) {
		sprintf(err_buf, "dev_1617_show() read %s @ %#x rc = %#x",
					reg_p->name, reg_p->offset, rc);
		DEV_ERROR_REPORT(dev, err_buf, SNSR_SHOW);
		return(FAILED);
	    }

	    /* Display Sensor info */
	    switch (cmd) {
	    case DEV_SHOW_ALL:
	    case DEV_SHOW_CONFIG:
	    case DEV_SHOW_REGISTERS:

		switch (reg_p->offset) {
		case MAX1617_CMD_RLTS:
		case MAX1617_CMD_RLHN:
		case MAX1617_CMD_RLLI:
		case MAX1617_CMD_RRTE:
		case MAX1617_CMD_RRHI:
		case MAX1617_CMD_RRLS:
		    temp = (char)data;
		    dev_print("%s is %d degrees C.\n", reg_p->name, temp);
		    break;
		case MAX1617_CMD_RSL:
		case MAX1617_CMD_MFGID:
		case MAX1617_CMD_DEVID:
		    dev_print("%s is 0x%02x\n", reg_p->name, data);
		    break;
		case MAX1617_CMD_RCL:
		    dev_print("%s is 0x%02x\n", reg_p->name, data);
		    dev_print("    Alert interrupt %s.\n",
			(data & MAX1617_RCL_MASK) ? " Disabled" : " Enabled");
		    dev_print("    %s mode.\n", (data & MAX1617_RCL_STOP) ?
			"Standby" : "One-shot or timer");
		    break;
		case MAX1617_CMD_RCRA:
		    dev_print("%s is 0x%02x - ", reg_p->name, data);
		    if (data > MAX1617_CRA_8HZ) {
			dev_print("Reserved\n");
		    } else {
			dev_print("%s Hz\n", conversion_rate_table[data].string);
		    }
		    break;
		default:
		    sprintf(err_buf, "dev_1617_show() Invalid command %02x",
					reg_p->offset);
		    DEV_ERROR_REPORT(dev, err_buf, SNSR_SHOW);
		    return (FAILED);
		    break;
		} /* endof switch(offset) */
		break;
	    case DEV_SHOW_BRIEF:
		dev_print("@ %#x = 0x%02X ", reg_p->offset, data);
		break;
	    default:
		assert(!"dev_1617_show");
		break;
	    } /* endof switch(cmd) */
	} /* endof if (WRITE_ONLY) */

	reg_p++;	/* Get next register */
    } /* endof while */

    return(PASSED);
}

/*****************************************************************
 * Name:	dev_1617_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		Print data to the crash log (via the provide print error) using
 *		the appropriate verbisity level requested by the host
 *
 * Input:	dev_object_t pointer to the Max1617A device
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
dev_1617_crsh(dev_object_t *dev, print_fn_t dev_print, dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("dev_1617_crsh(): No Crash info available for Max1617A\n");
    return(PASSED);
}

/*****************************************************************
 * Name:	dev_1617_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the Max1617A device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************/
static void
dev_1617_destroy(dev_object_t **dev)
{
    uint32_t rc;
    dev_max1617a_object_t *snsr;
    char err_buf[ERR_BUF_SIZE];

    if (dev == NULL) {
	return;
    }

    if (*dev == NULL) {
	return;
    }

    snsr = (dev_max1617a_object_t *)*dev;

    /* Unlock the I2C device */
    if ((rc = snsr->callout_fvt->close(snsr->i2c_p)) != PASSED) {
	sprintf(err_buf, "dev_1617_destroy() I2C close ret. code %#x", rc);
	DEV_ERROR_REPORT(*dev, err_buf, SNSR_DESTROY);
	return;
    }

    if (snsr->callout_fvt) {
	free(snsr->callout_fvt);	/* Free callout struct */
    }

    if (snsr->callin_fvt) {
	free(snsr->callin_fvt);		/* Free callin struct */
    }

    free(snsr->base.dev_object_fvt);	/* Free dev_object_t */
}

/********************************************************************
 *
 * Function:	max1617a_alter_reg
 *
 * Description:	Peek-n-poke Max1617A register.
 *
 * Inputs:	dev_object_t pointer to the Max1617A device
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
max1617a_alter_reg(dev_object_t *dev, print_fn_t dev_print)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    reg_info_t *reg_p;
    max1617a_rd_wr_t *rd_wr_p;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *)dev;
    snsr_callout_fvt_t *callout_p = snsr->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    sn_c cmd;
    sn_d old_data, new_data;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(sn_d);
    new_i2c_if.i2c_bus_no = snsr->i2c_p->i2c_bus_no;
    new_i2c_if.i2c_dev = snsr->i2c_p->i2c_dev;

    dev_print("\nRegister number:\n");

    /* Parse through the register table to search for writeable registers */
    reg_p = &sensor_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name) {
	if (reg_p->type & READ_ONLY) {
	    /* Read only register */
	} else {
	    /* Write only or read/write register */
	    dev_print("   %02x - %s\n", reg_p->offset, reg_p->name);
	}
	reg_p++;	/* update the register table pointer */
    } /* endof while */

    /* Get the register to peek-n-poke */
    cmd = gethex_answer("Enter the register number:", 0, 0, MAX1617_CMD_DEVID);

    /* Check if the register is read/writeable */
    /* Traverse through read/write conversion table. Using device ID register
     * which is read only command, as the terminator.
     */ 
    rd_wr_p = &rd_wr_table[0];

    while (rd_wr_p->wr != MAX1617_CMD_DEVID) {
	if (cmd == rd_wr_p->wr) {
	    /* Valid register */
	    break;	/* Found it */
	}
	rd_wr_p++;	/* Update to the next entry */
    } /* endof while */

    if (rd_wr_p->wr == MAX1617_CMD_DEVID) {
	/* Register read/write pair not found */
	sprintf(err_buf,"max1617a_alter_reg() - %#x not writeable", cmd);
	DEV_ERROR_REPORT(dev, err_buf, SNSR_ALTER);
	return(FAILED);
    }

    /* Find the register text in the register table */
    reg_p = &sensor_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != cmd) {
	/* Not requested register */
	reg_p++;	/* update the register table pointer */
    }

    /* Got the read/write pair. Some registers cannot be read, but writeable */
    switch(cmd) {
    case MAX1617_CMD_OSHT:
    case MAX1617_CMD_SPOR:
	/* Write only register */
	old_data = 0;
	break;
    default:
	/* Readable registers. Read the register first. */
	new_i2c_if.buf = (char *)&old_data;
	new_i2c_if.offset = rd_wr_p->rd;

	rc = (*callout_p->rd)(&new_i2c_if);
	if (rc != PASSED) {
	    sprintf(err_buf, "max1617a_alter_reg() read %s cmd %#x rc = %#x",
					reg_p->name, reg_p->offset, rc);
	    DEV_ERROR_REPORT(dev, err_buf, SNSR_ALTER);
	    return(FAILED);
	}
	/* Got the data */
	break;
    } /* endof switch cmd */

    /* Get the new data */
    new_data = gethex_answer("Enter the data:", old_data, 0, 0xFF);

    /* Write the new data */
    new_i2c_if.buf = (char *)&new_data;
    new_i2c_if.offset = rd_wr_p->wr;

#ifdef SNSR_DEBUG
    printf("\nwrite %#x with cmd %#x\n", new_data, new_i2c_if.offset);
#endif /* SNSR_DEBUG */

    rc = (*callout_p->wr)(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "max1617a_alter_reg() write %s cmd %#x rc = %#x",
					reg_p->name, reg_p->offset, rc);
	DEV_ERROR_REPORT(dev, err_buf, SNSR_ALTER);
    }

    return(rc);
}

/********************************************************************
 *
 * Function:    max1617a_alter_reg
 *
 * Description: Peek-n-poke Max1617A register.
 *
 * Inputs:      dev_object_t pointer to the Max1617A device
 *              A device print function vector
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *********************************************************************
 */
static int
max1617a_clear_alert(dev_object_t *dev, print_fn_t dev_print)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *)dev;
    snsr_callout_fvt_t *callout_p = snsr->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    sn_d data;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(sn_d);
    new_i2c_if.i2c_bus_no = snsr->i2c_p->i2c_bus_no;
    new_i2c_if.i2c_dev = snsr->i2c_p->i2c_dev;
    new_i2c_if.buf = (char *)&data;
    new_i2c_if.offset = 0;	/* not used, but clear it anyway */

    rc = (*callout_p->rd)(&new_i2c_if);
    if (rc != PASSED) {
	sprintf(err_buf, "max1617a_clear_alert() read failed. rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, SNSR_ALERT);
	return(FAILED);
    }

    dev_print("max1617a_clear_alert(): Address %#x generates alert\n", data);
    return(PASSED);
}

/********************************************************************
 *
 * Function:	max1617a_show_temp
 *
 * Description:	Display Sensor Diode (Max1617A) temperatures.
 *
 * Inputs:	dev_object_t pointer to the Max1617A device
 *		A device print function vector
 *		Display format defined in display_format_t in common.h
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 * Assumptions:
 *
 *********************************************************************
 */
static int
max1617a_show_temp(dev_object_t *dev, print_fn_t dev_print, int format)
{
    n2g_i2c_if_t new_i2c_if;
    uint32_t rc;
    dev_max1617a_object_t *snsr = (dev_max1617a_object_t *)dev;
    snsr_callout_fvt_t *callout_p = snsr->callout_fvt;
    char err_buf[ERR_BUF_SIZE];
    sn_d data;
    char temp;

    /* Setup I2C API interface struct */
    new_i2c_if.size = sizeof(data);
    new_i2c_if.buf = (char *)&data;
    new_i2c_if.i2c_bus_no = snsr->i2c_p->i2c_bus_no;
    new_i2c_if.i2c_dev = snsr->i2c_p->i2c_dev;

#ifdef READ_LOCAL_TEMP
    /* Read local temperature */
    new_i2c_if.offset = MAX1617_CMD_RLTS;

    rc = (*callout_p->rd)(&new_i2c_if);

    if (rc != PASSED) {
	sprintf(err_buf, "max1617a_show_temp() read Local temperature register "
			 "failed with rc = %#x", rc);
	DEV_ERROR_REPORT(dev, err_buf, SNSR_SHOW_TEMP);
	return(FAILED);
    }

    temp = (char)data;

    switch(format) {
    case DISPLAY_M2M:
	dev_print("MAX1617LTEMP:%dC\n", temp);
	break;
    case DISPLAY_HCI:
    default:
	dev_print("\nMax1617A chip temperature is %d degrees Celsius\n", temp);
	break;
    } /* endof switch */
#endif /* READ_LOCAL_TEMP */

    /* Read remote temperature */
    new_i2c_if.offset = MAX1617_CMD_RRTE;

    rc = (*callout_p->rd)(&new_i2c_if);

    if (rc != PASSED) {
        sprintf(err_buf, "max1617a_show_temp() read Remote temperature "
			 " register failed with rc = %#x", rc);
        DEV_ERROR_REPORT(dev, err_buf, SNSR_SHOW_TEMP);
        return(FAILED);
    }

    temp = (char)data;

    switch(format) {
    case DISPLAY_M2M:
	dev_print("MAX1617RTEMP:%dC\n", temp);
	break;
    case DISPLAY_HCI:
    default:
	dev_print("CPU die temperature is %d degrees Celsius\n", temp);
	break;
    } /* endof switch */

    return(PASSED);
}

/*------------------------------------------------------------------
$Log: dev_max1617a.c,v $
Revision 1.2  2012/03/28 00:38:08  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:02  ptong
Initial archive of ng_diag module


$Endlog$
*/
