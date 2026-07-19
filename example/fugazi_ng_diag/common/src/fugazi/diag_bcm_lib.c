/* $Id: diag_bcm_lib.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_bcm_lib.c - Fugazi Boradcom chip library.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019-2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "dash_fpga.h"
#include "diag_bcm_lib.h"
#include "diag_bcm57412_test.h"
#include "diag_bcm82757_test.h"
#include "nvsysvars.h"
#include "platform_eth.h"


static int force_intf_lrm_flag = 0;
static bcm_plp_pm_interface_t line_side_intf_type = bcm_pm_InterfaceSFI;
boolean bcm82757_fw_downloaded = DISABLE; /* Skip BCM82757 FW download */

static const reg_info_t fugazi_bcm_82757_tsce_reg[] = {
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

static const bcm_phy_regs_t fugazi_bcm82757_phy_reg_tbl[] = {
    {"TSEC PCS", FUGAZI_MIURA_DEV_PCS, fugazi_bcm_82757_tsce_reg},
};
#define FUGAZI_BCM82757_NUM_PHY_INTF (sizeof(fugazi_bcm82757_phy_reg_tbl) /      \
                                      sizeof(struct bcm_phy_regs_t_))

/******************************************************************************
 *
 * Function: fugazi_bcm57412_init_wrap
 *
 * Description: This function is the wrapper function with BCM57412 initial
 *
 * Inputs      : bnxt : broadcom structure
 *               settings : broadcom settings
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static int fugazi_bcm57412_init_wrap (struct fugazi_bnxt *bnxt,
                                      struct fugazi_bnxt_settings *settings)
{
    int rc;

    if ((rc = fugazi_bnxt_init(bnxt, settings))) {
        log_err("fugazi_bnxt_init failed\n");
        return rc;
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: fugazi_bcm57412_exit_wrap
 *
 * Description: This is the wrapper function exit the bcm57412
 *
 * Inputs      : bnxt : broadcom bnxt structure
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static void fugazi_bcm57412_exit_wrap (struct fugazi_bnxt *bnxt)
{
    fugazi_bnxt_exit(bnxt);
}

/******************************************************************************
 * Function: fugazi_bcm57412_init
 *
 * Description: Initial the BCM82757, and mapping PCIe bus.  
 *
 * Inputs      : fugazi - platform sturcture
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
static int fugazi_bcm57412_init (struct fugazi *fugazi)
{
    int ix, jx;
    int rc;
    struct fugazi_bnxt_settings old_settings[NR_FUGAZI_BNXT] = 
    {
        {
            .pci_domain = 0,
            .pci_bus = 0x01,
            .pci_dev = 0,
            .pci_func = 0,
        },
        {
            .pci_domain = 0,
            .pci_bus = 0x02,
            .pci_dev = 0,
            .pci_func = 0,
        },
    };
    struct fugazi_bnxt_settings new_settings[NR_FUGAZI_BNXT] = 
    {
        {
            .pci_domain = 0,
            .pci_bus = 0x17,
            .pci_dev = 0,
            .pci_func = 0,
        },
        {
            .pci_domain = 0,
            .pci_bus = 0x18,
            .pci_dev = 0,
            .pci_func = 0,
        },
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
        {
            .pci_domain = 0,
            .pci_bus = 0x67,
            .pci_dev = 0,
            .pci_func = 0,
        },
        {
            .pci_domain = 0,
            .pci_bus = 0x68,
            .pci_dev = 0,
            .pci_func = 0,
        },
    };
    struct fugazi_bnxt_settings auto_settings[NR_FUGAZI_BNXT];
    struct fugazi_bnxt_settings *settings;
    struct pci_dev *pci = NULL;

    settings = &old_settings[0]; /* The default PCIe port init */
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
    log_info("Found first BCM57412 at %04x:%02x:%02x.%d: vendor = %x (%x)\n", 
              pci->domain, pci->bus, pci->dev, pci->func,pci->vendor,
              BCM57412_PCI_VENDOR);

    pci_dev_put(pci);

    for (ix = 0; ix < MAX_FUGAZI_MAC; ix++) {
        if ((rc = fugazi_bcm57412_init_wrap(&fugazi->bnxt[ix], &settings[ix]))) {
            for (jx = 0; jx < ix; jx++) {
                fugazi_bcm57412_exit_wrap(&fugazi->bnxt[jx]);
            }
            return rc;
        }
    }
    
    return (PASSED);
}
/******************************************************************************
 * Function: fugazi_bcm57412_exit
 *
 * Description: Exit the BCM82757  
 *
 * Inputs      : fugazi - platform sturcture
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
static void fugazi_bcm57412_exit (struct fugazi *fugazi)
{
    fugazi_bcm57412_exit_wrap(&fugazi->bnxt[1]);
    fugazi_bcm57412_exit_wrap(&fugazi->bnxt[0]);
}

/******************************************************************************
 * Function: fugazi_bcm82757_mdio_read
 *
 * Description: Read BCM82757 register via MIDO Clause 45
 *
 * Inputs      : ctx - context
 *               devad - device
 *               addr - address
 *               data - read back value
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
static int fugazi_bcm82757_mdio_read (void* ctx, uint16_t phy_id, uint16_t devad,
                                      uint16_t addr, uint16_t *data)
{
    devad |= MDIO_PHY_ID_C45;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        log_info("DBG : devad=0x%x,phy_id=0x%x,addr=0x%x\n", devad, phy_id, addr);
    }
    return fugazi_bnxt_mdio_read(ctx, phy_id, devad, addr, data);
}

/******************************************************************************
 * Function: fugazi_bcm82757_mdio_write
 *
 * Description: Write data to BCM82757 register via MIDO Clause 45
 *
 * Inputs      : ctx - context
 *               devad - device
 *               addr - address
 *               data - write value
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
static int fugazi_bcm82757_mdio_write (void* ctx, uint16_t phy_id, uint16_t devad,
                                       uint16_t addr, uint16_t data)
{
    devad |= MDIO_PHY_ID_C45;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        log_info("DBG : devad=0x%x,phy_id=0x%x,addr=0x%x \n",devad,phy_id,addr);
        log_info("DBG : ctx=0x%X \n",ctx);
    }
    return fugazi_bnxt_mdio_write(ctx, phy_id, devad, addr, data);
}

/******************************************************************************
 * Function: fugazi_bcm82757_init
 *
 * Description: Initial BCM82757 data structure
 *
 * Inputs      : fugazi - fugazi structure
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_init (struct fugazi *fugazi)
{
    struct fugazi_miura_settings settings[NR_FUGAZI_BNXT] = {
        {
            .type  = "miura",
            .ctx   = &fugazi->bnxt[0],
            .phy_id= BCM82757_PHY_ID_1,
            .read  = fugazi_bcm82757_mdio_read,
            .write = fugazi_bcm82757_mdio_write,
            .miura_id = MIURA_PHY_ID_1,
        },
        {
            .type  = "miura",
            .ctx   = &fugazi->bnxt[1],
            .phy_id= BCM82757_PHY_ID_2,
            .read  = fugazi_bcm82757_mdio_read,
            .write = fugazi_bcm82757_mdio_write,
            .miura_id = MIURA_PHY_ID_2,
        },
    };
    struct fugazi_miura *miura = &fugazi->miura;
    int rc = PASSED;
    /* Skip BCM82757 initial */
    if (bcm82757_fw_downloaded == DISABLE) {
        return (PASSED);
    }
    /* Initial 1st BCM82757 PHY data structure */
    if ((rc = fugazi_miura_init(miura, &settings[0]))) {
        cterr('f', 0, "BCM82757 PHY0 firmware download failed");
        rc = FAILED;
    }

    /* Initial 2nd BCM82757 PHY data structure */
    if ((rc = fugazi_miura_init(miura, &settings[1]))) {
        cterr('f', 0, "BCM82757 PHY1 firmware download failed");
        rc = FAILED;
    }

    miura->info.lane_map = 0x3; /* represents lane 0 and 1 */
    if (rc == FAILED) {
        bcm82757_fw_downloaded = ENABLE;
    } else {
        bcm82757_fw_downloaded = DISABLE;
    }
    return (rc);
}

