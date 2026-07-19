/* $Id: diag_led_util.c,v 1.2 2021/09/24 01:21:06 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "error.h"
#include "queryflags.h"
#include "nvsysvars.h"
#include "menu.h"
#include "diag_moka_fpga_lib.h"
#include "diag_sirius_fpga_lib.h"
#include "platform_cookie.h"
#include "diag_ge_phy_lib.h"
#include "diag_esw_lib.h"
#include "diag_moka_fpga_util.h"
#include "diag_led_util.h"


/*******************************************************************************
 *                          Function Declaration
 *******************************************************************************
 */
void diag_led_util(void);
int diag_esw_phy_led_util(void);
int phy_all_led_control(int);
int phy_single_led_control(int, uint32_t);
int phy_led_yellow_on(int, uint32_t);
int phy_led_yellow_off(int, uint32_t);
/*******************************************************************************
 *                                   Menus                                     
 *******************************************************************************
 */

/* LED Utility Menu */
static submenu_xtable_t led_util_tbl[] = {
    {"FPGA register Read Util",    (PFT)fpga_reg_rd_util,          0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"FPGA register Write Util",   (PFT)fpga_reg_wr_util,          0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"Turn all Green LED ON",      (PFT)diag_led_all_green_on_util,    0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"Turn all Yellow LED ON",     (PFT)diag_led_all_yellow_on_util,   0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},
    {"Turn all LED OFF",           (PFT)diag_led_all_off_util,         0,
     0, 
     (type_t(*)())0,               0,
     (type_t(*)())0,               0},

    {"ESW PHY LED Utility",       (PFT)diag_esw_phy_led_util,      0,
     0,    
     (type_t(*)())0,               0, 
     (type_t(*)())0,               0},   
};

#define LED_UTIL_TBL_SIZE (sizeof(led_util_tbl) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_util_menu_pri_items[LED_UTIL_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t led_util_menu_sec_items[LED_UTIL_TBL_SIZE + MAX_BASE_ITEMS];

static menuinfo_t led_util_menu = {
    "%s Utility Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    led_util_menu_pri_items,
};
static menuinfo_t *led_util_menu_p = &led_util_menu;


/*******************************************************************************
 *
 * Function   : diag_led_util
 * Description: Entry function of LED Diag Utilities.
 * Inputs     : None
 * Outputs    : None
 *
 *******************************************************************************
 */
void diag_led_util (void)
{
    build_primary_submenu(led_util_tbl, LED_UTIL_TBL_SIZE,
                          "LED", &led_util_menu_p);
    build_secondary_submenu(led_util_tbl, LED_UTIL_TBL_SIZE,
                            led_util_menu_sec_items);

    menu(&led_util_menu, led_util_menu_sec_items, 0);
}

