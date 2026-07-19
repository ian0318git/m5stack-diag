/* $Id: diag_bcm82757_test.c,v 1.2 2021/06/02 08:22:34 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/diag_bcm82757_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_bcm82757_test.c - Fugazi BCM82757 Diag test.
 *
 * Dec. 2018, Nocken Zou <yozou@cisco.com>
 *
 * Copyright (c) 2019 - 2021 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <stdio.h>
#include <linux/mii.h>

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
#include "oir_ltc4215_api.h"
#include "platform_i2c.h"
#include "cross_platform.h"
#include "ngio.h"
#include "pca.h"
#include "slot.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "linux_api.h"
#include "dash_fpga.h"
#include "cookie_4.h"
#include "platform_fru.h"
#include "nmc93c46.h"
#include "smart_cookie.h"
#include "diag_miura_reg.h"
#include "diag_bcm_lib.h"
#include "ethernet.h"
#include "diag_eth_traf.h"
#include "diag_bcm82757_test.h"
#include "platform_ext_lpbk.h"
#include "diag_bcm57412_utils.h"
#include "platform_synce_pll_utils.h"


#define F_GRP       (MF_CONTINUOUS | MF_DOGRP)
#define F_GRP_E     (F_GRP | MF_SHOW_ERRCOUNT)
#define F_ALL       (F_GRP | MF_DOALL)
#define F_ALL_E     (F_ALL | MF_SHOW_ERRCOUNT)

struct fugazi fugazi, *fugazi_struct = &fugazi;
struct fugazi_miura *miura = &fugazi.miura;
static int fugazi_bcm82757_port_list[] = {ETH0, ETH1, ETH2, ETH3};
static int fugazi_port_speed[] = {FUGAZI_PORT_SPEED_1G, FUGAZI_PORT_SPEED_10G};

static int fugazi_bcm82757_cl37_set(fugazi_lane_t, fugazi_if_side_t, unsigned int);
static int fugazi_eth_port_set_speed(int, int);
static int fugazi_bcm82757_check_link(fugazi_lane_t, unsigned int *, unsigned int *);
static long bcm82757_show_fw_version_f(void);
static long bcm82757_analog_f(void);
static long bcm82757_force_line_side_lrm(void);
static long bcm82757_external_lpbk_port_test(void);
static long bcm82757_external_lpbk_test(fugazi_lane_t);
static long bcm82757_power_get_f(void);
static long bcm82757_power_set_f(void);

int  bcm82752_emphasis_setting(void);
long bcm82757_init_f(void);
int  bcm82757_PHY_init(int, int);
long bcm82757_reset(int);
int  bcm82757_recover_clock(int, int);
int  fugazi_bcm82757_check_link_stable(int, int, unsigned int *, unsigned int *);

/*
 * Function: fugazi_diag_init 
 *
 * Description: Initial BCM82757 chip
 *
 * Inputs      : NONE 
 * Outputs     : PASSED / FAILED
 */
int fugazi_diag_init(int argc, char *argv[])
{
    int rc = PASSED;
    char *tname = "Fugazi diag initialization";

    testname("%s", tname);
    prpass(testpass, "\n\r%s, ", tname);

    printf("Checking SyncE status of configuration loaded from EEPROM...");
    rc |= idt8a3_check_eeprom_config_status();
    if (rc == PASSED) {
    	printf("Success\n");
    } else {
        cterr('f', 0, "SyncE configuration loaded from EEPROM failed!");
        return (FAILED);
    }

    printf("Checking SyncE system PLL lock status...");
    rc |= idt8a3_check_dpll_lock_status();
    if (rc == PASSED) {
    	printf("Locked\n");
    } else {
        cterr('f', 0, "SyncE system PLL is not locked!");
        return (FAILED);
    }

    printf("Reset 10GE PHY...\n");
    bcm82757_reset(0);

    bcm82757_fw_downloaded = DISABLE; /* Skip BCM82757 FW download in the board initial */
    if ((rc |= fugazi_init(fugazi_struct))) {
        cterr('f', 0, "Fugazi initialize PHY failed!");
        return rc;
    }
    bcm82757_fw_downloaded = ENABLE;
    return 0;
}

/*
 * Function: fugazi_diag_exit
 *
 * Description: Exit BCM57412 MAC and BCM82757 PHY
 *
 * Inputs      : NONE
 * Outputs     : NONE
 */
void fugazi_diag_exit(void)
{
    fugazi_exit(fugazi_struct);
}
/*
 * Function: fugazi_bcm82757_pkt_message_statistic
 *
 * Description: Save BCM82757 ethernet packet information.
 *
 * Inputs      : port - BCM82757 10G PHY port number.
 * Outputs     : NONE
 */
static void fugazi_bcm82757_pkt_message_statistic(int port) {
    char pkg_statistic_cmd1[64];
    char pkg_statistic_cmd2[64];
    sprintf(pkg_statistic_cmd1, "\nethtool -S eth%d > /tmp/pkt_statistic_log 2>&1\n", port);
    sprintf(pkg_statistic_cmd2, "\nifconfig eth%d >> /tmp/pkt_statistic_log 2>&1\n", port);
    system(pkg_statistic_cmd1);
    system(pkg_statistic_cmd2);
}

/*
 * Function: fugazi_bcm82757_show_pkt_statistic
 *
 * Description: Display BCM82757 ethernet packet information.
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static void fugazi_bcm82757_show_pkt_statistic(void)
{
    char show_pkg_statistic_cmd[64];
    sprintf(show_pkg_statistic_cmd, "\ncat /tmp/pkt_statistic_log\n");
    system(show_pkg_statistic_cmd);
}
/*
 * Function: bcm82757_reg_read
 *
 * Description: BCM82757 register read utility
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_reg_read (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            FUGAZI_MIURA_DEV_PMA_PMD,
                            FUGAZI_MIURA_DEV_PCS,
                            FUGAZI_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0x8b00, 0, 0xffffffff);

    rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                   regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 read error");
        return (FAILED);
    }
    prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
    return (PASSED);
}
/*
 * Function: bcm82757_reg_write
 *
 * Description: BCM82757 register write utility
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_reg_write (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    devaddr = gethex_answer("Enter DEV ID(PMA/PMD:1, PCS:3, CL73_AN:7)",
                            FUGAZI_MIURA_DEV_PMA_PMD,
                            FUGAZI_MIURA_DEV_PCS,
                            FUGAZI_MIURA_DEV_CL73_AN);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = fugazi_bcm82757_write(fugazi_struct, lane, if_side, devaddr, 
                                    regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 write error");
        return (FAILED);
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return (PASSED);
}
/*
 * Function: bcm82757_reg_mdio_read
 *
 * Description: BCM82757 register read via MDIO without API
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_reg_mdio_read (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0x8b00, 0, 0xffffffff);
    if_side = 0;
    devaddr = FUGAZI_MIURA_DEV_PMA_PMD;

    rc = fugazi_bcm82757_read_mdio(fugazi_struct, lane, if_side, devaddr, 
                                   regaddr, &data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 read error");
        return (FAILED);
    }
    prt("%d.%#.4x --> %#.8x\n", devaddr, regaddr, data);
    return (PASSED);
}
/*
 * Function: bcm82757_reg_mdio_write
 *
 * Description: BCM82757 register write via MDIO without API
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_reg_mdio_write (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = 0;
    devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    regaddr = gethex_answer("Enter PHY reg(0x0 - 0xffffffff)", 0, 0, 0xffffffff);
    data = gethex_answer("Enter value", 0, 0, 0xffffffff);

    rc = fugazi_bcm82757_write_mdio(fugazi_struct, lane, if_side, devaddr, 
                                    regaddr, data);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 write error");
        return (FAILED);
    }
    prt("%d.%#.4x <-- %#.8x\n", devaddr, regaddr, data);
    return (PASSED);
}
/*
 * Function: bcm82757_regs_dump
 *
 * Description: Display BCM82757 register utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_regs_dump (void)
{
    fugazi_lane_t lane;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);

    if (fugazi_bcm82757_regs_dump(fugazi_struct, lane)) {
        prt("bcm82757_regs_dump err on lane %d\n", lane);
        return FAILED;
    }
    return (PASSED);
}
/*
 * Function: bcm82757_status_dump
 *
 * Description: Display BCM82757 status register utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_status_dump (void)
{
    int rc;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int flags;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    flags = gethex_answer("Enter Dump Flags", 0, 0, 0xffffffff);

    fugazi_struct->miura.info.flags = flags;
    rc = fugazi_bcm82757_dump(fugazi_struct, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 dump error");
        return (FAILED);
    }
    return (PASSED);
}
/*
 * Function: bcm82757_mac_diagnostic_dump
 *
 * Description: Display BCM82757 MAC register utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_mac_diagnostic_dump (void)
{
    int rc;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    rc = fugazi_bcm82757_mac_dump(fugazi_struct, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 dump error");
        return (FAILED);
    }
    return (PASSED);
}
/*
 * Function: bcm82757_link_status
 *
 * Description: Display BCM82757 link status utility
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_link_status (void)
{
    int rc;
    unsigned int link_status=0;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;

    lane = gethex_answer("Enter Port(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    if (lane == 0xff) {
        int lane_index;
        unsigned int sys_link_status, net_link_status;

        /* display all PHY port link status */
        prt("\n\rPHY port  System  Network\n");
        for (lane_index=0; lane_index<MAX_NR_FUGAZI_LANE; lane_index++) {
            sys_link_status = 0;
            net_link_status = 0;
            rc |= fugazi_bcm82757_link_status(fugazi_struct, lane_index,
                                        FUGAZI_IF_SIDE_SYS, &sys_link_status);
            rc |= fugazi_bcm82757_link_status(fugazi_struct, lane_index,
                                        FUGAZI_IF_SIDE_LINE, &net_link_status);
            prt(" %d          %s      %s \n", lane_index,
                (sys_link_status)? "up" : "down", 
                (net_link_status)? "up" : "down");
        }
    } else {
        if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

        if (lane >= MAX_NR_FUGAZI_LANE) {
            printf("ERROR: incorrect Port number %d, exceed max Port %d\n",
                    lane, (MAX_NR_FUGAZI_LANE-1));
            return (FAILED);
        }

        /* display particular PHY port link status */
        rc = fugazi_bcm82757_link_status(fugazi_struct, lane, if_side, &link_status);
        prt("PHY port %d link status at %s side: %s (%d)\n", lane,
            (if_side)? "System" : "Network",
            (link_status)? "up" : "down", link_status);
    }

    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 link status get error");
        return (FAILED);
    }

    return (rc);
}
/*
 * Function: bcm82757_firmware_download
 *
 * Description: Download BCM82757 firmware utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_firmware_download (void)
{
    struct fugazi_miura *miura = &fugazi_struct->miura;

    fugazi_miura_reset(miura);

    bcm82757_fw_downloaded = ENABLE;
    if (fugazi_bcm82757_init(fugazi_struct)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within the FPGA");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within the FPGA");
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm82757_config_loopback
 *
 * Description: Enable BCM82757 loopback utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_config_loopback (void)
{
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int lb_mode, enable;
    unsigned int lp_type;

    lane = gethex_answer("Enter Port(0, 3)", 0, 0, 3);
    lp_type = getdec_answer("Enter Loopback mode (1:System Shallow, "
                            "2:System Deep, 3:Line Shallow, 4:Line Deep)",
                            1, 0, 4);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    switch (lp_type) {
    case 1: /* System Shallow loopback */
    	if_side = FUGAZI_IF_SIDE_SYS;
    	lb_mode = REMOTE_PMD;    /* Remote PMD */
    	break;
    case 2: /* System Deep loopback */
    	if_side = FUGAZI_IF_SIDE_LINE;
    	lb_mode = DIGITAL_PMD;   /* Digital PMD */
    	break;
    case 3: /* Line Shallow loopback */
    	if_side = FUGAZI_IF_SIDE_LINE;
    	lb_mode = REMOTE_PMD;    /* Remote PMD */
    	break;
    case 4: /* Line Deep loopback */
    	if_side = FUGAZI_IF_SIDE_SYS;
    	lb_mode = DIGITAL_PMD;   /* Digital PMD */
    	break;
    default:
        printf("\nUnknow Loopback type %d!!\n\r", lp_type);
        return (FAILED);
    }

    if (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, lb_mode, 
                                     enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return (FAILED);
    }
    fugazi_bcm82757_loopback_get(fugazi_struct, lane, if_side, lb_mode, &enable);
    if (enable) {
        printf("\nLoopback mode enable");
    } else {
        printf("\nLoopback mode disable");
    }

    return (PASSED);
}