/******************************************************************************
 * Function: fugazi_bcm82757_exit
 *
 * Description: Exit BCM82757 Miura driver
 *
 * Inputs      : fugazi - fugazi structure
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
static void fugazi_bcm82757_exit (struct fugazi *fugazi)
{
    struct fugazi_miura *miura = &fugazi->miura;

    fugazi_miura_exit(miura);
}

/******************************************************************************
 * Function: fugazi_init
 *
 * Description: Initial BCM57412 MAC and BCM82757 PHY
 *
 * Inputs      : fugazi - fugazi structure
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_init (struct fugazi *fugazi)
{
    int rc;

    memset(fugazi, 0, sizeof(*fugazi));

    if ((rc = fugazi_bcm57412_init(fugazi))) {
        log_err("fugazi_bcm57412_init failed\n");
        return rc;
    }
    if (bcm82757_fw_downloaded == ENABLE) {
        if ((rc = fugazi_bcm82757_init(fugazi))) {
            log_err("fugazi_bcm82757_init failed\n");
            return rc;
        }
    }

    return (PASSED);
}

/******************************************************************************
 * Function: fugazi_exit
 *
 * Description: Exit BCM57412 MAC and BCM82757 PHY
 *
 * Inputs      : fugazi - fugazi structure
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
void fugazi_exit (struct fugazi *fugazi)
{
    fugazi_bcm82757_exit(fugazi);
    fugazi_bcm57412_exit(fugazi);
}
/******************************************************************************
 * Function: fugazi_bcm82757_port_map
 *
 * Description: BCM57412 PCIe bus mapping.
 *
 * Inputs      : fugazi - fugazi structure
 *               miura - fugazi miura structure
 *               lane - lane no.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
void fugazi_bcm82757_port_map (struct fugazi *fugazi, struct fugazi_miura *miura, 
                               fugazi_lane_t lane)
{
    if (lane < 2) {
        miura->settings.ctx = &fugazi->bnxt[0];
        miura->settings.phy_id = BCM82757_PHY_ID_1;
    } else {
        miura->settings.ctx = &fugazi->bnxt[1];
        miura->settings.phy_id = BCM82757_PHY_ID_2;
    }

}
/******************************************************************************
 * Function: fugazi_bcm82757_read_mdio
 *
 * Description: Read BCM82757 value via MDIO
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               devaddr - device address
 *               *data - read back value.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_read_mdio (struct fugazi *fugazi, fugazi_lane_t lane, 
                               fugazi_if_side_t if_side, uint32_t devaddr, 
                               uint32_t regaddr, uint32_t *data)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    int rc;
    uint16_t data_tmp;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    rc = fugazi_bcm82757_mdio_read(miura->settings.ctx, (uint16_t)miura->settings.phy_id,  
                                  (uint16_t)devaddr, (uint16_t)regaddr, &data_tmp);
    *data = data_tmp;
    return (PASSED);
}

/******************************************************************************
 * Function: fugazi_bcm82757_write_mdio
 *
 * Description: Write data into BCM82757 via MDIO
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               devaddr - device address
 *               data - write value.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_write_mdio (struct fugazi *fugazi, fugazi_lane_t lane, 
                                fugazi_if_side_t if_side, uint32_t devaddr, 
                                uint32_t regaddr, uint32_t data)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;
    devaddr |= MDIO_PHY_ID_C45;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    fugazi_bcm82757_mdio_write(miura->settings.ctx, (uint16_t)miura->settings.phy_id,  
                              (uint16_t)devaddr, (uint16_t)regaddr, data);
    return (PASSED);
}
/******************************************************************************
 * Function: fugazi_bcm82757_read
 *
 * Description: Read BCM82757 value
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               devaddr - device address
 *               *data - read back value.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_read (struct fugazi *fugazi,
                          fugazi_lane_t lane, fugazi_if_side_t if_side,
                          uint32_t devaddr, uint32_t regaddr, uint32_t *data)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;

    /* fugazi_log_level = FUGAZI_LOG_LEVEL_DBG; */
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_reg_value_get(miura->type, *info, devaddr, regaddr, data);
}

