/* $Id: testcard_tlk_10232.c,v 1.3 2017/03/30 08:34:09 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/testcard_tlk_10232.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_tlk_10232.c
 *
 * Description: TestCard Ethernet related diag tests and utilities.
 *              tlk_10232 chip supports 10G-KR for greyhound GE
 *
 * Copyright (c) 2014-2017 by Cisco Systems, Inc.
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
#include "common_utils.h"
#include "proto.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "testcard_tlk_10232.h"
#include "testcard_xaui.h" /* for bcm chip config */
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_api.h"
#include "platform_fru.h"

/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_tlk10232_menu(int);
static void build_tc_tlk10232_utils(int);
static int tc_tlk10232_mdio_rd(uint, ushort, ulong, ushort *);
static int tc_tlk10232_mdio_wr(uint, ushort, ulong, ushort);
static int tc_tlk10232_get_dev_addr(char *, uint16_t *);
static void tc_tlk10232_reg_access(void);
static void tc_tlk10232_path_reset(int);
static void tc_tlk10232_restore_setting(void);
static void tc_config_tlk10232_path(int, int);
static void tc_config_tlk10232_loopback(int, int);
static void set_tlk10232_port_int_lpbk(int); 
static void tc_tlk10232_reg_dump(void);
static void set_port_1gkx_lpbk(int);
static void set_port_10gkr_lpbk(int);
static void load_file_for_cfg(void);
static void set_port0_lpbk(int);
static void set_port1_lpbk(int);
int tc_tlk10232_internal_loopback_test(int);
void tc_tlk10232_cleanup(int);
void tc_tlk10232_set_mode(int);
void tc_tlk10232_power_off(int);
void tc_tlk10232_init_10gkr_ti_setting(void);

int tc_xaui_phy_reg_wr (int smi_addr, uint16_t dev_type, uint16_t reg_addr,
                               uint16_t write_data);

int tc_xaui_phy_reg_rd (int smi_addr, uint16_t dev_type, uint16_t reg_addr, uint16_t *data);


/******************************************************************************* 
 *                                   Externs                                   *
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

#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)
#define REG_BIT(x) (1 << (x))


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * TLK10232 Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_tlk_10232_diag_table[] = {
    {"TLK10232 Utilities",    /* done */
     (PFT)build_tc_tlk10232_utils,           0,
     0,                         (PFT)0, 0,
     (PFT)build_tc_tlk10232_utils,   TRUE},
    {"TLK10232 Internal Loopback Test",
     (PFT)tc_tlk10232_internal_loopback_test,   0,
     MF_3,                      (PFT)0, 0,
     (PFT)tc_tlk10232_internal_loopback_test,   0},
};

#define TC_TLK_10232_DIAG_TABLE_SIZE (sizeof(tc_tlk_10232_diag_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_tlk_10232_diag_pri_items[TC_TLK_10232_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_tlk_10232_diag_sec_items[TC_TLK_10232_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_tlk_10232_diag = {
    "TestCard TLK10232 SubMenu",   /* title */
    0,                             /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,         /* shows major flags */
    0,                             /* generic prompt */
    0,                             /* size -- bumped by add_menu_item() */
    tc_tlk_10232_diag_pri_items,
};

static struct menuinfo *tc_tlk_10232_diag_p = &tc_tlk_10232_diag;



/*
 * TLK10232 Utilities SubMenu   
 */
