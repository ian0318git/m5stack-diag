/* $Id: platform_pem_fan.c,v 1.2 2019/08/06 06:56:13 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_pem_fan.c,v $
 *------------------------------------------------------------------
 *
 * platform_pem_fan.c: Fan Control Utilities
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"

#include "menu.h"
#include "types.h"
#include "queryflags.h"

#include "platform_psu.h"
#include "platform_idprom.h"
#include "platform_idprom_utils.h"
#include "platform_pem_fan.h"
#include "platform_pem_utils.h"

/* Global variable & define */
static int psu_no_now;

/* function prototype */
static int fan_spd_get(int);
static int fan_spd_get_wr(void);
static int fan_spd_set(void);
static int manual_fan_spd_test(void);
static int auto_fan_test(void);

/*
 * PSU Fan Utility Menu.
 */
static submenu_xtable_t pem_fan_menu_table[] = {
    {"Auto Fan test",                  (PFT)auto_fan_test,              0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Manual fan test",                (PFT)manual_fan_spd_test,        0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Fan speed set",                  (PFT)fan_spd_set,                0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
    {"Fan speed get",                  (PFT)fan_spd_get_wr,             0,
        0,                             (PFT)0,                          0,
      (PFT)0,                          0},
};

#define PEM_FAN_MENU_TABLE_SIZE \
        (sizeof(pem_fan_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t pem_fan_menu_primary_items[PEM_FAN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t pem_fan_menu_secondary_items[PEM_FAN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo pemfandiag = {
  "PEM Fan Utility Menu",           /* title */
  0,                                /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,            /* shows major flags */
  0,                                /* generic prompt */
  0,                                /* size -- bumped by add_menu_item() */
  pem_fan_menu_primary_items,
};
static struct menuinfo *pemfandiagp = &pemfandiag;


/**********************************************************************
 *
 * Function: build_pem_fan_menu
 *
 * Description: Build psu fan menu.
 *
 * Inputs: psu_no - current psu number
 *
 * Outputs: None.
 *
 **********************************************************************
 */
void
build_pem_fan_menu (int psu_no)
{

    psu_no_now = psu_no;
printf("psu_no now %d \n", psu_no_now);
    build_primary_submenu(pem_fan_menu_table, PEM_FAN_MENU_TABLE_SIZE,
                          "PEM Fan Utility Menu", &pemfandiagp);
    build_secondary_submenu(pem_fan_menu_table, PEM_FAN_MENU_TABLE_SIZE,
                            pem_fan_menu_secondary_items);
    menu(&pemfandiag, pem_fan_menu_secondary_items, 0);
}

/*******************************************************************************
 *
 * Function   : auto_fan_test
 * Description: Antomatically fan speed testing on both PSUs. (MAX & min)
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int auto_fan_test (void) {

    uchar speed, bak_spd, cur_spd;

    /* save the current fan speed */
    printf("Backup current speed \n");
    if (rp1ruve_pem_fan_get(psu_no_now, &bak_spd)) {
        printf("Cannot get speed from PEM%d \n", psu_no_now);
    } else {
        printf("Current speed on PEM%d is %d%% \n", psu_no_now, bak_spd);
    }

    /* -------- Set to min ---------- */
    printf("Set speed to min 60%% \n");
    speed = RP1RUVE_MIN_FAN_SPEED;
    if (rp1ruve_pem_fan_set(psu_no_now, speed)) {
        printf("Set to min speed %d%% on PEM%d failed\n", speed, psu_no_now);
    }

    msleep(BANK_TEST_DELAY);

    /* is speed equal to min ? */
    if (rp1ruve_pem_fan_get(psu_no_now, &cur_spd)) {
        printf("Cannot get speed from PEM%d \n", psu_no_now);
    } else {
        printf("Current speed on PEM%d is %d%% \n", psu_no_now, cur_spd);
    }

    if( cur_spd != speed) {
        printf("PEM0 setiing min FAN spd failed. current spd=%d%% expect spd=%d%% \n "
        , cur_spd, speed);
    } 

    /* -------- Set to MAX ---------- */
    printf("Set speed to MAX 100%% \n");
    speed = RP1RUVE_MAX_FAN_SPEED;
    if (rp1ruve_pem_fan_set(psu_no_now, speed)) {
        printf("Set to MAX speed %d%% on PEM%d failed\n", speed, psu_no_now);
    }
  
    msleep(BANK_TEST_DELAY);

    /* is speed equal to max ? */
    if (rp1ruve_pem_fan_get(psu_no_now, &cur_spd)) {
        printf("Cannot get speed from PEM%d \n", psu_no_now);
    } else {
        printf("Current speed on PEM%d is %d%% \n", psu_no_now, cur_spd);
    }

    if( cur_spd != speed) {
        printf("PEM0 setiing MAX FAN spd failed. current spd=%d%% expect spd=%d%% \n "
        , cur_spd, speed);
    }


    /* -------- restore fan speed  ---------- */
    printf("Restore speed  ....");
    speed = bak_spd;
    if (rp1ruve_pem_fan_set(psu_no_now, speed)) {
        printf("Restore speed %d%% on PEM%d failed\n", speed, psu_no_now);
    } else {
        printf("done!\n");
    }
   
    return PASSED;
}


/*******************************************************************************
 *
 * Function   : manual_fan_spd_test
 * Description: enter fan speed manually for testing fan speed on both PSUs. 
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int manual_fan_spd_test (void) {

    uint rv = PASSED;
    uchar speed_new, bak_spd, cur_spd;
    boolean reset_fan = FALSE;
  
    printf("We provide duty cycle 20-100%% \n");
    speed_new = getdec_answer("FAN Speed 20-100: ", 60, 20, 100);

    if (getc_answer("Reset to original speed after test? ", "yn", 'y') == 'y') {
        reset_fan = TRUE;
    }

    /* save the current fan speed */
    printf("Backup current speed \n");
    if (rp1ruve_pem_fan_get(psu_no_now, &bak_spd)) {
        printf("Cannot get speed from PEM%d \n", psu_no_now);
    } else {
        printf("Current speed on PEM%d is %d%% \n", psu_no_now, bak_spd);
    }

    /* Set to user input */
    printf("Setting speed \n");
    if (rp1ruve_pem_fan_set(psu_no_now, speed_new)) {
        printf("Set to speed %d%% on PEM%d failed\n", speed_new, psu_no_now);
    }

    msleep(BANK_TEST_DELAY);
    
    /* Check the speed and temp */
    if (rp1ruve_pem_fan_get(psu_no_now, &cur_spd)) {
        printf("Cannot get speed from PEM%d \n", psu_no_now);
    } else {
        printf("Current speed on PEM%d is %d%% \n", psu_no_now, cur_spd);
    }

    if (cur_spd != speed_new) {
        printf("PEM%d current spd is %d%% setting speed is %d%%\n",
         psu_no_now, cur_spd, speed_new);
    } else {
        printf("Speed check ... OK\n");
    }
   
    /* re-set the original fan speed */
    if ( reset_fan ) {
        printf("Restore speed ...");
        if (rp1ruve_pem_fan_set(psu_no_now, bak_spd)) {
            printf("Set to speed %d%% on PEM%d failed\n", bak_spd, psu_no_now);
        } else {
            printf("done !!\n");
        }
    }

    return rv;
}

/*******************************************************************************
 *
 * Function   : fan_spd_set
 * Description: set fan speed for PSU manually.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int fan_spd_set (void) {

    uchar speed_new;

    printf("We provide duty cycle from 20-100%%\n");

    speed_new = getdec_answer("Select speed level:", 60, 20, 100);

    if (rp1ruve_pem_fan_set(psu_no_now, speed_new)) {
        printf("Set to speed %d%% on PEM%d failed\n", speed_new, psu_no_now);
    } 

    return PASSED;
}

/*******************************************************************************
 *
 * Function   : fan_spd_get_wr
 * Description: Wrapper of get fan speed for PSU.
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int fan_spd_get_wr (void) {

    return (fan_spd_get(psu_no_now));
}

/*******************************************************************************
 *
 * Function   : fan_spd_get
 * Description: Get fan speed for PSU.
 * Inputs     : psu_num - PSU number.
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int fan_spd_get (int psu_num) {

    uchar bak_spd;

    /* Get current fan speed */
    if (rp1ruve_pem_fan_get(psu_num, &bak_spd)) {
        printf("Cannot get speed from PEM%d \n", psu_num);
    } else {
        printf("PSU%d fan speed is running at %d%% \n", psu_num, bak_spd);
    }

    return PASSED;   
}

/*******************************************************************************
 *
 * Function   : display_pem_fan_spd
 * Description: show fan speed for the 2 PEM on Juno
 * Inputs     : NONE
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
void display_pem_fan_spd (void) {

    if (check_psu_present(PSU_ONE) == TRUE) {
        fan_spd_get(PSU_ONE);
    }
    if (check_psu_present(PSU_TWO) == TRUE) {
        fan_spd_get(PSU_TWO);
    }

    return;
}


/*
 *------------------------------------------------------------------
 * $Log: platform_pem_fan.c,v $
 * Revision 1.2  2019/08/06 06:56:13  alpeng
 * merge curie, switzer and nightwatch to trunk
 *
 * Revision 1.1.2.1  2018/06/22 08:05:19  alpeng
 * move curie diag to neptune/curie_1RU directory
 *
 * Revision 1.1.2.1  2018/05/30 02:39:37  alpeng
 * porting neptune x86 to curie
 *
 * Revision 1.2  2018/05/18 09:25:00  alpeng
 *  Neptune merge to trunk with tag <neptune-branch-0518>
 *
 * Revision 1.1.2.1  2016/06/02 22:04:02  jskow
 * Move Overlord/x86 specific files to Neptune/x86.
 *
 * Revision 1.4  2013/12/18 06:32:59  hroni
 * use toolchain in router/bin to do make with TOOLS_VER=c4.5.3-p1, TOOLS_ARCH=x86_64
 *
 * Revision 1.3  2013/09/26 01:37:44  alpeng
 * extend the fan duty cycle range to 20-100%
 *
 * Revision 1.2  2013/09/11 02:25:08  alpeng
 * 1. support Juno fan info and display on initialize stage.
 * 2. support fedora rootfs
 *
 * Revision 1.1  2013/05/31 12:43:15  danchung
 * Porting PSU source code from Nightster for Juno.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
