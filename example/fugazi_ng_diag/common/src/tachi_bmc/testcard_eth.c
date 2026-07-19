/* $Id: testcard_eth.c,v 1.2 2016/04/20 11:25:31 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/testcard_eth.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_eth.c
 *
 * Description: TestCard Ethernet related diag tests and utilities.
 *
 * Copyright (c) 2013-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "byteswap.h"
#include "common.h"
#include "common_utils.h"
#include "error.h"
#include "menu.h"
#include "nvmonvars.h"
#include "i2c_api.h"
#include "slot.h"
#include "plat_defs.h"
#include "ngio_testcard.h"
#include "testcard_fpga.h"
#include "testcard_eth.h"
#include "platform_i2c.h"
#include "queryflags.h"
#include "dash_fpga.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_api.h"


/******************************************************************************* 
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_eth_menu(int);
int  set_port_ext_lpbk_stub(uint, uint);
int  tc_sgmii_1g_lpbk_test(int);

static void build_tc_eth_utils(int);
static int  tc_read_ge_phy_reg(void);
static int  tc_alter_ge_phy_reg(void);
static void tc_set_ge1_int_lp(void);
static void tc_clr_ge1_int_lp(void);
static void tc_set_ge0_int_lp(void);
static void tc_clr_ge0_int_lp(void);


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


/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * Ethernet Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_eth_diag_table[] = {
    {"Ethernet Utilities",
     (PFT)build_tc_eth_utils,     TRUE,
     0,                           (PFT)0, 0, (PFT)build_tc_eth_utils, TRUE},
    {"SGMII 1000Base-T Internal loopback test", 
     (PFT)tc_sgmii_1g_lpbk_test,  TC_ETH_INT_LPBK,
     (MF_CONTINUOUS | MF_DOALL),  (PFT)0, 0, (PFT)0,                  0},
    {"SGMII 1000Base-T External loopback test", 
     (PFT)tc_sgmii_1g_lpbk_test,  TC_ETH_EXT_LPBK,
     (MF_CONTINUOUS),             (PFT)0, 0, (PFT)0,                  0},
};

#define TC_ETH_DIAG_TABLE_SIZE (sizeof(tc_eth_diag_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_eth_diag_pri_items[TC_ETH_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_eth_diag_sec_items[TC_ETH_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_eth_diag = {
    "TestCard Ethernet SubMenu",   /* title */
    0,                             /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,         /* shows major flags */
    0,                             /* generic prompt */
    0,                             /* size -- bumped by add_menu_item() */
    tc_eth_diag_pri_items,
};

static struct menuinfo *tc_eth_diag_p = &tc_eth_diag;


/*
 * GE Ethernet Utilities SubMenu
 */
static submenu_xtable_t eth_utils_tbl[] = {
    {"Read GE PHY register",         (PFT)tc_read_ge_phy_reg,          FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Alter GE PHY register",        (PFT)tc_alter_ge_phy_reg,         FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Set GE0 internal loopback",    (PFT)tc_set_ge0_int_lp,        FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Clr GE0 internal loopback",    (PFT)tc_clr_ge0_int_lp,        FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Set GE1 internal loopback",    (PFT)tc_set_ge1_int_lp,        FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
    {"Clr GE1 internal loopback",    (PFT)tc_clr_ge1_int_lp,        FALSE,
     0, (PFT)0, 0, (PFT)0, 0},
};

#define ETH_UTILS_TBL_SIZE (sizeof(eth_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_eth_utils_pri_items[ETH_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_eth_utils_sec_items[ETH_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_eth_utils = {
    "TestCard Ethernet Utilities",   /* title */
    0,                               /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,           /* shows major flags */
    0,                               /* generic prompt */
    0,                               /* size -- bumped by add_menu_item() */
    tc_eth_utils_pri_items,
};

static struct menuinfo *tc_eth_utils_p = &tc_eth_utils;


