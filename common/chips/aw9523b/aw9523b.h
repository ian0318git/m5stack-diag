/*
 * aw9523b.h — AW9523B I2C GPIO Expander Driver
 *
 * Common chip driver.  AW9523B provides 16 GPIO pins (P0_0–P0_7, P1_0–P1_7)
 * with individual direction and output control, plus LED PWM mode.
 *
 * CoreS3 connections:
 *   P0_0 = FT6336U TOUCH_RST      P1_0 = GC0308 CAM_RST
 *   P0_2 = AW88298 AW_RST         P1_1 = ILI9342C LCD_RST
 *                                 P1_2 = FT6336U TOUCH_INT
 *                                 P1_3 = AW88298 AW_INT
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

#define AW9523B_REG_INPUT0      0x00   /* Port 0 input value               */
#define AW9523B_REG_INPUT1      0x01   /* Port 1 input value               */
#define AW9523B_REG_OUTPUT0     0x02   /* Port 0 output value              */
#define AW9523B_REG_OUTPUT1     0x03   /* Port 1 output value              */
#define AW9523B_REG_CONF0       0x04   /* Port 0 I/O dir (0=out, 1=in)    */
#define AW9523B_REG_CONF1       0x05   /* Port 1 I/O dir (0=out, 1=in)    */
#define AW9523B_REG_CHIP_ID     0x10   /* Chip ID register                 */
#define AW9523B_REG_SOFTRST     0x7F   /* Software reset                   */

/* Expected chip ID */
#define AW9523B_CHIP_ID_VAL     0x23

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

typedef uint16_t aw9523b_pin_mask_t;

/* Pin mapping helper: P0_N = (1 << N), P1_N = (1 << (N+8)) */
#define AW9523B_PIN(p1, n)  ((aw9523b_pin_mask_t)(1 << ((p1) ? ((n) + 8) : (n))))
#define AW9523B_P0(n)       AW9523B_PIN(0, n)
#define AW9523B_P1(n)       AW9523B_PIN(1, n)

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int  aw9523b_init(i2c_master_dev_handle_t dev);
void aw9523b_deinit(void);

/*===========================================================================*/
/* GPIO Control                                                              */
/*===========================================================================*/

int  aw9523b_pin_set_direction(uint8_t pin, int output);
int  aw9523b_pin_write(uint8_t pin, int level);
int  aw9523b_pin_read(uint8_t pin, int *level);
int  aw9523b_port_write(int port, uint8_t value);
int  aw9523b_port_read(int port, uint8_t *value);

/*===========================================================================*/
/* Chip Info                                                                 */
/*===========================================================================*/

uint8_t aw9523b_chip_id(void);

#ifdef __cplusplus
}
#endif
