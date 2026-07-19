/* $Id: diag_led_util.c,v 1.2 2021/04/15 00:52:26 achiu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/phoenix/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c - This file is for LED utility 
 *
 *
 * Copyright (c) 2008-2019 by Cisco Systems, Inc.
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
#include "plat_defs.h"
#include "dnv_eth_lib.h"
#include "diag_led_test.h"
#include "diag_cpld_lib.h"
#include "sys/io.h"

extern int pwr_seq_pwr_led_green(void);
extern int pwr_seq_pwr_led_amber(void);
extern int pwr_seq_pwr_led_off(void);

/* Local functions */
int diag_psu_led_util(void);
int diag_pwr_led_util(void);
int diag_stat_led_util(void);
int diag_fan_led_util(void);
int diag_temp_led_util(void);
int diag_ssd_led_util(void);
int diag_console_led_util(void);
int diag_I350_RJ45_led_util(void);
int diag_port80_led_util(void);
int diag_debug_led_util(void);
void diag_debug_led_all_off(void);
void diag_debug_led_normal_mode(void);


/******************************************************************************
 *
 * Function: diag_psu_led_util
 *
 * Description: PSU LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_psu_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("PSU LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* Access FPGA Register for controll PSU LED */
            if (fpga_read_reg(FPGA_PSU_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
            
            /* Turn on led with green */
            reg_val &= PHOENIX_LED_OFF;
            reg_val |= PWR_PSU_LED_GREEN;
            if (fpga_write_reg(FPGA_PSU_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {
            if (fpga_read_reg(FPGA_PSU_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            /* Turn on led with amber */
            reg_val &= PHOENIX_LED_OFF;
            reg_val |= PWR_PSU_LED_YELLOW;

            if (fpga_write_reg(FPGA_PSU_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {
            /* Turn off all led */
            reg_val &= PHOENIX_LED_OFF;
            if (fpga_write_reg(FPGA_PSU_LED, reg_val) != PASSED) {
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
 * Function: diag_stat_led_util
 *
 * Description: System status LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_stat_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("System Status LED Supported Mode:\n");
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
            reg_val &= PHOENIX_LED_OFF;
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
            reg_val &= PHOENIX_LED_OFF;
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
            reg_val &= PHOENIX_LED_OFF;
        
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
 * Function: diag_fan_led_util
 *
 * Description: FAN LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_fan_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("Fan LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        /* Access FPGA Register for environment LED */
        if (fpga_read_reg(FPGA_ENV_LED, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }

        if (test_mode == 0) {

            /* Turn on led with green */
            reg_val &= ~ENV_LED_FAN_MASK;
            reg_val |= ENV_LED_FAN_GREEN;
            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            /* Turn on led with amber */
            reg_val &= ~ENV_LED_FAN_MASK;
            reg_val |= ENV_LED_FAN_YELLOW;
            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {

            /* Turn off all led */
            reg_val &= ~ENV_LED_FAN_MASK;
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
 * Function: diag_temp_led_util
 *
 * Description: TEMP LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_temp_led_util (void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("TEMP LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        /* Access FPGA Register for environment LED */
        if (fpga_read_reg(FPGA_ENV_LED, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }

        if (test_mode == 0) {

            /* Turn on led with green */
            reg_val &= ~ENV_LED_TEMP_MASK;
            reg_val |= ENV_LED_TEMP_GREEN;
            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            /* Turn on led with amber */
            reg_val &= ~ENV_LED_TEMP_MASK;
            reg_val |= ENV_LED_TEMP_YELLOW;
            if (fpga_write_reg(FPGA_ENV_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {

            /* Turn off all led */
            reg_val &= ~ENV_LED_TEMP_MASK;
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
 * Function: diag_ssd_led_util
 *
 * Description: Utility of SSD LED
 *
 * Inputs      : None
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_ssd_led_util(void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("SSD LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {

            /* Access FPGA Register for controll system LED */
            if (fpga_read_reg(FPGA_SSD_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            /* Turn on led with green */
            reg_val &= ~SSD_LED_MASK;
            reg_val |= SSD_LED_GREEN;
            if (fpga_write_reg(FPGA_SSD_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 1) {

            if (fpga_read_reg(FPGA_SSD_LED, &reg_val) != PASSED) {
                printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }

            /* Turn on led with amber */
            reg_val &= ~SSD_LED_MASK;
            reg_val |= SSD_LED_YELLOW;

            if (fpga_write_reg(FPGA_SSD_LED, reg_val) != PASSED) {
                printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
                return (FAILED);
            }
        } else if (test_mode == 2) {

            /* Turn off all led */
            reg_val &= ~SSD_LED_MASK;
            if (fpga_write_reg(FPGA_SSD_LED, reg_val) != PASSED) {
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
            reg_val &= PHOENIX_LED_OFF;

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
            reg_val &= PHOENIX_LED_OFF;
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
 * Function: diag_debug_led_util
 *
 * Description: Debug LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_debug_led_util (void)
{
    uint32_t test_mode = 0;
    int ret = PASSED;

    while (1) {
        printf("Debug LED Supported Mode:\n");
        printf("[0] Turn all Green on.\n");
        printf("[1] Turn all Amber on.\n");
        printf("[2] Turn all LEDs off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* Turn on PWR green LED. */
            if (PASSED != pwr_seq_pwr_led_green()) {
                printf("Failed to turn on PWR green LED\n");
                ret = FAILED;
                break;
            }

            /* Turn on all green LEDs via debug LED register. */
            if (fpga_register_operation(FPGA_DEBUG_LED, ~PHOENIX_DEBUG_LED_MASK,
                                        DEBUG_LED_GREEN) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it GREEN.\n", __LINE__ );
                ret = FAILED;
                break;
            }
        } else if (test_mode == 1) {
            /* Turn on PWR amber LED. */
            if (PASSED != pwr_seq_pwr_led_amber()) {
                printf("Failed to turn on PWR amber LED\n");
                ret = FAILED;
                break;
            }

            /* Turn on all amber LEDs via debug LED register. */
            if (fpga_register_operation(FPGA_DEBUG_LED, ~PHOENIX_DEBUG_LED_MASK,
                                        DEBUG_LED_YELLOW) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it amber.\n", __LINE__ );
                ret = FAILED;
                break;
            }
        } else if (test_mode == 2) {
            /* Back to normal mode first. */
            if (fpga_register_operation(FPGA_DEBUG_LED, ~PHOENIX_DEBUG_LED_MASK,
                                        PHOENIX_LED_OFF) != PASSED) {
                cterr('t', 0, "%d:Failed to turn it OFF.\n", __LINE__ );
                ret = FAILED;
                break;
            }

            /* Turn off all LEDs */
            diag_debug_led_all_off();

        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }
    /* Back to normal before exit function. */
    diag_debug_led_normal_mode();

    return (ret);
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
    uint32_t reg_val = 0, test_mode = 0, test_port = 0;

    while (1) {
        printf("Port80 LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn off.\n");
        printf("[f] Leave the utility.\n");
        printf("\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        /* Set read/write permission start from Port80 on, only one port*/
        if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_ON) < 0) {
            printf("Failed to set access of Port80 on.");
            return (FAILED);
        }

        if (test_mode == 0) {
            test_port = gethex_answer("Enter LED Port number: ", 0, 0, 0x7);
            /* Turn on Port80 led with green*/
            reg_val = inb(PORT80_ADDR);
            outb(reg_val | (PORT80_LED_ON << test_port), PORT80_ADDR);

        } else if (test_mode == 1) {
            test_port = gethex_answer("Enter LED Port number: ", 0, 0, 0x7);
            /* Turn off Port80 led */
            reg_val = inb(PORT80_ADDR);
            outb(reg_val & (~(PORT80_LED_OFF << test_port)), PORT80_ADDR);

        } else if (test_mode == 0xf) {
            reg_val = inb(PORT80_ADDR);
            outb(reg_val & PORT80_LED_ALL_OFF, PORT80_ADDR);
            
            /* Set read/write permission start from Port80 off, only one port*/
            if (ioperm(PORT80_ADDR, ACCESS_PORT_NUM, PORT80_ACCESS_OFF) < 0) {
                printf("Failed to set access of Port80 off");
                return (FAILED);
            }

            break;
        } else {
            printf("Wrong test mode!\n");
        }
    }

    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_pwr_led_util
 *
 * Description: PWR LED utility
 *
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_pwr_led_util (void)
{
    uint32_t test_mode = 0;

    while (1) {
        printf("PWR LED Supported Mode:\n");
        printf("[0] Turn Green on.\n");
        printf("[1] Turn Amber on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        if (test_mode == 0) {
            /* Turn on Green LED */
            if (PASSED != pwr_seq_pwr_led_green()) {
                printf("Failed to turn on green LED\n");
                return (FAILED);
            }
        } else if (test_mode == 1) {
            /* Turn on Amber LED */
            if (PASSED != pwr_seq_pwr_led_amber()) {
                printf("Failed to turn on amber LED\n");
                return (FAILED);
            }
        } else if (test_mode == 2) {
            /* Turn off */
            if (PASSED != pwr_seq_pwr_led_off()) {
                printf("Failed to turn off LED\n");
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
 * Function: diag_console_led_util
 *
 * Description: Utility of console LED
 *
 * Inputs      : None
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_console_led_util(void)
{
    uint32_t reg_offset = 0, reg_val = 0, test_mode = 0;

    while (1) {
        printf("Console LED Supported Mode:\n");
        printf("[0] Turn RJ45 LED on.\n");
        printf("[1] Turn Micro-USB LED on.\n");
        printf("[2] Turn off.\n");
        printf("[f] Leave the utility.\n");
        test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

        /* Access FPGA Register for controll system LED */
        if (fpga_read_reg(FPGA_CONSOLE_MULTIPLEXER, &reg_val) != PASSED) {
            printf("Failed to read FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }

        if (test_mode == 0) {
            /* Turn on RJ45 LED */
            reg_val &= ~CONSOLE_LED_MASK;
            reg_val |= SERIAL_CONSOLE_LED_ON;
        } else if (test_mode == 1) {
            /* Turn on micro-USB LED */
            reg_val &= ~CONSOLE_LED_MASK;
            reg_val |= USB_CONSOLE_LED_ON;
        } else if (test_mode == 2) {
            /* Turn off all LEDs */
            reg_val &= ~CONSOLE_LED_MASK;
            reg_val |= CONSOLE_LED_FORCE_MODE;
        } else if (test_mode == 3) {
            /* Turn on all LEDs */
            reg_val &= ~CONSOLE_LED_MASK;
            reg_val |= CONSOLE_LED_MASK;
        } else if (test_mode == 4) {
            /* Set to normal mode */
            reg_val &= ~CONSOLE_LED_MASK;
        } else if (test_mode == 0xf) {
            break;
        } else {
            printf("Wrong test mode!\n");
        }

        if (fpga_write_reg(FPGA_CONSOLE_MULTIPLEXER, reg_val) != PASSED) {
            printf("Failed to write FPGA register 0x%04X.\n", reg_offset);
            return (FAILED);
        }
    }
    return (PASSED);
}


/******************************************************************************
 *
 * Function: diag_debug_led_all_off
 *
 * Description: Function for debug LED to turn off all LEDs.
 *
 * Inputs      : None
 *
 * Outputs     : None
 *
 *****************************************************************************/
void diag_debug_led_all_off(void)
{
    uint32_t reg_val = 0;

    /* PWR LED */
    pwr_seq_pwr_led_off();

    /* PSU LED */
    fpga_read_reg(FPGA_PSU_LED, &reg_val);
    reg_val &= PHOENIX_LED_OFF;
    fpga_write_reg(FPGA_PSU_LED, reg_val);

    /* STAT LED */
    cpld_read_reg(CPLD_STATUS_LED_REG, &reg_val);
    reg_val &= PHOENIX_LED_OFF;
    cpld_write_reg(CPLD_STATUS_LED_REG, reg_val);

    /* FAN LED, TEMP LED */
    fpga_read_reg(FPGA_ENV_LED, &reg_val);
    reg_val &= ~(ENV_LED_FAN_MASK | ENV_LED_TEMP_MASK);
    fpga_write_reg(FPGA_ENV_LED, reg_val);

    /* SSD LED */
    if (has_m2_device()) {
        fpga_read_reg(FPGA_SSD_LED, &reg_val);
        reg_val &= ~SSD_LED_MASK;
        fpga_write_reg(FPGA_SSD_LED, reg_val);
    }

    /* Console LED */
    fpga_read_reg(FPGA_CONSOLE_MULTIPLEXER, &reg_val);
    reg_val &= ~CONSOLE_LED_MASK;
    reg_val |= CONSOLE_LED_FORCE_MODE;
    fpga_write_reg(FPGA_CONSOLE_MULTIPLEXER, reg_val);

    return;
}


/******************************************************************************
 *
 * Function: diag_debug_led_normal_mode
 *
 * Description: Function for debug LED to restore some special registers.
 *              -- Debug LED register
 *              -- Console LED register
 *
 * Inputs      : None
 *
 * Outputs     : None
 *
 *****************************************************************************/
void diag_debug_led_normal_mode(void)
{
    uint32_t reg_val = 0;

    /* Debug LED */
    fpga_register_operation(FPGA_DEBUG_LED, ~PHOENIX_DEBUG_LED_MASK,
                            PHOENIX_LED_OFF);

    /* Console LED */
    fpga_read_reg(FPGA_CONSOLE_MULTIPLEXER, &reg_val);
    reg_val &= ~CONSOLE_LED_MASK;
    fpga_write_reg(FPGA_CONSOLE_MULTIPLEXER, reg_val);

    return;
}
