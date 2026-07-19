/* $Id: diag_temperature_sensor_lib.c,v 1.2 2013/10/08 08:48:29 tirawan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/woodlawn_linux/diag_temperature_sensor_lib.c,v $ 
 *-----------------------------------------------------------------------------
 * diag_temperature_sensor_lib.c - Utility Menu and Functions for Woodlawn 
 *
 * February 2012, Leslie Chen
 * Copyright (c) 2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "i2c_api.h"
#include "platform_i2c.h"
#include "i2c_dev.h"
#include "n2g_api_rc.h"
#include "dev_tmp421.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************
 *  Externs
 *****************************************************************************/
extern uint32 err_report(dev_object_t *, char *, uint32);
extern void tmp421_dev_create(dev_object_t *, dev_error_report_t);
extern int32_t cavium_i2c_fd1;

/******************************************************************************
 *  Functions Declaration
 *****************************************************************************/
static uint32_t tmp421_i2c_read(uint32, uint *);
static uint32_t tmp421_i2c_write(uint32, uint *);

/******************************************************************************
 *  TMP421 Device Driver Object
 *****************************************************************************/
static int tmp421_dev_init = FALSE;
static dev_tmp421_object_t tmp421_obj;

/*****************************************************************************
 *
 * Function: im_get_pca9541_obj
 *
 * Description: Get PCA9541 device driver object for IM
 *
 * Inputs      : None
 * Outputs     : p_obj - TMP421 object pointer
 *
 *****************************************************************************/
dev_object_t *get_tmp421_obj (void)
{
    dev_tmp421_object_t *p_obj = &tmp421_obj;

    if (tmp421_dev_init == FALSE) {

        /* Create the device driver object */
        tmp421_dev_create((dev_object_t *)p_obj,
                           (dev_error_report_t)err_report);

        /* Attach the callin function */
        p_obj->base.dev_object_fvt->dev_attach((dev_object_t *)p_obj);

        /* Assign the callout function */
        p_obj->callout_fvt->rd  = tmp421_i2c_read;
        p_obj->callout_fvt->wr = tmp421_i2c_write;

        tmp421_dev_init = TRUE;
    }

    return ((dev_object_t *)p_obj);
}

/******************************************************************************
 *
 * Function    : tmp421_i2c_read
 * Description : TMP421 Register Read through Cavium I2C iface
 * Input       : addr  - register offset.
 *               buf   - read buffer
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
static uint32_t tmp421_i2c_read (uint32 addr, uint *buf)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_TMP421;
    int rv;

    uchar tmp_buf;
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to read back the register value */
    rv = read_i2c_reg(&i2c_dev, &tmp_buf, addr, sizeof(tmp421_p));
    *buf = tmp_buf;
    return (rv);
}

/******************************************************************************
 *
 * Function    : tmp421_i2c_write
 * Description : TMP421 Register Write through Cavium I2C iface
 * Input       : addr  - register offset.
 *               data  - data for write
 *
 * Output: PASSED/FAILED
 *
 *****************************************************************************/
static uint32_t tmp421_i2c_write (uint32 addr, uint *data)
{
    n2g_i2c_dev_t i2c_dev;
    uint i2c_slave_addr;
    i2c_slave_addr = CAVIUM_TMP421;
    
    /* Open the Cavium I2C bus 1 */
    if (open_i2c(&i2c_dev, i2c_slave_addr, CPU_I2C1) == FAILED) {
        return (FAILED);
    }

    /* Call the I2C common I2C api to write the register value */
    return (write_i2c_reg(&i2c_dev, (uchar *)data, addr, sizeof(tmp421_p)));
}

/*-------------------------------------------------
 * $Log: diag_temperature_sensor_lib.c,v $
 * Revision 1.2  2013/10/08 08:48:29  tirawan
 * Woodlawn collapsed to main trunk
 *
 * Revision 1.1.4.2  2013/08/20 10:58:54  tirawan
 * Branch into woodlawn-branch2 and port woodlawn code
 *
 * Revision 1.1.2.1  2013/04/24 10:37:18  tirawan
 * Initial check-in for woodlawn linux code
 *
 * Revision 1.1  2013/03/13 06:42:53  kuangik
 * Add for the first time
 *
 * Revision 1.9  2012/10/24 10:39:02  leslie
 * Fix and clean up code.
 *
 * Revision 1.8  2012/08/30 06:33:50  leslie
 * Fix the issue of dump register and register test.
 *
 * Revision 1.7  2012/08/18 02:36:23  leslie
 * Open I2C bus 1
 *
 * Revision 1.6  2012/08/03 10:16:55  evanli
 * Mapping to latest O2 source code on 20120726
 *
 * Revision 1.4  2012/07/19 06:20:09  leslie
 * Remove library functoin open_i2c
 *
 * Revision 1.3  2012/05/30 01:39:50  leslie
 * Open i2c_1 instead of open i2c_0
 *
 * Revision 1.2  2012/03/26 07:19:32  kody
 * Modify and add TMP421 temperature sensor test code.
 *
 * Revision 1.1  2012/02/10 07:02:57  leslie
 * Add Woodlawn temperature sensor lib file.
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