/******************************************************************************
 * Function: fugazi_bcm82757_write
 *
 * Description: Write data into BCM82757
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               devaddr - device address
 *               data - write value.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_write (struct fugazi *fugazi,
                           fugazi_lane_t lane, fugazi_if_side_t if_side,
                           uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;
    devaddr |= MDIO_PHY_ID_C45;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_reg_value_set(miura->type, *info, devaddr, regaddr, data);
}

/******************************************************************************
 * Function: fugazi_bcm82757_dump
 *
 * Description: Dump BCM82757 value
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_dump (struct fugazi *fugazi,
                          fugazi_lane_t lane, fugazi_if_side_t if_side)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_phy_status_dump(miura->type, *info);
}
/******************************************************************************
 * Function: fugazi_bcm82757_regs_show
 *
 * Description: Display BCM82757 register value
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               phy_reg_ptr - phy reg table
 * Outputs     : None
 *****************************************************************************/
static void fugazi_bcm82757_regs_show (struct fugazi *fugazi, fugazi_lane_t lane, 
                                       fugazi_if_side_t if_side, 
				                       const bcm_phy_regs_t *phy_reg_ptr)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int rdval;
    const reg_info_t *reg_ptr;
    int dev_id, reg_offset;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;
    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    reg_ptr = phy_reg_ptr->intfregs;
    dev_id = phy_reg_ptr->phy_intf;
    while (reg_ptr->size.size != 0) {
        reg_offset = reg_ptr->offset;
        if (bcm_plp_reg_value_get(miura->type, *info, dev_id, reg_offset, &rdval)) {
            prt("WORN: bcm82757 reg value get failed\n");
        }
        log_info("%s : %-32s reg %d.%#.8x = %#.4x\n", phy_reg_ptr->intfname, 
                  reg_ptr->name, dev_id, reg_offset, rdval);
        reg_ptr++;
        fugazi_mdelay(10);
    }
}
/******************************************************************************
 * Function: fugazi_bcm82757_regs_dump
 *
 * Description: Dump BCM82757 register value
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_regs_dump (struct fugazi *fugazi, fugazi_lane_t lane)
{
    unsigned int ix, intf_move;
    const bcm_phy_regs_t *phy_reg_ptr;
    fugazi_if_side_t if_side;

    intf_move = FUGAZI_BCM82757_NUM_PHY_INTF;

    /* dump all page */
    prt("***************line side********************\n");
    phy_reg_ptr = &fugazi_bcm82757_phy_reg_tbl[0];
    if_side = FUGAZI_IF_SIDE_SYS;
    for (ix = 0; ix < intf_move; ix++) {
        fugazi_bcm82757_regs_show(fugazi, lane, if_side, phy_reg_ptr);
        phy_reg_ptr++;
    }

    prt("***************sys side********************\n");
    phy_reg_ptr = &fugazi_bcm82757_phy_reg_tbl[0];
    if_side = FUGAZI_IF_SIDE_SYS;
    for (ix = 0; ix < intf_move; ix++) {
        fugazi_bcm82757_regs_show(fugazi, lane, if_side, phy_reg_ptr);
        phy_reg_ptr++;
    }

    return 0;
}
/******************************************************************************
 * Function: fugazi_bcm82757_dump
 *
 * Description: Dump BCM82757 mac register value
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_mac_dump (struct fugazi *fugazi,
                              fugazi_lane_t lane, fugazi_if_side_t if_side)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->mac_info.phy_info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_mac_diagnostic_dump(miura->type, miura->mac_info);
}

/******************************************************************************
 * Function: fugazi_bcm82757_link_status
 *
 * Description: Get BCM82757 link status
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               link_status - link status
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_link_status (struct fugazi *fugazi,
                                 fugazi_lane_t lane, fugazi_if_side_t if_side,
                                 unsigned int *link_status)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_link_status_get(miura->type, *info, link_status);
}

/******************************************************************************
 * Function: fugazi_bcm82757_display_eye_scan
 *
 * Description: Display bcm827575 eye diagram
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_display_eye_scan (struct fugazi *fugazi, fugazi_lane_t lane, 
                                      fugazi_if_side_t if_side)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_display_eye_scan(miura->type, *info);
}

/******************************************************************************
 * Function: fugazi_bcm82757_loopback_set
 *
 * Description: Setup bcm827575 loopback mode
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               lb_mode - loopback mode (Remote/Digital )
 *               enable - enable / disable
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_loopback_set (struct fugazi *fugazi,
                                  fugazi_lane_t lane, fugazi_if_side_t if_side,
                                  unsigned int lb_mode, unsigned int enable)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_loopback_set(miura->type, *info, lb_mode, enable);
}
/******************************************************************************
 * Function: fugazi_bcm82757_loopback_get
 *
 * Description: This API is used to get the status of specified loopback whether or not enabled
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               lb_mode - loopback mode (Remote/Digital )
 *               enable - enable / disable
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_loopback_get (struct fugazi *fugazi,
                                  fugazi_lane_t lane, fugazi_if_side_t if_side,
                                  unsigned int lb_mode, unsigned int *enable)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_loopback_get(miura->type, *info, lb_mode, enable);
}
/******************************************************************************
 * Function: fugazi_bcm82757_interrupt
 *
 * Description: Setup bcm827575 Interrupt 
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               intr_type - Interrupt mode 
 *               enable - enable / disable
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_interrupt_set (struct fugazi *fugazi,
                                   fugazi_lane_t lane, fugazi_if_side_t if_side,
                                   unsigned int intr_type, unsigned int enable)
{
    int rc = PASSED;
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    if (enable == BCM82757_INT_CLEAR) {
        /* clear PHY's interrupt */
        rc = bcm_plp_intr_status_clear(miura->type, *info, intr_type);
    } else {
        /* enable/disable PHY's interrupt */
        rc = bcm_plp_intr_enable_set(miura->type, *info, intr_type, enable);
    }
    return (rc);
}
/******************************************************************************
 * Function: fugazi_bcm82757_interrupt_get
 *
 * Description: Get bcm827575 Interrupt value
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               intr_type - Interrupt mode 
 *               dataout - Interrupt value
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_interrupt_get (struct fugazi *fugazi,
                                   fugazi_lane_t lane, fugazi_if_side_t if_side,
                                   unsigned int intr_type, uint32_t *dataout)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_intr_enable_get(miura->type, *info, intr_type, dataout);
}


/******************************************************************************
 * Function: fugazi_bcm82757_prbs_set
 *
 * Description: Setup bcm827575 prbs
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               tx_rx - Tx, Rx, or both side PRBS
 *               prbs - prbs mode
 *               enable - enable / disable
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_prbs_set (struct fugazi *fugazi,
                              fugazi_lane_t lane, fugazi_if_side_t if_side,
                              unsigned int tx_rx, fugazi_prbs_t prbs,
                              unsigned int enable)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int poly;
    int rc;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

    switch (prbs) {
    case FUGAZI_PRBS_7:
        poly = 0;
        break;
    case FUGAZI_PRBS_9:
        poly = 1;
        break;
    case FUGAZI_PRBS_11:
        poly = 2;
        break;
    case FUGAZI_PRBS_15:
        poly = 3;
        break;
    case FUGAZI_PRBS_23:
        poly = 4;
        break;
    default:
    case FUGAZI_PRBS_31:
        poly = 5;
        break;
    }

    if (enable && line_side_intf_type == bcm_pm_InterfaceLRM) {
        /* LRM, need to write 0xb into 0x4800d0d1 */
        rc = bcm_plp_reg_value_set(miura->type, *info, FUGAZI_MIURA_DEV_PCS, 
                                   BCMI_MIURA_PRBS_GENERATOR_CONTROL, 0xb);
        if (rc) {
            log_err("bcm_plp_set_value failed\n");
            return rc;
        }
    }

    rc = bcm_plp_prbs_set(miura->type, *info, tx_rx, poly, 0, 0, enable);
    if (!rc && !enable)
        rc = bcm_plp_prbs_clear(miura->type, *info, tx_rx);
    return rc;
}

