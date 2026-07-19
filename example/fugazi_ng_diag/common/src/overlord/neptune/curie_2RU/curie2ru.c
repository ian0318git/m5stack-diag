/* $Id: curie2ru.c,v 1.1 2020/01/09 01:01:56 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru.c - Curie2ru interfaces.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "dash_fpga.h"
#include "curie2ru.h"
#include "common_utils.h"
#include "curie2ru_quadra28_reg.h"
#include "curie2ru_miura_reg.h"

#define CURIE2RU_LANE_TO_MIURA(lane)    (1 << lane)
#define CURIE2RU_IF_SIDE_TO_MIURA(if_side)                  \
    ((if_side == CURIE2RU_IF_SIDE_SYS) ?                    \
     CURIE2RU_MIURA_SYS_SIDE : CURIE2RU_MIURA_LINE_SIDE)

#define CURIE2RU_IF_SIDE_TO_QUADRA28(if_side)               \
    ((if_side == CURIE2RU_IF_SIDE_SYS) ?                    \
     CURIE2RU_MIURA_SYS_SIDE : CURIE2RU_MIURA_LINE_SIDE)

typedef struct curie2ru_bcm_phy_regs_t_ {
    const char *intfname;
    int phy_intf;
    const reg_info_t *intfregs;
} curie2ru_bcm_phy_regs_t;

static void curie2ru_nim_eth_init()
{
    system("brctl addif br0 eth7");
    system("brctl addif br0 eth6");
}

static void curie2ru_nim_eth_exit()
{
    system("brctl delif br0 eth6");
    system("brctl delif br0 eth7");
}

static int __curie2ru_bcm57412_init(struct curie2ru_bnxt *bnxt,
                                    struct curie2ru_bnxt_settings *settings)
{
    int rc;

    if ((rc = curie2ru_bnxt_init(bnxt, settings))) {
        log_err("curie2ru_bnxt_init failed\n");
        return rc;
    }

    return 0;
}

static void __curie2ru_bcm57412_exit(struct curie2ru_bnxt *bnxt)
{
    curie2ru_bnxt_exit(bnxt);
}

#define BCM57412_PCI_VENDOR  0x14e4
#define BCM57412_PCI_DEVICE  0x16d6

static int curie2ru_bcm57412_init(struct curie2ru *curie)
{
    struct curie2ru_bnxt_settings old_settings[NR_CURIE2RU_BNXT] = {
        {
            .pci_domain = 0,
            .pci_bus = 0x65,
            .pci_dev = 0,
            .pci_func = 0,
        },
        {
            .pci_domain = 0,
            .pci_bus = 0x66,
            .pci_dev = 0,
            .pci_func = 0,
        },
    };
    struct curie2ru_bnxt_settings new_settings[NR_CURIE2RU_BNXT] = {
        {
            .pci_domain = 0,
            .pci_bus = 0x95,
            .pci_dev = 0,
            .pci_func = 0,
        },
        {
            .pci_domain = 0,
            .pci_bus = 0x96,
            .pci_dev = 0,
            .pci_func = 0,
        },
    };
    struct curie2ru_bnxt_settings auto_settings[NR_CURIE2RU_BNXT];
    struct curie2ru_bnxt_settings *settings;
    struct pci_dev *pci = NULL;
    int rc;

    settings = &old_settings[0];
    pci = pci_dev_get_by_path(settings->pci_domain, settings->pci_bus,
                              settings->pci_dev, settings->pci_func);
    if (!pci || pci->vendor != BCM57412_PCI_VENDOR ||
        pci->device != BCM57412_PCI_DEVICE) {
        settings = &new_settings[0];
        pci_dev_put(pci);
        pci = pci_dev_get_by_path(settings->pci_domain, settings->pci_bus,
                                  settings->pci_dev, settings->pci_func);
    }

    if (!pci || pci->vendor != BCM57412_PCI_VENDOR ||
        pci->device != BCM57412_PCI_DEVICE) {
        settings = &auto_settings[0];
        pci_dev_put(pci);
        log_warn("can't find BCM57412 on expected bus, try to autodetect\n");
        pci = pci_dev_get_by_id(BCM57412_PCI_VENDOR, BCM57412_PCI_DEVICE, 1);
        if (!pci) {
            log_err("no BCM57412 found\n");
            return -1;
        }

        settings[0].pci_domain  = pci->domain;
        settings[0].pci_bus     = pci->bus;
        settings[0].pci_dev     = pci->dev;
        settings[0].pci_func    = pci->func;

        log_info("Found first BCM57412 at %04x:%02x:%02x.%d\n", pci->domain,
                 pci->bus, pci->dev, pci->func);

        pci_dev_put(pci);
        pci = pci_dev_get_by_id(BCM57412_PCI_VENDOR, BCM57412_PCI_DEVICE, 3);
        if (!pci) {
            log_err("missing second BCM57412\n");
            return -1;
        }

        settings[1].pci_domain  = pci->domain;
        settings[1].pci_bus     = pci->bus;
        settings[1].pci_dev     = pci->dev;
        settings[1].pci_func    = pci->func;

        log_info("Found second BCM57412 at %04x:%02x:%02x.%d\n", pci->domain,
                 pci->bus, pci->dev, pci->func);
    }

    pci_dev_put(pci);

    if ((rc = __curie2ru_bcm57412_init(&curie->bnxt[0], &settings[0])))
        return rc;
    if ((rc = __curie2ru_bcm57412_init(&curie->bnxt[1], &settings[1]))) {
        __curie2ru_bcm57412_exit(&curie->bnxt[0]);
        return rc;
    }

    return 0;
}

static void curie2ru_bcm57412_exit(struct curie2ru *curie)
{
    __curie2ru_bcm57412_exit(&curie->bnxt[1]);
    __curie2ru_bcm57412_exit(&curie->bnxt[0]);
}

static int curie2ru_bcm82757_mdio_read(void *ctx, uint8_t prtad, uint8_t devad,
                                       uint16_t addr, uint16_t *data)
{
    return curie2ru_bnxt_mdio_read(ctx, prtad, devad, addr, data);
}

static int curie2ru_bcm82757_mdio_write(void *ctx, uint8_t prtad, uint8_t devad,
                                        uint16_t addr, uint16_t data)
{
    return curie2ru_bnxt_mdio_write(ctx, prtad, devad, addr, data);
}

#define C2RU_BCM82757_MDIO_ADDR     0x10

static int curie2ru_bcm82757_init(struct curie2ru *curie)
{
    struct curie2ru_miura_settings settings = {
        .type  = "miura",
        .ctx   = &curie->bnxt[0],
        .read  = curie2ru_bcm82757_mdio_read,
        .write = curie2ru_bcm82757_mdio_write,
        .prtad = 0,
    };
    struct curie2ru_miura *miura = &curie->miura;
    int rc;

    if (!is_curie_2ru_p1a())
        settings.prtad = C2RU_BCM82757_MDIO_ADDR;

    /* miura sdk init */
    if ((rc = curie2ru_miura_init(miura, &settings))) {
        log_err("curie2ru_miura_init failed\n");
        return rc;
    }
    miura->info.lane_map = 0x3; /* lane 0,1 */

    return 0;
}

