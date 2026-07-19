/*
 *------------------------------------------------------------------
 *
 * diag_lte_test.c - SWI LTE EM74XX/WP760X Function
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
#include <unistd.h>
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
#include "plat_defs.h"
#include "setjmps.h"
#include "proto.h"
#include "gpio.h"
#include "plat_defs.h"
#include "diag_lte_test.h"
#include "diag_lte_telit_lib.h"
#include "diag_lte_telit_util.h"
#include "highrise_cpld_lib.h"

static int diag_lte_telit_modem_detect_test(int);
static int diag_lte_telit_sim_test(int);
static int diag_lte_telit_usb2p0_test(int);
static int diag_lte_telit_w_disable_pin_test(int);
static int diag_lte_telit_modem_pwron_pin_test(int);
static int diag_lte_telit_gps_antenna_test(int);
static int diag_lte_telit_main_rssi_test(int);
static int diag_lte_telit_div_rssi_test(int);
static int diag_lte_telit_shdn_test(int);
extern int do_all_menu_items(struct menuinfo *);

static submenu_xtable_t diag_lte_table[] = {
    {"Modem Detection Test", 
     (type_t(*)())diag_lte_telit_modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 0 Card Test", 
     (type_t(*)())diag_lte_telit_sim_test, SIM0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 1 Card Test", 
     (type_t(*)())diag_lte_telit_sim_test, SIM1,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB 2.0 Detection Test", 
     (type_t(*)())diag_lte_telit_usb2p0_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"W_DISABLE Pin Test", 
     (type_t(*)())diag_lte_telit_w_disable_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Shutdown Indicator Pin Test", 
     (type_t(*)())diag_lte_telit_shdn_test, TRUE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem GPS Antenna", 
     (type_t(*)())diag_lte_telit_gps_antenna_test, TRUE,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())diag_lte_telit_has_dedicated_gps_antenna, 0,
     (type_t(*)())0, 0},
    {"Modem Main RSSI Test", 
     (type_t(*)())diag_lte_telit_main_rssi_test, LTE_ANTENNA_CON_0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DIV RSSI Test", 
     (type_t(*)())diag_lte_telit_div_rssi_test, LTE_ANTENNA_CON_0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Main #1 RSSI Test", 
     (type_t(*)())diag_lte_telit_main_rssi_test, LTE_ANTENNA_CON_1,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())diag_lte_telit_has_2_rssi_antenna, 0, (type_t(*)())0, 0},
    {"Modem DIV #1 RSSI Test", 
     (type_t(*)())diag_lte_telit_div_rssi_test, LTE_ANTENNA_CON_1,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())diag_lte_telit_has_2_rssi_antenna, 0, (type_t(*)())0, 0},
    {"Modem_Power_ON Pin Test", 
     (type_t(*)())diag_lte_telit_modem_pwron_pin_test, TRUE,
     MF_CONTINUOUS | MF_HIDDEN_EXE | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Utilities", 
     (type_t(*)())diag_lte_telit_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define DIAG_LTE_TABLE_SZ \
        (sizeof(diag_lte_table) / sizeof(submenu_xtable_t))


static mitem_t diag_lte_pri_test_items[DIAG_LTE_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t diag_lte_sec_test_items[DIAG_LTE_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t diag_lte_test_menu = {
    "LTE Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    diag_lte_pri_test_items,
};
static menuinfo_t *diag_lte_test_menup = &diag_lte_test_menu;

/*******************************************************************************
 * Function   : diag_lte_telit_main
 * Description: Main Entry point for LTE test
 * Inputs     : Need to test all menu or not
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_lte_telit_main (boolean mb_temp_test_items_executed)
{
    int modem_found = 0;
    int modem_uport = 0;
    int ret = FAILED;
    int timeout, reset = 0;
    unsigned long val;
    int modem_active = FALSE;

    /* Store all USB port info that LTE modem might use from platform*/
    diag_lte_telit_store_usb_devinfo();

    /* Enable RF because default CPLD will turn RF off */
    if (diag_lte_telit_w_disable1_ctrl(DISABLE) != PASSED) {
        cterr('f', 0, "Failed to de-assert WDISABLE1# pin.");
        goto __exit;
    }
    /* Supply VBATT */
    if (diag_lte_telit_modem_pwron_pin_ctrl(HIGH) != PASSED) {
        cterr('f', 0, "Failed to assert power pin.");
        goto __exit;
    }

    /* Base on LM940/960 product spec., the initialization takes 30s, but actually 
     * it need much more time within many Telit modem units, so we need to extend the timeout
     * of waiting for modem power up*/
    timeout = 8*MODEM_LM9X0_PWR_ON_DELAY;

