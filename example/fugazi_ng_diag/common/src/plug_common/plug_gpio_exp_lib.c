/* $Id: plug_gpio_exp_lib.c,v 1.3 2018/11/23 09:02:32 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_gpio_exp_lib.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : plug_gpio_exp_lib.c
 * Description: Pluggable GPIO Expander Library Functions
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
#include "plug_slot.h"
#include "plug_gpio_exp_test.h"
#include "plug_gpio_exp_lib.h"
#include "plug_common_host_impl.h"

static int plug_gpio_exp_dev = MANDATORY;
 
extern uint32 err_report(dev_object_t *, char *, uint32);

static uint32_t plug_gpio_i2c_rd(uint32, ushort *);
static uint32_t plug_gpio_i2c_wr(uint32, ushort *);

int plug_gpio_exp_dev_create(dev_pca9555_object_t *);
void plug_gpio_exp_set_device(int);
int plug_gpio_exp_drive_port(int, int, int, int); 
int plug_gpio_exp_read_port(int, int, int, int *);
int plug_gpio_exp_config_port(int, int, int, int);

extern int plug_curr_i2c_ctrl;


/*******************************************************************************
 *
 * Function    : plug_gpio_exp_dev_create
 * Description : Function to create PCA9555 Device Object
 * Inputs      : gpio_obj - Pointer of PCA9555 device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_dev_create (dev_pca9555_object_t *gpio_obj)
{
    dev_object_t *dev = (dev_object_t *)gpio_obj;

    /* Create common device object */
    pca9555_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    if (gpio_obj->base.dev_object_fvt->dev_attach) {
        gpio_obj->base.dev_object_fvt->dev_attach(dev);
    } else {
        printf("%s: Something is wrong. Attach function is NULL\n", __func__);
        return (FAILED);
    }

    /* Setup call-out function vectors */
    gpio_obj->callout_fvt->rd = plug_gpio_i2c_rd;
    gpio_obj->callout_fvt->wr = plug_gpio_i2c_wr;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : plug_gpio_exp_set_device
 * Description : Function to set GPIO Expander to Mandatory or Optional
 * Inputs      : which_dev - Mandatory or Optional
 * Outputs     : None
 *
 *******************************************************************************
 */
void plug_gpio_exp_set_device (int which_dev)
{
    plug_gpio_exp_dev = which_dev;    
}


/*******************************************************************************
 *
 * Function    : plug_gpio_exp_config_port
 * Description : Function to configure port direction to Output/Input
 * Inputs      : which_dev - Mandatory or Optional
 *               which_port - Port 0 or Port 1
 *               which_pin - 0 ~ 15
 *               direction - Input/Output
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_config_port (int which_dev, int which_port, 
                               int which_pin, int direction) 
{
    dev_pca9555_object_t pca_data;
    dev_pca9555_object_t *pca_obj = &pca_data;
    int ret;

    /* Assign mandatory or optional since its i2c address is different */
    plug_gpio_exp_set_device(which_dev);

    plug_gpio_exp_dev_create(pca_obj);

    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret = pca_obj->callin_fvt->config_port((dev_object_t *)pca_obj, which_port,
                                            which_pin, direction);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);

}


/*******************************************************************************
 *
 * Function    : plug_gpio_exp_drive_port
 * Description : Function to drive port to High/Low
 * Inputs      : which_dev - Mandatory or Optional
 *               which_port - Port 0 or Port 1
 *               which_pin - 0 ~ 15
 *               value - 0 for Low, 1 for High
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_drive_port (int which_dev, int which_port, 
                              int which_pin, int value) 
{
    dev_pca9555_object_t pca_data;
    dev_pca9555_object_t *pca_obj = &pca_data;
    int ret;

    /* Assign mandatory or optional since its i2c address is different */
    plug_gpio_exp_set_device(which_dev);

    plug_gpio_exp_dev_create(pca_obj);

    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret = pca_obj->callin_fvt->drive_port((dev_object_t *)pca_obj, which_port,
                                           which_pin, value);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);

}


/*******************************************************************************
 *
 * Function    : plug_gpio_exp_read_port
 * Description : Function to read the current value of the port 
 * Inputs      : which_dev - Mandatory or Optional
 *               which_port - Port 0 or Port 1
 *               which_pin - 0 ~ 15
 *               *value - 0 for Low, 1 for High
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_read_port (int which_dev, int which_port, 
                             int which_pin, int *value) 
{
    dev_pca9555_object_t pca_data;
    dev_pca9555_object_t *pca_obj = &pca_data;
    int ret;

    /* Assign mandatory or optional since its i2c address is different */
    plug_gpio_exp_set_device(which_dev);

    plug_gpio_exp_dev_create(pca_obj);

    if (pca_obj == NULL) {
        cterr('f', 0, "%s: Null Object\n", __func__);
        return (FAILED);
    }

    ret = pca_obj->callin_fvt->read_port((dev_object_t *)pca_obj, which_port,
                                          which_pin, value);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);
}


/*******************************************************************************
 *
 * Function    : plug_gpio_i2c_rd
 * Description : Function implementation of PCA9555 I2C Read
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t plug_gpio_i2c_rd (uint32 offset, ushort *data) 
{
    if (plug_gpio_exp_dev == MANDATORY) {
        return (plug_common_host_i2c_rd_2bytes(plug_curr_i2c_ctrl, 
                                               PLUG_MAN_I2C_ADDR_GPIO_EXP, 
                                               offset, data));
    } else {
        return (plug_common_host_i2c_rd_2bytes(plug_curr_i2c_ctrl,
                                               PLUG_OPT_I2C_ADDR_GPIO_EXP, 
                                               offset, data));
    }
}


/*******************************************************************************
 *
 * Function    : plug_gpio_i2c_wr
 * Description : Function implementation of PCA9555 I2C Write
 * Inputs      : offset - Offset
 *               *data - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
static uint32_t plug_gpio_i2c_wr (uint32 offset, ushort *data)
{
    if (plug_gpio_exp_dev == MANDATORY) {
        return (plug_common_host_i2c_wr_2bytes(plug_curr_i2c_ctrl, 
                                               PLUG_MAN_I2C_ADDR_GPIO_EXP, 
                                               offset, *data));
    } else {
        return (plug_common_host_i2c_wr_2bytes(plug_curr_i2c_ctrl, 
                                               PLUG_OPT_I2C_ADDR_GPIO_EXP, 
                                               offset, *data));
    }
}

/*-------------------------------------------------
$Log: plug_gpio_exp_lib.c,v $
Revision 1.3  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.1  2018/10/15 06:50:18  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:40:39  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.2  2017/07/20 17:23:10  tirawan
Add Pluggable host implementation codes

Revision 1.1.2.1  2017/07/13 06:32:18  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.6  2017/06/25 06:40:47  tirawan
Initialize mux number in i2c read/write function

Revision 1.1.2.5  2017/06/22 19:27:10  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