static int curie2ru_bcm82752_mdio_read(void *ctx, uint8_t mdio, uint8_t devad,
                                       uint16_t addr, uint16_t *data)
{
    return curie2ru_bnxt_mdio_read(ctx, mdio, devad, addr, data);
}

static int curie2ru_bcm82752_mdio_write(void *ctx, uint8_t mdio, uint8_t devad,
                                        uint16_t addr, uint16_t data)
{
    return curie2ru_bnxt_mdio_write(ctx, mdio, devad, addr, data);
}

int curie2ru_bcm82752_init(struct curie2ru *curie)
{
    struct curie2ru_quadra28_settings settings[] = {
        {
            .type  = "quadra28",
            .ctx   = &curie->bnxt[0],
            .read  = curie2ru_bcm82752_mdio_read,
            .write = curie2ru_bcm82752_mdio_write,
        },
        {
            .type  = "quadra28",
            .ctx   = &curie->bnxt[0],
            .read  = curie2ru_bcm82752_mdio_read,
            .write = curie2ru_bcm82752_mdio_write,
        }
    };
    struct curie2ru_quadra28 *q28 = curie->quadra28;
    int rc;

    if ((rc = curie2ru_quadra28_init(q28, settings))) {
        log_err("curie2ru_quadra28_init failed\n");
        return rc;
    }

    return 0;
}

static void curie2ru_bcm82757_exit(struct curie2ru *curie)
{
    struct curie2ru_miura *miura = &curie->miura;

    curie2ru_miura_exit(miura);
}

static void curie2ru_bcm82752_exit(struct curie2ru *curie)
{
    struct curie2ru_quadra28 *q28 = curie->quadra28;

    curie2ru_quadra28_exit(q28);
}

/* Curie 2RU platform initialization */
int curie2ru_init(struct curie2ru *curie)
{
    int rc;

    memset(curie, 0, sizeof(*curie));

    curie2ru_nim_eth_init();

    if ((rc = curie2ru_bcm57412_init(curie))) {
        log_err("curie2ru_bcm57412_init failed\n");
        return rc;
    }

    if (is_uranium()) {
        if ((rc = curie2ru_bcm82757_init(curie))) {
            log_err("curie2ru_bcm82757_init failed\n");
            return rc;
        }
    } else if (is_thorium()) {
        if ((rc = curie2ru_bcm82752_init(curie))) {
            log_err("curie2ru_bcm82752_init failed\n");
            return rc;
        }
    }

    return 0;
}

/* Curie 2RU platform exit */
void curie2ru_exit(struct curie2ru *curie)
{
    if (is_uranium())
        curie2ru_bcm82757_exit(curie);
    else if (is_thorium())
        curie2ru_bcm82752_exit(curie);
    curie2ru_bcm57412_exit(curie);
    curie2ru_nim_eth_exit();
}

static const reg_info_t curie2ru_bcm_82757_tsce_reg[] = {
    {"TSCE RX PCS X4 CTRL",
     BCMI_TSCE_XGXS_RX_X4_PCS_CTL0r, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE TX PCS X4 STATUS",
     BCMI_TSCE_XGXS_TX_X4_PCS_STSr, READ_ONLY, {2}, 0xFFFF, 0x0000},
    {"TSCE RX PMA X4 CTRL",
     BCMI_TSCE_XGXS_RX_X4_PMA_CTL0r, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE RX LATCH X4 STATUS 1",
     BCMI_TSCE_XGXS_RX_X4_PCS_LATCH_STS1r, READ_ONLY, {2}, 0xFFFF, 0x0000},
    {"TSCE RX LATCH X4 STATUS 0",
     BCMI_TSCE_XGXS_RX_X4_PCS_LATCH_STS0r, READ_ONLY, {2}, 0xFFFF, 0x0000},
    {"TSCE RX LIVE X4 STATUS",
     BCMI_TSCE_XGXS_RX_X4_PCS_LIVE_STSr, READ_ONLY, {2}, 0xFFFF, 0x0000},
    {"TSCE PMD X1 STATUS",
     BCMI_TSCE_XGXS_PMD_X1_STSr, READ_ONLY, {2}, 0xFFFF, 0x0000},
    {"TSCE PMD X4 CTL",
     BCMI_TSCE_XGXS_PMD_X4_CTLr, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE PMD X4 MODE",
     BCMI_TSCE_XGXS_PMD_X4_MODEr, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE PMD X4 STATUS",
     BCMI_TSCE_XGXS_PMD_X4_STSr, READ_ONLY, {2}, 0xFFFF, 0x0000},
    {"TSCE PKTGEN CRCERRCNTr",
     BCMI_TSCE_XGXS_PKTGEN_CRCERRCNTr, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE PKTGEN TXPKTCNT Ur",
     BCMI_TSCE_XGXS_PATGEN_TXPKTCNT_Ur, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE PKTGEN TXPKTCNT Lr",
     BCMI_TSCE_XGXS_PATGEN_TXPKTCNT_Lr, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE PKTGEN RXPKTCNT Ur",
     BCMI_TSCE_XGXS_PATGEN_RXPKTCNT_Ur, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"TSCE PKTGEN RXPKTCNT Lr",
     BCMI_TSCE_XGXS_PATGEN_RXPKTCNT_Lr, READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const curie2ru_bcm_phy_regs_t curie2ru_bcm82757_phy_reg_tbl[] = {
    {"TSEC PCS", CURIE2RU_MIURA_DEV_PCS, curie2ru_bcm_82757_tsce_reg},
};

#define CURIE2RU_BCM82757_NUM_PHY_INTF (sizeof(curie2ru_bcm82757_phy_reg_tbl) /      \
                               sizeof(struct curie2ru_bcm_phy_regs_t_))

/* wapper for BCM82757 register read */
int curie2ru_bcm82757_read(struct curie2ru *curie,
                           curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                           uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_reg_value_get(miura->type, *info, devaddr, regaddr, data);
}

/* wapper for BCM82757 register write */
int curie2ru_bcm82757_write(struct curie2ru *curie,
                            curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                            uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_reg_value_set(miura->type, *info, devaddr, regaddr, data);
}

/* wapper for BCM82757 status dump */
int curie2ru_bcm82757_dump(struct curie2ru *curie,
                           curie2ru_lane_t lane, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_phy_status_dump(miura->type, *info);
}

/* dump all registers in a register table */
static void curie2ru_bcm82757_regs_show(struct curie2ru *curie, curie2ru_lane_t lane, curie2ru_if_side_t if_side, const curie2ru_bcm_phy_regs_t *phy_reg_ptr)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int rdval;
    const reg_info_t *reg_ptr;
    int dev_id, reg_offset;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    reg_ptr = phy_reg_ptr->intfregs;
    dev_id = phy_reg_ptr->phy_intf;
    while (reg_ptr->size.size != 0) {
        reg_offset = reg_ptr->offset;
        if (bcm_plp_reg_value_get(miura->type, *info, dev_id, reg_offset, &rdval)) {
            prt("WORN: bcm82757 reg value get failed\n");
        }
        printf("%s : %-32s reg %d.%#.8x = %#.4x\n", phy_reg_ptr->intfname, reg_ptr->name,
                dev_id, reg_offset, rdval);
        reg_ptr++;
        curie2ru_mdelay(10);
    }
}

