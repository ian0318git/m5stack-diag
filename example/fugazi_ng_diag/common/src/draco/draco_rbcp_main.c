/* $Id: draco_rbcp_main.c,v 1.2 2016/01/21 01:50:02 olin2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/draco/draco_rbcp_main.c,v $
 *------------------------------------------------------------------
 * Filename: draco_rbcp_main.c
 *
 * Description: The RBCP main source code
 * Author: Times Huang
 *
 * Copyright (c) 2016 by cisco Systems, Inc.
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
#include "router_if.h"
#include "ngsm_draco.h"
#include "draco_rbcp_main.h"
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

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/

long build_draco_rbcp_menu(int);

int draco_rbcp_pwroff_intel(void);
int draco_rbcp_pwron_intel(void);
int draco_rbcp_bmc_console_switch(void);
int draco_rbcp_intel_console_switch(void);
int draco_rbcp_heartbeat_test(int);    /* also called by interface test */
int draco_rbcp_registration_test(int);  /* also called by interface test */
void clear_draco_regis_done_flag(int);


/***********************************************************************
 *  Externs
 ************************************************************************/
extern int do_all_menu_items(struct menuinfo *);

extern int draco_test_slot;
extern ushort draco_board_id;
extern void show_margins_cterr_wrapper(void);
/***********************************************************************
 *  Global Variable
 ************************************************************************/
static int regis_done_flag[MAX_NUM_DRACO_SLOTS]={FALSE};
#define ENHANCED_ERR_MSG_EXAMPLE 1

