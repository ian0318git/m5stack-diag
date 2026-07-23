/*
 * es7210.c — ES7210 Audio ADC (Dual Microphone) Driver (I2C)
 *
 * Common chip driver — uses abstract diag_i2c_t transport.
 * I2S audio data path is handled by the platform (HAL/test file).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "es7210.h"
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
    return s_i2c->write(s_bus, ES7210_ADDR, buf, 2);
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, ES7210_ADDR, &reg, 1, val, 1);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int es7210_init(const diag_i2c_t *i2c, void *bus)
{
    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    uint8_t id = 0;
    if (read_reg(ES7210_REG_CHIP_ID, &id) != 0) {
        return -1;
    }

    /* Reset ADC */
    write_reg(ES7210_REG_RESET, ES7210_MAIN_RESET);
    write_reg(ES7210_REG_RESET, 0x00);

    /* Enable ADC, set 48 kHz */
    write_reg(ES7210_REG_MAIN_CTRL, ES7210_MAIN_ENABLE);
    write_reg(ES7210_REG_ADC_CTRL, ES7210_ADC_48KHZ);

    /* Set mic gain (default: 0 dB for both channels) */
    write_reg(ES7210_REG_MIC1_L, 0x00);
    write_reg(ES7210_REG_MIC1_R, 0x00);
    write_reg(ES7210_REG_MIC2_L, 0x00);
    write_reg(ES7210_REG_MIC2_R, 0x00);

    return 0;
}

void es7210_deinit(void)
{
    if (s_i2c && s_bus) {
        write_reg(ES7210_REG_MAIN_CTRL, 0x00);  /* disable */
    }
    s_i2c = NULL;
    s_bus = NULL;
}

uint8_t es7210_chip_id(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t id = 0;
    read_reg(ES7210_REG_CHIP_ID, &id);
    return id;
}

int es7210_info(es7210_info_t *info)
{
    if (!info || !s_i2c || !s_bus) return -1;
    memset(info, 0, sizeof(*info));
    read_reg(ES7210_REG_CHIP_ID, &info->chip_id);
    return 0;
}