/* dump all registers in register tables */
int curie2ru_bcm82757_regs_dump(struct curie2ru *curie, curie2ru_lane_t lane)
{
    unsigned int ix, intf_move;
    const curie2ru_bcm_phy_regs_t *phy_reg_ptr;
    curie2ru_if_side_t if_side;

    intf_move = CURIE2RU_BCM82757_NUM_PHY_INTF;

    /* dump all page */
    prt("***************line side********************\n");
    phy_reg_ptr = &curie2ru_bcm82757_phy_reg_tbl[0];
    if_side = CURIE2RU_IF_SIDE_LINE;
    for (ix = 0; ix < intf_move; ix++) {
        curie2ru_bcm82757_regs_show(curie, lane, if_side, phy_reg_ptr);
        phy_reg_ptr++;
    }

    prt("***************sys side********************\n");
    phy_reg_ptr = &curie2ru_bcm82757_phy_reg_tbl[0];
    if_side = CURIE2RU_IF_SIDE_SYS;
    for (ix = 0; ix < intf_move; ix++) {
        curie2ru_bcm82757_regs_show(curie, lane, if_side, phy_reg_ptr);
        phy_reg_ptr++;
    }

    return 0;
}

/* wrapper for MAC diagnostic dump */
int curie2ru_bcm82757_mac_dump(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_mac_diagnostic_dump(miura->type, miura->mac_info);
}

/* wrapper for bcm_plp_link_status_get */
int curie2ru_bcm82757_link_status(struct curie2ru *curie,
                                  curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                  unsigned int *link_status)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_link_status_get(miura->type, *info, link_status);
}

/* wrapper for eye diagram */
int curie2ru_bcm82757_display_eye_scan(struct curie2ru *curie,
                                       curie2ru_lane_t lane, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_display_eye_scan(miura->type, *info);
}

int curie2ru_bcm82757_rx_get(struct curie2ru *curie,
                             curie2ru_lane_t lane, bcm_plp_rx_t* rx, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_rx_get(miura->type, *info, rx);
}

int curie2ru_bcm82757_rx_set(struct curie2ru *curie,
                             curie2ru_lane_t lane, bcm_plp_rx_t rx, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_rx_set(miura->type, *info, &rx);
}

int curie2ru_bcm82757_loopback_set(struct curie2ru *curie,
                                   curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                   unsigned int lb_mode, unsigned int enable)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_loopback_set(miura->type, *info, lb_mode, enable);
}

static int force_intf_lrm_flag = 0;
static bcm_plp_pm_interface_t line_side_intf_type = bcm_pm_InterfaceSFI;

int curie2ru_bcm82757_prbs_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               curie2ru_prbs_t prbs, unsigned int enable)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int poly;
    int rc;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);

    switch (prbs) {
    case CURIE2RU_PRBS_7:
        poly = 0;
        break;
    case CURIE2RU_PRBS_9:
        poly = 1;
        break;
    case CURIE2RU_PRBS_11:
        poly = 2;
        break;
    case CURIE2RU_PRBS_15:
        poly = 3;
        break;
    case CURIE2RU_PRBS_23:
        poly = 4;
        break;
    default:
    case CURIE2RU_PRBS_31:
        poly = 5;
        break;
    }

    if (enable && line_side_intf_type == bcm_pm_InterfaceLRM) {
        /* LRM, need to write 0xb into 0x4800d0d1 */
        rc = bcm_plp_reg_value_set(miura->type, *info, CURIE2RU_MIURA_DEV_PCS, 0x4800d0d1, 0xb);
        if (rc) {
            log_err("bcm_plp_set_value failed\n");
            return rc;
        }
    }

    rc = bcm_plp_prbs_set(miura->type, *info, 0, poly, 0, 0, enable);
    if (!rc && !enable)
        rc = bcm_plp_prbs_clear(miura->type, *info, 0);
    return rc;
}

int curie2ru_bcm82757_prbs_clear_rx_stat(struct curie2ru *curie, curie2ru_lane_t lane, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);

    if ((rc = bcm_plp_prbs_status_get(miura->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }
    return 0;
}

int curie2ru_bcm82757_prbs_check(struct curie2ru *curie,
                                 curie2ru_lane_t lane, curie2ru_if_side_t if_side)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);

    if ((rc = bcm_plp_prbs_status_get(miura->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }

    if (prbs_lock) {
        prt("prbs locked\n");
        prt("prbs error count: %d\n", error_count);
    } else {
        prt("prbs unlock\n");
    }
    if (prbs_lock_loss)
        prt("prbs lock loss\n");
    return (!prbs_lock || prbs_lock_loss || error_count) ? -1 : 0;
}

int curie2ru_bcm82757_firmware_lane_set(struct curie2ru *curie,
                                        curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_firmware_lane_config_set(miura->type,*info ,firmware_lane_config);
}

int curie2ru_bcm82757_firmware_lane_get(struct curie2ru *curie,
                                        curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;

    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_firmware_lane_config_get(miura->type,*info ,firmware_lane_config);
}

static int __curie2ru_bcm82757_cl73_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               unsigned int enable, int cl37_en)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *info = &miura->info;
    int rc;
    unsigned short tech_ability = 5;
    unsigned short fec_ability = 0;
    unsigned short pause_ability = 0;
    bcm_plp_an_config_t an_config = {
        .cl72_en = 1,
        .tech_ability = 5,
    };

    if (cl37_en) {
        tech_ability = 5;
        an_config.cl72_en = 0;
        an_config.tech_ability = 5;
        /* cl37 enable */
        info->flags = 1<<1;
    }
    info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);

    rc = bcm_plp_cl73_ability_set(miura->type, *info, tech_ability,
                                  fec_ability, pause_ability, an_config);
    if (rc) {
        log_err("bcm_plp_cl73_ability_set failed, return code [%d]\n", rc);
        return rc;
    }

    rc = bcm_plp_cl73_set(miura->type, *info, enable);
    if (rc) {
        log_err("bcm_plp_cl73_set failed, return code [%d]\n", rc);
        return rc;
    }

    return 0;
}

