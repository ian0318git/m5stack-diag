/*
 * ltr553.c — LTR-553ALS-WA Proximity + Ambient Light Sensor (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "ltr553.h"
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
    return s_i2c->write(s_bus, LTR553_ADDR, buf, 2);
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, LTR553_ADDR, &reg, 1, val, 1);
}

static int read_reg16(uint8_t reg_low, uint16_t *val)
{
    uint8_t buf[2];
    if (s_i2c->write_then_read(s_bus, LTR553_ADDR, &reg_low, 1, buf, 2) != 0) {
        return -1;
    }
    *val = (uint16_t)(buf[0]) | ((uint16_t)(buf[1]) << 8);
    return 0;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int ltr553_probe(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t id = 0;
    if (read_reg(LTR553_REG_PART_ID, &id) != 0) {
        return -1;
    }

    /* Enable ALS and proximity sensors.
     * Device responded — probe succeeds regardless of part ID match.
     * Different sensor revisions may return different IDs. */
    write_reg(LTR553_REG_ALS_CONTR, LTR553_ALS_CONTR_ON);
    write_reg(LTR553_REG_PROX_CONTR, LTR553_PROX_CONTR_ON);

    return 0;
}

int ltr553_read_all(ltr553_data_t *data)
{
    if (!data || !s_i2c || !s_bus) return -1;
    memset(data, 0, sizeof(*data));

    read_reg(LTR553_REG_PART_ID, &data->part_id);

    /* ALS channel data (16-bit each) */
    read_reg16(LTR553_REG_ALS_CH1_0, &data->als_ch1);  /* visible + IR */
    read_reg16(LTR553_REG_ALS_CH0_0, &data->als_ch0);  /* visible only */

    /* Proximity (8-bit) */
    read_reg(LTR553_REG_PROX_DATA, &data->proximity);

    return 0;
}