/*******************************************************************************
 *
 * Function   : build_tc_eth_menu
 * Description: Build TestCard Ethernet Tests and Utilities SubMenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_eth_menu (int submenu)
{
    build_primary_submenu(tc_eth_diag_table, TC_ETH_DIAG_TABLE_SIZE,
                          "TestCard Ethernet SubMenu", &tc_eth_diag_p);
    build_secondary_submenu(tc_eth_diag_table, TC_ETH_DIAG_TABLE_SIZE,
                            tc_eth_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_eth_diag, tc_eth_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_eth_diag_p);
    }
}


/*******************************************************************************
 *
 * Function   : build_tc_eth_utils
 * Description: Build TestCard Ethernet related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_eth_utils (int submenu)
{
    build_primary_submenu(eth_utils_tbl, ETH_UTILS_TBL_SIZE,
                          "TestCard Ethernet Utils SubMenu", &tc_eth_utils_p);
    build_secondary_submenu(eth_utils_tbl, ETH_UTILS_TBL_SIZE,
                            tc_eth_utils_sec_items);

    menu(&tc_eth_utils, tc_eth_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   :	tc_ge_phy_mdio_rd
 * Description:	Utility to read TestCard GE PHY register.
 * Inputs     :	port_num - number of GE PHY inferface port 
 *              reg_addr - the addr. of register
 *              data     - place to put the read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_ge_phy_mdio_rd (uint port, ulong reg_addr, uint16_t *data)
{
    uint16_t chk_data = SMI_CHAN_BUSY, ctr = 0, in_data = 0;

    /* 1. Write the FPGA SMI0 control register (0x06) */
    in_data = ((CLAUSE_22_FRAME << SMI_SOF_SHIFT) +
               (READ_22_FRAME << SMI_OP_CODE_SHIFT) +
               ((TC_PHY_ADDR + port) << SMI_PHY_ADDR_SHIFT) +
               (reg_addr << SMI_DEV_ADDR_SHIFT));

    if (tc_fpga_reg_wr(SMI0_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 2. Poll the FPGA Speed/Status register (0x04) 
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < ETH_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI0_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI0_STAT_REG_OFFSET);
            return (FAILED);
        }
    }

    /* 3. Read the FPGA data register (0x08) */
    if (tc_fpga_reg_rd(SMI0_ADDR_REG_OFFSET, data) != PASSED) {
        printf("%s:%d Failed to read FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_ADDR_REG_OFFSET);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_ge_phy_mdio_wr
 * Description:	Utility to read TestCard GE PHY register.
 * Inputs     :	port_num - number of GE PHY inferface port 
 *              reg_addr - the addr. of register
 *              data     - data to write in
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_ge_phy_mdio_wr (uint port, ulong reg_addr, uint16_t data)
{
    uint16_t chk_data = SMI_CHAN_BUSY, ctr = 0, in_data = 0;

    /* 1. Write data into the FPGA data register (0x08) */
    if (tc_fpga_reg_wr(SMI0_ADDR_REG_OFFSET, data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_ADDR_REG_OFFSET);
        return (FAILED);
    }

    /* 2. Write the FPGA SMI0 control register (0x06) */
    in_data = ((CLAUSE_22_FRAME << SMI_SOF_SHIFT) +
               (WRITE_FRAME << SMI_OP_CODE_SHIFT) +
               ((TC_PHY_ADDR + port) << SMI_PHY_ADDR_SHIFT) +
               (reg_addr << SMI_DEV_ADDR_SHIFT));

    if (tc_fpga_reg_wr(SMI0_CTRL_REG_OFFSET, in_data) != PASSED) {
        printf("%s:%d Failed to write FPGA register 0x%02X.\n",
               __FUNCTION__, __LINE__, SMI0_CTRL_REG_OFFSET);
        return (FAILED);
    }

    /* 3. Poll the FPGA Speed/Status register (0x04) 
     *    until the SMI Channel Busy bit is cleared.
     */
    while ((chk_data & SMI_CHAN_BUSY) && (ctr < ETH_MAX_RETRY)) {
        if (tc_fpga_reg_rd(SMI0_STAT_REG_OFFSET, &chk_data) != PASSED) {
            printf("%s:%d Failed to read FPGA register 0x%02X.\n",
                   __FUNCTION__, __LINE__, SMI0_STAT_REG_OFFSET);
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_ge_phy_reg_rd
 * Description:	Utility to read TestCard GE PHY register.
 * Inputs     :	port     - number of port
 *              page     - page number of register
 *              reg_addr - Address of register
 *              data     - buffer to put read back register value
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_ge_phy_reg_rd (uint port, uint page, uint reg_addr, uint16_t *data)
{
    uint16_t tmp_data = 0;

    /* Set page */
    if (tc_ge_phy_mdio_rd(port, PHY_PAGE_ADDR, &tmp_data) != PASSED) {
        printf("%s:%d Failed to read GE%d PHY, Page Address register.\n",
               __FUNCTION__, __LINE__, port);
        return (FAILED);
    }

    tmp_data = (tmp_data & (uint16_t)(~(PHY_PAGE_NUM_MSK))); 
    tmp_data |= page;

    if (tc_ge_phy_mdio_wr(port, PHY_PAGE_ADDR, tmp_data) != PASSED) {
        printf("%s:%d Failed to write GE%d PHY, Page Address register.\n",
               __FUNCTION__, __LINE__, port);
        return (FAILED);
    }

    /* Read the register */
    if (tc_ge_phy_mdio_rd(port, reg_addr, data) != PASSED) {
        printf("%s:%d Failed to read GE%d PHY, page %d, reg. %d.\n",
               __FUNCTION__, __LINE__, port, page, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_ge_phy_reg_wr
 * Description:	Utility to write TestCard GE PHY register.
 * Inputs     :	port     - number of port
 *              page     - page number of register
 *              reg_addr - Address of register
 *              data     - data to write into register
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_ge_phy_reg_wr (uint port, uint page, uint reg_addr, uint16_t data)
{
    uint16_t tmp_data = 0;

    /* Set page */
    if (tc_ge_phy_mdio_rd(port, PHY_PAGE_ADDR, &tmp_data) != PASSED) {
        printf("%s:%d Failed to read GE%d PHY, Page Address register.\n",
               __FUNCTION__, __LINE__, port);
        return (FAILED);
    }

    tmp_data = (tmp_data & (uint16_t)(~(PHY_PAGE_NUM_MSK))); 
    tmp_data |= page;

    if (tc_ge_phy_mdio_wr(port, PHY_PAGE_ADDR, tmp_data) != PASSED) {
        printf("%s:%d Failed to write GE%d PHY, Page Address register.\n",
               __FUNCTION__, __LINE__, port);
        return (FAILED);
    }

    /* Write to the register */
    if (tc_ge_phy_mdio_wr(port, reg_addr, data) != PASSED) {
        printf("%s:%d Failed to write GE%d PHY, page %d, reg. %d.\n",
               __FUNCTION__, __LINE__, port, page, reg_addr);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_read_ge_phy_reg
 * Description:	Utility to read TestCard GE PHY register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_read_ge_phy_reg (void)
{
    uint  port = 0, page = 0, reg_addr = 0;
    uint16_t reg_val = 0;

    /* Get info from user */
    port = getdec_answer("Enter the port you want to read:", 0, 0, 1);
    page = getdec_answer("Enter the page of register:", 0, 0, 255);
    reg_addr = getdec_answer("Enter the num. of register:", 0, 0, 31);

    /* Read the specified PHY register */
    if (tc_ge_phy_reg_rd(port, page, reg_addr, &reg_val) != PASSED) {
        return (FAILED);
    }
    printf("The value of GE%d PHY, page %d, reg. %d = 0x%04X.\n",
           port, page, reg_addr, reg_val);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_alter_ge_phy_reg
 * Description:	Utility to read TestCard GE PHY register.
 * Inputs     :	None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_alter_ge_phy_reg (void)
{
    uint  port = 0, page = 0, reg_addr = 0;
    uint16_t write_data = 0;

    /* Get info from user */
    port = getdec_answer("Enter the port you want to read:", 0, 0, 1);
    page = getdec_answer("Enter the page of register:", 0, 0, 255);
    reg_addr = getdec_answer("Enter the num. of register:", 0, 0, 31);
    write_data = gethex_answer("Enter 16-bit data you want to write in:",
                               0x0, 0x0, 0xFFFF);

    /* Write the specified PHY register */
    if (tc_ge_phy_reg_wr(port, page, reg_addr, write_data) != PASSED) {
        return (FAILED);
    }
    printf("Done to write 0x%04X to GE%d PHY, page %d, reg. %d.\n",
           write_data, port, page, reg_addr);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	set_phy_reg_by_mask
 * Description:	Function to enable/disable PHY register per bit.
 * Inputs     :	port     - Number of port be set
 *              page     - Page num of register
 *              reg_addr - Register address
 *              mask     - Mask of register content
 *              set_data - Data want to set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int set_phy_reg_by_mask (uint port, uint page, uint reg_addr,
                                uint16_t mask, uint16_t set_data)
{
    uint16_t reg_val = 0;

    if (tc_ge_phy_reg_rd(port, page, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read page %2d, reg. %2d\n",
               __FUNCTION__, __LINE__, page, reg_addr);
        return (FAILED);
    }

    if ((reg_val & mask) == set_data) {
        return (PASSED);
    }

    reg_val &= (uint16_t)(~mask);
    reg_val |= set_data;

    if (tc_ge_phy_reg_wr(port, page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%04X to page %2d, reg. %2d\n",
               __FUNCTION__, reg_val, page, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	set_phy_reg_per_bit
 * Description:	Function to enable/disable PHY register per bit.
 * Inputs     :	port     - Number of port be set
 *              page     - Page num of register
 *              reg_addr - Register address
 *              set_data - Data content that want to set
 *              opt      - Enable/Disable the bit 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int set_phy_reg_per_bit (uint port, uint page, uint reg_addr,
                                uint16_t set_data, boolean opt)
{
    uint16_t reg_val = 0;

    if (tc_ge_phy_reg_rd(port, page, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read page %2d, reg. %2d\n",
               __FUNCTION__, __LINE__, page, reg_addr);
        return (FAILED);
    }

    if (((reg_val & set_data) && (opt == ENABLE)) ||
        ((!(reg_val & set_data)) && (opt == DISABLE))) {
        return (PASSED);
    }

    if (opt == ENABLE) {
        reg_val |= set_data;
    } else {
        reg_val &= (uint16_t)(~set_data);
    }

    if (tc_ge_phy_reg_wr(port, page, reg_addr, reg_val) != PASSED) {
        printf("%s: Failed to write 0x%04X to page %2d, reg. %2d\n",
               __FUNCTION__, reg_val, page, reg_addr);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   :	tc_ge_phy_clear_int_lpbk
 * Description:	Function to release TestCard GE PHY from Internal loopback mode.
 * Inputs     :	port - Number of port be set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tc_ge_phy_clear_int_lpbk (uint port)
{
    uint16_t mode = 0, mask = 0;

    /* To release PHY from loopback mode,
     * need to Enable Auto-Negotiation, and then
     * soft-reset PHY to let other settings back to default.
     */
    mask = (CCR_COPPER_RST | CCR_AUTO_NEG);
    mode = (CCR_COPPER_RST | CCR_AUTO_NEG);

    return (set_phy_reg_by_mask(port, PHY_CCR_PAGE, PHY_CCR_ADDR,
                                mask, mode));
}


/*******************************************************************************
 *
 * Function   :	set_tc_phy_ctrl_internal
 * Description:	Function to set TestCard GE PHY Control register to
 *              internal loopback mode.
 * Inputs     :	port - Number of port be set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int set_tc_phy_ctrl_internal (uint port)
{
    uint16_t mode = 0, mask = 0;

    /* To set PHY into loopback mode, need to set control register
     * to Disable Auto-Negotiation, select speed, and set duplex mode,
     * then soft-reset PHY to enable those settings.
     */
    mask = (CCR_COPPER_RST | CCR_AUTO_NEG | CCR_COP_DUP | CCR_1000MBPS);
    mode = (CCR_COPPER_RST | CCR_COP_DUP | CCR_1000MBPS);

    return (set_phy_reg_by_mask(port, PHY_CCR_PAGE, PHY_CCR_ADDR,
                                mask, mode));
}


/*******************************************************************************
 *
 * Function   :	set_port_int_lpbk
 * Description:	Function to set PHY internal loopback mode per port.
 * Inputs     :	port - Number of port be set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int set_port_int_lpbk (uint port)
{
    /* 1. Set PHY Control reg (reg 0, page 0). */
    if (set_tc_phy_ctrl_internal(port) != PASSED) {
        printf("%s: Failed to set TestCard GE PHY Port%d control register.\n",
               __FUNCTION__, port);
        return (FAILED);
    }

    /* 2. Enable PHY loopback by setting bit 14 of reg 0, page 0. */
    if (set_phy_reg_per_bit(port, PHY_CCR_PAGE, PHY_CCR_ADDR,
                            CCR_LPBK, ENABLE) != PASSED) {
        printf("%s: Failed to Enable Port%d PHY loopback.\n",
               __FUNCTION__, port);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   :	tc_sgmii_1g_lpbk_test
 * Description:	TestCard SGMII 1000Base-T loopback test (Host->TestCard->Host).
 * Inputs     :	lpbk_type - Type of loopback (External/Internal)
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tc_sgmii_1g_lpbk_test (int lpbk_type)
{
    int num_pkt = 300, main_result = PASSED;
    uint test_port = 0;
    int ctrl_plane_sgmii_port;

    for (test_port = 0; test_port < TC_MAX_ETH_PORT; test_port++) {

        if (lpbk_type == TC_ETH_EXT_LPBK) {
            testname("TestCard Eth%d SGMII External loopback", test_port);
        } else if (lpbk_type == TC_ETH_INT_LPBK) {
            testname("TestCard Eth%d SGMII Internal loopback", test_port);
        }

        if (lpbk_type == TC_ETH_EXT_LPBK) {
            /* Based on PHY(Marvell 88E1548) datasheet, for 1000Base-T mode,
             * register 18_6.3 of tested port must be set to 1 to enable the
             * external loopback stub mode.
             */
            if (set_port_ext_lpbk_stub(test_port, ENABLE) != PASSED) {
                cterr('f', 0, "\n%s: Failed to Enable Eth%d into loopback mode.",
                      __FUNCTION__, test_port);
                return (FAILED);
            }
        } else if (lpbk_type == TC_ETH_INT_LPBK) {
            if (set_port_int_lpbk(test_port) != PASSED) {
                cterr('f', 0, "\n%s: Failed to Enable Eth%d into loopback mode.",
                      __FUNCTION__, test_port);
                return (FAILED);
            }
        }

        /* Do SGMII loopback test. */
        prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

#if 0
        ctrl_plane_sgmii_port = get_ctrl_plane_sgmii_port(); 

        main_result = sgmii_lpbk_util(ctrl_plane_sgmii_port, num_pkt);
#endif 
        if (eth_pkt_txrx(ETH1_MAC1, num_pkt, FALSE) == FAILED) {
            main_result = FAILED;
        }

        if (main_result != PASSED) {
            cterr('f', 0, "\n%s: Failed to do TestCard Eth%d"
                          "(from Host side SGMII%d)",
                          __FUNCTION__, test_port, ctrl_plane_sgmii_port);
        }

        /* Exit Loopback Mode */
        if (lpbk_type == TC_ETH_EXT_LPBK) {
            /* Release tested port from external loopback stub mode. */
            if (set_port_ext_lpbk_stub(test_port, DISABLE) != PASSED) {
                if (main_result != PASSED) {
                    printf("\n%s: Failed to let Eth%d exit loopback mode.\n",
                           __FUNCTION__, test_port);
                } else {
                    cterr('f', 0, "\n%s: Failed to let Eth%d exit "
                                  "loopback mode.",
                                  __FUNCTION__, test_port);
                }

                return (FAILED);
            }
        } else if (lpbk_type == TC_ETH_INT_LPBK) {
            if (tc_ge_phy_clear_int_lpbk(test_port) != PASSED) {
                if (main_result != PASSED) {
                    printf("\n%s: Failed to let Eth%d exit loopback mode.\n",
                           __FUNCTION__, test_port);
                } else {
                    cterr('f', 0, "\n%s: Failed to let Eth%d exit "
                                  "loopback mode.",
                                  __FUNCTION__, test_port);
                }

                return (FAILED);
            }
        }
    }

    return (main_result);
}

/*******************************************************************************
 *
 * Function   :	tc_set_ge0_int_lp
 * Description:	Function to set ge0 internal loopback
 * Inputs     :	None 
 * Outputs    : None
 *
 *******************************************************************************
 */
void tc_set_ge0_int_lp (void) 
{
    set_port_int_lpbk(0);
}


/*******************************************************************************
 *
 * Function   :	tc_set_ge0_int_lp
 * Description:	Function to clean ge0 internal loopback
 * Inputs     :	None 
 * Outputs    : None
 *
 *******************************************************************************
 */
void tc_clr_ge0_int_lp (void) 
{
    tc_ge_phy_clear_int_lpbk(0);
}


/*******************************************************************************
 *
 * Function   :	tc_set_ge1_int_lp
 * Description:	Function to set ge1 internal loopback
 * Inputs     :	None 
 * Outputs    : None
 *
 *******************************************************************************
 */
void tc_set_ge1_int_lp (void)    
{
    set_port_int_lpbk(1);
}


/*******************************************************************************
 *
 * Function   :	tc_set_ge1_int_lp
 * Description:	Function to clean ge1 internal loopback
 * Inputs     :	None 
 * Outputs    : None
 *
 *******************************************************************************
 */
void tc_clr_ge1_int_lp (void)    
{
    tc_ge_phy_clear_int_lpbk(1);
}


/*******************************************************************************
 *
 * Function   :	set_port_ext_lpbk_stub
 * Description:	Function to Enable/Disable specific port external loopback stub
 *              by set/unset PHY(88E1548P) checker control reg.(18_6.3)
 * Inputs     :	port   - number of specific port
 *              option - enable/disable external loopback stub
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int set_port_ext_lpbk_stub (uint port, uint option)
{
    uint16_t reg_val = 0, write_data = 0;

    /* Read the PHY checker control reg.(18_6.3) */
    if (tc_ge_phy_reg_rd(port, PHY_CHK_CTRL_PAGE,
                         PHY_CHK_CTRL, &reg_val) != PASSED) {
        printf("%s: Failed to read TestCard Eth%d PHY(Marvell 88E1548) "
               "checker control register(page %d, reg. %d).\n",
               __FUNCTION__, port, PHY_CHK_CTRL_PAGE, PHY_CHK_CTRL);
        return (FAILED);
    }

    if (option) {
        /* Enable external loopback stub */
        write_data = (reg_val | CHK_CTRL_REG_STUB_EN);
    } else {
        /* Disable external loopback stub */
        write_data = (reg_val & (~CHK_CTRL_REG_STUB_EN));
    }

    if (tc_ge_phy_reg_wr(port, PHY_CHK_CTRL_PAGE,
                         PHY_CHK_CTRL, write_data) != PASSED) {
        printf("%s: Failed to write TestCard Eth%d PHY(Marvell 88E1548) "
               "checker control register(page %d, reg. %d).\n",
               __FUNCTION__, port, PHY_CHK_CTRL_PAGE, PHY_CHK_CTRL);
        return (FAILED);
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: testcard_eth.c,v $
Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.6  2015/10/14 09:12:00  alpeng
support new testcard for p1b

Revision 1.1.2.5  2015/10/01 09:20:35  alpeng
update testcard eth test to send packets from bmc eth1

Revision 1.1.2.4  2015/09/18 06:58:55  alpeng
using function return nim i2c bus num; set loopback for testcard GE test, send pkt from Lewis

Revision 1.1.2.3  2015/09/14 09:23:32  alpeng
build goldschlager and dreamliner entry; update testcard util to support ge test manually

Revision 1.1.2.2  2015/08/21 06:46:29  alpeng
support ge/xaui test for testcard; clean up repo;

Revision 1.1.2.1  2015/07/31 10:40:00  alpeng
first check in for testcard

$Endlog$
*/