/******************************************************************************
 * Function: fugazi_bcm82757_prbs_clear_rx_stat
 *
 * Description: Clear bcm827575 prbs rx stat
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_prbs_clear_rx_stat (struct fugazi *fugazi,
                                        fugazi_lane_t lane, fugazi_if_side_t if_side)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    int rc;
    
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;
    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

    if ((rc = bcm_plp_prbs_rx_stat(miura->type, *info, 0))) {
        log_err("bcm_plp_prbs_rx_stat failed\n");
        return rc;
    }

    return rc;
}
/******************************************************************************
 * Function: fugazi_bcm82757_prbs_check
 *
 * Description: Check bcm827575 prbs mode
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_prbs_check (struct fugazi *fugazi,
                                fugazi_lane_t lane, fugazi_if_side_t if_side)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    unsigned int prbs_lock;
    unsigned int prbs_lock_loss;
    unsigned int error_count;
    unsigned int data_m, data_l;
    uint32_t regaddr;
    int rc;
    uint32_t dev_addr  = 1;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

    if ((rc = bcm_plp_prbs_status_get(miura->type, *info,
                                      &prbs_lock, &prbs_lock_loss,
                                      &error_count))) {
        log_err("bcm_plp_prbs_status_get failed\n");
        return rc;
    }

    /* CSCvr24877 : Check PRBS_CHK_ERR_CNT register when inject error from external */
    /* MSB must read before LSB register in the error counter */
    dev_addr = FUGAZI_MIURA_DEV_PCS;
    regaddr = BCM82757_PRBS_CHK_ERR_CNT_MSB; 
    if (bcm_plp_reg_value_get(miura->type, *info, dev_addr, regaddr, &data_m)) {
        log_err("BCM82757 register read error\n");
        return rc;
    }
    regaddr = BCM82757_PRBS_CHK_ERR_CNT_LSB; 
    if (bcm_plp_reg_value_get(miura->type, *info, dev_addr, regaddr, &data_l)) {
        log_err("BCM82757 register read error\n");
        return rc;
    }
    error_count = data_l;
    if (prbs_lock) {
        prt("Port %d prbs locked\n",lane);
        prt("Port %d prbs error count: %d\n",lane ,error_count);
    } else { 
        prt("Port %d prbs unlock\n",lane);
    }
    if (prbs_lock_loss) {
        prt("Port %d prbs lock loss\n",lane);
    }
    return (!prbs_lock || prbs_lock_loss || error_count) ? -1 : 0;
}

