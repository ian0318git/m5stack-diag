/* $Id: testcard_plx_pcie_sw.c,v 1.2 2016/04/20 11:25:32 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/testcard_plx_pcie_sw.c,v $
 *------------------------------------------------------------------
 * Filename   : testcard_plx_pcie_sw.c
 *
 * Description: Juno and USD PCIe switch, PLX PEX8618,
 *              related diag tests and utilities.
 *
 * Copyright (c) 2014-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
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
#include "linux_pciutils.h"
#include "i2c_api.h"
#include "menu.h"
#include "error.h"
#include "cross_platform.h" /* WIC_MODULE, SM_MODULE */
#include "platform_slot.h"  /* get_wic_device_id */
#include "ngio_testcard.h"


/*******************************************************************************
 *                            Function Prototypes                              *
 *******************************************************************************
 */
void build_tc_plx_menu(int);
static void build_tc_plx_utils(int);
int testcard_pcie_linkup_test(void);
int testcard_pcie_linkup_test_wrapper(void);

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
#define REG_BIT(x) (1 << (x))
#define MF_1    (MF_CONTINUOUS | MF_DOGRP)
#define MF_2    (MF_1 | MF_DOALL)
#define MF_3    (MF_2 | MF_SHOW_ERRCOUNT)
#define MF_4    (MF_1 | MF_SHOW_ERRCOUNT)

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
#if 0  /* fix me on intel side */
    char pcie_info[80];
    char buffer[80];
    FILE  *fp;

    char wic_pcie_bus_num[][8] = {":01.0", ":02.0", ":03.0", ":05.0",
                                  ":0b.0", ":0d.0", ":08.0", ":0a.0",
                                  ":0c.0", ":0e.0" };

    char slot_pcie_bus_num[2][8];

    char *pcie_lanes =  "x1";
    char rescan_cmd[80] = "echo 1 > /sys/bus/pci/devices/0000:" ; 
    char secbus_cmd[80];
    char *fname = "/tmp/lspci_secondary_bus"; 
    char plx_bus_num[8];
    int bus_index = 0;
    int plx_exist = 0;
    int scan_done = 0;
    int slot,mod;

    testname("TestCard PCIe linkup");

    /* Get PCIe bus number of PLX port 0 of the platform  */
    port0_bus_num = get_ngio_pcie_bus_num() - 1;

    /*add '0' to plx_bus_num string if port0_bus_num is 1-digit in hexadecimal*/
    if ((port0_bus_num+1) <= 0xf) {
        sprintf(plx_bus_num, "0%x", port0_bus_num+1);
    } else {
        sprintf(plx_bus_num, "%x", port0_bus_num+1);
    }

    mod = testcard_if_p->type;
    slot = testcard_if_p->slot;

    /* Assign pcie bus number for corresponding platforms and wic slot */
    if (slot == 1) {
        /* utah wic slot1  */
        bus_index = 2;
        disable_port_reg = 0x234;
        disable_port_reg_value = 2;
    } else if (slot == 2) {
        /* utah wic slot2  */
        bus_index = 4;
        disable_port_reg = 0x234;
        disable_port_reg_value = 0x20;
    } else {
        /* utah wic slot3  */
        bus_index = 5;
        disable_port_reg = 0x234;
        disable_port_reg_value = 0x40;
    }

    /* get secondary bus number */
    strcat(plx_bus_num,  wic_pcie_bus_num[bus_index]);
    sprintf(secbus_cmd,"lspci -vv -s %s | grep secondary | cut -c 29-30 > %s",
            plx_bus_num, fname );
    system(secbus_cmd);
    fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("Failed to open %s\n",fname);
        return (FAILED);
    }
    fscanf(fp, "%s", slot_pcie_bus_num[1]);
    /* assign the pcie bus num and its secondary bus num to be checked */
    strcpy(slot_pcie_bus_num[0], plx_bus_num); 
    strcat(slot_pcie_bus_num[1], ":00.0");

    /* generate local pcie bus rescan command according to specific pcie bus*/
    strcat(rescan_cmd,plx_bus_num);
    strcat(rescan_cmd,"/rescan");

    system(rescan_cmd);
    system("sh /overlord/bin/generic_pcie_lane.sh");

    /* close fp before reuse fp */
    fclose(fp);

    fp = fopen("/ovld_pcie_lane_err.txt", "r");
    if (fp == NULL) {

        printf("Failed to open /ovld_pcie_lane_err.txt");
        return (FAILED);

    }

    /* scan the text file ovld_pcie_lane_err.txt
     * to check the pcie lanes number
     */
    bus_index = 0;
    while ((fscanf(fp, "%s", pcie_info) != EOF) && (scan_done != 1)) {
       if (strcmp(pcie_info, "PLX") == 0) {
            plx_exist = 1;
       }
       if (strcmp(pcie_info,slot_pcie_bus_num[bus_index]) == 0) {
            do {
            }while ( (strcmp(pcie_info, "Width") != 0) && 
                     (fscanf(fp, "%s", pcie_info) != EOF) );

            fscanf(fp, "%s", pcie_info);
            pcie_info[strlen(pcie_info)-1] = '\0';

            if (strcmp(pcie_info,pcie_lanes) != 0) {
                sprintf(buffer, "For bus number %s; detected %s lanes, expected %s lanes.",
                    slot_pcie_bus_num[bus_index],pcie_info+1,pcie_lanes+1);
                printf("\n%s",buffer);
                return (FAILED);
            }

        bus_index++ ;
        }
    }

    fclose(fp);

    if (plx_exist == 0) {
        printf("PLX PCIe switch is not detected!!\n");
        return (FAILED);
    }
#endif 

    return (PASSED);
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
int testcard_pcie_linkup_test_wrapper (void)
{
    printf("FIX ME: test me on intel side \n");
#if 0
    int retry, error = 0;

    if (testcard_pcie_linkup_test() == FAILED) {
        error = 1;

        if (is_plx()) {
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
    }   

    if (error == 0) {
        prpass(testpass,"WIC testcard pcie linkup successfully!!\n");
    }
    else {
        cterr('f',0,"WIC testcard PCIe linkup failed!!\n");
        return (FAILED);
    }

#endif 

    return (PASSED);
}

/*
 *------------------------------------------------------------------
 * $Log: testcard_plx_pcie_sw.c,v $
 * Revision 1.2  2016/04/20 11:25:32  benchen2
 * add tachi fru portion
 *
 * Revision 1.1.2.1  2015/07/31 10:40:03  alpeng
 * first check in for testcard
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
