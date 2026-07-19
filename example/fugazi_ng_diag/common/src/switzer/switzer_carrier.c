/* $Id: switzer_carrier.c,v 1.1 2020/05/22 02:28:47 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_carrier.c,v $
 *------------------------------------------------------------------
 *
 * switzer_carrier.c - Switzer-carrier interfaces.
 *
 * Mar. 2019, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "switzer_priv.h"
#include "switzer_common.h"
#include "switzer_carrier.h"
#include "linux_pci.h"

extern void *switzer_ngio_i2c_master();

static uint32_t pm_addrs[] = {SWITZER_CARRIER_I2C_ADDR_PM1, SWITZER_CARRIER_I2C_ADDR_PM2};

static int __switzer_carrier_nim_i2c_init(struct switzer_carrier *mod,
                                         struct switzer_settings *settings, int slot)
{
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->fpga.wic_i2c[slot],
        .addr          = 0,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };

    if (!(mod->wic[slot].i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_dash_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void __switzer_carrier_nim_i2c_exit(struct switzer_carrier *mod, int slot)
{
    switzer_dash_i2c_slave_remove(mod->wic[slot].i2c);
}

static int switzer_carrier_nim_i2c_init(struct switzer_carrier *mod,
                                   struct switzer_settings *settings)
{
    int i;
    for (i = 0; i < SWITZER_CARRIER_SLOT_NUM; i++) {
        if (__switzer_carrier_nim_i2c_init(mod, settings, i))
            goto slot_i2c_err;
    }

    return 0;

slot_i2c_err:
    while (i-- > 0) {
        __switzer_carrier_nim_i2c_exit(mod, i);
    }
    return -1;
}

static void switzer_carrier_nim_i2c_exit(struct switzer_carrier *mod)
{
    int i;
    for (i = 0; i < SWITZER_CARRIER_SLOT_NUM; i++) {
        __switzer_carrier_nim_i2c_exit(mod, i);
    }
}

static int switzer_carrier_pm_init(struct switzer_carrier *mod,
                                   struct switzer_settings *settings)
{
    int i;
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->fpga.pm_i2c,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };

    for (i = 0; i < 2; i++) {
        i2c_settings.addr = pm_addrs[i];
        if (!(mod->pm[i].i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
            log_err("switzer_dash_i2c_slave_probe failed\n");
            return -1;
        }
    }

    return 0;
}

static void switzer_carrier_pm_exit(struct switzer_carrier *mod)
{
    int i;
    for (i = 0; i < 2; i++) {
        switzer_dash_i2c_slave_remove(mod->pm[i].i2c);
    }
}

static int switzer_carrier_pcw_i2c_init(struct switzer_carrier *mod,
                                        struct switzer_settings *settings)
{
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->host_fpga.i2c,
        .addr          = SWITZER_CARRIER_I2C_PCIE_SWITCH,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };

    if (!(mod->pci_switch.i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_dash_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_carrier_pcw_i2c_exit(struct switzer_carrier *mod)
{
    switzer_dash_i2c_slave_remove(mod->pci_switch.i2c);
}

static int switzer_carrier_clk_init(struct switzer_carrier *mod,
                                    struct switzer_settings *settings)
{
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->fpga.clk_i2c,
        .addr          = SWITZER_CARRIER_I2C_ADDR_CLK,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };

    if (!(mod->clk.i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_dash_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_carrier_clk_exit(struct switzer_carrier *mod)
{
    switzer_dash_i2c_slave_remove(mod->clk.i2c);
}

static int switzer_carrier_spi_prom_init(struct switzer_carrier *mod,
                                         struct switzer_settings *settings)
{
    struct switzer_spi_prom_settings prom_settings = {
        .base = &mod->fpga.csrs->mb_ctrl.spi_prom_csrs,
    };

    if (!(mod->spi.prom = switzer_spi_prom_probe(&prom_settings))) {
        log_err("switzer_spi_prom_probe failed\n");
        return -1;
    }
    return 0;
}

static void switzer_carrier_spi_prom_exit(struct switzer_carrier *mod)
{
    switzer_spi_prom_remove(mod->spi.prom);
}

static int switzer_carrier_host_i2c_master_init(struct switzer_carrier *mod,
                                              struct switzer_settings *settings)
{
    struct switzer_carrier_host_fpga *fpga = &mod->host_fpga;
    struct switzer_dash_i2c_master_settings i2c_settings;

    i2c_settings.base = switzer_ngio_i2c_master();
    if (!(fpga->i2c = switzer_dash_i2c_master_probe(&i2c_settings))) {
        log_err("pm_i2c switzer_i2c_master_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_carrier_host_i2c_master_exit(struct switzer_carrier *mod)
{
    struct switzer_carrier_host_fpga *fpga = &mod->host_fpga;
    switzer_dash_i2c_master_remove(fpga->i2c);
}

static int switzer_carrier_fpga_i2c_init(struct switzer_carrier *mod,
                                         struct switzer_settings *settings)
{
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    struct switzer_dash_i2c_master_settings i2c_settings;
    int i;

    i2c_settings.base = &fpga->csrs->pm_i2c;
    if (!(fpga->pm_i2c = switzer_dash_i2c_master_probe(&i2c_settings))) {
        log_err("pm_i2c switzer_i2c_master_probe failed\n");
        return -1;
    }

    i2c_settings.base = &fpga->csrs->clk_i2c;
    if (!(fpga->clk_i2c = switzer_dash_i2c_master_probe(&i2c_settings))) {
        log_err("clk_i2c switzer_i2c_master_probe failed\n");
        goto err;
    }

    for (i = 0; i < SWITZER_CARRIER_SLOT_NUM; i++) {
        i2c_settings.base = &fpga->csrs->wic_i2c[i];
        if (!(fpga->wic_i2c[i] = switzer_dash_i2c_master_probe(&i2c_settings))) {
            log_err("wic_i2c %d switzer_i2c_master_probe failed\n", i);
            goto err1;
        }
    }

    return 0;

err1:
    while (i-- > 0) {
        switzer_dash_i2c_master_remove(fpga->wic_i2c[i]);
    }
    switzer_dash_i2c_master_remove(fpga->clk_i2c);
err:
    switzer_dash_i2c_master_remove(fpga->pm_i2c);
    return -1;
}

static void switzer_carrier_fpga_i2c_exit(struct switzer_carrier *mod)
{
    int i;
    struct switzer_carrier_fpga *fpga = &mod->fpga;

    for (i = 0; i < SWITZER_CARRIER_SLOT_NUM; i++) {
        switzer_dash_i2c_master_remove(fpga->wic_i2c[i]);
    }

    switzer_dash_i2c_master_remove(fpga->clk_i2c);
    switzer_dash_i2c_master_remove(fpga->pm_i2c);
}

static int switzer_carrier_fpga_init(struct switzer_carrier *mod,
                                     struct switzer_settings *settings)
{
    struct switzer_carrier_fpga *fpga = &mod->fpga;
    struct pci_dev *pci;
    int rc;

    if (!(pci = switzer_pci_dev_get(settings->pci_domain, settings->pci_bus,
                                    settings->pci_dev, settings->pci_func))) {
        log_err("pci_dev_get_by_path failed: %04x:%02x:%02x.%01x\n",
                settings->pci_domain, settings->pci_bus,
                settings->pci_dev, settings->pci_func);
        return -ENODEV;
    }

    fpga->pci = pci;

    fpga->map.paddr = (void *)pci->bar[0].address;
    fpga->map.length = pci->bar[0].size;

    if ((rc = switzer_file_mmap(NULL, &fpga->map,
                                SWITZER_MMAP_READ | SWITZER_MMAP_WRITE)))
        goto err;
    fpga->csrs = fpga->map.vaddr;

    if ((rc = pci_dev_enable(pci, true) < 0))
        log_warn("fail to enable pci device: "
                 "%04lx:%02lx:%02lx.%ld-%04lx:%04lx\n",
                 pci->domain, pci->bus, pci->dev, pci->func,
                 pci->vendor, pci->device);

    rc = -ENXIO;

    if ((rc = switzer_carrier_fpga_i2c_init(mod, settings)) < 0) {
        log_err("switzer_carrier_fpga_i2c_init failed\n");
        goto err1;
    }

    return 0;

err1:
    switzer_file_munmap(&fpga->map);
err:
    pci_dev_remove(fpga->pci);
    pci_dev_put(fpga->pci);
    return rc;
}

static void switzer_carrier_fpga_exit(struct switzer_carrier *mod)
{
    struct switzer_carrier_fpga *fpga = &mod->fpga;

    switzer_carrier_fpga_i2c_exit(mod);
    switzer_file_munmap(&fpga->map);
    pci_dev_remove(fpga->pci);
    pci_dev_put(fpga->pci);

}

static uint8_t __switzer_carrier_get_pci_secondary_bus(uint8_t pci_bus,
                                                       uint8_t pci_dev, uint8_t pci_func)
{
    struct pci_dev *pci;
    uint8_t bus = 0;

    if (!(pci = pci_dev_get_by_path(0, pci_bus, pci_dev, pci_func))) {
        log_err("pci_dev_get_by_path failed: %04x:%02x:%02x.%01x\n",
                0, pci_bus, pci_dev, pci_func);
        return 0;
    }

    if (pci_read_config_byte(pci, 0x19, &bus)) {
        log_err("pci_read_config_byte failed\n");
        return 0;
    }
    pci_dev_put(pci);

    return bus;
}

uint8_t switzer_carrier_get_pci_dev_bus(struct switzer_carrier *mod, uint8_t dev)
{
    uint8_t downstream_ports_bus;
    struct switzer_carrier_pcisw *pci_switch = &mod->pci_switch;

    /* Get PCIE switch downstream bus number */
    downstream_ports_bus = __switzer_carrier_get_pci_secondary_bus(pci_switch->pci_bus,
                                                 pci_switch->pci_dev, pci_switch->pci_func);

    /* Get PCIe device bus number */
    return __switzer_carrier_get_pci_secondary_bus(downstream_ports_bus, dev, 0);
}