/******************************************************************************
 * Function: fugazi_bcm82757_prbs_clear_error
 *
 * Description: Clear bcm827575 prbs error counters by direct register write (No API).
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_prbs_clear_error (struct fugazi *fugazi, fugazi_lane_t lane, 
                                      fugazi_if_side_t if_side )
{
    int    rc = PASSED;
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    uint32_t dev_addr  = 1;
    uint32_t data, offset, i;
    uint32_t addr_array_sys[2]  = { 0x4200d0db, 0x4200d0da };
    uint32_t addr_array_line[2] = { 0x4800d0db, 0x4800d0da };

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

    for ( i=0; i<2; i++ ) {
        /* read MSB first */
        if (info->if_side == FUGAZI_MIURA_LINE_SIDE) {
            /* clear error counter at line side by read reg. 0x4800d0db, 0x4800d0da */
            offset = addr_array_line[i];
        }
        else {
            /* clear error counter at system side by read reg. 0x4200d0db, 0x4200d0da */
            offset = addr_array_sys[i];
        }
        rc |= bcm_plp_reg_value_get( miura->type, *info, dev_addr, offset, &data);
        if (rc) {
            log_err("bcm_plp_reg_value_get() failed for PHY-ID [%d], "
                    "phy_addr [0x%x], reg offset [0x%x], return code [%d]\n",
                     miura->settings.phy_id, info->phy_addr, offset, rc);
            return rc;
        }
    }

    return rc;
}


/******************************************************************************
 * Function: fugazi_bcm82757_firmware_lane_set
 *
 * Description: Setup bcm827575 lane side mode
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               firmware_lane_config - setup the firmware config.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_firmware_lane_set (struct fugazi *fugazi,
                                       fugazi_lane_t lane, fugazi_if_side_t if_side,
                                       bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_firmware_lane_config_set(miura->type,*info ,firmware_lane_config);
}

/******************************************************************************
 * Function: fugazi_bcm82757_firmware_lane_get
 *
 * Description: Get bcm827575 lane side config
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               firmware_lane_config - setup the firmware config.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_firmware_lane_get (struct fugazi *fugazi,
                                       fugazi_lane_t lane, fugazi_if_side_t if_side,
                                       bcm_plp_pm_firmware_lane_config_t *firmware_lane_config)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);
    return bcm_plp_firmware_lane_config_get(miura->type,*info ,firmware_lane_config);
}

/******************************************************************************
 * Function: fugazi_bcm82757_cl73_set
 *
 * Description: Setup bcm827575 cl73 config
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               if_side - LINE / SYS side
 *               enable - enable / disable
  *              cl37_en - 1: disable cl73, 0: enable cl73
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_cl73_set (struct fugazi *fugazi,
                              fugazi_lane_t lane, fugazi_if_side_t if_side,
                              unsigned int enable, int cl37_en)
{
    struct fugazi_miura *miura = &fugazi->miura;
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

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

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

    return (PASSED);
}

/******************************************************************************
 * Function: fugazi_bcm82757_config_macsec_cleanup
 *
 * Description: Cleanup bcm827575 macsec config
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
void fugazi_bcm82757_config_macsec_cleanup (struct fugazi *fugazi, 
                                            fugazi_lane_t lane)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    plp_info->phy_addr = lane / 2;

    plp_info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    fugazi_miura_macsec_exit(&fugazi->miura);
}

/******************************************************************************
 * Function: fugazi_bcm82757_config_macsec_bypass
 *
 * Description: Setup bcm827575 macsec bypass
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               speed - network speed.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_config_macsec_bypass (struct fugazi *fugazi, 
                                          fugazi_lane_t lane, int speed)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *plp_info = &miura->info;
    bcm_plp_sec_phy_access_t sec_info;
    int aux = 0;
    int rc;
    bcm_plp_pm_interface_t line_side_interface = line_side_intf_type;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    plp_info->phy_addr = lane / 2;

    if ((rc = fugazi_miura_macsec_init(miura, 1))) {
        return rc;
    }
    if (speed == FUGAZI_PORT_SPEED_1G) {
        line_side_interface = bcm_pm_Interface1000X;
    }
    memset(&sec_info, 0, sizeof(sec_info));

    /* Set Secy Config set for Both Egress and Ingress */
    plp_info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    plp_info->if_side = FUGAZI_MIURA_LINE_SIDE;
    memcpy(&sec_info.phy_info, plp_info, sizeof(bcm_plp_access_t));
    sec_info.macsec_side = FUGAZI_MIURA_EGRESS;

    /* Secy Config set */
    rc = bcm_plp_secy_config_set(miura->type, &sec_info);
    if (rc) {
        log_err("port %d bcm_plp_secy_config_set failed for device-id [%d], "
                "return code [%d]\n", lane, sec_info.macsec_side, rc);
        return rc;
    }

    /* Mode Config set for each port */
    plp_info->if_side = FUGAZI_MIURA_LINE_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 line_side_interface, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }

    plp_info->if_side = FUGAZI_MIURA_SYS_SIDE;
    rc = bcm_plp_mode_config_set(miura->type, *plp_info, speed,
                                 bcm_pm_InterfaceXFI, bcm_pm_RefClk156Mhz,
                                 bcm_pm_Interface_mode_IEEE, &aux);
    if (rc) {
        log_err("Error in setting Config\n");
        return rc;
    }
    /* Egress, Set SECY to Bypass */
    /* CSCvq67625 : Fixed "BCM82757 Loopback test" intermittent failure. 
     *              BCM57412 FW/Driver also need upgrade.
     *              bnxt_en driver version: 216.1.48.0
     *              firmware version: 216.1.96.0 
     */
    
    plp_info->if_side  = FUGAZI_MIURA_LINE_SIDE;
    memcpy(&sec_info.phy_info, plp_info, sizeof(bcm_plp_access_t));
    sec_info.macsec_side = FUGAZI_MIURA_EGRESS;
    rc = bcm_plp_secy_bypass_set(miura->type, &sec_info, 1);
    if (rc) {
        log_err("port %d bcm_plp_secy_bypass_set failed for PHY-ID [%d], "
                "macsec_side [%d], return code [%d]\n",
                 lane, plp_info->phy_addr, sec_info.macsec_side, rc);
        return rc;
    }
    /* Ingress, Set SECY to Bypass */
    sec_info.macsec_side = FUGAZI_MIURA_INGRESS;
    rc = bcm_plp_secy_bypass_set(miura->type, &sec_info, 1);
    if (rc) {
        log_err("port %d bcm_plp_secy_bypass_set failed for PHY-ID [%d], "
                "macsec_side [%d], return code [%d]\n",
                 lane, plp_info->phy_addr, sec_info.macsec_side, rc);
        return rc;
    }

    return rc;
}

