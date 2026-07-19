/* $Id: testcard_plx_pcie_sw.c,v 1.2 2019/10/17 02:16:27 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/testcard_plx_pcie_sw.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_plx_pcie_sw.c
 *
 * Description: Juno and USD PCIe switch, PLX PEX8618,
 *              related diag tests and utilities.
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "types.h"
#include "plat_defs.h"
#include "testcard_plx_pcie_sw.h"
#include "dash_fpga.h"
#include "ngio.h"
#include "i2c_api.h"
#include "menu.h"
#include "error.h"
#include "cross_platform.h" /* WIC_MODULE, SM_MODULE */
#include "platform_slot.h"  /* get_wic_device_id */
#include "ngio_testcard.h"
#include "nvmonvars.h"
#include "linux_pciutils.h"

/* parse PCI speed */
#define PCI_EXP_LINK_STA_SPD_MASK  0x0000000f
#define PCI_EXP_LINK_STA_SPD_2DOT5 0x00000001
#define PCI_EXP_LINK_STA_SPD_5GT   0x00000002
#define PCI_EXP_LINK_STA_SPD_8GT   0x00000003

/* parse PCI width */
#define PCI_EXP_LINK_STA_WID_MASK  0x000003f0
#define PCI_EXP_LINK_STA_WID_1     0x00000001
#define PCI_EXP_LINK_STA_WID_2     0x00000002
#define PCI_EXP_LINK_STA_WID_4     0x00000004
#define PCI_EXP_LINK_STA_WID_8     0x00000008
#define PCI_EXP_LINK_WID_SHIFT     0x00000004

#define PCI_DEV_0      0
#define PCI_FUN_0      0
#define PCI_CAP_PTR_OFFSET      0x34

/*******************************************************************************
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_plx_menu(int);
static void build_tc_plx_utils(int);
void host_pcie_master_mode(void);
void testcard_pcie_slave_mode(void);
void testcard_pcie_normal_mode(void);
int testcard_plx_pcie_utp_lpbk_test(void);
int testcard_pcie_linkup_test(void);
int testcard_pcie_linkup_test_wrapper(int);
int tabei_plx_pcie_link_up_test(void);
extern int get_pcie_cap_struct_ptr(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_status(uint32_t, uint16_t, int, uint);
extern int get_pcie_link_cap(uint32_t, uint16_t, int, uint);

/*******************************************************************************
 *                                    Externs                                  *
 *******************************************************************************
 */
extern int do_all_menu_items(struct menuinfo *);
extern uint pcie_config_read(uint32_t, uint32_t, uint16_t, uint, uint);
extern void pcie_config_write(uint32_t, uint32_t, uint16_t, uint, uint, uint32_t);
extern uint32_t get_ngio_pcie_bus_num(void);
/* port bus dev fn reg val */

/*******************************************************************************
 *                           Global  & Define                                  *
 *******************************************************************************
 */
static int host_pcie_bus_num, testcard_pcie_bus_num; 
static int pcie_port_odd; 
static uint32_t port0_bus_num;
static uint disable_port_reg;
static uint32_t disable_port_reg_value; 
#define REG_BIT(x) (1 << (x))
#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

/* Platform motherboard HW revision
 * Added to deal with differences between platform revisions
 */
//static unsigned int plat_bd_rev = 999;

/*******************************************************************************
 *                                   Menus                                     *
 *******************************************************************************
 */
/*
 * Testcard PLX Tests and Utilities Main Menu
 */
static submenu_xtable_t tc_plx_diag_table[] = {
    {"Testcard PCIe linkup test", 
     (PFT)testcard_pcie_linkup_test_wrapper,   0,
     MF_3,                      (PFT)0, 0, 
     (PFT)testcard_pcie_linkup_test_wrapper,   0},
};

#define TC_PLX_DIAG_TABLE_SIZE (sizeof(tc_plx_diag_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_plx_diag_pri_items[TC_PLX_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_plx_diag_sec_items[TC_PLX_DIAG_TABLE_SIZE + MAX_BASE_ITEMS];
 
static struct menuinfo tc_plx_diag = {
    "TestCard PLX SubMenu",        /* title */
    0,                             /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,         /* shows major flags */
    0,                             /* generic prompt */
    0,                             /* size -- bumped by add_menu_item() */
    tc_plx_diag_pri_items,
};

static struct menuinfo *tc_plx_diag_p = &tc_plx_diag;

/*
 * Testcard PLX Utilities SubMenu
 */
static submenu_xtable_t tc_plx_utils_tbl[] = {
    {"Left for future using. ", 
     (PFT)build_tc_plx_utils,       0,        0,
     (PFT)0,                          0,   (PFT)0, 0},
};


#define TC_PLX_UTILS_TBL_SIZE (sizeof(tc_plx_utils_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t tc_plx_utils_pri_items[TC_PLX_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t tc_plx_utils_sec_items[TC_PLX_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo tc_plx_utils = {
    "TestCard PLX Utilities",        /* title */
    0,                               /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,           /* shows major flags */
    0,                               /* generic prompt */
    0,                               /* size -- bumped by add_menu_item() */
    tc_plx_utils_pri_items,
};

