/* $Id: plug_lte_test.c,v 1.14 2020/01/17 03:06:05 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_test.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_test.c - PLUGGABLE LTE Main Functions
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
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
#include "plug_slot.h"
#include "plug_lte_util.h"
#include "plug_gpio_exp_test.h"
#include "plug_gpio_exp_lib.h"
#include "plug_temp_sensor_test.h"
#include "plug_host_fpga_lib.h"
#include "plug_common_lib.h"
#include "plug_lte_host.h"
#include "plug_lte_test.h"
#include "plug_lte_lib.h"
#include "plug_lte_at.h"

int plug_lte_main(void *);

static int plug_lte_ts_test(int);
static int plug_lte_intf_test(int);
static int plug_lte_man_gpio_exp_test(void);
static int plug_lte_opt_gpio_exp_test(void);
static int plug_lte_modem_detect_test(int);
static int plug_lte_main_rssi_test(int);
static int plug_lte_div_rssi_test(int);
static int plug_lte_gps_antennae_test(int);
static int plug_lte_sim_test(int);
static int plug_lte_dport_test(int);
static int plug_lte_gps_pin_test(int);
static int plug_lte_usb2p0_fwdl_test(int);
static int plug_lte_i2c_rst_pin_test(int);

extern int do_all_menu_items(struct menuinfo *);

static submenu_xtable_t plug_lte_table[] = {
    {"Thermal Sensor Test", (type_t(*)())plug_lte_ts_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_lte_has_temp_sensor, 0, (type_t(*)())0, 0},
    {"Mandatory GPIO Expander (0x4E) Test", (type_t(*)())plug_lte_man_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Optional GPIO Expander (0x4C) Test", (type_t(*)())plug_lte_opt_gpio_exp_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Detection Test", (type_t(*)())plug_lte_modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Main RSSI Test", (type_t(*)())plug_lte_main_rssi_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DIV RSSI Test", (type_t(*)())plug_lte_div_rssi_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem GPS Antenna", (type_t(*)())plug_lte_gps_antennae_test, TRUE,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 0 Card Test", (type_t(*)())plug_lte_sim_test, SIM0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM 1 Card Test", (type_t(*)())plug_lte_sim_test, SIM1,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())plug_lte_has_2_sim_slot, 0, (type_t(*)())0, 0},
    {"USB Debug Port Detection Test", (type_t(*)())plug_lte_dport_test, TRUE,
     MF_CONTINUOUS | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"USB 2.0 Detection Test", (type_t(*)())plug_lte_usb2p0_fwdl_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())is_plug_lte_em, 0, (type_t(*)())0, 0},
    {"GPS Pin Test", (type_t(*)())plug_lte_gps_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())is_plug_lte_wp, 0, (type_t(*)())0, 0},
    {"I2C Reset Pin Test", (type_t(*)())plug_lte_i2c_rst_pin_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Utilities", (type_t(*)())plug_lte_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define PLUG_LTE_TABLE_SZ \
        (sizeof(plug_lte_table) / sizeof(submenu_xtable_t))


static mitem_t plug_lte_pri_test_items[PLUG_LTE_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t plug_lte_sec_test_items[PLUG_LTE_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t plug_lte_test_menu = {
    "Pluggable LTE Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    plug_lte_pri_test_items,
};
static menuinfo_t *plug_lte_test_menup = &plug_lte_test_menu;

static struct plug_intf_t *plug;
char plug_lte_usb2p0_devinfo[64];
char plug_lte_usb3p0_devinfo[64];
char plug_lte_usb_dport_devinfo[64];

int modem_is_shutdown = FALSE;

/*******************************************************************************
 * Function   : plug_lte_main
 * Description: Main Entry point for Pluggable LTE 
 * Inputs     : *plug - Pointer to Pluggable Data structure
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_main (void *in)
{
    int ix;
    int modem_found = FALSE;
    int ret = PASSED;

    /* Sanity check */
    if (in == NULL) {
        cterr('f', 0, "Null pointer");
        return (FAILED);
    }

    plug = (struct plug_intf_t *)in;
    plug_lte_host_get_plug_usb_devinfo(plug->slot, plug_lte_usb2p0_devinfo,
                                       plug_lte_usb3p0_devinfo,
                                       plug_lte_usb_dport_devinfo);
    plug_lte_set_ctype((int)plug->id);

    /* Suppress printk so kernel doesn't print out bunch of messages */
    system(SYS_SUPPRESS_PRINTK);

    /* Initialize GPIO Expander Output Value */
    if (plug_lte_gpio_exp_out_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Direction Failed");
        ret = FAILED;
        goto __exit;
    }

    /* Initialize GPIO Expander Direction (Input/Output) */
    if (plug_lte_gpio_exp_dir_init() == FAILED) {
        cterr('f', 0, "Initialize GPIO Direction Failed");
        ret = FAILED;
        goto __exit;
    }

    plug_lte_modem_pwr_ctrl(TRUE);
    plug_lte_insmod(TRUE);

    /* Probe whether the modem is detected */
    if (is_plug_lte_em()) {
        printf("Probing the modem in USB 3.0 mode ...");
        fflush(stdout);

        for (ix = 0; ix < PROBE_LTE_USB_TOUT; ix++) {
            if (plug_lte_usb_detect(plug_lte_usb3p0_devinfo, 
                                    MODEM_SWI_USB_VID, PLUG_USB3P0_SPEED) 
                                    == PASSED) {
                modem_found = TRUE;
                break;
            }
            msleep(PLUG_LTE_POLLING_DELAY);
        }
    } else {
        printf("Probing the modem in USB 2.0 mode ...");
        fflush(stdout);

        for (ix = 0; ix < PROBE_LTE_USB_TOUT; ix++) {
            if (plug_lte_usb_detect(plug_lte_usb2p0_devinfo, 
                                    MODEM_SWI_USB_VID, PLUG_USB2P0_SPEED) 
                                    == PASSED) {
                modem_found = TRUE;
                break;
            }
            msleep(PLUG_LTE_POLLING_DELAY);
        }
    }

    if (modem_found == TRUE) {
        printf("OK\n");
    } else {
        cterr('f', 0, "SWI Modem is not detected");
        ret = FAILED;
        goto __exit;
    }

    /* Set the USB device info which is used for transmitting AT command */ 
    plug_lte_store_usb_devinfo(plug_lte_usb2p0_devinfo, 
                               plug_lte_usb3p0_devinfo);
    plug_lte_set_at_usb_devinfo(0);

    if (plug->test_type == IFACE_TEST) {
        if(plug_lte_intf_test(0) != PASSED) {
            ret = FAILED;
        }
        goto __exit;
    }

    /* Check SIM0 is inserted or not.
       If SIM card is inserted, check modem carrier is match or not */
    if (plug_lte_sim_detect(0) == TRUE) {
        /* Check modem carrier is matched */
        if (plug_lte_chk_modem_carrier_is_match() == FAILED) {
            ret = FAILED;
            goto __exit;
        }
    }

    /* Fix CSCvj58024:Set modem carrier to aviod modem auto switches image
     * while transmitting AT command with host */
    if (plug_lte_set_modem_carrier() == FAILED) {
        cterr('f', 0, "Failed to set LTE modem carrier");
        ret = FAILED;
        goto __exit;
    }
    
    build_primary_submenu(plug_lte_table, PLUG_LTE_TABLE_SZ, 
                          "Pluggable LTE", &plug_lte_test_menup);

    build_secondary_submenu(plug_lte_table, PLUG_LTE_TABLE_SZ,
                            plug_lte_sec_test_items);

    if (plug->menu_display) {
        menu(&plug_lte_test_menu, plug_lte_sec_test_items, '\0');
    } else {
        do_all_menu_items(&plug_lte_test_menu);
    }

