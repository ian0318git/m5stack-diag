/* $Id: dev_pca9555.c,v 1.4 2019/08/05 02:52:17 alpeng Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pca9555/dev_pca9555.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_pca9555.c
 *
 * Description:	16-bit GPIO Expander PCA9555 Device Driver
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "common.h"
#include "dev_pca9555.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#ifdef LINUX_APP
#include <assert.h>
#endif

static uint32 dev_pca9555_attach(dev_object_t *);
static uint32 dev_pca9555_detach(dev_object_t *);
static uint32 dev_pca9555_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_pca9555_restart(dev_object_t *);
static int dev_pca9555_show(dev_object_t *);
static uint32 dev_pca9555_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void dev_pca9555_destroy(dev_object_t **);
static int dev_pca9555_alter_reg(dev_object_t *);
static int dev_pca9555_test_reg(dev_object_t *dev);
static int dev_i2c_wr(ulong, int, ulong, void *);
static int dev_i2c_rd(ulong addr, int, ulong *, void *);
static int dev_pca9555_config_port(dev_object_t *, int, int, int);
static int dev_pca9555_drive_port(dev_object_t *, int, int, int);
static int dev_pca9555_read_port(dev_object_t *, int, int, int *);

void pca9555_dev_create(dev_object_t *, dev_error_report_t);

static char pca9555_err_buf[PCA9555_ERR_BUF_SIZE];

/* Global variables */

/* Registers test table */
reg_info_t_ext pca9555_reg_ext = {2, dev_i2c_rd, dev_i2c_wr, 0};

/* PCA9555 registers table. This device has registers with different sizes.
 */
reg_info_t default_pca9555_reg_table[] =
{
    {"Input Port 0",		INPUT_PORT_0,	PCA9555_REG_RO_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0x00},
    {"Input Port 1",		INPUT_PORT_1,	PCA9555_REG_RO_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0x00},
    {"Output Port 0",		OUTPUT_PORT_0,	PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0xFF},
    {"Output Port 1",		OUTPUT_PORT_1,	PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0xFF},
    {"Polarity Inversion Port 0",		POLARITY_INV_PORT_0,	PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0x00},
    {"Polarity Inversion Port 1",		POLARITY_INV_PORT_1,	PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0x00},
    {"Configuration Port 0",		CONFIG_PORT_0,	PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},	0x00, 0xFF},
    {"Configuration Port 1",		CONFIG_PORT_1,	PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},	0xFF, 0xFF},
    {0, 0, 0, {0}, 0, 0},
};

reg_info_t* pca9555_reg_map;

/******************************************************************************
 *
 * Name:	pca9555_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the PCA9555 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void pca9555_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in pca9555_dev_create()", 0);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    pca9555->base.dev_object_fvt->dev_attach	= dev_pca9555_attach;
    pca9555->base.dev_object_fvt->dev_detach	= dev_pca9555_detach;
    pca9555->base.dev_object_fvt->dev_reconfig_needed = dev_pca9555_reconfig;
    pca9555->base.dev_object_fvt->dev_restart	= dev_pca9555_restart;
    pca9555->base.dev_object_fvt->dev_error_report	= error_report_fn;
    pca9555->base.dev_object_fvt->dev_collect_crashinfo = dev_pca9555_crsh;
    pca9555->base.dev_object_fvt->dev_destroy	= dev_pca9555_destroy;
    pca9555->base.dev_object_fvt->dev_name	= "PCA9555 16-bit GPIO Expander";

    pca9555->callin_fvt = (pca9555_callin_fvt_t *)
                           malloc(sizeof(pca9555_callin_fvt_t));
    pca9555->callout_fvt = (pca9555_callout_fvt_t *)
                            malloc(sizeof(pca9555_callout_fvt_t));

    pca9555->base.dev_state = DEV_STATE_CREATE;
}

/******************************************************************************
 *
 * Name:	dev_pca9555_attach()
 *
 * Description:	Attach the PCA9555 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the PCA9555 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_pca9555_attach (dev_object_t *dev)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;

    if (pca9555->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_pca9555_attach() callin malloc", PCA9555_ATTACH);
        return (FAILED);
    }

    if (pca9555->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_pca9555_attach() callout malloc", PCA9555_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    pca9555->callin_fvt->register_test  = dev_pca9555_test_reg;
    pca9555->callin_fvt->dump_register  = dev_pca9555_show;
    pca9555->callin_fvt->alter_register = dev_pca9555_alter_reg;
    pca9555->callin_fvt->config_port    = dev_pca9555_config_port;
    pca9555->callin_fvt->drive_port     = dev_pca9555_drive_port;
    pca9555->callin_fvt->read_port      = dev_pca9555_read_port;

    pca9555->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 * Name:	dev_pca9555_detach()
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
 * Input:	Pointer to the PCA9555 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_pca9555_detach (dev_object_t *dev)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, pca9555->base.dev_object_fvt);

    pca9555->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_pca9555_reconfig_needed
 *
 * Description:	To check whether device re-configuration is needed during
 *		        (re)initialization. Based on the provided context information,
 *		        the boolean return value, and possibly other factors external
 *	        	to the device object, the caller shall decide whether to invoke
 *        		either dev_restart or dev_init, but not both. In general, the
 *        		boolean return value alone is not sufficient to decide whether
 *        		the device can safely be restarted or whether it must be fully
 *        		initialized from scratch.
 *
 * Input:	dev_object_t pointer to the PCA9555 device
 *		    void * - a device/platform specific context handle
 *		    boolean * - a pointer to a boolean
 *
 * Returns:	PASSED/FAILED, context information and a boolean value.
 *		    The boolean value shall be set to TRUE if the device must be
 *		    reconfigured from scratch and it shall be set to FALSE otherwise
 *
 * Assumptions:	The dev_attach() function has been called and successfully
 *
 *****************************************************************************/
