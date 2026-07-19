/* $Id: diag_moka_fpga_test.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_moka_fpga_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_moka_fpga_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "common_utils.h"
#include "queryflags.h"
#include "menu.h"
#include <stdio.h>
#include "proto.h"
#include <fcntl.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "diag_moka_fpga_lib.h"
#include "nvmonvars.h"
#include <string.h>
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include "diag_moka_fpga_test.h"
#include "diag_cpu_lib.h"


/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int diag_aikido_reg_test (void);

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int diag_fpga_reg_test(int);
int diag_fpga_f_intr_test(int);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
/*
 * FPGA Diag tests SubMenu
 */
static submenu_xtable_t fpga_diag_tbl[] = {
   {"FPGA Utility",                (type_t(*)())diag_moka_fpga_util,       FALSE,
    0,
    (type_t(*)())0,                0,
    (type_t(*)())diag_moka_fpga_util,   TRUE}, 
   {"FPGA Register Test",          (type_t(*)())diag_fpga_reg_test,   0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
    (type_t(*)())0,                0,
    (type_t(*)())0,                0},
   {"Aikido Register Test",          (type_t(*)())diag_aikido_reg_test,   0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
    (type_t(*)())0,                0,
    (type_t(*)())0,                0},
   {"FPGA Force Interrupt Test",  (type_t(*)())diag_fpga_f_intr_test, 0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
    (type_t(*)())0,                0,
    (type_t(*)())0,                0},
};

#define FPGA_DIAG_TBL_SIZE (sizeof(fpga_diag_tbl) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t fpga_diag_pri_items[FPGA_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t fpga_diag_sec_items[FPGA_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t fpga_diag_menu = {
    "%s Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    fpga_diag_pri_items,
};
menuinfo_t *fpga_diag_menu_p = &fpga_diag_menu;

/*
 * FPGA register test
 */
static reg_info_t fpga_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Force External Interrupt Mask",      FPGA_EXT_INTR_MASK_REG,   FPGA_RW,
     {(unsigned long)&plat_fpga_reg_ext},   0x000FFBFF,               0x0},
    {"END",                                0x00,                     0,
     {0},                                  0x0,                      0x0},
};

static plat_reg_bit_t fpga_f_intr_test_tbl[] = {
    {"xDSL Dying Gasp(DG_FPGA_INT_L)",      0},
    {"CONS UART0(CONS_UART0_DSR_L)",        1},
    {"Management GE PHY(MGMT_FPGA_INT_L)",  2},
    {"PoE(POEDC_FPGA_INT_L)",               3},
    {"xDSL(DSL_FPGA_INT_L)",                4},
    {"WAN GE1 PHY(GEWAN1_FPGA_INT_L)",      5},
    {"GE Switch(ESW_FPGA_INT_L)",           6},
    {"WAN GE0 PHY(GEWAN0_FPGA_INT_L)",      7},
    {"SIM CARD 1 detect(SIM1_FPGA_CD)",     8},
    {"SIM CARD 0 detect(SIM0_FPGA_CD)",     9},
    {"SFP Loss of Signal(SFP_0_FPGA_LOS)", 11},
    {"SFP TX Fault(SFP_0_FPGA_TX_FAULT)",  12},
    {"Thermal sensor(THEM_FPGA_INT_L)",    13},
    {"SPI Controller",                     14},
    {"I2C Controller",                     15},
    {"(USB_SW_FPGA_OC)",                   16},
};

