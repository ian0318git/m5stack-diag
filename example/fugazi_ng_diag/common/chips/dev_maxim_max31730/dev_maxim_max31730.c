/* $Id: dev_maxim_max31730.c,v 1.2 2019/01/10 06:23:18 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_maxim_max31730/dev_maxim_max31730.c,v $
 *------------------------------------------------------------------
 * Filename   :	dev_maxim_max31730.c
 * Description: Common driver of Maxim Max31730, a 3-Channel Remote 
 *              Temperature Sensor.
 *
 * Copyright (c) 2018 - 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/time.h>
#include "defs.h"
#include "common.h"
#include "dev_maxim_max31730.h"
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


/*******************************************************************************
 *                             Function Prototypes                             *
 *******************************************************************************
 */
void          max31730_dev_create(dev_object_t *, dev_error_report_t);
static uint32 dev_max31730_attach(dev_object_t *);
static int    dev_i2c_rd(ulong, int, ulong *, void *);
static int    dev_i2c_wr(ulong, int, ulong, void *);
static uint32 dev_max31730_detach(dev_object_t *);
static uint32 dev_max31730_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_max31730_restart(dev_object_t *);
static uint32 dev_max31730_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void   dev_max31730_destroy(dev_object_t **);
static int    dev_max31730_show_reg(dev_object_t *);
static int    dev_max31730_dump_reg(dev_object_t *);
static int    dev_max31730_alter_reg(dev_object_t *);
static int    dev_max31730_show_temp(dev_object_t *);
static int    dev_max31730_test_reg(dev_object_t *);
static int    dev_max31730_test_intr(dev_object_t *, int);


/*******************************************************************************
 *                               Global variables                              *
 *******************************************************************************
 */
/* Registers test table */
static reg_info_t_ext max31730_reg_ext = {MAX31730_REG_ONE_BYTE_ACCESS,
                                          dev_i2c_rd, dev_i2c_wr, 0};
static reg_info_t_ext max31730_temp_ext = {MAX31730_REG_TWO_BYTE_ACCESS,
                                           dev_i2c_rd, dev_i2c_wr, 0};