static int switzer_carrier_device_pcie_settings_get(struct switzer_carrier *mod,
                                                    struct switzer_settings *settings,
                                                    uint8_t dev)
{
    if (settings == NULL)
        return -1;

    settings->pci_domain = 0;
    settings->pci_bus = switzer_carrier_get_pci_dev_bus(mod, dev);
    settings->pci_dev = 0;
    settings->pci_func = 0;

    return 0;
}

#define PCI_PREFETCHABLE_MEMORY_BASE_ADDRESS_REG 0x24
static uint32_t __set_pci_prefetchable_memory(uint32_t bus, uint16_t dev, uint32_t func,
                                              uint32_t base, uint32_t limit)
{
    struct pci_dev *pci;
    uint32_t val;
    val = (base & 0x0000FFFF) + ((limit & 0x0000FFFF) << 16);
    int rc;

    if (!(pci = pci_dev_get_by_path(0, bus, dev, func))) {
        log_err("pci_dev_get_by_path failed: %04x:%02x:%02x.%01x\n",
                0, bus, dev, func);
        return -1;
    }

    rc = pci_write_config_dword(pci, PCI_PREFETCHABLE_MEMORY_BASE_ADDRESS_REG, val);

    pci_dev_put(pci);

    return rc;
}

static int switzer_carrier_pcie_switch_configuration(struct switzer_carrier *mod,
                                                     struct switzer_settings *settings)
{
    uint32_t val;
    uint32_t base, secondary_bus;
    struct switzer_carrier_pcisw *pci_switch = &mod->pci_switch;
    struct pci_dev *pci;

