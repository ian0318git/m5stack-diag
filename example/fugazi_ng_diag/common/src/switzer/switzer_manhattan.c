/* $Id:
 * $Source:
 *------------------------------------------------------------------
 *
 * switzer_manhattan.c - Switzer-Manhattan interfaces.
 *
 * Feb. 2020, Shiyu Wu <shiywu@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#include "switzer_priv.h"
#include "switzer_common.h"
#include "switzer_miura_reg.h"
#include "switzer_manhattan.h"

#define SWITZER_LANE_TO_MIURA(lane)    (1 << lane)

static const int tx_diable_pin_map[] = {12, 13, 14, 15};

static const int eth_port_pci_func[] = {SWITZER_MANHATTAN_PCI_FUNC_PORT1,
                                        SWITZER_MANHATTAN_PCI_FUNC_PORT2,
                                        SWITZER_MANHATTAN_PCI_FUNC_PORT3,
                                        SWITZER_MANHATTAN_PCI_FUNC_PORT4};
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

extern void *switzer_ngio_i2c_master();

static uint32_t pm_addrs[] = {SWITZER_MANHATTAN_I2C_ADDR_PM1, SWITZER_MANHATTAN_I2C_ADDR_PM2};
static uint32_t poe_addrs[] = {SWITZER_MANHATTAN_I2C_ADDR_POE1, SWITZER_MANHATTAN_I2C_ADDR_POE2};

ssize_t switzer_manhattan_i2c_read(struct switzer_i2c_slave *slave,
                                   uint8_t cmd, void *buf, size_t count)
{
    // TODO: Complete i2c access by V710
    return 0;
}

ssize_t switzer_manhattan_i2c_write(struct switzer_i2c_slave *slave,
                                    uint8_t cmd, const void *buf, size_t count)
{
    // TODO: Complete i2c access by V710
    return 0;
}

#define _NETDEV_SOCK_GET    0
#define _NETDEV_SOCK_OPEN   1
#define _NETDEV_SOCK_CLOSE  2
static inline int netdev_sock_fd(const int op)
{
    static int fd = -1;
    if (fd == -1 || op == _NETDEV_SOCK_OPEN) { /* intentionally not '&&' */
        fd = socket(AF_UNIX,SOCK_DGRAM, 0);
        if (fd < 0) {
            log_err("create socket failed.\n");
            return -1;
        }
    }
    else if (fd >= 0 && op == _NETDEV_SOCK_CLOSE) {
        close(fd);
        fd = -1;
    }
    return fd;
}

#define _MANHATTAN_RD 0
#define _MANHATTAN_WR 1
static int __mdio_rdwr_dbg = 0;
static int switzer_manhattan_phy_rdwr(int rdwr, char *ifname, uint16_t phyad, uint16_t devad,
                                       uint16_t regad, uint16_t *data)
{
    int ret = 0;
    int fd  = -1;
    int op  = 0;
    struct ifreq ifr;
    struct mii_ioctl_data *cdata = (struct mii_ioctl_data *)&ifr.ifr_ifru;


    if (0 > (fd = (netdev_sock_fd(_NETDEV_SOCK_GET)))) {
        log_err("Failed to get netdev fd.\n");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_ifrn.ifrn_name, ifname);

    cdata->phy_id |= devad == 0xFFFF ? (1 << 10) : 0;
    cdata->phy_id |= phyad << 5;
    cdata->phy_id |= devad & ((1 << 5) - 1);
    cdata->reg_num = regad;

    if (__mdio_rdwr_dbg) {
        prt("cdata->phy_id = 0x%04x, cdata->reg_num = 0x%04x\n", cdata->phy_id, cdata->reg_num);
        if (rdwr == _MANHATTAN_WR) {
            prt("%s(WR) ifname-%-8s phyad-0x%04x devad-0x%04x regad-0x%04x data-0x%04x\n",
                __func__, ifname, phyad, devad, regad, *data);
        }
    }

    op = rdwr == _MANHATTAN_RD ? SIOCGMIIREG : SIOCSMIIREG;
    if (op == SIOCSMIIREG)
        cdata->val_in  = *data;

    ret = ioctl(fd, op, &ifr);
    if (ret < 0) {
        log_err("ioctl(%d) failed, ifname:%s, err:%s.\n", op, ifname, strerror(errno));
        return -2;
    }

    if (op == SIOCGMIIREG) {
        *data = cdata->val_out;
        if (__mdio_rdwr_dbg) {
            prt("%s(RD) ifname-%-8s phyad-0x%04x devad-0x%04x regad-0x%04x data:0x%04x\n",
                __func__, ifname, phyad, devad, regad, *data);
        }
    }
    return 0;
}

static int switzer_manhattan_x710_rdwr(int rdwr,
    const struct switzer_manhattan_x710_acc *ctx, int port, uint32_t off, uint32_t *data)
{
    int ret = 0;
    int fd  = -1;
    struct ifreq ifr;
    struct i40e_ioctl_reg *cdata = (struct i40e_ioctl_reg *)&ifr.ifr_ifru;

    if (0 > (fd = (netdev_sock_fd(_NETDEV_SOCK_GET)))) {
        log_err("Failed to get netdev fd.\n");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_ifrn.ifrn_name, &ctx->ifname[port][0]);

    cdata->op  = rdwr;
    cdata->off = off;
    if (rdwr == _MANHATTAN_WR)
        cdata->data = *data;

    ret = ioctl(fd, SIOCDEVREG, &ifr);
    if (ret < 0) {
        log_err("ioctl(%d) failed, err:%s.\n", SIOCDEVREG, strerror(errno));
        return -2;
    }

    if (rdwr == _MANHATTAN_RD)
        *data = cdata->data;

    return 0;
}

static int switzer_manhattan_mdio_rd_82757(void *ctx, uint8_t devad,
                                       uint16_t addr, uint16_t *data)
{
    struct switzer_manhattan_phy_acc *_ctx = &(((struct switzer_manhattan *)ctx)->ctx_ephy);
    __mdio_rdwr_dbg = ((struct switzer_manhattan *)ctx)->mdio_dbg;
    return switzer_manhattan_phy_rdwr(_MANHATTAN_RD, _ctx->ifname, 0, devad, addr, data);
}

