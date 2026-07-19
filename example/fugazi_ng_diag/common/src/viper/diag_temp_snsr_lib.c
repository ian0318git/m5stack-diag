 /* $Id: diag_temp_snsr_lib.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_temp_snsr_lib.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : diag_temp_sensor_lib.c
 * Description: Viper NXP LM75BD module Temperature Sensor Library
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
#include <string.h>

#include "common.h"
#include "proto.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "platform_fru.h"
#include "i2c_api.h"
#include "dev_object.h"
#include "diag_i2c_addr.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "diag_i2c_lib.h"
#include "diag_fpga.h"
#include "platform_cookie.h"
#include "dnv_gpio_lib.h"
#include "i2c_dev.h"
#include "dev_nxp_lm75b.h"
#include "diag_temp_snsr_lib.h"

extern uint32 err_report(dev_object_t *, char *, uint32);

static uint32_t diag_ts_i2c_rd(uint32, ushort *);
static uint32_t diag_ts_i2c_wr(uint32, ushort *);

int diag_ts_dev_create(dev_lm75b_object_t *);
int diag_ts_show_temp(void);

/*******************************************************************************
 *
 * Function    : diag_ts_dev_create
 * Description : Function to create LM75BD Device Object
 * Inputs      : gpio_obj - Pointer of TMPX75 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_ts_dev_create (dev_lm75b_object_t *ts_obj)
{
    dev_object_t *dev = (dev_object_t *)ts_obj;

    /* Create common device object */
    lm75b_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    ts_obj->base.dev_object_fvt->dev_attach(dev);

    /* Setup call-out function vectors */
    ts_obj->callout_fvt->rd = diag_ts_i2c_rd;
    ts_obj->callout_fvt->wr = diag_ts_i2c_wr;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_ts_show_temp
 * Description : Function to display MB temperature
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_ts_show_temp (void) 
{
    dev_lm75b_object_t ts_data;
    dev_lm75b_object_t *ts_obj = &ts_data;
    int ret;

    diag_ts_dev_create(ts_obj);

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
 * Function    : diag_ts_i2c_rd
 * Description : Function implementation of LM75BD I2C Read
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
uint32_t diag_ts_i2c_rd (uint32 offset, ushort *data) 
{
    n2g_i2c_if_t *i2c_if;
    

	i2c_if = (n2g_i2c_if_t *)(get_n2g_i2c_if(I2C_CTRL_ZERO,
										     I2C_MUX_ZERO,
										     MB_I2C_ADDR_MB_TEMP_LM75));
                                             
    i2c_if->size = sizeof(ushort);
    i2c_if->buf = (char *)data;
    i2c_if->offset = offset;

    if (n2g_i2c_read(i2c_if) != PASSED) {
        printf("%s: n2g_i2c_read failed\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_ts_i2c_wr
 * Description : Function implementation of LM75BD I2C Write
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
uint32_t diag_ts_i2c_wr (uint32 offset, ushort *data)
{
    int      diag_i2c_fd = get_i2c_fd(CPU_I2C0);
    int      i2c_dev_addr = MB_I2C_ADDR_MB_TEMP_LM75, ret_code = 0;
    
    if (diag_i2c_fd < 0) {
        printf("%s:%d i2c-2 descriptor is not exists.\n", __func__, __LINE__);
        return (FAILED);
    }

    ret_code = ioctl(diag_i2c_fd, I2C_SLAVE, i2c_dev_addr);
    if (ret_code < 0) {
        printf("%s:%d Failed to connect to device %#x(rc = %#x).",
               __func__, __LINE__, i2c_dev_addr, ret_code);
        return (FAILED);
    }

    if (i2c_smbus_write_word_data(diag_i2c_fd, offset, *data) < 0) {
        printf("%s %d Failed to do I2C write %#x to %#x of device %#x.",
               __func__, __LINE__, *data, offset, i2c_dev_addr);
        return (FAILED);
    }
    return (PASSED);
}

/*------------------------------------------------------------------
$Log: diag_temp_snsr_lib.c,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.4  2018/07/03 05:38:55  harrchan
Follow the coding rule to clean up code

Revision 1.1.2.3  2018/04/11 08:52:32  lucywang
Modified Thermal Interrupt test to use chip object

Revision 1.1.2.2  2018/04/02 07:18:30  lucywang
Added Interrupt test for Thermal Sensor

Revision 1.1.2.1  2018/03/28 07:55:52  lucywang
Changed Thermal sersor to LM75B, TBD : bug fix


$Endlog$
*/
