/* $Id: diag_temp_sensor_lib.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_temp_sensor_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_temp_sensor_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "common.h"
#include "i2c_api.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_moka_fpga_lib.h"
#include "diag_i2c_lib.h"
#include "dev_maxim_max31730.h"
#include "diag_temp_sensor_lib.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int diag_ts_dev_create(dev_max31730_object_t *, n2g_i2c_if_t *);
static uint32_t ts_intr_confirm(boolean);


/*******************************************************************************
 *
 * Function    : diag_ts_dev_create
 * Description : Function to create Temperature sensor, Maxim max31370,
 *               device object.
 *               It includes to create common device object, attach device,
 *               and setup call-out function vectors.
 * Inputs      : ts_obj - Pointer of max31730 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_ts_dev_create (dev_max31730_object_t *ts_obj, n2g_i2c_if_t *ts_i2c_if)
{
    dev_object_t *dev = (dev_object_t *)ts_obj;

    /* Create common device object */
    max31730_dev_create(dev, (dev_error_report_t)err_report);

    /* Setup call-out function vectors */
    ts_obj->callout_fvt->open = n2g_i2c_open;
    ts_obj->callout_fvt->close = n2g_i2c_close;
    ts_obj->callout_fvt->rd = n2g_i2c_read;
    ts_obj->callout_fvt->wr = n2g_i2c_write;
    ts_obj->callout_fvt->intr_confirm = ts_intr_confirm;

    /* Setup I2C API parameter struct */
    ts_i2c_if->i2c_bus_type = CPU_I2C1;         /* I2C bus number */
    ts_i2c_if->i2c_dev = MB_I2C_ADDR_MB_TEMP;   /* I2C device enum */

    ts_obj->i2c_p = ts_i2c_if;

    /* Attach deivce */
    if (ts_obj->base.dev_object_fvt->dev_attach(dev) != PASSED) {
        printf("%s: Failed to attach Temperature sensor, Maxim max31730, chip.\n",
               __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : ts_intr_confirm
 * Description : Function to confirm Temperature sensor, Maxim max31370,
 *               interrupt status.
 *               It includes to create common device object, attach device,
 *               and setup call-out function vectors.
 * Inputs      : expt_val - expected value(0:No pending/1:Interrupt pending)
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t ts_intr_confirm (boolean expt_val)
{
    uint reg_addr = (uint)FPGA_EXTER_INT_PENDING_REG;
    uint reg_msk = (uint)MB_THERM_INTERRUPT_PENDING;
    uint reg_val = 0;
    uint chk_val = (uint)(expt_val << EIPR_MB_THERM_INTR); 

    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read FPGA register 0x%04X.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    reg_val |= reg_msk;
    if (fpga_write_32_reg(reg_addr, reg_val) != PASSED) {
        printf("%s(%d): Failed to write FPGA register 0x%04X.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    reg_val = (uint)(~chk_val);
    if (fpga_read_32_reg(reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read FPGA register 0x%04X.\n",
               __func__, __LINE__, reg_addr);
        return (FAILED);
    }

    if ((reg_val & reg_msk) != chk_val) {
        printf("%s(): MB Thermal sensor interrupt status is NOT as expected.\n",
               __func__);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_temp_sensor_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