static int switzer_manhattan_mdio_wr_82757(void* ctx, uint8_t devad,
                                        uint16_t addr, uint16_t data)
{
    struct switzer_manhattan_phy_acc *_ctx = &(((struct switzer_manhattan *)ctx)->ctx_ephy);
    __mdio_rdwr_dbg = ((struct switzer_manhattan *)ctx)->mdio_dbg;
    return switzer_manhattan_phy_rdwr(_MANHATTAN_WR, _ctx->ifname, 0, devad, addr, &data);
}

static int switzer_manhattan_mdio_rd_54194(void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data)
{
    struct switzer_manhattan_phy_acc *_ctx = &(((struct switzer_manhattan *)ctx)->ctx_ephy);
    return ctx == NULL ? -(__LINE__) : (
            __mdio_rdwr_dbg = ((struct switzer_manhattan *)ctx)->mdio_dbg,
            switzer_manhattan_phy_rdwr(_MANHATTAN_RD, _ctx->ifname, mdio_addr, 0xFFFF, reg_addr, (uint16_t *)data));
}

static int switzer_manhattan_mdio_wr_54194(void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int data)
{
    struct switzer_manhattan_phy_acc *_ctx = &(((struct switzer_manhattan *)ctx)->ctx_ephy);
    uint16_t _data = data & 0xFFFF;
    return ctx == NULL ? -(__LINE__) : (
            __mdio_rdwr_dbg = ((struct switzer_manhattan *)ctx)->mdio_dbg,
            switzer_manhattan_phy_rdwr(_MANHATTAN_WR, _ctx->ifname, mdio_addr, 0xFFFF, reg_addr, &_data));
}

int switzer_manhattan_ephy_read(struct switzer_manhattan *mod,
                               switzer_lane_t lane, switzer_if_side_t if_side,
                               uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_reg_value_get(miura->type, miura->info, devaddr, regaddr, data);
}

int switzer_manhattan_ephy_write(struct switzer_manhattan *mod,
                                switzer_lane_t lane, switzer_if_side_t if_side,
                                uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_reg_value_set(miura->type, miura->info, devaddr, regaddr, data);
}

int switzer_manhattan_iphy_read(struct switzer_manhattan *mod,
                               uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    uint16_t  port = devaddr >> 16;
    __mdio_rdwr_dbg = mod->mdio_dbg;
    return switzer_manhattan_phy_rdwr(_MANHATTAN_RD, mod->ctx_iphy[port].ifname, mod->ctx_iphy[port].phy_id,
                                      devaddr & 0xffff, regaddr, (uint16_t *)data);
}

int switzer_manhattan_iphy_write(struct switzer_manhattan *mod,
                                uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    int ret = 0;
    uint16_t port = devaddr >> 16;
    __mdio_rdwr_dbg = mod->mdio_dbg;
    ret = switzer_manhattan_phy_rdwr(_MANHATTAN_WR, mod->ctx_iphy[port].ifname, mod->ctx_iphy[port].phy_id,
                                      devaddr & 0xffff, regaddr, (uint16_t *)&data);
    if (__mdio_rdwr_dbg && ret == 0) {
        prt("Read back after write >>{\n");
            switzer_manhattan_phy_rdwr(_MANHATTAN_RD, mod->ctx_iphy[port].ifname, mod->ctx_iphy[port].phy_id,
                                      devaddr & 0xffff, regaddr, (uint16_t *)&data);
        prt("}<< Read back after write\n");
    }
    return ret;
}


int switzer_manhattan_x710_read(struct switzer_manhattan *mod, int port, uint32_t off, uint32_t *data)
{
    return switzer_manhattan_x710_rdwr(_MANHATTAN_RD, &mod->ctx_x710, port, off, data);
}

int switzer_manhattan_x710_write(struct switzer_manhattan *mod, int port, uint32_t off, uint32_t data)
{
    return switzer_manhattan_x710_rdwr(_MANHATTAN_WR, &mod->ctx_x710, port, off, &data);
}

int switzer_manhattan_x710_gpio_set(struct switzer_manhattan *mod, int port, int gpio_idx, int bitv)
{
    uint32_t reg = ((gpio_idx << I40E_GLGEN_GPIO_SET_GPIO_INDX_SHIFT) & I40E_GLGEN_GPIO_SET_GPIO_INDX_MASK) |
                   ((bitv     << I40E_GLGEN_GPIO_SET_SDP_DATA_SHIFT)  & I40E_GLGEN_GPIO_SET_SDP_DATA_MASK ) |
                   ((1        << I40E_GLGEN_GPIO_SET_DRIVE_SDP_SHIFT) & I40E_GLGEN_GPIO_SET_DRIVE_SDP_MASK);

    return switzer_manhattan_x710_write(mod, port, I40E_GLGEN_GPIO_SET, reg);
}

int switzer_manhattan_x710_gpio_get(struct switzer_manhattan *mod, int port, int gpio_idx, uint32_t *st_reg)
{
    uint32_t reg = 0;
    if (0 > switzer_manhattan_x710_read(mod, port, I40E_GLGEN_GPIO_STAT, &reg))
        return -1;
    if (st_reg)
        *st_reg = reg;
    return (reg) & (1 << gpio_idx) ? 1 : 0;
}

int switzer_manhattan_x710_gpio_ctrl_set(struct switzer_manhattan *mod, int port, int gpio_idx, uint32_t ctl_val)
{
    return switzer_manhattan_x710_write(mod, port, I40E_GLGEN_GPIO_CTL(gpio_idx), ctl_val);
}

int switzer_manhattan_x710_gpio_ctrl_get(struct switzer_manhattan *mod, int port, int gpio_idx, uint32_t *ctl_val)
{
    return switzer_manhattan_x710_read(mod, port, I40E_GLGEN_GPIO_CTL(gpio_idx), ctl_val);
}

