/*
 * aw9523b.c — AW9523B I2C GPIO Expander Driver
 *
 * Common chip driver.
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

    /* Only verify chip ID — no register writes that could affect unknown pins */
    uint8_t id = 0;
    if (read_reg(AW9523B_REG_CHIP_ID, &id) != 0) {
        ESP_LOGE(TAG, "AW9523B not responding");
        return -1;
    }
    if (id != AW9523B_CHIP_ID_VAL) {
        ESP_LOGW(TAG, "Unexpected chip ID: 0x%02X (expected 0x%02X)",
                 id, AW9523B_CHIP_ID_VAL);
    }

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

int aw9523b_pin_set_gpio_mode(uint8_t pin)
{
    if (!s_dev || pin > 15) return -1;

    int port = (pin < 8) ? 0 : 1;
    /* AW9523B regs: 0x12 = Port 0 LED mode, 0x13 = Port 1 LED mode.
     * Bit = 1 means LED/PWM mode, 0 means GPIO mode. */
    uint8_t reg = (port == 0) ? 0x12 : 0x13;
    uint8_t bit = (uint8_t)(1 << (pin & 7));
    uint8_t val = 0;

    if (read_reg(reg, &val) != 0) return -1;

    val &= ~bit;   /* Clear the LED mode bit → GPIO mode */

    return write_reg(reg, val);
}

uint8_t aw9523b_chip_id(void)
{
    if (!s_dev) return 0;
    uint8_t id = 0;
    read_reg(AW9523B_REG_CHIP_ID, &id);
    return id;
}
