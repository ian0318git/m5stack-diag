/* $Id: skye_led.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_led.c,v $
 *------------------------------------------------------------------------------
 * 
 * skye_led.c: File for Skye all LED test and utilities.
 *
 * Aug. 14, 2013 - palin2 created for ShrinkRay.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <gxio/common.h>
#include <gxio/gpio.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "types.h" 
#include "queryflags.h" 
#include "nvmonvars.h"
#include "skye_led.h"
#include "diag_mv1514_test.h"

/*******************************************************************************
 *                           Function Prototypes
 *******************************************************************************
 */
int skye_ge_led_ctrl(int);

/*******************************************************************************
 *                                Externs
 *******************************************************************************
 */
extern int szalinski_get_led_status(void);
extern int szalinski_led_ctrl(int);
extern int led_function_on(int);
extern int led_blink_function_on(int);
extern int led_function_off(int);
extern boolean check_cpu(int);

/*******************************************************************************
 *                                Globals
 *******************************************************************************
 */

/*******************************************************************************
 *                                 Menu
 *******************************************************************************
 */

/*
 * Skye GE LED utility SubMenu
 */
submenu_xtable_t skye_ge_led_tbl[] = {
    {"Turn GE Link/Act LED ON Green Solid",   (PFT)skye_ge_led_ctrl,
     GE_LINK_GREEN_SOLID,                     0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn GE Link/Act LED ON Green Blink",   (PFT)skye_ge_led_ctrl,
     GE_LINK_GREEN_BLINK,                     0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn GE Link/Act LED OFF",              (PFT)skye_ge_led_ctrl,
     GE_LINK_OFF,                             0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn GE Speed LED ON Green Solid",      (PFT)skye_ge_led_ctrl,
     GE_SPEED_GREEN_SOLID,                    0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn GE Speed LED ON Amber Solid",      (PFT)skye_ge_led_ctrl,
     GE_SPEED_AMBER_SOLID,                    0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn GE Speed LED OFF",                 (PFT)skye_ge_led_ctrl,
     GE_SPEED_OFF,                            0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
};

#define SHRINKRAY_GE_LED_TBL_SZ \
        (sizeof(skye_ge_led_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t skye_ge_led_pri[SHRINKRAY_GE_LED_TBL_SZ + MAX_BASE_ITEMS];
static mitem_t skye_ge_led_sec[SHRINKRAY_GE_LED_TBL_SZ + MAX_BASE_ITEMS];

static struct menuinfo skye_ge_led = {
    "%s utility SubMenu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    skye_ge_led_pri,
};
static struct menuinfo *skye_ge_led_p = &skye_ge_led;

/*
 * Skye FPGA(Szalinski) LED utility SubMenu
 */
submenu_xtable_t skye_fpga_led_tbl[] = {
    {"Get Szalinski LEDs status",             (PFT)szalinski_get_led_status,
     0,                                       0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn System LED Blinking in Green",     (PFT)szalinski_led_ctrl,
     SYS_LED_GREEN_BLINK,                     0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn System LED Blinking in Yellow",    (PFT)szalinski_led_ctrl,
     SYS_LED_YELLOW_BLINK,                    0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn System LED Solid in Green",        (PFT)szalinski_led_ctrl,
     SYS_LED_GREEN_SOLID,                     0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn System LED Solid in Yellow",       (PFT)szalinski_led_ctrl,
     SYS_LED_YELLOW_SOLID,                    0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn System LED OFF",                   (PFT)szalinski_led_ctrl,
     SYS_LED_TURN_OFF,                        0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn eUSB LED ON",                      (PFT)szalinski_led_ctrl,
     EUSB_LED_TURN_ON,                        0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Turn eUSB LED OFF",                     (PFT)szalinski_led_ctrl,
     EUSB_LED_TURN_OFF,                       0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
    {"Disable eUSB LED test",                 (PFT)szalinski_led_ctrl,
     EUSB_LED_TEST_DIS,                       0,          (type_t(*)())0,
     0,                                       (PFT)0,     0},
};

#define SHRINKRAY_FPGA_LED_TBL_SZ \
        (sizeof(skye_fpga_led_tbl) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t skye_fpga_led_pri[SHRINKRAY_FPGA_LED_TBL_SZ + MAX_BASE_ITEMS];
static mitem_t skye_fpga_led_sec[SHRINKRAY_FPGA_LED_TBL_SZ + MAX_BASE_ITEMS];

static struct menuinfo skye_fpga_led = {
    "%s utility SubMenu",	/* title */
    0,				/* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	/* shows major flags */
    0,				/* generic prompt */
    0,				/* size -- bumped by add_menu_item() */
    skye_fpga_led_pri,
};
static struct menuinfo *skye_fpga_led_p = &skye_fpga_led;


/*******************************************************************************
 *
 * Function   : skye_ge_led_ctrl
 * Description: Function to control Skye GE LEDs.
 * Inputs     : opt - reserved for future use
 * Outputs    : None
 *
 *******************************************************************************
 */
int
skye_ge_led_ctrl (int opt)
{
    /* Based on Shrinkray HFS, these two front panel GE LEDs
     * are lighted by Szalinski, and set GEPHY LED signals
     * by CPU0 through MDC/MDIO.
     */

    /* So will update this function after get MDC/MDIO control sub-functions. */
    printf("%s: Need to be updated after get MDC/MDIO access functions.\n",
           __FUNCTION__);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_ge_led_util
 * Description: Entry function for Skye GE LED utilities.
 * Inputs     : menu_opt - reserved for future use
 * Outputs    : None
 *
 *******************************************************************************
 */
void
skye_ge_led_util (int menu_opt)
{
    build_primary_submenu(skye_ge_led_tbl, SHRINKRAY_GE_LED_TBL_SZ,
                          "Skye GE LED", &skye_ge_led_p);
    build_secondary_submenu(skye_ge_led_tbl, SHRINKRAY_GE_LED_TBL_SZ,
                            skye_ge_led_sec);

    menu(&skye_ge_led, skye_ge_led_sec, 0);
}

/*******************************************************************************
 *
 * Function   : skye_fpga_led_util
 * Description: Function for Skye FPGA LED utilities.
 * Inputs     : menu_opt - reserved for future use
 * Outputs    : None
 *
 *******************************************************************************
 */
void
skye_fpga_led_util (int menu_opt)
{
    build_primary_submenu(skye_fpga_led_tbl, SHRINKRAY_FPGA_LED_TBL_SZ,
                          "Skye FPGA LED", &skye_fpga_led_p);
    build_secondary_submenu(skye_fpga_led_tbl, SHRINKRAY_FPGA_LED_TBL_SZ,
                            skye_fpga_led_sec);

    menu(&skye_fpga_led, skye_fpga_led_sec, 0);
}

/*******************************************************************************
 *
 * Function   : skye_led_solid_ctrl
 * Description: Function for Skye LED solid control.
 * Inputs     : cpu_id - CPU 0 & CPU 1
 *              led_opt - ALL_LED, GREEN_LED, AMBER_LED
 *              ctrl_opt - LED_ON, LED_OFF
 * Outputs    : PASSED /FAILED
 *
 *******************************************************************************
 */
static int
skye_led_solid_ctrl (int cpu_id, int led_opt, int ctrl_opt)
{
    /* Control Front Panel GE LED(s) */
    if (ctrl_opt == LED_OFF) {
        led_function_off(ALL_LED);
    } else if (ctrl_opt == LED_ON) {
        if (led_opt == GREEN_LED) {
            /* Turn ON GE Link & Speed in GREEN */
            led_function_on(ALL_GREEN);
        } else if (led_opt == AMBER_LED) {
            /* Turn ON GE Speed in AMBER */
            led_function_on(LED0);
        } else {
            printf("Unknown LED light type (0x%02X).\n", led_opt);
            return (FAILED);
        }
    } else {
        printf("Unknown LED control type (0x%02X).\n", ctrl_opt);
        return (FAILED);
    }

    if (cpu_id == SLAVE_CPU) {
        if (ctrl_opt == LED_ON) {
            msleep(2000);
        }

        return (PASSED);
    }

    /* Control Szalinski LED(s) */
    if (ctrl_opt == LED_OFF) {
        /* Turn OFF System LED */
        szalinski_led_ctrl(SYS_LED_TURN_OFF);

        /* Turn OFF eUSB LED */
        szalinski_led_ctrl(EUSB_LED_TURN_OFF);
    } else if (ctrl_opt == LED_ON) {
        if (led_opt == GREEN_LED) {
            /* Turn ON System LED in Green */
            szalinski_led_ctrl(SYS_LED_GREEN_SOLID);

            /* Turn ON eUSB LED */
            szalinski_led_ctrl(EUSB_LED_TURN_ON);

            msleep(2000);
        } else if (led_opt == AMBER_LED) {
            /* Turn ON System LED in Amber */
            szalinski_led_ctrl(SYS_LED_YELLOW_SOLID);

            msleep(2000);
        } else {
            printf("Unknown LED light type (0x%02X).\n", led_opt);
            return (FAILED);
        }
    } else {
        printf("Unknown LED control type (0x%02X).\n", ctrl_opt);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_led_blink_ctrl
 * Description: Entry function for Skye LED b control.
 * Inputs     : cpu_id - CPU 0 & CPU 1
 *              led_opt - ALL_LED, GREEN_LED, AMBER_LED
 *              ctrl_opt - LED_BLINK, LED_OFF
 * Outputs    : PASSED /FAILED
 *
 *******************************************************************************
 */
int
skye_led_blink_ctrl (int cpu_id, int led_opt, int ctrl_opt)
{
    /* Control Front Panel GE LED(s) */
    if (ctrl_opt == LED_OFF) {
        led_function_off(ALL_LED);
    } else if (ctrl_opt == LED_BLINK) {
        if (led_opt == GREEN_LED) {
            /* Turn ON GE Link & Speed in GREEN */
            led_blink_function_on(LED1);
            led_blink_function_on(LED2);
            led_blink_function_on(ALL_GREEN);
        } else if (led_opt == AMBER_LED) {
            /* Turn ON GE Speed in AMBER */
            led_blink_function_on(LED0);
        } else {
            printf("Unknown LED light type (0x%02X).\n", led_opt);
            return (FAILED);
        }
    } else {
        printf("Unknown LED control type (0x%02X).\n", ctrl_opt);
        return (FAILED);
    }

    if (cpu_id == SLAVE_CPU) {
        if ((ctrl_opt == LED_ON) || (ctrl_opt == LED_BLINK)) {
            msleep(2000);
        }
        return (PASSED);
    }

    /* Control Szalinski LED(s) */
    if (ctrl_opt == LED_OFF) {
        /* Turn OFF System LED */
        szalinski_led_ctrl(SYS_LED_TURN_OFF);

        /* Turn OFF eUSB LED */
        szalinski_led_ctrl(EUSB_LED_TURN_OFF);
    } else if (ctrl_opt == LED_BLINK) {
        if (led_opt == GREEN_LED) {
            /* Turn ON System LED in Green */
            szalinski_led_ctrl(SYS_LED_GREEN_BLINK);

            msleep(2000);
        } else if (led_opt == AMBER_LED) {
            /* Turn ON System LED in Amber */
            szalinski_led_ctrl(SYS_LED_YELLOW_BLINK);

            msleep(2000);
        } else {
            printf("Unknown LED light type (0x%02X).\n", led_opt);
            return (FAILED);
        }
    } else {
        printf("Unknown LED control type (0x%02X).\n", ctrl_opt);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : skye_led_test
 * Description: Entry function for Skye LED test.
 * Inputs     : opt - future use
 * Outputs    : PASSED /FAILED
 *
 *******************************************************************************
 */
int 
skye_led_test (int opt)
{
    int cpu_id = 0;

    if (check_cpu(0)==TRUE) {
        cpu_id = 0;
    } else {
        cpu_id = 1;
    }

    testname("CPU%d LED", cpu_id);

    /* Display LED test message */
    printf("\nExercise each LED in the following sequence:\n");
    printf("Off, Green On, Green Blink, Amber On, Amber Blink\n");
    printf("!!! This is a visual test. User needs to decide whether the LED "
           "works correctly or not !!!\n");

    /* Turn all LED(s) OFF */
    prpass(testpass, "ALL LED OFF");
    skye_led_solid_ctrl(cpu_id, ALL_LED, LED_OFF);

    /* Turn all Green LED(s) ON */
    prpass(testpass, "GREEN LED ON");
    skye_led_solid_ctrl(cpu_id, GREEN_LED, LED_ON);
    skye_led_solid_ctrl(cpu_id, GREEN_LED, LED_OFF);
    /* Turn all Green LED(s) BLINK */
    prpass(testpass, "GREEN LED BLINK");
    skye_led_blink_ctrl(cpu_id, GREEN_LED, LED_BLINK);
    skye_led_blink_ctrl(cpu_id, GREEN_LED, LED_OFF);
    /* Turn all Amber LED(s) ON */
    prpass(testpass, "AMBER LED ON");
    skye_led_solid_ctrl(cpu_id, AMBER_LED, LED_ON);
    skye_led_solid_ctrl(cpu_id, AMBER_LED, LED_OFF);
    /* Turn all Amber LED(s) BLINK */
    prpass(testpass, "AMBER LED BLINK");
    skye_led_blink_ctrl(cpu_id, AMBER_LED, LED_BLINK);
    skye_led_solid_ctrl(cpu_id, ALL_LED, LED_OFF);

    led_function_off (ALL_LED);
    szalinski_led_ctrl(EUSB_LED_TEST_DIS);


    return (PASSED);
}


/*----------------------------------------------------
$Log: skye_led.c,v $
Revision 1.2  2015/05/25 03:59:16  steja
Add Support Skye SM

Revision 1.1.4.4  2015/05/11 13:45:46  steja
Code clean up <CSCuu14285>

Revision 1.1.4.3  2015/04/30 08:33:54  steja
Clean up code

Revision 1.1.4.2  2015/04/29 11:36:36  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:55  palin2
Initial check-in Skye module side Diag code.

------------------------------------------------------
skye_led.c:
Revision 1.2.8.1  2014/06/06 11:54:21  steja
Add Shrinkray LED Test

Revision 1.2  2014/02/27 15:01:48  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.4.2  2013/09/13 07:00:09  palin2
Initial check-in ShrinkRay SM side Diag code.

Revision 1.1.2.2  2013/08/15 02:16:32  palin2
Add comment in GE LED control utility.
Based on HFS, GE LEDs are lighted by Szalinski but set by CPU0 through
MDC/MDIO. So add comment here and will update later after get MDC/MDIO
access function.

Revision 1.1.2.1  2013/08/14 11:36:09  palin2
1. Add ShrinkRay GE LED utility High-level code.
2. Initial check-in "skye_led.c" for ShrinkRay LED related test and utilities.
3. Move Szalinski LED utility to "skye_led.c".

------------------------------------------------------
$Endlog$
*/