static submenu_xtable_t draco_rbcp_submenu_tbl[] = {
    { "RBCP Registration Test", (type_t(*)())draco_rbcp_registration_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP heart beat Test",   (type_t(*)())draco_rbcp_heartbeat_test, DRACO_BP_GE0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to Intel", (type_t(*)())draco_rbcp_intel_con_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP console switch to BMC", (type_t(*)())draco_rbcp_bmc_con_switch, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP power off Intel", (type_t(*)())draco_rbcp_pwroff_intel, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP power on Intel", (type_t(*)())draco_rbcp_pwron_intel, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "RBCP Registration Util", (type_t(*)())draco_rbcp_registration_test, TRUE,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define DRACO_RBCP_SUBMENU_TABLE_SZ \
                (sizeof(draco_rbcp_submenu_tbl)/sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t draco_rbcp_primary_items[DRACO_RBCP_SUBMENU_TABLE_SZ +
                                        MAX_BASE_ITEMS];
static mitem_t draco_rbcp_secondary_items[DRACO_RBCP_SUBMENU_TABLE_SZ +
                                          MAX_BASE_ITEMS];

static menuinfo_t draco_rbcp_main_menu = {
    "Draco RBCP Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    draco_rbcp_primary_items,
};
static menuinfo_t *draco_rbcp_menup = &draco_rbcp_main_menu;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_draco_rbcp_menu
 *
 * Description: Build Draco RBCP tests and utilities menu.
 *
 * Inputs:      show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_draco_rbcp_menu (int show_menu)
{

    build_primary_submenu(draco_rbcp_submenu_tbl, DRACO_RBCP_SUBMENU_TABLE_SZ,
                          "RBCP Main Menu", &draco_rbcp_menup);
    build_secondary_submenu(draco_rbcp_submenu_tbl, DRACO_RBCP_SUBMENU_TABLE_SZ,
                            draco_rbcp_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(draco_rbcp_menup, draco_rbcp_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(draco_rbcp_menup);
        /* prcomplete(testpass, errcount, 0); */
    }

    return (PASSED);
}


/***********************************************************************
 *  Static Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: draco_rbcp_registration_test
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
int draco_rbcp_registration_test (int menu_option)
{
    int rc;

    #ifdef ENHANCED_ERR_MSG_EXAMPLE
     uchar ngwic_get_pid[FRU_SIZE] = {0};
     uchar ngwic_get_loc[FRU_SIZE] = {0};
    #endif
    /* Enabled RBCP registeration if called from utils */
    if (menu_option == TRUE) {
        regis_done_flag[draco_test_slot] = FALSE;
    }

    if (!regis_done_flag[draco_test_slot]) {
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

         memcpy(ngwic_get_pid,(char*)&draco_board_id,2); 
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
            return (FAILED);
        } else {
            prcomplete(testpass, errcount, 0);
        }
    } else {
        /* RBCP Registration was already done earlier*/
        printf("\nRegistration was already done. So nothing executed!\n");
    }
 
    /* Once RBCP registration is done, set this flag */
    regis_done_flag[draco_test_slot] = TRUE;

    return (PASSED);
}
/**********************************************************************
 *
 * Function: draco_rbcp_heartbeat_test
 *
 * Description: This function provides RBCP testing.
 *
 * Input :  bp_port - Backplane GE Port number
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int draco_rbcp_heartbeat_test (int bp_port)
{
    int rc,j;

    /* Disable BP loopback for tested port */
    set_draco_bp_loopback(draco_test_slot, bp_port, FALSE);

    /* Enable BP loopback for the other port */
    set_draco_bp_loopback(draco_test_slot, !bp_port, TRUE);

    /* call RBCP register to ensure we have registered RBCP into BMC */
    draco_rbcp_registration_test(FALSE);

    prpass(testpass, "RBCP heartbeat"); 

    /* retries: see CDETS: CSCtx52567 */
    for (j = 0; j < RBCP_RETRIES; j++) {
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
        return (FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: draco_rbcp_pwroff_intel
 *
 * Description: This function power off Intel.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int draco_rbcp_pwroff_intel(void)
{
    int rc,jx;

    prpass(testpass, "RBCP Intel power off"); 

    /* call RBCP register to ensure we have registered RBCP into BMC */
    draco_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx = 0; jx < RBCP_RETRIES; jx++) {
        rc = rbcp_intel_power_off();
            if (!rc) {
                break;
            }
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
        return (FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: draco_rbcp_pwron_intel
 *
 * Description: This function power off Intel.
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
int draco_rbcp_pwron_intel(void)
{
    int rc,jx;

    prpass(testpass, "RBCP Intel power on"); 

    /* call RBCP register to ensure we have registered RBCP into BMC */
    draco_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx = 0; jx < RBCP_RETRIES; jx++) {
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
        return (FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }
    
    return (PASSED);
}

/**********************************************************************
 *
 * Function: draco_rbcp_bmc_console_switch
 *
 * Description: console switch to BMC
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

int draco_rbcp_bmc_console_switch (void)
{
    int rc,jx;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    draco_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx = 0; jx < RBCP_RETRIES; jx++) {
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
       return (FAILED);
    } else {
        prcomplete(testpass, errcount, 0);
    }

    return (PASSED);
}
/**********************************************************************
 *
 * Function: draco_rbcp_intel_console_switch
 *
 * Description: console switch to Intel
 *
 * Input :  None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */

int draco_rbcp_intel_console_switch (void)
{
    int rc,jx;

    /* call RBCP register to ensure we have registered RBCP into BMC */
    draco_rbcp_registration_test (FALSE);

    /* retries: see CDETS: CSCtx52567 */
    for (jx = 0; jx < RBCP_RETRIES; jx++) {
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
        return (FAILED);
    } else {
         prcomplete(testpass, errcount, 0);
    }

    return (PASSED);
}

void clear_draco_regis_done_flag (int slot)
{
    regis_done_flag[slot] = FALSE;
}

/*------------------------------------------------------------------
 * $Log: draco_rbcp_main.c,v $
 * Revision 1.2  2016/01/21 01:50:02  olin2
 * Collapse Draco-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2015/07/27 02:05:51  olin2
 * Initial commit code for Draco
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------
 */

