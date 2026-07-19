/* $Id: testcard_xaui.c,v 1.2 2021/04/15 00:52:30 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/testcard_xaui.c,v $
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
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "testcard_xaui.h"
#include "testcard_tlk_10232.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "diag_eth_pkt_txrx.h" /* for get sgmii port num */
#include "dnv_eth_lib.h"

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
 * Inputs     :	SMI Address, Device Type, Register Address,
 *              and write data to put write register value
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
 * Outputs    : None
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
 * Outputs    : None
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
    int ctrl_plane_sgmii_port;
    int test_port = 0;
    int retry = 6;
    int ntc_gesw_chk, tc_max_eth_port = TC_MAX_ETH_PORT; 
    char eth_interface[8];
    char *iface_name = eth_interface;

    if (testcard_if_p->slot == 1) {
        sprintf(iface_name, PHOENIX_ETH0_BP);
    } else if (testcard_if_p->slot == 2){
        sprintf(iface_name, PHOENIX_ETH1_BP);
    } else {
        printf("Error: unsupported slot=%d\n", testcard_if_p->slot);
    }

    /* Phoenix NIM only has 1 ge intf */
    tc_max_eth_port = PHOENIX_NIM_MAX_ETH_PORT;

    /* the test port is just the same as smi address */
    for (test_port = 0; test_port < tc_max_eth_port; test_port++) {

        if (test_port == 0) {
            ntc_gesw_chk = ntc_gesw_p0_type;
        } else {
            ntc_gesw_chk = ntc_gesw_p1_type;
        } 

        testname("TestCard XAUI port%d loopback", test_port);
  
        prpass(testpass, "initialize tlk10232 settting\n");
        while (retry > 0) {
            tc_tlk10232_init_10gkr_ti_setting(); 
            printf("\nTest_port:%d\n", test_port);

            /* disable another port to bypass packet conflict */
            if (test_port == 0) {
                tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);

                prpass(testpass, "set system loopback on bcm\n");
                tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x206E);

            } else { 
                tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
                prpass(testpass, "set system loopback on bcm\n");
                tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x206E);
            }

            ctrl_plane_sgmii_port = testcard_if_p->slot;

            if (ntc_chk_eth_linkup(ctrl_plane_sgmii_port) == FALSE) {
                printf("link up failed on host eth port, retry to bring up link again..\n");
                retry--;
                continue;
            }

            /* Do SGMII loopback test. */
            prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

            if (eth_pkt_txrx(iface_name, 3, FALSE) != PASSED) {
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

