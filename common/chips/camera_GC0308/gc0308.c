/*
 * gc0308.c — GC0308 0.3 MP Camera Sensor Driver (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "gc0308.h"
#include <string.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;
static uint8_t           s_chip_id = 0;

/*===========================================================================*/
/* I2C helpers                                                               */
/*===========================================================================*/

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, GC0308_ADDR, &reg, 1, val, 1);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int gc0308_probe(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t id = 0;
    if (read_reg(GC0308_REG_CHIP_ID, &id) != 0) {
        return -1;
    }

    s_chip_id = id;
    return (id == GC0308_CHIP_ID_VAL) ? 0 : -1;
}

uint8_t gc0308_chip_id(void)
{
    return s_chip_id;
}
