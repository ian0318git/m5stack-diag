/* $Id: platform_fan.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_fan.c,v $
 *------------------------------------------------------------------
 * Filename:	platform_fan.c
 *
 * Description:  accessing dash FPGA registers to control 
 *               FAN related information. 
 *
 *
 * Copyright (c) 2016-2020 by Cisco Systems, Inc.
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
void build_fan_menu (void)
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

    fan_num = getdec_answer("Fan num 1-4:", 1, 1, 4);

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

    fan_num = getdec_answer("Fan num 1-4:", 1, 1, 4);

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

    status = get_fan_status();

    printf("FAN tray present: %s \n", (status & FAN_TRAY_PRESENT) ? "YES" : "NO");
    /* CSCvo59196-22 : Changed FAN enable status from FPGA FAN status register to control register.*/    
    status = get_fan_control_reg();

    /* Fugazi has 4 fans */
    for (fan_num = 1; fan_num < 5; fan_num++) {
       /* FAN1_ENABLE - 0x100;
        * FAN2_ENABLE - 0x200;
        * FAN3_ENABLE - 0x400;
        * FAN4_ENABLE - 0x800;
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

/*********************************************************************
 *
 * Function:	idv_fan_spd_test
 *
 * Description: Individual fan speed test 
 * when the diags application is started, NIOS will be put to ‘diags sleep mode’.  
 * The fans rotation will be determined by the last setting NIOS put in depending on temperature and altitude.
 * 1. Log the current PWM setting (reg +0x20-0x2C)
 * 2. Log the current PWM slope (reg +0x8)
 * 3. Write PWM slope = 0x3FF
 * 4. Set fan speed PWM to 40%.  Value should be 40%*2000 = 800
 * 5. Let the fan speed settle, wait ~10s
 * 6. Log RPS register, low-speed
 * 7. Set fan speed PWM to 50%.  Value should be 50%*2000 = 1000
 * 8. Let the fan speed settle, mid-speed
 * 9. Set fan speed PWM to 60%.  Value should be 60%*2000 = 1200
 * 10. Let the fan speed settle, high-speed
 * 11. Test Max fan speed, make sure the Fan's tachometer can speed up to 100%
 * 12. Write PWM slope original value
 * 13. Write PWM % original value
 * 14. Calculate the slope between the high-speed and low-speed reading.  Interpolate the mid-speed reading.
 * 15. Check that the mid-speed reading is on the slope within 10%. could be adjusted – Initially default to 10%)
 *
 * CSCvo59196-19 : Fugazi: Add Fan speed test in default test
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
    uint techo_40rps, techo_50rps, techo_60rps, techo_100rps;
    uint techo_40rpm, techo_50rpm, techo_60rpm, techo_100rpm;
    uint cal_techo_50rpm, techo_50rpm_low, techo_50rpm_high;

    /* 1. Log the current PWM slope
     */
    save_pwm_slope = fan_pwm_slope_read();
    printf("Fan %d Current pwm slope is 0x%x. \n", fan_num, save_pwm_slope);

    /* 2. Log the current PWM setting (i.e. fan speed)
     */
    save_pwm_setting = fan_speed_rd(fan_num);
    if (save_pwm_setting > FAN_SPD_100PER_PWM) {
        printf("Get fan %d speed(PWM) failed, value is 0x%x, which is larger "
               "than 100 percent value 0x%x \n",
	    fan_num, save_pwm_setting, FAN_SPD_100PER_PWM);
        return(FAILED);
    }
    printf("Fan %d Current speed(PWM) setting is 0x%x \n", fan_num, 
            save_pwm_setting);

    /* 3. Write PWM slope = 0x3FF (max)
     */
    fan_pwm_slope_write(FAN_PWM_SLOPE_MAX);

    /* 4. Set fan speed PWM to 40%, 50%, 60%, and log the tachnometer
     */
    fan_speed_wr(fan_num, FAN_SPD_40PER_PWM);
    printf("Fan %d Setting Fan Speed PWM to 40%%, Waiting ...(10 secs)\n", 
            fan_num);
    msleep(FAN_TEST_DURATION);
    techo_40rps = tachometer_rps_read(fan_num);
    techo_40rpm = techo_40rps * RPS_TO_RPM;
    printf("Fan %d Tachometer at 40pct PWM is 0x%x => RPM %d, \n", fan_num, 
            techo_40rps, techo_40rpm);

    fan_speed_wr(fan_num, FAN_SPD_50PER_PWM);
    printf("Fan %d Setting Fan Speed PWM to 50%%, Waiting ...(10 secs)\n", 
            fan_num);
    msleep(FAN_TEST_DURATION);
    techo_50rps = tachometer_rps_read(fan_num);
    techo_50rpm = techo_50rps * RPS_TO_RPM;
    printf("Fan %d tachometer at 50pct PWM is 0x%x => RPM %d, \n", fan_num, 
            techo_50rps, techo_50rpm);

    fan_speed_wr(fan_num, FAN_SPD_60PER_PWM);
    printf("Fan %d Setting Fan Speed PWM to 60%%, Waiting ...(10 secs)\n", 
            fan_num);
    msleep(FAN_TEST_DURATION);
    techo_60rps = tachometer_rps_read(fan_num);
    techo_60rpm = techo_60rps * RPS_TO_RPM;
    printf("Fan %d Tachometer at 60pct PWM is 0x%x => RPM %d, \n", fan_num, 
            techo_60rps, techo_60rpm);

    /* 5 Test Max fan speed, make sure the Fan's tachometer can speed up to 100%
     */
    fan_speed_wr(fan_num, FAN_SPD_100PER_PWM);
    printf("Fan %d Testing Fan Speed PWM to 100%%, Waiting ...(10 secs)\n", 
            fan_num);
    msleep(FAN_TEST_DURATION);
    techo_100rps = tachometer_rps_read(fan_num);
    techo_100rpm = techo_100rps * RPS_TO_RPM;
    printf("Fan %d Tachometer RPM at 100pct PWM, RPM is %d, tolerance range: "
           "min (%f), high (%f) \n", fan_num, techo_100rpm, 
           FAN_SPD_100RPM * LOWER_TOLERANCE, 
           FAN_SPD_100RPM * UPPER_TOLERANCE);
    
    if ((techo_100rpm < (FAN_SPD_100RPM * LOWER_TOLERANCE)) || 
        (techo_100rpm > (FAN_SPD_100RPM * UPPER_TOLERANCE))) {
        cterr('f',0, "Fan %d speed test failed. Measured RPM at 100pct (%d) is "
              "outside tolerance.\n", fan_num, techo_100rpm);
	    return(FAILED);
    }
    /* 6. Restore PWM slope original value, PWM % original value
     */
    fan_pwm_slope_write(save_pwm_slope);
    fan_speed_wr(fan_num, save_pwm_setting);
    printf("Fan %d Restore pwm slope to 0x%x and restore pwm setting to 0x%x\n", 
            fan_num, save_pwm_slope, save_pwm_setting);

    if ((techo_40rpm >= techo_50rpm) || (techo_50rpm >= techo_60rpm)) {
	    cterr('f',0, "Fan speed is not correct - low RPM is not less than high "
              "RPM from tachometer read at Fan %d.\n", fan_num);
        return(FAILED);
    }
    /* 7. Calculate the slope between the high-speed and low-speed reading.  Interpolate the mid-speed reading.
     * Check that the mid-speed reading is on the slope within X%.
     * (X is currently unknown and may have to be adjusted could be an
     * input parameter for now. Initially default to 10%)
     */
    cal_techo_50rpm = (techo_60rpm - techo_40rpm) / 2 + techo_40rpm;
    techo_50rpm_low = cal_techo_50rpm * LOWER_BOUNDARY;
    techo_50rpm_high = cal_techo_50rpm * UPPER_BOUNDARY;
    printf("Calculated RPM at 50pct is %d. 10pct tolerance range: min (%d), "
           "high (%d) \n", cal_techo_50rpm, techo_50rpm_low, techo_50rpm_high);
    
    if ((techo_50rpm == 0) || (techo_50rpm_low == 0) || (techo_50rpm_high == 0)) {
        cterr('f',0, "Cannot read Fan speed, mid-speed read RPM is zero at Fan"
              " %d.\n", fan_num);
        return(FAILED);
    }

    if ((techo_50rpm < techo_50rpm_low) || (techo_50rpm > techo_50rpm_high)) {
        cterr('f',0, "Fan %d speed test failed. Measured RPM at 50pct (%d) is "
              "outside tolerance.\n", fan_num, techo_50rpm);
	    return(FAILED);
    }
    printf("Fan %d Speed passed. Measured RPM at 50pct (%d) is within tolerance."
           "\n", fan_num, techo_50rpm);

    return(PASSED);
}

