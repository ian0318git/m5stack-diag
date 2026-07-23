/*
 * touch_FT6336.c — FT6336 Capacitive Touch Controller (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "touch_FT6336.h"
#include <string.h>

/*===========================================================================*/
/* Module state                                                              */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;
static uint16_t          s_addr = FT6336_ADDR;

/*===========================================================================*/
/* I2C helpers                                                               */
/*===========================================================================*/

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, s_addr, &reg, 1, val, 1);
}

static int read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    return s_i2c->write_then_read(s_bus, s_addr, &reg, 1, buf, len);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int touch_FT6336_init(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;
    s_addr = FT6336_ADDR;

    /* Verify presence */
    uint8_t mode = 0;
    if (read_reg(FT6336_REG_DEVICE_MODE, &mode) != 0) {
        return -1;
    }

    return 0;
}

void touch_FT6336_deinit(void)
{
    s_i2c = NULL;
    s_bus = NULL;
}

int touch_FT6336_read(touch_FT6336_data_t *data)
{
    if (!data || !s_i2c || !s_bus) return -1;
    memset(data, 0, sizeof(*data));

    uint8_t tds = 0;
    if (read_reg(FT6336_REG_TD_STATUS, &tds) != 0) return -1;

    tds &= 0x0F;
    if (tds > 2) tds = 2;
    data->point_count = tds;

    for (uint8_t i = 0; i < tds; i++) {
        uint8_t base = (i == 0) ? FT6336_REG_TOUCH1_X : FT6336_REG_TOUCH2_X;
        uint8_t buf[6];
        if (read_regs(base, buf, 6) != 0) return -1;

        data->points[i].x     = ((uint16_t)(buf[0] & 0x0F) << 8) | buf[1];
        data->points[i].y     = ((uint16_t)(buf[2] & 0x0F) << 8) | buf[3];
        data->points[i].event = (buf[4] >> 6) & 0x03;
        data->points[i].id    = buf[4] & 0x0F;
    }
    return 0;
}

uint8_t touch_FT6336_firmware_version(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t ver = 0;
    read_reg(FT6336_REG_FW_VERSION, &ver);
    return ver;
}