int switzer_manhattan_ephy_autoneg_remote_ability_get(struct switzer_manhattan *mod,
                                                     switzer_lane_t lane,
                                                     switzer_if_side_t if_side,
                                                     unsigned short *fec_ability,
                                                     unsigned short *pause_ability,
                                                     bcm_plp_an_config_t *an_config)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_miura_autoneg_remote_ability_get(miura->info, fec_ability, pause_ability, an_config);
}

int switzer_manhattan_ephy_config_tx_disable(struct switzer_manhattan *mod,
                                            switzer_lane_t lane, int en)
{
    struct switzer_miura *miura = &mod->miura;

    if (bcm_plp_cfg_gpio_pin_set(miura->type, miura->info, tx_diable_pin_map[lane], 0, en, en)) {
        log_warn("bcm_plp_cfg_gpio_pin_set failed\n");
        return -1;
    }

    return 0;
}

static int switzer_manhattan_registers_show(struct switzer_miura *miura,
                                            const switzer_bcm_phy_regs_t *phy_reg_ptr)
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

int switzer_manhattan_registers_dump(struct switzer_manhattan *mod,
                                     switzer_lane_t lane, switzer_if_side_t if_side)
{
    int i, intf_move;
    struct switzer_miura *miura = &mod->miura;
    const switzer_bcm_phy_regs_t *phy_reg_ptr;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    phy_reg_ptr = &bcm82757_phy_standard_reg_tbl[0];
    intf_move = SWITZER_BCM82757_NUM_PHY_INTF;

    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    for (i = 0; i < intf_move; i++) {
        if (switzer_manhattan_registers_show(miura, phy_reg_ptr)){
            log_err("switzer_manhattan_registers_show err\n");
            return -1;
        }
        phy_reg_ptr++;
    }
    return 0;
}
int switzer_manhattan_ephy_dump(struct switzer_manhattan *mod,
                               switzer_lane_t lane, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_phy_status_dump(miura->type, miura->info);
}

int switzer_manhattan_ephy_mac_dump(struct switzer_manhattan *mod,
                                   switzer_lane_t lane, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_mac_diagnostic_dump(miura->type, miura->mac_info);
}

ssize_t switzer_manhattan_sfp_read(struct switzer_manhattan *mod,
                                   switzer_lane_t lane, uint8_t addr,
                                   uint8_t cmd, void *buf, size_t count)
{
    ssize_t sz = count;
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    miura->info.if_side = SWITZER_MIURA_LINE_SIDE;

    if ((sz = bcm_plp_module_read(miura->type, miura->info,
                                  addr, cmd, count, buf))) {
        log_warn("bcm_plp_module_read failed\n");
        return -1;
    }

    return sz;
}

ssize_t switzer_manhattan_sfp_write(struct switzer_manhattan *mod,
                                    switzer_lane_t lane, uint8_t addr,
                                    uint8_t cmd, const void *buf, size_t count)
{
    ssize_t sz = count;
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
    miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    if ((sz = bcm_plp_module_write(miura->type, miura->info,
                                   addr, cmd, count, (void *)buf))) {
        log_warn("bcm_plp_module_write failed\n");
        return -1;
    }

    return sz;
}

int switzer_manhattan_ephy_link_status(struct switzer_manhattan *mod, switzer_lane_t lane,
                                      switzer_if_side_t if_side, unsigned int *link_status)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_link_status_get(miura->type, *info, link_status);
}

int switzer_manhattan_display_eye_scan(struct switzer_manhattan *mod,
                                       switzer_lane_t lane, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_display_eye_scan(miura->type, *info);
}

int switzer_manhattan_miura_loopback_set(struct switzer_manhattan *mod,
                                         switzer_lane_t lane, switzer_if_side_t if_side,
                                         unsigned int lb_mode, unsigned int enable)
{
    struct switzer_miura *miura = &mod->miura;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);

    if (if_side == SWITZER_IF_SIDE_SYS)
        miura->info.if_side = SWITZER_MIURA_SYS_SIDE;
    else
        miura->info.if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_loopback_set(miura->type, miura->info, lb_mode, enable);
}

