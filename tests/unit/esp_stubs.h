/*
 * esp_stubs.h — Minimal ESP-IDF stubs for host-side unit testing
 *
 * Provides just enough of the ESP-IDF/FreeRTOS API so chip drivers
 * can be compiled and tested on the host (Linux/macOS).
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <unistd.h>

/* FreeRTOS stubs */
#define pdMS_TO_TICKS(ms)  (ms)

static inline void vTaskDelay(uint32_t ticks)
{
    /* Approximate: 1 tick = 1 ms in our stub */
    usleep((useconds_t)ticks * 1000);
}

/* ESP-ROM stubs */
static inline void esp_rom_delay_us(uint32_t us)
{
    usleep((useconds_t)us);
}

/* ESP error type */
typedef int esp_err_t;
#define ESP_OK    0
#define ESP_FAIL  -1
#define ESP_ERR_INVALID_STATE  0x103
#define ESP_ERR_NOT_FOUND      0x105
#define ESP_ERR_TIMEOUT        0x107
#define ESP_ERR_INVALID_ARG    0x102
#define ESP_ERR_NO_MEM         0x101

/* ESP log stub */
#define ESP_LOGE(tag, fmt, ...)  ((void)0)
#define ESP_LOGW(tag, fmt, ...)  ((void)0)
#define ESP_LOGI(tag, fmt, ...)  ((void)0)
#define ESP_LOGD(tag, fmt, ...)  ((void)0)