/* Maxim MAX31730 registers table */
reg_info_t max31730_reg_table[] =
{
    {"Local Temperature MSB",                  MAX31730_LOC_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Local Temperature LSB",                  MAX31730_LOC_TEMP_LSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 1 Temperature MSB",               MAX31730_R1_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 1 Temperature LSB",               MAX31730_R1_TEMP_LSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 2 Temperature MSB",               MAX31730_R2_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 2 Temperature LSB",               MAX31730_R2_TEMP_LSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 3 Temperature MSB",               MAX31730_R3_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 3 Temperature LSB",               MAX31730_R3_TEMP_LSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Highest Temperature MSB",                MAX31730_HST_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Highest Temperature LSB",                MAX31730_HST_TEMP_LSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Highest Temperature Enable",             MAX31730_HST_TEMP_EN,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x0F},
    {"Configuration",                          MAX31730_CONFIG,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x00,                                     0x10},
    {"Custom Ideality Factor",                 MAX31730_CUST_IDEALITY_FACTOR,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x3F,                                     0x18},
    {"Custom Ideality Enable",                 MAX31730_CUST_IDEALITY_EN,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0E,                                     0x00},
    {"Custom Offset",                          MAX31730_CUST_OFFST,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x77},
    {"Custom Offset Enable",                   MAX31730_CUST_OFFST_EN,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0E,                                     0x00},
    {"Filter Enable",                          MAX31730_FILTER_EN,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0E,                                     0x00},
    {"Beta Compensation Enable",               MAX31730_BETA_COMP_EN,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0E,                                     0x00},
    {"Beta Value Channel 1",                   MAX31730_BETA_VAL_CH1,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x00},
    {"Beta Value Channel 2",                   MAX31730_BETA_VAL_CH2,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x00},
    {"Beta Value Channel 3",                   MAX31730_BETA_VAL_CH3,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x00},
    {"Local Thermal High Limit MSB",           MAX31730_LOC_HLIM_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x7F},
    {"Local Thermal High Limit LSB",           MAX31730_LOC_HLIM_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 1 Thermal High Limit MSB",        MAX31730_R1_HLIM_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x7F},
    {"Remote 1 Thermal High Limit LSB",        MAX31730_R1_HLIM_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 2 Thermal High Limit MSB",        MAX31730_R2_HLIM_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x7F},
    {"Remote 2 Thermal High Limit LSB",        MAX31730_R2_HLIM_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 3 Thermal High Limit MSB",        MAX31730_R3_HLIM_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x7F},
    {"Remote 3 Thermal High Limit LSB",        MAX31730_R3_HLIM_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Thermal Low Limit (All Channels) MSB",   MAX31730_ALL_LLIM_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0xC9},
    {"Thermal Low Limit (All Channels) LSB",   MAX31730_ALL_LLIM_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Thermal Status, High Temperature",       MAX31730_THERM_STATUS_H,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x00},
    {"Thermal Status, Low Temperature",        MAX31730_THERM_STATUS_L,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x00},
    {"THERM Mask",                             MAX31730_THERM_MASK,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x00},
    {"Temperature Channel Enable",             MAX31730_TEMP_CH_EN,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0F,                                     0x0F},
    {"Diode Fault Status",                     MAX31730_DIO_FAULT_STATUS,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0x0E,                                     0x00},
    {"Local Reference Temperature MSB",        MAX31730_LOC_REF_TEMP_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Local Reference Temperature LSB",        MAX31730_LOC_REF_TEMP_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 1 Reference Temperature MSB",     MAX31730_R1_REF_TEMP_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 1 Reference Temperature LSB",     MAX31730_R1_REF_TEMP_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 2 Reference Temperature MSB",     MAX31730_R2_REF_TEMP_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 2 Reference Temperature LSB",     MAX31730_R2_REF_TEMP_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 3 Reference Temperature MSB",     MAX31730_R3_REF_TEMP_MSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Remote 3 Reference Temperature LSB",     MAX31730_R3_REF_TEMP_LSB,
     MAX31730_REG_RW_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x00},
    {"Manufacturer ID",                        MAX31730_MFR_ID,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x4D},
    {"Revision Code",                          MAX31730_REV,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_reg_ext},
     0xFF,                                     0x01},
    { 0, 0, 0, {0}, 0, 0},
};

/* Maxim MAX31730 registers table */
reg_info_t max31730_temp_reg_table[] =
{
    {"Local Temperature",                      MAX31730_LOC_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_temp_ext},
     0xFF,                                     0x00},
    {"Remote 1 Temperature",                   MAX31730_R1_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_temp_ext},
     0xFF,                                     0x00},
    {"Remote 2 Temperature",                   MAX31730_R2_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_temp_ext},
     0xFF,                                     0x00},
    {"Remote 3 Temperature",                   MAX31730_R3_TEMP_MSB,
     MAX31730_REG_RO_FLAG,                     {(utype_t)&max31730_temp_ext},
     0xFF,                                     0x00},
    { 0, 0, 0, {0}, 0, 0},
};

/* Maxim MAX31730 registers table for interrupt test */
intr_test_obj_t max31730_intr_test_obj_table[] =
{
    {"Local Thermal",                     &max31730_reg_ext,
     "Local Thermal High Limit MSB",      MAX31730_LOC_HLIM_MSB,
     0xC0,                                0x7F,
     "Thermal High Status",               MAX31730_THERM_STATUS_H,
     0x01,                                0},
    {"Remote 1 Thermal",                  &max31730_reg_ext,
     "Remote 1 Thermal High Limit MSB",   MAX31730_R1_HLIM_MSB,
     0xC0,                                0x7F,
     "Thermal High Status",               MAX31730_THERM_STATUS_H,
     0x02,                                0},
    {"Remote 2 Thermal",                  &max31730_reg_ext,
     "Remote 2 Thermal High Limit MSB",   MAX31730_R2_HLIM_MSB,
     0xC0,                                0x7F,
     "Thermal High Status",               MAX31730_THERM_STATUS_H,
     0x04,                                0},
    {"Remote 3 Thermal",                  &max31730_reg_ext,
     "Remote 3 Thermal High Limit MSB",   MAX31730_R3_HLIM_MSB,
     0xC0,                                0x7F,
     "Thermal High Status",               MAX31730_THERM_STATUS_H,
     0x08,                                0},
};


/*******************************************************************************
 *
 * Function   : max31730_dev_create
 * Description:	Create object with various device function.
 * Inputs     : *dev          - dev_object_t pointer to Maxim MAX31730 device
 *		err_report_fn - error reporting function pointer
 * Outputs    : none
 *
 *******************************************************************************
 */
void max31730_dev_create (dev_object_t *dev, dev_error_report_t err_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_max31730_object_t *max31730 = (dev_max31730_object_t *)dev;

    /* Allocate memory for the device object */
    dev_fvt = (dev_object_fvt_t *)(malloc(sizeof(dev_object_fvt_t)));
    if (dev_fvt == NULL) {
        /* Unable to allocate memory */
        err_report_fn(dev, "malloc failed at dev_create.", MAX31730_DEV_STATE);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    max31730->base.dev_object_fvt->dev_attach            = dev_max31730_attach;
    max31730->base.dev_object_fvt->dev_detach            = dev_max31730_detach;
    max31730->base.dev_object_fvt->dev_reconfig_needed   = dev_max31730_reconfig;
    max31730->base.dev_object_fvt->dev_restart           = dev_max31730_restart;
    max31730->base.dev_object_fvt->dev_error_report      = err_report_fn;
    max31730->base.dev_object_fvt->dev_collect_crashinfo = dev_max31730_crsh;
    max31730->base.dev_object_fvt->dev_destroy           = dev_max31730_destroy;
    max31730->base.dev_object_fvt->dev_name              = "Maxim MAX31730";

    max31730->callin_fvt = (max31730_callin_fvt_t *)
                           (malloc(sizeof(max31730_callin_fvt_t)));
    max31730->callout_fvt = (max31730_callout_fvt_t *)
                            (malloc(sizeof(max31730_callout_fvt_t)));

    max31730->base.dev_state = DEV_STATE_CREATE;
}

/*******************************************************************************
 *
 * Function   : dev_max31730_attach
 * Description:	Attach Maxim MAX31730 device for use.
 *              This function will initialize, setup all necessary pointers
 *              and bring the chip to operation.
 * Inputs     : Pointer to the Maxim MAX31730 device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32 dev_max31730_attach (dev_object_t *dev)
{
    dev_max31730_object_t *max31730 = (dev_max31730_object_t *)dev;

    if (max31730->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "callin_fvt is NULL", MAX31730_ATTACH);
        return (FAILED);
    }

    if (max31730->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "callout_fvt is NULL", MAX31730_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    max31730->callin_fvt->register_test  = dev_max31730_test_reg;
    max31730->callin_fvt->interrupt_test = dev_max31730_test_intr;
    max31730->callin_fvt->show_register  = dev_max31730_show_reg;
    max31730->callin_fvt->alter_register = dev_max31730_alter_reg;
    max31730->callin_fvt->dump_register  = dev_max31730_dump_reg;
    max31730->callin_fvt->show_temp      = dev_max31730_show_temp;

    max31730->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_detach
 * Description:	detach the device specific functions from the caller.
 *              All of the device specific function are connected to the
 *              dev_do_nothing() function, except for the dev_attach() function.
 *
 *              Also, the dev_state must be assigned the value of
 *              DEV_STATE_DETACH.
 *
 *              Since, some platforms may want to detach the device, but not
 *              release memory resources(via a free () in the dev_destroy()),
 *              this function can be executed to accomplish this task. However,
 *              a detached device need to be re-attached(via dev_attach())
 *              before it can be used again.
 * Inputs     : Pointer to Maxim MAX31730 device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32 dev_max31730_detach (dev_object_t *dev)
{
    dev_max31730_object_t *max31730 = (dev_max31730_object_t *)dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, max31730->base.dev_object_fvt);

    max31730->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_reconfig_needed
 * Description:	To check whether device re-configuration is needed during
 *              (re)initialization. Based on the provided context information,
 *              the boolean return value, and possibly other factors external
 *              to the device object, the caller shall decide whether to invoke
 *              either dev_restart or dev_init, but not both. In general, the
 *        	boolean return value alone is not sufficient to decide whether
 *              the device can safely be restarted or whether it must be fully
 *              initialized from scratch.
 * Inputs     : *dev            - dev_object_t pointer to Maxim MAX31730 device
 *		*context_handle - a device/platform specific context handle
 *		*reconfig       - a pointer to a boolean
 * Outputs    : PASSED/FAILED, context information and a boolean value.
 *              The boolean value shall be set to TRUE if the device must be
 *              reconfigured from scratch and it shall be set to FALSE otherwise
 *
 * Assumptions:	The dev_attach() function has been called and successfully
 *
 *******************************************************************************
 */
static uint32 dev_max31730_reconfig (dev_object_t *dev, void *context_handle,
                                  boolean *reconfig)
{
    *reconfig = FALSE;   /* No need to reconfig from scratch */
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_restart
 * Description:	To restart a previously initialized device without resetting
 *              the device or changing its configuration.
 *              For example, during a failover event.
 *
 *              Change the state of the device from its current state
 *              to an initial state. Also, dev_state must be assigned the
 *              value of DEV_STATE_INIT.
 * Inputs     : *dev - dev_object_t pointer to Maxim MAX31730 device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *
 *******************************************************************************
 */
static uint32 dev_max31730_restart (dev_object_t *dev)
{
    dev_max31730_object_t *max31730 = (dev_max31730_object_t *)dev;

    max31730->base.dev_state = DEV_STATE_INIT;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_crsh
 * Description:	Allow platforms to collect data from a device during a crash.
 *              Print data to the crash log (via the provide print error) using
 *              the appropriate verbisity level requested by the host
 * Inputs     : *dev      - dev_object_t pointer to Maxim MAX31730 device
 *		dev_print - A crash print function vector.
 *              verbosity - A verbosity level.
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	A device print function vector has been provided by the host
 *              platform which implements the crash logging functionality. It
 *              could be the mechanism to log info to the Compact Flash before
 *              the device crash and now retrieve them. The dev_attch()
 *              function has been called and successfully executed.
 *
 *******************************************************************************
 */
static uint32 dev_max31730_crsh (dev_object_t *dev, print_fn_t dev_print,
                              dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("%s(): No Crash info available for Maxim MAX31730.\n", __func__);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_destroy
 * Description:	Destroy the dev_object structure and free all the resources.
 * Inputs     : **dev - dev_object_t pointer to Maxim MAX31730 device
 * Outputs    : none
 *
 * Assumptions:	The dev_attch() function has been called and successfully.
 *
 *******************************************************************************
 */
static void dev_max31730_destroy (dev_object_t **dev)
{
    dev_max31730_object_t *max31730;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    max31730 = (dev_max31730_object_t *)*dev;

    if (max31730->callout_fvt) {
        free(max31730->callout_fvt);   /* Free callout struct */
    }

    if (max31730->callin_fvt) {
        free(max31730->callin_fvt);   /* Free callin struct */
    }

    free(max31730->base.dev_object_fvt);   /* Free dev_object_t */
}

/*******************************************************************************
 *
 * Function   : dev_max31730_show_reg
 * Description:	Function to get and show Maxim MAX31730 reigster.
 * Inputs     : *dev - dev_object_t pointer to the Maxim MAX31730 device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *******************************************************************************
 */
static int dev_max31730_show_reg (dev_object_t *dev)
{
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)dev;
    uchar reg_addr = 0;
    ulong reg_val = 0;

    reg_addr = (uchar)gethex_answer("\nEnter register offset in hex(0 ~ 51h)",
                                    MAX31730_MFR_ID, MAX31730_LOC_TEMP_MSB,
                                    MAX31730_REV);

    if (dev_i2c_rd((ulong)reg_addr,
                   MAX31730_REG_ONE_BYTE_ACCESS,
                   &reg_val,
                   (void *)max31730_p) != PASSED) {
        printf("%s(): Failed to read register %#x.\n", __func__, reg_addr);
        return (FAILED);
    }

    printf("%s reg. 0x%02X = 0x%02X\n",
           max31730_p->base.dev_object_fvt->dev_name,
           reg_addr,
           (uchar)reg_val);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_alter_reg
 * Description:	Function to alter Maxim MAX31730 register.
 * Inputs     : *dev - dev_object_t pointer to Maxim MAX31730 device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_max31730_alter_reg (dev_object_t *dev)
{
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)dev;
    uchar reg_addr = 0, wr_val = 0;
    ulong reg_val = 0;

    reg_addr = (uchar)gethex_answer("\nEnter register offset in hex(0 ~ 51h)",
                                    MAX31730_HST_TEMP_EN,
                                    MAX31730_LOC_TEMP_MSB,
                                    MAX31730_REV);

    if (dev_i2c_rd((ulong)reg_addr,
                   MAX31730_REG_ONE_BYTE_ACCESS,
                   &reg_val,
                   (void *)max31730_p) != PASSED) {
        printf("%s(): Failed to read register %#x.\n", __func__, reg_addr);
        return (FAILED);
    }

    wr_val = (uchar)gethex_answer("Enter the alter value in hex",
                                  reg_val, 0, 0xFF);

    if (dev_i2c_wr((ulong)reg_addr,
                   MAX31730_REG_ONE_BYTE_ACCESS,
                   (ulong)wr_val,
                   (void *)max31730_p) != PASSED) {
        printf("%s(): Failed to write register 0x%02X.\n", __func__, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (dev_i2c_rd((ulong)reg_addr,
                   MAX31730_REG_ONE_BYTE_ACCESS,
                   &reg_val,
                   (void *)max31730_p) != PASSED) {
        printf("%s(): Failed to read register %#x.\n", __func__, reg_addr);
        return (FAILED);
    }

    printf("Done writing 0x%02X to %s reg. 0x%02X, its value = 0x%02X now.\n",
           wr_val,
           max31730_p->base.dev_object_fvt->dev_name,
           reg_addr,
           (uchar)reg_val);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_dump_reg
 * Description:	Function to dump all Maxim MAX31730 reigsters.
 * Inputs     : *dev - dev_object_t pointer to the MAX31730 device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *******************************************************************************
 */
static int dev_max31730_dump_reg (dev_object_t *dev)
{
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)dev;
    ulong reg_val = 0;
    reg_info_t *reg_p = &max31730_reg_table[0];

    printf("\n%s registers:\n", max31730_p->base.dev_object_fvt->dev_name);

    while (reg_p->name != NULL) {
        reg_val = 0;

        if (reg_p->size.ext->rd_ptr((ulong)reg_p->offset,
                                    reg_p->size.ext->size,
                                    (ulong *)&reg_val,
                                    (void *)max31730_p) != PASSED) {
            printf("%s(): Failed to read register %#x.\n",
                   __func__, reg_p->offset);
            return (FAILED);
        }

        printf("%-36s reg.(0x%02X): 0x%02X\n",
               reg_p->name, reg_p->offset, (uchar)reg_val);

        reg_p++;
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_test_reg
 * Description: Function to test I2C interface between Host and Maxim MAX31730
 *              by accessing its registers.
 *              Restores original register values after test.
 * Inputs     : *dev - dev_object_t pointer to Maxim MAX31730 device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions: create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_max31730_test_reg (dev_object_t *dev)
{
    char err_buf[MAX31730_ERR_BUF_SIZE];
    reg_info_t *test_reg_p;

    /* Init pointer to beginning of register table */
    test_reg_p = &max31730_reg_table[0];
    test_reg_p->size.ext->param = (void *)dev;

    if (register_tests(0, test_reg_p) != PASSED) {
        sprintf(err_buf, "%s: Maxim MAX31730 register Test Failed", __func__);
        DEV_ERROR_REPORT(dev, err_buf, MAX31730_REG_TEST);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_test_intr
 * Description: Function to test interrupt between Host and Maxim MAX31730.
 *              This test force MAX31730 interrupt by set the low-temperature
 *              threshold and THERM mask.
 * Inputs     : *dev - dev_object_t pointer to Maxim MAX31730 device
 *              intr_source - Local(0)/Remote1(1)/Remote2(2)/Remote3(3)
 * Outputs    : PASSED/FAILED
 *
 * Assumptions: create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_max31730_test_intr (dev_object_t *dev, int intr_source)
{
    int ret_val = FAILED;
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)dev;
    intr_test_obj_t *test_p = &max31730_intr_test_obj_table[intr_source];
    struct timeval t_start, t_end;
    double t_diff = 0;
    ulong reg_val = 0;

    /* Confirm there's no pending interrupt from MAX31730 */
    ret_val = max31730_p->callout_fvt->intr_confirm(MAX31730_INTR_NOT_TRIG);
    if (ret_val != PASSED) {
        printf("%s(%d): Got unexpected interrupt.\n", __func__, __LINE__);
	return (FAILED);
    }

    /* Force to trigger interrupt */
    if (test_p->ext->wr_ptr((ulong)test_p->therm_thr_offset,
                            test_p->ext->size,
                            (ulong)test_p->test_thr_val,
                            (void *)max31730_p) != PASSED) {
        printf("%s(): Failed to write %s reg.(%#x) at testing %s.\n",
               __func__, test_p->therm_thr_name,
               test_p->therm_thr_offset, test_p->intr_name);
        return (FAILED);
    }

    ret_val = FAILED;
    gettimeofday(&t_start, NULL);
    do {
        reg_val = 0;
        if (test_p->ext->rd_ptr((ulong)test_p->therm_status_offset,
                                test_p->ext->size,
                                (ulong *)&reg_val,
                                (void *)max31730_p) != PASSED) {
            printf("%s(): Failed to read %s reg.(%#x) at testing %s.\n",
                   __func__, test_p->therm_status_name,
                   test_p->therm_status_offset, test_p->intr_name);
            return (FAILED);
        }

        if (reg_val == (ulong)test_p->test_status_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff = (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                          (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < MAX31730_MAX_POLLING_USEC); /* Polling time: 1sec. */

    if (ret_val != PASSED) {
        printf("%s(): Failed to force trigger MAX31730 interrupt.\n", __func__);
        return (FAILED);
    }

    /* Confirm Host get interrupt that MAX31730 triggers */
    ret_val = FAILED;
    ret_val = max31730_p->callout_fvt->intr_confirm(MAX31730_INTR_IS_TRIG);
    if (ret_val != PASSED) {
        printf("%s(%d): Didn't get expected interrupt.\n", __func__, __LINE__);
	return (FAILED);
    }

    /* Release interrupt from force triggered */
    if (test_p->ext->wr_ptr((ulong)test_p->therm_thr_offset,
                            test_p->ext->size,
                            (ulong)test_p->reset_thr_val,
                            (void *)max31730_p) != PASSED) {
        printf("%s(): Failed to write %s reg.(%#x) at testing %s.\n",
               __func__, test_p->therm_thr_name,
               test_p->therm_thr_offset, test_p->intr_name);
        return (FAILED);
    }

    t_diff = 0;
    ret_val = FAILED;
    gettimeofday(&t_start, NULL);
    do {
        reg_val = 0;
        if (test_p->ext->rd_ptr((ulong)test_p->therm_status_offset,
                                test_p->ext->size,
                                (ulong *)&reg_val,
                                (void *)max31730_p) != PASSED) {
            printf("%s(): Failed to read %s reg.(%#x) at testing %s.\n",
                   __func__, test_p->therm_status_name,
                   test_p->therm_status_offset, test_p->intr_name);
            return (FAILED);
        }

        if (reg_val == (ulong)test_p->reset_status_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff = (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                          (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < MAX31730_MAX_POLLING_USEC); /* Polling time: 1sec. */

    if (ret_val != PASSED) {
        printf("%s(): Failed to force MAX31730 interrupt stop.\n", __func__);
        return (FAILED);
    }

    /* Confirm there's no pending interrupt from MAX31730 */
    ret_val = FAILED;
    ret_val = max31730_p->callout_fvt->intr_confirm(MAX31730_INTR_NOT_TRIG);
    if (ret_val != PASSED) {
        printf("%s(%d): Got unexpected interrupt.\n", __func__, __LINE__);
	return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_max31730_show_temp
 * Description: Display temperature
 * Inputs     : *dev - dev_object_t pointer to Maxim MAX31730 device
 * Outputs    : PASSED/FAILED 
 *
 * Assumptions: create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_max31730_show_temp (dev_object_t *dev)
{
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)dev;
    ushort reg_val = 0;
    reg_info_t *reg_p = &max31730_temp_reg_table[0];
    float temp_val = 0.0;


    printf("\n%s current sensed Temperature:\n",
           max31730_p->base.dev_object_fvt->dev_name);

    while (reg_p->name != NULL) {
        reg_val = 0;

        if (reg_p->size.ext->rd_ptr((ulong)reg_p->offset,
                                    reg_p->size.ext->size,
                                    (ulong *)&reg_val,
                                    (void *)max31730_p) != PASSED) {
            printf("%s(): Failed to read register %#x.\n",
                   __func__, reg_p->offset);
            return (FAILED);
        }

        /* Based on MAX31730 datasheet, definition of
         * Temp./Reference Temp./Thermal-Limit register as below:
         *
         * +-----------------+-------------------+
         * |       MSB       |        LSB        |
         * +------+----------+---------+---------+
         * |  D15 | D14 ~ D8 | D7 ~ D4 | D3 ~ D0 |
         * +------+----------+---------+---------+
         * | Sign |  Valid Temp. Value |    0    |
         * +------+--------------------+---------+
         * => Temperature data format is 12bits, and its unit is 0.0625 degreeC.
         * => The sensed temperature is negative if Sign == 1,
         *    and sensed temperature value is two's complement of read back
         *    register value when Sign == 1.
         */
        if (reg_val & MAX31730_TEMP_SIGN_BIT) {
            reg_val = (ushort)(~reg_val);
            reg_val = (ushort)((reg_val & MAX31730_VALID_TEMP_MASK) + 1);
            temp_val = ((-1) * ((ushort)(reg_val >> MAX31730_VALID_TEMP_SHIFT))
                        * MAX31730_TEMP_UNIT); 
        } else {
            temp_val = (((ushort)(reg_val >>  MAX31730_VALID_TEMP_SHIFT))
                        * MAX31730_TEMP_UNIT); 
        }

        printf("%-20s: %.3f degree Celcius.(Reg. value: 0x%04X)\n",
               reg_p->name, temp_val, reg_val);

        reg_p++;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_i2c_rd
 * Description: Function to read Maxim MAX31730 register by I2C.
 * Inputs     : offset - offset of register to be written
 *              size   - number of bytes to write
 *              *buf   - pointer to the data buffer to be read
 *              param  - pointer to Maxim MAX31730 device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_i2c_rd (ulong offset, int size, ulong *buf, void *param)
{
    uint32 retv = FAILED;
    char err_buf[MAX31730_ERR_BUF_SIZE];
    n2g_i2c_if_t i2c_if;
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)param;
    ushort reg_val = 0;

    /* Setup the interface struct for I2C API read */
    i2c_if.offset = offset;
    i2c_if.buf = (char *)&reg_val;
    i2c_if.size = size;
    i2c_if.i2c_bus_type = max31730_p->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = max31730_p->i2c_p->i2c_dev;

    /* Call the I2C Read API */
    retv = max31730_p->callout_fvt->rd(&i2c_if);
    if (retv != PASSED) {
	/* Read Failed */
        sprintf(err_buf, "%s(): Failed to read reg. %lx(return code %#x)\n",
                         __func__, offset, retv);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, MAX31730_I2C_RD);
	return (FAILED);
    }

    /* Based on Maxim MAX31730 datasheet,
     * need byteswap the read back data when i2c read reg. 2-bytes in one time:
     * Temperature/Highest Temperature/Thermal High Limit/
     * Thermal Low Limit/Reference Temperature.
     * Because above registers are MSByte comes first, then LSByte.
     */
    if ((size == MAX31730_REG_TWO_BYTE_ACCESS) &&
        ((offset == MAX31730_LOC_TEMP_MSB)
         || (offset == MAX31730_R1_TEMP_MSB)
         || (offset == MAX31730_R2_TEMP_MSB)
         || (offset == MAX31730_R3_TEMP_MSB)
         || (offset == MAX31730_HST_TEMP_MSB)
         || (offset == MAX31730_LOC_HLIM_MSB)
         || (offset == MAX31730_R1_HLIM_MSB)
         || (offset == MAX31730_R2_HLIM_MSB)
         || (offset == MAX31730_R3_HLIM_MSB)
         || (offset == MAX31730_ALL_LLIM_MSB)
         || (offset == MAX31730_LOC_REF_TEMP_MSB)
         || (offset == MAX31730_R1_REF_TEMP_MSB) 
         || (offset == MAX31730_R2_REF_TEMP_MSB) 
         || (offset == MAX31730_R3_REF_TEMP_MSB))) {
        reg_val = (ushort)((reg_val >> 8) | (reg_val << 8));
    }

    *buf = (ulong)reg_val;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_i2c_wr
 * Description: Function to write Maxim MAX31730 register by I2C.
 * Inputs     : offset - offset of register to be read
 *              size   - number of bytes to be read
 *              data   - write data
 *              param  - Pointer to Maxim MAX31730 device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_i2c_wr (ulong offset, int size, ulong data, void *param)
{
    uint32 ret_val = FAILED;
    n2g_i2c_if_t i2c_if;
    char err_buf[MAX31730_ERR_BUF_SIZE];
    dev_max31730_object_t *max31730_p = (dev_max31730_object_t *)param;
    ushort wr_data =(ushort)data;

    /* Based on Maxim MAX31730 datasheet,
     * need byteswap the write in data when i2c write reg. 2-bytes in one time:
     * Thermal High Limit/Thermal Low Limit/Reference Temperature.
     * Because above registers are MSByte comes first, then LSByte.
     */
    if ((size == MAX31730_REG_TWO_BYTE_ACCESS) &&
        ((offset == MAX31730_LOC_HLIM_MSB)
         || (offset == MAX31730_R1_HLIM_MSB)
         || (offset == MAX31730_R2_HLIM_MSB)
         || (offset == MAX31730_R3_HLIM_MSB)
         || (offset == MAX31730_ALL_LLIM_MSB)
         || (offset == MAX31730_LOC_REF_TEMP_MSB)
         || (offset == MAX31730_R1_REF_TEMP_MSB) 
         || (offset == MAX31730_R2_REF_TEMP_MSB) 
         || (offset == MAX31730_R3_REF_TEMP_MSB))) {
        wr_data = (ushort)((wr_data >> 8) | (wr_data << 8));
    }

    /* Setup the interface struct for I2C API write */
    i2c_if.offset = offset;
    i2c_if.buf = (char *)&wr_data;
    i2c_if.size = size;
    i2c_if.i2c_bus_type = max31730_p->i2c_p->i2c_bus_type;
    i2c_if.i2c_dev = max31730_p->i2c_p->i2c_dev;

    /* Call the I2C Write API */
    ret_val = max31730_p->callout_fvt->wr(&i2c_if);
    if (ret_val != PASSED) {
	/* Write Failed */
        sprintf(err_buf, "%s(): Failed to write reg. %lx(return code %#x)\n",
                         __func__, offset, ret_val);
	DEV_ERROR_REPORT((dev_object_t *)param, err_buf, MAX31730_I2C_WR);
	return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: dev_maxim_max31730.c,v $
 * Revision 1.2  2019/01/10 06:23:18  wilbhuan
 * The beginning of Maxim Integrated MAX31730 Temperature Sensor device driver.
 *
 *-------------------------------------------------
 */