int switzer_manhattan_miura_prbs_set(struct switzer_manhattan *mod,
                                     switzer_lane_t lane, switzer_if_side_t if_side,
                                     switzer_prbs_t prbs, unsigned int enable)
{
    struct switzer_miura *miura = &mod->miura;
    unsigned int poly;
    int rc;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
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

int switzer_manhattan_prbs_clear_rx_stat(struct switzer_manhattan *mod,
                                         switzer_lane_t lane, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->lane_map = SWITZER_LANE_TO_MIURA(lane);
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

int switzer_manhattan_miura_prbs_check(struct switzer_manhattan *mod,
                                       switzer_lane_t lane, switzer_if_side_t if_side)
{
    struct switzer_miura *miura = &mod->miura;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
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

int switzer_manhattan_miura_firmware_lane_set(struct switzer_manhattan *mod,
                                              switzer_lane_t lane, switzer_if_side_t if_side,
                                              bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_firmware_lane_config_set(miura->type,*info ,firmware_lane_config);
}

int switzer_manhattan_miura_firmware_lane_get(struct switzer_manhattan *mod,
                                              switzer_lane_t lane, switzer_if_side_t if_side,
                                              bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = SWITZER_LANE_TO_MIURA(lane);
    if (if_side == SWITZER_IF_SIDE_SYS)
        info->if_side = SWITZER_MIURA_SYS_SIDE;
    else
        info->if_side = SWITZER_MIURA_LINE_SIDE;
    return bcm_plp_firmware_lane_config_get(miura->type,*info ,firmware_lane_config);
}

int switzer_manhattan_miura_cl73_set(struct switzer_manhattan *mod, switzer_lane_t lane,
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

    miura->info.lane_map = SWITZER_LANE_TO_MIURA(lane);
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

void switzer_manhattan_miura_config_macsec_cleanup(struct switzer_manhattan *mod)
{
    switzer_miura_macsec_exit(&mod->miura);
}

int switzer_manhattan_miura_config_macsec_bypass(struct switzer_manhattan *mod,
                                                 switzer_lane_t lane, int port_speed)
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
    plp_info->lane_map = SWITZER_LANE_TO_MIURA(lane);
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

int switzer_manhattan_miura_config_interface_macsec_bypass(struct switzer_manhattan *mod,
                                                           switzer_lane_t lane,
                                                           bcm_pm_interface_t sys_inf,
                                                           bcm_pm_interface_t line_inf,
                                                           int port_speed)
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
    plp_info->lane_map = SWITZER_LANE_TO_MIURA(lane);
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
                                 line_inf, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }

    plp_info->if_side = SWITZER_MIURA_SYS_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 sys_inf, bcm_pm_RefClk156Mhz,
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

int switzer_manhattan_miura_tx_get(struct switzer_manhattan *mod, switzer_lane_t lane,
                                   switzer_if_side_t if_side, bcm_plp_tx_t *tx_param)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    int rc;

    plp_info->lane_map = SWITZER_LANE_TO_MIURA(lane);
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

int switzer_manhattan_miura_tx_set(struct switzer_manhattan *mod, switzer_lane_t lane,
                                   switzer_if_side_t if_side, bcm_plp_tx_t *tx_param)
{
    struct switzer_miura *miura = &mod->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    int rc;

    plp_info->lane_map = SWITZER_LANE_TO_MIURA(lane);
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

static int switzer_manhattan_host_i2c_master_init(struct switzer_manhattan *mod,
                                                  struct switzer_settings *settings)
{
    struct switzer_manhattan_host_fpga *fpga = &mod->host_fpga;
    struct switzer_dash_i2c_master_settings i2c_settings;

    i2c_settings.base = switzer_ngio_i2c_master();
    if (!(fpga->i2c = switzer_dash_i2c_master_probe(&i2c_settings))) {
        log_err("pm_i2c switzer_dash_i2c_master_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_manhattan_host_i2c_master_exit(struct switzer_manhattan *mod)
{
    struct switzer_manhattan_host_fpga *fpga = &mod->host_fpga;
    switzer_dash_i2c_master_remove(fpga->i2c);
}

static int switzer_manhattan_pm_init(struct switzer_manhattan *mod,
                                     struct switzer_settings *settings)
{
    int i;
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->host_fpga.i2c,
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

static void switzer_manhattan_pm_exit(struct switzer_manhattan *mod)
{
    int i;
    for (i = 0; i < 2; i++) {
        switzer_dash_i2c_slave_remove(mod->pm[i].i2c);
    }
}

static int switzer_manhattan_poe_init(struct switzer_manhattan *mod,
                                      struct switzer_settings *settings)
{
    int i;
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->host_fpga.i2c,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };
    for (i = 0; i < (sizeof(poe_addrs)/sizeof(poe_addrs[0])); i++) {
        i2c_settings.addr = poe_addrs[i];
        if (!(mod->poe[i].i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
            log_err("switzer_dash_i2c_slave_probe failed\n");
            return -1;
        }
    }

    return 0;
}

static void switzer_manhattan_poe_exit(struct switzer_manhattan *mod)
{
    int i;
    for (i = 0; i < (sizeof(poe_addrs)/sizeof(poe_addrs[0])); i++) {
        switzer_dash_i2c_slave_remove(mod->poe[i].i2c);
    }
}

static int _pci_to_eth_port(uint16_t domain, uint8_t bus, uint8_t dev, uint8_t fun, char *devname)
{
    int   ret  = -1;
    DIR   *dp  = NULL;
    FILE  *fp  = NULL;
    struct dirent *de = NULL ;
    char  buf[256];

    unsigned int _dom;
    unsigned int _bus;
    unsigned int _dev;
    unsigned int _fun;

    /** To found the eth interface by pci info.
     ** Example info to parse:
     **     [V0.0.0] thall# cat /sys/class/net/eth10/device/uevent
     **     DRIVER=i40e
     **     PCI_CLASS=20000
     **     PCI_ID=8086:104F
     **     PCI_SUBSYS_ID=8086:0000
     **     PCI_SLOT_NAME=0000:31:00.2 <--
     **     MODALIAS=pci:v00008086d0000104Fsv00008086sd00000000bc02sc00i00
     **/

    if (!(dp = opendir("/sys/class/net"))) {
        log_err("Failed to open dir '/sys/class/net'\n");
        return -1;
    }

    devname[0] = 0;
    while (devname[0] == 0 && (de = readdir(dp))) {
        if (de->d_name[0] == '.')
            continue;
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf) - 1, "/sys/class/net/%s/device/uevent", de->d_name);
        if (NULL == (fp = fopen(buf, "r"))) {
            /* log_warn("  Failed to open file '%s'\n", buf); */
            continue;
        }
        memset(buf, 0, sizeof(buf));
        while (devname[0] == 0 && fgets(buf, sizeof(buf) - 1, fp)) {
            /* PCI_SLOT_NAME=0000:31:00.2 */
            if (strncmp(buf, "PCI_SLOT_NAME=", strlen("PCI_SLOT_NAME=")) == 0) {
                /* log_info("check %s", buf); */
                if (4 == sscanf(buf, "PCI_SLOT_NAME=%x:%x:%x.%x", &_dom, &_bus, &_dev, &_fun)) {
                    if (_dom == domain && _bus == bus && _dev == dev && _fun == fun) {
                        memcpy(devname, de->d_name, IFNAMSIZ);
                        ret = 0;
                    }
                }
            }
        }
        fclose(fp);
        fp = NULL;
    }
    if (fp)
        fclose(fp);
    if (dp)
        closedir(dp);
    return ret;
}

static inline int _pci_to_eth_port_retry(int try, int interv_ms,
    uint16_t domain, uint8_t bus, uint8_t dev, uint8_t fun, char *devname)
{
    int ret = 0;
    int idx = 0;
    while((0 > (ret = _pci_to_eth_port(domain, bus, dev, fun, devname))) && \
          idx++ < try && (switzer_udelay(1000*interv_ms), 1));
    return ret;
}

static int switzer_manhattan_port_init(struct switzer_manhattan *mod,
                                       struct switzer_settings *settings)
{
    int i = 0;

    for (i = 0; i < SWITZER_MAX_PORTS_OF_EACH_PAIR; i++) {
        strncpy(mod->eth_port.intnl_port[i], mod->ctx_x710.ifname[i], IFNAMSIZ);
    }

    // Switer-2.5G doesn't have external PHY.
    if (is_switzer_manhattan_4t(mod)) {
        for (i = 0; i < SWITZER_MAX_PORTS_OF_EACH_PAIR; i++) {
            strncpy(mod->eth_port.extnl_port[i], mod->ctx_x710.ifname[i + SWITZER_MAX_PORTS_OF_EACH_PAIR], IFNAMSIZ);
        }
    }

    return 0;
}

static void switzer_manhattan_port_exit(struct switzer_manhattan *mod)
{
}

static int switzer_manhattan_lm75_init(struct switzer_manhattan *mod,
                                       struct switzer_settings *settings)
{
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->host_fpga.i2c,
        .addr          = SWITZER_MANHATTAN_I2C_ADDR_LM75,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };

    if (!(mod->lm75.i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_dash_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_manhattan_lm75_exit(struct switzer_manhattan *mod)
{
    switzer_dash_i2c_slave_remove(mod->lm75.i2c);
}

static int switzer_manhattan_i2c_mux_init(struct switzer_manhattan *mod,
                                          struct switzer_settings *settings)
{
    struct switzer_dash_i2c_slave_settings i2c_settings = {
        .master        = mod->host_fpga.i2c,
        .addr          = SWITZER_MANHATTAN_I2C_ADDR_MUX,
        .sub_addr_len  = 0,
        .sub_addr      = 0,
        .mux           = 0,
        .i2c_speed     = SWITZER_DASH_I2C_100K,
        .addr_extended = SWITZER_DASH_I2C_NOMAL_ADDR,
    };

    if (!(mod->i2c_mux.i2c = switzer_dash_i2c_slave_probe(&i2c_settings))) {
        log_err("switzer_dash_i2c_slave_probe failed\n");
        return -1;
    }

    return 0;
}

static void switzer_manhattan_i2c_mux_exit(struct switzer_manhattan *mod)
{
    switzer_dash_i2c_slave_remove(mod->i2c_mux.i2c);
}

static int switzer_manhattan_x710_gpio_init(struct switzer_manhattan *mod)
{
    uint32_t ctrl = 0;

    {
        /* SWITZER_MANHATTAN_INTPHY_RST : By NVM : TODO? */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// High when SDP_DATA is set to 1
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate during reset
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , OUT_DEFAULT );// High
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x11, PHY_PIN_NAME);// PhyRst, TODO: meaningful to X710?
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3 , PRT_BIT_MAP );// Port-0 and port-1
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_INTPHY_RST, ctrl))
            return -(__LINE__);
    }

    if (is_switzer_manhattan_4t(mod)) {
        /* SWITZER_MANHATTAN_EXTPHY_RST */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// High when SDP_DATA is set to 1
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate during reset
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_DEFAULT );// Low
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x11, PHY_PIN_NAME);// PhyRst, TODO: meaningful to X710?
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0xc , PRT_BIT_MAP );// Port-2 and port-3
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_EXTPHY_RST, ctrl))
            return -(__LINE__);

        /* 4T: SWITZER_MANHATTAN_P2_LASI0_INT == 2T: SWITZER_MANHATTAN_EXTPHY_INT*/
        /* 4T: SWITZER_MANHATTAN_P3_LASI1_INT */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_DIR     );// Input
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_DEFAULT );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x11, PHY_PIN_NAME);// PhyInt, TODO
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x4 , PRT_BIT_MAP );// Port-2
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_P2_LASI0_INT, ctrl))
            return -(__LINE__);

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 3   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x8 , PRT_BIT_MAP );// Port-3
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_P3_LASI1_INT, ctrl))
            return -(__LINE__);

        /* SWITZER_MANHATTAN_LED_PORT2_ENB_YEL */
        /* SWITZER_MANHATTAN_LED_PORT2_ENB_GRN */
        /* SWITZER_MANHATTAN_LED_PORT3_ENB_YEL */
        /* SWITZER_MANHATTAN_LED_PORT3_ENB_GRN */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// Driven high
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP? TODO
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , LED_INVRT   );// Active Low(Don't care if func is sdp)
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , LED_BLINK   );// Don't blink(Don't care if func is sdp)
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , LED_MODE    );// Always on(Don't care if func is sdp)
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , OUT_DEFAULT );// High
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Don't care

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x4 , PRT_BIT_MAP );// Port-2
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT2_ENB_YEL, ctrl))
            return -(__LINE__);

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x4 , PRT_BIT_MAP );// Port-2
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT2_ENB_GRN, ctrl))
            return -(__LINE__);

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 3   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x8 , PRT_BIT_MAP );// Port-3
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT3_ENB_YEL, ctrl))
            return -(__LINE__);

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x8 , PRT_BIT_MAP );// Port-3
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT3_ENB_GRN, ctrl))
            return -(__LINE__);

        /* SWITZER_MANHATTAN_SFP_P3_DSL_DYGP TODO*/
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// High when SDP_DATA is set to 1
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate during reset
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , OUT_DEFAULT );// High(Pulled up externally)
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Rsvd
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x0 , PRT_BIT_MAP );// No port
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_SFP_P3_DSL_DYGP, ctrl))
            return -(__LINE__);


        /* SWITZER_MANHATTAN_GPIO5_POR_BYPASS */

        #if 0
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// High when SDP_DATA is set to 1
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate during reset
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , OUT_DEFAULT );// High(Pulled up externally)
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Rsvd
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x0 , PRT_BIT_MAP );// No port
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_SFP_P3_DSL_DYGP, ctrl))
            return -(__LINE__);
        #endif


        /* SWITZER_MANHATTAN_EXTPHY_GPIO0_0 :TODO: Usage ? */
        /* SWITZER_MANHATTAN_EXTPHY_GPIO0_1 :TODO: Usage ? */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// High when SDP_DATA is set to 1
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate during reset
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_DEFAULT );// Low
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Rsvd

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x4 , PRT_BIT_MAP );// Port-2
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_EXTPHY_GPIO0_0, ctrl))
            return -(__LINE__);

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 3   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x8 , PRT_BIT_MAP );// Port-3
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_EXTPHY_GPIO0_1, ctrl))
            return -(__LINE__);
    }

    {
        /* ACTIVE LED config */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_FUNC    );// LED
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , LED_INVRT   );// Active Low
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , LED_BLINK   );// Blink
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0xd , LED_MODE    );// MAC Active
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        if (is_switzer_manhattan_1m(mod)) {
            I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl,       PRT_NUM     );
            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PRT_NUM     );//MAC_PORT 0
            if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT0_ACTIVE_GRN, ctrl))
                return -(__LINE__);
        }
        if (is_switzer_manhattan_2t(mod)) {
            I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl,       PRT_NUM     );
            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM     );//MAC_PORT 1
            if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT1_ACTIVE_GRN, ctrl))
                return -(__LINE__);

            I40E_GLGEN_GPIO_CTRL_FLD_CLR(ctrl,       PRT_NUM     );
            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 3   , PRT_NUM     );//MAC_PORT 3
            if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT0_ACTIVE_GRN, ctrl))
                return -(__LINE__);
        }
    }

    #if 0
    {
        /* Default after power on
           PRT PRT_NA DIR TRI_CTL OUT_CTL FUN LED_INVT LED_BLNK LED_M INT_M OUT_DEF PHY_PIN PRT_BMAP
           0   0      1   1       0       0   0        0        16    0     0       3f      0       LED0_1
           1   0      1   1       0       0   0        0        16    0     0       3f      0       LED1_1
           2   0      1   1       0       0   0        0        16    0     0       3f      0       LED2_1
           3   0      1   1       0       0   0        0        16    0     0       3f      0       LED3_1
           These 4 are controlled by NVM?????
        */

        /* SWITZER_MANHATTAN_LED_PORT0_LNK_GRN : TODO: By NVM? */
        /* SWITZER_MANHATTAN_LED_PORT1_LNK_GRN : TODO: By NVM? */
        /* SWITZER_MANHATTAN_LED_PORT2_LNK_GRN : TODO: By NVM? */
        /* SWITZER_MANHATTAN_LED_PORT3_LNK_GRN : TODO: By NVM? */

        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// Driven high
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP? TODO
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , LED_INVRT   );// Active Low
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , LED_BLINK   );// Don't blink
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , LED_MODE    );// Always on? TODO
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_DEFAULT );// Low(Active Low)
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Don't care

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x1 , PRT_BIT_MAP );// Port-0
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT0_LNK_GRN, ctrl))
            return -(__LINE__);

        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x2 , PRT_BIT_MAP );// Port-1
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT1_LNK_GRN, ctrl))
            return -(__LINE__);

        if (is_switzer_manhattan_4t(mod)) {
            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x4 , PRT_BIT_MAP );// Port-2
            if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT2_LNK_GRN, ctrl))
                return -(__LINE__);

            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 3   , PRT_NUM     );// Don't care
            I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x8 , PRT_BIT_MAP );// Port-3
            if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_LED_PORT3_LNK_GRN, ctrl))
                return -(__LINE__);
        }
    }
    #endif

    {
        /* SWITZER_MANHATTAN_PSE_INTB */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_DIR     );// Input
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_DEFAULT );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Rsvd
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x4 , PRT_BIT_MAP );// Don't care
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_PSE_INTB, ctrl))
            return -(__LINE__);
    }

    {
        /* SWITZER_MANHATTAN_PRI_IF_RDY */
        ctrl = 0;
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 2   , PRT_NUM     );// Don't care
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PRT_NUM_NA  );// SDP val controlled by GLGEN_GPIO_SET
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , PIN_DIR     );// Output
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 1   , TRI_CTL     );// High when SDP_DATA is set to 1
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_CTL     );// Tristate during reset
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , PIN_FUNC    );// SDP
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , INT_MODE    );// NO Intr
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0   , OUT_DEFAULT );// Low
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0x3f, PHY_PIN_NAME);// Rsvd
        I40E_GLGEN_GPIO_CTRL_FLD_SET(ctrl, 0xc , PRT_BIT_MAP );// Port-2 and port-3
        if (0 > switzer_manhattan_x710_gpio_ctrl_set(mod, 0, SWITZER_MANHATTAN_PRI_IF_RDY, ctrl))
            return -(__LINE__);
    }

    return 0;
}

