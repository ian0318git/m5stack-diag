/* $Id: switzer_10g_test.c,v 1.7 2020/09/25 01:41:00 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_10g_test.c,v $
 *------------------------------------------------------------------
 *
 * switzer_10g_test.c - Switzer-10G NGWIC.
 *
 * Sep. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>

#include "common.h"
#include "types.h"
#include "defs.h"
#include "setjmps.h"
#include "signals.h"
#include "menu.h"
#include "error.h"
#include "proto.h"
#include "strings.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "dev_ltc4215.h"
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "platform_slot.h"
#include "dash_fpga.h"
#include "cookie_4.h"
#include "platform_fru.h"
#include "nmc93c46.h"
#include "smart_cookie.h"

#ifdef TABEIL
#include "dnv_eth_lib.h"
#else
#include "platform_eth_pkt_txrx.h"
#endif
#include "switzer_traf.h"

#include "switzer_common.h"
#include "switzer_miura_reg.h"
#include "switzer_10g.h"

#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)

static struct switzer_10g __mod, *mod = &__mod;

static long utils_fpga_reg_read(void)
{
    uint32_t offset, data;

    offset = gethex_answer("FPGA offset", 0, 0, 0xffff);

    data = *(volatile uint32_t *)(mod->fpga.map.vaddr + offset);

    prt("%#.4x --> %#.8x\n", offset, data);
    return PASSED;
}

static long utils_fpga_reg_write(void)
{
    uint32_t offset, data;

    offset = gethex_answer("FPGA offset", 0, 0, 0xffff);
    data = gethex_answer("Data to write", 0, 0, 0xffffffff);

    *(volatile uint32_t *)(mod->fpga.map.vaddr + offset) = data;

    prt("%#.4x <-- %#.8x\n", offset, data);
    return PASSED;
}

static long utils_phy_reg_read(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);

    rc = switzer_10g_phy_read(mod, if_side, devaddr, regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 read error");
        return FAILED;
    }
    prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

static long phy_registers_dump(void)
{
    int rc;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_10g_registers_dump(mod, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 registers dump error");
        return FAILED;
    }
    return PASSED;
}

static long utils_phy_reg_write(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_PMA_PMD,
                            SWITZER_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 write error");
        return FAILED;
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

static long phy_autoneg_remote_ability_get(void)
{
    int rc;
    unsigned short fec_ability, pause_ability;
    bcm_plp_an_config_t an_config;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_10g_phy_autoneg_remote_ability_get(mod, if_side, &fec_ability, &pause_ability, &an_config);
    if (rc) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 autoneg_remote_ability_get error");

        return FAILED;
    }
    prt("fec_ability: %u, pause_ability: %u\n"
        "an_config: master_lane %u, cl72_en %u, tech_ability %u\n",
        fec_ability, pause_ability, an_config.master_lane, an_config.cl72_en, an_config.tech_ability);
    return PASSED;
}

static long utils_sfp_reg_read(void)
{
    int rc;
    uint8_t addr, cmd;
    size_t count;
    char buf[BUF_SIZE];

    cmd = gethex_answer("SFP offset", 0, 0, 0xff);
    count = gethex_answer("SFP read size", 128, 0, sizeof(buf));
    addr = gethex_answer("SFP address", SWITZER_10G_I2C_ADDR_SFP, 0, 0xff);

    rc = switzer_10g_sfp_read(mod, addr, cmd, buf, count);
    if (rc < 0) {
        cterr_add_component("SFP",
                            "I2C controller within the FPGA");
        cterr_add_debug("Check SFP",
                        "Check I2C controller within the FPGA");
        cterr('f', 0, "SFP read error");
        return FAILED;
    }
    switzer_hex_dump(buf, count, addr);
    return PASSED;
}

static long utils_sfp_reg_write(void)
{
    int rc;
    uint8_t addr, cmd;
    size_t count;
    char hex[BUF_SIZE], buf[BUF_SIZE/2];

    cmd = gethex_answer("SFP offset", 0, 0, 0xff);
    hex[0] = '\0';
    prt("SFP write data hexadecimal bytes:  ");
    get_line(hex, sizeof(hex));

    count = strlen(hex) / 2;
    if (switzer_hex_to_bin(hex, buf, count) < 0) {
        cterr('f', 0, "Hexadecimal string format error: %s\n", hex);
        return FAILED;
    }

    addr = gethex_answer("SFP address", SWITZER_10G_I2C_ADDR_SFP, 0, 0xff);

    rc = switzer_10g_sfp_write(mod, addr, cmd, buf, count);
    if (rc < 0) {
        cterr_add_component("SFP",
                            "I2C controller within the FPGA");
        cterr_add_debug("Check SFP",
                        "Check I2C controller within the FPGA");
        cterr('f', 0, "SFP write error");
        return FAILED;
    }

    return PASSED;
}

static long utils_spi_prom_read_status(void)
{
    return switzer_utils_spi_prom_read_status(mod->spi.prom);
}

static long utils_spi_prom_write_status(void)
{
    return switzer_utils_spi_prom_write_status(mod->spi.prom);
}

static long utils_spi_prom_read(void)
{
    return switzer_utils_spi_prom_read(mod->spi.prom);
}

static long utils_spi_prom_write(void)
{
    return switzer_utils_spi_prom_write(mod->spi.prom, TRUE);
}

static long utils_spi_prom_program(void)
{
    return switzer_utils_spi_prom_write(mod->spi.prom, FALSE);
}

static long utils_spi_prom_erase(void)
{
    return switzer_utils_spi_prom_erase(mod->spi.prom);
}

static long ds4424_reg_read(void)
{
    cterr_add_component("DS4424 on Switzer",
                        "I2C controller in the FPGA");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C controller in the FPGA");
    return switzer_utils_i2c_reg_read(mod->pm.i2c);
}

static long ds4424_reg_write(void)
{
    cterr_add_component("DS4424 on Switzer",
                        "I2C controller in the FPGA");
    cterr_add_debug("DS4424 on Switzer",
                    "Check the I2C controller in the FPGA");
    return switzer_utils_i2c_reg_write(mod->pm.i2c);
}

static const reg_info_t ds4424_reg_tbl[] = {
    {"OUT0", 0xf8, READ_ONLY, {1}, 0xF, 0x0},
    {"OUT2", 0xf9, READ_ONLY, {1}, 0xF, 0x0},
    {"OUT3", 0xfa, READ_ONLY, {1}, 0xF, 0x0},
    {"OUT4", 0xfb, READ_ONLY, {1}, 0xF, 0x0},
    {"End of Register", 0, 0, {0}, 0, 0},
};

static long ds4424_reg_test(void)
{
    const reg_info_t *reg;
    uint8_t data;

    prpass(testpass, "DS4424 Register Test ");

    for (reg = ds4424_reg_tbl; reg->size.size; reg++) {
        if (switzer_i2c_slave_read(mod->pm.i2c,
                                   reg->offset, &data, sizeof(data)) < 0) {
            cterr_add_component("DS4424 on Switzer",
                                "I2C controller in the FPGA");
            cterr_add_debug("DS4424 on Switzer",
                            "Check the I2C controller in the FPGA");
            cterr('f', 0, "Register Test on DS4424 failed");
            return FAILED;
        }
    }
    return PASSED;
}

/* Utils submenu items */
static submenu_xtable_t utils_submenu_table[] = {
    {"FPGA Register Read", utils_fpga_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"FPGA Register Write", utils_fpga_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Register Read", utils_phy_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Register Write", utils_phy_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"SFP Register Read", utils_sfp_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"SFP Register Write", utils_sfp_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"LTC4215 Register Read", switzer_ltc4215_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"LTC4215 Register Write", switzer_ltc4215_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"PCA9555 Register Read", switzer_pca_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"PCA9555 Register Write", switzer_pca_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"DS4424 Register Read", ds4424_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"DS4424 Register Write", ds4424_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Read Status", utils_spi_prom_read_status, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Write Status", utils_spi_prom_write_status, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Read", utils_spi_prom_read, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Write", utils_spi_prom_write, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Program", utils_spi_prom_program, 0,
     0, NULL, 0, NULL, 0},
    {"SPI Prom Erase", utils_spi_prom_erase, 0,
     0, NULL, 0, NULL, 0},
    {"Escape to OS Shell (debugging only)", switzer_os_shell, 0,
     0, NULL, 0, NULL, 0},
};

