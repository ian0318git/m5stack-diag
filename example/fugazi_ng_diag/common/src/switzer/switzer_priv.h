/* $Id: switzer_priv.h,v 1.4 2021/04/12 13:37:35 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_priv.h,v $
 *------------------------------------------------------------------
 *
 * switzer_priv.h - Switzer private definitions.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_PRIV_H__
#define __SWITZER_PRIV_H__

#define _XOPEN_SOURCE 500

#include <stdint.h>
#include <unistd.h>

#include "switzer_fpga.h"

#define BUF_SIZE    4096
#define SWITZER_PORT_SPEED_1G  1000
#define SWITZER_PORT_SPEED_10G 10000

struct switzer_settings {
    uint16_t pci_domain;
    uint8_t pci_bus;
    uint8_t pci_dev;
    uint8_t pci_func;
};

typedef enum {
    SWITZER_IF_SIDE_LINE,
    SWITZER_IF_SIDE_SYS,

    MAX_NR_SWITZER_IF_SIDE
} switzer_if_side_t;

typedef enum {
    SWITZER_LED_OFF,
    SWITZER_LED_GREEN,
    SWITZER_LED_RED,
    SWITZER_LED_AMBER,
    SWITZER_LED_YELLOW,
    SWITZER_LED_BLUE,
    MAX_NR_SWITZER_LED
} switzer_led_t;

typedef enum {
    SWITZER_VMARG_NORMAL,
    SWITZER_VMARG_HIGH,
    SWITZER_VMARG_LOW,
    MAX_NR_SWITZER_VMARG
} switzer_vmarg_t;

typedef enum {
    SWITZER_PRBS_7,
    SWITZER_PRBS_9,
    SWITZER_PRBS_11,
    SWITZER_PRBS_15,
    SWITZER_PRBS_23,
    SWITZER_PRBS_31,
    MAX_NR_SWITZER_PRBS
} switzer_prbs_t;

struct ngio_intf_t;

struct n2g_i2c_iface;

struct ngio_intf_t *switzer_ngio(void);
struct n2g_i2c_iface *switzer_ngio_oir(void);
struct n2g_i2c_iface *switzer_ngio_pca(void);

long switzer_ltc4215_reg_test(void);
long switzer_ltc4215_reg_read(void);
long switzer_ltc4215_reg_write(void);
long switzer_ltc4215_power_info(void);
long switzer_ltc4215_power_off(void);
long switzer_ltc4215_power_on(void);
long switzer_power_led_util(uint8_t led_color);

long switzer_pca_reg_read(void);
long switzer_pca_reg_write(void);

long switzer_utils_i2c_reg_read(struct switzer_i2c_slave *i2c);
long switzer_utils_i2c_reg_write(struct switzer_i2c_slave *i2c);
long switzer_utils_dash_i2c_reg_read(struct switzer_dash_i2c_slave *i2c);
long switzer_utils_dash_i2c_reg_write(struct switzer_dash_i2c_slave *i2c);

long switzer_utils_spi_prom_read_status(struct switzer_spi_prom *prom);
long switzer_utils_spi_prom_write_status(struct switzer_spi_prom *prom);
long switzer_utils_spi_prom_read(struct switzer_spi_prom *prom);
long switzer_utils_spi_prom_write(struct switzer_spi_prom *prom, int sane);
long switzer_utils_spi_prom_erase(struct switzer_spi_prom *prom);
long switzer_utils_spi_prom_test(struct switzer_spi_prom *prom);

long switzer_os_shell(void);

int switzer_10g_test(struct ngio_intf_t *ngio);
int switzer_carrier_test(struct ngio_intf_t *ngio);
int switzer_manhattan_2m_test(struct ngio_intf_t *ngio);
int switzer_manhattan_4t_test(struct ngio_intf_t *ngio);
int switzer_manhattan_1m_test(struct ngio_intf_t *ngio);
int switzer_manhattan_2t_test(struct ngio_intf_t *ngio);

/* ******************** platform definitions ******************** */

struct menuinfo;

extern int do_all_menu_items(struct menuinfo *);

#endif /* __SWITZER_PRIV_H__ */
