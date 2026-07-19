/* $Id: diag_led_test.c,v 1.2 2019/01/10 06:36:23 wilbhuan Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/diag_led_test.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_test.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
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
#include "platform_cookie.h"
#include "diag_enhance_err_msg_lib.h"
#include "diag_moka_fpga_lib.h"
#include "diag_temp_sensor_util.h"
#include "diag_led_util.h"
#include "diag_led_test.h"

/*******************************************************************************
 *                                   Menus                                     
 *******************************************************************************
 */

/* LED Test Menu */
static submenu_xtable_t led_diag_tbl[] = {
    {"LED utilities",   (PFT)diag_led_util, 0,
     0, 
     (type_t(*)())0,    0,
     (type_t(*)())0,    0},
    {"LED test",        (PFT)diag_all_led_test,  0,
     (MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT),
     (type_t(*)())0,    0,
     (type_t(*)())0,    0},
};

#define LED_DIAG_TBL_SIZE (sizeof(led_diag_tbl) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_diag_menu_pri_items[LED_DIAG_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t led_diag_menu_sec_items[LED_DIAG_TBL_SIZE + MAX_BASE_ITEMS];

static menuinfo_t led_diag_menu = {
    "%s Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    led_diag_menu_pri_items,
};
static menuinfo_t *led_diag_menu_p = &led_diag_menu;


/*******************************************************************************
 *
 * Function   : diag_led_test
 * Description: Entry function of LED Diag test.
 * Inputs     : exe_all_testmenu - To decide whether to show test menu(TRUE/FALSE)
 *                              or do all related tests directly
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_led_test (boolean exe_all_testmenu)
{
    build_primary_submenu(led_diag_tbl, LED_DIAG_TBL_SIZE,
                          "LED", &led_diag_menu_p);
    build_secondary_submenu(led_diag_tbl, LED_DIAG_TBL_SIZE,
                            led_diag_menu_sec_items);

    if (exe_all_testmenu == TRUE) {
        do_all_menu_items(led_diag_menu_p);
    } else {
        menu(&led_diag_menu, led_diag_menu_sec_items, 0);
    }
}

/*******************************************************************************
 *
 * Function    : diag_all_led_test
 * Description : Function to test LEDs by turn ON/OFF them.
 *               This test requires user to judge PASSED or FAILED.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_led_test (void)
{
    uint reg_offset = 0, reg_val = 0;

    uchar mb_get_loc[FRU_SIZE] = {0};
    uchar mb_get_pid[FRU_SIZE] = {0};
#ifdef ENHANCE_ERROR_MSG_RDY
    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = MB;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */
    platform_get_pid((char *)mb_get_pid);
    strcpy((char *)mb_get_loc, "MB");
    platform_fru_table[fru_table_offset].pid_string = mb_get_pid;
    platform_fru_table[fru_table_offset].location_string = mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("Marvell Armada 7040", "Local Bus", "System FPGA");

    /* Segment 5: register and memory dump */
    //cterr_add_reg_dump((PFV)bcm63168_get_xdsl_profile);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_plat_curr_temps);

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug("If any failure occurs in this test, "
                    "try the Debugging Steps to narrow down the issue.",
                    "Check the LED GPIO to see if its' "
                    "implementation is identical to FPGA specification.",
                    "If step a is OK, check the GPIO interface "
                    "between MB and FPGA.");
#endif

    char *curr_testname = "LEDs function";
    int rc = PASSED;

    testname(curr_testname);
    prpass(testpass, "%s, ", curr_testname);

    /* Turn all LEDs OFF*/
    printf("Turn all LEDs Off\n");
    diag_led_all_off_util(); 
    sleep(LED_TEST_PERIOD);   

    /* Turn LED Green ON */
    printf("Need Visual testing!!\n");
    printf("Turn LED Green ON\n");
    diag_led_all_green_on_util(); 
    sleep(LED_TEST_PERIOD);

    /* Turn LED Green OFF*/
    printf("Turn LED Green OFF\n");
    diag_led_all_off_util(); 
    sleep(LED_TEST_PERIOD);  

    /* Turn LED Yellow ON */
    printf("Turn LED Yellow ON\n");
    diag_led_all_yellow_on_util(); 
    sleep(LED_TEST_PERIOD);   

    /* Turn LED Yellow OFF*/
    printf("Turn LED Yellow OFF\n");
    diag_led_all_off_util(); 
    sleep(LED_TEST_PERIOD);  

    /* Turn Power OK LED */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = (uint)(PWR_OK_LED | STAT_LED_G);
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __FUNCTION__, reg_offset);
    }

    return (rc);
}

/*-------------------------------------------------
 * $Log: diag_led_test.c,v $
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
