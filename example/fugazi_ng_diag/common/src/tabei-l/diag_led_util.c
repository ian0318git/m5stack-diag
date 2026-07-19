 /* $Id: diag_led_util.c,v 1.5 2020/08/06 07:54:55 kehuang2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c - This file is for LED utility 
 *
 *
 * Copyright (c) 2008-2020 by Cisco Systems, Inc.
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
#include "dev_88e151x.h"
#include "dnv_eth_lib.h"
#include "diag_gephy_lib.h"
#include "diag_led_test.h"
#include "diag_cpld_lib.h"
#include "sys/io.h"


/* Local functions */
int diag_env_led_util(void);
int diag_status_led_util(void);
int diag_I350_RJ45_led_util(void);
int diag_I350_SFP_led_util(void);
int diag_hdd_led_util(void);
int diag_gephy_led_util(int);
int diag_port80_led_util(void);
int diag_all_led_util (void);
int access_port80_led (int, int);
int access_gephy_led (int, int);

/******************************************************************************
 *
 * Function: diag_env_led_util
 *
 * Description: System LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_env_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("Environment LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            /* Access FPGA Register for controll system LED */
            if (fpga_read_reg(FPGA_ENV_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with green */
            reg_val &= TABEI_LED_OFF;
            reg_val |= ENV_LED_GREEN; 
            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            if (fpga_read_reg(FPGA_ENV_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on system led with amber */
            reg_val &= TABEI_LED_OFF;
            reg_val |= ENV_LED_YELLOW;

            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {

            /* Turn off all led */
            reg_val &= TABEI_LED_OFF;
            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_status_led_util
 *
 * Description: Power Supply LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_status_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("Status LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        reg_offset = CPLD_STATUS_LED_REG;
        if (test_mode == 0) {
            if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        
            /* Turn on led with green */
            reg_val &= TABEI_LED_OFF;
            reg_val |= STATUS_LED_GREEN; 
        
            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 1) {
            if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        
            /* Turn on led with amber */
            reg_val &= TABEI_LED_OFF;
            reg_val |= STATUS_LED_YELLOW; 
        
            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 2) {
            if (cpld_read_reg(reg_offset, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        
            /* Turn off */
            reg_val &= TABEI_LED_OFF;
        
            if (cpld_write_reg(reg_offset, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_I350_RJ45_led_util
 *
 * Description: I350 RJ45 LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_I350_RJ45_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0, led_num = 0;

    while (1) {
        printf("I350 RJ45 LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            /* Access FPGA Register for controll system LED */
            if (fpga_read_reg(FPGA_I350_RJ45_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on led with green */
            reg_val &= TABEI_LED_OFF;
            
            led_num = gethex_answer("Enter LED number: (0~3),4.For all", 0x4, 0, 0x4);
            switch (led_num){
                case 0:
                    reg_val |= RJ45_PORT0_SPEED_LED;
                    break;
                case 1:
                    reg_val |= RJ45_PORT0_LINK_LED;
                    break;
                case 2:
                    reg_val |= RJ45_PORT1_SPEED_LED;
                    break;
                case 3:
                    reg_val |= RJ45_PORT1_LINK_LED;
                    break;
                case 4:
                    reg_val |= RJ45_LED_ON;
                    break;
                default:
                    printf("Wrong LED number\n");
                    return (FAILED);
            }

            if (fpga_write_reg(FPGA_I350_RJ45_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            /* Turn off all led */
            reg_val &= TABEI_LED_OFF;
            if (fpga_write_reg(FPGA_I350_RJ45_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_I350_SFP_led_util
 *
 * Description: I350 SFP LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_I350_SFP_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0, led_num = 0;

    while (1) {
        printf("I350 SFP LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            /* Access FPGA Register for controll system LED */
            if (fpga_read_reg(FPGA_I350_SFP_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on led with green */
            reg_val &= TABEI_LED_OFF;

            led_num = gethex_answer("Enter LED number: (0~3),4.For all", 0x4, 0, 0x4);
            switch (led_num){
                case 0:
                    reg_val |= SFP_LED_PORT0_SPD_GREEN;
                    break;
                case 1:
                    reg_val |= SFP_LED_PORT0_EN_GREEN;
                    break;
                case 2:
                    reg_val |= SFP_LED_PORT1_SPD_GREEN;
                    break;
                case 3:
                    reg_val |= SFP_LED_PORT1_EN_GREEN;
                    break;
                case 4:
                    reg_val |= SFP_LED_GREEN;
                    break;
                default:
                    printf("Wrong LED number\n");
                    return (FAILED);
            }

            if (fpga_write_reg(FPGA_I350_SFP_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 1) {
            /* Access FPGA Register for controll system LED */
            if (fpga_read_reg(FPGA_I350_SFP_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on led with amber */
            reg_val &= TABEI_LED_OFF;

            led_num = gethex_answer("Enter LED number: (0~1),2.For all", 0x2, 0, 0x2);
            switch (led_num){
                case 0:
                    reg_val |= SFP_LED_PORT0_EN_YELLOW;
                    break;
                case 1:
                    reg_val |= SFP_LED_PORT1_EN_YELLOW;
                    break;
                case 2:
                    reg_val |= SFP_LED_YELLOW;
                    break;
                default:
                    printf("Wrong LED number\n");
                    return (FAILED);
            }

            if (fpga_write_reg(FPGA_I350_SFP_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

        } else if (test_mode == 2) {

            /* Turn off all led */
            reg_val &= TABEI_LED_OFF;
            if (fpga_write_reg(FPGA_I350_SFP_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
}
/******************************************************************************
 *
 * Function: diag_hdd_led_util
 *
 * Description: HDD LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_hdd_led_util (void)
{
    uint32_t test_mode = 0;

    while (1) {
        printf("HDD LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* HDD LED Turn Green */
            if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                        HDD_LED_GREEN) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it GREEN.\n", __LINE__ );
                return (FAILED);
            }

        } else if (test_mode == 1) {
            /* HDD LED Turn Yellow */
            if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                        HDD_LED_YELLOW) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it YELLOW.\n", __LINE__ );
                return (FAILED);
            }

        } else if (test_mode == 2) {
            /* HDD LED Turn OFF */
            if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                        TABEI_LED_OFF) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it OFF.\n", __LINE__ );
                return (FAILED);
            }

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
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
        rc = diag_gephy_dev_create(TABEI_GE0_88E1514_PHY, gephy_obj_p);
    } else {
        rc = diag_gephy_dev_create(TABEI_GE1_88E1514_PHY, gephy_obj_p);
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

            rc = gephy_obj_p->callin_fvt->gephy_set_led_single_on((dev_object_t *)gephy_obj_p);
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
            printf("Wrong test mode!\n");
        }
    }
_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    return (rc);
}
/******************************************************************************
 *
 * Function: diag_port80_led_util
 *
 * Description : Utiliy of Port 80 LED
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_port80_led_util(void)
{
    uint32_t  test_mode = 0, test_port = 0;

    while (1) {
        printf("Port80 LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        printf("\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            test_port = gethex_answer("Enter LED Port number:(0~7, 8 for all) ", 0, 0, 0x8);
            /* Turn on Port80 led with green*/
            if (access_port80_led(test_port, PORT80_ACCESS_ON) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it ON.\n", __LINE__ );
                return (FAILED);
            }

        } else if (test_mode == 1) {
            test_port = gethex_answer("Enter LED Port number:(0~7, 8 for all) ", 0, 0, 0x8);
            /* Turn off Port80 led */
            if (access_port80_led(test_port, PORT80_ACCESS_OFF) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it OFF.\n", __LINE__ );
                return (FAILED);
            }

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: access_gephy_led
 *
 * Description: Gephy 1514 LED access
 *
 * Inputs      : which_gephy:  GE0 / GE1
 *               on_off:  GEPHY_ACCESS_ON / GEPHY_ACCESS_OFF
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int access_gephy_led (int which_gephy, int on_off)
{
    dev_88e151x_object_t gephy_obj;
    dev_88e151x_object_t *gephy_obj_p = &gephy_obj;
    int rc = FAILED;

    /* Create device for GE1 */
    rc = diag_gephy_dev_create(which_gephy, gephy_obj_p);
    if (rc == FAILED) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }

    if (on_off == PORT80_ACCESS_ON) {
        /* Write 88e151x LED Register to turn on light */
        rc = gephy_obj_p->callin_fvt->gephy_set_led_on((dev_object_t *)gephy_obj_p);
        if (rc == FAILED) {
            goto _exit;
        } 
    } else if (on_off == PORT80_ACCESS_OFF) {
        /* Write 88e151x LED Register to default status */
        rc = gephy_obj_p->callin_fvt->gephy_set_led_off((dev_object_t *)gephy_obj_p); 
        if (rc == FAILED) {
            goto _exit;
        } 
    } else {
        printf("Wrong mode\n");
        return (FAILED);
    }

_exit:
    gephy_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&gephy_obj_p);
    return (rc);

}

/******************************************************************************
 *
 * Function: access_port80_led
 *
 * Description: Port80 LED access
 *
 * Inputs      : test_port : 0~7 for each port, 8 for all port
 *               on_off:  PORT80_ACCESS_ON / PORT80_ACCESS_OFF
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int access_port80_led (int test_port, int on_off)
{

    uint32_t reg_val = 0;

    /* Set read/write permission start from Port80 on, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_ON) < 0) {
        cterr('f', 0, "Failed to set access of Port80 on.");
        return (FAILED);
    }
    if (on_off == PORT80_ACCESS_ON) {
        /* Turn on Port80 led with green*/
        reg_val = inb(PORT80_ADDR);
        if (test_port <= PORT80_BIT7) {
            outb(reg_val | (PORT80_LED_ON << test_port), PORT80_ADDR);
        } else {
            outb(reg_val | PORT80_LED_ALL_ON, PORT80_ADDR);
        }
    } else if (on_off == PORT80_ACCESS_OFF) {
        /* Turn off Port80 led */
        reg_val = inb(PORT80_ADDR);
        if (test_port <= PORT80_BIT7) {
            outb(reg_val & (~(PORT80_LED_OFF << test_port)), PORT80_ADDR);
        } else {
            outb(reg_val & PORT80_LED_ALL_OFF, PORT80_ADDR);
        }
    } else {
        printf("Wrong mode\n");
        return (FAILED);
    }

    msleep(DELAY_FOR_LED_TEST);
    /* Set read/write permission start from Port80 off, only one port*/
    if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_OFF) < 0) {
        cterr('f', 0, "Failed to set access of Port80 off");
        return (FAILED);
    }
    return (PASSED);

}

/******************************************************************************
 *
 * Function: diag_all_led_util
 *
 * Description : Utiliy for all LED ON/OFF
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/

int diag_all_led_util (void)
{
    uint32_t test_mode = 0;

    while (1) {
        printf("All LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* All LED Turn Green */
            if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                        HDD_LED_GREEN) != PASSED) {
                cterr('t', 0, "%d:Failed to turn HDD LED GREEN.\n", __LINE__ );
                return (FAILED);
            }

            if (is_tabeil() == TRUE) {
                /* Turn on GE0 LED */
                if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_ON) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn GE0 LED ON.\n", __LINE__ );
                    return (FAILED);
                }
            
                /* Turn on GE1 LED */
                if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_ON) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn GE1 LED ON.\n", __LINE__ );
                    return (FAILED);
                }
            }

            /* Turn on Port80 led */
            if (access_port80_led(PORT80_ALL, PORT80_ACCESS_ON) != PASSED) {
                cterr('t', 0, "%d:Failed to turn Port80 LED ON.\n", __LINE__ );
                return (FAILED);
            }

            /* Promethium */
            if (is_promethium() == TRUE) {
                /* Promethium for status LED */
                if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_MASK, 
                                            STATUS_LED_GREEN) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn Status LED GREEN.\n", __LINE__ );
                    return (FAILED);
                }

                /* Promethium for environment LED */
                if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_MASK, 
                                            ENV_LED_GREEN) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn Environment LED GREEN.\n", __LINE__ );
                    return (FAILED);
                }

            }

        } else if (test_mode == 1) {
            /* All LED Turn Yellow */
            if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                        HDD_LED_YELLOW) != PASSED) {
                cterr('t', 0, "%d:Failed to turn HDD LED YELLOW.\n", __LINE__ );
                return (FAILED);
            }

            if (is_tabeil() == TRUE) {
                /* Turn off GE0 LED */
                if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn GE0 LED OFF.\n", __LINE__ );
                    return (FAILED);
                }
            
                /* Turn off GE1 LED */
                if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn GE1 LED OFF.\n", __LINE__ );
                    return (FAILED);
                }
            }

            /* Turn off Port80 led */
            if (access_port80_led(PORT80_ALL, PORT80_ACCESS_OFF) != PASSED) {
                cterr('t', 0, "%d:Failed to turn Port80 LED OFF.\n", __LINE__ );
                return (FAILED);
            }

            /* Promethium */
            if (is_promethium() == TRUE) {
                /* Promethium for status LED */
                if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_MASK, 
                                            STATUS_LED_YELLOW) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn Status LED YELLOW.\n", __LINE__ );
                    return (FAILED);
                }

                /* Promethium for environment LED */
                if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_MASK, 
                                            ENV_LED_YELLOW) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn Environment LED YELLOW.\n", __LINE__ );
                    return (FAILED);
                }

            }

        } else if (test_mode == 2) {
            /* All LED Turn OFF */
            if (fpga_register_operation(FPGA_HDD_LED, TABEI_HDD_LED_MASK, 
                                        TABEI_LED_OFF) != PASSED) {
                cterr('t', 0, "%d:Failed to turn HDD LED OFF.\n", __LINE__ );
                return (FAILED);
            }

            if (is_tabeil() == TRUE) {
                /* Turn off GE0 LED */
                if (access_gephy_led(TABEI_GE0_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn GE0 LED OFF.\n", __LINE__ );
                    return (FAILED);
                }
            
                /* Turn off GE1 LED */
                if (access_gephy_led(TABEI_GE1_88E1514_PHY, GEPHY_ACCESS_OFF) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn GE1 LED OFF.\n", __LINE__ );
                    return (FAILED);
                }
            }

            /* Turn off Port80 led */
            if (access_port80_led(PORT80_ALL, PORT80_ACCESS_OFF) != PASSED) {
                cterr('t', 0, "%d:Failed to turn Port80 LED OFF.\n", __LINE__ );
                return (FAILED);
            }

            /* Promethium */
            if (is_promethium() == TRUE) {
                /* Promethium for status LED */
                if (cpld_register_operation(CPLD_STATUS_LED_REG, TABEI_LED_MASK, 
                                            TABEI_LED_OFF) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn Status LED OFF.\n", __LINE__ );
                    return (FAILED);
                }
                /* Promethium for environment LED */
                if (fpga_register_operation(FPGA_ENV_LED, TABEI_LED_MASK, 
                                            TABEI_LED_OFF) != PASSED) {
                    cterr('t', 0, "%d:Failed to turn Environment LED OFF.\n", __LINE__ );
                    return (FAILED);
                }

            }


        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    return (PASSED);
}


/*-------------------------------------------------
 * $Log: diag_led_util.c,v $
 * Revision 1.5  2020/08/06 07:54:55  kehuang2
 * Collapse Promethium into main trunk
 *
 * Revision 1.4  2019/12/30 06:02:00  kehuang2
 * CSCvs55860: Support All LED ON/OFF
 *
 * Revision 1.3  2019/11/25 08:55:52  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.2  2019/10/17 02:16:22  kehuang2
 * Collapse Tabei-L into main trunk
 *
 * Revision 1.1.2.12  2019/08/29 03:49:27  kehuang2
 * Clean up code by the comment of code review
 *
 * Revision 1.1.2.11  2019/07/08 01:49:20  kehuang2
 * Rename variable to avoid redefined with PIM module
 *
 * Revision 1.1.2.10  2019/06/11 08:14:14  kehuang2
 * Update LED utility
 *
 * Revision 1.1.2.9  2019/05/21 09:18:51  kehuang2
 * Support Port80 LED
 *
 * Revision 1.1.2.8  2019/05/21 03:18:00  kehuang2
 *
 * 1.SFP EN LED Support base on PreP2B respin
 * 2.Support SFP Mux access utility
 *
 * Revision 1.1.2.7  2019/04/24 07:59:21  kehuang2
 * Update CPLD access
 *
 * Revision 1.1.2.6  2019/04/19 03:15:29  kehuang2
 * 1.Support CPLD access 2.Support new FPGA 3.Clean up code
 *
 * Revision 1.1.2.5  2019/03/26 09:58:45  kehuang2
 * Support LED Test
 *
 * Revision 1.1.2.4  2018/12/26 03:48:33  harrchan
 * LED Test
 *
 * Revision 1.1.2.3  2018/11/16 05:42:11  olin2
 * Clean up code
 *
 * Revision 1.1.2.2  2018/10/24 02:47:27  harrchan
 * 88E1514 GEPHY test
 *
 * Revision 1.1.2.1  2018/10/02 01:49:59  harrchan
 * Initial commit for Tabei-L P1A bring up.
 *
 * $Endlog$
 *-------------------------------------------------
 */
