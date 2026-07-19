/* $Id: diag_poe_psu.c,v 1.3 2017/10/19 13:41:11 palin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_poe_psu.c,v $
 *------------------------------------------------------------------
 *
 * Filename   : diag_poe_psu.c
 * Description: TSN PoE PSU Diag tests and utilities.
 *
 * Copyright (c) 2016 ~ 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "menu.h"
#include "error.h"
#include "cli_cmd.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "common_utils.h"
#include "platform_cookie.h"
#include "platform_fpga.h"
#include "plat_defs.h"
#include "platform_fru.h"
#include "platform_poe_psu.h"
#include "tsn_comm.h"
#include "diag_poe_psu.h"


/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void       diag_poe_psu(int);
int        diag_psu_reg_test(int);
int        diag_psu_intr_test(int);
static int psu_regtest_rd_fn(ulong, int, ulong *, void *);
static int psu_regtest_wr_fn(ulong, int, ulong, void *);

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */
static int            pse_port_num = UNDEFINED_POE_PORTS;
static reg_info_t_ext psu_reg_ext = {TSN_PSU_REG_WIDTH,
                                     psu_regtest_rd_fn,
                                     psu_regtest_wr_fn,
                                     0};

/* Registers test table */
static reg_info_t psu_reg_test_tbl[] = {
    /* Format: NAME, OFFSET, TYPE, SIZE, MASK, RESET_VAL. */
    {"Interrupt Mask",            TPS2386B_INTR_MASK_REG,
     (READ_WRITE | REG_ACCESS),   {(unsigned long)&psu_reg_ext},
     0xFF,                        0x80},
    {"Operating Mode",            TPS2386B_OP_MODE_REG,
     (READ_WRITE | REG_ACCESS),   {(unsigned long)&psu_reg_ext},
     0xFF,                        0x00},
    {"END",                       0x00,
     0x0,                         {0},
     0x0,                         0x0},
};


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * PoE PSU Diag Menu
 */
static submenu_xtable_t psu_diag_tbl[] = {
    {"PoE PSU utilities",          (type_t(*)())plat_psu_utils,     TRUE,
     0,                            
     (type_t(*)())0,               0,
     (type_t(*)())plat_psu_utils,  FALSE},
    {"PoE PSU register test",      (type_t(*)())diag_psu_reg_test,  TRUE,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"PSU to FPGA interrupt test", (type_t(*)())diag_psu_intr_test, TRUE,
     (MF_CONTINUOUS | MF_DOGRP | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
};

#define PSU_DIAG_TBL_SIZE (sizeof(psu_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t psu_diag_pri_items[PSU_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t psu_diag_sec_items[PSU_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo psu_diag = {
    "%s Menu",               /* title */
    0,                       /* title string added by init_empty_menu */
    0,                       /* do not show major flags */
    0,                       /* generic prompt */
    0,                       /* size -- bumped by add_menu_item() */
    psu_diag_pri_items,
};

static struct menuinfo *psu_diag_p = &psu_diag;


/*******************************************************************************
 *
 * Function   : diag_poe_psu
 * Description:	Function performs PoE PSU Diag tests or utilities.
 * Inputs     : opt - Option to determine to run Diag tests / show submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_poe_psu (int opt)
{
    /* Build Menu */
    build_primary_submenu(psu_diag_tbl, PSU_DIAG_TBL_SIZE,
			  "PoE PSU", &psu_diag_p);
    build_secondary_submenu(psu_diag_tbl, PSU_DIAG_TBL_SIZE,
			    psu_diag_sec_items);

    if (opt) {
        menu_exec_doall_diags(psu_diag_p);
    } else {
        /* Entered with submenu */
        menu(&psu_diag, psu_diag_sec_items, 0);
    }
}

