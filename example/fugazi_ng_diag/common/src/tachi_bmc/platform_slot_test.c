/* $Id: platform_slot_test.c,v 1.3 2017/03/30 08:34:08 hondwang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/platform_slot_test.c,v $
 *------------------------------------------------------------------
 *
 * platform_slot_test.c - Platform specific slot test functions.
 *                        An entry for NIM test on BMC. 
 *
 * Oct 2015, Alan Peng
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
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
#include "slot.h"
#include "error.h"
#include "menu.h"
#include "oir_ltc4215_api.h"
#include "platform_slot_test.h"
#include "diag_nc_common.h"
#include "diag_eth_pkt_txrx.h"
#include "diag_eth_pkt_txrx_api.h"
#include "diag_lewis_gesw_test.h"
#include "intel_tests.h"

#define MM_1    (MF_CONTINUOUS | MF_DOGRP)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)

/* Functions */
static int ltc4215_register_test(void);
static int is_item_available(int);
static int tachi_bmc_nim_test(void *, char *, unsigned int);
extern int diag_nc_nim_dl_test (void);
extern int slot_get_info(struct ngio_intf_t *ngio, char*);

/* Dreamliner Functions */
static uint32_t nim_dl_gephy_lpbk_test(int); 

/* Static */
static void (*tachi_nim_saved_diag_exec)(void) = NULL;
static unsigned int test_item = 0; 
static struct ngio_intf_t *tachi_nim_iface; 
static void *oir_if;


/* General Menu items */
static submenu_xtable_t main_menu_table[] = {
    {"LTC4215 Register Test", (PFT)ltc4215_register_test,   0, MM_3, 
      (type_t(*)())is_item_available, NIM_OIR_TEST,
      (type_t(*)())0, 0},
    {"Dreamliner Tests (nc)", (PFT)diag_nc_nim_dl_test,     0, MM_3, 
      (type_t(*)())is_item_available, NIM_DL_GENARAL_TEST,
      (type_t(*)())0, 0},
    {"Dreamliner GE0 to PHY internal lpbk test", (PFT)nim_dl_gephy_lpbk_test,
       NIM_DL_GE0_PHY_INTR_LPBK, MM_3, 
      (type_t(*)())is_item_available, NIM_DL_GE0_INT_LPBK_TEST,
      (type_t(*)())0, 0},
    {"Dreamliner GE1 to PHY internal lpbk test", (PFT)nim_dl_gephy_lpbk_test, 
       NIM_DL_GE1_PHY_INTR_LPBK, MM_3, 
      (type_t(*)())is_item_available, NIM_DL_GE1_INT_LPBK_TEST,
      (type_t(*)())0, 0},
    {"Dreamliner PHY external lpbk test", (PFT)nim_dl_gephy_lpbk_test,
       NIM_DL_PHY_EXT_LPBK, MM_3, 
      (type_t(*)())is_item_available, NIM_DL_EXT_LPBK_TEST,
      (type_t(*)())0, 0},
};

