/*
 * aw88298.c — AW88298 I2S Speaker Amplifier Driver (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 * I2S audio data path is handled by the platform (HAL/test file).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "aw88298.h"
#include <string.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;

/*===========================================================================*/
/* I2C helpers                                                               */
/*===========================================================================*/

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return s_i2c->write(s_bus, AW88298_ADDR, buf, 2);
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, AW88298_ADDR, &reg, 1, val, 1);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int aw88298_init(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t id = 0;
    if (read_reg(AW88298_REG_CHIP_ID, &id) != 0) {
        return -1;
    }

    /* Soft reset */
    write_reg(AW88298_REG_SYS_CTRL, AW88298_SYS_RST);

    return 0;
}

void aw88298_deinit(void)
{
    aw88298_disable();
    s_i2c = NULL;
    s_bus = NULL;
}

int aw88298_enable(void)
{
    if (!s_i2c || !s_bus) return -1;
    return write_reg(AW88298_REG_SYS_CTRL, AW88298_SYS_ENABLE);
}

int aw88298_disable(void)
{
    if (!s_i2c || !s_bus) return -1;
    return write_reg(AW88298_REG_SYS_CTRL, AW88298_SYS_PDOWN);
}

int aw88298_set_gain(uint8_t gain)
{
    if (!s_i2c || !s_bus || gain > 15) return -1;
    return write_reg(AW88298_REG_GAIN_CTRL, gain & 0x0F);
}

uint8_t aw88298_chip_id(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t id = 0;
    read_reg(AW88298_REG_CHIP_ID, &id);
    return id;
}

int aw88298_read_fault(uint8_t *fault)
{
    if (!s_i2c || !s_bus || !fault) return -1;
    return read_reg(AW88298_REG_FAULT, fault);
}