/******************************************************************************
 * Function: fugazi_bcm82757_show_fw_version
 *
 * Description: display bcm82747 firmware version
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               link_status - link status
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_show_fw_version (struct fugazi *fugazi, 
                                     unsigned int *fw_version, 
                                     unsigned int *fw_crc)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;

    return bcm_plp_firmware_info_get(miura->type, *info, fw_version, fw_crc);
}


/******************************************************************************
 * Function: fugazi_bcm82780_recover_clock
 *
 * Description: display bcm82747 firmware version
 *
 * Inputs      : fugazi - fugazi structure
 *               phy_port - 0 ~ 3 (1st .. last bcm82780 PHY port on Fugazi board)
 *               enable - 1/0: enable/disable output CLK_RCVCLK0/1
 * Outputs     : PASSED / FAILED
 * Note: From Broadcom Knowledge Base KB0027720 (Diag only implement for 10G Non-LRM):
 * (1). RCLK enabling sequence for 10G Non-LRM mode (Like, SFP-10G-LR/SR)
 *  to enable RCLK with 156.25MHz, below sequence can be followed:
 *   Write 4700CB2A.13=0 (non-LRM mode clk select)
 *   Write 4800D094.[10:0]=0x0 (for div66 (156.25MHz), based on 4800D090.[11:10] below)
 *   Write 4800D090.[11:10]=0x3 (for div 66 (156.25MHz))
 *   Write 4800D090.9=1 (output clk enable)
 *   Write 4800D093.1=1
 *   Write 4800D0B5.4=1
 *   Write 4800D0B4[14:12]=3
 *
 * (2). RCLK enabling sequence for 10G LRM mode (SFP-10G-LRM)
 *  to enable RCLK with 156.25MHz, below sequence can be followed:
 *   Write 4700CB2A.13=1 (LRM mode clk select)
 *   Write 4700CB26[14:13]=0x3 (for div 66 (156.25MHz))
 *   Write 4700CB26.12=1 (enable rclk)
 *   Write 4800D0B5.4=1
 *   Write 4800D0B4[14:12]=3
 *
 *****************************************************************************/
#define BCM_CLOCK_REG 6
int fugazi_bcm82780_recover_clock (struct fugazi *fugazi, int port, int enable )
{
    int    rc = PASSED;
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    uint32_t dev_addr  = 1;
    uint32_t data, readback_data, i;
    uint32_t addr_array[BCM_CLOCK_REG] = { 0x4700CB2A, 0x4800D094, 0x4800D090, 0x4800D093, 0x4800D0B5, 0x4800D0B4 };

    fugazi_bcm82757_port_map(fugazi, miura, port);
    info->phy_addr = port / 2;
    info->lane_map = FUGAZI_LANE_TO_MIURA(port);
    info->if_side = FUGAZI_MIURA_LINE_SIDE;

    if ( enable == 1 ) {
        /* Enable recovered clock output */
        for ( i=0; i<BCM_CLOCK_REG; i++ ) {
            rc |= bcm_plp_reg_value_get( miura->type, *info, dev_addr, addr_array[i], &data);
            if (rc) {
                log_err("bcm_plp_reg_value_get() failed for PHY-ID [%d], "
                        "phy_addr [0x%x], reg offset [0x%x], return code [%d]\n",
                        miura->settings.phy_id, info->phy_addr, addr_array[i], rc);
                return rc;
            }
            /*
             * Registers to be programmed to achive synce 156.25Mhz freq.
             * 1.CB2A[13]=0       (non-LRM mode clk select)
             * 1.D094[10:0]=0x000 (for div66 (156.25MHz), based on 4800D090.[11:10] below)
             * 1.D090[11:10] = 11 (156.25MHz :divide by 66)
             * 1.D090[9]=1        (output clk enable)
             * 1.D093[1]=1
             * 1.D0B5[4]=1
             * 1.D0B4[14:12]=011
             */
            if ( i == 0 ) { data &= ~ ( BIT_13 );               }
            if ( i == 1 ) { data &= ~ ( 0x7ff );                }
            if ( i == 2 ) { data |=   ( BIT_11|BIT_10|BIT_9 );  }  // output clk enable
            if ( i == 3 ) { data |=   ( BIT_1  );               }
            if ( i == 4 ) { data |=   ( BIT_4  );               }
            if ( i == 5 ) { data &= ~ ( BIT_14 );               }
            if ( i == 5 ) { data |=   ( BIT_13|BIT_12 );        }

            rc |= bcm_plp_reg_value_set( miura->type, *info, dev_addr, addr_array[i], data);
            if (rc) {
                log_err("bcm_plp_reg_value_set() failed for PHY-ID [%d], "
                        "phy_addr [0x%x], reg offset [0x%x], data [0x%x], return code [%d]\n",
                        miura->settings.phy_id, info->phy_addr, addr_array[i], data, rc);
                return rc;
            }

            usleep( 1000 );
            rc |= bcm_plp_reg_value_get( miura->type, *info, dev_addr, addr_array[i], &readback_data);
            if (rc) {
                log_err("bcm_plp_reg_value_get() failed for PHY-ID [%d], "
                        "phy_addr [0x%x], reg offset [0x%x], return code [%d]\n",
                        miura->settings.phy_id, info->phy_addr, addr_array[i], rc);
                return rc;
            }
            if ((readback_data & data) != data) {
                log_err("Read back data 0x%x is not same as data wrote 0x%x, failed for PHY-ID [%d], "
                        "phy_addr [0x%x], reg offset [0x%x]\n",
                        readback_data, data,
                        miura->settings.phy_id, info->phy_addr, addr_array[i]);
                return FAILED;
            }

        } /* for ( i=0; i<BCM_CLOCK_REG; i++ ) { */
    } else {
        /* Disable recovered clock output */
        rc |= bcm_plp_reg_value_get( miura->type, *info, dev_addr, addr_array[4], &data); //0x4800D0B5
        if (rc) {
            log_err("bcm_plp_reg_value_get() failed for PHY-ID [%d], "
                   "phy_addr [0x%x], reg offset [0x%x], return code [%d]\n",
                   miura->settings.phy_id, info->phy_addr, addr_array[4], rc);
            return rc;
        }
        data &= ~(BIT_4);
        rc |= bcm_plp_reg_value_set( miura->type, *info, dev_addr, addr_array[4], data);
        if (rc) {
            log_err("bcm_plp_reg_value_set() failed for PHY-ID [%d], "
                    "phy_addr [0x%x], reg offset 0x%X, data [0x%x], return code [%d]\n",
                    miura->settings.phy_id, info->phy_addr, addr_array[4], data, rc);
            return rc;
        }

        usleep( 1000 );
        rc |= bcm_plp_reg_value_get( miura->type, *info, dev_addr, addr_array[4], &readback_data);
        if (rc) {
            log_err("bcm_plp_reg_value_get() failed for PHY-ID [%d], "
                    "phy_addr [0x%x], reg offset [0x%x], return code [%d]\n",
                    miura->settings.phy_id, info->phy_addr, addr_array[4], rc);
            return rc;
        }
        if ((readback_data & data) != data) {
            log_err("Read back data 0x%x is not same as data wrote 0x%x, failed for PHY-ID [%d], "
                    "phy_addr [0x%x], reg offset [0x%x]\n",
                    readback_data, data,
                    miura->settings.phy_id, info->phy_addr, addr_array[4]);
            return FAILED;
        }

    }

    return (rc);
}