static struct menuinfo *tc_plx_utils_p = &tc_plx_utils;


/*******************************************************************************
 *
 * Function   : build_tc_plx_menu
 * Description: Build TestCard PLX Tests and Utilities SubMenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_tc_plx_menu (int submenu)
{
    build_primary_submenu(tc_plx_diag_table, TC_PLX_DIAG_TABLE_SIZE,
                          "TestCard PLX SubMenu", &tc_plx_diag_p);
    build_secondary_submenu(tc_plx_diag_table, TC_PLX_DIAG_TABLE_SIZE,
                            tc_plx_diag_sec_items);

    if (submenu) {
        /* Entered with submenu */
        menu(&tc_plx_diag, tc_plx_diag_sec_items, 0);
    } else {
        do_all_menu_items(tc_plx_diag_p);
    }
}

/*******************************************************************************
 *
 * Function   : build_tc_plx_utils
 * Description: Build TestCard PLX related utilities submenu.
 * Inputs     : submenu - TRUE if invoked from submenu
 * Outputs    : None
 *
 *******************************************************************************
 */
static void build_tc_plx_utils (int submenu)
{
    build_primary_submenu(tc_plx_utils_tbl, TC_PLX_UTILS_TBL_SIZE,
                          "TestCard PLX Utils SubMenu", &tc_plx_utils_p);
    build_secondary_submenu(tc_plx_utils_tbl, TC_PLX_UTILS_TBL_SIZE,
                            tc_plx_utils_sec_items);

    menu(&tc_plx_utils, tc_plx_utils_sec_items, 0);
}


/*******************************************************************************
 *
 * Function   : host_pcie_master_mode
 * Description: set host(platform) plx into master mode and setup utp pattern
 *              for test pattern.
 * Inputs     : none
 * Outputs    : None
 *
 *******************************************************************************
 */
void host_pcie_master_mode (void) {

    int reg_phy_layer_cmd, reg_val;

    /* Set User Test Pattern content  */
    pcie_config_write(0, host_pcie_bus_num, 0, 0, UTP_BYTE0_3, 0x03020100);
    pcie_config_write(0, host_pcie_bus_num, 0, 0, UTP_BYTE4_7, 0x07060504);
    pcie_config_write(0, host_pcie_bus_num, 0, 0, UTP_BYTE8_11, 0x0b0a0908);
    pcie_config_write(0, host_pcie_bus_num, 0, 0, UTP_BYTE12_15, 0x0f0e0d0c);
    sleep(1);

    if (pcie_port_odd) {
        reg_phy_layer_cmd = 0x224;
    } else { 
        reg_phy_layer_cmd = 0x220;
    }

    /* 0x220 or 0x224 to even/odd to master mode  */
    /* Set the loopback command bit according port number [bit 0] */
    /* Disable scrambler [bit 1]  */
    reg_val = (REG_BIT(0) | REG_BIT(1)); 
    pcie_config_write(0, host_pcie_bus_num, 0, 0, reg_phy_layer_cmd, reg_val);
    sleep(1);

    /* 0x220 or 0x224 to even or odd to master mode  */
    /* Check loopback ready bit [bit 4]*/
    reg_val = pcie_config_read(0, host_pcie_bus_num, 0, 0, reg_phy_layer_cmd);
    if (!(reg_val & REG_BIT(4))) {
        printf("(Info: Platform PLX port %d Loopback ready bit is not set, continue to TX.)\n", host_pcie_bus_num);
    }

    return; 
}


/*******************************************************************************
 *
 * Function   : testcard_pcie_slave_mode
 * Description: set testcard plx into slave mode 
 * Inputs     : none
 * Outputs    : none
 *
 *******************************************************************************
 */
void testcard_pcie_slave_mode (void) {

    int reg_phy_layer_cmd, reg_val;
   /* the default mode is slave mode */ 
   /* must not scramble the returning data */

    if (pcie_port_odd) {
        reg_phy_layer_cmd = 0x224;
    } else {
        reg_phy_layer_cmd = 0x220;
    }

    /* 0x220 or 0x224 to even/odd to master mode  */
    /* Disable scrambler [bit 1] */
    reg_val = REG_BIT(1);
    pcie_config_write(0, testcard_pcie_bus_num, 0, 0, reg_phy_layer_cmd, reg_val);

    return;
}

/*******************************************************************************
 *
 * Function   : testcard_pcie_normal_mode
 * Description: set testcard plx into normal mode (default)
 * Inputs     : none
 * Outputs    : none
 *
 *******************************************************************************
 */
void testcard_pcie_normal_mode (void) {

    int reg_phy_layer_cmd;
   /* the default mode is slave mode */
   /* must not scramble the returning data */

    if (pcie_port_odd) {
        reg_phy_layer_cmd = 0x224;
    } else {
        reg_phy_layer_cmd = 0x220;
    }
    /* 0x220 or 0x224 to even/odd to master mode  */
    /* 0 to default value */
    pcie_config_write(0, testcard_pcie_bus_num, 0, 0, reg_phy_layer_cmd, 0);

    return;
}


/*******************************************************************************
 *
 * Function   : testcard_pcie_linkup_test 
 * Description: Function to check the WIC testcard PCIe linkup status by using
 *              the information provided from the script "generic_pcie_lane.sh"
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int testcard_pcie_linkup_test (void)
{

    testname("TestCard PCIe linkup");

    return (tabei_plx_pcie_link_up_test());
}

/*******************************************************************************
 *
 * Function   : testcard_pcie_linkup_test_wrapper 
 * Description: test card pcie linkup wrapper function 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int testcard_pcie_linkup_test_wrapper (int slot)
{
    int retry, error = 0;


    if (testcard_pcie_linkup_test() == FAILED) {
        error = 1;

            for (retry = 0; retry < 30 ; retry++) {

                printf("\ndisable/enable downstream port for linkup recovery ...\n");
          
                /* disable downstream port */
                pcie_config_write(0,port0_bus_num,0,0,disable_port_reg,disable_port_reg_value);
                usleep(10000);

                /* enable downstream port */
                pcie_config_write(0,port0_bus_num,0,0,disable_port_reg,0);
                usleep(10000);

                /* disable downstream port */
                pcie_config_write(0,port0_bus_num,0,0,disable_port_reg,disable_port_reg_value);
                usleep(10000);

                /* enable downstream port */
                pcie_config_write(0,port0_bus_num,0,0,disable_port_reg,0);
                usleep(10000);

                if (testcard_pcie_linkup_test() == PASSED) {
                    error = 0;
                    break;
                }
            }
    }   

    if (error == 0) {
        prpass(testpass,"testcard pcie linkup successfully!!\n");
    }
    else {
        cterr('f',0,"testcard PCIe linkup failed!!\n");
        return (FAILED);
    }


    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : tabei_plx_pcie_link_up_test
 * Description: the old legacy code is complicate and not portable
 *              using this new function to refine it. 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int tabei_plx_pcie_link_up_test (void) 
{
    int result = PASSED;
    uint32_t bus, reg_val, sta_val; 
    uint32_t sta_s, sta_w; 

    /* Check PLX pcie switch bus number */
    bus = get_pcie_bus_num(TESTCARD_PLX_PCIE_VID, TESTCARD_PLX_PCIE_DID); 

    printf("PCI bus: %d\n", bus);

    if (bus == UNKNOWN_PCI_BUS_NUM) { 
        cterr('f',0, "Unknown PCI bus number");
        return (FAILED);
    }

    prpass(testpass, "%s", "able to get bus num of PLX PCIe switch");  
    reg_val = get_pcie_cap_struct_ptr(bus, PCI_DEV_0, PCI_FUN_0, PCI_CAP_PTR_OFFSET);
    if (reg_val == FAILED) {
        cterr('f',0, "Can't get PCI cap pointer");
        return (FAILED);
    }

    sta_val = get_pcie_link_status(bus, PCI_DEV_0, PCI_FUN_0, reg_val);

    /* Speed - bit 0~3 */
    sta_s = sta_val & PCI_EXP_LINK_STA_SPD_MASK;
    /* Width - bit 4~9 */
    sta_w = (sta_val & PCI_EXP_LINK_STA_WID_MASK) >> PCI_EXP_LINK_WID_SHIFT;

    /* check speed */
    if (sta_s == PCI_EXP_LINK_STA_SPD_5GT) {
        prpass(testpass, "Link speed is 5G ");
    } else {
        cterr('f',0, "Link speed is not 5G");     
        result = FAILED;  /* fail through */
    }

    /* check width */ 
    if (sta_w == PCI_EXP_LINK_STA_WID_1) {
        prpass(testpass, "Link width is x1 "); 
    } else {
        cterr('f',0, "Link width should be x1, please check the width"); 
        result = FAILED;  /* fail through */
    }

    if (result != FAILED) { 
        prpass(testpass, "PCIe lane scan success. ");
    }

    return (result);
}


/*
 *------------------------------------------------------------------
 * $Log: testcard_plx_pcie_sw.c,v $
 * Revision 1.2  2019/10/17 02:16:27  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.4  2019/07/31 08:00:49  olin2
 * Clean up code
 *
 * Revision 1.1.2.3  2018/12/22 07:20:13  olin2
 * Clean up code
 *
 * Revision 1.1.2.2  2018/10/23 11:34:26  olin2
 * Support Testcard test
 *
 * Revision 1.1.2.1  2018/10/09 09:22:06  olin2
 * Initial commit for NIM test
 *
 * Revision 1.12.2.2  2018/08/20 18:26:39  alpeng
 * upgrade testcard plx pcie link up test for curie
 *
 * Revision 1.12.2.1  2018/08/10 08:15:52  alpeng
 * update eth num for sm1; support sync signal test for curie; skip plx pcie check (wait for rommon); skip ge1 on SM for both tlk10232 and xaui loopback test (wait for rommon); remove NIM slot2 test; fixed pcie lane scan test
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