    if (!(pci = switzer_pci_dev_get(pci_switch->pci_domain, pci_switch->pci_bus,
                                    pci_switch->pci_dev, pci_switch->pci_func))) {
        log_err("pci_dev_get_by_path failed: %04x:%02x:%02x.%01x\n",
                pci_switch->pci_domain, pci_switch->pci_bus,
                pci_switch->pci_dev, pci_switch->pci_func);
        return -1;
    }

    if (pci_read_config_dword(pci, PCI_PREFETCHABLE_MEMORY_BASE_ADDRESS_REG, &val)) {
        log_err("pci_read_config_byte failed\n");
        return -1;
    }

    /* set upstream port's perfetchable memory */
    base = (val & 0x0000FFFF);
    __set_pci_prefetchable_memory(pci_switch->pci_bus, pci_switch->pci_dev,
                                  pci_switch->pci_func, base, base + 0x0FF0);

    /* set downstream NIM1 port's perfetchable memory */
    secondary_bus = pci_switch->pci_bus + 1;
    __set_pci_prefetchable_memory(secondary_bus, SWITZER_CARRIER_PCIE_FUNC_NIM0,
                                  pci_switch->pci_func, base, base + 0x07F0);

    /* set downstream NIM2 port's perfetchable memory */
    base += 0x0800;
    __set_pci_prefetchable_memory(secondary_bus, SWITZER_CARRIER_PCIE_FUNC_NIM1,
                                  pci_switch->pci_func, base, base + 0x07F0);

