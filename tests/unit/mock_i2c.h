/*
 * mock_i2c.h — Mock diag_i2c_t transport for host-side unit testing
 *
 * Stores a register file (256 bytes) that the mock reads/writes.
 * Use mock_i2c_setup() before each test, then call chip driver
 * functions as normal.  After the test, use mock_i2c_reg() to
 * assert register values.
 *
 * Usage:
 *   uint8_t regs[256];
 *   void *bus = mock_i2c_setup(regs);
 *   power_AXP2101_init(&mock_transport, bus);
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "diag_transport.h"
#include <string.h>

/* Register file — the mock reads and writes here (set by mock_i2c_setup) */
static uint8_t *mock_regs = NULL;

/*===========================================================================*/
/* Mock callbacks                                                            */
/*===========================================================================*/

static int mock_write(void *bus, uint16_t addr, const void *data, size_t len)
{
    (void)bus;
    (void)addr;
    const uint8_t *buf = (const uint8_t *)data;
    if (!mock_regs || len == 0) return 0;

    /* I2C register write: data[0] = starting register, data[1..] = values */
    if (len >= 2) {
        uint8_t reg = buf[0];
        for (size_t i = 1; i < len; i++) {
            mock_regs[reg++] = buf[i];
        }
    }
    return 0;
}

static int mock_read(void *bus, uint16_t addr, void *data, size_t len)
{
    (void)bus;
    (void)addr;
    return 0;  /* not used by any chip driver */
}

static int mock_write_then_read(void *bus, uint16_t addr,
                                 const void *wdata, size_t wlen,
                                 void *rdata, size_t rlen)
{
    (void)bus;
    (void)addr;
    if (!mock_regs) return -1;

    if (wlen != 1) return -1;
    uint8_t reg = *(const uint8_t *)wdata;
    uint8_t *out = (uint8_t *)rdata;

    for (size_t i = 0; i < rlen; i++) {
        out[i] = mock_regs[reg + i];
    }
    return 0;
}

static int mock_probe(void *bus, uint16_t addr)
{
    (void)bus;
    (void)addr;
    return 0;  /* always ACK */
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

/** Singleton mock transport — use in test functions */
static const diag_i2c_t mock_transport = {
    .write           = mock_write,
    .read            = mock_read,
    .write_then_read = mock_write_then_read,
    .probe           = mock_probe,
};

/**
 * @brief Initialise the mock I2C with a register file.
 *
 * Zeroes all registers, sets up the mock, and returns the
 * opaque bus handle to pass to chip driver init().
 *
 * @param regs  256-byte array for the register file.
 * @return      Opaque bus pointer (pass to chip driver as void *bus).
 */
static inline void *mock_i2c_setup(uint8_t *regs)
{
    mock_regs = regs;
    memset(mock_regs, 0, 256);
    return (void *)mock_regs;  /* any non-NULL pointer works */
}

/** Convenience: set a register value in the mock register file. */
static inline void mock_i2c_set_reg(uint8_t reg, uint8_t val)
{
    if (mock_regs) mock_regs[reg] = val;
}

/** Convenience: read back a register value from the mock. */
static inline uint8_t mock_i2c_reg(uint8_t reg)
{
    return mock_regs ? mock_regs[reg] : 0;
}