/*********************************************************************
 *
 * Function:	fan_speed_test
 *
 * Description: Platfrom fan speed test
 *              CSCvo59196-19 : Add Fan speed test in default test  
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

    /* Fugazi has 4 fans
     */
    for (fn = FAN_NO_1; fn <= FAN_NO_4; fn++) {
        printf("\nFan %d speed test... \n", fn);

        if (idv_fan_spd_test(fn) == FAILED) {
	        result = FAILED;
	    }
    }

    return(result);
}
/*-------------------------------------------------
 * $Log: platform_fan.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.5  2021/01/21 07:43:47  iachang
 * CSCvo59196-22 : Changed FAN enable status from FPGA FAN status register to control register.
 *
 * Revision 1.1.8.4  2020/10/20 03:38:44  iachang
 * CSCvo59196-19 : Add Fan PWM 100% speed test
 *
 * Revision 1.1.8.3  2020/10/14 02:06:05  iachang
 * CSCvo59196-19 : Add Fan speed test in default test
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.6  2020/08/06 11:32:08  iachang
 * Code clean up.
 *
 * Revision 1.1.6.5  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.4  2019/07/19 07:35:30  letsai
 * 1. Support LED control.
 * 2. Support smart fan.
 * 3. Change BCM 54194 phy reset bit.
 *
 * Revision 1.1.6.3  2019/03/30 00:56:02  letsai
 * 1. Add USB console detect utility.
 * 2. Modify FAN utility.
 * 3. Remove unused items.
 * 4. Fix BCM54194 phy register test.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:36  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
