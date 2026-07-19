/* $Id: platform_fan.c,v 1.2 2021/04/15 00:52:27 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/platform_fan.c,v $
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
void build_fan_menu(void);
void build_fan_test_menu(int);
static int fan_nebs_pin_test(void);
int fan_speed_test(void);
int fan_full_speed_test(void);


#define FANS_NUM 4
#define FAN_TEST_DURATION 8000 /* ms */
#define FAN_RETRY_DURATION 4000 /* ms */
#define FAN_RETRY_TIMES 2

typedef struct {
    uint rpm;
    uint rpm_h;
    uint rpm_l;
    uint pwm;
} fan_rpm_tbl_t;

enum {
    E_PWM_IDX_INVALID = -1,
    E_PWM_0_PCT_IDX = 0,
    E_PWM_10_PCT_IDX = 1,
    E_PWM_20_PCT_IDX = 2,
    E_PWM_30_PCT_IDX = 3,
    E_PWM_40_PCT_IDX = 4,
    E_PWM_50_PCT_IDX = 5,
    E_PWM_60_PCT_IDX = 6,
    E_PWM_70_PCT_IDX = 7,
    E_PWM_80_PCT_IDX = 8,
    E_PWM_90_PCT_IDX = 9,
    E_PWM_100_PCT_IDX = 10,
    E_PWM_IDX_MAX = 11,
};

#define FAN_SPEED_TEST_CASES_NUM 4
static int phoenix_fan_spd_tcs[FAN_SPEED_TEST_CASES_NUM] = {
    E_PWM_30_PCT_IDX,
    E_PWM_50_PCT_IDX,
    E_PWM_70_PCT_IDX,
    E_PWM_90_PCT_IDX,
};

static int phoenix_fan_full_spd_tc = E_PWM_100_PCT_IDX;

/* Note:
 * Below Spec. value is Sunon fan. This pass criteria will be also for Nidec fan.
 * So, thermal team and hardware team suggest the pass criteria are as below.
 * The pass criteria have included Nidec fan Spec.
 * Maximum RPM. follws Sunon fan Spec.
 * Minimum RPM. follow Nidec fan Spec,
 */
static fan_rpm_tbl_t phoenix_fan_rpm_spec_table[] = {
    {     0,   500,    0, FAN_SPD_0PCT_PWM   },  /*   0% PWM duty cycle, +-500 */
    {     0,   500,    0, FAN_SPD_10PCT_PWM  },  /*  10% PWM duty cycle, +-500 */
    {  2200,  2700, 1460, FAN_SPD_20PCT_PWM  },  /*  20% PWM duty cycle, +-500 */
    {  3630,  4130, 2874, FAN_SPD_30PCT_PWM  },  /*  30% PWM duty cycle, +-12% */
    {  5170,  5770, 4024, FAN_SPD_40PCT_PWM  },  /*  40% PWM duty cycle, +-12% */
    {  6600,  7200, 5174, FAN_SPD_50PCT_PWM  },  /*  50% PWM duty cycle, +-12% */
    {  8030,  8630, 6323, FAN_SPD_60PCT_PWM  },  /*  60% PWM duty cycle, +-12% */
    {  9570, 10290, 7473, FAN_SPD_70PCT_PWM  },  /*  70% PWM duty cycle, +-12% */
    { 11000, 12100, 8624, FAN_SPD_80PCT_PWM  },  /*  80% PWM duty cycel, +-12% */
    { 11000, 12100, 8624, FAN_SPD_90PCT_PWM  },  /*  90% PWM duty cycel, +-12% */
    { 11000, 12100, 8624, FAN_SPD_100PCT_PWM },  /* 100% PWM duty cycel, +-12% */
};

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

