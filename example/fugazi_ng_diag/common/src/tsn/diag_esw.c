/* $Id: diag_esw.c,v 1.12 2019/03/07 09:51:32 lucywang Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_esw.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_esw.c
 * Description: TSN Ethernet Switch Diag tests and utilities.
 *
 * Copyright (c) 2016~2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "proto.h"
#include "queryflags.h"
#include "menu.h"
#include "ethernet.h"
#include "common_utils.h"
#include "plat_defs.h"
#include "nvmonvars.h"
#include "platform_smi.h"
#include "platform_fpga.h"
#include "platform_ge_phy.h"
#include "smi_api.h"
#include "dev_mrvl_ge.h"
#include "tsn_comm.h"
#include "platform_ext_lpbk.h"
#include "platform_esw.h"
#include "platform_cpu.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_sensor.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int        tsn_esw_utils(int);
int        tsn_esw_reg_rd_util(int);
int        tsn_esw_reg_wr_util(int);
int        tsn_esw_phy_reg_rd_util(int);
int        tsn_esw_phy_reg_wr_util(int);
int        tsn_esw_reg_rd(int, int, ushort *);
int        tsn_esw_reg_wr(int, int, ushort);
int        tsn_esw_phy_reg_rd(int, int, int, ushort *);
int        tsn_esw_phy_reg_wr(int, int, int, ushort);
int        tsn_esw_reg_test(int);
int        tsn_esw_smi_c45_rd(int, int, int, ushort *);
int        tsn_esw_smi_c45_wr(int, int, int, ushort);
int        tsn_esw_smi_c45_rd_util(int);
int        tsn_esw_smi_c45_wr_util(int);
int        tsn_esw_phy_xmdio_rd(int, int, int, ushort *);
int        tsn_esw_phy_xmdio_wr(int, int, int, ushort);
int        tsn_esw_phy_xmdio_rd_util(int);
int        tsn_esw_phy_xmdio_wr_util(int);
int        tsn_set_esw_port_cap_util(int);
int        tsn_reset_esw_to_default(int);
int        tsn_esw_show_phy_status(int);
static int esw_set_allports_forward_util(void);
static int esw_set_allports_forward(void);
static int esw_ext_lpbk_test(int);
static int esw_send_packet_util(int);
static int esw_set_1k_testmode(int, ushort);
static int esw_set_1k_testmode_util(int);
static int esw_port_vod_adjust_util(int);
static int esw_port_vod_adjust(int, int, int);
static int esw_force_led_onoff_util(int);
static int esw_config_port_cap(int, int, boolean);
static int tsn_cpu_esw_mac_lpbk_test(int);
static int esw_phy_mac_lpbk_test(int, int);
static int esw_phy_mac_lpbk_test_util(int);
static int esw_force_speed_fn(int, int);
static int esw_force_phy_speed(int, int);
static int esw_force_mac_speed(int, int);
static int esw_enable_autonego_fn(int);
static int esw_enable_mac_autonego(int);
static int esw_enable_phy_autonego(int);
static int esw_conf_ext_stub(int, boolean);
static int tsn_config_cpu_port_link_up(int);
int tsn_esw_port_onoff(int, boolean);
static int tsn_esw_wrap_workaround(int);
static int diag_esw_intr_test(int);
static int mrvl_88e6176_intr_test(void);
static int mrvl_88e6176_assert_intr(void);
static int mrvl_88e6176_deassert_intr(void);
static int mrvl_88e6176_chk_intr_assert(void);
static int mrvl_88e6176_chk_intr_deassert(void);
int has_mrvl_88e6176(void);
/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */

#define ENHANCE_ERROR_MSG_RDY 1
#define SMIOP_TIMEOUT_CTR     200
#define TSN_SMI_WAIT_CUNTR    200
#define TSN_ESW_MAX_GEPORT    8
#define TSN_ESW_MAX_PORT      0xA
#define ESW_ACCESS_WAITTIME   20
#define ESW_RETRY_MAX         100
#define ESW_PHY_RST_TIMEOUT   1000
#define SEC_TO_MICROSEC            1000000
#define ESW_MAX_POLLINGTIME_USEC   5000000   /* 5sec */

/* ESW supported speed table */
static int     esw_speed_tbl[] = {SPD_1000MBPS};

static boolean tsn_board_sku = TSN_H_MB;

static submenu_xtable_t esw_submenu_tbl[] = {
    {"ESW Utilities",
     (type_t(*)())tsn_esw_utils,                                 FALSE,
     0,
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"Register Test",
     (type_t(*)())tsn_esw_reg_test,                              0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"CPU to switch PHY MAC Loopback Test",
     (type_t(*)())tsn_cpu_esw_mac_lpbk_test,                     ETH1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"External Loopback Test",
     (type_t(*)())esw_ext_lpbk_test,                             ETH1,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,                                             0,
     (type_t(*)())0,                                             0},
    {"ESW Interrupt Test",
     (type_t(*)())diag_esw_intr_test,                            0,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())has_mrvl_88e6176,                              0,
     (type_t(*)())0,                                             0},

};

#define ESW_SUBMENU_TBL_SZ (sizeof(esw_submenu_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t esw_pri_items[ESW_SUBMENU_TBL_SZ + MAX_BASE_ITEMS];
static mitem_t esw_sec_items[ESW_SUBMENU_TBL_SZ + MAX_BASE_ITEMS];

menuinfo_t esw_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    esw_pri_items,
};
menuinfo_t *esw_subtest_menup = &esw_subtest_menu;

