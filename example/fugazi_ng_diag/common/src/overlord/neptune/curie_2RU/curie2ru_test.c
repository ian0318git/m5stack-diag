/* $Id: curie2ru_test.c,v 1.4 2021/01/11 11:06:37 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/curie2ru_test.c,v $
 *------------------------------------------------------------------
 *
 * curie2ru_test.c - Curie2ru test interfaces.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/if.h>

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
#include "platform_eth_pkt_txrx.h"

#include "curie2ru_miura_reg.h"
#include "curie2ru_quadra28_reg.h"
#include "curie2ru.h"
#include "ethernet.h"
#include "eth_traf.h"

#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)

static struct curie2ru curie2ru, *curie = &curie2ru;
static int curie2ru_bcm82757_port_list[] = {ETH4, ETH5};
static int curie2ru_bcm82752_port_list[] = {ETH4, ETH5};
static int curie2ru_port_speed[] = {CURIE2RU_PORT_SPEED_1G, CURIE2RU_PORT_SPEED_10G};

static int curie2ru_bcm82757_socket_test(int port);
static int curie2ru_eth_port_set_speed(int port, int speed);
static int curie2ru_bcm82757_check_link(curie2ru_lane_t lane, unsigned int *sys_link, unsigned int *line_link);
static void curie2ru_bcm82757_set_tx_serdes(curie2ru_lane_t lane, int speed);
static void curie2ru_bcm82757_chk_tx_serdes(curie2ru_lane_t lane, int speed);
int curie2ru_bcm82757_set_sfp_present(curie2ru_lane_t lane);

static long bcm82757_reg_read(void);
static long bcm82757_reg_write(void);
static long bcm82757_status_dump(void);
static long bcm82757_regs_dump(curie2ru_lane_t lane);
static long bcm82757_mac_diagnostic_dump(void);
static long bcm82757_link_status(void);
static long bcm82757_firmware_download(void);
static long bcm82757_display_eye_scan(void);
static long bcm82757_config_loopback(void);
static long bcm82757_force_line_side_lrm(void);
static long bcm82757_config_prbs(void);
static long bcm82757_firmware_lane_config_set(void);
static long bcm82757_config_cl73(void);
static long bcm82757_config_cl37(void);
static long bcm82757_config_macsec_bypass(void);
static long bcm82757_rx_get(void);
static long bcm82757_rx_set(void);

static long bcm82757_register_test(void);
static long bcm82757_prbs_line_side_test(curie2ru_lane_t lane);
static long bcm82757_internal_lpbk_test(curie2ru_lane_t lane);
static long bcm82757_external_lpbk_test(curie2ru_lane_t lane);
static long bcm82757_cl37_external_lpbk_test(curie2ru_lane_t lane);
static long bcm82752_side_band_test(int);
static long bcm82757_side_band_test(int);

/* BCM82757 submenu items */
static submenu_xtable_t bcm82757_submenu_table[] = {
    {"BCM82757 Register Read", bcm82757_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Write", bcm82757_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Status dump", bcm82757_status_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Lane 0 Regs dump", bcm82757_regs_dump, CURIE2RU_LANE_0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Lane 1 Regs dump", bcm82757_regs_dump, CURIE2RU_LANE_1,
     0, NULL, 0, NULL, 0},
    {"BCM82757 MAC Diagnostic Dump", bcm82757_mac_diagnostic_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 link status", bcm82757_link_status, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Firmware Download", bcm82757_firmware_download, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Display Eye Scan", bcm82757_display_eye_scan, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Config Loopback", bcm82757_config_loopback, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Force Line Side Lrm", bcm82757_force_line_side_lrm, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Config PRBS", bcm82757_config_prbs, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Config Firmware Lane", bcm82757_firmware_lane_config_set, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Config Clause 73", bcm82757_config_cl73, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Config Clause 37", bcm82757_config_cl37, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Config Macsec Bypass", bcm82757_config_macsec_bypass, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Rx Get", bcm82757_rx_get, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Rx Set", bcm82757_rx_set, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Test", bcm82757_register_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 0 Line Side PRBS Test", bcm82757_prbs_line_side_test, CURIE2RU_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 1 Line Side PRBS Test", bcm82757_prbs_line_side_test, CURIE2RU_LANE_1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 0 Internal Loopback Test", bcm82757_internal_lpbk_test, CURIE2RU_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 1 Internal Loopback Test", bcm82757_internal_lpbk_test, CURIE2RU_LANE_1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 0 External loopback Test", bcm82757_external_lpbk_test, CURIE2RU_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 1 External loopback Test", bcm82757_external_lpbk_test, CURIE2RU_LANE_1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 0 cl37 External loopback Test", bcm82757_cl37_external_lpbk_test, CURIE2RU_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Lane 1 cl37 External loopback Test", bcm82757_cl37_external_lpbk_test, CURIE2RU_LANE_1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Port 0 Side Band Test", bcm82757_side_band_test, CURIE2RU_LANE_0,
     F_GRP_E, NULL, 0, NULL, 0},
    {"BCM82757 Port 1 Side Band Test", bcm82757_side_band_test, CURIE2RU_LANE_1,
     F_GRP_E, NULL, 0, NULL, 0},
};

#define BCM82757_SUBMENU_TABLE_SZ (sizeof(bcm82757_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t bcm82757_submenu_primary_items[BCM82757_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t bcm82757_submenu_secondary_items[BCM82757_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char bcm82757_submenu_title[] = "Curie2ru BCM82757 10G PHY Subtest Menu";

static menuinfo_t bcm82757_submenu = {
    bcm82757_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    bcm82757_submenu_primary_items,
};

static menuinfo_t *bcm82757_submenup = &bcm82757_submenu;


static long bcm82752_internal_lpbk_test(int port);
static long bcm82752_external_lpbk_test(int port);
static long bcm82752_prbs_line_side_test(int port);
static long bcm82752_register_test(void);
static long bcm82752_toggle_port_speed_flag(int port);
static long bcm82752_mode_config(void);
static long bcm82752_reg_read(void);
static long bcm82752_reg_write(void);
static long bcm82752_status_dump(void);
static long bcm82752_regs_dump(int port);
static long bcm82752_phy_diagnostic_dump(void);
static long bcm82752_link_status(void);
static long bcm82752_firmware_download(void);
static long bcm82752_display_eye_scan(void);
static long bcm82752_config_loopback(void);
static long bcm82752_config_cl37(void);
static long bcm82752_check_cl37(void);

static submenu_xtable_t bcm82752_submenu_table[] = {
    {"BCM82752 Port 0 Internal Loopback Test", bcm82752_internal_lpbk_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 1 Internal Loopback Test", bcm82752_internal_lpbk_test, 1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 0 External loopback Test", bcm82752_external_lpbk_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 1 External loopback Test", bcm82752_external_lpbk_test, 1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 0 prbs line side Test", bcm82752_prbs_line_side_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 1 prbs line side Test", bcm82752_prbs_line_side_test, 1,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 register Test", bcm82752_register_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 0 Side Band Test", bcm82752_side_band_test, 0,
     F_GRP_E, NULL, 0, NULL, 0},
    {"BCM82752 Port 1 Side Band Test", bcm82752_side_band_test, 1,
     F_GRP_E, NULL, 0, NULL, 0},

    {"BCM82752 Port 0 speed flag toggle", bcm82752_toggle_port_speed_flag, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Port 1 speed flag toggle", bcm82752_toggle_port_speed_flag, 1,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Config Mode", bcm82752_mode_config, 0,
     0, NULL, 0, NULL, 0},

    {"BCM82752 Register Read", bcm82752_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Register Write", bcm82752_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Status dump", bcm82752_status_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Port 0 regs dump", bcm82752_regs_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Port 1 regs dump", bcm82752_regs_dump, 1,
     0, NULL, 0, NULL, 0},
    {"BCM82752 PHY Diagnostic Dump", bcm82752_phy_diagnostic_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 link status", bcm82752_link_status, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Firmware Download", bcm82752_firmware_download, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Display Eye Scan", bcm82752_display_eye_scan, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Config Loopback", bcm82752_config_loopback, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 Config Clause 37", bcm82752_config_cl37, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82752 check Clause 37", bcm82752_check_cl37, 0,
     0, NULL, 0, NULL, 0},
};

#define BCM82752_SUBMENU_TABLE_SZ (sizeof(bcm82752_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t bcm82752_submenu_primary_items[BCM82752_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t bcm82752_submenu_secondary_items[BCM82752_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char bcm82752_submenu_title[] = "Curie2ru BCM82752 1G PHY Subtest Menu";

static menuinfo_t bcm82752_submenu = {
    bcm82752_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    bcm82752_submenu_primary_items,
};

static menuinfo_t *bcm82752_submenup = &bcm82752_submenu;

/* Curie 2RU platform initialization */
int curie2ru_diag_init(int argc, char *argv[])
{
    int rc;

    if ((rc = curie2ru_init(curie)))
        return rc;

    return 0;
}

/* Curie 2RU platform exit */
void curie2ru_diag_exit(void)
{
    curie2ru_exit(curie);
}

/* get SFP module type from EERPOM */
static int bcm82757_get_sfp_type(int port)
{
    char get_sfp_type_cmd[128], buffer[1024];
    FILE *fp;
    int i, flag_len;
    char *flag = "Ethernet: ";

    flag_len = strlen(flag);
    sprintf(get_sfp_type_cmd, "ethtool -m eth%d | grep \"Transceiver type\" > sfp_type_log 2>&1", port);
    system(get_sfp_type_cmd);
    if ((fp = fopen("sfp_type_log", "r")) == NULL) {
        prt("sfp_type_log open err\n");
        return -1;
    }
    memset(buffer, 0, sizeof(buffer));
    fread(buffer, sizeof(buffer), 1, fp);
    prt("%s\n", buffer);
    for (i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == 'E') {
            if (!strncmp(&buffer[i], flag, flag_len)) {
                curie2ru_bcm82757_line_side_interface_set(&buffer[i+flag_len]);
                break;
            }
        }
    }
    fclose(fp);
    return 0;
}

/* utility to force PHY LRM mode */
static long bcm82757_force_line_side_lrm(void)
{
    int force;
    force = gethex_answer("force toggle lrm (disable:0, enable:1)", 0, 0, 1);
    force_line_side_intf_lrm(force);
    return PASSED;
}