__wait_lte_up:
    /* HW should finished LTE power up sequence successfully */
    while (timeout--) {
        if (hr_cpld_reg_read_32(HR_CPLD_MODEM_STATUS, &val) != PASSED) {
            cterr('f', 0, "Failed to read HR_CPLD_MODEM_STATUS!");
            goto __exit;
        } else {
            if (val & HR_CPLD_MODEM_STA_ACTIVE) {
                modem_active = TRUE;
                break;
            } else {
                fprintf(stdout, "Waiting for modem to be active, timeout in %ds ...", timeout);
                fflush(stdout);
                sleep(1);
                fprintf(stdout, "\r\033[K");
            }
        }
    }
    
    /* Add work around here: 
     * Trying modem HW reset if the modem dropped by default
     * */
    if (!modem_active && !reset) {
        fprintf(stdout, "Modem dropped, trying HW reset!\n");
        /* Try modem HW reset*/
        if (diag_lte_telit_modem_reset() == FAILED) {
            cterr('f', 0, "Failed to reset modem!");
            goto __exit;
        } else {
            timeout = 8*MODEM_LM9X0_PWR_ON_DELAY;
            reset = TRUE;
            goto __wait_lte_up;
        }
    } else {
        if (!modem_active) {
            cterr('f', 0, "Modem cannot get into active state");
            goto __exit;
        }
    }

    /*Detect LTE modem*/
    diag_lte_telit_modem_searching(&modem_found, &modem_uport);
    if (modem_found != TRUE) {
        cterr('f', 0, "LTE modem is not detected");
        goto __exit;
    } else {
        if (USB3P0 == modem_uport) {
            printf("LTE modem is found on usb3.0 port\n");
        } else {
            printf("LTE modem is found on usb2.0 port\n");
        }
        diag_lte_telit_set_current_usb_port(modem_uport);
    }

    /* Detect LTE modem type */
    diag_lte_telit_set_modem_sku();

    /* Set modem in operation mode
     * Note: the modem sometimes is in abnormal test mode when it cannot be set back to the 
     * operation mode. In this case, it needs to config the modem with the work around method*/
    if (diag_lte_telit_set_testmode(OPERATION_MODE) != PASSED) {
        fprintf(stdout, "FW known issue: if cannot force the modem in operation mode, need to try\n");
        fprintf(stdout, "to try one of the below methods with the AT command utility manually!\n");
        fprintf(stdout, "Please try method 1 first, as method 2 may cause modem reboot!\n");
        fprintf(stdout, "Method 1: AT#TESTMODE=\"TM\"\n");
        fprintf(stdout, "          AT#TESTMODE=\"OM\"\n");
        fprintf(stdout, "Method 2: AT#TESTMODE=\"ESC\"\n");
    }

    build_primary_submenu(diag_lte_table, DIAG_LTE_TABLE_SZ, 
                         "LTE(Telit)", &diag_lte_test_menup);

    build_secondary_submenu(diag_lte_table, DIAG_LTE_TABLE_SZ,
                            diag_lte_sec_test_items);
    
    if (mb_temp_test_items_executed) {
        do_all_menu_items(diag_lte_test_menup);
    } else {
        menu(&diag_lte_test_menu, diag_lte_sec_test_items, 0);
    }
    ret = PASSED;
__exit:   
    return (ret);
}

