/* $Id: dev_pca9557.c,v 1.3 2019/08/05 02:52:17 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_pca9557/dev_pca9557.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_pca9557.c
 *
 * Description:	16-bit GPIO Expander PCA9557 Device Driver
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "defs.h"
#include "common.h"
#include "dev_pca9557.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#ifdef LINUX_APP
#include <assert.h>
#endif

static uint32 dev_pca9557_attach(dev_object_t *);
static uint32 dev_pca9557_detach(dev_object_t *);
static uint32 dev_pca9557_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_pca9557_restart(dev_object_t *);
static int dev_pca9557_show(dev_object_t *);
static uint32 dev_pca9557_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void dev_pca9557_destroy(dev_object_t **);
static int dev_pca9557_alter_reg(dev_object_t *);
static int dev_pca9557_test_reg(dev_object_t *dev);
static int dev_i2c_wr(ulong, int, ulong, void *);
static int dev_i2c_rd(ulong addr, int, ulong *, void *);
static int dev_pca9557_config_port(dev_object_t *, int, int, int);
static int dev_pca9557_drive_port(dev_object_t *, int, int, int);
static int dev_pca9557_read_port(dev_object_t *, int, int, int *);

void pca9557_dev_create(dev_object_t *, dev_error_report_t);

static char pca9557_err_buf[PCA9557_ERR_BUF_SIZE];

/* Global variables */
#define PCA9557_REG_RO_FLAG              (READ_ONLY | REG_ACCESS)
/* No REG_ACCESS here accroding to different reg size */
#define PCA9557_REG_RW_FLAG              (READ_WRITE| SAVE_RESTORE | REG_ACCESS)

/* Registers test table */
static reg_info_t_ext pca9557_reg_ext = {1, dev_i2c_rd, dev_i2c_wr, 0};

/* PCA9557 registers table. This device has registers with different sizes.
 */
reg_info_t pca9557_reg_table[] =
{
    {"Input Port",		INPUT_PORT,	PCA9557_REG_RO_FLAG,
            {(ulong)&pca9557_reg_ext},	0x00, 0x00},
    {"Output Port",		OUTPUT_PORT,	PCA9557_REG_RW_FLAG,
            {(ulong)&pca9557_reg_ext},	0x00, 0xFF},
    {"Polarity Inversion Port",		POLARITY_INV_PORT,	PCA9557_REG_RW_FLAG,
            {(ulong)&pca9557_reg_ext},	0x00, 0x00},
    {"Configuration Port",		CONFIG_PORT,	PCA9557_REG_RW_FLAG,
            {(ulong)&pca9557_reg_ext},	0x00, 0xFF},
    {0, 0, 0, {0}, 0, 0},
};


