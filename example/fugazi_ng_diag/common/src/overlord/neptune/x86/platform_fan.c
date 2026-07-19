/* $Id: platform_fan.c,v 1.2 2018/05/18 09:24:59 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/platform_fan.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_fan.c
 *
 * Description:  accessing dash FPGA registers to control 
 *               FAN related information. 
 *
 *
 * Copyright (c) 2016-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "byteswap.h"
#include "common.h"
#include "defs.h"
#include "proto.h"
#include "error.h"
#include "mon_plat_defs.h"
#include "cross_platform.h"
#include "menu.h"
#include "n2g_api_rc.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "dash_fpga.h"

void show_fan_sts(void);
static int get_fan_bit(int);
static void fan_en_util(void);
static void fan_dis_util(void);
static void fan_pwm_ctrl(void);
static void show_fan_rps(void);
static void change_fan_speed(void);
void show_fan_info(void);

static submenu_xtable_t fan_menu_table[] = {
    {"Show fan status ",                   (PFT)show_fan_sts,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Fan enable utility ",                (PFT)fan_en_util,                 0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Fan disable utility ",               (PFT)fan_dis_util,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Fan PWM control ",                   (PFT)fan_pwm_ctrl,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Show single fan RPS ",               (PFT)show_fan_rps,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Change fan speed ",                  (PFT)change_fan_speed,            0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
};

#define FAN_MENU_TABLE_SIZE (sizeof(fan_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fan_menu_primary_items[FAN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t fan_menu_secondary_items[FAN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo fandiag = {
    "FAN Control Unit Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    fan_menu_primary_items,
};

static struct menuinfo *fandiagp = &fandiag;

/**********************************************************************
 *
 * function:	build_fan_menu
 *
 * Description:	Build FAN Control Unit menu.
 *
 * Input:	None.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void
build_fan_menu (void)
{
    set_nios_mode(NIOS_DISABLE_MODE);
    
    build_primary_submenu(fan_menu_table, FAN_MENU_TABLE_SIZE,
			  "FAN Control Unit Utility Menu", &fandiagp);
    build_secondary_submenu(fan_menu_table, FAN_MENU_TABLE_SIZE,
			    fan_menu_secondary_items);
    menu(&fandiag, fan_menu_secondary_items, 0);

    set_nios_mode(NIOS_DIAG_MODE);
}

/*********************************************************************
 *
 * Function:    show_fan_sts
 *
 * Description: Display fan status which is reading from FPGA.
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 *********************************************************************
 */
void show_fan_sts (void) {

    int status; 

    /* leverage existing function. */
    show_fan_info();

    status = get_fan_status();
    printf("Aggergate rotation alert? %s \n", (status & AGGRE_ALERT) ? "YES" : "NO");
    printf("NEBS FAN tary/filter installed? %s \n", (status & NEBS_FAN_TRAY_PRESENT) ? "YES" : "NO");

    return;
}

/*********************************************************************
 *
 * Function:    get_fan_bit
 *
 * Description: according to the fan number that user select, 
 *              to get the fan bit on fan ctrl register of FPGA. 
 *
 * Inputs:      fan_num - fan number 
 * Outputs:     fan_num - fan number(bit) on FPGA
 *
 *********************************************************************
 */
static int get_fan_bit (int fan_num) {

    switch(fan_num) {
    case 1:
        fan_num = FAN1_OPTION;
    break;
    case 2:
        fan_num = FAN2_OPTION;
    break;
    case 3:
        fan_num = FAN3_OPTION;
    break;
    case 4:
        fan_num = FAN4_OPTION;
    break;
    default:
        printf("Unknown fan number %d \n", fan_num);
        return 0; /* will check it on calling function */
    break;
    }

    return (fan_num);
}

/*********************************************************************
 *
 * Function:    fan_en_util
 *
 * Description: enable fan utility
 *
 * Inputs:      None
 * Outputs:     None
 *
 *********************************************************************
 */
static void fan_en_util (void)
{
    int fan_num;

    if (!is_dagger()) {
        fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);
    } else {
        fan_num = 1; /* Dagger platform only has Fan 1  */
    }

    /* get specific fan bit on FPGA register */
    fan_num = get_fan_bit(fan_num);
    if (fan_num == 0) {
        printf("Cannot get fan bit\n");
        return;
    }

    if (getc_answer("Enable fan? (y/n)", "yn", 'y') == 'y') {
       enable_fan_ctrl(fan_num);
    }

    return;
}

/*********************************************************************
 *
 * Function:    fan_dis_util
 *
 * Description: disable fan utility
 *
 * Inputs:      None
 * Outputs:     None
 *
 *********************************************************************
 */
static void fan_dis_util (void)
{
    int fan_num;

    if (!is_dagger()) {
        fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);
    } else {
        fan_num = 1; /* Dagger platform only has Fan 1  */
    }

    /* get specific fan bit on FPGA register */
    fan_num = get_fan_bit(fan_num);
    if (fan_num == 0) {
        printf("Cannot get fan bit\n");
        return;
    }

    if (getc_answer("Disable fan? (y/n)", "yn", 'n') == 'y') {
       disable_fan_ctrl(fan_num);
    }

    return;
}

