/* $Id: switzer_carrier.h,v 1.1 2020/05/22 02:28:47 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_carrier.h,v $
 *------------------------------------------------------------------
 *
 * switzer_carrier.h - Switzer-carrier interfaces.
 *
 * Mar. 2019, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_CARRIER_H__
#define __SWITZER_CARRIER_H__

#include "linux_pci.h"
#include "switzer_fpga.h"
#include "csrs/sm_csrs_top.h"
#include "switzer_priv.h"
#include "switzer_ngio.h"

#define SWITZER_CARRIER_I2C_ADDR_PSW    0x34
#define SWITZER_CARRIER_I2C_ADDR_PM1    0x30
#define SWITZER_CARRIER_I2C_ADDR_PM2    0x10
#define SWITZER_CARRIER_I2C_ADDR_FPGA   0x1A
#define SWITZER_CARRIER_I2C_ADDR_CLK    0x6B
#define SWITZER_CARRIER_I2C_PCIE_SWITCH	0x38

#define SWITZER_CARRIER_PCIE_FUNC_PSW   0
#define SWITZER_CARRIER_PCIE_FUNC_FPGA  5
#define SWITZER_CARRIER_PCIE_FUNC_NIM0  7
#define SWITZER_CARRIER_PCIE_FUNC_NIM1  9

struct switzer_carrier_fpga {
    struct sm_csrs_top *csrs;
    struct pci_dev *pci;
    struct switzer_mmap map;
    struct switzer_mdio *mdio;
    struct switzer_dash_i2c_master *pm_i2c;
    struct switzer_dash_i2c_master *clk_i2c;
    struct switzer_dash_i2c_master *wic_i2c[SWITZER_CARRIER_SLOT_NUM];
    struct switzer_dash_i2c_slave *i2c[3];
};

struct switzer_carrier_host_fpga {
    struct switzer_dash_i2c_master *i2c;
};


struct switzer_carrier_pm {
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_carrier_spi {
    struct switzer_spi_prom *prom;
};

struct switzer_carrier_clk {
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_carrier_wic {
    struct switzer_dash_i2c_slave *i2c;
    struct ngio_intf_t *ngio;
};

struct switzer_carrier_pcisw {
    uint16_t pci_domain;
    uint8_t pci_bus;
    uint8_t pci_dev;
    uint8_t pci_func;
    struct switzer_dash_i2c_slave *i2c;
};

struct switzer_carrier {
    struct switzer_carrier_fpga fpga;
    struct switzer_carrier_pm pm[2];
    struct switzer_carrier_clk clk;
    struct switzer_carrier_spi spi;
    struct switzer_carrier_pcisw pci_switch;
    int testing_slot;
    struct switzer_carrier_wic wic[SWITZER_CARRIER_SLOT_NUM];
    /* platform */
    struct ngio_intf_t *ngio;
    struct switzer_carrier_host_fpga host_fpga;
};

int switzer_carrier_init(struct switzer_carrier *mod,
                         struct switzer_settings *settings);
void switzer_carrier_exit(struct switzer_carrier *mod);

uint8_t switzer_carrier_get_pci_dev_bus(struct switzer_carrier *mod, uint8_t func);

#endif
