/* $Id: canis_rbcp_main.c,v 1.11 2013/05/02 21:35:57 shhuang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/canis/canis_rbcp_main.c,v $
 *------------------------------------------------------------------
 * Filename: canis_rbcp_main.c
 *
 * Description: The RBCP main source code
 * Author: Times Huang
 *
 * Copyright (c) 2012-2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "canis_rbcp_main.h"
#include "canis_rbcp_lib.h"
#include "canis_rbcp_platform.h"

#include <stdio.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
int canis_rbcp_heartbeat_test(void);    /* also called by interface test */
int canis_rbcp_registration_test(int);  /* also called by interface test */

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

int canis_rbcp_menu(void);
long build_canis_rbcp_menu(int);

int canis_rbcp_pwroff_intel(void);
int canis_rbcp_pwron_intel(void);
int canis_rbcp_bmc_console_switch(void);
int canis_rbcp_intel_console_switch(void);


/***********************************************************************
 *  Externs
 ************************************************************************/
extern int do_all_menu_items(struct menuinfo *);

extern int canis_rbcp_register(void);
extern int canis_rbcp_ping(void);
extern int canis_rbcp_intel_power_on(void);
extern int canis_rbcp_intel_power_off(void);
extern int canis_rbcp_bmc_con_switch(int);
extern int canis_rbcp_intel_con_switch(int);
extern int canis_get_mac(uchar);

extern int canis_test_slot;

/***********************************************************************
 *  Global Variable
 ************************************************************************/
static int regis_done_flag[MAX_NUM_CANIS_SLOTS]={FALSE};

