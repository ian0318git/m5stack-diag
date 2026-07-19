/* $Id: dev_tmp421.c,v 1.2 2013/10/08 08:48:25 tirawan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_tmp421/dev_tmp421.c,v $
 *------------------------------------------------------------------
 * Filename:	dev_tmp421.c
 *
 * Description:	Remote and Local TEMPERATURE SENSOR in SOT23-8 TMP421
 * Copyright (c) 2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "defs.h"
#include "common.h"
#include "dev_tmp421.h"
#include "dev_print.h"
#include "free.h"
#include "proto.h"
#include "common_utils.h"
#include "types.h"
#include "error.h"
#ifdef LINUX_APP
#include <assert.h>
#endif
/*===================================================================*
 *                    Function Prototypes                            *
 *===================================================================*/
void tmp421_dev_create(dev_object_t *, dev_error_report_t);

static uint32 dev_tmp421_attach(dev_object_t *);
static uint32 dev_tmp421_detach(dev_object_t *);
static uint32 dev_tmp421_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_tmp421_restart(dev_object_t *);
static uint32 dev_tmp421_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static int dev_tmp421_check_chip_id(dev_object_t *);
static void dev_tmp421_destroy(dev_object_t **);
static int dev_tmp421_alter_reg(dev_object_t *);
static int dev_tmp421_show(dev_object_t *);
static int dev_tmp421_show_temp(dev_object_t *);
static int dev_tmp421_test_reg(dev_object_t *);
static int dev_i2c_rd(ulong, int, ulong *, void *);
static int dev_i2c_wr(ulong, int, ulong, void *);

/* Global variables */
#define TMP421_REG_RO_FLAG              (READ_ONLY | REG_ACCESS)
/* No REG_ACCESS here accroding to different reg size */
#define TMP421_REG_RW_FLAG              (READ_WRITE| SAVE_RESTORE | REG_ACCESS)

/* Registers test table */
static reg_info_t_ext tmp421_reg_ext = {1, dev_i2c_rd, dev_i2c_wr, 0};

/* TMP421 registers table. This device has registers with different sizes.
 */
reg_info_t tmp421_reg_table[] =
{
    {"Local Temperature High Byte",		TMP421_PTR_LOC_TEMP_HIGH,	TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},	0xFF, 0x00},
    {"Remote Temperature 1 High Byte",  TMP421_PTR_RM_TEMP_1_HIGH,  TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},    0xFF, 0x00},
    {"Status Register",                 TMP421_PTR_STATUS,          TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},    0x80, 0x00},
    {"Configuration Register 1",        TMP421_PTR_CONF_1,          TMP421_REG_RW_FLAG,
            {(ulong)&tmp421_reg_ext},    0x44, 0x00},
    {"Configuration Register 2",        TMP421_PTR_CONF_2,          TMP421_REG_RW_FLAG,
            {(ulong)&tmp421_reg_ext},    0x1C, 0x1C},
    {"Conversion Rate Register",        TMP421_PTR_CV_RATE_1,       TMP421_REG_RW_FLAG,
            {(ulong)&tmp421_reg_ext},    0x07, 0x04},
    {"Local Temperature Low Byte",      TMP421_PTR_LOC_TEMP_LOW,    TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},    0xFF, 0x00},
    {"Remote Temperature 1 Low Byte",   TMP421_PTR_RM_TEMP_1_LOW,   TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},    0xFF, 0x00},
    {"N Correction 1",                  TMP421_PTR_N_CORRECTION_1,  TMP421_REG_RW_FLAG,
            {(ulong)&tmp421_reg_ext},    0xFF, 0x00},
    {"Manufacturer ID",                 TMP421_PTR_MANFAC_ID,       TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},    0x55, 0x55},
    {"Device ID",                       TMP421_PTR_DEVICE_ID,       TMP421_REG_RO_FLAG,
            {(ulong)&tmp421_reg_ext},    0x21, 0x21}, 
    {0, 0, 0, {0}, 0, 0},
};

static char tmp421_err_buf[TMP421_ERR_BUF_SIZE];

/******************************************************************************
 *
 * Name:	tmp421_dev_create()
 *
 * Description:	Create object with various device function
 *        		point to "do nothing"
 *
 * Input:	dev_object_t pointer to the TMP421 device.
 *		    error reporting function pointer.
 *
 * Returns:	none
 *
 *****************************************************************************/
