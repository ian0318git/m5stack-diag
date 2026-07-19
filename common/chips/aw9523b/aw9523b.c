/*
 * aw9523b.c — AW9523B I2C GPIO Expander Driver
 *
 * Common chip driver.
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#include "aw9523b.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "AW9523B";
static i2c_master_dev_handle_t s_dev = NULL;
static bool s_init = false;

/*===========================================================================*/
/* I2C helpers                                                               */
/*===========================================================================*/

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return (i2c_master_transmit(s_dev, buf, 2, -1) == ESP_OK) ? 0 : -1;
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return (i2c_master_transmit_receive(s_dev, &reg, 1, val, 1, -1)
            == ESP_OK) ? 0 : -1;
}

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

int aw9523b_init(i2c_master_dev_handle_t dev)
{
    if (!dev) return -1;
    s_dev = dev;

    /* Verify chip ID */
    uint8_t id = 0;
    if (read_reg(AW9523B_REG_CHIP_ID, &id) != 0) {
        ESP_LOGE(TAG, "AW9523B not responding");
        return -1;
    }
    if (id != AW9523B_CHIP_ID_VAL) {
        ESP_LOGW(TAG, "Unexpected chip ID: 0x%02X (expected 0x%02X)",
                 id, AW9523B_CHIP_ID_VAL);
    }

    /* Set all pins to GPIO mode (not LED PWM) by clearing reg 0x12/0x13 */
    /* AW9523B: reg 0x12 controls P0 LED mode, 0x13 controls P1 LED mode */
    write_reg(0x12, 0x00);
    write_reg(0x13, 0x00);

    s_init = true;
    ESP_LOGI(TAG, "AW9523B initialised (chip_id=0x%02X)", id);
    return 0;
}

void aw9523b_deinit(void)
{
    s_init = false;
    s_dev = NULL;
}

int aw9523b_pin_set_direction(uint8_t pin, int output)
{
    if (!s_dev || pin > 15) return -1;

    int port = (pin < 8) ? 0 : 1;
    uint8_t reg = (port == 0) ? AW9523B_REG_CONF0 : AW9523B_REG_CONF1;
    uint8_t bit = (uint8_t)(1 << (pin & 7));
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;

    if (output) {
        val &= ~bit;   /* 0 = output */
    } else {
        val |= bit;    /* 1 = input */
    }

    return write_reg(reg, val);
}

int aw9523b_pin_write(uint8_t pin, int level)
{
    if (!s_dev || pin > 15) return -1;

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
    if (!s_dev || pin > 15 || !level) return -1;

    int port = (pin < 8) ? 0 : 1;
    uint8_t reg = (port == 0) ? AW9523B_REG_INPUT0 : AW9523B_REG_INPUT1;
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;

    *level = (val & (1 << (pin & 7))) ? 1 : 0;
    return 0;
}

int aw9523b_port_write(int port, uint8_t value)
{
    if (!s_dev || port < 0 || port > 1) return -1;

    uint8_t reg = (port == 0) ? AW9523B_REG_OUTPUT0 : AW9523B_REG_OUTPUT1;
    return write_reg(reg, value);
}

int aw9523b_port_read(int port, uint8_t *value)
{
    if (!s_dev || port < 0 || port > 1 || !value) return -1;

    uint8_t reg = (port == 0) ? AW9523B_REG_INPUT0 : AW9523B_REG_INPUT1;
    return read_reg(reg, value);
}

uint8_t aw9523b_chip_id(void)
{
    if (!s_dev) return 0;
    uint8_t id = 0;
    read_reg(AW9523B_REG_CHIP_ID, &id);
    return id;
}
