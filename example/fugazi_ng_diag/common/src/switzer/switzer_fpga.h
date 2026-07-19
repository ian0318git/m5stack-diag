/* $Id: switzer_fpga.h,v 1.3 2020/05/22 02:28:47 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_fpga.h,v $
 *------------------------------------------------------------------
 *
 * switzer_fpga.h - Switzer FPGA driver interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_FPGA_H__
#define __SWITZER_FPGA_H__

#include <unistd.h>
#include <stdint.h>

/* ****************************** MDIO ****************************** */

struct switzer_mdio_settings {
    void *base;                 /* base address */
};
struct switzer_mdio;

struct switzer_mdio *switzer_mdio_probe(struct switzer_mdio_settings *settings);
void switzer_mdio_remove(struct switzer_mdio *mdio);
int switzer_mdio_read(struct switzer_mdio *mdio,
                      uint8_t prtad, uint8_t devad,
                      uint16_t addr, uint16_t *data);
int switzer_mdio_write(struct switzer_mdio *mdio,
                       uint8_t prtad, uint8_t devad,
                       uint16_t addr, uint16_t data);

/* ****************************** I2C ****************************** */

#define SWITZER_I2C_READ  0
#define SWITZER_I2C_WRITE 1

typedef enum {
    SWITZER_I2C_XFER_UNKNOWN,
    /* standard xfer, per smbus20, section 5.5 Bus Protocols */
    /*
     * 5.5.1. quick command
     * 5.5.2. send byte
     * 5.5.3. receive byte
     */
    SWITZER_I2C_XFER_QUICK_BYTES,
    /*
     * 5.5.4. write byte/word
     * 5.5.5. read byte/word
     */
    SWITZER_I2C_XFER_BYTES,
    /* 5.5.7. block read/write */
    SWITZER_I2C_XFER_BLOCK_BYTES,

    /* extended command xfer, per pmbus spec 1.2, part 1 */
    SWITZER_I2C_XFER_EXT_BYTES,

    MAX_NR_SWITZER_I2C_XFER
} switzer_i2c_xfer_t;

struct switzer_i2c_master_settings {
    void *base;                 /* base address */
};
struct switzer_i2c_master;

struct switzer_i2c_slave_settings {
    struct switzer_i2c_master *master;
    uint8_t addr;               /* slave address */
    uint8_t port;               /* port select */
    uint16_t freq;              /* KHz */
    switzer_i2c_xfer_t xfer;    /* transfer mode */
};
struct switzer_i2c_slave;

struct switzer_i2c_master *
switzer_i2c_master_probe(struct switzer_i2c_master_settings *settings);
void switzer_i2c_master_remove(struct switzer_i2c_master *master);

struct switzer_i2c_slave *
switzer_i2c_slave_probe(struct switzer_i2c_slave_settings *settings);
void switzer_i2c_slave_remove(struct switzer_i2c_slave *slave);
ssize_t __switzer_i2c_slave_read(struct switzer_i2c_slave *slave,
                                 switzer_i2c_xfer_t xfer, uint8_t addr,
                                 uint8_t cmd, void *buf, size_t count);
ssize_t __switzer_i2c_slave_write(struct switzer_i2c_slave *slave,
                                  switzer_i2c_xfer_t xfer, uint8_t addr,
                                  uint8_t cmd, const void *buf, size_t count);
ssize_t switzer_i2c_slave_read(struct switzer_i2c_slave *slave,
                               uint8_t cmd, void *buf, size_t count);
ssize_t switzer_i2c_slave_write(struct switzer_i2c_slave *slave,
                                uint8_t cmd, const void *buf, size_t count);
uint8_t switzer_i2c_slave_addr(struct switzer_i2c_slave *slave);

/* ****************************** DASH I2C ****************************** */

#define SWITZER_DASH_I2C_100K 0
#define SWITZER_DASH_I2C_400K 1

#define SWITZER_DASH_I2C_NOMAL_ADDR     0
#define SWITZER_DASH_I2C_EXTENDED_ADDR  1