    pci_dev_remove(pci);
    pci_dev_put(pci);
    pci_rescan();

    return 0;
}

static int switzer_carrier_pcie_switch_init(struct switzer_carrier *mod,
                                            struct switzer_settings *settings)
{
    int rc;
    struct switzer_carrier_pcisw *pci_switch = &mod->pci_switch;
    pci_switch->pci_domain = settings->pci_domain;
    pci_switch->pci_bus = settings->pci_bus;
    pci_switch->pci_dev = settings->pci_dev;
    pci_switch->pci_func = settings->pci_func;

    if ((rc = switzer_carrier_pcw_i2c_init(mod, settings))) {
        log_err("switzer_carrier_pcieswitch_i2c_init failed\n");
        return rc;
    }

    if ((rc = switzer_carrier_pcie_switch_configuration(mod, settings))) {
        log_err("switzer_carrier_pcie_switch_configuration failed\n");
        return rc;
    }

    return 0;
}

static void switzer_carrier_pcie_switch_exit(struct switzer_carrier *mod)
{
    switzer_carrier_pcw_i2c_exit(mod);
}

int switzer_carrier_init(struct switzer_carrier *mod,
                         struct switzer_settings *settings)
{
    int rc;
    struct switzer_settings fpga_settings;

    if ((rc = switzer_carrier_host_i2c_master_init(mod, settings))) {
        log_err("switzer_carrier_host_i2c_master_init failed\n");
        goto err;
    }

    if ((rc = switzer_carrier_pcie_switch_init(mod, settings))) {
        log_err("switzer_carrier_pcie_switch_init failed\n");
        return rc;
    }

    if ((rc = switzer_carrier_device_pcie_settings_get(mod, &fpga_settings,
                                                       SWITZER_CARRIER_PCIE_FUNC_FPGA))) {
        log_err("switzer_carrier_device_pcie_settings_init failed\n");
        goto err1;
    }
    if ((rc = switzer_carrier_fpga_init(mod, &fpga_settings))) {
        log_err("switzer_carrier_fpga_init failed\n");
        goto err1;
    }

    if ((rc = switzer_carrier_pm_init(mod, settings))) {
        log_err("switzer_carrier_pm_init failed\n");
        goto err2;
    }

    if ((rc = switzer_carrier_clk_init(mod, settings))) {
        log_err("switzer_carrier_clk_init failed\n");
        goto err3;
    }

    if ((rc = switzer_carrier_nim_i2c_init(mod, settings))) {
        log_err("switzer_carrier_nim_i2c_init failed\n");
        goto err4;
    }

    if ((rc = switzer_carrier_spi_prom_init(mod, settings))) {
        log_err("switzer_carrier_spi_prom_init failed\n");
        goto err5;
    }

    return 0;

err5:
    switzer_carrier_nim_i2c_exit(mod);
err4:
    switzer_carrier_clk_exit(mod);
err3:
    switzer_carrier_pm_exit(mod);
err2:
    switzer_carrier_fpga_exit(mod);
err1:
    switzer_carrier_host_i2c_master_exit(mod);
err:
    switzer_carrier_pcie_switch_exit(mod);
    return rc;
}

void switzer_carrier_exit(struct switzer_carrier *mod)
{
    switzer_carrier_spi_prom_exit(mod);
    switzer_carrier_nim_i2c_exit(mod);
    switzer_carrier_clk_exit(mod);
    switzer_carrier_pm_exit(mod);
    switzer_carrier_fpga_exit(mod);
    switzer_carrier_pcie_switch_exit(mod);
    switzer_carrier_host_i2c_master_exit(mod);
}