int curie2ru_bcm82757_cl73_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               unsigned int enable)
{
    return __curie2ru_bcm82757_cl73_set(curie, lane, if_side, enable, 0);
}

int curie2ru_bcm82757_cl37_set(struct curie2ru *curie,
                               curie2ru_lane_t lane, curie2ru_if_side_t if_side,
                               unsigned int enable)
{
    return __curie2ru_bcm82757_cl73_set(curie, lane, if_side, enable, 1);
}

void curie2ru_bcm82757_config_macsec_cleanup(struct curie2ru *curie)
{
    curie2ru_bcm82757_macsec_exit(curie);
}

typedef struct curie2ru_line_intf_type {
    char *type_name;
    bcm_pm_interface_t type;
} curie2ru_line_intf_type_t;

static const curie2ru_line_intf_type_t curie2ru_bcm82757_line_intf_table[] = {
    {"10G Base-SR", bcm_pm_InterfaceSR},
    {"10G Base-LRM", bcm_pm_InterfaceLRM},
};

#define CURIE2RU_BCM82757_NUM_LINE_INTF_TYPE (sizeof(curie2ru_bcm82757_line_intf_table) /      \
                               sizeof(struct curie2ru_line_intf_type))

void force_line_side_intf_lrm(int force)
{
    if (force) {
        line_side_intf_type = bcm_pm_InterfaceLRM;
        force_intf_lrm_flag = 1;
    } else {
        force_intf_lrm_flag = 0;
    }
}

void curie2ru_bcm82757_line_side_interface_set(char *type)
{
    int i;
    const curie2ru_line_intf_type_t *line_intf;
    line_intf = &curie2ru_bcm82757_line_intf_table[0];

    if (force_intf_lrm_flag) {
        return ;
    }
    for (i = 0; i < CURIE2RU_BCM82757_NUM_LINE_INTF_TYPE; i++) {
        if (!strncmp(type, line_intf->type_name, strlen(line_intf->type_name))) {
            line_side_intf_type = line_intf->type;
            break;
        }
        line_intf++;
    }
    if (i == CURIE2RU_BCM82757_NUM_LINE_INTF_TYPE) {
        line_side_intf_type = bcm_pm_InterfaceSFI;
    }
}

int curie2ru_bcm82757_line_side_interface_lrm(void)
{
    return line_side_intf_type == bcm_pm_InterfaceLRM;
}

/* core API to set mode with MACSEC bypassed */
int curie2ru_bcm82757_set_macsec_bypass_mode(struct curie2ru *curie, curie2ru_lane_t lane, int speed)
{
    struct curie2ru_miura *miura = &curie->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    bcm_plp_sec_phy_access_t sec_info;
    bcm_plp_pm_interface_t line_side_interface = line_side_intf_type;
    int aux = 0;
    int rc;

    if (speed == CURIE2RU_PORT_SPEED_1G) {
        line_side_interface = bcm_pm_Interface1000X;
    }
    memset(&sec_info, 0, sizeof(sec_info));
    /* Set Secy Config set for Both Egress and Ingress */
    plp_info->lane_map = CURIE2RU_LANE_TO_MIURA(lane);
    plp_info->if_side = CURIE2RU_MIURA_LINE_SIDE;
    memcpy(&sec_info.phy_info, plp_info, sizeof(bcm_plp_access_t));
    sec_info.macsec_side = CURIE2RU_MIURA_EGRESS;

    /* Secy Config set */
    rc = bcm_plp_secy_config_set(miura->type, &sec_info);
    if (rc) {
        log_err("bcm_plp_secy_config_set failed for device-id [%d], "
                "return code [%d]\n", sec_info.macsec_side, rc);
        return rc;
    }

    /* Mode Config set for each port */
    plp_info->if_side = CURIE2RU_MIURA_LINE_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 line_side_interface, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }

    plp_info->if_side = CURIE2RU_MIURA_SYS_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 bcm_pm_InterfaceXFI, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }
    /* Ingress, Set SECY to Bypass */
    sec_info.macsec_side = CURIE2RU_MIURA_INGRESS;
    rc = bcm_plp_secy_bypass_set(miura->type, &sec_info, 1);
    if (rc) {
        log_err("bcm_plp_secy_bypass_set failed for PHY-ID [%d], "
                "macsec_side [%d], return code [%d]\n",
                plp_info->phy_addr, sec_info.macsec_side, rc);
        return rc;
    }

    return rc;
}

int curie2ru_bcm82757_macsec_init(struct curie2ru *curie, int bypass)
{
    struct curie2ru_miura *miura = &curie->miura;
    return curie2ru_miura_macsec_init(miura, bypass);
}

void curie2ru_bcm82757_macsec_exit(struct curie2ru *curie) {
    struct curie2ru_miura *miura = &curie->miura;

    curie2ru_miura_macsec_exit(miura);
}

int curie2ru_bcm82757_config_macsec_bypass(struct curie2ru *curie, curie2ru_lane_t lane, int speed)
{
    int rc;

    if ((rc = curie2ru_bcm82757_macsec_init(curie, 1)))
        return rc;

    rc = curie2ru_bcm82757_set_macsec_bypass_mode(curie, lane, speed);
    return rc;
}

extern int bcm_base_t_autoneg_ability_set(bcm_plp_access_t phy_info,
                                          bcm_plp_base_t_ability_t *ability);
extern int bcm_base_t_autoneg_ability_get(bcm_plp_access_t phy_info,
                                          bcm_plp_base_t_ability_t *ability);
extern int bcm_base_t_autoneg_set(bcm_plp_access_t phy_info, int  enable);
extern int bcm_base_t_autoneg_get(bcm_plp_access_t phy_info, int *enable, int *an_done);