#define UTILS_SUBMENU_TABLE_SZ (sizeof(utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t utils_submenu_primary_items[UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t utils_submenu_secondary_items[UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char utils_submenu_title[] = "Switzer 10G Utilities Menu";

static menuinfo_t utils_submenu = {
    utils_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    utils_submenu_primary_items,
};

static menuinfo_t *utils_submenup = &utils_submenu;

static long io_utils(void)
{
    build_primary_submenu(utils_submenu_table, UTILS_SUBMENU_TABLE_SZ,
                          utils_submenu_title, &utils_submenup);
    build_secondary_submenu(utils_submenu_table, UTILS_SUBMENU_TABLE_SZ,
                            utils_submenu_secondary_items);
    menu(utils_submenup, utils_submenu_secondary_items, '\0');
    return PASSED;
}

/* Power submenu items */

static int switzer_10g_device_init(struct switzer_10g *mod);
static void switzer_10g_device_exit(struct switzer_10g *mod);
static int switzer_10g_sock_test(void);

static long power_on(void)
{
    prt("\nPower On the module.\n");

    if (switzer_ltc4215_power_on()) {
        log_warn("ltc4215 power on failed\n");
        return FAILED;
    }

    if (switzer_10g_device_init(mod) < 0)
        return FAILED;
    return PASSED;
}

static long power_off(void)
{
    uint8_t ans;

    prt("\nProceed with Power Off? (y/n) ");
    ans = getchar();
    putchar(ans);
    prt("\n");
    if (ans != 'y' && ans != 'Y') {
        prt("\nPower Off ABORT!\n");
        return PASSED;
    }
    switzer_10g_device_exit(mod);
    switzer_ltc4215_power_off();
    return PASSED;
}

#define VM_OUT_1_0_ADDR 0xFB
#define VM_OUT_1_2_ADDR 0xFA
#define VM_OUT_3_3_ADDR 0xF8

#define VM_DATA_STEP 8
#define VM_DATA_MAX 64
#define VM_DATA_MASK 0x7F

#define VM_MAX_PERCENT 5

#define VM_DELAY_M 200

#define VM_SOURCE_FLAG 0x00
#define VM_SINK_FLAG (0x01 << 7)

#define VM_CALCULATE_COEFFICIENT 10

static double DS4424_data_to_voltage(uint32_t basevoltage, uint8_t vmdata)
{
    double vl;

    if ((vmdata & ~VM_DATA_MASK) == VM_SOURCE_FLAG) {
        vl = basevoltage * (100.0 + (double)(vmdata & VM_DATA_MASK) * VM_MAX_PERCENT / VM_DATA_MAX) / 1000.0;
    } else {
        vl = basevoltage * (100.0 - (double)(vmdata & VM_DATA_MASK) * VM_MAX_PERCENT / VM_DATA_MAX) / 1000.0;
    }

    return vl;
}

static uint32_t DS4424_get_base_voltage(uint8_t addr)
{
    uint32_t basevoltage;

    switch (addr) {
    case VM_OUT_1_0_ADDR:
        basevoltage = 10;
        break;
    case VM_OUT_1_2_ADDR:
        basevoltage = 12;
        break;
    case VM_OUT_3_3_ADDR:
        basevoltage = 33;
        break;
    default:
        basevoltage = 0;
        break;
    }

    return basevoltage;
}

static long __power_set_vmarg(uint8_t addr, uint8_t predata, uint8_t tagdata)
{
    uint8_t predataflag, tagdataflag;                      //bit7,source or sink flag
    uint32_t basevoltage;

    if ((tagdata & VM_DATA_MASK) > VM_DATA_MAX) {
        cterr('f', 0, "vmdata out of range");
        return FAILED;
    }

    predataflag = predata & ~VM_DATA_MASK;
    tagdataflag = tagdata & ~VM_DATA_MASK;

    if (predataflag == tagdataflag) {
        basevoltage = DS4424_get_base_voltage(addr);
        if (basevoltage == 0) {
            cterr('f', 0, "wrong addr param %02x", addr);
            return FAILED;
        }

        do {
            /* Set voltage from predata to tagdata using step VM_DATA_STEP */
            if (predata < tagdata - VM_DATA_STEP)
                predata += VM_DATA_STEP;
            else if (predata > tagdata + VM_DATA_STEP)
                predata -= VM_DATA_STEP;
            else
                predata = tagdata;

            if (switzer_i2c_slave_write(mod->pm.i2c, addr, &predata, sizeof(predata)) < 0) {
                cterr_add_component("DS4424 on Switzer",
                                    "I2C controller in the FPGA");
                cterr_add_debug("DS4424 on Switzer",
                                "Check the I2C controller in the FPGA");
                cterr('f', 0, "Set DS4424 voltage failed, data = %d", predata);
                return FAILED;
            }

            prt("change voltage to %0.4fV\n", DS4424_data_to_voltage(basevoltage, predata));

            switzer_mdelay(VM_DELAY_M);
        } while (predata != tagdata);
    } else {
        //First set predata to 0, then to tagdata
        if (__power_set_vmarg(addr, predata, 0 | predataflag) == FAILED)
            return FAILED;

        if (__power_set_vmarg(addr, 0 | tagdataflag, tagdata) == FAILED)
            return FAILED;
    }

    return PASSED;
}

