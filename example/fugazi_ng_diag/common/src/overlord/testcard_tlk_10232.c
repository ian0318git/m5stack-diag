/* $Id: testcard_tlk_10232.c,v 1.22 2020/05/22 02:28:35 qingcwan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/testcard_tlk_10232.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_tlk_10232.c
 *
 * Description: TestCard Ethernet related diag tests and utilities.
 *              tlk_10232 chip supports 10G-KR for greyhound GE
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
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
#include "i2c_address.h"
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "testcard_tlk_10232.h"
#include "testcard_xaui.h" /* for bcm chip config */
#include "bcm_gesw_defs.h" /* for bcm sdk */
#include "platform_eth_pkt_txrx.h" /* for get sgmii port num */
#include "cookie_4.h"
#include "cross_platform.h" /* WIC_MODULE, SM_MODULE */


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
static void tc_tlk10232_path_select(void);
static void set_tlk10232_port_int_lpbk(int); 
static void set_tlk10232_port_int_lpbk_util(int); 
static void tc_tlk10232_clear_int_lpbk(void);
static int tmp_send_pkt_util(void);
static void tc_tlk10232_reg_dump(void);
static void tc_tlk10232_init_1gkx_setting(int);
static void tc_tlk10232_init_10g_setting(int); 
static void load_file_for_cfg(void);
static int tc_tlk10232_chk_link_speed(int);
static void ext_sfp_lpbk_test(void);
static void toggle_tc_config_tlk10232_loopback(int); 


int tc_tlk10232_internal_loopback_test(int);
void tc_tlk10232_cleanup(int);
void tc_tlk10232_set_mode(int);
void tc_tlk10232_power_off(int);
void tc_tlk10232_init_10gkr_ti_setting(void);

static int c2ru_tc_tlk10232_internal_loopback_test (int dummy);

/******************************************************************************* 
 *                                   Externs                                   *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern int tc_fpga_reg_rd(uint32_t, uint16_t *);
extern int tc_fpga_reg_wr(uint32_t, uint16_t);
extern int sgmii_lpbk_util(int, int);

extern ngio_if        *tc_ngio_p;

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
    {"TLK 10232 tx path select",
     (PFT)tc_tlk10232_path_select,       0,        0,
     (PFT)0,                          0,   (PFT)0, 0},
    {"TLK 10232 clear internal loopback ",
     (PFT)tc_tlk10232_clear_int_lpbk,    0,        0,
     (PFT)0,                          0,   (PFT)0, 0},
    {"Access TLK 10232 Register",
     (PFT)tc_tlk10232_reg_access,     0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Dump TLK 10232 Register",
     (PFT)tc_tlk10232_reg_dump,     0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Send packet host SGMII -> gesw -> NGIO(DBG)",
     (PFT)tmp_send_pkt_util,  CPU_SGMII_PORT1,  0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port0 to 1g-kx mode and lpbk at BCM(DBG)", 
     (PFT)tc_tlk10232_init_1gkx_setting,      0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port1 to 1g-kx mode and lpbk at BCM(DBG)", 
     (PFT)tc_tlk10232_init_1gkx_setting,      1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port0 to 10g mode and lpbk at BCM(DBG)", 
     (PFT)tc_tlk10232_init_10g_setting,      0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port1 to 10g mode and lpbk at BCM(DBG)", 
     (PFT)tc_tlk10232_init_10g_setting,      1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port0 lpbk at TLK(DBG)", 
     (PFT)set_tlk10232_port_int_lpbk_util,        0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port1 lpbk at TLK(DBG)", 
     (PFT)set_tlk10232_port_int_lpbk_util,        1,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"init TI setting (DBG)", 
     (PFT)tc_tlk10232_init_10gkr_ti_setting,  0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Load file for configure tlk and bcm (DBG)", 
     (PFT)load_file_for_cfg,  0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"SFP loopback test (need ext SFP plug-in)(DBG)", 
     (PFT)ext_sfp_lpbk_test,  0,        0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port0 to 1G", 
     (PFT)tc_tlk10232_set_mode,       0xa, 0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"Set port1 to 1G", 
     (PFT)tc_tlk10232_set_mode,       0xb, 0, 
     (PFT)0,                          0,   (PFT)0, 0},
    {"toggle port0 loopback on TLK", 
     (PFT)toggle_tc_config_tlk10232_loopback, 0xa, 0,
     (PFT)0,                          0,   (PFT)0, 0},
    {"toggle port1 loopback on TLK",
     (PFT)toggle_tc_config_tlk10232_loopback, 0xb, 0, 
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

int tmp_send_pkt_util (void)
{
    int main_result, ctrl_plane_sgmii_port; 

    ctrl_plane_sgmii_port = get_sgmii_port_num(testcard_if_p->slot, TYPE_SWITCH);

    printf("default eth port is eth%d \n", ctrl_plane_sgmii_port); 
    ctrl_plane_sgmii_port = getdec_answer("Enter new eth port num :", 0x0, 0x0, 10);
    printf("Now eth port is eth%d \n", ctrl_plane_sgmii_port); 

    main_result = sgmii_lpbk_util(ctrl_plane_sgmii_port, 300);
    if (main_result != PASSED) {
        printf("\n%s: Failed To send packet to testcard  "
                      "(from Host side SGMII%d) \n",
                      __FUNCTION__, ctrl_plane_sgmii_port);
        return FAILED;
    } else { 
        printf("Passed - 300 packets loopback \n"); 
        return PASSED;  
    }
}

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
    unsigned int slot;
    slot = testcard_if_p->slot;

    if ((slot == NGSM4_SLOT) && (is_neptune() || is_vg450())) {
        printf("Neptune NGSM4 is not support GE ports connection \n");
        return; 
    }

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

    /* select transmit source, default from Channel LS */
    tc_tlk10232_path_reset(smi_addr);

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_path_select
 *
 * Description: a wrapper for calling config_tlk10232_path
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_path_select (void)
{
    int path, smi_addr;
    printf("TLK10232 DST (transmit) path selection\n");

    smi_addr = gethex_answer("CHA addr 0xa; CHB addr 0xb", 0xa, 0xa, 0xb);
    printf("SAME_LS_INPUT = 0; SAME_HS_INPUT = 1; \n"
           " ALT_LS_INPUT = 2; ALT_HS_OUTPUT = 3; \n");
    path = getdec_answer("Enter value", 0, 0, 3);

    tc_config_tlk10232_path(smi_addr, path);

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

    if (is_curie_1ru()) { 
        /* keep auto neg on, enable auto-neg ability for 1g (7.0x11=0x20) */
        dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
        regnum = 0x11;
        tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x20);
    } else {
        dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
        regnum = TLK10232_AN_CONTROL;  /* 0x0 */
        tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x2000);
    }

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;          /* 0xE */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x0008);

    return;
}