void tmp421_dev_create (dev_object_t *dev, dev_error_report_t error_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *)dev;

    /* Allocate memory for the device object */
    if ((dev_fvt = (dev_object_fvt_t *)malloc(sizeof(dev_object_fvt_t))) ==
		    NULL) {
	/* Unable to allocate memory */
	error_report_fn(dev, "malloc failure in tmp421_dev_create()", 0);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    tmp421->base.dev_object_fvt->dev_attach	= dev_tmp421_attach;
    tmp421->base.dev_object_fvt->dev_detach	= dev_tmp421_detach;
    tmp421->base.dev_object_fvt->dev_reconfig_needed = dev_tmp421_reconfig;
    tmp421->base.dev_object_fvt->dev_restart	= dev_tmp421_restart;
    tmp421->base.dev_object_fvt->dev_error_report	= error_report_fn;
    tmp421->base.dev_object_fvt->dev_collect_crashinfo = dev_tmp421_crsh;
    tmp421->base.dev_object_fvt->dev_destroy	= dev_tmp421_destroy;
    tmp421->base.dev_object_fvt->dev_name	= "TMP421 Digital Temperature Sensor";

    tmp421->callin_fvt = (ts_callin_fvt_t *)
                                malloc(sizeof(ts_callin_fvt_t));
    tmp421->callout_fvt = (ts_callout_fvt_t *)
                                malloc(sizeof(ts_callout_fvt_t));

    tmp421->base.dev_state = DEV_STATE_CREATE;
}

/******************************************************************************
 *
 * Name:	dev_tmp421_attach()
 *
 * Description:	Attach the TMP421 device for use. This function will
 *		        initialize and setup all necessary pointers and bring the
 *        		chip to operation.
 *
 * Input:	Pointer to the TMP421 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_tmp421_attach (dev_object_t *dev)
{
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *) dev;

    if (tmp421->callin_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_tmp421_attach() callin malloc", TMP421_ATTACH);
	return(FAILED);
    }

    if (tmp421->callout_fvt == NULL) {
	DEV_ERROR_REPORT(dev, "dev_tmp421_attach() callout malloc", TMP421_ATTACH);
	return(FAILED);
    }

    /* init the call in function */
    tmp421->callin_fvt->register_test = dev_tmp421_test_reg;
    tmp421->callin_fvt->show_temp = dev_tmp421_show_temp;
    tmp421->callin_fvt->dump_register = dev_tmp421_show;
    tmp421->callin_fvt->alter_register = dev_tmp421_alter_reg;
    tmp421->callin_fvt->check_chip_id = dev_tmp421_check_chip_id;

    tmp421->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/******************************************************************************
 *
 * Name:	dev_tmp421_detach()
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
 * Input:	Pointer to the TMP421 device object
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
static uint32 dev_tmp421_detach (dev_object_t *dev)
{
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *) dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, tmp421->base.dev_object_fvt);

    tmp421->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);

}

/******************************************************************************
 * Name:	dev_tmp421_reconfig_needed
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
 * Input:	dev_object_t pointer to the TMP421 device
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
static uint32 dev_tmp421_reconfig(dev_object_t *dev, void *context_handle,
                                  boolean *reconfig)
{
    *reconfig = FALSE;		/* No need to reconfig from scratch */
    return(PASSED);
}

/******************************************************************************
 * Name:	dev_tmp421_restart
 *
 * Description:	To restart a previously initialized device without resetting
 *		        the device or changing its configuration.
 *		        For example, during a failover event.
 *
 *		        Change the state of the device from its current state
 *		        to an initial state. Also, dev_state must be assigned the
 *		        value of DEV_STATE_INIT.
 *
 * Input:	dev_object_t pointer to the TMP421 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *		        called and successfully executed.
 *
 *****************************************************************************/
static uint32 dev_tmp421_restart(dev_object_t *dev)
{
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *) dev;

    tmp421->base.dev_state = DEV_STATE_INIT;
    return(PASSED);
}

/******************************************************************************
 * Name:	dev_tmp421_show
 *
 * Description:	Provide platforms with a mechanism to display some common
 *		        device information via the device print function argument.
 *
 * Input:	dev_object_t pointer to the TMP421 device
 *
 * Returns:	PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *		        platform which implements the print logging functionality. The
 *		        dev_attach() function has been called and successfully executed
 *
 *****************************************************************************/