static uint32 dev_pca9555_reconfig (dev_object_t *dev, void *context_handle,
                                   boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_pca9555_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the PCA9555 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_pca9555_restart (dev_object_t *dev)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;

    pca9555->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_pca9555_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		        device information via the device print function argument.
 *
 * Input:	dev_object_t pointer to the PCA9555 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		        platform which implements the print logging functionality. The
 *		        dev_attach() function has been called and successfully executed
 *
 *****************************************************************************/
static int dev_pca9555_show (dev_object_t *dev)
{
    pca9555_reg_ext.param = (void *)dev;

    if (register_display(0, default_pca9555_reg_table) == FAILED) {
        sprintf(pca9555_err_buf, "%s: PCA9555 register display fail",
                              __FUNCTION__);
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_DISPLAY);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Name:	dev_pca9555_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		        Print data to the crash log (via the provide print error) using
 *		        the appropriate verbisity level requested by the host
 *
 * Input:	dev_object_t pointer to the PCA9555 device
 *		    A crash print function vector.
 *		    A verbosity level.
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	A device print function vector has been provided by the host
 *		        platform which implements the crash logging functionality. It
 *		        could be the mechanism to log info to the Compact Flash before
 *		        the device crash and now retrieve them. The dev_attch()
 *		        function has been called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_pca9555_crsh (dev_object_t *dev, print_fn_t dev_print,
                                dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("dev_pca9555_crsh(): No Crash info available for PCA9555\n");
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_pca9555_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the PCA9555 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_pca9555_destroy (dev_object_t **dev)
{
    dev_pca9555_object_t *pca9555;;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    pca9555 = (dev_pca9555_object_t *)*dev;

    if (pca9555->callout_fvt) {
        free(pca9555->callout_fvt);	/* Free callout struct */
    }

    if (pca9555->callin_fvt) {
        free(pca9555->callin_fvt);		/* Free callin struct */
    }

    free(pca9555->base.dev_object_fvt);	/* Free dev_object_t */
}


/******************************************************************************
 *
 * Function:	dev_pca9555_alter_reg
 *
 * Description:	Alter PCA9555 register.
 *
 * Inputs:	dev_object_t pointer to the PCA9555 device
 *		    A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9555_alter_reg(dev_object_t *dev)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0x07): ",
                              0x0, 0x0, CONFIG_PORT_1);

    if (pca9555->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    printf("Original value: reg %#.2x, data %#.2x\n", reg_addr, reg_data);

    /* alter register with new msb value */
    reg_data = gethex_answer("Enter the new data (hex): ", reg_data, 0, 0xFFFF);

    if (pca9555->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    if (pca9555->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    printf("Register: reg %#.2x, data %#.2x\n", reg_addr, reg_data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_pca9555_test_reg
 *
 * Description: Tests the PCA9555 registers.
 *              Restores original register values after test.
 *
 * Inputs:  dev_object_t pointer to the PCA9555 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9555_test_reg (dev_object_t *dev)
{
    pca9555_reg_ext.param = (void *)dev;
    int stat;

    if (pca9555_reg_map == NULL) {
        stat = register_tests(0, default_pca9555_reg_table);
    } else {
        stat = register_tests(0, pca9555_reg_map);
    }

    if (stat == FAILED) {
        sprintf(pca9555_err_buf, "%s: PCA9555 Register Test fail",
                __FUNCTION__);
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_REG_TEST);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:    dev_i2c_rd
 *
 * This function: read PCA9555 register
 *
 * Input :  addr - offset of register to be written.
 *          size - number of bytes to write.
 *          data  - write data.
 *          param - Pointer to the PCA9555 device object
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_rd (ulong addr, int size, ulong *buf, void *param)
{
    ulong byte_addr = addr;
    int rc;

    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *)param;
    rc = pca9555->callout_fvt->rd((uint32)byte_addr, (ushort *)buf);

    return (rc);
}

/******************************************************************************
 *
 * Function:    dev_i2c_wr
 *
 * This function: write PCA9555 register
 *
 * Input :  addr - offset of register to be read.
 *          size - number of bytes to be read.
 *          buf  - points to the data buffer to be read.
 *          param - Pointer to the PCA9555 device object
 *
 * Output:  PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_wr (ulong addr, int size, ulong data, void *param)
{
    uint32 byte_addr = (uint32)addr;
    ulong byte_data = data;

    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *)param;
    return (pca9555->callout_fvt->wr(byte_addr, (ushort *)&byte_data));
}


/******************************************************************************
 *
 * Function:	dev_pca9555_config_port
 *
 * Description:	Configure Port direction (Input/Output)
 *
 * Inputs:	dev_object_t pointer to the PCA9555 device print function vector
 *          which_port - Port 0 or Port 1
 *          which_pin - 0 ~ 7
 *          direction - Input / Output
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9555_config_port (dev_object_t *dev, int which_port, 
                                    int which_pin, int direction)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    /* Assign register address based on port number */
    if (which_port == PORT_0) {
        reg_addr = CONFIG_PORT_0;
    } else {
        reg_addr = CONFIG_PORT_1;
    }

    /* Read the register */
    if (pca9555->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    /* Clear the register if the direction is output */
    if (direction == PORT_DIR_OUTPUT) {
        reg_data &= ~(0x1 << which_pin);
    } else {
        reg_data |= (0x1 << which_pin);
    }

    /* Now, write the register */
    if (pca9555->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:	dev_pca9555_drive_port
 *
 * Description:	Drive output port to High/Low
 *              (Assuming the port is already configured to Output)
 *
 * Inputs:	dev_object_t pointer to the PCA9555 device print function vector
 *          which_port - Port 0 or Port 1
 *          which_pin - 0 ~ 7
 *          value - 0 for Low, 1 for High
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9555_drive_port (dev_object_t *dev, int which_port,
                                   int which_pin, int value)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    /* Assign register address based on port number */
    if (which_port == PORT_0) {
        reg_addr = OUTPUT_PORT_0;
    } else {
        reg_addr = OUTPUT_PORT_1;
    }

    /* Read the register */
    if (pca9555->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    if (value == PORT_VAL_LOW) {
        reg_data &= ~(0x1 << which_pin);
    } else {
        reg_data |= (0x1 << which_pin);
    }

    /* Now, write the register */
    if (pca9555->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:	dev_pca9555_read_port
 *
 * Description:	Read the current value of the port (High/Low)
 *              (Assuming the port is already configured to Input)
 * 
 * Inputs:	dev_object_t pointer to the PCA9555 device print function vector
 *          which_port - Port 0 or Port 1
 *          which_pin - 0 ~ 7
 *          *value - 0 for Low, 1 for High
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9555_read_port (dev_object_t *dev, int which_port,
                                  int which_pin, int *value)
{
    dev_pca9555_object_t *pca9555 = (dev_pca9555_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    /* Assign register address based on port number */
    if (which_port == PORT_0) {
        reg_addr = INPUT_PORT_0;
    } else {
        reg_addr = INPUT_PORT_1;
    }

    /* Read the register */
    if (pca9555->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9555_err_buf, PCA9555_ALTER);
        return (FAILED);
    }

    if ((reg_data >> which_pin) & 0x1) {
        *value = PORT_VAL_HIGH;
    } else {
        *value = PORT_VAL_LOW;
    }

    return (PASSED);
}

