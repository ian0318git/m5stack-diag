/* $Id: diag_plug_fpga.c,v 1.3 2018/11/23 08:49:51 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_plug_fpga.c,v $
 *-----------------------------------------------------------------------------
 *
 * Filename   : diag_plug_fpga.c
 * Description: Pluggable FPGA Diag tests and utilities.
 *
 * Copyright (c) 2016 ~ 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
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
#include "defs.h"
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
#include "nvmonvars.h"

#include "tsn_comm.h"
#include "diag_plug_fpga.h"
#include "plug_host_fpga_lib.h"
#include "platform_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "platform_sensor.h"
#include <string.h>

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
#define ENHANCE_ERROR_MSG_RDY 1
#define PLUG_FPGA_RONLY                             (READ_ONLY | REG_ACCESS)
#define PLUG_FPGA_RW                                (READ_WRITE | REG_ACCESS)

/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
int diag_plug_fpga(int);

static int diag_plug_fpga_reg_test(int);
static int diag_plug_fpga_f_intr_test(int);
static int plug_fpga_reg_test_write_fn(ulong, int, ulong, void *);
static int plug_fpga_reg_test_read_fn(ulong, int, ulong *, void *);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */
/*
 * Pluggable FPGA Diag tests SubMenu
 */
static submenu_xtable_t plug_fpga_diag_tbl[] = {
   {"FPGA Utility",                (type_t(*)())tsn_fpga_utils,       FALSE,
    0,
    (type_t(*)())0,                0,
    (type_t(*)())tsn_fpga_utils,   TRUE}, 
   {"Pluggable FPGA Register Test",          (type_t(*)())diag_plug_fpga_reg_test, 0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
    (type_t(*)())0,                0,
    (type_t(*)())0,                0},
   {"Pluggable FPGA Force Interrupt Test",  (type_t(*)())diag_plug_fpga_f_intr_test, 0,
    (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
    (type_t(*)())0,                0,
    (type_t(*)())0,                0},
};

#define PLUG_FPGA_DIAG_TBL_SIZE (sizeof(plug_fpga_diag_tbl) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t plug_fpga_diag_pri_items[PLUG_FPGA_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t plug_fpga_diag_sec_items[PLUG_FPGA_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

menuinfo_t plug_fpga_diag_menu = {
    "%s Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    plug_fpga_diag_pri_items,
};
menuinfo_t *plug_fpga_diag_menu_p = &plug_fpga_diag_menu;

static reg_info_t_ext plug_fpga_reg_ext = {PLUG_FPGA_REG_WIDTH,
                                               plug_fpga_reg_test_read_fn,
                                               plug_fpga_reg_test_write_fn,
                                               0};
/*
 * FPGA register test
 */
static reg_info_t plug_fpga_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Scratchpad Register",      PLUG_FPGA_SCRATCHPAD_OFFSET,   PLUG_FPGA_RW,
     {(unsigned long)&plug_fpga_reg_ext},   0xFFFFFFFF,          0x0},
    {"END",                                0x00,                     0,
     {0},                                  0x0,                      0x0},
};