/*
 * Function: bcm82757_config_prbs
 *
 * Description: Config BCM82757 PRBS utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_config_prbs (void)
{
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int action, poly, enable = 0;
    unsigned int tx_rx = 0;
    fugazi_prbs_t prbs = FUGAZI_PRBS_31;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (if_side == FUGAZI_IF_SIDE_SYS) {
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
    tx_rx = getdec_answer("Enter direction(both:0, Receive side:1, Transmit side:2)",
                           0, 0, 2);
    action = gethex_answer("Enter Action(Check:0, Enable:1, Disable:2, Clear:3)",
                           0, 0, 3);

    switch (action) {
    default:
    case 0:
        if (fugazi_bcm82757_prbs_check(fugazi_struct, lane, if_side)) {
            cterr('f', 0, "BCM82757 PRBS check failed");
            return (FAILED);
        }
        break;

    case 1:
        poly = getdec_answer("PRBS Polynomial(7, 9, 11, 15, 23, 31)", 31, 0, 31);
        switch (poly) {
        case 7:
            prbs = FUGAZI_PRBS_7;
            break;
        case 9:
            prbs = FUGAZI_PRBS_9;
            break;
        case 11:
            prbs = FUGAZI_PRBS_11;
            break;
        case 15:
            prbs = FUGAZI_PRBS_15;
            break;
        case 23:
            prbs = FUGAZI_PRBS_23;
            break;
        default:
        case 31:
            prbs = FUGAZI_PRBS_31;
            break;
        }
        enable = 1;
    case 2:
        if (fugazi_bcm82757_prbs_set(fugazi_struct, lane, if_side, tx_rx, prbs, enable)) {
            cterr('f', 0, "BCM82757 PRBS set failed");
            return (FAILED);
        }
        break;
    case 3:
    	/* clear PRBS error counters */
        if (fugazi_bcm82757_prbs_clear_error(fugazi_struct, lane, if_side)) {
            cterr('f', 0, "BCM82757 PRBS clear error counter failed");
            return (FAILED);
        }
        break;
    }

    return (PASSED);
}