static int switzer_manhattan_x710_init(struct switzer_manhattan *mod,
                                       struct switzer_settings *settings)
{
    int i  = 0;
    int j  = 0;
    int N  = 0;
    int ret= 0;
    char ethname[IFNAMSIZ + 8];

    N = is_switzer_manhattan_4t(mod) ? SWITZER_MANHATTAN_PORT_NUM:
        is_switzer_manhattan_2t(mod) ? SWITZER_MANHATTAN_PORT_NUM:SWITZER_MANHATTAN_IPORT_NUM;

    if (0 > netdev_sock_fd(_NETDEV_SOCK_OPEN)) {
        return -1;
    }

    for(j = 0; j < 2; j++) {
        memset(&mod->ctx_x710, 0, sizeof(mod->ctx_x710));
        for (i = 0; i < N; i++) {
            memset(ethname, 0, sizeof(ethname));
            ret = _pci_to_eth_port_retry(5, 1000, settings->pci_domain,
                                        settings->pci_bus, settings->pci_dev, i, ethname);
            if (ret < 0) {
                log_err("Failed to find eth of pci %04x:%02x:%02x.%x\n",
                        settings->pci_domain, settings->pci_bus, settings->pci_dev, i);
                return -1;
            }
            prt("%s found %-8s of pci %04x:%02x:%02x.%x\n", __func__, ethname,
                settings->pci_domain, settings->pci_bus, settings->pci_dev, i);

            memcpy(mod->ctx_x710.ifname[i], ethname, IFNAMSIZ);
        }

        /* Do it again, on some host, udev will rename the eth port name
         * TODO: better method be devised.
         */
        prt("Sleep 2s to let udev rename the eth port. TODO:Better way maybe devised.\n");
        sleep(2);
    }

