/* $Id: testcard_xaui.c,v 1.18 2020/01/09 01:02:20 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_xaui.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_xaui.c
 *
 * Description: TestCard XAUI related diag tests and utilities.
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteswap.h"
#include "common.h"
#include "proto.h"
#include "common_utils.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "i2c_address.h"
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "testcard_xaui.h"
#include "testcard_tlk_10232.h"
#include "testcard_eth.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "bcm_gesw_defs.h" /* is_bcm_greyhound */
#include "platform_eth_pkt_txrx.h" /* for get sgmii port num */

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_xaui_menu(int);
int  tc_xaui_lpbk_test(int);
int new_tc_xaui_lpbk_test(void);

static void build_tc_xaui_utils(int);
static void tc_alter_xaui_phy_reg(void);
static void tc_load_fw_xaui_phy(void);
static int  tc_xaui_get_lpbk_name(int, char *);
static int  tc_xaui_lpbk_mode_access(int, boolean);
static int  tc_xaui_reg_bit_access(int, uint16_t, uint16_t, uint16_t, boolean);


/******************************************************************************* 
 *                                  Externs                                    *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int tc_fpga_reg_rd(uint32_t, uint16_t *);
extern int tc_fpga_reg_wr(uint32_t, uint16_t);
extern int sgmii_lpbk_util(int, int);


/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
 */


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * XAUI Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_xaui_diag_table[] = {
    {"XAUI Utilities",
     (PFT)build_tc_xaui_utils,      TRUE,
     0,                          (PFT)0, 0, (PFT)build_tc_xaui_utils, TRUE},
    {"XAUI internal loopback test", 
     (PFT)new_tc_xaui_lpbk_test,     0,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, TRUE,
     (PFT)0,                   0},
    {"XAUI PCS/PMD internal loopback test", 
     (PFT)tc_xaui_lpbk_test,        BCM8727_PCSPMD_LPBK,
     (MF_CONTINUOUS | MF_DOALL), (PFT)for_10gkr_testcard, FALSE,
     (PFT)0,                   0},
    {"XAUI external loopback test", 
     (PFT)tc_xaui_lpbk_test,        BCM8727_EXT_LPBK,
     (MF_CONTINUOUS),            (PFT)for_10gkr_testcard, FALSE,
     (PFT)0,                   0},
};

#define TC_XAUI_DIAG_TABLE_SIZE (sizeof(tc_xaui_diag_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_xaui_diag_pri_items[TC_XAUI_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_xaui_diag_sec_items[TC_XAUI_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_xaui_diag = {
    "TestCard XAUI SubMenu",       /* title */
    0,                             /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,         /* shows major flags */
    0,                             /* generic prompt */
    0,                             /* size -- bumped by add_menu_item() */
    tc_xaui_diag_pri_items,
};

static struct menuinfo *tc_xaui_diag_p = &tc_xaui_diag;


/*
 * XAUI Utilities SubMenu
 */
static submenu_xtable_t xaui_utils_tbl[] = {
    {"Alter XAUI PHY register",      (PFT)tc_alter_xaui_phy_reg,       FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Load Firmware for XAUI PHY",   (PFT)tc_load_fw_xaui_phy,         FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
};

#define XAUI_UTILS_TBL_SIZE (sizeof(xaui_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_xaui_utils_pri_items[XAUI_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_xaui_utils_sec_items[XAUI_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_xaui_utils = {
    "TestCard XAUI Utilities",       /* title */
    0,                               /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,           /* shows major flags */
    0,                               /* generic prompt */
    0,                               /* size -- bumped by add_menu_item() */
    tc_xaui_utils_pri_items,
};

static struct menuinfo *tc_xaui_utils_p = &tc_xaui_utils;