static long power_set_per_vmarg(uint8_t addr, uint32_t per, uint8_t ssflag)
{
    uint8_t predata, tagdata;
    uint32_t tmpdata;

    if (per > VM_MAX_PERCENT) {
        cterr('f', 0, "Set percent out of range");
        return FAILED;
    }

    if (switzer_i2c_slave_read(mod->pm.i2c, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }

    tmpdata = per * VM_DATA_MAX * VM_CALCULATE_COEFFICIENT / VM_MAX_PERCENT;
    if (tmpdata % VM_CALCULATE_COEFFICIENT >= 5) {
        tagdata = (uint8_t)(tmpdata / VM_CALCULATE_COEFFICIENT + 1);
    } else {
        tagdata = (uint8_t)(tmpdata / VM_CALCULATE_COEFFICIENT);
    }

    tagdata |= ssflag;

    return __power_set_vmarg(addr, predata, tagdata);
}

static long power_auto_set_vmarg(uint8_t addr, switzer_vmarg_t vmarg)
{
    uint32_t per;
    uint8_t ssflag;

    switch (vmarg) {
    case SWITZER_VMARG_NORMAL:
        per = 0;
        ssflag = VM_SOURCE_FLAG;
        break;
    case SWITZER_VMARG_LOW:
        per = VM_MAX_PERCENT;
        ssflag = VM_SINK_FLAG;
        break;
    case SWITZER_VMARG_HIGH:
        per = VM_MAX_PERCENT;
        ssflag = VM_SOURCE_FLAG;
        break;
    default:
        return FAILED;
    }

    return power_set_per_vmarg(addr, per, ssflag);
}

static long power_user_set_vmarg(void)
{
    uint8_t predata, addr, ssflag;
    uint32_t basevoltage, per, anr;

    anr = getdec_answer("Choose a current source (0 for 1.0V, 1 for 1.2V, 2 for 3.3V)", 0, 0, 2);
    switch (anr) {
    case 0:
        addr = VM_OUT_1_0_ADDR;
        break;
    case 1:
        addr = VM_OUT_1_2_ADDR;
        break;
    case 2:
        addr = VM_OUT_3_3_ADDR;
        break;
    default:
        return FAILED;
    }

    basevoltage = DS4424_get_base_voltage(addr);
    if (switzer_i2c_slave_read(mod->pm.i2c, addr, &predata, sizeof(predata)) < 0) {
        cterr_add_component("DS4424 on Switzer",
                            "I2C controller in the FPGA");
        cterr_add_debug("DS4424 on Switzer",
                        "Check the I2C controller in the FPGA");
        cterr('f', 0, "Read DS4424 voltage failed");
        return FAILED;
    }
    prt("\nPresent voltage is %0.3fV", DS4424_data_to_voltage(basevoltage, predata));

    anr = getdec_answer("Source or Sink? (0 for sink, 1 for source)", 0, 0, 1);
    switch (anr) {
    case 0:
        ssflag = VM_SINK_FLAG;
        break;
    case 1:
        ssflag = VM_SOURCE_FLAG;
        break;
    default:
        return FAILED;
    }

    per = getdec_answer("Set percent(0 to 5)", 0, 0, 5);

    return power_set_per_vmarg(addr, per, ssflag);
}

static long power_set_1_0_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(VM_OUT_1_0_ADDR, vmarg);
}

static long power_set_1_2_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(VM_OUT_1_2_ADDR, vmarg);
}

static long power_set_3_3_vmarg(switzer_vmarg_t vmarg)
{
    return power_auto_set_vmarg(VM_OUT_3_3_ADDR, vmarg);
}

static submenu_xtable_t power_submenu_table[] = {
    {"Power Status", switzer_ltc4215_power_info, 0, 0, NULL, 0, NULL, 0},
    {"Power Off", power_off, 0, 0, NULL, 0, NULL, 0},
    {"Power On", power_on, 0, 0, NULL, 0, NULL, 0},
    {"Set 1.0V to Normal",	(PFT)power_set_1_0_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 1.0V to Margin High",	(PFT)power_set_1_0_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 1.0V to Margin Low", (PFT)power_set_1_0_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Set 1.2V to Normal",	(PFT)power_set_1_2_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 1.2V to Margin High",	(PFT)power_set_1_2_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 1.2V to Margin Low", (PFT)power_set_1_2_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"Set 3.3V to Normal",	(PFT)power_set_3_3_vmarg, SWITZER_VMARG_NORMAL,
     0, NULL, 0, NULL, 0},
    {"Set 3.3V to Margin High",	(PFT)power_set_3_3_vmarg, SWITZER_VMARG_HIGH,
     0, NULL, 0, NULL, 0},
    {"Set 3.3V to Margin Low", (PFT)power_set_3_3_vmarg, SWITZER_VMARG_LOW,
     0, NULL, 0, NULL, 0},
    {"User Set", (PFT)power_user_set_vmarg, 0, 0, NULL, 0, NULL, 0},
};

#define POWER_SUBMENU_TABLE_SZ (sizeof(power_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t power_submenu_primary_items[POWER_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t power_submenu_secondary_items[POWER_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char power_submenu_title[] = "Switzer 10G Power Utilities Menu";

static menuinfo_t power_submenu = {
    power_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    power_submenu_primary_items,
};

static menuinfo_t *power_submenup = &power_submenu;

static long power_utils(void)
{
    build_primary_submenu(power_submenu_table, POWER_SUBMENU_TABLE_SZ,
                          power_submenu_title, &power_submenup);
    build_secondary_submenu(power_submenu_table, POWER_SUBMENU_TABLE_SZ,
                            power_submenu_secondary_items);
    menu(power_submenup, power_submenu_secondary_items, '\0');
    return PASSED;
}

static long power_led_util(void)
{
    int led_color;
    led_color = getdec_answer("led color:\n"
            "0 : led off\n"
            "1 : led amber\n"
            "2 : led green\n"
            "3 : led amber only\n"
            "4 : led green only\n"
            "5 : led amber off\n"
            "6 : led green off", 0, 0, 6);
    if (switzer_power_led_util(led_color)) {
        cterr('f', 0, "power led util fialed");
        return FAILED;
    }
    return PASSED;
}

static long sfp_led_util(void)
{
    int led_color;
    led_color = getdec_answer("led color:\n"
            "0 : led off\n"
            "1 : led green\n"
            "3 : led amber" , 0, 0, 3);
    if (switzer_10g_led_set(mod, led_color) < 0) {
        cterr_add_component("SFP LED",
                            "GPIO on BCM82757");
        cterr_add_debug("Check SFP LED",
                        "Check GPIO on BCM82757");
        cterr('f', 0, "SFP LED Test failed");
        return FAILED;
    }
    return PASSED;
}

