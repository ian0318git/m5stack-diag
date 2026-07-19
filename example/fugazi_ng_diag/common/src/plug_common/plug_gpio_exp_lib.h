/* $Id: plug_gpio_exp_lib.h,v 1.2 2018/01/20 04:53:29 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_common/plug_gpio_exp_lib.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : plug_gpio_exp_lib.h
 * Description: Header file of Pluggable GPIO Expander Library
 * 
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLUG_GPIO_EXP_LIB_H__
#define __PLUG_GPIO_EXP_LIB_H__

#include "dev_pca9555.h"

#define PLUG_MAN_I2C_ADDR_GPIO_EXP     (0x4E >> 1)   /* Pluggable Mandatory GPIO Expander */
#define PLUG_OPT_I2C_ADDR_GPIO_EXP     (0x4C >> 1)   /* Pluggable Optional GPIO Expander */

extern int plug_gpio_exp_dev_create(dev_pca9555_object_t *);
extern void plug_gpio_exp_set_device(int);
extern int plug_gpio_exp_drive_port(int, int, int, int); 
extern int plug_gpio_exp_read_port(int, int, int, int *);
extern int plug_gpio_exp_config_port(int, int, int, int);
extern int get_gpio_exp_type(void);

#endif