/*
 * The register settings for the 1G ref clock retimer mode (for each channel)
 * are as follows:
 * 1. Write 1.C8D8.7 = 0x0.
 * 2. Read 1.C843 (make sure it reads bit 7 of 1.C843 equal to 0).
 * 3. Write 1.C8D9.4 = 0x1 (ref clock mode).
 * 4. Write 1.C8D8 = 0x0081.
 * 5. Read 1.C843 (to make sure it reads the same value as 1.C8D8).
 * 6. Write 1.0000.15 = 0x1 (IEEE reset, self-clearing bit).
 * 7. Wait 500 ms (read back 1.0000.15 to make sure it is clear).
 *
 * The register settings for the 1G recovered clock retimer mode (for each channel)
 * are as follows:
 * 1. Write 1.C8D8.7 = 0x0.
 * 2. Read 1.C843 (make sure it reads bit 7 of 1.C843 equal to 0).
 * 3. Write 1.C8D9.4 = 0x0 (recovered clock mode).
 * 4. Write 1.C8D8 = 0x0081.
 * 5. Read 1.C843 (to make sure it reads the same value as 1.C8D8).
 * 6. Write 1.0000.15 = 0x1 (IEEE reset, self-clearing bit).
 * 7. Wait 500 ms (read back 1.0000.15 to make sure it is clear).
 */

/*
 * The Clause 37 (auto-neg) is only supported in 1G REF_CLK_Retimer mode.
 * Try the below steps for 1G REF_CLK_Retimer (aka retimer) mode settings with
 * CL-37 (auto-neg) to each channel.
 * a. Write 1.C8d8.7 = 0x0
 * b. Read 1.C843 until it is equal to 1.c8d8
 * c. Write 1.C8d9.4 = 0x1 (retime)
 * d. Write 1.C8d8 = 0x0081 (1G setting)
 * e. Read 1.C843 until it is equal to 1.c8d8
 * f. Write 1.0000.15 = 0x1 (ieee reset, self-clearing bit.)
 * g. Wait 500ms
 * h. Write 1.C8E5.0 = 0x0 (when 1G fiber module used)
 * Set the following register in line side for AN (Clause - 37)
 * i. Write 1.0009.0 = 0x1 (Disable Tx)
 * j. Write 7.ffe0.12 = 0x1 (AN Enable)
 * k. Write 1.0009.0 = 0x0 (Enable Tx)
 * l. Check 7.ffe1.2 = 0x1 (link status) [read twice]
 * m. Check 7.ffe1.5 = 0x1 (AN complete
 */

