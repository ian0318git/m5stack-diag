/* $Id: platform_led.c,v 1.2 2019/08/06 06:56:13 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/platform_led.c,v $
 *------------------------------------------------------------------
 * Filename: platform_led.c
 *
 * Description: Platform specific code for controlling the system LED
 *
 * Copyright (c) 2017-2019 by Cisco Systems, Inc.
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

static int led_cf_util(void);
static int change_led_blink_time(void);
static int force_all_led_util(void);
static int led_pwr_unit_util(int); 
static int led_rj45_eth_util(int); 
static int led_rj45_eth_blink_util(int);
static int led_sfp_util(int); 
static int led_sfp_blink_util(int); 
static int led_env_util(void); 
static int led_sys_status_util(void);

/*
 * LED utility
 */
                                                                      
static struct mitem led_items[] = {
    {"Compact Flash LED util",             0, 0, (type_t(*)())led_cf_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Change Ethernet/SFP LED Blink Time",  0,0, (type_t(*)())change_led_blink_time, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Force all LED util",                  0,0, (type_t(*)())force_all_led_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Power supply unit1 LED util",         0,0, (type_t(*)())led_pwr_unit_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"System Status LED util",              0,0, (type_t(*)())led_sys_status_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Environmental LED util",              0,0, (type_t(*)())led_env_util,  /* fan led */
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"RJ-45 eth Port0 LED util",            0,0, (type_t(*)())led_rj45_eth_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"RJ-45 eth Port1 LED util",            0,0, (type_t(*)())led_rj45_eth_util,
                             (type_t *)&one,  0, (type_t(*)())0,       0},
    {"RJ-45 eth Port2 LED util",            0,0, (type_t(*)())led_rj45_eth_util,
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"RJ-45 eth Port3 LED util",            0,0, (type_t(*)())led_rj45_eth_util, 
                            (type_t *)&three, 0, (type_t(*)())is_utah_false,       0},
    {"Blink RJ-45 eth Port0 LED util",      0,0, (type_t(*)())led_rj45_eth_blink_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Blink RJ-45 eth Port1 LED util",      0,0, (type_t(*)())led_rj45_eth_blink_util, 
                             (type_t *)&one,  0, (type_t(*)())0,       0},
    {"Blink RJ-45 eth Port2 LED util",      0,0, (type_t(*)())led_rj45_eth_blink_util,
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"Blink RJ-45 eth Port3 LED util",      0,0, (type_t(*)())led_rj45_eth_blink_util, 
                            (type_t *)&three, 0, (type_t(*)())is_utah_false,       0},
    {"SFP0 LED util",                       0,0, (type_t(*)())led_sfp_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"SFP1 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                             (type_t *)&one,  0, (type_t(*)())0,       0},
    {"Blink SFP0 LED util",                 0,0, (type_t(*)())led_sfp_blink_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Blink SFP1 LED util",                 0,0, (type_t(*)())led_sfp_blink_util, 
                             (type_t *)&one,  0, (type_t(*)())0,       0},
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
 * Function: led_cf_util
 *
 * Description: This function exercises the compact flash LED 
 *              function of the	LED Compact Flash On/Off Register   
 *              in the register of FPGA.
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_cf_util (void) 
{
    uint32_t choice = 0, choice_led = 0;
    uint32_t led_cf_reg, field_mask, act_mask, act_timer;
    char *cf_name;
    boolean exit_flag = FALSE;

    led_cf_reg = field_mask = act_mask = act_timer = 0;

    cf_name = "Compact Flash";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", cf_name);
	printf("  0. Show %s green LED reg\n", cf_name);
	printf("  1. Toggle %s to yellow\n", cf_name);
	printf("  2. Set signal polarity inversion\n");
	printf("  3. Show activity signal status\n");
        printf("  4. Set green LED function\n");
        printf("  5. Set activity timer\n");
        printf("  6. Show LED compact flash On/Off register\n");
	printf("  7. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 7);

        led_cf_reg = get_led_status(LED_CTRL_CF);
   
	switch(choice) {
	case 0: /* read only */
	  /* Note that this bit is a direct reflection 
	   * of the bits in the Flash LED Control Register (0x20). */
	  field_mask = CF_LED_GREEN;
	  act_mask = (field_mask & led_cf_reg) >> OFFSET_CF_LED_REG;

	  printf("%s green LED reg = %#.4x\n", cf_name, act_mask);
	  continue;
        break;

	case 1:
          field_mask = CF_LED_YELLOW;
          act_mask = (led_cf_reg & CF_LED_YELLOW) ? ~CF_LED_YELLOW : CF_LED_YELLOW;
	break;

	case 2:
	  printf("  0. The activity signal polarity is active low\n");
	  printf("  1. The polarity of the activity signal is active high\n");
          choice_led = gethex_answer("Enter selection:", 0, 0, 1);
	  field_mask = CF_LED_POLARITY_INVER;
          act_mask = (choice_led == 0) ? ~CF_LED_POLARITY_INVER : CF_LED_POLARITY_INVER;
	break;

	case 3:
	  field_mask = CF_LED_SIG_STATUS;
	  act_mask = ((field_mask & led_cf_reg) >> OFFSET_CF_SIG_STATUS_REG);
          printf("%s activity signal is %s \n", cf_name,
             (act_mask == 0) ? "low" : "high");
	break;

	case 4:
          printf("  0. Green LED is driven directly from Set %s green LED\n" ,cf_name);
	  printf("  1. Green LED is ON for external activity signal\n");
	  printf("  2. Green LED is OFF/ON for external activity signal\n");;    
	  printf("  3. Green LED is ON then OFF for external activity signal\n");
	  printf("  Note: Option 2 and 3 should turn on set");
	  printf(" Compact Flash Green LED,\n");
	  printf("  and the time is determined by the activity timer.\n");

	  choice_led = gethex_answer("Enter selection:", 0, 0, 3);

          field_mask = CF_LED_GREEN_FUNC;
          act_mask = (choice_led << CF_LED_GREEN_OFFSET);

	break;

	case 5:
          act_timer = (led_cf_reg & CF_LED_TIMER);
          field_mask = CF_LED_TIMER;
          act_timer = gethex_answer("Enter activity timer:", act_timer, 0, 0x3FF);
	  act_mask = act_timer;
	break;   

	case 6:   	    
	    printf("LED Compact Flash On/Off Register %#.8x\n", led_cf_reg);
	    continue;
	break;   
	case 7:
	    exit_flag = TRUE;
	break;
	default:
            printf("Not support this item. \n");
	break;
	}
 
        led_cf_reg = (led_cf_reg & ~field_mask) | act_mask;
        set_led_reg(LED_CTRL_CF, led_cf_reg);
    }
    return(PASSED);
}


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
static int 
change_led_blink_time (void)
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
static int 
force_all_led_util (void) 
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
static int 
led_pwr_unit_util (int unit_num)
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
static int 
led_rj45_eth_util (int port_num)
{
    uint32_t choice, led_rj45_eth_reg, g_link_mask, g_speed_mask;
    uint32_t act_mask;

    g_link_mask = g_speed_mask = act_mask = 0;
    while (1) {
        printf("\n LED RJ-45 Ethernet port %d utility:\n", port_num);
	printf("  0. Toggle port %d green speed LED\n", port_num);
	printf("  1. Toggle port %d green link LED\n", port_num);
        printf("  2. Show LED RJ-45 Ethernet On/Off Register\n");
	printf("  3. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 4);
	led_rj45_eth_reg = get_led_status(LED_CTRL_RJ45_ONOFF);
        
        if (choice == 3) {
            return PASSED;
        }
        
        if (choice == 2) {
            printf("LED_CTRL_RJ45_ONOFF is %#x\n", led_rj45_eth_reg);
            continue;
        }

        switch (port_num) {
        case 0:
            g_link_mask = RJ45_ETH_PORT0_LED_GREEN;
            g_speed_mask = RJ45_ETH_PORT0_LED_BLINK;
            break;
        case 1:
            g_link_mask = RJ45_ETH_PORT1_LED_GREEN;
            g_speed_mask =  RJ45_ETH_PORT1_LED_BLINK;
            break;
        case 2:
            g_link_mask = RJ45_ETH_PORT2_LED_GREEN;
            g_speed_mask = RJ45_ETH_PORT2_LED_BLINK;
            break;
        case 3:
            g_link_mask = RJ45_ETH_PORT3_LED_GREEN;
            g_speed_mask = RJ45_ETH_PORT3_LED_BLINK;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }
        
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
 * Input : sfp_num - SFP number 0, 1
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_sfp_util (int sfp_num)
{
    uint32_t choice, led_sfp_reg, g_link_mask, y_link_mask, g_speed_mask;
    uint32_t act_mask;

    led_sfp_reg = g_link_mask = y_link_mask = g_speed_mask = act_mask = 0;
    while (1) {
        printf("\n LED SFP port %d utility:\n", sfp_num);
        printf("  0. Toggle port %d green speed LED\n", sfp_num);
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
            g_speed_mask = SFP0_LED_BLINK;
            break;
        case 1:
            g_link_mask = SFP1_LED_GREEN;
            y_link_mask = SFP1_LED_YELLOW;
            g_speed_mask = SFP1_LED_BLINK;
            break;
        default:
            printf("Not support this item. \n");
        break;
        }

        switch (choice) {
        case 0:
            act_mask = g_speed_mask;
            break;
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
static int 
led_rj45_eth_blink_util (int port_num) 
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
 * Function: led_sfp_blink_util
 *
 * Description: This function test the system LED blinking feature, for the  
 *              SFP ports 0, 1, 2, 3 
 *
 * Input : SPF port 0, 1, 2, 3
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_sfp_blink_util (int sfp_num) 
{
    uint32_t choice, blk_sfp_reg;

    while (1) {
        printf("\nSFP port %d LED utility:\n", sfp_num);
        printf("  0. Turn off port %d blink\n", sfp_num);
        printf("  1. Turn port %d one blink\n", sfp_num);
        printf("  2. Turn port %d two blinks\n", sfp_num);
        printf("  3. Turn port %d three blinks\n", sfp_num);
        printf("  4. Show led SFP register\n");
        printf("  5. exit\n");

        choice = gethex_answer("Enter selection:", 0, 0, 5);

        if (choice == 5)
            return(PASSED);

	blk_sfp_reg = get_led_status(LED_CTRL_RJ45_BLINK_EN);

        if (choice == 4) {
            printf("LED SFP blink register %#.4x\n", blk_sfp_reg);
            /* In LED_CTRL_RJ45_BLINK_EN[15:8] is SFP register*/
            continue;
        }

        /* turn off blinking of the chosen port, start from bit 8 */
        blk_sfp_reg &= ~( 0x300 << (sfp_num * 2));
        set_led_reg(LED_CTRL_RJ45_BLINK_EN, blk_sfp_reg);

        /* turn on blinking of the chosen port */
        if (choice) {
            blk_sfp_reg |= ( (choice << 8) << (sfp_num * 2));
            set_led_reg(LED_CTRL_RJ45_BLINK_EN, blk_sfp_reg);
        }


    }
    return(PASSED);

}

/*-----------------------------------------------------------------------------
 *
 * Function: led_env_util
 *
 * Description: This function to control ENV LEDs by accessing 
 *              the LED Environmental Register of FPGA.
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_env_util (void) 
{
    uint32_t choice = 0;
    uint32_t led_env_reg = 0, field_mask, act_mask;
    uint32_t time_interval = 0;
    char *env_name;
    boolean exit_flag = FALSE;
    uint32_t led_group_mask = (FAN_GREEN_LED | FAN_YELLOW_LED |
			       FAN_RED_LED | FAN_YELLOW_LED_BLINK);
    
    env_name = "Enviromental";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", env_name);
	printf("  0. Toogle FAN LED to Green\n");
	printf("  1. Toogle FAN LED to Yellow\n");
	printf("  2. Toogle FAN LED to Red\n");
	printf("  3. Toogle FAN LED to Yellow Blinking\n");
	printf("  4. Set FAN LED Blinking Timer\n");
	printf("  5. Show LED %s Register\n", env_name);
	printf("  6. Exit\n");

	choice = getdec_answer("Enter selection:", 6, 0, 6);

        led_env_reg = get_led_status(LED_CTRL_ENV);
	field_mask = 0;
	act_mask = 0;

	switch(choice) {
	case 0:
          field_mask = led_group_mask;
          if (led_env_reg & FAN_GREEN_LED) {
              act_mask = (led_env_reg & ~FAN_GREEN_LED);
          } else {
              act_mask = FAN_GREEN_LED;
          }
	break;
	case 1:
          field_mask = led_group_mask;
          if (led_env_reg & FAN_YELLOW_LED) {
              act_mask = (led_env_reg & ~FAN_YELLOW_LED);
          } else {
              act_mask = FAN_YELLOW_LED;
          }
	break;
	case 2:
          field_mask = led_group_mask;
          if (led_env_reg & FAN_RED_LED) {
              act_mask = (led_env_reg & ~FAN_RED_LED);
          } else {
              act_mask = FAN_RED_LED;
          }
	break;
	case 3:
	  /* When FAN_YELLOW_LED_BLINK and FAN_YELLOW_LED both are
	   * 1, yellow LED blinks.
	   */
          field_mask = led_group_mask;
          if (led_env_reg & FAN_YELLOW_LED_BLINK) {
	    act_mask = (led_env_reg & ~(FAN_YELLOW_LED_BLINK | FAN_YELLOW_LED));
          } else {
              act_mask = FAN_YELLOW_LED_BLINK | FAN_YELLOW_LED;
          }
	break;
        case 4:
          time_interval = 0;
          time_interval = getdec_answer("Enter the FAN LED Blinking timer(ms):",
                                        100, 0, 1023);
          field_mask = FAN_LED_BLINK_TIME_MSK;
          act_mask = time_interval;
        break;
        case 5:
          printf("\nLED %s Register = 0x%08X.\n", env_name, led_env_reg);
        break;
	case 6:
          exit_flag = TRUE;
	break;
	default:
          printf("Not support this item.\n");
	break;
	}

        if ((field_mask != 0) || (act_mask != 0)) {
            led_env_reg = (led_env_reg & ~field_mask) | act_mask;
            set_led_reg(LED_CTRL_ENV, led_env_reg);
        }
    }

    return (PASSED);
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

    printf("Exit. Restore original value \n"); 
    set_cpld_sys_status_led_ctrl_reg(ori_val);

    return (PASSED);
}


/* -------------------- End of file ------------------- */

/******** History ******** 
$Log: platform_led.c,v $
Revision 1.2  2019/08/06 06:56:13  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.4  2019/03/20 21:15:04  ptong
Completed Curie 1RU LED check util

Revision 1.1.2.3  2019/03/19 00:28:41  alpeng
commit led change;

Revision 1.1.2.2  2018/11/02 09:01:56  alpeng
remove led field which is not available on Curie

Revision 1.1.2.1  2018/06/22 08:05:18  alpeng
move curie diag to neptune/curie_1RU directory

Revision 1.1.2.1  2018/05/30 02:39:37  alpeng
porting neptune x86 to curie

Revision 1.2  2018/05/18 09:25:00  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.1.2.8  2018/01/10 23:17:59  ptong
Dress up LED util menu

Revision 1.1.2.7  2017/05/08 01:35:25  meho
Added SFP+ led blinking utility.

Revision 1.1.2.6  2017/02/16 08:31:33  leschen
Keep not supported item in menu to let user know they are not supported.

Revision 1.1.2.5  2017/02/15 02:36:36  leschen
Remove TE port green LED activity and support correct selection number.

Revision 1.1.2.4  2017/02/15 01:04:36  ptong
Swap the command selection

Revision 1.1.2.3  2017/02/13 02:49:28  leschen
Modify LED utility for Neptune

Revision 1.1.2.2  2016/10/20 17:15:11  leschen
Fix Neptune HDD LED control.

Revision 1.1.2.1  2016/08/22 06:54:21  leschen
Support Neptune LED utility.

*
*/