/*
 * Function: bcm82757_firmware_lane_config_set
 *
 * Description: Setup BCM82757 Lane Config utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_firmware_lane_config_set (void)
{
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    bcm_plp_pm_firmware_lane_config_t firmware_lane_config;

    cterr_add_component("BCM82757",
                        "MDIO controller within BCM57412");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within BCM57412");
    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 1, 0, 1);
    if (fugazi_bcm82757_firmware_lane_get(fugazi_struct, lane, if_side, 
                                          &firmware_lane_config)) {
        cterr('f', 0, "fugazi firmware lane get failed");
        return (FAILED);
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

    if (fugazi_bcm82757_firmware_lane_set(fugazi_struct, lane, if_side, 
                                          &firmware_lane_config)) {
        cterr('f', 0, "fugazi firmware lane set failed");
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm82757_config_cl73
 *
 * Description: Config BCM82757 cl73 utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_config_cl73 (void)
{
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int enable;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    /* Enable cl73, disable cl37 */
    if (fugazi_bcm82757_cl73_set(fugazi_struct, lane, if_side, enable, 0)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm82757_config_cl37
 *
 * Description: Config BCM82757 cl37 utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_config_cl37 (void)
{
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int enable;

    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1)", 1, 0, 1);

    if (fugazi_bcm82757_cl37_set(lane, if_side, enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm82757_config_macsec_bypass
 *
 * Description: Setup BCM82757 MACSEC bypass utility
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_config_macsec_bypass (void)
{
    int speed, speed_flag;
    fugazi_lane_t lane;
    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    speed_flag = gethex_answer("speed(1G:0, 10G:1)", 1, 0, 1);
    speed = fugazi_port_speed[speed_flag];
    if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, lane, speed)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm82757_interrupt
 *
 * Description: Enable/Disable BCM82757 LASI Interrupt utility 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_interrupt (void)
{
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int enable, intr_type, intr_data = 0;
    int lane_start, lane_end;


    lane = gethex_answer("Enter Port(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    if (lane == 0xff) {
        lane_start = 0;
        lane_end = MAX_NR_FUGAZI_LANE;
    } else {
        lane_start = lane;
        lane_end = lane + 1;
    }
    if_side = FUGAZI_IF_SIDE_SYS;
    enable = gethex_answer("Enter Enable(Disable:0, Enable:1, Clear:2)", 1, 0, 2);
    intr_type = BCM82757_LINK_DOWN_INT; /* Link Down Interrupt*/

    for (lane=lane_start; lane<lane_end; lane++) {
        if (fugazi_bcm82757_interrupt_set(fugazi_struct, lane, if_side, intr_type,
                                      enable)) {
            cterr_add_component("BCM82757", "MDIO controller within BCM57412");
            cterr_add_debug("Check BCM82757",
                            "Check MDIO controller within BCM57412");
            return (FAILED);
        }
        if (enable == BCM82757_INT_CLEAR) {
            printf("\nPort %x Interrupt is cleared",lane);
        } else {
            fugazi_bcm82757_interrupt_get(fugazi_struct, lane, if_side,
                                          intr_type, &intr_data);
            if (intr_type == intr_data) {
                printf("\nPort %x Interrupt mode enable",lane);
            } else {
                printf("\nPort %x Interrupt mode disable",lane);
            }
        }
    } /*for (lane=lane_start; lane<lane_end; lane++) { */

    return (PASSED);
}
/*
 * Function: bcm82757_display_eye_scan
 *
 * Description: BCM82757 EYE scan tool utility
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_display_eye_scan (void)
{
    int rc;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    int speed, port, ix;
    unsigned int sys_link, line_link;
    printf("\nPlease plug-in SFP and loopback before Eye scan\n");
    lane = gethex_answer("Enter Port(0 ~ 3)", 0, 0, 3);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);

    /* Initial PHY and check link status */
    ix = lane;
    port = fugazi_bcm82757_port_list[ix];
    speed = FUGAZI_PORT_SPEED_10G;
    
    if (fugazi_eth_port_set_speed(port, speed)) {
        cterr('f', 0, "bcm57412 set speed failed");
        return (FAILED);
    }
    if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, ix, speed)) {
        cterr('f', 0, "set macsec 10g bypass failed");
        return (FAILED);
    }
    fugazi_bcm82757_config_macsec_cleanup(fugazi_struct, ix);

    /* Checking link up with particular link side which perform eye scan */
    if (fugazi_bcm82757_check_link(ix, &sys_link, &line_link)){
        cterr('f', 0, "bcm82757 check link failed");
        return (FAILED);
    }
    if ((!sys_link) && (if_side == FUGAZI_IF_SIDE_SYS)) {
        cterr('f', 0, "bcm82757 System side no link");
        return (FAILED);
    }
    if ((!line_link) && (if_side == FUGAZI_IF_SIDE_LINE)) {
        cterr('f', 0, "bcm82757 Line side no link");
        return (FAILED);
    }

    rc = fugazi_bcm82757_display_eye_scan(fugazi_struct, lane, if_side);
    if (rc < 0) {
        cterr_add_component("BCM82757", "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 eye scan failed");
        return (FAILED);
    }
    return (PASSED);
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
 * Function: __bcm82757_register_test
 *
 * Description: BCM82757 register test
 *
 * Inputs      : regs : which register
 *               lane : test lane number
 *               if_side : Line or Sys side
 * Outputs     : PASSED / FAILED
 */
static int __bcm82757_register_test (const reg_info_t *regs, fugazi_lane_t lane, 
                                     fugazi_if_side_t if_side)
{
    uint32_t i;
    uint32_t regaddr, devaddr;
    uint32_t data, data_orig, data_test;

    while (regs->size.size != 0) {
        regaddr = regs->offset;
        devaddr = regaddr >> 16;
        if (fugazi_bcm82757_read_mdio(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data_orig) < 0) {
            cterr('f', 0, "Error reading %s register offset %d.%#x",
                  regs->name, devaddr, regaddr);
            return (FAILED);
        }

        if (regs->type == READ_WRITE) {
            /* ripple 1 test */
            for (i = 0; i < regs->size.size * 8; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                /* Write to register under test */
                if (fugazi_bcm82757_write_mdio(fugazi_struct, lane, if_side, 
                                               devaddr, regaddr, data_test) < 0 ||
                    fugazi_bcm82757_read_mdio(fugazi_struct, lane, if_side, 
                                              devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Ripple one test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return (FAILED);
                }
            }

            /* ripple 0 test */
            for (i = 0; i < regs->size.size * 8; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                data_test = (~(1 << i)) & regs->mask;
                /* Write to register under test */
                if (fugazi_bcm82757_write_mdio(fugazi_struct, lane, if_side, 
                                               devaddr, regaddr, data_test) < 0 ||
                    fugazi_bcm82757_read_mdio(fugazi_struct, lane, if_side, 
                                              devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Ripple zero test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return (FAILED);
                }
            }

            /* pattern test */
            data = 0x5adb;
            for (i = 0; i < 2; i++) {
                data_test = (1 << i) & regs->mask;
                if (!data_test)
                    continue;
                /* Write to register under test */
                if (fugazi_bcm82757_write_mdio(fugazi_struct, lane, if_side, 
                                               devaddr, regaddr, data_test) < 0 ||
                    fugazi_bcm82757_read_mdio(fugazi_struct, lane, if_side, 
                                              devaddr, regaddr, &data) < 0 ||
                    (data & regs->mask) != data_test) {
                    cterr('f', 0, "Pattern test failed when accessing %s "
                          "Register offset %d.%#x, Expect %#x, Read %#x",
                          regs->name, devaddr, regaddr, data_test, data);
                    return (FAILED);
                }
                data = ~data;   /* complement data pattern */
            }

            /* restore original value */
            if (fugazi_bcm82757_write_mdio(fugazi_struct, lane, if_side, devaddr, 
                                           regaddr, data_test) < 0) {
                cterr('f', 0, "Error restoring %s register offset %d.%#x",
                      regs->name, devaddr, regaddr);
                return (FAILED);
            }
        }
        regs++;
    }

    return (PASSED);
}

/*
 * Function: bcm82757_register_test
 *
 * Description: BCM82757 register test wrapper function
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_register_test (void)
{
    fugazi_if_side_t if_side = FUGAZI_IF_SIDE_SYS;
    int ix, result = PASSED;

    testname("BCM82757 Register");

    for (ix = FUGAZI_LANE_0; ix < MAX_NR_FUGAZI_LANE; ix++) {
        prpass(testpass, "\nBCM82757 Port %x, ", ix);
        if (__bcm82757_register_test(bcm82757_miura_reg_tbl, FUGAZI_LANE_0 + ix ,
                                     if_side) == FAILED) {
            cterr_add_component("BCM82757", "MDIO controller within BCM57412");
            cterr_add_debug("Check BCM82757",
                            "Check MDIO controller within BCM57412");
            cterr('f', 0, "Register Test on BCM82757 prot %x failed", 
                  FUGAZI_LANE_0 + ix);
            result = FAILED;  /* fail through */
        }
        printf("\nDBG : BCM82757 Port %x pass, ", ix);

    }
    prpass(testpass, "BCM82757 Register Test Passed, ");
    return (result);
}
/*
 * Function: bcm82757_internal_lpbk_test
 *
 * Description: BCM82757 PHY internal test 
 *
 * Inputs      : lane : lane number
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_internal_lpbk_test (fugazi_lane_t lane)
{
    int speed, port, ix, rc, result = PASSED;
    unsigned int sys_link, line_link;
    int retry = 0, link_up_count = 0;

    cterr_add_component("BCM82757", "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757", "Check MDIO controller within the FPGA");
    for (ix = lane; ix < BCM82757_LANE_MAX; ix++) {
        testname("BCM82757 Port %d Internal lpbk", ix);
        rc = PASSED; 
        port = fugazi_bcm82757_port_list[ix];
        speed = FUGAZI_PORT_SPEED_10G;
        if (fugazi_eth_port_set_speed(port, speed)) {
            cterr('f', 0, "bcm57412 port %d set speed failed", ix);
            result = FAILED;
        }
    
        if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, ix, speed)) {
            cterr('f', 0, "port %d set macsec 10g bypass failed", ix);
            result = FAILED;
        }
    
        if (fugazi_bcm82757_loopback_set(fugazi_struct, ix, FUGAZI_IF_SIDE_LINE, 
                                         DIGITAL_PMD, ENABLE)) {
            cterr('f', 0, "port %d set loopback failed", ix);
            result = FAILED;
        }
    
        retry = 0;
        link_up_count = 0;
        /* CSCvo59196-15 Fugazi: BCM82757 loopback test failed get hand up on Apollo */
        /* max wait timeout for link up is 1 min */
        while (retry++ < LINK_MAX_CHECK) {
            if (fugazi_bcm82757_check_link(ix, &sys_link, &line_link)){
                cterr('f', 0, "bcm82757 port %d check link failed", ix);
                result = FAILED;
            }
            if (!sys_link) {
                link_up_count = 0;
            } else {
                link_up_count++;
            }
        
            /* All up, return now */
            if (link_up_count >= MAX_LINKUP_CONSISTENCY) {
        	    printf("\nBCM82757 port %d link_up\n", ix);
                break;
            } else { /* Delay and try again */
                fugazi_mdelay(10);
            }
        }
        /* Link up : only if it is consistent up 1 second */
        if (link_up_count < MAX_LINKUP_CONSISTENCY) {
        	printf("%s: link_up_count=%d, retry=%d,\n",
                    __FUNCTION__, link_up_count, retry);
            cterr('f', 0, "bcm82757 sys no link");
            result = FAILED;
        }

    	fugazi_bcm82757_pkt_message_statistic(port);
    
        fugazi_mdelay(500);
        /* CSCvq67625 : Fixed BCM82757 Loopback Test lost packet issue */
        prpass(testpass, "Start Packet Tx/Rx Test, ");
        if ((rc = fugazi_set_packet(BCM82757_PORT_ETH, port, speed)) == FAILED) {
            printf("\n\nIn the end of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            show_eth_counter(BCM82757_PORT_ETH, port);
            fugazi_bcm82757_show_pkt_statistic();
    	    fugazi_bcm82757_pkt_message_statistic(port);
            fugazi_bcm82757_show_pkt_statistic();
            fugazi_bcm82757_regs_dump(fugazi_struct, ix);
            cterr('f', 0, "fugazi socket test failed");
            result = rc = FAILED;
        }
 
        if (fugazi_bcm82757_loopback_set(fugazi_struct, ix, FUGAZI_IF_SIDE_LINE, 
                                         DIGITAL_PMD, DISABLE)) {
            cterr('f', 0, "port %d set loopback failed", ix);
            result = FAILED;
        }
        prpass(testpass, "Port %d Internal Loopback Test %s \n", ix, 
               rc? "Failed" : "Passed");
        fugazi_bcm82757_config_macsec_cleanup(fugazi_struct, ix);
    }

    return (result);
 }