/*******************************************************************************
 *
 * Function    : diag_led_all_green_on_util
 * Description : Function to turn all Green LEDs ON.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_all_green_on_util (void)
{
    uint reg_offset = 0, reg_val = 0;

    /* Turn all LEDs OFF */
    if (diag_led_all_off_util() != PASSED) {
        printf("%s: Failed to turn all LEDs OFF.\n", __func__);
        return (FAILED);
    }

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = (uint)(POE_PRESENT_LED_G | 
                     POE_STAT_LED |
                     MICRO_USB_LED | 
                     USB_LED | 
                     VPN_OK_LED | 
                     CONSOLE_LED);
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }


    /* Turn Status LED to Green */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = STAT_LED_G;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    if (diag_ge_phy_all_led_on(PLAT_LED_GE0) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs ON.\n", __func__);
    }

    if (diag_ge_phy_all_led_on(PLAT_LED_GE1) != PASSED) {
        printf("%s: Failed to turn all GE1 LEDs ON.\n", __func__);
    }

    if (diag_esw_all_phy_green_led_on() != PASSED) {
        printf("%s: Failed to turn all 1680 LEDs ON.\n", __func__);
    }

    /* If this is Star with pluggable slot, turn on pluggable FPGA debug LEDs */
    if (platform_has_pluggable()) {
        reg_offset = (uint) PLUG_FPGA_DBG_LED_ADDR_REG;
        reg_val = 0x00;
        reg_val = PLUG_DBG_LED_ON;
        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_led_all_yellow_on_util
 * Description : Function to turn all Yellow LEDs ON.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_all_yellow_on_util (void)
{
    uint reg_offset = 0, reg_val = 0;

    /* Turn all LEDs OFF */
    if (diag_led_all_off_util() != PASSED) {
        printf("%s: Failed to turn all LEDs OFF.\n", __func__);
        return (FAILED);
    }

    /* Turn Status LED to Yellow */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = STAT_LED_Y;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = (uint)(
                     POE_PRESENT_LED_Y | 
                     POE_P0_LED |
                     POE_P1_LED | 
                     POE_P2_LED | 
                     POE_P3_LED | 
                     AUX_LED
                     );
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : diag_led_all_off_util
 * Description : Function to turn all LEDs OFF.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_led_all_off_util (void)
{
    
    uint reg_offset = 0, reg_val = 0;

    reg_offset = (uint)FPGA_LED_REG;
    reg_val = 0x000000;
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    /* Turn Power OK and Status LED off */
    reg_offset = (uint)FPGA_LPC_STAT_LED_CTRL_REG;
    reg_val = (uint)(PWR_OK_LED_OFF | STAT_LED_OFF);
    if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
    }

    if (diag_ge_phy_all_led_off(PLAT_LED_GE0) != PASSED) {
        printf("%s: Failed to turn all GE0 LEDs OFF.\n", __func__);
    }

    if (diag_ge_phy_all_led_off(PLAT_LED_GE1) != PASSED) {
        printf("%s: Failed to turn all GE1 LEDs OFF.\n", __func__);
    }

    if (diag_esw_all_phy_green_led_off() != PASSED) {
        printf("%s: Failed to turn all 1680 LEDs OFF.\n", __func__);
    }

    /* If this sku with pluggable slot, turn off pluggable FPGA debug LEDs */
    if (platform_has_pluggable()) {
        reg_offset = (uint) PLUG_FPGA_DBG_LED_ADDR_REG;
        reg_val = PLUG_DBG_LED_OFF;
        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
        }
    }

    return (PASSED);
}

/******************************************************************************
 *
 * Function: diag_esw_phy_led_util
 *
 * Description: Ethernet switch PHY led utility.
 * Inputs      : None
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int diag_esw_phy_led_util (void)
{

    uint32_t test_mode = 0, test_target = 0, test_port = 0;
    uint32_t actual_port_num;

    while (1) {
        printf("Ethernet Switch LED Supported Mode:\n");
        printf("[0] Select Single 88E1680 Port LED.\n");
        printf("[1] Select All 88E1680 Port LED.\n");
        printf("[f] Leave the utility.\n");
        test_target = gethex_answer("Enter target LED: ", 0, 0, 0xf);


        switch (test_target) {
            case LEAVE_MENU:
                return (PASSED);

            case SINGLE_LED:
                printf("[0] Turn Green on.\n");
                printf("[1] Turn Yellow on.(Only for port 0-3)\n");
                printf("[2] Turn off.\n");
                printf("[f] Leave the utility.\n");
                test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

                if (test_mode == LEAVE_MENU) {
                    return (PASSED);
                }

                test_port = gethex_answer("Enter Port number: ", 0, 0, (ELIXIR_ESW_PORT_NUM - 1));	
                actual_port_num = test_port;
            
                /* Modify according to the actual board port number */
                if (actual_port_num % 2 == 0) {
                    actual_port_num += 1;
                } else {
                    actual_port_num -= 1;
                }

                if (phy_single_led_control(test_mode, actual_port_num) != PASSED) {
                    goto _exit;
                }
            break;

            case ALL_LED:
                printf("[0] Turn Green on.\n");
                printf("[1] Turn Yellow on.(Only for port 0-3)\n");
                printf("[2] Turn off.\n");
                printf("[f] Leave the utility.\n");
                test_mode = gethex_answer("Enter LED mode: ", 0, 0, 0xf);

                if (test_mode == LEAVE_MENU) {
                    return (PASSED);
                }

                if (phy_all_led_control(test_mode) != PASSED) {
                    goto _exit;
                }
            break;    

            default:
                 printf("Wrong test target!");
            break;
        }
    }             

    return (PASSED);
	
 _exit:

    return (FAILED);
	
}