/******************************************************************************
 *
 * Name:	pca9557_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the PCA9557 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void pca9557_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in pca9557_dev_create()", 0);
        printf("%s: NULL\n", __func__);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    pca9557->base.dev_object_fvt->dev_attach	= dev_pca9557_attach;
    pca9557->base.dev_object_fvt->dev_detach	= dev_pca9557_detach;
    pca9557->base.dev_object_fvt->dev_reconfig_needed = dev_pca9557_reconfig;
    pca9557->base.dev_object_fvt->dev_restart	= dev_pca9557_restart;
    pca9557->base.dev_object_fvt->dev_error_report	= error_report_fn;
    pca9557->base.dev_object_fvt->dev_collect_crashinfo = dev_pca9557_crsh;
    pca9557->base.dev_object_fvt->dev_destroy	= dev_pca9557_destroy;
    pca9557->base.dev_object_fvt->dev_name	= "PCA9557 8-bit GPIO Expander";

    pca9557->callin_fvt = (pca9557_callin_fvt_t *)
                           malloc(sizeof(pca9557_callin_fvt_t));
    pca9557->callout_fvt = (pca9557_callout_fvt_t *)
                            malloc(sizeof(pca9557_callout_fvt_t));

    pca9557->base.dev_state = DEV_STATE_CREATE;
}

/******************************************************************************
 *
 * Name:	dev_pca9557_attach()
 *
 * Description:	Attach the PCA9557 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the PCA9557 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_pca9557_attach (dev_object_t *dev)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;

    if (pca9557->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_pca9557_attach() callin malloc", PCA9557_ATTACH);
        return (FAILED);
    }

    if (pca9557->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_pca9557_attach() callout malloc", PCA9557_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    pca9557->callin_fvt->register_test  = dev_pca9557_test_reg;
    pca9557->callin_fvt->dump_register  = dev_pca9557_show;
    pca9557->callin_fvt->alter_register = dev_pca9557_alter_reg;
    pca9557->callin_fvt->config_port    = dev_pca9557_config_port;
    pca9557->callin_fvt->drive_port     = dev_pca9557_drive_port;
    pca9557->callin_fvt->read_port      = dev_pca9557_read_port;

    pca9557->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_pca9557_detach()
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
 * Input:	Pointer to the PCA9557 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_pca9557_detach (dev_object_t *dev)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, pca9557->base.dev_object_fvt);

    pca9557->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_pca9557_reconfig_needed
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
 * Input:	dev_object_t pointer to the PCA9557 device
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
static uint32 dev_pca9557_reconfig (dev_object_t *dev, void *context_handle,
                                   boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_pca9557_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the PCA9557 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_pca9557_restart (dev_object_t *dev)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;

    pca9557->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_pca9557_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		        device information via the device print function argument.
 *
 * Input:	dev_object_t pointer to the PCA9557 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		        platform which implements the print logging functionality. The
 *		        dev_attach() function has been called and successfully executed
 *
 *****************************************************************************/
static int dev_pca9557_show (dev_object_t *dev)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;
    uint reg_addr;
    uchar reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register addres (0x0 ~ 0x03): ",
                              0x0, 0x0, CONFIG_PORT);

    if (pca9557->callout_fvt->rd(reg_addr, (char *)&reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    printf("Value: reg %#.2x, data %#.2x\n", reg_addr, reg_data);

    return (PASSED);
}

/******************************************************************************
 * Name:	dev_pca9557_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		        Print data to the crash log (via the provide print error) using
 *		        the appropriate verbisity level requested by the host
 *
 * Input:	dev_object_t pointer to the PCA9557 device
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
static uint32 dev_pca9557_crsh (dev_object_t *dev, print_fn_t dev_print,
                                dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("dev_pca9557_crsh(): No Crash info available for PCA9557\n");
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_pca9557_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the PCA9557 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_pca9557_destroy (dev_object_t **dev)
{
    dev_pca9557_object_t *pca9557;;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    pca9557 = (dev_pca9557_object_t *)*dev;

    if (pca9557->callout_fvt) {
        free(pca9557->callout_fvt);	/* Free callout struct */
    }

    if (pca9557->callin_fvt) {
        free(pca9557->callin_fvt);		/* Free callin struct */
    }

    free(pca9557->base.dev_object_fvt);	/* Free dev_object_t */
}