/*******************************************************************************
 * Function   : diag_lte_telit_modem_detect_test 
 * Description: To detect LTE Telit modem for Telit LTE
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_modem_detect_test (int input)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;
    int modem_uport = 0;
    int modem_found_3p0 = FALSE;

    testname("Modem USB3.0 Detection");
    printf("Start LTE modem USB3.0 detection test\n");
    diag_lte_telit_get_current_usb_port(&modem_uport);
    if (USB3P0 != modem_uport) {
        printf("Switch LTE modem to USB3.0 mode ...\n");
        if (diag_lte_telit_config_modem_to_usb3p0() != PASSED) {
            cterr('f', 0, "Switch LTE modem to USB3.0 mode failed\n");
            return (FAILED);
        }
    } else {
        modem_found_3p0 = diag_lte_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);
        if (!modem_found_3p0) {
            cterr('f', 0, "Failed to search LTE modem on USB3.0 bus\n");
            return (FAILED);
        }
    }

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }
    
    rc = diag_lte_telit_obj_p->callin_fvt->modem_detection_test(
                                           (dev_object_t *)&diag_lte_telit_obj);
    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection failed");
    }
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    return (rc);
}

/*******************************************************************************
 * Function   : diag_lte_telit_sim_test 
 * Description: To detect LTE SIM 0 card for LTE
 * Inputs     : test_sim - SIM0 or SIM1
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_sim_test (int test_sim)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;

    testname("SIM slot %d", test_sim);

    /* Check whether SIM card is detected by GPIO expander */
    if (diag_lte_telit_gpio_sim_card_detect(test_sim) != TRUE) {
        cterr('f', 0, "SIM card is not detected by GPIO expander");
        return (FAILED);
    }

    /* Read SIM card PIN request status through AT command to check
     * whether modem can access SIM card or not */
    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "SIM slot %d test", test_sim);
    rc = diag_lte_telit_obj_p->callin_fvt->sim_detect_test((dev_object_t *)
                                                           &diag_lte_telit_obj,
                                                           test_sim);
    if (rc != PASSED) {
        cterr('f', 0, "SIM slot %d test fails", test_sim);
    }

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (rc);
}

/*******************************************************************************
 * Function   : diag_lte_telit_usb2p0_test 
 * Description: Run USB enumeration in USB 2.0 mode from the host
 * Inputs     : input - not used 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_usb2p0_test (int input)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;
    int modem_uport = 0;
    int modem_found_2p0 = FALSE;

    testname("Modem USB2.0 Detection");

    printf("Start LTE modem USB2.0 detection test\n");
    diag_lte_telit_get_current_usb_port(&modem_uport);
    if (USB2P0 != modem_uport) {
        printf("Switch LTE modem to USB2.0 mode ...\n");
        if (diag_lte_telit_config_modem_to_usb2p0() != PASSED) {
            cterr('f', 0, "Switch LTE modem to USB2.0 mode failed\n");
            return (FAILED);
        }
    } else {
        modem_found_2p0 = diag_lte_telit_usb_is_found(USB2P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);
        if (!modem_found_2p0) {
            cterr('f', 0, "Failed to search LTE modem on USB2.0 bus\n");
            return (FAILED);
        }
    }

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }
    
    rc = diag_lte_telit_obj_p->callin_fvt->modem_detection_test(
                                           (dev_object_t *)&diag_lte_telit_obj);
    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection failed");
    }
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    /* Switch back to USB3.0 mode*/
    printf("Switch LTE modem back to USB3.0 mode\n");
    if (diag_lte_telit_config_modem_to_usb3p0() != PASSED) {
        cterr('f', 0, "LTE modem switch back to USB3.0 mode failed\n");
        return (FAILED);
    }

    return (rc);
}

