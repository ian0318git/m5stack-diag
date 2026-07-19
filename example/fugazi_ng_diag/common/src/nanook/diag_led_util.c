 /* $Id: diag_led_util.c,v 1.2 2019/12/11 10:10:30 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nanook/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c - This file is for LED utility 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endians.h"
#include "defs.h"
#include "types.h"
#include "nvsysvars.h"
#include "error.h"
#include "pcmap.h"
#include "strings.h"
#include "menu.h"
#include "common.h"
#include "cross_platform.h"
#include "mb_tests.h"
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "dash_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dnv_eth_lib.h"
#include "diag_led_test.h"
#include "common_utils.h"
#include "diag_led_util.h"
#include "diag_cpld_lib.h"
#include "diag_esw_lib.h"
#include "diag_esw_util.h"


/* Local functions */
int diag_sys_led_util(void);
int diag_async_led_util (void);
int diag_88E1543_RJ45_led_util(void);
int diag_88E1543_SFP_led_util (void);
int diag_system_led_off (void);
int diag_all_88E1543_RJ45_led_off (void);
int diag_all_88E1543_SFP_led_off (void);
int diag_all_async_leds_on (void);
int diag_all_async_leds_off (void);
int diag_all_green_leds_on (void);
int diag_all_yellow_leds_on (void);
int diag_all_leds_off (void);