/******************************************************************************
 * Function: fugazi_bcm82757_tx_analog_get
 *
 * Description: This API is used to get 10G Transmitter Analog per serdes lane.
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               tx_analog - point to buffer to store Transmitter Analog parameters.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_tx_analog_get (struct fugazi *fugazi,
                                   fugazi_lane_t lane, bcm_plp_tx_t *tx_analog)
{
    int rc = PASSED;
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;
    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_MIURA_LINE_SIDE;

    rc = bcm_plp_tx_get(miura->type, *info, tx_analog);

    return (rc);
}

/******************************************************************************
 * Function: fugazi_bcm82757_tx_analog_set
 *
 * Description: This API is used to Set 10G Transmitter Analog per serdes lane.
 *
 * Inputs      : fugazi - fugazi structure
 *               lane - lane no.
 *               tx_analog - Transmitter Analog parameters to be set.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_tx_analog_set (struct fugazi *fugazi,
                                   fugazi_lane_t lane, bcm_plp_tx_t *tx_analog)
{
    int rc = PASSED;
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;

    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;
    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_MIURA_LINE_SIDE;

    rc = bcm_plp_tx_set(miura->type, *info, tx_analog);

    return (rc);
}

/******************************************************************************
 * Function: force_line_side_intf_lrm
 *
 * Description: force the BCM82757 line side with LRM mode.
 *
 * Inputs      : force - enable / disable
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
void force_line_side_intf_lrm (int force)
{
    if (force) {
        line_side_intf_type = bcm_pm_InterfaceLRM;
        force_intf_lrm_flag = 1;
    } else {
        force_intf_lrm_flag = 0;
    }
}

/******************************************************************************
 * Function: bcm82757_power_get
 *
 * Description: Get the power status of a specified lane from specified interface side.
 *
 * Inputs: fugazi - fugazi structure
 *         lane - lane no.
 *         if_side - LINE / SYS side
 *         *power_rx - point to store to store Rx power status (0-PowerOff, 1-PowerOn)
 *         *power_tx - point to store to store Tx power status (0-PowerOff, 1-PowerOn)
 *
 * Outputs: PASSED / FAILED
 *****************************************************************************/
int bcm82757_power_get (struct fugazi *fugazi,
                        fugazi_lane_t lane, fugazi_if_side_t if_side,
                        unsigned int *power_rx, unsigned int *power_tx)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

    /* Get power status */
    return (bcm_plp_power_get(miura->type, *info, power_rx, power_tx));
}

/******************************************************************************
 * Function: bcm82757_power_set
 *
 * Description: Set the power of a transmitter or receiver of specified lane to
 *              specified lane from specified interface side.
 *
 * Inputs: fugazi - fugazi structure
 *         lane - lane no.
 *         if_side - LINE / SYS side
 *         power_rx - Rx power state to set (0:Off, 1: On, 2:OffOn, 3: NoChange)
 *         power_tx - Tx power state to set (0:Off, 1: On, 2:OffOn, 3: NoChange)
 *
 * Outputs: PASSED / FAILED
 *****************************************************************************/
int bcm82757_power_set (struct fugazi *fugazi,
                        fugazi_lane_t lane, fugazi_if_side_t if_side,
                        unsigned int power_rx, unsigned int power_tx)
{
    struct fugazi_miura *miura = &fugazi->miura;
    bcm_plp_access_t *info = &miura->info;
    fugazi_bcm82757_port_map(fugazi, miura, lane);
    info->phy_addr = lane / 2;

    info->lane_map = FUGAZI_LANE_TO_MIURA(lane);
    info->if_side = FUGAZI_IF_SIDE_TO_MIURA(if_side);

    /* Get power status */
    return (bcm_plp_power_set(miura->type, *info, power_rx, power_tx));
}

