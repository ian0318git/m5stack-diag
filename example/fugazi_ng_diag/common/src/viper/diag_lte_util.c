/* $Id: diag_lte_util.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_lte_util.c,v $
 *-----------------------------------------------------------------------------
 * diag_lte_util.c - LTE relative util function
 *
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "queryflags.h"
#include "common.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "plat_defs.h"
#include "diag_fpga.h"
#include "platform_fru.h"
#include "diag_lte_test.h"
#include "diag_lte_lib.h"
#include "diag_lte_util.h"


/*******************************************************************************
 *                             Functions Declaration                           *
 *******************************************************************************
 */
static int lte_wp76xx_simdetect_pin_test(int);
static int display_wp76xx_simdetect_stat_util(int);


/*******************************************************************************
 *                               Global Variable                               *
 *******************************************************************************
 */


/*
 * LTE Utilities Submenu
 */
static submenu_xtable_t lte_utils_table[] = {
    {"LTE WP76xx SIM_DETECT pin Test",
     (PFT)lte_wp76xx_simdetect_pin_test,              0,
     0,
     (type_t(*) ())0,                                 0,
     (type_t(*) ())0,                                 0},
    {"Display LTE WP76xx SIM_DETECT pin state",
     (type_t(*)())display_wp76xx_simdetect_stat_util, 0,
     0,
     (type_t(*)())0,                                  0,
     (type_t(*)())0,                                  0},
};

#define LTE_UTILS_TABLE_SIZE (sizeof(lte_utils_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t lte_utils_primary_items[LTE_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t lte_utils_secondary_items[LTE_UTILS_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo lte_utils_diag = {
    "LTE Utilities Submenu",    /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    lte_utils_primary_items,
};

static struct menuinfo *lte_utils_diagp = &lte_utils_diag;


/*******************************************************************************
 *                                    Functions                                *
 *******************************************************************************
 */


/*******************************************************************************
 *
 * Function   : build_lte_utils_menu
 * Description: To build LTE utilities submenu
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void build_lte_utils_menu (void) {

    build_primary_submenu(lte_utils_table, LTE_UTILS_TABLE_SIZE,
                          "LTE Utilities Submenu", 
                          &lte_utils_diagp);
    build_secondary_submenu(lte_utils_table, LTE_UTILS_TABLE_SIZE,
                            lte_utils_secondary_items);
    menu(&lte_utils_diag, lte_utils_secondary_items, 0);
}

/*******************************************************************************
 *
 * Function   : display_wp76xx_simdetect_stat_util
 * Description: Wrapped utility to display LTE WP76xx SIM_DETECT pin state.
 *              This function gets WP76xx LTE SIM_DETECT pin state
 *              by use AT!BSGPIO AT command, then display the read back state.
 * Inputs     : Dummy
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int display_wp76xx_simdetect_stat_util (int dummy)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;

    printf("SIM Detection PIN Status\n");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    lte_obj_p->callin_fvt->display_sim_detect_stat((dev_object_t *)&lte_obj);

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);


    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : wp76xx_sim_detect_pin_test
 * Description: Wrapped function to test LTE wp76xx SIM_DETECT pin.
 *              This function is to test wp76xx LTE SIM_DETECT pin
 *              by check if the state that AT!BSGPIO read back is as expected.
 *              Besides, this function also provides usr_prompt parameter for
 *              user prompt display enable/disable.
 * Inputs     : exp_sim_stat - Expected SIM status: PRESENT(1)/NOT_PRESENT(0)
 *
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int wp76xx_sim_detect_pin_test (boolean exp_sim_stat)
{
    char usr_input = 0;
    char usr_act_str[LTE_TESTMSG_BUFSZ];
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;

    testname("SIM Detect PIN");

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    memset(usr_act_str, 0, sizeof(usr_act_str));


    /* Configure parameters based on testing SIM number */
    if (exp_sim_stat == SIM_PRESENT) {
        sprintf(usr_act_str, "install SIM card");
    } else {
        sprintf(usr_act_str, "remove SIM card");
    }   

    printf("\n\n### Please %s .\n", usr_act_str);
    do {
        printf("\r### Press 'y' to continue the Test: ");
        usr_input = getchar();
        if (usr_input == 'y') {
            break;
        }   
    } while (usr_input != 'y');
    
    if (exp_sim_stat == SIM_PRESENT) {
    
        if (diag_lte_sim_detected_by_fpga() != TRUE) {
            printf("*** Warning!!! SIM is not detected by FPGA."
                   "Please insert SIM card and test again.\n");
            return (FAILED);
        }

        rc = lte_obj_p->callin_fvt->sim_detect_pin_present((dev_object_t *)&lte_obj, 
                                                            SIM_DET_PIN_PRE);
 
        if (rc != PASSED) {
            printf("*** SIM Detection PIN fails\n");
        }
    } else {
    
        if (diag_lte_sim_detected_by_fpga() != FALSE) {
            printf("*** Warning!!! SIM is detected by FPGA. "
                   "Please remove SIM card and test again.\n");
            return (FAILED);
        }

        rc = lte_obj_p->callin_fvt->sim_detect_pin_present((dev_object_t *)&lte_obj,
                                                            SIM_DET_PIN_NO_PRE);
 
        if (rc != PASSED) {
            printf("*** SIM Detection PIN fails\n");
        }
    }   


    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);


    return (rc);
}


/*******************************************************************************
 *
 * Function   : lte_wp76xx_simdetect_pin_test
 * Description: Wrapped function to test LTE WP76xx SIM_DETECT pin.
 * Inputs     : Dummy
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int lte_wp76xx_simdetect_pin_test (int dummy)
{
    /* CSCvk17781 - LTE SIM SLot 0 Detect pin issue
     *
     * This utility test is to enhance the coverage of WP76xx LTE SIM_DETECT pin.
     * It requires users to insert and remove SIM card during the test.
     *
     * Based on comment from SWI(Sierra wireless) on wp76xx:
     *   - AT!ENTERCND="A710" is required to enable the AT!BSGPIO command.
     *   - AT!BSGPIO?34 can be used to check the state of SIM_DETECT signal.
     */

    char test_name[LTE_TESTMSG_BUFSZ];

    memset(test_name, 0, sizeof(test_name));

    sprintf(test_name, "WP76xx LTE SIM_DETECT pin");

    testname(test_name);

    /* Test SIM_DETECT pin when SIM is present */
    if (wp76xx_sim_detect_pin_test(SIM_PRESENT) != PASSED) {
        cterr('f', 0, "SIM is inserted "
               "but SIM_DETECT state is Low.");
        return (FAILED);
    }

    /* Test SIM_DETECT pin when SIM is NOT present */
    if (wp76xx_sim_detect_pin_test(SIM_NOT_PRESENT) != PASSED) {
        cterr('f', 0, "SIM is NOT inserted "
               "but SIM_DETECT state is High.");
        return (FAILED);
    }

    return (PASSED);
}




/* end of file */


/******** History ********
*---------------------------------------------------
$Log: diag_lte_util.c,v $
Revision 1.2  2018/08/06 02:31:51  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.1.2.1  2018/07/09 08:27:33  olin2
CSCvk17781: Support util to verify SIM Detect pin




$Endlog$
*/