/*******************************************************************************
 *
 * Function   : build_tc_xaui_menu
 * Description: Build TestCard XAUI Tests and Utilities SubMenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_xaui_menu (int submenu)
{
    unsigned int slot;
    slot = testcard_if_p->slot;

    if ((slot == NGSM4_SLOT) && (is_neptune() || is_vg450())) {
        printf("Neptune NGSM4 is not support GE->XAUI connection \n");
        return;
    }

    if (is_goldbeach()) {
        printf("\nGoldbeach didn't have XAUI interface\n");
        return;
    }

    if (is_curie_2ru()) {
        printf("\nCurie2RU didn't have XAUI interface\n");
        return;
    }

    build_primary_submenu(tc_xaui_diag_table, TC_XAUI_DIAG_TABLE_SIZE,
                          "TestCard XAUI SubMenu", &tc_xaui_diag_p);
    build_secondary_submenu(tc_xaui_diag_table, TC_XAUI_DIAG_TABLE_SIZE,
                            tc_xaui_diag_sec_items);

    /* Release XAUI PHY from Reset */
    if (tc_fpga_unreset_device(XAUI_RESET) != PASSED) {
        return;
    }

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_xaui_diag, tc_xaui_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_xaui_diag_p);
    }

    return;
}


/*******************************************************************************
 *
 * Function   : build_tc_xaui_utils
 * Description: Build TestCard XAUI related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_xaui_utils (int submenu)
{
    build_primary_submenu(xaui_utils_tbl, XAUI_UTILS_TBL_SIZE,
                          "TestCard XAUI Utils SubMenu", &tc_xaui_utils_p);
    build_secondary_submenu(xaui_utils_tbl, XAUI_UTILS_TBL_SIZE,
                            tc_xaui_utils_sec_items);

    menu(&tc_xaui_utils, tc_xaui_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   :	tc_xaui_phy_reg_rd
 * Description:	Function to read TestCard XAUI PHY register 
 *              by accessing TestCard FPGA registers.
 * Inputs     :	Device Type, Register Address,
 *              and Data buffer to put read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_xaui_phy_reg_rd (int smi_addr, uint16_t dev_type, uint16_t reg_addr, uint16_t *data)
{
    uint16_t chk_data = SMI_CHAN_BUSY, ctr = 0, in_data = 0;

    /* 1. Write the FPGA SMI1 Data/Address register (0x0E) */
    in_data = reg_addr;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_ADDR_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 2. Write the FPGA SMI1 Control register (0x0C) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (CLAUSE_45_ADDR << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_type << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 3. Poll the FPGA Speed/Status register (0x0A) 
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < XAUI_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI1_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI1_STAT_REG_OFFSET);
            return (FAILED);
        }
        ctr++;
        if (ctr == XAUI_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI1_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }
    }

    /* 4. Write the FPGA SMI1 Control register (0x0C) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (READ_45_P_INCR_ADDR << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_type << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 5. Poll the FPGA Speed/Status register (0x0A)
     *    until the SMI Channel Busy bit is cleared.
     */
    ctr = 0;
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < XAUI_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI1_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI1_STAT_REG_OFFSET);
            return (FAILED);
        }
        ctr++;
        if (ctr == XAUI_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI1_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }

    }

    /* 6. Read the FPGA SMI1 Data/Address register (0x0E) */
    if (tc_fpga_reg_rd(SMI1_ADDR_REG_OFFSET, data) != PASSED) {
        printf("%s:%d Failed to read FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_ADDR_REG_OFFSET);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_xaui_phy_reg_wr
 * Description:	Utility to write TestCard GE PHY register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_xaui_phy_reg_wr (int smi_addr, uint16_t dev_type, uint16_t reg_addr,
                               uint16_t write_data)
{
    uint16_t chk_data = SMI_CHAN_BUSY, ctr = 0, in_data = 0;

    /* 1. Write the FPGA SMI1 Data/Address register (0x0E) */
    in_data = reg_addr;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_ADDR_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 2. Write the FPGA SMI1 Control register (0x0C) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (CLAUSE_45_ADDR << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_type << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 3. Poll the FPGA Speed/Status register (0x0A) 
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < XAUI_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI1_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI1_STAT_REG_OFFSET);
            return (FAILED);
        }
        ctr++;
        if (ctr == XAUI_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI1_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }
    }

    /* 4. Write the FPGA SMI1 Data/Address register (0x0E) */
    in_data = write_data;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_ADDR_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 5. Write the FPGA SMI1 Control register (0x0C) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (WRITE_FRAME << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_type << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI1_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI1_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 6. Poll the FPGA Speed/Status register (0x0A) 
     *    until the SMI Channel Busy bit is cleared.
     */
    ctr = 0;
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < XAUI_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI1_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI1_STAT_REG_OFFSET);
            return (FAILED);
        }
        ctr++;
        if (ctr == XAUI_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI1_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_xaui_get_dev_addr
 * Description:	Function to get related device address from User.
 * Inputs     :	type_name, dev_type - buffer to put specific data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_xaui_get_dev_addr (char *type_name, uint16_t *dev_type)
{
    uint16_t type_opt = 0;

    printf("\n[1] PMA/PMD or user-defined register.\n");
    printf("[2] PCS or user-defined register.\n");
    printf("[3] PHY XS or user-defined register.\n");
    printf("[4] CL37 AN.\n");
    type_opt = getdec_answer("Enter Device Type you want to read:", 3, 1, 4);

    switch (type_opt) {
    case PMAPMD_OPT:
        sprintf(type_name, "PMA/PMD");
        *dev_type = BCM8727_PHY_PMAPMD;
        break;
    case PCS_OPT:
        sprintf(type_name, "PCS");
        *dev_type = BCM8727_PHY_PCS;
        break;
    case PHYXS_OPT:
        sprintf(type_name, "PHY XS");
        *dev_type = BCM8727_PHY_PHYXS;
        break;
    case CL37AN_OPT:
        sprintf(type_name, "CL37 AN");
        *dev_type = BCM8727_PHY_CL37AN;
        break;
    default:
        printf("%s:%d Invalid Device Type: %d.\n",
               __FUNCTION__, __LINE__, type_opt);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tc_load_fw_xaui_phy
 * Description: Utility to alter testcard xaui PHY register.
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static void tc_load_fw_xaui_phy (void) 
{
    ushort dev_addr, smi_addr;
    ushort wrval, rdval;

    /* we are accessing user defined register, 
     * the dev_addr could be PMA/PMD, PCS or PHY XS (1,3, or 4)
     */
    dev_addr = BCM8727_PHY_PMAPMD; 
    for (smi_addr = 0; smi_addr < 2; smi_addr ++) {
        printf("progrem address %d \n", smi_addr);
        /* new setting */
        tc_xaui_phy_reg_rd(smi_addr, dev_addr, 0xCA1F, &rdval);
        wrval = rdval | 0x0400; 
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA1F, wrval);
        msleep(100);
        wrval &= ~(0x0400);
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA1F, wrval);
        msleep(50);
        tc_xaui_phy_reg_rd(smi_addr, dev_addr, 0x0000, &rdval);
        wrval = rdval | 0x8000; 
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0x0000, wrval);
        msleep(1);

        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA10, 0x0001);
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA10, 0x008C);
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA85, 0x0001);
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA10, 0x018A);
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA10, 0x0188);
        msleep(100);
        tc_xaui_phy_reg_wr(smi_addr, dev_addr, 0xCA85, 0x0000);
    }

    printf("Done\n");
    return;
}