/*******************************************************************************
 * Function   : diag_lte_telit_w_disable_pin_test 
 * Description: Function to test W_DISABLE1# pin
 *              Modem will enter LPM(Low Power Mode) if W_DISABLE1# pin is
 *              asserted
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_w_disable_pin_test (int input)
{
    int lpm_rc = FALSE;
    int om_rc = FALSE;
    int set_psav_rc = FAILED;
    int ix;
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;

    testname("W_DISABLE pin");
    prpass(testpass, "W_DISABLE pin");
    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    /* W_DISABLE1# pin is used to switch modem between operation mode and power
     * saving mode. And the modem power saving mode can be configured as LPM
     * (Low Power Mode), Power Saving Mode, DG(Dying Gasp) or no event to be
     * performed. In this test, we configured modem power saving mode as LPM */
    /* Check whether the modem power saving modem is configured as LPM */

    prpass(testpass, "Set modem power saving configuration...");
    set_psav_rc = diag_lte_telit_obj_p->callin_fvt->
                                        modem_pwrsaving_mode_ctrl(
                                        (dev_object_t *)
                                        &diag_lte_telit_obj,
                                        MODEM_LPM);
    if (set_psav_rc != PASSED) {
        cterr('f', 0, "Failed to set modem power saving mode.");
        goto __exit;
    }

    /* Assert W_DISABLE1# */
    if (diag_lte_telit_w_disable1_ctrl(ENABLE) != PASSED) {
        cterr('f', 0, "Failed to assert WDISABLE1# pin.");
        return (FAILED);
    }
    
    /* Check whether modem is in LPM(Low Power Mode) */
    for (ix = 0; ix < MODEM_MODE_SWITCHING_TOUT; ix ++) {
        lpm_rc = diag_lte_telit_obj_p->callin_fvt->modem_in_lpm(
                                                   (dev_object_t *)
                                                   &diag_lte_telit_obj);
        if (lpm_rc == TRUE) {
            break;
        }
        msleep(LTE_TELIT_POLLING_DELAY);
    }

    if (lpm_rc != TRUE) {
        cterr('f', 0, "Modem didn't switch to LPM as expected");
        goto __exit;
    }

    /* De-assert W_DISABLE1# */
    if (diag_lte_telit_w_disable1_ctrl(DISABLE) != PASSED) {
        cterr('f', 0, "Failed to de-assert WDISABLE1# pin.");
        goto __exit;
    }

    /* Check whether modem is back to online mode */
    for (ix = 0; ix < MODEM_MODE_SWITCHING_TOUT; ix ++) {
        om_rc = diag_lte_telit_obj_p->callin_fvt->modem_is_online(
                                                  (dev_object_t *)
                                                  &diag_lte_telit_obj);
        if (om_rc == TRUE) {
            break;
        }
        msleep(LTE_TELIT_POLLING_DELAY);
    }

    if (om_rc != TRUE) {
        cterr('f', 0, "Modem didn't switch to online mode as expected");
        goto __exit;
    }

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (PASSED);

__exit:
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (FAILED);
}

/*******************************************************************************
 * Function   : diag_lte_telit_modem_pwron_pin_test 
 * Description: Function to test Modem_Power_ON pin
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_modem_pwron_pin_test (int input)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int shdn_dis = FALSE;
    int audis = FALSE;
    int fastshdn_f = 0, audio_f = 0;

    
    printf("[WARNING] This test is dependent on Telit FW.\n"
           "FW version before 32.00.0X2-B008(included) has known issue.\n");

    testname("Modem_Power_ON Pin");

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    /* Check whether modem fast shutdown is disable.*/
    /* The Modem_Power_ON pin is used to trigger fast shutdown feature,
     * disabling fast shutdown feature is needed to avoid shutdown modem
     * during testing. */
    prpass(testpass, "Check whether modem fast shutdown feature is disable...");
    shdn_dis = diag_lte_telit_obj_p->callin_fvt->modem_fast_shutdown_is_disable(
                                                 (dev_object_t *)
                                                 &diag_lte_telit_obj);

    /* If no, disable the fast shutdown feature */
    if (shdn_dis != TRUE) {
        prpass(testpass, "Disabling modem fast shutdown feature...");
        if (diag_lte_telit_obj_p->callin_fvt->modem_disable_fast_shutdown(
                                              (dev_object_t *)
                                              &diag_lte_telit_obj) == PASSED) {
            fastshdn_f = 1;
        } else {
            cterr('f', 0, "Failed to disable modem fast shutdown feature");
            goto __exit;
        }
    }

    /* Check whether modem audio feature is disable. Since Modem_Power_ON pin
     * connects to Telit modem GPIO_05 pin, which is an extended GPIO pin, the
     * value of it only be readable while modem audio feature is disable */
    prpass(testpass, "Check whether modem audio feature is disable...");
    audis = diag_lte_telit_obj_p->callin_fvt->modem_audio_is_disable(
                                              (dev_object_t *)
                                              &diag_lte_telit_obj);

    /* If no, disable the modem audio feature */
    if (audis != TRUE) {
        prpass(testpass, "Disabling modem audio feature...");
        if (diag_lte_telit_obj_p->callin_fvt->modem_disable_audio(
                                              (dev_object_t *)
                                              &diag_lte_telit_obj) == PASSED) {
            audio_f = 1;
        } else {
            cterr('f', 0, "Failed to disable modem audio feature");
            goto __exit;
        }
    }

    /* If fast shutdown feature or audio feature need to disable, then reboot */
    if ((fastshdn_f || audio_f) == 1) {
        prpass(testpass, "Soft reboot modem via AT command...");
        if (diag_lte_telit_obj_p->callin_fvt->modem_reboot((dev_object_t *) 
                                              &diag_lte_telit_obj) != PASSED) {
            cterr('f', 0, "Failed to soft reboot modem");
            goto __exit;
        }
    }

    /* Check modem GPIO pin value is correct or not according to Modem_Power_ON 
     * pin is set to low */
    prpass(testpass, "Set Modem_Power_ON pin to low.");
    if (diag_lte_telit_set_modem_pwron_pin_test(LOW) != PASSED) {
        cterr('f', 0, "Failed to set Modem_Power_ON pin to low");
        goto __exit;
    }

    /* Check modem GPIO pin value is correct or not according to Modem_Power_ON 
     * pin is set to high */
    prpass(testpass, "Set Modem_Power_ON pin to high.");
    if (diag_lte_telit_set_modem_pwron_pin_test(HIGH) != PASSED) {
        cterr('f', 0, "Failed to set Modem_Power_ON pin to high");
        goto __exit;
    }

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (PASSED);