static const reg_info_t curie2ru_bcm_82752_pma_pmd_reg[] = {
    {"PMD Control", BCMI_QUADRA28_PMD_CONTROLr, READ_WRITE, {2}, 0x0000, 0x2040},
    {"PMD Status", BCMI_QUADRA28_STATUSr, READ_ONLY,  {2}, 0x0000, 0x0080},
    {"PMD PHY ID 0", BCMI_QUADRA28_PMD_IDENTIFIER_0r, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PMD PHY ID 1", BCMI_QUADRA28_PMD_IDENTIFIER_1r, READ_ONLY,  {2}, 0x0000, 0x5250},
    {"PMD Speed Ability", BCMI_QUADRA28_PMD_SPEED_ABILITYr, READ_ONLY, {2}, 0x0000, 0x0011},
    {"PMD Devices in Package 1", BCMI_QUADRA28_DEVICES_IN_PACKAGE_1r, READ_ONLY,  {2}, 0x0000, 0x008A},
    {"PMD Devices in Package 2", BCMI_QUADRA28_DEVICES_IN_PACKAGE_2r, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Control 2", BCMI_QUADRA28_PMD_CONTROL_2r, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"PMD Status 2", BCMI_QUADRA28_STATUS_2r, READ_ONLY, {2}, 0x0000, 0xBFE1},
    {"PMD Transmit Disable", BCMI_QUADRA28_PMD_TRANSMIT_DISABLEr, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Receive Signal Detect", BCMI_QUADRA28_PMD_RECEIVE_SIGNAL_DETECTr, READ_ONLY,{2}, 0x0000, 0x0000},
    {"PMD Extended Ability", BCMI_QUADRA28_PMD_EXTENDED_ABILITYr, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PMD Organizationally Unique ID 0", BCMI_QUADRA28_PMD_OUI_ID0r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PMD Organizationally Unique ID 1", BCMI_QUADRA28_PMD_OUI_ID1r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const reg_info_t curie2ru_bcm_82752_dev_pcs_reg[] = {
    {"PCS Control 1", BCMI_QUADRA28_PCS_CONTROLr, READ_WRITE, {2}, 0x0000, 0x2040},
    {"PCS Status 1", BCMI_QUADRA28_PCS_STATUS1r, READ_ONLY,  {2}, 0x0000, 0x8020},
    {"PCS PHY ID 0", BCMI_QUADRA28_PCS_IDENTIFIER_0r, READ_ONLY,  {2}, 0x0000, 0xAE02},
    {"PCS PHY ID 1", BCMI_QUADRA28_PCS_IDENTIFIER_1r, READ_ONLY,  {2}, 0x0000, 0x5250},
    {"PCS Speed Ability", BCMI_QUADRA28_SPEED_ABILITYr, READ_ONLY, {2}, 0x0000, 0x0001},
    {"PCS Devices in Package 1", BCMI_QUADRA28_DEVICES_IN_PACKAGE1r, READ_ONLY,  {2}, 0x0000, 0x0005},
    {"PCS Control 2", BCMI_QUADRA28_PCS_CONTROL_2r, READ_WRITE, {2}, 0x0003, 0x0000},
    {"PCS Status 2", BCMI_QUADRA28_PCS_STATUS_2r, READ_ONLY,  {2}, 0x0000, 0x8401},
    {"PCS EEE CAPABILITY", BCMI_QUADRA28_PCS_EEE_CAPABILITYr, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R WAKE ERR CNT", BCMI_QUADRA28_TENGBASE_R_PCS_WAKE_ERROR_COUNTER_r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-X STATUS", BCMI_QUADRA28_TENGBASE_X_PCS_STATUSr, READ_ONLY, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R STATUS", BCMI_QUADRA28_TENGBASE_R_PCS_STATUSr, READ_ONLY, {2}, 0x0000, 0x000C},
    {"PCS 10GBASE-R STATUS 2", BCMI_QUADRA28_TENGBASE_R_PCS_STATUS_2r, READ_ONLY,  {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A0", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_A0_r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A1", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_A1r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A2", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_A2r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed A3", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_A3r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B0", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_B0r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B1", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_B1r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B2", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_B2r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Seed B3", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_SEED_B3r, READ_WRITE, {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Test Control", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_CONTROLr, READ_WRITE,  {2}, 0x0000, 0x0000},
    {"PCS 10GBASE-R Jitter Test Error Count", BCMI_QUADRA28_TENGBASE_R_PCS_JITTER_TEST_ERROR_COUNTERr, READ_ONLY, {2}, 0x0000, 0x0000},
    {"end", 0x00, 0, {0}, 0, 0},
};

static const curie2ru_bcm_phy_regs_t curie2ru_bcm82752_phy_standard_reg_tbl[] = {
    {"DEV_PMA_PMD", CURIE2RU_QUADRA28_DEV_PMA_PMD, curie2ru_bcm_82752_pma_pmd_reg},
    {"DEV_PCS", CURIE2RU_QUADRA28_DEV_PCS, curie2ru_bcm_82752_dev_pcs_reg},
};

#define CURIE2RU_BCM82752_NUM_PHY_INTF (sizeof(curie2ru_bcm82752_phy_standard_reg_tbl) /      \
                               sizeof(struct curie2ru_bcm_phy_regs_t_))

static void curie2ru_bcm82752_regs_show(struct curie2ru *curie, int port, curie2ru_if_side_t if_side, const curie2ru_bcm_phy_regs_t *phy_reg_ptr)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int rdval;
    const reg_info_t *reg_ptr;
    int dev_id, reg_offset;

    dev_id = phy_reg_ptr->phy_intf;
    reg_ptr = phy_reg_ptr->intfregs;
    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    while (reg_ptr->size.size != 0) {
        reg_offset = (reg_ptr->offset) & 0xFFFF;
        bcm_plp_reg_value_get(q28->type, *info, dev_id, reg_offset, &rdval);
        printf("%s : %-32s reg %d.%#.4x = %#.4x\n", phy_reg_ptr->intfname, reg_ptr->name,
                dev_id, reg_offset, rdval);
        reg_ptr++;
        curie2ru_mdelay(10);
    }
}

int curie2ru_bcm82752_regs_dump(struct curie2ru *curie, int port)
{
    uint ix, intf_move;
    const curie2ru_bcm_phy_regs_t *phy_reg_ptr;
    curie2ru_if_side_t if_side;

    phy_reg_ptr = &curie2ru_bcm82752_phy_standard_reg_tbl[0];
    intf_move = CURIE2RU_BCM82752_NUM_PHY_INTF;

    /* dump all page */
    prt("***************line side********************\n");
    phy_reg_ptr = &curie2ru_bcm82752_phy_standard_reg_tbl[0];
    if_side = CURIE2RU_IF_SIDE_LINE;
    for (ix = 0; ix < intf_move; ix++) {
        curie2ru_bcm82752_regs_show(curie, port, if_side, phy_reg_ptr);
        phy_reg_ptr++;
    }

    prt("***************sys side********************\n");
    phy_reg_ptr = &curie2ru_bcm82752_phy_standard_reg_tbl[0];
    if_side = CURIE2RU_IF_SIDE_LINE;
    for (ix = 0; ix < intf_move; ix++) {
        curie2ru_bcm82752_regs_show(curie, port, if_side, phy_reg_ptr);
        phy_reg_ptr++;
    }

    return 0;
}

static int bcm82752_mode_clear_check(struct curie2ru_quadra28 *q28)
{
    int timer_count = 200;
    bcm_plp_access_t *info = &q28->info;

    do {
        uint32_t data;
        bcm_plp_reg_value_get(q28->type, *info, 1, 0xC843, &data);
        if (!(data & 0x80))
            break;
        usleep(5000);
        timer_count--;
    } while (timer_count > 0);

    if (timer_count <= 0) {
        printf("failed to clear mode\n");
        return -1;
    }

    return 0;
}

static int bcm82752_mode_check(struct curie2ru_quadra28 *q28, uint16_t mode)
{
    int timer_count = 200;
    bcm_plp_access_t *info = &q28->info;

    do {
        uint32_t data;
        bcm_plp_reg_value_get(q28->type, *info, 1, 0xC843, &data);
        if (data == mode)
            break;
        usleep(5000);
        timer_count--;
    } while (timer_count > 0);

    if (timer_count <= 0) {
        printf("failed to set mode %04x\n", mode);
        return -1;
    }

    return 0;
}

int curie2ru_bcm82752_set_cl37_an(struct curie2ru *curie, int port, int enable)
{
    uint32_t data;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0009, &data);
    data |= 1;
    bcm_plp_reg_value_set(q28->type, *info, 1, 0x0009, data);

    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe0, &data);
    if (enable)
        data |= (1 << 12);
    else
        data &= ~(1 << 12);
    bcm_plp_reg_value_set(q28->type, *info, 7, 0xffe0, data);

    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0009, &data);
    data &= ~1;
    bcm_plp_reg_value_set(q28->type, *info, 1, 0x0009, data);

    return 0;
}

int curie2ru_bcm82752_check_cl37_an(struct curie2ru *curie, int port, int *an, int *link, int *done)
{
    uint32_t data;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe0, &data);
    *an = (data & 0x1000) ? 1 : 0;

    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe1, &data);
    bcm_plp_reg_value_get(q28->type, *info, 7, 0xffe1, &data);

    *link = (data & 0x0004) ? 1 : 0;
    *done = (data & 0x0020) ? 1 : 0;

    return 0;
}

int curie2ru_bcm82752_1g_config(struct curie2ru *curie, int port, int recovered)
{
    uint32_t data;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0xC8D8, &data);
	data &= ~(1 << 7);
    bcm_plp_reg_value_set(q28->type, *info, 1, 0xC8D8, data);

    if (bcm82752_mode_clear_check(q28) < 0)
        return -1;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0xC8D9, &data);
    if (recovered)
        data &= ~(1 << 4);
    else
        data |= (1 << 4);
    bcm_plp_reg_value_set(q28->type, *info, 1, 0xC8D9, data);

    bcm_plp_reg_value_set(q28->type, *info, 1, 0xC8D8, 0x0081);

    if (bcm82752_mode_check(q28, 0x0081) < 0)
        return -1;

    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0000, &data);
    data |= (1 << 15);
    bcm_plp_reg_value_set(q28->type, *info, 1, 0x0000, data);

    usleep(500 * 1000);
    bcm_plp_reg_value_get(q28->type, *info, 1, 0x0000, &data);

    if (data & (1 << 15)) {
        printf("error: 1.0000.15 not self-cleard\n");
        return -1;
    }

	return 0;
}