__exit:
    /* If using utility to shutdown modem, flag "modem_is_shutdown"
       will be set to "TRUE" */
    if (modem_is_shutdown == FALSE) {
        if (modem_found == TRUE) {
            if (plug_lte_modem_shutdown() != PASSED) {
                ret = FAILED; 
            }
        }
    }
    plug_lte_insmod(FALSE);

    /* Set modem_is_shutdown to default value at the end of the test */
    modem_is_shutdown = FALSE;

    return (ret);
}

/*******************************************************************************
 * Function   : plug_lte_man_gpio_exp_test 
 * Description: GPIO Expander Test for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_man_gpio_exp_test (void)
{
    printf("Mandatory GPIO Expander (0x4E) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");

    plug_lte_set_gpio_exp_test_reg(MANDATORY);

    if (plug_gpio_exp_reg_test(MANDATORY) != PASSED) {
        cterr('f', 0, "Mandatory GPIO Expander (0x4E) Test fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_opt_gpio_exp_test 
 * Description: GPIO Expander Test for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_opt_gpio_exp_test (void)
{
    printf("Optional GPIO Expander (0x4C) Test\n");
    testname("GPIO Expander");
    prpass(testpass, "GPIO Expander");

    plug_lte_set_gpio_exp_test_reg(OPTIONAL);

    if (plug_gpio_exp_reg_test(OPTIONAL) != PASSED) {
        cterr('f', 0, "Optional GPIO Expander (0x4C) Test fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_ts_test
 * Description: Thermal Sensor Test for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_ts_test (int input)
{
    testname("Thermal Sensor");
    prpass(testpass, "Thermal Sensor");

    if (plug_temp_sensor_reg_test() != PASSED) {
        cterr('f', 0, "Thermal Sensor Test fails");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_modem_detect_test
 * Description: To detect LTE modem for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_modem_detect_test (int input)
{
    testname("Modem Detection");
    prpass(testpass, "Modem  Detection");

    if (plug_lte_at_run_cmd(RSSI_LTE_ATI_TEST) != PASSED) {
        cterr('f', 0, "Modem Detection failed");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_main_rssi_test
 * Description: To get LTE main RSSI for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_main_rssi_test (int input)
{
    int at_test_cmd;
    int ctype;

    testname("LTE Main RSSI");
    prpass(testpass, "LTE Main RSSI");

    /* Get controller type */
    plug_lte_get_ctype(&ctype); 

    if (is_plug_lte_em()) {
        at_test_cmd = RSSI_LTE_B8_MAIN_TEST;
        printf("\nFreq = %s MHz, Power = %s dBm\n", RSSI_B8_FREQ, RSSI_AMP);
    } else {
        /* WP7601/03/10 and WP7605/07/08/09 are using different band and frequency*/
        if ((ctype == PLUGGABLE_LTE_WP7601) || 
            (ctype == PLUGGABLE_LTE_WP7603) ||
            (ctype == PLUGGABLE_LTE_WP7610)) {
            at_test_cmd = RSSI_LTE_B4_MAIN_TEST;
            printf("\nFreq = %s MHz, Power = %s dBm\n",
                    RSSI_B4_FREQ, RSSI_AMP);
        } else {
            at_test_cmd = RSSI_LTE_B1_MAIN_TEST;
            printf("\nFreq = %s MHz, Power = %s dBm\n",
                    RSSI_B1_FREQ, RSSI_AMP);
        }
    }

    if (plug_lte_at_run_cmd(at_test_cmd) != PASSED) {
        cterr('f', 0, "LTE main RSSI failed");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_div_rssi_test
 * Description: To get LTE DIV RSSI for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_div_rssi_test (int input)
{
    int at_test_cmd;
    int ctype;

    testname("LTE DIV RSSI");
    prpass(testpass, "LTE DIV RSSI");

    /* Get controller type */
    plug_lte_get_ctype(&ctype); 

    if (is_plug_lte_em()) {
        at_test_cmd = RSSI_LTE_B8_DIV_TEST;
        printf("\nFreq = %s MHz, Power = %s dBm\n", RSSI_B8_FREQ, RSSI_AMP);
    } else {
        /* WP7601/03/10 and WP7605/07/08/09 are using different band and frequency*/
        if ((ctype == PLUGGABLE_LTE_WP7601) || 
            (ctype == PLUGGABLE_LTE_WP7603) ||
            (ctype == PLUGGABLE_LTE_WP7610)) {
            at_test_cmd = RSSI_LTE_B4_DIV_TEST;
            printf("\nFreq = %s MHz, Power = %s dBm\n",
                    RSSI_B4_FREQ, RSSI_AMP);
        } else {
            at_test_cmd = RSSI_LTE_B1_DIV_TEST;
            printf("\nFreq = %s MHz, Power = %s dBm\n",
                    RSSI_B1_FREQ, RSSI_AMP);
        }
    }

    if (plug_lte_at_run_cmd(at_test_cmd) != PASSED) {
        cterr('f', 0, "LTE DIV RSSI failed");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_gps_antennae_test
 * Description: To get LTE GPS antennae information for Pluggable LTE
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_gps_antennae_test (int input)
{
    testname("LTE GPS Antenna");
    prpass(testpass, "LTE GPS Antenna");
    printf("\nSignal Generator Freq = 1575.52 MHz, Power = -110dBm.\n");
    
    if (plug_lte_at_run_cmd(RSSI_LTE_GPS_TEST) != PASSED) {
        cterr('f', 0, "LTE GPS Antenna failed");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_sim_test
 * Description: To detect LTE SIM 0 card for Pluggable LTE
 * Inputs     : test_sim - SIM 0 or SIM 1
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_sim_test (int test_sim)
{
    char test_title[64];
    int at_cmd_test;

    sprintf(test_title, "LTE SIM-%d Detection", test_sim);

    testname(test_title);
    prpass(testpass, test_title);

    /* Detect SIM card through detection pin */
    if (plug_lte_sim_detect(test_sim) == FALSE) {
        cterr('f', 0, "SIM is not detected by GPIO Expander");
        return (FAILED);
    }

    if (is_plug_lte_em()) {
        if (test_sim == SIM0) {
            at_cmd_test = LTE_SIM0_DETECT_TEST;
        } else {
            at_cmd_test = LTE_SIM1_DETECT_TEST;
        }
    } else {
        /* WP modem */
        /* 1. Switch to the uims interface which is not in use
         *    to avoid power glitch
        */
        at_cmd_test = LTE_WP_SIM_PROTECT;

        if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
            cterr('f', 0, "Failed to switch SIM option");
            return (FAILED);
        }

        /* 2. Set SIM mux */
        plug_lte_sim_sel(test_sim);

        msleep(LTE_SIM_MUX_SWITCH_DELAY);

        /* WP modem only supports one SIM */
        at_cmd_test = LTE_SIM0_DETECT_TEST;
    }
    
    /* 3. Detect SIM card from Modem through AT command */
    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        cterr('f', 0, "SIM is not detected by modem");
        return (FAILED);
    }
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_usb2p0_fwdl_test
 * Description: Run USB enumeration in USB 2.0 mode from the host
 * Inputs     : input
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_usb2p0_fwdl_test (int input)
{
    int ret = FAILED;
    int at_cmd_test;
    int restore_ret = FAILED;
    int ix, retry, reset_count;
    int stat = PASSED;

    testname("USB 2.0 Detection via AT command");
    prpass(testpass, "USB2.0 Detection via AT command");

    at_cmd_test = PLUG_LTE_BOOT_MODE;

    for (retry = 1; retry <= USB2P0_MAX_RETRY_TIME; retry++) {
        for (reset_count = 1; reset_count <= MAX_RESET_TIME; reset_count++) {
            /* 1. Send AT command to enter to Boot&Hold mode */
            if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
                cterr('f', 0, "Failed to send set B&H mode command");
                goto __exit;
            }

            /* 3. Ensure the modem in USB3.0 disconnects with host */
            printf("Check if modem disconnects with host in USB3.0 mode...\n");
            if (plug_lte_check_modem_rdy(FALSE) != PASSED) {
                stat = FAILED;
            } else {
                stat = PASSED;
                printf("Modem disconnects with host.\n");
            }

            /* Hard reset LTE modem through GPIO exp. to recover modem 
             * if it's in wrong state 
             */
            if (stat == FAILED) {
                if (reset_count < MAX_RESET_TIME) {
                    printf("Hard reset modem, usb2.0 detection loop = %d"
                           ", reset_count = %d\n", retry, reset_count);
                    if (plug_lte_em_hard_reset() != PASSED) {
                        printf("Modem's in wrong state\n");
                    }
                } else {
                    cterr('f', 0, "Modem does not switch to USB2.0 mode");
                    goto __exit;
                }
            } else {
                break;
            }
        }

        printf("Detecting modem in USB 2.0 mode ...");
        fflush(stdout);
        /* 4. Check whether modem can be enumerated through USB 2.0 */
        for (ix = 0; ix < LTE_USB_SWITCH_MODE_TOUT; ix++) {
            ret = plug_lte_usb_detect(plug_lte_usb2p0_devinfo, 
                                      MODEM_SWI_USB_VID, PLUG_USB2P0_SPEED);
            if (ret == PASSED) {
                printf("OK\n");
                fflush(stdout);
                break;
            }
            msleep(PLUG_LTE_POLLING_DELAY);
        }

        /* Hard reset LTE modem through GPIO exp. to recover modem 
         * if it's in wrong state 
         */
        if (ret == FAILED) {
            if (retry < USB2P0_MAX_RETRY_TIME) {
                printf("Hard reset modem, usb2.0 detection loop = %d"
                       ", reset_count = %d\n", retry, reset_count);
                if (plug_lte_em_hard_reset() != PASSED) {
                    printf("Modem's in wrong state\n");
                }
            } else {
                cterr('f', 0, "USB 2.0 Detection failed");
                goto __exit;
            }
        } else {
            break;
        }
    }

    /* 5. Check if modem comes back in USB 3.0 mode */
    printf("Check if modem comes back in USB3.0 mode...\n");
    for (ix = 0; ix < LTE_USB_ENUM_TOUT; ix++) {
        restore_ret = plug_lte_usb_detect(plug_lte_usb3p0_devinfo, 
                                          MODEM_SWI_USB_VID, PLUG_USB3P0_SPEED);
        if (restore_ret == PASSED) {
            break;
        }
        msleep(PLUG_LTE_POLLING_DELAY);
    }

    msleep(MODEM_USB_RENUM_DELAY);

    if (restore_ret == PASSED) {
        printf("OK\n");
    } else {
        cterr('f', 0, "Restoring back to USB 3.0 failed\n");
        goto __exit;
    }

    return (PASSED);

__exit:
    printf("If test failes, prints out modem current temperature:\n");
    if (plug_lte_modem_temp_util(0) != PASSED) {
        printf("Failed to read modem temperature\n");
    }
    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_usb_dport_test
 * Description: Run USB enumeration on mini USB debug port
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_dport_test (int input)
{
    int ret = FAILED;
    int ix;

    testname("USB Debug Port Detection");
    prpass(testpass, "USB debug port test");

    /* Enable USB debug port through GPIO expander */
    plug_lte_usb_deb_enable(TRUE);

    /* Checks whether modem can be enumerated through USB 2.0 debug port */
    for (ix = 0; ix < LTE_USB_ENUM_TOUT; ix++) {
        ret = plug_lte_usb_detect(plug_lte_usb_dport_devinfo,
                                  MODEM_SWI_USB_VID, PLUG_USB2P0_SPEED);
        if (ret == PASSED) {
            break;
        }
        msleep(PLUG_LTE_POLLING_DELAY);
    }

    if (ret == FAILED) {
        cterr('f', 0, "LTE Debug Port USB Enumeration Failed");
        return (FAILED);
    }

    /* Disable USB debug port through GPIO expander */
    plug_lte_usb_deb_enable(FALSE);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_gps_pin_test 
 * Description: Function to check GPS pin value 
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_gps_pin_test (int input)
{
#ifdef GPS_PIN_TEST_W_ANTENNA    
    int ix, slot, at_cmd_test, gps_sync_status_bit;
    struct timeval t_start, t_curr;
    uint value, cost_time = 0;
#endif 
    testname("GPS Pin Test");
    prpass(testpass, "GPS Pin Test");

#ifdef GPS_PIN_TEST_W_ANTENNA    
    int gps_status;
    slot = plug->slot; 

    /* Enable GPS */
    at_cmd_test = LTE_GPS_ENABLE;
    printf("Enabling GPS...\n");
    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        cterr('f', 0, "Failed to enable GPS");
        return (FAILED);
    }

    /* Need some time to start reset, check if modem is in reset mode */
    stat = plug_lte_check_modem_rdy(FALSE);  
    if (stat != PASSED) {
        cterr('f', 0, "Modem failed to reset");
        return (FAILED);
    } else {
        stat = FAILED;
    }

    /* Check if modem is out of reset */
    if (plug_lte_check_modem_rdy(TRUE) != PASSED) {
        cterr('f', 0, "Pluggable LTE is not ready");
        return (FAILED);
    }

    msleep(MODEM_USB_RESET_DELAY);

    /* Enable GPS DR_SYNC feature */
    at_cmd_test = LTE_GPS_DR_SYNC_TEST;
    printf("Enable GPS Dead Reckoning Synchronization feature.\n");
    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        cterr('f', 0, "Failed to enable GPS DR feature");
        return (FAILED);
    }

    /* Check the current status of GPS position fix to see 
     * whether we can get GPS fixes or not */
    printf("Polling for GPS position fixes...\n");
    at_cmd_test = LTE_GPS_FIXES_STATUS;
    for (ix = 0; ix < GPS_FIXES_MAX_RETRY_TIME; ix ++) {
        stat = plug_lte_at_run_cmd(at_cmd_test);
        if (stat == PASSED) {
            printf("Got GPS fixes.\n");
            break;
        }
        msleep(PLUG_LTE_POLLING_DELAY);
    }
    if (stat != PASSED) {
        cterr('f', 0, "Failed to get GPS fixes");
        return (FAILED);
    } else {
        stat = FAILED;
    }

    /* Polling Sirius FPGA for GPS DR_SYNC pulse */
    printf("Polling for GPS DR_SYNC pulse.\n");
    gettimeofday(&t_start, NULL);
    while (cost_time < MAX_POLLING_TIME) {
        if (plug_lte_get_gps_pin_status(slot, &gps_status) != PASSED) {
            return (FAILED);
        }
        if (gps_status) {
            printf("Got GPS dead reckoning pulse.\n");
            stat = PASSED;
            break;
        }
        gettimeofday(&t_curr, NULL);
        cost_time = (t_curr.tv_sec - t_start.tv_sec);
    }
    if (stat != PASSED) {
        cterr('f', 0, "GPS Pin Test Failed");
        return (FAILED);
    }
#else   /* Test with diagnostic mode */ 
    /* Force GPS DR_SYNC high */
    if (plug_lte_force_gps_pin_val(plug->slot, HIGH) != PASSED) {
        cterr('f', 0, "Failed to set GPS DR_SYNC signal");
        return (FAILED);
    }

    /* Force GPS DR_SYNC low */
    if (plug_lte_force_gps_pin_val(plug->slot, LOW) != PASSED) {
        cterr('f', 0, "Failed to clear GPS DR_SYNC signal");
        return (FAILED);
    }

    /* Since we use "AT!BSGPIO" command to force the DR_SYNC signal high/low.
     * This procedure is only for diagnostic use and need to reset or power
     * cycle the modem to return normal signal functionality.
     */
    printf("\n!!!!Please power-cycle the modem after running this test!!!!\n");
#endif    
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_i2c_rst_pin_test 
 * Description: Function to test I2C_RESET_L pin
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_i2c_rst_pin_test (int input)
{
    int ret = FAILED;
    int i2c_addr = PLUG_I2C_CTRL_OFFSET; 
    struct plug_intf_t *plug_i2c_rst;
    uchar data_buf[32];

    testname("I2C RESET pin test");
    prpass(testpass, "I2C RESET pin test");

    plug_i2c_rst = plug;
    /* Setup the register for pluggable FPGA I2C control */
    if (plug_i2c_rst->slot == PLUG_SLOT_2) {
        i2c_addr = i2c_addr + PLUG_FPGA_I2C_OFFSET;
    }

    /* Assert I2C_RESET_L pin */
    if (plug_i2c_rst->i2c_reset((void*)plug_i2c_rst)) {
        cterr('f', 0, "Reset Pluggable LTE slot %d fail", plug_i2c_rst->slot);
        return (FAILED);
    }

    /* Check if we recevice an ACK */
    ret = plug_common_fpga_i2c_ack_check(i2c_addr, PLUG_LTE_FPGA_I2C_ACK_MUX,
                                         PLUG_LTE_ACT2_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_REG_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_SUB_ADD,
                                         PLUG_LTE_FPGA_I2C_ACK_DATA_LEN,
                                         data_buf);

    /* Test fails if we still got an ACK while I2C_RESET_L is asserted */
    if (ret == PASSED) {
        ret = FAILED;
    } else {
        ret = PASSED;
    }

    /* Un-reset I2C interface */
    if (plug_i2c_rst->i2c_unreset((void*)plug_i2c_rst)) {
        cterr('f', 0, "Unreset testcard slot %d fail", plug_i2c_rst->slot);
        return (FAILED);
    }
    msleep(PLUG_MODULE_I2C_UNRESET_DELAY);
    
    if (ret == PASSED) {
        return (PASSED);
    } else {
        cterr('f', 0, "module reset pin test slot %d fail", plug_i2c_rst->slot);
        return (FAILED);
    }
}


/*******************************************************************************
 * Function   : plug_lte_intf_test
 * Description: To test pluggable LTE I/O interface
 *              EM module - USB2.0/USB3.0/I2C
 *              WP module - USB2.0/I2C/GPS pin
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_intf_test (int input)
{
    /* Accessing modem to verify USB interface */
    if (plug_lte_modem_detect_test(0) != PASSED) {
        printf("Pluggable USB interface test failed.\n");
        return (FAILED);
    }

    /* Running mandatory GPIO Expander register test to verify I2C interface */
    if (plug_lte_man_gpio_exp_test() != PASSED) {
        printf("Pluggable I2C interface test failed.\n");
        return (FAILED);
    }
    printf("Pluggable I2C interface test passed.\n");

    /* Running I2C reset pin test to verify I2C_RESET_L pin */
    if (plug_lte_i2c_rst_pin_test(0) != PASSED) {
        printf("Pluggable I2C reset pin test failed.\n");
        return (FAILED);
    }
    printf("Pluggable I2C reset pin test passed.\n");

    /* For LTE-EM module, test USB2.0 interface */
    if (is_plug_lte_em() == TRUE) {
        if (plug_lte_usb2p0_fwdl_test(0) != PASSED) {
            printf("Pluggable USB2.0 interface test failed.\n");
            return (FAILED);
        }
    } else {  
        /* For LTE-WP module, need to test GPS pin */
        /* Note that the current GPS pin test procedure is only suitable for
         * WP76xx modem
         */
        if (plug_lte_gps_pin_test(0) != PASSED) {
            printf("Pluggable LTE GPS pin test failed.\n");
            return (FAILED);
        }
    }
        
    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_lte_test.c,v $
Revision 1.14  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.13  2019/08/15 09:27:51  shjung
Supported WP7610 PIM

Revision 1.12  2019/06/14 05:48:11  shjung
Supported WP7605 modules

Revision 1.11  2018/11/23 09:15:07  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.10.32.6  2018/11/21 01:02:50  shjung
Added GPIO expander test register table and modified RF test macro name based on test RF band

Revision 1.10.32.5  2018/11/06 08:26:28  shjung
Implement error message of GPIO expander tests

Revision 1.10.32.4  2018/10/24 06:23:21  shjung
Added I2C reset pin test in I/O interface test, and modified kernel debug message level

Revision 1.10.32.2  2018/10/15 07:43:26  shjung
Re-struct for pluggable-LTE common codes

Revision 1.10.32.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.10  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.9  2018/05/21 08:11:29  shjung
Merged code from star-branch-c110x

Revision 1.8  2018/04/13 09:35:00  shjung

1. Fix CSCvh79986 and CSCvh79979: Added modem tty device file descriptor
   slef test to ensure communication between host and modem is good
2. Modified code based on Pluggable LTE WP7601/03 ER code review
3. Put all cterr functions to the outer file
4. Modified modem USB device enumeration timeout and GPS pin vaule polling
   timeout

Revision 1.7  2018/03/29 10:26:53  shjung
Remove modem reset test

Revision 1.6  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.5  2018/02/27 07:03:24  shjung
Remove debug port test from default test item

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.6  2018/05/21 07:40:24  shjung
Based on PRRQ code review(CSCvj53467) comment: Check return value of soft power-off function

Revision 1.3.2.3  2018/03/23 06:15:36  shjung
Slow down USB write speed from host to LTE modem

Revision 1.3.2.2  2018/03/16 07:47:28  shjung
1. Correct the default value of WP SAFE_POWER_REMOVAL and implement WP modem power-off function 2. Check modem status after hard reset

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:45  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.4  2018/02/01 23:41:02  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.3  2018/01/20 20:00:23  shjung
Remove Dying Gasp/Super Caps Charging Test

Revision 1.2.2.2  2018/01/20 06:56:34  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:09  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.21  2018/01/09 06:10:00  shjung
Added test criteria for the hold-up time of super caps, which are used for dying gasp feature

Revision 1.1.4.20  2017/12/15 08:16:52  shjung
Set longer polling time for modem reset test

Revision 1.1.4.19  2017/12/15 08:04:18  shjung
Replaced temperature sensor with GPIO expander register test and added GPS pin test in I/O interface test

Revision 1.1.4.18  2017/12/14 00:38:16  hondwang
Fix plug LTE USB2.0 detection issue.

Revision 1.1.4.17  2017/12/13 15:14:51  shjung
Added dying gasp test for pluggable LTE-EM module

Revision 1.1.4.16  2017/12/13 09:26:17  shjung
Added check mechanism to ensure the connectivity of modem to host is as expected

Revision 1.1.4.15  2017/12/13 08:33:24  shjung
Added diagnostic test mode for pluggable LTE-WP76xx GPS pin test

Revision 1.1.4.14  2017/12/08 12:28:46  shjung
Check if usb device attaches to tty successfully before capture corresponding ttyUSB number

Revision 1.1.4.13  2017/12/06 13:45:49  shjung
Modified SIM detection on LTE-WP modem to avoid power glitch

Revision 1.1.4.12  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.11  2017/11/08 02:56:07  shjung
Modified the timeout mechanism of GPS pin test

Revision 1.1.4.10  2017/10/30 14:15:14  shjung
Added GPS pin test for LTE-WP module

Revision 1.1.4.9  2017/10/25 04:40:50  shjung
Modified pluggable module USB interface power-on/off sequence and USB interface mode configuration

Revision 1.1.4.8  2017/09/06 01:37:27  shjung
Code clean up.

Revision 1.1.4.7  2017/09/04 14:41:02  shjung
Correct test flag

Revision 1.1.4.6  2017/08/30 02:03:46  shjung
Update AT command for modem reset and ensure modem finish reset test

Revision 1.1.4.5  2017/08/28 07:53:27  shjung
Added pluggable I/O interface test

Revision 1.1.4.4  2017/08/16 08:31:58  tirawan
Re-enumerate USB modem by reconnecting modem when enabling USB 2.0 or USB 3.0

Revision 1.1.4.3  2017/08/15 14:08:38  hondwang
star branch c9xx initial check in

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.4  2017/07/26 01:50:52  tirawan
Fix SIM select mux for WP and extend AT Command timeout to 30secs

Revision 1.1.2.3  2017/07/24 22:51:25  tirawan
Add Pluggable AT command functions

Revision 1.1.2.2  2017/07/20 17:22:50  tirawan
Add USB 2.0 test and Debug port, and host implementation function prototype

Revision 1.1.2.1  2017/07/13 06:32:21  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.3  2017/06/25 06:41:23  tirawan
Initialize GPIO Expander Output port before configuring its direction

Revision 1.1.2.2  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