static submenu_xtable_t fan_test_menu_table[] = {
    { "FAN Utility ", (PFT)build_fan_menu, 0, 0,
      (type_t(*)())0, 0, (PFT)0, 0 },

    { "NEBS FAN Tray Pin Test ", (PFT)fan_nebs_pin_test, 0,
      MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
      (type_t(*)())0, 0, (PFT)0, 0},

    { "Check FAN Full Speed ", (PFT)fan_full_speed_test, 0,
      MF_CONTINUOUS | MF_DOGRP | MF_DOALL,
      (type_t(*)())0, 0, (PFT)0, 0},

    { "Check FAN Multiple Speeds ", (PFT)fan_speed_test, 0, 0,
      (type_t(*)())0, 0, (PFT)0, 0},
};

#define FAN_MENU_TABLE_SIZE (sizeof(fan_menu_table) / sizeof(submenu_xtable_t))
#define FAN_TEST_MENU_TABLE_SIZE (sizeof(fan_test_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fan_menu_primary_items[FAN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t fan_menu_secondary_items[FAN_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t fan_test_menu_primary_items[FAN_TEST_MENU_TABLE_SIZE + MAX_BASE_ITEMS];
static mitem_t fan_test_menu_secondary_items[FAN_TEST_MENU_TABLE_SIZE + MAX_BASE_ITEMS];

static struct menuinfo fandiag = {
    "FAN Control Unit Utility Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    fan_menu_primary_items,
};

static struct menuinfo fantestdiag = {
    "FAN Test Menu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    fan_test_menu_primary_items,
};

static struct menuinfo *fandiagp = &fandiag;
static struct menuinfo *fantestdiagp = &fantestdiag;


/**********************************************************************
 *
 * function:	build_fan_menu
 *
 * Description:	Build FAN Control Unit Utility menu.
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


/**********************************************************************
 *
 * function:	build_fan_test_menu
 *
 * Description:	Build FAN test menu.
 *
 * Input:	opt.
 *
 * Output:	None.
 *
 **********************************************************************
 */
