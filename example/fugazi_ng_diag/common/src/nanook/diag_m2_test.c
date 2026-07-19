 /* $Id: diag_m2_test.c,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_m2_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_m2_test.c - This file is for m.2 device test 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_common.h"
#include "dash_fpga.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "diag_m2_test.h"
#include "diag_storage_lib.h"
#include "linux_block_test.h"

/*
 * Global variables
 */


/* Local functions */
int diag_m2_sata_test(void);
int diag_m2_usb_test(void);
int is_m2_usb_device (void);
int is_m2_sata_device (void);
int check_m2_device (unsigned int *);

/* Local build menu functions */
int build_m2_test_menu(boolean);


/*
 * Sub Menu used for "M.2 device test -> M.2 Device submenu test"
 */

submenu_xtable_t m2_submenu_table[] = {

    {"M.2 SATA Test",
     (PFT) diag_m2_sata_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
    
    {"M.2 USB Test",
     (PFT) diag_m2_usb_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},
};

#define M2_SUBMENU_TABLE_SIZE (sizeof(m2_submenu_table) / \
                               sizeof(submenu_xtable_t))

static mitem_t m2_primary_items[M2_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t m2_secondary_items[M2_SUBMENU_TABLE_SIZE + MAX_BASE_ITEMS];

menuinfo_t m2_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    m2_primary_items,
};

menuinfo_t *m2_submenup = &m2_subtest_menu;



/*******************************************************************************
 *
 * Function   : build_m2_test_menu
 * Description: M.2 device Test Menu
 * Inputs     : Test/Menu 
 * Outputs    : None
 *
 *******************************************************************************
 */