static submenu_xtable_t tc_tlk_10232_utils_tbl[] = {
    {"Access TLK 10232 Register",
     (PFT)tc_tlk10232_reg_access,     0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Dump TLK 10232 Register",
     (PFT)tc_tlk10232_reg_dump,     0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port0 to 1g-kx mode and lpbk(DBG)", 
     (PFT)set_port_1gkx_lpbk,         0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port1 to 1g-kx mode and lpbk(DBG)", 
     (PFT)set_port_1gkx_lpbk,         1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port0 to 10g-kr mode and lpbk(DBG)", 
     (PFT)set_port_10gkr_lpbk,        0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port1 to 10g-kr mode and lpbk(DBG)", 
     (PFT)set_port_10gkr_lpbk,        1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Enable port0 lpbk(DBG)", 
     (PFT)set_port0_lpbk,             1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Enable port1 lpbk(DBG)", 
     (PFT)set_port1_lpbk,             1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Disable port0 lpbk(DBG)", 
     (PFT)set_port0_lpbk,             0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Disable port1 lpbk(DBG)", 
     (PFT)set_port1_lpbk,             0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"cleanup port0 (DBG)", 
     (PFT)tc_tlk10232_cleanup,        0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"cleanup port1 (DBG)", 
     (PFT)tc_tlk10232_cleanup,        1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set to 10gkr mode TI (DBG)", 
     (PFT)tc_tlk10232_init_10gkr_ti_setting,  0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Load file for configure tlk and bcm (DBG)", 
     (PFT)load_file_for_cfg,  0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
};

#define TC_TLK_10232_UTILS_TBL_SIZE (sizeof(tc_tlk_10232_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_tlk_10232_utils_pri_items[TC_TLK_10232_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_tlk_10232_utils_sec_items[TC_TLK_10232_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_tlk_10232_utils = {
    "TestCard TLK10232 Utilities",   /* title */
    0,                               /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,           /* shows major flags */
    0,                               /* generic prompt */
    0,                               /* size -- bumped by add_menu_item() */
    tc_tlk_10232_utils_pri_items,
};

static struct menuinfo *tc_tlk_10232_utils_p = &tc_tlk_10232_utils;

/*******************************************************************************
 *
 * Function   : build_tc_tlk10232_menu
 * Description: Build TestCard TLK10232 Tests and Utilities SubMenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_tlk10232_menu (int submenu)
{
    build_primary_submenu(tc_tlk_10232_diag_table, TC_TLK_10232_DIAG_TABLE_SIZE,
                          "TestCard TLK10232 SubMenu", &tc_tlk_10232_diag_p);
    build_secondary_submenu(tc_tlk_10232_diag_table, TC_TLK_10232_DIAG_TABLE_SIZE,
                            tc_tlk_10232_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_tlk_10232_diag, tc_tlk_10232_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_tlk_10232_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   : build_tc_tlk10232_utils
 * Description: Build TestCard TLK10232 related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_tlk10232_utils (int submenu)
{
    build_primary_submenu(tc_tlk_10232_utils_tbl, TC_TLK_10232_UTILS_TBL_SIZE,
                          "TestCard TLK10232 Utils SubMenu", &tc_tlk_10232_utils_p);
    build_secondary_submenu(tc_tlk_10232_utils_tbl, TC_TLK_10232_UTILS_TBL_SIZE,
                            tc_tlk_10232_utils_sec_items);

    menu(&tc_tlk_10232_utils, tc_tlk_10232_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   : tc_tlk10232_mdio_rd
 * Description: Utility to read TestCard TLK10232 register.
 * Inputs     : smi_addr - channel A is 8, channel B is 9. 
 *              dev_addr - device address
 *              reg_addr - the addr. of register
 *              data     - place to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 * Note       : dev_addr = 0x1E - vendor specific device register
 *                       = 0x01 - PMA/PMD register 
 *                       = 0x03 - PCS register
 *                       = 0x07 - auto-neg register 
 *******************************************************************************
 */
static int tc_tlk10232_mdio_rd (uint smi_addr, ushort dev_addr, ulong reg_addr, ushort *data)
{
    uint16_t chk_data = SMI_CHAN_BUSY, ctr = 0, in_data = 0;

    /* 1. Write the FPGA SMI0 Data/Address register (0x08) */
    in_data = reg_addr;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_ADDR_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 2. Write the FPGA SMI0 Control register (0x06) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (CLAUSE_45_ADDR << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_addr << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 3. Poll the FPGA Speed/Status register (0x04)
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < TLK_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI0_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI0_STAT_REG_OFFSET);
            return (FAILED);
        }

        ctr++; 
        if (ctr == TLK_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI0_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        } 
    }

    /* 4. Write the FPGA SMI0 Control register (0x06) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (READ_45_P_INCR_ADDR << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_addr << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 5. Poll the FPGA Speed/Status register (0x04)
     *    until the SMI Channel Busy bit is cleared.
     */
    ctr = 0; 
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < TLK_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI0_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI0_STAT_REG_OFFSET);
            return (FAILED);
        }

        ctr++;
        if (ctr == TLK_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI0_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }
    }


    /* 6. Read the FPGA SMI0 Data/Address register (0x08) */
    if (tc_fpga_reg_rd(SMI0_ADDR_REG_OFFSET, data) != PASSED) {
        printf("%s:%d Failed to read FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_ADDR_REG_OFFSET);
        return (FAILED);
    }

    return (PASSED);

}

