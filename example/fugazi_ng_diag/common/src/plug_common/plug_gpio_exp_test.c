/* $Id: plug_gpio_exp_test.c,v 1.3 2018/11/23 09:02:32 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_gpio_exp_test.c,v $
 *-----------------------------------------------------------------------------
 * 
 * Filename   : plug_gpio_exp_test.c
 * Description: Pluggable GPIO Expander Test Functions
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "byteswap.h"
#include "common_utils.h"
#include "nvmonvars.h"
#include "plug_slot.h"
#include "dev_pca9555.h"
#include "plug_gpio_exp_test.h"
#include "plug_gpio_exp_lib.h"

int plug_gpio_exp_reg_test(int);
int plug_gpio_exp_show_reg(int);
int plug_gpio_exp_alter_reg(int);

/*******************************************************************************
 *
 * Function    : plug_gpio_exp_reg_test
 * Description : Function to execute GPIO Expander Register Test
 * Inputs      : which_dev - Mandatory (0x4E) or Optional (0x4C)
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_reg_test (int which_dev)
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

    ret = pca_obj->callin_fvt->register_test((dev_object_t *)pca_obj);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);
}


/*******************************************************************************
 *
 * Function    : plug_gpio_exp_show_reg
 * Description : Function to display GPIO Expander Register
 * Inputs      : which_dev - Mandatory (0x4E) or Optional (0x4C)
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_show_reg (int which_dev)
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

    ret = pca_obj->callin_fvt->dump_register((dev_object_t *)pca_obj);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);
}


/*******************************************************************************
 *
 * Function    : plug_gpio_exp_alter_reg
 * Description : Function to alter GPIO Expander Register
 * Inputs      : which_dev - Mandatory (0x4E) or Optional (0x4C)
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int plug_gpio_exp_alter_reg (int which_dev)
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

    ret = pca_obj->callin_fvt->alter_register((dev_object_t *)pca_obj);

    pca_obj->base.dev_object_fvt->dev_destroy((dev_object_t **)&pca_obj);

    return (ret);
}

/*-------------------------------------------------
$Log: plug_gpio_exp_test.c,v $
Revision 1.3  2018/11/23 09:02:32  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.1  2018/10/15 06:50:18  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 04:53:29  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.2  2017/08/08 07:40:40  hondwang
add pluggable for star-branch-c9xx

Revision 1.1.2.1  2017/07/13 06:32:18  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.4  2017/06/22 19:27:10  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

