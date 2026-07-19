/* $Id: dev_tmpx75.c,v 1.3 2019/08/05 02:52:16 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_tmpx75/dev_tmpx75.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_tmpx75.c
 *
 * Description:	TMPx75 Digital Temperature Sensor Device Driver
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdlib.h>
#include "defs.h"
#include "common.h"
#include "dev_tmpx75.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#include "byteswap.h"
#ifdef LINUX_APP
#include <assert.h>
#endif

static uint32 dev_tmpx75_attach(dev_object_t *);
static uint32 dev_tmpx75_detach(dev_object_t *);
static uint32 dev_tmpx75_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_tmpx75_restart(dev_object_t *);
static uint32 dev_tmpx75_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void dev_tmpx75_destroy(dev_object_t **);
static int dev_tmpx75_alter_reg(dev_object_t *);
static int dev_tmpx75_test_reg(dev_object_t *dev);
static int dev_tmpx75_show(dev_object_t *);
static int dev_i2c_wr(ulong, int, ulong, void *);
static int dev_i2c_rd(ulong addr, int, ulong *, void *);
static int dev_tmpx75_show_temp(dev_object_t *);
void tmpx75_dev_create(dev_object_t *, dev_error_report_t);

static char tmpx75_err_buf[TMPX75_ERR_BUF_SIZE];

/* Global variables */
#define TMPX75_REG_RO_FLAG               (READ_ONLY | REG_ACCESS)
/* No REG_ACCESS here accroding to different reg size */
#define TMPX75_REG_RW_FLAG               (READ_WRITE| SAVE_RESTORE | REG_ACCESS)

/* Registers test table */
static reg_info_t_ext tmpx75_reg_ext = {2, dev_i2c_rd, dev_i2c_wr, 0};

/* TMPX75 registers table. This device has registers with different sizes.
 */
reg_info_t tmpx75_reg_table[] =
{
    {"Temperature",             TS_PTR_TEMP,    TMPX75_REG_RO_FLAG,
            {(ulong)&tmpx75_reg_ext},	0xFF80, 0x0000},
    {"Configuration",           TS_PTR_CFG,     TMPX75_REG_RO_FLAG,
            {(ulong)&tmpx75_reg_ext},	0x1F, 0x00},
    {"T-Low",    TS_PTR_THYST,   TMPX75_REG_RW_FLAG,
            {(ulong)&tmpx75_reg_ext},	0xF0FF, 0x0000},
    {"T-High",TS_PTR_TOS,     TMPX75_REG_RW_FLAG,
            {(ulong)&tmpx75_reg_ext},	0xF0FF, 0x0000},
    {0, 0, 0, {0}, 0, 0},
};


/******************************************************************************
 *
 * Name:	tmpx75_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the TMPX75 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void tmpx75_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		            NULL) {
        /* Unable to allocate memory */
        error_report_fn(dev, "malloc failure in tmpx75_dev_create()", 0);
	    return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    tmpx75->base.dev_object_fvt->dev_attach	= dev_tmpx75_attach;
    tmpx75->base.dev_object_fvt->dev_detach	= dev_tmpx75_detach;
    tmpx75->base.dev_object_fvt->dev_reconfig_needed = dev_tmpx75_reconfig;
    tmpx75->base.dev_object_fvt->dev_restart	= dev_tmpx75_restart;
    tmpx75->base.dev_object_fvt->dev_error_report	= error_report_fn;
    tmpx75->base.dev_object_fvt->dev_collect_crashinfo = dev_tmpx75_crsh;
    tmpx75->base.dev_object_fvt->dev_destroy	= dev_tmpx75_destroy;
    tmpx75->base.dev_object_fvt->dev_name	= "TMPX75/ADT75 Temperature Sensor";

    tmpx75->callin_fvt = (tmpx75_callin_fvt_t *)
                          malloc(sizeof(tmpx75_callin_fvt_t));
    tmpx75->callout_fvt = (tmpx75_callout_fvt_t *)
                           malloc(sizeof(tmpx75_callout_fvt_t));

    tmpx75->base.dev_state = DEV_STATE_CREATE;
}

/******************************************************************************
 *
 * Name:	dev_tmpx75_attach()
 *
 * Description:	Attach the TMPX75 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the TMPX75 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_tmpx75_attach (dev_object_t *dev)
{
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *) dev;

    if (tmpx75->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_tmpx75_attach() callin malloc", TMPX75_ATTACH);
        return (FAILED);
    }

    if (tmpx75->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "dev_tmpx75_attach() callout malloc", TMPX75_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    tmpx75->callin_fvt->register_test  = dev_tmpx75_test_reg;
    tmpx75->callin_fvt->dump_register  = dev_tmpx75_show;
    tmpx75->callin_fvt->alter_register = dev_tmpx75_alter_reg;
    tmpx75->callin_fvt->show_temp      = dev_tmpx75_show_temp;

    tmpx75->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_tmpx75_detach()
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
 * Input:	Pointer to the TMPX75 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_tmpx75_detach (dev_object_t *dev)
{
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, tmpx75->base.dev_object_fvt);

    tmpx75->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}


/******************************************************************************
 * Name:	dev_tmpx75_reconfig_needed
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
 * Input:	dev_object_t pointer to the TMPX75 device
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
static uint32 dev_tmpx75_reconfig (dev_object_t *dev, void *context_handle,
                                   boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_tmpx75_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the TMPX75 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_tmpx75_restart (dev_object_t *dev)
{
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *) dev;

    tmpx75->base.dev_state = DEV_STATE_INIT;
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_tmpx75_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		        device information via the device print function argument.
 *
 * Input:	dev_object_t pointer to the TMPX75 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		        platform which implements the print logging functionality. The
 *		        dev_attach() function has been called and successfully executed
 *
 *****************************************************************************/