    if (0 > switzer_manhattan_x710_gpio_init(mod)) {
        log_err("Failed to init X710 GPIOs\n");
        return -2;
    }

    return 0;
}

static int switzer_manhattan_x710_exit(struct switzer_manhattan *mod)
{
    memset(&mod->ctx_x710, 0, sizeof(mod->ctx_x710));
    netdev_sock_fd(_NETDEV_SOCK_CLOSE);
    return 0;
}

static int switzer_manhattan_intphy_init(struct switzer_manhattan *mod,
                                      struct switzer_settings *settings)
{
    int i = 0;

    memset(&mod->ctx_iphy[0], 0, sizeof(mod->ctx_iphy));
    for (i = 0; i < SWITZER_MANHATTAN_IPORT_NUM; i++) {
        memcpy(&mod->ctx_iphy[i].ifname[0], &mod->ctx_x710.ifname[i][0], IFNAMSIZ);
        mod->ctx_iphy[i].phy_id = I40E_AQ_PHY_REG_ACCESS_INTERNAL;
    }
    return 0;
}

/*
 * reset  : 0-unreset, 1-reset
 */
static int switzer_manhattan_extphy_reset(struct switzer_manhattan *mod, int reset)
{
    if (0 > switzer_manhattan_x710_gpio_set(mod, 0, SWITZER_MANHATTAN_EXTPHY_RST, !reset))
        return -1;
    return 0;
}

