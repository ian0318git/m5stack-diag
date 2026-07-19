/* $Id: plug_lte_telit_test.c,v 1.5 2019/06/26 03:52:55 shjung Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_test.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_test.c - Pluggable LTE Telit Main Function
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "cookie_4.h"
#include "queryflags.h"
#include "plug_slot.h"
#include "plug_gpio_exp_test.h"
#include "plug_temp_sensor_test.h"
#include "plug_host_fpga_lib.h"
#include "plug_common_lib.h"
#include "plug_lte_telit_host.h"
#include "plug_lte_telit_test.h"
#include "plug_lte_telit_lib.h"
#include "plug_lte_telit_util.h"

int plug_lte_telit_main(void *);

static int plug_lte_telit_intf_test(int);
static int plug_lte_telit_ts_test(int);
static int plug_lte_telit_man_gpio_exp_test(void);
static int plug_lte_telit_opt_gpio_exp_test(void);
static int plug_lte_telit_modem_detect_test(int);
static int plug_lte_telit_main_rssi_test(int);
static int plug_lte_telit_div_rssi_test(int);
static int plug_lte_telit_gps_antenna_test(int);
static int plug_lte_telit_sim_test(int);
static int plug_lte_telit_dport_test(int);
static int plug_lte_telit_usb2p0_test(int);
static int plug_lte_telit_i2c_rst_pin_test(int);
static int plug_lte_telit_w_disable_pin_test(int);
static int plug_lte_telit_modem_pwron_pin_test(int);
static int plug_lte_telit_pcie3p0_test(int);

extern int do_all_menu_items(struct menuinfo *);


static submenu_xtable_t plug_lte_telit_table[] = {
    {"Thermal Sensor Test", 
     (type_t(*)())plug_lte_telit_ts_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_lte_telit_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Mandatory GPIO Expander (0x4E) Test", 
     (type_t(*)())plug_lte_telit_man_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Optional GPIO Expander (0x4C) Test", 
     (type_t(*)())plug_lte_telit_opt_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Detection Test", 
     (type_t(*)())plug_lte_telit_modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 0 Card Test", 
     (type_t(*)())plug_lte_telit_sim_test, SIM0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 1 Card Test", 
     (type_t(*)())plug_lte_telit_sim_test, SIM1,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB 2.0 Detection Test", 
     (type_t(*)())plug_lte_telit_usb2p0_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"I2C Reset Pin Test", 
     (type_t(*)())plug_lte_telit_i2c_rst_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"W_DISABLE Pin Test", 
     (type_t(*)())plug_lte_telit_w_disable_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem_Power_ON Pin Test", 
     (type_t(*)())plug_lte_telit_modem_pwron_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PCIe3.0 Test", 
     (type_t(*)())plug_lte_telit_pcie3p0_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_lte_telit_supports_pcie_intf, 0, (type_t(*)())0, 0},
    {"USB Debug Port Detection Test", 
     (type_t(*)())plug_lte_telit_dport_test, TRUE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem GPS Antenna", 
     (type_t(*)())plug_lte_telit_gps_antenna_test, TRUE,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_lte_telit_has_dedicated_gps_antenna, 0,
     (type_t(*)())0, 0},
    {"Modem Main RSSI Test", 
     (type_t(*)())plug_lte_telit_main_rssi_test, LTE_ANTENNA_CON_0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DIV RSSI Test", 
     (type_t(*)())plug_lte_telit_div_rssi_test, LTE_ANTENNA_CON_0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Main #1 RSSI Test", 
     (type_t(*)())plug_lte_telit_main_rssi_test, LTE_ANTENNA_CON_1,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_lte_telit_has_2_rssi_antenna, 0, (type_t(*)())0, 0},
    {"Modem DIV #1 RSSI Test", 
     (type_t(*)())plug_lte_telit_div_rssi_test, LTE_ANTENNA_CON_1,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_lte_telit_has_2_rssi_antenna, 0, (type_t(*)())0, 0},
    {"Utilities", 
     (type_t(*)())plug_lte_telit_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_LTE_TELIT_TABLE_SZ \
        (sizeof(plug_lte_telit_table) / sizeof(submenu_xtable_t))

static mitem_t plug_lte_telit_pri_test_items[PLUG_LTE_TELIT_TABLE_SZ +
                                             MAX_BASE_ITEMS];
static mitem_t plug_lte_telit_sec_test_items[PLUG_LTE_TELIT_TABLE_SZ + 
                                             MAX_BASE_ITEMS];

static menuinfo_t plug_lte_telit_test_menu = {
    "Pluggable LTE(Telit) Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_lte_telit_pri_test_items,
};
static menuinfo_t *plug_lte_telit_test_menup = &plug_lte_telit_test_menu;

static struct plug_intf_t *plug;

/*******************************************************************************
 * Function   : plug_lte_telit_main
 * Description: Main entry point for Pluggable LTE Telit
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_telit_main (void *in)
{
    int modem_ready = FALSE;
    int ret = PASSED;

    /* Sanity check */
    if (in == NULL) {
        cterr('f', 0, "Null pointer");
        return (FAILED);
    }

    plug = (struct plug_intf_t *)in;

    /* Suppress printk so kernel won't print out brunch of messages */
    system(TELIT_SYS_SUPPRESS_PRINTK);

    /* Store all USB port info that LTE modem might use from platform */
    plug_lte_telit_store_usb_devinfo(plug->slot);

    /* Initialize GPIO Expander Output value */
    if (plug_lte_telit_gpio_exp_out_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Expander Output Value Failed");
        ret = FAILED;
        goto __exit;
    }
       
    /* Initialize GPIO Expander Direction (Input/Output) */
    if (plug_lte_telit_gpio_exp_dir_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Expander Direction Failed");
        ret = FAILED;
        goto __exit;
    }
   
    plug_lte_telit_modem_pwr_ctrl(TRUE);
    /* Load USB serial driver for LTE modem */
    plug_lte_telit_insmod(TRUE);

    fflush(stdout);

    /* Restore modem to default test setup */
    printf("Restore modem to default setup...");
    if (plug_lte_telit_set_modem_default_feature() != PASSED){
        ret = FAILED;
        cterr('f', 0, "Failed to set modem to default setup.");
        goto __exit;
    } else {
        modem_ready = TRUE;
        printf("OK\n");
    }
    
    if (plug->test_type == IFACE_TEST) {
        if (plug_lte_telit_intf_test(0) != PASSED) {
            ret = FAILED;
        }
        goto __exit;
    }
    
    build_primary_submenu(plug_lte_telit_table, PLUG_LTE_TELIT_TABLE_SZ,
                          "Pluggable LTE(Telit)", &plug_lte_telit_test_menup);

    build_secondary_submenu(plug_lte_telit_table, PLUG_LTE_TELIT_TABLE_SZ,
                            plug_lte_telit_sec_test_items);

    if (plug->menu_display) {
        menu(&plug_lte_telit_test_menu, plug_lte_telit_sec_test_items, '\0');
    } else {
        do_all_menu_items(&plug_lte_telit_test_menu);
    }

