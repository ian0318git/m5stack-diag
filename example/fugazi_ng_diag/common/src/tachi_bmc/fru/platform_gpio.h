/* $Id: platform_gpio.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/platform_gpio.h,v $
 *
 *      File:   platform_gpio.h
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#ifndef _PLATFORM_GPIO_H_
#define _PLATFORM_GPIO_H_

extern int platform_gpio_data_get(char *strname, uint8_t *data);
extern int platform_gpio_data_set(char *strname, uint8_t data);
extern int platform_gpio_data_dump(char **gpio_name, int num_of_gpios);
extern int platform_gpio_mezz_present_dump();
extern int platform_gpio_mezz_present(int slot);
#endif