static int switzer_manhattan_extphy_reset_54194(void *priv, int reset)
{
    return switzer_manhattan_extphy_reset((struct switzer_manhattan *)priv, reset);
}

/*
 * Params:
 *      priv    - struct switzer_manhattan *
 *      act     - action
 *                0      : Check if intr occured, 'arg' contains the value to be compared with
 *                others : Reserved for expansion
 *      arg     - Depends on 'act'
 */
static int switzer_manhattan_extphy_intr_cb_54194(void *priv, int act, void *arg)
{
    uint32_t intr_st = 0;
    int      ret     = 0;

    switch(act) {
    case 0:
        if (0 > (ret = switzer_manhattan_x710_gpio_get(priv, 0, SWITZER_MANHATTAN_EXTPHY_INT, &intr_st))) {
            log_err("Failed to get X710 GPIO SWITZER_MANHATTAN_EXTPHY_INT, ret:%d\n", ret);
            return -(__LINE__);
        }
        printf("SWITZER_MANHATTAN_EXTPHY_INT status in X710:%d\n", !ret); /* SWITZER_MANHATTAN_EXTPHY_INT line is pulled-up */
        return (!ret) == (long)arg ? 0 : -(__LINE__);
    default:
        log_err("Invalid action specified:%d\n", act);
        return -(__LINE__);
    }
}

static int switzer_manhattan_extphy_init_54194(struct switzer_manhattan *mod,
                                               struct switzer_settings *settings)
{
    int ret = 0;
    int i   = 0;
    char eth_if[MANHATTAN_BCM54194_PORT_NUMB][IFNAMSIZ] = {{0,},};


    if (!is_switzer_manhattan_2t(mod))
        return 0;

    for(i = 0; i < SWITZER_MANHATTAN_EPORT_NUM; i++) {
        strncpy(&eth_if[i][0], &mod->ctx_x710.ifname[SWITZER_MANHATTAN_IPORT_NUM + i][0], IFNAMSIZ);
    }

    if (0 != (ret = manhattan_bcm54194_init(mod,
                        switzer_manhattan_mdio_rd_54194,
                        switzer_manhattan_mdio_wr_54194,
                        switzer_manhattan_extphy_intr_cb_54194,/* intr check externally, etc*/
                        switzer_manhattan_extphy_reset_54194, eth_if))) {
        log_err("switzer_manhattan_extphy_init_54194 failed, ret:%d\n", ret);
        return ret;
    }
    mod->mdio_dbg = 0; /* disable it here */

    printf("%s Done\n", __func__);
    return ret;
}

static int switzer_manhattan_extphy_exit_54194(struct switzer_manhattan *mod)
{
    if (!is_switzer_manhattan_2t(mod))
        return 0;
    manhattan_bcm54194_exit(mod);
    return 0;
}

static int switzer_manhattan_extphy_init_82757(struct switzer_manhattan *mod,
                                               struct switzer_settings *settings)
{
    int rc;
    struct switzer_miura_settings miura_settings = {
        .type  = "miura",
        .ctx   = mod,
        .read  = switzer_manhattan_mdio_rd_82757,
        .write = switzer_manhattan_mdio_wr_82757,
    };
    struct switzer_miura *miura = &mod->miura;
    uint16_t data;

    if (!is_switzer_manhattan_4t(mod))
        return 0;

    if ((rc = switzer_manhattan_mdio_rd_82757(mod,
            BCMI_MIURA_DIRECT_CHIP_CNTRL_CHIP_IDr >> 16,
            BCMI_MIURA_DIRECT_CHIP_CNTRL_CHIP_IDr & 0xffff, &data))) {
        log_err("Cannot access external PHY, rc:%d\n", rc);
        return rc;
    }
    prt("External Phy ID 0x%04x\n", data);
    if (!(data == 0x2757 || data == 0x2756)) {
        log_err("Invalid BCM8275 ID\n");
        return -1;
    }
    mod->mdio_dbg = 0; /* disable it here */

    /* miura sdk init */
    if ((rc = switzer_miura_init(miura, &miura_settings))) {
        log_err("switzer_miura_init failed\n");
        return rc;
    }
    miura->info.lane_map = 0x1; /* lane 0 */

    /* disable TX_Disable */
    if (switzer_manhattan_ephy_config_tx_disable(mod, SWITZER_LANE_0, 0)) {
        log_warn("switzer_manhattan_ephy_config_tx_disable failed\n");
        return -1;
    }

