/*
 * aw88298.h — AW88298 I2S Speaker Amplifier Driver (I2C)
 *
 * Common chip driver — platform-agnostic.
 * Audio data is transferred via I2S (not covered by this driver).
 * Test tone generation is done in the test file.
 *
 * Reference: Awinic AW88298 datasheet
 * CoreS3: AW_RST via AW9523B P0_2, AW_INT via AW9523B P1_3
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "diag_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

#define AW88298_ADDR            0x36

#define AW88298_REG_CHIP_ID     0x00
#define AW88298_CHIP_ID_VAL     0x81   /* Expected chip ID */

#define AW88298_REG_SYS_CTRL    0x00
#define AW88298_REG_SYS_STS     0x01
#define AW88298_REG_GAIN_CTRL   0x02
#define AW88298_REG_I2S_CTRL    0x03
#define AW88298_REG_SYS_CTRL2   0x06
#define AW88298_REG_FAULT       0x07

/* SYS_CTRL bits */
#define AW88298_SYS_ENABLE      (1 << 0)
#define AW88298_SYS_PDOWN       (1 << 1)
#define AW88298_SYS_RST         (1 << 3)

/* FAULT register bits */
#define AW88298_FAULT_OVERTEMP  (1 << 0)
#define AW88298_FAULT_OVERCUR   (1 << 1)

/*===========================================================================*/
/* Lifecycle                                                                 */
/*===========================================================================*/

int  aw88298_init(const diag_i2c_t *i2c, void *bus);
void aw88298_deinit(void);

/*===========================================================================*/
/* Control                                                                   */
/*===========================================================================*/

int  aw88298_enable(void);
int  aw88298_disable(void);
int  aw88298_set_gain(uint8_t gain);  /* 0–15, amp gain in dB steps */

/*===========================================================================*/
/* Status                                                                    */
/*===========================================================================*/

uint8_t aw88298_chip_id(void);
int     aw88298_read_fault(uint8_t *fault);  /* bitmask of AW88298_FAULT_* */

#ifdef __cplusplus
}
#endif