/*
 * Function: bcm82757_reg_read
 * Description: Utility to read PHY registers
 *
 * Input: none
 * Return: PASSED/FAILED
 */
static long bcm82757_reg_read(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_MIURA_DEV_PMA_PMD;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE2RU_MIURA_DEV_PMA_PMD,
                            CURIE2RU_MIURA_DEV_PCS,
                            CURIE2RU_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);

    rc = curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 read error %d", rc);
        return FAILED;
    }
    prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

/*
 * Function: bcm82757_reg_write
 * Description: Utility to write PHY registers
 *
 * Input: none
 * Return: PASSED/FAILED
 */
static long bcm82757_reg_write(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_MIURA_DEV_PMA_PMD;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE2RU_MIURA_DEV_PMA_PMD,
                            CURIE2RU_MIURA_DEV_PCS,
                            CURIE2RU_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 write error %d", rc);
        return FAILED;
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

/*
 * Function: bcm82757_status_dump
 * Description: Utility to dump PHY status
 *
 * Input: none
 * Return: PASSED/FAILED
 */
static long bcm82757_status_dump(void)
{
    int rc;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    unsigned int flags;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    flags = gethex_answer("Enter Dump Flags", 0, 0, 0xffffffff);

    curie->miura.info.flags = flags;
    rc = curie2ru_bcm82757_dump(curie, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 dump error");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82757_status_dump
 * Description: Utility to all dump PHY registers in the register table
 *
 * Input: lane - PHY lane number
 * Return: PASSED/FAILED
 */
static long bcm82757_regs_dump(curie2ru_lane_t lane)
{
    if (curie2ru_bcm82757_regs_dump(curie, lane)) {
        prt("bcm82757_regs_dump err on lane %d\n", lane);
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82757_mac_diagnostic_dump
 * Description: Utility to dump PHY HOST side status 
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_mac_diagnostic_dump(void)
{
    int rc;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie2ru_bcm82757_mac_dump(curie, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 dump error");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82757_link_status
 * Description: Utility to check link status
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_link_status(void)
{
    int rc;
    unsigned int link_status;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie2ru_bcm82757_link_status(curie, lane, if_side, &link_status);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 link status get error");
        return FAILED;
    }
    prt("phy link status: %d\n",link_status);
    return PASSED;
}

static int bcm82757_set_macsec_bypass_flag = 0;
static int bcm82757_macsec_init_flag = 0;

/*
 * Function: bcm82757_firmware_download
 * Description: Utility to download firmware
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_firmware_download(void)
{
    struct curie2ru_miura *miura = &curie->miura;

    curie2ru_miura_reset(miura);

    if (curie2ru_miura_fw_download(miura)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }
    bcm82757_set_macsec_bypass_flag = 0;
    bcm82757_macsec_init_flag = 0;

    return PASSED;
}

/*
 * Function: bcm82757_config_loopback
 * Description: Utility to configure loopback mode
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_config_loopback(void)
{
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    unsigned int lb_mode, enable;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    lb_mode = gethex_answer("Enter Loopback mode", 1, 0, 10);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (curie2ru_bcm82757_loopback_set(curie, lane, if_side, lb_mode, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82757_rx_get
 * Description: Utility to get RX SERDES
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_rx_get(void)
{
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    bcm_plp_rx_t rx;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (curie2ru_bcm82757_rx_get(curie, lane, &rx, if_side)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }
    prt("vga, enable: %d, value: %d\n"
        "peaking_filter, enable: %d, value: %d\n"
        "low_freq_peaking_filter, enable: %d, value: %d\n"
        "high_freq_peaking_filter, enable: %d, value: %d\n"
        "ffe1, enable: %d, value: %d\n"
        "ffe2, enable: %d, value: %d\n",
         rx.vga.enable, rx.vga.value,
         rx.peaking_filter.enable, rx.peaking_filter.value,
         rx.low_freq_peaking_filter.enable, rx.low_freq_peaking_filter.value,
         rx.high_freq_peaking_filter.enable, rx.high_freq_peaking_filter.value,
         rx.ffe1.enable, rx.ffe1.value,
         rx.ffe2.enable, rx.ffe2.value);

    int i;
    for (i = 0; i < rx.num_of_dfe_taps; i++) {
        prt("dfe[%d], enable: %d, value: %d\n", i, rx.dfe[i].enable, rx.dfe[i].value);
    }
    return PASSED;
}

/*
 * Function: bcm82757_rx_set
 * Description: Utility to set RX SERDES
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_rx_set(void)
{
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    bcm_plp_rx_t rx;
    int i;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    rx.vga.enable = gethex_answer("rx vga enable(disable:0, enable:1)", 0, 0, 1);
    rx.vga.value = gethex_answer("rx vga value:", 0, 0, 4096);
    rx.num_of_dfe_taps = gethex_answer("number of dfe taps(0-14", 0, 0, 14);
    for (i = 0; i < rx.num_of_dfe_taps; i++) {
        prt("dfe[%d]\n", i);
        rx.dfe[i].enable = gethex_answer("rx dfe enable(disable:0, enable:1)", 0, 0, 1);
        rx.dfe[i].value = gethex_answer("rx dfe value:", 0, 0, 4096);
    }
    rx.peaking_filter.enable = gethex_answer("rx peaking filter enable(disable:0, enable:1)", 0, 0, 1);
    rx.peaking_filter.value = gethex_answer("rx peaking filter value:", 0, 0, 4096);
    rx.low_freq_peaking_filter.enable = gethex_answer("rx low freq peaking filter enable(disable:0, enable:1)", 0, 0, 1);
    rx.low_freq_peaking_filter.value = gethex_answer("rx low freq peaking filter value:", 0, 0, 4096);
    rx.high_freq_peaking_filter.enable = gethex_answer("rx high freq peaking filter enable(disable:0, enable:1)", 0, 0, 1);
    rx.low_freq_peaking_filter.value = gethex_answer("rx high freq peaking filter value:", 0, 0, 4096);
    rx.ffe1.enable = gethex_answer("rx ffe1 enable(disable:0, enable:1)", 0, 0, 1);
    rx.ffe1.value = gethex_answer("rx ffe1 value:", 0, 0, 4096);
    rx.ffe2.enable = gethex_answer("rx ffe2 enable(disable:0, enable:1)", 0, 0, 1);
    rx.ffe2.value = gethex_answer("rx ffe2 value:", 0, 0, 4096);

    if (curie2ru_bcm82757_rx_set(curie, lane, rx, if_side)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82757_config_prbs
 * Description: Utility to configure PRBS
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_config_prbs(void)
{
    int port;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    unsigned int action, poly, enable = 0;
    curie2ru_prbs_t prbs = CURIE2RU_PRBS_31;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (if_side == CURIE2RU_IF_SIDE_SYS) {
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
        if (curie2ru_bcm82757_prbs_check(curie, lane, if_side)) {
            cterr('f', 0, "BCM82757 PRBS check failed");
            return FAILED;
        }
        break;

    case 1:
        poly = getdec_answer("PRBS Polynomial(7, 9, 11, 15, 23, 31)", 31, 0, 31);
        switch (poly) {
        case 7:
            prbs = CURIE2RU_PRBS_7;
            break;
        case 9:
            prbs = CURIE2RU_PRBS_9;
            break;
        case 11:
            prbs = CURIE2RU_PRBS_11;
            break;
        case 15:
            prbs = CURIE2RU_PRBS_15;
            break;
        case 23:
            prbs = CURIE2RU_PRBS_23;
            break;
        default:
        case 31:
            prbs = CURIE2RU_PRBS_31;
            break;
        }
        enable = 1;
    case 2:
        if (enable) {
            port = curie2ru_bcm82757_port_list[lane];
            if (bcm82757_get_sfp_type(port)) {
                cterr('f', 0, "get sfp type failed");
                return FAILED;
            }
        }
        if (curie2ru_bcm82757_prbs_set(curie, lane, if_side, prbs, enable)) {
            cterr('f', 0, "BCM82757 PRBS set failed");
            return FAILED;
        }
        if (enable == 1) {
            curie2ru_mdelay(2000);
            if (curie2ru_bcm82757_prbs_clear_rx_stat(curie, lane, if_side)) {
                cterr('f', 0, "BCM82757 PRBS rx stat failed");
                return FAILED;
            }
        }
        break;
    }

    return PASSED;
}

/*
 * Function: bcm82757_firmware_lane_config_set
 * Description: Utility to configure firmware
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_firmware_lane_config_set(void)
{
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    bcm_plp_pm_firmware_lane_config_t firmware_lane_config;

    cterr_add_component("BCM82757",
                        "MDIO controller within BCM57412");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within BCM57412");
    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);
    if (curie2ru_bcm82757_firmware_lane_get(curie, lane, if_side, &firmware_lane_config)) {
        cterr('f', 0, "curie2ru firmware lane get failed");
        return FAILED;
    }
    prt("\ncurrent firmware cfg en/disable: %d\n", firmware_lane_config.ena_dis);
    firmware_lane_config.ena_dis = gethex_answer(
        "ena_dis(Disable:0, Enable:1)\n"
        "input new ena_dis", 1, 0, 1);
    prt("\ncurrent firmware mode: %d\n", firmware_lane_config.firmware_mode);
    firmware_lane_config.firmware_mode = gethex_answer(
        "0 : default\n"
        "1 : dfe mode\n"
        "2 : osdfe mode\n"
        "3 : baud rate dfe mode\n"
        "4 : low power dfe mode\n"
        "5 : media type sfp dac\n"
        "6 : media type xlaui\n"
        "7 : media type optical sr4\n"
        "input new firmware mode", 0, 0, 7);
    prt("\ncurrent firmware AN en/disable: %d\n", firmware_lane_config.AnEnabled);
    firmware_lane_config.AnEnabled = gethex_answer(
        "Autonego( Disable:0, Enable:1)\n"
        "input new AN en/dis", 1, 0, 1);
    prt("\ncurrent firmware media type: %d\n", firmware_lane_config.MediaType);
    firmware_lane_config.MediaType = gethex_answer(
        "MediaType(PcbBackTrace:0, Copper:1, Optics:2)\n"
        "input new Media", 1, 0, 2);
    prt("\ncurrent firmware DbLoss: %d\n", firmware_lane_config.DbLoss);
    firmware_lane_config.DbLoss = gethex_answer(
        "DbLossValue(0 - 100)\n"
        "input new DbLoss" , 0, 0, 100);

    if (curie2ru_bcm82757_firmware_lane_set(curie, lane, if_side, &firmware_lane_config)) {
        cterr('f', 0, "curie2ru firmware lane set failed");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82757_config_cl73
 * Description: Utility to configure CL73 AN
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_config_cl73(void)
{
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    unsigned int enable;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (curie2ru_bcm82757_cl73_set(curie, lane, if_side, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82757_config_cl37
 * Description: Utility to configure 1G CL37 AN
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_config_cl37(void)
{
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;
    unsigned int enable;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (curie2ru_bcm82757_cl37_set(curie, lane, if_side, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82757_config_macsec_bypass
 * Description: This function bypasses MACSEC
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_config_macsec_bypass(void)
{
    int speed, speed_flag, port;
    curie2ru_lane_t lane;
    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    speed_flag = gethex_answer("speed(1G:0, 10G:1)", 1, 0, 1);
    speed = curie2ru_port_speed[speed_flag];
    port = curie2ru_bcm82757_port_list[lane];

    if (bcm82757_get_sfp_type(port)) {
        cterr('f', 0, "get sfp type failed");
        return FAILED;
    }
    if (!bcm82757_macsec_init_flag) {
        if (curie2ru_bcm82757_macsec_init(curie, 1)) {
            cterr('f', 0, "macsec init failed");
            return FAILED;
        }
        bcm82757_macsec_init_flag = 1;
    }
    if (curie2ru_bcm82757_set_macsec_bypass_mode(curie, lane, speed)) {
        cterr('f', 0, "set macsec bypass mode failed");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82757_display_eye_scan
 * Description: Utility to display eye diagram
 *
 * Input:
 * Return: PASSED/FAILED
 */
static long bcm82757_display_eye_scan(void)
{
    int rc;
    curie2ru_lane_t lane;
    curie2ru_if_side_t if_side;

    lane = gethex_answer("Enter Lane(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie2ru_bcm82757_display_eye_scan(curie, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 eye scan failed");
        return FAILED;
    }
    return PASSED;
}

static const reg_info_t bcm82757_miura_reg_tbl[] = {
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

/*
 * Function: __bcm82752_register_test
 *
 * For each register from regs, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : interface structure pointer, info for all registers
 * Output: PASS/FAIL
 */
static int __bcm82757_register_test(const reg_info_t *regs,
                                    curie2ru_lane_t lane, curie2ru_if_side_t if_side)
{
    uint32_t i;
    uint32_t regaddr, devaddr;
    uint32_t data, data_orig, data_test;

    while (regs->size.size != 0) {
        regaddr = regs->offset;
        devaddr = regaddr >> 16;
        if (curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &data_orig) < 0) {
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
                if (curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data_test) < 0 ||
                    curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &data) < 0 ||
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
                if (curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data_test) < 0 ||
                    curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &data) < 0 ||
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
                if (curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data_test) < 0 ||
                    curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Pattern test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
                data = ~data;   /* complement data pattern */
            }

            /* restore original value */
            if (curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data_test) < 0) {
                cterr('f', 0, "Error restoring %s register offset %d.%#x",
                      regs->name, devaddr, regaddr);
                return FAILED;
            }
        }
        regs++;
    }

    return PASSED;
}

static long bcm82757_register_test(void)
{
    curie2ru_if_side_t if_side = CURIE2RU_IF_SIDE_SYS;

    testname("BCM82757 Register");

    if (__bcm82757_register_test(bcm82757_miura_reg_tbl, CURIE2RU_LANE_0, if_side) == FAILED ||
        __bcm82757_register_test(bcm82757_miura_reg_tbl, CURIE2RU_LANE_1, if_side) == FAILED) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "Register Test on BCM82757 failed");
        return FAILED;
    }
    prpass(testpass, "BCM82757 Register Test Passed, ");
    return PASSED;
}

