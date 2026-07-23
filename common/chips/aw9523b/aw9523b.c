/*
 * aw9523b.c — AW9523B I2C GPIO Expander Driver
 *
 * Common chip driver — platform-agnostic.
 *
 * NOTE: Avoid full-register writes (0x12/0x13 = 0x00) that affect ALL
 * pins at once. On the CoreS3, undocumented AW9523B pins may control
 * USB_OTG_EN or other power-direction signals.  Always use per-pin
 * read-modify-write.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "aw9523b.h"
#include <string.h>

/*===========================================================================*/
/* Module state — refcounted so multiple callers can safely share the chip   */
/*===========================================================================*/

static const diag_i2c_t *s_i2c = NULL;
static void             *s_bus = NULL;
static int               s_refcount = 0;

/*===========================================================================*/
/* I2C helpers (use abstract transport)                                      */
/*===========================================================================*/

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return s_i2c->write(s_bus, AW9523B_ADDR, buf, 2);
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return s_i2c->write_then_read(s_bus, AW9523B_ADDR, &reg, 1, val, 1);
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int aw9523b_init(const diag_i2c_t *i2c, void *bus)
{
    if (s_refcount > 0) {
        s_refcount++;
        return 0;
    }

    if (!i2c || !bus) return -1;
    s_i2c = i2c;
    s_bus = bus;

    /* Only verify chip ID — no register writes that could affect unknown pins */
    uint8_t id = 0;
    if (read_reg(AW9523B_REG_CHIP_ID, &id) != 0) {
        return -1;
    }

    s_refcount = 1;
    return 0;
}

void aw9523b_deinit(void)
{
    if (s_refcount <= 0) return;
    s_refcount--;
    if (s_refcount > 0) return;

    s_i2c = NULL;
    s_bus = NULL;
}

int aw9523b_pin_set_direction(uint8_t pin, int output)
{
    if (!s_i2c || !s_bus || pin > 15) return -1;

    int port = (pin < 8) ? 0 : 1;
    uint8_t reg = (port == 0) ? AW9523B_REG_CONF0 : AW9523B_REG_CONF1;
    uint8_t bit = (uint8_t)(1 << (pin & 7));
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;

    if (output) {
        val &= ~bit;
    } else {
        val |= bit;
    }

    return write_reg(reg, val);
}

int aw9523b_pin_write(uint8_t pin, int level)
{
    if (!s_i2c || !s_bus || pin > 15) return -1;

    int port = (pin < 8) ? 0 : 1;
    uint8_t reg = (port == 0) ? AW9523B_REG_OUTPUT0 : AW9523B_REG_OUTPUT1;
    uint8_t bit = (uint8_t)(1 << (pin & 7));
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;

    if (level) {
        val |= bit;
    } else {
        val &= ~bit;
    }

    return write_reg(reg, val);
}

int aw9523b_pin_read(uint8_t pin, int *level)
{
    if (!s_i2c || !s_bus || pin > 15 || !level) return -1;

    int port = (pin < 8) ? 0 : 1;
    uint8_t reg = (port == 0) ? AW9523B_REG_INPUT0 : AW9523B_REG_INPUT1;
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;

    *level = (val & (1 << (pin & 7))) ? 1 : 0;
    return 0;
}

int aw9523b_port_write(int port, uint8_t value)
{
    if (!s_i2c || !s_bus || port < 0 || port > 1) return -1;

    uint8_t reg = (port == 0) ? AW9523B_REG_OUTPUT0 : AW9523B_REG_OUTPUT1;
    return write_reg(reg, value);
}

int aw9523b_port_read(int port, uint8_t *value)
{
    if (!s_i2c || !s_bus || port < 0 || port > 1 || !value) return -1;

    uint8_t reg = (port == 0) ? AW9523B_REG_INPUT0 : AW9523B_REG_INPUT1;
    return read_reg(reg, value);
}

int aw9523b_pin_set_gpio_mode(uint8_t pin)
{
    if (!s_i2c || !s_bus || pin > 15) return -1;

    int port = (pin < 8) ? 0 : 1;
    /* AW9523B regs: 0x12 = Port 0 LED mode, 0x13 = Port 1 LED mode.
     * Bit = 1 means LED/PWM mode, 0 means GPIO mode. */
    uint8_t reg = (port == 0) ? 0x12 : 0x13;
    uint8_t bit = (uint8_t)(1 << (pin & 7));
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;
    val &= ~bit;
    return write_reg(reg, val);
}

uint8_t aw9523b_chip_id(void)
{
    if (!s_i2c || !s_bus) return 0;
    uint8_t id = 0;
    read_reg(AW9523B_REG_CHIP_ID, &id);
    return id;
}