static int dev_tmp421_show(dev_object_t *dev)
{

    tmp421_reg_ext.param = (void *)dev;

    if (register_display(0, tmp421_reg_table) == FAILED) {
        sprintf(tmp421_err_buf, "%s: Thermal sensor register display fail",
                              __FUNCTION__);
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_DISPLAY);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Name:	dev_tmp421_crsh
 *
 * Description:	Allow platforms to collect data from a device during a crash.
 *		        Print data to the crash log (via the provide print error) using
 *		        the appropriate verbisity level requested by the host
 *
 * Input:	dev_object_t pointer to the TMP421 device
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
static uint32 dev_tmp421_crsh(dev_object_t *dev, print_fn_t dev_print,
                              dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("dev_tmp421_crsh(): No Crash info available for TMP421\n");
    return(PASSED);
}

/******************************************************************************
 * Name:	dev_tmp421_destroy
 *
 * Description:	Destroy the dev_object structure and free all the resources.
 *
 * Input:	dev_object_t pointer to the TMP421 device
 *
 * Returns:	none
 *
 * Assumptions:	The dev_attch() function has been called and successfully
 *
 *****************************************************************************/
static void dev_tmp421_destroy(dev_object_t **dev)
{
    dev_tmp421_object_t *tmp421;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    tmp421 = (dev_tmp421_object_t *)*dev;

    if (tmp421->callout_fvt) {
        free(tmp421->callout_fvt);	/* Free callout struct */
    }

    if (tmp421->callin_fvt) {
        free(tmp421->callin_fvt);		/* Free callin struct */
    }

    free(tmp421->base.dev_object_fvt);	/* Free dev_object_t */
}

/******************************************************************************
 *
 * Function:	dev_tmp421_alter_reg
 *
 * Description:	Alter TMP421 register.
 *
 * Inputs:	dev_object_t pointer to the TMP421 device
 *		    A device print function vector
 *
 * Outputs:	PASSED - No errors encounterd.
 *		    FAILED - Errors encounterd.
 *
 * Assumptions:	create and dev_attach have to be called first. dev_destroy will
 *		        also be called after the exit.
 *
 *****************************************************************************/
static int dev_tmp421_alter_reg(dev_object_t *dev)
{
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *) dev;
    uint reg_addr, reg_data;

    printf("\n");

    reg_addr = gethex_answer("Enter register address (0x0 ~ 0xff): ",
                              0x0, 0x0, 0xff);

    if (tmp421->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_ALTER);
        return (FAILED);
    }

    printf("Original value: reg %#.2x, data %#.2x\n", reg_addr, reg_data);

    /* alter register with new msb value */
    reg_data = gethex_answer("Enter the new data (hex): ", reg_data, 0, 0xFF);

    if (tmp421->callout_fvt->wr(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_ALTER);
        return (FAILED);
    }

    if (tmp421->callout_fvt->rd(reg_addr, &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_ALTER);
        return (FAILED);
    }

    printf("Register: reg %#.2x, data %#.2x\n", reg_addr, reg_data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_tmp421_test_reg
 *
 * Description: Tests the TMP421 registers.
 *              Restores original register values after test.
 *
 * Inputs:  dev_object_t pointer to the TMP421 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_tmp421_test_reg (dev_object_t *dev)
{
    tmp421_reg_ext.param = (void *)dev;

    if (register_tests(0, tmp421_reg_table) == FAILED) {
        sprintf(tmp421_err_buf, "%s: Thermal sensor register display fail",
                              __FUNCTION__);
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_REG_TRST);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 * Name:    dev_tmp421_show_temp
 *
 * Description: Displays TMP421 Temperature.
 *
 * Input:   Pointer to the tmp421 device object
 *
 * Returns: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_tmp421_show_temp (dev_object_t *dev)
{
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *) dev;
    uint high_temp_reg, low_temp_reg;
    uint temperature;

    /* Get the local temperature high byte */
    if (tmp421->callout_fvt->rd((char)TMP421_PTR_LOC_TEMP_HIGH,
                                             &high_temp_reg) == FAILED) {
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_SHOW_TEMP);
        return (FAILED);
    }

    /* Get the local temperature low byte */
    if (tmp421->callout_fvt->rd((char)TMP421_PTR_LOC_TEMP_LOW,
                                             &low_temp_reg) == FAILED) {
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_SHOW_TEMP);
        return (FAILED);
    }
    /* Avoiding the non-correct value while calculate the temperature. */
    high_temp_reg &= 0xFF;
    low_temp_reg &= 0xFF;

    printf("\n");

    /* The temperature calculating is according to the TMP421 datasheet
     * table 1 and table 2
     */
    if (high_temp_reg >> SIGN_BITS_SHIFT) {
        /* Temperature is Negative, do two's complement convert */
        high_temp_reg = (~high_temp_reg & 0xFF) + 1;
        /* The first 4 bits are meaning bits */
        low_temp_reg >>= FLOATING_BITS_SHIFT;
        /* Use decimal to replace the floating calculating */
        temperature = (high_temp_reg * TMP_MULTIPLE_10000) - (low_temp_reg * 625);
        printf("Temperature is -%d.%d Degree Celsius\n",
                (temperature / TMP_MULTIPLE_10000),
                (temperature % TMP_MULTIPLE_10000));
    } else {
        /* Temperature is Positive */
        /* The first 4 bits are meaning bits */
        low_temp_reg >>= FLOATING_BITS_SHIFT;
        /* Use decimal to replace the floating calculating */
        temperature = (high_temp_reg * TMP_MULTIPLE_10000) + (low_temp_reg * 625);
        printf("Temperature is %d.%d Degree Celsius\n",
                (temperature / TMP_MULTIPLE_10000),
                (temperature % TMP_MULTIPLE_10000));
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_tmp421_check_chip_id
 *
 * Description: Check TMP421 chip ID
 *
 * Inputs:  dev_object_t pointer to the TMP421 device
 *          A device print function vector
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 * Assumptions: create and dev_attach have to be called first. dev_destroy will
 *              also be called after the exit.
 *
 *****************************************************************************/
static int dev_tmp421_check_chip_id (dev_object_t *dev)
{
    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *) dev;
    uint reg_data;
    
    if (tmp421->callout_fvt->rd((char)TMP421_PTR_DEVICE_ID,
                                &reg_data) == FAILED) {
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_ALTER);
        return (FAILED);
    }

    if (reg_data != TMP421_DEVICE_ID) {
        sprintf(tmp421_err_buf, "%s: TMP421 check device id fail",
                              __FUNCTION__);
        DEV_ERROR_REPORT(dev, tmp421_err_buf, TMP421_CHECK_ID);
        return (FAILED);
    }
    prpass(testpass, "tmp421 id 0x%.8x check success", reg_data);
    return (PASSED);
}

