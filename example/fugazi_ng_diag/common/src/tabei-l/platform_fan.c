/* $Id: platform_fan.c,v 1.5 2020/09/21 06:11:19 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/platform_fan.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_fan.c
 *
 * Description:  accessing dash FPGA registers to control 
 *               FAN related information. 
 *
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
#include "diag_fpga.h"
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
void display_smartfan_info_workaround(void);

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
 * Function:    get_fan_status
 *
 * Description: read fan status register to see if fan is rotating.
 *
 * Inputs:      command - including fan number, aggregate rotation alert
 *                        and NEBS fan tray/filter installed
 *
 * Output:      TRUE/FALSE
 *
 *********************************************************************
 */
int get_fan_status (void) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint32_t status = env_fan->status;

    assert(dash_fpga);

    return (status);

}

/*********************************************************************
 *
 * Function:    enable_fan_ctrl
 *
 * Description: enable fan rotating
 *
 * Inputs:      fan_no - fan number 
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void enable_fan_ctrl (uint fan_no) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    assert(dash_fpga);

    env_fan->ctrl |=  fan_no;


    return;
}

/*********************************************************************
 *
 * Function:    disable_fan_ctrl
 *
 * Description: disable fan rotating.
 *
 * Inputs:      fan_no - fan number
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void disable_fan_ctrl (uint fan_no) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    assert(dash_fpga);

    env_fan->ctrl &=  ~fan_no;

    return;
}

/*********************************************************************
 *
 * Function:    fan_pwm_slope_read
 *
 * Description: read fan PWM slope register
 *
 * Inputs:      NONE
 *
 * Output:      NONE
 *
 *********************************************************************
 */
uint fan_pwm_slope_read (void) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint pwm_slope = env_fan->pwm_slope;

    assert(dash_fpga);

    return (pwm_slope);
}

/*********************************************************************
 *
 * Function:    fan_pwm_slope_write
 *
 * Description: write fan PWM slope register
 *
 * Inputs:      NONE
 *
 * Output:      pwm_slope - PWM slope for writting.
 *
 *********************************************************************
 */
void fan_pwm_slope_write (int pwm_slope) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    assert(dash_fpga);
    env_fan->pwm_slope = pwm_slope;

    return;
}

/*********************************************************************
 *
 * Function:    tachometer_rps_read
 *
 * Description: return tachometer RPS for specific fan number.
 *
 * Inputs:      fan_num - fan number
 *
 * Output:      NONE
 *
 *********************************************************************
 */
uint tachometer_rps_read (int fan_num) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint tach_rps = 0xFFFF;

    assert(dash_fpga);

    switch (fan_num) {
    case 1:
        tach_rps = env_fan->tach_rps1;
    break;
    case 2:
        tach_rps = env_fan->tach_rps2;
    break;
    default:
        printf("Unknown fan number %d\n", fan_num);
    break;
    }

    return (tach_rps);
}

/*********************************************************************
 *
 * Function:    fan_speed_rd
 *
 * Description: read fan speed 
 *
 * Inputs:      fan_no - fan number
 *
 * Output:      curr_spd - current speed 
 *
 *********************************************************************
 */
uint fan_speed_rd (int fan_no) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;
    uint curr_spd;

    assert(dash_fpga);

    switch (fan_no) {
    case FAN_NO_1:
        curr_spd = env_fan->speed1;
    break;
    case FAN_NO_2:
        curr_spd = env_fan->speed2;
    break;
    default:
        printf("Unknown fan number %d \n", fan_no);
        return 0x7D1; /* larger than max speed, for detect error */
    break;
    }

    return (curr_spd);
}

/*********************************************************************
 *
 * Function:    fan_speed_wr
 *
 * Description: write fan speed
 *
 * Inputs:      fan_no - fan number, fan_spd - fan speed 
 *
 * Output:      NONE
 *
 *********************************************************************
 */