    if (switzer_manhattan_ephy_config_tx_disable(mod, SWITZER_LANE_1, 0)) {
        log_warn("switzer_manhattan_ephy_config_tx_disable failed\n");
        return -1;
    }
    return 0;
}

static int switzer_manhattan_extphy_init(struct switzer_manhattan *mod,
                                      struct switzer_settings *settings)
{
    int rc;

    memset(&mod->ctx_ephy, 0, sizeof(mod->ctx_ephy));
    /* Use the first external port of x710 */
    memcpy(&mod->ctx_ephy.ifname[0], &mod->ctx_x710.ifname[0 + SWITZER_MANHATTAN_IPORT_NUM][0], IFNAMSIZ);
    mod->ctx_ephy.phy_id = I40E_AQ_PHY_REG_ACCESS_EXTERNAL;

    if (0 != (rc = switzer_manhattan_extphy_reset(mod, 0))) {
        log_err("switzer_manhattan_extphy_reset failed, rc:%d\n", rc);
        return rc;
    }
    switzer_udelay(100000); /* delay 100ms TODO*/

    if (0 != (rc = switzer_manhattan_extphy_init_82757(mod, settings))) {
        log_err("switzer_manhattan_extphy_init_82757 failed, rc:%d\n", rc);
        return rc;
    }

    if (0 != (rc = switzer_manhattan_extphy_init_54194(mod, settings))) {
        log_err("switzer_manhattan_extphy_init_54194 failed, rc:%d\n", rc);
        return rc;
    }
    return 0;
}

static int switzer_manhattan_phy_init(struct switzer_manhattan *mod,
                                      struct switzer_settings *settings)
{
    int rc = 0;

    if ((rc = switzer_manhattan_intphy_init(mod, settings))) {
        log_err("switzer_manhattan_intphy_init failed\n");
        return rc;
    }

    if ((rc = switzer_manhattan_extphy_init(mod, settings))) {
        log_err("switzer_manhattan_extphy_init failed\n");
        return rc;
    }

    return 0;
}

static void switzer_manhattan_phy_exit(struct switzer_manhattan *mod)
{
    if (is_switzer_manhattan_4t(mod))
        switzer_miura_exit(&mod->miura);

    switzer_manhattan_extphy_exit_54194(mod);

    memset(&mod->ctx_iphy[0], 0, sizeof(mod->ctx_iphy));
    memset(&mod->ctx_ephy   , 0, sizeof(mod->ctx_ephy));
}

int is_switzer_manhattan_2m(struct switzer_manhattan *mod)
{
    return mod->manhattan_type == SWITZER_MANHATTAN_2M ? 1 : 0;
}

int is_switzer_manhattan_4t(struct switzer_manhattan *mod)
{
    if (mod->chk_4t)
        return mod->manhattan_type == SWITZER_MANHATTAN_4T ? 1 : 0;
    return 0;
}

int is_switzer_manhattan_1m(struct switzer_manhattan *mod)
{
    return mod->manhattan_type == SWITZER_MANHATTAN_1M ? 1 : 0;
}

int is_switzer_manhattan_2t(struct switzer_manhattan *mod)
{
    return mod->manhattan_type == SWITZER_MANHATTAN_2T ? 1 : 0;
}

int switzer_manhattan_stage_get(struct switzer_manhattan *mod)
{
    return mod->stage;
}

int switzer_manhattan_stage_set(struct switzer_manhattan *mod, int stage)
{
    mod->stage = (stage >= SWITZER_MANHATTAN_STAGE_INIT_0) &&
                 (stage <  SWITZER_MANHATTAN_STAGE_MAX)    ?
                  stage : (log_err("Invalid stage value:%d\n", stage), mod->stage);
    return mod->stage;
}

int switzer_manhattan_init(struct switzer_manhattan *mod,
                           struct switzer_settings *settings)
{
    int rc = 0;

    if ((rc = switzer_manhattan_host_i2c_master_init(mod, settings))) {
        log_err("switzer_manhattan_host_i2c_master_init failed\n");
        return rc;
    }

    if ((rc = switzer_manhattan_pm_init(mod, settings))) {
        log_err("switzer_manhattan_pm_init failed\n");
        goto err;
    }

    if ((rc = switzer_manhattan_poe_init(mod, settings))) {
        log_err("switzer_manhattan_poe_init failed\n");
        goto err1;
    }

    if ((rc = switzer_manhattan_i2c_mux_init(mod, settings))) {
        log_err("switzer_manhattan_poe_init failed\n");
        goto err2;
    }
    if ((rc = switzer_manhattan_lm75_init(mod, settings))) {
        log_err("switzer_manhattan_lm75_init failed\n");
        goto err3;
    }

    if ((rc = switzer_manhattan_x710_init(mod, settings))) {
        log_err("switzer_manhattan_x710_init failed\n");
        goto err4;
    }

    if ((rc = switzer_manhattan_phy_init(mod, settings))) {
        log_err("switzer_manhattan_phy_init failed\n");
        goto err5;
    }

    if ((rc = switzer_manhattan_port_init(mod, settings))) {
        log_err("switzer_manhattan_port_init failed\n");
        goto err6;
    }

    return 0;

err6:
    switzer_manhattan_phy_exit(mod);
err5:
    switzer_manhattan_x710_exit(mod);
err4:
    switzer_manhattan_lm75_exit(mod);
err3:
    switzer_manhattan_i2c_mux_exit(mod);
err2:
    switzer_manhattan_poe_exit(mod);
err1:
    switzer_manhattan_pm_exit(mod);
err:
    switzer_manhattan_host_i2c_master_exit(mod);
    return rc;
}

void switzer_manhattan_exit(struct switzer_manhattan *mod)
{
    switzer_manhattan_port_exit(mod);
    switzer_manhattan_phy_exit(mod);
    switzer_manhattan_x710_exit(mod);
    switzer_manhattan_lm75_exit(mod);
    switzer_manhattan_i2c_mux_exit(mod);
    switzer_manhattan_poe_exit(mod);
    switzer_manhattan_pm_exit(mod);
    switzer_manhattan_host_i2c_master_exit(mod);
}