/*******************************************************************************
 *
 * Function   :	tc_tlk10232_mdio_wr
 * Description:	Utility to read TestCard TLK10232 register.
 * Inputs     :	smi_addr - SMI addr. channel A is 8, channel B is 9.
 *              dev_addr - device address
 *              reg_addr - the addr. of register
 *              wr_data  - write data 
 * Outputs    : PASSED/FAILED
 *
 * Note       : dev_addr = 0x1E - vendor specific device register
 *                       = 0x01 - PMA/PMD register
 *                       = 0x03 - PCS register
 *                       = 0x07 - auto-neg register
 *******************************************************************************
 */
static int tc_tlk10232_mdio_wr (uint smi_addr, ushort dev_addr, ulong reg_addr, ushort wr_data)
{
    uint16_t chk_data = SMI_CHAN_BUSY, ctr = 0, in_data = 0;

    /* 1. Write the FPGA SMI0 Data/Address register (0x08) */
    in_data = reg_addr;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_ADDR_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 2. Write the FPGA SMI0 Control register (0x06) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (CLAUSE_45_ADDR << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_addr << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 3. Poll the FPGA Speed/Status register (0x04)
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < TLK_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI0_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI0_STAT_REG_OFFSET);
            return (FAILED);
        }
        ctr++;
        if (ctr == TLK_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI0_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }
    }

    /* 4. Write the FPGA SMI0 Data/Address register (0x08) */
    in_data = wr_data;

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_ADDR_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 5. Write the FPGA SMI0 Control register (0x06) */
    in_data = ((CLAUSE_45_FRAME << SMI_SOF_SHIFT) +
               (WRITE_FRAME << SMI_OP_CODE_SHIFT) +
               (smi_addr << SMI_PHY_ADDR_SHIFT) +
               (dev_addr << SMI_DEV_ADDR_SHIFT));

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("%s:%d write in data = 0x%04X.\n", __FUNCTION__, __LINE__, in_data);
    }

    if (tc_fpga_reg_wr(SMI0_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 6. Poll the FPGA Speed/Status register (0x04)
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < TLK_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI0_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI0_STAT_REG_OFFSET);
            return (FAILED);
        }
        ctr++;
        if (ctr == TLK_MAX_RETRY) {
            printf("%s:%d SMI channel is busy. SMI0_STAT_REG_OFFSET = 0x%02X.\n",
                   __FUNCTION__, __LINE__, chk_data);
            return (FAILED);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tc_tlk10232_get_dev_addr
 * Description: Function to get related device address from User.
 * Inputs     : type_name, dev_type - buffer to put specific data
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_tlk10232_get_dev_addr (char *type_name, uint16_t *dev_type)
{
    uint16_t type_opt = 0;

    printf("\n[1] Vendor Specific Device register.\n");
    printf("[2] PMA/PMD register.\n");
    printf("[3] PCS register.\n");
    printf("[4] Auto-Negotiation register.\n");
    printf("[5] User Define register.\n");
    type_opt = getdec_answer("Enter Device Type you want to read:", 1, 1, 5);

    switch (type_opt) {
    case TLK_VDR_SPE_OPT:
        sprintf(type_name, "Vendor Specific");
        *dev_type = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        break;
    case TLK_PMAPMD_OPT:
        sprintf(type_name, "PMA/PMD");
        *dev_type = TLK10232_PMAPMD_DEV_ADDR; /* 0x1 */
        break;
    case TLK_PCS_OPT:
        sprintf(type_name, "PCS");
        *dev_type = TLK10232_PCS_DEV_ADDR; /* 0x3 */
        break;
    case TLK_AUTO_NEG_OPT:
        sprintf(type_name, "Auto-Negotiation");
        *dev_type = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
        break;
    case TLK_USER_DEF_OPT:
        sprintf(type_name, "User defined");
        printf("Warning: make sure the device addr you want to access\n");
        *dev_type = gethex_answer("Enter device addr you want to read:", 0x0, 0x0, 0xFF);
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
 * Function   : tc_tlk10232_reg_access
 * Description: Utility for access TestCard TLK10232 register.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void tc_tlk10232_reg_access (void)
{
    char c;
    ushort rdval, wrval, dev_addr;
    int smi_addr;
    int regnum, regnum_max = 0xFFFF;
    char type_name[TC_BUF_SIZE];

    printf("\nPort0 (Channel A) addr is 0xA; Port1 (Channel B) addr is 0xB\n");
    smi_addr = gethex_answer("\nEnter addr", 0, 0, 0xFF);
    printf("\nSMI addr is : %d\n", smi_addr);

    /* Get info from user */
    if (tc_tlk10232_get_dev_addr(type_name, &dev_addr) != PASSED) {
        printf("%s: Failed to get Device addr. from User.\n", __FUNCTION__);
        return;
    }

    do {
        regnum = gethex_answer("\nEnter reg addr", 0, 0, regnum_max);
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("Current value of dev 0x%x reg 0x%x = 0x%x\n",
                dev_addr, regnum, rdval);

        c = getc_answer("Do you want to change value?", "yn",'n');

        if (c == 'y') {
            wrval = gethex_answer("Enter value:", 0, 0, 0xffff);
            tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, wrval);
            tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
            printf("Read back dev 0x%x reg 0x%x = 0x%x\n", 
                     dev_addr, regnum, rdval);
        }
    } while(getc_answer("Continue?", "yn", 'y') == 'y');

    return;
}



/*******************************************************************************
 *
 * Function   : tc_tlk10232_reg_dump
 * Description: Utility for dump TestCard TLK10232 register.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
static void tc_tlk10232_reg_dump (void)
{
    ushort rdval, dev_addr;
    int smi_addr;
    int regnum;
    char type_name[TC_BUF_SIZE];

    printf("\nPort0 (Channel A) addr is 0xA; Port1 (Channel B) addr is 0xB\n");
    smi_addr = gethex_answer("\nEnter addr", 0, 0, 0xFF);
    printf("\nSMI addr is : %d\n", smi_addr);

    /* Get info from user */
    if (tc_tlk10232_get_dev_addr(type_name, &dev_addr) != PASSED) {
        printf("%s: Failed to get Device addr. from User.\n", __FUNCTION__);
        return;
    }

    if (dev_addr == 0x1e) {
    printf("vendor specific registers\n");

    for (regnum = 0x0000; regnum < 0x0020; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8000; regnum < 0x8005; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8021; regnum < 0x8030; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8040; regnum < 0x8042; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8100; regnum < 0x8102; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }

    } else if (dev_addr == 0x1) {
    printf("pma/pmd registers\n");

    for (regnum = 0x0000; regnum < 0x0010; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x0096; regnum < 0x00B0; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8001; regnum < 0x8020; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x9001; regnum < 0x9004; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }

    } else if (dev_addr == 0x3) {
    printf("pcs registers\n");

    for (regnum = 0x0000; regnum < 0x0002; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x0008; regnum < 0x0009; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x0020; regnum < 0x0030; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8000; regnum < 0x8001; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x8010; regnum < 0x8011; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }

    } else if (dev_addr == 0x7) {
    printf("auto neg registers\n");

    for (regnum = 0x0000; regnum < 0x001F; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }
    for (regnum = 0x0030; regnum < 0x0031; regnum++) {
        tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
        printf("0x%x reg 0x%x = 0x%x\n", dev_addr, regnum, rdval);
    }


    } else {
    printf("invalid dev register\n");


    }

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_path_reset
 *
 * Description: This function config tlk_10232 path reset. 
 *
 * Inputs      : smi_addr - GE0, CH. A HS, addr 0xa; GE1, CH. B HS, addr 0xb
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_path_reset (int smi_addr)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    /* Clear data path. 0x1E.0x000e. val 0x0000->0x0008 */
    /* CHA data path reset */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;          /* 0xE */

    tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
    wrval = rdval | REG_BIT(3); /*  TLK_10232_PATH_RESET; */ 
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, wrval);

    return;
}

