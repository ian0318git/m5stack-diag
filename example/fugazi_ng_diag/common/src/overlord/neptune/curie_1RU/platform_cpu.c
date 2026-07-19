 /* $Id: platform_cpu.c,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_cpu.c,v $
 *------------------------------------------------------------------
 *
 * platform_cpu.c
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "platform_cpu.h"
#include "linux_coretest.h"

#define CURIE_CPU_NUM 16


/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);

/*
 * CPU Test Menu
 */

static submenu_xtable_t cpu_test_table[] = {
    {"CPU core test", (type_t(*)())cpu_core_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define CPU_TEST_TABLE_SZ \
        (sizeof(cpu_test_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t cpu_pri_test_items[CPU_TEST_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t cpu_sec_test_items[CPU_TEST_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t cpu_test_menu = {
    "CPU Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    cpu_pri_test_items,
};
static menuinfo_t *cpu_test_menup = &cpu_test_menu;


/*******************************************************************************
 * Function: cpu_core_test
 *
 * Description : Test for stress of cpu core & memory. 
 *              
 * Inputs: test option - auto or user assign
 *
 * Output: PASSED/FAILED
 ******************************************************************************* 
 */
int cpu_core_test (int do_more_test)
{
    int rc = FAILED;
    char *tname = "CPU core";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);
    
    if (linux_cpu_core_test(CURIE_CPU_NUM) != PASSED) {
        cterr('f', 0, "%s test failed.", tname);
        return (FAILED);
    }

    if ((rc == PASSED)) {  
        prpass(testpass, "%s test passed, ", tname);
    }

    prcomplete(testpass, errcount, (char *)0);
    return (rc);
}

/**********************************************************************
 *
 * Function: build_cpu_test_menu
 *
 * Description: Build CPU test menu.
 *
 * Inputs: db_test_items_executed - TRUE for do all of tests. FALSE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
int build_cpu_test_menu (int db_test_items_executed)
{
    int rc = FAILED;
    char *tname = "CPU";

    testname(tname);

    build_primary_submenu(cpu_test_table,
                          CPU_TEST_TABLE_SZ, "CPU",
                          &cpu_test_menup);

    build_secondary_submenu(cpu_test_table,
                            CPU_TEST_TABLE_SZ,
                            cpu_sec_test_items);

    if (db_test_items_executed) {
        do_all_menu_items(&cpu_test_menu);
    } else {
        menu(&cpu_test_menu, cpu_sec_test_items, '\0');
    }

    return (rc);
}

/*-------------------------------------------------
$Log: platform_cpu.c,v $
Revision 1.2  2019/08/06 06:56:12  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.1  2018/09/27 08:05:10  meho
Added multi-core test.


*/
