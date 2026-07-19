/* $Id: dev_nxp_lm75b.c,v 1.3 2018/08/06 02:30:59 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_nxp_lm75b/dev_nxp_lm75b.c,v $
 *------------------------------------------------------------------
 * Filename   :	dev_nxp_lm75b.c
 * Description:	Common driver of NXP LM75B, a digital
 *              temperature sensor and thermal watchdog.
 *              (Type number: LM75BD/LM75BDP/LM75BGD/LM75BTP)
 *
 * Copyright (c) 2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "defs.h"
#include "common.h"
#include "dev_nxp_lm75b.h"
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
void          lm75b_dev_create(dev_object_t *, dev_error_report_t);
static uint32 dev_lm75b_attach(dev_object_t *);
static int    dev_i2c_rd(ulong, int, ulong *, void *);
static int    dev_i2c_wr(ulong, int, ulong, void *);
static uint32 dev_lm75b_detach(dev_object_t *);
static uint32 dev_lm75b_reconfig(dev_object_t *, void *, boolean *);
static uint32 dev_lm75b_restart(dev_object_t *);
static uint32 dev_lm75b_crsh(dev_object_t *, print_fn_t, dev_show_cmd);
static void   dev_lm75b_destroy(dev_object_t **);
static int    dev_lm75b_show_reg(dev_object_t *);
static int    dev_lm75b_dump_reg(dev_object_t *);
static int    dev_lm75b_alter_reg(dev_object_t *);
static int    dev_lm75b_test_reg(dev_object_t *dev);
static int    dev_lm75b_show_temp(dev_object_t *);

static char   lm75b_err_buf[LM75B_ERR_BUF_SIZE];

/*******************************************************************************
 *                               Global variables                              *
 *******************************************************************************
 */
#define LM75B_REG_RO_FLAG               (READ_ONLY | REG_ACCESS)
/* No REG_ACCESS here accroding to different reg size */
#define LM75B_REG_RW_FLAG               (READ_WRITE| SAVE_RESTORE | REG_ACCESS)

/* Registers test table */
static reg_info_t_ext lm75b_1byte_reg_ext = {1, dev_i2c_rd, dev_i2c_wr, 0};
static reg_info_t_ext lm75b_2byte_reg_ext = {2, dev_i2c_rd, dev_i2c_wr, 0};

/* LM75B registers table. This device has registers with different sizes.
 */
reg_info_t lm75b_reg_table[] =
{
    {"Temperature",                   LM75B_TEMP,    LM75B_REG_RO_FLAG,
     {(ulong)&lm75b_2byte_reg_ext},   0xFFE0,        0x0000},
    {"Configuration",                 LM75B_CONF,    LM75B_REG_RW_FLAG,
     {(ulong)&lm75b_1byte_reg_ext},   0x1F,          0x00},
    {"Thyst",                         LM75B_THYST,   LM75B_REG_RW_FLAG,
     {(ulong)&lm75b_2byte_reg_ext},   0xFF80,        0x4B00},
    {"Tos",                           LM75B_TOS,     LM75B_REG_RW_FLAG,
     {(ulong)&lm75b_2byte_reg_ext},   0xFF80,        0x5000},
    {0, 0, 0, {0}, 0, 0},
};


/*******************************************************************************
 *
 * Function   : lm75b_dev_create
 * Description:	Create object with various device function.
 * Inputs     : *dev          - dev_object_t pointer to NXP LM75B device
 *		err_report_fn - error reporting function pointer
 * Outputs    : none
 *
 *******************************************************************************
 */
