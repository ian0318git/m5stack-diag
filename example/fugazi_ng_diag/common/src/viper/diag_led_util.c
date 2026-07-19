 /* $Id: diag_led_util.c,v 1.2 2018/08/06 02:31:51 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c - This file is for LED utility 
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
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
#include "diag_fpga.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "plat_defs.h"
#include "dev_88e6176.h"
#include "diag_esw_lib.h"
#include "dev_88e151x.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_led_test.h"



/* Local functions */
int diag_sys_led_util(void);
int diag_vpn_led_util(void);
int diag_lte_rssi_led_util(void);
int diag_lte_sim_led_util(void);
int diag_esw_led_util(void);
int diag_gephy_led_util(int);
int diag_all_leds_off (void);
int diag_all_green_leds_on(void);
int diag_all_yellow_leds_on(void);


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

    while (1) {
        printf("System LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            /* Access FPGA Register for controll system LED */
            if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with green */
            reg_val &= LED_OFF;
            if (this_is_viper_j()) {
                /* Enable JDM system LED test mode first*/
                if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) 
                                   != PASSED) {
                    printf("Failed to enable JDM Debug Test Mode\n");
                    return (FAILED);
                }
                reg_val |= SYS_OK_LED_GREEN_J; 
            } else {
                reg_val |= SYS_OK_LED_GREEN; 
            }
            if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with amber */
            reg_val &= LED_OFF;
            if (this_is_viper_j()) {
                /* Enable JDM system LED test mode first*/
                if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) 
                                   != PASSED) {
                    printf("Failed to enable JDM Debug Test Mode\n");
                    return (FAILED);
                }
                reg_val |= SYS_OK_LED_AMBER_J;
            } else {
                reg_val |= SYS_OK_LED_AMBER;
            }
            if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {

            /* Turn off all led */
            reg_val &= LED_OFF;
            if (this_is_viper_j()) {
                /* Enable JDM system LED test mode first*/
                if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) 
                                   != PASSED) {
                    printf("Failed to enable JDM Debug Test Mode\n");
                    return (FAILED);
                }  
            }
            if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 0xf) {
            if (this_is_viper_j()) {
                /* Disable JDM system LED test mode first*/
                if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_OFF) 
                                   != PASSED) {
                    printf("Failed to disable JDM Debug Test Mode\n");
                    return (FAILED);
                }  
            }
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_system_amber_led_on
 *
 * Description: This function will turn led amber on
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_system_amber_led_on (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    /* Access FPGA Register for controll system LED */
    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn on system led with green */
    reg_val &= LED_OFF;
    if (this_is_viper_j()) {
	 if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) 
                        != PASSED) {
             printf("Failed to enable JDM Debug Test Mode\n");
             return (FAILED);
        }
        reg_val |= SYS_OK_LED_AMBER_J; 
    } else {
        reg_val |= SYS_OK_LED_AMBER; 
    }
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_system_green_led_on
 *
 * Description: Turn on system led with green
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_system_green_led_on (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    /* Access FPGA Register for controll system LED */
    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn on system led with green */
    reg_val &= LED_OFF;
    if (this_is_viper_j()) {
	    if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) != PASSED) {
            printf("Failed to enable JDM Debug Test Mode\n");
            return (FAILED);
        }
        reg_val |= SYS_OK_LED_GREEN_J; 
    } else {
        reg_val |= SYS_OK_LED_GREEN; 
    }
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
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

    /* Access FPGA Register for controll system LED */
    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }
    
    /* Turn off system led with */
    /* Turn off all led */
	reg_val &= LED_OFF;
        if (this_is_viper_j()) {
            if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) 
                               != PASSED) {
             printf("Failed to disable JDM Debug Test Mode\n");
             return (FAILED);
            }
        }
	if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
		printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
		return (FAILED);
	}

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_lte_rssi_led_util
 *
 * Description: Utility of LTE RSSI LED 
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_lte_rssi_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("LTE RSSI LED Supported Mode:\n");
        printf("[0] Turn LED with high rssi \t- Green steady on.\n");
        printf("[1] Turn LED with medium rssi \t- Green yellow blink.\n");
        printf("[2] Turn LED with low rssi \t- Yellow steady on.\n");
        printf("[3] Turn LED with weak rssi \t- Yellow blink.\n");
        printf("[4] Turn LED with no rssi \t- LED off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on LTE Modem RSSI with high */
            reg_val &= LED_OFF;
            reg_val |= LTE_MOD_HIGH_RSSI; 
            if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 1) {

            if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on LTE Modem RSSI with medium */
            reg_val &= LED_OFF;
            reg_val |= LTE_MOD_MEDIUM_RSSI; 
            if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 2) {

            if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on LTE Modem RSSI with low */
            reg_val &= LED_OFF;
            reg_val |= LTE_MOD_LOW_RSSI; 
            if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 3) {

            if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on LTE Modem RSSI with weak */
            reg_val &= LED_OFF;
            reg_val |= LTE_MOD_RSSI; 
            if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 4) {

            if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on LTE Modem RSSI with no */
            reg_val &= LED_OFF;
            reg_val |= LTE_MOD_NO_RSSI; 
            if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
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
 * Function: diag_lte_sim_led_util
 *
 * Description:  Utility of LTE SIM card LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_lte_sim_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("LTE SIM LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (this_is_viper_j()) {
            /* Enable JDM system LED test mode first, for disable blinking.*/
            if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_ON) != PASSED) {
                 printf("Failed to enable JDM Debug Test Mode\n");
                 return (FAILED);
            }
        }	

        if (test_mode == 0) {

            if (this_is_viper_j()) {
                if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
                    printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                    return (FAILED);
                }
            
                /* Turn on LTE SIM card LED*/
                reg_val &= LED_OFF;
                reg_val |= LTE_SIM_ACT_LED; 
                if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
                    printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                    return (FAILED);
                }
            } else { 
                if (fpga_read_reg(FPGA_LTE_SIM_LED, &reg_val) != PASSED) {
                    printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                    return (FAILED);
                }
            
                /* Turn on LTE SIM card LED*/
                reg_val &= LED_OFF;
                reg_val |= LTE_SIM_LED_ON; 
                if (fpga_write_reg(FPGA_LTE_SIM_LED, reg_val) != PASSED) {
                    printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                    return (FAILED);
                } 
            }

        } else if (test_mode == 1) {
        
            if (this_is_viper_j()) {
	         /* Turn off all led */
                reg_val &= LED_OFF;
                if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
                    printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                    return (FAILED);
                }
            } else {
                /* Turn off all led */
                reg_val &= LTE_SIM_LED_OFF;
                if (fpga_write_reg(FPGA_LTE_SIM_LED, reg_val) != PASSED) {
                    printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                    return (FAILED);
                }
            }

        } else if (test_mode == 0xf) {
            if (this_is_viper_j()) {
                /* Disable JDM system LED test mode*/
                if (fpga_write_reg(FPGA_MANUFAC_TEST_MODE_REG, DBG_TEST_MODE_OFF) != PASSED) {
                    printf("Failed to disable JDM Debug Test Mode\n");
                    return (FAILED);
                }
            } 
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_vpn_led_util
 *
 * Description: Utility of VPN LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_vpn_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("VPN LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            /* Access FPGA Register for controll VPN LED */
            /* Turn on VPN led with green */
            reg_val &= LED_OFF;
            reg_val |= VPN_OK_LED_GREEN; 
            if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 1) {

            /* Turn off all led */
            reg_val &= LED_OFF;
            if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
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
 * Function: diag_vpn_led_on
 *
 * Description: Turn on VPN LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_vpn_led_on (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    /* Access FPGA Register for controll VPN LED */
    /* Turn on VPN led with green */
    reg_val &= LED_OFF;
    reg_val |= VPN_OK_LED_GREEN; 
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_vpn_led_off
 *
 * Description: Turn off VPN LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_vpn_led_off (void)
{
    uint32_t reg_offset = 0, reg_val = 0;

    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    /* Turn off all led */
    reg_val &= LED_OFF;
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
        return (FAILED);
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_esw_led_util
 *
 * Description: Utility of ethernet switch LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_led_util (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc;
    uint32_t test_mode = 0, test_port;

    /* Create 88e6176 device driver */
    rc = diag_esw_dev_create(esw_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    while (1) {
        printf("Ethernet Switch LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);
        test_port = gethex_answer("Enter Port number: ", 0, 0, ESW_PORT3);

        if (test_mode == 0) {

            rc = esw_obj_p->callin_fvt->esw_set_led_on((dev_object_t *)esw_obj_p,
                            test_port);
            if ( rc == FAILED) {
                goto _exit;
            } 

        } else if (test_mode == 1) {

            rc = esw_obj_p->callin_fvt->esw_set_led_off((dev_object_t *)esw_obj_p,
                            test_port);
            if (rc == FAILED) {
                goto _exit;
            } 

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_esw_led_on
 *
 * Description: Turn on ESW LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_led_on (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc;
    uint32_t test_port;

    /* Create 88e6176 device driver */
    rc = diag_esw_dev_create(esw_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    test_port = 0x0;
    rc = esw_obj_p->callin_fvt->esw_set_led_on((dev_object_t *)esw_obj_p,
					test_port);
    if ( rc == FAILED) {
        goto _exit;
    }
    
    test_port = 0x1;
    rc = esw_obj_p->callin_fvt->esw_set_led_on((dev_object_t *)esw_obj_p,
					test_port);
    if ( rc == FAILED) {
        goto _exit;
    }
	
    test_port = 0x2;
    rc = esw_obj_p->callin_fvt->esw_set_led_on((dev_object_t *)esw_obj_p,
					test_port);
    if ( rc == FAILED) {
        goto _exit;
    }
	
    test_port = 0x3;
    rc = esw_obj_p->callin_fvt->esw_set_led_on((dev_object_t *)esw_obj_p,
					test_port);
    
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_esw_led_off
 *
 * Description: Turn off ESW LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_led_off (void)
{
    dev_88e6176_object_t esw_obj;
    dev_88e6176_object_t *esw_obj_p  = &esw_obj;
    int rc;
    uint32_t test_port;

    /* Create 88e6176 device driver */
    rc = diag_esw_dev_create(esw_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed",__func__);
        return (FAILED);
    }

    test_port=0x0;
    rc = esw_obj_p->callin_fvt->esw_set_led_off((dev_object_t *)esw_obj_p,
					test_port);
    if ( rc == FAILED) {
        goto _exit;
    }
	
    test_port=0x1;
    rc = esw_obj_p->callin_fvt->esw_set_led_off((dev_object_t *)esw_obj_p,
					test_port);
    if ( rc == FAILED) {
        goto _exit;
    }
	
    test_port=0x2;
    rc = esw_obj_p->callin_fvt->esw_set_led_off((dev_object_t *)esw_obj_p,
					test_port);
    if ( rc == FAILED) {
        goto _exit;
    }
	
    test_port=0x3;
    rc = esw_obj_p->callin_fvt->esw_set_led_off((dev_object_t *)esw_obj_p,
					test_port);    
    
_exit:
    esw_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&esw_obj_p);
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_gephy_led_util
 *
 * Description: Utility of gephy0/1 LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_led_util (int gephy_num)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;
    uint32_t test_mode = 0;


    /* Create device for GE0/GE1 */
    if (gephy_num == GEPHY0) {
        rc = diag_gephy_dev_create(VIPER_88E1514_PHY, gephy_obj_p);
    } else {
        rc = diag_gephy_dev_create(VIPER_GE1_88E1514_PHY, gephy_obj_p);
    }
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    while (1) {
        printf("GEPHY%d LED Supported Mode:\n", gephy_num);
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p);
            if ( rc == FAILED) {
                goto _exit;
            } 

        } else if (test_mode == 1) {

            rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p); 
            if (rc == FAILED) {
                goto _exit;
            } 

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!");
        }
    }
_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_gephy_led_on
 *
 * Description: Turn on gephy led
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_led_on (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    // GEPHY0
    rc = diag_gephy_dev_create(VIPER_88E1514_PHY, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p);
    if ( rc == FAILED) {
        goto _exit;
    } 
	
    // GEPHY1
    rc = diag_gephy_dev_create(VIPER_GE1_88E1514_PHY, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    
    rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p);
    if ( rc == FAILED) {
        goto _exit;
    }

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    return (rc);
}

/******************************************************************************
 *
 * Function: diag_gephy_led_off
 *
 * Description: Turn off gephy led
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_gephy_led_off (void)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc;

    // GEPHY0
    rc = diag_gephy_dev_create(VIPER_88E1514_PHY, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p); 
    if (rc == FAILED) {
        goto _exit;
    }  
	
    // GEPHY1
    rc = diag_gephy_dev_create(VIPER_GE1_88E1514_PHY, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    
    rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p); 
    if (rc == FAILED) {
        goto _exit;
    } 

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    return (rc);
}

/*******************************************************************************
 *
 * Function    : diag_all_yellow_leds_on
 * Description : Function to turn SKY all Yellow LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_yellow_leds_on (void)
{
    uint32_t reg_val = 0;
	
    diag_all_leds_off();
    diag_system_amber_led_on();
    
    if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
        printf("Failed to read FPGA register.\n");
    }
	
    /* Turn on LTE Modem RSSI with high */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_LOW_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register.\n");
    }
	
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_all_green_leds_on
 * Description : Function to turn SKY all Green LEDs ON.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_green_leds_on (void)
{
	uint32_t reg_val = 0;
	
    diag_all_leds_off();
    diag_system_green_led_on();
    diag_esw_led_on();
    diag_gephy_led_on();
    
    if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
        printf("Failed to read FPGA register.\n");
    }
	
    /* Turn on LTE Modem RSSI with high */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_HIGH_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register.\n");
    }
	
    if (fpga_read_reg(FPGA_LED_REG, &reg_val) != PASSED) {
        printf("Failed to read FPGA register.\n");
    }
	
	/* Turn on LTE SIM card LED and VPN LED*/
    reg_val |= LTE_SIM_ACT_LED; 
	reg_val |= VPN_OK_LED_GREEN; 
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register.\n");
    } 
	
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_all_leds_off
 * Description : Function to turn SKY all LEDs OFF.
 * Inputs      : opt - reserved for future use.
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_all_leds_off (void)
{
    uint32_t reg_val = 0;
	
    diag_vpn_led_off();
    diag_esw_led_off();
    diag_gephy_led_off();
    diag_system_led_off();
    
    if (fpga_read_reg(FPGA_LTE_RSSI_LED, &reg_val) != PASSED) {
        printf("Failed to read FPGA register.\n");
    }
	
    /* Turn off LTE Modem RSSI LED */
    reg_val &= LED_OFF;
    reg_val |= LTE_MOD_NO_RSSI; 
    if (fpga_write_reg(FPGA_LTE_RSSI_LED, reg_val) != PASSED) {
        printf("Failed to write FPGA register.\n");
    }
	
    /* Turn off all led */
    reg_val  = 0;
    reg_val &= LED_OFF;
    if (fpga_write_reg(FPGA_LED_REG, reg_val) != PASSED) {
        printf("Failed to write FPGA register.\n");
    }
    
    return (PASSED);
}
/*-------------------------------------------------
 * $Log: diag_led_util.c,v $
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.12  2018/07/25 06:37:42  lucywang
 * Modified for ViperJ SIM LED utility
 *
 * Revision 1.1.2.11  2018/06/27 09:10:36  lucywang
 * Added description for "LTE RSSI LED Supported Mode"
 *
 * Revision 1.1.2.10  2018/06/27 06:27:49  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.9  2018/06/05 01:36:38  harrchan
 * Add LTE SIM card LED on/off utility
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