static void set_port0_lpbk (int enable) {

    tc_config_tlk10232_loopback(TC_TLK10232_CHA_ADDR, enable); 
    return; 
}

static void set_port1_lpbk (int enable) {

    tc_config_tlk10232_loopback(TC_TLK10232_CHB_ADDR, enable); 
    return; 
}

/******************************************************************************
 *
 * Function: tc_config_tlk10232_loopback
 *
 * Description: This function en/disable tlk10232 on loobpack mode 
 *
 * Inputs      : smi_addr - GE0, CH. A HS, addr 0xa; GE1, CH. B HS, addr 0xb
 *               enable - enable/disable loobpack mode. 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static void tc_config_tlk10232_loopback (int smi_addr, int enable)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR; /* 0x1E */
    regnum = TLK10232_LOOPBACK_TP_CTRL;   /* 0xb */

    tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);

    if (enable) {
        wrval = rdval | 0x8; 
    } else {
        wrval = rdval & (~0x0008); 
    }

    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, wrval);

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_cleanup
 *
 * Description: This function en/disable tlk10232 on loobpack mode
 *
 * Inputs      : test_port - ge0 or ge1
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
void tc_tlk10232_cleanup (int test_port)
{
    int smi_addr;

    if (test_port == 0) {
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }

    tc_config_tlk10232_loopback(smi_addr, DISABLE);
    tc_tlk10232_restore_setting(); 

    return;
}

