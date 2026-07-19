/* $Id: platform_led.c,v 1.4 2013/11/26 08:40:36 hroni Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_led.c,v $
 *------------------------------------------------------------------
 * Filename: platform_led.c
 *
 * Description: Platform specific code for controlling the system LED
 *
 * Copyright (c) 2013 by Cisco Systems, Inc.
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
static int led_voice_mdl_util(void);
static int led_pwr_unit_util(int unit_num);
static int led_poe_pwr_unit_util(int unit_num);
static int led_poe_dag_crd_util(void);
static int led_rj45_eth_util(int port_num);
static int led_rj45_eth_blink_util(int port_num);
static int led_sfp_util(int sfp_num);
static int led_sfp_blink_util(int sfp_num);
static int led_eth_util(void);
static int led_eth_blink_util(void);
static int led_hdd_util(void);
static int led_env_util(void); 

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
    {"Voice module LED util",               0,0, (type_t(*)())led_voice_mdl_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Power supply unit1 LED util",         0,0, (type_t(*)())led_pwr_unit_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Power supply unit2 LED util",         0,0, (type_t(*)())led_pwr_unit_util,
                             (type_t *)&one,  0, (type_t(*)())is_utah_false,       0},
    {"POE power supply boost LED util",     0,0, (type_t(*)())led_poe_pwr_unit_util, 
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"POE power supply unit1 LED util",     0,0, (type_t(*)())led_poe_pwr_unit_util,
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"POE power supply unit2 LED util",     0,0, (type_t(*)())led_poe_pwr_unit_util, 
                             (type_t *)&one,  0, (type_t(*)())is_utah_false,       0},
    {"POE daughter card LED util",          0,0, (type_t(*)())led_poe_dag_crd_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Hard Disk Drive LED util",            0,0, (type_t(*)())led_hdd_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Environmental LED util",              0,0, (type_t(*)())led_env_util, 
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
    {"SFP2 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"SFP3 LED util",                       0,0, (type_t(*)())led_sfp_util, 
                            (type_t *)&three, 0, (type_t(*)())is_utah_false,       0},
    {"Blink SFP0 LED util",                 0,0, (type_t(*)())led_sfp_blink_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Blink SFP1 LED util",                 0,0, (type_t(*)())led_sfp_blink_util, 
                             (type_t *)&one,  0, (type_t(*)())0,       0},
    {"Blink SFP2 LED util",                 0,0, (type_t(*)())led_sfp_blink_util, 
                             (type_t *)&two,  0, (type_t(*)())0,       0},
    {"Blink SFP3 LED util",                 0,0, (type_t(*)())led_sfp_blink_util, 
                            (type_t *)&three, 0, (type_t(*)())is_utah_false,       0},
    {"Management eth LED util",             0,0, (type_t(*)())led_eth_util, 
                             (type_t *)&zero, 0, (type_t(*)())0,       0},
    {"Blink Management eth LED util",       0,0, (type_t(*)())led_eth_blink_util, 
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
 * Function: led_voice_mdl_util
 *
 * Description: This function exercises the voice module LED 
 *              function of the LED Miscellaneous On/Off Register  
 *              in the register of FPGA.
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_voice_mdl_util (void)
{
    uint32_t choice, led_voice_mdl_reg, field_mask, act_mask;
    char *voice_name;
    boolean exit_flag = FALSE;

    choice = led_voice_mdl_reg = field_mask = act_mask = 0;

    voice_name = "voice module";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", voice_name);
	printf("  0. Turn %s LED off\n", voice_name);
	printf("  1. Turn %s to green\n", voice_name);
	printf("  2. Turn %s to yellow\n", voice_name);
        printf("  3. Show LED Miscellaneous On/Off Register\n");
	printf("  4. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 4);
	
	led_voice_mdl_reg = get_led_status(LED_CTRL_MISC);
	field_mask = MSK_VOICE_MODULE_LED;

	switch(choice) {
	case 0:
	    act_mask = VOICE_MODULE_LED_OFF;    
	break;
	case 1:
            act_mask = VOICE_MODULE_LED_GREEN;
	break;
	case 2:
	    act_mask = VOICE_MODULE_LED_YELLOW;
	break;
	case 3:
	    printf("LED Miscellaneous On/Off Register = %#.4x\n",
		   led_voice_mdl_reg);
	    continue; 
	break;
	case 4:
	    exit_flag = TRUE;
	break;
	default:
            printf("Not support this item. \n");
        break;
	}

        led_voice_mdl_reg = (led_voice_mdl_reg & ~field_mask) | act_mask;
	set_led_reg(LED_CTRL_MISC, led_voice_mdl_reg);
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
 * Function: led_poe_pwr_unit_util
 *
 * Description: This function exercises the POE power supply unit1 and unit2
 *              LED function of the LED POE Power Supply On/Off Register 
 *              in the register of FPGA
 *
 * Input : unit_num - poe power supply unit number 1 or 2
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_poe_pwr_unit_util (int unit_num)
{
    uint32_t choice, led_poe_pwr_reg, field_mask, act_mask;
    uint32_t off_led, green_led, yellow_led;
    char *unit_name;
    boolean exit_flag = FALSE;

    choice = led_poe_pwr_reg =  field_mask = act_mask = 0; 
    off_led = green_led = yellow_led = 0;

    switch(unit_num){
    case 0:
      field_mask = MSK_POE_PWR_SUPPLY_UNIT1_LED;
      off_led = POE_PWR_SUPPLY_UNIT1_LED_OFF;
      green_led = POE_PWR_SUPPLY_UNIT1_LED_GREEN;
      yellow_led = POE_PWR_SUPPLY_UNIT1_LED_YELLOW;      
      unit_name = "unit1";
    break;	
    case 1:
      field_mask =  MSK_POE_PWR_SUPPLY_UNIT2_LED;
      off_led = POE_PWR_SUPPLY_UNIT2_LED_OFF;
      green_led = POE_PWR_SUPPLY_UNIT2_LED_GREEN;
      yellow_led = POE_PWR_SUPPLY_UNIT2_LED_YELLOW;      
      unit_name = "unit2";
    break;	
    case 2:
      field_mask = MSK_POE_PWR_SUPPLY_BOOST_LED;
      off_led = POE_PWR_SUPPLY_BOOST_LED_OFF;
      green_led = POE_PWR_SUPPLY_BOOST_LED_GREEN;
      unit_name = "boost";
    break;
    default:
      printf("not support this unit\n");
    break;
    }
    

    while (!exit_flag) {
        printf("\n POE power supply %s LED utility:\n", unit_name);
	printf("  0. Turn %s LED off\n", unit_name);
	printf("  1. Turn %s to green\n", unit_name);
	printf("  2. Turn %s to yellow\n", unit_name);
	printf("  3. Show LED POE Power Supply On/Off Register\n");
	printf("  4. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 4);

	led_poe_pwr_reg = get_led_status(LED_CTRL_POE_PWR);

	switch(choice) {
	case 0:
	  act_mask = off_led;	 
	break;

	case 1:
	  act_mask = green_led;
	break;

	case 2:
	  if(unit_num == 2){
	    printf("Not support on boost LED\n");
	    continue;
	  }
	  act_mask = yellow_led;
	break;

	case 3:
	  printf("LED POE Power Supply On/Off Register %#.4x\n",
		    led_poe_pwr_reg);
          continue;
        break;

	case 4:
	    exit_flag = TRUE;
	    break;
	default:
            printf("Not support this item. \n");
        break;
	}
  
        led_poe_pwr_reg = (led_poe_pwr_reg & ~field_mask) | act_mask;
        set_led_reg(LED_CTRL_POE_PWR, led_poe_pwr_reg);
    }
    return(PASSED);
}



/*-----------------------------------------------------------------------------
 *
 * Function: led_poe_dag_crd_util
 *
 * Description: This function exercises the POE daughter card LED 
 *              function of the LED POE daughter card On/Off register 
 *              in the register of FPGA.
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_poe_dag_crd_util (void)
{
    uint32_t choice, led_poe_crd_reg, field_mask, act_mask;
    char *eth_name;
    boolean exit_flag = FALSE;

    choice = led_poe_crd_reg = field_mask = act_mask = 0;

    eth_name = "POE daughter card";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", eth_name);
	printf("  0. Turn %s LED off\n", eth_name);
	printf("  1. Turn %s to green\n", eth_name);
	printf("  2. Turn %s to yellow\n", eth_name);
        printf("  3. Show LED POE daughter card On/Off Register\n");
	printf("  4. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 4);

	led_poe_crd_reg = get_led_status(LED_CTRL_POE_DAUGH);
	
	field_mask = MSK_POE_DAG_CARD_LED;

	switch(choice) {
	case 0:
	    act_mask = POE_DAG_CARD_LED_OFF;    
        break;
	case 1:
            act_mask = POE_DAG_CARD_LED_GREEN;
        break;
	case 2:
	    act_mask = POE_DAG_CARD_LED_YELLOW;
        break;
	case 3:
	    printf("LED POE daughter card On/Off Register %#.4x\n",
            led_poe_crd_reg);
        continue; 
        break;
	case 4:
	    exit_flag = TRUE;
	    return(PASSED);
        break;
	default:
            printf("Not support this item. \n");
        break;
	}

        led_poe_crd_reg = (led_poe_crd_reg & ~field_mask) | act_mask;
        set_led_reg(LED_CTRL_POE_DAUGH, led_poe_crd_reg);
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
    uint32_t choice, led_rj45_eth_reg, g_link_mask, y_link_mask, g_speed_mask;
    uint32_t act_mask;

    g_link_mask = y_link_mask = g_speed_mask = act_mask = 0;
    while (1) {
        printf("\n LED RJ-45 Ethernet port %d utility:\n", port_num);
	printf("  0. Toggle port %d green speed LED\n", port_num);
	printf("  1. Toggle port %d green link LED\n", port_num);
	printf("  2. Toggle port %d ambert LED\n", port_num);
        printf("  3. Show LED RJ-45 Ethernet On/Off Register\n");
	printf("  4. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 5);
	led_rj45_eth_reg = get_led_status(LED_CTRL_RJ45_ONOFF);
        
        if (choice == 4)
            return PASSED;
        
        if (choice == 3) {
            printf("LED_CTRL_RJ45_ONOFF is %#x\n", led_rj45_eth_reg);
            continue;
        }

        switch (port_num) {
        case 0:
            g_link_mask = RJ45_ETH_PORT0_LED_GREEN;
            y_link_mask = RJ45_ETH_PORT0_LED_YELLOW;
            g_speed_mask = RJ45_ETH_PORT0_LED_BLINK;
            break;
        case 1:
            g_link_mask = RJ45_ETH_PORT1_LED_GREEN;
            y_link_mask = RJ45_ETH_PORT1_LED_YELLOW;
            g_speed_mask =  RJ45_ETH_PORT1_LED_BLINK;
            break;
        case 2:
            g_link_mask = RJ45_ETH_PORT2_LED_GREEN;
            g_speed_mask = RJ45_ETH_PORT2_LED_BLINK;
            /* error check here */
            break;
        case 3:
            g_link_mask = RJ45_ETH_PORT3_LED_GREEN;
            g_speed_mask = RJ45_ETH_PORT3_LED_BLINK;
            /* error check here */
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
 * Input : sfp_num - SFP number 0, 1, 2, 3
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

        choice = gethex_answer("Enter selection:", 0, 0, 5);

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
        case 2:
            g_link_mask = SFP2_LED_GREEN;
            y_link_mask = SFP2_LED_YELLOW;
            g_speed_mask = SFP2_LED_BLINK;
            break;
        case 3:
            g_link_mask = SFP3_LED_GREEN;
            y_link_mask = SFP3_LED_YELLOW;
            g_speed_mask = SFP3_LED_BLINK;
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
 * Function: led_eth_util
 *
 * Description: This function exercises the management ethernet LED 
 *              function of the LED management ethernet on/off register
 *              in the register of FPGA.
 *              If the blink register is not turned on, it will just
 *              trun on LED without blink.
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_eth_util (void)
{
    uint32_t choice, led_eth_reg, field_mask, act_mask;
    char *eth_name;
    boolean exit_flag = FALSE;

    choice = led_eth_reg = field_mask = act_mask = 0;

    eth_name = "Management Ethernet";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", eth_name);
	printf("  0. Toggle %s port green link\n", eth_name);
	printf("  1. Toggle %s port green speed\n", eth_name);
        printf("  2. Show LED Management Ethernet On/Off Register\n");
	printf("  3. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 4);

	led_eth_reg = get_led_status(LED_CTRL_MGMT_ONOFF);

	switch(choice) {
	case 0:
	    field_mask = MSK_MNG_ETH_LED_GREEN_LINK;
            if (led_eth_reg & MNG_ETH_LED_GREEN_LINK_ON)
	      act_mask = MNG_ETH_LED_GREEN_LINK_ASSERT;
            else 
	      act_mask = MNG_ETH_LED_GREEN_LINK_ON;   
	break;

	case 1:
	    field_mask = MSK_MNG_ETH_LED_GREEN_SPD;
            if (led_eth_reg & MNG_ETH_LED_GREEN_SPD_ON)
	      act_mask = MNG_ETH_LED_GREEN_SPD_ASSERT;
            else 
	      act_mask = MNG_ETH_LED_GREEN_SPD_ON;   
	break;

	case 2:
	    printf("LED Management Ethernet On/Off Register %#.4x\n", led_eth_reg);
            continue;
	break;

	case 3:
	    exit_flag = TRUE;
	break;

	default:
          printf("Not support this item. \n");
	break;
	}

        led_eth_reg = (led_eth_reg & ~field_mask) | act_mask;
	set_led_reg(LED_CTRL_MGMT_ONOFF, led_eth_reg);
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
 * Function: led_eth_blink_util
 *
 * Description: This function test the system LED blinking feature, for the  
 *              Management ethernet LED
 *
 * Input : None
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_eth_blink_util (void) 
{
    uint32_t choice, blk_eth_reg, field_mask, act_mask;
    char *eth_name;
    boolean exit_flag = FALSE;

    eth_name = "Blink management ethernet";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", eth_name);
	printf("  0. Turn off %s blink\n", eth_name);
	printf("  1. Turn %s one blink\n", eth_name);
	printf("  2. Turn %s two blinks\n", eth_name);
	printf("  3. Turn %s three blinks\n", eth_name);
        printf("  4. Show LED SFP blink register\n");
	printf("  5. exit\n");

	choice = gethex_answer("Enter selection:", 0, 0, 5);

	blk_eth_reg = get_led_status(LED_CTRL_ETH_BLINK_EN);
	field_mask = MSK_MNG_ETH_BLINK_LED;

	switch(choice) {
	case 0:
	    act_mask = MNG_ETH_BLINK_LED_OFF;    
	    break;
	case 1:
            act_mask = MNG_ETH_BLINK_ONE;
	    break;
	case 2:
	    act_mask = MNG_ETH_BLINK_TWO;
	    break;
	case 3:
	    act_mask = MNG_ETH_BLINK_THREE;
	    break;
	case 4:
	    printf("LED Management Ethernet On/Off Register %#.4x\n",
		   blk_eth_reg);
		  continue;
	    break;
	case 5:
	    exit_flag = TRUE;
	    return(PASSED);
	    break;
	default:
            printf("Not support this item. \n");
	break;
	}
        blk_eth_reg = (blk_eth_reg & ~field_mask) | act_mask;
	set_led_reg(LED_CTRL_ETH_BLINK_EN, blk_eth_reg);
    }
    return(PASSED);
}

/*-----------------------------------------------------------------------------
 *
 * Function: led_hdd_util
 *
 * Description: This function to control HDD LEDs by accessing 
 *              the Hard Disk Drive Register of FPGA.
 *
 * Input : None 
 * 
 * Output: Always PASSED (even though there is nothing to be returned)
 *         to be called from submenu.
 *-----------------------------------------------------------------------------
 */
static int 
led_hdd_util (void) 
{
    uint32_t choice = 0, choice_led = 0;
    uint32_t led_hdd_reg = 0, field_mask = 0, act_mask = 0;
    uint32_t time_interval = 0;
    char *hdd_name;
    boolean exit_flag = FALSE;

    hdd_name = "Hard Disk Drive";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", hdd_name);
	printf("  0. Toogle HDD2 Status LED to Green\n");
	printf("  1. Toggle HDD2 Status LED to Yellow\n");
	printf("  2. Toogle HDD2 Status LED to Green Blinking\n");
	printf("  3. Toggle HDD2 Status LED to Yellow Blinking\n");
	printf("  4. Turn HDD2 Status LED OFF\n");
	printf("  5. Toogle HDD1 Status LED to Green\n");
	printf("  6. Toggle HDD1 Status LED to Yellow\n");
	printf("  7. Toogle HDD1 Status LED to Green Blinking\n");
	printf("  8. Toggle HDD1 Status LED to Yellow Blinking\n");
	printf("  9. Turn HDD1 Status LED OFF\n");
	printf(" 10. Set HDD Status LED Blinking Timer\n");
	printf(" 11. Set HDD Signal polarity Inversion\n");
	printf(" 12. Show HDD Activity signal status\n");
        printf(" 13. Toggle HDD Activity LED to Green\n");
        printf(" 14. Set HDD Activity LED on-off when Ext. ACT signal is asserted\n");
        printf(" 15. Set HDD Activity LED on-off only ACT bit(bit 12) is set\n");
        printf(" 16. Set HDD Activity LED ON only Ext. ACT signal is asserted\n");
        printf(" 17. Set HDD Activity LED is driven directly by ACT bit(bit 12)\n");
	printf(" 18. Set HDD Activity Timer\n");
	printf(" 19. Show LED Hard Disk Drive Register\n");
	printf(" 20. Exit\n");

	choice = getdec_answer("Enter selection:", 20, 0, 20);

        led_hdd_reg = get_led_status(LED_CTRL_HD_DRIVER);
   
	switch(choice) {
	case 0:
          field_mask = HDD2_STAT_LED_MSK;
          if ((led_hdd_reg & HDD2_STAT_LED_MSK) == HDD2_LED_GREEN) {
              act_mask = HDD2_LED_OFF;
          } else {
              act_mask = HDD2_LED_GREEN;
          }
	break;
	case 1:
          field_mask = HDD2_STAT_LED_MSK;
          if ((led_hdd_reg & HDD2_STAT_LED_MSK) == HDD2_LED_YELLOW) {
              act_mask = HDD2_LED_OFF;
          } else {
              act_mask = HDD2_LED_YELLOW;
          }
	break;
	case 2:
          field_mask = HDD2_STAT_LED_MSK;
          if ((led_hdd_reg & HDD2_STAT_LED_MSK) == HDD2_LED_GREEN_BLINK) {
              act_mask = HDD2_LED_OFF;
          } else {
              act_mask = HDD2_LED_GREEN_BLINK;
          }
	break;
	case 3:
          field_mask = HDD2_STAT_LED_MSK;
          if ((led_hdd_reg & HDD2_STAT_LED_MSK) == HDD2_LED_YELLOW_BLINK) {
              act_mask = HDD2_LED_OFF;
          } else {
              act_mask = HDD2_LED_YELLOW_BLINK;
          }
	break;
	case 4:
          field_mask = HDD2_STAT_LED_MSK;
          act_mask = HDD2_LED_OFF;
	break;
	case 5:
          field_mask = HDD1_STAT_LED_MSK;
          if ((led_hdd_reg & HDD1_STAT_LED_MSK) == HDD1_LED_GREEN) {
              act_mask = HDD1_LED_OFF;
          } else {
              act_mask = HDD1_LED_GREEN;
          }
	break;
	case 6:
          field_mask = HDD1_STAT_LED_MSK;
          if ((led_hdd_reg & HDD1_STAT_LED_MSK) == HDD1_LED_YELLOW) {
              act_mask = HDD1_LED_OFF;
          } else {
              act_mask = HDD1_LED_YELLOW;
          }
	break;
	case 7:
          field_mask = HDD1_STAT_LED_MSK;
          if ((led_hdd_reg & HDD1_STAT_LED_MSK) == HDD1_LED_GREEN_BLINK) {
              act_mask = HDD1_LED_OFF;
          } else {
              act_mask = HDD1_LED_GREEN_BLINK;
          }
	break;
	case 8:
          field_mask = HDD1_STAT_LED_MSK;
          if ((led_hdd_reg & HDD1_STAT_LED_MSK) == HDD1_LED_YELLOW_BLINK) {
              act_mask = HDD1_LED_OFF;
          } else {
              act_mask = HDD1_LED_YELLOW_BLINK;
          }
	break;
	case 9:
          field_mask = HDD1_STAT_LED_MSK;
          act_mask = HDD1_LED_OFF;
	break;
        case 10:
          time_interval = 0;
          time_interval = getdec_answer("Enter the Blinking interval(ms):",
                                        250, 0, 1023);
          field_mask = HDD_STAT_BLINK_TIME_MSK;
          act_mask = (time_interval << HDD_STAT_BLINK_TIME_OFF);
        break;
	case 11:
	  printf("  0. Set polarity of Activity signal to Active Low\n");
	  printf("  1. Set polarity of Activity signal to Active High\n");
          choice_led = gethex_answer("Enter selection:", 0, 0, 1);
	  field_mask = HDD_ACT_INVERSION;
          act_mask = (choice_led == 0) ? ~HDD_ACT_INVERSION : HDD_ACT_INVERSION;
	break;
        case 12:
          if (led_hdd_reg & HDD_ACT_STAT) {
             printf("The Activity Signal is at Logic High.\n");
          } else {
             printf("The Activity Signal is at Logic Low.\n");
          }
        break;
        case 13:
          field_mask = HDD_ACT_GREEN;
          if (led_hdd_reg & HDD_ACT_GREEN) {
              act_mask = HDD2_LED_OFF;
          } else {
              act_mask = HDD_ACT_GREEN;
          }
        break;
        case 14:
          field_mask = HDD_ACT_FUN_MSK;
          act_mask = HDD_ACT_BLINK;
        break;
        case 15:
          field_mask = HDD_ACT_FUN_MSK;
          act_mask = HDD_ACT_BLINK_B12_ON;
        break;
        case 16:
          field_mask = HDD_ACT_FUN_MSK;
          act_mask = HDD_ACT_EXT_ONLY;
        break;
        case 17:
          field_mask = HDD_ACT_FUN_MSK;
          act_mask = HDD_ACT_DRV_FROM_B12;
        break;
        case 18:
          time_interval = 0;
          time_interval = getdec_answer("Enter the Activity timer(ms):",
                                        100, 0, 1023);
          field_mask = HDD_ACT_TIME_MSK;
          act_mask = time_interval;
        break;
        case 19:
          printf("\nLED %s Register = 0x%08X.\n", hdd_name, led_hdd_reg);
        break;
	case 20:
          exit_flag = TRUE;
	break;
	default:
          printf("Not support this item.\n");
	break;
	}

        if ((field_mask != 0) || (act_mask != 0)) { 
            led_hdd_reg = (led_hdd_reg & ~field_mask) | act_mask;
            set_led_reg(LED_CTRL_HD_DRIVER, led_hdd_reg);
        }
    }

    return (PASSED);
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
    uint32_t led_env_reg = 0, field_mask = 0, act_mask = 0;
    uint32_t time_interval = 0;
    char *env_name;
    boolean exit_flag = FALSE;

    env_name = "Enviromental";    
    while (!exit_flag) {
        printf("\n%s LED utility:\n", env_name);
	printf("  0. Toogle Temperature LED to Green\n");
	printf("  1. Toogle Temperature LED to Yellow\n");
	printf("  2. Toogle FAN LED to Green\n");
	printf("  3. Toogle FAN LED to Yellow\n");
	printf("  4. Toogle FAN LED to Yellow Blinking\n");
	printf("  5. Set FAN LED Blinking Timer\n");
	printf("  6. Show LED %s Register\n", env_name);
	printf("  7. Exit\n");

	choice = getdec_answer("Enter selection:", 7, 0, 7);

        led_env_reg = get_led_status(LED_CTRL_ENV);
   
	switch(choice) {
	case 0:
          field_mask = TEMP_GREEN_LED;
          if (led_env_reg & TEMP_GREEN_LED) {
              act_mask = (led_env_reg & ~TEMP_GREEN_LED);
          } else {
              act_mask = TEMP_GREEN_LED;
          }
	break;
	case 1:
          field_mask = TEMP_YELLOW_LED;
          if (led_env_reg & TEMP_YELLOW_LED) {
              act_mask = (led_env_reg & ~TEMP_YELLOW_LED);
          } else {
              act_mask = TEMP_YELLOW_LED;
          }
	break;
	case 2:
          field_mask = FAN_GREEN_LED;
          if (led_env_reg & FAN_GREEN_LED) {
              act_mask = (led_env_reg & ~FAN_GREEN_LED);
          } else {
              act_mask = FAN_GREEN_LED;
          }
	break;
	case 3:
          field_mask = FAN_YELLOW_LED;
          if (led_env_reg & FAN_YELLOW_LED) {
              act_mask = (led_env_reg & ~FAN_YELLOW_LED);
          } else {
              act_mask = FAN_YELLOW_LED;
          }
	break;
	case 4:
          field_mask = FAN_YELLOW_LED_BLINK;
          if (led_env_reg & FAN_YELLOW_LED_BLINK) {
              act_mask = (led_env_reg & ~FAN_YELLOW_LED_BLINK);
          } else {
              act_mask = FAN_YELLOW_LED_BLINK;
          }
	break;
        case 5:
          time_interval = 0;
          time_interval = getdec_answer("Enter the FAN LED Blinking timer(ms):",
                                        100, 0, 1023);
          field_mask = FAN_LED_BLINK_TIME_MSK;
          act_mask = time_interval;
        break;
        case 6:
          printf("\nLED %s Register = 0x%08X.\n", env_name, led_env_reg);
        break;
	case 7:
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


/* -------------------- End of file ------------------- */

/******** History ******** 
$Log: platform_led.c,v $
Revision 1.4  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.3  2013/11/11 21:18:40  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.2  2013/07/30 10:05:16  danchung
Modified for LED test on Utah.

Revision 1.1  2013/05/09 05:42:37  alpeng
moving overlord common code from x86

Revision 1.8  2012/11/29 02:14:33  palin2
To add HDD and Environmental LEDs control utility spoort.

Revision 1.7  2012/11/28 02:36:17  palin2
Fixed Ethernet LED utility: Management and RJ45.

Revision 1.6  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.5  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.4  2012/07/23 08:58:14  alpeng
update LED util and clean up code

Revision 1.3  2012/05/30 16:45:03  palin2
Clean up compile warnings.

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
