/* $Id: platform_led.c,v 1.3 2021/06/03 09:57:05 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_led.c,v $
 *------------------------------------------------------------------
 * Filename:    katar_platform_led.c
 *
 * Description:	accessing dash FPGA registers to control 
 *				LED related information. 
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
#include <unistd.h>
#include <sys/mman.h>

#include "common.h"
#include "types.h"
#include "defs.h"
#include "error.h"
#include "proto.h"
#include "menu.h"
#include "queryflags.h"
#include "cross_platform.h"
#include "common_utils.h"
#include "dev_print.h"
#include "platform_fpga.h"
#include "linux_api.h"
#include "platform_eth.h"
#include "mb_tests.h"
#include "platform_poe.h"

extern unsigned long diagflag_xram;
extern uint32_t diag_pci_get_device_bus(ushort vendor, ushort device, uint32_t *bus, uint32_t device_num);
extern void *mmap_device (char *path, size_t size, off_t offset);
extern int get_AQC_pci_num(int unit_num);
extern void AQ_set_LED_On(unsigned int port, unsigned int Led_on);
extern int katar_get_poe_54V_present (void);

static int show_led_sts(int dummy);
static int led_unit_color_all_util (int unit_num);
static int led_unit_color_util (int unit_num);
static int poe_led_unit_color_util (int unit_num);
static int phy_led_unit_color_util (int unit_num);
static int led_unit_blink_util (int unit_num);

/*
 * LED utility
 */
                                                                      
