/* $Id: platform_fan.c,v 1.5 2020/12/23 11:31:27 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/platform_fan.c,v $
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
#include "fpga_smartfan.h"

void show_fan_sts(void);
static int get_fan_bit(int);
static void fan_en_util(void);
static void fan_dis_util(void);
static void fan_pwm_ctrl(void);
static void show_fan_rps(void);
static void change_fan_speed(void);
void show_fan_info(void);
void display_smartfan_info(void);
static void set_fan_rpm_tolerance(void);

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
    {"Display smartfan info ",             (PFT)display_smartfan_info,       0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Fan speed test tolerance",           (PFT)set_fan_rpm_tolerance,       0,
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
    build_primary_submenu(fan_menu_table, FAN_MENU_TABLE_SIZE,
			  "FAN Control Unit Utility Menu", &fandiagp);
    build_secondary_submenu(fan_menu_table, FAN_MENU_TABLE_SIZE,
			    fan_menu_secondary_items);
    menu(&fandiag, fan_menu_secondary_items, 0);
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
        fan_num = FAN1_ENABLE;
    break;
    case 2:
        fan_num = FAN2_ENABLE;
    break;
    case 3:
        fan_num = FAN3_ENABLE;
    break;
    case 4:
        fan_num = FAN4_ENABLE;
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

    if (is_curie_2ru())
        fan_num = getdec_answer("Fan num 1-4:", 1, 1, 4);
    else
        fan_num = getdec_answer("Fan num 1-3:", 1, 1, 3);

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

    if (is_curie_2ru())
        fan_num = getdec_answer("Fan num 1-4:", 1, 1, 4);
    else
        fan_num = getdec_answer("Fan num 1-3:", 1, 1, 3);

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

    fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);

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

    fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);

    fan_spd = fan_speed_rd(fan_num);
    if (fan_spd > FAN_SPD_100PER_PWM) {
        printf("Get fan speed failed, fan speed is 0x%x, which is larger than 100 percent value 0x%x \n",
                fan_spd, FAN_SPD_100PER_PWM);
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
    int fan_num_end;

    status = get_fan_status();

    printf("FAN tray present: %s \n", (status & FAN_TRAY_PRESENT) ? "YES" : "NO");

    /* curie 1ru has 3 fans while 2ru has 4 fans */
    fan_num_end = is_curie_2ru() ? 5 : 4;
    for (fan_num = 1; fan_num < fan_num_end; fan_num++) {
       /* FAN1_ROTATION - 0x100;
        * FAN2_ROTATION - 0x200;
        * FAN3_ROTATION - 0x400;
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

/*********************************************************************
 *
 * Function:    display_smartfan_info
 *
 * Description: Display smart fan info which is reading from FPGA.
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 *********************************************************************
 */
void display_smartfan_info (void) {
    fpga_smartfan_display(0);
    fpga_smartfan_display(1);
    fpga_smartfan_display(2);
    fpga_smartfan_display(3);
    printf("\n");
}

static int fan_rpm_tolerance = 10;

/*********************************************************************
 *
 * Function: set_fan_rpm_tolerance
 *
 * Description: Change the RPM tolerance in fan speed test.
 *
 *********************************************************************
 */
static void set_fan_rpm_tolerance(void)
{
    fan_rpm_tolerance = getdec_answer("Fan RPM tolerance 1-100::", fan_rpm_tolerance, 1, 100);
}

/*********************************************************************
 *
 * Function:	idv_fan_spd_test
 *
 * Description: Individual fan speed test
 *
 * Inputs:	int fan_num
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static int idv_fan_spd_test (int fan_num) {

    uint save_pwm_slope;
    uint save_pwm_setting;
    uint techometer_30pct, techometer_50pct, techometer_70pct;
    uint rps_30pct, rps_50pct, rps_70pct;
    uint cal_rps_50pct, rps_50pct_low, rps_50pct_high;

    /* 1. Log the current PWM slope
     */
    save_pwm_slope = fan_pwm_slope_read();
    printf("Current pwm slope is 0x%x. Set it to max 0x3ff. \n", save_pwm_slope);

    /* 2. Log the current PWM setting (i.e. fan speed)
     */
    save_pwm_setting = fan_speed_rd(fan_num);
    if (save_pwm_setting > FAN_SPD_100PER_PWM) {
        printf("Get fan %d speed(PWM) failed, value is 0x%x, which is larger than 100 percent value 0x%x \n",
	       fan_num, save_pwm_setting, FAN_SPD_100PER_PWM);
        return(FAILED);
    }
    printf("Current fan %d speed(PWM) setting is 0x%x \n", fan_num, save_pwm_setting);

    /* 3. Write PWM slope = 0x3FF (max)
     */
    fan_pwm_slope_write(0x3ff);

    /* 4. Set fan speed PWM to 30%, 50%, 70%, and log the technometer
     */
    fan_speed_wr(fan_num, FAN_SPD_30PER_PWM);
    msleep(10*1000);
    techometer_30pct = tachometer_rps_read(fan_num);
    rps_30pct = techometer_30pct * 60;
    printf("Fan %d techometer at 30pct PWM is 0x%x => RPS %d, \n", fan_num, techometer_30pct, rps_30pct);

    fan_speed_wr(fan_num, FAN_SPD_50PER_PWM);
    msleep(10*1000);
    techometer_50pct = tachometer_rps_read(fan_num);
    rps_50pct = techometer_50pct * 60;
    printf("Fan %d techometer at 50pct PWM is 0x%x => RPS %d, \n", fan_num, techometer_50pct, rps_50pct);

    fan_speed_wr(fan_num, FAN_SPD_70PER_PWM);
    msleep(10*1000);
    techometer_70pct = tachometer_rps_read(fan_num);
    rps_70pct = techometer_70pct * 60;
    printf("Fan %d techometer at 70pct PWM is 0x%x => RPS %d, \n", fan_num, techometer_70pct, rps_70pct);

    /* 5. Restore PWM slope original value, PWM % original value
     */
    fan_pwm_slope_write(FAN_PWM_SLOPE_DEFAULT);
    fan_speed_wr(fan_num, save_pwm_setting);
    printf("Set pwm slope to default 0x%x and restore pwm setting to 0x%x\n", FAN_PWM_SLOPE_DEFAULT, save_pwm_setting);

    /* 6. Calculate the slope between the high-speed and low-speed reading.  Interpolate the mid-speed reading.
     * Check that the mid-speed reading is on the slope within X%.
     * (X is currently unknown and may have to be adjusted – could be an
     * input parameter for now. Initially default to 5%)
     */
    cal_rps_50pct = (rps_70pct - rps_30pct) / 2 + rps_30pct;
    rps_50pct_low = cal_rps_50pct * (100 - fan_rpm_tolerance) / 100;
    rps_50pct_high = cal_rps_50pct * (100 + fan_rpm_tolerance) / 100;
    printf("Calculated RPS at 50pct is %d. %dpct tolerance range: min (%d), high (%d) \n",
	   cal_rps_50pct, fan_rpm_tolerance, rps_50pct_low, rps_50pct_high);

    if ((rps_50pct < rps_50pct_low) || (rps_50pct > rps_50pct_high)) {
        cterr('f',0, "Fan %d speed test failed. Measured RPS at 50pct (%d) is outside tolerance.\n",
	      fan_num, rps_50pct);
	return(FAILED);
    }
    printf("Fan %d speed passed. Measured RPS at 50pct (%d) is within tolerance.\n",
	   fan_num, rps_50pct);

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	fan_speed_test
 *
 * Description: Platfrom fan speed test
 *
 * Inputs:	void
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
int fan_speed_test (void)
{
    char *tname = "Fan speed";
    int status, fn;
    int result = PASSED;

    testname("%s", tname);
    prpass(testpass, "");

    /* Check if fan tray is installed. BST station only test PCB assembly.
     * Print warning while let the test pass.
     */
    status = get_fan_status();
    if ((status & FAN_TRAY_PRESENT) == 0) {
        cterr('w',0, "Fan tray not present. Skip test. ");
        return(result);
    }

    /* Curie 2RU has 4 fans
     */
    for (fn=1; fn <= 4; fn++) {
        printf("\nFan %d speed test... \n", fn);

        if (idv_fan_spd_test(fn) == FAILED) {
	        result = FAILED;
	    }
    }

    return(result);
}

/*
 *-----------------------------------------------------------------------------
$Log: platform_fan.c,v $
Revision 1.5  2020/12/23 11:31:27  jiajliu
update default speed tolerance to 10% and add modification menu

Revision 1.4  2020/09/07 07:56:30  jiajliu
Merge 1RU Check if fan tray present in the fan speed test

Revision 1.3  2020/08/11 10:48:22  jiajliu
Leverage the fan speed test of Curie 1RU

Revision 1.2  2020/07/03 03:44:22  jiajliu
fixed fan number for change_fan_speed

Revision 1.1  2020/01/09 01:02:01  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