/******************************************************************************
 *
 * Function: set_tlk10232_port_int_lpbk_util
 *
 * Description: this util set tlk10232 to internal loopback 
 *              from GE switch to tlk10232 ge port
 *
 * Inputs      : port - ge port 
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
static void set_tlk10232_port_int_lpbk_util (int port) {

    int path, smi_addr; 
    int ntc_gesw_chk; 

    path = SAME_HS_INPUT;
    if (port == 0) {
        ntc_gesw_chk = ntc_gesw_p0_type; 
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        ntc_gesw_chk = ntc_gesw_p1_type; 
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }
    
    /* ntc_ntpn_chk() for Neptune series ngio special setting */
    if (is_goldbeach() || ntc_gesw_chk) {
        /* Goldbeach used GE0 for 1g-kx mode, need to disable ge1 link and setup ge0 link */
        /* disable the other port power to bypass conflict issue */
        if (port == 0) {
            prpass(testpass, "disable GE1 link state ");
            prpass(testpass, "config GE0 as 1g-kx mode ");
            tc_tlk10232_set_mode(TC_TLK10232_CHA_ADDR);
            tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
        } else {
            tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
        }
    } else {
        /* GE1 for 1g-kx mode, need to disable ge0 link and setup ge1 link */
        /* disable the other port power to bypass conflict issue */
        if (port == 1) {
            prpass(testpass, "disable GE0 link state ");
            prpass(testpass, "config GE1 as 1g-kx mode ");
            tc_tlk10232_set_mode(TC_TLK10232_CHB_ADDR);
            tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
        } else {
            tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
        }
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
    int ntc_gesw_chk; 

    path = SAME_HS_INPUT;
    if (port == 0) {
        ntc_gesw_chk = ntc_gesw_p0_type; 
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        ntc_gesw_chk = ntc_gesw_p1_type; 
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }
    
    /* ti setting for init tlk chip */
    prpass(testpass, "Init TLK10232 state...");
    tc_tlk10232_init_10gkr_ti_setting(); 

    /* ntc_ntpn_chk() for Neptune series ngio special setting */
    if (is_goldbeach() || ntc_gesw_chk) {
        /* Goldbeach used GE0 for 1g-kx mode, need to disable ge1 link and setup ge0 link */
        /* disable the other port power to bypass conflict issue */
        if (port == 0) {
            prpass(testpass, "disable GE1 link state ");
            prpass(testpass, "config GE0 as 1g-kx mode ");
            tc_tlk10232_set_mode(TC_TLK10232_CHA_ADDR);
            tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
        } else {
            tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
        }
    } else {
        /* GE1 for 1g-kx mode, need to disable ge0 link and setup ge1 link */
        /* disable the other port power to bypass conflict issue */
        /* Curie GE1 is also 10G-KR */
        if (port == 1) {
            prpass(testpass, "disable GE0 link state ");
            prpass(testpass, "config GE1 as 1g-kx mode ");
            tc_tlk10232_set_mode(TC_TLK10232_CHB_ADDR);
            tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
        } else {
            tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
        }
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

    int ge_port, status, tgt_device, slot, cnt = 20;

    if (testcard_if_p->type == TC_NGSM) {
        tgt_device = TGT_DEV_NGSM;
    } else {
        tgt_device = TGT_DEV_NGWIC;
    }

    slot = testcard_if_p->slot;

    ge_port = ovld_get_ge_sw_port_num(slot, tgt_device, test_port);

    while (cnt) {
        /* the return value is used for check gesw chip status,
         * we don't need it
         */
        bcm_gesw_ge_link_status_get(bcm_uid, ge_port, &status); 

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
        return (FAILED); 
    } else {
        return (PASSED);
    }

}

/******************************************************************************
 *
 * Function: tc_tlk10232_clear_int_lpbk
 *
 * Description: this function set register to default value. 
 *
 * Inputs      : None
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_clear_int_lpbk (void) {

    int dev_addr, regnum;
    ushort  wrval;

    /* 1. MODE_SEL. 0x1E.0x0001 */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_CHANNEL_CTRL;        /* 0x1 */

    wrval = 0xB00; /* default value */
    tc_tlk10232_mdio_wr(TC_TLK10232_CHA_ADDR, dev_addr, regnum, wrval);
    tc_tlk10232_mdio_wr(TC_TLK10232_CHB_ADDR, dev_addr, regnum, wrval);


    /* 2. auto-neg. 0x7.0x0000. bit12 */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CTRL;              /* 0x0 */

    wrval = 0x3000;
    tc_tlk10232_mdio_wr(TC_TLK10232_CHA_ADDR, dev_addr, regnum, wrval);
    tc_tlk10232_mdio_wr(TC_TLK10232_CHB_ADDR, dev_addr, regnum, wrval);

    printf("Set both Channel A and B \n");
    printf("dev_addr:0x1E, reg:1 = 0xB00\n");
    printf("dev_addr:0x7,  reg:0 = 0x3000\n");

    return;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_chk_link_speed               
 *
 * Description: check tlk10232 link status 
 *
 * Inputs      : port  
 * Outputs     : PASSED / FAILED
 *
 */
static int tc_tlk10232_chk_link_speed (int port) {

    int mask, smi_addr;
    int ntc_gesw_chk;
    ushort rdval; 

    if (port == NGIO_GE0) {
        ntc_gesw_chk = ntc_gesw_p0_type;
        smi_addr = TC_TLK10232_CHA_ADDR; /* 0xA */
    } else {
        ntc_gesw_chk = ntc_gesw_p1_type;
        smi_addr = TC_TLK10232_CHB_ADDR;  /* 0xB */
    }

    if (ntc_gesw_chk == FALSE) {   /* FALSE for 10g */
       mask = TLK10232_AN_10GKR; 
    } else {
       mask = TLK10232_AN_1GKX; 
    }

    if ((is_curie_1ru()) && (port == NGIO_GE1)) { 
        if (ntc_gesw_chk == FALSE) {   /* FALSE for 1g */
            mask = TLK10232_AN_1GKX; 
        } else {
            mask = TLK10232_AN_10GKR; 
        }
        msleep(100);/* refine link problem */
    }

    tc_tlk10232_mdio_rd(smi_addr, TLK10232_AUTO_NEG_DEV_ADDR, TLK10232_AN_BP_STATUS, &rdval);
    if (rdval & mask) {
       prpass(testpass, "port link up on %s", (mask == TLK10232_AN_1GKX) ? "1G-KX" : "10G-KR"); 
       return (PASSED); 
    } else {
       return (FAILED); 
    }

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
    int num_pkt = 300, main_result = FAILED;
    int test_port = 0;
    int ctrl_plane_sgmii_port;
    int retry = RETRY_CNT_10G_KR_TC, rc;
    int tc_max_eth_port = 0, test_both_speed = FALSE;

    if (is_curie_2ru())
        return c2ru_tc_tlk10232_internal_loopback_test(dummy);

    if (is_goldbeach()) {
        tc_max_eth_port = GB_TC_MAX_ETH_PORT;
    } else {
        tc_max_eth_port = TC_MAX_ETH_PORT;
    }

    /* Curie 1RU NIM only has 1 ge intf */
    if (is_curie_1ru()) {
        if (testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_NIM) { 
            tc_max_eth_port = CURIE_NIM_MAX_ETH_PORT; 
        }
    }

    for (test_port = 0; test_port < tc_max_eth_port; test_port++) {

        if (is_curie_1ru()) {
            test_both_speed = TRUE;  /* 10G and 1G */
            testname("TestCard GE%d Internal loopback", test_port); 
        } else {
            testname("TestCard GE%d %s Internal loopback",
            test_port, (test_port & 1) ? "1G-KX" : "10G-KR" );
        }

second_speed: 

        while (retry > 0) {
            /* set speed and loobpack on tlk10232 */
            set_tlk10232_port_int_lpbk(test_port);

            /* get host eth port for send packet */ 
            ctrl_plane_sgmii_port = get_sgmii_port_num(testcard_if_p->slot, TYPE_SWITCH);

            if (is_curie_1ru()) {
                if ((testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_SM) || 
                    (testcard_if_p->is_10gkr == TC_10GKR_SM_ON_SM)) {  
                    if (test_port != 0 ) { /* SM GE0 is eth6, GE1 is eth8 */
                        ctrl_plane_sgmii_port++;
                        ctrl_plane_sgmii_port++;
                    }
                }
            } 

            /* 0. check link status on host eth port */
            /* isolate this check, since previous projects are FCS. */
            if (ntc_chk_eth_linkup(ctrl_plane_sgmii_port) == FALSE) { 
                printf("\nlink up failed on host eth port. retry to bring up link again.\n");
                retry--; 
                continue;
            } 

            /* 1. check link status on gesw */
            if (!(is_goldbeach()) && !(is_curie_1ru())) {
                /* check link status on gesw */
                rc = tc_tlk10232_chk_gesw_link_status(test_port);
                if (rc == FAILED) {
                    printf("\nlink up failed on gesw. retry to bring up link again.\n");
                    retry--; 
                    continue;
                } 
            } 

            /* 2. check link status on tlk10232 */
            rc = tc_tlk10232_chk_link_speed(test_port);
            if (rc == FAILED) {
                printf("\nlink speed is not matched on tlk10232, retry again.\n");
                retry--;
                continue;
            }

            /* Do SGMII loopback test. */
            prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

            /* all mediums are checked, send packet now */
            main_result = sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt);
            if (main_result != PASSED) {
                printf("\n%s: Failed To do TestCard GE%d loopback %s mode "
                          "(from Host side SGMII%d)", __FUNCTION__,
                          test_port, (test_port & 1) ? "1G-KX" : "10G-KR",
                          ctrl_plane_sgmii_port);
                printf("\nretry again...\n");
                retry--;
            } else {
                break;
            }
        } /* while */

        if (retry == 0) {
            cterr('f', 0, "%s: Failed on TestCard GE%d", __FUNCTION__, test_port);
            main_result = FAILED; 
        }

        /* restore/clean up setting */
        prpass(testpass, "clean up and restore setting.. ");
        tc_tlk10232_cleanup(test_port); 

        if (main_result != PASSED) {
            return (FAILED);
        }

        if (test_both_speed) { 
            if (test_port == NGIO_GE0) {
                ntc_gesw_p0_type = ~ntc_gesw_p0_type;
            } else { 
                ntc_gesw_p1_type = ~ntc_gesw_p1_type;
            }
            test_both_speed = FALSE; 
            goto second_speed; 
        }

    }  /* test_port */

    return (main_result);
}