__exit:   
    if (modem_ready == TRUE) {
        if (plug_lte_telit_modem_pwr_ctrl(FALSE) != PASSED) {
            cterr('f', 0, "Failed to soft power-off Telit modem");
        }
    }    
    plug_lte_telit_insmod(FALSE);
    
    return (ret);
}


/*******************************************************************************
 * Function   : plug_lte_telit_ts_test 
 * Description: Thermal Sensor test for Pluggable LTE 
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_ts_test(int input)
{
    testname("Thermal Sensor");
    prpass(testpass, "Thermal Sensor");

    if (plug_temp_sensor_reg_test() != PASSED) {
        cterr('f', 0, "Thermal Sensor Test Fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_man_gpio_exp_test 
 * Description: GPIO Expander Test for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_man_gpio_exp_test (void)
{
    printf("Mandatory GPIO Expander (0x4E) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");

    plug_lte_telit_set_gpio_exp_test_reg(MANDATORY);

    if (plug_gpio_exp_reg_test(MANDATORY) != PASSED) {
        cterr('f', 0, "Mandatory GPIO Expander (0x4E) Test fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_opt_gpio_exp_test 
 * Description: GPIO Expander Test for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_opt_gpio_exp_test (void)
{
    printf("Optional GPIO Expander (0x4C) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");

    plug_lte_telit_set_gpio_exp_test_reg(OPTIONAL);

    if (plug_gpio_exp_reg_test(OPTIONAL) != PASSED) {
        cterr('f', 0, "Optional GPIO Expander (0x4C) Test fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_detect_test 
 * Description: To detect LTE Telit modem for Pluggable LTE
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_modem_detect_test (int input)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    testname("Modem Detection");

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem Detection");
    rc = plug_lte_telit_obj_p->callin_fvt->modem_detection_test(
                                           (dev_object_t *)&plug_lte_telit_obj);
    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
    }
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_main_rssi_test 
 * Description: To get LTE main RSSI for Pluggable LTE
 * Inputs     : which_connector - which antenna connector number
 *                                For LM960, there're primary #0/#1 and
 *                                secondary #0/#1 connectors.
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_main_rssi_test (int which_connector)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;
    int msku;
    char test_name[128] = {0, };

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    plug_lte_telit_get_modem_sku(&msku);
    if (msku == PLUG_LTE_TELIT_LM940) {
        sprintf(test_name, "Modem Main RSSI Test");
    } else {
        sprintf(test_name, "Modem Main #%d RSSI Test", which_connector);
    }

    testname(test_name);

    prpass(testpass, test_name);
    rc = plug_lte_telit_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)
                                                           &plug_lte_telit_obj,
                                                           MAIN_RSSI,
                                                           which_connector);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

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
    
    if (plug_lte_telit_soft_reboot(USB3P0) != PASSED) {
        cterr('f', 0, "Failed to reboot modem.");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_div_rssi_test 
 * Description: To get LTE DIV RSSI for Pluggable LTE
 * Inputs     : input - not used
 * Inputs     : which_connector - which antenna connector number
 *                                For LM960, there're primary #0/#1 and
 *                                secondary #0/#1 connectors.
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_div_rssi_test (int which_connector)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int msku;
    int rc = FAILED;
    char test_name[128] = {0, };

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    plug_lte_telit_get_modem_sku(&msku);
    if (msku == PLUG_LTE_TELIT_LM940) {
        sprintf(test_name, "Modem DIV RSSI Test");
    } else {
        sprintf(test_name, "Modem DIV #%d RSSI Test", which_connector);
    }

    testname(test_name);

    prpass(testpass, test_name);
    rc = plug_lte_telit_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)
                                                           &plug_lte_telit_obj,
                                                           DIV_RSSI,
                                                           which_connector);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
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

    if (plug_lte_telit_soft_reboot(USB3P0) != PASSED) {
        cterr('f', 0, "Failed to reboot modem.");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_gps_antenna_test 
 * Description: To get LTE GPS antenna information for Pluggable LTE 
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_gps_antenna_test (int input)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    
    int rc = FAILED;
    char test_name[128] = {0, };

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    sprintf(test_name, "Modem GPS Antenna");

    testname(test_name);
    rc = plug_lte_telit_obj_p->callin_fvt->modem_gps_test((dev_object_t *)
                                                          &plug_lte_telit_obj);

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
    }
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    
    return (rc);
    
}


/*******************************************************************************
 * Function   : plug_lte_telit_sim_test 
 * Description: To detect LTE SIM 0 card for Pluggable LTE
 * Inputs     : test_sim - SIM0 or SIM1
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_sim_test (int test_sim)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    testname("SIM slot %d test", test_sim);

    /* Check whether SIM card is detected by GPIO expander */
    if (plug_lte_telit_gpio_exp_sim_card_detect(test_sim) != TRUE) {
        cterr('f', 0, "SIM card is not detected by GPIO expander");
        return (FAILED);
    }

    /* Read SIM card PIN request status through AT command to check
     * whether modem can access SIM card or not */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "SIM slot %d test", test_sim);
    rc = plug_lte_telit_obj_p->callin_fvt->sim_detect_test((dev_object_t *)
                                                           &plug_lte_telit_obj,
                                                           test_sim);
    if (rc != PASSED) {
        cterr('f', 0, "SIM slot %d test fails", test_sim);
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_dport_test
 * Description: Run USB enumeration on mini USB debug port
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_dport_test (int input)
{
    int rc = FAILED, restore_rc = FAILED;
    int modem_found_3p0 = FALSE;
    int modem_found_2p0 = FALSE;

    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    testname("USB Debug Port Detection");
    prpass(testpass, "USB debug port test");

    /* 1. Enable USB debug port through GPIO expander */
    prpass(testpass, "Enable debug port.\n");
    if (plug_lte_telit_usb_deb_enable(TRUE) != PASSED) {
        cterr('f', 0, "Failed to enable debug port");
        return (FAILED);
    }

    /* 2. Switch modem to high-speed mode(USB2.0) */
    prpass(testpass, "Switch modem USB mode to USB2.0 mode.\n");
    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                            HIGH_SPD_USB);
    if (rc != PASSED) {
        cterr('f', 0, "Switching modem USB mode(2.0) fails");
        goto __exit;
    }

    /* 3. Checks modem enumeration through USB 2.0 debug port */
    modem_found_2p0 = plug_lte_telit_usb_is_found(DEBUG_USB, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);
    if (modem_found_2p0 != TRUE) {
        cterr('f', 0, "LTE Telit Debug Port USB Enumeration Failed");
        goto __exit;
    } else {
        plug_lte_telit_set_current_usb_port(DEBUG_USB);
    }

    /* 4. Restore modem to super-speed mode(USB3.0) */
    prpass(testpass, "Switch modem USB mode to USB3.0 mode.\n");
    restore_rc = plug_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &plug_lte_telit_obj,
                                                   SUPER_SPD_USB);
    if (restore_rc != PASSED) {
        cterr('f', 0, "Switching modem to USB mode(3.0) fails");
        goto __exit;
    }

    /* 5. Polling USB3.0 bus to see if modem is detected */
    modem_found_3p0 = plug_lte_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);

    if (modem_found_3p0 != TRUE) {
        cterr('f', 0, "Modem is not found on USB3.0 bus");
        goto __exit;
    }
    plug_lte_telit_set_current_usb_port(USB3P0);

    /* 6. Disable USB debug port through GPIO expander */
    prpass(testpass, "Disable debug port.\n");
    if (plug_lte_telit_usb_deb_enable(FALSE) != PASSED) {
        cterr('f', 0, "Failed to disable debug port");
        goto __exit;
    }
    
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (PASSED);
__exit:
    /* Based on review comment, print out the recovery process when test
     * fails */
    printf("\n[WARNING]Please perform the following process before re-test if "
           "the failure is caused by incorrectly installed USB cable:\n\n"
           "    1. Well install the USB cable between PIM and platform front"
           " panel USB port\n"
           "    2. Go to 'Utilities' menu\n"
           "    3. Run 'Modem USB Mode Switching Utility' to switch modem to"
           " USB3.0 mode\n"
           "    4. Run 'External USB Debug bus Enable/Disable Utility' to "
           "disable debug port\n\n"
           "Without the above process, the modem will keep in wrong state"
           " which might affect the other tests\n");

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    plug_lte_telit_set_current_usb_port(USB3P0);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_usb2p0_test 
 * Description: Run USB enumeration in USB 2.0 mode from the host
 * Inputs     : input - not used 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_usb2p0_test (int input)
{
    int rc = FAILED, restore_rc = FAILED;
    int modem_found_2p0 = FALSE, modem_found_3p0 = FALSE;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    testname("Modem USB2.0 interface");

    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem USB2.0 interface Test");

    /* Switch modem to high-speed mode(USB2.0) */
    prpass(testpass, "Switch modem USB mode to USB2.0 mode.\n");
    rc = plug_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                           HIGH_SPD_USB);
    if (rc != PASSED) {
        cterr('f', 0, "Switching modem USB mode(2.0) fails");
        goto __exit;
    }

    /* Polling USB2.0 bus to see if modem is detected */
    modem_found_2p0 = plug_lte_telit_usb_is_found(USB2P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);

    if (modem_found_2p0 != TRUE) {
        cterr('f', 0, "Modem is not found on USB2.0 bus");
        goto __exit;
    } else {
        plug_lte_telit_set_current_usb_port(USB2P0);
    }

    /* Restore modem to super-speed mode(USB3.0) */
    prpass(testpass, "Switch modem USB mode to USB3.0 mode.\n");
    restore_rc = plug_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &plug_lte_telit_obj,
                                                   SUPER_SPD_USB);
    if (restore_rc != PASSED) {
        cterr('f', 0, "Switching modem to USB mode(3.0) fails");
        goto __exit;
    }

    /* Polling USB3.0 bus to see if modem is detected */
    modem_found_3p0 = plug_lte_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);

    if (modem_found_3p0 != TRUE) {
        cterr('f', 0, "Modem is not found on USB3.0 bus");
        goto __exit;
    }
    plug_lte_telit_set_current_usb_port(USB3P0);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (PASSED);
    