int curie2ru_bcm82752_10g_config(struct curie2ru *curie, int port)
{
    int s_if_type = bcm_pm_InterfaceXFI;
    int if_type = bcm_pm_InterfaceSR;
    int ref_clk = bcm_pm_RefClk156Mhz;
    int if_mode = 0;
    bcm_plp_device_aux_modes_t s_aux_mode;
    bcm_plp_device_aux_modes_t aux_mode;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    int phy_id = info->phy_addr;
    int rv, sp = 0, intf = 0, ref = 0, mode = 0;
    int repeater = 0;
    int speed = CURIE2RU_PORT_SPEED_10G;

    rv = bcm_plp_repeater_mode_set(q28->type, *info, repeater);
    if (rv != 0) {
       printf("bcm_plp_repeater_mode_set failed rv=%d \n",rv);
       return rv;
    }

    printf("sys interface %d, line interface %d, repeater %d\n", s_if_type,
           if_type, repeater);

    memset(&s_aux_mode, 0, sizeof(bcm_plp_device_aux_modes_t));
    memset(&aux_mode, 0, sizeof(bcm_plp_device_aux_modes_t));
    s_aux_mode.pass_thru = 1;

    info->if_side = CURIE2RU_QUADRA28_SYS_SIDE;
    rv = bcm_plp_mode_config_set(q28->type, *info, speed,
                                 s_if_type, ref_clk, if_mode,
                                 (void*)&s_aux_mode);
    if (rv != 0) {
       printf("bcm_plp_mode_config_set failed with rv:%d port:0x%x\n", rv, port);
       return rv;
    }
    printf("Mode config set success for phy_id = %d, Interface = %d, speed = %d"
           " if_mode:0x%x ref_clk:%d\n",
           phy_id, s_if_type, speed, if_mode, ref_clk);
    rv = bcm_plp_mode_config_get(q28->type, *info, &sp,
                                 &intf, &ref, &mode, (void*)&aux_mode);
    if (rv != 0) {
       printf("bcm_plp_mode_config_get failed with rv:%d port:0x%x\n", rv, port);
       return rv;
    }
    printf("Mode config get success for phy_id = %d, Interface = %d, speed = %d"
           " if_mode:0x%x ref_clk:%d\n", phy_id, intf, sp, mode, ref);

    if((speed == sp) && (s_if_type == intf) && (ref_clk == ref)){
       printf("PASSED : Mode config set successfully\n");
    } else {
       printf("FAIL : Mode config set fail\n");
       return -1;
    }

    info->if_side = CURIE2RU_QUADRA28_LINE_SIDE;
    rv = bcm_plp_mode_config_set(q28->type, *info, speed, if_type,
                                 ref_clk, if_mode, (void*)&s_aux_mode);
    if (rv != 0) {
        printf("bcm_plp_mode_config_set failed with rv:%d port:0x%x\n", rv, port);
        return rv;
    }
    printf("Mode config set success for phy_id = %d, Interface = %d, speed = %d"
            " if_mode:0x%x ref_clk:%d\n",
            phy_id, if_type, speed, if_mode, ref_clk);
    rv = bcm_plp_mode_config_get(q28->type, *info, &sp, &intf, &ref, &mode,
                                 (void*)&aux_mode);
    if (rv != 0) {
        printf("bcm_plp_mode_config_get failed with rv:%d port:0x%x\n", rv, port);
        return rv;
    }
    printf("Mode config get success for phy_id = %d, Interface = %d, speed = %d"
           " if_mode:0x%x ref_clk:%d\n", phy_id, intf, sp, mode, ref);

    if ((speed == sp) && (if_type == intf) && (ref_clk == ref)) {
        printf("PASSED : Mode config set successfully\n");
    } else {
        printf("FAIL : Mode config set fail\n");
        return -1;
    }

    return 0;
}

int curie2ru_bcm82752_mode_config(struct curie2ru *curie, int port, int speed)
{
    if (speed == CURIE2RU_PORT_SPEED_1G)
        return curie2ru_bcm82752_1g_config(curie, port, 0);
    else
        return curie2ru_bcm82752_10g_config(curie, port);
}

/* wrapper for BCM82752 register read */
int curie2ru_bcm82752_read(struct curie2ru *curie,
                           int port, curie2ru_if_side_t if_side,
                           uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);

    return bcmphy_quadra28_reg_read(q28, info->if_side, q28->id,
                                    info->lane_map, devaddr, &regaddr, data, 1);
}

/* wrapper for BCM82752 register write */
int curie2ru_bcm82752_write(struct curie2ru *curie,
                            int port, curie2ru_if_side_t if_side,
                            uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);

    return bcmphy_quadra28_reg_write(q28, info->if_side, q28->id,
                                     info->lane_map, devaddr, &regaddr, &data, 1);
}

int curie2ru_bcm82752_dump(struct curie2ru *curie,
                           int port, curie2ru_if_side_t if_side)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_phy_status_dump(q28->type, *info);
}