/*********************************************************************
 *
 * Function:	fan_pwm_ctrl
 *
 * Description:	Fan PWM Slope
 *              This register specifies how many steps the PWM can change 
 *              each second.  A value of 0 is not allowed and will be 
 *              automatically changed to a 1. 
 *              The slope only applies to normal operation; 
 *              in alert mode the PWM to the fan can change instantaneously.
 *              The default value of 0x14 corresponds to a 10% 
 *              change in PWM/second.
 *
 * Inputs:	None.
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static void fan_pwm_ctrl (void)
{
    uint pwm_slope;

    pwm_slope = fan_pwm_slope_read();

    printf("Current fan PWM slope is 0x%x \n", pwm_slope);
    if (getc_answer("Change the PWM slope?", "yn", 'n') == 'y') {
         pwm_slope = gethex_answer("Enter the data:", pwm_slope, 0x1, 0x7FF);
         fan_pwm_slope_write(pwm_slope);
    }

    return;
}

/*********************************************************************
 *
 * Function:    show_fan_rps
 *
 * Description: Display fan rps from tachometer
 *              Fan speed indicated as revolutions per second
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 *********************************************************************
 */
static void show_fan_rps (void) {

    uint tach_rpm, fan_num;

    if (!is_dagger()) {
        fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);
    } else {
        fan_num = 1; /* Dagger platform only has one fan */
    }

    tach_rpm = tachometer_rps_read(fan_num);
    printf("Fan%d rps is %d \n", fan_num, tach_rpm);

    return;
}


/*********************************************************************
 *
 * Function:	change_fan_speed
 *
 * Description:	Fan speed (PWM) control utility. 
 *
 * Inputs:	None.
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static void change_fan_speed (void) {

    uint fan_num, fan_spd; 

    if (!is_dagger()) {
        fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);
    } else {
        fan_num = 1; /* Dagger platform only has one fan */
    }

    fan_spd = fan_speed_rd(fan_num);
    if (fan_spd > FAN_SPD_100PER_PWM) {
        printf("Get fan speed failed, fan speed is %d, which is larger than 100%% \n",
             FAN_SPD_100PER_PWM);
        return;
    }

    printf("Current fan%d speed(PWM) is 0x%x \n", fan_num, fan_spd);
    if (getc_answer("Change the speed?", "yn", 'n') == 'y') {
         printf("min - 0%%, max - 100%%(0x%x), default - 35%% (0x%x) \n",
                FAN_SPD_100PER_PWM, FAN_SPD_35PER_PWM);
         fan_spd = gethex_answer("Enter the data:", fan_spd, FAN_SPD_0PER_PWM, FAN_SPD_100PER_PWM);
    }

    fan_speed_wr(fan_num, fan_spd);

    msleep(10);
    fan_spd = fan_speed_rd(fan_num);
    printf("Update fan%d speed(PWM) is 0x%x \n", fan_num, fan_spd);

    return;
}

/*********************************************************************
 *
 * Function:    show_fan_info
 *
 * Description: show fan detail info - enable  and RPM
 *
 * Inputs:      None.
 *
 * Outputs:     PASSE/FAILED.
 *
 *********************************************************************
 */
void show_fan_info (void) {

    int status, result = 0;
    uint tach_rpm, tach_rps, fan_num, fan_mask = 1;

    status = get_fan_status();

    printf("FAN tray present: %s \n", (status & FAN_TRAY_PRESENT) ? "YES" : "NO");

    for (fan_num = 1; fan_num < 5; fan_num++) {
       if ((is_vg450() || is_neptune()) && fan_num > 3) {
            /* neptune has 3 fans, Triton and Proteus have 4 fans */
            continue; 
       }

       /* FAN1_ROTATION - 0x100;
        * FAN2_ROTATION - 0x200;
        * FAN3_ROTATION - 0x400;
        * FAN4_ROTATION - 0x800;
        */
       result = status & fan_mask << (fan_num + 7); /* 7 = ia start from 1 */
       tach_rps = tachometer_rps_read(fan_num);
       
       /* tach_rps == 0xFFFF means unknown fan speed */
       if (tach_rps == 0xFFFF) {
           printf("FAN %d is %s. Running with unknown speed\n",
                   fan_num, result ? "enable" : "DISABLE");
       } else { 
           tach_rpm = tach_rps * 60;
           printf("FAN %d is %s. Running at %dRPM\n",
                    fan_num, result ? "enable" : "DISABLE", tach_rpm);
       }
    }

}
/*------------------------------------------------------------------
$Log: platform_fan.c,v $
Revision 1.2  2018/05/18 09:24:59  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.4  2018/04/11 08:27:10  leschen
Add new argument to display plug-in itmes and env status.

Revision 1.1.2.3  2017/11/22 07:35:10  leschen
Supporting Barsoom VG450 which FGPA sub board type is 0x43.

Revision 1.1.2.2  2017/07/05 06:31:15  alpeng
update fan info, update PSU and remove pem files

Revision 1.1.2.1  2016/06/06 09:38:04  leschen
Support Neptune fan utility.

#
*/
