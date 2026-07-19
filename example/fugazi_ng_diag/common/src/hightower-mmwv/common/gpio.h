/*------------------------------------------------------------------
 *
 * gpio.h - gpio control APIs header base on GPIO Sysfs interface
 *
 * May 2019, markzha
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30


int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin, int dir);
int gpio_read(int pin, int *vaule);
int gpio_write(int pin, int value);

#endif