void lm75b_dev_create (dev_object_t *dev, dev_error_report_t err_report_fn)
{
    dev_object_fvt_t	*dev_fvt;
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;

    /* Allocate memory for the device object */
    dev_fvt = (dev_object_fvt_t *)(malloc(sizeof(dev_object_fvt_t)));
    if (dev_fvt == NULL) {
        /* Unable to allocate memory */
        err_report_fn(dev, "malloc failed at dev_create.", LM75B_DEV_STATE);
	return;
    }

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, dev_fvt);

    lm75b->base.dev_object_fvt->dev_attach	      = dev_lm75b_attach;
    lm75b->base.dev_object_fvt->dev_detach	      = dev_lm75b_detach;
    lm75b->base.dev_object_fvt->dev_reconfig_needed   = dev_lm75b_reconfig;
    lm75b->base.dev_object_fvt->dev_restart           = dev_lm75b_restart;
    lm75b->base.dev_object_fvt->dev_error_report      = err_report_fn;
    lm75b->base.dev_object_fvt->dev_collect_crashinfo = dev_lm75b_crsh;
    lm75b->base.dev_object_fvt->dev_destroy	      = dev_lm75b_destroy;
    lm75b->base.dev_object_fvt->dev_name              = "NXP LM75B";

    lm75b->callin_fvt = (lm75b_callin_fvt_t *)
                         (malloc(sizeof(lm75b_callin_fvt_t)));
    lm75b->callout_fvt = (lm75b_callout_fvt_t *)
                          (malloc(sizeof(lm75b_callout_fvt_t)));

    lm75b->base.dev_state = DEV_STATE_CREATE;
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_attach
 * Description:	Attach NXP LM75B device for use.
 *              This function will initialize, setup all necessary pointers
 *              and bring the chip to operation.
 * Inputs     : Pointer to the NXP LM75B device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32 dev_lm75b_attach (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;

    if (lm75b->callin_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "callin_fvt is NULL", LM75B_ATTACH);
        return (FAILED);
    }

    if (lm75b->callout_fvt == NULL) {
        DEV_ERROR_REPORT(dev, "callout_fvt is NULL", LM75B_ATTACH);
        return (FAILED);
    }

    /* init the call in function */
    lm75b->callin_fvt->register_test  = dev_lm75b_test_reg;
    lm75b->callin_fvt->show_register  = dev_lm75b_show_reg;
    lm75b->callin_fvt->alter_register = dev_lm75b_alter_reg;
    lm75b->callin_fvt->dump_register  = dev_lm75b_dump_reg;
    lm75b->callin_fvt->show_temp      = dev_lm75b_show_temp;

    lm75b->base.dev_state = DEV_STATE_ATTACH;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_detach
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
 * Inputs     : Pointer to NXP LM75B device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static uint32 dev_lm75b_detach (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;

    /* Init the device object structure to default "do nothing" */
    init_default_dev_object(dev, lm75b->base.dev_object_fvt);

    lm75b->base.dev_state = DEV_STATE_DETACH;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_reconfig_needed
 * Description:	To check whether device re-configuration is needed during
 *              (re)initialization. Based on the provided context information,
 *              the boolean return value, and possibly other factors external
 *              to the device object, the caller shall decide whether to invoke
 *              either dev_restart or dev_init, but not both. In general, the
 *        	boolean return value alone is not sufficient to decide whether
 *              the device can safely be restarted or whether it must be fully
 *              initialized from scratch.
 * Inputs     : *dev            - dev_object_t pointer to NXP LM75B device
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
static uint32 dev_lm75b_reconfig (dev_object_t *dev, void *context_handle,
                                  boolean *reconfig)
{
    *reconfig = FALSE;   /* No need to reconfig from scratch */
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_restart
 * Description:	To restart a previously initialized device without resetting
 *              the device or changing its configuration.
 *              For example, during a failover event.
 *
 *              Change the state of the device from its current state
 *              to an initial state. Also, dev_state must be assigned the
 *              value of DEV_STATE_INIT.
 * Inputs     : *dev - dev_object_t pointer to NXP LM75B device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	The dev_attach() and dev_reconfig_needed() functions has been
 *              called and successfully executed.
 *
 *******************************************************************************
 */
static uint32 dev_lm75b_restart (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;

    lm75b->base.dev_state = DEV_STATE_INIT;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_crsh
 * Description:	Allow platforms to collect data from a device during a crash.
 *              Print data to the crash log (via the provide print error) using
 *              the appropriate verbisity level requested by the host
 * Inputs     : *dev      - dev_object_t pointer to NXP LM75B device
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
static uint32 dev_lm75b_crsh (dev_object_t *dev, print_fn_t dev_print,
                              dev_show_cmd verbosity)
{
    /* more development in this section */
    dev_print("dev_lm75b_crsh(): No Crash info available for NXP LM75B.\n");
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_destroy
 * Description:	Destroy the dev_object structure and free all the resources.
 * Inputs     : **dev - dev_object_t pointer to NXP LM75B device
 * Outputs    : none
 *
 * Assumptions:	The dev_attch() function has been called and successfully.
 *
 *******************************************************************************
 */
static void dev_lm75b_destroy (dev_object_t **dev)
{
    dev_lm75b_object_t *lm75b;

    if (dev == NULL) {
        return;
    }

    if (*dev == NULL) {
        return;
    }

    lm75b = (dev_lm75b_object_t *)*dev;

    if (lm75b->callout_fvt) {
        free(lm75b->callout_fvt);   /* Free callout struct */
    }

    if (lm75b->callin_fvt) {
        free(lm75b->callin_fvt);   /* Free callin struct */
    }

    free(lm75b->base.dev_object_fvt);   /* Free dev_object_t */
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_show_reg
 * Description:	Function to get and show NXP LM75B reigster.
 * Inputs     : *dev - dev_object_t pointer to the LM75B device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *******************************************************************************
 */
static int dev_lm75b_show_reg (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;
    uint32             reg_offset = 0;
    ulong              reg_val = 0;
    reg_info_t         *reg_p = &lm75b_reg_table[0];

    printf("\n");

    reg_offset = (uint32)gethex_answer("Enter register offset (0x0 ~ 0x03)",
                                       LM75B_TEMP, LM75B_TEMP, LM75B_TOS);

    reg_p += reg_offset;

    if (reg_p->size.ext->rd_ptr((ulong)reg_offset,
                                reg_p->size.ext->size,
                                (ulong *)&reg_val,
                                (void *)lm75b) != PASSED) {
        printf("%s: Failed to read register %#x.\n", __func__, reg_offset);
        return (FAILED);
    }

    printf("%s %s reg.(%#x): ",
           lm75b->base.dev_object_fvt->dev_name,
           reg_p->name,
           reg_p->offset);

    if (reg_p->size.ext->size == 1) {
        /* 1-byte register */
        printf("0x%02X\n", (ushort)reg_val);
    } else {
        /* 2-byte register */
        printf("0x%04X\n", (ushort)reg_val);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_alter_reg
 * Description:	Function to alter NXP LM75B register.
 * Inputs     : *dev - dev_object_t pointer to NXP LM75B device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_lm75b_alter_reg (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;
    uint32             reg_offset = 0;
    ulong              reg_val = 0;
    reg_info_t         *reg_p = &lm75b_reg_table[0];

    printf("\n");

    reg_offset = (uint32)gethex_answer("Enter register offset(0x0 ~ 0x03)",
                                       LM75B_TEMP, LM75B_TEMP, LM75B_TOS);

    reg_p += reg_offset;

    if (reg_p->size.ext->rd_ptr((ulong)reg_offset,
                                reg_p->size.ext->size,
                                (ulong *)&reg_val,
                                (void *)lm75b) != PASSED) {
        printf("%s: Failed to read register %#x.\n", __func__, reg_offset);
        return (FAILED);
    }

    printf("Current %s %s reg.(%#x): ",
           lm75b->base.dev_object_fvt->dev_name,
           reg_p->name,
           reg_p->offset);

    if (reg_p->size.ext->size == 1) {
        /* 1-byte register */
        printf("0x%02X\n", (ushort)reg_val);
    } else {
        /* 2-byte register */
        printf("0x%04X\n",  (ushort)reg_val);
    }

    /* Alter register with new value */
    reg_val = (ushort)gethex_answer("Enter the new data in hex",
                                    reg_val, 0, 0xFFFF);

    if (reg_p->size.ext->wr_ptr((ulong)reg_offset,
                                reg_p->size.ext->size,
                                (ulong)reg_val,
                                (void *)lm75b) != PASSED) {
        printf("%s: Failed to read register %#x.\n", __func__, reg_offset);
        return (FAILED);
    }

    reg_val = 0;
    if (reg_p->size.ext->rd_ptr((ulong)reg_offset,
                                reg_p->size.ext->size,
                                (ulong *)&reg_val,
                                (void *)lm75b) != PASSED) {
        printf("%s: Failed to read register %#x.\n", __func__, reg_offset);
        return (FAILED);
    }

    printf("Current %s %s reg.(%#x): ",
           lm75b->base.dev_object_fvt->dev_name,
           reg_p->name,
           reg_p->offset);

    if (reg_p->size.ext->size == 1) {
        /* 1-byte register */
        printf("0x%02X\n",  (ushort)reg_val);
    } else {
        /* 2-byte register */
        printf("0x%04X\n",  (ushort)reg_val);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_dump_reg
 * Description:	Function to dump all NXP LM75B reigsters.
 * Inputs     : *dev - dev_object_t pointer to the LM75B device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions:	The device printf function vector has been provided by the host
 *              platform which implements the print logging functionality. The
 *              dev_attach() function has been called and successfully executed
 *
 *******************************************************************************
 */
static int dev_lm75b_dump_reg (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;
    ulong              reg_val = 0;
    reg_info_t         *reg_p = &lm75b_reg_table[0];
    int                ctr = 0, total_reg_num = 0;

    total_reg_num = (int)((sizeof(lm75b_reg_table) / sizeof(reg_info_t)) - 1);
    for (ctr = 0; ctr < total_reg_num; ctr++, reg_p++) {
        reg_val = 0;

        if (reg_p->size.ext->rd_ptr((ulong)reg_p->offset,
                                    reg_p->size.ext->size,
                                    (ulong *)&reg_val,
                                    (void *)lm75b) != PASSED) {
            printf("%s: Failed to read register %#x.\n",
                   __func__, reg_p->offset);
            return (FAILED);
        }

        if (ctr == 0) {
            printf("\n%s registers:\n", lm75b->base.dev_object_fvt->dev_name);
        }

        printf("%-13s reg.(0x%02X): ",
               reg_p->name,
               reg_p->offset);

        if (reg_p->size.ext->size == 1) {
            /* 1-byte register */
            printf("0x%02X\n", (ushort)reg_val);
        } else {
            /* 2-byte register */
            printf("0x%04X\n", (ushort)reg_val);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_test_reg
 * Description: Function to test I2C interface between Host and NXP LM75B
 *              by accessing its registers.
 *              Restores original register values after test.
 * Inputs     : *dev - dev_object_t pointer to NXP LM75B device
 * Outputs    : PASSED/FAILED
 *
 * Assumptions: create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_lm75b_test_reg (dev_object_t *dev)
{
    lm75b_1byte_reg_ext.param = (void *)dev;
    lm75b_2byte_reg_ext.param = (void *)dev;

    if (register_tests(0, lm75b_reg_table) != PASSED) {
        sprintf(lm75b_err_buf, "%s: NXP LM75B registers Test Failed", __func__);
        DEV_ERROR_REPORT(dev, lm75b_err_buf, LM75B_REG_TEST);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_lm75b_show_temp
 * Description: Display temperature
 * Inputs     : *dev - dev_object_t pointer to NXP LM75B device
 * Outputs    : PASSED/FAILED 
 *
 * Assumptions: create and dev_attach have to be called first.
 *              dev_destroy will also be called after the exit.
 *
 *******************************************************************************
 */
static int dev_lm75b_show_temp (dev_object_t *dev)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)dev;
    ushort             reg_val = 0, temp_data = 0;
    reg_info_t         *reg_p = &lm75b_reg_table[LM75B_TEMP];
    float              temp_in_deg_c = 0;

    if (reg_p->size.ext->rd_ptr((ulong)reg_p->offset,
                                reg_p->size.ext->size,
                                (ulong *)&reg_val,
                                (void *)lm75b) != PASSED) {
        printf("%s: Failed to read register %#x.\n",
               __func__, reg_p->offset);
        return (FAILED);
    }

    /* Based on NXP LM75B datasheet,
     * only the 11 most significant bits should be used.
     * And way to calculate the Temp value in degreeC from 11-bit Temp data is:
     * 1. If Temp date MSByte bit D10 = 0, then temp is positive and
     *    Temp value(degreeC) = +(Temp data) x 0.125 degreeC
     * 2. If Temp data MSByte bit D10 = 1, then temp is negative and
     *    Temp value(degreeC) = -(two's complement of Temp data) x 0.125 degreeC
     */
    if ((reg_val & LM75B_TEMP_SIGN_BIT) == LM75B_TEMP_SIGN_BIT) {
        reg_val = ~reg_val;
        temp_data = (ushort)(reg_val >> 5) + 1;
        temp_in_deg_c = (temp_data * LM75B_TEMP_RESOLUTION * (-1));
    } else {
        temp_data = (ushort)(reg_val >> 5);
        temp_in_deg_c = (temp_data * LM75B_TEMP_RESOLUTION);
    }

    printf("Temperature: %.3f degree Celcius.\n", temp_in_deg_c);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_i2c_rd
 * Description: Function to read NXP LM75B register by I2C.
 * Inputs     : offset - offset of register to be written
 *              size   - number of bytes to write
 *              *buf   - pointer to the data buffer to be read
 *              param  - pointer to NXP LM75B device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_i2c_rd (ulong offset, int size, ulong *buf, void *param)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)param;
    ushort             reg_val = 0;

    if (lm75b->callout_fvt->rd((uint32)offset, (ushort *)&reg_val) != PASSED) {
        return (FAILED);
    }

    /* Based on NXP LM75B datasheet,
     * when read 2-byte register(Temp, Tos, or Thyst),
     * I2C data are MSByte comes first, then LSByte.
     * So do byteswap here.
     */
    if (size == 2) {
        reg_val = (ushort)((reg_val >> 8) | (reg_val << 8));
    }

    *buf = (ulong)reg_val;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dev_i2c_wr
 * Description: Function to write NXP LM75B register by I2C.
 * Inputs     : offset - offset of register to be read
 *              size   - number of bytes to be read
 *              data   - write data
 *              param  - Pointer to NXP LM75B device object
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int dev_i2c_wr (ulong offset, int size, ulong data, void *param)
{
    dev_lm75b_object_t *lm75b = (dev_lm75b_object_t *)param;
    ushort             wr_data =(ushort)data;

    /* Based on NXP LM75B datasheet,
     * when read 2-byte register(Temp, Tos, or Thyst),
     * I2C data are MSByte comes first, then LSByte.
     * So do byteswap here.
     */
    if (size == 2) {
        wr_data = (ushort)((wr_data >> 8) | (wr_data << 8));
    }

    if (lm75b->callout_fvt->wr((uint32)offset, (ushort *)&wr_data) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: dev_nxp_lm75b.c,v $
Revision 1.3  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.2.12.2  2018/04/23 09:28:43  lucywang
Fixed Temp Sensor Interrupt test bug in low temp

Revision 1.2.12.1  2018/03/29 06:37:41  lucywang
Fixed bug of Thermal Sensor Test

Revision 1.2  2018/01/20 04:14:46  hondwang
merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/09 09:20:24  hondwang
Add for Star project

Revision 1.1.2.1  2017/07/04 15:07:52  palin2
Created common driver for NXP LM75B digital temperature sensor.

$Endlog$
*/