/*
 * Function: bcm82757_external_lpbk_port_test
 *
 * Description: BCM82757 PHY external signal port lopback test
 *
 * Inputs      : lane : lane number
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_external_lpbk_port_test (void)
{
    fugazi_lane_t lane;

    lane = gethex_answer("Enter Port(1 ~ 4; 0x0-all ports)", 0, 1, 4);
    bcm82757_external_lpbk_test(lane);
    return (PASSED);
}
/*
 * Function: bcm82757_external_lpbk_test
 *
 * Description: BCM82757 PHY external lopback test
 *
 * Inputs      : lane : lane number
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_external_lpbk_test (fugazi_lane_t lane)
{
    int speed, port, max_port, ix, rc, result = PASSED;
    unsigned int sys_link, line_link;
    int retry = 0, link_up_count = 0;

    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        prt("External loopback flag is off, skip external loopback test\n");
        return (PASSED);
    }

    cterr_add_component("BCM82757",
                        "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");
    if (lane != FUGAZI_LANE_0) {
        max_port = lane;
        lane--;
    } else {
        max_port = BCM82757_LANE_MAX;
    }
    for (ix = lane; ix < max_port; ix++) {
        testname("BCM82757 Port %d External lpbk", ix);
        rc = PASSED; 
        if (fugazi_sfp_present(ix)) {
            cterr('f', 0, "Fugazi port %d sfp not present", ix);
            result = FAILED;
        }
    
        port = fugazi_bcm82757_port_list[ix];
        speed = FUGAZI_PORT_SPEED_10G;
        if (fugazi_eth_port_set_speed(port, speed)) {
            cterr('f', 0, "bcm57412 port %d set speed failed", ix);
            result = FAILED;
        }
    
        if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, ix, speed)) {
            cterr('f', 0, "set port %d macsec 10g bypass failed", ix);
            result = FAILED;
        }
        retry = 0;
        link_up_count = 0;
        /* CSCvo59196-15 Fugazi: BCM82757 loopback test failed get hand up on Apollo */
        /* max wait timeout for link up is 1 min */
        while (retry++ < LINK_MAX_CHECK) { 
            if (fugazi_bcm82757_check_link(ix, &sys_link, &line_link)){
                cterr('f', 0, "bcm82757 port %d check link failed", ix);
                result = FAILED;
            }
            if (!sys_link || !line_link) {
                link_up_count = 0;
            } else {
                link_up_count++;
            }
        
            /* All up, return now */
            if (link_up_count >= MAX_LINKUP_CONSISTENCY) {
        	    printf("\nBCM82757 port %d link_up\n", ix);
                break;
            } else { /* Delay and try again */
                fugazi_mdelay(10);
            }
        }
        /* Link up : only if it is consistent up for 1 second */
        if (link_up_count < MAX_LINKUP_CONSISTENCY) {
            printf("%s: link_up_count=%d, retry=%d (sys_link %s, line_link %s)\n",
                    __FUNCTION__, link_up_count, retry,
                    (sys_link)? "up":"down", (line_link)? "up":"down");
            cterr('f', 0, "bcm82757 port %d no link", ix);
            result = FAILED;
        }

    	fugazi_bcm82757_pkt_message_statistic(port);
    
        fugazi_mdelay(500);
        /* CSCvq67625 : Fixed BCM82757 Loopback Test lost packet issue */
        prpass(testpass, "Start Packet Tx/Rx Test, ");
        if ((rc = fugazi_set_packet(BCM82757_PORT_ETH, port, speed)) == FAILED) {
            printf("\n\nIn the end of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            show_eth_counter(BCM82757_PORT_ETH, port);
            fugazi_bcm82757_show_pkt_statistic();
    	    fugazi_bcm82757_pkt_message_statistic(port);
            fugazi_bcm82757_show_pkt_statistic();
            fugazi_bcm82757_regs_dump(fugazi_struct, ix);
            cterr('f', 0, "fugazi socket test failed");
            result = rc = FAILED;
        }

        prpass(testpass, "Port %d External Loopback Test %s \n", ix, 
               rc? "Failed" : "Passed");
        fugazi_bcm82757_config_macsec_cleanup(fugazi_struct, ix);
    }
    return (result);
}

/*
 * Function: bcm82757_cl37_external_lpbk_test
 *
 * Description: BCM82757 PHY C137 1G external lopback test
 *
 * Inputs      : lane : lane number
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_cl37_external_lpbk_test (fugazi_lane_t lane)
{
    int speed, port, ix, rc, result = PASSED;
    unsigned int sys_link, line_link;
    int retry = 0, link_up_count = 0;

    cterr_add_component("BCM82757", "MDIO controller within the FPGA");
    cterr_add_debug("Check BCM82757",
                    "Check MDIO controller within the FPGA");

    for (ix = lane; ix < BCM82757_LANE_MAX; ix++) {
        testname("BCM82757 Port %d cl37 External lpbk", ix);
        rc = PASSED; 
        if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            prt("cl37 External loopback flag is off, skip the Port %d external"
                " loopback test\n", ix);
            return (PASSED);
        }
    
        if (fugazi_sfp_present(ix)) {
            cterr('f', 0, "Fugazi port %d sfp not present", ix);
            result = FAILED;
        }
    
        port = fugazi_bcm82757_port_list[ix];
        speed = FUGAZI_PORT_SPEED_1G;
        if (fugazi_eth_port_set_speed(port, speed)) {
            cterr('f', 0, "bcm57412 set speed failed");
            result = FAILED;
        }
    
        if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, ix, speed)) {
            cterr('f', 0, "port %d set macsec 10g bypass failed", ix);
            result = FAILED;
        }
    
        if (fugazi_bcm82757_cl37_set(ix, FUGAZI_IF_SIDE_LINE, 1)) {
            cterr('f', 0, "port %d set cl37 failed", ix);
            result = FAILED;
        }
    
        retry = 0;
        link_up_count = 0;
        /* CSCvo59196-15 Fugazi: BCM82757 loopback test failed get hand up on Apollo */
        /* max wait timeout for link up is 1 min */
        while (retry++ < LINK_MAX_CHECK) {
            if (fugazi_bcm82757_check_link(ix, &sys_link, &line_link)){
                cterr('f', 0, "bcm82757 port %d check link failed", ix);
                result = FAILED;
            }
            if (!sys_link || !line_link) {
                link_up_count = 0;
            } else {
                link_up_count++;
            }
        
            /* All up, return now */
            if (link_up_count >= MAX_LINKUP_CONSISTENCY) {
        	    printf("\nBCM82757 port %d link_up\n", ix);
                break;
            } else { /* Delay and try again */
                fugazi_mdelay(10);
            }
        }
        /* Link up : only if it is consistent up for 1 second */
        if (link_up_count < MAX_LINKUP_CONSISTENCY) {
            printf("%s: link_up_count=%d, retry=%d (sys_link %s, line_link %s)\n",
                    __FUNCTION__, link_up_count, retry,
                    (sys_link)? "up":"down", (line_link)? "up":"down");
            cterr('f', 0, "bcm82757 port %d no link", ix);
            result = FAILED;
        }


    	fugazi_bcm82757_pkt_message_statistic(port);
    
        fugazi_mdelay(500);
        /* CSCvq67625 : Fixed BCM82757 Loopback Test lost packet issue */
        prpass(testpass, "Start Packet Tx/Rx Test, ");
        if ((rc = fugazi_set_packet(BCM82757_PORT_ETH, port, speed)) == FAILED) {
            printf("\n\nIn the end of the test - Display Linux "
                   "Ethernet counters - speed = %d\n", speed);
            show_eth_counter(BCM82757_PORT_ETH, port);
            fugazi_bcm82757_show_pkt_statistic();
    	    fugazi_bcm82757_pkt_message_statistic(port);
            fugazi_bcm82757_show_pkt_statistic();
            fugazi_bcm82757_regs_dump(fugazi_struct, ix);
            cterr('f', 0, "fugazi port %d socket test failed", ix);
            result = rc = FAILED;
        }
    
        if (fugazi_bcm82757_cl37_set(ix, FUGAZI_IF_SIDE_LINE, 0)) {
            cterr('f', 0, "port %d set cl37 failed", ix);
            result = FAILED;
        }
    
        prpass(testpass, "Port %d cl37 External Loopback Test %s \n", ix, 
               rc? "Failed" : "Passed");
        fugazi_bcm82757_config_macsec_cleanup(fugazi_struct, ix);
    }
    return (result);
}


/*
 * Function: __bcm82757_prbs_line_side_test
 *
 * Description: BCM82757 PRBS line side loopback test
 *
 * Inputs      : lane : lane number
 *               prbs : which prbs
 *               delay_sec : delay time
 * Outputs     : PASSED / FAILED
 */
