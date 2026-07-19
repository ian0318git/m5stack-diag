/* $Id: switzer_10g.c,v 1.5 2021/04/12 13:37:34 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_10g.c,v $
 *------------------------------------------------------------------
 *
 * switzer_10g.c - Switzer-10G interfaces.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <errno.h>

#include "switzer_priv.h"
#include "switzer_common.h"
#include "switzer_miura_reg.h"
#include "switzer_10g.h"

enum switzer_82757_intf
{
    BCM82757_PCS_INTF,
    BCM82757_PMA_PMD_INTF,
    BCM82757_MAX_INTF
};

typedef struct switzer_reg_info {
    char *name;
    unsigned int devaddr;
    unsigned int offset;
    unsigned char type;
    unsigned long size;
    unsigned int mask;
    unsigned int reset_val;
} switzer_reg_info_t;

typedef struct switzer_bcm_phy_regs {
    const char *intfname;
    int phy_intf;
    const switzer_reg_info_t *intfregs;
} switzer_bcm_phy_regs_t;

static const switzer_reg_info_t bcm_82757_pcs_reg[] = {
    {"RX_X4_PCS_CTRL0", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PCS_CONTROL0r,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"TX_X4_PCS_STS", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PCS_STATUSr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_PCS_LIVE_STS", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PCS_LIVE_STATUSr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_PCS_LATCH_STS0", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PCS_LATCH_STATUS0r,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_PCS_LATCH_STS1", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PCS_LATCH_STATUS1r,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_PCS_GBOX_STS", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_RX_GBOX_STATUSr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_FEC0", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_FEC_CONTROL0r,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_FEC1", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_FEC_CONTROL1r,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_FEC_CORRBLKSH", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_FEC_CORRECTED_COUNTERHr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_FEC_CORRBLKSL", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_FEC_CORRECTED_COUNTERLr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_FEC_UNCORRBLKSH", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_FEC_UNCORRECTED_COUNTERHr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_FEC_UNCORRBLKSL", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_FEC_UNCORRECTED_COUNTERLr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_T12_FEC_CORRBLKSH", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_T12_FEC_CORRECTED_COUNTERHr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_T12_FEC_CORRBLKSL", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_T12_FEC_CORRECTED_COUNTERLr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_T12_FEC_UNCORRBLKSH", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_T12_FEC_UNCORRECTED_COUNTERHr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"RX_X4_T12_FEC_UNCORRBLKSL", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_T12_FEC_UNCORRECTED_COUNTERLr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"PATGEN_RXPKTCNT_U", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_RX_PKT_COUNTERUr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"PATGEN_RXPKTCNT_L", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_RX_PKT_COUNTERLr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"PATGEN_TXPKTCNT_U", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_TX_PKT_COUNTERUr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"PATGEN_TXPKTCNT_L", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_TX_PKT_COUNTERLr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"PATGEN_CRCERRCNT", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PKT_CRC_ERRCOUNTERr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"PATGEN_PRTPERRCTR", SWITZER_MIURA_DEV_PCS, BCMI_MIURA_INDIRECT_PKT_PRTP_ERRCOUNTERr,
        SWITZER_MIURA_READ_WRITE, 4, 0x00000000, 0x0000},
    {"end", 0, 0x00, 0, 0, 0, 0},
};

static const switzer_bcm_phy_regs_t bcm82757_phy_standard_reg_tbl[] = {
    {"PCS", BCM82757_PCS_INTF, bcm_82757_pcs_reg},
};

#define SWITZER_BCM82757_NUM_PHY_INTF sizeof(bcm82757_phy_standard_reg_tbl) /\
                                                sizeof(switzer_bcm_phy_regs_t)


extern int bcm_plp_miura_autoneg_remote_ability_get(bcm_plp_access_t phy_info,
                   unsigned short *fec_ability, unsigned short *pause_ability,
                   bcm_plp_an_config_t* an_config);

static int switzer_10g_fpga_init(struct switzer_10g *mod,
                                 struct switzer_settings *settings)
{
    struct switzer_10g_fpga *fpga = &mod->fpga;
    struct pci_dev *pci;
    struct switzer_mdio_settings mdio_settings;
    struct switzer_i2c_master_settings i2c_settings;
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

    mdio_settings.base = &fpga->csrs->phy_ctrl;
    if (!(fpga->mdio = switzer_mdio_probe(&mdio_settings))) {
        log_err("switzer_mdio_probe failed\n");
        goto err1;
    }

    i2c_settings.base = &fpga->csrs->sfp_i2c;
    if (!(fpga->i2c = switzer_i2c_master_probe(&i2c_settings))) {
        log_err("switzer_i2c_master_probe failed\n");
        goto err2;
    }

    return 0;
err2:
    switzer_mdio_remove(fpga->mdio);
err1:
    switzer_file_munmap(&fpga->map);
err:
    pci_dev_remove(fpga->pci);
    pci_dev_put(fpga->pci);
    return rc;
}

static void switzer_10g_fpga_exit(struct switzer_10g *mod)
{
    struct switzer_10g_fpga *fpga = &mod->fpga;

    switzer_i2c_master_remove(fpga->i2c);
    switzer_mdio_remove(fpga->mdio);
    switzer_file_munmap(&fpga->map);
    pci_dev_remove(fpga->pci);
    pci_dev_put(fpga->pci);
}

static int switzer_10g_mdio_read(void* ctx, uint8_t devad,
                                 uint16_t addr, uint16_t *data)
{
    return switzer_mdio_read(ctx, 0, devad, addr, data);
}

static int switzer_10g_mdio_write(void* ctx, uint8_t devad,
                                  uint16_t addr, uint16_t data)
{
    return switzer_mdio_write(ctx, 0, devad, addr, data);
}

int switzer_10g_phy_read(struct switzer_10g *mod, switzer_if_side_t if_side,
                         uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_reg_value_get(miura->type, miura->info, devaddr, regaddr, data);
}

int switzer_10g_phy_write(struct switzer_10g *mod, switzer_if_side_t if_side,
                          uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_reg_value_set(miura->type, miura->info, devaddr, regaddr, data);
}

int switzer_10g_phy_autoneg_remote_ability_get(struct switzer_10g *mod, switzer_if_side_t if_side,
                                       unsigned short *fec_ability, unsigned short *pause_ability,
                                       bcm_plp_an_config_t *an_config)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_miura_autoneg_remote_ability_get(miura->info, fec_ability, pause_ability, an_config);
}

static int switzer_10g_registers_show(struct switzer_miura *miura, const switzer_bcm_phy_regs_t *phy_reg_ptr)
{
    int rc;
    uint32_t data, regaddr, devaddr;
    const switzer_reg_info_t *reg_ptr;

    reg_ptr = phy_reg_ptr->intfregs;

    while (reg_ptr->size != 0) {
        devaddr = reg_ptr->devaddr;
        regaddr = reg_ptr->offset;
        rc = bcm_plp_reg_value_get(miura->type, miura->info, devaddr, regaddr, &data);
        if (rc) {
            log_err("reg read err\n");
            return rc;
        }
        prt("%s : %-32s reg %d.%#.4x = %#.4x\n", phy_reg_ptr->intfname, reg_ptr->name,
                devaddr, regaddr, data);
        reg_ptr++;
    }
    return 0;
}

int switzer_10g_registers_dump(struct switzer_10g *mod, switzer_if_side_t if_side)
{
    int i, intf_move;
    struct switzer_miura *miura = &mod->miura;
    const switzer_bcm_phy_regs_t *phy_reg_ptr;

    miura->info.lane_map = 0x1;
    phy_reg_ptr = &bcm82757_phy_standard_reg_tbl[0];
    intf_move = SWITZER_BCM82757_NUM_PHY_INTF;

    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    for (i = 0; i < intf_move; i++) {
        if (switzer_10g_registers_show(miura, phy_reg_ptr)){
            log_err("switzer_10g_registers_show err\n");
            return -1;
        }
        phy_reg_ptr++;
    }
    return 0;
}
int switzer_10g_phy_dump(struct switzer_10g *mod, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_phy_status_dump(miura->type, miura->info);
}

int switzer_10g_phy_mac_dump(struct switzer_10g *mod, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_mac_diagnostic_dump(miura->type, miura->mac_info);
}

int switzer_10g_phy_link_status(struct switzer_10g *mod,
                                switzer_if_side_t if_side, unsigned int *link_status)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_link_status_get(miura->type, *info, link_status);
}

int switzer_10g_display_eye_scan(struct switzer_10g *mod, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_display_eye_scan(miura->type, *info);
}

static int switzer_10g_phy_init(struct switzer_10g *mod,
                                struct switzer_settings *settings)
{
    struct switzer_miura_settings miura_settings = {
        .type  = "miura",
        .ctx   = mod->fpga.mdio,
        .read  = switzer_10g_mdio_read,
        .write = switzer_10g_mdio_write,
    };
    struct switzer_miura *miura = &mod->miura;
    struct nim_te_csrs_top *csrs = mod->fpga.csrs;
    int rc;

    /* unreset phy */
    PHY_CSRS__PHY_CTRL_REG__TGE_PHY_RESET__CLR(csrs->phy_ctrl.phy_ctrl);
    switzer_mdelay(50);         /* arbitrary */

    /* miura sdk init */
    if ((rc = switzer_miura_init(miura, &miura_settings))) {
        log_err("switzer_miura_init failed\n");
        return rc;
    }
    miura->info.lane_map = 0x1; /* lane 0 */

    return 0;
}