int build_m2_test_menu (boolean mb_temp_test_items_executed)
{
    char *menu_title= "M.2 Device Test";
    unsigned int m2_device = 0;
    char *tname = "M.2 Device read/write";

    testname("%s access", tname);

    if (check_menu_flag(D_EXT_LOOPBACK)) {
        printf("\n External loopback flag is off, skip '%s'\n", tname);
        prcomplete(testpass, errcount, (char *)0);
        return (PASSED);
    }

    if (check_m2_device(&m2_device) == FAILED) {
        return (FAILED);
    }
    
    if (m2_device == FPGA_M2_USB_PRESENT) {
        printf("M2 USB DEV Present\n");
    } else if (m2_device == FPGA_M2_SATA_PRESENT) {
        printf("M2 SATA DEV Present\n");
    } else if (m2_device == FGPA_M2_NO_DEVICE) {
        cterr('w', 0, "M.2 slot vacant. ");
        return (FAILED);
    }

    build_primary_submenu(m2_submenu_table, M2_SUBMENU_TABLE_SIZE,
                          menu_title, &m2_submenup);
    build_secondary_submenu(m2_submenu_table, M2_SUBMENU_TABLE_SIZE,
                            m2_secondary_items);

    if (mb_temp_test_items_executed) {
        menu(&m2_subtest_menu, m2_secondary_items, 0);
    } else {
        do_all_menu_items(m2_submenup);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : m2_device_test
 * Description: main test for m2 device test.
 * Inputs     : slot, 0 or 1.
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int m2_device_tests (uchar *m2_device_name)
{
    char src[32];
    int retval;

    /* get M2 Device name */
    sprintf(src, "%s", m2_device_name);

    retval = linux_block_test(src, 0, BLOCK_SIZE_512B, BLOCK_TEST_RANDOM, TRUE);

    return (retval);
}

/*******************************************************************************
 *
 * Function   : diag_m2_sata_test
 * Description: main test for m2 sata test
 * Inputs     : void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_m2_sata_test (void) {

    int rc = FAILED;  
    char *tname = "M.2 SATA read/write";
    char *m2_dev = "/dev/m2sata";
    
    if (is_m2_sata_device() == FALSE) {
        return (PASSED);
    }

    testname("%s access", tname);

    prpass(testpass, "%s, ", tname);

    rc = m2_device_tests((uchar *)m2_dev);
    if (rc == FAILED) {
        cterr('f', 0, "M.2 SATA test failed.");
    }

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}


/*******************************************************************************
 *
 * Function   : diag_m2_usb_test
 * Description: main test for m2 usb test
 * Inputs     : void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_m2_usb_test (void) {

    int rc = FAILED;
    char *tname = "M.2 USB read/write";
    char *m2_dev = "/dev/m2usb";
    
    if (is_m2_usb_device() == FALSE) {
        return (PASSED);
    }

    testname("%s access", tname);

    prpass(testpass, "%s, ", tname);

    rc = m2_device_tests((uchar *)m2_dev);
    if (rc == FAILED) {
        cterr('f', 0, "M.2 USB test failed.");
    }

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : check_m2_device
 * Description: Check M2 device(USB or SATA)
 * Inputs     : which_m2_dev - Get M.2 USB, M.2 SATA or none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int check_m2_device (unsigned int *which_m2_dev)
{
    uint32_t reg_val = 0;

    dash_fpga_reg_read(FPGA_M2_CTLSTS_REG, &reg_val);

    /* Check Bit 4 M.2 Module Present first */
    if ((reg_val & FPGA_M2_MODULE_PRESENT) == FPGA_M2_MODULE_PRESENT) {

        if ((reg_val & FPGA_M2_DEVICE_PRESENT_MASK) == FPGA_M2_DEVICE_PRESENT_MASK) {
            cterr('f', 0, "FPGA M.2 Present bit has some problem.(%x)\n", reg_val);
            return (FAILED);
        }

        if ((reg_val & FPGA_M2_USB_PRESENT) == FPGA_M2_USB_PRESENT) {
            *which_m2_dev = FPGA_M2_USB_PRESENT;
        } else if ((reg_val & FPGA_M2_SATA_PRESENT) == FPGA_M2_SATA_PRESENT) {
            *which_m2_dev = FPGA_M2_SATA_PRESENT;
        } else {
            cterr('f', 0, "Cannot get correct M.2 device. (%x)\n", reg_val);
            return (FAILED);
        }

    } else {
        *which_m2_dev = FGPA_M2_NO_DEVICE;
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : check_m2_device
 * Description: Check M2 device(USB, SATA or PCIE)
 * Inputs     : None
 * Outputs    : 
 *
 *******************************************************************************
 */
int check_m2_device_utility (void)
{
    FILE *fp;
    char buf[12] = {0};

    system(CHECK_M2_DEV_SCRIPT);

    fp = fopen(M2_DEVICE_PLUG, "r");
    if (fp == NULL) {
        cterr('f',0, "can't open file plug_m2_device.txt");
        return (FAILED);
    }

    fgets(buf, sizeof(buf), fp);

    if (strstr(buf, M2_SATA_DEV)) {
        printf("M2_SATA_DEV\n");
    } else if (strstr(buf, M2_USB_DEV)) {
        printf("M2_USB_DEV\n");
    } else if (strstr(buf, M2_PCIE_DEV)) {
        printf("M2_PCIE_DEV\n");
    } else {
        printf("No M2 device\n");
    }

    fclose(fp);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : is_m2_usb_device
 * Description: is M2 USB Device
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_m2_usb_device (void)
{
    unsigned int m2_device = 0;

    check_m2_device(&m2_device);

    if (m2_device == FPGA_M2_USB_PRESENT) {
        return (TRUE);
    } 

    return (FALSE);

}

/*******************************************************************************
 *
 * Function   : is_m2_sata_device
 * Description: is M2 SATA Device
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_m2_sata_device (void)
{
    unsigned int m2_device = 0;

    check_m2_device(&m2_device);

    if (m2_device == FPGA_M2_SATA_PRESENT) {
        return (TRUE);
    } 

    return (FALSE);

}

/*-------------------------------------------------
 * $Log: diag_m2_test.c,v $
 * Revision 1.2  2019/12/11 10:10:30  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