__exit:
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    plug_lte_telit_set_current_usb_port(USB3P0);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_i2c_rst_pin_test 
 * Description: Function to test I2C_RESET_L
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_i2c_rst_pin_test (int input)
{
    int ret = FAILED;
    int i2c_addr = PLUG_I2C_CTRL_OFFSET;
    struct plug_intf_t *plug_i2c_rst;
    uchar data_buf[32];

    testname("I2C RESET pin");
    prpass(testpass, "I2C RESET pin");

    plug_i2c_rst = plug;
    /* Setup the register for pluggable FPGA I2C control */
    if (plug_i2c_rst->slot == PLUG_SLOT_2) {
       i2c_addr = i2c_addr + PLUG_FPGA_I2C_OFFSET; 
    }

    /* Assert I2C_RESET_L pin */
    if (plug_i2c_rst->i2c_reset((void *)plug_i2c_rst)) {
        cterr('f', 0, "Failed to reset Pluggable LTE slot %d I2C bus",
              plug_i2c_rst->slot);
        return (FAILED);
    }

    /* Check if we receive an ACK */
    ret = plug_common_fpga_i2c_ack_check(i2c_addr, PLUG_LTE_FPGA_I2C_ACK_MUX,
                                         PLUG_LTE_ACT2_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_REG_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_SUB_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_DATA_LEN,
                                         data_buf); 
    /* Test fails if we still got an ACK while I2C_RESET_L is asserted */
    if (ret == PASSED) {
        cterr('f', 0, "Received an ACK from ACT2 chip while I2C bus is in reset"
              " mode(slot %d)", plug_i2c_rst->slot);
        return (FAILED);
    }

    /* Un-reset I2C interface */
    if (plug_i2c_rst->i2c_unreset((void*)plug_i2c_rst)) {
        cterr('f', 0, "Failed to unreset Pluggable LTE slot %d I2C bus",
              plug_i2c_rst->slot);
        return (FAILED);
    }
    msleep(PLUG_LTE_ACT2_1P5_UNRESET_DELAY);
     
    /* Check if we receive an ACK */
    ret = plug_common_fpga_i2c_ack_check(i2c_addr, PLUG_LTE_FPGA_I2C_ACK_MUX,
                                         PLUG_LTE_ACT2_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_REG_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_SUB_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_DATA_LEN,
                                         data_buf); 
    if (ret != PASSED) {
        cterr('f', 0, "No ACK from ACT2 chip (slot %d)", plug_i2c_rst->slot);
    }

    return (ret);
}