/******************************************************************************
 *
 * Function: tc_config_tlk10232_path
 *
 * Description: This function config tlk10232 input source:
 *              from LS/HS/channel
 *              the default source for receive source is HS 
 *
 * Inputs      : smi_addr - GE0, CH. A HS, addr 0xa; GE1, CH. B HS, addr 0xb
 *               path - TLK10232 channel source config.
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_config_tlk10232_path (int smi_addr, int path)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR; /* 0x1E */
    regnum = TLK10232_DST_CTRL_2_REG; /* 0x18 */

    /* select transmit source, default from Channel LS */
    tc_tlk10232_path_reset(smi_addr);

    tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
    rdval = rdval & (~TLK10232_DST_DATA_SRC_SEL);  /* rdval & (~0xC000) */

    switch (path) {
        /* bit[14:15] = 0, select same channel LS input 
         *            = 1, select same channel HS input 
         *            = 2, select alternate channel LS input 
         *            = 3, select alternate channel HS output 
         */
        case SAME_LS_INPUT:
            wrval = rdval;
            break; 
        case SAME_HS_INPUT: 
            wrval = rdval | REG_BIT(14);   /* set HS for tx input source */
            break;  
        case ALT_LS_INPUT: 
            wrval = rdval | REG_BIT(15);  
            break;  
        case ALT_HS_OUTPUT: 
            wrval = rdval | REG_BIT(15) | REG_BIT(14); 
            break;  
        default :
            printf("TLK10232 doesn't support this path config\n");
            return;
            break; 
    }

    /* write to reg */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, wrval);

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_power_off
 *
 * Description: disable the power of the channel 
 *
 * Inputs      : smi addr - addr for channel a or b
 * Outputs     : None
 *
 *****************************************************************************/
void tc_tlk10232_power_off (int smi_addr) 
{
    int dev_addr, regnum;
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_CHANNEL_CTRL;        /* 0x1 */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x8000);

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_set_mode
 *
 * Description: setting mode to 1g-kx 
 *
 * Inputs      : smi_addr - port smi address 
 * Outputs     : None
 *
 *****************************************************************************/
void tc_tlk10232_set_mode (int smi_addr)
{
    int dev_addr, regnum;
    
    /* set port as 1g-kx mode (force speed mode) */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_CHANNEL_CTRL;        /* 0x1 */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x0300);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
    regnum = TLK10232_AN_CONTROL;  /* 0x0 */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x2000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;          /* 0xE */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x0008);

    return;
}


