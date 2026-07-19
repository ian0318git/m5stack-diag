/*
 * touch_FT6336.h — FT6336 Capacitive Touch Controller (I2C)
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Register Map                                                              */
/*===========================================================================*/

#define FT6336_REG_DEVICE_MODE   0x00
#define FT6336_REG_GESTURE_ID    0x01
#define FT6336_REG_TD_STATUS     0x02
#define FT6336_REG_TOUCH1_X      0x03
#define FT6336_REG_TOUCH1_Y      0x05
#define FT6336_REG_TOUCH1_EV     0x07
#define FT6336_REG_TOUCH2_X      0x09
#define FT6336_REG_TOUCH2_Y      0x0B
#define FT6336_REG_TOUCH2_EV     0x0D
#define FT6336_REG_FW_VERSION    0xA3

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  id;
    uint8_t  event;    /* 0=down, 1=up, 2=contact, 3=none */
} touch_FT6336_point_t;

typedef struct {
    uint8_t              point_count;
    touch_FT6336_point_t points[2];
} touch_FT6336_data_t;

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int touch_FT6336_init(i2c_master_dev_handle_t dev);
void touch_FT6336_deinit(void);

/*===========================================================================*/
/* Read                                                                      */
/*===========================================================================*/

int touch_FT6336_read(touch_FT6336_data_t *data);
uint8_t touch_FT6336_firmware_version(void);

#ifdef __cplusplus
}
#endif
