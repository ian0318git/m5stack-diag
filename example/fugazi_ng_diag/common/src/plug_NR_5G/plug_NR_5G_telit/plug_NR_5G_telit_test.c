/*
 * $Id: plug_NR_5G_telit_test.c,v 1.4 2021/07/15 18:23:23 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_NR_5G/plug_NR_5G_telit/plug_NR_5G_telit_test.c,v $
 *------------------------------------------------------------------
 *
 * plug_NR_5G_telit_test.c - Pluggable Telit Main Function
 *
 * Copyright (c) 2020 by Cisco Systems, Inc.
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
#include "plug_NR_5G_telit_host.h"
#include "plug_NR_5G_telit_test.h"
#include "plug_NR_5G_telit_lib.h"
#include "plug_NR_5G_telit_util.h"

#include "dev_NR_5G_telit_at.h"


int plug_NR_5g_telit_main(void *);

static int plug_NR_5g_telit_display_modem_info(void);
static int plug_NR_5g_telit_intf_test(int);
static int plug_NR_5g_telit_ts_test(int);
static int plug_NR_5g_telit_man_gpio_exp_test(void);
static int plug_NR_5g_telit_opt_gpio_exp_test(void);
static int plug_NR_5g_telit_modem_detect_test(int);
static int plug_NR_5g_telit_ant_rx_test(int);
static int plug_NR_5g_telit_rssi_test(int, int, int);
static int plug_NR_5g_telit_ant_tx_test (int);
static int plug_NR_5g_telit_gps_antenna_test(int);
static int plug_NR_5g_telit_sim_test(int);
static int plug_NR_5g_telit_dport_test(int);
static int plug_NR_5g_telit_usb2p0_test(int);
static int plug_NR_5g_telit_i2c_rst_pin_test(int);
static int plug_NR_5g_telit_w_disable_pin_test(int);
static int plug_NR_5g_telit_modem_pwron_pin_test(int);
static int plug_NR_5g_telit_pcie3p0_test(int);
static int plug_NR_5g_telit_ant_tx_menu (int which_connector);
static int plug_NR_5g_telit_ant_rx_menu (int opt);

int band_to_test = BAND_N41; 

extern boolean plug_test_not_supported (void);
extern int do_all_menu_items(struct menuinfo *);
extern int plug_NR_5G_telit_set_shutdown_indicator(void);
extern  int plug_NR_5g_telit_led_ctrl_util(int);
extern int plug_NR_5g_telit_enable_op_mode_util(int);

extern nr_sub6_band_struct nr_sub6_band_tbl[];
extern int band_tbl_size;

//Antenna rx test menu
static submenu_xtable_t plug_NR_5g_telit_rx_test_tbl[] = {
    {"Modem ALL Antenna RX Test",
     (type_t(*)())plug_NR_5g_telit_ant_rx_test, 0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Individual Antenna Rx Test",
     (type_t(*)())plug_NR_5g_telit_ant_rx_test, 1,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_TELIT_RX_TEST_TBL_SZ \
        (sizeof(plug_NR_5g_telit_rx_test_tbl) / sizeof(submenu_xtable_t))

static mitem_t plug_NR_5g_telit_rx_test_pri_items[PLUG_TELIT_RX_TEST_TBL_SZ+
                                                 MAX_BASE_ITEMS];
static mitem_t plug_NR_5g_telit_rx_test_sec_items[PLUG_TELIT_RX_TEST_TBL_SZ+
                                                 MAX_BASE_ITEMS];

menuinfo_t plug_NR_5g_telit_rx_test_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_NR_5g_telit_rx_test_pri_items,
};
menuinfo_t *plug_NR_5g_telit_rx_test_menup = &plug_NR_5g_telit_rx_test_menu;


//Antenna Tx test menu
static submenu_xtable_t plug_NR_5g_telit_tx_test_tbl[] = {
    {"Modem ANT0 TX Test",
     (type_t(*)())plug_NR_5g_telit_ant_tx_test, NR_5G_ANTENNA_0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem ANT1 Tx Test",
     (type_t(*)())plug_NR_5g_telit_ant_tx_test, NR_5G_ANTENNA_1,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem ANT2 TX Test",
     (type_t(*)())plug_NR_5g_telit_ant_tx_test, NR_5G_ANTENNA_2,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_TELIT_TX_TEST_TBL_SZ \
        (sizeof(plug_NR_5g_telit_tx_test_tbl) / sizeof(submenu_xtable_t))

static mitem_t plug_NR_5g_telit_tx_test_pri_items[PLUG_TELIT_TX_TEST_TBL_SZ+
                                                 MAX_BASE_ITEMS];
static mitem_t plug_NR_5g_telit_test_sec_items[PLUG_TELIT_TX_TEST_TBL_SZ+
                                                 MAX_BASE_ITEMS];

menuinfo_t plug_NR_5g_telit_tx_test_menu = {
    "%s Menu",
    0,
    (PFT)show_endnote,
    0,
    0,
    plug_NR_5g_telit_tx_test_pri_items,
};
menuinfo_t *plug_NR_5g_telit_tx_test_menup = &plug_NR_5g_telit_tx_test_menu;

static submenu_xtable_t plug_NR_5g_telit_table[] = {
    {"Thermal Sensor Test", 
     (type_t(*)())plug_NR_5g_telit_ts_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_NR_5g_telit_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Mandatory GPIO Expander (0x4E) Test", 
     (type_t(*)())plug_NR_5g_telit_man_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Optional GPIO Expander (0x4C) Test", 
     (type_t(*)())plug_NR_5g_telit_opt_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Detection Test", 
     (type_t(*)())plug_NR_5g_telit_modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 0 Card Test", 
     (type_t(*)())plug_NR_5g_telit_sim_test, SIM0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 1 Card Test", 
     (type_t(*)())plug_NR_5g_telit_sim_test, SIM1,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB 2.0 Detection Test", 
     (type_t(*)())plug_NR_5g_telit_usb2p0_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"I2C Reset Pin Test", 
     (type_t(*)())plug_NR_5g_telit_i2c_rst_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"W_DISABLE Pin Test", 
     (type_t(*)())plug_NR_5g_telit_w_disable_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem_Power_ON Pin Test", 
     (type_t(*)())plug_NR_5g_telit_modem_pwron_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"PCIe3.0 Test", 
     (type_t(*)())plug_NR_5g_telit_pcie3p0_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_NR_5g_telit_supports_pcie_intf, 0, (type_t(*)())0, 0},
    {"USB Debug Port Detection Test", 
     (type_t(*)())plug_NR_5g_telit_dport_test, TRUE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
     (type_t(*)())plug_test_not_supported, 0, (type_t(*)())0, 0},
    {"Modem Antenna RX Tests", 
     (type_t(*)())plug_NR_5g_telit_ant_rx_menu, 0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Antenna TX Tests", 
     (type_t(*)())plug_NR_5g_telit_ant_tx_menu, 0,
     MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"LED Control Utilities",
     (type_t(*)())plug_NR_5g_telit_led_ctrl_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Utilities",
     (type_t(*)())plug_NR_5g_telit_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_NR_5G_TELIT_TABLE_SZ \
        (sizeof(plug_NR_5g_telit_table) / sizeof(submenu_xtable_t))

static mitem_t plug_NR_5g_telit_pri_test_items[PLUG_NR_5G_TELIT_TABLE_SZ +
                                             MAX_BASE_ITEMS];
static mitem_t plug_NR_5g_telit_sec_test_items[PLUG_NR_5G_TELIT_TABLE_SZ +
                                             MAX_BASE_ITEMS];

static menuinfo_t plug_NR_5g_telit_test_menu = {
    "Pluggable NR_5G Telit module Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_NR_5g_telit_pri_test_items,
};
static menuinfo_t *plug_NR_5g_telit_test_menup = &plug_NR_5g_telit_test_menu;

static struct plug_intf_t *plug;

/*******************************************************************************
 * Function   : plug_NR_5g_telit_main
 * Description: Main entry point for Pluggable Telit modem test
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_NR_5g_telit_main (void *in)
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

    /* Store all USB port info that  modem might use from platform */
    plug_NR_5g_telit_store_usb_devinfo(plug->slot);

    /* Initialize GPIO Expander Output value */
    if (plug_NR_5g_telit_gpio_exp_out_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Expander Output Value Failed");
        ret = FAILED;
        goto __exit;
    }

    /* Initialize GPIO Expander Direction (Input/Output) */
    if (plug_NR_5g_telit_gpio_exp_dir_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Expander Direction Failed");
        ret = FAILED;
        goto __exit;
    }
   
    plug_NR_5g_telit_modem_pwr_ctrl(TRUE);
    /* Load USB serial driver for modem */
    plug_NR_5g_telit_insmod(TRUE);

    fflush(stdout);

    /* Restore modem to default test setup */
    printf("Restore modem to default setup...");
    if (plug_NR_5g_telit_set_modem_default_feature() != PASSED){
        ret = FAILED;
        cterr('f', 0, "Failed to set modem to default setup.");
        goto __exit;
    } else {
        modem_ready = TRUE;
        printf("OK\n");
    }

    if (plug->test_type == IFACE_TEST) {
    	printf ("\nInterface test");
        if (plug_NR_5g_telit_intf_test(0) != PASSED) {
            ret = FAILED;
        }
        goto __exit;
    }

    if (plug_NR_5g_telit_display_modem_info()) goto __exit;
    printf ("\n\n");

    build_primary_submenu(plug_NR_5g_telit_table, PLUG_NR_5G_TELIT_TABLE_SZ,
                          "Pluggable NR_5G Telit module", &plug_NR_5g_telit_test_menup);

    build_secondary_submenu(plug_NR_5g_telit_table, PLUG_NR_5G_TELIT_TABLE_SZ,
                            plug_NR_5g_telit_sec_test_items);

    if (plug->menu_display) {
        menu(&plug_NR_5g_telit_test_menu, plug_NR_5g_telit_sec_test_items, '\0');
    } else {
        do_all_menu_items(&plug_NR_5g_telit_test_menu);
    }