static submenu_xtable_t canis_rbcp_submenu_tbl[] = {
    { "RBCP Registration Test", (type_t(*)())canis_rbcp_registration_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP heart beat Test",   (type_t(*)())canis_rbcp_heartbeat_test, FALSE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to Intel", (type_t(*)())canis_rbcp_intel_con_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to BMC", (type_t(*)())canis_rbcp_bmc_con_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP power off Intel", (type_t(*)())canis_rbcp_pwroff_intel, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP power on Intel", (type_t(*)())canis_rbcp_pwron_intel, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Registration Util", (type_t(*)())canis_rbcp_registration_test, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define CANIS_RBCP_SUBMENU_TABLE_SZ \
                (sizeof(canis_rbcp_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t canis_rbcp_primary_items[CANIS_RBCP_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t canis_rbcp_secondary_items[CANIS_RBCP_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

static menuinfo_t canis_rbcp_main_menu = {
    "Canis RBCP Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    canis_rbcp_primary_items,
};
static menuinfo_t *canis_rbcp_menup = &canis_rbcp_main_menu;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_canis_rbcp_menu
 *
 * Description: Build Canis RBCP tests and utilities menu.
 *
 * Inputs:      show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_canis_rbcp_menu (int show_menu)
{

    build_primary_submenu(canis_rbcp_submenu_tbl, CANIS_RBCP_SUBMENU_TABLE_SZ,
                          "RBCP Main Menu", &canis_rbcp_menup);
    build_secondary_submenu(canis_rbcp_submenu_tbl, CANIS_RBCP_SUBMENU_TABLE_SZ,
                            canis_rbcp_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(canis_rbcp_menup, canis_rbcp_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(canis_rbcp_menup);
        //prcomplete(testpass, errcount, 0);
    }

    return(PASSED);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/


/**********************************************************************
 *
 * Function: canis_rbcp_registration_test
 *
 * Description: This function provides RBCP testing.
 *
 * Input :  menu_option - 0 : indicates its a test
 *			  1 : indicates its a utility
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int canis_rbcp_registration_test (int menu_option)
{
    int rc;

    /* Enabled RBCP registeration if called from utils */
    if (menu_option==TRUE) {
        regis_done_flag[canis_test_slot] = FALSE;
    }

    if (!regis_done_flag[canis_test_slot]) {
        /* RBCP Registration is not yet done */
        prpass(testpass, "RBCP Registration");

        rc = canis_rbcp_register();

        if (rc) {
            cterr('f', 0, "RBCP registration test failed");
            return(FAILED);
        } else {
            prcomplete(testpass, errcount, 0);
        }
    } else {
        /* RBCP Registration was already done earlier*/
        printf("\nRegistration was already done. So nothing executed!\n");
    }
 
    /* Once RBCP registration is done, set this flag */
    regis_done_flag[canis_test_slot] = TRUE;

    return (PASSED);
}

/**********************************************************************
 *
 * Function: canis_rbcp_heartbeat_test
 *
 * Description: This function provides RBCP testing.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int canis_rbcp_heartbeat_test (void)
{
    int rc,j;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    canis_rbcp_registration_test (FALSE);

    prpass(testpass, "RBCP heartbeat"); 

    /* retries: see CDETS: CSCtx52567 */
    for (j=0; j < CANIS_RBCP_RETRIES; j++) {
        rc = canis_rbcp_ping();
        if (!rc) {
            break;
        }
    }

    if (rc) {
        switch (rc) {
            case RBCP_SEND_FAILURE:
                cterr('f', 0, "RBCP send heartbeat opcode failed");
                break;
            case RBCP_RECV_FAILURE:
                cterr('f', 0, "Did not receive Heartbeat opcode from BMC"
                      " after %d tries\n", CANIS_RBCP_RETRIES);
                break;
            default:
                cterr('f', 0, "RBCP heartbeat test failed");
        }
        return(FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: canis_rbcp_pwroff_intel
 *
 * Description: This function power off Intel.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int canis_rbcp_pwroff_intel(void)
{
    int rc,jx;

    prpass(testpass, "RBCP Intel power off"); 

    /* call RBCP register to ensure we have registered RBCP into BMC */
    canis_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < CANIS_RBCP_RETRIES; jx++) {
    rc = canis_rbcp_intel_power_off();
        if (!rc) break;
    }

    if (rc) {
        switch (rc) {
            case RBCP_SEND_FAILURE:
                cterr('f', 0, "RBCP send CISCO_SCP_INTEL_PWR_OFF opcode"
                      " failed");
                break;
            case RBCP_RECV_FAILURE:
                cterr('f', 0, "Did not receive CISCO_SCP_INTEL_PWR_OFF opcode"
                      " from BMC\n");
                break;
            default:
                cterr('f', 0, "RBCP Intel power off failed");
        }
        return(FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: canis_rbcp_pwron_intel
 *
 * Description: This function power off Intel.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int canis_rbcp_pwron_intel(void)
{
    int rc,jx;

    prpass(testpass, "RBCP Intel power on"); 

    /* call RBCP register to ensure we have registered RBCP into BMC */
    canis_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < CANIS_RBCP_RETRIES; jx++) {
    rc = canis_rbcp_intel_power_on();
        if (!rc) {
            break;
        }
    }

    if (rc) {
        switch (rc) {
            case RBCP_SEND_FAILURE:
                cterr('f', 0, "RBCP send CISCO_SCP_INTEL_PWR_ON opcode"
                      " failed");
                break;
            case RBCP_RECV_FAILURE:
                cterr('f', 0, "Did not receive CISCO_SCP_INTEL_PWR_ON opcode"
                      " from BMC\n");
                break;
            default:
                cterr('f', 0, "RBCP Intel power on failed");
            }
        return(FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }
    
    return (PASSED);
}

/*  fix warning, defined but not used
static int canis_rbcp_tbd (void)
{

    printf("\n!!!  Test not yet implemented !!!\n");

    return (PASSED);
}
*/

int canis_rbcp_bmc_console_switch (void)
{
    int rc,jx;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    canis_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < CANIS_RBCP_RETRIES; jx++) {
        rc = canis_rbcp_con_sw_bmc();
        if (!rc) {
            break;
        }
    }

    if (rc) {
        switch (rc) {
            case RBCP_SEND_FAILURE:
                cterr('f', 0, "RBCP send CISCO_SCP_UART_CONSOLE_SW to BMC opcode"
                      " failed");
                break;
            case RBCP_RECV_FAILURE:
                cterr('f', 0, "Did not receive CISCO_SCP_UART_CONSOLE_SW opcode"
                      " from BMC\n");
                break;
            default:
                cterr('f', 0, "RBCP Intel power on failed");
       }
       return(FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }

    return(PASSED);
}

int canis_rbcp_intel_console_switch (void)
{
    int rc,jx;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    canis_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < CANIS_RBCP_RETRIES; jx++) {
        rc = canis_rbcp_con_sw_intel();
        if (!rc) {
            break;
        }
    }

    if (rc) {
        switch (rc) {
            case RBCP_SEND_FAILURE:
                cterr('f', 0, "RBCP send CISCO_SCP_UART_CONSOLE_SW to BMC opcode"
                      " failed");
                break;
            case RBCP_RECV_FAILURE:
                cterr('f', 0, "Did not receive CISCO_SCP_UART_CONSOLE_SW opcode"
                      " from BMC\n");
                 break;
            default:
                 cterr('f', 0, "RBCP Intel power on failed");
        }
        return(FAILED);
    } else {
         prcomplete(testpass, errcount, 0);
    }

    return(PASSED);
}

void clear_regis_done_flag (int slot)
{
    regis_done_flag[slot] = FALSE;
}
/*------------------------------------------------------------------
 * $Log: canis_rbcp_main.c,v $
 * Revision 1.11  2013/05/02 21:35:57  shhuang
 * Add Canis BMC ready bit set too early work-around function. (CSCug49316)
 * Add utility to switch to the work-around function.
 *
 * Revision 1.10  2012/12/20 06:24:16  hondwang
 * Fill matrix valuse. Print debug info and increase retry to six
 *
 * Revision 1.9  2012/10/11 07:28:20  hondwang
 * porting multi card insert issue fix from G2. CSCua22608
 *
 * Revision 1.8  2012/09/18 20:40:39  shhuang
 * Added canis interface test. Cleaned up.
 *
 * Revision 1.7  2012/08/02 18:49:41  shhuang
 * Added LTC4215 register r/w test and utilities.
 * Added Canis SM power off/on/cycle utilities.
 * Minor fixes of testname/prpass strings.
 *
 * Revision 1.6  2012/07/09 19:26:17  shhuang
 * Added function to check BMC ready during RPCP submenu init.
 * Cleaned up prass function calls.
 *
 * Revision 1.5  2012/06/27 09:39:46  hondwang
 * revise canis source code for proper indent
 *
 * Revision 1.4  2012/06/08 06:45:05  hondwang
 * Fix canis complier warning on O2 x86
 *
 * Revision 1.3  2012/04/24 08:30:56  hondwang
 * Add RBCP for Canis
 *
 * Revision 1.2  2012/04/10 06:10:46  hondwang
 * fix warning message
 *
 * Revision 1.1  2012/03/29 18:46:42  ksabzwar
 * Initial check in into ng_diag
 *
 * Revision 1.1.4.2  2012/03/10 01:18:28  ksabzwar
 * First check-in for Canis user menu for Overloard platform
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