static struct mitem led_items[] = {
    {"Show FPGA LED status",                     0,0, (type_t(*)())show_led_sts,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"System LED color all util",               0,0, (type_t(*)())led_unit_color_all_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0}, 
    {"System LED color util",               0,0, (type_t(*)())led_unit_color_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
	{"High Availability LED color util",	0,0, (type_t(*)())led_unit_color_util,
							 (type_t *)&one, 0, (type_t(*)())0,	   0},
	{"Alarm LED color util",				0,0, (type_t(*)())led_unit_color_util,
							 (type_t *)&two, 0, (type_t(*)())0,	   0},
	{"System LED blink util",				0,0, (type_t(*)())led_unit_blink_util,
							 (type_t *)&zero, 0, (type_t(*)())0,	   0},
	{"High Availability LED blink util",	0,0, (type_t(*)())led_unit_blink_util,
							 (type_t *)&one, 0, (type_t(*)())0,    0},
	{"Alarm LED blink util",				0,0, (type_t(*)())led_unit_blink_util,
							 (type_t *)&two, 0, (type_t(*)())0,    0},
#ifdef ENABLE_POE_MODULE
	{"POE Port 0 Power LED color util",		0,0, (type_t(*)())poe_led_unit_color_util,
							 (type_t *)&zero, 0, (type_t(*)())0,	   0},
	{"POE Port 1 Power LED color util",		0,0, (type_t(*)())poe_led_unit_color_util,
							 (type_t *)&one, 0, (type_t(*)())0,	   0},
#endif
	{"POE System Power LED color util",		0,0, (type_t(*)())poe_led_unit_color_util,
							 (type_t *)&two, 0, (type_t(*)())0,	   0},
    {"GE PHY Port 0 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"GE PHY Port 1 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&one, 0, (type_t(*)())0,    0},
    {"10G PHY Port 0 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&two, 0, (type_t(*)())0,       0},
    {"10G PHY Port 1 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&three, 0, (type_t(*)())0,    0},
    {"2.5G PHY Port 0 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&four, 0, (type_t(*)())0,       0},
    {"2.5G PHY Port 1 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&five, 0, (type_t(*)())0,       0},
    {"2.5G PHY Port 2 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&six, 0, (type_t(*)())0,       0},
    {"2.5G PHY Port 3 Power LED color util",     0,0, (type_t(*)())phy_led_unit_color_util,
                             (type_t *)&seven, 0, (type_t(*)())0,       0},
};

static struct menuinfo led_menu = {
    "LED utility Menu",
    0,
    0,
    0,
    sizeof(led_items)/sizeof(struct mitem),
    led_items,
};
struct menuinfo *led_menup = &led_menu;

char led_name[LED_MAX][256] = {
	"System",
	"High Availability",
	"Alarm",
};

char poe_led_name[LED_POE_MAX][256] = {
	"POE Port 0 Power",
	"POE Port 1 Power",
	"POE System Power"
};

char phy_led_name[ETH_PORT_MAX][256] = {
    "GE PHY port 0",
    "GE PHY port 1",
    "10G PHY port 0",
    "10G PHY port 1",
    "2.5G PHY port 0",
    "2.5G PHY port 1",
    "2.5G PHY port 2",
    "2.5G PHY port 3",
};

static void set_I211_LEDCTRL(int unit_num, int bForceOn)
{
	uint32_t bus[2];
	uint32_t id_val,offset_val,vend_id,dev_id,dev_num;
	unsigned long addr = 0;
	off_t mmap_start;

	if(unit_num>ETH_GE_PORT1)
	{
		printf("Wrong unit_num:%d\n",unit_num);
		return;
	}

	dev_num = diag_pci_get_device_bus(0x8086,0x1539,bus,2);
	if(dev_num !=2)
	{
		printf("only found %d I211 dev \n",dev_num);
		return;
	}

	id_val = pci_config_read(bus[unit_num], 0x00, 0x00, 0x00);
    dev_id = (id_val & 0xFFFF0000) >> 16;
    vend_id = (id_val & 0x0000FFFF);

	if((dev_id!=0x1539)||(vend_id!=0x8086))
	{
		printf("VID:0x%x ; DID:0x%x \n",vend_id, dev_id);
		return;
	}
	offset_val = pci_config_read(bus[unit_num], 0x00, 0x00, 0x10);

	mmap_start = offset_val & ~(getpagesize()-1);
	offset_val &= getpagesize()-1;
	offset_val += 0xe00; //LEDCTL
	addr = (unsigned long)mmap_device("mem", 0x1000, mmap_start);
	if(bForceOn)
		register_write((addr +  offset_val), 0x000E0000, BW_32BITS);
	else
		register_write((addr +  offset_val), 0x00020202, BW_32BITS);

	munmap((void *)addr, 0x1000);
	return;
}

static void set_AQC_LEDCTRL(int unit_num, int bForceOn)
{
	char cmd[1024];
	int pci_num = 0;

    if(unit_num>ETH_10GE_PORT1)
    {
        printf("Wrong unit_num:%d\n",unit_num);
        return;
    }
	unit_num -= ETH_10GE_PORT0;

	if( access( "/diag_utils/aqdiag/atltool/atltool", F_OK ) == -1 )
	{
		printf("atltool not exist ,please use correct kernel image\n");
		return;
	}
	
	pci_num = get_AQC_pci_num(unit_num);

	if(pci_num == -1)
		return;

	if(bForceOn)
		sprintf(cmd,"/diag_utils/aqdiag/atltool/atltool -d 0000-%02x:00.0 -wr 0x480 0x100 > /dev/null 2>&1", pci_num);
	else
		sprintf(cmd,"/diag_utils/aqdiag/atltool/atltool -d 0000-%02x:00.0 -wr 0x480 0x0 > /dev/null 2>&1", pci_num);

	system(cmd);
	msleep(100);
	return;
}

static void set_phy_LEDCTRL(int unit_num, int bForceOn)
{
	switch(unit_num)
	{
		case ETH_GE_PORT0:
		case ETH_GE_PORT1:
			set_I211_LEDCTRL(unit_num, bForceOn);
			break;

		case ETH_10GE_PORT0:
		case ETH_10GE_PORT1:
			set_AQC_LEDCTRL(unit_num, bForceOn);
            break;

		case ETH_2P5GE_PORT0:
		case ETH_2P5GE_PORT1:
		case ETH_2P5GE_PORT2:
		case ETH_2P5GE_PORT3:
			AQ_set_LED_On((unit_num-ETH_2P5GE_PORT0),bForceOn);
			break;

		default:
			printf("Not support this item(%d). \n",unit_num);
			break;
	}
	return;
}

/*********************************************************************
 *
 * Function:    show_led_sts
 *
 * Description: Display led status which is reading from FPGA.
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 *********************************************************************
 */
static int show_led_sts(int dummy) 
{
	char *unit_name;
	int color=0 ,blink=0;
	int unit_num;

	for(unit_num=0;unit_num< LED_MAX;unit_num++)
	{
		color = katar_get_led_color(unit_num);
		blink = katar_get_led_blink(unit_num);
		
		unit_name = (char *) &led_name[unit_num];
		printf("- %s LED is ", unit_name);
		switch(color)
		{
			case STAT_LED_OFF:
				printf("off");
				break;
			case STAT_LED_G:
				printf("green");
				break;
			case STAT_LED_A:
				printf("amber");
				break;
			default:
				break;
		}
		{
			printf(" blink ");
			switch(blink)
			{
				case STAT_NO_BLINK:
					printf("off\n");
					break;
				case STAT_SLOW_BLINK:
					printf("slow\n");
					break;
				case STAT_FAST_BLINK:
					printf("fast\n");
					break;
				default:
					printf("\n");
					break;
			}
		}
		
	}
#ifdef ENABLE_POE_MODULE
	for(unit_num=0;unit_num< LED_POE_MAX;unit_num++)
	{
		color = katar_get_poe_led_color(unit_num);
		
		unit_name = (char *) &poe_led_name[unit_num];
		printf("- %s LED is ", unit_name);
		switch(color)
		{
			case STAT_LED_OFF:
				printf("off\n");
				break;
			case STAT_LED_ON:
				printf("on\n");
				break;
			default:
				break;
		}
	}
#endif
    return(PASSED);	
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_unit_color_all_util
 *
 * Description: This function set led on/off/colors
 *
 * Input : unit_num - led unit
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_unit_color_all_util (int unit_num)
{
    uint32_t choice=0;
	int i=0;
    boolean exit_flag = FALSE;
	int set_color;

    while (!exit_flag) {

		printf("  0. Turn All LED off\n");
		printf("  1. Turn All LED to green\n");
		printf("  2. Turn front LED to amber\n");
		printf("  3. Turn front LED to red\n");
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 4, 0, 0xFF);

        switch(choice) {
        case 0:
			for(i=0;i<LED_MAX;i++)
				katar_set_led_ctrl(i , -1 , STAT_LED_OFF);
#ifdef ENABLE_POE_MODULE
			for(i=0;i<LED_POE_MAX;i++)
				katar_set_poe_led_color(i , STAT_LED_OFF);
#else
			katar_set_poe_led_color(LED_POE_SYS , STAT_LED_OFF);
#endif
			for(i=0;i<ETH_PORT_MAX;i++)
				set_phy_LEDCTRL(i , STAT_LED_OFF);
            break;
		case 1:	//Green
			for(i=0;i<LED_MAX;i++)
				katar_set_led_ctrl(i , -1 , STAT_LED_G);
		
			//Need to power on POE_SYS to enable AQR412C poe port led	
			katar_set_poe_led_color(LED_POE_SYS , STAT_LED_ON);
#ifdef ENABLE_POE_MODULE
			katar_set_poe_led_color(LED_POE_P0 , STAT_LED_OFF);
			katar_set_poe_led_color(LED_POE_P1 , STAT_LED_OFF);
#endif
            for(i=0;i<ETH_PORT_MAX;i++)
                set_phy_LEDCTRL(i , STAT_LED_ON);
            break;
		case 2:	//Amber , Alarm color is red
		case 3:	//Red , Alarm color is red
			for(i=0;i<LED_MAX;i++)
			{
				set_color = STAT_LED_OFF;

				if(choice == 2 && i != LED_ALARM)
					set_color = STAT_LED_A;
				else if(choice == 3 && i == LED_ALARM)
					set_color = STAT_LED_A;

				katar_set_led_ctrl(i , -1 , set_color);
			}

			for(i=0;i<ETH_PORT_MAX;i++)
                set_phy_LEDCTRL(i , STAT_LED_OFF);
#ifdef ENABLE_POE_MODULE
			katar_set_poe_led_color(LED_POE_P0 , STAT_LED_ON);
			katar_set_poe_led_color(LED_POE_P1 , STAT_LED_ON);
#else
			katar_set_poe_led_color(LED_POE_SYS , STAT_LED_OFF);
#endif
			break;
        case 4:
            exit_flag = TRUE;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_unit_color_util
 *
 * Description: This function set led on/off/colors
 *
 * Input : unit_num - led unit
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_unit_color_util (int unit_num)
{
    uint32_t choice=0, led_pwr_reg=0;
    char *unit_name;
	int color=0;
    boolean exit_flag = FALSE;

	unit_name = (char *) &led_name[unit_num];

    while (!exit_flag) {

		led_pwr_reg = katar_get_led_ctrl_reg();
		color = katar_get_led_color(unit_num);
		
        printf("\n power %s LED utility:\n", unit_name);
		printf("Current %s LED is ", unit_name);
		switch(color)
		{
			case STAT_LED_OFF:
				printf("off\n");
				break;
			case STAT_LED_G:
				printf("green\n");
				break;
			case STAT_LED_A:
				printf("amber\n");
				break;
			default:
				printf("\n");
				break;
		}
		
        printf("  0. Turn %s LED off\n", unit_name);
        printf("  1. Turn %s LED to green\n", unit_name);
	    printf("  2. Turn %s LED to amber\n", unit_name);
        printf("  3. Show LED control Register\n");
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 4, 0, 0xF);

        switch(choice) {
        case 0:
			katar_set_led_ctrl(unit_num , -1 , STAT_LED_OFF);
            break;
        case 1:
			katar_set_led_ctrl(unit_num , -1 , STAT_LED_G);
            break;
		case 2:
			katar_set_led_ctrl(unit_num , -1 , STAT_LED_A);
			break;
        case 3:
            printf("LED control Register %#.4x\n", led_pwr_reg);
            break;
        case 4:
            exit_flag = TRUE;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

    }
    return(PASSED);

}


/*-----------------------------------------------------------------------------
 *
 * Function: poe_led_unit_color_util
 *
 * Description: This function set led on/off
 *
 * Input : unit_num - led unit
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
poe_led_unit_color_util (int unit_num)
{
    uint32_t choice=0;
    char *unit_name;
	int color=0;
    boolean exit_flag = FALSE;

	unit_name = (char *) &poe_led_name[unit_num];

    while (!exit_flag) {

		color = katar_get_poe_led_color(unit_num);
		
        printf("\n power %s LED utility:\n", unit_name);
		printf("Current %s LED is ", unit_name);
		switch(color)
		{
			case STAT_LED_OFF:
				printf("off\n");
				break;
			case STAT_LED_ON:
				printf("on\n");
				break;
			default:
				printf("\n");
				break;
		}
		
        printf("  0. Turn %s LED off\n", unit_name);
        printf("  1. Turn %s LED on\n", unit_name);
        printf("  2. exit\n");

        choice = gethex_answer("Enter selection:", 2, 0, 2);

        switch(choice) {
        case 0:
			katar_set_poe_led_color(unit_num , STAT_LED_OFF);
            break;
        case 1:
			katar_set_poe_led_color(unit_num , STAT_LED_ON);
            break;
        case 2:
            exit_flag = TRUE;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: ge_phy_led_unit_color_util
 *
 * Description: This function set led on/off
 *
 * Input : unit_num - led unit
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
phy_led_unit_color_util (int unit_num)
{
    uint32_t choice=0;
    char *unit_name;
    boolean exit_flag = FALSE;

	unit_name = (char *) &phy_led_name[unit_num];

    while (!exit_flag) {
		if(katar_get_poe_54V_present()&&(unit_num == ETH_2P5GE_PORT0 || unit_num == ETH_2P5GE_PORT1))
		{
			if(katar_get_poe_led_color(LED_POE_SYS)==STAT_LED_OFF)
				printf("\nLED_POE_SYS is off , %s LED won't work",unit_name);
		}

        printf("\n power %s LED utility:\n", unit_name);
		
        printf("  0. Set %s LED default\n", unit_name);
        printf("  1. Force %s LED on\n", unit_name);
        printf("  2. exit\n");

        choice = gethex_answer("Enter selection:", 2, 0, 2);
		if(choice == 2)
			exit_flag = TRUE;
		else
			set_phy_LEDCTRL(unit_num,choice);
    }
    return(PASSED);
}


/*-----------------------------------------------------------------------------
 *
 * Function: led_unit_blink_util
 *
 * Description: This function set led on/off/colors
 *
 * Input : unit_num - led unit
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_unit_blink_util (int unit_num)
{
    uint32_t choice=0, led_pwr_reg=0;
    char *unit_name;
	int blink=0;
    boolean exit_flag = FALSE;


	if(unit_num > LED_ALARM)
	{
		printf("Not support this item. \n");
		return(PASSED);
	}

	unit_name = (char *) &led_name[unit_num];
	
    while (!exit_flag) {

		led_pwr_reg = katar_get_led_ctrl_reg();
		blink = katar_get_led_blink(unit_num);
		
        printf("\n blink %s LED utility:\n", unit_name);
		printf("Current %s LED blink ", unit_name);
		switch(blink)
		{
			case STAT_NO_BLINK:
				printf("off\n");
				break;
			case STAT_SLOW_BLINK:
				printf("slow\n");
				break;
			case STAT_FAST_BLINK:
				printf("fast\n");
				break;
			default:
				printf("\n");
				break;
		}

		printf("  0. Turn %s LED blink off\n", unit_name);
        printf("  1. Turn %s LED to slow blink\n", unit_name);
        printf("  2. Turn %s LED to fast blink\n", unit_name);
        printf("  3. Show LED control Register\n");
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 4, 0, 4);

        switch(choice) {
        case 0:
			katar_set_led_ctrl(unit_num , STAT_NO_BLINK , -1);
            break;
        case 1:
			katar_set_led_ctrl(unit_num , STAT_SLOW_BLINK , -1);
            break;
        case 2:
			katar_set_led_ctrl(unit_num , STAT_FAST_BLINK , -1);
            break;
        case 3:
            printf("LED control Register %#.4x\n", led_pwr_reg);
            break;
        case 4:
            exit_flag = TRUE;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

    }
    return(PASSED);

}


/*
 *------------------------------------------------------------------
 * $Log: platform_led.c,v $
 * Revision 1.3  2021/06/03 09:57:05  iachang
 * CSCvy54501 : Fixed daily build compiler issue
 *
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.8  2019/04/25 00:32:43  mikech2
 * Change I211 LEDCTL register default value
 *
 * Revision 1.1.2.7  2019/03/21 07:00:40  benlu
 * Modify the default value of select menu
 *
 * Revision 1.1.2.6  2019/03/13 06:47:06  mikech2
 * Add trun led red in System LED color all util
 *
 * Revision 1.1.2.5  2019/03/05 03:33:35  mikech2
 * Use Aquantia tool dump SFP eeprom
 *
 * Revision 1.1.2.4  2019/02/26 01:51:09  mikech2
 * Fix AQC led control issue
 *
 * Revision 1.1.2.3  2019/02/12 08:06:30  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.2  2019/01/29 08:02:05  mikech2
 * remove POE test for katar P2 build
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.6  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.5  2018/11/22 03:25:21  mikech2
 * Add AQC107 phy led control
 *
 * Revision 1.1.2.4  2018/11/15 07:19:04  mikech2
 * Modify light led all function
 *
 * Revision 1.1.2.3  2018/11/14 06:10:52  mikech2
 * Add I211 phy register control
 *
 * Revision 1.1.2.2  2018/11/13 07:50:11  mikech2
 * Fix pcie bus scan issue
 *
 * Revision 1.1.2.1  2018/10/22 08:02:29  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.3  2018/10/22 03:32:46  mikech2
 * Add PHY led control(without AQC100/107)
 *
 * Revision 1.1.2.2  2018/10/02 02:32:15  mikech2
 * Modify FPGA register according SPEC 1.7.1
 *
 * Revision 1.1.2.1  2018/06/20 07:31:37  mikech2
 * Add fan/led/margin control menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