/* List of GE phy Utilities */
static submenu_xtable_t esw_util_items[] = {
    {"spd1000 ext. lpbk util",       (type_t(*)())esw_send_packet_util, SPD_1000MBPS, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"spd100 ext. lpbk util",       (type_t(*)())esw_send_packet_util, SPD_100MBPS, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"spd10 ext. lpbk util",       (type_t(*)())esw_send_packet_util, SPD_10MBPS, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Read",  (type_t(*)())fpga_reg_rd_util,    0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"FPGA register Write", (type_t(*)())fpga_reg_wr_util,    0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ESW set all ports forwarding",      (type_t(*)())esw_set_allports_forward_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ESW Register Read",      (type_t(*)())tsn_esw_reg_rd_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ESW Register Write",      (type_t(*)())tsn_esw_reg_wr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ESW PHY Register Read",       (type_t(*)())tsn_esw_phy_reg_rd_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"ESW PHY Register Write",      (type_t(*)())tsn_esw_phy_reg_wr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"SMI C45 read",            (type_t(*)())tsn_esw_smi_c45_rd_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"SMI C45 write",           (type_t(*)())tsn_esw_smi_c45_wr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPU register Read",   (type_t(*)())tsn_cpureg_rd_util,  0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"CPU register Write",  (type_t(*)())tsn_cpureg_wr_util,  0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"Set PHY 1000Base-T Test mode",  (type_t(*)())esw_set_1k_testmode_util,  0,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
    {"Adjust port VOD",               (type_t(*)())esw_port_vod_adjust_util,  0,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
    {"Turn port LED ON/OFF",          (type_t(*)())esw_force_led_onoff_util,  0,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
    {"Config LAN port capability",    (type_t(*)())tsn_set_esw_port_cap_util, 0,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
    {"Reset and re-init ESW",         (type_t(*)())tsn_reset_esw_to_default,  FALSE,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
    {"CPU to ESW PHY MAC lpbk test",  (type_t(*)())esw_phy_mac_lpbk_test_util,  FALSE,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
    {"PHY XMDIO register read",            (type_t(*)())tsn_esw_phy_xmdio_rd_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},
    {"PHY XMDIO register write",           (type_t(*)())tsn_esw_phy_xmdio_wr_util, 0, 0,
     (type_t(*)())0, 0,     (type_t(*)())0, 0},

};

#define ESW_UTIL_SIZE (sizeof(esw_util_items) / sizeof(submenu_xtable_t))

/*
 * ESW utility items (filled in from xtable)
 */
static mitem_t esw_util_pri_items[ESW_UTIL_SIZE + MAX_BASE_ITEMS];
static mitem_t esw_util_sec_items[ESW_UTIL_SIZE + MAX_BASE_ITEMS];

/*
 * GE PHY Utility Submenu
 */
menuinfo_t esw_util_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    esw_util_pri_items,
};

menuinfo_t *esw_util_menup = &esw_util_menu;


/*******************************************************************************
 *
 * Function   : esw_set_allports_forward_util
 * Description: Utility to set TSN switch all ports forwarding.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_set_allports_forward_util (void)
{
    /* Set ESW all ports forwarding */
    if (esw_set_allports_forward() != PASSED) {
        printf("%s: Failed to set ESW all ports forwarding.", __FUNCTION__);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_set_allports_forward
 * Description: Function to set TSN switch all ports forwarding.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_set_allports_forward (void)
{
    int    ctr = 0;
    int    reg_addr = (int)ESW_PORTCTR_REG;
    ushort reg_val = 0; 
    int    start_port = 0, end_port = 0;

    /* Set parameters based on board SKU type: TSN-H / TSN-M */
    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H ESW has 8 GE ports: port1 ~ 8. */
        start_port = ESW_PORT0;
        end_port = ESW_PORT10;
    } else {
        /* TSN-M ESW has 4 GE ports: port0 ~ 3. */
        start_port = (int)(TSN_M_ESW_PORT_REG_BASE + ESW_PORT0);
        end_port = (int)(TSN_M_ESW_PORT_REG_BASE + ESW_PORT6);
    }

    for (ctr = start_port; ctr <= end_port; ctr++) {
        reg_val = 0;
        if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, reg%d.",
                   __FUNCTION__, __LINE__, ctr, reg_addr);
            return (FAILED);
        }
        reg_val |= (ushort)(ESW_PCR_FORWARD);

        if (tsn_esw_reg_wr(ctr, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to set ESW port%d forwarding.",
                   __FUNCTION__, __LINE__, ctr);
            return (FAILED);
        }

        msleep(20);

        /* confirm port in forwarding mode */
        reg_val = 0;
        if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, reg%d.",
                   __FUNCTION__, __LINE__, ctr, reg_addr);
            return (FAILED);
        }

        if ((reg_val & (ushort)ESW_PCR_FORWARD) != (ushort)ESW_PCR_FORWARD) {
            printf("%s: Failed to set ESW port%d forwarding mode.",
                   __FUNCTION__, ctr);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_init
 * Description: Function to init TSN switch(Marvell 88E6390).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_init (void)
{
    int    ctr = 0;
    ushort reg_val = 0;
    int    port_num = 0;
    int    cpu_port = 0;
    int    dev_num = (int)ESW_SGMII_DEVNUM;
    int    reg_addr = (int)ESW_SGMII_CONTR_REG;
    int    start_port = 0, end_port = 0;
    uint   fpga_reg_offset = (uint)FPGA_EXTER_DEV_RST_REG;
    uint   fpga_reg_val = 0;

    /* Release ESW from reset if needed */
    if (fpga_read_32_reg(fpga_reg_offset, &fpga_reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.0x%04X.\n",
               __FUNCTION__, fpga_reg_offset);
        return (FAILED);
    }

    if ((fpga_reg_val & (uint)EXT_ESW_RESET) == EXT_ESW_RESET) {
        fpga_reg_val &= (uint)(~EXT_ESW_RESET);

        if (fpga_write_32_reg(fpga_reg_offset, fpga_reg_val) != PASSED) {
            printf("%s: Failed to release ESW from reset.\n", __FUNCTION__);
            return (FAILED);
        }
        msleep(50);
    }

    /* Power Up all GE ports */
    /* Set parameters based on board SKU type: TSN-H / TSN-M */
    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H ESW has 8 GE ports: port1 ~ 8. */
        start_port = ESW_PORT1;
        end_port = ESW_PORT8;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P ESW has 2 GE ports: port0 ~ 1. */
        start_port = ESW_PORT0;
        end_port = ESW_PORT1;
    } else {
        /* TSN-M ESW has 4 GE ports: port0 ~ 3. */
        start_port = ESW_PORT0;
        end_port = ESW_PORT3;
    }

    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (tsn_esw_phy_reg_rd(ctr, 0, 0, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, page%d reg%d.",
                   __FUNCTION__, __LINE__, ctr, 0, 0);
            return (FAILED);
        }
        reg_val &= (short)(~ESW_CCR_PWRDWN);

        if (tsn_esw_phy_reg_wr(ctr, 0, 0, reg_val) != PASSED) {
            printf("%s:%d Failed to power up ESW port%d.",
                   __FUNCTION__, __LINE__, ctr);
            return (FAILED);
        }
    } 

    /* Power up ESW to CPU SERDES port:
     * TSN-H: ESW port9
     * TSN-M: ESW port5(but SERDES register addr. 0xF)
     */
    if (this_is_tsn_h_sku() == TRUE) {
        cpu_port = (int)ESW_PORT9;

        reg_val = 0;
        if (tsn_esw_smi_c45_rd(cpu_port, dev_num,
                               reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, dev%d reg 0x%04X.",
                   __FUNCTION__, __LINE__, cpu_port, dev_num, reg_addr);
            return (FAILED);
        }
        reg_val &= (short)(~ESW_SGMII_PWRDWN);

        if (tsn_esw_smi_c45_wr(cpu_port, dev_num,
                               reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to power up ESW port%d.",
                   __FUNCTION__, __LINE__, cpu_port);
            return (FAILED);
        }
    } else {
        cpu_port = (int)(TSN_M_ESW_CPU_PORT_ADDR);

        reg_val = 0;
        if (tsn_esw_phy_reg_rd(cpu_port, (int)REG_PAGE(1),
                               0, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW SERDES register(page%d reg%d).",
                   __FUNCTION__, REG_PAGE(1), 0);
            return (FAILED);
        }
        reg_val &= (short)(~ESW_CCR_PWRDWN);

        if (tsn_esw_phy_reg_wr(cpu_port, (int)REG_PAGE(1),
                               0, reg_val) != PASSED) {
            printf("%s: Failed to power up ESW CPU port.", __FUNCTION__);
            return (FAILED);
        }
    }

    msleep(20);

    /* Set ESW all ports forwarding */
    if (esw_set_allports_forward() != PASSED) {
        printf("%s: Failed to set ESW all ports forwarding.", __FUNCTION__);
        return (FAILED);
    }

    /* Add RGMII TX & RX delay to communicate with WiFi module */
    /* ESW port to WLAN module:
     * TSN-H: ESW port0
     * TSN-M: ESW port6
     */
    if (this_is_tsn_h_sku() == TRUE) {
        port_num = (int)ESW_PORT0;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* ignore WIFI port setup */
        return (PASSED);
    } else {
        port_num = (int)(TSN_M_ESW_PORT_REG_BASE + ESW_PORT6);
    }

    reg_addr = (int)ESW_PHYCTR_REG;

    reg_val = (ushort)(ESW_PCR_RGMII_RX_DELAY |
                       ESW_PCR_RGMII_TX_DELAY |
                       ESW_PCR_FORCE_SPEED |
                       ESW_PCR_FORCE_LINK |
                       ESW_PCR_F_FULLDPX |
                       ESW_PCR_FORCE_DPX |
                       ESW_PCR_1000MBPS);

    if (tsn_esw_reg_wr(port_num, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to let ESW port%d force link down.",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    reg_val |= (ushort)(ESW_PCR_F_LINKUP);
    if (tsn_esw_reg_wr(port_num, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to let ESW port%d link down.",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_reg_test
 * Description: Function to perform TSN switch(Marvell 88E6390) register test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_reg_test (int opt)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "SMI", "Marvell 88E6390 Ethernet Switch");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Check the PCIe to see whether it's ok or not.");
#endif

    int    ctr = 0, ret_val = PASSED;
    ushort orig_val = 0, test_pattern = REG_PAGE(3), read_back = 0;
    int    start_port = 0, end_port = 0;
    char   *tname = "Switch register";

    testname(tname);
    prpass(testpass, "%s, ", tname);

    /* Set parameters based on board SKU type: TSN-H / TSN-M */
    if (tsn_board_sku == TSN_H_MB) {
        /* TSN-H ESW has 8 GE ports: port1 ~ 8. */
        start_port = ESW_PORT1;
        end_port = ESW_PORT8;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P ESW has 2 GE ports: port0 ~ 1. */
        start_port = ESW_PORT0;
        end_port = ESW_PORT1;
    } else {
        /* TSN-M ESW has 4 GE ports: port0 ~ 3. */
        start_port = ESW_PORT0;
        end_port = ESW_PORT3;
    }

    for (ctr = start_port; ctr <= end_port; ctr++) {
        prpass(testpass, "Testing Switch port%d page reg. ", ctr);
        if (tsn_esw_phy_reg_rd(ctr,
                               (int)REG_PAGE(0),
                               (int)ESW_GEPHY_PAGE_ADDR,
                               &orig_val) != PASSED) {
            cterr('f', 0, "Failed to read original value of port%d page reg.",
                          ctr);
            return (FAILED);
        }

        if (tsn_esw_phy_reg_wr(ctr,
                               (int)REG_PAGE(0),
                               (int)ESW_GEPHY_PAGE_ADDR,
                               test_pattern) != PASSED) {
            cterr('f', 0, "Failed to write test pattern to port%d page reg.",
                          ctr);
            return (FAILED);
        }

        if (tsn_esw_phy_reg_rd(ctr,
                               (int)REG_PAGE(0),
                               (int)ESW_GEPHY_PAGE_ADDR,
                               &read_back) != PASSED) {
            cterr('f', 0, "Failed to read value of port%d page reg.", ctr);
            return (FAILED);
        }

        if (read_back != test_pattern) {
            cterr('f', 0, "Data mismatched: test_pattern %#x; read_back %#x.\n"
                          "Failed to do port%d page reg.",
                          test_pattern, read_back, ctr);
            ret_val = FAILED;
        }

        if (tsn_esw_phy_reg_wr(ctr,
                               (int)REG_PAGE(0),
                               (int)ESW_GEPHY_PAGE_ADDR,
                               orig_val) != PASSED) {
            cterr('f', 0, "Failed to restore port%d page register", ctr);
            return (FAILED);
        }

        if (ret_val != PASSED) {
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_reg_wr_util
 * Description: Utility to write TSN ethernet switch(Marvell 88E6390) register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_send_packet_util (int opt)
{
    if (tsn_sgmii_lpbk_test(ETH1, opt) != PASSED) {
        printf("ESW  %dmbps ext. loopback test failed.\n", opt);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_reg_wr_util
 * Description: Utility to write TSN ethernet switch(Marvell 88E6390) register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_reg_wr_util (int opt)
{
    int    dev_addr = 0, reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;

    dev_addr = gethex_answer("Enter SMI device addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI register addr.: ", 0, 0, 0x1F);

    if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }
    wr_data = (ushort)gethex_answer("Enter wanted SMI register value: ",
                                    reg_val, 0, 0xFFFF);

    if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Writed 0x%04X to Switch SMI device %#x, register %#x.\n",
               wr_data, dev_addr, reg_addr);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_reg_rd_util
 * Description: Utility to read TSN ethernet switch(Marvell 88E6390) register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_reg_rd_util (int opt)
{
    int    dev_addr = 0, reg_addr = 0;
    ushort reg_val = 0;

    dev_addr = gethex_answer("Enter SMI device addr.: ", 0, 0, 0x1F);
    reg_addr = gethex_answer("Enter SMI register addr.: ", 0, 0, 0x1F);

    if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("Switch SMI device %#x, register %#x: 0x%04X.\n",
               dev_addr, reg_addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_reg_rd_util
 * Description: Utility to read TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_reg_rd_util (int opt)
{
    int    phy_port = 0, reg_page, reg_addr = 0;
    ushort reg_val = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    reg_page = gethex_answer("Enter page number of PHY register(0 ~ 0xff): ",
                             0, 0, 0xff);
    reg_addr = getdec_answer("Enter PHY register addr.(0 ~ 31): ", 0, 0, 31);

    if (tsn_esw_phy_reg_rd(phy_port, reg_page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    } else {
        printf("ESW port%d PHY: page %d, register %d = 0x%04X.\n",
               phy_port, reg_page, reg_addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_reg_wr_util
 * Description: Utility to write TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_reg_wr_util (int opt)
{
    int    phy_port = 0, reg_page, reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    reg_page = gethex_answer("Enter page number of PHY register(0 ~ 0xff): ",
                             0, 0, 0xff);
    reg_addr = getdec_answer("Enter PHY register addr.(0 ~ 31): ", 0, 0, 31);

    if (tsn_esw_phy_reg_rd(phy_port, reg_page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }
    wr_data = (ushort)gethex_answer("Enter wanted ESW PHY register value: ",
                                    reg_val, 0, 0xFFFF);

    if (tsn_esw_phy_reg_wr(phy_port, reg_page, reg_addr, wr_data) != PASSED) {
        return (FAILED);
    } else {
        printf("Writed 0x%04X to ESW port%d PHY: page %d, register %d.\n",
               wr_data, phy_port, reg_page, reg_addr);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_smi_c45_rd_util
 * Description: Utility to do SMI Clause45 read on TSN switch(Marvell 88E6390).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_smi_c45_rd_util (int opt)
{
    int    phy_port = 0, dev_num, reg_addr = 0;
    ushort rd_buf = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    dev_num = getdec_answer("Enter device number(0 ~ 255): ", 4, 0, 255);
    reg_addr = gethex_answer("Enter register addr.(0 ~ 0xFFFF): ",
                             0x2002, 0, 0xFFFF);

    if (tsn_esw_smi_c45_rd(phy_port, dev_num, reg_addr, &rd_buf) != PASSED) {
        return (FAILED);
    } else {
        printf("SMI C45: port %d, Device%d, register%#x = 0x%04X.\n",
               phy_port, dev_num, reg_addr, rd_buf);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_smi_c45_rd
 * Description: Function to read TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_smi_c45_rd (int phy_port, int dev_num, int reg_addr, ushort *buf)
{
    int    smi_dev = 0, smi_reg = 0;
    ushort reg_val = 0, smi_cmd = 0, wr_data = 0;
    int    ctr = 0;

    /* Set register address to SMI PHY Data Reg(0x19) */
    wr_data = (ushort)(reg_addr & 0xFFFF);
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    /* Set access command to write address to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_WR_ADDR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    for (ctr = 0; ctr < TSN_SMI_WAIT_CUNTR; ctr++) {
        msleep(5);
  
        /* Check if SMI access is done. */
        reg_val = 0;
        smi_dev = (int)ESW_SMIDEV_GLOB2;
        smi_reg = (int)ESW_GLOB2_PC;
        if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }
    
        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        }

        if (ctr == (TSN_SMI_WAIT_CUNTR - 1)) {
            printf("%s:%d Time Out! SMI access still NOT done.\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    /* Set access command to read data back to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_RD_DATA |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    for (ctr = 0; ctr < TSN_SMI_WAIT_CUNTR; ctr++) {
        msleep(5);
  
        /* Check if SMI access is done. */
        reg_val = 0;
        smi_dev = (int)ESW_SMIDEV_GLOB2;
        smi_reg = (int)ESW_GLOB2_PC;
        if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }
    
        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        }

        if (ctr == (TSN_SMI_WAIT_CUNTR - 1)) {
            printf("%s:%d Time Out! SMI access still NOT done.\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    /* Read back the register value from SMI PHY Data Reg(0x19) */
    reg_val = 0;
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    *buf = reg_val;    

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_smi_c45_wr_util
 * Description: Utility to write TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_smi_c45_wr_util (int opt)
{
    int    phy_port = 0, dev_num, reg_addr = 0;
    ushort rd_buf = 0, wr_data = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    dev_num = getdec_answer("Enter device number(0 ~ 255): ", 4, 0, 255);
    reg_addr = gethex_answer("Enter register addr.(0 ~ 0xFFFF): ",
                             0x2002, 0, 0xFFFF);

    if (tsn_esw_smi_c45_rd (phy_port, dev_num, reg_addr, &rd_buf) != PASSED) {
        return (FAILED);
    }
    wr_data = (ushort)gethex_answer("Enter wanted ESW PHY register value: ",
                                    rd_buf, 0, 0xFFFF);

    if (tsn_esw_smi_c45_wr(phy_port, dev_num, reg_addr, wr_data) != PASSED) {
        return (FAILED);
    } else {
        printf("SMI C45: Writed 0x%04X to port%d, Device%d, register%#x.\n",
               wr_data, phy_port, dev_num, reg_addr);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_smi_c45_wr
 * Description: Function to read TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_smi_c45_wr (int phy_port, int dev_num, int reg_addr, ushort wr_in)
{
    int    smi_dev = 0, smi_reg = 0;
    ushort reg_val = 0, smi_cmd = 0, wr_data = 0;
    int    ctr = 0;

    /* Set register address to SMI PHY Data Reg(0x19) */
    wr_data = (ushort)(reg_addr & 0xFFFF);
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    /* Set access command to write address to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_WR_ADDR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    for (ctr = 0; ctr < TSN_SMI_WAIT_CUNTR; ctr++) {
        msleep(5);
  
        /* Check if SMI access is done. */
        reg_val = 0;
        smi_dev = (int)ESW_SMIDEV_GLOB2;
        smi_reg = (int)ESW_GLOB2_PC;
        if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }
    
        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        }

        if (ctr == (TSN_SMI_WAIT_CUNTR - 1)) {
            printf("%s:%d Time Out! SMI access still NOT done.\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    /* Set write in data to SMI PHY Data Reg(0x19) */
    wr_data = wr_in;
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    /* Set access command to start write data to SMI PHY Command Reg(0x18) */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C45 |
               (ushort)SMIOP_C45_WR_DATA |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(dev_num & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    for (ctr = 0; ctr < TSN_SMI_WAIT_CUNTR; ctr++) {
        msleep(5);
  
        /* Check if SMI access is done. */
        reg_val = 0;
        smi_dev = (int)ESW_SMIDEV_GLOB2;
        smi_reg = (int)ESW_GLOB2_PC;
        if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }
    
        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        }

        if (ctr == (TSN_SMI_WAIT_CUNTR - 1)) {
            printf("%s:%d Time Out! SMI access still NOT done.\n",
                   __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_xmdio_rd_util
 * Description: Utility to do PHY XMDIO read on TSN/Star switch(Marvell 88E6390/88E6176).
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_xmdio_rd_util (int opt)
{
    int    phy_port = 0, dev_num, reg_addr = 0;
    ushort rd_buf = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    dev_num = getdec_answer("Enter device number(0 ~ 255): ", 4, 0, 255);
    reg_addr = gethex_answer("Enter register addr.(0 ~ 0xFFFF): ",
                             0x2002, 0, 0xFFFF);

    if (tsn_esw_phy_xmdio_rd(phy_port, dev_num, reg_addr, &rd_buf) != PASSED) {
        return (FAILED);
    } else {
        printf("PHY XMDIO: port %d, Device%d, register%#x = 0x%04X.\n",
               phy_port, dev_num, reg_addr, rd_buf);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_xmdio_rd
 * Description: Function to read TSN/Star switch(Marvell 88E6390/88E6176)'s PHY XMDIO register.
 * Inputs     : phy_port - port number of PHY
 *              dev_num  - device number 
 *              reg_addr - address of wanted register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_xmdio_rd (int phy_port, int dev_num, int reg_addr, ushort *buf)
{
    ushort reg_val = 0;

	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(13), ((0x0 << 14) | (dev_num & 0x1f))) != PASSED) {
        printf("%s:%d Failed to write port%d PHY reg 13\n",
               __FUNCTION__, __LINE__, phy_port);
        return (FAILED);
    }
	
	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(14), (reg_addr & 0xffff)) != PASSED) {
        printf("%s:%d Failed to write port%d PHY reg 14\n",
               __FUNCTION__, __LINE__, phy_port);
        return (FAILED);
    }
	
	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(13), ((0x1 << 14) | (dev_num & 0x1f))) != PASSED) {
        printf("%s:%d Failed to write port%d PHY reg 13\n",
               __FUNCTION__, __LINE__, phy_port);
        return (FAILED);
    }
	
	if (tsn_esw_phy_reg_rd(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(14), &reg_val) != PASSED) {
        printf("%s: Failed to read port%d PHY reg 14\n",
               __FUNCTION__, phy_port);
		return(FAILED);
    }

	*buf = reg_val;
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_xmdio_wr_util
 * Description: Utility to write TSN/Star switch(Marvell 88E6390/88E6176)'s PHY register.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_xmdio_wr_util (int opt)
{
    int    phy_port = 0, dev_num, reg_addr = 0;
    ushort rd_buf = 0, wr_data = 0;

    phy_port = gethex_answer("Enter port number(0 ~ 0xf): ", 0x1, 0, 0xf);
    dev_num = getdec_answer("Enter device number(0 ~ 255): ", 4, 0, 255);
    reg_addr = gethex_answer("Enter register addr.(0 ~ 0xFFFF): ",
                             0x2002, 0, 0xFFFF);

    if (tsn_esw_phy_xmdio_rd (phy_port, dev_num, reg_addr, &rd_buf) != PASSED) {
        return (FAILED);
    }
    wr_data = (ushort)gethex_answer("Enter wanted PHY XMDIO register value: ",
                                    rd_buf, 0, 0xFFFF);

    if (tsn_esw_phy_xmdio_wr(phy_port, dev_num, reg_addr, wr_data) != PASSED) {
        return (FAILED);
    } else {
        printf("PHY XMDIO: Writed 0x%04X to port%d, Device%d, register%#x.\n",
               wr_data, phy_port, dev_num, reg_addr);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_xmdio_wr
 * Description: Function to read TSN/Star switch(Marvell 88E6390/88E6176)'s PHY XMDIO register.
 * Inputs     : phy_port - port number of PHY
 *              dev_num  - device number
 *              reg_addr - address of wanted register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_xmdio_wr (int phy_port, int dev_num, int reg_addr, ushort wr_in)
{
	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(13), ((0x0 << 14) | (dev_num & 0x1f))) != PASSED) {
        printf("%s:%d Failed to write port%d PHY reg 13\n",
               __FUNCTION__, __LINE__, phy_port);
        return (FAILED);
    }
	
	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(14), (reg_addr & 0xffff)) != PASSED) {
        printf("%s:%d Failed to write port%d PHY reg 14\n",
               __FUNCTION__, __LINE__, phy_port);
        return (FAILED);
    }
	
	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(13), ((0x1 << 14) | (dev_num & 0x1f))) != PASSED) {
        printf("%s:%d Failed to write port%d PHY reg 13\n",
               __FUNCTION__, __LINE__, phy_port);
        return (FAILED);
    }
	
	if (tsn_esw_phy_reg_wr(phy_port, (int)PHY_PAGE(0), (int)PHY_REG(14), wr_in) != PASSED) {
        printf("%s: Failed to read port%d PHY reg 14\n",
               __FUNCTION__, phy_port);
		return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_reg_rd
 * Description: Function to read TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_reg_rd (int phy_port, int reg_page, int reg_addr, ushort *buf)
{
    int    smi_dev = 0, smi_reg = 0, phy_page_reg = (int)PHY_REG(22);
    ushort reg_val = 0, smi_cmd = 0, wr_data = 0;

    /* Check if SMI bus is available */
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    
    if ((reg_val & (ushort)SMI_CMD_SMIBUSY) != 0) {
        printf("%s: SMI bus is busy so can't be accessed now.\n", __FUNCTION__);
        return (FAILED);
    }

    /* If user wants to read value of Page addr. register(reg22),
     * then no need to change page. */
    if (reg_addr == ESW_GEPHY_PAGE_ADDR) {
        smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
                   (ushort)SMI_CMD_SMIMODE_C22 |
                   (ushort)SMI_CMD_SMIOP_RD |
                   (ushort)((phy_port & 0x1f) << 5) |
                   (ushort)(phy_page_reg & 0x1f));

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
                   __FUNCTION__, __LINE__, smi_cmd);
        }

        smi_dev = (int)ESW_SMIDEV_GLOB2;
        smi_reg = (int)ESW_GLOB2_PC;
        if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
            return (FAILED);
        }

        smi_dev = (int)ESW_SMIDEV_GLOB2;
        smi_reg = (int)ESW_GLOB2_PD;
        reg_val = 0;
        if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
                   __FUNCTION__, __LINE__, smi_dev, smi_reg);
            return (FAILED);
        }
        *buf = reg_val;

        return (PASSED);
    }

    /* Change page */
    wr_data = (ushort)(reg_page & 0xFF);
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_WR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(phy_page_reg & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_RD |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(phy_page_reg & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    reg_val = 0;
    if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    
    if (reg_val != (ushort)(reg_page & 0xFF)) {
        printf("%s: Failed to set page number of ESW PHY%d register.\n",
               __FUNCTION__, phy_port);
        printf("%s: reg_val = %#x; and reg_page = %#x.\n",
               __FUNCTION__, reg_val, (ushort)(reg_page & 0xFF));
        return (FAILED);
    }

    /* Read register value */
    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_RD |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(reg_addr & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    reg_val = 0;
    if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
        return (FAILED);
    }
    *buf = reg_val;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_phy_reg_wr
 * Description: Function to write TSN switch(Marvell 88E6390)'s PHY register.
 * Inputs     : phy_port - port number of PHY
 *              reg_page - page number of wanted PHY register
 *              reg_addr - address of wanted PHY register
 *              wr_in    - the write in value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_phy_reg_wr (int phy_port, int reg_page, int reg_addr, ushort wr_in)
{
    int    smi_dev = 0, smi_reg = 0, phy_page_reg = (int)PHY_REG(22);
    ushort reg_val = 0, smi_cmd = 0, wr_data = 0;

    /* Check if SMI bus is available */
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    
    if ((reg_val & (ushort)SMI_CMD_SMIBUSY) != 0) {
        printf("%s: SMI bus is busy so can't be accessed now.\n", __FUNCTION__);
        return (FAILED);
    }

    /* Change page */
    /* If user wants to read value of Page addr. register(reg22),
     * then no need to change page.
     */
    if (reg_addr == ESW_GEPHY_PAGE_ADDR) {
        wr_data = (ushort)(wr_in & 0xFF);
    } else {
        wr_data = (ushort)(reg_page & 0xFF);
    }
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_WR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(phy_page_reg & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_RD |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(phy_page_reg & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        return (FAILED);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    reg_val = 0;
    if (tsn_esw_reg_rd(smi_dev, smi_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }
    
    if (reg_val != wr_data) {
        printf("%s: Failed to set page number of ESW PHY%d register.\n",
               __FUNCTION__, phy_port);
        printf("%s: reg_val = %#x; and write in data = %#x.\n",
               __FUNCTION__, reg_val, wr_data);
        return (FAILED);
    }

    /* If user wants to read value of Page addr. register(reg22),
     * then no need to change page.
     */
    if (reg_addr == ESW_GEPHY_PAGE_ADDR) {
        return (PASSED);
    }

    /* Write register value */
    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PD;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, wr_in) != PASSED) {
        printf("%s:%d Failed to write ESW SMI dev %#x reg. %#x\n",
               __FUNCTION__, __LINE__, smi_dev, smi_reg);
        return (FAILED);
    }

    smi_cmd = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_WR |
               (ushort)((phy_port & 0x1f) << 5) |
               (ushort)(reg_addr & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, smi_cmd);
    }

    smi_dev = (int)ESW_SMIDEV_GLOB2;
    smi_reg = (int)ESW_GLOB2_PC;
    if (tsn_esw_reg_wr(smi_dev, smi_reg, smi_cmd) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_reg_rd
 * Description: Function to read TSN ethernet switch(Marvell 88E6390) register.
 * Inputs     : dev_addr - SMI device addr.
 *              reg_addr - SMI register addr.
 *              *buf     - buffer to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_reg_rd (int dev_addr, int reg_addr, ushort *buf)
{
    int    phy_addr = 0;
    int    smi_cmd_reg = (int)TSN_ESW_SMI_CMD_REG;
    int    smi_data_reg = (int)TSN_ESW_SMI_DATA_REG;
    ushort reg_val = 0, wr_data = 0;
    int    ctr = 0;

    if (this_is_tsn_h_sku() == TRUE) {
           phy_addr = (int)TSN_H_ESW_SMIADDR;
    } else {
           phy_addr = (int)TSN_M_ESW_SMIADDR;
    }


    /* Comfirm SMI bus is ready for access. */
    for (ctr = 0; ctr < TSN_SMI_RETRY_MAX; ctr++) {
    /* Confirm SMI bus is ready for access.
     * Since TSN switch(Marvell 88E6390) is set to multi chip address mode,
     * this is by checking SMIBusy(bit15) of SMI Command Register(0x0).
     */
        if (tsn_smi_read(phy_addr, smi_cmd_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read Switch(%#X) command reg.(%#X)\n",
               __FUNCTION__, __LINE__, phy_addr, smi_cmd_reg);
            return (FAILED);
        }
        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        } else {
            if (ctr == (TSN_SMI_RETRY_MAX - 1)) {
                printf("%s: TIME OUT !! SMI internal bus is still busy.\n", __FUNCTION__);
                return (FAILED);
            }
        }
    }

    wr_data = ((ushort)SMI_CMD_SMIBUSY |
               (ushort)SMI_CMD_SMIMODE_C22 |
               (ushort)SMI_CMD_SMIOP_RD |
               (ushort)((dev_addr & 0x1f) << 5) |
               (ushort)(reg_addr & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] wr_data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, wr_data);
    }

    if (tsn_smi_write(phy_addr, smi_cmd_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to SMI write Switch(%#X) command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, phy_addr, smi_cmd_reg);
        return (FAILED);
    } 

    for (ctr = 0; ctr < SMIOP_TIMEOUT_CTR; ctr++) {
        reg_val = 0;
        if (tsn_smi_read(phy_addr, smi_cmd_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read Switch(%#X) command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, phy_addr, smi_cmd_reg);
            return (FAILED);
        }

        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        } else {
            if (ctr == (SMIOP_TIMEOUT_CTR - 1)) {
                printf("%s:%d Failed ! SMI OP Time out.\n",
                       __FUNCTION__, __LINE__);
                return (FAILED);
            }
        }
        msleep(5);
    }

    reg_val = 0;
    if (tsn_smi_read(phy_addr, smi_data_reg, &reg_val) != PASSED) {
        printf("%s: Failed to read SMI PHY(0x%02X) register 0x%02X.\n",
               __FUNCTION__, phy_addr, smi_data_reg);
        return (FAILED);
    }
    *buf = reg_val;

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_reg_wr
 * Description: Function to write TSN ethernet switch(Marvell 88E6390) register.
 * Inputs     : dev_addr - SMI device addr.
 *              reg_addr - SMI register addr.
 *              wr_data  - buffer to put the value that wanted to write in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_reg_wr (int dev_addr, int reg_addr, ushort wr_data)
{
    int    phy_addr = 0;
    int    smi_cmd_reg = (int)TSN_ESW_SMI_CMD_REG;
    int    smi_data_reg = (int)TSN_ESW_SMI_DATA_REG;
    ushort reg_val = 0, cmd_data = 0;
    int    ctr = 0;

    if (this_is_tsn_h_sku() == TRUE) {
           phy_addr = (int)TSN_H_ESW_SMIADDR;
    } else {
           phy_addr = (int)TSN_M_ESW_SMIADDR;
    }

    /* Confirm SMI bus is ready for access.
     * Since TSN switch(Marvell 88E6390) is set to multi chip address mode,
     * this is by checking SMIBusy(bit15) of SMI Command Register(0x0).
     */
    if (tsn_smi_read(phy_addr, smi_cmd_reg, &reg_val) != PASSED) {
        printf("%s:%d Failed to SMI read Switch(%#X) command reg.(%#X)\n",
               __FUNCTION__, __LINE__, phy_addr, smi_cmd_reg);
        return (FAILED);
    }
    
    if ((reg_val & (ushort)SMI_CMD_SMIBUSY) != 0) {
        printf("%s: SMI bus is busy so can't be accessed now.\n", __FUNCTION__);
        return (FAILED);
    }

    /* First, put the wanted write in data to SMI data register. */
    if (tsn_smi_write(phy_addr, smi_data_reg, wr_data) != PASSED) {
        printf("%s:%d Failed to SMI write Switch(%#X) command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, phy_addr, smi_data_reg);
        return (FAILED);
    } 

    cmd_data = ((ushort)SMI_CMD_SMIBUSY |
                (ushort)SMI_CMD_SMIMODE_C22 |
                (ushort)SMI_CMD_SMIOP_WR |
                (ushort)((dev_addr & 0x1f) << 5) |
                (ushort)(reg_addr & 0x1f));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("DBG[%s:%d] wr_data for SMI command reg. = 0x%04X.\n",
               __FUNCTION__, __LINE__, cmd_data);
    }

    if (tsn_smi_write(phy_addr, smi_cmd_reg, cmd_data) != PASSED) {
        printf("%s:%d Failed to SMI write Switch(%#X) command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, phy_addr, smi_cmd_reg);
        return (FAILED);
    } 

    for (ctr = 0; ctr < SMIOP_TIMEOUT_CTR; ctr++) {
        reg_val = 0;
        if (tsn_smi_read(phy_addr, smi_cmd_reg, &reg_val) != PASSED) {
            printf("%s:%d Failed to SMI read Switch(%#X) command reg.(%#X)\n",
                   __FUNCTION__, __LINE__, phy_addr, smi_cmd_reg);
            return (FAILED);
        }

        if ((reg_val & (ushort)SMI_CMD_SMIBUSY) == 0) {
            break;
        } else {
            if (ctr == (SMIOP_TIMEOUT_CTR - 1)) {
                printf("%s:%d Failed ! SMI OP Time out.\n",
                       __FUNCTION__, __LINE__);
                return (FAILED);
            }
        }
        msleep(5);
    }
    return (PASSED);
}

/*******************************************************************************
 *    
 * Function   : esw_ext_lpbk_test
 * Description: Function to do Ethernet Switch ports external loopback test.
 * Inputs     : phy_num - phy number
 * Outputs    : PASSED/FAILED
 *               
 *******************************************************************************
 */
static int esw_ext_lpbk_test (int phy_num)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    if (this_is_supernova()) {
        cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E6176 Ethernet Switch", "Marvell 88E1112 GE WAN Phy");
    } else {
        cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E6390 Ethernet Switch", "Marvell 88E1112 GE WAN Phy");
    }

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Change another external loopback fixture.",
                    "If step a. is still failed, try the internal "
                    "loopback to check if internal loopback is OK.",
                    "If step b. is OK, we can know PHY has problems. "
                    "If step b is failed, please try step d.",
                    "Observe PCIe register status to "
                    "check if switch configuration is normal.",
                    "If step d. is OK, we can assume the interface "
                    "between Host SoC and switch has problems.");
#endif

    int ctr = 0, speed_ctr = 0;
    int test_speed[] = {SPD_10MBPS, SPD_100MBPS, SPD_1000MBPS};
    int ret_val = PASSED;
    int start_port = 0, end_port = 0;

    char *tname = "Switch Ext. loopback";
    testname(tname);

    /* Check if Ext. Loopback Flag is ON */
    if (check_ext_lpbk_flag() != TRUE) {
        printf("Skip %s test beacuse Ext. Loopback Flag is OFF.\n", tname);
        return (PASSED);
    }

    prpass(testpass, "%s, ", tname);

    system("ifconfig eth1 up");

    /* Set parameters based on board SKU type: TSN-H / TSN-M */
    if (tsn_board_sku == TSN_H_MB) {
        /* TSN-H ESW has 8 GE ports: port1 ~ 8. */
        start_port = (int)ESW_PORT1;
        end_port = (int)ESW_PORT8;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P ESW has 2 GE ports: port0 ~ 1. */
        start_port = (int)(ESW_PORT0);
        end_port = (int)(ESW_PORT1);
    } else {
        /* TSN-M ESW has 4 GE ports: port0 ~ 3. */
        start_port = (int)(ESW_PORT0);
        end_port = (int)(ESW_PORT3);
    }

    /* Need to config CPU GE MAC to be link up at the beginning before setup 
     * the GE Switch. 
     */
    if (tsn_config_cpu_port_link_up(SPD_1000MBPS) != PASSED) {
        cterr('f', 0, "Failed config cpu port link up");
        return (FAILED);
    }

    /* Prepare for ESW external loopback test:
     * 1. Disable all ports.
     * 2. Enable PHY Stub.
     */
    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (tsn_esw_port_onoff(ctr, DISABLE) != PASSED) {
            cterr('f', 0, "Failed to disable ethernet switch port%d", ctr);
            return (FAILED);
        }

    }

    /* Start to test in different speed port by port */
    for (ctr = start_port; ctr <= end_port; ctr++) {
        
        /* CSCvj11429, Based on Marvell FAE need to apply this workaround
         * configuration to avoid link stuck, then enable port at last step. */
        if (tsn_board_sku == TSN_H_MB) {
            if (tsn_esw_wrap_workaround(0)) {
                cterr('f', 0, "Failed to perform link stuck workaround");
                return (FAILED);
            }
        }

        /* Enable Switch port */
        if (tsn_esw_port_onoff(ctr, ENABLE) != PASSED) {
            cterr('f', 0, "Failed to enable ethernet switch port%d", ctr);
            return (FAILED);
        }

        for (speed_ctr = 0; speed_ctr < 3; speed_ctr++) {
            prpass(testpass, "Testing Switch port-%d, speed-%d",
                             ctr, test_speed[speed_ctr]);

            /* Enable PHY Stub */
            if (test_speed[speed_ctr] == SPD_1000MBPS) {
                if (esw_conf_ext_stub(ctr, ENABLE) != PASSED) {
                    cterr('f', 0, "Failed to enable port%d PHY Stub", ctr);
                    return (FAILED);
                }
            }

            /* Configure MAC and PHY speed based on testing request */
            if (esw_force_speed_fn(ctr, test_speed[speed_ctr]) != PASSED) {
                cterr('f', 0, "Failed set port%d at %dmbps",
                              ctr, test_speed[speed_ctr]);
                ret_val = FAILED;
                break;
            }

            if (tsn_sgmii_lpbk_test(ETH1, test_speed[speed_ctr]) != PASSED) {
                tsn_esw_show_phy_status(ctr);
                cterr('f', 0, "Failed at port%d %dmbps",
                              ctr, test_speed[speed_ctr]);
                ret_val = FAILED;
                break;
            }
            msleep(50);

            /* Disable PHY Stub */
            if (esw_conf_ext_stub(ctr, DISABLE) != PASSED) {
                cterr('f', 0, "Failed to disable port%d PHY Stub", ctr);
                return (FAILED);
            }
        }

        /* Re-enable Switch port MAC and PHY Auto-nego after test */
        if (esw_enable_autonego_fn(ctr) != PASSED) {
            cterr('f', 0, "Failed to enable port%d auto-negotiation", ctr);
            return (FAILED);
        }

        /* Disable Switch port */
        if (tsn_esw_port_onoff(ctr, DISABLE) != PASSED) {
            cterr('f', 0, "Failed to disable ethernet switch port%d", ctr);
            return (FAILED);
        }
    }

    /* Enable Switch port */
    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (tsn_esw_port_onoff(ctr, ENABLE) != PASSED) {
            cterr('f', 0, "Failed to enable ethernet switch port%d", ctr);
            return (FAILED);
        }
    }

    if (ret_val != PASSED) {
        return (FAILED);
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: esw_diag_test
 *
 * Description: Entrance of TSN Ethernet Switch Diag menu.
 *
 * Inputs      : port - port number
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************/
int esw_diag_test (int show_menu)
{
    build_primary_submenu(esw_submenu_tbl,
                          ESW_SUBMENU_TBL_SZ,
                          "Ethernet Switch", &esw_subtest_menup);
    build_secondary_submenu(esw_submenu_tbl,
                            ESW_SUBMENU_TBL_SZ,
                            esw_sec_items);

    /* Set parameters based on board SKU type: TSN-H / TSN-M */
    if (this_is_tsn_h_sku() == TRUE) {
        tsn_board_sku = TSN_H_MB;
    } else {
        tsn_board_sku = TSN_M_MB;
    }

    if (show_menu) {
        menu(esw_subtest_menup, esw_sec_items, '\0' );

    } else {
        menu_exec_doall_diags(esw_subtest_menup);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : tsn_ge_phy_utils
 * Description :
 * Inputs      : opt - reserve for future use
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int tsn_esw_utils (int opt)
{
    build_primary_submenu(esw_util_items, ESW_UTIL_SIZE,
                          "Ethernet Switch Utilities", &esw_util_menup);
    build_secondary_submenu(esw_util_items, ESW_UTIL_SIZE,
                            esw_util_sec_items);

    menu(esw_util_menup, esw_util_sec_items, '\0' );

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_set_1k_testmode_util
 * Description: Utility to set TSN LAN Switch PHY 1000BaseT test mode.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_set_1k_testmode_util (int opt)
{
    int    reg_page = 0, reg_addr = 0;
    ushort reg_val = 0, test_mode = 0;
    int    phy_port = 1;
    int    def_port = 0, s_port = 0, e_port = 0;
    int    ctr = 0;

    if (tsn_board_sku == TSN_H_MB) {
        def_port = ESW_PORT1;
        s_port = ESW_PORT1;
        e_port = ESW_PORT9;

        printf("\nTSN-H Switch port mapping - \n");
        printf("-------------------------\n");
        printf("|  2  |  4  |  6  |  8  |\n");
        printf("|-----+-----+-----+-----|\n");
        printf("|  1  |  3  |  5  |  7  |\n");
        printf("-------------------------\n");
        phy_port = (int)gethex_answer("Enter port num(1 ~ 8, 9 for all): ",
                                      def_port, s_port, e_port);
    } else {
        def_port = ESW_PORT0;
        s_port = ESW_PORT0;
        e_port = ESW_PORT4;

        printf("\nTSN-M Switch port mapping - \n");
        printf("-------------------------\n");
        printf("|  0  |  1  |  2  |  3  |\n");
        printf("-------------------------\n");
        phy_port = (int)gethex_answer("Enter port num(0 ~ 3, 4 for all): ",
                                      def_port, s_port, e_port);
    }

    if (phy_port != e_port) {
        /* Got the current mode. */
        reg_page = (int)REG_PAGE(0);
        reg_addr = (int)GEPHY_1000T_CNTL_REG;
        if (tsn_esw_phy_reg_rd(phy_port, reg_page,
                               reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to get test mode(Reg. %d_%d)\n",
                   __FUNCTION__, reg_addr, reg_page);
	    return(FAILED);
        }
        test_mode = (ushort)((reg_val & ONEK_CNTL_TESTMODE_MASK) >>
                             ONEK_CNTL_TESTMODE_SHIFT);

        s_port = phy_port;
        e_port = (s_port + 1);
    }

    printf("\nTest modes -\n");
    printf("    0 - Normal Mode\n");
    printf("    1 - Test Mode 1 - Transmit Waveform Test\n");
    printf("    2 - Test Mode 2 - Transmit Jitter Test (Master mode)\n");
    printf("    3 - Test Mode 3 - Transmit Jitter Test (Slave mode)\n");
    printf("    4 - Test Mode 4 - Transmit Distortion Test\n");
    test_mode = (ushort)gethex_answer("Enter the test mode: ", test_mode, 0, 4);

    for (ctr = s_port; ctr < e_port; ctr++) {
        if (esw_set_1k_testmode(ctr, test_mode) != PASSED) {
            printf("Failed to set LAN port%d PHY 1000Base-T Test mode.\n", ctr);
            return (FAILED);
        }
        printf("Done setting LAN Switch port%d PHY 1000Base-T Test mode.\n", ctr);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_set_1k_testmode
 * Description: Function to set TSN LAN Switch PHY 1000BaseT test mode.
 * Inputs     : phy_port - number of port to set
 *              test_mode - test mode to set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_set_1k_testmode (int phy_port, ushort test_mode)
{
    int    reg_page = 0, reg_addr = 0;
    ushort reg_val = 0;
    int    ctr = 0;

    /* Enable TX_TCLK */
    reg_page = (int)REG_PAGE(6);
    reg_addr = (int)GEPHY_MISC_TEST_REG;
    if (tsn_esw_phy_reg_rd(phy_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get test mode(Reg. %d_%d)\n",
               __FUNCTION__, reg_page, reg_addr);
	return(FAILED);
    }
    reg_val |= (ushort)MTR_TX_TCLK_EN;

    msleep(10);

    if (tsn_esw_phy_reg_wr(phy_port, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to set port%d PHY 1000Base-T Test mode(%d).\n",
	       __FUNCTION__, phy_port, test_mode);
	return(FAILED);
    }

    /* Got the current mode. */
    reg_val = 0;
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)GEPHY_1000T_CNTL_REG;
    if (tsn_esw_phy_reg_rd(phy_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to get test mode(Reg. %d_%d)\n",
               __FUNCTION__, reg_page, reg_addr);
	return(FAILED);
    }
    reg_val &= (ushort)(~ONEK_CNTL_TESTMODE_MASK); /* clear the test mode */
    reg_val |= (ushort)(test_mode << ONEK_CNTL_TESTMODE_SHIFT);

    msleep(10);

    /* Write the new data */
    if (tsn_esw_phy_reg_wr(phy_port, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to set port%d PHY 1000Base-T Test mode(%d).\n",
	       __FUNCTION__, phy_port, test_mode);
	return(FAILED);
    }

    /* Recover the page */
    reg_val = (ushort)PHY_PAGE(0);
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)MRVL88E1112_PAGE_ADDR_REG;
    if (tsn_esw_phy_reg_wr(phy_port, reg_page, reg_addr, reg_val) != PASSED) {
	printf("%s: Failed to recover port%d PHY page addr. to page%d.\n",
	       __FUNCTION__, phy_port, reg_val);
	return(FAILED);
    }

    if (test_mode == OCR_TESTMODE_NORMAL) {
        reg_val = 0;
        reg_page = (int)PHY_PAGE(0);
        reg_addr = (int)MRVL1112_COP_CTRL_REG;
        if (tsn_esw_phy_reg_rd(phy_port, reg_page,
                               reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW Reg. %d_%d\n",
                   __FUNCTION__, reg_addr, reg_page);
	    return(FAILED);
        }

        reg_val |= (ushort)COP_CTRL_RESET;
        if (tsn_esw_phy_reg_wr(phy_port, reg_page,
                               reg_addr, reg_val) != PASSED) {
	    printf("%s: Failed to apply a soft reset.\n", __FUNCTION__);
	    return(FAILED);
        }

        for (ctr = 0; ctr < GEPHY_MAX_RETRY; ctr++) {
            msleep(10);

            reg_val = 0;
            if (tsn_esw_phy_reg_rd(phy_port, reg_page,
                                   reg_addr, &reg_val) != PASSED) {
                printf("%s:%d Failed to read ESW Reg. %d_%d\n",
                       __FUNCTION__, __LINE__, reg_addr, reg_page);
	        return(FAILED);
            }

            if ((reg_val & (ushort)COP_CTRL_RESET) == 0) {
                break;
            }

            if (ctr == (GEPHY_MAX_RETRY - 1)) {
                printf("%s: Time out but PHY still in soft reset process.\n",
                       __FUNCTION__);
	        return(FAILED);
            }
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_port_vod_adjust_util
 * Description: Utility to adjust TSN LAN Switch port VOD.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_port_vod_adjust_util (int opt)
{
    int def_port = 0, s_port = 0, e_port = 0;
    int esw_port = 7, eth_mode = 2, vod_val = 0;
    int ctr = 0;

    if (tsn_board_sku == TSN_H_MB) {
        def_port = ESW_PORT1;
        s_port = ESW_PORT1;
        e_port = (ESW_PORT8 + 1);

        printf("\nTSN-H Switch port mapping - \n");
        printf("-------------------------\n");
        printf("|  2  |  4  |  6  |  8  |\n");
        printf("|-----+-----+-----+-----|\n");
        printf("|  1  |  3  |  5  |  7  |\n");
        printf("-------------------------\n");
        esw_port = (int)gethex_answer("Enter port num(1 ~ 8, 9 for all): ",
                                      def_port, s_port, e_port);
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
		def_port = ESW_PORT0;
        s_port = ESW_PORT0;
        e_port = (ESW_PORT1 + 1);

        if (this_is_star_c1109_2p()) {
            printf("\nStar C1109-2P Switch port mapping - \n");
        } else {
            printf("\nSupernova C959-2P Switch port mapping - \n");
        }
        printf("-------------\n");
        printf("|  0  |  1  |\n");
        printf("-------------\n");
        esw_port = (int)gethex_answer("Enter port num(0 ~ 1, 2 for all): ",
                                      def_port, s_port, e_port);
    } else {
        def_port = ESW_PORT0;
        s_port = ESW_PORT0;
        e_port = (ESW_PORT3 + 1);

        printf("\nTSN-M Switch port mapping - \n");
        printf("-------------------------\n");
        printf("|  0  |  1  |  2  |  3  |\n");
        printf("-------------------------\n");
        esw_port = (int)gethex_answer("Enter port num(0 ~ 3, 4 for all): ",
                                      def_port, s_port, e_port);
    }

    if (esw_port != e_port) {
        s_port = esw_port;
        e_port = (s_port + 1);
    }

    printf("\nEthernet modes -\n");
    printf("    1 - 10 Mbps\n");
    printf("    2 - 100 Mbps\n");
    printf("    3 - 1000 Mbps\n");
    eth_mode = (int)gethex_answer("Enter mode: ", 0x2, 0x1, 0x3);

    printf("\nVOD modes -\n");
    printf(" 0:  0%%,   1:  -2%%,   2:  -4%%,   3:  -6%%\n");
    printf(" 4: -8%%,   5: -10%%,   6: -12%%,   7: -14%%\n");
    printf(" 8:  0%%,   9:   2%%,   a:   4%%,   b:   6%%\n");
    printf(" c:  8%%,   d:  10%%,   e:  12%%,   f:  14%%\n");
    vod_val = (int)gethex_answer("Enter VOD value: ", 0x0, 0x0, 0xf);

    for (ctr = s_port; ctr < e_port; ctr++) {
        if (esw_port_vod_adjust(esw_port, eth_mode, vod_val) != PASSED) {
            printf("Failed to adjust port%d VOD.\n", ctr);
            return (FAILED);
        }
        printf("Done adjust LAN Switch port%d VOD.\n", ctr);
    }

   return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_port_vod_adjust
 * Description: Function to adjust TSN LAN Switch port VOD.
 * Inputs     : esw_port - port number of switch
 *              eth_mode - 10/100/1000Mbps
 *              vod_val  - % that want to adjust 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_port_vod_adjust (int esw_port, int eth_mode, int vod_val)
{
    int    reg_page = (int)0xFC;
    int    reg_addr = (int)REG_ADDR(17);
    ushort reg_val = 0;

    if (eth_mode == 3) {
        reg_addr = (int)REG_ADDR(18);
    }

    /* Adjust VOD */
    if (tsn_esw_phy_reg_rd(esw_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read Reg. %d_%d.\n",
               __FUNCTION__, reg_addr, reg_page);
	return(FAILED);
    }

    if (eth_mode == 3) {
        reg_val = (ushort)((vod_val << 12) |
                           (vod_val << 8) |
                           (vod_val << 4) |
                           (vod_val));
    } else if (eth_mode == 1) {
        reg_val &= (ushort)(~0x00ff);
        reg_val |= (ushort)((vod_val << 4) | vod_val);
    } else {
        reg_val &= (ushort)(~0xff00);
        reg_val |= (ushort)((vod_val << 12) | (vod_val << 8));
    }

    msleep(10);

    if (tsn_esw_phy_reg_wr(esw_port, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to set Reg. %d_%d to 0x%04X.\n",
               __FUNCTION__, reg_addr, reg_page, reg_val);
	return(FAILED);
    }

    msleep(10);

    /* Recover Page Addr to 0x0 to trigger adjust process */
    reg_page = (int)REG_PAGE(0);
    reg_addr = (int)REG_ADDR(22);
    reg_val = 0;
    if (tsn_esw_phy_reg_wr(esw_port, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to trigger adjust process by set page addr to %d.\n",
	       __FUNCTION__, reg_val);
	return(FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_force_led_onoff
 * Description: Function to force TSN Switch port LEDs ON/OFF.
 * Inputs     : port_opt - port number
 *              onoff    - turn LED(s) ON/OFF
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_force_led_onoff (int port_opt, boolean onoff)
{
    int    start_addr = 0, end_addr = 0;
    int    ctr = 0;
    int    reg_addr = (int)ESW_LED_CONTR_REG;
    ushort reg_val = 0;

    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H */
        if (port_opt == ALL_ESW_LEDS) {
            start_addr = ESW_PORT1;
            end_addr = (int)(start_addr + TSN_H_ESW_GEPORTS);
        } else {
            if ((port_opt < ESW_PORT1) || (port_opt > ESW_PORT8)) {
                printf("%s: Input port(%d) is out of TSN-H bound(1 ~ 8).\n",
                       __FUNCTION__, port_opt);
                return (FAILED);
            }
            start_addr = port_opt;
            end_addr = (start_addr + 1);
        }
    } else {
        /* TSN-M */
        if (port_opt == ALL_ESW_LEDS) {
            start_addr = (int)(TSN_M_ESW_PORT_REG_BASE + ESW_PORT0);
            end_addr = (int)(start_addr + TSN_M_ESW_GEPORTS);
        } else {
            if ((port_opt < ESW_PORT1) || (port_opt > ESW_PORT4)) {
                printf("%s: Input port(%d) is out of TSN-M bound(1 ~ 4).\n",
                       __FUNCTION__, port_opt);
                return (FAILED);
            }
            start_addr = (int)(TSN_M_ESW_PORT_REG_BASE + (port_opt - 1));
            end_addr = (start_addr + 1);
        }
    }

    if (onoff == ESW_LED_F_ON) {
        reg_val = (ushort)(ESW_LCR_UPDATE |
                           ESW_LCR_LED1_F_ON |
                           ESW_LCR_LED0_F_ON);
    } else {
        reg_val = (ushort)(ESW_LCR_UPDATE |
                           ESW_LCR_LED1_F_OFF |
                           ESW_LCR_LED0_F_OFF);
    }

    for (ctr = start_addr; ctr < end_addr; ctr++) {
        if (tsn_esw_reg_wr(ctr, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to set ESW port%d forwarding.",
                   __FUNCTION__, __LINE__, ctr);
            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_force_led_onoff_util
 * Description: Utility to force TSN Switch port LEDs ON/OFF.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_force_led_onoff_util (int opt)
{
    int     port_opt = 1;
    boolean onoff = 0;

    port_opt = gethex_answer("Enter port number(0xf for all): ", 0x1, 0x1, 0xf);
    onoff = (boolean)getdec_answer("Turn it ON/OFF.(0: OFF, 1: ON): ", 0, 0, 1);

    if (tsn_esw_force_led_onoff(port_opt, onoff) != PASSED) {
        printf("Failed to Turn %s ESW LED.\n",
               (onoff == 1) ? "ON" : "OFF");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_force_led_onoff_util
 * Description: wrap utility to force TSN Switch port LEDs ON/OFF.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_force_led_onoff_util (int opt)
{   
    return (esw_force_led_onoff_util(0));
}

/*******************************************************************************
 *
 * Function   : tsn_set_esw_port_cap_util
 * Description: Utility to config TSN Switch port capability.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_set_esw_port_cap_util (int opt)
{
    int     port_opt = 0xa, speed_opt = 0x3;
    boolean duplex_opt = 1;

    port_opt = gethex_answer("Enter Port Number('a' for all): ", 0xa, 0x0, 0xa);
    speed_opt = gethex_answer("Enter wanted Speed(1-10M, 2-100M, 3-1000M): ",
                              0x3, 0x1, 0x3);
    duplex_opt = (boolean)gethex_answer("Enter wanted Duplex(0-Half, 1-Full): ",
                                        1, 0, 1);

    if (esw_config_port_cap(port_opt, speed_opt, duplex_opt) != PASSED) {
        return (FAILED);
    }
    printf("\nDone config. LAN port(s) successfullly.");

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_config_port_cap
 * Description: Function to config TSN Switch port capability.
 * Inputs     : port_opt - port number (0xa for all ports)
 *              speed    - 10(0x1)/100(0x2)/1000(0x3)Mbps
 *              duplex   - Full(0x1) / Half(0x0)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_config_port_cap (int port_opt, int speed, boolean duplex)
{
    int    ctr = 0, start_port = 0, end_port = 0, retry = 0;
    int    page = (int)REG_PAGE(0), reg_addr = 0;
    ushort reg_val = 0, reg_msk = 0;

    if ((speed > ESW_SET_PORT_1G) || (speed < ESW_SET_PORT_10M)) {
        printf("Speed option should be %d:10M, %d:100M, or %d: 1G.\n",
               ESW_SET_PORT_10M, ESW_SET_PORT_100M, ESW_SET_PORT_1G);
        return (FAILED);
    }

    /* Get port(s) that user wants to config. */
    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H: ESW has 8 LAN ports(1 ~ 8). */
        if ((port_opt >= ESW_PORT1) && (port_opt <= ESW_PORT8)) {
            start_port = port_opt;
            end_port = port_opt;
        } else if (port_opt == ESW_ALL_PHY_PORTS) {
            start_port = ESW_PORT1;
            end_port = ESW_PORT8;
        } else {
            printf("%s: Unknown LAN port(%d).\n", __FUNCTION__, port_opt);
            printf("%s: TSN-H has 8 LAN ports(1 ~ 8).\n", __FUNCTION__);
            return (FAILED);
        }
     } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P: ESW has 2 LAN ports(0 ~ 1). */
        if ((port_opt >= ESW_PORT0) && (port_opt <= ESW_PORT1)) {
            start_port = port_opt;
            end_port = port_opt;
        } else if (port_opt == ESW_ALL_PHY_PORTS) {
            start_port = ESW_PORT0;
            end_port = ESW_PORT1;
        } else {
            printf("%s: Unknown LAN port(%d).\n", __FUNCTION__, port_opt);
            printf("%s: TSN-M has 4 LAN ports(0 ~ 3).\n", __FUNCTION__);
            return (FAILED);
        }
    } else {
        /* TSN-M: ESW has 4 LAN ports(0 ~ 3). */
        if ((port_opt >= ESW_PORT0) && (port_opt <= ESW_PORT3)) {
            start_port = port_opt;
            end_port = port_opt;
        } else if (port_opt == ESW_ALL_PHY_PORTS) {
            start_port = ESW_PORT0;
            end_port = ESW_PORT3;
        } else {
            printf("%s: Unknown LAN port(%d).\n", __FUNCTION__, port_opt);
            printf("%s: TSN-M has 4 LAN ports(0 ~ 3).\n", __FUNCTION__);
            return (FAILED);
        }
    }

    for (ctr = start_port; ctr <= end_port; ctr++) {
        prpass(testpass, "Config. LAN port%d 1000Base-T Control Reg(9_0) ", ctr);
        /* 1. Config. 1000BASE-T Control Reg(9_0) */
        reg_addr = (int)ESWPHY_1000TCR_ADDR;
        if (tsn_esw_phy_reg_rd(ctr, page, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, page%d reg%d.",
                   __FUNCTION__, __LINE__, ctr, page, reg_addr);
            return (FAILED);
        }
        
        if (speed != (int)ESW_SET_PORT_1G) {
            reg_msk = (ushort)(ESWPHY_1G_CNTR_1000FD | ESWPHY_1G_CNTR_1000HD);

            if ((reg_val & reg_msk) != 0) {
                reg_val &= (ushort)(~reg_msk);

                if (tsn_esw_phy_reg_wr(ctr, page, reg_addr, reg_val) != PASSED) {
                    printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                           __FUNCTION__, __LINE__, ctr, page, reg_addr);
                    return (FAILED);
                }
                msleep(ESW_ACCESS_WAITTIME);
            }
        } else {
            if (duplex == ESW_SET_PORT_FD) {
                reg_msk = (ushort)ESWPHY_1G_CNTR_1000FD;

                if ((reg_val & reg_msk) == 0) {
                    reg_val |= reg_msk;

                    if (tsn_esw_phy_reg_wr(ctr, page, reg_addr,
                                           reg_val) != PASSED) {
                        printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                               __FUNCTION__, __LINE__, ctr, page, reg_addr);
                        return (FAILED);
                    }
                    msleep(ESW_ACCESS_WAITTIME);
                }
            } else {
                printf("\nLAN%d don't support 1000BaseT Half-duplex mode.\n", ctr);
                continue;
            }
        }

        /* 2. Config. Copper Auto-Negotitation Advertisement Reg(4_0) */
        prpass(testpass, "Config. LAN port%d Copper Auto-Negotitation Reg(4_0) ",
                         ctr);
        reg_addr = (int)ESWPHY_CANAR_ADDR;
        reg_val = 0;
        if (tsn_esw_phy_reg_rd(ctr, page, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, page%d reg%d.",
                   __FUNCTION__, __LINE__, ctr, page, reg_addr);
            return (FAILED);
        }

        if (speed == (int)ESW_SET_PORT_10M) {
            if (duplex == ESW_SET_PORT_HD) {
                reg_msk = (ushort)(ESWPHY_COP_ANAR_100FD |
                                   ESWPHY_COP_ANAR_100HD |
                                   ESWPHY_COP_ANAR_10FD);

                if (((reg_val & reg_msk) != 0) ||
                    ((reg_val & ESWPHY_COP_ANAR_10HD) == 0)) {
                    reg_val &= (ushort)(~reg_msk);
                    reg_val |= ESWPHY_COP_ANAR_10HD;

                    if (tsn_esw_phy_reg_wr(ctr, page, reg_addr,
                                           reg_val) != PASSED) {
                        printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                               __FUNCTION__, __LINE__, ctr, page, reg_addr);
                        return (FAILED);
                    }
                    msleep(ESW_ACCESS_WAITTIME);
                }
            } else {
                reg_msk = (ushort)(ESWPHY_COP_ANAR_100FD |
                                   ESWPHY_COP_ANAR_100HD |
                                   ESWPHY_COP_ANAR_10HD);

                if (((reg_val & reg_msk) != 0) ||
                    ((reg_val & ESWPHY_COP_ANAR_10FD) == 0)) {
                    reg_val &= (ushort)(~reg_msk);
                    reg_val |= ESWPHY_COP_ANAR_10FD;

                    if (tsn_esw_phy_reg_wr(ctr, page, reg_addr,
                                           reg_val) != PASSED) {
                        printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                               __FUNCTION__, __LINE__, ctr, page, reg_addr);
                        return (FAILED);
                    }
                    msleep(ESW_ACCESS_WAITTIME);
                }
           }
        } else if (speed == (int)ESW_SET_PORT_100M) {
            if (duplex == ESW_SET_PORT_HD) {
                reg_msk = (ushort)(ESWPHY_COP_ANAR_100FD |
                                   ESWPHY_COP_ANAR_10FD |
                                   ESWPHY_COP_ANAR_10HD);

                if (((reg_val & reg_msk) != 0) ||
                    ((reg_val & ESWPHY_COP_ANAR_100HD) == 0)) {
                    reg_val &= (ushort)(~reg_msk);
                    reg_val |= ESWPHY_COP_ANAR_100HD;

                    if (tsn_esw_phy_reg_wr(ctr, page, reg_addr,
                                           reg_val) != PASSED) {
                        printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                               __FUNCTION__, __LINE__, ctr, page, reg_addr);
                        return (FAILED);
                    }
                    msleep(ESW_ACCESS_WAITTIME);
                }
            } else {
                reg_msk = (ushort)(ESWPHY_COP_ANAR_100HD |
                                   ESWPHY_COP_ANAR_10FD |
                                   ESWPHY_COP_ANAR_10HD);

                if (((reg_val & reg_msk) != 0) ||
                    ((reg_val & ESWPHY_COP_ANAR_100FD) == 0)) {
                    reg_val &= (ushort)(~reg_msk);
                    reg_val |= ESWPHY_COP_ANAR_100FD;

                    if (tsn_esw_phy_reg_wr(ctr, page, reg_addr,
                                           reg_val) != PASSED) {
                        printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                               __FUNCTION__, __LINE__, ctr, page, reg_addr);
                        return (FAILED);
                    }
                    msleep(ESW_ACCESS_WAITTIME);
                }
            }
        }

        /* 3. Soft-reset ESW port PHY to apply new config. */
        prpass(testpass, "Soft-reset LAN port%d to apply new config. ", ctr);
        reg_val = (ushort)(ESWPHY_CCR_COP_RST | ESWPHY_CCR_AN_EN);
        reg_addr = (int)ESWPHY_CCR_ADDR;

        if (tsn_esw_phy_reg_wr(ctr, page, reg_addr, reg_val) != PASSED) {
            printf("%s:%d Failed to write ESW port%d, page%d reg%d.",
                   __FUNCTION__, __LINE__, ctr, page, reg_addr);
            return (FAILED);
        }
        msleep(ESW_ACCESS_WAITTIME);

        for (retry = 0; retry < ESW_RETRY_MAX; retry++) {
            reg_val = 0;

            if (tsn_esw_phy_reg_rd(ctr, page, reg_addr, &reg_val) != PASSED) {
                printf("%s:%d Failed to read ESW port%d, page%d reg%d.",
                       __FUNCTION__, __LINE__, ctr, page, reg_addr);
                return (FAILED);
            }

            if ((reg_val & ESWPHY_CCR_COP_RST) == 0) {
                break;
            }

            if (retry == (int)(ESW_RETRY_MAX - 1)) {
                printf("%s: Time out but PHY still not finish reset.",
                       __FUNCTION__);
                return (FAILED);
            }
        
            msleep(ESW_ACCESS_WAITTIME);
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_reset_esw_to_default
 * Description: Function to reset TSN switch and re-init it.
 * Inputs     : quiet_opt - To print message(opt = FALSE) or not(opt = TRUE)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_reset_esw_to_default (int quiet_opt) {
    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("This function will reset TSN switch and re-init it.\n");
    }

    /* 1. Reset TSN swtich. */
    /* 1-1. Put switch in Reset. */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, TRUE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to put switch in Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    msleep(ESW_RESET_ONE_SEC);

    /* 1-2. Release switch from Reset. */
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, EXT_ESW_RESET, FALSE,
                          WAITTIME_20_MS) != PASSED) {
        printf("%s: Failed to release switch from Reset.\n", __FUNCTION__);
        return (FAILED);
    }

    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("TSN switch is reset successfully.\n");
    }

    /* 2. Re-init TSN switch. */
    if (tsn_esw_init() != PASSED) {
        printf("%s: Failed to init TSN ethernet switch.\n", __FUNCTION__);
        return (FAILED);
    }

    if (((NVRAM)->diagflag & D_VERBOSE) || (quiet_opt == FALSE)) {
        printf("TSN switch is reset and re-inited successfully.\n");
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_phy_mac_lpbk_test
 * Description: Function to do TSN CPU to switch PHY MAC loopback test.
 * Inputs     : port_num - number of ESW port
 *              test_spd - test speed(10/100/1000 mbps) 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_phy_mac_lpbk_test (int port_num, int test_spd) {
    int    eth_num = (int)TSN_ESW_ETHNUM;
    int    reg_page = 0, reg_addr = 0;
    ushort phy_reg = 0;
    int    timer = 0;
    uint   cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(TSN_ESW_CPU_MACNUM);
    uint   cpu_reg = 0, cpu_reg_wrdata = 0;
    int    smi_dev_addr = 0;
    ushort smi_reg = 0, smi_spd_chk = 0;
    int    ctr = 0, start_port = 0, end_port = 0;

    /* 1. Check inputs */
    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H */
        if ((port_num < (int)ESW_PORT1) || (port_num > (int)ESW_PORT8)) {
            printf("%s: Unsupported TSN-H switch port(%d).\n",
                   __FUNCTION__, port_num);
            return (FAILED);
        }
    } else {
        /* TSN-M */
        if ((port_num < (int)ESW_PORT0) || (port_num > (int)ESW_PORT4)) {
            printf("%s: Unsupported TSN-M switch port(%d).\n",
                   __FUNCTION__, port_num);
            return (FAILED);
        }
    }

    if ((test_spd != SPD_10MBPS) && (test_spd != SPD_100MBPS) &&
        (test_spd != SPD_1000MBPS)) {
        printf("%s: Unsupported test speed(%d).\n", __FUNCTION__, test_spd);
        return (FAILED);
    }

    /* 2. Configure ESW PHY */
    /* 2-1. Set ESW PHY MAC speed */
    reg_page = REG_PAGE(2);
    reg_addr = REG_ADDR(21);
    phy_reg = 0;
    if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr, &phy_reg) != PASSED) {
        printf("%s(%d): Failed to read ESW port%d PHY reg %d_%d.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d PHY reg %d_%d = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page, phy_reg);
    }

    phy_reg &= (ushort)(~ESWPHY_MSCR2_MACSPD_MSK);
    switch(test_spd) {
        case SPD_10MBPS:
            phy_reg |= (ushort)ESWPHY_MSCR2_MACSPD_10MBPS;
            break;
        case SPD_100MBPS:
            phy_reg |= (ushort)ESWPHY_MSCR2_MACSPD_100MBPS;
            break;
        case SPD_1000MBPS:
            phy_reg |= (ushort)ESWPHY_MSCR2_MACSPD_1000MBPS;
            break;
        default:  
            printf("%s(%d): Unsupported test speed(%d).\n",
                   __FUNCTION__, __LINE__, test_spd);
            return (FAILED);
            break;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d PHY reg %d_%d set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page, phy_reg);
    }

    if (tsn_esw_phy_reg_wr(port_num, reg_page, reg_addr, phy_reg) != PASSED) {
        printf("%s(%d): Failed to write ESW port%d PHY reg %d_%d.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    /* 2-2. Set PHY Loopback mode */
    reg_page = REG_PAGE(0);
    reg_addr = REG_ADDR(0);
    phy_reg = 0;
    if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr, &phy_reg) != PASSED) {
        printf("%s(%d): Failed to read ESW port%d PHY reg %d_%d.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    phy_reg &= (ushort)(~ESWPHY_CCR_AN_EN);
    phy_reg |= (ushort)ESWPHY_CCR_COP_RST;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d PHY reg %d_%d set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page, phy_reg);
    }

    if (tsn_esw_phy_reg_wr(port_num, reg_page, reg_addr, phy_reg) != PASSED) {
        printf("%s(%d): Failed to write ESW port%d PHY reg %d_%d.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    do {
        phy_reg = 0;
        if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr,
                               &phy_reg) != PASSED) {
            printf("%s(%d): Failed to read ESW port%d PHY reg %d_%d.\n",
                   __FUNCTION__, __LINE__, port_num, reg_addr, reg_page);
            return (FAILED);
        }

        if ((phy_reg & (ushort)ESWPHY_CCR_COP_RST) == 0) {
            break;
        }

        msleep(ESW_ACCESS_WAITTIME);
        timer += ESW_ACCESS_WAITTIME;
    }while(timer < ESW_PHY_RST_TIMEOUT);

    if (timer >= ESW_PHY_RST_TIMEOUT) {
        printf("%s(%d): TIMEOUT! ESW port%d PHY didn't complete Reset.\n",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    phy_reg |= (ushort)ESWPHY_CCR_LPBK;
    if (tsn_esw_phy_reg_wr(port_num, reg_page, reg_addr, phy_reg) != PASSED) {
        printf("%s(%d): Failed to write ESW port%d PHY reg %d_%d.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    /* 3. Config. ESW port */
    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H */
        smi_dev_addr = port_num;
        start_port = (int)ESW_PORT1;
        end_port = (int)ESW_PORT8;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P */
        smi_dev_addr = (int)(port_num + TSN_M_ESW_PORT_REG_BASE);
        start_port = (int)ESW_PORT0;
        end_port = (int)ESW_PORT1;
    } else {
        /* TSN-M */
        smi_dev_addr = (int)(port_num + TSN_M_ESW_PORT_REG_BASE);
        start_port = (int)ESW_PORT0;
        end_port = (int)ESW_PORT3;
    }

    /* 3-1. Config. ESW port SMI register 0x1 */
    reg_addr = (int)REG_ADDR(1);
    smi_reg = 0;
    if (tsn_esw_reg_rd(smi_dev_addr, reg_addr, &smi_reg) != PASSED) {
        printf("%s(%d): Failed to read ESW port%d SMI register 0x%02X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    if ((smi_reg & (ushort)(ESW_PCR_FORCE_LINK | ESW_PCR_F_LINKUP)) !=
        (ushort)(ESW_PCR_FORCE_LINK)) {
        smi_reg |= (ushort)ESW_PCR_FORCE_LINK;
        smi_reg &= (ushort)(~ESW_PCR_F_LINKUP);

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s(%d): ESW port%d SMI reg. 0x%02X set data = 0x%04X.\n",
                   __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
        }

        if (tsn_esw_reg_wr(smi_dev_addr, reg_addr, smi_reg) != PASSED) {
            printf("%s(%d): Failed to write ESW port%d SMI register 0x%02X.\n",
                   __FUNCTION__, __LINE__, port_num, reg_addr);
            return (FAILED);
        }
    }

    smi_reg = (ushort)(ESW_PCR_FORCE_SPEED | ESW_PCR_FORCE_LINK |
                       ESW_PCR_FORCE_DPX | ESW_PCR_F_FULLDPX);

    switch(test_spd) {
        case SPD_10MBPS:
            break;
        case SPD_100MBPS:
            smi_reg |= (ushort)ESW_PCR_100MBPS;
            smi_spd_chk = (ushort)ESW_PSR_100MBPS;
            break;
        case SPD_1000MBPS:
            smi_reg |= (ushort)ESW_PCR_1000MBPS;
            smi_spd_chk = (ushort)ESW_PSR_1000MBPS;
            break;
        default:  
            printf("%s(%d): Unsupported test speed(%d).\n",
                   __FUNCTION__, __LINE__, test_spd);
            return (FAILED);
            break;
    }

    smi_reg |= (ushort)ESW_PCR_F_LINKUP;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): ESW port%d SMI reg. 0x%02X set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
    }

    if (tsn_esw_reg_wr(smi_dev_addr, reg_addr, smi_reg) != PASSED) {
        printf("%s(%d): Failed to write ESW port%d SMI register 0x%02X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    msleep(ESW_ACCESS_WAITTIME);

    reg_addr = (int)REG_ADDR(0);
    smi_reg = 0;
    if (tsn_esw_reg_rd(smi_dev_addr, reg_addr, &smi_reg) != PASSED) {
        printf("%s(%d): Failed to read ESW port%d SMI register 0x%02X.\n",
               __FUNCTION__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    if ((smi_reg & (ushort)ESW_PSR_LINK) != (ushort)ESW_PSR_LINKUP) {
        printf("%s(%d): Failed to force link up ESW port%d.\n",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    if ((smi_reg & (ushort)ESW_PSR_DPX) != (ushort)ESW_PSR_FULLDPX) {
        printf("%s(%d): Failed to force full duplex ESW port%d.\n",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    if ((smi_reg & (ushort)ESW_PSR_SPD_MSK) != smi_spd_chk) {
        printf("%s(%d): Failed to force correct speed ESW port%d.\n",
               __FUNCTION__, __LINE__, port_num);
        return (FAILED);
    }

    /* 3-2. Disable all other ESW ports */
    reg_addr = (int)REG_ADDR(4);
    for (ctr = start_port; ctr <= end_port; ctr++) {
        if (ctr == port_num) {
            continue;
        }

        if (this_is_tsn_h_sku() == TRUE) {
            /* TSN-H */
            smi_dev_addr = ctr;
        } else {
            /* TSN-M */
            smi_dev_addr = (ctr + (int)TSN_M_ESW_PORT_REG_BASE);
        }

        smi_reg = 0;
        if (tsn_esw_reg_rd(smi_dev_addr, reg_addr, &smi_reg) != PASSED) {
            printf("%s(%d): Failed to read ESW port%d SMI register 0x%02X.\n",
                   __FUNCTION__, __LINE__, ctr, reg_addr);
            return (FAILED);
        }

        if ((smi_reg & (ushort)ESW_PCR_PS_MSK) != (ushort)ESW_PCR_PORT_DIS) {
            smi_reg &= (ushort)(~ESW_PCR_PS_MSK);

            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("%s(%d): ESW port%d SMI reg. 0x%02X set data = 0x%04X.\n",
                       __FUNCTION__, __LINE__, port_num, reg_addr, smi_reg);
            }

            if (tsn_esw_reg_wr(smi_dev_addr, reg_addr, smi_reg) != PASSED) {
                printf("%s(%d): Failed to write ESW port%d SMI register 0x%02X.\n",
                       __FUNCTION__, __LINE__, port_num, reg_addr);
                return (FAILED);
            }
        }
    }

    /* 4. Config. CPU GE MAC(to switch) */
    /* 4-1. Force link Down CPU GE MAC2 before set it up */
    cpu_reg = 0;
    if (tsn_mem_read32(cpu_reg_addr, &cpu_reg) != PASSED) {
        printf("%s(%d): Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    if ((cpu_reg & (uint)PANCR_FORCE_LINK_MSK) != (uint)PANCR_FORCE_LINK_DOWN) {
        cpu_reg &= (uint)PANCR_FORCE_LINK_MSK;
        cpu_reg |= (uint)PANCR_FORCE_LINK_DOWN;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): CPU register 0x%08X set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr, cpu_reg);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg) != PASSED) {
        printf("%s(%d): Failed to write CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    /* 4-2. Force link Up CPU GE MAC2 with new set-ups */
    cpu_reg_wrdata = (uint)(PANCR_RESERVED | PANCR_SET_FULL_DUPLEX |
                            PANCR_SUPPORT_FC);

    switch(test_spd) {
        case SPD_10MBPS:
            break;
        case SPD_100MBPS:
            cpu_reg_wrdata |= (uint)PANCR_SET_MII_100;
            break;
        case SPD_1000MBPS:
            cpu_reg_wrdata |= (uint)PANCR_SET_SGMII_1000;
            break;
        default:  
            printf("%s(%d): Unsupported test speed(%d).\n",
                   __FUNCTION__, __LINE__, test_spd);
            return (FAILED);
            break;
    }

    cpu_reg_wrdata |= (uint)PANCR_FORCE_LINK_UP;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): CPU register 0x%08X set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr, cpu_reg_wrdata);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_wrdata) != PASSED) {
        printf("%s(%d): Failed to write CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    msleep(ESW_ACCESS_WAITTIME);

    cpu_reg = 0;
    if (tsn_mem_read32(cpu_reg_addr, &cpu_reg) != PASSED) {
        printf("%s(%d): Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    if (cpu_reg != cpu_reg_wrdata) {
        printf("%s(%d): Failed to set CPU register 0x%08X to 0x%04X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr, cpu_reg_wrdata);
        return (FAILED);
    }

    /* 5. Run SGMII loopback test */
    if (tsn_sgmii_lpbk_test(eth_num, test_spd) != PASSED) {
        printf("%s: Failed to run ESW(eth%d) loopback test ",
               __FUNCTION__, eth_num);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_phy_mac_lpbk_test_util
 * Description: Utility to do TSN CPU to switch PHY MAC loopback test.
 *              This is for debug purpose.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_phy_mac_lpbk_test_util (int opt) {
    int port_num = 0;
    int s_port = 0, e_port = 0;
    int spd_ctr = 0, test_spd = 0;

    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H */
        s_port = (int)ESW_PORT1;
        e_port = (int)ESW_PORT8;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P */
        s_port = (int)ESW_PORT0;
        e_port = (int)ESW_PORT1;
    } else {
        /* TSN-M */
        s_port = (int)ESW_PORT0;
        e_port = (int)ESW_PORT3;
    }

    port_num = gethex_answer("Enter ESW port number: ", s_port, s_port, e_port);
    spd_ctr = gethex_answer("Enter Testspeed(0-10mbps, 1-100mbps, 2-1000mbps): ",
                            0, 0, 2);

    test_spd = esw_speed_tbl[spd_ctr];

    if (esw_phy_mac_lpbk_test(port_num, test_spd) != PASSED) {
        printf("CPU to ESW port%d PHY MAC loopback test at %dmbps failed.\n",
               port_num, test_spd);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_cpu_esw_mac_lpbk_test
 * Description: Function to do TSN CPU to switch PHY MAC loopback test.
 * Inputs     : opt - reserve for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_cpu_esw_mac_lpbk_test (int opt) {
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "SGMII", "Marvell 88E6390 Ethernet Switch", "Marvell 88E1112 GE WAN Phy");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Observe PCIe register status to check "
                    "if switch configuration is normal.",
                    "If step1 is OK, we can assume the interface "
                    "between Host SoC and switch has problems.");
#endif

    char tname[32]; 
    int  p_ctr = 0, spd_ctr = 0, test_spd = 0;
    int  start_port = 0, end_port = 0;
    int  total_spd = 0;

    memset(tname, 0, sizeof(tname));
    sprintf(tname, "CPU to switch PHY MAC loopback");
    testname(tname);
    prpass(testpass, "%s, ", tname);

    total_spd = sizeof(esw_speed_tbl) / sizeof(int);

    if (this_is_tsn_h_sku() == TRUE) {
        /* TSN-H */
        start_port = (int)ESW_PORT1;
        end_port = (int)ESW_PORT8;
    } else if (this_is_star_c1109_2p() || this_is_supernova_c959_2p()) {
        /* Star C1109-2P / Supernova C959-2P */
        start_port = (int)ESW_PORT0;
        end_port = (int)ESW_PORT1;
    } else {
        /* TSN-M */
        start_port = (int)ESW_PORT0;
        end_port = (int)ESW_PORT3;
    }

    for (p_ctr = start_port; p_ctr <= end_port; p_ctr++) {
        for (spd_ctr = 0; spd_ctr < total_spd; spd_ctr++) {
            test_spd = esw_speed_tbl[spd_ctr];
            prpass(testpass, "Testing switch port%d in %dmbps ",
                             p_ctr, test_spd);

            if (esw_phy_mac_lpbk_test(p_ctr, test_spd) != PASSED) {
                cterr('f', 0, "Failed at ESW port%d in %dmbps ", p_ctr, test_spd);
                return (FAILED);
            }

            if (tsn_reset_esw_to_default(TRUE) != PASSED) {
                cterr('f', 0, "Failed reset switch ");
                return (FAILED);
            }
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_show_phy_status
 * Description: Function to show switch PHY status per port.
 * Inputs     : esw_port - port number of switch
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_show_phy_status (int esw_port)
{
    ushort result = 0;
    uint   speed = 0;
    int    reg_page = (int)PHY_PAGE(0);
    int    reg_addr = 0;
    ushort reg_val = 0;

    /* Get value of Copper Auto-nego Adv. register(4_0) */
    reg_addr = (int)COP_AUTONEG_ADV_REG4;
    if (tsn_esw_phy_reg_rd(esw_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read ESW Port%d PHY Reg.(%d_%d).\n",
               __FUNCTION__, __LINE__, esw_port, reg_addr, reg_page);
        return (FAILED);
    }
    printf("Port%d PHY Copper Auto-Nego Adv Reg(%d_%d) = 0x%04X.\n",
           esw_port, reg_addr, reg_page, reg_val);

    /* Get value of Copper Specific Status register 1(17_0) */
    reg_addr = (int)COP_STATUS_REG17;
    reg_val = 0;
    if (tsn_esw_phy_reg_rd(esw_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d): Failed to read ESW Port%d PHY Reg.(%d_%d).\n",
               __FUNCTION__, __LINE__, esw_port, reg_addr, reg_page);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Port%d PHY Copper Auto-Nego Adv Reg(%d_%d) = 0x%04X.\n",
               esw_port, reg_addr, reg_page, reg_val);
    }

    result = ((reg_val & (ushort)COP_P0R17_SPEED) >> COP_P0R17_SPEED_OFFSET);

    switch (result) {
    case COP_P0R17_SPEED_1000:
        speed = SPD_1000MBPS;
    break;
    case COP_P0R17_SPEED_100:
        speed = SPD_100MBPS;
    break;
    case COP_P0R17_SPEED_10:
        speed = SPD_10MBPS;
    break;
    default:
        printf("Unknown speed value: %d.\n", result);
    break;
    }   
    prpass(testpass, "PHY Speed is %d Mbps", speed);

    if (reg_val & (ushort)COP_P0R17_DUPLEX_FULL) {
        prpass(testpass, "PHY is Full Duplex");
    } else {
        prpass(testpass, "PHY is Half Duplex");
    }
   
    if (reg_val & (ushort)COP_P0R17_COP_LINK_UP) {
        prpass(testpass, "Copper Link Up");
    } else { 
        prpass(testpass, "Copper Link Down");
    }
   
    if (reg_val & (ushort)COP_P0R17_GLOBAL_LINK_UP) {
        prpass(testpass, "Global Link Status is Up");
    } else { 
        prpass(testpass, "Global Link Status is Down");
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_config_cpu_port_link_up
 * Description: Function to config CPU GE PORT MAC to link up 
 *              test_spd - speed that want to set (10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_config_cpu_port_link_up (int test_spd)
{
    uint   cpu_reg_addr = (uint)CPU_PORT_AN_CONF_REG(TSN_ESW_CPU_MACNUM);
    uint   cpu_reg = 0, cpu_reg_wrdata = 0;
    
    /* Config. CPU GE MAC(to switch) */
    /* Force link Down CPU GE MAC2 before set it up */
    cpu_reg = 0;
    if (tsn_mem_read32(cpu_reg_addr, &cpu_reg) != PASSED) {
        printf("%s(%d): Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    if ((cpu_reg & (uint)PANCR_FORCE_LINK_MSK) != (uint)PANCR_FORCE_LINK_DOWN) {
        cpu_reg &= (uint)PANCR_FORCE_LINK_MSK;
        cpu_reg |= (uint)PANCR_FORCE_LINK_DOWN;
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): CPU register 0x%08X set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr, cpu_reg);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg) != PASSED) {
        printf("%s(%d): Failed to write CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    /* Force link Up CPU GE MAC2 with new set-ups */
    cpu_reg_wrdata = (uint)(PANCR_RESERVED | PANCR_SET_FULL_DUPLEX |
                            PANCR_SUPPORT_FC);

    switch (test_spd) {
        case SPD_10MBPS:
            break;
        case SPD_100MBPS:
            cpu_reg_wrdata |= (uint)PANCR_SET_MII_100;
            break;
        case SPD_1000MBPS:
            cpu_reg_wrdata |= (uint)PANCR_SET_SGMII_1000;
            break;
        default:  
            printf("%s(%d): Unsupported test speed(%d).\n",
                   __FUNCTION__, __LINE__, test_spd);
            return (FAILED);
            break;
    }

    cpu_reg_wrdata |= (uint)PANCR_FORCE_LINK_UP;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s(%d): CPU register 0x%08X set data = 0x%04X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr, cpu_reg_wrdata);
    }

    if (tsn_mem_write32(cpu_reg_addr, cpu_reg_wrdata) != PASSED) {
        printf("%s(%d): Failed to write CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    msleep(ESW_ACCESS_WAITTIME);

    cpu_reg = 0;
    if (tsn_mem_read32(cpu_reg_addr, &cpu_reg) != PASSED) {
        printf("%s(%d): Failed to read CPU register 0x%08X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr);
        return (FAILED);
    }

    if (cpu_reg != cpu_reg_wrdata) {
        printf("%s(%d): Failed to set CPU register 0x%08X to 0x%04X.\n",
               __FUNCTION__, __LINE__, cpu_reg_addr, cpu_reg_wrdata);
        return (FAILED);
    }

    return (PASSED);
    
}


/*******************************************************************************
 *
 * Function   : esw_force_speed_fn
 * Description: Function to force switch MAC and PHY in specific speed.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_force_speed_fn (int port_num, int speed_opt)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)REG_ADDR(17);
    ushort reg_val = 0, chk_msk = 0, chk_val = 0;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    /* Configure PHY side */
    if (esw_force_phy_speed(port_num, speed_opt) != PASSED) {
        printf("%s(%d) Failed to force Port%d PHY at %dmbps.\n",
               __func__, __LINE__, port_num, speed_opt);
        return (FAILED);
    }

    /* Configure MAC side */
    if (esw_force_mac_speed(port_num, speed_opt) != PASSED) {
        printf("%s(%d) Failed to force Port%d MAC at %dmbps.\n",
               __func__, __LINE__, port_num, speed_opt);
        return (FAILED);
    }

    /* Confirm port status */
    /* Based on datasheet, we can confirm PHY link status,
     * linked speed and duplex from Copper Specific Status Register(17_0)
     */
    chk_msk = (ushort)(ESWPHY_CSSR1_RESOLVED |
                       ESWPHY_CSSR1_COP_LINK |
                       ESWPHY_CSSR1_LINK_STAT);
    chk_val = (ushort)(ESWPHY_CSSR1_RESOLVED |
                       ESWPHY_CSSR1_RT_LINK_UP |
                       ESWPHY_CSSR1_COP_LINK_UP);

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr,
                               &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d PHY register %d_%d.\n",
                   __func__, __LINE__, port_num, reg_addr, reg_page);
            return (FAILED);
        }

        if ((reg_val & chk_msk) == chk_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d PHY reg. %d_%d = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_page, reg_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Timeout! And ESW port%d PHY still NOT link up.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    /* Confirm the config. value is as expected. */
    /* Confirm PHY link up speed */
    chk_msk = (ushort)ESWPHY_CSSR1_SPEED;
    switch(speed_opt) {
        case SPD_10MBPS:
            chk_val = (ushort)ESWPHY_CSSR1_SPD_10MBPS;
            break;
        case SPD_100MBPS:
            chk_val = (ushort)ESWPHY_CSSR1_SPD_100MBPS;
            break;
        case SPD_1000MBPS:
            chk_val = (ushort)ESWPHY_CSSR1_SPD_1000MBPS;
            break;
        default:  
            printf("%s(%d): Unsupported speed(%d).\n",
                   __func__, __LINE__, speed_opt);
            return (FAILED);
            break;
    }

    if ((reg_val & chk_msk) != chk_val) {
        printf("%s(%d) Failed to set ESW port%d PHY link at %dmbps.\n",
               __func__, __LINE__, port_num, speed_opt);
        return (FAILED);
    }

    /* Confirm PHY link up duplex */
    chk_msk = (ushort)ESWPHY_CSSR1_DUPLEX;
    chk_val = (ushort)ESWPHY_CSSR1_FULLDUP;
    if ((reg_val & chk_msk) != chk_val) {
        printf("%s(%d) Failed to set ESW port%d PHY link at Full-duplex.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_force_phy_speed
 * Description: Function to force switch PHY in specific speed.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_force_phy_speed (int port_num, int speed_opt)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)REG_ADDR(0);
    ushort reg_val = 0, chk_val = 0;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    /* Based on datasheet, to force PHY at specific speed needs to
     * configure PHY Copper Control Register(page0, reg0; 0_0).
     */
    /* Config. speed */
    switch(speed_opt) {
        case SPD_10MBPS:
            chk_val = (ushort)COP_SPD_10Mbps;
            break;
        case SPD_100MBPS:
            chk_val = (ushort)COP_SPD_100Mbps;
            break;
        case SPD_1000MBPS:
            chk_val = (ushort)COP_SPD_1000Mbps;
            break;
        default:  
            printf("%s(%d): Unsupported speed(%d).\n",
                   __func__, __LINE__, speed_opt);
            return (FAILED);
            break;
    }

    /* Disable Auto-negotiation and force Duplex mode to Full-duplex */
    chk_val |= (ushort)COP_CTRL_DUPLEX_FULL;

    /* Based on datasheet, software reset is needed for speed change. */
    reg_val = (ushort)(chk_val | COP_CTRL_RESET);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Value that wanted write to ESW port%d "
               "PHY reg. %d_%d = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_page, reg_val);
    }

    if (tsn_esw_phy_reg_wr(port_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d PHY register %d_%d.\n",
               __func__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    /* Confirm by ESW port PHY out of reset and is configured as expected */
    do {
        gettimeofday(&t_start, NULL);
        reg_val = (ushort)COP_CTRL_RESET;

        if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr,
                               &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d PHY register %d_%d.\n",
                   __func__, __LINE__, port_num, reg_addr, reg_page);
            return (FAILED);
        }

        if ((reg_val & (ushort)COP_CTRL_RESET) == 0) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d PHY reg. %d_%d = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_page, reg_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Timeout! And ESW port%d PHY still in RESET mode.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    /* Confirm the config. value is as expected.
     * Ignore Copper Reset bit because it is cleared automatically
     * after itself out of reset based on datasheet.
     */
    if (reg_val != chk_val) {
        printf("%s(%d) Failed to force ESW port%d PHY at %dmbps.\n",
               __func__, __LINE__, port_num, speed_opt);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_force_mac_speed
 * Description: Function to force switch MAC in specific speed.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 *              speed_opt - speed that want to set(10/100/1000mbps)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_force_mac_speed (int port_num, int speed_opt)
{
    int    esw_port = 0;
    int    reg_addr = (int)ESW_PCR_ADDR;
    ushort reg_val = 0, chk_val = 0, spd_val = 0;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    /* Get the mapped ESW port number */
    if (tsn_board_sku == TSN_H_MB) {
        esw_port = port_num;
    } else {
        /* TSN-M */
        esw_port = (int)(port_num + TSN_M_ESW_PORT_REG_BASE);
    }

    /* Config. speed */
    switch(speed_opt) {
        case SPD_10MBPS:
            spd_val = (ushort)ESW_PCR_10MBPS;
            break;
        case SPD_100MBPS:
            spd_val = (ushort)ESW_PCR_100MBPS;
            break;
        case SPD_1000MBPS:
            spd_val = (ushort)ESW_PCR_1000MBPS;
            break;
        default:  
            printf("%s(%d): Unsupported speed(%d).\n",
                   __func__, __LINE__, speed_opt);
            return (FAILED);
            break;
    }

    /* Check if MAC speed is expected */
    chk_val = (ushort)(ESW_PCR_F_LINKUP |
                       ESW_PCR_FORCE_LINK |
                       ESW_PCR_F_FULLDPX |
                       ESW_PCR_FORCE_DPX |
                       spd_val);

    if (tsn_board_sku == TSN_H_MB) {
        chk_val |= (ushort)ESW_PCR_FORCE_SPEED;
    }

    if (tsn_esw_reg_rd(esw_port, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d Reg.0x%02X = 0x%04X.\n",
               __func__, __LINE__, port_num, reg_addr, reg_val);
        printf("DBG[%s(%d)] check value = 0x%04X.\n\n",
               __func__, __LINE__, chk_val);
    }

    /* If yes, return PASS directly; if no, change MAC speed.*/
    if (reg_val == chk_val) {
        return (PASSED);
    }

    /* Force link down MAC before change its speed based on datasheet */
    reg_val &= (ushort)(~(ESW_PCR_F_LINKUP | ESW_PCR_FORCE_LINK));
    reg_val |= (ushort)ESW_PCR_FORCE_LINK;
    reg_val &= (ushort)(~(ESW_PCR_DEF_VAL | ESW_PCR_FORCE_SPEED));
    reg_val &= (ushort)(~(ESW_PCR_F_FULLDPX | ESW_PCR_FORCE_DPX));
    reg_val |= (ushort)spd_val;
    
    if (tsn_esw_reg_wr(esw_port, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    /* Confirm ESW port MAC is linked down.
     * By checking Link status bit(bit 11) of Port Status Reg.(0x0)
     */
    reg_addr = (int)ESW_PSR_ADDR;
    do {
        gettimeofday(&t_start, NULL);
        reg_val = (ushort)ESW_PSR_LINKUP;

        if (tsn_esw_reg_rd(esw_port, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
                   __func__, __LINE__, port_num, reg_addr);
            return (FAILED);
        }

        if ((reg_val & (ushort)ESW_PSR_LINK) == 0) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d Reg.0x%02X = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to force ESW port%d MAC link down.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    /* Config. ESW testing port MAC as expected */
    reg_addr = (int)ESW_PCR_ADDR;

    if (tsn_esw_reg_wr(esw_port, reg_addr, chk_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    /* Confirm ESW testing port MAC is configured correctly */
    t_diff = 0;
    ret_val = FAILED;
    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (tsn_esw_reg_rd(esw_port, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
                   __func__, __LINE__, port_num, reg_addr);
            return (FAILED);
        }

        if (reg_val == chk_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d Reg.0x%02X = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to configure ESW port%d MAC for test.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_enable_autonego_fn
 * Description: Function to enable switch MAC and PHY auto-negotiation.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_enable_autonego_fn (int port_num)
{
    /* Configure MAC side */
    if (esw_enable_mac_autonego(port_num) != PASSED) {
        printf("%s(%d) Failed to enable Port%d MAC auto-negotiation.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    /* Configure PHY side */
    if (esw_enable_phy_autonego(port_num) != PASSED) {
        printf("%s(%d) Failed to enable Port%d PHY auto-negotiation.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_enable_mac_autonego
 * Description: Function to enable switch MAC auto-negotiation.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_enable_mac_autonego (int port_num)
{
    int    esw_port = 0;
    int    reg_addr = (int)ESW_PCR_ADDR;
    ushort reg_val = 0, chk_val = (ushort)ESW_PCR_DEF_VAL;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    /* Get the mapped ESW port number */
    if (tsn_board_sku == TSN_H_MB) {
        esw_port = port_num;
    } else {
        /* TSN-M */
        esw_port = (int)(port_num + TSN_M_ESW_PORT_REG_BASE);
    }

    /* Check if needs to enable auto-negotiation */
    if (tsn_esw_reg_rd(esw_port, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    /* If MAC auto-negotitation is already enabled, just return PASS. */
    if (reg_val == chk_val) {
        return (PASSED);
    }

    /* Try to enable MAC auto-negotitation */
    /* Force link down MAC before re-config based on datasheet */
    reg_val &= (ushort)(~(ESW_PCR_F_LINKUP | ESW_PCR_FORCE_LINK));
    reg_val |= (ushort)ESW_PCR_FORCE_LINK;
    if (tsn_esw_reg_wr(esw_port, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    /* Confirm ESW port MAC is linked down.
     * By checking Link status bit(bit 11) of Port Status Reg.(0x0)
     */
    reg_addr = (int)ESW_PSR_ADDR;
    do {
        gettimeofday(&t_start, NULL);
        reg_val = (ushort)ESW_PSR_LINKUP;

        if (tsn_esw_reg_rd(esw_port, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
                   __func__, __LINE__, port_num, reg_addr);
            return (FAILED);
        }

        if ((reg_val & (ushort)ESW_PSR_LINK) == 0) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d Reg.0x%02X = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to force ESW port%d MAC link down.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    /* Enable MAC auto-negotiation */
    reg_addr = (int)ESW_PCR_ADDR;
    if (tsn_esw_reg_wr(esw_port, reg_addr, chk_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d Reg.0x%02X\n",
               __func__, __LINE__, port_num, reg_addr);
        return (FAILED);
    }

    /* Confirm ESW testing port MAC is configured correctly */
    t_diff = 0;
    ret_val = FAILED;
    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        if (tsn_esw_reg_rd(esw_port, reg_addr, &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d Reg.0x%02X\n",
                   __func__, __LINE__, port_num, reg_addr);
            return (FAILED);
        }

        if (reg_val == chk_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d Reg.0x%02X = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to enable ESW port%d MAC auto-negotiation.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : esw_enable_phy_autonego
 * Description: Function to enable switch PHY auto-negotiation.
 *              This is for testing purpose.
 * Inputs     : port_num - port number that want to config.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_enable_phy_autonego (int port_num)
{
    int    reg_page = (int)REG_PAGE(0);
    int    reg_addr = (int)REG_ADDR(0);
    ushort reg_val = 0, chk_val = 0;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int    ret_val = FAILED;

    /* Check if needs to enable auto-negotiation */
    chk_val = (ushort)(COP_CTRL_AUTONEG |
                       COP_CTRL_DUPLEX_FULL |
                       COP_SPD_1000Mbps);
    if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s(%d) Failed to read ESW port%d PHY register %d_%d.\n",
               __func__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    /* If MAC auto-negotitation is already enabled, just return PASS. */
    if (reg_val == chk_val) {
        return (PASSED);
    }

    /* Based on datasheet, software reset is needed for reconfig PHY. */
    reg_val = (ushort)(chk_val | COP_CTRL_RESET);

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Value that wanted write to ESW port%d "
               "PHY reg. %d_%d = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_page, reg_val);
    }

    if (tsn_esw_phy_reg_wr(port_num, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s(%d) Failed to write ESW port%d PHY register %d_%d.\n",
               __func__, __LINE__, port_num, reg_addr, reg_page);
        return (FAILED);
    }

    /* Confirm by ESW port PHY out of reset and is configured as expected */
    do {
        gettimeofday(&t_start, NULL);
        reg_val = (ushort)COP_CTRL_RESET;

        if (tsn_esw_phy_reg_rd(port_num, reg_page, reg_addr,
                               &reg_val) != PASSED) {
            printf("%s(%d) Failed to read ESW port%d PHY register %d_%d.\n",
                   __func__, __LINE__, port_num, reg_addr, reg_page);
            return (FAILED);
        }

        if (reg_val == chk_val) {
            ret_val = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\n\nDBG[%s(%d)] Now ESW port%d PHY reg. %d_%d = 0x%04X.\n\n",
               __func__, __LINE__, port_num, reg_addr, reg_page, reg_val);
        printf("DBG[%s(%d)] chk_val = 0x%04X.\n\n", __func__, __LINE__, chk_val);
    }

    if (ret_val != PASSED) {
        printf("%s(%d) Failed to enable ESW port%d PHY auto-negotiation.\n",
               __func__, __LINE__, port_num);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_port_onoff
 * Description: Function to enable/disable TSN ethernet switch port.
 * Inputs     : p_num - Ethernet switch port number
 *              opt - ENABLE/DISABLE ethernet switch port
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_port_onoff (int p_num, boolean opt)
{
    ushort reg_val = 0, exp_val = 0;
    ushort reg_mask = (ushort)ESW_PCR_PS_MSK;
    int reg_addr = (int)ESW_PORTCTR_REG;
    int chk_result = FAILED;
    struct timeval t_start, t_end;
    double t_diff = 0;
    int esw_p_num = 0;

    /* Config expected value */
    if (opt == ENABLE) {
        exp_val = (ushort)ESW_PCR_PORT_FORWARD;
    } else {
        exp_val = (ushort)ESW_PCR_PORT_DIS;
    }

    /* Getting mapped ethernet switch port number */
    if (tsn_board_sku == TSN_H_MB) {
        esw_p_num = p_num;
    } else {
        esw_p_num = (int)(p_num + TSN_M_ESW_PORT_REG_BASE);
    }

    /* Read current Port Control register(0x4) out */
    if (tsn_esw_reg_rd(esw_p_num, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d, reg%d.",
               __func__, __LINE__, p_num, reg_addr);
        return (FAILED);
    }

    if ((reg_val & reg_mask) == exp_val) {
        return (PASSED);
    }

    reg_val &= (ushort)(~reg_mask);
    reg_val |= exp_val;

    if (tsn_esw_reg_wr(esw_p_num, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to write ESW port%d, reg%d.",
               __func__, __LINE__, p_num, reg_addr);
        return (FAILED);
    }

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        /* Read current PHY reg. 18_6 out */
        if (tsn_esw_reg_rd(esw_p_num, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d, reg%d.",
                   __func__, __LINE__, p_num, reg_addr);
            return (FAILED);
        }

        if ((reg_val & reg_mask) == exp_val) {
            chk_result = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if (chk_result != PASSED) {
        printf("%s:%d Failed to %s ESW port%d.\n",
               __func__, __LINE__, (opt == ENABLE)?"enable":"disable", p_num);
    }
    return (chk_result);
}

/*******************************************************************************
 *
 * Function   : esw_conf_ext_stub
 * Description: Function to set(enable)/clear(disable) switch PHY reg 18_6.3.
 *              This is needed for external loopback test at 1000base-T.
 * Inputs     : opt - ENABLE/DISABLE.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int esw_conf_ext_stub (int phy_port, boolean opt)
{
    ushort reg_val = 0, exp_val = 0;
    ushort reg_mask = (ushort)ESWPHY_CHKREG_STUB_EN_MSK;
    int reg_page = (int)REG_PAGE(6);
    int reg_addr = (int)PHY_REG(18);
    int reg_bit = (int)ESWPHY_CHKREG_STUB_EN_BIT;
    int chk_result = FAILED;
    struct timeval t_start, t_end;
    double t_diff = 0;

    if (opt == ENABLE) {
        exp_val = (ushort)ESWPHY_CHKREG_STUB_EN;
    }

    /* Read current PHY reg. 18_6 out */
    if (tsn_esw_phy_reg_rd(phy_port, reg_page, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW PHY%d Checker Control reg(%d_%d).\n",
               __func__, __LINE__, phy_port, reg_addr, reg_page);
        return (FAILED);
    }

    if ((reg_val & reg_mask) == exp_val) {
        return (PASSED);
    }

    reg_val &= (ushort)(~reg_mask);
    reg_val |= exp_val;

    if (tsn_esw_phy_reg_wr(phy_port, reg_page, reg_addr, reg_val) != PASSED) {
        printf("%s:%d Failed to write ESW PHY%d Checker Control reg(%d_%d).\n",
               __func__, __LINE__, phy_port, reg_addr, reg_page);
        return (FAILED);
    }

    do {
        gettimeofday(&t_start, NULL);
        reg_val = 0;

        /* Read current PHY reg. 18_6 out */
        if (tsn_esw_phy_reg_rd(phy_port, reg_page,
                               reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW PHY%d "
                   "Checker Control reg(%d_%d).\n",
                   __func__, __LINE__, phy_port, reg_addr, reg_page);
            return (FAILED);
        }

        if ((reg_val & reg_mask) == exp_val) {
            chk_result = PASSED;
            break;
        }

        gettimeofday(&t_end, NULL);
        t_diff += (double)(((t_end.tv_sec - t_start.tv_sec) * SEC_TO_MICROSEC) +
                           (t_end.tv_usec - t_start.tv_usec));
    } while (t_diff < ESW_MAX_POLLINGTIME_USEC); /* Polling time: 5sec. */

    if (chk_result != PASSED) {
        printf("%s:%d Failed to %s ESW PHY%d Enable STUB Test bit(%d_%d.%d).\n",
               __func__, __LINE__, (opt == ENABLE)?"set":"clear", phy_port,
               reg_addr, reg_page, reg_bit);
    }
    return (chk_result);
}


/*******************************************************************************
 *
 * Function   : tsn_esw_wrap_workaround
 * Description: CSCvj11429, Based on Marvell FAE need to apply this workaround
 *              configuration to avoid link stuck. 
 * Inputs     : opt - reserved
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tsn_esw_wrap_workaround (int opt) 
{
    if (tsn_esw_reg_wr(DATA_REG, REG_RSVD, ERRATA_REG_0X1C0) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFC00) != PASSED) {
        goto err_ret; 
    }
   
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFC20) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFC40) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFC60) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFC80) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFCA0) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFCC0) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFCE0) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFD00) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFD20) != PASSED) {
        goto err_ret; 
    }
    
    if (tsn_esw_reg_wr(CMD_REG, REG_RSVD, ERRATA_DBG_CTRL_0XFD40) != PASSED) {
        goto err_ret; 
    }

    return (PASSED);

err_ret:
    printf("%s(%d) Failed to write ESW\n",
          __func__, __LINE__);
    return (FAILED);

}

/******************************************************************************
 * Function: diag_esw_intr_test
 *
 * Description: Function to do Ethernet Switch interrupt test.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED / FAILED
 *****************************************************************************/
int diag_esw_intr_test (int opt)
{
    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "Marvell 88E6176 Ethernet Switch", "Moka FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);
    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Execute SMI Register Test to make sure PHY is accessible",
                    "Check whether the interface between Switch and FPGA is"
                    "damaged or the soldering issue.");
#endif

    int rc = FAILED;

    /* enhance error msg: setting test name */
    char test_name[32] = "ESW Interrupt test";
    testname(test_name);
    prpass(testpass, "%s, ", test_name);
    printf("\n");

    /* 88E6176 Interrupt test*/
    rc = mrvl_88e6176_intr_test();

    if (rc != PASSED) {
        cterr('f', 0, "%s:%d: Interrupt test failed on 88E6176\n", 
              __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    
    /* Re-init ESW. */
    rc = tsn_esw_init();
    if (rc == FAILED) {
        cterr('f', 0, "%s:%d:Fail to re-init ESW device\n", __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
    }

    return (rc);
}

/******************************************************************************
 *
 * Function:    mrvl_88e6176_intr_test
 *
 * Description: Tests the 88E6176 interrupt function.
 *
 * Outputs: PASSED - No errors encounterd.
 *          FAILED - Errors encounterd.
 *
 *****************************************************************************/
static int mrvl_88e6176_intr_test (void)
{
    ushort reg_val, ori_reg_val;
    int smi_addr, reg_addr;
    
    /* read Global 1(0x1b) offset 0x4 original data */
    smi_addr = ESW_SMIDEV_GLOB1;
    reg_addr = GLOBAL_CTRL_REG;
    if (tsn_esw_reg_rd(smi_addr, reg_addr, &ori_reg_val) == FAILED) {
        printf("%s:%d: Failed to read Device Reg with smi_addr:0x%x reg_addr:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr);
        return (FAILED);
    }

    /* de-assert interrupt  */
    if (mrvl_88e6176_deassert_intr() != PASSED) {
        printf("%s:%d: Failed to de-assert interrupt!!\n", __func__, __LINE__);
        return (FAILED);
    }
   
    /* check the interrupt pin is de-asserted */
    if (mrvl_88e6176_chk_intr_deassert() != TRUE) {
        printf("%s:%d: The interrupt pin is not de-asserted!!\n", __func__, __LINE__);
        return (FAILED);
    }
    
    /* assert interrupt */
    if (mrvl_88e6176_assert_intr() != PASSED) {
        printf("%s:%d: Failed to assert interrupt!!\n", __func__, __LINE__);
        return (FAILED);
    }

    /* check the interrupt pin is asserted */
    if (mrvl_88e6176_chk_intr_assert() != TRUE) {
        printf("%s:%d: The interrupt pin is not asserted!!\n", __func__, __LINE__);
        return (FAILED);
    }
    
    /* recover Global 1(0x1b) offset 0x4 with original data */
    reg_val = ori_reg_val;
    if (tsn_esw_reg_wr(smi_addr, reg_addr, reg_val) == FAILED) {
        printf("%s:%d: Failed to write Device Reg with smi_addr:0x%x reg_addr:0x%x data:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr, reg_val);
        return (FAILED);
    }

    /* check Global 1(0x1b) offset 0x4 data is recovered */
    if (tsn_esw_reg_rd(smi_addr, reg_addr, &reg_val) == FAILED) {
        printf("%s:%d: Failed to read Device Reg with smi_addr:0x%x reg_addr:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr);
        return (FAILED);
    }
    if(reg_val != ori_reg_val) {
        printf("%s:%d: Device Reg with smi_addr:0x%x reg_addr:0x%x is not recovered",
               __func__, __LINE__, smi_addr, reg_addr);
        printf("%s:%d: Original data:0x%x, but read data:0x%x\n",
               __func__, __LINE__,ori_reg_val, reg_val);
        return (FAILED);
    }
    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl_88e6176_deassert_intr
 *
 * This function: De-assert ethernet switch interrupt.
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int mrvl_88e6176_deassert_intr (void)
{
    /* In the normal situation, the INTn pin is active low (EEIntEn bit is 1) */
    /* After write 0 to EEIntEn, INTn level (volt) be pulsed. */

    /* write Global 1(0x1b) offset 0x4 with data 0x0, interrupt is asserted */
    int smi_addr = ESW_SMIDEV_GLOB1; 
    int reg_addr = GLOBAL_CTRL_REG;
    ushort reg_val = EEINT_ENABLE;
    if (tsn_esw_reg_wr(smi_addr, reg_addr, reg_val) == FAILED) {
        printf("%s:%d: Failed to write Device Reg with smi_addr:0x%x reg_addr:0x%x data:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr, reg_val);
        return (FAILED);
    }   
    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl_88e6176_assert_intr
 *
 * This function: assert ethernet switch interrupt.
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static int mrvl_88e6176_assert_intr (void)
{
    /* Write 1 to EEIntEn, INTn level (volt) will return to 0 */

    /* write Global 1(0x1b) offset 0x4 with data 0x1, interrupt is de-asserted */
    int smi_addr = ESW_SMIDEV_GLOB1; 
    int reg_addr = GLOBAL_CTRL_REG;
    ushort reg_val = EEINT_DISABLE;
    if (tsn_esw_reg_wr(smi_addr, reg_addr, reg_val) == FAILED) {
        printf("%s:%d: Failed to write Device Reg with smi_addr:0x%x reg_addr:0x%x data:0x%x\n", 
               __func__, __LINE__, smi_addr, reg_addr, reg_val);
        return (FAILED);
    } 
    return (PASSED);
}

/**********************************************************************
 *
 * Function: mrvl_88e6176_chk_intr_deassert
 *
 * This function: Checking the interrupt pin is de-asserted
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
static int mrvl_88e6176_chk_intr_deassert (void)
{
    int rc, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < MRVL88E6176_INTR_POLLING_ROUND; ix++) {
        rc = diag_check_esw_ext_no_intr_pending();
        if (rc == TRUE) {
            break;
        }
        msleep(MRVL88E6176_INTR_POLLING_PERIOD);
    }

    if (rc == FALSE) {
        printf("%s:%d: The interrupt pin is not de-asserted after polling!!\n", __func__, __LINE__);
    }
    
    return (rc);
}


/**********************************************************************
 *
 * Function: mrvl_88e6176_chk_intr_assert
 *
 * This function: Checking the interrupt pin is asserted
 * Output: TRUE/FALSE
 *
 **********************************************************************
 */
static int mrvl_88e6176_chk_intr_assert (void)
{
    int rc, ix;

    /* As Hardware Team's suggestion, while platform running in high/low temp.(EDVT), 
     * the electric reaction of Interrupt Pin might not instantaneous,
     * hence, checking the status of Interrupt Pin by polling. */
    for (ix = 0; ix < MRVL88E6176_INTR_POLLING_ROUND; ix++) {
        rc = diag_check_esw_ext_intr_pending();
        if (rc == TRUE) {
            break;
        }
        msleep(MRVL88E6176_INTR_POLLING_PERIOD);
    }
    
    if (rc == FALSE) {
        printf("%s:%d: The interrupt pin is not asserted after polling!!\n", __func__, __LINE__);
    }
    
    return (rc);
}

/*******************************************************************************
 * Function   : has_mrvl_88e6176
 * Description: Function to check whether this platform has mevl 88e6176 or not
 * Outputs    : TRUE / FALSE
 *
 *******************************************************************************
 */
int has_mrvl_88e6176 (void)
{
    if (this_is_star() || this_is_supernova()) {
        return (TRUE);
    }
    return (FALSE);
}
/*-------------------------------------------------
 * $Log: diag_esw.c,v $
 * Revision 1.12  2019/03/07 09:51:32  lucywang
 * [Supernova] PID changed : C1101L-4P --> C951-4P, C1109L-2P --> C959-2P
 *
 * Revision 1.11  2019/01/24 03:30:48  letsai
 * Update Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.10  2019/01/24 01:07:22  letsai
 * Add Supernova GE0/ESW Interrupt Test (CSCvo04335).
 *
 * Revision 1.9  2019/01/18 05:54:46  yungchen
 * Merge Supernova branch to the main trunk (CSCvn79871)
 *
 * Revision 1.8  2018/05/21 09:00:13  steja
 * Fix these two issues.
 * 1. CSCvj11429 - Found intermittent GE Switch External Loopback
 * 2. CSCvj11436 - Found GE Switch Ext. loopback fail after use permutation test
 *
 * Revision 1.7  2018/05/15 09:37:32  steja
 * CSCvj38863: Enhanced LED single test utility
 *
 * Revision 1.6  2018/04/15 22:03:30  palin2
 * Merged Vulcan back to maintrunk.
 *
 * Revision 1.5  2018/02/09 09:56:53  hondwang
 * Merge Star branch star-branch-c9xx to main trunk
 *
 * Revision 1.4  2018/01/23 11:38:18  steja
 * Merge tsn-gfast-branch4 code to maintrunk for support TSN-G.Fast (CSCvh40981)
 *
 * Revision 1.3.4.1  2018/01/23 09:56:53  palin2
 * Enhanced code readability.
 *
 * Revision 1.3  2017/12/01 13:50:34  palin2
 * Fixed CSCvg97205: Added force Switch MAC and PHY speed function back to external loopback test.
 * Sync from TSN trunk : Added force Switch MAC and PHY speed function back to external loopback test(CSCvg97205).
 *
 * Revision 1.2.4.5  2017/11/20 07:54:31  lucywang
 * Changed PID to C1101/C1109-2P/C1109-4P
 *
 * Revision 1.2.4.4  2017/09/28 21:44:42  hondwang
 * add internal SMI bus busy check function to replace delay timing
 *
 * Revision 1.2.4.3  2017/09/15 02:58:11  lucywang
 * ignore port 6(WIFI) setting of 88E6176 to avoid switch external loopback failure on C949-2P
 *
 * Revision 1.2.4.2  2017/08/30 11:40:17  lucywang
 * only test 2 ports of 88E6176 on C909
 *
 * Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
 * star branch c9xx initial check in
 *
 * Revision 1.2  2017/08/02 14:21:44  steja
 * Support TSN-H/M platform code
 *
 * Revision 1.1.8.2  2017/07/29 03:41:02  steja
 * tsn-branch5 synced with Maintrunk repositories
 *
 * Revision 1.1.6.4  2017/07/24 14:14:10  palin2
 * 1. To improve code readability.
 * 2. All changes are verified before check-in.
 *
 * Revision 1.1.6.3  2017/07/21 06:06:23  palin2
 * Code clean up.
 *
 * Revision 1.1.6.2  2017/07/20 13:38:03  steja
 * tsn-branch4 merge with maintrunk
 *
 * Revision 1.1.4.12.2.2  2017/07/17 13:54:44  palin2
 * Code cleanup.
 *
 * Revision 1.1.4.12.2.1  2017/07/12 12:23:08  palin2
 * Added Ext. Loopback Flag support.
 *
 * Revision 1.1.4.12  2016/11/01 07:29:18  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.11  2016/10/04 06:39:07  petteng
 * Add enhanced error message
 *
 * Revision 1.1.4.10  2016/09/28 04:36:14  palin2
 * Added CPU to ESW PHY MAC loopback test.
 *
 * Revision 1.1.4.9  2016/09/13 13:56:39  palin2
 * Fixed compiler issue.
 *
 * Revision 1.1.4.8  2016/07/29 14:27:47  palin2
 * Added utility and function to config. Switch port to specific speed and mode.
 *
 * Revision 1.1.4.7  2016/07/17 10:52:56  palin2
 * 1. Added function and utility to set GE WAN PHY Transmitter Type.
 * 2. Clean up code.
 *
 * Revision 1.1.4.6  2016/07/06 05:17:58  palin2
 * Enhanced Switch init function.
 *
 * Revision 1.1.4.5  2016/07/05 14:26:51  palin2
 * Added utililty to force ON/OFF TSN Switch port LED(s).
 *
 * Revision 1.1.4.4  2016/07/04 15:29:28  palin2
 * 1. Updated TSN-M Switch part related code after bring up.
 * 2. Added utility to change LAN PHY port VOD setup for HW.
 *
 * Revision 1.1.4.3  2016/06/30 14:06:31  steja
 * Pick up the latest from tsn-branch1
 *
 * Revision 1.1.4.2  2016/06/30 06:22:47  steja
 * tsn-branch2 sync with main trunk
 * 
 * Revision 1.1.2.6  2016/06/29 14:14:51  palin2
 * 1. Updated code to support TSN-M.
 * 2. Added utility to set LAN PHY 1000Base-T Test mode.
 *
 * Revision 1.1.2.5  2016/05/26 10:17:38  palin2
 * Optimise SMI read write function and Switch init function.
 *
 * Revision 1.1.2.4  2016/05/26 03:09:22  palin2
 * Added TSN Switch init function, and SMI C45 read/write utility.
 *
 * Revision 1.1.2.3  2016/05/24 01:20:11  palin2
 * Updated GE Switch and PHY utilities.
 *
 * Revision 1.1.2.2  2016/05/03 16:00:57  palin2
 * 1. Added Switch register test.
 * 2. Added Switch external loopback test to support 10 and 100Mbps speed.
 *
 * Revision 1.1.2.1  2016/04/29 10:14:56  palin2
 * Updated code and added support ext. loopback test after bring up Switch.
 *
 * $Endlog$
 *-------------------------------------------------
 */