/******************************************************************************
 *
 * Function: set_tlk10232_port_int_lpbk
 *
 * Description: this function set tlk10232 to internal loopback 
 *              from GE switch to tlk10232 ge port
 *
 * Inputs      : port - ge port 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static void set_tlk10232_port_int_lpbk (int port) {

    int path, smi_addr;

    path = SAME_HS_INPUT;
    if (port == 0) {
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }

    /* ti setting for init tlk chip */
    prpass(testpass, "Init TLK10232 state...");
    tc_tlk10232_init_10gkr_ti_setting(); 


    /* GE1 for 1g-kx mode, need to disable ge0 link and setup ge1 link */
    /* disable the other port power to bypass conflict issue */
    if (port == 1) {
        prpass(testpass, "disable GE0 link state ");
        prpass(testpass, "config GE1 as 1g-kx mode ");
        /* Remove 1G-KX setting for work at 10G-KR mode
        *  tc_tlk10232_set_mode(TC_TLK10232_CHB_ADDR);
        */
        tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
    } else {
        /* port 0 also 1g-kx */
        prpass(testpass, "disable GE1 link state ");
        prpass(testpass, "config GE0 as 1g-kx mode ");
        /* Remove 1G-KX setting for work at 10G-KR mode
        *  tc_tlk10232_set_mode(TC_TLK10232_CHA_ADDR);
        */
        tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
    }

    /* set path and loobpack */ 
    prpass(testpass, "config HS as transmit input GE%d ", port);
    tc_config_tlk10232_path(smi_addr, path);

    /* non-GH plat link up after setup loopback */
    prpass(testpass, "config tlk chip as loopback GE%d ", port);
    tc_config_tlk10232_loopback(smi_addr, ENABLE);

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_chk_gesw_link_status
 *
 * Description: this function check gesw link status. 
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_chk_gesw_link_status (int test_port) {
 
    msleep(1000);  /* use delay as check link time duration */
    return (PASSED);
    printf("FIX ME: %s, need to get gesw link sts from cetus \n", __FUNCTION__);
#if 0
    int ge_port, status, rc, tgt_device, slot, cnt = 20; 

    tgt_device = TGT_DEV_NGWIC;

    slot = testcard_if_p->slot;

    ge_port = ovld_get_ge_sw_port_num(slot, tgt_device, test_port);

    while (cnt) {
        /* the rc value is used for check gesw chip status, we don't need it */
        rc = bcm_gesw_ge_link_status_get(bcm_uid, ge_port, &status); 

        if (status == 1) {
            /* link up */
            /* printf("link up!!!!!!!! \n"); */
            break;
        } else {
           msleep(300);
           cnt--; 
           /* printf("ge_port%d status  0x%x \n",ge_port, status);  */
        }
    }

    if (cnt == 0) {
        printf("\nGESW port%d is not link up\n", ge_port);
        return FAILED; 
    } else {

        return PASSED;
    }
#endif 
        return PASSED;

}

/******************************************************************************
 *
 * Function: tc_tlk10232_internal_loopback_test
 *
 * Description: this function perform the internal loopback test
 *              from GE switch to tlk10232 ge port
 *
 * Inputs      : dummy 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int tc_tlk10232_internal_loopback_test (int dummy)
{
    int num_pkt = 300, main_result = PASSED;
    int test_port = 0;
    int retry = 6, rc;

    fru_table_offset = tc_fru_table_offset;
    platform_fru_table[fru_table_offset].pid_string = testcard_10gkr_pid;
    platform_fru_table[fru_table_offset].location_string = nim_10gkr_loc;
    cterr_add_component("Loopback test", "10G-KR Testcard NIM TLK10232");
    cterr_add_debug("Packet loopback path: ",
                    "BMC->6320->X710->GE switch->NIM TLK10232(loopback)",
                    "Check GE switch loopback test",
                    "Check GE switch vlan setting",
                    "Check X710 loopback test",
                    "Check 6320 loopback test",
                    "Check NC setting for switch vlan setup",
                    "Check NIM FPGA reg test for verify I2C to NIM");

    for (test_port = 0; test_port < TC_MAX_ETH_PORT; test_port++) {
        
        testname("TestCard GE%d 10G-KR Internal loopback", test_port);

        while (retry > 0) {
            set_tlk10232_port_int_lpbk(test_port);

            /* check link status on gesw */
            rc = tc_tlk10232_chk_gesw_link_status(test_port);
            if (rc == FAILED) {
                printf("link up failed. retry to bring up link again... ");
                retry--; 
                continue;
            } 

            /* Do SGMII loopback test. */
            prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

#if 0
            ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port(); 

            main_result = sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt);
#endif 
            if (eth_pkt_txrx(ETH1_MAC1, num_pkt, FALSE) == FAILED) {
                main_result = FAILED;
            } else {
                main_result = PASSED;
            }
            if (main_result != PASSED) {
                printf("\n%s: Failed To do TestCard GE%d loopback %s mode "
                          "(from Host side %s)", __FUNCTION__,
                          test_port, "1G-KX", ETH1_MAC1);
                printf("retry again...\n");
                retry--;
            } else {
                break;
            }
        } 

        if (retry == 0) {
            cterr('f', 0, "%s: Failed on TestCard GE%d", __FUNCTION__, test_port);
        }

        /* restore/clean up setting */
        prpass(testpass, "clean up and restore setting.. ");
        tc_tlk10232_cleanup(test_port); 

        if (main_result != PASSED) {
            return (FAILED);
        }

    }  /* test_port */

    return (main_result);
}