/*******************************************************************************
 *
 * Function   :	tc_alter_xaui_phy_reg
 * Description:	Utility to alter testcard xaui PHY register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static void tc_alter_xaui_phy_reg (void)
{
    char c;
    ushort rdval, wrval, dev_addr;
    int smi_addr;
    int regnum, regnum_max = 0xFFFF;
    char type_name[TC_BUF_SIZE];

    printf("\nPort0 (Channel 0) addr is 0x0; Port1 (Channel 1) addr is 0x1\n");
    smi_addr = gethex_answer("\nEnter addr", 0, 0, 0xFF);
    printf("\nSMI addr is : %d\n", smi_addr);

    /* Get info from user */
    if (tc_xaui_get_dev_addr(type_name, &dev_addr) != PASSED) {
        printf("%s: Failed to get Device addr. from User.\n", __FUNCTION__);
        return;
    }

    do {
        regnum = gethex_answer("\nEnter reg addr", 0, 0, regnum_max);
        tc_xaui_phy_reg_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("Current value of dev 0x%x reg 0x%x = 0x%x\n",
                dev_addr, regnum, rdval);

        c = getc_answer("Do you want to change value?", "yn",'n');

        if (c == 'y') {
            wrval = gethex_answer("Enter value:", 0, 0, 0xffff);
            tc_xaui_phy_reg_wr(smi_addr, dev_addr, regnum, wrval);
            tc_xaui_phy_reg_rd(smi_addr, dev_addr, regnum, &rdval);
            printf("Read back dev 0x%x reg 0x%x = 0x%x\n",
                     dev_addr, regnum, rdval);
        }
    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return;
}