/*-------------------------------------------------
 * $Log: diag_bcm_lib.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.3  2021/05/04 18:40:22  pdoong
 * Change config bcm82757 cl37 mode from directly register write to all bcm82757 API
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.35  2020/08/25 04:34:38  pdoong
 * Add bcm82757_power_get()/bcm82757_power_set() functions to get/Set power state.
 *
 * Revision 1.1.6.33  2020/08/19 09:11:50  iachang
 * PRRQ CSCvo59196-4 : BCM82757 10G PHY code review
 *
 * Revision 1.1.6.32  2020/08/05 08:33:23  iachang
 * Code clean up.
 *
 * Revision 1.1.6.31  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.30  2020/03/25 01:07:49  iachang
 * BCM82757 register read/write utility and register test, using mdio directly access instead of call 10G PHY Broadcom's API.
 *
 * Revision 1.1.6.29  2020/03/18 06:09:54  iachang
 * Replace BCM82757 register r/w funciton from API to direct access.
 * bcm_plp_reg_value_get/bcm_plp_reg_value_set to fugazi_bcm82757_mdio_read/fugazi_bcm82757_mdio_write
 *
 * Revision 1.1.6.28  2020/02/25 08:02:49  iachang
 * Modify BCM82757 LASI test utility.
 *
 * Revision 1.1.6.27  2020/02/18 01:44:52  iachang
 * Add cterr informaiton when BCM82757 download failed.
 *
 * Revision 1.1.6.26  2020/02/13 06:41:04  iachang
 * If BCM82757 FW download failed, next time will download FW again.
 *
 * Revision 1.1.6.25  2020/02/12 06:47:18  iachang
 * If 1'st BCM82757 PHY FW download failed, continue downlow 2'nd PHY FW
 *
 * Revision 1.1.6.24  2020/01/15 07:30:08  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.23  2019/11/26 03:36:55  iachang
 * CSCvq67625 : Fixed "BCM82757 Loopback test" intermittent issue.
 *
 * Revision 1.1.6.22  2019/11/14 08:29:11  iachang
 * Implement SFP present function.
 *
 * Revision 1.1.6.21  2019/10/04 06:05:22  iachang
 * BCM82757 force line side SFP LRM utility
 *
 * Revision 1.1.6.20  2019/09/16 11:23:42  iachang
 * CSCvr24877 : Display PRBS error count when inject error from external
 *
 * Revision 1.1.6.19  2019/08/30 22:00:17  pdoong
 * Add Clear error counter option to clear PRBS error counter.
 *
 * Revision 1.1.6.18  2019/08/29 20:49:26  pdoong
 * Add BCM82757 Analog utility
 *
 * Revision 1.1.6.17  2019/08/16 04:38:43  iachang
 * Display BCM82757 PRBS lock infor and error count
 * BCM82757 check link status need link up consistency 10 times.
 * Fixed BCM82757 only port 0 test all PRBS mode issue.
 *
 * Revision 1.1.6.16  2019/08/05 06:38:05  iachang
 * BCM82757 LINE_SIDE interface type : bcm_pm_Interface1000X for 1G and bcm_pm_InterfaceSFI for 10G
 *
 * Revision 1.1.6.15  2019/08/02 03:32:38  iachang
 * Add BCM82757 Regs dump utility
 * Add packet count check when BCM82757 loopback test failed.
 *
 * Revision 1.1.6.14  2019/07/19 02:29:36  iachang
 * Sync loopback funtion with Curie-2RU
 * Changed Loopback funciton from Curie-2RU to ISR common function tx_rx_diag()
 * Changed BCM82757 print message "lane" to "port"
 *
 * Revision 1.1.6.13  2019/07/18 22:40:01  pdoong
 * Removed dbg msg from bcm82757 recovered clock routine
 *
 * Revision 1.1.6.12  2019/06/21 06:58:46  iachang
 * Support BCM82757 Eye scan utility.
 * Add BCM82757 interrupt utility.
 *
 * Revision 1.1.6.11  2019/06/14 23:58:21  pdoong
 * Add configure bcm82757 10G PHY to generate recovered clock output
 *
 * Revision 1.1.6.10  2019/06/13 14:21:20  iachang
 * Add BCM82757 interrupt utility
 *
 * Revision 1.1.6.9  2019/05/14 02:01:49  pdoong
 * Added to sysyem info to display SyncE/bam82757 firmware version
 *
 * Revision 1.1.6.8  2019/04/17 22:44:16  iachang
 * Modify BCM82757 PHY ID for P1A2 board.
 *
 * Revision 1.1.6.7  2019/04/06 01:07:49  iachang
 * BCM82757 10G PHY pass clause 45 parameter into bnxt_en driver. This change also need driver support
 *
 * Revision 1.1.6.6  2019/04/01 22:34:07  iachang
 * Support 2nd BCM82757 utility.
 *
 * Revision 1.1.6.5  2019/03/19 22:34:58  iachang
 * Correct PHY address mapping and bring up 10G PHY internal loopabck
 *
 * Revision 1.1.6.4  2019/03/18 23:16:14  iachang
 * Bing up 2'nd BCM82757 PHY FW download and external loopback.
 *
 * Revision 1.1.6.3  2019/03/14 21:46:47  iachang
 * Bring up BCM82757 first PHY.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 * Revision 1.1.2.2  2019/03/10 09:58:32  iachang
 * Clean up code.
 *
 * Revision 1.1.2.1  2019/03/07 07:16:49  iachang
 * Separate BCM82757 test and utility items.
 *
 * Revision 1.1.2.3  2019/02/20 07:09:28  iachang
 * BCM82757 Internal,External loopback test
 *
 * Revision 1.1.2.2  2019/02/19 01:55:40  iachang
 * Update BCM57412 PCI bus and BCM82757 FW download
 *
 * Revision 1.1.2.1  2019/02/18 07:16:50  letsai
 * Support BCM phy tests
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