static long __bcm82757_prbs_line_side_test (fugazi_lane_t lane,
                                            fugazi_prbs_t prbs, uint32_t delay_sec)
{
    uint32_t enable = 1;
    fugazi_if_side_t if_side = FUGAZI_IF_SIDE_LINE;
    unsigned int tx_rx = 0;

    /* enable prbs */
    if (fugazi_bcm82757_prbs_set(fugazi_struct, lane, if_side, tx_rx, prbs, 
                                 enable)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 PRBS set enable failed on port %d", lane);
        return (FAILED);
    }

    /* clear prbs rx stat*/
    mdelay(delay_sec * 1000);
    if (fugazi_bcm82757_prbs_clear_rx_stat(fugazi_struct, lane, if_side)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 PRBS clear rx stat failed on lane %d", lane);
        return (FAILED);
    }

    /* check prbs */
    mdelay(delay_sec * 1000);
    if (fugazi_bcm82757_prbs_check(fugazi_struct, lane, if_side)) {
        cterr_add_component("BCM82757", "SFP plugin link status");
        cterr_add_debug("Check BCM82757",
                        "Check the SFP plugin link status");
        cterr('f', 0, "BCM82757 PRBS check failed on port %d", lane);
        return (FAILED);
    }

    /* disable prbs */
    enable = 0;
    if (fugazi_bcm82757_prbs_set(fugazi_struct, lane, if_side, tx_rx, prbs, enable)) {
        cterr_add_component("BCM82757", "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 PRBS set disable failed on port %d", lane);
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: bcm82757_prbs_line_side_test
 *
 * Description: BCM82757 PRBS line side loopback test wrapper function
 *
 * Inputs      : lane : lane number
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_prbs_line_side_test (fugazi_lane_t lane)
{
    int speed, ix, result = PASSED;
    fugazi_prbs_t prbs;
    int prbs_list[] = {7, 9, 11, 15, 23, 31};

    speed = FUGAZI_PORT_SPEED_10G;

    for (ix = lane; ix < BCM82757_LANE_MAX; ix++) {
        testname("BCM82757 Port %d Line Side PRBS \n", ix);
        if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
            prt("External loopback flag is off, skip the port %d Line Side PRBS"
                " Test\n", ix);
            return (PASSED);
        }
    
        if (fugazi_sfp_present(ix)) {
            cterr('f', 0, "Fugazi port %d sfp not present", ix);
            result = FAILED;
        }
    
        if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, ix, speed)) {
            cterr('f', 0, "port %d set macsec 10g bypass failed", ix);
            result = FAILED;
        }
        prbs = FUGAZI_PRBS_7;
        while (prbs <= FUGAZI_PRBS_31) {
            printf("\nBCM82757 Port %d Line Side PRBS_%d Test\n",ix, 
                    prbs_list[prbs]);
            if (__bcm82757_prbs_line_side_test(ix, prbs, PRBS_TEST_DELAY) == 
                FAILED) {
                result = FAILED;
            }
            prbs++;
        }
    
        prpass(testpass, "Port %d Line Side PRBS Test Passed, ", ix);
        fugazi_bcm82757_config_macsec_cleanup(fugazi_struct, ix);
    }
    return (result);
}

/******************************************************************************
 *  List of Utilities used for BCM82757
 *****************************************************************************/
static submenu_xtable_t bcm82757_util_items[] = {
    {"BCM82757 Register Read", bcm82757_reg_read, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Write", bcm82757_reg_write, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Read (MDIO)", bcm82757_reg_mdio_read, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Register Write (MDIO)", bcm82757_reg_mdio_write, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Status dump", bcm82757_status_dump, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Regs dump", bcm82757_regs_dump, 0,
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
    {"BCM82757 Show Firmware Version", bcm82757_show_fw_version_f, 0,
     0, NULL, 0, NULL, 0},
    {"Reset BCM82757 PHY", bcm82757_reset, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Interrupt utility", bcm82757_interrupt, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 initialization", bcm82757_init_f, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Analog utility", bcm82757_analog_f, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Force Line Side LRM", bcm82757_force_line_side_lrm, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Signal port External loopback test", bcm82757_external_lpbk_port_test, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Get Power Status utility", bcm82757_power_get_f, 0,
     0, NULL, 0, NULL, 0},
    {"BCM82757 Set Power Status utility", bcm82757_power_set_f, 0,
     0, NULL, 0, NULL, 0},
};
#define BCM82757_TESTS_UTIL_SIZE (sizeof(bcm82757_util_items) / sizeof(submenu_xtable_t))
static mitem_t bcm82757_tests_primary_util_items[BCM82757_TESTS_UTIL_SIZE +
                                                 MAX_BASE_ITEMS];
static mitem_t bcm82757_tests_secondary_util_items[BCM82757_TESTS_UTIL_SIZE +
                                                   MAX_BASE_ITEMS];

menuinfo_t bcm82757_util_menu = {
    "BCM82757 Utility Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    bcm82757_tests_primary_util_items,
};
menuinfo_t *bcm82757_util_menup = &bcm82757_util_menu;

/*******************************************************************************
 *
 * Function    : bcm82757_utility
 * Description :
 * Inputs      : menu_option - display utility for BCM82757
 *               tests.
 * Outputs     : PASSED / FAILED
 *
 ******************************************************************************/
static int bcm82757_utility (int show_menu)
{
    build_primary_submenu(bcm82757_util_items, BCM82757_TESTS_UTIL_SIZE,
                          "BCM57412 Utilities Menu", &bcm82757_util_menup);
    build_secondary_submenu(bcm82757_util_items, BCM82757_TESTS_UTIL_SIZE,
                            bcm82757_tests_secondary_util_items);

    menu(bcm82757_util_menup, bcm82757_tests_secondary_util_items, '\0' );

    return (PASSED);
}


/* BCM82757 submenu items */
static submenu_xtable_t bcm82757_submenu_table[] = {
    {"BCM82757 Register Test", bcm82757_register_test, 0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Line Side PRBS Test", bcm82757_prbs_line_side_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Internal Loopback Test", bcm82757_internal_lpbk_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 External loopback Test", bcm82757_external_lpbk_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 cl37 External loopback Test", bcm82757_cl37_external_lpbk_test, FUGAZI_LANE_0,
     F_ALL_E, NULL, 0, NULL, 0},
    {"BCM82757 Utility", (type_t(*)())bcm82757_utility, FALSE,
     0, NULL, 0, (type_t(*)())bcm82757_utility, TRUE},
};

#define BCM82757_SUBMENU_TABLE_SZ (sizeof(bcm82757_submenu_table) / sizeof(submenu_xtable_t))

static mitem_t bcm82757_submenu_primary_items[BCM82757_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];
static mitem_t bcm82757_submenu_secondary_items[BCM82757_SUBMENU_TABLE_SZ+MAX_BASE_ITEMS];

static char bcm82757_submenu_title[] = "Fugazi BCM82757 10G PHY Subtest Menu";

static menuinfo_t bcm82757_submenu = {
    bcm82757_submenu_title,
    0,                          /* mtparam added by init_empty_menu */
    (PFT)show_endnote,          /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    bcm82757_submenu_primary_items,
};

static menuinfo_t *bcm82757_submenup = &bcm82757_submenu;

/*
 * Function: fugazi_bcm82757_test
 *
 * Description: Build the BCM82757 main menu
 *
 * Inputs      : show_menu - display menu instead of running all BCM82757 tests.
 * Outputs     : PASSED / FAILED
 */
int fugazi_bcm82757_test (int show_menu)
{
    testname("BCM82757");

    if ((fugazi_bcm82757_init(fugazi_struct))) {
        cterr('f', 0, "fugazi_bcm82757_init failed");
    }

    build_primary_submenu(bcm82757_submenu_table, BCM82757_SUBMENU_TABLE_SZ,
                          bcm82757_submenu_title, &bcm82757_submenup);
    build_secondary_submenu(bcm82757_submenu_table, BCM82757_SUBMENU_TABLE_SZ,
                            bcm82757_submenu_secondary_items);

    /* Emphasis compliance setting */
    bcm82752_emphasis_setting();

    if (show_menu) {
        menu_exec_doall_diags(bcm82757_submenup);
    } else {
        menu(bcm82757_submenup, bcm82757_submenu_secondary_items, '\0');
    }
    return (PASSED);
}

/*
 * Function: fugazi_bcm82757_cl37_set
 *
 * Description: BCM82757 setup C137 utility
 *
 * Inputs      : lane - test lane number
 *               if_side - test from witch side
 *               enable - enable / disable
 * Outputs     : PASSED / FAILED
 */
static int fugazi_bcm82757_cl37_set (fugazi_lane_t lane, fugazi_if_side_t if_side, unsigned int enable)
{
    int rc;

    /* CSCvo59196-31: call bcm82757 API to config cl37 (same as Curie2RU),
     * instead of by directly write config value to bcm82757 registers */
    /* Disable cl73, enable cl37 */
    rc = fugazi_bcm82757_cl73_set(fugazi_struct, lane, if_side, enable, 1);
    if (rc < 0) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        cterr('f', 0, "BCM82757 write error");
        return (FAILED);
    }

    return (PASSED);

}

/*
 * Function: fugazi_eth_port_set_speed
 *
 * Description: Setup BCM82757 PHY speed utility
 *
 * Inputs      : port - Port number
 *               speed - PHY speed
 * Outputs     : PASSED / FAILED
 */
static int fugazi_eth_port_set_speed (int port, int speed)
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
        return (FAILED);
    }

    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    rc = ioctl(sockfd, SIOCETHTOOL, &ifr);
    if (rc < 0) {
        cterr('f', 0, "eth port get current setting failed");
        return (FAILED);
    }
    ecmd.cmd = ETHTOOL_SSET;
    ecmd.speed = speed;
    rc = ioctl(sockfd, SIOCETHTOOL, &ifr);
    if (rc < 0) {
        cterr('f', 0, "eth port speed set failed");
        return (FAILED);
    }
    close(sockfd);
    return (PASSED);
}

/*
 * Function: fugazi_bcm82757_check_link
 *
 * Description: Check BCM82757 PHY link status utility
 *
 * Inputs      : lane - lane number
 *               sys_link - link with sys side.
 *               line_link - link with line side.
 * Outputs     : PASSED / FAILED
 */
