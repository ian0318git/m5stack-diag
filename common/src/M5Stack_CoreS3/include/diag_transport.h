/*
 * diag_transport.h — Abstract I2C / SPI transport seam
 *
 * DOMAIN layer — pure C, zero hardware dependency.
 *
 * These types let chip drivers communicate over I2C and SPI without
 * coupling to ESP-IDF, Linux, or any other platform.  Each platform
 * provides concrete adapters that implement these interfaces;
 * chip drivers only see the abstract types.
 *
 * Usage:
 *   // Chip driver init accepts abstract handles:
 *   int my_chip_init(diag_i2c_t *i2c, void *i2c_bus);
 *
 *   // Platform provides the concrete adapter:
 *   static diag_i2c_t esp_i2c = {
 *       .read  = esp_i2c_read,
 *       .write = esp_i2c_write,
 *       .write_then_read = esp_i2c_wr_rd,
 *   };
 *   my_chip_init(&esp_i2c, bus_handle);
 *
 * Copyright (c) 2025 by M5Stack
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* I2C Transport                                                            */
/*===========================================================================*/

/**
 * @brief Abstract I2C bus operations.
 *
 * Every function returns 0 on success, -1 on error.
 * `write_then_read` may be NULL if the chip never needs combined
 * write+re-start+read transactions (common for register-based devices).
 */
typedef struct {
    /**
     * @brief Write data to an I2C slave.
     * @param bus      Opaque bus handle (platform-specific).
     * @param addr     7-bit I2C slave address.
     * @param data     Data to write.
     * @param len      Number of bytes to write.
     * @return 0 on success, -1 on error.
     */
    int (*write)(void *bus, uint16_t addr, const void *data, size_t len);

    /**
     * @brief Read data from an I2C slave.
     * @param bus      Opaque bus handle (platform-specific).
     * @param addr     7-bit I2C slave address.
     * @param data     Buffer to receive data.
     * @param len      Number of bytes to read.
     * @return 0 on success, -1 on error.
     */
    int (*read)(void *bus, uint16_t addr, void *data, size_t len);

    /**
     * @brief Write then restart-read (combined transaction).
     *
     * Typical for register-based chips:
     *   write reg_addr → restart → read register value
     *
     * @param bus      Opaque bus handle.
     * @param addr     7-bit slave address.
     * @param wdata    Data to write (e.g. register address).
     * @param wlen     Write length.
     * @param rdata    Buffer for read data.
     * @param rlen     Read length.
     * @return 0 on success, -1 on error.
     */
    int (*write_then_read)(void *bus, uint16_t addr,
                           const void *wdata, size_t wlen,
                           void *rdata, size_t rlen);

    /**
     * @brief Probe whether a slave ACKs its address.
     * @param bus      Opaque bus handle.
     * @param addr     7-bit address to probe.
     * @return 0 if device ACKs, -1 if NACK or error.
     */
    int (*probe)(void *bus, uint16_t addr);
} diag_i2c_t;

/*===========================================================================*/
/* SPI Transport                                                            */
/*===========================================================================*/

/**
 * @brief Abstract SPI bus operations.
 *
 * Every function returns 0 on success, -1 on error.
 */
typedef struct {
    /**
     * @brief Transmit data over SPI (MOSI only).
     * @param bus      Opaque bus handle (platform-specific).
     * @param data     Data to transmit.
     * @param len      Number of bytes.
     * @return 0 on success, -1 on error.
     */
    int (*transmit)(void *bus, const void *data, size_t len);

    /**
     * @brief Full-duplex transfer (MOSI + MISO simultaneously).
     * @param bus      Opaque bus handle.
     * @param tx_data  Data to transmit (may be NULL for read-only).
     * @param rx_data  Buffer for received data (may be NULL for write-only).
     * @param len      Number of bytes.
     * @return 0 on success, -1 on error.
     */
    int (*transfer)(void *bus, const void *tx_data, void *rx_data, size_t len);
} diag_spi_t;

/*===========================================================================*/
/* GPIO (simple pin control, so basic it doesn't need its own type)          */
/*===========================================================================*/

/**
 * @brief Abstract GPIO output controller.
 *
 * Used by chip drivers that need to control reset or enable lines
 * that may be on a GPIO expander rather than a direct MCU pin.
 */
typedef struct {
    void *context;   /**< Opaque handle passed to set(). */
    void (*set)(void *context, int level);  /**< 0 = low, !0 = high. */
} diag_gpio_t;

#ifdef __cplusplus
}
#endif