/* dump packet status for debug */
static void curie2ru_bcm82757_pkt_message_statistic(int port) {
    char pkg_statistic_cmd1[64];
    char pkg_statistic_cmd2[64];
    sprintf(pkg_statistic_cmd1, "\nethtool -S eth%d > /tmp/pkt_statistic_log 2>&1\n", port);
    sprintf(pkg_statistic_cmd2, "\nifconfig eth%d >> /tmp/pkt_statistic_log 2>&1\n", port);
    system(pkg_statistic_cmd1);
    system(pkg_statistic_cmd2);
}

static void curie2ru_bcm82757_show_pkt_statistic(void)
{
    char show_pkg_statistic_cmd[64];
    sprintf(show_pkg_statistic_cmd, "\ncat /tmp/pkt_statistic_log\n");
    system(show_pkg_statistic_cmd);
}

static void curie2ru_bcm82757_show_eye_scan(curie2ru_if_side_t if_side, curie2ru_lane_t lane)
{
    if (if_side == CURIE2RU_IF_SIDE_LINE) {
        prt("**************line side********************\n");
    } else {
        prt("**************sys side********************\n");
    }
    curie2ru_bcm82757_display_eye_scan(curie, lane, if_side);
}

/*
 * Function: bcm82757_internal_lpbk_test
 * Description: This function performs the internal loopback test
 *              from MAC to BCM82757.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_internal_lpbk_test(curie2ru_lane_t lane)
{
    int speed, port;
    unsigned int sys_link, line_link;

    testname("BCM82757 Lane %d Internal lpbk", lane);

    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");
    port = curie2ru_bcm82757_port_list[lane];
    speed = CURIE2RU_PORT_SPEED_10G;

    if (bcm82757_set_macsec_bypass_flag) {
        struct curie2ru_miura *miura = &curie->miura;
        curie2ru_miura_reset(miura);
        if (curie2ru_miura_fw_download(miura)) {
            cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
            cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
            return FAILED;
        }
        bcm82757_set_macsec_bypass_flag = 0;
    }

    if (bcm82757_get_sfp_type(port)) {
        cterr('f', 0, "get sfp type failed");
        return FAILED;
    }

    if (curie2ru_eth_port_set_speed(port, speed)) {
        cterr('f', 0, "bcm57412 set speed failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_config_macsec_bypass(curie, lane, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }
    bcm82757_set_macsec_bypass_flag = 1;

    if (curie2ru_bcm82757_line_side_interface_lrm()) {
        curie2ru_mdelay(3000);
    }
    if (curie2ru_bcm82757_loopback_set(curie, lane, CURIE2RU_IF_SIDE_LINE, 1, 1)) {
        cterr('f', 0, "set loopback failed");
        return FAILED;
    }

    curie2ru_bcm82757_set_tx_serdes(lane, speed);

    if (curie2ru_bcm82757_check_link(lane, &sys_link, &line_link)){
        cterr('f', 0, "bcm82757 check link failed");
        return FAILED;
    }

    if (!sys_link || !line_link) {
        if (!sys_link)
            cterr('f', 0, "bcm82757 no sys link");
        if (!line_link)
            cterr('f', 0, "bcm82757 no line link");
        return FAILED;
    }

    curie2ru_bcm82757_pkt_message_statistic(port);
    curie2ru_mdelay(500);
    if (curie2ru_bcm82757_socket_test(port)) {
        curie2ru_bcm82757_show_eye_scan(CURIE2RU_IF_SIDE_LINE, lane);
        curie2ru_bcm82757_show_eye_scan(CURIE2RU_IF_SIDE_SYS, lane);
        curie2ru_bcm82757_show_pkt_statistic();
        curie2ru_bcm82757_pkt_message_statistic(port);
        curie2ru_bcm82757_show_pkt_statistic();
        curie2ru_bcm82757_chk_tx_serdes(lane, speed);
        curie2ru_bcm82757_regs_dump(curie, lane);
        curie2ru_bcm82757_regs_dump(curie, lane);
        cterr('f', 0, "curie2ru socket test failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_loopback_set(curie, lane, CURIE2RU_IF_SIDE_LINE, 1, 0)) {
        cterr('f', 0, "set loopback failed");
        return FAILED;
    }

    prpass(testpass, "lane %d Internal Loopback Test Passed, ", lane);
    curie2ru_bcm82757_config_macsec_cleanup(curie);
    bcm82757_set_macsec_bypass_flag = 0;
    return PASSED;
}

/*
 * Function: bcm82757_external_lpbk_test
 * Description: This function performs the external loopback test
 *              from MAC to external SFP.
 *
 * Inputs      : lane - PHY lane index
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_external_lpbk_test(curie2ru_lane_t lane)
{
    int speed, port;
    unsigned int sys_link, line_link;

    testname("BCM82757 Lane %d External lpbk", lane);

    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("External loopback flag is off, skip the lane %d external loopback test\n", lane);
        return PASSED;
    }
    port = curie2ru_bcm82757_port_list[lane];
    speed = CURIE2RU_PORT_SPEED_10G;

    if (bcm82757_set_macsec_bypass_flag) {
        struct curie2ru_miura *miura = &curie->miura;
        curie2ru_miura_reset(miura);
        if (curie2ru_miura_fw_download(miura)) {
            cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
            cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
            return FAILED;
        }
        bcm82757_set_macsec_bypass_flag = 0;
    }

    if (bcm82757_get_sfp_type(port)) {
        cterr('f', 0, "get sfp type failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_set_sfp_present(lane)) {
        cterr('f', 0, "set sfp present failed");
        return FAILED;
    }

    if (curie2ru_eth_port_set_speed(port, speed)) {
        cterr('f', 0, "bcm57412 set speed failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_config_macsec_bypass(curie, lane, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }
    bcm82757_set_macsec_bypass_flag = 1;

    curie2ru_bcm82757_set_tx_serdes(lane, speed);

    if (curie2ru_bcm82757_check_link(lane, &sys_link, &line_link)){
        cterr('f', 0, "bcm82757 check link failed");
        return FAILED;
    }

    if (!sys_link || !line_link) {
        if (!sys_link)
            cterr('f', 0, "bcm82757 no sys link");
        if (!line_link)
            cterr('f', 0, "bcm82757 no line link");
        return FAILED;
    }

    curie2ru_bcm82757_pkt_message_statistic(port);
    curie2ru_mdelay(500);
    if (curie2ru_bcm82757_socket_test(port)) {
        curie2ru_bcm82757_show_eye_scan(CURIE2RU_IF_SIDE_LINE, lane);
        curie2ru_bcm82757_show_eye_scan(CURIE2RU_IF_SIDE_SYS, lane);
        curie2ru_bcm82757_show_pkt_statistic();
        curie2ru_bcm82757_pkt_message_statistic(port);
        curie2ru_bcm82757_show_pkt_statistic();
        curie2ru_bcm82757_chk_tx_serdes(lane, speed);
        curie2ru_bcm82757_regs_dump(curie, lane);
        curie2ru_bcm82757_regs_dump(curie, lane);
        cterr('f', 0, "curie2ru socket test failed");
        return FAILED;
    }

    prpass(testpass, "lane %d External Loopback Test Passed, ", lane);
    curie2ru_bcm82757_config_macsec_cleanup(curie);
    bcm82757_set_macsec_bypass_flag = 0;
    return PASSED;
}

/*
 * Function: bcm82757_cl37_external_lpbk_test
 * Description: This function performs the 1G external loopback test
 *              from MAC to external SFP.
 *
 * Inputs      : lane - PHY lane index
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_cl37_external_lpbk_test(curie2ru_lane_t lane)
{
    int speed, port;
    unsigned int sys_link, line_link;

    testname("BCM82757 Lane %d cl37 External lpbk", lane);
    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("cl37 External loopback flag is off, skip the lane %d external loopback test\n", lane);
        return PASSED;
    }

    port = curie2ru_bcm82757_port_list[lane];
    speed =CURIE2RU_PORT_SPEED_1G;

    if (bcm82757_set_macsec_bypass_flag) {
        struct curie2ru_miura *miura = &curie->miura;
        curie2ru_miura_reset(miura);
        if (curie2ru_miura_fw_download(miura)) {
            cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
            cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
            return FAILED;
        }
        bcm82757_set_macsec_bypass_flag = 0;
    }

    if (bcm82757_get_sfp_type(port)) {
        cterr('f', 0, "get sfp type failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_set_sfp_present(lane)) {
        cterr('f', 0, "set sfp present failed");
        return FAILED;
    }

    if (curie2ru_eth_port_set_speed(port, speed)) {
        cterr('f', 0, "bcm57412 set speed failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_config_macsec_bypass(curie, lane, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }
    bcm82757_set_macsec_bypass_flag = 1;


    if (curie2ru_bcm82757_cl37_set(curie, lane, CURIE2RU_IF_SIDE_LINE, 1)) {
        cterr('f', 0, "set cl37 failed");
        return FAILED;
    }

    curie2ru_bcm82757_set_tx_serdes(lane, speed);

    if (curie2ru_bcm82757_check_link(lane, &sys_link, &line_link)){
        cterr('f', 0, "bcm82757 check link failed");
        return FAILED;
    }

    if (!sys_link || !line_link) {
        if (!sys_link)
            cterr('f', 0, "bcm82757 no sys link");
        if (!line_link)
            cterr('f', 0, "bcm82757 no line link");
        return FAILED;
    }

    curie2ru_bcm82757_pkt_message_statistic(port);
    curie2ru_mdelay(500);
    if (curie2ru_bcm82757_socket_test(port)) {
        curie2ru_bcm82757_show_pkt_statistic();
        curie2ru_bcm82757_pkt_message_statistic(port);
        curie2ru_bcm82757_show_pkt_statistic();
        curie2ru_bcm82757_chk_tx_serdes(lane, speed);
        curie2ru_bcm82757_regs_dump(curie, lane);
        curie2ru_bcm82757_regs_dump(curie, lane);
        cterr('f', 0, "curie2ru socket test failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_cl37_set(curie, lane, CURIE2RU_IF_SIDE_LINE, 0)) {
        cterr('f', 0, "set cl37 failed");
        return FAILED;
    }

    prpass(testpass, "lane %d cl37 External Loopback Test Passed, ", lane);
    curie2ru_bcm82757_config_macsec_cleanup(curie);
    bcm82757_set_macsec_bypass_flag = 0;
    return PASSED;
}

#define PRBS_TEST_DELAY 1

static long __bcm82757_prbs_line_side_test(curie2ru_lane_t lane,
                                           curie2ru_prbs_t prbs, uint32_t delay_sec)
{
    uint32_t enable = 1;
    curie2ru_if_side_t if_side = CURIE2RU_IF_SIDE_LINE;

    /* enable prbs */
    if (curie2ru_bcm82757_prbs_set(curie, lane, if_side, prbs, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 PRBS set enable failed on lane %d", lane);
        return FAILED;
    }

    /* clear prbs rx stat*/
    curie2ru_mdelay(2000);
    if (curie2ru_bcm82757_prbs_clear_rx_stat(curie, lane, if_side)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 PRBS clear rx stat failed on lane %d", lane);

    }
    curie2ru_mdelay(delay_sec * 1000);
    /* check prbs */
    if (curie2ru_bcm82757_prbs_check(curie, lane, if_side)) {
        cterr_add_component("BCM82757",
                            "SFP plugin link status");
        cterr_add_debug("Check BCM82757",
                        "Check the SFP plugin link status");
        cterr('f', 0, "BCM82757 PRBS check failed on lane %d", lane);
        return FAILED;
    }

    /* disable prbs */
    enable = 0;
    if (curie2ru_bcm82757_prbs_set(curie, lane, if_side, prbs, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 PRBS set disable failed on lane %d", lane);
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82757_prbs_line_side_test
 * Description: This function performs the PRBS line side test
 *              from MAC to external SFP.
 *
 * Inputs      : lane - PHY lane index
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_prbs_line_side_test(curie2ru_lane_t lane)
{
    int speed, port;
    curie2ru_prbs_t prbs;

    testname("BCM82757 Lane %d Line Side PRBS", lane);

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("External loopback flag is off, skip the lane %d external loopback test\n", lane);
        return PASSED;
    }

    speed = CURIE2RU_PORT_SPEED_10G;
    port = curie2ru_bcm82757_port_list[lane];

    if (bcm82757_set_macsec_bypass_flag) {
        struct curie2ru_miura *miura = &curie->miura;
        curie2ru_miura_reset(miura);
        if (curie2ru_miura_fw_download(miura)) {
            cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
            cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
            return FAILED;
        }
        bcm82757_set_macsec_bypass_flag = 0;
    }

    if (bcm82757_get_sfp_type(port)) {
        cterr('f', 0, "get sfp type failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_set_sfp_present(lane)) {
        cterr('f', 0, "set sfp present failed");
        return FAILED;
    }

    if (curie2ru_bcm82757_config_macsec_bypass(curie, lane, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return FAILED;
    }
    bcm82757_set_macsec_bypass_flag = 1;

    curie2ru_bcm82757_set_tx_serdes(lane, speed);
    if (curie2ru_bcm82757_line_side_interface_lrm()) {
        curie2ru_mdelay(3000);
    }
    prbs = CURIE2RU_PRBS_31;
    if (__bcm82757_prbs_line_side_test(lane, prbs, PRBS_TEST_DELAY) == FAILED) {
        curie2ru_bcm82757_show_eye_scan(CURIE2RU_IF_SIDE_LINE, lane);
        curie2ru_bcm82757_chk_tx_serdes(lane, speed);
        curie2ru_bcm82757_regs_dump(curie, lane);
        curie2ru_bcm82757_regs_dump(curie, lane);
        return FAILED;
    }

    prpass(testpass, "Lane %d Line Side PRBS Test Passed, ", lane);
    curie2ru_bcm82757_config_macsec_cleanup(curie);
    bcm82757_set_macsec_bypass_flag = 0;

    return PASSED;
}

/*
 * Function: curie2ru_bcm82757_test
 * Description: entry for all BCM82757 tests and utilities
 *
 * Inputs      : show_menu - 1 for menu and 0 for testing all
 * Outputs     : PASSED / FAILED
 */
int curie2ru_bcm82757_test(int show_menu)
{
    testname("BCM82757");
    system("ifconfig eth4 up");
    system("ifconfig eth5 up");

    if (!curie->miura.downloaded)
        cterr('f', 0, "bcm82757 firmware download failed at diag boot time.");

    build_primary_submenu(bcm82757_submenu_table, BCM82757_SUBMENU_TABLE_SZ,
                          bcm82757_submenu_title, &bcm82757_submenup);
    build_secondary_submenu(bcm82757_submenu_table, BCM82757_SUBMENU_TABLE_SZ,
                            bcm82757_submenu_secondary_items);
    if (show_menu)
        menu(bcm82757_submenup, bcm82757_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(bcm82757_submenup);
    return PASSED;
}

static long bcm82757_side_band_test (int port)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_MIURA_DEV_PMA_PMD;
    uint32_t rx_los_reg[] =  {PORT_0_OPT_RX_LOS_REG,   PORT_1_OPT_RX_LOS_REG};
    uint32_t tx_flt_reg[] =  {PORT_0_OPT_TX_FAULT_REG, PORT_1_OPT_TX_FAULT_REG};
    uint32_t mod_abs_reg[] = {PORT_0_OPT_MOD_ABS_REG,  PORT_1_OPT_MOD_ABS_REG};
    curie2ru_if_side_t if_side = CURIE2RU_IF_SIDE_LINE;
    int jx, result = PASSED;

    printf("Please do not run it for critical phase now!\n");
    testname("BCM82757 Side Band");

    /* rx_los status reg. = 0x8a5f/0x8a61 bit2 */
    /* tx_fault status reg. = 0x8a67/0x8a69 bit2 */
    /* mod_abs status reg. = 0xc86f/0x8a71 bit2 (SFP present) */

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("\nPlease remove the SFP and loopback\n");
        /* No plug-in SFP, rx_los,tx_flt,mod_abs are high */
        prpass(testpass, "Port %x Side Band Test, ",port);

        /* RX_LOS Test */
        regaddr = rx_los_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", port);
            result = FAILED;
        }

        if (!(data & RX_LOS_MASK)) {
            cterr('f', 0, "BCM82757 port %d rx_los (0x%x) error", port, data);
            result = FAILED;
        }

        /* TX_FAULT Test */
        regaddr = tx_flt_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 read error");
            result = FAILED;
        }

        if (!(data & TX_FLT_MASK)) {
            cterr('f', 0, "BCM82757 port %d tx_fault (0x%x) error", port, data);
            result = FAILED;
        }

        /* SFP_PRESENT Test */
        regaddr = mod_abs_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", port);
            result = FAILED;
        }

        if (!(data & MOD_ABS_MASK)) {
            cterr('f', 0, "BCM82757 port %d sfp_present (0x%x) error", port, data);
            result = FAILED;
        }

        if (result == PASSED) {
            printf("\nPort %d Side Band Pass\n",port);
        } else {
            goto exit;
        }
    } else {
        printf("\nPlease plug-in the SFP and loopback\n");
        /* Test1: tx_dis enable, rx_los=high, tx_flt,sfp_present=low */
        prpass(testpass, "Port %x TX_DIS Enable, ",port);

        result = bcm57412_port_sideband_tx_dis(port, ENABLE);
        msleep(SIDEBAND_ASSERT_TIME);

        /* RX_LOS Test */
        regaddr = rx_los_reg[port];
        port = port;
        printf("\n");
        for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
            rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", port);
                result = FAILED;
            }
            if (!(data & RX_LOS_MASK)) {
                result = FAILED;
                msleep(SIDEBAND_ASSERT_TIME);
                continue;
            }
            break;
        }

        if ((jx == SIDEBAND_TIMEOUT)) {
            cterr('f', 0, "BCM82757 port %d rx_los (0x%x) error", port, data);
            result = FAILED;
        }

        /* TX_FAULT Test */
        regaddr = tx_flt_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", port);
            result = FAILED;
        }

        if ((data & TX_FLT_MASK)) {
            cterr('f', 0, "BCM82757 port %d tx_fault (0x%x) error", port, data);
            result = FAILED;
        }

        /* SFP_PRESENT Test */
        regaddr = mod_abs_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", port);
            result = FAILED;
        }

        if ((data & MOD_ABS_MASK)) {
            cterr('f', 0, "BCM82757 port %d sfp_present (0x%x) error", port,
                   data);
            result = FAILED;
        }

        if (result == PASSED) {
            printf("\nPort %d Side Band Pass, ",port);
        } else {
            goto exit;
        }

        /* Test2: tx_dis disable,  rx_los=low, tx_flt, sfp_present=low */
        prpass(testpass, "Port %x TX_DIS Disable, ",port);

        result = bcm57412_port_sideband_tx_dis(port, DISABLE);
        msleep(SIDEBAND_ASSERT_TIME);

        /* RX_LOS Test */
        regaddr = rx_los_reg[port];
        printf("\n");
        for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
            rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82757 port %d read error", port);
                result = FAILED;
            }
            if ((data & RX_LOS_MASK)) {
                result = FAILED;
                msleep(SIDEBAND_ASSERT_TIME);
                continue;
            }
            break;
        }

        if ((jx == SIDEBAND_TIMEOUT)) {
            cterr('f', 0, "BCM82757 port %d rx_los (0x%x) error", port, data);
            result = FAILED;
        }

        /* TX_FAULT Test */
        regaddr = tx_flt_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", port);
            result = FAILED;
        }

        if ((data & TX_FLT_MASK)) {
            cterr('f', 0, "BCM82757 port %d tx_fault (0x%x) error", port, data);
            result = FAILED;
        }

        /* SFP_PRESENT Test */
        regaddr = mod_abs_reg[port];
        rc = curie2ru_bcm82757_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d read error", port);
            result = FAILED;
        }

        if ((data & MOD_ABS_MASK)) {
            cterr('f', 0, "BCM82757 port %d sfp_present (0x%x) error", port,
                   data);
            result = FAILED;
        }

        if (result == PASSED) {
            printf("\nPort %d Side Band Pass\n",port);
        } else {
            goto exit;
        }
    }