/******************************************************************************
 *
 * Function: tc_tlk10232_init_1gkx_setting
 *
 * Description: this function set ge port speed to 1g-kx mode 
 *              and set bcm loopback on
 *
 * Inputs      : is_chb - config channel b
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_init_1gkx_setting (int is_chb)
{
    int ch1_addr = TC_TLK10232_CHA_ADDR;
    int ch2_addr = TC_TLK10232_CHB_ADDR;
    int test_port = 0; 

    tc_tlk10232_init_10gkr_ti_setting();

    if (is_chb == TRUE) {
        test_port = 1; 
        ch1_addr = TC_TLK10232_CHB_ADDR;
        ch2_addr = TC_TLK10232_CHA_ADDR;
    }

    tc_tlk10232_set_mode(ch1_addr);
    tc_tlk10232_power_off(ch2_addr); 

    /* set xaui address, dev_addr = 0x4, reg 0x8000, val 0x262E */
    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x262E);

    /* set xaui address, dev_addr = 0x4, reg 0x8017, val 0x00F0 */
    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8017, 0x00F0);

    return;

}

/******************************************************************************
 *
 * Function: tc_tlk10232_init_10g_setting
 *
 * Description: this function set ge port speed to 10g-kr mode
 *              and set bcm loopback on
 *
 * Inputs      : None - 10g-kr available on ge0 only.
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_init_10g_setting (int is_chb)
{
    int ch2_addr = TC_TLK10232_CHB_ADDR;
    int test_port = 0; 

    /* ch1 is already set up to 10g on ti init setting */
    tc_tlk10232_init_10gkr_ti_setting();

    if (is_chb == TRUE) {
        test_port = 1; 
        ch2_addr = TC_TLK10232_CHA_ADDR;
    }

    tc_tlk10232_power_off(ch2_addr); 
    
    /* set xaui address , dev_addr = 0x4, reg 0x8000, val 0x206E */
    tc_xaui_phy_reg_wr(test_port, 0x4, 0x8000, 0x206E);

    return; 
}

/******************************************************************************
 *
 * Function: curie1ru_tc_tlk10232_init_10gkr_ti_setting
 *
 * Description: the init setting is for curie1ru only, not backward compatible 
 *              for previous platform. 
 *
 * Inputs      : None 
 * Outputs     : None
 *
 *****************************************************************************/
void curie1ru_tc_tlk10232_init_10gkr_ti_setting (void)
{
    int dev_addr, regnum;
    int cha_addr = TC_TLK10232_CHA_ADDR;
    ushort rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610); /* global reset */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */ 
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x2000); /* disable auto nego */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* disable link training */

    tc_tlk10232_mdio_wr(cha_addr, 1, 0xab, 1);  /* enable 10G-KR FEC */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0008); /* channel data path reset  */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x024d); /* magic register 0x9000 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL; /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* enable default tx trigger*/

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* trigger loading default HS TX setting value */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* normal op for tx trigger  */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9001;      /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200); /* magic register 0x9001 */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002); /* magic registe 0x96 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00); /* magic register 0x9005 */

    /* 30.3 and 30.4 are 9848 and 3502 - newly value tune by CRDC Jack. */ 
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x9848); 
    /* force attenuator off, HS swing change from 1020 to 1340 mVdfpp */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3502); 
   /* HS_TWCRF : cursor reduction to 22%, default is 0%; HS serdes PEAK_disable;
      HS clock data recovery alg -Sixteen vote threshold -> four vote threshold ;
      HS clock data recovert alg freq from 2x mode to 1x mode */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x8000;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* auto neg FEC req */

    /* newly add, 0x80 means autoneg support 10G */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
    regnum = 0x11; /* magic register */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x80);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000); /* enable auto nego */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x0200;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* restart auto nego */

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

    /* we keep old setting to prevent effect previous platforms 
     * curie1ru need to use new setting for improvement TLK10232 connection */
    if (is_curie_1ru()) {
        return (curie1ru_tc_tlk10232_init_10gkr_ti_setting()); 
    } 

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610); /* global reset */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x2000); /* disable auto nego */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* disable link training */

    tc_tlk10232_mdio_wr(cha_addr, 1, 0xab, 1);  /* enable 10G-KR FEC */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0008); /* channel data path reset  */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x024d); /* magic register 0x9000 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL; /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* enable default tx trigger*/

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* trigger loading default HS TX setting value */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* normal op for tx trigger  */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9001;      /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200); /* magic register 0x9001 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002); /* magic registe 0x96 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00); /* magic register 0x9005 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0xe888);
    /* force attenuator off, HS swing change from 1020 to 1340 mVdfpp */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x5252);
   /* HS_TWCRF : cursor reduction to 22%, default is 0%; HS serdes PEAK_disable;
      HS clock data recovery alg -Sixteen vote threshold -> four vote threshold ;
      HS clock data recovert alg freq from 2x mode to 1x mode */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x8000;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* auto neg FEC req */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0001);  /* bit0 for TI use only */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000); /* enable auto nego */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x0200;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* restart auto nego */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x01 */
    regnum = TLK10232_LT_TRAIN_CONTROL;   /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0003); /* link training enable and restart */


    return;
}

/* Curie 2RU Crosspoint Related APIs Start
 * Curie 2RU runs PRBS between 2 SM test cards to test crosspoint
 * When crosspoint opened, TLK10232s are conncted together and
 * corresponding host MAC ports are directly connected. */

/* wrapper for register utilities */
void tc_tlk10232_reg_rw(TLK10232_REGISTER_TEST_TYPE_E type)
{
    switch (type) {
        case TLK10232_REGISTER_READ_WRITE_TEST:
            tc_tlk10232_reg_access();
            break;
        case TLK10232_REGISTER_DUMP_TEST:
            tc_tlk10232_reg_dump();
            break;
    }
}