/*******************************************************************************
 *
 * Function   : psu_regtest_rd_fn
 * Description: PoE PSU controller register read function for register test.
 * Inputs     : addr   - PSU register offset
 *              size   - PSU register size
 *              *buf   - pointer to read buffer
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
static int psu_regtest_rd_fn (ulong addr, int size, ulong *buf, void *param)
{
    if (tsn_psu_reg_rd((uint32_t)addr, (char *)buf) != PASSED) {
        printf("%s: Failed to read TSN PoE PSU reg.(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : psu_regtest_wr_fn
 * Description: PoE PSU controller register write function for register test.
 * Inputs     : addr   - PSU register offset
 *              size   - PSU register size
 *              data   - write in data
 *              *param - pointer to param
 * Outputs    : PASSED / FAILED
 *
 *******************************************************************************
 */
static int psu_regtest_wr_fn (ulong addr, int size, ulong data, void *param)
{
    if (tsn_psu_reg_wr((uint32_t)addr, (char)data) != PASSED) {
        printf("%s: Failed to write TSN PoE PSU reg.(0x%lx).\n",
               __FUNCTION__, addr);
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : diag_psu_reg_test
 * Description: Function performs PoE PSU register test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_psu_reg_test (int opt)
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
    cterr_add_component("Marvell Armada 88F7040", "TI 2386B", "DDR RAM");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Confirm related reigstess are read- and writeable by "
                    "using utility to access them manually. "
                    "Swap PoE module with Golden sample. "
                    "Check if I2C interface between CPU and PoE PSU controler"
                    " is good.");
#endif

    char * tname = "Registers";
    testname(tname);

    if (register_tests(0, psu_reg_test_tbl) != PASSED) {
        cterr('f', 0, "Failed at PoE PSU");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_has_poe
 * Description: Function to check if this TSN has PoE feature.
 * Inputs     : opt - reserve for future use
 * Outputs    : TRUE(yes)/FALSE(no)
 *
 *******************************************************************************
 */
boolean tsn_has_poe (int opt)
{
    uint reg_offset = 0, reg_val = 0;
    
    reg_offset = (uint)FPGA_CARD_AND_PWR_REG;
    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA Card and Power Present reg."
               "(0x%04X)\n", __FUNCTION__, reg_offset);
        return (FALSE);
    }
 
    /* On TSN series, PoE module is option for customer.
     * So determine to show PoE related diags by check if PoE module is present.
     */
    if ((reg_val & FPGA_CPP_POE_PRESENT) != FPGA_CPP_POE_PRESENT) {
        return (FALSE);
    }
    return (TRUE);
}