__exit:   
    if (modem_ready == TRUE) {
        if (plug_NR_5g_telit_modem_pwr_ctrl(FALSE) != PASSED) {
            cterr('f', 0, "Failed to soft power-off Telit modem");
        }
    }    

    plug_NR_5g_telit_insmod(FALSE);

    return (ret);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_ts_test
 * Description: Thermal Sensor test for Pluggable module 
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ts_test(int input)
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
 * Function   : plug_NR_5g_telit_man_gpio_exp_test
 * Description: GPIO Expander Test for Pluggable module
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_man_gpio_exp_test (void)
{
    printf("Mandatory GPIO Expander (0x4E) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");

    plug_NR_5g_telit_set_gpio_exp_test_reg(MANDATORY);

    if (plug_gpio_exp_reg_test(MANDATORY) != PASSED) {
        cterr('f', 0, "Mandatory GPIO Expander (0x4E) Test fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_opt_gpio_exp_test
 * Description: GPIO Expander Test for Pluggable module
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_opt_gpio_exp_test (void)
{
    printf("Optional GPIO Expander (0x4C) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");

    plug_NR_5g_telit_set_gpio_exp_test_reg(OPTIONAL);

    if (plug_gpio_exp_reg_test(OPTIONAL) != PASSED) {
        cterr('f', 0, "Optional GPIO Expander (0x4C) Test fails");
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_NR_5g_telit_display_modem_info
 * Description: Display  modem manufacturer, model, firmware.
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_display_modem_info (void)
{
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;
    int rc = FAILED;


    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_bootup_msg(
                                           (dev_object_t *)&plug_NR_5G_telit_obj);
    if (rc != PASSED) {
        cterr('f', 0, "Modem not responding");
    }
    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);

    return (rc);
}

/*******************************************************************************
 * Function   : plug_NR_5g_telit_modem_detect_test
 * Description: To detect  Telit modem for Pluggable module
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_modem_detect_test (int input)
{
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;
    int rc = FAILED;

    testname("Modem Detection");

    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem Detection");
    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_detection_test(
                                           (dev_object_t *)&plug_NR_5G_telit_obj);
    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
    }
    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_ant_rx_menu
 * Description: Rx test Menu  
 * Inputs     : NA
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ant_rx_menu (int opt)
{

    build_primary_submenu(plug_NR_5g_telit_rx_test_tbl,
                          PLUG_TELIT_RX_TEST_TBL_SZ,
                          "modem Antenna RX test",
                          &plug_NR_5g_telit_rx_test_menup);

    build_secondary_submenu(plug_NR_5g_telit_rx_test_tbl,
                            PLUG_TELIT_RX_TEST_TBL_SZ,
                            plug_NR_5g_telit_rx_test_sec_items);

    menu(&plug_NR_5g_telit_rx_test_menu, plug_NR_5g_telit_rx_test_sec_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_NR_5g_telit_ant_rx_test
 * Description: Antenna RX test for Pluggable module
 * Inputs     : test_option - Test all ant rx/individual
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ant_rx_test (int test_option)
{
    int i, opt = 0;
    int ant_to_test = 0;
    int steps_to_exe = 0;
    int rc, break_this_test = 0;


    for (i = 0; i < band_tbl_size; i++){
        if (nr_sub6_band_tbl[i].band_num == band_to_test){
            break;
        }
    }
    if ( i >= band_tbl_size){
        cterr('f', 0, "\nTest band :%d is not valid", band_to_test);
        return FAILED;
    }


    if (test_option == 0){ //All antenna test
        //Telit suggested to wait 120 seconds if soft reboot is issued
        //before starting testmode AT command
        printf ("\nWait for %d sec before starting the antenna test...\n", MODEM_ANT_WAIT_TIME);
        fflush(stdout);
        sleep (MODEM_ANT_WAIT_TIME);

        //Iterate all 5 antennas
        for (ant_to_test = 0 ; ant_to_test < MODEM_NUM_OF_ANT; ant_to_test++) {

            if (ant_to_test < MODEM_NUM_CELLULAR_ANT){
                steps_to_exe = (ant_to_test == NR_5G_ANTENNA_0) ? INIT_AND_GET_PWR : \
                               (ant_to_test == NR_5G_ANTENNA_3) ? GET_PWR_AND_EXIT : \
                                                                  GET_ONLY_PWR ;
                //CDETS:CSCvz00046, Telit case:228975 : FN980 RSSI readings jumps when 
                //issuing the AT testmode command multiple times.
                //Telit asked to add Offset of 2500Khz freq with Center frequency
                printf ("\nSet the Function generator frequency to %fMhz and " \
                         "Power to -60dBm", (float)(nr_sub6_band_tbl[i].band_freq + FC_OFFSET)/1000);
            } else {
                printf ("\nSet the Function generator frequency to 1575.52 and " \
                                                           "Power to -110dBm");
            }

            printf ("\nConnect the cable between Ant%d and Signal generator " \
                    "and press Enter to continue...", ant_to_test);
            getchar();

            do {
                //RSSI test
                if (ant_to_test < MODEM_NUM_CELLULAR_ANT){
                    rc = plug_NR_5g_telit_rssi_test (ant_to_test, steps_to_exe, band_to_test);
                } else {
                //GPS test
                    rc = plug_NR_5g_telit_gps_antenna_test(0);
                }

                //if the test result is pass move on to next antenna
                if (rc == PASSED) {
                    break_this_test = 1;
                }
                //if the test fails and its GNSS Ant quit the ant test
                //exit test mode and test manually
                else if (ant_to_test == NR_5G_ANTENNA_4) {
                    break_this_test = 1;
                } else {
                    opt = getdec_answer("Enter 0 to retest or 1 to continue "
                                        "or 2 to exit: ", 0, 0, 2);
                    if (opt == 0) {
                        break_this_test = 0;
                        if (ant_to_test == NR_5G_ANTENNA_3) {
                            steps_to_exe = GET_PWR_AND_EXIT;
                        } else {
                            steps_to_exe = GET_ONLY_PWR;
                        }
                    } else if (opt == 1) {
                        if (ant_to_test == NR_5G_ANTENNA_3){
                            //Exit the testmode.
                            plug_NR_5g_telit_enable_op_mode_util(0);
                            printf ("\nWait for 90 sec before starting the GPS test...");
                            fflush (stdout);
                            sleep (MODEM_GPS_WAIT_TIME);
                        }
                        break_this_test = 1;
                    } else {
                        break_this_test = 1;
                    }
                        
                } 
            } while (break_this_test != 1);

            //Check for failure
            if (rc != PASSED) {
                cterr('f', 0, "Antenna %d fails",ant_to_test );
            }

            //check for exit
            if ((rc != PASSED) && (opt == 2)) {
                break;
            }
        }
    }else {  //Individial antenna test
        printf ("\n!!!This Option is only for debugging purpose!!!");
        opt = getdec_answer("\nEnter which antenna to test : 0-Ant0, 1-Ant1, 2-Ant2, 3-Ant3, 4-Ant4 : ", 0, 0, 4);
        if (opt < MODEM_NUM_CELLULAR_ANT){
            //CDETS:CSCvz00046, Telit case:228975 : FN980 RSSI readings jumps when 
            //issuing the AT testmode command multiple times.
            //Telit asked to add Offset of 2500Khz freq with Center frequency
            printf ("\nSet the Function generator frequency to %fMhz and " \
                     "Power to -60dBm",(float)(nr_sub6_band_tbl[i].band_freq + FC_OFFSET)/1000);
        } else {
            printf ("\nSet the Function generator frequency to 1575.52 and " \
                                                       "Power to -110dBm");
        }
        printf ("\nConnect the cable between the Signal generator and Ant%d", opt);
        //Telit suggested to wait 120 seconds if soft reboot is issue
        //before starting testmode AT command
        printf ("\nWait for %d sec before starting the antenna test...\n", MODEM_ANT_WAIT_TIME);
        fflush(stdout);
        sleep (MODEM_ANT_WAIT_TIME);

        if (opt < MODEM_NUM_CELLULAR_ANT) {
            rc = plug_NR_5g_telit_rssi_test(opt, INIT_GETPWR_EXIT, band_to_test);
        } else {
            rc = plug_NR_5g_telit_gps_antenna_test(0); 
        }
    }


    if (rc != PASSED) {

        printf("\n[WARNING]Please perform the following process"
               " if the failure is caused by incorrect setup:\n\n"
               "    1. Go to 'Utilities' menu\n"
               "    2. Run 'Enable Modem Operation Mode Utility'\n\n"
               "Without the above process, the modem will keep in wrong state"
               " which might affect the other tests\n");
        return (FAILED);
    }

    if (plug_NR_5g_telit_soft_reboot(USB3P0) != PASSED) {
        cterr('f', 0, "Failed to reboot modem.");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_rssi_test
 * Description: To get Antenna RSSI for Pluggable module
 * Inputs     : which_connector - which antenna connector number
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_rssi_test (int which_connector, int test_seq, int band)
{
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;
    int rc = FAILED;
    char test_name[128] = {0, };

    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    sprintf(test_name, "\nModem Antenna #%s RSSI Test", 
                (which_connector == NR_5G_ANTENNA_0)  ? "ANT0"  : \
                (which_connector == NR_5G_ANTENNA_1)  ? "ANT1"   : \
                (which_connector == NR_5G_ANTENNA_2)  ? "ANT2" : \
                (which_connector == NR_5G_ANTENNA_3)  ? "ANT3" : \
                                                    "invalid port");

    testname(test_name);

    prpass(testpass, test_name);
    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_rssi_test((dev_object_t *)
                                                           &plug_NR_5G_telit_obj,
                                                           which_connector, band, test_seq);

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_ant_tx_menu
 * Description: Menu transmit  RSSI from Pluggable module
 * Inputs     : NA
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ant_tx_menu (int which_connector)
{

    build_primary_submenu(plug_NR_5g_telit_tx_test_tbl,
                          PLUG_TELIT_TX_TEST_TBL_SZ,
                          "modem Antenna TX test",
                          &plug_NR_5g_telit_tx_test_menup);

    build_secondary_submenu(plug_NR_5g_telit_tx_test_tbl,
                            PLUG_TELIT_TX_TEST_TBL_SZ,
                            plug_NR_5g_telit_test_sec_items);

    menu(&plug_NR_5g_telit_tx_test_menu, plug_NR_5g_telit_test_sec_items, '\0');

    return (PASSED);
}



/*******************************************************************************
 * Function   : plug_NR_5g_telit_ant_tx_test
 * Description: To transmit  RSSI from Pluggable module
 * Inputs     : which_connector - which antenna connector number
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_ant_tx_test (int which_connector)
{


    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;
    int rc = FAILED;
    char test_name[128] = {0, };
    float sgl_pwr;
    int band;

    sprintf(test_name, "Modem Antenna %s RSSI Test", 
                (which_connector == NR_5G_ANTENNA_0) ? "ANT0" :
                (which_connector == NR_5G_ANTENNA_1) ? "ANT1" :
                (which_connector == NR_5G_ANTENNA_2) ? "ANT2" : "invalid port");

    testname(test_name);

    prpass(testpass, test_name);
     
    printf ("\nWait for %d Sec to start modem trasmit signal...", MODEM_ANT_WAIT_TIME);
    fflush(stdout);
    sleep(MODEM_ANT_WAIT_TIME);

    switch (which_connector) {
        case NR_5G_ANTENNA_0:
            band = NR_5G_TX_CONFIG_N1;
            printf ("\nSet the test equipment to receive 1950 Mhz "
                                                    "and power +30dbm");
            break;
        case NR_5G_ANTENNA_1:
            band = NR_5G_TX_CONFIG_N79;
            printf ("\nSet the test equipment to receive 4699.95 Mhz "
                                                    "and power +30dbm");
            break;
        case NR_5G_ANTENNA_2:
            band = NR_5G_TX_CONFIG_N25;
            printf ("\nSet the test equipment to receive 1882.5 Mhz "
                                                    "and power +30dbm");
            break;
        default :
            cterr('f', 0, "Invalid antenna port");
            break;

    }
    
    fflush(stdout);

    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_ant_tx_test((dev_object_t *)
                                                           &plug_NR_5G_telit_obj,
                                                           which_connector, 
                                                           band);
    if (rc != PASSED) {
        goto _exit;
    }

    printf ("\nCheck the test equipment and enter the signal power level : ");
    scanf ("%f", &sgl_pwr);

    //expected reading is 23, Tolerance +/-5dbm
    if ((sgl_pwr < MODEM_TX_POWER_LOW ) || (sgl_pwr > MODEM_TX_POWER_HIGH)) {
        printf ("Expected power is between %d and %d",
                    MODEM_TX_POWER_LOW, MODEM_TX_POWER_HIGH);
        rc = FAILED;
        goto _exit;
    }

    rc |= plug_NR_5G_telit_obj_p->callin_fvt->modem_enable_op_mode((dev_object_t *)
                                                           &plug_NR_5G_telit_obj);
    if (rc != PASSED) {
        goto _exit;
    }

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);

    if (plug_NR_5g_telit_soft_reboot(USB3P0) != PASSED) {
        cterr('f', 0, "Failed to reboot modem.");
        return (FAILED);
    }

    printf ("\nwait additional 60sec as per telit ask");
    fflush (stdout);
    sleep (DELAY_60_SEC);

    return (PASSED);

_exit:
    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    cterr('f', 0, "%s fails", test_name);
    printf("\n[WARNING]Please perform the following process before re-test"
           " if the failure is caused by incorrect setup:\n\n"
           "    1. Go to 'Utilities' menu\n"
           "    2. Run 'Enable Modem Operation Mode Utility'\n\n"
           "Without the above process, the modem will keep in wrong state"
           " which might affect the other tests\n");
    return (FAILED);
}

/*******************************************************************************
 * Function   : plug_NR_5g_telit_gps_antenna_test
 * Description: To get GPS antenna information for Pluggable module 
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_gps_antenna_test (int input)
{
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;
    
    int rc = FAILED;
    char test_name[128] = {0, };

    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    sprintf(test_name, "Modem GNSS Antenna");

    testname(test_name);

    prpass(testpass, test_name);

    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_gps_test((dev_object_t *)
                                                          &plug_NR_5G_telit_obj);

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    
    return (rc);
    
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_sim_test
 * Description: To detect SIM 0 card for Pluggable module
 * Inputs     : test_sim - SIM0 or SIM1
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_sim_test (int test_sim)
{
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;
    int rc = FAILED;

    testname("SIM slot %d test", test_sim);

    /* Check whether SIM card is detected by GPIO expander */
    if (plug_NR_5g_telit_gpio_exp_sim_card_detect(test_sim) != TRUE) {
        cterr('f', 0, "SIM card is not detected by GPIO expander");
        return (FAILED);
    }

    plug_NR_5G_telit_sim_sel(test_sim);

    /* Read SIM card PIN request status through AT command to check
     * whether modem can access SIM card or not */
    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "SIM slot %d test", test_sim);
    rc =  plug_NR_5G_telit_obj_p->callin_fvt->modem_simin_pin_present(
                                           (dev_object_t *)&plug_NR_5G_telit_obj,
                                            test_sim);
    if (rc != PASSED) {
        cterr('f', 0, "SIM card is not detected by AT command");
        plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
        return (FAILED);

    }


    rc = plug_NR_5G_telit_obj_p->callin_fvt->sim_detect_test((dev_object_t *)
                                                           &plug_NR_5G_telit_obj,
                                                           test_sim);
    if (rc != PASSED) {
        cterr('f', 0, "SIM slot %d test fails", test_sim);
    }

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_dport_test
 * Description: Run USB enumeration on mini USB debug port
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_dport_test (int input)
{
    int rc = FAILED, restore_rc = FAILED;
    int modem_found_3p0 = FALSE;
    int modem_found_2p0 = FALSE;

    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;

    testname("USB Debug Port Detection");
    prpass(testpass, "USB debug port test");

    prpass(testpass, "Enable debug port.\n");

    /* 1. Switch modem to high-speed mode(USB2.0) */
    prpass(testpass, "Switch modem USB mode to USB2.0 mode.\n");
    /* Create device object */
    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                           (dev_object_t *)&plug_NR_5G_telit_obj,
                                            HIGH_SPD_USB);
    if (rc != PASSED) {
        cterr('f', 0, "Switching modem USB mode(2.0) fails");
        goto __exit;
    }

    /* 2. Enable USB debug port through GPIO expander */
    if (plug_NR_5g_telit_usb_deb_enable(TRUE) != PASSED) {
        cterr('f', 0, "Failed to enable debug port");
        return (FAILED);
    }

    msleep(PLUG_MODULE_ACT2_1P5_UNRESET_DELAY); 

    /* 3. Checks modem enumeration through USB 2.0 debug port */
    modem_found_2p0 = plug_NR_5g_telit_usb_is_found(DEBUG_USB, TRUE,
                                                  PROBE_NR_5G_TELIT_USB_TOUT);
    if (modem_found_2p0 != TRUE) {
        cterr('f', 0, "Telit Debug Port USB Enumeration Failed");
        goto __exit;
    } else {
        plug_NR_5g_telit_set_current_usb_port(DEBUG_USB);
    }

    /* 4. Restore modem to super-speed mode(USB3.0) */
    prpass(testpass, "Switch modem USB mode to USB3.0 mode.\n");
    restore_rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &plug_NR_5G_telit_obj,
                                                   SUPER_SPD_USB);
    if (restore_rc != PASSED) {
        cterr('f', 0, "Switching modem to USB mode(3.0) fails");
        goto __exit;
    }

    /* 4a. Disable USB debug port through GPIO expander */             
    prpass(testpass, "Disable debug port.\n");                        
    if (plug_NR_5g_telit_usb_deb_enable(FALSE) != PASSED) {
        cterr('f', 0, "Failed to disable debug port");                
        goto __exit;                                                  
    }                                 

        msleep(PLUG_MODULE_ACT2_1P5_UNRESET_DELAY); 

    /* 5. Polling USB3.0 bus to see if modem is detected */
    modem_found_3p0 = plug_NR_5g_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_NR_5G_TELIT_USB_TOUT);

    if (modem_found_3p0 != TRUE) {
        cterr('f', 0, "Modem is not found on USB3.0 bus");
        goto __exit;
    }
    plug_NR_5g_telit_set_current_usb_port(USB3P0);

    /* 6. Disable USB debug port through GPIO expander */
    prpass(testpass, "Disable debug port.\n");
    if (plug_NR_5g_telit_usb_deb_enable(FALSE) != PASSED) {
        cterr('f', 0, "Failed to disable debug port");
        goto __exit;
    }
    
    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    return (PASSED);
__exit:
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

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    plug_NR_5g_telit_set_current_usb_port(USB3P0);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_usb2p0_test
 * Description: Run USB enumeration in USB 2.0 mode from the host
 * Inputs     : input - not used 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_usb2p0_test (int input)
{
    int rc = FAILED, restore_rc = FAILED;
    int modem_found_2p0 = FALSE, modem_found_3p0 = FALSE;
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;

    testname("Modem USB2.0 interface");

    /* Create device object */
    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    prpass(testpass, "Modem USB2.0 interface Test");

    /* Switch modem to high-speed mode(USB2.0) */
    prpass(testpass, "Switch modem USB mode to USB2.0 mode.\n");
    rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                           (dev_object_t *)&plug_NR_5G_telit_obj,
                                           HIGH_SPD_USB);
    if (rc != PASSED) {
        cterr('f', 0, "Switching modem USB mode(2.0) fails");
        goto __exit;
    }

    /* Polling USB2.0 bus to see if modem is detected */
    modem_found_2p0 = plug_NR_5g_telit_usb_is_found(USB2P0, TRUE,
                                                  PROBE_NR_5G_TELIT_USB_TOUT);

    if (modem_found_2p0 != TRUE) {
        cterr('f', 0, "Modem is not found on USB2.0 bus");
        goto __exit;
    } else {
        plug_NR_5g_telit_set_current_usb_port(USB2P0);
    }

    /* Restore modem to super-speed mode(USB3.0) */
    prpass(testpass, "Switch modem USB mode to USB3.0 mode.\n");
    restore_rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &plug_NR_5G_telit_obj,
                                                   SUPER_SPD_USB);
    if (restore_rc != PASSED) {
        cterr('f', 0, "Switching modem to USB mode(3.0) fails");
        goto __exit;
    }

    /* Polling USB3.0 bus to see if modem is detected */
    modem_found_3p0 = plug_NR_5g_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_NR_5G_TELIT_USB_TOUT);

    if (modem_found_3p0 != TRUE) {
        cterr('f', 0, "Modem is not found on USB3.0 bus");
        goto __exit;
    }
    plug_NR_5g_telit_set_current_usb_port(USB3P0);

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    return (PASSED);
    
