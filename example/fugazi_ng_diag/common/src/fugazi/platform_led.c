/* $Id: platform_led.c,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_led.c,v $
 *------------------------------------------------------------------
 * Filename: platform_led.c
 *
 * Description: Platform specific code for controlling the system LED
 *
 * Copyright (c) 2017-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

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
#include "platform_led.h"
#include "dash_fpga.h"

static int change_led_blink_time(void);
static int force_all_led_util(void);
static int led_pwr_unit_util(int); 
static int led_rj45_eth_util(int); 
static int led_rj45_eth_blink_util(int);
static int led_sfp_util(int); 
static int led_sfp_plus_util(int); 
static int led_sys_status_util(void);
static int led_alarm_util(void);


/*
 * LED utility
 */
                                                                      
static struct mitem led_items[] = {
    {"Change RJ-45 eth LED Blink Time",     0,0, (type_t(*)())change_led_blink_time, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Force all LED util",                  0,0, (type_t(*)())force_all_led_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Power supply unit1 LED util",         0,0, (type_t(*)())led_pwr_unit_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"RJ-45 eth Port0 LED util",            0,0, (type_t(*)())led_rj45_eth_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Blink RJ-45 eth Port0 LED util",      0,0, (type_t(*)())led_rj45_eth_blink_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"SFP0 LED util",                       0,0, (type_t(*)())led_sfp_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"SFP1 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                             (type_t *)&one,  0, (type_t(*)())0,       0},
    {"SFP2 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"SFP3 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                            (type_t *)&three, 0, (type_t(*)())0,       0},
    {"SFP4 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                            (type_t *)&four,  0, (type_t(*)())0,       0},
    {"SFP5 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                            (type_t *)&five,  0, (type_t(*)())0,       0},
    {"SFP6 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                            (type_t *)&six,   0, (type_t(*)())0,       0},
    {"SFP7 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                            (type_t *)&seven, 0, (type_t(*)())0,       0},
    {"SFP0+ LED util",                      0,0, (type_t(*)())led_sfp_plus_util,
                            (type_t *)&zero,  0, (type_t(*)())0,       0},
    {"SFP1+ LED util",                      0,0, (type_t(*)())led_sfp_plus_util, 
                             (type_t *)&one,  0, (type_t(*)())0,       0},
    {"SFP2+ LED util",                      0,0, (type_t(*)())led_sfp_plus_util, 
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"SFP3+ LED util",                      0,0, (type_t(*)())led_sfp_plus_util, 
                            (type_t *)&three, 0, (type_t(*)())0,       0},
    {"System Status LED util",              0,0, (type_t(*)())led_sys_status_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Alarm LED util",                      0,0, (type_t(*)())led_alarm_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
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


/*-----------------------------------------------------------------------------
 *
 * Function: change_led_blink_time
 *
 * Description: This function changes the LED config register to
 *              adjust the led blink time
 *
 * Input : none
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int change_led_blink_time (void)
{
    char msgbuf[64];
    uint32_t led_cfg_reg;
    uint8_t ontime, offtime, pausetime;

    led_cfg_reg = get_led_status(LED_CTRL_BLINK_DURA);
    
    ontime = ((led_cfg_reg & MSK_LED_ON_TIME_REG) >>
	      OFFSET_ETH_LED_CFG_ON);
    offtime = ((led_cfg_reg & MSK_LED_OFF_TIME_REG) >>
	      OFFSET_ETH_LED_CFG_OFF);
    pausetime = ((led_cfg_reg & MSK_LED_PAUSE_TIME_REG) >>
	      OFFSET_ETH_LED_CFG_PAUSE);

    printf("\n(Note: LED time params are in 1/8 second increment)\n");
    sprintf(msgbuf, "Enter LED on time (1 to 15), current value");
    ontime = getdec_answer(msgbuf, ontime, 1, 15);
    sprintf(msgbuf, "Enter LED off time (1 to 15), current value");
    offtime = getdec_answer(msgbuf, offtime, 1, 15);
    sprintf(msgbuf, "Enter LED pause time (1 to 15), current value");
    pausetime = getdec_answer(msgbuf, pausetime, 1, 15);

    led_cfg_reg = ((ontime << OFFSET_ETH_LED_CFG_ON) |
		   (offtime << OFFSET_ETH_LED_CFG_OFF) |
		   (pausetime << OFFSET_ETH_LED_CFG_PAUSE));

    
    set_led_reg(LED_CTRL_BLINK_DURA, led_cfg_reg);
    
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 * 
 * Function: force_all_led_util 
 * 
 * Description: This function exercises the force all LED function of the 
 *              LED Debug Register in the register of FPGA 
 * 
 * Input : none 
 * 
 * Output: Always PASSED (even though there is nothing to be returned) 
 *         to be called from submenu. 
 *-----------------------------------------------------------------------------
 */ 
static int force_all_led_util (void) 
{ 
    uint32_t choice, sys_led_ctrl; 
    boolean exit_flag = FALSE;

    while(!exit_flag) {
        printf("\nForce all LED utility:\n");
	printf("  0. Don't force\n");
	printf("  1. Force to green color\n");
	printf("  2. Force to yellow color\n");
	printf("  3. Cycle all LED color\n");
	printf("  4. Show LED register\n");
	printf("  5. exit\n");
  
	choice = gethex_answer("Enter selection:", 0, 0, 5);

	sys_led_ctrl = get_led_status(LED_CTRL_DEBUG);
	switch(choice) {
	case 0:
	    sys_led_ctrl = (sys_led_ctrl & ~MSK_FORCE_ALL_LED);
	    break;
	case 1:
	    sys_led_ctrl = ((sys_led_ctrl & ~MSK_FORCE_ALL_LED )|
			    FORCE_ALL_LED_GREEN);
	    break;
	case 2:
	    sys_led_ctrl = ((sys_led_ctrl & ~MSK_FORCE_ALL_LED) |
			    FORCE_ALL_LED_YELLOW);
	    break;
	case 3:
	    sys_led_ctrl = ((sys_led_ctrl & ~MSK_FORCE_ALL_LED) |
			    FORCE_ALL_LED_CYCLE);
	    break;
	case 4:
	    printf("LED Debug Register %#.4x\n", sys_led_ctrl);
	    continue;
	    break;
	case 5:
	    exit_flag = TRUE;
	    break;
	default:
	    break;
	}
	set_led_reg(LED_CTRL_DEBUG, sys_led_ctrl);
    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_pwr_unit_util
 *
 * Description: This function exercises the power supply unit1 and unit2
 *              LED function of the LED Power Supply On/Off Register 
 *              in the register of FPGA
 *
 * Input : unit_num - power supply unit number 1 or 2
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int led_pwr_unit_util (int unit_num)
{
    uint32_t choice, led_pwr_reg, field_mask, act_mask;
    char *unit_name;
    boolean exit_flag = FALSE;

    choice = led_pwr_reg = field_mask = act_mask = 0;

    unit_name = (unit_num == 0) ? "unit1" : "unit2";

    while (!exit_flag) {
        printf("\n power supply %s LED utility:\n", unit_name);
	printf("  0. Turn %s LED off\n", unit_name);
	printf("  1. Turn %s to green\n", unit_name);
	printf("  2. Turn %s to yellow\n", unit_name);
	printf("  3. Show LED Power Supply On/Off Register\n");
	printf("  4. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 4);

	led_pwr_reg = get_led_status(LED_CTRL_PWR);
	
	field_mask = (unit_num == 0) ? MSK_PWR_SUPPLY_UNIT1_LED : MSK_PWR_SUPPLY_UNIT2_LED;

	switch(choice) {
	case 0:
	    act_mask = (unit_num == 0) ? PWR_SUPPLY_UNIT1_LED_OFF : PWR_SUPPLY_UNIT2_LED_OFF;
	    break;
	case 1:
	    act_mask = (unit_num == 0) ? PWR_SUPPLY_UNIT1_LED_GREEN : PWR_SUPPLY_UNIT2_LED_GREEN;
	    break;
	case 2:
	    act_mask = (unit_num == 0) ? PWR_SUPPLY_UNIT1_LED_YELLOW : PWR_SUPPLY_UNIT2_LED_YELLOW;
	    break;
	case 3:
	    printf("LED Power Supply On/Off Register %#.4x\n", led_pwr_reg);
	    continue;
	    break;
	case 4:
	    exit_flag = TRUE;
	    break;
	default:
            printf("Not support this item. \n");
        break;
	}

	led_pwr_reg = (led_pwr_reg & ~field_mask) | act_mask;
	set_led_reg(LED_CTRL_PWR, led_pwr_reg);
    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_rj45_eth_util
 *
 * Description: This function exercises the RJ-45 Ethernet LED function of the
 *              LED RJ-45 ethernet on/off register in the register of FPGA
 *              If the blink register is not turned on, it will just
 *              trun on LED without blink.
 *
 * Input : port_num - RJ-45 port number 0, 1, 2, 3
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int led_rj45_eth_util (int port_num)
{
    uint32_t choice, led_rj45_eth_reg, g_link_mask, g_speed_mask;
    uint32_t act_mask;

    g_link_mask =  g_speed_mask = act_mask = 0;
    while (1) {
        printf("\n LED RJ-45 Ethernet port %d utility:\n", port_num);
	    printf("  0. Toggle port %d green speed LED\n", port_num);
	    printf("  1. Toggle port %d green link LED\n", port_num);
        printf("  2. Show LED RJ-45 Ethernet On/Off Register\n");
	    printf("  3. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 3);
	led_rj45_eth_reg = get_led_status(LED_CTRL_RJ45_ONOFF);
        
        if (choice == 3) {
            return PASSED;
        }

        if (choice == 2) {
            printf("LED_CTRL_RJ45_ONOFF is %#x\n", led_rj45_eth_reg);
            continue;
        }

        g_link_mask = RJ45_ETH_PORT0_LED_GREEN;
        g_speed_mask = RJ45_ETH_PORT0_LED_BLINK;
        
        switch (choice) {
        case 0:
            act_mask = g_speed_mask;
            break;
        case 1:
            act_mask = g_link_mask;
            break;
        }

        if (led_rj45_eth_reg & act_mask) {
            led_rj45_eth_reg &= ~act_mask; 
        } else {
            led_rj45_eth_reg |= act_mask;
        }
        set_led_reg(LED_CTRL_RJ45_ONOFF, led_rj45_eth_reg);
        
    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_sfp_util
 *
 * Description: This function exercises the SFP LED function of the
 *              LED SFP on/off register in the register of FPGA
 *              If the blink register is not turned on, it will just
 *              trun on LED without blink.
 *
 * Input : sfp_num - SFP number 0, 1, 2, 3, 4, 5, 6, 7
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int led_sfp_util (int sfp_num)
{
    uint32_t choice, led_sfp_reg, g_link_mask, y_link_mask;
    uint32_t act_mask;

    led_sfp_reg = g_link_mask = y_link_mask = act_mask = 0;
    while (1) {
        printf("\n LED SFP port %d utility:\n", sfp_num);
        printf("  1. Toggle port %d green link LED\n", sfp_num);
        printf("  2. Toggle port %d ambert LED\n", sfp_num);
        printf("  3. Show LED RJ-45 Ethernet On/Off Register\n");
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 0, 0, 4);

        if (choice == 4)
            return PASSED;

        led_sfp_reg = get_led_status(LED_CTRL_SFP_ONOFF);

        if (choice == 3) {
            printf("LED_CTRL_SFP_ONOFF is %#x\n", led_sfp_reg);
            continue;
        }

        switch (sfp_num) {
        case 0:
            g_link_mask = SFP0_LED_GREEN;
            y_link_mask = SFP0_LED_YELLOW;
            break;
        case 1:
            g_link_mask = SFP1_LED_GREEN;
            y_link_mask = SFP1_LED_YELLOW;
            break;
        case 2:
            g_link_mask = SFP2_LED_GREEN;
            y_link_mask = SFP2_LED_YELLOW;
            break;
        case 3:
            g_link_mask = SFP3_LED_GREEN;
            y_link_mask = SFP3_LED_YELLOW;
            break;
        case 4:
            g_link_mask = SFP4_LED_GREEN;
            y_link_mask = SFP4_LED_YELLOW;
            break;
        case 5:
            g_link_mask = SFP5_LED_GREEN;
            y_link_mask = SFP5_LED_YELLOW;
            break;
        case 6:
            g_link_mask = SFP6_LED_GREEN;
            y_link_mask = SFP6_LED_YELLOW;
            break;
        case 7:
            g_link_mask = SFP7_LED_GREEN;
            y_link_mask = SFP7_LED_YELLOW;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

        switch (choice) {
        case 1:
            act_mask = g_link_mask;
            break;
        case 2:
            act_mask = y_link_mask;
            break;
        }

        if (led_sfp_reg & act_mask) {
            led_sfp_reg &= ~act_mask;
        } else {
            led_sfp_reg |= act_mask;
        }
        set_led_reg(LED_CTRL_SFP_ONOFF, led_sfp_reg);

    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_rj45_eth_blink_util
 *
 * Description: This function test the system LED blinking feature, for the  
 *              RJ-45 ethernet ports 0, 1, 2, 3 
 *
 * Input : RJ-45 ethernet port 0, 1, 2, 3
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int led_rj45_eth_blink_util (int port_num) 
{
    uint32_t choice, blk_rj45_eth_reg = 0;

    while (1) {
        printf("\nPort %d LED utility:\n", port_num);
	printf("  0. Turn off port %d blink\n", port_num);
	printf("  1. Turn port %d one blink\n", port_num);
	printf("  2. Turn port %d two blinks\n", port_num);
	printf("  3. Turn port %d three blinks\n", port_num);
        printf("  4. Show LED RJ-45 ethernet blink register\n");
	printf("  5. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 5);

        if (choice == 5)
	    return(PASSED);

	if (choice == 4) {
	    printf("LED RJ-45 ethernet blink register %#.4x\n", blk_rj45_eth_reg);
	   /* In LED_RJ45_SFP_BLINK_REG_PTR[7:0] is RJ-45 Eth register*/
            continue;
        }

	blk_rj45_eth_reg = get_led_status(LED_CTRL_RJ45_BLINK_EN);

        /* turn off blinking of the chosen port */
        blk_rj45_eth_reg &= ~( 0x3 << (port_num * 2)) ;
        set_led_reg(LED_CTRL_RJ45_BLINK_EN, blk_rj45_eth_reg);

        /* turn on blinking of the chosen port */
        if (choice) {
            blk_rj45_eth_reg |= ( choice << (port_num * 2)) ;
            set_led_reg(LED_CTRL_RJ45_BLINK_EN, blk_rj45_eth_reg);
        }

    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_sfp_plus_util
 *
 * Description: This function exercises the SFP+ LED function of the
 *              LED SFP+ on/off register in the register of FPGA
 *              If the blink register is not turned on, it will just
 *              trun on LED without blink.
 *
 * Input : sfp_num - SFP number 0, 1, 2, 3
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int led_sfp_plus_util (int sfp_num)
{
    uint32_t choice, led_sfp_reg, g_link_mask, y_link_mask;
    uint32_t act_mask;

    led_sfp_reg = g_link_mask = y_link_mask = act_mask = 0;
    while (1) {
        printf("\n LED SFP+ port %d utility:\n", sfp_num);
        printf("  1. Toggle port %d green link LED\n", sfp_num);
        printf("  2. Toggle port %d yellow LED\n", sfp_num);
        printf("  3. Show LED SFP+ On/Off Register\n");
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 0, 0, 5);

        if (choice == 4)
            return PASSED;

        led_sfp_reg = get_led_status(LED_CTRL_SFP_PLUS_ONOFF);

        if (choice == 3) {
            printf("LED_CTRL_SFP_PLUS_ONOFF is %#x\n", led_sfp_reg);
            continue;
        }

        switch (sfp_num) {
        case 0:
            g_link_mask = SFP0_PLUS_LED_GREEN;
            y_link_mask = SFP0_PLUS_LED_YELLOW;
            break;
        case 1:
            g_link_mask = SFP1_PLUS_LED_GREEN;
            y_link_mask = SFP1_PLUS_LED_YELLOW;
            break;
        case 2:
            g_link_mask = SFP2_PLUS_LED_GREEN;
            y_link_mask = SFP2_PLUS_LED_YELLOW;
            break;
        case 3:
            g_link_mask = SFP3_PLUS_LED_GREEN;
            y_link_mask = SFP3_PLUS_LED_YELLOW;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

        switch (choice) {
        case 1:
            act_mask = g_link_mask;
            break;
        case 2:
            act_mask = y_link_mask;
            break;
        }

        if (led_sfp_reg & act_mask) {
            led_sfp_reg &= ~act_mask;
        } else {
            led_sfp_reg |= act_mask;
        }
        set_led_reg(LED_CTRL_SFP_PLUS_ONOFF, led_sfp_reg);

    }
    return(PASSED);
}

/*------------------------------------------------------------
 *
 * Function: led_sys_status_util
 *
 * Description: This function to control System status  LEDs by accessing 
 *              the LPC Status LED Control Register of FPGA
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *------------------------------------------------------------
 */
static int 
led_sys_status_util (void)
{
    uint32_t choice = 0;
    uint32_t led_sys_reg = 0, ori_val = 0; 
    char sys_name[32];
    boolean exit_flag = FALSE;

    /* back up val */
    ori_val = get_cpld_sys_status_led_ctrl_reg(); 

    sprintf(sys_name, "%s", "System status"); 
    while (!exit_flag) {
        printf("\n%s LED utility:\n", sys_name);
	printf("  0. Set System status LED to Green\n");
	printf("  1. Set System status LED to Yellow\n");
	printf("  2. Set System status LED to Red\n");
	printf("  3. Set System status LED to Off\n");
	printf("  4. Show LED %s Register\n", sys_name);
	printf("  5. Exit\n");

	choice = getdec_answer("Enter selection:", 5, 0, 5);

        led_sys_reg = get_cpld_sys_status_led_ctrl_reg(); 
   
	switch(choice) {
	case 0:
          led_sys_reg = CPLD_SYS_STATUS_LED_GREEN; 
	break;
	case 1:
          led_sys_reg = CPLD_SYS_STATUS_LED_YELLOW; 
	break;
	case 2:
          led_sys_reg = CPLD_SYS_STATUS_LED_RED;    
	break;
	case 3:
          led_sys_reg = CPLD_SYS_STATUS_LED_OFF; 
	break;
        case 4:
          printf("\nCPLD LED %s Register = 0x%08X.\n", 
                 sys_name, led_sys_reg);
        break;
	case 5:
          exit_flag = TRUE;
	break;
	default:
          printf("Not support this item.\n");
	break;
	}

        set_cpld_sys_status_led_ctrl_reg(led_sys_reg); 
    }

    /* MFG request to keep last LED state when exit.
    printf("Exit. Restore original value \n"); 
    set_cpld_sys_status_led_ctrl_reg(ori_val);
    */
    return (PASSED);
}
/*------------------------------------------------------------
 *
 * Function:  led_alarm_util
 *
 * Description: This function to control Alarm  LEDs by accessing 
 *              the LPC Alarm Management Control Register of FPGA
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *------------------------------------------------------------
 */
static int 
led_alarm_util (void)
{
    uint32_t choice = 0;
    uint32_t led_alarm_reg = 0, ori_val = 0; 
    char alarm_name[32];
    boolean exit_flag = FALSE;

    /* back up val */
    ori_val = get_cpld_alarm_led_ctrl_reg(); 

    sprintf(alarm_name, "%s", "Alarm"); 
    while (!exit_flag) {
        printf("\n%s LED utility:\n", alarm_name);
        printf("  0. Set Minor Alarm LED On\n");
        printf("  1. Set Minor Alarm LED Off\n");
        printf("  2. Set Major Alarm LED On\n");
        printf("  3. Set Major Alarm LED Off\n");
        printf("  4. Set Critical Alarm LED On\n");
        printf("  5. Set Critical Alarm LED Off\n");
        printf("  6. Show LED %s Register\n", alarm_name);
        printf("  7. Exit\n");

        choice = getdec_answer("Enter selection:", 7, 0, 7);

        led_alarm_reg = get_cpld_alarm_led_ctrl_reg(); 
       
        switch(choice) {
        case 0:
              led_alarm_reg |= CPLD_MINOR_ALARM_LED;
        break;
        case 1:
              led_alarm_reg &= ~CPLD_MINOR_ALARM_LED; 
        break;
        case 2:
              led_alarm_reg |= CPLD_MAJOR_ALARM_LED;    
        break;
        case 3:
              led_alarm_reg &= ~CPLD_MAJOR_ALARM_LED; 
        break;
        case 4:
              led_alarm_reg |= CPLD_CRITICAL_ALARM_LED;    
        break;
        case 5:
              led_alarm_reg &= ~CPLD_CRITICAL_ALARM_LED; 
        break;
        case 6:
              printf("\nCPLD LED %s Register = 0x%08X.\n", 
                     alarm_name, led_alarm_reg);
            break;
        case 7:
              exit_flag = TRUE;
        break;
        default:
              printf("Not support this item.\n");
        break;
        }

        set_cpld_alarm_led_ctrl_reg(led_alarm_reg); 
    }

    /* MFG request to keep last LED state when exit. 
    printf("Exit. Restore original value \n"); 
    set_cpld_alarm_led_ctrl_reg(ori_val);
    */
    return (PASSED);
}
/*-------------------------------------------------
 * $Log: platform_led.c,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.3  2020/09/01 09:00:30  iachang
 * MFG request to keep last LED state when exit . (“System status” and “Alarm” LEDs )
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.5  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.4  2020/06/03 11:17:35  iachang
 * Add support Alarm and System LED utility
 *
 * Revision 1.1.6.3  2019/07/19 07:35:30  letsai
 * 1. Support LED control.
 * 2. Support smart fan.
 * 3. Change BCM 54194 phy reset bit.
 *
 * Revision 1.1.6.2  2019/03/14 03:48:37  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
