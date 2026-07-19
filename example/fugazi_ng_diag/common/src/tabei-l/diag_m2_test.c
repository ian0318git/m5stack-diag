 /* $Id: diag_m2_test.c,v 1.3 2021/05/13 08:49:58 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_m2_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_m2_test.c - This file is for m.2 device test 
 *
 *
 * Copyright (c) 2018-2021 by Cisco Systems, Inc.
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
#include "diag_fpga.h"
#include "nvmonvars.h"
#include "queryflags.h"
#include "diag_m2_test.h"
#include "diag_storage_lib.h"
#include "linux_block_test.h"
#include "m2_testcard_host_impl.h"

/*
 * Global variables
 */


/* Local functions */
int diag_m2_sata_test(void);
int diag_m2_usb_test(void);
int diag_m2_pcie_test(void);
int is_m2_sata_device (void);
int is_m2_usb_device (void);
int is_m2_nvme_device (void);
int check_m2_device (unsigned int *);

/* Local build menu functions */
int build_m2_test_menu(boolean);


/*
 * Sub Menu used for "M.2 device test -> M.2 Device submenu test"
 */

submenu_xtable_t m2_submenu_table[] = {
    {"M.2 USB Test",
     (PFT) diag_m2_usb_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"M.2 PCIe Test",
     (PFT) diag_m2_pcie_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())has_m2pcie, 0, (PFT) 0, 0},

    {"M.2 SATA Test",
     (PFT) diag_m2_sata_test, FALSE,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())has_m2sata, 0, (PFT) 0, 0},
 
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

    /* Check if M.2 testcard is plugged */
    if (is_m2_testcard_in() == TRUE) {
        printf("\nNow is plugging in M.2 testcard, skip this test.\n");
        return (PASSED);
    }

    /* Check M.2 Present */
    if (check_m2_device(&m2_device) == FAILED) {
        return (FAILED);
    }

    if (has_m2pcie() == TRUE) {
        if (m2_device == FPGA_M2_PCIE_PRESENT) {
            printf("M2 PCIE DEV Present\n");
        } else if (m2_device == FPGA_M2_USB_PRESENT) {
            printf("M2 USB DEV Present\n");
        } else if (m2_device == FGPA_M2_NO_DEVICE) {
            cterr('w', 0, "M.2 slot vacant. ");
            return (FAILED);
        }
    } else {
        if (m2_device == FPGA_M2_USB_PRESENT) {
            printf("M2 USB DEV Present\n");
        } else if (m2_device == FPGA_M2_SATA_PRESENT) {
            printf("M2 SATA DEV Present\n");
        } else if (m2_device == FGPA_M2_NO_DEVICE) {
            cterr('w', 0, "M.2 slot vacant. ");
            return (FAILED);
        }
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
int diag_m2_sata_test (void) 
{

    int rc = FAILED;  
    char *tname = "M.2 SATA read/write";
    char *m2_dev = M2_SATA_DEV;

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
int diag_m2_usb_test (void) 
{

    int rc = FAILED;
    char *tname = "M.2 USB read/write";
    char *m2_dev = M2_USB_DEV;

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
 * Function   : diag_m2_pcie_test
 * Description: main test for m2 PCIE test
 * Inputs     : void
 * Outputs    : PASSED or FAILED.
 *
 *******************************************************************************
 */
int diag_m2_pcie_test (void)
{
    int rc = FAILED;
    char *tname = "M.2 PCIE read/write";
    char *m2_dev = M2_NVME_DEV;

    if (is_m2_nvme_device() == FALSE) {
        return (PASSED);
    }

    testname("%s access", tname);

    prpass(testpass, "%s, ", tname);

    rc = m2_device_tests((uchar *)m2_dev);
    if (rc == FAILED) {
        cterr('f', 0, "M.2 PCIE test failed.");
    }

    prcomplete(testpass, errcount, (char *)0);

    return (rc);
}

/*******************************************************************************
 *
 * Function   : check_m2_device
 * Description: Check M2 device(USB or PCIE)
 * Inputs     : which_m2_dev - Get M.2 USB, M.2 PCIE or none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int check_m2_device (unsigned int *which_m2_dev)
{
    uint32_t reg_val = 0;

    fpga_read_reg(FPGA_M2_CTLSTS_REG, &reg_val);

    /* Check Bit 4 M.2 Module Present first */
    if ((reg_val & FPGA_M2_MODULE_PRESENT) == FPGA_M2_MODULE_PRESENT) {

        if ((reg_val & FPGA_M2_DEVICE_PRESENT_MASK) == FPGA_M2_DEVICE_PRESENT_MASK) {
            cterr('f', 0, "FPGA M.2 Present bit has some problem.(%x)\n", reg_val);
            return (FAILED);
        }

        if ((reg_val & FPGA_M2_PCIE_PRESENT) == FPGA_M2_PCIE_PRESENT) {
            *which_m2_dev = FPGA_M2_PCIE_PRESENT;
        } else if ((reg_val & FPGA_M2_USB_PRESENT) == FPGA_M2_USB_PRESENT) {
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
 * Function   : check_m2_device_utility
 * Description: Check M2 device(USB or PCIE)
 * Inputs     : which_m2_dev - Get M.2 USB, M.2 PCIE or none
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int check_m2_device_utility (void)
{
    unsigned int m2_device = 0;

    if (check_m2_device(&m2_device) == FAILED) {
        return (FAILED);
    }


    if (m2_device == FPGA_M2_PCIE_PRESENT) {
        printf("M2_PCIE_DEV\n");
    } else if (m2_device == FPGA_M2_USB_PRESENT) {
        printf("M2_USB_DEV\n");
    } else if (m2_device == FGPA_M2_NO_DEVICE) {
        printf("No M.2 device\n");
    } else {
        printf("*** WARNING unknown status\n");
    }

    return (PASSED);

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
 * Function   : is_m2_nvme_device
 * Description: is M2 NVMe Device
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *
 *******************************************************************************
 */
int is_m2_nvme_device (void)
{
    unsigned int m2_device = 0;

    check_m2_device(&m2_device);

    if (m2_device == FPGA_M2_PCIE_PRESENT) {
        return (TRUE);
    } 

    return (FALSE);

}

/*-------------------------------------------------
 * $Log: diag_m2_test.c,v $
 * Revision 1.3  2021/05/13 08:49:58  kodko
 * Support M.2 testcard.
 *
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.13  2019/09/27 01:36:25  kehuang2
 * Clean up code
 *
 * Revision 1.1.2.12  2019/09/10 06:14:15  olin2
 * Clean up code
 *
 * Revision 1.1.2.11  2019/08/06 07:20:28  kehuang2
 * Update present function base on the comment of code review
 *
 * Revision 1.1.2.10  2019/07/11 03:38:39  kehuang2
 * Support M.2 detect mechanism on fortnite
 *
 * Revision 1.1.2.9  2019/07/04 03:23:36  kehuang2
 * Combine Tabei-L sereies image together(Fortnite, Tabei-L, Promethium)
 *
 * Revision 1.1.2.8  2019/06/20 09:05:56  kehuang2
 * Support linux_block_test function
 *
 * Revision 1.1.2.7  2019/05/22 07:42:30  olin2
 * Enhance M.2 device test and remove M.2 SATA test
 *
 * Revision 1.1.2.6  2019/05/07 06:08:33  olin2
 * Check M.2 device present through FPGA
 *
 * Revision 1.1.2.5  2019/05/03 02:43:16  olin2
 * Modify M.2 NVMe device name
 *
 * Revision 1.1.2.4  2019/03/21 06:13:14  olin2
 * Add external loopback flag check
 *
 * Revision 1.1.2.3  2019/01/18 02:30:15  olin2
 * Clean up code
 *
 * Revision 1.1.2.2  2019/01/03 03:16:48  harrchan
 * Add distinguish sku function
 *
 * Revision 1.1.2.1  2018/12/21 07:09:47  olin2
 * Update M.2 device menu
 *
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