exit:
    return (result);
}

/*
 * Function: curie2ru_bcm82757_socket_test
 * Description: This function performs traffic test
 *
 * Inputs      : port - network interface index
 * Outputs     : PASSED / FAILED
 */
static int curie2ru_bcm82757_socket_test(int port)
{
    char port_name[10];
    struct eth_traf_tx_task_settings tx_settings;
    struct eth_traf_rx_task_settings rx_settings;

    tx_settings.mode = ETH_TRAF_TX_MODE_FIXED;
    tx_settings.check = ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_settings.len = 256;
    tx_settings.burst = 1;
    tx_settings.interval = 10000;
    rx_settings.chk_mode = ETH_TRAF_RX_MODE_CHECK_BIT;

    sprintf(port_name, "eth%d", port);
    if (eth_traf_util_test(port_name, port_name, &tx_settings, &rx_settings, 1)) {
        prt("eth traf failed on eth%d", port);
        return -1;
    }
    return 0;
}

/*
 * Function: curie2ru_bcm82757_set_sfp_present
 * Description: This function checks whether SFP is present
 *
 * Inputs      : lane - PHY lane index
 * Outputs     : PASSED / FAILED
 */
int curie2ru_bcm82757_set_sfp_present(curie2ru_lane_t lane)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_MIURA_DEV_PMA_PMD;
    curie2ru_if_side_t if_side;

    if (!is_curie_2ru_p1a()) {
        return PASSED;
    }

    if_side = CURIE2RU_IF_SIDE_LINE;
    if (lane == CURIE2RU_LANE_0) {
        regaddr = BCMI_MIURA_DIRECT_CTRL_PORT0_CONFIGr;
    } else if (lane == CURIE2RU_LANE_1) {
        regaddr = BCMI_MIURA_DIRECT_CTRL_PORT1_CONFIGr;
    }

    rc = curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 read error");
        return FAILED;
    }
    data &= ~(1<<10);
    rc = curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 write error");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: curie2ru_bcm82757_chk_tx_serdes
 * Description: This function checks BCM82757 TX SERDES
 *
 * Inputs      : lane - PHY lane index
 *               speed - speed value
 * Outputs     : PASSED / FAILED
 */
