/* $Id: platform_fan.c,v 1.2 2019/06/14 05:24:50 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_fan.c,v $
 *------------------------------------------------------------------
 * Filename:    katar_platform_fan.c
 *
 * Description:	accessing dash FPGA registers to control 
 *				FAN related information. 
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
#include <sys/io.h>

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
#include "platform_fpga.h"
#include "linux_api.h"

#define FAN_TEST_COUNT			5
#define FAN_PRECISION			60
#define FAN_HIGH_THRESHOLD		(((6300/FAN_PRECISION)+1)*FAN_PRECISION)	//SPEC 5300 + 10% is for free air environment, use 6300 when installed
#define FAN_LOW_THRESHOLD		((4770/FAN_PRECISION)*FAN_PRECISION)		//From SPEC 5300 - 10%
#define FAN_MIN_SPEED			1000

void show_fan_sts(void);
static void katar_fan_pwm_ctrl(void);
static void katar_fan_threshold_ctrl (void);
static void katar_cpu_force_high_setting(void);
void katar_show_fan_info(void);

extern float get_current_temperature (int ts_id);

static submenu_xtable_t fan_menu_table[] = {
    {"Show fan status ",                   (PFT)show_fan_sts,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Fan PWM control ",                   (PFT)katar_fan_pwm_ctrl,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
    {"Fan RPM threshold ",                 (PFT)katar_fan_threshold_ctrl,                0,
	0,                                 (type_t(*)())0, 0, (PFT)0, 0},
	{"Fan force high setting ",            (PFT)katar_cpu_force_high_setting,                0,
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
void show_fan_sts (void) 
{
	katar_show_fan_info();
	return;
}

/*********************************************************************
 *
 * Function:	fan_pwm_ctrl
 *
 * Description:	Fan PWM control
 *
 * Inputs:	None.
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static void katar_fan_pwm_ctrl (void)
{
	int pwm_set,pwm_cur;

	pwm_cur = katar_get_fan_pwm_current();
	pwm_set = katar_get_fan_pwm_setting();

    printf("Current fan PWM setting is %d%% current PWM is %d%%\n", pwm_set,pwm_cur);
    if (getc_answer("Change the PWM setting?", "yn", 'n') == 'y') {
         pwm_set = getdec_answer("Enter the data:", pwm_set, 40, 100);
         katar_set_fan_pwm(pwm_set);
    }

    return;
}

#define GPIO46_MASK (1<<(46-32))
static boolean katar_get_fan_force_high_gpio(void)
{
    uint32_t gpiobase;
    uint32_t gpio_lv_2;

    gpiobase = pci_config_read(0x00, 0x1F, 0x00, 0x48);

    if(gpiobase == 0xffffffff) {
        printf("Cannot read GPIOBASE, are you running me as root?");
    } else if(gpiobase == 0) {
        printf("GPIOBASE not implemented at %04x", 0x48);
    } else if(!(gpiobase & 1)) {
        printf("GPIOBASE is not an I/O BAR");
    } else if(!(gpiobase & 0xfffc)) {
        printf("Cannot set GPIOBASE");
    }

    gpiobase &= 0xfffc;
    if(ioperm(gpiobase, 128, 1) == -1) {
        printf("Cannot access I/O ports %04x:%04x", gpiobase, gpiobase + 128);
    }
    gpio_lv_2 = inl(gpiobase+0x38);
    ioperm(gpiobase, 128, 0);

	if(gpio_lv_2&GPIO46_MASK)
		return TRUE;
	else
		return FALSE;
}

int katar_set_fan_force_high_gpio(boolean bSetOn)
{
	uint32_t gpiobase;
	int rc = FAILED;
	uint32_t gpio_use_2,gpio_sel_2,gpio_lv_2;
	
	gpiobase = pci_config_read(0x00, 0x1F, 0x00, 0x48);

	if(gpiobase == 0xffffffff) {
		printf("Cannot read GPIOBASE, are you running me as root?");
		return rc;
	} else if(gpiobase == 0) {
		printf("GPIOBASE not implemented at %04x", 0x48);
		return rc;
	} else if(!(gpiobase & 1)) {
		printf("GPIOBASE is not an I/O BAR");
		return rc;
	} else if(!(gpiobase & 0xfffc)) {
		printf("Cannot set GPIOBASE");
		return rc;
	}

	gpiobase &= 0xfffc;
	if(ioperm(gpiobase, 128, 1) == -1) {
	    printf("Cannot access I/O ports %04x:%04x", gpiobase, gpiobase + 128);
		return rc;
	}

	gpio_use_2 = inl(gpiobase+0x30);
	gpio_sel_2 = inl(gpiobase+0x34);
	gpio_lv_2 = inl(gpiobase+0x38);

	if (diagflag_xram & D_DEBUG_OPTIONS)
	{
		printf("gpio_use_2 = 0x%x ",gpio_use_2);
		printf("gpio_sel_2 = 0x%x ",gpio_sel_2);
		printf("gpio_lv_2 = 0x%x \n",gpio_lv_2);
	}

	//Check if GPIO46 set to output GPIO
	if((gpio_use_2&GPIO46_MASK)!=GPIO46_MASK)
	{
		if (diagflag_xram & D_DEBUG_OPTIONS)
			printf("GPIO46 isn't set to GPIO 0x%x\n",gpio_use_2);
		outl((gpio_use_2|GPIO46_MASK),gpiobase+0x30);
	}
	if((gpio_sel_2&GPIO46_MASK)!=0)
	{
		if (diagflag_xram & D_DEBUG_OPTIONS)
			printf("GPIO46 isn't set to output 0x%x\n",gpio_sel_2);
		outl(gpio_sel_2&(~GPIO46_MASK),gpiobase+0x34);
	}

	if(!bSetOn)
	{
		outl((gpio_lv_2&(~GPIO46_MASK)),gpiobase+0x38);
		rc = PASSED;
	}else
	{
		outl((gpio_lv_2|GPIO46_MASK),gpiobase+0x38);
		rc = PASSED;
	}

	ioperm(gpiobase, 128, 0);
	return rc;	
}

static void katar_cpu_force_high_setting(void)
{
	boolean Current = katar_get_fan_force_high_gpio();

	printf("CPU force fan speed is set %s\n",Current?"high":"low");
	if (getc_answer("Change the setting?", "yn", 'n') == 'y') {
		katar_set_fan_force_high_gpio(!Current);
		Current = katar_get_fan_force_high_gpio();
		printf("CPU force fan speed is set %s\n",Current?"high":"low");
    }else
		printf("Exit by user\n");
}

int katar_mb_fan_high_test(int dummy)
{
	uint waitcount = FAN_TEST_COUNT;
	uint tach_rpm0,tach_rpm1;
	char *tname = "Fan high speed test";
	int rc = FAILED;

	testname("%s", tname);

	//Force fan to high 
	if(!katar_get_fan_force_high_gpio())
	{
		katar_set_fan_force_high_gpio(TRUE);
		msleep(5000);
	}

	do {
		tach_rpm0 = katar_get_fan_speed(0);
        tach_rpm1 = katar_get_fan_speed(1);

		if((tach_rpm0>=FAN_HIGH_THRESHOLD) || (tach_rpm1>=FAN_HIGH_THRESHOLD))
		{
			msleep(1000);
            waitcount--;
		}else if((tach_rpm0>FAN_LOW_THRESHOLD) && (tach_rpm1>FAN_LOW_THRESHOLD))
			rc = PASSED;
		else
		{
			msleep(1000);
			waitcount--;
		}
	}while((waitcount>0) && (rc==FAILED));

	//Enable fan pwm control after test
	katar_set_fan_force_high_gpio(FALSE);
	if(rc==FAILED)
	{
		cterr('f',0, "FAN0 speed:%d , FAN1 speed:%d(expect between %d - %d)",tach_rpm0,tach_rpm1,FAN_LOW_THRESHOLD,FAN_HIGH_THRESHOLD);
	}else
	{
		prpass(testpass, NULL);
	}
	return rc;
}

int katar_mb_fan_low_test (int dummy)
{
    uint waitcount = FAN_TEST_COUNT;
    uint tach_rpm0,tach_rpm1;
    char *tname = "Fan low speed test";
    int rc = FAILED;
	float temp = get_current_temperature(0); 

    testname("%s", tname);

    //Check if force high
    if(katar_get_fan_force_high_gpio())
    {
        katar_set_fan_force_high_gpio(FALSE);
        msleep(5000);
    }

	if(temp < 5)
	{
		prpass(testpass, "Skip test if temperature < 5 Celsius");
		return PASSED;
	}

    do {
        tach_rpm0 = katar_get_fan_speed(0);
        tach_rpm1 = katar_get_fan_speed(1);

        if((tach_rpm0>FAN_MIN_SPEED) && (tach_rpm1>FAN_MIN_SPEED))
	        rc = PASSED;
        else
        {
            msleep(1000);
            waitcount--;
        }
    }while((waitcount>0)&& (rc==FAILED) );

    if(rc==FAILED)
    {
        cterr('f',0, "FAN0 speed:%d , FAN1 speed:%d",tach_rpm0,tach_rpm1);
    }else
    {
		prpass(testpass, NULL);
    }
    return rc;
}

/*********************************************************************
 *
 * Function:	katar_fan_threshold_ctrl
 *
 * Description:	Fan RPM threshold
 *
 * Inputs:	None.
 *
 * Outputs:	PASSE/FAILED.
 *
 *********************************************************************
 */