/******************************************************************************
 *
 * Function: tc_tlk10232_check_status
 *
 * Description: this function checks target status
 *
 * Inputs      : channel_addr - MDIO address.
                 type - status type
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_check_status(int channel_addr, TLK10232_STATUS_TYPE_E type)
{
    int rc;
    int dev_addr, regnum;
    ushort rdval = 0;

    switch (type) {
    case TLK10232_HS_AZ_DONE:
        dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        regnum = TLK10232_CHANNEL_STATUS_1;     /* 0xF */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = (rdval & REG_BIT(12)) ? PASSED : FAILED;
        break;
    case TLK10232_AGC_LOCKED:
        dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        regnum = TLK10232_CHANNEL_STATUS_1;     /* 0xF */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = (rdval & REG_BIT(11)) ? PASSED : FAILED;
        break;
    case TLK10232_PLL_STATUS_LOCKED:
        dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        regnum = TLK10232_CHANNEL_STATUS_1;     /* 0xF */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc =((rdval & REG_BIT(1)) && (rdval & REG_BIT(0))) ? PASSED : FAILED;
        break;
    case TLK10232_AUTO_NEGOTIATION:
        dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
        regnum = TLK10232_AN_STATUS;     /* 0x1 */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = ( (rdval & REG_BIT(5)) && (rdval & REG_BIT(2)) ) ? PASSED : FAILED;
        break;
    case TLK10232_LINK_TRAINING:
        dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
        regnum = TLK10232_LT_TRAIN_STATUS;     /* 0x97 */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = ( !(rdval & REG_BIT(3)) && (rdval & REG_BIT(0)) ) ? PASSED : FAILED;
        break;
    case TLK10232_KR_MODE:
        dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
        regnum = TLK10232_AN_BP_STATUS;     /* 0x30 */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = (rdval & REG_BIT(3)) ? PASSED : FAILED;
        break;
    case TLK10232_KR_PCS_RX_LINK_STATUS:
        dev_addr = TLK10232_PCS_DEV_ADDR;  /* 0x3 */
        regnum = TLK10232_KR_PCS_STATUS_1;     /* 0x20 */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = (rdval & REG_BIT(12)) ? PASSED : FAILED;
        break;
    case TLK10232_KR_PCS_BLOCK_LOCK:
        dev_addr = TLK10232_PCS_DEV_ADDR;  /* 0x3 */
        regnum = TLK10232_KR_PCS_STATUS_1;     /* 0x20 */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        rc = (rdval & REG_BIT(0)) ? PASSED : FAILED;
        break;
    default :
        rc = FAILED;
        break;
    }

    return rc;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_set_1gkx_mode
 *
 * Description: this function sets 1G-KX mode
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static int tc_tlk10232_set_1gkx_mode(int channel_addr)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    /* reset device */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;          /* 0x0 */
    wrval = 0x8610;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* REFCLK input frequency and selection */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_CH_CONTROL_1;          /* 0x1D */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_CHANNEL_CTRL;          /* 0x1 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(1);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* Mode selection */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_CH_CONTROL_1;          /* 0x1D */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval | REG_BIT(13);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_CHANNEL_CTRL;    /* 0x1 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(11);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_SERDES_CTRL_1;    /* 0x2 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(3);
    wrval = wrval | REG_BIT(2) | REG_BIT(1) | REG_BIT(0);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* HS Serdes settings */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    wrval = 0xa848;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    wrval = 0x1500;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_HS_SERDES_CTRL_4;    /* 0x5  */
    wrval = 0x2000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* Reserved Register settings */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_MC_AUTO_CONTROL;          /* 0x8021 */
    wrval = 0x001F;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* Issue Data path Reset */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;    /* 0xE  */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval | REG_BIT(3);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    msleep(1000);

    return PASSED;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_set_10gkr_mode
 *
 * Description: this function sets 10G-KR mode
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static int tc_tlk10232_set_10gkr_mode(int channel_addr)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    /* reset device */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;          /* 0x0 */
    wrval = 0x8610;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* REFCLK input frequency and selection */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_CH_CONTROL_1;          /* 0x1D */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_CHANNEL_CTRL;          /* 0x1 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(1);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* Default HS Tx settings loading and LT controls */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    wrval = 0x2000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;    /* 0x96 */
    wrval = 0x0000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;    /* 0xE */
    wrval = 0x0008;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX9000;    /* 0x9000 */
    wrval = 0x024D;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_TRIGGER_EN_CTRL;    /* 0x8101 */
    wrval = 0x0004;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;    /* 0x8100 */
    wrval = 0x0004;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_TRIGGER_LOAD_CTRL;    /* 0x8100 */
    wrval = 0x0000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX9001;    /* 0x9001 */
    wrval = 0x0201;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    wrval = 0x3000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;          /* 0x96 */
    wrval = 0x0002;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX9005;          /* 0x9005 */
    wrval = 0x1c00;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    /* HS Serdes settings */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    wrval = 0xe848;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    wrval = 0x1500;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_HS_SERDES_CTRL_4;    /* 0x5  */
    wrval = 0x39a0;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    /* Issue AN_RESTART */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;    /* 0x0  */
    wrval = 0x3200;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    msleep(1000);

    return PASSED;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_set_10gkr_no_an_mode
 *
 * Description: this function sets 10G-KR without AN mode
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static int tc_tlk10232_set_10gkr_no_an_mode(int channel_addr)
{
    int dev_addr, regnum;
    ushort  wrval;

    /* reset device */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;          /* 0x0 */
    wrval = 0x8610;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* disable  FEC*/
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    wrval = 0x2000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;    /* 0x96 */
    wrval = 0x0000;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* enable FEC */
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_KR_FEC_CONTROL;    /* 0xab */
    wrval = 0x0003;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_TI_RESERVED_CONTROL; /* 0x8020 */
    wrval = 0x03ff;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    /* HS Serdes settings */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    wrval = 0xc848;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    wrval = 0x1542;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    regnum = TLK10232_HS_SERDES_CTRL_4;    /* 0x5  */
    wrval = 0x398c;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;    /* 0xE */
    wrval = 0x0008;
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    return PASSED;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_set_mode_type
 *
 * Description: this function sets mode
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_set_mode_type(int channel_addr, TLK10232_MODE_TYPE_E mode_type)
{
    int rc;

    switch (mode_type) {
    case TLK10232_MODE_1GKX:
        rc = tc_tlk10232_set_1gkx_mode(channel_addr);
        break;
    case TLK10232_MODE_10GKR:
        rc = tc_tlk10232_set_10gkr_mode(channel_addr);
        break;
    case TLK10232_MODE_10GKR_NO_AN:
        rc = tc_tlk10232_set_10gkr_no_an_mode(channel_addr);
        break;
    default:
        rc = FAILED;
        break;
    }

    return rc;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_set_prbs
 *
 * Description: this function configures PRBS 
 *
 * Inputs      : channel_addr - MDIO address.
                 prbs_type - PRBS type
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_set_prbs(int channel_addr, TLK10232_PRBS_PATTERN_TYPE_E prbs_type)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    /* Reserved Register settings */
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_MC_AUTO_CONTROL;          /* 0x8021 */
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, 0x001F);

    switch (prbs_type) {
    case TLK10232_PRBS_PATTERN_31:
        dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        regnum = TLK10232_LOOPBACK_TP_CTRL;          /* 0xB */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        wrval = rdval | REG_BIT(10);
        wrval |= REG_BIT(9);
        wrval |= REG_BIT(8);
        tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
        break;
    case TLK10232_PRBS_PATTERN_23:
        dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        regnum = TLK10232_LOOPBACK_TP_CTRL;          /* 0xB */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        wrval = rdval | REG_BIT(10);
        wrval |= REG_BIT(9);
        wrval &= ~REG_BIT(8);
        tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
        break;
    case TLK10232_PRBS_PATTERN_7:
        dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
        regnum = TLK10232_LOOPBACK_TP_CTRL;          /* 0xB */
        tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
        wrval = rdval | REG_BIT(10);
        wrval &= ~REG_BIT(9);
        wrval |= REG_BIT(8);
        tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
        break;
    default:
        break;
    }

    return PASSED;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_clear_1gkx_registers
 *
 * Description: this function clears 1G-KX settings
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_clear_1gkx_registers(int channel_addr)
{
    int dev_addr, regnum;
    ushort rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_CHANNEL_STATUS_1;     /* 0xF */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_ERROR_COUNTER;     /* 0x10 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_LN0_ERROR_COUNTER;     /* 0x11 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_STATUS_1;     /* 0x15 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_KX_STATUS_1;     /* 0xA1*/
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
}

/******************************************************************************
 *
 * Function: tc_tlk10232_clear_10gkr_registers
 *
 * Description: this function clears 10G-KR settings
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : None
 *
 *****************************************************************************/
static void tc_tlk10232_clear_10gkr_registers(int channel_addr)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_CHANNEL_STATUS_1;     /* 0xF */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_ERROR_COUNTER;     /* 0x10 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_LN0_ERROR_COUNTER;     /* 0x11 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_LN1_ERROR_COUNTER;     /* 0x12 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_LN2_ERROR_COUNTER;     /* 0x13 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_LN3_ERROR_COUNTER;     /* 0x14 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_CONFIG_CONTROL;     /* 0xC */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(13);
    wrval &= ~REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_STATUS_1;     /* 0x15 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_CONFIG_CONTROL;     /* 0xC */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval & ~REG_BIT(13);
    wrval |= REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_STATUS_1;     /* 0x15 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_CONFIG_CONTROL;     /* 0xC */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval | REG_BIT(13);
    wrval &= ~REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_STATUS_1;     /* 0x15 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_CONFIG_CONTROL;     /* 0xC */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval | REG_BIT(13);
    wrval |= REG_BIT(12);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LS_STATUS_1;     /* 0x15 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_PMA_STATUS_1;     /* 0x1 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK20232_PMA_STATUS_2;     /* 0x8 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_PCS_DEV_ADDR;  /* 0x3 */
    regnum = TLK10232_PCS_STATUS_1;     /* 0x1 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    dev_addr = TLK10232_PCS_DEV_ADDR;  /* 0x3 */
    regnum = TLK10232_PCS_STATUS_2;     /* 0x8 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
}

/******************************************************************************
 *
 * Function: tc_tlk10232_clear_latched_registers
 *
 * Description: this function clears 1G or 10G mode settings
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : None
 *
 *****************************************************************************/
void tc_tlk10232_clear_latched_registers(int channel_addr, TLK10232_MODE_TYPE_E mode_type)
{
    switch (mode_type) {
    case TLK10232_MODE_1GKX:
        tc_tlk10232_clear_1gkx_registers(channel_addr);
        break;
    case TLK10232_MODE_10GKR:
        tc_tlk10232_clear_10gkr_registers(channel_addr);
        break;
    default:
        break;
    }
}

/******************************************************************************
 *
 * Function: tc_tlk10232_clear_error_counters
 *
 * Description: this function clears error counters
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_clear_error_counters(int channel_addr)
{
    int dev_addr, regnum;
    ushort rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_ERROR_COUNTER;     /* 0x10 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    printf("read clear error counters %#x\n", rdval);

    return tc_tlk10232_check_error_counters(channel_addr);
}

/******************************************************************************
 *
 * Function: tc_tlk10232_prbs_generate
 *
 * Description: this function enables/disables to run PRBS
 *
 * Inputs      : channel_addr - MDIO address.
                 enable - 1 for enabling and 0 for disabling
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_prbs_generate(int channel_addr, int enable)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LOOPBACK_TP_CTRL;     /* 0xB */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    if (enable) {
        wrval = rdval | REG_BIT(13);
    } else {
        wrval = rdval & ~REG_BIT(13);
    }
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    return PASSED;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_prbs_verify
 *
 * Description: this function enables/disables to verify PRBS
 *
 * Inputs      : channel_addr - MDIO address.
                 enable - 1 for enabling and 0 for disabling
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_prbs_verify(int channel_addr, int enable)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_LOOPBACK_TP_CTRL;    /* 0xB */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    if (enable) {
        wrval = rdval | REG_BIT(12);
    } else {
        wrval = rdval & ~REG_BIT(12);
    }
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);

    return PASSED;
}

