/* $Id: diag_boot_flash_test.c,v 1.4 2019/07/11 12:31:26 alicehua Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nutella/diag_boot_flash_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_boot_flash_test.c - Boot flash test
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "diag_storage_lib.h"
#include "diag_boot_flash_test.h"
#include "linux_block_test.h"



/*
 * Global variables
 */


/* Local functions */
int diag_boot_flash_test(int);
int build_boot_flash_menu(boolean);


/*
 * Sub Menu used for "Boot flash test -> Boot flash submenu test"
 */
submenu_xtable_t boot_flash_submenu_table[] = {
    {"Primary Bootflash Test",
     (PFT) diag_boot_flash_test, FIRST_BOOTFLASH,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, (PFT) 0, 0},

    {"Secondary Bootflash Test",
     (PFT) diag_boot_flash_test, SECONDARY_BOOTFLASH,
     MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
     (type_t(*)())0, 0, 
     (PFT) 0, 0},
};

#define BOOT_FLASH_SUBMENU_TABLE_SIZE (sizeof(boot_flash_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/*
 * "motherboard test -> boot flash test" primary & secondary submenu items (filled in from xtable)
 */
static mitem_t boot_flash_primary_items[BOOT_FLASH_SUBMENU_TABLE_SIZE +
                                      MAX_BASE_ITEMS];
static mitem_t boot_flash_secondary_items[BOOT_FLASH_SUBMENU_TABLE_SIZE +
                                        MAX_BASE_ITEMS];

menuinfo_t boot_subtest_menu = {
    "%s Subtest Menu",
    0,                          /* mtparam added by init_empty_menu */
    (PFT) show_endnote,         /* notes missing WICs in combos */
    0,                          /* use generic prompt */
    0,                          /* size (bumped by add_menu_item() */
    boot_flash_primary_items,
};

menuinfo_t *boot_submenup = &boot_subtest_menu;


/*******************************************************************************
 *
 * Function   : build_boot_flash_menu
 * Description: build the menu of boot flash 
 * Inputs     : Test/Menu 
 * Outputs    : PASSED
 *
 *******************************************************************************
 */
int build_boot_flash_menu (boolean mb_temp_test_items_executed)
{
    char *tname = "Bootflash Test";
    testname(tname);

    build_primary_submenu(boot_flash_submenu_table, BOOT_FLASH_SUBMENU_TABLE_SIZE,
                          "Boot flash test", &boot_submenup);
    build_secondary_submenu(boot_flash_submenu_table, BOOT_FLASH_SUBMENU_TABLE_SIZE,
                            boot_flash_secondary_items);
    if (mb_temp_test_items_executed) {
        menu(&boot_subtest_menu, boot_flash_secondary_items, 0);
    } else {
        do_all_menu_items(boot_submenup);
    }
    return (PASSED);
}



/******************************************************************************
 *
 * Function: diag_boot_flash_test
 *
 * Description: Boot flash test
 *
 * Inputs      : select - select primary or secondary bootflash test
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_boot_flash_test (int select)
{
    char *tname[] = {"Boot flash 1", "Boot flash 2"};
    uint reg_addr;
    uint32_t  reg_bk;
    uint wr_data;
    
    reg_addr = FPGA_SPI_CONTROL_REG;
    fpga_read_reg(reg_addr, &reg_bk);
    

    if (select == FIRST_BOOTFLASH) {
        wr_data = ENABLE_FIRST_BOOTFLASH;
        testname("%s", tname[0]);
    } else if (select == SECONDARY_BOOTFLASH) {
        wr_data = ENABLE_SECONDARY_BOOTFLASH;
        testname("%s", tname[1]);
    } else {
        cterr('f','0',"Wrong index of select bootflash\n");
        return (FAILED);
    }

    /*
     * Nutella sets MUX from FPGA to select the bootflash
     */
    fpga_write_reg(reg_addr, wr_data);
    prpass(testpass, "switch flash done");
    if (linux_block_test(BOOT_FLASH_DEV_PATH, SECTOR_OFFSET, BOOTFLASH_TEST_LEN, 
                         BLOCK_TEST_SEQUENTIAL, TRUE) != PASSED) {
        cterr('f', 0, "SPI bootflash test failed.");
        return (FAILED);
    }
    prpass(testpass, "Boot Flash read/write test passed, ");
    prcomplete(testpass, errcount, (char*)0);
    
    /* Restore MUX setting*/ 
    fpga_write_reg(reg_addr, reg_bk);

    return (PASSED);
}
/******** History ********
$Log: diag_boot_flash_test.c,v $
Revision 1.4  2019/07/11 12:31:26  alicehua
Collapse Nutella codes into main trunk.

$Endlog$
*/
