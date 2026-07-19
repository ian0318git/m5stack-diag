/* $Id: plug_temp_sensor_lib.c,v 1.2 2018/01/20 04:53:29 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_temp_sensor_lib.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : plug_temp_sensor_lib.c
 * Description: Pluggable Temperature Sensor Library
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include <stdio.h>

#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "proto.h"
#include "dev_tmpx75.h"
#include "plug_temp_sensor_lib.h"
#include "plug_common_host_impl.h"

extern uint32 err_report(dev_object_t *, char *, uint32);

static uint32_t plug_ts_i2c_rd(uint32, ushort *);
static uint32_t plug_ts_i2c_wr(uint32, ushort *);

int plug_ts_dev_create(dev_tmpx75_object_t *);
int plug_ts_show_temp(void);

extern int plug_curr_i2c_ctrl;

/*******************************************************************************
 *
 * Function    : plug_ts_dev_create
 * Description : Function to create TMPX75 Device Object
 * Inputs      : gpio_obj - Pointer of TMPX75 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_ts_dev_create (dev_tmpx75_object_t *ts_obj)
{
    dev_object_t *dev = (dev_object_t *)ts_obj;

    /* Create common device object */
    tmpx75_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    ts_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    ts_obj->callout_fvt->rd = plug_ts_i2c_rd;
    ts_obj->callout_fvt->wr = plug_ts_i2c_wr;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_ts_show_temp
 * Description : Function to display temperature
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_ts_show_temp (void) 
{
    dev_tmpx75_object_t ts_data;
    dev_tmpx75_object_t *ts_obj = &ts_data;
    int ret;

    plug_ts_dev_create(ts_obj);

    if (ts_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret = ts_obj->callin_fvt->show_temp((dev_object_t *)ts_obj);

    ts_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&ts_obj);

    return (ret);
}

 /*******************************************************************************
 *
 * Function    : plug_ts_i2c_rd
 * Description : Function implementation of TMPX75 I2C Read
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t plug_ts_i2c_rd (uint32 offset, ushort *data) 
{
    return (plug_common_host_i2c_rd_2bytes(plug_curr_i2c_ctrl, PLUG_I2C_ADDR_TEMP, 
                                           offset, data));
}


/*******************************************************************************
 *
 * Function    : plug_ts_i2c_wr
 * Description : Function implementation of TMPX75 I2C Write
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t plug_ts_i2c_wr (uint32 offset, ushort *data)
{
    return (plug_common_host_i2c_wr_2bytes(plug_curr_i2c_ctrl, PLUG_I2C_ADDR_TEMP,
                                           offset, *data));
}

/*-------------------------------------------------
$Log: plug_temp_sensor_lib.c,v $
Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:40:40  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.2  2017/07/20 17:23:10  tirawan
Add Pluggable host implementation codes

Revision 1.1.2.1  2017/07/13 06:32:19  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.6  2017/06/25 06:40:47  tirawan
Initialize mux number in i2c read/write function

Revision 1.1.2.5  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