/*******************************************************************************
 *
 * Function   :	tc_xaui_get_lpbk_name
 * Description:	Get related name of loopback mode.
 * Inputs     :	lpbk_type - Type of loopback mode
 *              lpbk_name - buffer to put name of loopback
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_xaui_get_lpbk_name (int lpbk_type, char *lpbk_name)
{
    /* Reset buffer */
    memset(lpbk_name, 0, TC_BUF_SIZE);

    switch (lpbk_type) {
    case BCM8727_EXT_LPBK:
        sprintf(lpbk_name, "External");
        break;
    case BCM8727_SYS_LPBK:
        sprintf(lpbk_name, "System Internal");
        break;
    case BCM8727_IND_LANE_LPBK:
        sprintf(lpbk_name, "Independent Lane Internal");
        break;
    case BCM8727_PCSPMD_LPBK:
        sprintf(lpbk_name, "PCS/PMD Internal");
        break;
    case BCM8727_XGXS_LPBK:
        sprintf(lpbk_name, "XGXS");
        break;
    case BCM8727_LINE_LPBK:
        sprintf(lpbk_name, "Line");
        break;
    case BCM8727_PRBS_LPBK:
        sprintf(lpbk_name, "PRBS");
        break;
    default:
        printf("%s: Unknown Loopback Type (%d).\n",
               __FUNCTION__, lpbk_type);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_xaui_reg_bit_access
 * Description:	Function to set/unset bit of  TestCard XAUI, BCM8727, register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_xaui_reg_bit_access (int smi_addr, uint16_t dev_type, uint16_t reg_addr,
                                   uint16_t set_data, boolean set_opt)
{
    uint16_t reg_val = 0;

    /* Read the specified PHY register */
    if (tc_xaui_phy_reg_rd(smi_addr, dev_type, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nThe value of XAUI PHY, (Dev.Reg: %01X.%04X) = 0x%04X.\n",
               dev_type, reg_addr, reg_val);
    }

    if (set_opt == ENABLE) {
        reg_val |= set_data;
    } else {
        reg_val &= ~(set_data);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("set_opt = %d, and set_data = 0x%04X.\n", set_opt, set_data);
    }

    /* Write the specified PHY register */
    if (tc_xaui_phy_reg_wr(smi_addr, dev_type, reg_addr, reg_val) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_xaui_lpbk_mode_access
 * Description:	Function to put TestCard XAUI, BCM8727, into loopback mode.
 * Inputs     :	lpbk_type - Type of Loobp mode
 *              lpbk_opt  - ENABLE/DISABLE loobpack mode
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_xaui_lpbk_mode_access (int lpbk_type, boolean lpbk_opt)
{
    uint16_t lpbk_dev = 0, lpbk_reg = 0, lpbk_data = 0;
    int smi_addr = TC_XAUI_PHY_ADDR;

    /* dev 3, reg 0, bit 14 for loopback */
    switch (lpbk_type) {
    case BCM8727_PCSPMD_LPBK:   /*  0x4  */
        lpbk_dev = BCM8727_PHY_PCS; /* 0x3 */
        lpbk_reg = PCS_CTRL1_REG_OFF;  /* 0x00  */
        lpbk_data = PCS_CTRL1_LPBK;  /* 0x4000 bit 14 */
        break;
    default:
        printf("%s: Unknown Loopback Type (%d).\n", __FUNCTION__, lpbk_type);
        return (FAILED);
    }

    if (tc_xaui_reg_bit_access(smi_addr, lpbk_dev, lpbk_reg, lpbk_data, lpbk_opt) != PASSED) {
        printf("%s:%d Failed to access BCM8727 register (%01X.%04X).\n",
               __FUNCTION__, __LINE__, lpbk_dev, lpbk_reg);
        return (FAILED);
    }

    /* Based on BCM8727 datasheet, we should set the lock detect by pass bit
     * (1.CA7B.14) to ignores the incorrect state of the unconnected port.
     */
    if (tc_xaui_reg_bit_access(smi_addr, 
                               BCM8727_PHY_PMAPMD,
                               BCM8727_LOCK_DET_REG_OFF,
                               BCM8727_LOCK_DET_BYPASS,
                               lpbk_opt) != PASSED) {
        printf("%s:%d Failed to access BCM8727 register (%01X.%04X).\n",
               __FUNCTION__, __LINE__, BCM8727_PHY_PMAPMD,
               BCM8727_LOCK_DET_REG_OFF);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_xaui_lpbk_test
 * Description:	TestCard XAUI loopback test (Host->TestCard->Host).
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_xaui_lpbk_test (int lpbk_type)
{
    int  num_pkt = 300;
    char lpbk_name[TC_BUF_SIZE];
    int ctrl_plane_sgmii_port;

    if (lpbk_type != BCM8727_EXT_LPBK) {
        if (tc_xaui_lpbk_mode_access(lpbk_type, ENABLE) != PASSED) {
            cterr('f', 0, "%s: Failed to enter loopback mode.", __FUNCTION__);
            return (FAILED);
        }
    }

    if (tc_xaui_get_lpbk_name(lpbk_type, lpbk_name) != PASSED) {
        cterr('f', 0, "%s: Failed to get loopback name.", __FUNCTION__);
        return (FAILED);
    }

    testname("TestCard XAUI %s loopback", lpbk_name);
    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    if (is_overlord() || is_juno()) {
        ctrl_plane_sgmii_port = CPU_SGMII_PORT1;
    }
    else {
	/* USD */
        ctrl_plane_sgmii_port = CPU_SGMII_PORT3;
    }

    if (sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt) != PASSED) {
        cterr('f', 0, "\n%s: Failed to do TestCard XAUI "
                      "%s loopback test from Host side SGMII%d.",
                      __FUNCTION__, lpbk_name, ctrl_plane_sgmii_port);
        return (FAILED);
    }

    if (lpbk_type != BCM8727_EXT_LPBK) {
        if (tc_xaui_lpbk_mode_access(lpbk_type, DISABLE) != PASSED) {
            cterr('f', 0, "%s: Failed to exit loopback mode.", __FUNCTION__);
            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : new_tc_xaui_lpbk_test
 * Description: New TestCard XAUI loopback test 
 *             (Host->TC TLK ->TC Xaui(loobpack)->TC TLK->Host).
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int new_tc_xaui_lpbk_test (void)
{
    int  num_pkt = 300;
    int ctrl_plane_sgmii_port;
    int test_port = 0;
    int retry = 6, rc;
    int ntc_gesw_chk, tc_max_eth_port = TC_MAX_ETH_PORT; 

    if (is_goldbeach()) {
        printf("\nGoldbeach didn't have XAUI interface\n");
        return (PASSED);
    }

    if (is_curie_2ru()) {
        printf("\nCurie2RU didn't have XAUI interface\n");
        return (PASSED);
    }

    /* Curie 1RU/2RU NIM only has 1 ge intf */
    if (is_curie_1ru() || is_curie_2ru()) {
        if (testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_NIM) {
            tc_max_eth_port = CURIE_NIM_MAX_ETH_PORT;
        }
    }

    /* the test port is just the same as smi address */
    for (test_port = 0; test_port < tc_max_eth_port; test_port++) {

        if (test_port == 0) {
            ntc_gesw_chk = ntc_gesw_p0_type;
        } else {
            ntc_gesw_chk = ntc_gesw_p1_type;
        } 

        testname("TestCard XAUI port%d loopback", test_port);
  
        prpass(testpass, "initialize tlk10232 settting");
        while (retry > 0) {
            tc_tlk10232_init_10gkr_ti_setting(); 

            /* disable another port to bypass packet conflict */
            if (test_port == 0) {
                tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);

                /* we dont want to change legacy code to bypass 
                 * is_bcm_greyhound() check, then we check cure before 
                 * is_bcm_greyhound() */
                if (is_curie_1ru() || is_curie_2ru()) {
                    prpass(testpass, "set system loopback on bcm");
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x206E);
                } else if (!is_bcm_greyhound() || ntc_gesw_chk) { 
                    /* non-GH platform or ntpc nim2 and nim3, 1g */ 
                    tc_tlk10232_set_mode(TC_TLK10232_CHA_ADDR);
                    prpass(testpass, "set independent lane loopback on bcm");
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x262E);
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8017, 0x00F0);
                } else  {  /* GH platform, 10G */
                    prpass(testpass, "set system loopback on bcm");
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x206E);
                }
            } else { /* port 1 always using 1g-kx speed */
                tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
                if (is_curie_1ru() || is_curie_2ru()) {
                    /* afix - need to support 1G on Curie */
                    prpass(testpass, "Curie 1RU testing 10G for GE1"); 
                    prpass(testpass, "set system loopback on bcm");
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x206E);
                } else {
                    prpass(testpass, "setting speed to 1g-kx on tlk10232");
                    tc_tlk10232_set_mode(TC_TLK10232_CHB_ADDR);

                    /* independent lan loopback for 1g-kx */
                    /* set xaui address 1 (channel 1), dev_addr = 0x4, reg 0x8000, val 0x262E */
                    /* set xaui address 1 (channel 1), dev_addr = 0x4, reg 0x8017, val 0x00F0 */
                    prpass(testpass, "set independent lane loopback on bcm");
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x262E);
                    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8017, 0x00F0);
                }
            }

            ctrl_plane_sgmii_port = get_sgmii_port_num(testcard_if_p->slot, TYPE_SWITCH); 

            /* afix : need to use better way to support different NGIO */
            if (is_curie_1ru()) {
                if ((testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_SM) ||
                    (testcard_if_p->is_10gkr == TC_10GKR_SM_ON_SM)) {
                    if (test_port != 0 ) { /* SM GE0 is eth6, GE1 is eth8 */
                        /* afix - need to resolve it gracefully */
                        ctrl_plane_sgmii_port++;
                        ctrl_plane_sgmii_port++;
                    }
                }
            }
            if (is_curie_2ru()) {
                ctrl_plane_sgmii_port -= (test_port * 2);
            }

            if (is_curie_1ru() || is_curie_2ru()) {
                if (ntc_chk_eth_linkup(ctrl_plane_sgmii_port) == FALSE) {
                    printf("link up failed on host eth port, retry to bring up link again..\n");
                    retry--;
                    continue;
                }
            } else { 
                /* check link up status on gesw */
                rc = tc_tlk10232_chk_gesw_link_status(test_port);
                if (rc == FAILED) {
                    prpass(testpass, "retry to bring up link again ");
                    retry--;
                    continue; 
                } 
            }

            /* Do SGMII loopback test. */
            prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

            if (sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt) != PASSED) {
                printf("%s: Failed to do TestCard XAUI "
                          "port%d loopback test from Host side SGMII%d.",
                          __FUNCTION__, test_port, ctrl_plane_sgmii_port);
                printf("\nretry again...\n");
                retry--;
            } else {
                break;
            }

        } // while (retry > 0)

        if (retry == 0) {
            cterr('f', 0, "\n%s: Failed on TestCard XAUI%d", __FUNCTION__, test_port);
        }

        tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x212E);
        tc_xaui_phy_reg_wr(test_port, 0x4, 0x8017, 0xFF00);
        tc_tlk10232_cleanup(test_port);

    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: testcard_xaui.c,v $