static void katar_fan_threshold_ctrl (void)
{
	uint rpm_th;

	rpm_th = katar_get_fan_rpm_threshold();

    printf("Current RPM threshold : %d\n", rpm_th);
	if (getc_answer("Change the RPM threshold?", "yn", 'n') == 'y') {
         rpm_th = getdec_answer("Enter the data:", rpm_th, 0x1, 0xFFFF);
         katar_set_fan_threshold(rpm_th);
    }

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
void katar_show_fan_info (void) 
{
	uint tach_rpm, fan_num ,pwm_cur ,pwm_set ,rpm_th;

	pwm_cur = katar_get_fan_pwm_current();
	pwm_set = katar_get_fan_pwm_setting();
	rpm_th = katar_get_fan_rpm_threshold();

	if(katar_get_fan_force_high_gpio())
        printf("----  CPU force fan speed is set high, PWM control won't work ----\n");

	printf("Current PWM setting is %d%% running at %d%%\n", pwm_set,pwm_cur);
	printf("Current RPM threshold is %d\n",rpm_th);
	for (fan_num = 0; fan_num < 2; fan_num++) {
		tach_rpm = katar_get_fan_speed(fan_num);

		if (tach_rpm == 0xFFFF) {
           printf("FAN %d is Running with unknown speed\n",
                   fan_num);
       } else { 
           printf("FAN %d is Running at %d RPM\n",
                    fan_num, tach_rpm);
       }
	}
}

/*
 *------------------------------------------------------------------
 * $Log: platform_fan.c,v $
 * Revision 1.2  2019/06/14 05:24:50  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.9  2019/06/12 00:34:56  mikech2
 * Modify fan speed upper bound to 6300
 *
 * Revision 1.1.2.8  2019/06/11 07:44:23  mikech2
 * Remove fan upper bound according to vendor recommendation
 *
 * Revision 1.1.2.7  2019/06/05 07:56:49  mikech2
 * Add upper bound for fan high speed test
 *
 * Revision 1.1.2.6  2019/06/03 08:52:10  mikech2
 * Change FAN_HIGH_THRESHOLD to 4770
 *
 * Revision 1.1.2.5  2019/04/25 00:32:24  mikech2
 * Disable force fan high when enter diag
 *
 * Revision 1.1.2.4  2019/04/22 06:58:08  mikech2
 * Fix -5 degree mb test fail issue
 *
 * Revision 1.1.2.3  2019/03/13 06:47:24  mikech2
 * Modify fan test duration
 *
 * Revision 1.1.2.2  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.7  2019/01/17 07:14:19  mikech2
 * Modify according to Kwok's review comments
 *
 * Revision 1.1.2.6  2018/12/27 03:49:00  mikech2
 * Modify prpass usage
 *
 * Revision 1.1.2.5  2018/11/19 08:46:11  mikech2
 * Fix typo
 *
 * Revision 1.1.2.4  2018/11/08 06:00:15  mikech2
 * Add fan low and interrupt test in mb test and remove intr utility
 *
 * Revision 1.1.2.3  2018/11/01 07:25:16  mikech2
 * Add fan speed test in mb test
 *
 * Revision 1.1.2.2  2018/10/29 08:27:53  mikech2
 * Add force fan spped high control and auto check for mb test
 *
 * Revision 1.1.2.1  2018/10/22 08:02:28  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.1  2018/06/20 07:31:25  mikech2
 * Add fan/led/margin control menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