/******************************************************************************
 *
 * Function: tc_tlk10232_reset_datapath
 *
 * Description: this function resets datapath
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
void tc_tlk10232_reset_datapath(int channel_addr)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_RESET_CTRL;    /* 0xE  */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    wrval = rdval | REG_BIT(3);
    tc_tlk10232_mdio_wr(channel_addr, dev_addr, regnum, wrval);
}

/******************************************************************************
 *
 * Function: tc_tlk10232_check_error_counters
 *
 * Description: this function checks error counters 
 *
 * Inputs      : channel_addr - MDIO address.
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
int tc_tlk10232_check_error_counters(int channel_addr)
{
    int rc;
    int dev_addr, regnum;
    ushort rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_HS_ERROR_COUNTER;     /* 0x10 */
    tc_tlk10232_mdio_rd(channel_addr, dev_addr, regnum, &rdval);
    rc = (rdval == 0) ? PASSED : rdval;
    printf("check error counters %#x\n", rdval);

    return rc;
}

/* Curie 2RU Crosspoin Related APIs End */

/* Curie 2RU TLK10232 loopback test related APIs start
 * Curie 2RU needs special mode settings */

/******************************************************************************
 *
 * Function: c2ru_tc_bcm57412_tlk10232_10gkr_setting
 *
 * Description: this function configures 10KR of tlk10232 on BCM57412 SM test
                card for Curie 2RU.
 *
 * Inputs      : cha_addr - PHY MDIO address
 * Outputs     : None
 *
 *****************************************************************************/
static void c2ru_tc_bcm57412_tlk10232_10gkr_setting (int cha_addr)
{
    int dev_addr, regnum;
    ushort hs_serdes_ctrl_2, hs_serdes_ctrl_3, hs_serdes_ctrl_4;

    hs_serdes_ctrl_2 = cha_addr == TC_TLK10232_CHA_ADDR ? 0x9848 : 0x8848;
    hs_serdes_ctrl_3 = 0x1543;
    hs_serdes_ctrl_4 = cha_addr == TC_TLK10232_CHA_ADDR ? 0x3888 : 0x3688;
    printf("channel 0x%x hs_serdes_ctrl_2/3/4: 0x%04x/0x%04x/0x%04x\n",
            cha_addr, hs_serdes_ctrl_2, hs_serdes_ctrl_3, hs_serdes_ctrl_4);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610); /* global reset */

    mdelay(1);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x2000); /* disable auto nego */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* disable link training */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x000e); /* channel data path reset  */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x024d); /* magic register 0x9000 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL;     /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* enable default tx trigger*/

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* trigger loading default HS TX setting value */
    mdelay(1);
    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* normal op for tx trigger  */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9001;         /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0201); /* magic register 0x9001 */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000); /* enable auto nego */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002); /* magic registe 0x96 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00); /* magic register 0x9005 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0001);  /* bit0 for TI use only */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0xc000); /* auto neg FEC req */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, hs_serdes_ctrl_2);
    /* force attenuator off, HS swing change from 1020 to 1340 mVdfpp */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, hs_serdes_ctrl_3);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_4;    /* 0x5  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, hs_serdes_ctrl_4);

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TI_RESERVED_CONTROL; /* 0x8020 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x001f);

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002); /* magic registe 0x96 */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1200); /* restart auto nego */
}