static int fugazi_bcm82757_check_link (fugazi_lane_t lane, 
                                       unsigned int *sys_link, 
                                       unsigned int *line_link)
{
    int i, rc;
    fugazi_if_side_t if_side;

    for (i = 0; i < 6; i++) {
        if_side = FUGAZI_IF_SIDE_SYS;
        rc = fugazi_bcm82757_link_status(fugazi_struct, lane, if_side, sys_link);
        if (rc < 0) {
            cterr('f', 0, "bcm82757 get sys side link status failed");
            return (FAILED);
        }
        if (!(*sys_link)) {
            fugazi_mdelay(10);
            continue;
        }
        break;
    }
    for (i = 0; i < 6; i++) {
        if_side = FUGAZI_IF_SIDE_LINE;
        rc = fugazi_bcm82757_link_status(fugazi_struct, lane, if_side, line_link);
        if (rc < 0) {
            cterr('f', 0, "bcm82757 get line side link status failed");
            return (FAILED);
        }
        if (!(*line_link)) {
            fugazi_mdelay(10);
            continue;
        }
        break;
    }
    fugazi_mdelay(100);
    return (PASSED);
}

/*
 * Function: fugazi_config_bcm82757_macsec_bypass
 *
 * Description: Setup BCM82757 macsec bypass utility
 *
 * Inputs      : lane - lane number
 *               speed - phy speed.
 * Outputs     : PASSED / FAILED
 */
int fugazi_config_bcm82757_macsec_bypass (fugazi_lane_t lane, int speed)
{
    if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, lane, speed)) {
        cterr_add_component("BCM82757",
                            "MDIO controller within BCM57412");
        cterr_add_debug("Check BCM82757",
                        "Check MDIO controller within BCM57412");
        return (FAILED);
    }

    return (PASSED);
}

/*
 * Function: fugazi_cleanup_bcm82757_macsec
 *
 * Description: Clean BCM82757 macsec 
 *
 * Inputs      : lane - lane number
 * Outputs     : PASSED / FAILED
 */
void fugazi_cleanup_bcm82757_macsec (fugazi_lane_t lane)
{
    fugazi_bcm82757_config_macsec_cleanup(fugazi_struct, lane);
}

/*
 * Function: bcm82752_emphasis_setting
 *
 * Description: Based on compliance mask testing, the following changes are 
 *              required changes from the default values.
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 */
int bcm82752_emphasis_setting (void)
{
    int rc;
    uint32_t data, regaddr, devaddr = FUGAZI_MIURA_DEV_PMA_PMD;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side = FUGAZI_IF_SIDE_LINE;

    for (lane = 0; lane < BCM82757_LANE_MAX; lane++) {
        regaddr = BCM82757_TX_CTRL5_REG;
        data = 0x2000;
        rc = fugazi_bcm82757_write(fugazi_struct, lane, if_side, devaddr, 
                                   regaddr, data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d write reg(%#x) : %#x error", lane, 
                   regaddr, data);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            printf("%d.%#.4x --> %#.8x\n", lane, regaddr, data);
        }
        regaddr = BCM82757_TX_FIR_CTRL1_REG;
        data = 0x00E0;
        rc = fugazi_bcm82757_write(fugazi_struct, lane, if_side, devaddr, 
                                   regaddr, data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d write reg(%#x) : %#x error", lane, 
                   regaddr, data);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            printf("%d.%#.4x --> %#.8x\n", lane, regaddr, data);
        }

        regaddr = BCM82757_TX_FIR_CTRL2_REG;
        data = 0x8028;
        rc = fugazi_bcm82757_write(fugazi_struct, lane, if_side, devaddr, 
                                   regaddr, data);
        if (rc < 0) {
            cterr('f', 0, "BCM82757 port %d write reg(%#x) : %#x error", lane, 
                   regaddr, data);
            return (FAILED);
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            rc = fugazi_bcm82757_read(fugazi_struct, lane, if_side, devaddr, 
                                      regaddr, &data);
            printf("%d.%#.4x --> %#.8x\n", lane, regaddr, data);
        }
    }
    return (rc);
}


/******************************************************************************
 * Function: bcm82757_show_fw_version
 *
 * Description: display bcm82747 firmware version utility
 *
 * Inputs      : none
 * Outputs     : PASSED / FAILED
 *****************************************************************************/

int bcm82757_show_fw_version (void)
{
    int    rc = PASSED;
    unsigned int fw_version;
    unsigned int fw_crc;

    rc = fugazi_bcm82757_show_fw_version(fugazi_struct, &fw_version, &fw_crc);
    if (rc < 0) {
        cterr('f', 0, "BCM82757 get firmware info failed");
        return (FAILED);
    }

	printf("\n\rBCM82757 firmware version: 0x%x \n", fw_version);
	return (rc);
}

/*
 * Function: bcm82757_show_fw_version_f
 *
 * Description: utility entry point to show bcm82747 firmware version.
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_show_fw_version_f (void)
{
    /* calling for menu driven */
	return (bcm82757_show_fw_version());
}

/*
 * Function: bcm82757_reset
 *
 * Description: FPGA reset BCM82757 PHY 
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_reset (int print_msg)
{
    /* Reset BCM82757 10G PHY by FPGA bit_13 at FPGA version v0.9.1 */
    if (print_msg) {
        printf("Reset all 10G PHYs...\n");
    }
    reset_platform_ext_dev(FPGA_EXT_GE_QUAD_RST);
    msleep(BCM82757_PHY_RESET_TIME);
    if (print_msg) {
        printf("Unreset all 10G PHYs...\n");
    }
    unreset_platform_ext_dev(FPGA_EXT_GE_QUAD_RST);
    msleep(BCM82757_PHY_RESET_TIME);
    bcm82757_fw_downloaded = ENABLE;
    return (PASSED);
}

/******************************************************************************
 * Function: bcm82757_recover_clock
 *
 * Description: Enable/disable PHY's recovered clock output CLK_RCVCLK0/1 to
 *              idt8a335004 for "recover clock" test item.
 *              Refer from broadcom's Knowledge Base document, KB0027720.
 *
 * Input: phy_port - 0 ~ 3 (1st .. last bcm82780 PHY port on Fugazi board)
 *        enable - 1/0: enable/disable output CLK_RCVCLK0/1
 *
 * Return: PASSED/FAILED
 *****************************************************************************/

int bcm82757_recover_clock (int port, int enable )
{
	return ( fugazi_bcm82780_recover_clock( fugazi_struct, port, enable ) );
}

/******************************************************************************
 * Function: bcm82757_PHY_init
 *
 * Description: Initialize bcm827575 10G PHY with macsec bypass
 *
 * Input       : phy_port - 0 ~ 3 (1st .. last bcm82780 PHY port on Fugazi board)
 *               speed - network speed (1000 - 1G; 10000 - 10G).
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int bcm82757_PHY_init (int port, int speed)
{
    int    rc = PASSED;
    int    eth_port;


    if (port >= MAX_NR_FUGAZI_LANE) {
        cterr('f', 0, "PHY port %d is incorrect, range is 0 ~ %d",
        		port, (MAX_NR_FUGAZI_LANE-1));
        return (FAILED);
    }

    eth_port = fugazi_bcm82757_port_list[port];

    /* Initial PHY */

    printf("Intialize BCM82757 10GE PHY port %d ...\n", port);

  	if (fugazi_eth_port_set_speed(eth_port, speed)) {
   		cterr('f', 0, "BCM82757 set speed %d on PHY eth%d failed", speed, 
               eth_port);
   		return (FAILED);
   	}

   	if (fugazi_bcm82757_config_macsec_bypass(fugazi_struct, port, speed)) {
   		cterr('f', 0, "set BCM82757 macsec bypass at speed %d  on PHY port %d "
              "failed", speed, port);
   		return (FAILED);
   	}

	/* Clean BCM82757 macsec  */
	fugazi_cleanup_bcm82757_macsec(port);

    msleep(BCM82757_PHY_RESET_TIME);

    return (rc);
}