__exit:
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (FAILED);
}

/*******************************************************************************
 * Function   : diag_lte_telit_shdn_test
 * Description: Test modem gpio 3 shutdown indicator pin
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_shdn_test (int input)
{
    int iy, timeout;
    unsigned long val;

    /* 0. Monitor CPLD register modem status bit 3 (shutdown indicator) */
    prpass(testpass, "Monitoring SAFE_POWER_REMOVAL signal");

    /* Read cpld reg 0x10 bit3 equal 1 (modem is active)*/
    hr_cpld_reg_read_32(HR_CPLD_MODEM_STATUS, &val);
    if ((val & HR_CPLD_MODEM_STA_ACTIVE) != HR_CPLD_MODEM_STA_ACTIVE) {
        cterr('f', 0, "The CPLD Shutdown indicator signal didn't action "
              "as expected. value = %lx\n", val);
        return (FAILED);
    }

    /* 1. Check whether the shutdown indicator is set */
    prpass(testpass, "Check the configuration of modem shutdown "
           "indicator...");
    if (diag_lte_telit_soft_shutdown_indicator_is_set() != TRUE) {
        /* If not, set the shutdown indicator */
        prpass(testpass, "Setting modem shutdown indicator...");
        if (diag_lte_telit_set_shutdown_indicator() != PASSED) {
            cterr('f', 0, "Failed to set shutdown indicator\n");
            return (FAILED);
        }
    }

    /* 2. Power down modem by using AT command*/
    if (diag_lte_telit_modem_power_down() != PASSED) {
        cterr('f', 0, "Failed to power down LTE Telit modem\n");
        return (FAILED);
    }

    /* 3. Monitor SAFE_POWER_REMOVAL signal */
    prpass(testpass, "Monitoring SAFE_POWER_REMOVAL signal");

    for (iy = 0; iy < MODEM_CHK_PWR_TOUT; iy++) {
        timeout = HIGH;
        /* TBD  read cpld reg 0x10 bit3 equal 0*/
        hr_cpld_reg_read_32(HR_CPLD_MODEM_STATUS, &val);
        if ((val & HR_CPLD_MODEM_STA_ACTIVE) == LOW) {
            timeout = LOW;
            break;
        }
        msleep(LTE_TELIT_POLLING_DELAY);
    }

    if (timeout == HIGH) {
        cterr('f', 0, "The SAVE_POWER_REMOVAL signal didn't transition "
               "as expected.\n");
        return (FAILED);
    }

    return (PASSED);
}
/*******************************************************************************
 * Function   : diag_lte_telit_gps_antenna_test 
 * Description: To get LTE GPS antenna information for LTE 
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_gps_antenna_test (int input)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    
    int rc = FAILED;
    char test_name[128] = {0, };

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    sprintf(test_name, "Modem GPS Antenna");

    testname(test_name);
    rc = diag_lte_telit_obj_p->callin_fvt->modem_gps_test((dev_object_t *)
                                                          &diag_lte_telit_obj);

    if (rc != PASSED) {
        cterr('f', 0, "%s fails", test_name);
        /* Based on review comment, print out the recovery process when test
         * fails */
        printf("\n[WARNING]Please perform the following process before re-test"
               " if the failure is caused by incorrect setup:\n\n"
               "    1. Go to 'Utilities' menu\n"
               "    2. Run 'Enable Modem Operation Mode Utility Without ESC'\n\n"
               "Without the above process, the modem will keep in wrong state"
               " which might affect the other tests\n");
    }
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    
    return (rc);
    
}