/******************************************************************************
 *
 * Function: c2ru_tc_tlk10232_10gkr_setting
 *
 * Description: this function configures 10KR of tlk10232 on Neptune SM test
                card for Curie 2RU.
 *
 * Inputs      : cha_addr - PHY MDIO address
 * Outputs     : None
 *
 *****************************************************************************/
void c2ru_tc_tlk10232_10gkr_setting (int cha_addr)
{
    int dev_addr, regnum;
    ushort rdval;

    if ((tc_ngio_p->id == SM_BCM57412_TESTCARD) ||
        (tc_ngio_p->mod_type == DAUGHTER_CARD)) {
        c2ru_tc_bcm57412_tlk10232_10gkr_setting(cha_addr);
        return;
    }

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610); /* global reset */

    mdelay(1);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x2000); /* disable auto nego */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* disable link training */

    tc_tlk10232_mdio_wr(cha_addr, 1, 0xab, 1);  /* enable 10G-KR FEC */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0008); /* channel data path reset  */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x024d); /* magic register 0x9000 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL; /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* enable default tx trigger*/

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* trigger loading default HS TX setting value */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* normal op for tx trigger  */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX9001;      /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200); /* magic register 0x9001 */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002); /* magic registe 0x96 */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00); /* magic register 0x9005 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0xb848);
    /* force attenuator off, HS swing change from 1020 to 1340 mVdfpp */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    if (tc_ngio_p->id == SM_BCM57412_TESTCARD) {
        tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1542);
        printf("ctrl 3 0x1542\n");
    } else {
        tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3548);
        printf("ctrl 3 0x3548\n");
    }
    /* HS_TWCRF : cursor reduction to 22%, default is 0%; HS serdes PEAK_disable;
     * HS clock data recovery alg -Sixteen vote threshold -> four vote threshold;
     * HS clock data recovert alg freq from 2x mode to 1x mode */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x8000;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* auto neg FEC req */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0001);  /* bit0 for TI use only */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000); /* enable auto nego */

    /* newly add, 0x80 means autoneg support 10G */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
    regnum = 0x11; /* magic register */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x80);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x0200;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* restart auto nego */

    return;
}

/******************************************************************************
 *
 * Function: c2ru_tc_tlk10232_1gkx_setting
 *
 * Description: this function configures 1GKX of tlk10232 on Neptune SM test
                card for Curie 2RU.
 *
 * Inputs      : cha_addr - PHY MDIO address
 * Outputs     : None
 *
 *****************************************************************************/
void c2ru_tc_tlk10232_1gkx_setting(int smi_addr)
{
    int dev_addr, regnum;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x8610); /* global reset */

    mdelay(1);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = 0;
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x2000);

    /* keep auto neg on, enable auto-neg ability for 1g (7.0x11=0x20) */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
    regnum = 0x11;
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x20);


   dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
   regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
   tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x9848);

   dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
   regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
   tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x5542);

   dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
   regnum = TLK10232_HS_SERDES_CTRL_4;    /* 0x5  */
   tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x3400);

    if (testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_NIM) {
    #if 0
       dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
       regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
       tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x9848);

       dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
       regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
       tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x7540);
    #endif
    } else {
    #if 0
       printf("SM 1G-KX\n");
       dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
       regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
       tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0xb848);

       dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
       regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
       tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x5548);
    #endif
    }

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = 0;          /* 0x0 */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x3000);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = 0;          /* 0x0 */
    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, 0x3200);

    mdelay(1000);

    do {
        u_short hs_0x03 = 0, hs_0x04 = 0, hs_0x05 = 0;
        tc_tlk10232_mdio_rd(smi_addr, TLK10232_VDR_SPE_DEV_ADDR, TLK10232_HS_SERDES_CTRL_2, &hs_0x03);
        tc_tlk10232_mdio_rd(smi_addr, TLK10232_VDR_SPE_DEV_ADDR, TLK10232_HS_SERDES_CTRL_3, &hs_0x04);
        tc_tlk10232_mdio_rd(smi_addr, TLK10232_VDR_SPE_DEV_ADDR, TLK10232_HS_SERDES_CTRL_4, &hs_0x05);
        printf("HS SETTING: HS 0x03: %04x, 0x04: %04x, 0x05: %04x\n", hs_0x03, hs_0x04, hs_0x05);
    } while (0);
}

/******************************************************************************
 *
 * Function: c2ru_nim_tlk10232_10gkr_setting
 *
 * Description: this function configures 10GKR of tlk10232 on NIM test
                card for Curie 2RU.
 *
 * Inputs      : cha_addr - PHY MDIO address
 * Outputs     : None
 *
 *****************************************************************************/
void c2ru_nim_tlk10232_10gkr_setting (int cha_addr)
{
    int dev_addr, regnum;
    ushort rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1E */
    regnum = TLK10232_GLOBAL_CTRL_REG;     /* 0x0  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x8610); /* global reset */

    mdelay(1);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR;  /* 0x7 */
    regnum = TLK10232_AN_CONTROL;           /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x2000); /* disable auto nego */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* disable link training */

    tc_tlk10232_mdio_wr(cha_addr, 1, 0xab, 1);  /* enable 10G-KR FEC */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_RESET_CTRL;          /* 0xe  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0008); /* channel data path reset  */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX9000;         /* 0x9000  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x024d); /* magic register 0x9000 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_EN_CTRL; /* 0x8101  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* enable default tx trigger*/

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0004); /* trigger loading default HS TX setting value */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0000); /* normal op for tx trigger  */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;  /* 0x1 */
    regnum = TLK10232_VDR_HEX9001;      /* 0x9001  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0200); /* magic register 0x9001 */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX0096;         /* 0x96 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0002); /* magic registe 0x96 */

    dev_addr = TLK10232_PMAPMD_DEV_ADDR;   /* 0x1 */
    regnum = TLK10232_VDR_HEX9005;         /* 0x9005 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x1c00); /* magic register 0x9005 */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_2;    /* 0x3  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0xa848);
    /* force attenuator off, HS swing change from 1020 to 1340 mVdfpp */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_HS_SERDES_CTRL_3;    /* 0x4  */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x5502);
    /* HS_TWCRF : cursor reduction to 22%, default is 0%; HS serdes PEAK_disable;
     * HS clock data recovery alg -Sixteen vote threshold -> four vote threshold;
     * HS clock data recovert alg freq from 2x mode to 1x mode */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_ADV_3;            /* 0x12  */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x8000;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* auto neg FEC req */

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR;  /* 0x1e */
    regnum = TLK10232_TRIGGER_LOAD_CTRL;   /* 0x8100 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x0001);  /* bit0 for TI use only */

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x3000); /* enable auto nego */

    /* newly add, 0x80 means autoneg support 10G */
    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /*0x7*/
    regnum = 0x11; /* magic register */
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x80);

    dev_addr = TLK10232_AUTO_NEG_DEV_ADDR; /* 0x7 */
    regnum = TLK10232_AN_CONTROL;          /* 0x0 */
    tc_tlk10232_mdio_rd(cha_addr, dev_addr, regnum, &rdval);
    rdval = rdval | 0x0200;
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, rdval); /* restart auto nego */

    return;
}

/******************************************************************************
 *
 * Function: c2ru_set_tlk10232_port_int_lpbk
 *
 * Description: this function configures tlk10232 mode and loopback
 *
 * Inputs      : port - port index, speed - speed value, mode - mode string
 * Outputs     : None
 *
 *****************************************************************************/