/******************************************************************************
 *
 * Function:    dev_i2c_rd
 *
 * This function: read TM421 register
 *
 * Input :  addr - offset of register to be written.
 *          size - number of bytes to write.
 *          data  - write data.
 *          param - Pointer to the TMP421 device object
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_rd (ulong addr, int size, ulong *buf, void *param)
{
    ulong byte_addr = addr;
    int rc;

    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *)param;
    rc = tmp421->callout_fvt->rd((uint32)byte_addr, (uint *)buf);

    return (rc);
}

/******************************************************************************
 *
 * Function:    dev_i2c_wr
 *
 * This function: write TMP421 register
 *
 * Input :  addr - offset of register to be read.
 *          size - number of bytes to be read.
 *          buf  - points to the data buffer to be read.
 *          param - Pointer to the TMP421 device object
 *
 * Output:  PASSED/FAILED
 *
 *****************************************************************************/
static int dev_i2c_wr (ulong addr, int size, ulong data, void *param)
{
    uint32 byte_addr = (uint32)addr;
    char byte_data = (char)data;

    dev_tmp421_object_t *tmp421 = (dev_tmp421_object_t *)param;
    return (tmp421->callout_fvt->wr(byte_addr, (uint *)&byte_data));
}

/*------------------------------------------------------------------
 * $Log: dev_tmp421.c,v $
 * Revision 1.2  2013/10/08 08:48:25  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:48  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:58:02  tirawan
 * First Woodlawn linux integration
 *
 * Revision 1.2  2013/03/27 04:49:45  kuangik
 * Code cleanup after -Wall
 *
 * Revision 1.1  2013/03/13 06:42:14  kuangik
 * Add for the first time
 *
 * Revision 1.5  2012/10/24 10:52:19  leslie
 * Fix and clean up code.
 *
 * Revision 1.4  2012/08/30 06:26:01  leslie
 * Fix the issue of dump register and register test.
 *
 * Revision 1.3  2012/08/03 10:16:51  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.1  2012/03/26 07:23:53  kody
 * Add TMP421 temperature sensor device driver.
 *
 *------------------------------------------------------------------
 * $Endlog$
 *------------------------------------------------------------------
 */