/******************************************************************************
 *
 * Function: diag_sys_led_util
 *
 * Description: System LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_sys_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;   
 
    reg_offset = FPGA_LPC_LED_CTRL_REG;

    while (1) {
        printf("System LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
	 printf("[2] Turn Amber blink.\n");
        printf("[3] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            /* Access FPGA Register for controll system LED */
            if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with green */
            reg_val &= SYS_LED_OFF;
            reg_val |= SYS_LED_GREEN; 
            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with amber */
            reg_val &= SYS_LED_OFF;
            reg_val |= SYS_LED_AMBER;

            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {

            if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with amber blink*/
            reg_val &= SYS_LED_OFF;
            reg_val |= SYS_LED_AMBER_BLINK;

            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        }  else if (test_mode == 3) {

            /* Turn off all led */
            reg_val &= LED_OFF;
            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_async_led_util
 *
 * Description: Async LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_async_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_target =0, test_mode = 0;

    while (1) {
		
	 printf("Async FPGA LED Supported Mode:\n");
        printf("[0] Select Crocus 16 FPGA LED 0.\n");
	 printf("[1] Select Crocus 16 FPGA LED 1.\n");
	 printf("[2] Select Crocus 32 FPGA LED 0.\n");
	 printf("[3] Select Crocus 32 FPGA LED 1.\n");
	 printf("[4] Select Crocus 32 FPGA LED 2.\n");
	 printf("[5] Select Crocus 32 FPGA LED 3.\n");
	 printf("[6] Select All Crocus 16 and Crocus 32 FPGA LED.\n");
        printf("[f] Leave the utility.\n");
        test_target = gethex_answer("Enter target LED: ", 0, 0, 0xf);

	 if (test_target == 0xf) {
            break;
        }

	 printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
 
            /* Turn on system led with green */
            reg_offset = FPGA_LED_CROCUS_FPGA_CTRL_REG;

	     if (test_target == 0) {
                reg_val |= CROCUS_16_FPGA_LED0;
	     } else if (test_target == 1) {
	         reg_val |= CROCUS_16_FPGA_LED1;
	     } else if (test_target == 2) {
	         reg_val |= CROCUS_32_FPGA_LED0;
	     } else if (test_target == 3) {
	         reg_val |= CROCUS_32_FPGA_LED1;
	     } else if (test_target == 4) {
	         reg_val |= CROCUS_32_FPGA_LED2;
	     } else if (test_target == 5) {
	        reg_val |= CROCUS_32_FPGA_LED3;
	     }else {
	         reg_val |= FPGA_LED_CROCUS_FPGA_LED_GREEN;
	     }

            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
        } else if (test_mode == 1) {

            /* Turn off all led */
            reg_offset = FPGA_LED_CROCUS_FPGA_CTRL_REG;
            /* Access FPGA Register for controll async LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            if (test_target == 0) {
                reg_val &= ~(CROCUS_16_FPGA_LED0);
	     } else if (test_target == 1) {
	         reg_val &= ~(CROCUS_16_FPGA_LED1);
	     } else if (test_target == 2) {
	         reg_val &= ~(CROCUS_32_FPGA_LED0);
	     } else if (test_target == 3) {
	         reg_val &= ~(CROCUS_32_FPGA_LED1);
	     } else if (test_target == 4) {
	         reg_val &= ~(CROCUS_32_FPGA_LED2);
	     } else if (test_target == 5) {
	        reg_val &= ~(CROCUS_32_FPGA_LED3);
	     }else {
	         reg_val = FPGA_LED_CROCUS_FPGA_LED_OFF;
	     }
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        }else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_88E1543_RJ45_led_util
 *
 * Description: 88E1543 RJ45 LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_88E1543_RJ45_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_target =0, test_mode = 0;

    while (1) {
        printf("88E1543 RJ45 LED Supported Mode:\n");
        printf("[0] Select 88E1543 PORT 0 RJ45 LED.\n");
	 printf("[1] Select 88E1543 PORT 1 RJ45 LED.\n");
	 printf("[2] Select All 88E1543 RJ45 LED.\n");
        printf("[f] Leave the utility.\n");
        test_target = gethex_answer("Enter target LED: ", 0, 0, 0xf);

	 if (test_target == 0xf) {
            break;
        }

	 printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);


        if (test_mode == 0) {

            reg_offset = FPGA_LED_BLINK_EN_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            } 
            /* Disable Blink Control for RJ45 port */
            reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            reg_offset = FPGA_LED_RJ45_ONOFF_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
	     reg_val &= LED_OFF;
	     /* Enable RJ45 speed and link LED */
	     if (test_target == 0) {
		  /* PORT 0 */
                reg_val |= (FPGA_RJ45_PORT0_GREEN_LINK_LED | FPGA_RJ45_PORT0_SPEED_LED);
	     } else if (test_target == 1) {
	         /* PORT 1 */
	         reg_val |= (FPGA_RJ45_PORT1_GREEN_LINK_LED | FPGA_RJ45_PORT1_SPEED_LED);
	     } else {
	         /* PORT 0 & 1 */
	         reg_val |= ( FPGA_RJ45_PORT1_GREEN_LINK_LED | FPGA_RJ45_PORT1_SPEED_LED |
		 	FPGA_RJ45_PORT0_GREEN_LINK_LED | FPGA_RJ45_PORT0_SPEED_LED);
	     }
	     if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
        } else if (test_mode == 1) {

            reg_offset = FPGA_LED_BLINK_EN_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            } 
            /* Disable Blink Control for RJ45 port */
            reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            reg_offset = FPGA_LED_RJ45_ONOFF_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
	     reg_val &= LED_OFF;
	     /* Enable RJ45 amber link LED */
	     if (test_target == 0) {
		  /* PORT 0 */
                reg_val |= (FPGA_RJ45_PORT0_YELLOW_LINK_LED);
	     } else if (test_target == 1) {
	         /* PORT 1 */
	         reg_val |= (FPGA_RJ45_PORT1_YELLOW_LINK_LED);
	     } else {
	         /* PORT 0 & 1 */
	         reg_val |= (FPGA_RJ45_PORT1_YELLOW_LINK_LED | FPGA_RJ45_PORT0_YELLOW_LINK_LED);
	     }
	     if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }


        } else if (test_mode == 2) {

            reg_offset = FPGA_LED_BLINK_EN_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            } 
            /* Disable Blink Control for RJ45 port */
            reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

	     reg_offset = FPGA_LED_RJ45_ONOFF_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
	     /* Turn off all led */
	     reg_val &= LED_OFF;
	     if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_88E1543_SFP_led_util
 *
 * Description: 88E1543 SFP LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_88E1543_SFP_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_target =0, test_mode = 0;

    while (1) {
        printf("88E1543 SFP LED Supported Mode:\n");
        printf("[0] Select 88E1543 PORT 0 SFP LED.\n");
	 printf("[1] Select 88E1543 PORT 1 SFP LED.\n");
	 printf("[2] Select All 88E1543 SFP LED.\n");
        printf("[f] Leave the utility.\n");
        test_target = gethex_answer("Enter target LED: ", 0, 0, 0xf);

	 if (test_target == 0xf) {
            break;
        }

	 printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);


        if (test_mode == 0) {

            reg_offset = FPGA_LED_BLINK_EN_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            } 
            /* Disable Blink Control for RJ45 port */
            reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            reg_offset = FPGA_LED_SFP_ONOFF_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
	     reg_val &= LED_OFF;
	     /* Enable SFP speed and link LED */
	     if (test_target == 0) {
		  /* PORT 0 */
                reg_val |= (FPGA_SFP_PORT0_GREEN_LINK_LED | FPGA_SFP_PORT0_SPEED_LED);
	     } else if (test_target == 1) {
	         /* PORT 1 */
	         reg_val |= (FPGA_SFP_PORT1_GREEN_LINK_LED | FPGA_SFP_PORT1_SPEED_LED);
	     } else {
	         /* PORT 0 & 1 */
	         reg_val |= ( FPGA_SFP_PORT1_GREEN_LINK_LED | FPGA_SFP_PORT1_SPEED_LED |
		 	FPGA_SFP_PORT0_GREEN_LINK_LED | FPGA_SFP_PORT0_SPEED_LED);
	     }
	     if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
        } else if (test_mode == 1) {

            reg_offset = FPGA_LED_BLINK_EN_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            } 
            /* Disable Blink Control for RJ45 port */
            reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            reg_offset = FPGA_LED_SFP_ONOFF_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
	     reg_val &= LED_OFF;
	     /* Enable SFP amber link LED */
	     if (test_target == 0) {
		  /* PORT 0 */
                reg_val |= (FPGA_SFP_PORT0_YELLOW_LINK_LED);
	     } else if (test_target == 1) {
	         /* PORT 1 */
	         reg_val |= (FPGA_SFP_PORT1_YELLOW_LINK_LED);
	     } else {
	         /* PORT 0 & 1 */
	         reg_val |= (FPGA_SFP_PORT1_YELLOW_LINK_LED | FPGA_RJ45_PORT0_YELLOW_LINK_LED);
	     }
	     if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }


        } else if (test_mode == 2) {

            reg_offset = FPGA_LED_BLINK_EN_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            } 
            /* Disable Blink Control for RJ45 port */
            reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
            if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

	     reg_offset = FPGA_LED_SFP_ONOFF_REG;
            /* Access FPGA Register for controll system LED */
            if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
	     /* Turn off all led */
	     reg_val &= LED_OFF;
	     if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
			
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_sys_led_off
 *
 * Description: Turn off System LED 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_system_led_off (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    reg_offset = FPGA_LPC_LED_CTRL_REG;

    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(FPGA_ENV_LED, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn off system led with */
    reg_val &= LED_OFF;
    if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_all_88E1543_RJ45_led_off
 * Description : Function to turn Nanook all 88E1543 RJ45 LEDs oof.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_88E1543_RJ45_led_off (void)
{
    uint32_t reg_offset = 0, reg_val = 0;
	
    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for RJ45 port */
    reg_val &= ~(RJ45_PORT1_BLINK_CTRL | RJ45_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_RJ45_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    /* Turn off all led */
    reg_val &= LED_OFF;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);

}


/*******************************************************************************
 *
 * Function    : diag_all_88E1543_SFP_led_off
 * Description : Function to turn Nanook all 88E1543 SFP LEDs oof.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_88E1543_SFP_led_off (void)
{
    uint32_t reg_offset = 0, reg_val = 0;
	
    reg_offset = FPGA_LED_BLINK_EN_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    } 
    /* Disable Blink Control for SFP port */
    reg_val &= ~(SFP_PORT1_BLINK_CTRL | SFP_PORT0_BLINK_CTRL);
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    reg_offset = FPGA_LED_SFP_ONOFF_REG;
    /* Access FPGA Register for controll system LED */
    if (dash_fpga_reg_read(reg_offset, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    /* Turn off all led */
    reg_val &= LED_OFF;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    return (PASSED);

}


/*******************************************************************************
 *
 * Function    : diag_all_async_leds_on
 * Description : Function to turn Nanook all Async LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_async_leds_on (void)
{
    uint32_t reg_offset = 0, reg_val = 0;
	
    reg_offset = FPGA_LED_CROCUS_FPGA_CTRL_REG;
    reg_val |= FPGA_LED_CROCUS_FPGA_LED_GREEN;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
	
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_all_async_leds_off
 * Description : Function to turn Nanook all Async LEDs OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_async_leds_off (void)
{
    uint32_t reg_offset = 0, reg_val = 0;
	
    reg_offset = FPGA_LED_CROCUS_FPGA_CTRL_REG;
    reg_val = FPGA_LED_CROCUS_FPGA_LED_OFF;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
	
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_all_green_leds_on
 * Description : Function to turn Nanook all Green LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_green_leds_on (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    diag_all_async_leds_on();

    reg_offset = FPGA_LED_DBG_REG;
    reg_val |= FPGA_LED_DBG_GREEN;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    if (is_nanook_plus()) {
        diag_esw_all_phy_led_on();
    }
	
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_all_yellow_leds_on
 * Description : Function to turn Nanook all Yellow LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_yellow_leds_on (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    diag_all_leds_off();
	
    reg_offset = FPGA_LED_DBG_REG;
    reg_val |= FPGA_LED_DBG_AMBER;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
	
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_all_leds_off
 * Description : Function to turn Nanook all LEDs default.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_leds_off (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    reg_offset = FPGA_LED_DBG_REG;
    reg_val |= FPGA_LED_DBG_DEFAULT;
    if (dash_fpga_reg_write(reg_offset, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    diag_all_async_leds_off();
    diag_system_led_off();
    diag_all_88E1543_RJ45_led_off();
    diag_all_88E1543_SFP_led_off();
    if (is_nanook_plus()) {
        diag_esw_all_phy_led_off();
    }
	
    return (PASSED);
}
 
 
/*-------------------------------------------------
 * $Log: diag_led_util.c,v $
 * Revision 1.2  2019/12/11 10:10:30  lucywang
 * Merged Nanook to main trunk
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