static void curie2ru_bcm82757_chk_tx_serdes(curie2ru_lane_t lane, int speed)
{
    int rc;
    uint32_t data, temp, regaddr, devaddr = CURIE2RU_MIURA_DEV_PMA_PMD;
    curie2ru_if_side_t if_side;

    if_side = CURIE2RU_IF_SIDE_LINE;
    regaddr = BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_CTRL5r;

    if (speed == CURIE2RU_PORT_SPEED_10G) {
        data = 0x7000;
    } else {
        data = 0x2000;
    }
    rc = curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &temp);
    if (rc < 0) {
        prt("warn: get bcm82757 tx ctrl 5r value failed\n");
    }
    if (data != temp) {
        prt("NOTICE: expect bcm82757 tx ctrl 5r value is %04x, bcm82757 tx ctrl 5r value is %04x\n", data, temp);
    }

    regaddr = BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_FIR_CTRL1r;
    if (speed == CURIE2RU_PORT_SPEED_10G) {
        data = 0x00c0;
    } else {
        data = 0x00e0;
    }
    rc = curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &temp);
    if (rc < 0) {
        prt("warn: get bcm82757 tx fir ctrl1r value failed\n");
    }
    if (data != temp) {
        prt("NOTICE: expect bcm82757 tx fir ctrl1r value is %04x, bcm82757 tx fir ctrl1r value is %04x\n", data, temp);
    }

    regaddr = BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_FIR_CTRL2r;
    if (speed == CURIE2RU_PORT_SPEED_10G) {
        data = 0x80a4;
    } else {
        data = 0x8028;
    }
    rc = curie2ru_bcm82757_read(curie, lane, if_side, devaddr, regaddr, &temp);
    if (rc < 0) {
        prt("warn: get bcm82757 tx fir ctrl2r failed\n");
    }
    if (data != temp) {
        prt("NOTICE: expect bcm82757 tx fir ctrl1r value is %04x, bcm82757 tx fir ctrl2r value is %04x\n", data, temp);
    }
}

/*
 * Function: curie2ru_bcm82757_chk_tx_serdes
 * Description: This function configures BCM82757 TX SERDES
 *
 * Inputs      : lane - PHY lane index
 *               speed - speed value
 * Outputs     : PASSED / FAILED
 */
static void curie2ru_bcm82757_set_tx_serdes(curie2ru_lane_t lane, int speed)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_MIURA_DEV_PMA_PMD;
    curie2ru_if_side_t if_side;

    if_side = CURIE2RU_IF_SIDE_LINE;
    regaddr = BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_CTRL5r;

    if (speed == CURIE2RU_PORT_SPEED_10G) {
        data = 0x7000;
    } else {
        data = 0x2000;
    }
    rc = curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        prt("warn: failed to set bcm82757 tx ctrl 5r: %04x\n", data);
    }

    regaddr = BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_FIR_CTRL1r;
    if (speed == CURIE2RU_PORT_SPEED_10G) {
        data = 0x00c0;
    } else {
        data = 0x00e0;
    }
    rc = curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        prt("warn: failed to set bcm82757 tx fir ctrl1r: %04x\n", data);
    }

    regaddr = BCMI_MIURA_INDIRECT_MERLIN_PMD_TX_FIR_CTRL2r;
    if (speed == CURIE2RU_PORT_SPEED_10G) {
        data = 0x80a4;
    } else {
        data = 0x8028;
    }
    rc = curie2ru_bcm82757_write(curie, lane, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        prt("warn: failed to set bcm82757 tx fir ctrl1r: %04x\n", data);
    }
}

/*
 * Function: curie2ru_eth_port_set_speed
 * Description: This function configures MAC speed
 *
 * Inputs      : port - network interface index
 *               speed - speed value
 * Outputs     : PASSED / FAILED
 */
static int curie2ru_eth_port_set_speed(int port, int speed)
{
    struct ifreq ifr;
    int sockfd, rc;
    char devname[32];
    struct ethtool_cmd ecmd;

    sprintf(devname, "eth%d", port);
    strcpy(ifr.ifr_name, devname);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cterr('f', 0, "socket set up failed");
        return FAILED;
    }

    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    rc = ioctl(sockfd, SIOCETHTOOL, &ifr);
    if (rc < 0) {
        cterr('f', 0, "eth port get current setting failed");
        return FAILED;
    }
    ecmd.cmd = ETHTOOL_SSET;
    ecmd.speed = speed;
    rc = ioctl(sockfd, SIOCETHTOOL, &ifr);
    if (rc < 0) {
        cterr('f', 0, "eth port speed set failed");
        return FAILED;
    }
    close(sockfd);
    return PASSED;
}