Revision 1.18  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.17  2019/08/06 06:56:11  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.16.2.5  2018/08/28 16:31:18  alpeng
stablize testcard loobpack test on curie

Revision 1.16.2.4  2018/08/20 18:26:39  alpeng
upgrade testcard plx pcie link up test for curie

Revision 1.16.2.3  2018/08/16 18:22:38  alpeng
remove useless info on i2c_drv; fixed get_sgmii_port on platform_eth_pkt_txrx.c for curie; add wrapper for wic_test and sm_test for prepare eth info on platform_slot.c; support ge1 for SM on testcard;

Revision 1.16.2.2  2018/08/10 08:15:53  alpeng
update eth num for sm1; support sync signal test for curie; skip plx pcie check (wait for rommon); skip ge1 on SM for both tlk10232 and xaui loopback test (wait for rommon); remove NIM slot2 test; fixed pcie lane scan test

Revision 1.16.2.1  2018/08/09 10:18:52  alpeng
support NTC on Curie NIM1

Revision 1.16  2018/05/18 09:24:52  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.15  2016/10/17 11:22:56  iachang
Supported Goldbeach Platform.

Revision 1.14.30.9  2018/01/30 09:49:02  alpeng
 revert to old code; SM TC is not use 10G on GE1

Revision 1.14.30.8  2018/01/16 06:46:30  alpeng
first check in for 10G-KR SM testcard; we need to apply correct id once hw ready for it