static int dev_tmpx75_show (dev_object_t *dev)
{
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0x04): ",
                              0x0, 0x0, TS_PTR_OS);

    if (tmpx75->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmpx75_err_buf, TMPX75_ALTER);
        return (FAILED);
    }

    printf("Register: reg %#x, data %#x\n", reg_addr, reg_data);

    return (PASSED);
}

/******************************************************************************
 * Name:	dev_tmpx75_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		        Print data to the crash log (via the provide print error) using
 *		        the appropriate verbisity level requested by the host
 *
 * Input:	dev_object_t pointer to the TMPX75 device
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
static uint32 dev_tmpx75_crsh (dev_object_t *dev, print_fn_t dev_print,
                                dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("dev_tmpx75_crsh(): No Crash info available for TMPX75\n");
    return (PASSED);
}


/******************************************************************************
 * Name:	dev_tmpx75_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the TMPX75 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_tmpx75_destroy (dev_object_t **dev)
{
    dev_tmpx75_object_t *tmpx75;;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    tmpx75 = (dev_tmpx75_object_t *)*dev;

    if (tmpx75->callout_fvt) {
        free(tmpx75->callout_fvt);	/* Free callout struct */
    }

    if (tmpx75->callin_fvt) {
        free(tmpx75->callin_fvt);		/* Free callin struct */
    }

    free(tmpx75->base.dev_object_fvt);	/* Free dev_object_t */
}


/******************************************************************************
 *
 * Function:	dev_tmpx75_alter_reg
 *
 * Description:	Alter TMPX75 register.
 *
 * Inputs:	dev_object_t pointer to the TMPX75 device
 *		    A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_tmpx75_alter_reg(dev_object_t *dev)
{
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *) dev;
    uint reg_addr;
    ushort reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0x04): ",
                              0x0, 0x0, TS_PTR_OS);

    if (tmpx75->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmpx75_err_buf, TMPX75_ALTER);
        return (FAILED);
    }

    printf("Original value: reg %#x, data %#x\n", reg_addr, reg_data);

    /* alter register with new msb value */
    reg_data = gethex_answer("Enter the new data (hex): ", reg_data, 0, 0xFFFF);

    if (tmpx75->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmpx75_err_buf, TMPX75_ALTER);
        return (FAILED);
    }

    if (tmpx75->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmpx75_err_buf, TMPX75_ALTER);
        return (FAILED);
    }

    printf("Register: reg %#x, data %#x\n", reg_addr, reg_data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_tmpx75_test_reg
 *
 * Description: Tests the TMPX75 registers.
 *              Restores original register values after test.
 *
 * Inputs:  dev_object_t pointer to the TMPX75 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_tmpx75_test_reg (dev_object_t *dev)
{
    tmpx75_reg_ext.param = (void *)dev;

    if (register_tests(0, tmpx75_reg_table) == FAILED) {
        sprintf(tmpx75_err_buf, "%s: TMPX75 Register Test fail",
                __FUNCTION__);
        DEV_ERROR_REPORT(dev, tmpx75_err_buf, TMPX75_REG_TEST);
        return (FAILED);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:    dev_tmpx75_show_temp
 *
 * Description: Display temperature
 *
 * Inputs:  dev_object_t pointer to the TMPX75 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_tmpx75_show_temp (dev_object_t *dev)
{
    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *) dev;
    ushort reg_data;

    if (tmpx75->callout_fvt->rd(TS_PTR_TEMP, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmpx75_err_buf, TMPX75_SHOW_TEMP);
        return (FAILED);
    }

    /* Byte swap the data since Byte 1 carries T11:T4 and Byte 2 carries T3:T0 */
    reg_data = ((reg_data >> 8) & 0xFF) | ((reg_data & 0xFF) << 8);

    if (reg_data <= TS_TEMP_MAX) {
        printf("Temperature: %.4f Celcius\n", 
                (reg_data >> 4) * TS_TEMP_RESOLUTION);
    } else {
        printf("Temperature: %.4f Celcius\n", 
                ((reg_data >> 4) - 4096) * TS_TEMP_RESOLUTION);
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function:    dev_i2c_rd
 *
 * This function: read TMPX75 register
 *
 * Input :  addr - offset of register to be written.
 *          size - number of bytes to write.
 *          data  - write data.
 *          param - Pointer to the TMPX75 device object
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_rd (ulong addr, int size, ulong *buf, void *param)
{
    ulong byte_addr = addr;
    int rc;

    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *)param;
    rc = tmpx75->callout_fvt->rd((uint32)byte_addr, (ushort *)buf);

    return (rc);
}

/******************************************************************************
 *
 * Function:    dev_i2c_wr
 *
 * This function: write TMPX75 register
 *
 * Input :  addr - offset of register to be read.
 *          size - number of bytes to be read.
 *          buf  - points to the data buffer to be read.
 *          param - Pointer to the TMPX75 device object
 *
 * Output:  PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_wr (ulong addr, int size, ulong data, void *param)
{
    uint32 byte_addr = (uint32)addr;
    ulong byte_data = data;

    dev_tmpx75_object_t *tmpx75 = (dev_tmpx75_object_t *)param;
    return (tmpx75->callout_fvt->wr(byte_addr, (ushort *)&byte_data));
}