/********************************************************************
 *
 * Function: set_port_1gkx_lpbk
 *
 * Description: this function set port to 1gkx  internal loopback
 *
 * Inputs      : port - ge port
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************/
static void set_port_1gkx_lpbk (int port) {

    int path, smi_addr;

    path = SAME_HS_INPUT;
    if (port == 0) {
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }

    /* ti setting for init tlk chip */
    tc_tlk10232_init_10gkr_ti_setting();

    /* GE1 for 1g-kx mode, need to disable ge0 link and setup ge1 link */
    /* disable the other port power to bypass conflict issue */
    if (port == 1) {
        tc_tlk10232_set_mode(TC_TLK10232_CHB_ADDR);
        tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
    } else {
        /* port 0 also 1g-kx */
        tc_tlk10232_set_mode(TC_TLK10232_CHA_ADDR);
        tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
    }

    /* set path and loobpack */
    tc_config_tlk10232_path(smi_addr, path);

    /* non-GH plat link up after setup loopback */
    tc_config_tlk10232_loopback(smi_addr, ENABLE);

    return;
}

/********************************************************************
 *
 * Function: set_port_10gkr_lpbk
 *
 * Description: this function set port to 10gkr  internal loopback
 *
 * Inputs      : port - ge port
 * Outputs     : PASSED / FAILED
 *
 ********************************************************************/
static void set_port_10gkr_lpbk (int port) {

    int path, smi_addr;

    path = SAME_HS_INPUT;
    if (port == 0) {
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }

    /* ti setting for init tlk chip */
    tc_tlk10232_init_10gkr_ti_setting();

    if (port == 1) {
        tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
    } else {
        tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
    }

    /* set path and loobpack */
    tc_config_tlk10232_path(smi_addr, path);

    /* non-GH plat link up after setup loopback */
    tc_config_tlk10232_loopback(smi_addr, ENABLE);

    return;
}


/******************************************************************************
 *
 * Function: tc_tlk10232_init_10gkr_ti_setting
 *
 * Description: this function init tlk10232 via the configuration from TI.
 *
 * Inputs      : None 
 * Outputs     : None
 *
 *****************************************************************************/
void tc_tlk10232_init_10gkr_ti_setting (void)
{
    int dev_addr, regnum;
    int cha_addr = TC_TLK10232_CHA_ADDR;
    ushort rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */ 
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x2000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0008);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x024d);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL; /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9001;      /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0xe888);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x5252);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x8000;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0001);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x0200;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval);

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x01 */
    regnum = TLK10232_LT_TRAIN_CONTROL;   /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0003);


    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_restore_setting
 *
 * Description: this function restore the reigster value to default one. 
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_restore_setting (void)
{
    int dev_addr, regnum;
    int cha_addr = TC_TLK10232_CHA_ADDR;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0610);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0049);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL; /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9001;      /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0e02);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0xa048);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1100);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x4001);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000);

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x01 */
    regnum = TLK10232_LT_TRAIN_CONTROL;   /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610);

    return;
}