static void c2ru_set_tlk10232_port_int_lpbk (int port, int speed, const char *mode)
{
    int path, smi_addr;

    path = SAME_HS_INPUT;

    if (port == 0) {
        smi_addr = TC_TLK10232_CHA_ADDR;
    } else {
        smi_addr = TC_TLK10232_CHB_ADDR;
    }

    /* ti setting for init tlk chip */
    printf("\nInit TLK10232 %s state...\n", mode);

    if (speed == 1000) {
        c2ru_tc_tlk10232_1gkx_setting(smi_addr);
    } else {
        if (testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_NIM)
            if(tc_ngio_p->mod_type == DAUGHTER_CARD) {
                c2ru_tc_tlk10232_10gkr_setting(smi_addr);
            } else {
                c2ru_nim_tlk10232_10gkr_setting(smi_addr);
            }
        else
            c2ru_tc_tlk10232_10gkr_setting(smi_addr);
    }

    if (port == 1) {
        tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
    } else {
        tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
    }

    /* set path and loobpack */
    printf("config HS as transmit input GE%d\n", port);
    tc_config_tlk10232_path(smi_addr, path);

    /* non-GH plat link up after setup loopback */
    printf("config tlk chip as loopback GE%d\n", port);
    tc_config_tlk10232_loopback(smi_addr, ENABLE);

    return;
}

/******************************************************************************
 *
 * Function: c2ru_tc_tlk10232_chk_link_speed
 *
 * Description: this function checks tlk10232 mode
 *
 * Inputs      : port - port index, speed - speed value, mode - mode string
 * Outputs     : None
 *
 *****************************************************************************/
static int c2ru_tc_tlk10232_chk_link_speed (int port, int speed, const char *mode)
{

    int mask, smi_addr;
    ushort rdval;

    smi_addr = port == NGIO_GE0 ? TC_TLK10232_CHA_ADDR : TC_TLK10232_CHB_ADDR;
    mask = speed != 1000 ? TLK10232_AN_10GKR : TLK10232_AN_1GKX;

    tc_tlk10232_mdio_rd(smi_addr, TLK10232_AUTO_NEG_DEV_ADDR, TLK10232_AN_BP_STATUS, &rdval);
    if (rdval & mask) {
       printf("port link up on %s\n", mode);
       return (PASSED);
    }

    return (FAILED);
}

/******************************************************************************
 *
 * Function: c2ru_tc_tlk10232_link_check
 *
 * Description: this function checks tlk10232 link status
 *
 * Inputs      : port - port index, speed - speed value, mode - mode string
 * Outputs     : None
 *
 *****************************************************************************/
static int c2ru_tc_tlk10232_link_check(int ctrl_plane_sgmii_port, int test_port,
                                       int speed, const char *mode)
{
    int rc;

    /* 0. check link status on host eth port */
    /* isolate this check, since previous projects are FCS. */
    if (ntc_chk_eth_linkup(ctrl_plane_sgmii_port) == FALSE) {
        printf("\nlink up failed on host eth port. retry to bring up link again.\n");
        return FAILED;
    }

    /* 2. check link status on tlk10232 */
    rc = c2ru_tc_tlk10232_chk_link_speed(test_port, speed, mode);
    if (rc == FAILED) {
        printf("\nlink speed is not matched on tlk10232, retry again.\n");
        return rc;
    }

    return PASSED;
}

/******************************************************************************
 *
 * Function: c2ru_tc_tlk10232_intlb_mode_test
 *
 * Description: this function executes internal loopback test for one mdoe
 *
 * Inputs      : port - port index, speed - speed value, mode - mode string
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static int c2ru_tc_tlk10232_intlb_mode_test(int test_port, int speed, const char *mode)
{
    int rc;
    int num_pkt = 300;
    int retry = RETRY_CNT_10G_KR_TC;
    int ctrl_plane_sgmii_port;

    while (retry > 0) {
        c2ru_set_tlk10232_port_int_lpbk(test_port, speed, mode);

        /* get host eth port for send packet */
        ctrl_plane_sgmii_port = get_sgmii_port_num(testcard_if_p->slot, TYPE_SWITCH);
        ctrl_plane_sgmii_port -= (test_port * 2);

        rc = c2ru_tc_tlk10232_link_check(ctrl_plane_sgmii_port, test_port, speed, mode);
        if (rc == PASSED)
            break;

        retry--;
    }

    if (retry == 0) {
        cterr('f', 0, "%s: Failed on TestCard GE%d", __FUNCTION__, test_port);
        return FAILED;
    }

    rc = sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt);
    if (rc != PASSED) {
        cterr('f', 0, "Failed to do TestCard GE%d %s loopback on eth%d",
              test_port, mode, ctrl_plane_sgmii_port);
        return rc;
    }

    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);
    printf("\n");

    return PASSED;
}

/******************************************************************************
 *
 * Function: c2ru_tc_tlk10232_intlb_mode_test
 *
 * Description: this function executes internal loopback tests
 *
 * Inputs      : None
 * Outputs     : PASSED/FAILED
 *
 *****************************************************************************/
static int c2ru_tc_tlk10232_internal_loopback_test (int dummy)
{
    int i, rc;
    int test_port = 0;
    int tc_max_eth_port = 0;
    int speed = 10000;
    const char *mode = "10G-KR";
    int err_count = 0;

    tc_max_eth_port = TC_MAX_ETH_PORT;
    if (testcard_if_p->is_10gkr == TC_10GKR_NIM_ON_NIM) {
        tc_max_eth_port = CURIE_NIM_MAX_ETH_PORT;
    }

    for (test_port = 0; test_port < tc_max_eth_port; test_port++) {
        for (i = 0 ; i < 1; i++) {
            if (i == 1) {
                speed = 1000;
                mode = "1G-KX";
            } else {
                speed = 10000;
                mode = "10-KR";
            }

            testname("TestCard GE%d %s Internal loopback", test_port, mode);

            rc = c2ru_tc_tlk10232_intlb_mode_test(test_port, speed, mode);
            if (rc != PASSED)
                err_count++;
        }
    }

    if (err_count)
        return FAILED;

    return PASSED;
}

/* Curie 2RU TLK10232 loopback test related APIs end */

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
    tc_tlk10232_mdio_wr(cha_addr, dev_addr, regnum, 0x4000);

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

       msleep(10);
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
               tc_xaui_phy_reg_rd(smi_addr, dev_addr, regnum, &rdval);
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

/******************************************************************************
 *
 * Function: ext_sfp_lpbk_test
 *
 * Description: this test need to have external sfp loopback plug-in. 
 *
 * Inputs      : None
 * Outputs     : None
 *
 */
static void ext_sfp_lpbk_test (void)
{
    int test_port, ntc_gesw_chk, smi_addr; 

    printf("Please make sure SFP is already plugged in \n"); 

    /* BCM GE0 SMI addr = 0, GE1 SMI addr =1; 
     * it is the same as GE port definition */
    for (test_port = 0; test_port < TC_MAX_ETH_PORT; test_port++) {

        printf("Init GE%d, ", test_port); 
        /* init tlk10232 and also restore to 10G setting */
        tc_tlk10232_init_10gkr_ti_setting(); 

        /* disable opposite port, get link speed  */
        if (test_port == NGIO_GE0) {
            ntc_gesw_chk = ntc_gesw_p0_type;
            tc_tlk10232_power_off(TC_TLK10232_CHB_ADDR);
            smi_addr = TC_TLK10232_CHA_ADDR;
        } else {
            ntc_gesw_chk = ntc_gesw_p1_type;
            tc_tlk10232_power_off(TC_TLK10232_CHA_ADDR);
            smi_addr = TC_TLK10232_CHB_ADDR;
        } 

        if (ntc_gesw_chk == TRUE) { /* TRUE = 1G */
            printf("Testing 1G speed, "); 

            tc_tlk10232_set_mode(smi_addr); 
            /* dev1, 1.0=0x40; 1.7=0xd */
            /* based on datasheet, it is 1G bypass mode */
            tc_xaui_phy_reg_wr(test_port, BCM8727_PHY_PMAPMD, 
                               PMAPMD_CTRL_REG_OFF, PMAPMD_CTRL_REG_SET_1G); 

            tc_xaui_phy_reg_wr(test_port, BCM8727_PHY_PMAPMD,
                               PMD_CTRL_2_REG, PMD_CTRL_2_REG_SET_1G);
        } else {
            printf("Testing 10G speed, "); 
        }

        printf("Sending packets.... \n");
        /* now, send packet */
        tmp_send_pkt_util(); 

        if (ntc_gesw_chk == TRUE) {  /* restore to default val */
            /* default value: dev1, 1.0=0x2040; 1.7=0x8 */
            tc_xaui_phy_reg_wr(test_port, BCM8727_PHY_PMAPMD, 
                               PMAPMD_CTRL_REG_OFF, PMAPMD_CTRL_REG_DEF);

            tc_xaui_phy_reg_wr(test_port, BCM8727_PHY_PMAPMD, 
                               PMD_CTRL_2_REG, PMD_CTRL_2_REG_DEF); 
        }

    }

    return;
}