/******************************************************************************
 *
 * Function: phy_led_yellow_on
 *
 * Description: 1680 PHY yellow led control
 *
 * Inputs      : mode        -  All or single led
 *               test_port   -  which port of PHY   
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int phy_led_yellow_on (int mode, uint32_t test_port)
{
    uint reg_offset = (uint)FPGA_LED_REG;
    uint reg_val = 0;

    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA Reg0x%#X.\n", __func__, reg_offset);
        return (FAILED);
    }

    if (mode == ALL_LED) {
        reg_val |= (uint)(
            POE_P0_LED |
            POE_P1_LED | 
            POE_P2_LED | 
            POE_P3_LED  
        );

        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
            return (FAILED);
        }
    } else if (mode == SINGLE_LED){
        if (test_port < PHY_PORT_MAX_LED) {
            if (test_port == PHY_PORT0_LED) {
                reg_val |= (uint)POE_P1_LED;
            } else if (test_port == PHY_PORT1_LED){
                reg_val |= (uint)POE_P0_LED;
            } else if (test_port == PHY_PORT2_LED){
                reg_val |= (uint)POE_P3_LED;
            } else if (test_port == PHY_PORT3_LED){
                reg_val |= (uint)POE_P2_LED;
            }

            if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
                printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
                return (FAILED);
            }
        } else {
            printf("Port 4-7 does not support yellow led\n");
        }
    }

    return (PASSED);

}



/******************************************************************************
 *
 * Function: phy_led_yellow_off
 *
 * Description: 1680 PHY yellow led control
 *
 * Inputs      : mode        -  All or single led
 *               test_port   -  which port of PHY   
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int phy_led_yellow_off (int mode, uint32_t test_port)
{
    uint reg_offset = (uint)FPGA_LED_REG;
    uint reg_val = 0;

    if (fpga_read_32_reg(reg_offset, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA Reg0x%#X.\n", __func__, reg_offset);
        return (FAILED);
    }

    if (mode == ALL_LED) {
        reg_val &= (uint)(
            (~POE_P0_LED) &
            (~POE_P1_LED) & 
            (~POE_P2_LED) & 
            (~POE_P3_LED)
        );

        if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
            printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
            return (FAILED);
        }
    } else if (mode == SINGLE_LED) {
        if (test_port < PHY_PORT_MAX_LED) {
            if (test_port == PHY_PORT0_LED) {
                reg_val &= ((uint)(~POE_P1_LED));
            } else if (test_port == PHY_PORT1_LED){
                reg_val &= ((uint)(~POE_P0_LED));
            } else if (test_port == PHY_PORT2_LED){
                reg_val &= ((uint)(~POE_P3_LED));
            } else if (test_port == PHY_PORT3_LED){
                reg_val &= ((uint)(~POE_P2_LED));
            }

            if (fpga_write_32_reg(reg_offset, reg_val) != PASSED) {
                printf("%s: Failed to write FPGA Reg0x%#X.\n", __func__, reg_offset);
                return (FAILED);
            }
        } else {
            printf("Port 4-7 does not support yellow led\n");
        }
    }

    return (PASSED);

}


/******************************************************************************
 *
 * Function: phy_all_led_control
 *
 * Description: 1680 PHY all led control
 *
 * Inputs      : mode  -  what color of led
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int phy_all_led_control (int mode)
{
    int rc = PASSED;

    switch (mode) {
        case TURN_ON_GREEN:
            rc = diag_esw_all_phy_green_led_on();
        break;

        case TURN_ON_YELLOW:
            /* ALL_LED case don't need test_port parameter, so use 0 to fill */
            rc = phy_led_yellow_on(ALL_LED, 0);
        break;

        case TURN_OFF:
            /* turn off yellow led */
            /* ALL_LED case don't need test_port parameter, so use 0 to fill */
            rc = phy_led_yellow_off(ALL_LED, 0);
            if (rc != PASSED) {
                return (rc);
            }

             /* turn off green led */
            rc = diag_esw_all_phy_green_led_off();
        break;    
                       
        default:
             printf("Wrong test mode!");
        break;

    }

    return (rc);
}