/******************************************************************************
 *
 * Function: load_file_for_cfg
 *
 * Description: this function using file 'cfg.log' to configure the 
 *              TLK and BCM chip 
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
static void load_file_for_cfg (void)
{
    char str1[8], str2[8];
    int smi_addr, dev_addr, regnum, tmp;
    ushort val, rdval;
    int is_rd = 0, is_bcm = 0;
    FILE * fp;

    printf("usage:\n");
    printf("the file name should be \'cfg.log\' \n");
    printf("format: [chip] [r/w/b] [smi_addr] [dev_addr] [regnum] [val] \n");
    printf("the [val] should be assigned to any value even the operation is read \n");
    printf("the [r/w/b] \'b\' is read then write (using \'or\') \n");
    printf("e.g.:    b      r     0x1        0x2        0xFFFF   0xFFEE\n");
    printf("read bcm chip smi addr 1 (channel 1), dev_addr 2, reg 0xFFFF, 0xFFEE for fill dummy\n");
    printf("e.g.:    t      w     0x2        0x3        0xEEEE   0xAAAA\n");
    printf("write tlk chip smi addr 2 (channel 2), dev_addr 3, reg 0xEEEE, with val = 0xAAAA \n");
    printf("e.g.:    t      b     0x2        0x3        0xEEEE   0xAAAA\n");
    printf("for tlk chip smi addr 2 (channel 2), dev_addr 3;\n"
           "read reg 0xEEEE, then or with 0xAAAA \n");

    fp = fopen ("cfg.log", "rw+");

    while (fscanf(fp, "%s %s %x %x %x %x", str1, str2, &smi_addr, &dev_addr, &regnum, &tmp) == 6) {

       if (strcmp(str1, "b") == 0) { 
           is_bcm = 1;
       } else if (strcmp(str1, "t") == 0) {
           is_bcm = 0;
       } else {
           printf("Not support format \'%s\' \n"
                  " \'b\' for bcm chip  \n "
                  " \'t\' for tlk10232 chip  \n", str1);
           fclose(fp);
           return;
       } 

       if (strcmp(str2, "r") == 0) { 
           is_rd = 1;
       } else if (strcmp(str2, "w") == 0) { 
           is_rd = 0;
       } else if (strcmp(str2, "b") == 0) {
           is_rd = 2;
       }else {
           printf("Not support format \'%s\' \n"
                  " \'r\' for read  \n "
                  " \'w\' for write  \n", str2);
           fclose(fp);
           return;
       } 

       val = tmp;

       if (is_rd == 1) {
           if (is_bcm == TRUE) {
               printf("bcm rd,");
               tc_xaui_phy_reg_rd(smi_addr, dev_addr, regnum, &val);
           } else {
               printf("tlk rd,");
               tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &val);
           }
           printf("Current value of smi addr 0x%x dev 0x%x reg 0x%x = 0x%x\n",
               smi_addr, dev_addr, regnum, val);
           
       } else if (is_rd == 0) { 
           if (is_bcm == TRUE) {
               printf("bcm wr,");
               tc_xaui_phy_reg_wr(smi_addr, dev_addr, regnum, val);
           } else {
               printf("tlk wr,");
               tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, val);
           }
           printf("smi addr 0x%x dev 0x%x reg 0x%x = 0x%x\n",
               smi_addr, dev_addr, regnum, val);
       } else {
           if (is_bcm == TRUE) {
               printf("bcm wb,");
               tc_xaui_phy_reg_rd(smi_addr, dev_addr, regnum, &val);
               val = rdval | val;
               tc_xaui_phy_reg_wr(smi_addr, dev_addr, regnum, val);
           } else {
               printf("tlk wb,");
               tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);
               val = rdval | val;
               tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, val);
           }
           printf("smi addr 0x%x dev 0x%x reg 0x%x = 0x%x\n",
               smi_addr, dev_addr, regnum, val);
       }
    }

    fclose(fp);
    return;
}

/*------------------------------------------------------------------
$Log: testcard_tlk_10232.c,v $
Revision 1.3  2017/03/30 08:34:09  hondwang
Tachi-L brach merge

Revision 1.2.14.1  2017/02/14 01:00:00  hondwang
Change Test card from 1G-KX to 10G-KR

Revision 1.2  2016/04/20 11:25:32  benchen2
add tachi fru portion

Revision 1.1.2.9  2016/03/04 09:40:53  alpeng
update testcard enhance err msg

Revision 1.1.2.8  2015/12/30 08:37:23  alpeng
 support nc test, check intel and lewis ready before testing

Revision 1.1.2.7  2015/11/23 11:02:20  alpeng
update error msg

Revision 1.1.2.6  2015/10/28 06:18:17  alpeng
update util for en/disable loobpack

Revision 1.1.2.5  2015/10/27 09:45:19  alpeng
update utility for testcard

Revision 1.1.2.4  2015/10/14 09:12:00  alpeng
support new testcard for p1b

Revision 1.1.2.3  2015/10/01 09:20:35  alpeng
update testcard eth test to send packets from bmc eth1

Revision 1.1.2.2  2015/08/21 06:46:29  alpeng
support ge/xaui test for testcard; clean up repo;

Revision 1.1.2.1  2015/07/31 10:40:04  alpeng
first check in for testcard



$Endlog$
*/