#define MAIN_MENU_TABLE_SIZE \
        (sizeof(main_menu_table) / sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t main_menu_primary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t main_menu_secondary_items[MAIN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo maindiag = {
    "XXX Main Menu",            /* title - filled by modules  */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    main_menu_primary_items,
};
static struct menuinfo *maindiagp = &maindiag;

/*
 * Function:	nim_dl_gephy_lpbk_test
 * Description:	tachi dreamliner phy loopback test entry
 * Inputs:	type - 0: ge0 to phy internal loopback test 
 *                     1: ge1 to phy internal loopback test 
 *                     2: ge0 to phy external loopback test 
 * Output:	ret - PASSED/Failed. 
 */
uint32_t nim_dl_gephy_lpbk_test (int type) 
{
    uint32_t port, retval = PASSED; 

    if (type == NIM_DL_GE0_PHY_INTR_LPBK) {
        testname("Dreamliner GE0 to PHY Internal Loopback test");
    } else if (type == NIM_DL_GE1_PHY_INTR_LPBK) {
        testname("Dreamliner GE1 to PHY Internal Loopback test");
    } else {
        testname("Dreamliner GE0 to PHY External Loopback test");
    }
    
    system("rm /tmp/nc*");

    for (port = 1; port <= 8; port++) {

        prpass(testpass, " port%d ", port);
        diag_nc_nim_dl_lpbk(type, port, ENABLE); 

        if (eth_pkt_txrx(ETH1_MAC1, 3, FALSE) == FAILED) {
            cterr('f', 0, "Dreamliner lpbk test, type%d failed on port%d\n",
                          type, port);
            retval = FAILED;
            diag_nc_nim_dl_lpbk(type, port, DISABLE); 
            return (retval); 
        }
        diag_nc_nim_dl_lpbk(type, port, DISABLE); 
    }

    /* Reset Slot for fix LB only PASS twice issue.*/
    if (tachi_nim_iface->off) {
            tachi_nim_iface->off(tachi_nim_iface);
    }

    if (slot_get_info(tachi_nim_iface, "WIC") == FAILED) {
       cterr('f', 0, "fail to reset slot info.\n");
       return (FAILED);
    }

    return (retval);
}

/*
 * Function:	tachi_dl_test
 * Description:	tachi dreamliner test entry
 * Inputs:	nim - nim interface structure. 
 * Output:	ret - PASSED/Failed. 
 */
uint32_t tachi_dl_test (void *nim)
{
    char name_buf[32];
    unsigned int item;
    int ret;

    /* dreamliner needs intel and lewis ready to perform full test */
    if (check_intel_linux_ready()) {
        printf("Could not auto check INTEL linux ready.\n");
        return (FAILED);
    } 

    /* Name */
    strcpy(name_buf, "Dreamliner");

    /* init function */
    /* N/A for dreamliner */

    /* Interface for test */
    item |= NIM_OIR_TEST;

    /* General items for test */
    item |= NIM_DL_GENARAL_TEST; 
    item |= NIM_DL_GE0_INT_LPBK_TEST; 
    item |= NIM_DL_GE1_INT_LPBK_TEST; 
    item |= NIM_DL_EXT_LPBK_TEST; 

    /* Calling menu for test */
    ret = tachi_bmc_nim_test(nim, name_buf, item);

    return (ret);
}

/*
 * Function: ltc4215_register_test
 * Description: A wrapper function for LTC4215 register test.
 * Input : iface  - Patriot ds info
 * Output: PASSED/FAILED
 */
static int
ltc4215_register_test (void)
{
    prpass(testpass, "LTC4215 OIR Register test");
    return (oir_ltc4215_register_test(oir_if));
}

/*
 * Function:	is_item_available
 * Description:	This function check the item is available for NIM
 * Inputs:	item - item for check 
 * Output:	TRUE/FALSE
 */
static int 
is_item_available (int item)
{
    if (test_item & item) {
        return (TRUE); 
    }

    return (FALSE);
}

/*
 * Function:	tachi_nim_iface_test
 * Description:	tachi nim interface interface test. 
 * Inputs:	None 
 * Output:	PASSED/FAILED
 */
int tachi_nim_iface_test (void) 
{
    int ret = PASSED; 

    if (test_item & NIM_OIR_TEST) {
        ret |= ltc4215_register_test();
    }
    
    if (test_item & NIM_DL_GENARAL_TEST) {
        ret |= diag_nc_nim_dl_test();
    }
    
    if (test_item & NIM_DL_GE0_INT_LPBK_TEST) {
        ret |= nim_dl_gephy_lpbk_test(NIM_DL_GE0_PHY_INTR_LPBK);
    }

    sleep(NIM_DL_SEPAR_LB_TEST_DELAY);

    if (test_item & NIM_DL_GE1_INT_LPBK_TEST) {
        ret |= nim_dl_gephy_lpbk_test(NIM_DL_GE1_PHY_INTR_LPBK);
    }
   
    return (ret);
}

/*
 * Function:	tachi_nim_doall_test
 * Description:	tachi nim interface do all test. 
 * Inputs:	None 
 * Output:	PASSED/FAILED
 */
int tachi_nim_doall_test (void) 
{
    int ret = PASSED; 

    if (test_item & NIM_DL_GENARAL_TEST) {
        ret |= diag_nc_nim_dl_test();
    }
    if (test_item & NIM_DL_GE0_INT_LPBK_TEST) {
        ret |= nim_dl_gephy_lpbk_test(NIM_DL_GE0_PHY_INTR_LPBK);
    }
    sleep(NIM_DL_SEPAR_LB_TEST_DELAY);
    if (test_item & NIM_DL_GE1_INT_LPBK_TEST) {
        ret |= nim_dl_gephy_lpbk_test(NIM_DL_GE1_PHY_INTR_LPBK);
    }
    sleep(NIM_DL_SEPAR_LB_TEST_DELAY);
    if (test_item & NIM_DL_EXT_LPBK_TEST) {
        ret |= nim_dl_gephy_lpbk_test(NIM_DL_EXT_LPBK_TEST);
    }

    return (ret); 
}

/*
 * Function:	tachi_bmc_nim_test
 * Description:	general nim test menu and tests update. 
 * Inputs:	
 * Output:	
 */
int 
tachi_bmc_nim_test (void *nim, char *title, unsigned int intf)
{
    int ret; 
    char buf[32]; 

    /* get ngio struct */
    tachi_nim_iface = (struct ngio_intf_t *)nim; 
    oir_if = (void *)(tachi_nim_iface->oir); 

    /* update menu title */
    maindiag.mtitle = buf; 
    sprintf(buf, "%s Main Menu", title); 

    /* update interface tests */
    test_item = intf; 

    /* update doall tests */
    /* implement? */

    testname("Slot%d %s NIM ", tachi_nim_iface->slot, title);

    /* bring up menu */
    tachi_nim_saved_diag_exec = pre_diag_exec;
    pre_diag_exec = NULL;

    build_primary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE, dgmenustr,
                          &maindiagp);
    build_secondary_submenu(main_menu_table, MAIN_MENU_TABLE_SIZE,
                            main_menu_secondary_items);

    if (tachi_nim_iface->test_type == IFACE_TEST) {
        ret = tachi_nim_iface_test();
    } else if (tachi_nim_iface->menu_display == FALSE) {
        ret = tachi_nim_doall_test(); 
    } else {
        menu(maindiagp, main_menu_secondary_items, '\0');
    }

    /* clean up func, if it is needed */
    
    prcomplete(testpass, errcount, 0);

    return (ret); 
}

/******** History ******** 
$Log: platform_slot_test.c,v $
Revision 1.3  2017/03/30 08:34:08  hondwang
Tachi-L brach merge

Revision 1.2.14.2  2017/03/07 03:28:18  hondwang
Fix dreamliner and wallander IO interface test

Revision 1.2.14.1  2016/12/21 12:43:43  hondwang
Fix dreamliner loopback issue

Revision 1.2  2016/04/20 11:25:31  benchen2
add tachi fru portion

Revision 1.1.2.6  2015/12/30 08:37:23  alpeng
 support nc test, check intel and lewis ready before testing

Revision 1.1.2.5  2015/12/29 12:31:06  alpeng
support get_mb_pid for check MB sku

Revision 1.1.2.4  2015/12/17 16:37:02  jskow
Create Lewis utility menu/file to run some utilities from BMC.  Update Lewis test file functions to allow running utilities and tests on Switch.

Revision 1.1.2.3  2015/12/17 03:46:30  alpeng
support dreamliner nc and poe

Revision 1.1.2.2  2015/12/09 10:35:57  alpeng
update code to support lpbk test on bmc for dreamliner

Revision 1.1.2.1  2015/09/26 05:22:35  alpeng
update nim test entry


$Endlog$
*/