/*******************************************************************************
 * Function   : diag_lte_telit_main_rssi_test 
 * Description: To get LTE main RSSI for LTE
 * Inputs     : which_connector - which antenna connector number
 *                                For LM960, there're primary #0/#1 and
 *                                secondary #0/#1 connectors.
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_main_rssi_test (int which_connector)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;
    int msku;
    char test_name[128] = {0, };

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    diag_lte_telit_get_modem_sku(&msku);
    if (msku == LTE_TELIT_LM940) {
        sprintf(test_name, "Modem Main RSSI Test");
    } else {
        sprintf(test_name, "Modem Main #%d RSSI Test", which_connector);
    }

    testname(test_name);

    prpass(testpass, test_name);
    rc = diag_lte_telit_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)
                                                           &diag_lte_telit_obj,
                                                           MAIN_RSSI,
                                                           which_connector);

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    if (rc != PASSED) {
        cterr('f', 0, "%s fails", test_name);
        /* Based on review comment, print out the recovery process when test
         * fails */
        printf("\n[WARNING]Please perform the following process before re-test"
               " if the failure is caused by incorrect setup:\n\n"
               "    1. Go to 'Utilities' menu\n"
               "    2. Run 'Enable Modem Operation Mode Utility'\n\n"
               "Without the above process, the modem will keep in wrong state"
               " which might affect the other tests\n");
        return (FAILED);
    }
    
    if (diag_lte_telit_soft_reboot(USB3P0) != PASSED) {
        cterr('f', 0, "Failed to reboot modem.");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_lte_telit_div_rssi_test 
 * Description: To get LTE DIV RSSI for Telit LTE 
 * Inputs     : input - not used
 * Inputs     : which_connector - which antenna connector number
 *                                For LM960, there're primary #0/#1 and
 *                                secondary #0/#1 connectors.
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_div_rssi_test (int which_connector)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int msku;
    int rc = FAILED;
    char test_name[128] = {0, };

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    diag_lte_telit_get_modem_sku(&msku);
    if (msku == LTE_TELIT_LM940) {
        sprintf(test_name, "Modem DIV RSSI Test");
    } else {
        sprintf(test_name, "Modem DIV #%d RSSI Test", which_connector);
    }

    testname(test_name);

    prpass(testpass, test_name);
    rc = diag_lte_telit_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)
                                                           &diag_lte_telit_obj,
                                                           DIV_RSSI,
                                                           which_connector);

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    if (rc != PASSED) {
        cterr('f', 0, "%s fails", test_name);
        /* Based on review comment, print out the recovery process when test
         * fails */
        printf("\n[WARNING]Please perform the following process before re-test"
               " if the failure is caused by incorrect setup:\n\n"
               "    1. Go to 'Utilities' menu\n"
               "    2. Run 'Enable Modem Operation Mode Utility'\n\n"
               "Without the above process, the modem will keep in wrong state"
               " which might affect the other tests\n");
        return (FAILED);
    }

    if (diag_lte_telit_soft_reboot(USB3P0) != PASSED) {
        cterr('f', 0, "Failed to reboot modem.");
        return (FAILED);
    }

    return (PASSED);
}