/*******************************************************************************
 *
 * Function   : diag_psu_intr_test
 * Description: Function performs PoE PSU interrupt test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int diag_psu_intr_test (int opt)
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
    cterr_add_component("Marvell Armada 88F7040", "TI 2386B", "FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("Confirm related reigstess are read- and writeable by "
                    "using utility to access them manually. "
                    "Swap PoE module with Golden sample. "
                    "Check if I2C interface between CPU and PoE PSU controler"
                    " is good. "
                    "Check if interrupt path between FPGA and PoE PSU controler"
                    " is good.");
#endif

    uint32_t psu_reg_offset = 0;
    uchar    psu_reg_val = 0, psu_reg_wr = 0, config_val = 0;
    uchar    psu_def_intr_mask = 0;
    char     *tname = "PSE interrupt";
    uint     fpga_reg_offset = (uint)FPGA_EXTER_INT_PENDING_REG;
    uint     fpga_reg_val = 0;
    int      p_ctr = 0;

    testname(tname);

    /* Confirm expected PSE port number(TSN-H:4, TSN-M:2) */
    if (pse_port_num == UNDEFINED_POE_PORTS) {
        if (this_is_tsn_h_sku() == TRUE) {
            pse_port_num = TSN_H_POE_PORTS;
        } else {
            pse_port_num = TSN_M_POE_PORTS;
        }
    }
 
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d (%s)pse_port_num = %d.\n",
               __FUNCTION__, __LINE__,
               (this_is_tsn_h_sku() == TRUE) ? "TSN-H" : "TSN-M",
               pse_port_num);
    }

    /* 1. Masked all interrupt from PSU side */
    prpass(testpass, "Mask all PSE interrupts");
    /* 1.1 Get PSE default interrupt mask from Interrupt Mask Reg.(0x01h) */
    psu_reg_offset = (uint32_t)TPS2386B_INTR_MASK_REG;
    if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_def_intr_mask) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }

    /* 1.2 Masked all PSE interrupts */
    psu_reg_val = (uchar)PSE_INTR_MSK_MASKED_ALL;
    if (tsn_psu_reg_wr(psu_reg_offset, psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }
    msleep(10);

    /* 1.3 Confirm PSE Interrupt Mask Reg. is set correctly */
    psu_reg_val = 0;
    if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }

    if (psu_reg_val != (uchar)PSE_INTR_MSK_MASKED_ALL) {
        cterr('f', 0, "Failed to mask all PSU interrupts");
        return (FAILED);
    }

    /* 2. Confirm FPGA doesn't get interrupt from PoE PSU */
    prpass(testpass, "Confirm no PSE interrupt from FPGA side");
    /* 2.1 Clear pending interrupt on FPGA ext. interrupt pending Reg.(0x1128) */
    fpga_reg_val = (uint)POE_FPGA_INTR_PENDING;
    if (fpga_write_32_reg(fpga_reg_offset, fpga_reg_val) != PASSED) {
        printf("%s:%d Failed to write FPGA Reg.(0x%04X).\n",
               __FUNCTION__, __LINE__, fpga_reg_offset);
        return (FAILED);
    }
    msleep(100);

    fpga_reg_val = (uint)POE_FPGA_INTR_PENDING;
    if (fpga_read_32_reg(fpga_reg_offset, &fpga_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read FPGA Register(0x%04X)",
                      __FUNCTION__, __LINE__, fpga_reg_offset);
        return (FAILED);
    }

    if (fpga_reg_val & (uint)POE_FPGA_INTR_PENDING) {
        cterr('f', 0, "Failed! FPGA got Unexpected interrupt from PoE PSU");
        return (FAILED);
    }

    /* 3. Trigger interrupt form PoE PSU */
    prpass(testpass, "Trigger interrupt from PSE");
    psu_reg_offset = (uint32_t)TPS2386B_INTR_REG;
    if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read PSE reg.(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }

    /* 3.1 Check if there's already have an interrupt at PSE */
    if (psu_reg_val == 0) {
        /* 3.1.2 If no existing interrupt, then create one. */
        /* 3.1.2.1 Confirm PSE in Semi-auto mode */
        psu_reg_val = 0;
        psu_reg_offset = (uint32_t)TPS2386B_OP_MODE_REG;
        if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read PSE reg.(0x%02X)",
                          __FUNCTION__, __LINE__, psu_reg_offset);
            return (FAILED);
        }

        config_val = 0;
        for (p_ctr = PSE_PORT_ONE; p_ctr <= pse_port_num; p_ctr++) {
            config_val |= (uchar)(PSE_PORT_SEMIAUTO(p_ctr));
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d config_val @0x%02X = 0x%02X.\n",
                   __FUNCTION__, __LINE__, (uchar)psu_reg_offset, config_val);
        }

        if (psu_reg_val != config_val) {
            psu_reg_wr = config_val;
            if (tsn_psu_reg_wr(psu_reg_offset, psu_reg_wr) != PASSED) {
                cterr('f', 0, "%s:%d Failed to write PSU Register(0x%02X)",
                              __FUNCTION__, __LINE__, psu_reg_offset);
                return (FAILED);
            }

            msleep(TSN_PSE_ACCESS_TIME);

            psu_reg_val = 0;
            if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
                cterr('f', 0, "%s:%d Failed to read PSE reg.(0x%02X)",
                              __FUNCTION__, __LINE__, psu_reg_offset);
                return (FAILED);
            }

            if (psu_reg_val != config_val) {
                cterr('f', 0, "%s:%d Failed to set PSE %d ports to Semi-auto mode",
                              __FUNCTION__, __LINE__, pse_port_num);
                return (FAILED);
            }
        }

        /* 3.1.2.2 Enable PSE port detection and classification */
        psu_reg_offset = (uint32_t)TPS2386B_DETCLA_EN_REG;
        psu_reg_val = 0;
        if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read PoE PSE Detect/Class"
                          " reg.(0x%02X)",
                          __FUNCTION__, __LINE__, (uchar)psu_reg_offset);
            return (FAILED);
        }

        config_val = 0;
        for (p_ctr = PSE_PORT_ONE; p_ctr <= pse_port_num; p_ctr++) {
            config_val |= (uchar)(PSE_DETCLA_EN(p_ctr));
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("%s:%d config_val @0x%02X = 0x%02X.\n",
                   __FUNCTION__, __LINE__, (uchar)psu_reg_offset, config_val);
        }

        if (psu_reg_val != config_val) {
            if (tsn_psu_reg_wr(psu_reg_offset, config_val) != PASSED) {
                cterr('f', 0, "%s:%d Failed to read PoE PSE Detect/Class"
                              " reg.(0x%02X)",
                              __FUNCTION__, __LINE__, (uchar)psu_reg_offset);
                return (FAILED);
            }

            msleep(TSN_PSE_ACCESS_TIME);

            psu_reg_val = 0;
            if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
                cterr('f', 0, "%s:%d Failed to read PoE PSE Detect/Class"
                              " reg.(0x%02X)",
                              __FUNCTION__, __LINE__, (uchar)psu_reg_offset);
                return (FAILED);
            }

            if (psu_reg_val != config_val) {
                cterr('f', 0, "%s:%d Failed to enable PSE %d ports detection or "
                              "classification",
                              __FUNCTION__, __LINE__, pse_port_num);
                return (FAILED);
            }
        }

        msleep(TSN_PSE_ACCESS_TIME);

        /* 3.1.2.3 Confirm a PSE interrupt is triggered */
        psu_reg_val = 0;
        psu_reg_offset = (uint32_t)TPS2386B_INTR_REG;
        if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
            cterr('f', 0, "%s:%d Failed to read PSE reg.(0x%02X)",
                          __FUNCTION__, __LINE__, psu_reg_offset);
            return (FAILED);
        }

        if (psu_reg_val == 0) {
            cterr('f', 0, "%s:%d Failed to trigger interrupt from PSE",
                              __FUNCTION__, __LINE__);
            return (FAILED);
        }
    }

    psu_reg_offset = (uint32_t)TPS2386B_INTR_MASK_REG;
    psu_reg_wr = psu_reg_val;
    if (tsn_psu_reg_wr(psu_reg_offset, psu_reg_wr) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }
    msleep(10);

    psu_reg_val = 0;
    if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }

    if (psu_reg_val != psu_reg_wr) {
        cterr('f', 0, "Failed to trigger interrupt from PoE PSU");
        return (FAILED);
    }

    /* Check if FPGA get interrupt from PoE PSU */
    prpass(testpass, "Confirm FPGA get interrupt that from PSE");
    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    fpga_reg_val = (uint)POE_FPGA_INTR_PENDING;
    if (fpga_write_32_reg(fpga_reg_offset, fpga_reg_val) != PASSED) {
        printf("%s:%d Failed to write FPGA Reg.(0x%04X).\n",
               __FUNCTION__, __LINE__, fpga_reg_offset);
        return (FAILED);
    }
    msleep(100);

    fpga_reg_val = 0;
    if (fpga_read_32_reg(fpga_reg_offset, &fpga_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read FPGA Register(0x%04X)",
                      __FUNCTION__, __LINE__, fpga_reg_offset);
        return (FAILED);
    }

    if ((fpga_reg_val & (uint)POE_FPGA_INTR_PENDING) == 0) {
        cterr('f', 0, "Failed! FPGA didn't get interrupt from PoE PSU");
        return (FAILED);
    }

    /* Clear interrupt form PoE PSU */
    prpass(testpass, "Clear interrupt from PSE side");
    psu_reg_offset = (uint32_t)TPS2386B_INTR_MASK_REG;
    psu_reg_val = (uchar)PSE_INTR_MSK_MASKED_ALL;
    if (tsn_psu_reg_wr(psu_reg_offset, psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }
    msleep(10);

    psu_reg_val = 0;
    if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }

    if (psu_reg_val != (uchar)PSE_INTR_MSK_MASKED_ALL) {
        cterr('f', 0, "Failed to mask all PSU interrupts");
        return (FAILED);
    }

    /* Check if FPGA interrupt that from PoE PSU is cleared */
    prpass(testpass, "Confirm interrupt is cleared from FPGA side");
    /* Clear pending interrupt on FPGA external interrupt pending Reg.(0x1128) */
    fpga_reg_val = (uint)POE_FPGA_INTR_PENDING;
    if (fpga_write_32_reg(fpga_reg_offset, fpga_reg_val) != PASSED) {
        printf("%s:%d Failed to write FPGA Reg.(0x%04X).\n",
               __FUNCTION__, __LINE__, fpga_reg_offset);
        return (FAILED);
    }
    msleep(100);

    fpga_reg_val = (uint)POE_FPGA_INTR_PENDING;
    if (fpga_read_32_reg(fpga_reg_offset, &fpga_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read FPGA Register(0x%04X)",
                      __FUNCTION__, __LINE__, fpga_reg_offset);
        return (FAILED);
    }

    if ((fpga_reg_val & (uint)POE_FPGA_INTR_PENDING) != 0) {
        cterr('f', 0, "Failed! FPGA still got interrupt from PoE PSU");
        return (FAILED);
    }

    /* Restore PSU Interrupt Mask register setup */
    prpass(testpass, "Restore PSE interrupt mask value");
    psu_reg_offset = (uint32_t)TPS2386B_INTR_MASK_REG;
    if (tsn_psu_reg_wr(psu_reg_offset, psu_def_intr_mask) != PASSED) {
        cterr('f', 0, "%s:%d Failed to write PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }
    msleep(10);

    psu_reg_val = 0;
    if (tsn_psu_reg_rd(psu_reg_offset, (char *)&psu_reg_val) != PASSED) {
        cterr('f', 0, "%s:%d Failed to read PSU Register(0x%02X)",
                      __FUNCTION__, __LINE__, psu_reg_offset);
        return (FAILED);
    }

    if (psu_reg_val != psu_def_intr_mask) {
        cterr('f', 0, "Failed to restore PSU Interrupt Mask Reg(0x%02X)",
                      psu_reg_offset);
        return (FAILED);
    }
    return (PASSED);
}


/*------------------------------------------------------------------
$Log: diag_poe_psu.c,v $
Revision 1.3  2017/10/19 13:41:11  palin2
Fixed CSCvg23616: TSN PoE link down intermittently when connect to iPorter PoE tester.

Revision 1.2  2017/08/02 14:21:45  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:02  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/24 14:14:10  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.2  2017/07/20 13:38:04  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.4.2.2  2017/05/17 02:19:41  palin2
Updated function of triggering PoE PSE side interrupt.

Revision 1.1.4.4.2.1  2017/02/23 11:03:16  palin2
Updated code based on FPGA changes. These updates are verified on P2A TSN.

Revision 1.1.4.4  2016/11/15 01:20:02  palin2
Added PoE PSU to FPGA interrupt test.

Revision 1.1.4.3  2016/08/16 03:08:17  palin2
Unified test pass print outs.

Revision 1.1.4.2  2016/06/30 06:22:48  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.4  2016/06/16 07:51:11  palin2
Updated PoE related utilities and code.

Revision 1.1.2.3  2016/05/18 09:02:10  steja
Fix do all test for POE

Revision 1.1.2.2  2016/05/10 06:17:33  palin2
Updated PoE PSE related diag code after bring up.

Revision 1.1.2.1  2016/03/22 22:19:12  palin2
Added PoE PSU Diag.

$Endlog$
*/

