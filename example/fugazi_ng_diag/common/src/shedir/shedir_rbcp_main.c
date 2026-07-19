/* $Id: shedir_rbcp_main.c,v 1.3 2015/05/25 03:58:21 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/shedir/shedir_rbcp_main.c,v $
 *------------------------------------------------------------------
 * Filename: shedir_rbcp_main.c
 *
 * Description: The RBCP main source code
 * Author: Times Huang
 *
 * Copyright (c) 2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include <string.h>
#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "shedir_rbcp_main.h"
#include "rbcp_lib.h"
#include "rbcp_platform.h"
#include "platform_cookie.h"
#include "platform_margin_utils.h"
#include "module_fru.h"
#include <stdio.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
int shedir_rbcp_heartbeat_test(void);    /* also called by interface test */
int shedir_rbcp_registration_test(int);  /* also called by interface test */

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

long build_shedir_rbcp_menu(int);

int shedir_rbcp_pwroff_intel(void);
int shedir_rbcp_pwron_intel(void);
int shedir_rbcp_bmc_console_switch(void);
int shedir_rbcp_intel_console_switch(void);
/***********************************************************************
 *  Externs
 ************************************************************************/
extern int do_all_menu_items(struct menuinfo *);

extern int shedir_rbcp_intel_con_switch(void);
extern int shedir_rbcp_bmc_con_switch(void);
extern int shedir_test_slot;
extern ushort shedir_board_id;
extern void show_margins_cterr_wrapper(void);
/***********************************************************************
 *  Global Variable
 ************************************************************************/
static int regis_done_flag[MAX_NUM_SHEDIR_SLOTS]={FALSE};
#define ENHANCED_ERR_MSG_EXAMPLE 1