/*******************************************************************************
 * Function    : diag_plug_fpga
 * Description : Function performs the Pluggable FPGA diag submenu/tests.
 * Inputs      : opt - option to determine to show menu or not
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_plug_fpga (int opt)
{
    build_primary_submenu(plug_fpga_diag_tbl, PLUG_FPGA_DIAG_TBL_SIZE,
                          "Pluggable FPGA", &plug_fpga_diag_menu_p);
    build_secondary_submenu(plug_fpga_diag_tbl, PLUG_FPGA_DIAG_TBL_SIZE,
                            plug_fpga_diag_sec_items);

    if (opt) {
        menu(plug_fpga_diag_menu_p, plug_fpga_diag_sec_items, '\0' );
    } else {
        menu_exec_doall_diags(plug_fpga_diag_menu_p);
    }
    return (PASSED);
}


/*******************************************************************************
 * Function    : diag_plug_fpga_reg_test
 * Description : Function performs the Pluggable FPGA register test.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_plug_fpga_reg_test (int opt)
{
    char *tname ="Pluggable FPGA Register";

    testname(tname);
    prpass(testpass, "%s, ", tname);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        cterr('f', 0, "test - show enhanced error messages");
    }

    if (register_tests(0, plug_fpga_reg_test_tbl) != PASSED) {
        cterr('f', 0, "Pluggable FPGA Register test Failed");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}



/*******************************************************************************
 * Function    : diag_plug_fpga_f_intr_test
 * Description : Function performs the Pluggable FPGA interrupt test.
 * Inputs      : opt - reserved for future use
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
static int diag_plug_fpga_f_intr_test (int opt)
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
    cterr_add_component("Marvell Armada 7040", "Local Bus", "Sirius FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)tsn_display_temp_errormsg);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Make sure the pending status of corresponding item "
                    "has been reset.",
                    "Make sure the corresponding interrupt register "
                    "has been triggered.",
                    "Make sure CPU receives the interrupt signal.");
#endif
    uint  cpu_intr = 0, cpu_mpp = 0, gpio_dout_en = 0;
    uint  gpio_din_polar = 0, gpio_din = 0;
    uint  intr_stat = 0;

    char *tname ="Plug FPGA Force Interrupt";
    testname(tname);

    prpass(testpass, "%s, ", tname);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        cterr('f', 0, "test - show enhanced error messages");
    }

    /* Confirmed that CP_MPP50 is configured as GPIO_IN */
    if (tsn_mem_read32((uint)CP_MPP_CTRL_REG(6), &cpu_mpp) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side MPP info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((cpu_mpp & 0xf00) != 0) {
        cpu_mpp &= (uint)(~0xf00);
        if (tsn_mem_write32((uint)CP_MPP_CTRL_REG(6), cpu_mpp) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU MPP.", __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    if (tsn_mem_read32((uint)CP_GPIO_DATA_OUT_EN_REG(1),
                       &gpio_dout_en) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data OUT enable info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((gpio_dout_en & (uint)(1 << 18)) != 1) {
        gpio_dout_en |= (uint)(1 << 18);
        if (tsn_mem_write32((uint)CP_GPIO_DATA_OUT_EN_REG(1),
                            gpio_dout_en) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU GPIO Data OUT enable.",
                          __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }

    if (tsn_mem_read32((uint)CP_GPIO_DATA_IN_POLAR_REG(1),
                       &gpio_din_polar) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data IN Polarity info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((gpio_din_polar & (uint)(1 << 18)) != 1) {
        gpio_din_polar |= (uint)(1 << 18);
        if (tsn_mem_write32((uint)CP_GPIO_DATA_IN_POLAR_REG(1),
                            gpio_din_polar) != PASSED) {
            cterr('f', 0, "%s: Failed to set CPU GPIO Data IN Polarity.",
                          __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
        }
    }
    
    /* Need to clear I2C pendenting interrupt before testing */
    if (fpga_read_32_reg((uint)PLUG_INTR_STAT_REG,
                         &intr_stat) != PASSED) {
        cterr('f', 0, "%s: Failed to read Sirius FPGA "
                      "ext. interrupt status Reg(0x%04X).",
                      __FUNCTION__, PLUG_INTR_STAT_REG);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }
    if ((intr_stat & PLUG_INTR_STAT_I2C_BIT) != 0) {
         if (fpga_write_32_reg((uint)PLUG_I2C_INTR_OVRI_REG,
                           (uint)(PLUG_I2C_INTR_OVRI_CLEAN)) != PASSED) {
                cterr('f', 0, "Failed to clean plug FPGA pending interrupt I2C override Reg."
                          , __FUNCTION__);
                prcomplete(testpass, errcount, (char *)0);
                return (FAILED);
         }
    }

    /* Check CPU side */
    if (tsn_mem_read32((uint)CP_GPIO_DATA_IN_REG(1),
                       &gpio_din) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side GPIO Data IN info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((gpio_din & (uint)(1 << 18)) != 0) {
        cterr('f', 0, "%s: Failed !! CPU detected pending interrupt. gpio_in = 0x%80x.\n",
                      __FUNCTION__,gpio_din);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }


    /* Use I2C 0 interrupt to verify Sirius FPGA to CPU interrupt */
    
    /* Enable Interrupt Enable Reg for I2C */
    if (fpga_write_32_reg((uint)PLUG_INTR_ENA_REG,
                          (uint)(PLUG_INTR_ENA_I2C_BIT)) != PASSED) {
        cterr('f', 0, "%s:Failed to set Plug FPGA enable I2C interrupt Reg.",
                      __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }


    /* Enable I2C Interrupt Enable Reg */
    if (fpga_write_32_reg((uint)PLUG_I2C_INTR_ENA_REG,
                          (uint)(PLUG_I2C0_INTR_ENA_BIT)) != PASSED) {
        cterr('f', 0, "%s:Failed to set Plug FPGA interrupt I2C enable Reg.",
                      __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }

    /* Trigger force interrupt from plug FPGA */
    /* Enable I2C Interrupt Override Reg */
    if (fpga_write_32_reg((uint)PLUG_I2C_INTR_OVRI_REG,
                          (uint)(PLUG_I2C0_INTR_OVRI_BIT)) != PASSED) {
        cterr('f', 0, "%s:Failed to set Plug FPGA interrupt I2C override Reg.",
                      __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }

     msleep(PLUG_INT_ACT_WAIT);

     /* Check if CPU get interrupt from FPGA */
     if (tsn_mem_read32((uint)CP_GPIO_DATA_IN_REG(1), &cpu_intr) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side interrupt info.",
                      __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }

     if ((cpu_intr & TSN_PLUG_FPGA_CPU_INTR) != TSN_PLUG_FPGA_CPU_INTR) {
        cterr('f', 0, "Failed, CPU didn't get interrupt from PLUG FPGA.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }
 
     /* Turn off force interrupt from plug FPGA */
     if (fpga_write_32_reg((uint)PLUG_I2C_INTR_OVRI_REG,
                           (uint)(PLUG_I2C_INTR_OVRI_CLEAN)) != PASSED) {
            cterr('f', 0, "Failed to turn off plug FPGA interrupt I2C override Reg."
                          , __FUNCTION__);
            prcomplete(testpass, errcount, (char *)0);
            return (FAILED);
     }

    /* Turn off I2C Interrupt Enable Reg */
    if (fpga_write_32_reg((uint)PLUG_I2C_INTR_ENA_REG,
                          (uint)(PLUG_I2C_INTR_OVRI_CLEAN)) != PASSED) {
        cterr('f', 0, "%s:Failed to turn off Plug FPGA I2C Interrupt Enable Reg.",
                      __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }
     /* Turn off Interrupt Enable Reg for I2C */
    if (fpga_write_32_reg((uint)PLUG_INTR_ENA_REG,
                          (uint)(PLUG_I2C_INTR_OVRI_CLEAN)) != PASSED) {
        cterr('f', 0, "%s:Failed to turn off Plug FPGA Interrupt Enable Reg for I2C.",
                      __FUNCTION__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
     }

     msleep(PLUG_INT_ACT_WAIT);

     /* Confirm that no interrupt from plug FPGA to CPU */
    if (tsn_mem_read32((uint)CP_GPIO_DATA_IN_REG(1), &cpu_intr) != PASSED) {
        cterr('f', 0, "%s:%d Failed to check CPU side interrupt info.",
                          __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    if ((cpu_intr & TSN_PLUG_FPGA_CPU_INTR) == TSN_PLUG_FPGA_CPU_INTR) {
        cterr('f', 0, "Failed, CPU got unexpected interrupt from PLUG FPGA.");
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_reg_test_read_fn
 * Description: Pluggable FPGA register read function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              *buf   - pointer to read buffer
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_reg_test_read_fn (ulong addr, int size, ulong *buf, void *param)
{
    if (plug_fpga_reg_read((uint)addr, (uint *)buf) != PASSED) {
        printf("%s: Failed to read TSN Pluggable FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : plug_fpga_reg_test_write_fn
 * Description: Pluggable FPGA register write function for register test.
 * Inputs     : addr   - FPGA register offset
 *              size   - FPGA register size
 *              data   - write in data
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
static int plug_fpga_reg_test_write_fn (ulong addr, int size, ulong data, void *param)
{
    if (plug_fpga_reg_write((uint)addr, (uint)data) != PASSED) {
        printf("%s: Failed to write Pluggable FPGA Reg(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*-------------------------------------------------
$Log: diag_plug_fpga.c,v $
Revision 1.3  2018/11/23 08:49:51  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.52.1  2018/10/15 06:53:07  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/02/09 09:56:56  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.1.6.2  2018/01/20 05:57:48  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.5  2017/10/24 02:55:42  iachang
Turn off I2C interrupt reg when Pluggable FPGA interrupt test completed.

Revision 1.1.4.4  2017/09/28 21:46:11  hondwang
Add Moka and Sirius FPGA interrupt 1ms(HW suggest) wait

Revision 1.1.4.3  2017/09/21 19:30:17  hondwang
Poweroff pluggable module before testing

Revision 1.1.4.2  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.1.2.4  2017/06/22 19:27:09  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