static void toggle_tc_config_tlk10232_loopback (int smi_addr)
{
    int dev_addr, regnum;
    ushort  wrval, rdval;

    dev_addr = TLK10232_VDR_SPE_DEV_ADDR; /* 0x1E */
    regnum = TLK10232_LOOPBACK_TP_CTRL;   /* 0xb */

    tc_tlk10232_mdio_rd(smi_addr, dev_addr, regnum, &rdval);

    if (rdval & 0x8) {
        printf("disable loopback \n"); 
        wrval = rdval & (~0x0008);
    } else {
        wrval = rdval | 0x8;
        printf("enable loopback \n"); 
    } 

    tc_tlk10232_mdio_wr(smi_addr, dev_addr, regnum, wrval);

    return;
}


/*------------------------------------------------------------------
$Log: testcard_tlk_10232.c,v $
Revision 1.22  2020/05/22 02:28:35  qingcwan
Merge switzer-carrier code into main chunk.

Revision 1.21  2020/01/09 01:02:20  jiajliu
Merge Curie 2RU to main trunk

Revision 1.20  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.19.2.7  2019/05/20 07:50:38  alpeng
update ti init seq for curie based on HW suggestion

Revision 1.19.2.6  2019/02/25 08:27:06  alpeng
 support 1g test on curie

Revision 1.19.2.5  2018/11/23 09:45:40  alpeng
update utilities for debug

Revision 1.19.2.4  2018/08/28 16:31:18  alpeng
stablize testcard loobpack test on curie

Revision 1.19.2.3  2018/08/16 18:22:38  alpeng
remove useless info on i2c_drv; fixed get_sgmii_port on platform_eth_pkt_txrx.c for curie; add wrapper for wic_test and sm_test for prepare eth info on platform_slot.c; support ge1 for SM on testcard;

Revision 1.19.2.2  2018/08/10 08:15:52  alpeng
update eth num for sm1; support sync signal test for curie; skip plx pcie check (wait for rommon); skip ge1 on SM for both tlk10232 and xaui loopback test (wait for rommon); remove NIM slot2 test; fixed pcie lane scan test

Revision 1.19.2.1  2018/08/09 10:18:43  alpeng
support NTC on Curie NIM1

Revision 1.19  2018/05/22 02:31:11  alpeng
fixed compiler warning, CSCvj57934

Revision 1.18  2018/05/18 09:24:52  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.17  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.16  2017/07/14 02:51:39  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.15  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.14.30.11  2018/05/17 10:50:23  alpeng
 sync with trunk <trunk-051618>

Revision 1.14.30.10  2018/02/06 08:44:59  alpeng
support SFP external loopback test on testcard

Revision 1.14.30.9  2018/02/05 09:48:26  alpeng
add check link speed before sending packet; skip aux -54v detection check

Revision 1.14.30.8  2018/02/02 06:10:08  alpeng
add util and register info for debugging

Revision 1.14.30.7  2018/01/31 09:43:25  alpeng
revert to old code and enhance basic utilities for debugging

Revision 1.14.30.6  2018/01/16 06:46:30  alpeng
first check in for 10G-KR SM testcard; we need to apply correct id once hw ready for it

Revision 1.14.30.5  2017/11/27 05:59:42  leschen
Initial check in to support VG450.

Revision 1.14.30.4  2017/04/17 10:13:24  alpeng
add gesw ptype check

Revision 1.14.30.3  2017/04/06 02:09:45  leschen
Fix testcard xaui issue.

Revision 1.14.30.2  2017/04/05 06:45:04  leschen
Sync with <ng_diag-tag-032917>

Revision 1.14.30.1  2016/10/21 18:19:39  alpeng
update testcard for sm4

Revision 1.17  2017/08/10 10:10:41  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.16  2017/07/14 02:51:39  alpeng
fixed compiler warning, due to cross-compiler version was updated.

Revision 1.15  2016/10/16 12:28:18  iachang
Supported Goldbeach Platform.

Revision 1.14  2014/10/17 07:18:32  alpeng
supporting retry in case the gesw link is unstable packet cannot be sent

Revision 1.13  2014/08/28 09:39:09  alpeng
support new testcard on overlord

Revision 1.12  2014/08/20 06:21:18  alpeng
support new testcard on non-GH platforms

Revision 1.11  2014/08/06 03:17:07  alpeng
pre HW request, fixed the data display

Revision 1.10  2014/07/28 03:43:46  alpeng
check gesw link status before sending packet

Revision 1.9  2014/07/25 01:36:57  alpeng
support xaui loopback and sort out the test item for new testcard

Revision 1.8  2014/07/22 09:40:13  alpeng
support 10g-kr on ge0 and 1g-kx on ge1 for new testcard

Revision 1.7  2014/06/25 08:19:59  alpeng
add util for send pkt to eth3

Revision 1.6  2014/06/25 06:17:04  alpeng
a new item for init tlk chip

Revision 1.5  2014/06/24 11:09:43  alpeng
update the address for channel a and b

Revision 1.4  2014/06/06 08:14:52  alpeng
xaui test could be leveraged for new testcard

Revision 1.3  2014/06/06 07:04:45  alpeng
put plx and tlk10232 test into menu

Revision 1.2  2014/06/03 06:03:09  alpeng
first check in for plx on testcard; update the code for tlk10232 on testcard

Revision 1.1  2014/05/15 07:49:55  alpeng
first check in for tlk10232 chip on new testcard

Revision 1.3  2013/09/09 05:34:02  ptong
Replace #ifdef UTAH with is_utah, is_overlord, etc for platform depend code

Revision 1.2  2013/08/09 00:33:00  hroni
Utah uses SGMII port#4 for eth and xaui loopback test

Revision 1.1  2013/05/09 05:42:38  alpeng
moving overlord common code from x86

Revision 1.6  2013/02/15 10:34:36  palin2
Update the display of the TestCard Diag tests based on code review's comment.

Revision 1.5  2012/11/21 19:47:22  palin2
Use function "do_all_menu_items" to replace "menu_exec_doall_diags".

Revision 1.4  2012/10/25 21:08:04  palin2
Fixed issue that can't exit loopback mode well if loopback test Failed.

Revision 1.3  2012/09/24 17:37:42  palin2
1. Use "Internal loopback test" as default test for TestCard.
2. Unify all tests print out format for TestCard.

Revision 1.2  2012/09/24 01:55:32  palin2
Add TestCard GE Internal loopback test support.

Revision 1.1  2012/08/14 11:30:55  palin2
Removed "ovld_" from TestCard related filename because TestCard is not Overlord's unique.

Revision 1.2  2012/07/24 16:05:09  palin2
Add TestCard SGMII external loopback test support.

Revision 1.1  2012/07/23 17:33:55  palin2
Initial check-in for Overlord Test Card diag tests.


$Endlog$
*/