void fan_speed_wr (int fan_no, uint curr_spd) {

    unsigned long addr = get_platform_env_fan_base();
    env_fan_t *env_fan = (env_fan_t *)addr;

    switch (fan_no) {
    case FAN_NO_1:
        env_fan->speed1 = curr_spd;
    break;
    case FAN_NO_2:;
        env_fan->speed2 = curr_spd;
    break;
    default:
        return;
    break;
    }

    return;
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

    fan_num = getdec_answer("Fan num 1-2::", 1, 1, 2);

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

    fan_num = getdec_answer("Fan num 1-2::", 1, 1, 2);

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

    fan_num = getdec_answer("Fan num 1-2::", 1, 1, 2);

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

    fan_num = getdec_answer("Fan num 1-2::", 1, 1, 2);

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

    for (fan_num = 1; fan_num < 3; fan_num++) {

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
    printf("\n");
}

/*********************************************************************
 *
 * Function:    display_smartfan_info_workaround
 *
 * Description: Display smart fan info which is reading from FPGA.
 *              With retry mechanism as workaorund.
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 *********************************************************************
 */
void display_smartfan_info_workaround (void) {
    fpga_smartfan_display_workaround(FAN1);
    fpga_smartfan_display_workaround(FAN2);
    printf("\n");
}

/*********************************************************************
 *
 * Function:	idv_fan_spd_test
 *
 * Description: Individual fan speed test 
 *
 * Inputs:	int fan_num
 *
 * Outputs:	PASSED / FAILED.
 *
 *********************************************************************
 */
static int idv_fan_spd_test (int fan_num) {

    uint save_pwm_slope;
    uint save_pwm_setting;
    uint techo_40rps, techo_50rps, techo_60rps;
    uint techo_40rpm, techo_50rpm, techo_60rpm;
    uint cal_techo_50rpm, techo_50rpm_low, techo_50rpm_high;

    /* 1. Log the current PWM slope
     */
    save_pwm_slope = fan_pwm_slope_read();
    printf("Current pwm slope is 0x%x. \n", save_pwm_slope);

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
    fan_pwm_slope_write(FAN_PWM_SLOPE_MAX);

    /* 4. Set fan speed PWM to 40%, 50%, 60%, and log the technometer
     */
    fan_speed_wr(fan_num, FAN_SPD_40PER_PWM);
    printf("Setting Fan Speed PWM to 40%%, Waiting ...(10 secs)\n");
    msleep(FAN_TEST_DURATION);
    techo_40rps = tachometer_rps_read(fan_num);
    techo_40rpm = techo_40rps * RPS_TO_RPM;
    printf("Fan %d techometer at 40pct PWM is 0x%x => RPM %d, \n", fan_num, techo_40rps, techo_40rpm);

    fan_speed_wr(fan_num, FAN_SPD_50PER_PWM);
    printf("Setting Fan Speed PWM to 50%%, Waiting ...(10 secs)\n");
    msleep(FAN_TEST_DURATION);
    techo_50rps = tachometer_rps_read(fan_num);
    techo_50rpm = techo_50rps * RPS_TO_RPM;
    printf("Fan %d techometer at 50pct PWM is 0x%x => RPM %d, \n", fan_num, techo_50rps, techo_50rpm);

    fan_speed_wr(fan_num, FAN_SPD_60PER_PWM);
    printf("Setting Fan Speed PWM to 60%%, Waiting ...(10 secs)\n");
    msleep(FAN_TEST_DURATION);
    techo_60rps = tachometer_rps_read(fan_num);
    techo_60rpm = techo_60rps * RPS_TO_RPM;
    printf("Fan %d techometer at 60pct PWM is 0x%x => RPM %d, \n", fan_num, techo_60rps, techo_60rpm);

    /* 5. Restore PWM slope original value, PWM % original value
     */
    fan_pwm_slope_write(save_pwm_slope);
    fan_speed_wr(fan_num, save_pwm_setting);
    printf("Restore pwm slope to 0x%x and restore pwm setting to 0x%x\n", save_pwm_slope, save_pwm_setting);

    /* 6. Calculate the slope between the high-speed and low-speed reading.  Interpolate the mid-speed reading.
     * Check that the mid-speed reading is on the slope within X%.
     * (X is currently unknown and may have to be adjusted could be an
     * input parameter for now. Initially default to 10%)
     */
    cal_techo_50rpm = (techo_60rpm - techo_40rpm) / 2 + techo_40rpm;
    techo_50rpm_low = cal_techo_50rpm * LOWER_BOUNDARY;
    techo_50rpm_high = cal_techo_50rpm * UPPER_BOUNDARY;
    printf("Calculated RPS at 50pct is %d. 5pct tolerance range: min (%d), high (%d) \n", 
	cal_techo_50rpm, techo_50rpm_low, techo_50rpm_high);
    
    if ((techo_50rpm == 0) || (techo_50rpm_low == 0) || (techo_50rpm_high == 0)) {
        cterr('f',0, "Cannot read Fan speed.\n");
	    return(FAILED);
    }

    if ((techo_50rpm < techo_50rpm_low) || (techo_50rpm > techo_50rpm_high)) {
        cterr('f',0, "Fan %d speed test failed. Measured RPS at 50pct (%d) is outside tolerance.\n", 
	    fan_num, techo_50rpm);
	    return(FAILED);
    }
    printf("Fan %d speed passed. Measured RPS at 50pct (%d) is within tolerance.\n", 
	fan_num, techo_50rpm);

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
 * Outputs:	PASSED / FAILED.
 *
 *********************************************************************
 */
int fan_speed_test (void)
{
    char *tname = "Fan speed";
    int fn;
    int result = PASSED;

    testname("%s", tname);
    prpass(testpass, "");

    /* Tabei-L has 2 fans
     */
    for (fn=TEST_FAN1; fn <=TEST_FAN2; fn++) {
        printf("\nFan %d speed test... \n", fn);

        if (idv_fan_spd_test(fn) == FAILED) {
	        result = FAILED;
	    }
    }

    return(result);
}
/*------------------------------------------------------------------
$Log: platform_fan.c,v $
Revision 1.5  2020/09/21 06:11:19  kehuang2
CSCv74461: Fan speed test may have the concern to sample the speed in non-linear zone

Revision 1.4  2020/08/17 07:22:40  kehuang2
CSCvv34796: Support fan speed test

Revision 1.3  2020/08/06 07:54:55  kehuang2
Collapse Promethium into main trunk

Revision 1.2  2019/10/17 02:16:26  kehuang2
Collapse Tabei-L into main trunk

Revision 1.1.2.4  2019/09/02 08:38:02  olin2
support display smart fan info util

Revision 1.1.2.3  2018/12/05 06:39:20  olin2
Update Fan control for NIOS

Revision 1.1.2.2  2018/11/28 07:37:27  olin2
Update fan util

Revision 1.1.2.1  2018/11/15 06:56:06  olin2
initial commit for Fan utils

Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:36  alpeng
porting neptune x86 to curie

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
