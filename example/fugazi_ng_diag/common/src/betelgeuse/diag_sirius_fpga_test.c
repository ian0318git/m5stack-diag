/* $Id: diag_sirius_fpga_test.c,v 1.2 2019/01/10 06:36:24 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_sirius_fpga_test.c,v $
 *------------------------------------------------------------------
 * 
 * diag_sirius_fpga_test.c
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

#include "diag_sirius_fpga_test.h"
#include "diag_sirius_fpga_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_enhance_err_msg_lib.h"
#include "platform_cookie.h"
#include "diag_temp_sensor_util.h"
#include <string.h>
#include "diag_cpu_lib.h"

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
static int diag_plug_fpga_reg_test(int);
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
   {"FPGA Utility",                (type_t(*)())diag_moka_fpga_util,       FALSE,
    0,
    (type_t(*)())0,                0,
    (type_t(*)())diag_moka_fpga_util,   TRUE}, 
   {"Pluggable FPGA Register Test",          (type_t(*)())diag_plug_fpga_reg_test, 0,
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
 * Function    : diag_plug_fpga_test
 * Description : Function performs the Pluggable FPGA diag submenu/tests.
 * Inputs      : opt - option to determine to show menu or not
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_plug_fpga_test (int opt)
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
        printf("%s: Failed to readPluggable FPGA Reg(0x%lx).\n",
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
 * $Log: diag_sirius_fpga_test.c,v $
 * Revision 1.2  2019/01/10 06:36:24  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