static submenu_xtable_t led_submenu_table[] = {
    {"Power Led Util", power_led_util, 0, 0, NULL, 0, NULL, 0},
    {"SFP Led Util", sfp_led_util, 0, 0, NULL, 0, NULL, 0},
};

#define LED_SUBMENU_TABLE_SZ (sizeof(led_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t led_submenu_primary_items[LED_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t led_submenu_secondary_items[LED_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char led_submenu_title[] = "Switzer 10G Led Utilities Menu";

static menuinfo_t led_submenu = {
    led_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    led_submenu_primary_items,
};
static menuinfo_t *led_submenup = &led_submenu;

static long led_utils(void)
{
    build_primary_submenu(led_submenu_table, LED_SUBMENU_TABLE_SZ,
                          led_submenu_title, &led_submenup);
    build_secondary_submenu(led_submenu_table, LED_SUBMENU_TABLE_SZ,
                            led_submenu_secondary_items);
    menu(led_submenup, led_submenu_secondary_items, '\0');
    return PASSED;
}

static long phy_status_dump(void)
{
    int rc;
    switzer_if_side_t if_side;
    unsigned int flags;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    flags = gethex_answer("Enter Dump Flags", 0, 0, 0xffffffff);

    mod->miura.info.flags = flags;
    rc = switzer_10g_phy_dump(mod, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 dump error");
        return FAILED;
    }
    return PASSED;
}

static long phy_mac_diagnostic_dump(void)
{
    int rc;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_10g_phy_mac_dump(mod, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 dump error");
        return FAILED;
    }
    return PASSED;
}

static long phy_link_status(void)
{
    int rc;
    unsigned int link_status;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_10g_phy_link_status(mod, if_side, &link_status);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 link status get error");
        return FAILED;
    }
    prt("phy link status: %d\n",link_status);
    return PASSED;
}

#define PHY_LINK_UP_DELAY 2000

static int get_phy_link(switzer_if_side_t if_side, unsigned int *link_status)
{
    int i, rc;

    for (i = 0; i < 12; i++) {
        rc = switzer_10g_phy_link_status(mod, if_side, link_status);
        if (rc < 0) {
            cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
            cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
            cterr('f', 0, "BCM82757 link status get error");
            return FAILED;
        }
        if (*link_status) {
            break;
        }
        mdelay(PHY_LINK_UP_DELAY);
    }
    return PASSED;
}