Revision 1.14.30.7  2017/11/27 05:59:42  leschen
Initial check in to support VG450.

Revision 1.14.30.6  2017/04/17 10:13:24  alpeng
add gesw ptype check

Revision 1.14.30.5  2017/04/06 02:09:45  leschen
Fix testcard xaui issue.

Revision 1.14.30.4  2017/04/05 06:45:04  leschen
Sync with <ng_diag-tag-032917>

Revision 1.14.30.3  2016/12/19 22:12:07  ptong
Remove temporary added code for Neptune P1A bring-up

Revision 1.14.30.2  2016/10/24 19:06:24  ptong
Temporarily skip TC internal XAUI for neptune bringup

Revision 1.14.30.1  2016/10/21 18:19:39  alpeng
update testcard for sm4

Revision 1.14  2014/10/17 07:18:32  alpeng
supporting retry in case the gesw link is unstable packet cannot be sent

Revision 1.13  2014/08/28 09:39:09  alpeng
support new testcard on overlord

Revision 1.12  2014/08/20 06:21:18  alpeng
support new testcard on non-GH platforms

Revision 1.11  2014/08/11 09:00:04  alpeng
modified display per HW req

Revision 1.10  2014/08/04 07:36:30  alpeng
remove useless function

Revision 1.9  2014/07/28 03:43:46  alpeng
check gesw link status before sending packet