void build_fan_test_menu (int opt)
{
    build_primary_submenu(fan_test_menu_table, FAN_TEST_MENU_TABLE_SIZE,
			  "FAN Test Menu", &fantestdiagp);
    build_secondary_submenu(fan_test_menu_table, FAN_TEST_MENU_TABLE_SIZE,
			    fan_test_menu_secondary_items);

    if (opt) {
        /* Entered with submenu */
        menu(&fantestdiag, fan_test_menu_secondary_items, 0);
    } else {
        do_all_menu_items(fantestdiagp);
    }
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
    case FAN_NO_1:
        tach_rps = env_fan->tach_rps1;
    break;
    case FAN_NO_2:
        tach_rps = env_fan->tach_rps2;
    break;
    case FAN_NO_3:
        tach_rps = env_fan->tach_rps3;
    break;
    case FAN_NO_4:
        tach_rps = env_fan->tach_rps4;
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
    case FAN_NO_3:
        curr_spd = env_fan->speed3;
    break;
    case FAN_NO_4:
        curr_spd = env_fan->speed4;
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
    case FAN_NO_2:
        env_fan->speed2 = curr_spd;
    break;
    case FAN_NO_3:
        env_fan->speed3 = curr_spd;
    break;
    case FAN_NO_4:
        env_fan->speed4 = curr_spd;
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

    fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);

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

    fan_num = getdec_answer("Fan num 1-4::", 1, 1, 4);

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
    if (fan_spd > FAN_SPD_100PCT_PWM) {
        printf("Get fan speed failed, fan speed is %d, which is larger than 100%% \n",
             FAN_SPD_100PCT_PWM);
        return;
    }

    printf("Current fan%d speed(PWM) is 0x%x \n", fan_num, fan_spd);
    if (getc_answer("Change the speed?", "yn", 'n') == 'y') {
         printf("min - 0%%, max - 100%%(0x%x), default - 35%% (0x%x) \n",
                FAN_SPD_100PCT_PWM, FAN_SPD_35PCT_PWM);
         fan_spd = gethex_answer("Enter the data:", fan_spd, FAN_SPD_0PCT_PWM, FAN_SPD_100PCT_PWM);
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

    for (fan_num = FAN_NO_1; fan_num <= FAN_NO_4; fan_num++) {

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
 * Function: fan_nebs_pin_test
 *
 * Description: Check NEBS FAN tray pin, it must be 0 no matter FAN
 *              tray is installed or not in Phoenix.
 *
 * Inputs: None
 *
 * Outputs: PASSED/FAILED
 *
 *********************************************************************
 */
static int fan_nebs_pin_test (void)
{
    char *tname ="NEBS FAN Tray Pin Test";
    int ret = PASSED;
    int status;

    testname(tname);
    prpass(testpass, "");

    status = get_fan_status();
    if (status & NEBS_FAN_TRAY_PRESENT) {
        cterr('f', 0, "NEBS pin isn't 0, FAN status: 0x%x\n", status);
        ret = FAILED;
    }
    prcomplete(testpass, errcount, (char *)0);

    return (ret);
}


/*********************************************************************
 *
 * Function: fan_rpm_check
 *
 * Description: Check FAN RPM speed based on pass criteria, will retry
 *              2 times if failed.
 *              FAN speed may need to take some time to be stable.
 *
 * Inputs:  int fan - FAN NO.
 *          int idx - the index of test case.
 *
 * Outputs: PASSED / FAILED.
 *
 *********************************************************************
 */
static int fan_rpm_check(int fan, int idx)
{
    int retry;
    uint pwm, rps, rpm, rpm_l, rpm_h;

    pwm = fan_speed_rd(fan);
    rpm_l = phoenix_fan_rpm_spec_table[idx].rpm_l;
    rpm_h = phoenix_fan_rpm_spec_table[idx].rpm_h;

    for (retry = 0; retry <= FAN_RETRY_TIMES; retry++) {
        rps = tachometer_rps_read(fan);
        rpm = rps * RPS_TO_RPM;

        printf("\nFan %u, PWM %d%%(0x%X) speed %u RPM.\n"
               "Tolerance range: %u ~ %u \n",
               fan, idx*10, pwm, rpm, rpm_l, rpm_h);

        if (rpm > rpm_l && rpm < rpm_h) {
            return (PASSED);
        } else if (retry < FAN_RETRY_TIMES) {
            printf("Retrying %d, Wait %d ms ...\n", retry+1, FAN_RETRY_DURATION);
            msleep(FAN_RETRY_DURATION);
        }
    }

    return (FAILED);
}


/*********************************************************************
 *
 * Function:    all_fans_spd_test
 *
 * Description:  all fans speed test
 *
 * Inputs:  int* arTc - the test case table array
 *          int numTc - the number of test cases.
 *
 * Outputs: PASSED / FAILED.
 *
 *********************************************************************
 */
static int all_fans_spd_test (int *arTc, int numTc)
{
    uint save_pwm_slope;
    uint save_pwm_setting[FANS_NUM];
    uint fan, pwm;
    int tc, idx, fanChk, ret = PASSED;

    /* 1. Log the current PWM slope
     */
    save_pwm_slope = fan_pwm_slope_read();
    printf("Current pwm slope is 0x%x. \n", save_pwm_slope);

    /* 2. Log the current PWM setting (i.e. fan speed)
     */
    for (fan = FAN_NO_1; fan <= FANS_NUM; fan++) {
        save_pwm_setting[fan-1] = fan_speed_rd(fan);
        if (save_pwm_setting[fan-1] > FAN_SPD_100PCT_PWM) {
            printf("Get fan %d speed(PWM) failed, value is 0x%x, "
                   "which is larger than 100 percent value 0x%x \n",
                   fan, save_pwm_setting[fan-1], FAN_SPD_100PCT_PWM);
            return (FAILED);
        }
        printf("Current fan %d speed(PWM) setting is 0x%x \n",
               fan, save_pwm_setting[fan-1]);
    }

    /* 3. Write PWM slope = 0x3FF (max)
     */
    fan_pwm_slope_write(FAN_PWM_SLOPE_MAX);

    /* 4. Set the fan speed PWM based on test cases and check the technometer.
     *    Based on the all test cases' criteria to check the all FANs' RPM
     *    that read via technometer.
     *    The pass criteria should follow FAN or FAN tray spec. and discuss
     *    with hardware team.
     */
    for (tc = 0; tc < numTc; tc++ ) {
        idx = arTc[tc];
        if (idx >= E_PWM_IDX_MAX || idx < 0) {
            printf("Invalid PWM index of test case, %d\n", idx);
            ret = FAILED;
            continue;
        }

        pwm = phoenix_fan_rpm_spec_table[idx].pwm;
        for (fan = FAN_NO_1; fan <= FANS_NUM; fan++) {
            fan_speed_wr(fan, pwm);
        }

        printf("\nSetting Fan Speed PWM to %d%%, Waiting ...(%d ms)\n",
               idx*10, FAN_TEST_DURATION);
        msleep(FAN_TEST_DURATION);

        for (fan = FAN_NO_1; fan <= FANS_NUM; fan++) {
            fanChk = fan_rpm_check(fan, idx);
            if (fanChk != PASSED) {
                ret = FAILED;
                printf("\nFan %d PWM %d%% speed test failed.\n", fan, idx*10);
            } else {
                printf("\nFan %d PWM %d%% speed test passed.\n", fan, idx*10);
            }
        }
    }

    /* 5. Restore PWM slope original value, PWM % original value
     */
    fan_pwm_slope_write(save_pwm_slope);
    printf("\n");
    for (fan = FAN_NO_1; fan <= FANS_NUM; fan++) {
        fan_speed_wr(fan, save_pwm_setting[fan-1]);
        printf("Fan %d, restore PWM slope to 0x%x and restore PWM setting to 0x%x\n",
               fan, save_pwm_slope, save_pwm_setting[fan-1]);
    }

    return (ret);
}


/*********************************************************************
 *
 * Function:    fan_speed_test
 *
 * Description: Platfrom fan speed test
 *              Not default test case.
 *
 * Inputs:  None.
 *
 * Outputs: PASSED / FAILED.
 *
 *********************************************************************
 */
int fan_speed_test (void)
{
    char *tname = "Fan speed";
    int status;
    int ret = PASSED;

    testname("%s", tname);
    prpass(testpass, "\n");

    status = get_fan_status();
    if (!(status & FAN_TRAY_PRESENT)) {
        printf("%s(): FAN tray doesn't present\n", __func__);
        ret = FAILED;
        goto ret_exit;
    }

    /* All fans, 4 different speed test case. */
    ret = all_fans_spd_test(phoenix_fan_spd_tcs, FAN_SPEED_TEST_CASES_NUM);

ret_exit:
    printf("\nFan speed test is %s.\n", (ret == PASSED)?"passed":"failed");

    return (ret);
}


/*********************************************************************
 *
 * Function:    fan_full_speed_test
 *
 * Description: Platfrom fan full speed test
 *
 * Inputs:  None.
 *
 * Outputs: PASSED / FAILED.
 *
 *********************************************************************
 */
int fan_full_speed_test (void)
{
    char *tname = "Fan full speed";
    int status;
    int ret = PASSED;

    testname("%s", tname);
    prpass(testpass, "\n");

    status = get_fan_status();
    if (!(status & FAN_TRAY_PRESENT)) {
        printf("%s(): FAN tray doesn't present\n", __func__);
        ret = FAILED;
        goto ret_exit;
    }

    /* All fans, full speed test case */
    ret = all_fans_spd_test(&phoenix_fan_full_spd_tc, 1);

ret_exit:
    if (ret != PASSED) {
        cterr('f',0, "Fan full speed test is failed.\n");
    }
    prcomplete(testpass, errcount, (char *)0);

    return (ret);
}