static void switzer_10g_phy_exit(struct switzer_10g *mod)
{
    switzer_miura_exit(&mod->miura);
}

static int switzer_10g_sfp_init(struct switzer_10g *mod,
                                struct switzer_settings *settings)
{
    struct switzer_i2c_slave_settings i2c_settings = {
        .master = mod->fpga.i2c,
        .addr   = SWITZER_10G_I2C_ADDR_SFP,
        .port   = 0,
        .xfer   = SWITZER_I2C_XFER_BYTES,
    };
    struct switzer_miura *miura = &mod->miura;

    /* disable TX_Disable */
    if (bcm_plp_cfg_gpio_pin_set(miura->type, miura->info, 12, 0, 0, 0)) {
        log_warn("bcm_plp_cfg_gpio_pin_set failed\n");
        return -1;
    }

    if (!(mod->sfp.i2c = switzer_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_10g_sfp_exit(struct switzer_10g *mod)
{
    switzer_i2c_slave_remove(mod->sfp.i2c);
}

ssize_t switzer_10g_sfp_read(struct switzer_10g *mod, uint8_t addr,
                             uint8_t cmd, void *buf, size_t count)
{
    ssize_t sz = count;
    struct switzer_miura *miura = &mod->miura;

    if (mod->phy_i2c_enabled) {
        miura->info.lane_map = 0x1;
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
        if ((sz = bcm_plp_module_read(miura->type, miura->info,
                                      addr, cmd, count, buf))) {
            log_warn("bcm_plp_module_read failed\n");
            return -1;
        }
    } else {
        sz = __switzer_i2c_slave_read(mod->sfp.i2c, SWITZER_I2C_XFER_BYTES,
                                      addr, cmd, buf, count);
    }
    return sz;
}

ssize_t switzer_10g_sfp_write(struct switzer_10g *mod, uint8_t addr,
                              uint8_t cmd, const void *buf, size_t count)
{
    ssize_t sz = count;
    struct switzer_miura *miura = &mod->miura;

    if (mod->phy_i2c_enabled) {
        miura->info.lane_map = 0x1;
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
        if ((sz = bcm_plp_module_write(miura->type, miura->info,
                                       addr, cmd, count, (void *)buf))) {
            log_warn("bcm_plp_module_write failed\n");
            return -1;
        }
    } else {
        sz = __switzer_i2c_slave_write(mod->sfp.i2c, SWITZER_I2C_XFER_BYTES,
                                       addr, cmd, buf, count);
    }
    return sz;
}

static int switzer_10g_pm_init(struct switzer_10g *mod,
                               struct switzer_settings *settings)
{
    struct switzer_i2c_slave_settings i2c_settings = {
        .master = mod->fpga.i2c,
        .addr   = SWITZER_10G_I2C_ADDR_PM,
        .port   = 1,
        .xfer   = SWITZER_I2C_XFER_BYTES,
    };

    if (!(mod->pm.i2c = switzer_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_10g_pm_exit(struct switzer_10g *mod)
{
    switzer_i2c_slave_remove(mod->pm.i2c);
}

static int switzer_10g_led_init(struct switzer_10g *mod,
                                struct switzer_settings *settings)
{
    int rc;
    switzer_if_side_t if_side = SWITZER_IF_SIDE_LINE;
    uint32_t devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    uint32_t regaddr, data;

    data = 0x80e2;
    regaddr = BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO0_0_CONTROLr;
    if ((rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data)))
        goto err;
    regaddr = BCMI_MIURA_DIRECT_PAD_CNTRL_GPIO1_0_CONTROLr;
    if ((rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data)))
        goto err;
    data = 0x0a48;
    regaddr = BCMI_MIURA_DIRECT_CTRL_LED_OPMODEr;
    if ((rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data)))
        goto err;
    data = 0x10ff;
    regaddr = BCMI_MIURA_DIRECT_CTRL_LED_PARAMSr;
    if ((rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data)))
        goto err;
    return 0;
err:
    log_err("switzer_10g_phy_write failed\n");
    return rc;
}

static void switzer_10g_led_exit(struct switzer_10g *mod)
{
}

int switzer_10g_led_set(struct switzer_10g *mod, switzer_led_t color)
{
    struct nim_te_csrs_top *csrs = mod->fpga.csrs;
    switch (color) {
    case SWITZER_LED_OFF:
        SFP_CTRL_CSRS__SFP_LED_GREEN_REG__SFP_LED_GREEN__MODIFY(csrs->sfp_ctrl.sfp_led_green, 0);
        SFP_CTRL_CSRS__SFP_LED_AMBER_REG__SFP_LED_AMBER__MODIFY(csrs->sfp_ctrl.sfp_led_amber, 0);
        break;
    case SWITZER_LED_GREEN:
        SFP_CTRL_CSRS__SFP_LED_GREEN_REG__SFP_LED_GREEN__MODIFY(csrs->sfp_ctrl.sfp_led_green, 1);
        SFP_CTRL_CSRS__SFP_LED_AMBER_REG__SFP_LED_AMBER__MODIFY(csrs->sfp_ctrl.sfp_led_amber, 0);
        break;
    case SWITZER_LED_AMBER:
        SFP_CTRL_CSRS__SFP_LED_GREEN_REG__SFP_LED_GREEN__MODIFY(csrs->sfp_ctrl.sfp_led_green, 0);
        SFP_CTRL_CSRS__SFP_LED_AMBER_REG__SFP_LED_AMBER__MODIFY(csrs->sfp_ctrl.sfp_led_amber, 1);
        break;
    default:
        log_err("unsupported color %d\n", color);
        return -1;
    }

    return 0;
}

static int switzer_10g_spi_prom_init(struct switzer_10g *mod,
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

static void switzer_10g_spi_prom_exit(struct switzer_10g *mod)
{
    switzer_spi_prom_remove(mod->spi.prom);
}

ssize_t switzer_10g_spi_prom_read(struct switzer_10g *mod,
                                  uint32_t addr, void *buf, size_t count)
{
    return switzer_spi_prom_read(mod->spi.prom, addr, buf, count);
}

ssize_t switzer_10g_spi_prom_write(struct switzer_10g *mod,
                                   uint32_t addr, const void *buf, size_t count)
{
    return switzer_spi_prom_write(mod->spi.prom, addr, buf, count);
}

ssize_t switzer_10g_spi_prom_erase(struct switzer_10g *mod,
                                   uint32_t addr, size_t count)
{
    return switzer_spi_prom_erase(mod->spi.prom, addr, count);
}

int switzer_10g_init(struct switzer_10g *mod,
                     struct switzer_settings *settings)
{
    int rc;

    memset(mod, 0, sizeof(*mod));
    mod->phy_i2c_enabled = 1;
    mod->phy_gpio_enabled = 1;

    if ((rc = switzer_10g_fpga_init(mod, settings))) {
        log_err("switzer_10g_fpga_init failed\n");
        return rc;
    }
    if ((rc = switzer_10g_phy_init(mod, settings))) {
        log_err("switzer_10g_phy_init failed\n");
        goto err;
    }
    if ((rc = switzer_10g_sfp_init(mod, settings))) {
        log_err("switzer_10g_sfp_init failed\n");
        goto err1;
    }
    if ((rc = switzer_10g_pm_init(mod, settings))) {
        log_err("switzer_10g_pm_init failed\n");
        goto err2;
    }
    if ((rc = switzer_10g_led_init(mod, settings))) {
        log_err("switzer_10g_led_init failed\n");
        goto err3;
    }
    if ((rc = switzer_10g_spi_prom_init(mod, settings))) {
        log_err("switzer_10g_spi_prom_init failed\n");
        goto err4;
    }

    return 0;
err4:
    switzer_10g_spi_prom_exit(mod);
err3:
    switzer_10g_pm_exit(mod);
err2:
    switzer_10g_sfp_exit(mod);
err1:
    switzer_10g_phy_exit(mod);
err:
    switzer_10g_fpga_exit(mod);
    return rc;
}

void switzer_10g_exit(struct switzer_10g *mod)
{
    switzer_10g_spi_prom_exit(mod);
    switzer_10g_led_exit(mod);
    switzer_10g_pm_exit(mod);
    switzer_10g_sfp_exit(mod);
    switzer_10g_phy_exit(mod);
    switzer_10g_fpga_exit(mod);
}

int switzer_10g_miura_loopback_set(struct switzer_10g *mod,
                                   switzer_if_side_t if_side,
                                   unsigned int lb_mode, unsigned int enable)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_loopback_set(miura->type, miura->info, lb_mode, enable);
}

int switzer_10g_miura_prbs_set(struct switzer_10g *mod,
                               switzer_if_side_t if_side,
                               switzer_prbs_t prbs, unsigned int enable)
{
    struct switzer_miura *miura = &mod->miura;
    unsigned int poly;
    int rc;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    switch (prbs) {
    case SWITZER_PRBS_7:
        poly = 0;
        break;
    case SWITZER_PRBS_9:
        poly = 1;
        break;
    case SWITZER_PRBS_11:
        poly = 2;
        break;
    case SWITZER_PRBS_15:
        poly = 3;
        break;
    case SWITZER_PRBS_23:
        poly = 4;
        break;
    default:
    case SWITZER_PRBS_31:
        poly = 5;
        break;
    }

    rc = bcm_plp_prbs_set(miura->type, miura->info, 0, poly, 0, 0, enable);
    if (!rc && !enable)
        rc = bcm_plp_prbs_clear(miura->type, miura->info, 0);
    return rc;
}

int switzer_10g_prbs_clear_rx_stat(struct switzer_10g *mod, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;

    if ((rc = bcm_plp_prbs_status_get(miura->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }
    return 0;
}

int switzer_10g_miura_prbs_check(struct switzer_10g *mod,
                                 switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    if ((rc = bcm_plp_prbs_status_get(miura->type, miura->info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }

    if (prbs_lock) {
        prt("prbs locked\n");
        if (prbs_lock_loss) {
            prt("prbs lock loss\n");
        }
        prt("prbs error count: %d\n", error_count);
    } else {
        prt("prbs unlock\n");
    }
    return (!prbs_lock || prbs_lock_loss || error_count) ? -1 : 0;
}

int switzer_10g_miura_firmware_lane_set(struct switzer_10g *mod,
                                        switzer_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_firmware_lane_config_set(miura->type,*info ,firmware_lane_config);
}

int switzer_10g_miura_firmware_lane_get(struct switzer_10g *mod,
                                        switzer_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_firmware_lane_config_get(miura->type,*info ,firmware_lane_config);
}

int switzer_10g_miura_cl73_set(struct switzer_10g *mod,
                               switzer_if_side_t if_side, unsigned int enable)
{
    struct switzer_miura *miura = &mod->miura;
    int rc;
    unsigned short tech_ability = 5;
    unsigned short fec_ability = 0;
    unsigned short pause_ability = 0;
    bcm_plp_an_config_t an_config = {
        .cl72_en = 1,
        .tech_ability = 5,
    };

    miura->info.lane_map = 0x1;
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    rc = bcm_plp_cl73_ability_set(miura->type, miura->info, tech_ability,
                                  fec_ability, pause_ability, an_config);
    if (rc) {
        log_err("bcm_plp_cl73_ability_set failed, return code [%d]\n", rc);
        return rc;
    }

    rc = bcm_plp_cl73_set(miura->type, miura->info, enable);
    if (rc) {
        log_err("bcm_plp_cl73_set failed, return code [%d]\n", rc);
        return rc;
    }

    return 0;
}

void switzer_10g_miura_config_macsec_cleanup(struct switzer_10g *mod)
{
    switzer_miura_macsec_exit(&mod->miura);
}

int switzer_10g_miura_config_macsec_bypass(struct switzer_10g *mod, int port_speed)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    bcm_plp_sec_phy_access_t sec_info;
    int aux = 0, speed = port_speed;
    int rc;

    if ((rc = switzer_miura_macsec_init(miura, 1)))
        return rc;

    memset(&sec_info, 0, sizeof(sec_info));

    /* Set Secy Config set for Both Egress and Ingress */
    plp_info->lane_map = 0x1;
    plp_info->if_side = SWITZER_MIURA_LINE_SIDE;
    memcpy(&sec_info.phy_info, plp_info, sizeof(bcm_plp_access_t));
    sec_info.macsec_side = SWITZER_MIURA_EGRESS;

    /* Secy Config set */
    rc = bcm_plp_secy_config_set(miura->type, &sec_info);
    if (rc) {
        log_err("bcm_plp_secy_config_set failed for device-id [%d], "
                "return code [%d]\n", sec_info.macsec_side, rc);
        return rc;
    }

    /* Mode Config set for each port */
    plp_info->if_side = SWITZER_MIURA_LINE_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 bcm_pm_InterfaceSFI, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }

    plp_info->if_side = SWITZER_MIURA_SYS_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 bcm_pm_InterfaceKR, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }

    /* Egress, Set SECY to Bypass */
    plp_info->if_side  = SWITZER_MIURA_LINE_SIDE;
    memcpy(&sec_info.phy_info, plp_info, sizeof(bcm_plp_access_t));
    sec_info.macsec_side = SWITZER_MIURA_EGRESS;
    rc = bcm_plp_secy_bypass_set(miura->type, &sec_info, 1);
    if (rc) {
        log_err("bcm_plp_secy_bypass_set failed for PHY-ID [%d], "
                "macsec_side [%d], return code [%d]\n",
                plp_info->phy_addr, sec_info.macsec_side, rc);
        return rc;
    }

    /* Ingress, Set SECY to Bypass */
    sec_info.macsec_side = SWITZER_MIURA_INGRESS;
    rc = bcm_plp_secy_bypass_set(miura->type, &sec_info, 1);
    if (rc) {
        log_err("bcm_plp_secy_bypass_set failed for PHY-ID [%d], "
                "macsec_side [%d], return code [%d]\n",
                plp_info->phy_addr, sec_info.macsec_side, rc);
        return rc;
    }

    return rc;
}

int switzer_10g_miura_tx_get(struct switzer_10g *mod, switzer_if_side_t if_side,
                             bcm_plp_tx_t *tx_param)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    int rc;

    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    rc = bcm_plp_tx_get(miura->type, *plp_info, tx_param);

    if (rc) {
        log_err("Error in getting PHY tx Config\n");
    }

    return rc;
}

int switzer_10g_miura_tx_set(struct switzer_10g *mod, switzer_if_side_t if_side,
                             bcm_plp_tx_t *tx_param)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    int rc;

    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    rc = bcm_plp_tx_set(miura->type, *plp_info, tx_param);

    if (rc) {
        log_err("Error in setting PHY tx Config\n");
    }

    return rc;
}
