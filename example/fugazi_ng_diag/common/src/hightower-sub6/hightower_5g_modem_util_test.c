/* $Id: hightower_5g_modem_util_test.c,v 1.4 2021/06/30 20:04:56 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/hightower_5g_modem_util_test.c,v $
 *********************************************************************
 *
 * hightower_5g_modem_util_test.c -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
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
#include "gpio.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "queryflags.h"
#include "proto.h"
#include "hightower_5g_modem_lib.h"
#include "hightower_sub6.h"
#include "highrise_cpld_lib.h"

static int modem_run_at_cmd (int input);
static int modem_soft_reset_util (int input);
static int modem_temp_util(int);
static int modem_ext_usb_util (int opt);
static int diag_not_exist (int );
static int simdetect_pin_test (int sim_no);
static int simdet_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt);
static int show_simdetect_pin_status (int sim_num);
static int show_simdetect_gpiopin_status (int sim_num);
static int modem_rssi_utils (int);
static int modem_pwr_on_off_utils (int);

extern int diag_modem_get_tty_devname (char *tty_dev);
extern int modem_power_on (int dummy);
extern nr_sub6_band_struct nr_sub6_band_tbl[];
extern int band_tbl_size;
extern char modem_fw_version[64];

static submenu_xtable_t modem_utils[] = {
    {"AT Command Utility", (type_t(*)())modem_run_at_cmd, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Temperature Display Utility", (type_t(*)())modem_temp_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Host GPIO SIM_DETECT pin Status(SIM0)", 
     (type_t(*)())show_simdetect_gpiopin_status, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Host GPIO SIM_DETECT pin Status(SIM1)", 
     (type_t(*)())show_simdetect_gpiopin_status, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem SIM_DETECT pin Test(SIM0)",
     (type_t(*)())simdetect_pin_test, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem SIM_DETECT pin Test(SIM1)",
     (type_t(*)())simdetect_pin_test, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem SIM_DETECT pin Status(SIM0)", 
     (type_t(*)())show_simdetect_pin_status, SIM0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Show Modem SIM_DETECT pin Status(SIM1)", 
     (type_t(*)())show_simdetect_pin_status, SIM1,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem Soft Reset Utility", (type_t(*)())modem_soft_reset_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"External USB Debug bus Enable/Disable Utility",
     (type_t(*)())modem_ext_usb_util, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"mux select to Sim 0", (type_t(*)())diag_swi_5g_sim_selection, SIM0,
     0, (type_t(*)())diag_not_exist, 0, (type_t(*)())0, 0},
    {"mux select to Sim 1", (type_t(*)())diag_swi_5g_sim_selection, SIM1,
     0, (type_t(*)())diag_not_exist, 0, (type_t(*)())0, 0},
    {"RSSI Utility", (type_t(*)())modem_rssi_utils, TRUE,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"modem power on", (type_t(*)())modem_pwr_on_off_utils, 1,
     0, (type_t(*)())diag_not_exist, 0, (type_t(*)())0, 0},
    {"modem power off", (type_t(*)())modem_pwr_on_off_utils, 0,
     0, (type_t(*)())diag_not_exist, 0, (type_t(*)())0, 0},
};

#define MODEM_UTIL_TABLE_SZ \
        (sizeof(modem_utils) / sizeof(submenu_xtable_t))


static mitem_t modem_pri_util_items[MODEM_UTIL_TABLE_SZ+ MAX_BASE_ITEMS];
static mitem_t modem_sec_util_items[MODEM_UTIL_TABLE_SZ+ MAX_BASE_ITEMS];

static menuinfo_t modem_util_menu = {
    "Modem Utilities Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    modem_pri_util_items,
};
static menuinfo_t *modem_util_menup = &modem_util_menu;


/*******************************************************************************
 * Function   : hightower_modem_util
 * Description: Main Entry point for Pluggable LTE Utilities
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int hightower_modem_util (void)
{
    build_primary_submenu(modem_utils, MODEM_UTIL_TABLE_SZ,
                         "5G MODEM Utility", &modem_util_menup);

    build_secondary_submenu(modem_utils, MODEM_UTIL_TABLE_SZ,
                            modem_sec_util_items);

    menu(&modem_util_menu, modem_sec_util_items, '\0');

    return (PASSED);
}

/*******************************************************************************
 * Function   : modem_soft_reset_util
 * Description: Do the modem reset via AT command
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */

static int modem_soft_reset_util (int input)
{
    int rc = FAILED;
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;

    testname("Modem soft reset util");
    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create Telit Dev Object Fails");
        return (FAILED);
    }

    rc = diag_5g_swi_obj_p->callin_fvt->modem_reset_test(
                                           (dev_object_t *)&diag_5g_swi_obj);

    return rc;
}

/*******************************************************************************
 * Function   : modem_temp_util
 * Description: To get the modem temperature and print it on the screen
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_temp_util (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;
	
    testname("Modem Temp utils");
    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    rc = diag_5g_swi_obj_p->callin_fvt->modem_temperature_test(
                                           (dev_object_t *)&diag_5g_swi_obj);
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : modem_run_at_cmd
 * Description: To execute AT command for modem 
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_run_at_cmd (int input)
{
    const int maxlen = 128;
    char cmd[maxlen];
    char tty_dev_name[256];
    char modem_tty_port[15];

    printf("\n\n ### NOTE: Type CTRL-x "
                              "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(AT_COMMAND_UTIL_DELAY);

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB
     */
    if (diag_modem_get_tty_devname(tty_dev_name) != PASSED) {
        printf("%s:Can't get ttyUSB number\n", __func__);
        return (FAILED);
    }

    sprintf(modem_tty_port, "%s%s", TTY_PATH, tty_dev_name); 

    snprintf(cmd, maxlen-1, "microcom %s", modem_tty_port);

    system(cmd);

    return (PASSED);
}

/*******************************************************************************
 * Function   : modem_ext_usb_util
 * Description: External USB Debug Enable/Disable Utility
 * Inputs     : input - Not used
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int modem_ext_usb_util (int input)
{
    int opt;

    testname("External USB Enable/Disable utils");
    prpass(testpass, "SWI NR_5G  ");
    opt = getdec_answer("Enable/Disable Debug USB? (0-Disable, 1-Enable): ",
                        OPT_DISABLE, OPT_DISABLE, OPT_ENABLE);

    if (opt == OPT_ENABLE)  {
        printf("Enable Debug USB.\n");
        diag_swi_5g_usb_deb_enable(OPT_ENABLE);
    } else {
        printf("Disable Debug USB.\n");
        diag_swi_5g_usb_deb_enable(OPT_DISABLE);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_not_exist
 * Description: To mask the test
 * Inputs     : input - Not used
 * Outputs    : Pass
 *******************************************************************************
 */
static int diag_not_exist (int input) {
    return (0);

}

/*******************************************************************************
 * Function   : simdetect_pin_test
 * Description: Function to test modem SIM_DETECT using modem GPIO pin.
 *              This test verifies SIM0_DETECT pin and SIM1_DETECT pin
 * Inputs     : sim_no - SIM slot number(0/1)
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int simdetect_pin_test (int sim_no)
{
    /* This utility test is to enhance the coverage of SIM_DETECT
     * pin. It requires users to insert and remove SIM card during the test.
     */

    char test_name[TESTMSG_BUFSZ];


    memset(test_name, 0, sizeof(test_name));
    sprintf(test_name, "SIM%d_DETECT pin", sim_no);
    testname(test_name);
    prpass(testpass, "SWI NR_5G  ");

    if (diag_swi_5g_sim_selection (sim_no) == FAILED) {
        cterr('f', 0, "SIM selection failed");
        return (FAILED);

    }

    /* Test SIM_DETECT pin when SIM is present */
    if (simdet_pin_test(sim_no, SIM_PRESENT, ENABLE)
                                  != PASSED) {
        cterr('f', 0, "Failed, SIM%d is inserted "
              "but SIM_DETECT state is Low.", sim_no);
        return (FAILED);
    }

    if (simdet_pin_test(sim_no, SIM_NOT_PRESENT, ENABLE)
                                  != PASSED) {
        cterr('f', 0, "Failed, SIM%d is NOT inserted "
              "but SIM_DETECT state is High.", sim_no);
        return (FAILED);
    }

    return (PASSED);
}