__exit:
    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    plug_NR_5g_telit_set_current_usb_port(USB3P0);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_i2c_rst_pin_test
 * Description: Function to test I2C_RESET_L
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_i2c_rst_pin_test (int input)
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
        cterr('f', 0, "Failed to reset Pluggable slot %d I2C bus",
              plug_i2c_rst->slot);
        return (FAILED);
    }
    printf ("\nAsserted the I2C reset");
    /* Check if we receive an ACK */
    ret = plug_common_fpga_i2c_ack_check(i2c_addr, PLUG_MODULE_FPGA_I2C_ACK_MUX,
                                         PLUG_MODULE_ACT2_ADD,
                                         PLUG_MODULE_FPGA_I2C_ACK_REG_ADD,
                                         PLUG_MODULE_FPGA_I2C_ACK_SUB_ADD,
                                         PLUG_MODULE_FPGA_I2C_ACK_DATA_LEN,
                                         data_buf); 
    /* Test fails if we still got an ACK while I2C_RESET_L is asserted */
    if (ret == PASSED) {
        cterr('f', 0, "Received an ACK from ACT2 chip while I2C bus is in reset"
              " mode(slot %d)", plug_i2c_rst->slot);
        return (FAILED);
    }

    /* Un-reset I2C interface */
    if (plug_i2c_rst->i2c_unreset((void*)plug_i2c_rst)) {
        cterr('f', 0, "Failed to unreset Pluggable slot %d I2C bus",
              plug_i2c_rst->slot);
        return (FAILED);
    }
    msleep(PLUG_MODULE_ACT2_1P5_UNRESET_DELAY);
     
    /* Check if we receive an ACK */
    ret = plug_common_fpga_i2c_ack_check(i2c_addr, PLUG_MODULE_FPGA_I2C_ACK_MUX,
                                         PLUG_MODULE_ACT2_ADD,
                                         PLUG_MODULE_FPGA_I2C_ACK_REG_ADD,
                                         PLUG_MODULE_FPGA_I2C_ACK_SUB_ADD,
                                         PLUG_MODULE_FPGA_I2C_ACK_DATA_LEN,
                                         data_buf); 
    if (ret != PASSED) {
        cterr('f', 0, "No ACK from ACT2 chip (slot %d)", plug_i2c_rst->slot);
    }

    return (ret);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_w_disable_pin_test
 * Description: Function to test W_DISABLE1# pin
 *              Modem will enter LPM(Low Power Mode) if W_DISABLE1# pin is
 *              asserted
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_w_disable_pin_test (int input)
{
    int lpm_rc = FALSE;
    int om_rc = FALSE;
    int set_psav_rc = FAILED;
    int ix;
    dev_NR_5g_telit_object_t plug_NR_5G_telit_obj;
    dev_NR_5g_telit_object_t *plug_NR_5G_telit_obj_p = &plug_NR_5G_telit_obj;

    testname("W_DISABLE pin");
    prpass(testpass, "W_DISABLE pin");

    if (plug_NR_5g_telit_dev_create(plug_NR_5G_telit_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    /* W_DISABLE1# pin is used to switch modem between operation mode and power
     * saving mode. And the modem power saving mode can be configured as LPM
     * (Low Power Mode), Power Saving Mode, DG(Dying Gasp) or no event to be
     * performed. In this test, we configured modem power saving mode as LPM */
    /* Check whether the modem power saving modem is configured as LPM */

    prpass(testpass, "Set modem power saving configuration...");
    set_psav_rc = plug_NR_5G_telit_obj_p->callin_fvt->
                                        modem_pwrsaving_mode_ctrl(
                                        (dev_object_t *)
                                        &plug_NR_5G_telit_obj,
                                        MODEM_LPM);
    if (set_psav_rc != PASSED) {
        cterr('f', 0, "Failed to set modem power saving mode.");
        goto __exit;
    }

    /* Assert W_DISABLE1# */
    prpass(testpass, "Set W_DISABLE pin to HIGH...");
    if (plug_NR_5g_telit_w_disable1_ctrl(HIGH) != PASSED) {
        cterr('f', 0, "Failed to assert WDISABLE1# pin.");
        goto __exit;
    }
    
    /* Check whether modem is in LPM(Low Power Mode) */
    for (ix = 0; ix < MODEM_MODE_SWITCHING_TOUT; ix ++) {
        lpm_rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_in_lpm(
                                                   (dev_object_t *)
                                                   &plug_NR_5G_telit_obj);
        if (lpm_rc == TRUE) {
            break;
        }
        msleep(PLUG_NR_5G_TELIT_POLLING_DELAY);
    }

    if (lpm_rc != TRUE) {
        cterr('f', 0, "Modem didn't switch to LPM as expected");
        goto __exit;
    }

    /* De-assert W_DISABLE1# */
    prpass(testpass, "Set W_DISABLE pin to LOW...");
    if (plug_NR_5g_telit_w_disable1_ctrl(LOW) != PASSED) {
        cterr('f', 0, "Failed to de-assert WDISABLE1# pin.");
        goto __exit;
    }

    /* Check whether modem is back to online mode */
    for (ix = 0; ix < MODEM_MODE_SWITCHING_TOUT; ix ++) {
        om_rc = plug_NR_5G_telit_obj_p->callin_fvt->modem_is_online(
                                                  (dev_object_t *)
                                                  &plug_NR_5G_telit_obj);
        if (om_rc == TRUE) {
            break;
        }
        msleep(PLUG_NR_5G_TELIT_POLLING_DELAY);
    }

    if (om_rc != TRUE) {
        cterr('f', 0, "Modem didn't switch to online mode as expected");
        goto __exit;
    }

    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    return (PASSED);

__exit:
    plug_NR_5G_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_NR_5G_telit_obj);
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_modem_pwron_pin_test
 * Description: Function to test Modem_Power_ON pin
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_modem_pwron_pin_test (int input)
{
    
    testname("Modem_Power_ON Pin");

    if (plug_NR_5G_telit_set_shutdown_indicator() == FAILED) {
        cterr('f', 0, "Failed to configure shutdown indicator");
        goto __exit;
    }

    /* Check modem GPIO pin value is correct or not according to Modem_Power_ON 
     * pin is set to low */
    prpass(testpass, "Set Modem_Power_ON pin to low.");
    if (plug_NR_5G_telit_set_modem_pwron_pin_test(LOW) != PASSED) {
        cterr('f', 0, "Failed to set Modem_Power_ON pin to low");
        goto __exit;
    }

    msleep(1000);

    /* Check modem GPIO pin value is correct or not according to Modem_Power_ON 
     * pin is set to high */
    prpass(testpass, "Set Modem_Power_ON pin to high.");
    if (plug_NR_5G_telit_set_modem_pwron_pin_test(HIGH) != PASSED) {
        cterr('f', 0, "Failed to set Modem_Power_ON pin to high");
        goto __exit;
    }

    //CSCvz00046: Telit case : 234478 Random modem crash on "AT#SHDNIND?"
    //Telit recommendation to add 2 sec delay after the book_ok pin 
    //detected high after power to the modem
    sleep(2);
    return (PASSED);

__exit:
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_pcie3p0_test
 * Description: Function to test PCIe3.0 interface.
 * Inputs     : input - not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_pcie3p0_test (int input)
{
    /* PCIe3.0 interface is not supported by Telit for now */
    printf("WARNING: PCIe interface is not yet supported by Telit!!\n\n");

    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_NR_5g_telit_intf_test
 * Description: To test all pluggable I/O interface, which are between host
 *              and pluggable module through host connector
 *              Telit module - USB2.0/USB3.0/I2C/I2C_RESET_L/
 *                             PCIe3.0
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_NR_5g_telit_intf_test (int input)
{
    int msku;
    int pcie_support_f = 0;

    plug_NR_5g_telit_get_modem_sku(&msku);

    /* Accessing modem to verify USB3.0 interface */
    if (plug_NR_5g_telit_modem_detect_test(0) != PASSED) {
        printf("Pluggable Telit USB interface test fails.\n");
        return (FAILED);
    }

    /* To verify USB2.0 interface */
    if (plug_NR_5g_telit_usb2p0_test(0) != PASSED) {
        printf("Pluggable Telit USB2.0 interface test fails.\n");
        return (FAILED);
    }

    /* To verify I2C_RESET_L pin */
    if (plug_NR_5g_telit_i2c_rst_pin_test(0) != PASSED) {
        printf("Pluggable  Telit I2C RESET pin test fails.\n");
        return (FAILED);
    }

    /* To verify I2C interface */
    if (plug_NR_5g_telit_opt_gpio_exp_test() != PASSED) {
        printf("Pluggable Telit I2C interface test fails.\n");
        return (FAILED);
    }

    /* To verify PCIe3.0 interface */
    /* PCIe interface is not supported by Telit, temporarily skip it */
    pcie_support_f = plug_NR_5g_telit_supports_pcie_intf();

    if (pcie_support_f == TRUE) {
        if (msku == PLUG_NR_5G_TELIT_FN980) {
            if (plug_NR_5g_telit_pcie3p0_test(0) != PASSED) {
                printf("Pluggable NR_5G Telit PCIe3.0 test fails.\n");
                return (FAILED);
            }
        }
    }
    
    return (PASSED);
}
/*********************************************************************
 * $Log: plug_NR_5G_telit_test.c,v $
 * Revision 1.4  2021/07/15 18:23:23  tshanmug
 * Sears PIM Rx test and Power ON pin test updated to fix the issues
 *
 * Revision 1.3  2021/06/04 02:21:41  tshanmug
 * Sears pim debug port test removed because pilot unit will not have it
 *
 * Revision 1.2  2021/06/02 02:56:20  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.2.7  2021/04/15 22:00:20  tshanmug
 * Sears come out of Antenna test on fail when exit option is selected
 *
 * Revision 1.1.2.6  2021/02/27 00:43:08  tshanmug
 * Sears code cleanup
 *
 * Revision 1.1.2.5  2021/02/12 01:08:19  tshanmug
 * Sears multi band test support
 *
 * Revision 1.1.2.4  2020/12/02 03:57:22  tshanmug
 * Sears Antenna test updated
 *
 *
 * $Endlog$
 */