/*******************************************************************************
 *
 * Function    : diag_fpga_test
 * Description : Function performs the FPGA diag submenu/tests.
 * Inputs      : opt - option to determine to show menu or not
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_fpga_test (int opt)
{
    build_primary_submenu(fpga_diag_tbl, FPGA_DIAG_TBL_SIZE,
                          "FPGA", &fpga_diag_menu_p);
    build_secondary_submenu(fpga_diag_tbl, FPGA_DIAG_TBL_SIZE,
                            fpga_diag_sec_items);

    if (opt) {
        menu(fpga_diag_menu_p, fpga_diag_sec_items, '\0' );
    } else {
        menu_exec_doall_diags(fpga_diag_menu_p);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_fpga_reg_test
 * Description : Function performs the FPGA register test.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_fpga_reg_test (int opt)
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
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Make sure the target registers are not \"read only\".",
                    "If step a. is OK, "
                    "check the local bus interface between MB and CPLD.");
#endif

    char *tname ="FPGA Register";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    if (register_tests(0, fpga_reg_test_tbl) != PASSED) {
        cterr('f', 0, "FPGA Register test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_fpga_f_intr_test
 * Description: Function performs the FPGA force interrupt test.
 * Inputs     : opt - reserved for future use.
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_fpga_f_intr_test (int opt)
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
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Make sure the pending status of corresponding item "
                    "has been reset.",
                    "Make sure the corresponding interrupt register "
                    "has been triggered.",
                    "Make sure CPU receives the interrupt signal.");
#endif

    uint          pending_intr = 0, test_val = 0, intr_mask_orig = 0;
    uint          cpu_intr = 0, cpu_mpp = 0, gpio_dout_en = 0;
    uint          gpio_din_polar = 0, gpio_din = 0;
    plat_reg_bit_t *reg_p = 0;
    int           ctr = 0, total_f_intr_num = 0;

    char *tname ="FPGA Force Interrupt";
    testname(tname);
    prpass(testpass, "%s, ", tname);

    reg_p = &fpga_f_intr_test_tbl[0];
    total_f_intr_num = (sizeof(fpga_f_intr_test_tbl) / sizeof(plat_reg_bit_t));

    /* Confirmed that CP_MPP8 is configured as GPIO_IN */
    if (plat_mem_read32((uint)CP_MPP_CTRL_REG(1), &cpu_mpp) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side MPP info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((cpu_mpp & 0xf) != 0) {
        cpu_mpp &= (uint)(~0xf);
        if (plat_mem_write32((uint)CP_MPP_CTRL_REG(1), cpu_mpp) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU MPP.", __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    if (plat_mem_read32((uint)CP_GPIO_DATA_OUT_EN_REG(0),
                       &gpio_dout_en) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data OUT enable info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((gpio_dout_en & (uint)(1 << 8)) != 1) {
        gpio_dout_en |= (uint)(1 << 8);
        if (plat_mem_write32((uint)CP_GPIO_DATA_OUT_EN_REG(0),
                            gpio_dout_en) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU GPIO Data OUT enable.",
                          __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    if (plat_mem_read32((uint)CP_GPIO_DATA_IN_POLAR_REG(0),
                       &gpio_din_polar) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data IN Polarity info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((gpio_din_polar & (uint)(1 << 8)) != 1) {
        gpio_din_polar |= (uint)(1 << 8);
        if (plat_mem_write32((uint)CP_GPIO_DATA_IN_POLAR_REG(0),
                            gpio_din_polar) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU GPIO Data IN Polarity.",
                          __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    /* Get FPGA external interrupt pending info */
    if (fpga_read_32_reg((uint)FPGA_EXTER_INT_PENDING_REG,
                         &pending_intr) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA "
                      "ext. interrupt pending Reg(0x%04X).",
                      __FUNCTION__, FPGA_EXTER_INT_PENDING_REG);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if (fpga_read_32_reg((uint)FPGA_EXT_INTR_MASK_REG,
                         &intr_mask_orig) != PASSED) {
        cterr('f', 0, "%s: Failed to read FPGA ext. interrupt mask Reg(0x%04X).",
                      __FUNCTION__, FPGA_EXT_INTR_MASK_REG);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if (fpga_write_32_reg((uint)FPGA_EXT_INTR_MASK_REG,
                          pending_intr) != PASSED) {
        cterr('f', 0, "%s: Failed to set FPGA ext. interrupt mask Reg(0x%04X).",
                      __FUNCTION__, FPGA_EXT_INTR_MASK_REG);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* Check CPU side */
    if (plat_mem_read32((uint)CP_GPIO_DATA_IN_REG(0),
                       &gpio_din) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data IN info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((gpio_din & (uint)(1 << 8)) != 0) {
        cterr('f', 0, "%s: Failed !! CPU detected pending interrupt.",
                      __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    /* FPGA force interrupt test */
    for (ctr = 0; ctr < total_f_intr_num; ctr++, reg_p++) {
        test_val = (uint)(1 << reg_p->offset);

        /* Skip interrupt pending bit(s) */
        if (((pending_intr & test_val) == test_val) ||
            (test_val = 0x400)) {
            continue;
        }

        if (fpga_write_32_reg((uint)FPGA_EXT_INTR_MASK_REG,
                              (uint)(~test_val)) != PASSED) {
            cterr('f', 0, "%s:Failed to set FPGA ext. interrupt mask Reg."
                          " for %s(%d).",
                          __FUNCTION__, reg_p->name, reg_p->offset);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        /* Trigger force interrupt from FPGA */
        if (fpga_write_32_reg((uint)FPGA_EXTER_INT_PENDING_REG,
                              test_val) != PASSED) {
            cterr('f', 0, "Failed to trigger FPGA %s(%d) force interrupt"
                          " by accessing Reg(0x%04X).",
                          reg_p->name, reg_p->offset, FPGA_FORCE_EXT_INTR_REG);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        msleep(MOKA_INT_ACT_WAIT);

        /* Check if CPU get interrupt from FPGA */
        if (plat_mem_read32((uint)CP_GPIO_DATA_IN_REG(0), &cpu_intr) != PASSED) {
            cterr('f', 0, "%s:%d Failed to check CPU side interrupt info.",
                          __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        if ((cpu_intr & PLAT_FPGA_CPU_INTR) != PLAT_FPGA_CPU_INTR) {
            cterr('f', 0, "Failed, CPU didn't get %s(%d) interrupt from FPGA.",
                          reg_p->name, reg_p->offset);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        /* Turn off force interrupt from FPGA */
        if (fpga_write_32_reg((uint)FPGA_EXTER_INT_PENDING_REG,
                              (uint)(~test_val)) != PASSED) {
            cterr('f', 0, "Failed to turn off FPGA %s(%d) force interrupt"
                          " by accessing Reg(0x%04X).",
                          reg_p->name,reg_p->offset, FPGA_FORCE_EXT_INTR_REG);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        msleep(MOKA_INT_ACT_WAIT);

        /* Confirm that no interrupt from FPGA to CPU */
        if (plat_mem_read32((uint)CP_GPIO_DATA_IN_REG(0), &cpu_intr) != PASSED) {
            cterr('f', 0, "%s:%d Failed to check CPU side interrupt info.",
                          __FUNCTION__, __LINE__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }

        if ((cpu_intr & PLAT_FPGA_CPU_INTR) == PLAT_FPGA_CPU_INTR) {
            cterr('f', 0, "Failed, CPU got unexpected interrupt from FPGA.");
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    if (fpga_write_32_reg((uint)FPGA_EXT_INTR_MASK_REG, intr_mask_orig) != PASSED) {
        cterr('f', 0, "%s: Failed to recover FPGA ext. interrupt mask Reg(0x%04X).",
                      __FUNCTION__, FPGA_EXT_INTR_MASK_REG);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*-------------------------------------------------
 * $Log: diag_moka_fpga_test.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