int curie2ru_bcm82752_phy_dump(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side)
{
    int rv;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    bcm_plp_pm_phy_diagnostics_t diag;

    memset(&diag,0,sizeof(bcm_plp_pm_phy_diagnostics_t));
    info->if_side = CURIE2RU_IF_SIDE_TO_MIURA(if_side);
    rv = bcm_plp_phy_diagnostics_get(q28->type, *info, &diag);
    if (rv != 0) {
        printf("bcm_plp_phy_diagnostics_get failed with rv:%d"
               " phy_info.lane_map: 0x%x\n", rv, info->lane_map);
        return rv;
    }
    printf("==DSC phy_info.if_side %d phy_info.lane_map 0x%x==\n",
           info->if_side, info->lane_map);
    printf("signal_detect             = \t0x%x\n", diag.signal_detect);
    printf("vga_bias_reduced          = \t0x%x\n", diag.vga_bias_reduced);
    printf("postc_metric              = \t0x%x\n", diag.postc_metric);
    printf("osr_mode                  = \t0x%x\n", diag.osr_mode);
    printf("pmd_mode                  = \t0x%x\n", diag.rx_lock);
    printf("rx_ppm                    = \t0x%x\n", diag.rx_ppm);
    printf("tx_ppm                    = \t0x%x\n", diag.tx_ppm);
    printf("clk90_offset              = \t0x%x\n", diag.clk90_offset);
    printf("clkp1_offset              = \t0x%x\n", diag.clkp1_offset);
    printf("p1_lvl                    = \t0x%x\n", diag.p1_lvl);
    printf("m1_lvl                    = \t0x%x\n", diag.m1_lvl);
    printf("dfe1_dcd                  = \t0x%x\n", diag.dfe1_dcd);
    printf("dfe2_dcd                  = \t0x%x\n", diag.dfe2_dcd);
    printf("slicer_target             = \t0x%x\n", diag.slicer_target);
    printf("slicer_offset:offset_pe   = \t0x%x\n", diag.slicer_offset.offset_pe);
    printf("slicer_offset:offset_ze   = \t0x%x\n", diag.slicer_offset.offset_ze);
    printf("slicer_offset:offset_me   = \t0x%x\n", diag.slicer_offset.offset_me);
    printf("slicer_offset:offset_po   = \t0x%x\n", diag.slicer_offset.offset_po);
    printf("slicer_offset:offset_zo   = \t0x%x\n", diag.slicer_offset.offset_zo);
    printf("slicer_offset:offset_mo   = \t0x%x\n", diag.slicer_offset.offset_mo);
    printf("eyescan:heye_left         = \t0x%x\n", diag.eyescan.heye_left);
    printf("eyescan:heye_right        = \t0x%x\n", diag.eyescan.heye_right);
    printf("eyescan:veye_upper        = \t0x%x\n", diag.eyescan.veye_upper);
    printf("eyescan:veye_lower        = \t0x%x\n", diag.eyescan.veye_lower);
    printf("state_machine_status      = \t0x%x\n", diag.state_machine_status);
    printf("link_time                 = \t0x%x\n", diag.link_time);
    printf("pf_main                   = \t0x%x\n", diag.pf_main);
    printf("pf_hiz                    = \t0x%x\n", diag.pf_hiz);
    printf("pf_bst                    = \t0x%x\n", diag.pf_bst);
    printf("pf_low                    = \t0x%x\n", diag.pf_low);
    printf("pf2_ctrl                  = \t0x%x\n", diag.pf2_ctrl);
    printf("vga                       = \t0x%x\n", diag.vga);
    printf("dc_offset                 = \t0x%x\n", diag.dc_offset);
    printf("p1_lvl_ctrl               = \t0x%x\n", diag.p1_lvl_ctrl);
    printf("dfe1                      = \t0x%x\n", diag.dfe1);
    printf("dfe2                      = \t0x%x\n", diag.dfe2);
    printf("dfe3                      = \t0x%x\n", diag.dfe3);
    printf("dfe4                      = \t0x%x\n", diag.dfe4);
    printf("dfe5                      = \t0x%x\n", diag.dfe5);
    printf("dfe6                      = \t0x%x\n", diag.dfe6);
    printf("txfir_pre                 = \t0x%x\n", diag.txfir_pre);
    printf("txfir_main                = \t0x%x\n", diag.txfir_main);
    printf("txfir_post1               = \t0x%x\n", diag.txfir_post1);
    printf("txfir_post2               = \t0x%x\n", diag.txfir_post2);
    printf("txfir_post3               = \t0x%x\n", diag.txfir_post3);
    printf("tx_amp_ctrl               = \t0x%x\n", diag.tx_amp_ctrl);
    printf("br_pd_en                  = \t0x%x\n", diag.br_pd_en);

    return 0;
}

int curie2ru_bcm82752_link_status(struct curie2ru *curie,
                                  int port, curie2ru_if_side_t if_side,
                                  unsigned int *link_status)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_link_status_get(q28->type, *info, link_status);
}

int curie2ru_bcm82752_display_eye_scan(struct curie2ru *curie,
                                       int port, curie2ru_if_side_t if_side)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_display_eye_scan(q28->type, *info);
}

int curie2ru_bcm82752_loopback_set(struct curie2ru *curie,
                                   int port, curie2ru_if_side_t if_side,
                                   unsigned int lb_mode, unsigned int enable)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_loopback_set(q28->type, *info, lb_mode, enable);
}

int curie2ru_bcm82752_prbs_set(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side,
                               curie2ru_prbs_t prbs, unsigned int enable)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int poly;
    int rc;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);

    switch (prbs) {
    case CURIE2RU_PRBS_7:
        poly = 0;
        break;
    case CURIE2RU_PRBS_9:
        poly = 1;
        break;
    case CURIE2RU_PRBS_11:
        poly = 2;
        break;
    case CURIE2RU_PRBS_15:
        poly = 3;
        break;
    case CURIE2RU_PRBS_23:
        poly = 4;
        break;
    default:
    case CURIE2RU_PRBS_31:
        poly = 5;
        break;
    }

    rc = bcm_plp_prbs_set(q28->type, *info, 0, poly, 0, 0, enable);
    if (!rc && !enable)
        rc = bcm_plp_prbs_clear(q28->type, *info, 0);
    return rc;
}

int curie2ru_bcm82752_prbs_clear_rx_stat(struct curie2ru *curie, int port, curie2ru_if_side_t if_side)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    if ((rc = bcm_plp_prbs_status_get(q28->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }
    return 0;
}

int curie2ru_bcm82752_prbs_check(struct curie2ru *curie,
                                 int port, curie2ru_if_side_t if_side)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    int rc;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    if ((rc = bcm_plp_prbs_status_get(q28->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }

    if (prbs_lock) {
        prt("prbs locked\n");
        prt("prbs error count: %d\n", error_count);
    } else {
        prt("prbs unlock\n");
    }
    if (prbs_lock_loss)
        log_warn("prbs lock loss\n");
    return (!prbs_lock || prbs_lock_loss || error_count) ? -1 : 0;
}

int curie2ru_bcm82752_firmware_lane_set(struct curie2ru *curie,
                        int port, curie2ru_if_side_t if_side,
                        bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_firmware_lane_config_set(q28->type, *info,
                                            firmware_lane_config);
}

int curie2ru_bcm82752_firmware_lane_get(struct curie2ru *curie,
                            int port, curie2ru_if_side_t if_side,
                            bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);
    return bcm_plp_firmware_lane_config_get(q28->type, *info,
                                            firmware_lane_config);
}

int curie2ru_bcm82752_cl73_set(struct curie2ru *curie,
                               int port, curie2ru_if_side_t if_side,
                               unsigned int enable)
{
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];
    bcm_plp_access_t *info = &q28->info;
    int rc;
    unsigned short tech_ability = 5;
    unsigned short fec_ability = 0;
    unsigned short pause_ability = 0;
    bcm_plp_an_config_t an_config = {
        .cl72_en = 1,
        .tech_ability = 5,
    };

    info->if_side = CURIE2RU_IF_SIDE_TO_QUADRA28(if_side);

    rc = bcm_plp_cl73_ability_set(q28->type, *info, tech_ability,
                                  fec_ability, pause_ability, an_config);
    if (rc) {
        log_err("bcm_plp_cl73_ability_set failed, return code [%d]\n", rc);
        return rc;
    }

    rc = bcm_plp_cl73_set(q28->type, *info, enable);
    if (rc) {
        log_err("bcm_plp_cl73_set failed, return code [%d]\n", rc);
        return rc;
    }

    return 0;
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru.c,v $
Revision 1.1  2020/01/09 01:01:56  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