/*******************************************************************************
 * Function   : plug_lte_telit_w_disable_pin_test 
 * Description: Function to test W_DISABLE1# pin
 *              Modem will enter LPM(Low Power Mode) if W_DISABLE1# pin is
 *              asserted
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_w_disable_pin_test (int input)
{
    int lpm_rc = FALSE;
    int om_rc = FALSE;
    int set_psav_rc = FAILED;
    int ix;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    testname("W_DISABLE pin");
    prpass(testpass, "W_DISABLE pin");

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    /* W_DISABLE1# pin is used to switch modem between operation mode and power
     * saving mode. And the modem power saving mode can be configured as LPM
     * (Low Power Mode), Power Saving Mode, DG(Dying Gasp) or no event to be
     * performed. In this test, we configured modem power saving mode as LPM */
    /* Check whether the modem power saving modem is configured as LPM */

    prpass(testpass, "Set modem power saving configuration...");
    set_psav_rc = plug_lte_telit_obj_p->callin_fvt->
                                        modem_pwrsaving_mode_ctrl(
                                        (dev_object_t *)
                                        &plug_lte_telit_obj,
                                        MODEM_LPM);
    if (set_psav_rc != PASSED) {
        cterr('f', 0, "Failed to set modem power saving mode.");
        goto __exit;
    }

    /* Assert W_DISABLE1# */
    prpass(testpass, "Set W_DISABLE pin to HIGH...");
    if (plug_lte_telit_w_disable1_ctrl(HIGH) != PASSED) {
        cterr('f', 0, "Failed to assert WDISABLE1# pin.");
        goto __exit;
    }
    
    /* Check whether modem is in LPM(Low Power Mode) */
    for (ix = 0; ix < MODEM_MODE_SWITCHING_TOUT; ix ++) {
        lpm_rc = plug_lte_telit_obj_p->callin_fvt->modem_in_lpm(
                                                   (dev_object_t *)
                                                   &plug_lte_telit_obj);
        if (lpm_rc == TRUE) {
            break;
        }
        msleep(PLUG_LTE_TELIT_POLLING_DELAY);
    }

    if (lpm_rc != TRUE) {
        cterr('f', 0, "Modem didn't switch to LPM as expected");
        goto __exit;
    }

    /* De-assert W_DISABLE1# */
    prpass(testpass, "Set W_DISABLE pin to LOW...");
    if (plug_lte_telit_w_disable1_ctrl(LOW) != PASSED) {
        cterr('f', 0, "Failed to de-assert WDISABLE1# pin.");
        goto __exit;
    }

    /* Check whether modem is back to online mode */
    for (ix = 0; ix < MODEM_MODE_SWITCHING_TOUT; ix ++) {
        om_rc = plug_lte_telit_obj_p->callin_fvt->modem_is_online(
                                                  (dev_object_t *)
                                                  &plug_lte_telit_obj);
        if (om_rc == TRUE) {
            break;
        }
        msleep(PLUG_LTE_TELIT_POLLING_DELAY);
    }

    if (om_rc != TRUE) {
        cterr('f', 0, "Modem didn't switch to online mode as expected");
        goto __exit;
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (PASSED);

__exit:
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_pwron_pin_test 
 * Description: Function to test Modem_Power_ON pin
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_modem_pwron_pin_test (int input)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int shdn_dis = FALSE;
    int audis = FALSE;
    int dg_dis = FALSE;
    int fastshdn_f = 0, audio_f = 0;

    
    printf("[WARNING] This test is dependent on Telit FW.\n"
           "FW version before 32.00.0X2-B008(included) has known issue.\n");

    testname("Modem_Power_ON Pin");

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    /* The Modem_Power_ON pin is used to trigger both fast shutdown and 
     * dying gasp feature.
     * To test this pin, we need to avoid modem performs fast shutdown during
     * testing.
     * The following are cases triggering fast shutdown:
     *   (1) Send AT#FASTSHDN command
     *   (2) After set AT#FASTSHDN=1, toggle the pin from HIGH to LOW
     *   (3) After enabling dying gasp, toggle the pin from HIGH to LOW
     * Thus, disabling dying gasp and fast shutdown is needed.
     */
     /* To fix CSCvq12342: If dying gasp is enabled, modem shutdown while
      * toggling Modem_Power_ON pin.
      * Check whether the dying gasp is disable */
    prpass(testpass, "Check whether modem dying gasp feature is disable...");
    dg_dis = plug_lte_telit_obj_p->callin_fvt->modem_dying_gasp_is_disable(
                                              (dev_object_t *)
                                              &plug_lte_telit_obj);

    /* If no, disable the dying gasp feature */
    if (dg_dis != TRUE) {
        prpass(testpass, "Disabling modem dying gasp feature...");
        if (plug_lte_telit_obj_p->callin_fvt->modem_disable_dying_gasp(
                                              (dev_object_t *)
                                              &plug_lte_telit_obj) == PASSED) {
        } else {
            cterr('f', 0, "Failed to disable modem dying gasp feature");
            goto __exit;
        }
    }

    /* Check whether modem fast shutdown is disable.*/
    prpass(testpass, "Check whether modem fast shutdown feature is disable...");
    shdn_dis = plug_lte_telit_obj_p->callin_fvt->modem_fast_shutdown_is_disable(
                                                 (dev_object_t *)
                                                 &plug_lte_telit_obj);

    /* If no, disable the fast shutdown feature */
    if (shdn_dis != TRUE) {
        prpass(testpass, "Disabling modem fast shutdown feature...");
        if (plug_lte_telit_obj_p->callin_fvt->modem_disable_fast_shutdown(
                                              (dev_object_t *)
                                              &plug_lte_telit_obj) == PASSED) {
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
    audis = plug_lte_telit_obj_p->callin_fvt->modem_audio_is_disable(
                                              (dev_object_t *)
                                              &plug_lte_telit_obj);

    /* If no, disable the modem audio feature */
    if (audis != TRUE) {
        prpass(testpass, "Disabling modem audio feature...");
        if (plug_lte_telit_obj_p->callin_fvt->modem_disable_audio(
                                              (dev_object_t *)
                                              &plug_lte_telit_obj) == PASSED) {
            audio_f = 1;
        } else {
            cterr('f', 0, "Failed to disable modem audio feature");
            goto __exit;
        }
    }

    /* If fast shutdown feature or audio feature need to disable, then reboot */
    if ((fastshdn_f || audio_f) == 1) {
        prpass(testpass, "Soft reboot modem via AT command...");
        if (plug_lte_telit_obj_p->callin_fvt->modem_reboot((dev_object_t *) 
                                              &plug_lte_telit_obj) != PASSED) {
            cterr('f', 0, "Failed to soft reboot modem");
            goto __exit;
        }
    }

    /* Check modem GPIO pin value is correct or not according to Modem_Power_ON 
     * pin is set to low */
    prpass(testpass, "Set Modem_Power_ON pin to low.");
    if (plug_lte_telit_set_modem_pwron_pin_test(LOW) != PASSED) {
        cterr('f', 0, "Failed to set Modem_Power_ON pin to low");
        goto __exit;
    }

    /* Check modem GPIO pin value is correct or not according to Modem_Power_ON 
     * pin is set to high */
    prpass(testpass, "Set Modem_Power_ON pin to high.");
    if (plug_lte_telit_set_modem_pwron_pin_test(HIGH) != PASSED) {
        cterr('f', 0, "Failed to set Modem_Power_ON pin to high");
        goto __exit;
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (PASSED);

__exit:
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_pcie3p0_test 
 * Description: Function to test PCIe3.0 interface.
 *              This function is only for LM960 modem
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_pcie3p0_test (int input)
{
    /* PCIe3.0 interface is not supported by Telit for now */
    printf("WARNING: PCIe interface is not yet supported by Telit!!\n\n");

    return (FAILED);

#ifdef TO_DO
    /* Switch serdes type from USB3.0 interface to PCIe3.0 interface */
    prpass(testpass, "Switch serdes type to PCIe..");
    if (plug_lte_telit_select_modem_serdes(SERDES_PCIE_3P0) != PASSED) {
        cterr('f', 0, "Modem serdes switching fails");
        return (FAILED);
    }

    msleep(PLUG_LTE_TELIT_SERDES_SWITCHING_DELAY);

    /* Ensure the modem is not detected on USB bus */
    if (plug_lte_telit_usb_is_found(TRUE, MODEM_SERDES_SWITCH_PROBE_TOUT)
                                    == TRUE) {
        cterr('f', 0, "Modem is still detected as a USB 3.0 device");
        return (FAILED);
    }

    /* TODO!! Need to implement the test machenism of PCIe interface
     * communication*/

    /* Switch back to USB3.0 interface */
    prpass(testpass, "Switch serdes type back to USB3.0..");
    if (plug_lte_telit_select_modem_serdes(SERDES_USB_3P0) != PASSED) {
        cterr('f', 0, "Modem serdes switching fails");
        return (FAILED);
    }

    /* Check USB interface connectivity */
    if (plug_lte_telit_usb_is_found(TRUE, MODEM_SERDES_SWITCH_PROBE_TOUT)
                                    != TRUE) {
        cterr('f', 0, "Unexpected modem serdes type");
        return (FAILED);
    }

    return (PASSED);
#endif
}


/*******************************************************************************
 * Function   : plug_lte_telit_intf_test
 * Description: To test all pluggable LTE I/O interface, which are between host
 *              and pluggable module through host connector
 *              Telit module - USB2.0/USB3.0/I2C/I2C_RESET_L/
 *                             PCIe3.0(only for LM960)
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_intf_test (int input)
{
    int msku;
    int pcie_support_f = 0;

    plug_lte_telit_get_modem_sku(&msku);

    /* Accessing modem to verify USB3.0 interface */
    if (plug_lte_telit_modem_detect_test(0) != PASSED) {
        printf("Pluggable LTE Telit USB interface test fails.\n");
        return (FAILED);
    }

    /* To verify USB2.0 interface */
    if (plug_lte_telit_usb2p0_test(0) != PASSED) {
        printf("Pluggable LTE Telit USB2.0 interface test fails.\n");
        return (FAILED);
    }

    /* To verify I2C_RESET_L pin */
    if (plug_lte_telit_i2c_rst_pin_test(0) != PASSED) {
        printf("Pluggable LTE Telit I2C RESET pin test fails.\n");
        return (FAILED);
    }

    /* To verify I2C interface */
    if (plug_lte_telit_opt_gpio_exp_test() != PASSED) {
        printf("Pluggable LTE Telit I2C interface test fails.\n");
        return (FAILED);
    }

    /* To verify PCIe3.0 interface */
    /* PCIe interface is not supported by Telit, temporarily skip it */
    pcie_support_f = plug_lte_telit_supports_pcie_intf();

    if (pcie_support_f == TRUE) {
        if (msku == PLUG_LTE_TELIT_LM960) {
            if (plug_lte_telit_pcie3p0_test(0) != PASSED) {
                printf("Pluggable LTE Telit PCIe3.0 test fails.\n");
                return (FAILED);
            }
        }
    }
    
    return (PASSED);
}


/*------------------------------------------------------------------
$Log: plug_lte_telit_test.c,v $
Revision 1.5  2019/06/26 03:52:55  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.4  2019/06/14 05:46:08  shjung
Fixed CSCvq12342, disable modem dying gasp before toggling Modem_Power_ON pin to avoid modem perform fast shutdown and added enabling dying gasp utility

Revision 1.3  2019/05/20 07:28:06  shjung
1. Replace USB serial driver option.ko/usb_wwan.ko with GobiSerial.ko
2. Changes based on last code review comments.
3. Use poll mechanism to query modem functionality level in W_DISABLE pin test
4. Add 1 second delay after close tty device, which is following Telit's test script process.

Revision 1.2  2019/05/14 08:48:37  sherliu2
Support hyperloop

Revision 1.1.2.17  2019/05/09 07:50:19  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.16  2019/05/02 06:13:35  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.15  2019/04/17 10:09:11  sherliu2
remove mdev related

Revision 1.1.2.14  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.13  2019/04/08 06:41:22  shjung
Check the configuration of modem shutdown indicator while powering down the modem

Revision 1.1.2.12  2019/03/28 10:48:59  shjung
Code clean up

Revision 1.1.2.11  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.10  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.9  2019/03/12 02:53:52  shjung

1. Added query modem testmode status utility
2. Added enable OP mode utility

Revision 1.1.2.8  2019/02/12 01:38:52  sherliu2
Modified GPS Antenna Test

Revision 1.1.2.7  2019/02/11 07:59:45  sherliu2
Add GPS Antenna Test

Revision 1.1.2.6  2019/01/18 06:15:31  shjung

1. Added W_DISABLE pin test
2. Added modem USB mode switching utility
3. Added delay in modem reboot function based on spec
4. Removed USB mode resotre operation when debug port test failed
5. Code clean up

Revision 1.1.2.5  2019/01/15 10:22:19  shjung
Modified the mechanism to get modem USB device info

Revision 1.1.2.4  2019/01/02 02:09:27  shjung
Restore LTE back to USB3.0 mode while debug port test failed

Revision 1.1.2.3  2018/12/28 06:23:19  shjung
Temporarily remove PCIe test from I/O interface test since PCIe interface is not yet supported by Telit

Revision 1.1.2.2  2018/12/19 19:37:51  shjung
Modified debug port test

Revision 1.1.2.1  2018/12/14 00:50:16  shjung
Initial check-in for Hyperloop



$Endlog$
*/
