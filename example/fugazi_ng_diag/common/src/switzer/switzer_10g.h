/* $Id: switzer_10g.h,v 1.2 2019/08/06 06:56:16 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_10g.h,v $
 *------------------------------------------------------------------
 *
 * switzer_10g.h - Switzer-10G interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __SWITZER_10G_H__
#define __SWITZER_10G_H__

#include "linux_pci.h"
#include "switzer_fpga.h"
#include "switzer_miura.h"
#include "csrs/nim_te_csrs_top.h"
#include "switzer_priv.h"

#define SWITZER_10G_I2C_ADDR_SFP    0x50
#define SWITZER_10G_I2C_ADDR_PM     0x30
#define SWITZER_10G_I2C_ADDR_PCA    0x21

struct switzer_10g_fpga {
    struct nim_te_csrs_top *csrs;
    struct pci_dev *pci;
    struct switzer_mmap map;
    struct switzer_mdio *mdio;
    struct switzer_i2c_master *i2c;
};

struct switzer_10g_sfp {
    struct switzer_i2c_slave *i2c;
};

struct switzer_10g_pm {
    struct switzer_i2c_slave *i2c;
};

struct switzer_10g_spi {
    struct switzer_spi_prom *prom;
};

struct switzer_10g {
    struct switzer_10g_fpga fpga;
    struct switzer_miura miura;
    struct switzer_10g_sfp sfp;
    struct switzer_10g_pm pm;
    struct switzer_10g_spi spi;
    /* flags */
    unsigned int phy_i2c_enabled:1;
    unsigned int phy_gpio_enabled:1;
    /* platform */
    struct ngio_intf_t *ngio;
};

int switzer_10g_init(struct switzer_10g *mod,
                     struct switzer_settings *settings);
void switzer_10g_exit(struct switzer_10g *mod);

int switzer_10g_phy_read(struct switzer_10g *mod, switzer_if_side_t if_side,
                         uint32_t devaddr, uint32_t regaddr, uint32_t *data);
int switzer_10g_phy_write(struct switzer_10g *mod, switzer_if_side_t if_side,
                          uint32_t devaddr, uint32_t regaddr, uint32_t data);
int switzer_10g_phy_autoneg_remote_ability_get(struct switzer_10g *mod, switzer_if_side_t if_side,
                                        unsigned short *fec_ability, unsigned short *pause_ability,
                                        bcm_plp_an_config_t *an_config);
int switzer_10g_registers_dump(struct switzer_10g *mod, switzer_if_side_t if_side);
int switzer_10g_phy_dump(struct switzer_10g *mod, switzer_if_side_t if_side);
int switzer_10g_phy_mac_dump(struct switzer_10g *mod, switzer_if_side_t if_side);
int switzer_10g_phy_link_status(struct switzer_10g *mod,
                                switzer_if_side_t if_side, unsigned int *link_status);
int switzer_10g_display_eye_scan(struct switzer_10g *mod, switzer_if_side_t if_side);

ssize_t switzer_10g_sfp_read(struct switzer_10g *mod, uint8_t addr,
                             uint8_t cmd, void *buf, size_t count);
ssize_t switzer_10g_sfp_write(struct switzer_10g *mod, uint8_t addr,
                              uint8_t cmd, const void *buf, size_t count);

int switzer_10g_led_set(struct switzer_10g *mod, switzer_led_t color);

ssize_t switzer_10g_spi_prom_read(struct switzer_10g *mod,
                                  uint32_t addr, void *buf, size_t count);
ssize_t switzer_10g_spi_prom_write(struct switzer_10g *mod,
                                   uint32_t addr, const void *buf, size_t count);
ssize_t switzer_10g_spi_prom_erase(struct switzer_10g *mod,
                                   uint32_t addr, size_t count);

int switzer_10g_miura_loopback_set(struct switzer_10g *mod,
                                   switzer_if_side_t if_side,
                                   unsigned int lb_mode, unsigned int enable);
int switzer_10g_miura_firmware_lane_get(struct switzer_10g *mod,
                                        switzer_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);
int switzer_10g_miura_firmware_lane_set(struct switzer_10g *mod,
                                        switzer_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config);
int switzer_10g_miura_cl73_set(struct switzer_10g *mod,
                               switzer_if_side_t if_side, unsigned int enable);

void switzer_10g_miura_config_macsec_cleanup(struct switzer_10g *mod);
int switzer_10g_miura_config_macsec_bypass(struct switzer_10g *mod, int port_speed);

int switzer_10g_miura_tx_get(struct switzer_10g *mod, switzer_if_side_t if_side,
                             bcm_plp_tx_t *tx_param);
int switzer_10g_miura_tx_set(struct switzer_10g *mod, switzer_if_side_t if_side,
                             bcm_plp_tx_t *tx_param);

int switzer_10g_miura_prbs_set(struct switzer_10g *mod,
                               switzer_if_side_t if_side,
                               switzer_prbs_t prbs, unsigned int enable);
int switzer_10g_prbs_clear_rx_stat(struct switzer_10g *mod, switzer_if_side_t if_side);
int switzer_10g_miura_prbs_check(struct switzer_10g *mod,
                                 switzer_if_side_t if_side);

#endif /* __SWITZER_10G_H__ */