/*
 * Function: bcm82757_init_f
 *
 * Description: utility entry point to initialize bcm82757
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
long bcm82757_init_f (void)
{
    int rc;
    fugazi_lane_t lane;
    int lane_index, lane_index_begin, lane_index_last;
    int speed, speed_flag;

    lane = gethex_answer("Enter Lane(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    speed_flag = gethex_answer("speed(1G:0, 10G:1)", 1, 0, 1);
    speed = fugazi_port_speed[speed_flag];
    if ( lane == 0xff) {
        lane_index_begin = 0;
        lane_index_last = MAX_NR_FUGAZI_LANE;
    } else {
        lane_index_begin = lane;
        lane_index_last = lane_index_begin + 1;
    }

    for (lane_index = lane_index_begin; lane_index < lane_index_last;
         lane_index++) {
        prt("\n\rPHY port %d initialization ... ", lane_index );
        rc |= bcm82757_PHY_init(lane_index, speed);
        if (rc < 0) {
            prt("failed.");
        } else {
            prt("successful.");
        }
    }

	return (rc);
}


/*
 * Function: bcm82757_analog_f
 *
 * Description: utility entry point to get/set analog transmitter
 * pre, main, post,etc.
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_analog_f (void)
{
    int rc;
    fugazi_lane_t lane;
    unsigned int option;
    int pre, main_a, post, post2, post3, amp, tx_hpf, serdes_speed;
    int lane_index, lane_index_begin, lane_index_last;
    bcm_plp_tx_t tx_analog;

    lane = gethex_answer("Enter Lane(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    option = getdec_answer("0: get; 1: set)", 0, 0, 1);
    if ( lane == 0xff) {
        lane_index_begin = 0;
        lane_index_last = MAX_NR_FUGAZI_LANE;
    }
    else {
        lane_index_begin = lane;
        lane_index_last = lane_index_begin + 1;
    }

    for (lane_index=lane_index_begin; lane_index<lane_index_last; lane_index++) {
        if (option == 0) {
            /* get analog transmitter parameters */
            prt("\n\rPHY port %d get transmitter analog parameters:\n", lane_index );
            rc |= fugazi_bcm82757_tx_analog_get(fugazi_struct, lane_index, &tx_analog);
            if (rc < 0) {
                prt("fugazi_bcm82757_tx_analog_get() failed at port %d.", lane_index);
                return (rc);
            }
            else {
                printf("pre   : 0x%x\n", tx_analog.pre);
                printf("main  : 0x%x\n", tx_analog.main);
                printf("post  : 0x%x\n", tx_analog.post);
                printf("post2 : 0x%x\n", tx_analog.post2);
                printf("post3 : 0x%x\n", tx_analog.post3);
                printf("amp   : 0x%x\n", tx_analog.amp);
                printf("tx_hpf: 0x%x\n", tx_analog.tx_hpf);
                printf("serdes speed: 0x%x\n", tx_analog.serdes_speed);
            }
        } else {
            /* set analog transmitter parameters */
            prt("\n\rPHY port %d set transmitter analog parameters:\n", lane_index );
            rc |= fugazi_bcm82757_tx_analog_get(fugazi_struct, lane_index, &tx_analog);
            if (rc < 0) {
                prt("fugazi_bcm82757_tx_analog_get() failed at port %d.", lane_index);
                return (rc);
            }

            pre = gethex_answer("Enter Pre hex value", tx_analog.pre, 0, 0xffffffff);
            main_a = gethex_answer("Enter Main hex value", tx_analog.main, 0, 0xffffffff);
            post = gethex_answer("Enter Post hex value", tx_analog.post, 0, 0xffffffff);
            post2 = gethex_answer("Enter Post2 hex value", tx_analog.post2, 0, 0xffffffff);
            post3 = gethex_answer("Enter Post3 hex value", tx_analog.post3, 0, 0xffffffff);
            amp = gethex_answer("Enter AMP hex value", tx_analog.amp, 0, 0xffffffff);
            tx_hpf = gethex_answer("Enter tx_hpf hex value", tx_analog.tx_hpf, 0, 0xffffffff);
            serdes_speed = gethex_answer("Enter serdes speed hex value", tx_analog.serdes_speed, 0, 0xffffffff);

            tx_analog.pre = pre;
            tx_analog.main = main_a;
            tx_analog.post = post;
            tx_analog.post2 = post2;
            tx_analog.post3 = post3;
            tx_analog.amp = amp;
            tx_analog.tx_hpf = tx_hpf;
            tx_analog.serdes_speed = serdes_speed;

            rc |= fugazi_bcm82757_tx_analog_set(fugazi_struct, lane_index, &tx_analog);
            if (rc < 0) {
                prt("fugazi_bcm82757_tx_analog_set() failed at port %d.", lane_index);
                return (rc);
            }
        }
        if (option == 1) {
            prt("\n\rPHY transmitter analog parameters successful.\n");
        }
    }

    return (rc);
}



/******************************************************************************
 * Function: bcm82757_loopback_set
 *
 * Description: Setup bcm827575 loopback mode utility
 *
 * Inputs      : lane - lane no.
 *               if_side - LINE / SYS side
 *               lb_mode - loopback mode (Remote/Digital )
 *               enable - enable / disable
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int bcm82757_loopback_set (int lane, int if_side, unsigned int lb_mode, unsigned int enable)
{
    return (fugazi_bcm82757_loopback_set(fugazi_struct, lane, if_side, lb_mode, enable));
}

/*
 * Function: bcm82757_force_line_side_lrm
 *
 * Description: Config BCM82757 with LRM mode utility
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_force_line_side_lrm (void)
{
    int force;
    force = gethex_answer("force toggle lrm (disable:0, enable:1)", 0, 0, 1);
    force_line_side_intf_lrm(force);
    return (PASSED);
}

/*
 * Function: bcm82757_power_get_f
 *
 * Description: Get the power status of a specified lane from specified
 *              interface side utility.
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_power_get_f (void)
{
    int rc = PASSED;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int power_rx = 0;
    unsigned int power_tx = 0;
    int lane_start, lane_end;

    lane = gethex_answer("Enter Port(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (lane == 0xff) {
        lane_start = 0;
        lane_end = MAX_NR_FUGAZI_LANE;
    } else {
        lane_start = lane;
        lane_end = lane + 1;
    }

    prt("\nPHY port  Rx-Power  Tx-Power\n");

    for (lane=lane_start; lane<lane_end; lane++) {
        rc |= bcm82757_power_get(fugazi_struct, lane, if_side, &power_rx, &power_tx);
        if (rc < 0) {
            prt("bcm82757_power_get() failed at port %d, %s side.", lane,
                 (if_side)? "System" : "Line" );
            return (rc);
        }
        prt(" %d        %s(%d)      %s(%d) \n", lane,
            (power_rx)? "On" : "Off", power_rx,
            (power_tx)? "On" : "Off", power_tx);
    }

    return (rc);
}


/*
 * Function: bcm82757_power_set_f
 *
 * Description: Set the power of a transmitter or receiver of specified lane
 *              utility.
 *
 * Inputs      : NONE
 * Outputs     : PASSED / FAILED
 */
static long bcm82757_power_set_f (void)
{
    int rc = PASSED;
    fugazi_lane_t lane;
    fugazi_if_side_t if_side;
    unsigned int power_rx = 3;
    unsigned int power_tx = 3;
    int lane_start, lane_end;

    lane = gethex_answer("Enter Port(0 ~ 3; 0xff-all ports)", 0xff, 0, 0xff);
    if_side = gethex_answer("Enter Interface(Line:0, Sys:1)", 0, 0, 1);
    if (lane == 0xff) {
        lane_start = 0;
        lane_end = MAX_NR_FUGAZI_LANE;
    } else {
        lane_start = lane;
        lane_end = lane + 1;
    }
    power_rx = getdec_answer("Enter Rx power(0:Off, 1: On, 2:OffOn, 3: NoChange)", 3, 0, 3);
    power_tx = getdec_answer("Enter Tx power(0:Off, 1: On, 2:OffOn, 3: NoChange)", 3, 0, 3);


    for (lane=lane_start; lane<lane_end; lane++) {
        rc |= bcm82757_power_set(fugazi_struct, lane, if_side, power_rx, power_tx);
        if (rc < 0) {
            prt("bcm82757_power_set() failed at port %d, %s side.", lane,
                 (if_side)? "System" : "Line" );
            return (rc);
        }
    }

    return (rc);
}

