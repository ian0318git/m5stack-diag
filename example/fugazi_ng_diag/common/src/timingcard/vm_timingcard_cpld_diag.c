/* $Id: vm_timingcard_cpld_diag.c,v 1.2 2015/02/14 12:48:42 kodko Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/timingcard/vm_timingcard_cpld_diag.c,v $
 *------------------------------------------------------------------
 * Filename: vm_timingcard_cpld_diag.c
 *
 * Description: The Timing Card CPLD main source code
 * Author: Kody Ko
 *
 * Copyright (c) 2013-2015 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include "common.h"
#include "types.h"
#include "proto.h"
#include "strings.h"
#include "menu.h"
#include "nvsysvars.h"
#include "error.h"
#include "platform_i2c.h"
#include "i2c_api.h"
#include "common_utils.h"
#include "platform_fru.h"
#include "platform_cookie.h"
#include "vm_timingcard_zl3036x_lib.h"
#include "vm_timingcard_cpld_diag.h"
#include "vm_timingcard_cpld_lib.h"
#include "vm_timingcard_pca9557_lib.h"

#include <stdio.h>
#include <string.h>

/***********************************************************************
 *  Macro Definitions
 ************************************************************************/

/***********************************************************************
 *  Static Functions Declaration
 ************************************************************************/
static long timingcard_cpld_reg_test(void);
static long timingcard_cpld_io_expander_test(void);
static long timingcard_cpld_reg_alter(void);
static long timingcard_cpld_reg_dump(void);
static long cpld_upgrade_firmware(int);
static long reboot_timingcard(void);
static long init_zl3036x_gpio_dir(void);
#ifdef SLOW_UPGRADE
static long cpld_upgrade_from_cpld(void);
#endif
static long cpld_upgrade_from_io_expander(void);
static long speedup_cpld_upgrade_from_cpld(void);

/***********************************************************************
 *  Functions Declaration
 ************************************************************************/
long build_timingcard_cpld_menu(int);
long timingcard_cpld_utility_submenu(int);

/***********************************************************************
 * Extern function prototypes
 ***********************************************************************/
extern int do_all_menu_items(struct menuinfo *);
extern int timingcard_init_seq(void);

/***********************************************************************
 *  Global Variable
 ************************************************************************/
