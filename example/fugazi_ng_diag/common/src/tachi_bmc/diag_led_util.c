/* $Id: diag_led_util.c,v 1.3 2018/05/15 01:28:16 haohsu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_led_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_led_util.c - LED Utility
 *
 * January 2016, benchen2
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "queryflags.h"
#include "slot.h"
#include "menu.h"
#include "diag_fpga_lib.h"
#include "platform_i2c.h"
#include "diag_i2c_lib.h"
#include "diag_led_util.h"
#include "diag_lewis_gesw_test.h"
#include "diag_smi_lib.h"
#include "diag_gephy_util.h"

int diag_led_util(void);
static void diag_show_led(int);
static int diag_sfp_led_util(int);
static int diag_rj45_led_util(int);
static int diag_blink_rj45_led_util(int);
static int diag_blink_sfp_led_util(int);
static int diag_psu_led_util(int);
static int diag_poe_led_util(void);
static int diag_poe_dc_led_util(void);
static int diag_env_led_util(void);
static int diag_emmc_led_util(void);
static int diag_m2_led_util(void);
static int diag_88E1512_led_util(void);

/* Sub Menu used for LED utility.
 */
static submenu_xtable_t led_util_submenu_table[] = {
    {"All Green", (type_t(*)())diag_show_led, MB_LED_GREEN,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"All Amber", (type_t(*)())diag_show_led, MB_LED_AMBER,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Cycle all LEDs", (type_t(*)())diag_show_led, MB_LED_CYCLE,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Default", (type_t(*)())diag_show_led, MB_LED_DEFAULT,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SFP0 LED util",  (type_t(*)())diag_sfp_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"SFP1 LED util",  (type_t(*)())diag_sfp_led_util,1,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"RJ-45 Port0 LED util" ,(type_t(*)())diag_rj45_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"RJ-45 Port1 LED util" ,(type_t(*)())diag_rj45_led_util,1,
    0, (type_t (*)())0, 0, (type_t(*)())0,   0},
    {"88E1512 LED util" ,(type_t(*)())diag_88E1512_led_util,0,
    0, (type_t (*)())0, 0, (type_t(*)())0,   0},
    {"Blink RJ-45 Port0 LED util" ,(type_t(*)())diag_blink_rj45_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Blink RJ-45 Port1 LED util" ,(type_t(*)())diag_blink_rj45_led_util,1,
    0, (type_t (*)())0, 0, (type_t(*)())0,   0},
    {"Blink SFP0 LED util",  (type_t(*)())diag_blink_sfp_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Blink SFP1 LED util",  (type_t(*)())diag_blink_sfp_led_util,1,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Power supply unit1 LED util",  (type_t(*)())diag_psu_led_util,1,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Power supply unit2 LED util",  (type_t(*)())diag_psu_led_util,2,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"POE power supply unit LED util",  (type_t(*)())diag_poe_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"POE daughter card LED util",  (type_t(*)())diag_poe_dc_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"Environmental LED util",  (type_t(*)())diag_env_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"eMMC flash LED util",  (type_t(*)())diag_emmc_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
    {"M.2 flash LED util",  (type_t(*)())diag_m2_led_util,0,
    0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define LED_UTIL_SUBMENU_TABLE_SIZE (sizeof(led_util_submenu_table) / \
                       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t led_util_primary_items[LED_UTIL_SUBMENU_TABLE_SIZE +
                       MAX_BASE_ITEMS];
static mitem_t led_util_secondary_items[LED_UTIL_SUBMENU_TABLE_SIZE +
                       MAX_BASE_ITEMS];

menuinfo_t led_util_subtest_menu = {
    "%s Subtest Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    led_util_primary_items,
};
menuinfo_t *led_util_submenup = &led_util_subtest_menu;


int diag_led_util (void)
{
    build_primary_submenu(led_util_submenu_table,
                          LED_UTIL_SUBMENU_TABLE_SIZE,
                          "LED Utility", &led_util_submenup);
    build_secondary_submenu(led_util_submenu_table,
                            LED_UTIL_SUBMENU_TABLE_SIZE,
                            led_util_secondary_items);

    menu(led_util_submenup, led_util_secondary_items, '\0');
    return (PASSED);
}

static void diag_show_led (int arg)
{
    switch (arg) {
    case MB_LED_GREEN:
        diag_fpga_reg_write(MB_LED_DEB_REG, MB_LED_GREEN);
        diag_lewis_gesw_phy_led_green_util();
        diag_lewis_gesw_poe_led_green_util();
        break;
    case MB_LED_AMBER:
        diag_fpga_reg_write(MB_LED_DEB_REG, MB_LED_AMBER);
        diag_lewis_gesw_phy_led_amber_util();
        diag_lewis_gesw_poe_led_amber_util();
        break;
    case MB_LED_CYCLE:
        diag_fpga_reg_write(MB_LED_DEB_REG, MB_LED_CYCLE);
        break;
    case MB_LED_DEFAULT:
        diag_fpga_reg_write(MB_LED_DEB_REG, MB_LED_DEFAULT);
        diag_lewis_gesw_phy_led_clear_util();
        diag_lewis_gesw_poe_led_off_util();
        break;
    default:
        break;
    }
}

static int diag_sfp_led_util (int sfp_num) {

    /* This function is to control SFP LED on-off */
    uint32_t choice;
    uint32_t g_link_mask[2] = {SFP0_LINK_GREEN, SFP1_LINK_GREEN};
    uint32_t y_link_mask[2] = {SFP0_LINK_AMBER, SFP1_LINK_AMBER};
    uint32_t g_speed_mask[2] = {SFP0_SPEED_GREEN, SFP1_SPEED_GREEN};

    while (1) {
        printf("\n LED SFP port %d utility:\n", sfp_num);
        printf("  0. Toggle port %d green speed LED\n", sfp_num);
        printf("  1. Toggle port %d green link LED\n", sfp_num);
        printf("  2. Toggle port %d ambert LED\n", sfp_num);
        printf("  3. Toggle port %d  LED off\n", sfp_num);
        printf("  4. exit\n");

        choice = gethex_answer("Enter selection:", 0, 0, 4);

        if (choice == SFP_MENU_EXIT) 
            return (PASSED);
        

        switch (choice) {
            case SFP_SPEED_GREEN:
                if (sfp_num == 0) {
                    diag_fpga_reg_write(MB_LED_SFP_REG, g_speed_mask[0]);
                } else {
                    diag_fpga_reg_write(MB_LED_SFP_REG, g_speed_mask[1]);
                }
                break;
            case SFP_LINK_GREEN:
                if (sfp_num == 0) {
                    diag_fpga_reg_write(MB_LED_SFP_REG, g_link_mask[0]);
                } else {
                    diag_fpga_reg_write(MB_LED_SFP_REG, g_link_mask[1]);
                }
                break;
            case SFP_LINK_AMBER:
                if (sfp_num == 0) {
                    diag_fpga_reg_write(MB_LED_SFP_REG, y_link_mask[0]);
                } else {
                    diag_fpga_reg_write(MB_LED_SFP_REG, y_link_mask[1]);
                }
                break;
            case SFP_LED_OFF:
                diag_fpga_reg_write(MB_LED_SFP_REG, LED_OFF);
                break;
            default:
                printf("Not support this item. \n");
                break;
        }
    }
            return (PASSED); 
}


static int diag_rj45_led_util(int port_num) {

    /* This function is to control RJ-45 LED on-off */
    uint32_t choice;
    uint32_t g_link_mask[2] = {RJ450_LINK_GREEN, RJ451_LINK_GREEN};
    uint32_t g_speed_mask[2] = {RJ450_SPEED_GREEN, RJ451_SPEED_GREEN};

    while (1) {
        printf("\n LED RJ-45 Ethernet port %d utility:\n", port_num);
    printf("  0. Toggle port %d green speed LED\n", port_num);
    printf("  1. Toggle port %d green link LED\n", port_num);
    printf("  2. Toggle port %d LED off\n", port_num);
    printf("  3. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 3);

    if (choice == RJ45_MENU_EXIT)
        return PASSED;

    switch (choice) {
        case RJ45_SPEED_GREEN:
            if (port_num == 0) {
                diag_fpga_reg_write(MB_LED_RJ45_ETH_REG, g_speed_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_ETH_REG, g_speed_mask[1]);
            }
            break;
        case RJ45_LINK_GREEN:
            if (port_num == 0) {
                diag_fpga_reg_write(MB_LED_RJ45_ETH_REG, g_link_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_ETH_REG, g_link_mask[1]);
            }
            break;
        case RJ45_LED_OFF:
            diag_fpga_reg_write(MB_LED_RJ45_ETH_REG, LED_OFF);  
            break;
        default:
            printf("Not support this item. \n");
            break;
        }
    }
        return (PASSED);
}


static int diag_blink_rj45_led_util(int port_num) {

    /* This function is to control RJ-45 LED Blinking */
    uint32_t choice;
    uint32_t one_blink_mask[2] = {RJ450_BLK_ONE, RJ451_BLK_ONE}; 
    uint32_t two_blink_mask[2] = {RJ450_BLK_TWO, RJ451_BLK_TWO}; 
    uint32_t three_blink_mask[2] = {RJ450_BLK_THREE, RJ451_BLK_THREE};

    while (1) {
        printf("\n LED RJ-45 Ethernet port %d utility:\n", port_num);
    printf("  0. Turn port %d one blink\n", port_num);
    printf("  1. Turn port %d two blinks\n", port_num);
    printf("  2. Turn port %d three blinks\n", port_num);
    printf("  3. Turn port %d blinks off \n", port_num);
    printf("  4. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 4);

    if (choice == RJ45_BLK_MENU_EXIT)
        return PASSED;

    switch (choice) {
        case RJ45_BLK_ONE:
            if (port_num == 0) {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, one_blink_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, one_blink_mask[1]);
            }
            break;
        case RJ45_BLK_TWO :
            if (port_num ==0) {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, two_blink_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, two_blink_mask[1]);
            }
        case RJ45_BLK_THREE:
            if (port_num ==0) {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, three_blink_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, three_blink_mask[1]);
            }
            break;
        case RJ45_BLK_OFF:
            diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, LED_OFF);
            break;
        default:
            printf("Not support this item. \n");
            break;
        }
    }
        return (PASSED);
}


static int diag_blink_sfp_led_util(int sfp_num) {

    /* This function is to control SFP LED Blinking */
    uint32_t choice;
    uint32_t one_blink_mask[2] = {SFP0_BLK_ONE, SFP1_BLK_ONE}; 
    uint32_t two_blink_mask[2] = {SFP0_BLK_TWO, SFP1_BLK_TWO}; 
    uint32_t three_blink_mask[2] = {SFP0_BLK_THREE, SFP1_BLK_THREE};

    while (1) {
        printf("\n SFP LED port %d utility:\n", sfp_num);
    printf("  0. Turn port %d one blink\n", sfp_num);
    printf("  1. Turn port %d two blinks\n", sfp_num);
    printf("  2. Turn port %d three blinks\n", sfp_num);
    printf("  3. Turn port %d blinks off\n", sfp_num);
    printf("  4. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 4);

    if (choice == SFP_BLK_MENU_EXIT)
        return PASSED;

    switch (choice) {
        case SFP_BLK_ONE:
            if (sfp_num == 0) {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, one_blink_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, one_blink_mask[1]);
            }
            break;
        case SFP_BLK_TWO:
            if (sfp_num == 0) {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, two_blink_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, two_blink_mask[1]);
            }
            break;
        case SFP_BLK_THREE:
            if (sfp_num == 0) {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, three_blink_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, three_blink_mask[1]);
            }
            break;
        case SFP_BLK_OFF:
            diag_fpga_reg_write(MB_LED_RJ45_BLINK_EN_REG, LED_OFF);
            break;
        default:
            printf("Not support this item. \n");
            break;
        }
    }
        return (PASSED);
}