static int curie2ru_bcm82757_check_link(curie2ru_lane_t lane, unsigned int *sys_link, unsigned int *line_link)
{
    int i, rc;
    curie2ru_if_side_t if_side;

    for (i = 0; i < 6; i++) {
        if_side = CURIE2RU_IF_SIDE_SYS;
        rc = curie2ru_bcm82757_link_status(curie, lane, if_side, sys_link);
        if (rc < 0) {
            cterr('f', 0, "bcm57412 get sys side link status failed");
            return FAILED;
        }
        if (!(*sys_link)) {
            curie2ru_mdelay(2000);
            continue;
        }
        break;
    }
    for (i = 0; i < 6; i++) {
        if_side = CURIE2RU_IF_SIDE_LINE;
        rc = curie2ru_bcm82757_link_status(curie, lane, if_side, line_link);
        if (rc < 0) {
            cterr('f', 0, "bcm82757 get line side link status failed");
            return FAILED;
        }
        if (!(*line_link)) {
            curie2ru_mdelay(2000);
            continue;
        }
        break;
    }
    return PASSED;
}

/*
 * Function: curie2ru_config_bcm82757_macsec_bypass
 * Description: This function bypasses BCM82757 MACSEC
 *
 * Inputs      : port - network interface index
 *               speed - speed value
 * Outputs     : PASSED / FAILED
 */
int curie2ru_config_bcm82757_macsec_bypass(curie2ru_lane_t lane, int speed)
{
    if (curie2ru_bcm82757_config_macsec_bypass(curie, lane, speed)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: curie2ru_cleanup_bcm82757_macsec
 * Description: This function cleanups BCM SDK resources
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
void curie2ru_cleanup_bcm82757_macsec(void)
{
    curie2ru_bcm82757_config_macsec_cleanup(curie);
}

static int bcm82752_port0_speed = CURIE2RU_PORT_SPEED_1G;
static int bcm82752_port1_speed = CURIE2RU_PORT_SPEED_1G;

static inline int bcm82752_get_port_speed_flag(int port)
{
    return port == 0 ? bcm82752_port0_speed : bcm82752_port1_speed;
}

static void curie2ru_bcm82752_pkt_message_statistic(int port) {
    char pkg_statistic_cmd1[64];
    char pkg_statistic_cmd2[64];
    sprintf(pkg_statistic_cmd1, "\nethtool -S eth%d > /tmp/pkt_statistic_log 2>&1\n", port);
    sprintf(pkg_statistic_cmd2, "\nifconfig eth%d >> /tmp/pkt_statistic_log 2>&1\n", port);
    system(pkg_statistic_cmd1);
    system(pkg_statistic_cmd2);
}

static void curie2ru_bcm82752_show_pkt_statistic(void)
{
    char show_pkg_statistic_cmd[64];
    sprintf(show_pkg_statistic_cmd, "\ncat /tmp/pkt_statistic_log\n");
    system(show_pkg_statistic_cmd);
}

/*
 * Function: bcm82752_mode_config
 * Description: utility to configure PHY mode
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_mode_config(void)
{
    int port, speed_flag, speed;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    speed_flag = gethex_answer("Enter speed(1G:0, 10G:1)", 0, 0, 1);
    speed = curie2ru_port_speed[speed_flag];
    if (curie2ru_bcm82752_mode_config(curie, port, speed)) {
        cterr('f', 0, "failed to configure bcm82752 mode");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82752_reg_read
 * Description: utility to read PHY register
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_reg_read(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_QUADRA28_DEV_PMA_PMD;
    int port;
    curie2ru_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE2RU_QUADRA28_DEV_PMA_PMD,
                            CURIE2RU_QUADRA28_DEV_PCS,
                            CURIE2RU_QUADRA28_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);

    rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 read error");
        return FAILED;
    }
    prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

/*
 * Function: bcm82752_reg_write
 * Description: utility to write PHY register
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_reg_write(void)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_QUADRA28_DEV_PMA_PMD;
    int port;
    curie2ru_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            CURIE2RU_QUADRA28_DEV_PMA_PMD,
                            CURIE2RU_QUADRA28_DEV_PCS,
                            CURIE2RU_QUADRA28_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 write error");
        return FAILED;
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return PASSED;
}

/*
 * Function: bcm82752_status
 * Description: utility to dump PHY status
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_status_dump(void)
{
    int rc;
    int port;
    curie2ru_if_side_t if_side;
    unsigned int flags;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    flags = gethex_answer("Enter Dump Flags", 0, 0, 0xffffffff);

    curie->miura.info.flags = flags;
    rc = curie2ru_bcm82752_dump(curie, port, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 dump error");
        return FAILED;
    }
    return PASSED;
}

static long bcm82752_regs_dump(int port)
{
    if (curie2ru_bcm82752_regs_dump(curie, port)) {
        prt("bcm82752_regs_dump err on port %d\n", port);
        return FAILED;
    }
    return PASSED;
}

static long bcm82752_phy_diagnostic_dump(void)
{
    int rc;
    int port;
    curie2ru_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie2ru_bcm82752_phy_dump(curie, port, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 dump error");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: bcm82752_link_status
 * Description: utility to check PHY link status
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_link_status(void)
{
    int rc;
    unsigned int link_status;
    int port;
    curie2ru_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie2ru_bcm82752_link_status(curie, port, if_side, &link_status);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 link status get error");
        return FAILED;
    }
    prt("phy link status: %d\n",link_status);
    return PASSED;
}

/*
 * Function: bcm82752_firmware_download
 * Description: utility to download firmware
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_firmware_download(void)
{
    struct curie2ru_quadra28 *q28 = curie->quadra28;

    curie2ru_quadra28_reset(q28);

    if (curie2ru_quadra28_fw_download(q28)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within the FPGA");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82752_config_loopback
 * Description: utility to configure loopback mode
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_config_loopback(void)
{
    int port;
    curie2ru_if_side_t if_side;
    unsigned int lb_mode, enable;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    lb_mode = gethex_answer("Enter Loopback mode", 1, 0, 10);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (curie2ru_bcm82752_loopback_set(curie, port, if_side, lb_mode, enable)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82752_config_cl37
 * Description: utility to configure 1G CL37
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_config_cl37(void)
{
    int port;
    unsigned int enable;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (curie2ru_bcm82752_set_cl37_an(curie, port, enable)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }

    return PASSED;
}

/*
 * Function: bcm82752_check_cl37
 * Description: utility to check 1G CL37
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_check_cl37(void)
{
    int port, an, link, done;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);

    if (curie2ru_bcm82752_check_cl37_an(curie, port, &an, &link, &done)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        return FAILED;
    }
    prt("cl37 status: an enbale %d, link %d, done %d\n", an, link, done);

    return PASSED;
}

/*
 * Function: bcm82752_display_eye_scan
 * Description: utility to show eye diagram
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_display_eye_scan(void)
{
    int rc;
    int port;
    curie2ru_if_side_t if_side;

    port = gethex_answer("Enter Port(0, 1)", 0, 0, 1);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = curie2ru_bcm82752_display_eye_scan(curie, port, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 eye scan failed");
        return FAILED;
    }
    return PASSED;
}

/*
 * Function: curie2ru_bcm82752_check_link
 * Description: This function checks PHY link status
 *
 * Inputs      : port - PHY port index
                 sys_link - pointer to store the returned system side link
                 line_link - pointer to store the returned line side link
 * Outputs     : PASSED / FAILED
 */
static int curie2ru_bcm82752_check_link(int port,
                                        unsigned int *sys_link,
                                        unsigned int *line_link)
{
    int i, rc;
    curie2ru_if_side_t if_side;

    for (i = 0; i < 6; i++) {
        if_side = CURIE2RU_IF_SIDE_SYS;
        rc = curie2ru_bcm82752_link_status(curie, port, if_side, sys_link);
        if (rc < 0) {
            cterr('f', 0, "bcm82752 get sys side link status failed");
            return FAILED;
        }
        if (!(*sys_link)) {
            curie2ru_mdelay(2000);
            continue;
        }
        break;
    }
    for (i = 0; i < 6; i++) {
        if_side = CURIE2RU_IF_SIDE_LINE;
        rc = curie2ru_bcm82752_link_status(curie, port, if_side, line_link);
        if (rc < 0) {
            cterr('f', 0, "bcm82752 get line side link status failed");
            return FAILED;
        }
        if (!(*line_link)) {
            curie2ru_mdelay(2000);
            continue;
        }
        break;
    }
    return PASSED;
}

/* socket traffic test, same as BCM82757 */
static int curie2ru_bcm82752_socket_test(int ether1, int ether2)
{
    char ethnet1[8], ethnet2[8];
    struct eth_traf_tx_task_settings tx_settings;
    struct eth_traf_rx_task_settings rx_settings;

    tx_settings.mode = ETH_TRAF_TX_MODE_FIXED;
    tx_settings.check = ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_settings.len = 256;
    tx_settings.burst = 1;
    tx_settings.interval = 10000;
    rx_settings.chk_mode = ETH_TRAF_RX_MODE_CHECK_BIT;

    snprintf(ethnet1, sizeof(ethnet1), "eth%d", ether1);
    snprintf(ethnet2, sizeof(ethnet2), "eth%d", ether2);
    if (eth_traf_util_test(ethnet1, ethnet2, &tx_settings, &rx_settings, 1)) {
        prt("eth traf failed on eth%d\n", ether1);
        return -1;
    }
    return 0;
}

/* configure BCM82752 TX SERDES */
static void curie2ru_bcm82752_set_tx_serdes(struct curie2ru_quadra28 *q28)
{
    int _rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_QUADRA28_DEV_PMA_PMD;

    regaddr = BCMI_QUADRA28_TX_CTRL_5r;
    data = 0x2000;
    _rc = bcm_plp_reg_value_set(q28->type, q28->info, devaddr, regaddr, data);
    if (_rc) {
        prt("warn: failed to set bcm82752 tx ctrl5r: %04x\n", data);
    }

    regaddr = BCMI_QUADRA28_TXFIR_CONTROL1r;
    data = 0x00e0;
    _rc = bcm_plp_reg_value_set(q28->type, q28->info, devaddr, regaddr, data);
    if (_rc) {
        prt("warn: failed to set bcm82752 tx fir ctrl1r: %04x\n", data);
    }

    regaddr = BCMI_QUADRA28_TXFIR_CONTROL2r;
    data = 0x8028;
    _rc = bcm_plp_reg_value_set(q28->type, q28->info, devaddr, regaddr, data);
    if (_rc) {
        prt("warn: failed to set bcm82752 tx fir ctrl2r: %04x\n", data);
    }
}

/* check BCM82752 TX SERDES */
static void curie2ru_bcm82752_chk_tx_serdes(struct curie2ru_quadra28 *q28)
{
    int _rc;
    uint32_t data, temp, regaddr, devaddr = CURIE2RU_QUADRA28_DEV_PMA_PMD;

    regaddr = BCMI_QUADRA28_TX_CTRL_5r;
    data = 0x2000;
    _rc = bcm_plp_reg_value_get(q28->type, q28->info, devaddr, regaddr, &temp);
    if (_rc) {
        prt("warn: failed to get bcm82752 tx ctrl5r value\n");
    }
    if (data != temp) {
        prt("NOTICE: expect bcm82752 tx ctrl5r value is %04x, tx ctrl5r value is %04x", data, temp);
    }

    regaddr = BCMI_QUADRA28_TXFIR_CONTROL1r;
    data = 0x00e0;
    _rc = bcm_plp_reg_value_get(q28->type, q28->info, devaddr, regaddr, &temp);
    if (_rc) {
        prt("warn: failed to get bcm82752 tx fir ctrl1r value\n");
    }
    if (data != temp) {
        prt("NOTICE: expect bcm82752 tx fir ctrl1r value is %04x, tx fir ctrl1r value is %04x", data, temp);
    }

    regaddr = BCMI_QUADRA28_TXFIR_CONTROL2r;
    data = 0x8028;
    _rc = bcm_plp_reg_value_get(q28->type, q28->info, devaddr, regaddr, &temp);
    if (_rc) {
        prt("warn: failed to get bcm82752 tx fir ctrl2r value\n");
    }
    if (data != temp) {
        prt("NOTICE: expect bcm82752 tx fir ctrl2r value is %04x, tx fir ctrl1r value is %04x", data, temp);
    }
}

/*
 * Function: bcm82752_internal_lpbk_test
 * Description: This function performs the internal loopback test
 *              from MAC to BCM82757.
 *
 * Inputs      : port - PHY port index
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_internal_lpbk_test(int port)
{
    int speed, ether;
    unsigned int sys_link, line_link;
    const char *spdstr = "1G";
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];

    testname("BCM82752 Port %d Internal lpbk", port);

    cterr_add_component("BCM82752",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82752",
                    "Check MDIO controller within the FPGA");
    ether = curie2ru_bcm82752_port_list[port];
    speed = bcm82752_get_port_speed_flag(port);

    if (speed == CURIE2RU_PORT_SPEED_10G)
        spdstr = "1G";

    if (curie2ru_eth_port_set_speed(ether, speed)) {
        cterr('f', 0, "bcm57412 set speed failed");
        return FAILED;
    }

    if (curie2ru_bcm82752_mode_config(curie, port, speed)) {
        cterr('f', 0, "failed to configure bcm82752 %s mode", spdstr);
        return FAILED;
    }

    if (curie2ru_bcm82752_loopback_set(curie, port, CURIE2RU_IF_SIDE_LINE, 1, 1)) {
        cterr('f', 0, "set loopback failed");
        return FAILED;
    }

    curie2ru_bcm82752_set_tx_serdes(q28);
    if (curie2ru_bcm82752_check_link(port, &sys_link, &line_link)) {
        cterr('f', 0, "bcm82752 check link failed");
        return FAILED;
    }

    if (!sys_link || !line_link) {
        if (!sys_link)
            cterr('f', 0, "bcm82752 no sys link");
        if (!line_link)
            cterr('f', 0, "bcm82752 no line link");
        return FAILED;
    }

    /* add a delay to fix a sync bug in bnxt_en for ethtool -S */
    curie2ru_mdelay(3000);
    curie2ru_bcm82752_pkt_message_statistic(ether);
    curie2ru_mdelay(500);

    if (curie2ru_bcm82752_socket_test(ether, ether)) {
        curie2ru_bcm82752_show_pkt_statistic();
        curie2ru_bcm82752_pkt_message_statistic(ether);
        curie2ru_bcm82752_show_pkt_statistic();
        curie2ru_bcm82752_chk_tx_serdes(q28);
        curie2ru_bcm82752_regs_dump(curie, port);
        curie2ru_bcm82752_regs_dump(curie, port);
        cterr('f', 0, "curie2ru socket test failed");
        return FAILED;
    }

    if (curie2ru_bcm82752_loopback_set(curie, port, CURIE2RU_IF_SIDE_LINE, 1, 0)) {
        cterr('f', 0, "set loopback failed");
        return FAILED;
    }

    prpass(testpass, "port %d Internal Loopback Test Passed, ", port);

    return PASSED;
}

#define C800_MAGIC  0x38ff

/* special SFP configuration for P1A boards */
static void curie2ru_pla_bcm82752_conf_sfp(struct curie2ru_quadra28 *q28)
{
    if (is_curie_2ru_p1a()) {
        /* one magic setting for P1A boards suggested by HW team */
        int _rc;
        _rc = bcm_plp_reg_value_set(q28->type, q28->info, 1, 0xc800, C800_MAGIC);
        if (_rc) {
            printf("warn: failed to set magic 0xc800 with %d\n", C800_MAGIC);
        } else {
            unsigned int tmp;
            bcm_plp_reg_value_get(q28->type, q28->info, 1, 0xc800, &tmp);
            if (tmp != C800_MAGIC) {
                printf("Notice: magic setting 0xc800 is %04x\n", tmp);
            }
        }
    }
}

/*
 * Function: bcm82752_external_lpbk_test
 * Description: This function performs the external loopback test
 *              from MAC to external SFP.
 *
 * Inputs      : port - PHY port index
 * Outputs     : PASSED / FAILED
 */
static long bcm82752_external_lpbk_test(int port)
{
    int speed, ether;
    unsigned int sys_link, line_link;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];

    testname("BCM82752 Port %d External lpbk", port);

    cterr_add_component("BCM82752",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82752",
                    "Check MDIO controller within the FPGA");

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("External loopback flag is off, skip the port %d external loopback test\n", port);
        return PASSED;
    }

    speed = bcm82752_get_port_speed_flag(port);
    ether = curie2ru_bcm82752_port_list[port];

    if (curie2ru_eth_port_set_speed(ether, speed)) {
        cterr('f', 0, "bcm57412 set speed failed");
        return FAILED;
    }

    if (curie2ru_bcm82752_mode_config(curie, port, speed)) {
        cterr('f', 0, "failed to configure bcm82752 1G mode");
        return FAILED;
    }

    curie2ru_pla_bcm82752_conf_sfp(q28);
    curie2ru_bcm82752_set_tx_serdes(q28);

    if (curie2ru_bcm82752_check_link(port, &sys_link, &line_link)) {
        cterr('f', 0, "bcm82752 check link failed");
        return FAILED;
    }

    if (!sys_link || !line_link) {
        unsigned int tmp = 0;
        if (!sys_link)
            cterr('f', 0, "bcm82752 no sys link");
        if (!line_link)
            cterr('f', 0, "bcm82752 no line link");
        bcm_plp_reg_value_get(q28->type, q28->info, 1, 0xc800, &tmp);
        printf("Port %d 0xc800 is %04x\n", port, tmp);
        return FAILED;
    }

    /* add a delay to fix a sync bug in bnxt_en for ethtool -S */
    curie2ru_mdelay(3000);
    curie2ru_bcm82752_pkt_message_statistic(ether);
    curie2ru_mdelay(500);

    if (curie2ru_bcm82752_socket_test(ether, ether)) {
        curie2ru_bcm82752_show_pkt_statistic();
        curie2ru_bcm82752_pkt_message_statistic(ether);
        curie2ru_bcm82752_show_pkt_statistic();
        curie2ru_bcm82752_chk_tx_serdes(q28);
        curie2ru_bcm82752_regs_dump(curie, port);
        curie2ru_bcm82752_regs_dump(curie, port);
        cterr('f', 0, "curie2ru socket test failed");
        return FAILED;
    }

    prpass(testpass, "port %d External Loopback Test Passed, ", port);

    return PASSED;
}