/*******************************************************************************
 * Function   : simdet_pin_test
 * Description: Wrapped function to test modem SIM_DETECT pin.
 *              This test verifies SIM0_DETECT pin and SIM1_DETECT pin by
 *              reading modem SIMDETECT status through AT command
 * Inputs     : sim_num - SIM number(0/1)
 *              exp_sim_stat - expected status of SIM_DETECT pin
 *              usr_prompt - flag to determine whether to show up user prompt
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int simdet_pin_test (int sim_num, boolean exp_sim_stat,
                                      boolean usr_prompt)
{
    char usr_input = 0;
    char usr_act_str[TESTMSG_BUFSZ];
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    memset(usr_act_str, 0, sizeof(usr_act_str));

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create Dev Object Fails");
        return (FAILED);
    }

    /* Set user prompt string */
    if (exp_sim_stat == SIM_PRESENT) {
        sprintf(usr_act_str, "install SIM card to");
    } else {
        sprintf(usr_act_str, "remove SIM card from");
    }

    /* Print out user prompt if needed */
    if (usr_prompt == ENABLE) {
        printf("\n\n### Please %s SIM slot %d.\n", usr_act_str, sim_num);
        do {
            printf("\r### Press 'y' to continue the Test: ");
            usr_input = getchar();
            if (usr_input == 'y') {
                break;
            }
        } while (usr_input != 'y');
    }

    /* Check whether the SIM_DETECT pin is present */
    rc = diag_5g_swi_obj_p->callin_fvt->sim_detect_pin_present(
           (dev_object_t *)&diag_5g_swi_obj, exp_sim_stat, 0); //Only one external sim

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Unexpected SIM%d_DETECT pin status.", sim_num);
    }
    fflush(stdout);
    return (rc);
}

/*******************************************************************************
 * Function   : show_simdetect_pin_status
 * Description: Function to dump the status of modem SIM_DETECT pin.
 *              Based on comment from SWI(Sierra wireless):
 *              AT!BSGPIO?68 can be used to check the state of SIM_DETECT pin.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */

static int show_simdetect_pin_status (int sim_num) {
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    if (diag_swi_5g_sim_selection (sim_num) == FAILED) {
        cterr('f', 0, "SIM selection failed");
        return (FAILED);

    }
    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create Dev Object Fails");
        return (FAILED);
    }

    /* Check whether the SIM_DETECT pin is present */
    rc = diag_5g_swi_obj_p->callin_fvt->display_sim_detect_stat(
                                           (dev_object_t *)&diag_5g_swi_obj, 0);
    if (rc != PASSED) {
        cterr('f', 0, "Unexpected SIM%d_DETECT pin status.", sim_num);
    }
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    fflush(stdout);
    return (rc);


}

/*******************************************************************************
 * Function   : show_simdetect_gpiopin_status
 * Description: Function to dump the status of host SIM_DETECT pin.
 * Inputs     : sim_num - SIM number(0/1)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */

static int show_simdetect_gpiopin_status (int which_sim) 
{
    int val = HIGH, rc;
    if (which_sim == SIM0) {
        /* CP_MPP[27] value 0 mean present*/
        rc = gpio_read(SIM0_DETECT_L, &val);
        if (rc == -1) {
            printf("GPIO read failed\n");
            return (FAILED);
        }
    } else if (which_sim == SIM1) {
        /* CP_MPP[24] value 0 mean present*/
        rc = gpio_read(SIM1_DETECT_L, &val);
        if (rc == -1) {
            printf("GPIO read failed\n");
            return (FAILED);
        }
    } else {
        printf("%s:Invalid SIM slot number(%d)\n", __func__, which_sim);
        return (FAILED);
    }

    printf ("\nSim : %d Host GPIO SIM detect status : %d", 
                                     (which_sim == SIM0) ? 0 : 1, val);
                              

    return PASSED;

}