static long phy_firmware_download(void)
{
    struct switzer_miura *miura = &mod->miura;

    switzer_miura_reset(miura);

    if (switzer_miura_fw_download(miura)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_loopback(void)
{
    switzer_if_side_t if_side;
    unsigned int lb_mode, enable;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    lb_mode = gethex_answer("Enter Loopback mode", 1, 0, 10);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (switzer_10g_miura_loopback_set(mod, if_side, lb_mode, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_prbs(void)
{
    switzer_if_side_t if_side;
    unsigned int action, poly, enable = 0;
    switzer_prbs_t prbs = SWITZER_PRBS_31;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (if_side == SWITZER_IF_SIDE_SYS) {
        cterr_add_component("BCM82757",
                            "Serdes link on BCM82757 system side");
        cterr_add_debug("Check BCM82757",
                        "Check Serdes link on BCM82757 system side");
    } else {
        cterr_add_component("BCM82757",
                            "Serdes link on BCM82757 line side");
        cterr_add_debug("Check BCM82757",
                        "Check Serdes link on BCM82757 line side");
    }
    action = gethex_answer("Enter Action(Check:0, Enable:1, Disable:2)",
                           0, 0, 2);

    switch (action) {
    default:
    case 0:
        if (switzer_10g_miura_prbs_check(mod, if_side)) {
            cterr('f', 0, "BCM82757 PRBS check failed");
            return FAILED;
        }
        break;

    case 1:
        poly = getdec_answer("PRBS Polynomial(7, 9, 11, 15, 23, 31)", 31, 0, 31);
        switch (poly) {
        case 7:
            prbs = SWITZER_PRBS_7;
            break;
        case 9:
            prbs = SWITZER_PRBS_9;
            break;
        case 11:
            prbs = SWITZER_PRBS_11;
            break;
        case 15:
            prbs = SWITZER_PRBS_15;
            break;
        case 23:
            prbs = SWITZER_PRBS_23;
            break;
        default:
        case 31:
            prbs = SWITZER_PRBS_31;
            break;
        }
        enable = 1;
    case 2:
        if (switzer_10g_miura_prbs_set(mod, if_side, prbs, enable)) {
            cterr('f', 0, "BCM82757 PRBS set failed");
            return FAILED;
        }
        break;
    }

    return PASSED;
}

static long phy_firmware_lane_config_set(void)
{
    switzer_if_side_t if_side;
    bcm_plp_pm_firmware_lane_config_t firmware_lane_config;

    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);
    if (switzer_10g_miura_firmware_lane_get(mod, if_side, &firmware_lane_config)) {
        cterr('f', 0, "switzer firmware lane get failed");
        return FAILED;
    }
    firmware_lane_config.firmware_mode = gethex_answer(
                           "0 : default\n"
                           "1 : dfe mode\n"
                           "2 : osdfe mode\n"
                           "3 : baud rate dfe mode\n"
                           "4 : low power dfe mode\n"
                           "5 : media type sfp dac\n"
                           "6 : media type xlaui\n"
                           "7 : media type optical sr4\n", 0, 0, 7);
    firmware_lane_config.ena_dis = gethex_answer(
                           "ena_dis(Disable:0, Enable:1)", 1, 0, 1);
    firmware_lane_config.AnEnabled = gethex_answer(
                           "Autonego( Disable:0, Enable:1)", 1, 0, 1);
    firmware_lane_config.MediaType = gethex_answer(
                           "MediaType(PcbBackTrace:0, Copper:1, Optics:2)", 1, 0, 2);
    firmware_lane_config.DbLoss = gethex_answer(
                           "DbLossValue(0 - 100)", 1, 0, 2);

    if (switzer_10g_miura_firmware_lane_set(mod, if_side, &firmware_lane_config)) {
        cterr('f', 0, "switzer firmware lane set failed");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_cl73(void)
{
    switzer_if_side_t if_side;
    unsigned int enable;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (switzer_10g_miura_cl73_set(mod, if_side, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    return PASSED;
}

static long phy_config_macsec_bypass(void)
{
    int speed_flag, speed;
    speed_flag = gethex_answer("Speed(10G:0, 1G:1)", 0, 0, 1);

    if (speed_flag) {
        speed = SWITZER_PORT_SPEED_1G;
    } else {
        speed = SWITZER_PORT_SPEED_10G;
    }
    if (switzer_10g_miura_config_macsec_bypass(mod, speed)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    return PASSED;
}

static long phy_display_eye_scan(void)
{
    int rc;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = switzer_10g_display_eye_scan(mod, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 eye scan failed");
        return FAILED;
    }
    return PASSED;
}

static const reg_info_t phy_miura_reg_tbl[] = {
    {"General Purpose Register 1C", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Cr,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1D", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Dr,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1E", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Er,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1F", BCMI_MIURA_DIRECT_GEN_CNTRLS_GPREG_1Fr,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"End of General Purpose Register", 0, 0, {0}, 0, 0},
};

static int __phy_register_test(const reg_info_t *regs, switzer_if_side_t if_side)
{
    uint32_t i;
    uint32_t regaddr, devaddr;
    uint32_t data, data_orig, data_test;

    while (regs->size.size != 0) {
        regaddr = regs->offset;
        devaddr = regaddr >> 16;
        if (switzer_10g_phy_read(mod, if_side, devaddr, regaddr, &data_orig) < 0) {
            cterr('f', 0, "Error reading %s register offset %d.%#x",
                  regs->name, devaddr, regaddr);
            return FAILED;
        }

        if (regs->type == READ_WRITE) {
            /* ripple 1 test */
            for (i = 0; i < regs->size.size * 8; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                /* Write to register under test */
                if (switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data_test) < 0 ||
                    switzer_10g_phy_read(mod, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Ripple one test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
            }

            /* ripple 0 test */
            for (i = 0; i < regs->size.size * 8; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                data_test = (~(1 << i)) & regs->mask;
                /* Write to register under test */
                if (switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data_test) < 0 ||
                    switzer_10g_phy_read(mod, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Ripple zero test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
            }

            /* pattern test */
            data = 0x5adb;
            for (i = 0; i < 2; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                /* Write to register under test */
                if (switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data_test) < 0 ||
                    switzer_10g_phy_read(mod, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Pattern test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
                data = ~data;   /* complement data pattern */
            }

            /* restore original value */
            if (switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data_test) < 0) {
                cterr('f', 0, "Error restoring %s register offset %d.%#x",
                      regs->name, devaddr, regaddr);
                return FAILED;
            }
        }
        regs++;
    }

    return PASSED;
}

static long phy_register_test(void)
{
    switzer_if_side_t if_side = SWITZER_IF_SIDE_SYS;

    prpass(testpass, "BCM82757 Register Test ");

    if (__phy_register_test(phy_miura_reg_tbl, if_side) == FAILED) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "Register Test on BCM82757 failed");
        return FAILED;
    }
    return PASSED;
}

static long phy_intr_test(void)
{
    int rc, ret = FAILED;
    uint32_t reval, data, offset, devaddr = SWITZER_MIURA_DEV_PMA_PMD;
    uint32_t regaddr = BCMI_MIURA_DIRECT_PAD_CNTRL_LASI_0_CONTROLr;
    switzer_if_side_t if_side = SWITZER_IF_SIDE_LINE;
    prpass(testpass, "BCM82757 Interrupt Test ");

    /* read the current phy reg value*/
    rc = switzer_10g_phy_read(mod, if_side, devaddr, regaddr, &reval);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 read error");
        return FAILED;
    }

    /* reset phy reg */
    data = 0x8460;
    rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 write error");
        return FAILED;
    }

    /* check fpga intr reg */
    offset = 0x010C;
    data = *(volatile uint32_t *)(mod->fpga.map.vaddr + offset);
    if (data & 0x00000001) {
        cterr_add_component("BCM82757",
                            "LASI_0 signal input with the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check LASI_0 signal input with the FPGA");
        cterr('f', 0, "BCM82757 interrupt error");
        return FAILED;
    }

    /* Force output interrupt */
    data = 0x0E60;
    rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 write error");
        return FAILED;
    }

    /* check fpga intr reg */
    data = *(volatile uint32_t *)(mod->fpga.map.vaddr + offset);
    if (data & 0x00000001) {
        ret = PASSED;
    } else {
        cterr_add_component("BCM82757",
                            "LASI_0 signal input with the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check LASI_0 signal input with the FPGA");
        cterr('f', 0, "BCM82757 interrupt error");
    }

    /* restore phy reg */
    rc = switzer_10g_phy_write(mod, if_side, devaddr, regaddr, reval);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 write error");
        return FAILED;
    }

    return ret;
}

static int switzer_10g_get_eth_port(struct switzer_10g *mod)
{
    int ctrl_plane_port   = -1;
#if (defined CURIE_1RU) || (defined CURIE_2RU)
    unsigned int mod_type = -1;
    int          slot     = -1;
    int          pc_slot  = -1;

    if (!mod || !mod->ngio) {
        log_err("Invalid argument\n");
        return ctrl_plane_port;
    }

    mod_type = mod->ngio->mod_type;
    slot     = mod->ngio->slot;
    pc_slot  = mod->ngio->pc ? mod->ngio->pc->slot : -1;

    if (is_curie_1ru()) {
       /* curie 1ru NIM1 GE0=eth7, no GE1; SM1 GE0=eth6, GE1=eth8 */
       switch (mod_type) {
       case WIC_MODULE:
           ctrl_plane_port = CPU_SGMII_PORT7;
	   break;
       case DAUGHTER_CARD:
           switch (slot) {
           case 1:
               ctrl_plane_port = CPU_SGMII_PORT6;
               break;
           case 2:
               ctrl_plane_port = CPU_SGMII_PORT8;
               break;
           }
	   break;
       }
    } else if (is_curie_2ru()) {
        /* curie 2ru NIM1 GE0=eth7, no GE1; NIM2 GE0=eth6, no GE1
         * SM1 GE0=eth11, GE1=eth9, SM2 GE0=eth10, GE1=eth8 */
        switch (mod_type) {
        case WIC_MODULE:
            switch (slot) {
            case NGWIC1_SLOT:
                ctrl_plane_port = CPU_SGMII_PORT7;
                break;
            case NGWIC2_SLOT:
                ctrl_plane_port = CPU_SGMII_PORT6;
                break;
            }
            break;
        case DAUGHTER_CARD:
            switch (pc_slot){
            case NGSM1_SLOT:
                switch (slot) {
                case 1:
                    ctrl_plane_port = CPU_SGMII_PORT11;
                    break;
                case 2:
                    ctrl_plane_port = CPU_SGMII_PORT9;
                    break;
                }
                break;
            case NGSM2_SLOT:
                switch (slot) {
                case 1:
                    ctrl_plane_port = CPU_SGMII_PORT10;
                    break;
                case 2:
                    ctrl_plane_port = CPU_SGMII_PORT8;
                    break;
                }
                break;
            }
            break;
        }
    }
#elif defined TABEIL
    /* TODO */
#endif
    return ctrl_plane_port;
}

static long phy_internal_lpbk_test(int speed)
{
    unsigned int sys_link;
    int slot = mod->ngio->slot;
    unsigned int module_type = mod->ngio->mod_type;
    ngio_eth_speed_t new_speed, old_speed;
    int eth_port;
    char cmd[32];

    prpass(testpass, "PHY Internal Loopback Test, speed: %d ", speed);

    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");

    if (switzer_10g_miura_config_macsec_bypass(mod, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }

    if (switzer_10g_miura_loopback_set(mod, SWITZER_IF_SIDE_LINE, 1, 1)) {
        cterr('f', 0, "set loopback failed");
        return FAILED;
    }

    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_10g_miura_cl73_set(mod, SWITZER_IF_SIDE_SYS, 1)) {
            cterr('f', 0, "set cl73 failed");
            return FAILED;
        }
    } else {
        /* need force ngio eth to 1G in 1G lpbk test */
        if (is_curie_2ru()) {
            new_speed = NGIO_ETH_SPEED_1G;
            ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);
        }
    }

    if (get_phy_link(SWITZER_IF_SIDE_SYS, &sys_link)) {
        cterr('f', 0, "get phy link failed");
        return FAILED;
    }
    if (!sys_link) {
        prt("sys_link %d\n", sys_link);
        cterr('f', 0, "switzer link err");
        return FAILED;
    }
    mdelay(PHY_LINK_UP_DELAY);

    if (0 <= (eth_port = switzer_10g_get_eth_port(mod))) {
        sprintf(cmd, "ethtool -S eth%d", eth_port);
        if ((NVRAM)->diagflag & D_VERBOSE)
            system(cmd);
    } else {
        cmd[0] = 0;
    }

    if (switzer_10g_sock_test()) {
        mod->miura.info.flags = 0;
        switzer_10g_phy_dump(mod, 1);
        switzer_10g_display_eye_scan(mod, 1);
        cterr('f', 0, "switzer socket test failed");
        return FAILED;
    }
    if (((NVRAM)->diagflag & D_VERBOSE) && cmd[0])
        system(cmd);

    if (switzer_10g_miura_loopback_set(mod, SWITZER_IF_SIDE_LINE, 1, 0)) {
        cterr('f', 0, "set loopback failed");
        return FAILED;
    }

    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_10g_miura_cl73_set(mod, SWITZER_IF_SIDE_SYS, 0)) {
            cterr('f', 0, "set cl73 failed");
            return FAILED;
        }
    } else {
        if (is_curie_2ru()) {
            ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);
        }
    }

    switzer_10g_miura_config_macsec_cleanup(mod);
    return PASSED;
}

static long phy_external_lpbk_test(int speed)
{
    unsigned int sys_link, line_link;
    int slot = mod->ngio->slot;
    unsigned int module_type = mod->ngio->mod_type;
    ngio_eth_speed_t new_speed, old_speed;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the BCM82757 external loopback test\n");
        return PASSED;
    }

    prpass(testpass, "BCM82757 External Loopback Test, speed: %d ", speed);

    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");
    if (switzer_10g_miura_config_macsec_bypass(mod, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }
    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_10g_miura_cl73_set(mod, SWITZER_IF_SIDE_SYS, 1)) {
            cterr('f', 0, "set cl73 failed");
            return FAILED;
        }
    } else {
        /* need force ngio eth to 1G in 1G lpbk test */
        if (is_curie_2ru()) {
            new_speed = NGIO_ETH_SPEED_1G;
            ngio_cfg_eth_port_speed(module_type, slot, &new_speed, &old_speed);
        }
    }

    if (get_phy_link(SWITZER_IF_SIDE_LINE, &line_link)) {
        cterr('f', 0, "get phy link failed");
        return FAILED;
    }
    if (get_phy_link(SWITZER_IF_SIDE_SYS, &sys_link)) {
        cterr('f', 0, "get phy link failed");
        return FAILED;
    }
    if (!sys_link || !line_link) {
        prt("sys_link %d, line_link %d\n", sys_link, line_link);
        cterr('f', 0, "switzer link err");
        return FAILED;
    }
    mdelay(PHY_LINK_UP_DELAY);

    if (switzer_10g_sock_test()) {
        cterr('f', 0, "switzer socket test failed");
        return FAILED;
    }

    if (speed == SWITZER_PORT_SPEED_10G) {
        if (switzer_10g_miura_cl73_set(mod, SWITZER_IF_SIDE_SYS, 0)) {
            cterr('f', 0, "set cl73 failed");
            return FAILED;
        }
    } else {
        if (is_curie_2ru()) {
            ngio_cfg_eth_port_speed(module_type, slot, &old_speed, NULL);
        }
    }
    switzer_10g_miura_config_macsec_cleanup(mod);
    return PASSED;
}