static long __bcm82752_prbs_line_side_test(int port,
                                           curie2ru_prbs_t prbs, uint32_t delay_sec)
{
    uint32_t enable = 1;
    curie2ru_if_side_t if_side = CURIE2RU_IF_SIDE_LINE;

    /* enable prbs */
    if (curie2ru_bcm82752_prbs_set(curie, port, if_side, prbs, enable)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 PRBS set enable failed on port %d", port);
        return FAILED;
    }

    curie2ru_mdelay(2000);
    /* clear prbs rx stat */
    if (curie2ru_bcm82752_prbs_clear_rx_stat(curie, port, if_side)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 PRBS clear rx stat failed on port %d", port);
        return FAILED;
    }
    /* check prbs */
    curie2ru_mdelay(delay_sec * 1000);
    if (curie2ru_bcm82752_prbs_check(curie, port, if_side)) {
        cterr_add_component("BCM82752",
                            "SFP plugin link status");
        cterr_add_debug("Check BCM82752",
                        "Check the SFP plugin link status");
        cterr('f', 0, "BCM82752 PRBS check failed on port %d", port);
        return FAILED;
    }

    /* disable prbs */
    enable = 0;
    if (curie2ru_bcm82752_prbs_set(curie, port, if_side, prbs, enable)) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82752 PRBS set disable failed on port %d", port);
        return FAILED;
    }

    return PASSED;
}

/* BCM82752 PRBS line side test */
static long bcm82752_prbs_line_side_test(int port)
{
    int speed;
    curie2ru_prbs_t prbs = CURIE2RU_PRBS_7;
    struct curie2ru_quadra28 *q28 = &curie->quadra28[port];

    testname("BCM82752 Port %d prbs line side", port);

    cterr_add_component("BCM82752",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82752",
                    "Check MDIO controller within the FPGA");

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("External loopback flag is off, skip the port %d external loopback test\n", port);
        return PASSED;
    }

    speed = bcm82752_get_port_speed_flag(port);
    if (curie2ru_bcm82752_mode_config(curie, port, speed)) {
        cterr('f', 0, "failed to configure bcm82752 1G mode");
        return FAILED;
    }

    curie2ru_pla_bcm82752_conf_sfp(q28);
    curie2ru_bcm82752_set_tx_serdes(q28);

    curie2ru_mdelay(500);
    if (__bcm82752_prbs_line_side_test(port, prbs, PRBS_TEST_DELAY) == FAILED) {
        curie2ru_bcm82752_chk_tx_serdes(q28);
        curie2ru_bcm82752_regs_dump(curie, port);
        curie2ru_bcm82752_regs_dump(curie, port);
        return FAILED;
    }

    prpass(testpass, "port %d prbs line side Test Passed, ", port);

    return PASSED;
}