/******************************************************************************
 * Function: fugazi_bcm82757_check_link_stable
 *
 * Description: this function is used to check if BCM82757 PHY link is up at both
 *              Line and System side consistency for 2.6 second with interval 10ms
 *              to declare link up.
 *
 * Inputs      : lane - lane number
 *               which_side - 0: line, 1: system, 2: both
 *               sys_link - point to storage to store link with sys side.
 *               line_link - point to storage to store link with line side.
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int fugazi_bcm82757_check_link_stable (int lane, int which_side,
                                       unsigned int *sys_link,
                                       unsigned int *line_link)
{
    int retry=0, link_up_count=0;

    if ((lane < FUGAZI_LANE_0) || (lane >= MAX_NR_FUGAZI_LANE)) {
        printf("\n%s(): Incorrect PHY port %d to check link status!!!\n\r",
                __FUNCTION__, lane);
        return (FAILED);
    }

    if (which_side > MAX_NR_FUGAZI_IF_SIDE) {
        printf("\n%s(): Unknow which_side (%d) to check link status on port %d!!!\n\r",
                __FUNCTION__, which_side, lane);
        return (FAILED);
    }

    while (retry++ < LINK_MAX_CHECK) {
        printf(".");
        fflush(stdout);
        if (fugazi_bcm82757_check_link((fugazi_lane_t)lane, sys_link, line_link)) {
            return (FAILED);
        }

        switch (which_side) {
        case FUGAZI_IF_SIDE_LINE:
            if (!(*line_link)) {
                link_up_count = 0;
            } else {
                link_up_count++;
            }
            break;
        case FUGAZI_IF_SIDE_SYS:
            if (!(*sys_link)) {
                link_up_count = 0;
            } else {
                link_up_count++;
            }
            break;
        case MAX_NR_FUGAZI_IF_SIDE:
            if (!(*sys_link) || !(*line_link)) {
                link_up_count = 0;
            } else {
                link_up_count++;
            }
            break;
        }

        /* All up, return now */
        if (link_up_count >= MAX_LINKUP_CONSISTENCY) {
            break;
        } else { /* Delay and try again */
            fugazi_mdelay(10);
        }
    }
    printf("\n");

    /* Link up : only if it is consistent up for 1 second */
    if (link_up_count < MAX_LINKUP_CONSISTENCY) {
        printf("eth%d sys_link %s, line_link %s (link_up_count=%d, retry=%d) \n",
                lane, (*sys_link)? "up":"down", (*line_link)? "up":"down",
                link_up_count, retry);
    }

    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_bcm82757_test.c,v $
 * Revision 1.2  2021/06/02 08:22:34  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.7  2021/05/04 18:39:47  pdoong
 * Change config bcm82757 cl37 mode from directly register write to all bcm82757 API
 *
 * Revision 1.1.8.6  2021/04/29 01:43:11  pdoong
 * Add checking if PHY Network side link is up in 'SyncE Recovered Clock Test'
 *
 * Revision 1.1.8.5  2021/03/23 01:37:29  pdoong
 * Enhace BCM82757 10G PHY eye scan when checking link up with particular link side which performed eye scan.
 *
 * Revision 1.1.8.4  2021/03/19 18:36:52  pdoong
 * Enhace BCM82757 10G PHY test link down error msg to display down on which side of link.
 *
 * Revision 1.1.8.3  2020/10/07 08:07:42  iachang
 * CSCvo59196-15 Fugazi: Fixed BCM82757 ext loopback test failed get hand up on Apollo
 *
 * Revision 1.1.8.2  2020/08/26 02:37:48  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.55  2020/08/25 04:19:20  pdoong
 * Add bcm82757_power_get_f()/bcm82757_power_set_f() uility.
 *
 * Revision 1.1.6.54  2020/08/19 09:11:50  iachang
 * PRRQ CSCvo59196-4 : BCM82757 10G PHY code review
 *
 * Revision 1.1.6.53  2020/08/05 08:33:23  iachang
 * Code clean up.
 *
 * Revision 1.1.6.52  2020/08/03 09:25:40  iachang
 * Code clean up.
 *
 * Revision 1.1.6.51  2020/05/12 02:38:39  iachang
 * Fixed execute System Information Segmentation fault issue.
 *
 * Revision 1.1.6.50  2020/04/16 01:54:04  iachang
 * Add new utility to access BCM82757 register via MDIO
 *
 * Revision 1.1.6.49  2020/04/06 03:21:21  iachang
 * Add BCM82757 Signal port External loopback test utility.
 *
 * Revision 1.1.6.48  2020/03/25 01:07:49  iachang
 * BCM82757 register read/write utility and register test, using mdio directly access instead of call 10G PHY Broadcom's API.
 *
 * Revision 1.1.6.47  2020/03/19 06:31:40  iachang
 * Support Fugazi Side Band test
 *
 * Revision 1.1.6.46  2020/03/18 06:51:44  iachang
 * Create independent file for LASI test
 *
 * Revision 1.1.6.45  2020/03/13 07:08:42  iachang
 * Modify the LASI test interrupt / loopback disable if_side.
 *
 * Revision 1.1.6.44  2020/03/06 05:54:42  iachang
 * Implement BCM82757 Side Band Test
 *
 * Revision 1.1.6.43  2020/03/04 08:29:18  iachang
 * Correct BCM82757 Side_band register dump
 *
 * Revision 1.1.6.42  2020/03/03 02:26:26  iachang
 * Add BCM57412 sideband utility in BCM82757 for bring up
 *
 * Revision 1.1.6.41  2020/02/27 16:38:21  iachang
 * Add BCM82757 Side band register dump utilit
 *
 * Revision 1.1.6.40  2020/02/25 08:02:49  iachang
 * Modify BCM82757 LASI test utility.
 *
 * Revision 1.1.6.39  2020/02/13 09:06:49  iachang
 * Modify BCM82757 loopback test return method.
 *
 * Revision 1.1.6.38  2020/02/13 06:45:59  iachang
 * Even BCM82757 FW download failure, allow into menu to debug.
 *
 * Revision 1.1.6.37  2020/02/11 09:09:31  iachang
 * BCM82757 10G PHY test : HW would like test completed on all ports, not stop on failure port.
 *
 * Revision 1.1.6.36  2020/02/05 01:44:29  pdoong
 * change user input loopback mode in 'BCM82757 config loopback utility' to more meanful description
 *
 * Revision 1.1.6.35  2020/01/16 08:56:49  iachang
 * Add BCM82757 LASI test utility and enable fw download flag into PHY reset.
 *
 * Revision 1.1.6.34  2020/01/15 07:30:08  iachang
 * Skip BCM82757 fw download with Diag initial. It can save Diag menu boot up time, and help debug.
 *
 * Revision 1.1.6.33  2020/01/15 01:03:02  pdoong
 * In BCM82757 Interrupt utility, port input option, add option to config all the ports.
 *
 * Revision 1.1.6.32  2020/01/07 00:04:32  pdoong
 * Add Checking syncE system DPLL lock status at begin of PHY initialization.
 *
 * Revision 1.1.6.31  2019/12/24 07:15:09  iachang
 * Add "BCM82757 cl37 External loopback Test" into default test.
 *
 * Revision 1.1.6.30  2019/11/22 01:25:08  pdoong
 * Add Checking syncE status of configuration loaded from EEPROM.
 *
 * Revision 1.1.6.29  2019/11/14 09:29:50  iachang
 * Add SFP present check before GE External Loopback test.
 *
 * Revision 1.1.6.28  2019/11/14 08:29:11  iachang
 * Implement SFP present function.
 *
 * Revision 1.1.6.27  2019/10/04 06:05:21  iachang
 * BCM82757 force line side SFP LRM utility
 *
 * Revision 1.1.6.26  2019/10/02 01:19:36  iachang
 * Correct BCM82757 Utility Menu name.
 *
 * Revision 1.1.6.25  2019/09/16 11:23:42  iachang
 * CSCvr24877 : Display PRBS error count when inject error from external
 *
 * Revision 1.1.6.24  2019/08/30 22:00:04  pdoong
 * Add Clear error counter option to clear PRBS error counter.
 *
 * Revision 1.1.6.23  2019/08/29 20:48:50  pdoong
 * Add BCM82757 Analog utility
 *
 * Revision 1.1.6.22  2019/08/16 04:38:43  iachang
 * Display BCM82757 PRBS lock infor and error count
 * BCM82757 check link status need link up consistency 10 times.
 * Fixed BCM82757 only port 0 test all PRBS mode issue.
 *
 * Revision 1.1.6.21  2019/08/09 02:34:55  iachang
 * Move out BCM82757 cl37 External loopback Test in the default test.
 *
 * Revision 1.1.6.20  2019/08/02 03:32:38  iachang
 * Add BCM82757 Regs dump utility
 * Add packet count check when BCM82757 loopback test failed.
 *
 * Revision 1.1.6.19  2019/07/29 08:02:22  iachang
 * Update CSCvq67625 information into comment.
 *
 * Revision 1.1.6.18  2019/07/19 02:29:34  iachang
 * Sync loopback funtion with Curie-2RU
 * Changed Loopback funciton from Curie-2RU to ISR common function tx_rx_diag()
 * Changed BCM82757 print message "lane" to "port"
 *
 * Revision 1.1.6.17  2019/07/18 22:35:20  pdoong
 * Add BCM82757 PHY init utility
 *
 * Revision 1.1.6.16  2019/06/21 06:58:46  iachang
 * Support BCM82757 Eye scan utility.
 * Add BCM82757 interrupt utility.
 *
 * Revision 1.1.6.15  2019/06/17 06:02:18  iachang
 * Rename eth_traf.c/eth_traf.h to diag_eth_traf.c/diag_eth_traf.h
 * Add ethernet packet check message into cterr.
 *
 * Revision 1.1.6.14  2019/06/14 23:58:01  pdoong
 * Add configure bcm82757 10G PHY to generate recovered clock output
 *
 * Revision 1.1.6.13  2019/06/13 14:21:20  iachang
 * Add BCM82757 interrupt utility
 *
 * Revision 1.1.6.12  2019/06/13 08:26:57  iachang
 * Add print_msg flag with BCM82757 reset function.
 *
 * Revision 1.1.6.11  2019/06/12 11:34:08  iachang
 * Add BCM82757 reset when platform initial.
 * Add BCM82757 reset utility.
 *
 * Revision 1.1.6.10  2019/06/12 08:00:35  iachang
 * Merge BCM82757 signal port loopback test item.
 * Add more information with error message.
 *
 * Revision 1.1.6.9  2019/05/31 05:52:59  iachang
 * Modify the Ext. loopback OFF print message.
 *
 * Revision 1.1.6.8  2019/05/14 02:01:27  pdoong
 * Added to sysyem info to display SyncE/bam82757 firmware version
 *
 * Revision 1.1.6.7  2019/04/25 01:42:53  pdoong
 * added an option to display all the 10G phy port link status
 *
 * Revision 1.1.6.6  2019/04/15 21:09:42  iachang
 * Add Compliance mask test setting.
 *
 * Revision 1.1.6.5  2019/04/01 22:34:07  iachang
 * Support 2nd BCM82757 utility.
 *
 * Revision 1.1.6.4  2019/03/18 23:16:14  iachang
 * Bing up 2'nd BCM82757 PHY FW download and external loopback.
 *
 * Revision 1.1.6.3  2019/03/14 21:46:46  iachang
 * Bring up BCM82757 first PHY.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:35  letsai
 * Initial check in.
 *
 * Revision 1.1.2.2  2019/03/10 09:58:32  iachang
 * Clean up code.
 *
 * Revision 1.1.2.1  2019/03/07 07:16:48  iachang
 * Separate BCM82757 test and utility items.
 *
 * Revision 1.1.2.3  2019/02/25 09:15:55  iachang
 * Remove non-used items.
 *
 * Revision 1.1.2.2  2019/02/20 07:09:28  iachang
 * BCM82757 Internal,External loopback test
 *
 * Revision 1.1.2.1  2019/02/18 07:16:50  letsai
 * Support BCM phy tests
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */

