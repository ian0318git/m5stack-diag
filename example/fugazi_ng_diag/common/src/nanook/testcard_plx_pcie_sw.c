/* $Id: testcard_plx_pcie_sw.c,v 1.2 2019/12/11 10:10:36 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/testcard_plx_pcie_sw.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_plx_pcie_sw.c
 *
 * Description: Juno and USD PCIe switch, PLX PEX8618,
 *              related diag tests and utilities.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
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
int nanook_plx_pcie_link_up_test(void);
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
/*
    {"Testcard PLX Utilities",  
     (PFT)build_tc_plx_utils,           0,
     ,                         (PFT)0, 0,
     (PFT)build_tc_plx_utils,   TRUE},
*/
/*    {"Testcard PLX loopback test", 
     (PFT)testcard_plx_pcie_utp_lpbk_test,   0,
     MF_3,                      (PFT)0, 0, 
     (PFT)testcard_plx_pcie_utp_lpbk_test,   0},
*/
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


#if 0
/*******************************************************************************
 *
 * Function   : get_testcard_pcie_bus_num
 * Description: get testcard pcie bus number 
 * Inputs     : None - store into global variable: testcard_pcie_bus_num
 * Outputs    : None
 *
 *******************************************************************************
 */
static void get_testcard_pcie_bus_num (int mod, int slot) {

    testcard_pcie_bus_num = get_ngio_pcie_dev_bus_num(WIC_MODULE, slot); 

    return;
}
#endif

#if 0
/*******************************************************************************
 *
 * Function   : is_pcie_port_odd 
 * Description: check whether the plx port is odd/even
 * Inputs     : mod - module type, sm or wic
 *              slot - ngio slot number
 * Outputs    : None
 *
 *******************************************************************************
 */
static int is_pcie_port_odd (int mod, int slot) {

    int result; 
    if (mod == TC_NGWIC) {
        result = get_wic_device_no(slot);
    } else {
        result = get_sm_device_no(slot);
    } 
    
    return (result & 1); 
}
#endif

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

#if 0
/*******************************************************************************
 *
 * Function   : host_pcie_send_test_patt
 * Description: send test pattern from host pcie for utp loopback test 
 * Inputs     : slot - slot number. 
 * Outputs    : PASSED/FAILED
 *  
 *******************************************************************************
 */
int host_pcie_send_test_patt (int slot) {

    int reg_val, err_count, serdes_no; 
    int serdes_utp_en, reg_serdes_utp_en; 
    int reg_serdes_diag_data, serdes_diag_data;

    /* support wic only */
    serdes_no = get_wic_serdes_no(slot);

    switch (serdes_no) {
    case 4:
       if (is_dagger()){ 
           /* odd */
           serdes_utp_en = REG_BIT(16);
           reg_serdes_diag_data = 0x244;
           serdes_diag_data = 0; /* bit[25:24] = 0 */
       } else {
           /* sword */
           serdes_utp_en = REG_BIT(20); 
           reg_serdes_diag_data = 0x248;
       }
    break; 
    case 5: /* dagger */
       serdes_utp_en = REG_BIT(16);
       reg_serdes_diag_data = 0x244;
       serdes_diag_data = REG_BIT(24); /* bit[25:24] = 1 */
    break; 
    case 12:
       /* odd port */
       serdes_utp_en = REG_BIT(20);
       reg_serdes_diag_data = 0x24C;
       serdes_diag_data = 0; /* bit[25:24] = 0 */
    break; 
    case 13:
       /* odd port */
       serdes_utp_en = REG_BIT(21);
       reg_serdes_diag_data  = 0x24C;
       serdes_diag_data = REG_BIT(24); /* bit[25:24] = 01 */
    break; 
    case 14:
       /* odd port */
       serdes_utp_en = REG_BIT(22);
       reg_serdes_diag_data  = 0x24C;
       serdes_diag_data = REG_BIT(25); /* bit[25:24] = 10 */
    break; 
    default:
       printf("there is no serdes match to wic slot %d \n", slot );
    break;
    }

    if (pcie_port_odd) {
        reg_serdes_utp_en = 0x25C;
    } else { 
        reg_serdes_utp_en = 0x258;
    }

    /* Enable transmission of the 128-bit test pattern  */
    /* 0x258 to even ports serdes loopback en */
    /* 0x25C to odd  ports serdes loopback en */
    pcie_config_write(0, host_pcie_bus_num, 0, 0, reg_serdes_utp_en, serdes_utp_en);

    /* Select serdes diagnostic data according to port number  */
    /* 0x248 to even port enable utp mode */
    /* 0x24C to odd  port enable utp mode */
    pcie_config_write(0, host_pcie_bus_num, 0, 0, reg_serdes_diag_data, serdes_diag_data);
    sleep(5);

    /* Read error count of the loopback test */
    /* read 0x248 error count */ 
    reg_val = pcie_config_read(0, host_pcie_bus_num, 0, 0, reg_serdes_diag_data);

    /* bit [23:16] for err counter 0xFF0000, offser 16 */
    err_count = (int)((reg_val & UTP_ERR_COUNT_MSK) >> UTP_ERR_COUNT_OFS); 
    prpass(testpass, "PLX Loopback error count= %d", err_count);

    /* restore/reset to default value 0 */
    prpass(testpass, "restore setting for host plx sw ");
    pcie_config_write(0, host_pcie_bus_num, 0, 0, reg_serdes_utp_en, 0);
    pcie_config_write(0, host_pcie_bus_num, 0, 0, reg_serdes_diag_data, 0);

    if (err_count != 0) {
        cterr('f', 0, "testcard slot %d failed. ", slot);
        return (FAILED);
    }
    
    return (PASSED);
}
#endif

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

#if 0
/*******************************************************************************
 *
 * Function   : testcard_plx_pcie_utp_lpbk_test
 * Description: an entry for testcard plx utp test
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int testcard_plx_pcie_utp_lpbk_test (void) {

    int mod, slot, rv; 

    mod = testcard_if_p->type;
    slot = testcard_if_p->slot;

    testname("TestCard PLX UTP loopback");

    prpass(testpass, "%s%d ", testcard_if_p->type_name, testcard_if_p->slot);

    get_host_pcie_bus_num();  /* done */
    prpass(testpass, "get host pcie bus number=0x%x ", host_pcie_bus_num);

    pcie_port_odd = is_pcie_port_odd(mod, slot); /* done */

    get_testcard_pcie_bus_num(mod, slot);
    prpass(testpass, "get testcard pcie bus number=0x%x ", testcard_pcie_bus_num);

    prpass(testpass, "set host plx sw into master mode");
    host_pcie_master_mode();

    prpass(testpass, "set testcard plx sw into slave mode");
    testcard_pcie_slave_mode();

    prpass(testpass, "send test pattern from host to testcard");
    rv = host_pcie_send_test_patt(slot);

    prpass(testpass, "restore setting for testcard plx ");
    testcard_pcie_normal_mode();

    return (rv); 
}
#endif

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

    return (nanook_plx_pcie_link_up_test());
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
 * Function   : remove_pcie_device
 * Description: remove the pcie device
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void remove_pcie_device (void)
{
/*
# lspci -nn | grep -i 10b5:8617
02:00.0 PCI bridge [0604]: PLX Technology, Inc. PEX 8617 16-lane, 4-Port PCI Express Gen 2 (5.0 GT/s) Switch with P2P [10b5:8617] (rev ba)
# find /sys -name *02:00.0
/sys/bus/pci/devices/0000:02:00.0
/sys/bus/pci/drivers/pcieport/0000:02:00.0
/sys/devices/pci0000:00/0000:00:02.0/0000:02:00.0
# echo 1 >  /sys/devices/pci0000:00/0000:00:02.0/remove	
*/
    char buf1[128];
    char buf2[128];
    char buf3[128];
    char buf4[128];
    char lspci_cmd[80];
    char cat_cmd[80];
    char find_cmd[80] = "find /sys -name *" ; 
    char remove_cmd[80] = "echo 1 >  " ; 
    char *fname1 = "/tmp/lspci_testcard_bus"; 
    char *fname2 = "/tmp/find_testcard_bus"; 
    char *fname3 = "/tmp/find_testcard_root_bus"; 
    FILE  *fp;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nRevmoe the Test Card PCIe device\n");
    }
    /* Get Test Card PCIe bus number */
    sprintf(lspci_cmd,"lspci -nn | grep -i 10b5:8617 | cut -c 1-7 > %s",fname1);
    system(lspci_cmd);
    fp = fopen(fname1, "r");
    if (fp == NULL) {
        printf("Failed to open /tmp/lspci_find_testcard");
        fflush(stdout);
        return;
    }
    fscanf(fp, "%s", buf1);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nbuf1 = %s\n",buf1);
        fflush(stdout);
    }
    fclose(fp);
    /* Search by PCI address */
    strcat(find_cmd,buf1);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nDBG:find_cmd =  %s\n",find_cmd);
        fflush(stdout);
    }
    sprintf(buf2,"%s > %s",find_cmd,fname2);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nbuf2 = %s\n",buf2);
        fflush(stdout);
    }
    system(buf2);
    fp = fopen(fname2, "r");
    if (fp == NULL) {
        printf("Failed to open %s\n",fname2);
        fflush(stdout);
        return;
    }
    fscanf(fp, "%s", buf3);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nbuf3 = %s\n",buf3);
        fflush(stdout);
    }
    /* Find the PCIe device */
    sprintf(cat_cmd,"cat %s | grep -i /sys/devices/pci0000:00/0000:00 | cut -c 1-36 > %s",fname2,fname3);
    system(cat_cmd);
    fp = fopen(fname3, "r");
    if (fp == NULL) {
        printf("Failed to open %s",fname3);
        fflush(stdout);
        fclose(fp);
        return;
    }
    fscanf(fp, "%s", buf4);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nbuf4 = %s\n",buf4);
        fflush(stdout);
    }
    fclose(fp);
    /* Remove the device */
    strcat(remove_cmd,buf4);
    strcat(remove_cmd,"/remove");
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("\nRevmoe Command: %s\n",remove_cmd);
        fflush(stdout);
    }
    system(remove_cmd);
    sleep(1);
}


/*******************************************************************************
 *
 * Function   : nanook_plx_pcie_link_up_test
 * Description: the old legacy code is complicate and not portable
 *              using this new function to refine it. 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int nanook_plx_pcie_link_up_test (void) 
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
 * Revision 1.2  2019/12/11 10:10:36  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
