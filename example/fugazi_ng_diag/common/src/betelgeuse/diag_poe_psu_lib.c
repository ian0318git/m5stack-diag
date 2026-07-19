/* $Id: diag_poe_psu_lib.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_poe_psu_lib.c,v $
 *------------------------------------------------------------------
 * 
 * diag_poe_psu_lib.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include "common.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "error.h"
#include "diag_moka_fpga_lib.h"
#include "diag_i2c_lib.h"
#include "platform_i2c.h"
#include "diag_poe_psu_lib.h"
#include "dev_tps2386b.h"

/******************************************************************************
 *                          Function Declaration
 ******************************************************************************/
int diag_poe_dev_create(void *, void *);
static int diag_poe_chk_intr_assert(void);
static int diag_poe_chk_intr_deassert(void);

/******************************************************************************
 * Function   : platform_has_poe
 *
 * Description: Function to check if this platform has PoE feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 ******************************************************************************/
boolean platform_has_poe (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = (uint)FPGA_CARD_AND_PWR_REG;
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA Card and Power Present reg."
               "(0x%04X)\n", __FUNCTION__, reg_offset);
        return (FALSE);
    }
 
    /* PoE module is option for customer.
     * So determine to show PoE related diags by check if PoE module is present.
     */
    if ((reg_val & FPGA_CPP_POE_PRESENT) != FPGA_CPP_POE_PRESENT) {
        return (FALSE);
    }
    return (TRUE);
}

/******************************************************************************
 * Function    : diag_poe_dev_create
 *
 * Description : Function to create PoE device "TPS2386B" object.
 *               It includes to create common device object, attach device,
 *               and setup call-out function vectors.
 * Inputs      : poe_obj - Pointer of device object
 * Outputs     : PASSED / FAILED
 ******************************************************************************/
int diag_poe_dev_create (void *poe_obj_param, void *poe_i2c_if_param)
{
    dev_tps2386b_object_t *poe_obj = poe_obj_param;
    n2g_i2c_if_t *poe_i2c_if = poe_i2c_if_param;
    dev_object_t *dev = (dev_object_t *)poe_obj;

    /* Create common device object */
    tps2386b_dev_create(dev, (dev_error_report_t)err_report);

    /* Setup call-out function vectors */
    poe_obj->callout_fvt->open = n2g_i2c_open;
    poe_obj->callout_fvt->close = n2g_i2c_close;
    poe_obj->callout_fvt->rd = n2g_i2c_read;
    poe_obj->callout_fvt->wr = n2g_i2c_write;
    poe_obj->callout_fvt->chk_intr_assert = diag_poe_chk_intr_assert;
    poe_obj->callout_fvt->chk_intr_deassert = diag_poe_chk_intr_deassert;

    /* Setup I2C API parameter struct */
    poe_i2c_if->i2c_bus_type = CPU_I2C2;                /* I2C bus number */
    poe_i2c_if->i2c_dev = MB_I2C_ADDR_POE_30W_CTRLER;   /* I2C device enum */

    poe_obj->i2c_p = poe_i2c_if;

    /* Attach deivce */
    if (poe_obj->base.dev_object_fvt->dev_attach(dev) != PASSED) {
        printf("%s:%d:Failed to attach TPS2386B device driver\n",
               __func__, __LINE__);
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 * Function   : diag_poe_chk_intr_assert
 *
 * Description: Checing whether the Interrupt pin is asserted
 * Inputs     : NONE
 * Outputs    : PASSED / FAILED
 ******************************************************************************/
static int diag_poe_chk_intr_assert (void) 
{
   return (diag_check_ext_intr_pending(PENDING_BIT_POE));
}

/******************************************************************************
 * Function   : diag_poe_chk_intr_deassert
 *
 * Description: Checing whether the Interrupt pin is de-asserted
 * Inputs     : NONE
 * Outputs    : PASSED / FAILED
 ******************************************************************************/
static int diag_poe_chk_intr_deassert (void) 
{
   return (diag_check_ext_intr_no_pending(PENDING_BIT_POE));
}

/*-------------------------------------------------
 * $Log: diag_poe_psu_lib.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