static submenu_xtable_t timingcard_cpld_submenu_tbl[] = {
    { "CPLD utility", (type_t(*)())timingcard_cpld_utility_submenu, 0,
      0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "CPLD Register Test", (type_t(*)())timingcard_cpld_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "CPLD IO EXPANDER Test", (type_t(*)())timingcard_cpld_io_expander_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define TIMINGCARD_CPLD_SUBMENU_TABLE_SZ \
                (sizeof(timingcard_cpld_submenu_tbl)/sizeof(submenu_xtable_t))

/***********************************************************************
 * Primary & secondary submenu items (filled in from xtable)
 ************************************************************************/
static mitem_t timingcard_cpld_primary_items[TIMINGCARD_CPLD_SUBMENU_TABLE_SZ +
                                             MAX_BASE_ITEMS];
static mitem_t timingcard_cpld_secondary_items[TIMINGCARD_CPLD_SUBMENU_TABLE_SZ +
                                               MAX_BASE_ITEMS];

static menuinfo_t timingcard_cpld_main_menu = {
    "Timing Card CPLD Menu",
    0,                        /* mtparam added by init_empty_menu */
    0,                        /* notes missing WICs in combos */
    0,                        /* use generic prompt */
    0,                        /* size (bumped by add_menu_item() */
    timingcard_cpld_primary_items,
};
static menuinfo_t *timingcard_cpld_menup = &timingcard_cpld_main_menu;

/***********************************************************************
 * Timing Card utilities menu on Overlord platform
 ************************************************************************/
static mitem_t timingcard_cpld_util_submenu_table[] = {
    { "Alter CPLD Register", 0, 0, timingcard_cpld_reg_alter,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Dump CPLD Registers", 0, 0, timingcard_cpld_reg_dump,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Speed Up CPLD Upgrade From CPLD Utility", 0, 0, speedup_cpld_upgrade_from_cpld,
      (long *)&zero, 0, (type_t(*)())0, 0 },
#ifdef SLOW_UPGRADE
    { "CPLD Upgrade From CPLD Utility", 0, 0, cpld_upgrade_from_cpld,
      (long *)&zero, 0, (type_t(*)())0, 0 },
#endif
    { "CPLD Upgrade From IO Expander Utility", 0, 0, cpld_upgrade_from_io_expander,
      (long *)&zero, 0, (type_t(*)())0, 0 },
    { "Reboot Timing Card", 0, 0, reboot_timingcard,
      (long *)&zero, 0, (type_t(*)())0, 0 },
};

#define TIMINGCARD_CPLD_UTIL_SUBMENU_TABLE_SZ \
        (sizeof(timingcard_cpld_util_submenu_table)/sizeof(mitem_t))

static menuinfo_t timingcard_cpld_util_subtest_menu = {
    "Timing Card CPLD Utilities Menu",
    0,                                  /* title param */
    0,                                  /* show diag flags */
    0,
    TIMINGCARD_CPLD_UTIL_SUBMENU_TABLE_SZ,
    timingcard_cpld_util_submenu_table,
};

static menuinfo_t *timingcard_cpld_util_submenup = &timingcard_cpld_util_subtest_menu;

static int cpld_gpio_value[7] = {CPLD_GPIO_0, CPLD_GPIO_1, CPLD_GPIO_2,
                                 CPLD_GPIO_3, CPLD_GPIO_4, CPLD_GPIO_5,
                                 CPLD_GPIO_6};

extern unsigned char pof_cpld_fw_array[];
int upgrade_interface = UPGRADE_FROM_CPLD;

/***********************************************************************
 *  Functions
 ************************************************************************/

/**********************************************************************
 *
 * Function: build_timingcard_cpld_menu
 *
 * Description: Build Timing Card CPLD tests and utilities menu.
 *
 * Inputs:  show_menu - FALSE for tests. TRUE for submenu.
 *
 * Outputs: PASSED/FAILED.
 *
 **********************************************************************
 */
long build_timingcard_cpld_menu (int show_menu)
{

    build_primary_submenu(timingcard_cpld_submenu_tbl, TIMINGCARD_CPLD_SUBMENU_TABLE_SZ,
                          "CPLD Main Menu", &timingcard_cpld_menup);
    build_secondary_submenu(timingcard_cpld_submenu_tbl, TIMINGCARD_CPLD_SUBMENU_TABLE_SZ,
                            timingcard_cpld_secondary_items);

    if (show_menu) {
        /* Entered with submenu */
        menu(timingcard_cpld_menup, timingcard_cpld_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(timingcard_cpld_menup);
    }

    return(PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_utility_submenu().
 *
 * This function implements the Timing Card CPLD test/menu
 *
 * Input: menu_option - show menu option
 *
 * Output: PASSED/FAILED.
 *
 **********************************************************************
 */
long timingcard_cpld_utility_submenu (int menu_option)
{
    menu(timingcard_cpld_util_submenup, timingcard_cpld_util_submenu_table, '\0');

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_reg_test
 *
 * Wrapper for CPLD Register test.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_cpld_reg_test (void)
{
    char *tname = "Timing Card CPLD Register";
    char *debug_step1 = "Do the ZL3036X register test; this will clarify if"
                        " the path from Overlord FPGA to timing card is good. "
                        "If it passes then the path from Overlord Intel "
                        "processor to CPLD is good.";
    char *debug_step2 = "If step 1 passes, try to reprogram the CPLD firmware"
                        " with external JTAG header. Reboot timing card and "
                        "re-run the CPLD register test to check if it passes.";
    char *debug_step3 = "If the CPLD register test still fails after step 2,"
                        " then the CPLD might be damaged, try to replace a "
                        "new one (U1).";

    char mb_get_pid[FRU_SIZE] = {0};
    char mb_get_loc[FRU_SIZE] = {0};


    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = VM;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    get_mb_pid(mb_get_pid);
    strcpy(mb_get_loc, "MB-TimingCard");
    platform_fru_table[fru_table_offset].pid_string = (uchar *)mb_get_pid ;
    platform_fru_table[fru_table_offset].location_string = (uchar *)mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("CPLD");

#ifdef ENABLE_TO_USE
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)display_uart_regs_cterr_wrapper,
                        (PFI)display_multiboot);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_margins_cterr_wrapper,
                        (PFV)show_temp_cterr_wrapper);
#endif

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug(debug_step1, debug_step2, debug_step3);

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    if (timingcard_cpld_reg_test_lib() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: init_zl3036x_gpio_dir
 *
 * Wrapper for initialize the ZL3036X GPIO direction.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long init_zl3036x_gpio_dir (void)
{
    /* Set GPIO0 as input (control) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN0, 0)
        == FAILED) {
        return (FAILED);
    }

    /* Set GPIO1 as input (control) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN1, 0)
        == FAILED) {
        return (FAILED);
    }

    /* Set GPIO2 as output (status) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN2, ZL3036X_GPIO_STATUS_MODE_FUNC_0)
        == FAILED) {
        return (FAILED);
    }

    /* Set GPIO3 as output (status) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN3, ZL3036X_GPIO_STATUS_MODE_FUNC_0)
        == FAILED) {
        return (FAILED);
    }

    /* Set GPIO4 as output (status) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN4, ZL3036X_GPIO_STATUS_MODE_FUNC_0)
        == FAILED) {
        return (FAILED);
    }

    /* GPIO 5 6, CPLD now is set as input */
    /* Set GPIO5 as output (status) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN5, ZL3036X_GPIO_STATUS_MODE_FUNC_0)
        == FAILED) {
        return (FAILED);
    }

    /* Set GPIO6 as output (status) in ZL3036X */
    if (zl3036x_set_gpio_dir(ZL3036X_GPIO_FUNCTION_PIN6, ZL3036X_GPIO_STATUS_MODE_FUNC_0)
        == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_io_expander_test
 *
 * Description: This function perform the the IO Expander test that
 *              verifies all the GPIO pins between CPLD and ZL30363.
 *
 * Input : none
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_cpld_io_expander_test (void)
{
    int ix, jx, value;
    char *tname = "Timing Card IO Expander";
    char *debug_step1 = "Do the CPLD register test; this will clarify if the "
                        "path from Overlord Intel processor to timing card "
                        "CPLD and the CPLD firmware are working well";
    char *debug_step2 = "If step 1 passes, do the ZL3036X register test, "
                        "this will clarify if the ZL3036X is good.";
    char *debug_step3 = "If step 1 and step 2 pass, according to the failure "
                        "log, checks if the corresponding GPIO pin connection "
                        "between CPLD IO Expander and ZL3036X is good.";

    char mb_get_pid[FRU_SIZE] = {0};
    char mb_get_loc[FRU_SIZE] = {0};

    /*
     * 1. Subtests of the test function will reuse all variables
     * 2. All variables will be cleared automatically when
     *    entering and leaving each menu item.
     */
    /* Segment 1: PID | Unique_string : slot_info */
    fru_table_offset = VM;
    /* fru_table_offset should be set, otherwise, it will not */
    /* go to enhanced error message format in cterr() */
    /* set fru_table_offset to get the predefine value */
    /* or change mb_pid & mb_loc below */

    get_mb_pid(mb_get_pid);
    strcpy(mb_get_loc, "MB-TimingCard");
    platform_fru_table[fru_table_offset].pid_string = (uchar *)mb_get_pid ;
    platform_fru_table[fru_table_offset].location_string = (uchar *)mb_get_loc;

    /* Segment 2: Test step captured from prpass */
    /* Segment 3: Failure message captured from cterr */

    /* Segment 4: Components used */
    cterr_add_component("CPLD");

#ifdef ENABLE_TO_USE
    /* Segment 5: register and memory dump */
    cterr_add_reg_dump((PFV)display_uart_regs_cterr_wrapper,
                        (PFI)display_multiboot);

    /* Segment 6: Platform Environment initialized here*/
    cterr_add_env_dump((PFV)show_margins_cterr_wrapper,
                        (PFV)show_temp_cterr_wrapper);
#endif

    /* Segment 7: Top 3 Debugging Steps */
    cterr_add_debug(debug_step1, debug_step2, debug_step3);

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    /* Initialize the timging card */
    if (timingcard_init_seq() == FAILED) {
        return (FAILED);
    }

    /* Initialize the ZL3036X GPIO direction. */
    if (init_zl3036x_gpio_dir() == FAILED) {
        return (FAILED);
    }

    /* Do the GPIO 0~6 drive high and low test. */
    for (ix = CPLD_GPIO0; ix <= CPLD_GPIO6; ix++) {

        for (jx = 0; jx < 2; jx++) {
            /* Test GPIO drive low and high */
            if (jx == 0) {
                /* GPIO drive low */
                value = ~cpld_gpio_value[ix];
            } else {
                /* GPIO drive high */
                value = cpld_gpio_value[ix];
            }

            if (ix < CPLD_GPIO2) {
                /* CPLD drive the GPIO high/low */
                if (timingcard_cpld_drive_gpio(value) == FAILED) {
                    cterr('f', 0, "CPLD drive GPIO 0 fails");
                    return (FAILED);
                }

                /* Check the ZL3036X GPIO status */
                if (zl3036x_check_gpio_value(ix, ZL3036X_GPIO_PIN_IN_6_0,
                                             value, jx)
                    == FAILED) {
                    return (FAILED);
                }
            } else {
                /* ZL3036X set the GPIO enable */
                if (zl3036x_set_gpio_value(ZL3036X_GPIO_OUT_EN_6_0, cpld_gpio_value[ix])
                    == FAILED) {
                    return (FAILED);
                }

                /* ZL3036X drive the GPIO high/low */
                if (zl3036x_set_gpio_value(ZL3036X_GPIO_PIN_OUT_6_0, value)
                    == FAILED) {
                    return (FAILED);
                }

                /* Check the CPLD GPIO status */
                if (timingcard_cpld_check_gpio_val(ix, value, jx) == FAILED) {
                    return (FAILED);
                }
            }
        }
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: timingcard_cpld_reg_alter
 *
 * Wrapper for CPLD Register write utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_cpld_reg_alter (void)
{
    return util_oir_cpld_reg_write();
}

/**********************************************************************
 *
 * Function: timingcard_cpld_reg_dump
 *
 * Wrapper for CPLD Register Read utility.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long timingcard_cpld_reg_dump (void)
{
    return util_oir_cpld_reg_read();
}

/**********************************************************************
 *
 * Function: cpld_upgrade_from_cpld
 *
 * This function upgrades the NGVM Timing Card CPLD firmware from CPLD.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
#ifdef SLOW_UPGRADE
static long cpld_upgrade_from_cpld (void)
{
    return cpld_upgrade_firmware(UPGRADE_FROM_CPLD);
}
#endif

/**********************************************************************
 *
 * Function: cpld_upgrade_from_io_expander
 *
 * This function upgrades the NGVM Timing Card CPLD firmware from IO Expander.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long cpld_upgrade_from_io_expander (void)
{
    return cpld_upgrade_firmware(UPGRADE_FROM_IO_EXPANDER);
}

/**********************************************************************
 *
 * Function: speedup_cpld_upgrade_from_cpld
 *
 * This function speed up upgrades the NGVM Timing Card CPLD firmware from
 * IO Expander by different upgrade source code.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long speedup_cpld_upgrade_from_cpld (void)
{
    return cpld_upgrade_firmware(SPEED_UP_UPGRADE_FROM_CPLD);
}

/**********************************************************************
 *
 * Function: cpld_upgrade_firmware
 *
 * Description: This function upgrades the NGVM Timing Card CPLD firmware.
 *
 * Input : interface - upgrade interface
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long cpld_upgrade_firmware (int interface)
{
    uchar cpld_buf;

    /* Assigned the upgrade interface. */
    upgrade_interface = interface;

    /* Get current CPLD firmware version */
    if (timingcard_cpld_reg_read_lib(CPLD_VERSION, &cpld_buf) == FAILED) {
        printf("Current CPLD firmware is not responding.\n");
    } else {
        /* Display the CPLD firmware version */
        printf("Current CPLD firmware version is %#.2x\n", cpld_buf & 0xf);
    }

    printf("Upgrade new CPLD firmware version\n");

    printf("\n WARNING: Do not power off system during CPLD upgrade\n");

    if (getc_answer(" Do you want to start CPLD upgrade (Y/N)",
                     "yn", 'y')
        == 'y') {
        if (interface != SPEED_UP_UPGRADE_FROM_CPLD) {
            if (interface == UPGRADE_FROM_CPLD) {
                /* Before upgrade the CPLD firmware, needs to turn on the
                 * JTAG_ON bit. */
                if (timingcard_cpld_jtag_ctl(TRUE) == FAILED) {
                    return (FAILED);
                }
            }

            if (interface == UPGRADE_FROM_IO_EXPANDER) {
                /* Before upgrade the CPLD firmware, needs to initialize the
                 * IO Expander */
                if (timingcard_pca9557_init() == FAILED) {
                    return (FAILED);
                }

                /* Needs to turn off the JTAG_ON bit in order to use IO expander
                 * to upgrade the firmware. */
                cpld_buf = 0;
                if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
                    printf("Current CPLD firmware is not responding.\n");
                }
            }

            /* Do the CPLD firmware upgrade. */
            if (max2_cpld_program() == FAILED) {
                return (FAILED);
            }

            if (interface == UPGRADE_FROM_CPLD) {
                /* After upgrade the CPLD firmware, needs to turn off the
                 * JTAG_ON bit. */
                if (timingcard_cpld_jtag_ctl(FALSE) == FAILED) {
                    return (FAILED);
                }
            }
        } else {
            /* Only CPLD firmware version is at least greater than 3
             * supports this feature! */
            if ((cpld_buf & 0xf) < 0x4) {
                printf("Only CPLD firmware version is at least greater than 3 "
                        "supports this feature!\n");
                return (PASSED);
            }

            /* Upgrade the firmware thru CPLD */
            upgrade_interface = UPGRADE_FROM_CPLD;

            /* Do program the POF firmware file. */
            /* Before upgrade the CPLD firmware, needs to turn on the
             * JTAG_ON bit. */
            if (timingcard_cpld_jtag_ctl(TRUE) == FAILED) {
                return (FAILED);
            }

            if (timingcard_simply_program_cpld(pof_cpld_fw_array) == FAILED) {
                return (FAILED);
            }

#ifdef UPGRADE_FROM_IO_EXPANDER
            /* Upgrade the firmware thru CPLD */
            upgrade_interface = UPGRADE_FROM_IO_EXPANDER;
            /* Needs to turn off the JTAG_ON bit in order to use IO expander
             * to upgrade the firmware. */
            cpld_buf = 0;
            if (timingcard_cpld_reg_write_lib(CPLD_JTAG_CTL, cpld_buf) == FAILED) {
                return (FAILED);
            }

            /* Upgrade the CPLD firmware from simpler source code. */
            if (timingcard_pca9557_init() == FAILED) {
                return (FAILED);
            }

            if (timingcard_pca9557_program_cpld(pof_cpld_fw_array) == FAILED) {
                return (FAILED);
            }
#endif

            /* After upgrade the CPLD firmware, needs to turn off the
             * JTAG_ON bit. */
            if (timingcard_cpld_jtag_ctl(FALSE) == FAILED) {
                return (FAILED);
            }
        }
    }

    /* Do power cycle CPLD */
    if (timingcard_pca9557_power_cycle_cpld() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/**********************************************************************
 *
 * Function: reboot_timingcard
 *
 * Description: This function reboots NGVM Timing Card.
 *
 * Input : None
 *
 * Output: PASSED/FAILED
 *
 **********************************************************************
 */
static long reboot_timingcard (void)
{
    /* Do power cycle CPLD */
    if (timingcard_pca9557_power_cycle_cpld() == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

/******** History ********/
/*------------------------------------------------------------------------------
 * $Log: vm_timingcard_cpld_diag.c,v $
 * Revision 1.2  2015/02/14 12:48:42  kodko
 * Collapse timing card branch code into main trunk.
 *
 * Revision 1.1.2.8  2014/04/22 06:06:02  kodko
 * Support ZL30361 SKU.
 *
 * Revision 1.1.2.7  2014/03/31 02:09:43  kodko
 * Check the CPLD firmware to see if it can support the speed up upgrade.
 *
 * Revision 1.1.2.6  2014/03/19 07:13:50  kodko
 * Speed up the CPLD firmware upgrade time under 2 minutes.
 *
 * Revision 1.1.2.5  2014/03/10 08:00:10  kodko
 * Remove redundant code.
 *
 * Revision 1.1.2.4  2014/03/07 07:39:58  kodko
 * Mofify for speed up CPLD upgrade firmware by CPLD.
 *
 * Revision 1.1.2.3  2014/02/24 09:02:43  kodko
 * Initial bring up for CPLD firmware upgrade by CPLD it-self and IO Exapnder.
 *
 * Revision 1.1.2.2  2014/01/13 10:33:45  kodko
 * Initial bring up for timing card.
 *
 * Revision 1.1.2.1  2013/12/25 09:03:05  kodko
 * Initial check-in for NGVM  Timing Card.
 *
 *------------------------------------------------------------------------------
 * $Endlog $
 *------------------------------------------------------------------------------
 */