static int diag_psu_led_util(int unit_num) {

    /* This function is to control PSU LED on-off  */
    uint32_t choice;
    uint32_t g_mask[2] = {PSU1_GREEN, PSU2_GREEN};
    uint32_t y_mask[2] = {PSU1_AMBER, PSU2_AMBER};

    while (1) {
        printf("\n Power supply %d LED utility:\n", unit_num);
    printf("  0. Turn %d to green \n", unit_num);
    printf("  1. Turn %d to yellow\n", unit_num);
    printf("  2. Turn %d off\n", unit_num);
    printf("  3. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 3);

    if (choice == PSU_LED_MENU_EXIT)
        return PASSED;

    switch (choice) {
        case PSU_LED_GREEN:
            if (unit_num == 1) {
                diag_fpga_reg_write(MB_LED_PWR_SUP_REG, g_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_PWR_SUP_REG, g_mask[1]);
            }
            break;
        case PSU_LED_AMBER:
            if (unit_num == 1) {
                diag_fpga_reg_write(MB_LED_PWR_SUP_REG, y_mask[0]);
            } else {
                diag_fpga_reg_write(MB_LED_PWR_SUP_REG, y_mask[1]);
            }
            break;
        case PSU_LED_OFF:
            diag_fpga_reg_write(MB_LED_PWR_SUP_REG, LED_OFF);
            break;
        default:
            printf("Not support this item. \n");
            break;
        }
    }
        return (PASSED);
}


static int diag_poe_led_util(void) {

    /* This function is to control POE LED on-off */
    uint32_t choice;

    while (1) {
        printf("\n POE power supply LED utility:\n");
    printf("  0. Turn to green \n");
    printf("  1. Turn to yellow\n");
    printf("  2. Turn off\n");
    printf("  3. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 3);

    if (choice == POE_LED_MENU_EXIT)
        return PASSED;

        switch (choice) {
            case POE_LED_GREEN:
                diag_fpga_reg_write(MB_LED_POE_PWR_SUP_REG, POE_GREEN);
                break;
            case POE_LED_AMBER:
                diag_fpga_reg_write(MB_LED_POE_PWR_SUP_REG, POE_AMBER);
                break;
            case POE_LED_OFF:
                diag_fpga_reg_write(MB_LED_POE_PWR_SUP_REG, LED_OFF);
                break;
        }
    }
        return (PASSED);
}


static int diag_poe_dc_led_util(void) {

    /* This function is to control POE Daughter LED on-off */
    uint32_t choice;

    while (1) {
        printf("\n POE daughter card LED utility:\n");
    printf("  0. Turn to green \n");
    printf("  1. Turn to yellow\n");
    printf("  2. Turn off\n");
    printf("  3. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 3);

    if (choice == POE_DC_LED_MENU_EXIT)
        return PASSED;

        switch (choice) {
            case POE_DC_LED_GREEN:
                diag_fpga_reg_write(MB_LED_POE_DAU_CARD_REG, POE_DC_GREEN);
                break;
            case POE_DC_LED_AMBER:
                diag_fpga_reg_write(MB_LED_POE_DAU_CARD_REG, POE_DC_AMBER);
                break;
            case POE_DC_LED_OFF:
                diag_fpga_reg_write(MB_LED_POE_DAU_CARD_REG, LED_OFF);
                break;
        }
    }
        return (PASSED);
}


static int diag_env_led_util(void ) {

    /* This function is to control Envirement LED on-off */
    uint32_t choice;

    while (1) {
        printf("\n Enviromental LED utility:\n");
    printf("  0. Toogle Temperature LED to Green\n");
    printf("  1. Toogle Temperature LED to Yellow\n");
    printf("  2. Toogle FAN LED to Green\n");
    printf("  3. Toogle FAN LED to Yellow\n");
    printf("  4. Toogle FAN LED to Yellow Blinking\n");
    printf("  5. Toogle LED off\n");
    printf("  6. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 6);

    if (choice == ENV_MENU_EXIT)
        return PASSED;

        switch (choice) {
            case TEMP_LED_GREEN:
                diag_fpga_reg_write(MB_LED_ENV_REG, TEM_LED_GREEN);
                break;
            case TEMP_LED_AMBER:
                diag_fpga_reg_write(MB_LED_ENV_REG, TEM_LED_AMBER);
                break;
            case FANS_LED_GREEN:
                diag_fpga_reg_write(MB_LED_ENV_REG, FAN_LED_GREEN);
                break;
            case FANS_LED_AMBER:
                diag_fpga_reg_write(MB_LED_ENV_REG, FAN_LED_AMBER);
                break;
            case FAN_LED_BLK:
                diag_fpga_reg_write(MB_LED_ENV_REG, FAN_LED_AMBER_BLK);
                break;
            case ENV_LED_OFF:
                diag_fpga_reg_write(MB_LED_ENV_REG, LED_OFF);
                break;
        }
    }
        return (PASSED);
}

static int diag_emmc_led_util(void) {

    /* This function is to control eMMC LED on-off */
    uint32_t choice;

    while (1) {
        printf("\n eMMC flash LED utility:\n");
    printf("  0. Turn to green \n");
    printf("  1. Turn to green blink\n");
    printf("  2. Turn to yellow\n");
    printf("  3. Turn off\n");
    printf("  4. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 4);

    if (choice == EMMC_MENU_EXIT)
        return PASSED;

        switch (choice) {
            case EMMC_LED_GREEN:
                diag_fpga_reg_write(MB_LED_EMMC_REG, EMMC_GREEN);
                break;
            case EMMC_LED_BLK:
                diag_fpga_reg_write(MB_LED_EMMC_REG, EMMC_GREEN_BLK);
                break;
            case EMMC_LED_AMBER:
                diag_fpga_reg_write(MB_LED_EMMC_REG, EMMC_AMBER);
                break;
            case EMMC_LED_OFF:
                diag_fpga_reg_write(MB_LED_EMMC_REG, LED_OFF);
                break;
        }
    }
        return (PASSED);
}


static int diag_m2_led_util(void) {

    /* This function is to control M.2 LED on-ff */
    uint32_t choice;

    while (1) {
        printf("\n M.2 flash LED utility:\n");
    printf("  0. Turn to green \n");
    printf("  1. Turn to green blink\n");
    printf("  2. Turn off\n");
    printf("  3. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 3);

    if (choice == M2_MENU_EXIT)
        return PASSED;

        switch (choice) {
            case M2_LED_GREEN:
                diag_fpga_reg_write(MB_LED_M2_REG, M2_GREEN);
                break;
            case M2_LED_BLK:
                diag_fpga_reg_write(MB_LED_M2_REG, M2_GREEN_BLK);
                break;
            case M2_LED_AMBER:
                diag_fpga_reg_write(MB_LED_M2_REG, LED_OFF);
                break;
        }
    }
        return (PASSED);
}


static int diag_88E1512_led_util(void) {

    /* This function is to control 88E1512 LED on-off */
    uint32_t choice;

    while (1) {
        printf("\n88E1512 LED utility:\n");
    printf("  0. Toggle green speed LED \n");
    printf("  1. Toggle green link LED\n");
    printf("  2. Turn off\n");
    printf("  3. exit\n");

    choice = gethex_answer("Enter selection:", 0, 0, 3);

    if (choice == E1512_MENU_EXIT)
        return PASSED;

        switch (choice) {
            case E1512_SPEED_LED_GREEN:
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, 3);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, 16, PHY1512_SPEED_LED_ON);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0);
                break;
            case E1512_LINK_LED_GREEN:
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, 3);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, 16, PHY1512_LINK_LED_ON);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0);
                break;
            case E1512_LED_OFF:
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, 3);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, 16, PHY1512_SPEED_LED_OFF);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, 3);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID, 16, PHY1512_LINK_LED_OFF);
                diag_smi_reg_write(PHY_DEVICE_NAME, PHY_ID,MRV88E1512_PAGE_ADDRESS_REG, MRV88E1512_REG_PAGE_0);
                break;
        }
    }
        return (PASSED);
}
/*---------------------------------------------------------------
$Log: diag_led_util.c,v $
Revision 1.3  2018/05/15 01:28:16  haohsu
Added individual LED test for Tachi

Revision 1.2  2016/04/20 11:25:26  benchen2
add tachi fru portion

Revision 1.1.2.3  2016/01/11 10:29:05  jimmyya
Add lewis phy & Poe led NC functions

Revision 1.1.2.2  2016/01/08 12:49:09  benchen2
add led log section

$Endlog$
*/