/******************************************************************************
 *
 * Function: phy_single_led_control
 *
 * Description: 1680 PHY single led control
 *
 * Inputs      : mode         -  what color of led
 *               test_port    -  which port of PHY
 *
 * Outputs     : PASSED / FAILED
 *
 *****************************************************************************/
int phy_single_led_control (int mode, uint32_t test_port)
{
    int rc = PASSED;
    MAD_DEV *mad_dev;

    dev_88e1680_object_t *phy_88e1680_obj_p = NULL;
    dev_object_t *dev;

    /* Create 88e1680 device driver */
    phy_88e1680_obj_p = (dev_88e1680_object_t *)diag_get_phy_88e1680_obj(); 
    if (phy_88e1680_obj_p == NULL) {
        cterr('f', 0, "%s: Creating object failed", __func__);
        return (FAILED);
    }
    dev = (dev_object_t *)phy_88e1680_obj_p;

    mad_dev = &phy_mad_88e1680;

    switch (mode) {
        case TURN_ON_GREEN:
            rc = phy_88e1680_obj_p->callin_fvt->led_on(dev, mad_dev, test_port);
        break;

        case TURN_ON_YELLOW:
            rc = phy_led_yellow_on(SINGLE_LED, test_port);
        break;

        case TURN_OFF:
            /* turn off yellow led */
            rc = phy_led_yellow_off(SINGLE_LED, test_port);
            if (rc != PASSED) {
                return (rc);
            }

            /* turn off green led */
            rc = phy_88e1680_obj_p->callin_fvt->led_off(dev, mad_dev, test_port);
        break;    
                       
        default:
             printf("Wrong test mode!");
        break;

    }

    return (rc);
}






/*-------------------------------------------------
 * $Log: diag_led_util.c,v $
 * Revision 1.2  2021/09/24 01:21:06  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.11  2021/05/31 11:00:59  illiu
 * Clean up code
 *
 * Revision 1.1.2.10  2021/05/31 10:50:07  illiu
 * Modify ESW PHY LED Utility
 *
 * Revision 1.1.2.9  2021/04/12 08:44:52  illiu
 * Replace object-create method as object-get method (Device driver object)
 *
 * Revision 1.1.2.8  2021/03/18 08:10:06  illiu
 * 1. Replace variable phy_dev_88e1680 with phy_mad_88e1680
 * 2. Remove redundant variable
 *
 * Revision 1.1.2.7  2021/02/04 09:33:56  illiu
 * Modify Elixir 1680 phy led callin function
 *
 * Revision 1.1.2.6  2020/11/13 11:48:45  illiu
 * Fix GE0/1 Led
 *
 * Revision 1.1.2.5  2020/11/12 06:38:59  illiu
 * 1. Add Elixir 1680 phy led features to MB LED test/utility item
 * 2. Add ESW PHY LED Utility
 *
 * Revision 1.1.2.4  2020/10/26 07:08:28  harrchan
 * 1.Changed PID table in platform_i2c.c
 * 2.Modify menu item to match up Elixir hardware design.
 *
 * Revision 1.1.2.3  2020/09/14 05:49:44  harrchan
 * Remove DSL and GSHDSL relevant part
 *
 * Revision 1.1.2.2  2020/09/10 09:52:50  illiu
 * Delete 88E6390/88E6176 Switch related code
 *
 * Revision 1.1.2.1  2020/09/09 09:08:06  illiu
 * First version which has been ported with Dreamliner and Marvel CPSS
 *
 * Revision 1.2  2019/01/10 06:36:23  wilbhuan
 * The beginning of Betelgeuse application code.
 *
 *-------------------------------------------------
 */