struct switzer_dash_i2c_master_settings {
    void *base;                 /* base address */
};
struct switzer_dash_i2c_master;

struct switzer_dash_i2c_slave_settings {
    struct switzer_dash_i2c_master *master;
    uint32_t      addr;
    uint32_t      sub_addr_len;   /* I2C Sub address lenth (0-3) */
    uint32_t      sub_addr;       /* I2C Sub address */
    uint8_t       mux;            /* Mux number that I2C device connected to */
    uint8_t       i2c_speed;
    uint8_t       addr_extended;  /* Addr extended mode 1 - 10 bits addr, 0 - 7 bits addr */
};
struct switzer_dash_i2c_slave;

struct switzer_dash_i2c_master *
switzer_dash_i2c_master_probe(struct switzer_dash_i2c_master_settings *settings);
void switzer_dash_i2c_master_remove(struct switzer_dash_i2c_master *master);

struct switzer_dash_i2c_slave *
switzer_dash_i2c_slave_probe(struct switzer_dash_i2c_slave_settings *settings);
void switzer_dash_i2c_slave_remove(struct switzer_dash_i2c_slave *slave);

ssize_t __switzer_dash_i2c_slave_read(struct switzer_dash_i2c_slave *slave,
                                      uint8_t mux, uint32_t addr, uint8_t addr_extended,
                                      uint32_t sub_addr_sz, uint32_t sub_addr,
                                      uint8_t i2c_speed, void *cmd, uint8_t cmd_len,
                                      void *buf, size_t count);
ssize_t __switzer_dash_i2c_slave_write(struct switzer_dash_i2c_slave *slave,
                                       uint8_t mux, uint32_t addr, uint8_t addr_extended,
                                       uint32_t sub_addr_sz, uint32_t sub_addr,
                                       uint8_t i2c_speed, void *cmd, uint8_t cmd_len,
                                       const void *buf, size_t count);

ssize_t switzer_dash_i2c_slave_read(struct switzer_dash_i2c_slave *slave,
                                    uint8_t cmd, void *buf, size_t count);
ssize_t switzer_dash_i2c_slave_write(struct switzer_dash_i2c_slave *slave,
                                     uint8_t cmd, const void *buf, size_t count);

uint32_t switzer_dash_i2c_slave_addr(struct switzer_dash_i2c_slave *slave);
uint8_t switzer_dash_i2c_slave_mux(struct switzer_dash_i2c_slave *slave);
uint8_t switzer_dash_i2c_slave_sleep(struct switzer_dash_i2c_slave *slave);
uint32_t switzer_dash_i2c_slave_sub_addr(struct switzer_dash_i2c_slave *slave);
uint32_t switzer_dash_i2c_slave_sub_addr_len(struct switzer_dash_i2c_slave *slave);
uint8_t switzer_dash_i2c_slave_addr_extended(struct switzer_dash_i2c_slave *slave);

/* ****************************** SPI Prom ****************************** */

struct switzer_spi_prom_settings {
    void *base;                 /* base address */
};
struct switzer_spi_prom;

struct switzer_spi_prom *
switzer_spi_prom_probe(struct switzer_spi_prom_settings *settings);
void switzer_spi_prom_remove(struct switzer_spi_prom *prom);
int switzer_spi_prom_read_status(struct switzer_spi_prom *prom, uint8_t *status);
int switzer_spi_prom_write_status(struct switzer_spi_prom *prom, uint8_t status);
ssize_t switzer_spi_prom_read(struct switzer_spi_prom *prom,
                              uint32_t addr, void *buf, size_t count);
ssize_t switzer_spi_prom_write(struct switzer_spi_prom *prom,
                               uint32_t addr, const void *buf, size_t count);
ssize_t switzer_spi_prom_erase(struct switzer_spi_prom *prom,
                               uint32_t addr, size_t count);

#endif /* __SWITZER_FPGA_H__ */