/*******************************************************************************
 * Function   : modem_rssi_utils
 * Description: Function to test supported RX RSSI test for each antenna 
 * Inputs     : not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_rssi_utils (int flag) {
    int which_ant, which_band, option;
    int i;
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;
    int exp_power = SUB6_CONDUCT_TEST_EXP_POWER;

    printf ("\nwhich antenna to test (0-Main, 1-Aux, 2-MIMO1, 3-MIMO2) : ");
    scanf ("%d", &option);

    if ((option < 0) || (option > 3)){
        printf ("\nInvalid antenna port");
        return FAILED;
    }

    switch (option) {
        case 0:
            which_ant = MAIN_RSSI;
            break;
        case 1:
            which_ant = AUX_RSSI;
            break;
        case 2:
            which_ant = MIMO1_RSSI;
            break;
        case 3:
            which_ant = MIMO2_RSSI;
            break;
    }

    testname("Modem %s antenna RSSI",
              (which_ant == MAIN_RSSI) ? "main" :
              (which_ant == AUX_RSSI)  ? "AUX" :
              (which_ant == MIMO1_RSSI)? "mimo1" : "mimo2");

    prpass(testpass, "SWI NR_5G  ");


    printf ("\n\nList of supported bands and their freq in Mhz: \n");
    for (i = 0; i < band_tbl_size; i++){
        if (nr_sub6_band_tbl[i].test_supported_ant & which_ant)
            printf ("%d:%s  ",nr_sub6_band_tbl[i].band_num,
                              nr_sub6_band_tbl[i].rx_center_freq);
    }

    printf ("\n\nSelect a band to test : ");
    scanf ("%d", &which_band);

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create Dev Object Fails");
        return (FAILED);
    }


    rc = diag_5g_swi_obj_p->callin_fvt->modem_sub6_rssi_test(
           (dev_object_t *)&diag_5g_swi_obj, which_ant, 
               which_band, exp_power);

    if (rc != PASSED) {
        cterr('f', 0, "%s fails", testpass);
    }

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    fflush(stdout);
    return (rc);
}

/*******************************************************************************
 * Function   : modem_power_on_off
 * Description: Function to power on and off the modem
 * Inputs     : 0 - off 1- ON
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_pwr_on_off_utils (int on_off) {
    unsigned long int reg = 0 ;
    int rc = PASSED;

    if (on_off == 1) //Modem power ON
    {
        printf ("\nPower on the modem");
        rc |= hr_cpld_reg_read_32 (HR_CPLD_PWR_CYCLE, &reg);
        reg = reg & ~(0xf0);
        reg |= MODEM_CPLD_PWR_ON;
        rc |= hr_cpld_reg_write_32 (HR_CPLD_PWR_CYCLE, reg);

        sleep (DELAY_2SEC);

        rc |= hr_cpld_reg_read_32 (HR_CPLD_MODEM_STATUS, &reg);
        if ((reg & HR_CPLD_MODEM_STA_VREG_PWR_ON) == 0)  {
            cterr('f', 0, "PLA not detected (is still low).");
        }
        fflush(stdout);
        rc |= modem_power_on(0); //dummy param
        return rc;
    
    }else {
        printf ("\nPower off the modem");

        rc |= hr_cpld_reg_read_32 (HR_CPLD_PWR_CYCLE, &reg);
        reg = reg & ~(0xf0);
        reg |= MODEM_CPLD_PWR_OFF;
        rc |= hr_cpld_reg_write_32 (HR_CPLD_PWR_CYCLE, reg);

        sleep (MODEM_PWR_OFF_OR_ON_WAIT_TIME);

        rc |= hr_cpld_reg_read_32 (HR_CPLD_MODEM_STATUS, &reg);
        if ((reg & HR_CPLD_MODEM_STA_VREG_PWR_ON) != 0)  {
            cterr('f', 0, "PLA not detected (is still high).");
        }
        fflush(stdout);
        return rc;

    }

    return (0);
}

/*********************************************************************
 * $Log: hightower_5g_modem_util_test.c,v $
 * Revision 1.4  2021/06/30 20:04:56  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.3  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.6  2020/12/22 22:49:28  tshanmug
 * Empire prrq review comment fix
 *
 * Revision 1.1.4.5  2020/11/05 01:29:25  tshanmug
 * Empire Modem power ON/OFF utility updated in basic utility
 *
 * Revision 1.1.4.4  2020/10/12 15:48:35  tshanmug
 * Chrysler menu change, mmwave ant test added and Empire modem code cleanup
 *
 * Revision 1.1.4.3  2020/09/14 06:12:17  alpeng
 * replace highrise.h to hightower_sub6.h
 *
 * Revision 1.1.4.2  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