static const reg_info_t bcm82752_quadra28_reg_tbl[] = {
    {"General Purpose Register 0", BCMI_QUADRA28_GP_0r,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 1", BCMI_QUADRA28_GP_1r,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 2", BCMI_QUADRA28_GP_2r,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"General Purpose Register 3", BCMI_QUADRA28_GP_3r,
     READ_WRITE, {2}, 0xFFFF, 0x0000},
    {"End of General Purpose Register", 0, 0, {0}, 0, 0},
};

/*
 * Function: __bcm82752_register_test
 *
 * For each register from regs, this function checks for accessibility
 * and does a ripple 1 and a ripple 0 test if applicable (not all registers
 * are W/R register).
 *
 * Input : interface structure pointer, info for all registers
 * Output: PASS/FAIL
 */
static int __bcm82752_register_test(const reg_info_t *regs,
                                    int port, curie2ru_if_side_t if_side)
{
    uint32_t i;
    uint32_t regaddr, devaddr;
    uint32_t data, data_orig, data_test;

    while (regs->size.size != 0) {
        regaddr = regs->offset;
        devaddr = regaddr >> 16;
        if (curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data_orig) < 0) {
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
                if (curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data_test) < 0 ||
                    curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data) < 0 ||
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
                if (curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data_test) < 0 ||
                    curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data) < 0 ||
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
                if (curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data_test) < 0 ||
                    curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Pattern test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return FAILED;
                }
                data = ~data;   /* complement data pattern */
            }

            /* restore original value */
            if (curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data_orig) < 0) {
                cterr('f', 0, "Error restoring %s register offset %d.%#x",
                      regs->name, devaddr, regaddr);
                return FAILED;
            }
        }
        regs++;
    }

    return PASSED;
}

static long bcm82752_register_test(void)
{
    curie2ru_if_side_t if_side = CURIE2RU_IF_SIDE_LINE;

    testname("BCM82752 Register");

    if (__bcm82752_register_test(bcm82752_quadra28_reg_tbl, 0, if_side) == FAILED ||
        __bcm82752_register_test(bcm82752_quadra28_reg_tbl, 1, if_side) == FAILED) {
        cterr_add_component("BCM82752",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82752",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "Register Test on BCM82752 failed");
        return FAILED;
    }
    prpass(testpass, "BCM82752 Register Test Passed, ");
    return PASSED;
}

static long bcm82752_toggle_port_speed_flag(int port)
{
    int *speed, current;

    speed = port == 0 ? &bcm82752_port0_speed : & bcm82752_port1_speed;

    current = *speed;
    if (*speed == CURIE2RU_PORT_SPEED_10G) {
        *speed = CURIE2RU_PORT_SPEED_1G;
    } else {
        *speed = CURIE2RU_PORT_SPEED_10G;
    }

    printf("BCM82752 Port %d current speed flag %d, set to %d", port, current, *speed);

    return 0;
}

/* entry for BCM82752 tests and utilities */
int curie2ru_bcm82752_test(int show_menu)
{
    testname("BCM82752");
    system("ifconfig eth4 up");
    system("ifconfig eth5 up");

    build_primary_submenu(bcm82752_submenu_table, BCM82752_SUBMENU_TABLE_SZ,
                          bcm82752_submenu_title, &bcm82752_submenup);
    build_secondary_submenu(bcm82752_submenu_table, BCM82752_SUBMENU_TABLE_SZ,
                            bcm82752_submenu_secondary_items);
    if (show_menu)
        menu(bcm82752_submenup, bcm82752_submenu_secondary_items, '\0');
    else
        menu_exec_doall_diags(bcm82752_submenup);
    return PASSED;
}

static long bcm82752_side_band_test (int port)
{
    int rc;
    uint32_t data, regaddr, devaddr = CURIE2RU_QUADRA28_DEV_PMA_PMD;
    uint32_t rx_los_reg, tx_flt_reg, mod_abs;
    curie2ru_if_side_t if_side = CURIE2RU_IF_SIDE_LINE;
    int jx, result = PASSED;

    printf("Please do not run it for critical phase now!\n");
    testname("BCM82752 Side Band");

    /* Optical configuration status reg 0xc8e4 */
    /* Write optical configuration control reg 0xc800 with val 0x383f */
    /* rx_los status reg. = 0xc8e4 bit6 */
    /* tx_fault status reg. = 0xc8e4 bit5 */
    /* mod_abs status reg. = 0xc8e4 bit3 (SFP present) */

    rx_los_reg = OPT_CONF_STAT_REG;
    tx_flt_reg = OPT_CONF_STAT_REG;
    mod_abs    = OPT_CONF_STAT_REG;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("\nPlease remove the SFP and loopback\n");
        /* No plug-in SFP, rx_los,tx_flt,mod_abs are high */
        /* Write 0xc800 with val 0x383f */
        data = OPT_CONF_CTRL_VAL;
        regaddr = OPT_CONF_CTRL_REG;
        rc = curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d write error reg %x", port, regaddr);
            result = FAILED;
        }

        prpass(testpass, "Port %x Side Band Test, ",port);

        /* RX_LOS Test */
        regaddr = rx_los_reg;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d read error", port);
            result = FAILED;
        }

        if (!(data & RX_LOS_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d rx_los (0x%x) error", port, data);
            result = FAILED;
        }

        /* TX_FAULT Test */
        regaddr = tx_flt_reg;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 read error");
            result = FAILED;
        }

        if (!(data & TX_FLT_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d tx_fault (0x%x) error", port, data);
            result = FAILED;
        }

        /* SFP_PRESENT Test */
        regaddr = mod_abs;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d read error", port);
            result = FAILED;
        }

        if (!(data & MOD_ABS_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d sfp_present (0x%x) error", port, data);
            result = FAILED;
        }

        if (result == PASSED) {
            printf("\nPort %d Side Band Pass\n",port);
        } else {
            goto exit;
        }
    } else {
        printf("\nPlease plug-in the SFP and loopback\n");
        /* Test1: tx_dis enable, rx_los=high, tx_flt,sfp_present=low */
        /* Write 0xc800 with val 0x383f */
        data = OPT_CONF_CTRL_VAL;
        regaddr = OPT_CONF_CTRL_REG;
        rc = curie2ru_bcm82752_write(curie, port, if_side, devaddr, regaddr, data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d write error reg %x", port, regaddr);
            result = FAILED;
        }

        prpass(testpass, "Port %x TX_DIS Enable, ",port);

        result = bcm57412_port_sideband_tx_dis(port, ENABLE);
        msleep(SIDEBAND_ASSERT_TIME);

        /* RX_LOS Test */
        regaddr = rx_los_reg;
        port = port;
        printf("\n");
        for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
            rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", port);
                result = FAILED;
            }
            if (!(data & RX_LOS_STATUS_MASK)) {
                result = FAILED;
                msleep(SIDEBAND_ASSERT_TIME);
                continue;
            }
            break;
        }

        if ((jx == SIDEBAND_TIMEOUT)) {
            cterr('f', 0, "BCM82752 port %d rx_los (0x%x) error", port, data);
            result = FAILED;
        }

        /* TX_FAULT Test */
        regaddr = tx_flt_reg;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d read error", port);
            result = FAILED;
        }

        if ((data & TX_FLT_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d tx_fault (0x%x) error", port, data);
            result = FAILED;
        }

        /* SFP_PRESENT Test */
        regaddr = mod_abs;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d read error", port);
            result = FAILED;
        }

        if ((data & MOD_ABS_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d sfp_present (0x%x) error", port,
                   data);
            result = FAILED;
        }

        if (result == PASSED) {
            printf("\nPort %d Side Band Pass, ",port);
        } else {
            goto exit;
        }

        /* Test2: tx_dis disable,  rx_los=low, tx_flt, sfp_present=low */
        prpass(testpass, "Port %x TX_DIS Disable, ",port);

        result = bcm57412_port_sideband_tx_dis(port, DISABLE);
        msleep(SIDEBAND_ASSERT_TIME);

        /* RX_LOS Test */
        regaddr = rx_los_reg;
        printf("\n");
        for (jx = 0; jx < SIDEBAND_TIMEOUT ; jx++) {
            rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
            if (rc < 0) {
                cterr('f', 0, "BCM82752 port %d read error", port);
                result = FAILED;
            }
            if ((data & RX_LOS_STATUS_MASK)) {
                result = FAILED;
                msleep(SIDEBAND_ASSERT_TIME);
                continue;
            }
            break;
        }

        if ((jx == SIDEBAND_TIMEOUT)) {
            cterr('f', 0, "BCM82752 port %d rx_los (0x%x) error", port, data);
            result = FAILED;
        }

        /* TX_FAULT Test */
        regaddr = tx_flt_reg;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d read error", port);
            result = FAILED;
        }

        if ((data & TX_FLT_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d tx_fault (0x%x) error", port, data);
            result = FAILED;
        }

        /* SFP_PRESENT Test */
        regaddr = mod_abs;
        rc = curie2ru_bcm82752_read(curie, port, if_side, devaddr, regaddr, &data);
        if (rc < 0) {
            cterr('f', 0, "BCM82752 port %d read error", port);
            result = FAILED;
        }

        if ((data & MOD_ABS_STATUS_MASK)) {
            cterr('f', 0, "BCM82752 port %d sfp_present (0x%x) error", port,
                   data);
            result = FAILED;
        }

        if (result == PASSED) {
            printf("\nPort %d Side Band Pass\n",port);
        } else {
            goto exit;
        }
    }
exit:
    return (result);
}

/*
 *-----------------------------------------------------------------------------
$Log: curie2ru_test.c,v $
Revision 1.4  2021/01/11 11:06:37  xiaolaya
*** empty log message ***

Revision 1.3  2020/03/11 17:46:59  jiajliu
Refine code for bcm utlity and test

Revision 1.2  2020/02/25 09:09:41  jiajliu
Fix an issue in BCM82752 test caused by bnxt_en: ethtool -S

Revision 1.1  2020/01/09 01:01:58  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