Revision 1.8  2014/07/25 01:36:57  alpeng
support xaui loopback and sort out the test item for new testcard

Revision 1.7  2014/07/22 09:40:13  alpeng
support 10g-kr on ge0 and 1g-kx on ge1 for new testcard

Revision 1.6  2014/07/02 08:09:43  alpeng
add new testcard id for en/disable menu item and select smi addr

Revision 1.5  2014/06/06 08:14:52  alpeng
xaui test could be leveraged for new testcard

Revision 1.4  2014/04/22 06:18:21  alpeng
not support utah P1A anymore; remove is_utah_p1a()

Revision 1.3  2013/09/09 05:34:02  ptong
Replace #ifdef UTAH with is_utah, is_overlord, etc for platform depend code

Revision 1.2  2013/08/09 00:33:00  hroni
Utah uses SGMII port#4 for eth and xaui loopback test

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.5  2013/02/15 10:34:36  palin2
Update the display of the TestCard Diag tests based on code review's comment.

Revision 1.4  2012/11/21 19:47:23  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.3  2012/09/24 17:37:43  palin2
1. Use "Internal loopback test" as default test for TestCard.
2. Unify all tests print out format for TestCard.

Revision 1.2  2012/09/14 08:22:32  palin2
1. Add Test "XAUI PCS/PMD internal loopback test" support.
2. Add Utility "Alter XAUI PHY register" support.
3. Use XAUI PCS/PMD internal loopback test as default test.

Revision 1.1  2012/08/22 16:32:50  palin2
First check-in to supprt XAUI test on TestCard.

$Endlog$
*/