static submenu_xtable_t shedir_rbcp_submenu_tbl[] = {
    { "RBCP Registration Test", (type_t(*)())shedir_rbcp_registration_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP heart beat Test",   (type_t(*)())shedir_rbcp_heartbeat_test, FALSE,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to Intel", (type_t(*)())shedir_rbcp_intel_con_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to BMC", (type_t(*)())shedir_rbcp_bmc_con_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP power off Intel", (type_t(*)())shedir_rbcp_pwroff_intel, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP power on Intel", (type_t(*)())shedir_rbcp_pwron_intel, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Registration Util", (type_t(*)())shedir_rbcp_registration_test, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define SHEDIR_RBCP_SUBMENU_TABLE_SZ \
                (sizeof(shedir_rbcp_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t shedir_rbcp_primary_items[SHEDIR_RBCP_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t shedir_rbcp_secondary_items[SHEDIR_RBCP_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

static menuinfo_t shedir_rbcp_main_menu = {
    "Shedir RBCP Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    shedir_rbcp_primary_items,
};
static menuinfo_t *shedir_rbcp_menup = &shedir_rbcp_main_menu;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_shedir_rbcp_menu
 *
 * Description: Build Shedir RBCP tests and utilities menu.
 *
 * Inputs:      show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_shedir_rbcp_menu (int show_menu)
{

    build_primary_submenu(shedir_rbcp_submenu_tbl, SHEDIR_RBCP_SUBMENU_TABLE_SZ,
                          "RBCP Main Menu", &shedir_rbcp_menup);
    build_secondary_submenu(shedir_rbcp_submenu_tbl, SHEDIR_RBCP_SUBMENU_TABLE_SZ,
                            shedir_rbcp_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(shedir_rbcp_menup, shedir_rbcp_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(shedir_rbcp_menup);
        //prcomplete(testpass, errcount, 0);
    }

    return(PASSED);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: shedir_rbcp_registration_test
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
int shedir_rbcp_registration_test (int menu_option)
{
    int rc;

    #ifdef ENHANCED_ERR_MSG_EXAMPLE
     uchar ngwic_get_pid[FRU_SIZE] = {0};
     uchar ngwic_get_loc[FRU_SIZE] = {0};
    #endif
    /* Enabled RBCP registeration if called from utils */
    if (menu_option==TRUE) {
        regis_done_flag[shedir_test_slot] = FALSE;
    }

    if (!regis_done_flag[shedir_test_slot]) {
        #ifdef ENHANCED_ERR_MSG_EXAMPLE
            /*
             *      * 1. Subtests of the test function will reuse all variables
             *           * 2. All variables will be cleared automatically when
             *                *    entering and leaving each menu item.
             *                     */
            /* Segment 1: PID | Unique_string : slot_info */
            fru_table_offset = MB;
            /* fru_table_offset should be set, otherwise, it will not */
            /* go to enhanced error message format in cterr() */
            /* set fru_table_offset to get the predefine value */
            /* or change mb_pid & mb_loc below */

         memcpy(ngwic_get_pid,(char*)&shedir_board_id,2); 
         strcpy((char *)ngwic_get_loc, "RBCP Registration Test");
         //uchar component="RBCP Registration Test";
         platform_fru_table[fru_table_offset].pid_string = ngwic_get_pid ; 
         platform_fru_table[fru_table_offset].location_string = ngwic_get_loc;
 
         /* Segment 2: Test step captured from prpass */
         /* Segment 3: Failure message captured from cterr */

         /* Segment 4: Components used */
         cterr_add_component("E1 -> Intel -> NC-SI ->BMC");

         /* Segment 5: register and memory dump */

         /* Segment 6: Platform Environment initialized here*/
         //cterr_add_env_dump((PFV)show_margins_cterr_wrapper);

         /* Segment 7: Top 3 Debugging Steps */
         cterr_add_debug("Make sure the E1 to Intel Path is okay",
                         "Make sure the Intel to BMC path is okay ");
       #endif

        /* RBCP Registration is not yet done */
        prpass(testpass, "RBCP Registration");

        rc = rbcp_register();

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
    regis_done_flag[shedir_test_slot] = TRUE;

    return (PASSED);
}
/**********************************************************************
 *
 * Function: shedir_rbcp_heartbeat_test
 *
 * Description: This function provides RBCP testing.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int shedir_rbcp_heartbeat_test (void)
{
    int rc,j;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    shedir_rbcp_registration_test (FALSE);

    prpass(testpass, "RBCP heartbeat"); 

    /* retries: see CDETS: CSCtx52567 */
    for (j=0; j < RBCP_RETRIES; j++) {
        rc = rbcp_ping();
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
                      " after %d tries\n", RBCP_RETRIES);
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
 * Function: shedir_rbcp_pwroff_intel
 *
 * Description: This function power off Intel.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int shedir_rbcp_pwroff_intel(void)
{
    int rc,jx;

    prpass(testpass, "RBCP Intel power off"); 

    /* call RBCP register to ensure we have registered RBCP into BMC */
    shedir_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < RBCP_RETRIES; jx++) {
    rc = rbcp_intel_power_off();
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
 * Function: shedir_rbcp_pwron_intel
 *
 * Description: This function power off Intel.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int shedir_rbcp_pwron_intel(void)
{
    int rc,jx;

    prpass(testpass, "RBCP Intel power on"); 

    /* call RBCP register to ensure we have registered RBCP into BMC */
    shedir_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < RBCP_RETRIES; jx++) {
    rc = rbcp_intel_power_on();
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
static int shedir_rbcp_tbd (void)
{

    printf("\n!!!  Test not yet implemented !!!\n");

    return (PASSED);
}
*/

/**********************************************************************
 *
 * Function: shedir_rbcp_bmc_console_switch
 *
 * Description: console switch to BMC
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

int shedir_rbcp_bmc_console_switch (void)
{
    int rc,jx;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    shedir_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < RBCP_RETRIES; jx++) {
        rc = rbcp_con_sw_bmc();
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
/**********************************************************************
 *
 * Function: shedir_rbcp_intel_console_switch
 *
 * Description: console switch to Intel
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

int shedir_rbcp_intel_console_switch (void)
{
    int rc,jx;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    shedir_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx=0; jx < RBCP_RETRIES; jx++) {
        rc = rbcp_con_sw_intel();
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

void clear_shedir_regis_done_flag (int slot)
{
    regis_done_flag[slot] = FALSE;
}
/*------------------------------------------------------------------
 * $Log: shedir_rbcp_main.c,v $
 * Revision 1.3  2015/05/25 03:58:21  steja
 * Fix merge conflict issue
 *
 * Revision 1.2.2.2  2015/05/22 15:42:31  steja
 * Sync skye-branch2 with Maintrunk
 *
 * Revision 1.2  2015/05/14 05:33:24  hondwang
 * Merge Shedir NIM to maintrunk
 *
 * Revision 1.1.2.2  2014/12/11 08:34:03  hondwang
 * makefile change
 *
 * Revision 1.1.2.1  2014/08/29 03:17:10  hondwang
 * shedir project
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

