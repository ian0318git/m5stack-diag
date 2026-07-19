/* $Id: platform_cpu.c,v 1.2 2018/08/30 06:59:43 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/utah/platform_cpu.c,v $
 *------------------------------------------------------------------
 *
 * platform_cpu.c
 *
 * Copyright (c) 2014-2018 by cisco Systems, Inc.
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
#include "platform_fru.h"
#include "platform_cookie.h"
#include "proto.h"
#include "plat_defs.h"
#include "linux_coretest.h"


#define ENHANCE_ERROR_MSG_RDY 1

#define POLLING_INTRVL 100 
#define MAX_POLLING_COUNTS 100
#define VG400_CPU_NUM 4
int quiet_launch = 0;


/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);
extern int build_cpu_test_menu (int);
extern int cpu_core_test(void);

/*
 * Global variables
 */
extern int quiet_launch;


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
int cpu_core_test (void)
{

    int rc = FAILED;
    char *tname = "CPU core";


    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (linux_cpu_core_test(VG400_CPU_NUM) != PASSED) {
        cterr('f', 0, "%s test failed.", tname);
        return (FAILED);
    }

    if (!quiet_launch) {
        prpass(testpass, "%s stress, ", tname);
    }

    if ((rc == PASSED) && (quiet_launch != 1)) {  
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
Revision 1.2  2018/08/30 06:59:43  haohsu
Collapse Vg400-branch to Main Trunk

Revision 1.1.2.2  2018/08/01 02:40:53  haohsu
Vg400 code change for branch

Revision 1.1.2.1  2018/08/01 02:21:50  haohsu
Vg400 code change

Revision 1.7  2018/07/10 00:28:08  lucywang
Enhanced CPU core test to  make sure each CPU core is activated

Revision 1.6  2018/04/15 22:03:30  palin2
Merged Vulcan back to maintrunk.

Revision 1.5  2018/02/27 07:21:04  hondwang
Fix Star platform ondie temp in low temperature check issue

Revision 1.4  2018/02/12 09:13:46  hondwang
merge Star CPU frequency check into main trunk

Revision 1.3  2018/02/09 09:56:54  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.20.1  2018/01/20 06:27:23  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2.4.4  2018/02/12 08:39:01  hondwang
Add CPU frequency check

Revision 1.2.4.3  2017/12/11 08:45:21  hondwang
Remove tsn_confirm_devbus_config hard code for Star

Revision 1.2.4.2  2017/11/22 09:45:46  hondwang
Fix demo SKU and menu show

Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
star branch c9xx initial check in

Revision 1.2  2017/08/02 14:21:47  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:19  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.3  2017/07/25 08:31:55  steja
1. Remove unused code.
2. Verified before check-in

Revision 1.1.6.2  2017/07/20 13:38:06  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.5.6.2  2017/07/07 10:20:50  hondwang
Fix Makefile and device bus config issue

Revision 1.1.4.5.6.1  2017/06/23 02:20:18  tirawan
Upload Star Second FPGA read parameter if this platform is Star with Pluggable module and correct LTE reset initialization

Revision 1.1.4.5.2.1  2017/07/18 14:23:37  steja
Code Cleanup

Revision 1.1.4.5  2016/11/29 02:54:39  palin2
Dynamically getting device bus window base from CPU register.

Revision 1.1.4.4  2016/11/01 07:29:20  petteng
Add enhanced error message

Revision 1.1.4.3  2016/10/04 06:39:08  petteng
Add enhanced error message

Revision 1.1.4.2  2016/06/30 06:22:50  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.4  2016/06/12 18:02:39  palin2
Updated CPU on-die temperature sensor read function.

Revision 1.1.2.3  2016/06/03 01:00:46  palin2
Added function to show CPU on die temperature.

Revision 1.1.2.2  2016/03/27 14:17:34  steja
update based on code review comment 3/25/2016

Revision 1.1.2.1  2016/03/25 08:05:40  steja
Add CPU Stress tools



*/