/******************************************************************************
 *
 * Function:	dev_pca9557_alter_reg
 *
 * Description:	Alter PCA9557 register.
 *
 * Inputs:	dev_object_t pointer to the PCA9557 device
 *		    A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9557_alter_reg(dev_object_t *dev)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;
    uint reg_addr;
    char reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register addres (0x0 ~ 0x03): ",
                              0x0, 0x0, CONFIG_PORT);

    if (pca9557->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    printf("Original value: reg %#.2x, data %#.2x\n", reg_addr, reg_data);

    /* alter register with new msb value */
    reg_data = gethex_answer("Enter the new data (hex): ", reg_data, 0, 0xFFFF);

    if (pca9557->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    if (pca9557->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    printf("Register: reg %#.2x, data %#.2x\n", reg_addr, reg_data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_pca9557_test_reg
 *
 * Description: Tests the PCA9557 registers.
 *              Restores original register values after test.
 *
 * Inputs:  dev_object_t pointer to the PCA9557 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_pca9557_test_reg (dev_object_t *dev)
{
    pca9557_reg_ext.param = (void *)dev;

    if (register_tests(0, pca9557_reg_table) == FAILED) {
        sprintf(pca9557_err_buf, "%s: PCA9557 Register Test fail",
                __FUNCTION__);
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_REG_TEST);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:    dev_i2c_rd
 *
 * This function: read PCA9557 register
 *
 * Input :  addr - offset of register to be written.
 *          size - number of bytes to write.
 *          data  - write data.
 *          param - Pointer to the PCA9557 device object
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_rd (ulong addr, int size, ulong *buf, void *param)
{
    ulong byte_addr = addr;
    int rc;

    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *)param;
    rc = pca9557->callout_fvt->rd((uint32)byte_addr, (char *)buf);

    return (rc);
}

/******************************************************************************
 *
 * Function:    dev_i2c_wr
 *
 * This function: write PCA9557 register
 *
 * Input :  addr - offset of register to be read.
 *          size - number of bytes to be read.
 *          buf  - points to the data buffer to be read.
 *          param - Pointer to the PCA9557 device object
 *
 * Output:  PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_wr (ulong addr, int size, ulong data, void *param)
{
    uint32 byte_addr = (uint32)addr;
    ulong byte_data = data;

    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *)param;
    return (pca9557->callout_fvt->wr(byte_addr, (char *)&byte_data));
}


/******************************************************************************
 *
 * Function:	dev_pca9557_config_port
 *
 * Description:	Configure Port direction (Input/Output)
 *
 * Inputs:	dev_object_t pointer to the PCA9557 device print function vector
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
static int dev_pca9557_config_port (dev_object_t *dev, int which_port, 
                                    int which_pin, int direction)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;
    uint reg_addr;
    char reg_data;

    reg_addr = CONFIG_PORT;

    /* Read the register */
    if (pca9557->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    /* Clear the register if the direction is output */
    if (direction == PORT_DIR_OUTPUT) {
        reg_data &= ~(0x1 << which_pin);
    } else {
        reg_data |= (0x1 << which_pin);
    }

    /* Now, write the register */
    if (pca9557->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:	dev_pca9557_drive_port
 *
 * Description:	Drive output port to High/Low
 *              (Assuming the port is already configured to Output)
 *
 * Inputs:	dev_object_t pointer to the PCA9557 device print function vector
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
static int dev_pca9557_drive_port (dev_object_t *dev, int which_port,
                                   int which_pin, int value)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;
    uint reg_addr;
    char reg_data;

    reg_addr = OUTPUT_PORT;

    /* Read the register */
    if (pca9557->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    if (value == PORT_VAL_LOW) {
        reg_data &= ~(0x1 << which_pin);
    } else {
        reg_data |= (0x1 << which_pin);
    }

    /* Now, write the register */
    if (pca9557->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:	dev_pca9557_read_port
 *
 * Description:	Read the current value of the port (High/Low)
 *              (Assuming the port is already configured to Input)
 * 
 * Inputs:	dev_object_t pointer to the PCA9557 device print function vector
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
static int dev_pca9557_read_port (dev_object_t *dev, int which_port,
                                  int which_pin, int *value)
{
    dev_pca9557_object_t *pca9557 = (dev_pca9557_object_t *) dev;
    uint reg_addr;
    char reg_data;

    reg_addr = INPUT_PORT;

    /* Read the register */
    if (pca9557->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, pca9557_err_buf, PCA9557_ALTER);
        return (FAILED);
    }

    if ((reg_data >> which_pin) & 0x1) {
        *value = PORT_VAL_HIGH;
    } else {
        *value = PORT_VAL_LOW;
    }

    return (PASSED);
}