#define PRBS_TEST_DELAY 1

static long __phy_prbs_test(switzer_if_side_t if_side, switzer_prbs_t prbs, uint32_t delay_sec)
{
    uint32_t enable = 1;

    /* enable prbs */
    if (switzer_10g_miura_prbs_set(mod, if_side, prbs, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 PRBS set enable failed");
        return FAILED;
    }

    /* wait for prbs lock */
    switzer_mdelay(2000);
    /* clear prbs rx stat */
    if (switzer_10g_prbs_clear_rx_stat(mod, if_side)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller with the FPGA");
        cterr('f', 0, "BCM82757 PRBS clear rx stat failed");
    }

    /* check prbs */
    switzer_mdelay(delay_sec * 1000);
    if (switzer_10g_miura_prbs_check(mod, if_side)) {
        cterr_add_component("BCM82757",
                            "SFP plugin link status");
        cterr_add_debug("Check BCM82757",
                        "Check the SFP plugin link status");
        cterr('f', 0, "BCM82757 PRBS check failed");
        return FAILED;
    }

    /* disable prbs */
    enable = 0;
    if (switzer_10g_miura_prbs_set(mod, if_side, prbs, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        cterr('f', 0, "BCM82757 PRBS set disable failed");
        return FAILED;
    }

    return PASSED;
}

static long phy_prbs_test(switzer_if_side_t if_side)
{
    switzer_prbs_t prbs = SWITZER_PRBS_7;

    if (if_side == SWITZER_IF_SIDE_SYS) {
        prpass(testpass, "PHY SYS PRBS Test ");
    } else {
        if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            printf("External loopback flag is off, skip the line side prbs test\n");
            return PASSED;
        }

        prpass(testpass, "PHY LINE PRBS Test ");
    }

    if (switzer_10g_miura_config_macsec_bypass(mod, SWITZER_PORT_SPEED_10G)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }

    while (prbs <= SWITZER_PRBS_31) {
        if (__phy_prbs_test(if_side, prbs, PRBS_TEST_DELAY) == FAILED)
            return FAILED;
        prbs++;
    }

    switzer_10g_miura_config_macsec_cleanup(mod);

    return PASSED;
}

static void phy_show_tx_param(switzer_if_side_t if_side, bcm_plp_tx_t *tx_param) {
    if (if_side == SWITZER_IF_SIDE_SYS) {
        printf("\nsys side tx param\n"
               "pretap  : %d\n"
               "maintap : %d\n"
               "post    : %d\n"
               "post2   : %d\n"
               "post3   : %d\n"
               "amp     : %d\n\n",
               tx_param->pre, tx_param->main, tx_param->post, tx_param->post2,
               tx_param->post3, tx_param->amp);
    } else {
        printf("\nline side tx param\n"
               "pretap  : %d\n"
               "maintap : %d\n"
               "post    : %d\n"
               "post2   : %d\n\n",
               tx_param->pre, tx_param->main, tx_param->post, tx_param->post2);
    }
}

static long phy_tx_get()
{
    bcm_plp_tx_t tx_param;
    switzer_if_side_t if_side;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);

    if (switzer_10g_miura_tx_get(mod, if_side, &tx_param)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    phy_show_tx_param(if_side, &tx_param);

    return PASSED;
}

static long phy_tx_set()
{
    bcm_plp_tx_t tx_param;
    switzer_if_side_t if_side;
    int answ = 1;

    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);

    if (switzer_10g_miura_tx_get(mod, if_side, &tx_param)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    phy_show_tx_param(if_side, &tx_param);

    while (answ) {
        if (if_side == SWITZER_IF_SIDE_SYS) {
            answ = gethex_answer("Enter Tap(pre:1, main:2, post:3, "
                                 "post2:4, post3:5, amp:6, exit:0)", 0, 0, 6);
        } else {
            answ = gethex_answer("Enter Tap(pre:1, main:2, post:3, "
                                 "post2:4, exit:0)", 0, 0, 4);
        }

        switch (answ) {
        case 1:
            tx_param.pre = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 2:
            tx_param.main = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 3:
            tx_param.post = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 4:
            tx_param.post2 = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 5:
            tx_param.post3 = getdec_answer("Enter Value", 0, 0, 255);
            break;
        case 6:
            tx_param.amp = getdec_answer("Enter Value", 0, 0, 255);
            break;
        default:
        continue;
            break;
        }

        if (switzer_10g_miura_tx_set(mod, if_side, &tx_param)) {
            cterr_add_component("BCM82757",
                                "MDIO controller within the FPGA");
            cterr_add_debug("Check BCM82757",
                            "Check MDIO controller within the FPGA");
            return FAILED;
        }
        phy_show_tx_param(if_side, &tx_param);
    }
    return PASSED;
}

/* PHY Utils submenu items */
static submenu_xtable_t phy_utils_submenu_table[] = {
    {"Broadcom PHY Register Read", utils_phy_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Register Write", utils_phy_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Autoneg Remote Ability Get", phy_autoneg_remote_ability_get, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Registers dump", phy_registers_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Status dump", phy_status_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY MAC Diagnostic Dump", phy_mac_diagnostic_dump, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY link status get", phy_link_status, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Firmware Download", phy_firmware_download, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Display Eye Scan", phy_display_eye_scan, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Loopback", phy_config_loopback, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config PRBS", phy_config_prbs, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Firmware Lane", phy_firmware_lane_config_set, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Clause 73", phy_config_cl73, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY Config Macsec Bypass", phy_config_macsec_bypass, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY TX Config Get", phy_tx_get, 0,
     0, NULL, 0, NULL, 0},
    {"Broadcom PHY TX Config Set", phy_tx_set, 0,
     0, NULL, 0, NULL, 0},
};

#define PHY_UTILS_SUBMENU_TABLE_SZ (sizeof(phy_utils_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t phy_utils_submenu_primary_items[PHY_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t phy_utils_submenu_secondary_items[PHY_UTILS_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char phy_utils_submenu_title[] = "Switzer 10G PHY Utilities Menu";

static menuinfo_t phy_utils_submenu = {
    phy_utils_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    phy_utils_submenu_primary_items,
};

static menuinfo_t *phy_utils_submenup = &phy_utils_submenu;

static long phy_utils(int show_menu)
{
    build_primary_submenu(phy_utils_submenu_table, PHY_UTILS_SUBMENU_TABLE_SZ,
                          phy_utils_submenu_title, &phy_utils_submenup);
    build_secondary_submenu(phy_utils_submenu_table, PHY_UTILS_SUBMENU_TABLE_SZ,
                            phy_utils_submenu_secondary_items);
    menu(phy_utils_submenup, phy_utils_submenu_secondary_items, '\0');
    return PASSED;
}

/* PHY submenu items */
static submenu_xtable_t phy_submenu_table[] = {
    {"Broadcom PHY Utilities", phy_utils, 0,
     0, NULL, 0, phy_utils, 0},
    {"Broadcom PHY Register Test", phy_register_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Broadcom PHY Interrupt Test", phy_intr_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Broadcom PHY Line Side PRBS Test", phy_prbs_test, SWITZER_IF_SIDE_LINE,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Broadcom PHY 10G Internal Loopback Test", phy_internal_lpbk_test, SWITZER_PORT_SPEED_10G,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Broadcom PHY 10G External loopback Test", phy_external_lpbk_test, SWITZER_PORT_SPEED_10G,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Broadcom PHY 1G Internal Loopback Test", phy_internal_lpbk_test, SWITZER_PORT_SPEED_1G,
     F_ALL_E, NULL, 0, NULL, 0},
    {"Broadcom PHY 1G External loopback Test", phy_external_lpbk_test, SWITZER_PORT_SPEED_1G,
     F_ALL_E, NULL, 0, NULL, 0},
};

#define PHY_SUBMENU_TABLE_SZ (sizeof(phy_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t phy_submenu_primary_items[PHY_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t phy_submenu_secondary_items[PHY_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char phy_submenu_title[] = "Switzer 10G PHY Subtest Menu";

static menuinfo_t phy_submenu = {
    phy_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    phy_submenu_primary_items,
};

static menuinfo_t *phy_submenup = &phy_submenu;

static long phy_test(int show_menu)
{
    build_primary_submenu(phy_submenu_table, PHY_SUBMENU_TABLE_SZ,
                          phy_submenu_title, &phy_submenup);
    build_secondary_submenu(phy_submenu_table, PHY_SUBMENU_TABLE_SZ,
                            phy_submenu_secondary_items);
    if (show_menu)
        menu(phy_submenup, phy_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(phy_submenup);
    return PASSED;
}

static int fpga_reg_read(unsigned long addr, int size,
                         unsigned long *buf, void *param)
{
    *(uint32_t *)buf = *(volatile uint32_t *)(mod->fpga.map.vaddr + addr);
    return PASSED;
}

static int fpga_reg_write(unsigned long addr, int size,
                          unsigned long data, void *param)
{
    *(volatile uint32_t *)(mod->fpga.map.vaddr + addr) = (uint32_t)data;
    return PASSED;
}

static reg_info_t_ext fpga_reg_ext = {4, fpga_reg_read, fpga_reg_write, 0};

static reg_info_t fpga_reg_tbl[] =
{
    /* Register name, Offset, Type, Size, Mask, Reset Value */
    {"GENERAL_SCRATCH_PAD", 0x8,
     READ_WRITE | SAVE_RESTORE | REG_ACCESS, {(ulong)&fpga_reg_ext},
     0xffffffff, 0},
};

static long fpga_reg_test(void)
{
    prpass(testpass, "FPGA Register Test ");
    cterr_add_component("FPGA",
                        "PCIe bus");
    cterr_add_debug("Check FPGA",
                    "Check PCIe link status");
    return register_tests(0, fpga_reg_tbl);
}

static long sfp_led_test(void)
{
    prpass(testpass, "SFP LED Test ");

    if (switzer_10g_led_set(mod, SWITZER_LED_AMBER) < 0)
        goto err;
    mdelay(500);
    if (switzer_10g_led_set(mod, SWITZER_LED_GREEN) < 0)
        goto err;
    mdelay(500);
    if (switzer_10g_led_set(mod, SWITZER_LED_OFF) < 0)
        goto err;

    return PASSED;
err:
    cterr_add_component("SFP LED",
                        "GPIO on BCM82757");
    cterr_add_debug("Check SFP LED",
                    "Check GPIO on BCM82757");
    cterr('f', 0, "SFP LED Test failed");
    return FAILED;
}

static long spi_flash_test(void)
{
    return switzer_utils_spi_prom_test(mod->spi.prom);
}

/* Main menu items */
static submenu_xtable_t main_menu_table[] = {
    {"IO Utilities", io_utils, 0,
     0, NULL, 0, io_utils, 0},
    {"Power Utilities", power_utils, 0,
     0, NULL, 0, power_utils, 0},
    {"Led Utilities", led_utils, 0,
     0, NULL, 0, led_utils, 0},
    {"PHY Test", phy_test, 0,
     F_ALL, NULL, 0, phy_test, 1},
    {"FPGA Register Test", fpga_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"LTC4215 Register Test", switzer_ltc4215_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"DS4424 Register Test", ds4424_reg_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"SFP LED Test", sfp_led_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"SPI flash Test", spi_flash_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
};

#define MAIN_MENU_TABLE_SIZE                                \
    (sizeof(main_menu_table) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static menuinfo_t maindiag = {
    "Switzer 10G Main Menu",	/* title */
    0,                     /* title string added by init_empty_menu */
    (PFT)menu_show_dflags, /* shows major flags */
    0,                     /* generic prompt */
    0,                     /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static menuinfo_t *maindiagp = &maindiag;

static int switzer_10g_iface_test(struct ngio_intf_t *ngio)
{
    /* Testing the PCIe interface */
    if (fpga_reg_test()) {
        return FAILED;
    }

    /* Testing the I2C interface */
    if (switzer_ltc4215_reg_test()) {
        return FAILED;
    }

    /* Testing the GE interface */
    if (phy_external_lpbk_test(SWITZER_PORT_SPEED_10G)) {
        return FAILED;
    }
    if (phy_external_lpbk_test(SWITZER_PORT_SPEED_1G)) {
        return FAILED;
    }

    return PASSED;
}

static int switzer_10g_platform_init(struct switzer_10g *mod)
{
    n2g_i2c_if_t *pca = switzer_ngio_pca();

    /* PCA IO expender */
    pca->i2c_dev = SWITZER_10G_I2C_ADDR_PCA;
    pca->dev_name = "PCA9555";

    return 0;
}

static void switzer_10g_platform_exit(struct switzer_10g *mod)
{
}

static int switzer_10g_device_init(struct switzer_10g *mod)
{
    struct ngio_intf_t *ngio = switzer_ngio();
    struct switzer_settings settings = {
        .pci_domain = 0,
        .pci_bus = get_ngio_pcie_dev_bus_num(ngio->mod_type, ngio->slot),
        .pci_dev = 0,
        .pci_func = 0,
    };

    ngio->unreset(ngio);

    /* PCI ready is used to trigger a hotplug event, it should be invoked */
    /* before any PCI operation */
    ngio->pci_rdy(ngio, 1);

    if (switzer_10g_init(mod, &settings))
        return -1;
    mod->ngio = ngio;

    /* turn on the green light */
    if (util_oir_ltc4215_led(ngio->oir, OIR_LED_GREEN_ONLY))
        log_warn("util_oir_ltc4215_led failed.\n");

    return 0;
}

static void switzer_10g_device_exit(struct switzer_10g *mod)
{
    mod->ngio = NULL;
    switzer_10g_exit(mod);
}

int switzer_10g_test(struct ngio_intf_t *ngio)
{
    int rc = FAILED;

    if (switzer_10g_platform_init(mod))
        return FAILED;

    if (switzer_10g_device_init(mod))
        goto err;

    rc = PASSED;
    /* Display and interact with user until <ESC><RET> back to main menu */
    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);
    if (ngio->test_type == IFACE_TEST) {
        rc = switzer_10g_iface_test(ngio);
    } else if (ngio->menu_display == FALSE) {
        do_all_menu_items(maindiagp);
    } else {
        menu(maindiagp, main_menu_secondary_items, '\0');
    }

    switzer_10g_device_exit(mod);
err:
    switzer_10g_platform_exit(mod);
    return rc;
}

static int switzer_10g_sock_test(void)
{
    struct switzer_eth_traf_tx_task_settings tx_settings;
    struct switzer_eth_traf_rx_task_settings rx_settings;
    tx_settings.mode = SWITZER_ETH_TRAF_TX_MODE_RADOM_VIRTUAL_MAC;
    tx_settings.check = SWITZER_ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_settings.len = 150;
    tx_settings.burst = 1;
    tx_settings.duration = 500;
    rx_settings.chk_mode = SWITZER_ETH_TRAF_RX_MODE_CHECK_BIT;
#ifdef TABEIL
    if (switzer_eth_traf_util_test(TABEI_ETH_BP, TABEI_ETH_BP, &tx_settings, &rx_settings, 1)) {
        cterr('f', 0, "switzer socket test failed");
        return FAILED;
    }
#else
    if (switzer_eth_traf_util_test("br0", "br0", &tx_settings, &rx_settings, 1)) {
        return FAILED;
    }
#endif
    return PASSED;
}
