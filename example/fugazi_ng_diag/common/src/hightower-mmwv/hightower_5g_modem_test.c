/* $Id: hightower_5g_modem_test.c,v 1.4 2021/06/30 20:04:56 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/hightower_5g_modem_test.c,v $
 *********************************************************************
 *
 * hightower_5g_modem_test.c -
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
#include "types.h"
#include "error.h"
#include "menu.h"
#include "hightower_5g_modem_lib.h"
#include "hightower_mmwv.h"
#include "highrise_cpld_lib.h"
#include "gpio.h"
#include "proto.h"
//#include "nvmonvars.h"

static int modem_detect_test (int input);
static int ht_modem_sub6_rssi_test (int input);
static int ht_modem_sub6_tx_test (int input);
static int modem_sim_test (int test_sim);
static int modem_dpr_pin_test (void);
static int set_custom_config (int input);
static int modem_bootup_msg (char *, char *);
static int modem_fsn_detect (void);
static int modem_mmwv_rssi_test(int input);
static int modem_mmwv_transmit_test(int input);
int ht_modem_mmwave_ant_test (boolean modem_rssi_test_items_executed);
int ht_modem_sub6_ant_test (boolean modem_rssi_test_items_executed);
int ht_modem_sub6_rssi_ind_ant_test (int);
void modem_power_cycle (int is_modem_pwr_on);
static int modem_ant_con_test (int input);

int diag_gpio_sim_card_detect (int which_sim);
int modem_power_on (int dummy);
int modem_power_off (int dummy);
int modem_power_dnd (int dummy);
boolean test_not_supported_yet (void);

static int modem_power_do_not_disturb = 0;

extern swi_5g_modem_usb_config_t diag_5g_swi_usb_cfg;
extern int hightower_modem_util (void);
extern int quiet_launch;
extern unsigned int getdec_answer(char *msgstr, unsigned int currentval,
                                  unsigned int min, unsigned int max);

#define SWI_SYS_SUPPRESS_PRINTK         "dmesg -n 1"
submenu_xtable_t modem_mmwave_ant_test_menu_table[] = {
    {"Modem mmwave Antenna Beam ID 0 RX Test", (type_t(*)())modem_mmwv_rssi_test, 0,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 1 RX Test", (type_t(*)())modem_mmwv_rssi_test, 1,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 2 RX Test", (type_t(*)())modem_mmwv_rssi_test, 2,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 3 RX Test", (type_t(*)())modem_mmwv_rssi_test, 3,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 128 RX Test", (type_t(*)())modem_mmwv_rssi_test, 128,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 129 RX Test", (type_t(*)())modem_mmwv_rssi_test, 129,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 130 RX Test", (type_t(*)())modem_mmwv_rssi_test, 130,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 131 RX Test", (type_t(*)())modem_mmwv_rssi_test, 131,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 0 TX Test", (type_t(*)())modem_mmwv_transmit_test, 0,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 1 TX Test", (type_t(*)())modem_mmwv_transmit_test, 1,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 2 TX Test", (type_t(*)())modem_mmwv_transmit_test, 2,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 3 TX Test", (type_t(*)())modem_mmwv_transmit_test, 3,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 128 TX Test", (type_t(*)())modem_mmwv_transmit_test, 128,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 129 TX Test", (type_t(*)())modem_mmwv_transmit_test, 129,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 130 TX Test", (type_t(*)())modem_mmwv_transmit_test, 130,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna Beam ID 131 TX Test", (type_t(*)())modem_mmwv_transmit_test, 131,
        MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
};
#define MODEM_MMWAVE_ANT_TEST_MENU_TABLE_SZ \
        (sizeof(modem_mmwave_ant_test_menu_table) / sizeof(submenu_xtable_t))

static mitem_t modem_pri_mmwave_ant_test_items[MODEM_MMWAVE_ANT_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t modem_sec_mmwave_ant_test_items[MODEM_MMWAVE_ANT_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t modem_mmwave_ant_test_menu = {
    "5G Modem mmwave Antenna Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    modem_pri_mmwave_ant_test_items,
};
static menuinfo_t *modem_mmwave_ant_test_menup = &modem_mmwave_ant_test_menu;

//Sub6 non-signalling mode  OTA menu
submenu_xtable_t modem_sub6_ant_test_menu_table[] = {

    {"Modem sub6 All Antenna RX Test", (type_t(*)())ht_modem_sub6_rssi_test,   
        MAIN_RSSI | AUX_RSSI | MIMO1_RSSI | MIMO2_RSSI,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem sub6 Individual Antenna RX Test", (type_t(*)())ht_modem_sub6_rssi_ind_ant_test, 0,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem sub6 Main Antenna TX Test", (type_t(*)())ht_modem_sub6_tx_test, MAIN_RSSI,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define MODEM_SUB6_ANT_TEST_MENU_TABLE_SZ \
        (sizeof(modem_sub6_ant_test_menu_table) / sizeof(submenu_xtable_t))

static mitem_t modem_pri_sub6_ant_test_items[MODEM_SUB6_ANT_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t modem_sec_sub6_ant_test_items[MODEM_SUB6_ANT_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t modem_sub6_ant_test_menu = {
    "5G Modem SUB6 Antenna Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    modem_pri_sub6_ant_test_items,
};
static menuinfo_t *modem_sub6_ant_test_menup = &modem_sub6_ant_test_menu;

//Sub6 individual antenna ota test menu
submenu_xtable_t modem_sub6_ind_ant_test_menu_table[] = {
    {"Modem sub6 Main Antenna RX Test", (type_t(*)())ht_modem_sub6_rssi_test, MAIN_RSSI,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem sub6 AUX Antenna RX Test", (type_t(*)())ht_modem_sub6_rssi_test, AUX_RSSI,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem sub6 MIMO1 Antenna RX Test", (type_t(*)())ht_modem_sub6_rssi_test, MIMO1_RSSI,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem sub6 MIMO2 Antenna RX Antenna Test", (type_t(*)())ht_modem_sub6_rssi_test, MIMO2_RSSI,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

};

#define MODEM_SUB6_IND_ANT_TEST_MENU_TABLE_SZ \
        (sizeof(modem_sub6_ind_ant_test_menu_table) / sizeof(submenu_xtable_t))

static mitem_t modem_pri_sub6_ind_ant_test_items[MODEM_SUB6_IND_ANT_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t modem_sec_sub6_ind_ant_test_items[MODEM_SUB6_IND_ANT_TEST_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t modem_sub6_ind_ant_test_menu = {
    "5G Modem SUB6 Antenna Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    modem_pri_sub6_ind_ant_test_items,
};
static menuinfo_t *modem_sub6_ind_ant_test_menup = &modem_sub6_ind_ant_test_menu;

//modem test menu
submenu_xtable_t modem_menu_table[] = {
    {"Modem Detection Test", (type_t(*)())modem_detect_test, TRUE,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"SIM 0 Card Test", (type_t(*)())modem_sim_test, 0,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
       (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"SIM Card 1 Test", (type_t(*)())modem_sim_test, 1,
        MF_SHOW_ERRCOUNT,
     //MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem DPR PIN test", (type_t(*)())modem_dpr_pin_test, 0,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem mmwave Antenna connection status test", (type_t(*)())modem_ant_con_test, 0,
        MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem SUB6 Antenna Tests", (type_t(*)())ht_modem_sub6_ant_test, TRUE,
        MF_SHOW_ERRCOUNT,
         (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Modem MMWAVE Antenna Tests", (type_t(*)())ht_modem_mmwave_ant_test, TRUE,
        MF_SHOW_ERRCOUNT,
          (type_t(*)())0, 0, (type_t(*)())0, 0},

    {"Utilities",
        (type_t(*)())hightower_modem_util, TRUE,
        0, (type_t(*)())0, 0, (type_t(*)())0, 0},
};

#define MODEM_MENU_TABLE_SZ \
        (sizeof(modem_menu_table) / sizeof(submenu_xtable_t))

static mitem_t modem_pri_test_items[MODEM_MENU_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t modem_sec_test_items[MODEM_MENU_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t modem_test_menu = {
    "5G Modem Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    modem_pri_test_items,
};
static menuinfo_t *modem_test_menup = &modem_test_menu;

/*
 ******************************************************************************
 * Function   : ht_modem_diag_menu
 * Description: Main Entry point for Modem test menu
 * Inputs     :
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int ht_modem_diag_menu (boolean modem_test_items_executed)
{
    int ret = PASSED;
    int modem_found = FALSE;
    char *tname = "Modem";
    char modem_model_num[MODEM_MODEL_NUM_LEN];
    char modem_sku_num[MODEM_SKU_NUM_LEN];
    char fname[64];


    /* Suppress printk so kernel won't print out bunch of messages */
    system(SWI_SYS_SUPPRESS_PRINTK);

    testname(tname);
    prpass(testpass, "SWI NR_5G ");

    if (!modem_power_do_not_disturb) //Power DND
    {

        /* Check if the modem powered ON already, turn off if yes */
        sprintf(fname, "%s/%s", PCI_SYS_DEV_PATH, MODEM_SWI_PCI_BUS_NUM);
        if (access(fname, F_OK) == -1) {
            modem_found = FAILED;
        }

        if (modem_found == PASSED) {
            printf ("\nDetach the pci device...");
            fflush(stdout);
            modem_power_cycle(MODEM_PWR_OFF);
        }

        //Power ON the modem
        modem_power_cycle(MODEM_PWR_ON);
    } //Power DND



    //Check for modem presence here... <PCIe interface>
    modem_found = diag_swi_5g_modem_pci_detect(MODEM_SWI_PCI_BUS_NUM,
                        MODEM_SWI_PCI_VID, MODEM_SWI_PCI_DID);

    if (modem_found == PASSED) {
        printf ("Modem found... ");
    } else {
        cterr('f', 0, "SWI Modem is not detected");
        ret = FAILED;
        return ret;
    }

    //Configure the modem for DPR pin test and SIM test
    //As per SWI SIM switching needs SIM to turn off and ON
    //To make DPR pin test and SIM test need to perform custom
    //configure
    set_custom_config (DIAG_CUSTOM_CFG);

    if (!modem_power_do_not_disturb) //Power DND
    {
        ret = modem_bootup_msg(modem_model_num, modem_sku_num);
        if (ret == FAILED) {
            return (ret);
        }

        if (strstr(modem_model_num, MODEM_MODEL_9190) == NULL) {
            cterr('f', 0, "Supported modem model is %s "
                   "but the system has %s \n",
                   MODEM_MODEL_9190, modem_model_num);
        }

        if (strstr(modem_sku_num, MODEM_9190_MMWAVE_SUB6_SKU) == NULL) {
            cterr('w', 0, "Supported Modem SKU Number is %s "
                   "but the system has %s \n",
                   MODEM_9190_MMWAVE_SUB6_SKU, modem_sku_num);
        }
    }
    //Check the modem hardware revision.
    //DV3.x is only for development not for production. FSN ends with 'ae' is DV3.x
    modem_fsn_detect();

    //Modem menu
    build_primary_submenu(modem_menu_table, MODEM_MENU_TABLE_SZ, "5G Modem test",
                          &modem_test_menup);
    build_secondary_submenu(modem_menu_table, MODEM_MENU_TABLE_SZ,
                            modem_sec_test_items);

    if (modem_test_items_executed) {
        do_all_menu_items(&modem_test_menu);
    } else {
        menu(&modem_test_menu, modem_sec_test_items, '\0');
    }
    if (!modem_power_do_not_disturb) //Power DND
    {

        printf ("\nSet Modem to default settings... \n");
        //Revert the modem cfg to default.
        set_custom_config (DEFAUT_CUSTOM_CFG);
        modem_power_cycle(MODEM_PWR_OFF);
    }
    return ret;
}


/*******************************************************************************
 * Function   : modem_detect_test
 * Description: To detect modem
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_detect_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("Modem Detection");
    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }


    rc = diag_5g_swi_obj_p->callin_fvt->modem_detection_test(
                                           (dev_object_t *)&diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
    }
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : modem_fsn_detect
 * Description: To detect fsn number of the modem to check the hardware version
 *              is DV3.x?? if DV3.x throw error - modem version not supported
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_fsn_detect (void)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;


    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }


    rc = diag_5g_swi_obj_p->callin_fvt->modem_get_fsn_num(
                                           (dev_object_t *)&diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Modem FSN Detection fails");
    }

    if (strstr (diag_5g_swi_obj.fsn, DV3X) != NULL) {
        printf("\n*** Fatal error: Modem hardware revision is DV3.X!!! "
                      "Please use DV4.x or higher hardware revision\n");
    }

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : ht_modem_sub6_rssi_test
 * Description: To get main RSSI for 5G modem
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int ht_modem_sub6_rssi_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = PASSED;
    int band_to_test = BAND_N79;
    int exp_pwr = OTA_SUB6_N79_EXP_PWR;

    testname("Modem %s antenna RSSI",
              (input == MAIN_RSSI) ? "Main" :
              (input == AUX_RSSI)  ? "Aux" :
              (input == MIMO1_RSSI)? "mimo1" :
              (input == MIMO2_RSSI)? "mimo2" : "ALL");

    prpass(testpass, "SWI NR_5G  ");

    printf("\nSet Freq = 4699.995  MHz, Power = -20 dBm" );
    printf ("\nPlease wait Antenna test is going on...\n");
    fflush (stdout);

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }
    rc = diag_5g_swi_obj_p->callin_fvt->modem_sub6_ota_rssi_test(
                    (dev_object_t *)&diag_5g_swi_obj, input, band_to_test, exp_pwr);

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

   if (rc != PASSED) {
        cterr('f', 0, "%s fails", testpass);
        return (FAILED);
    }

    printf ("\nAntenna RX readings: ");
    (input == MAIN_RSSI) ? printf ("\nMain : %d",diag_5g_swi_obj.ant_rx_value[MODEM_ANT_MAIN_PORT]) :
    (input == AUX_RSSI)  ? printf ("\nAux : %d",diag_5g_swi_obj.ant_rx_value[MODEM_ANT_AUX_PORT])  :
    (input == MIMO1_RSSI)? printf ("\nM1 : %d",diag_5g_swi_obj.ant_rx_value[MODEM_ANT_M1_PORT])   :
    (input == MIMO2_RSSI)? printf ("\nM2 : %d",diag_5g_swi_obj.ant_rx_value[MODEM_ANT_M2_PORT])   : 
                           printf ("\nMain : %d, Aux : %d, M1: %d, M2: %d", 
                                                 diag_5g_swi_obj.ant_rx_value[MODEM_ANT_MAIN_PORT],
                                                 diag_5g_swi_obj.ant_rx_value[MODEM_ANT_AUX_PORT],
                                                 diag_5g_swi_obj.ant_rx_value[MODEM_ANT_M1_PORT],
                                                 diag_5g_swi_obj.ant_rx_value[MODEM_ANT_M2_PORT]);

    if (input & MAIN_RSSI) {
        if ((diag_5g_swi_obj.ant_rx_value[0] < OTA_SUB6_MIN) || 
            (diag_5g_swi_obj.ant_rx_value[0] > OTA_SUB6_MAX)){
            printf ("\nExpected value is between :%d and :%d", \
                                    OTA_SUB6_MIN, OTA_SUB6_MAX);
            cterr('f', 0, "Main Antenna test failed. " \
                          "Check the Signal generator and UUT position.");
            rc = FAILED;
        }
    }

    if (input & AUX_RSSI) {
        if ((diag_5g_swi_obj.ant_rx_value[3] < OTA_SUB6_MIN) || 
            (diag_5g_swi_obj.ant_rx_value[3] > OTA_SUB6_MAX)){
            printf ("\nExpected value is between :%d and :%d", \
                                    OTA_SUB6_MIN, OTA_SUB6_MAX);
            cterr('f', 0, "Aux Antenna test failed." \
                   "Check the Signal generator and UUT position.");
            rc = FAILED;
        }
    }

    if (input & MIMO1_RSSI) {
        if ((diag_5g_swi_obj.ant_rx_value[1] < OTA_SUB6_MIN) || 
            (diag_5g_swi_obj.ant_rx_value[1] > OTA_SUB6_MAX)){
            printf ("\nExpected value is between :%d and :%d", \
                                    OTA_SUB6_MIN, OTA_SUB6_MAX);
            cterr('f', 0, "MIMO1 Antenna test failed." \
                   "Check the Signal generator and UUT position.");
            rc = FAILED;
        }
    }

    if (input & MIMO2_RSSI) {
        if ((diag_5g_swi_obj.ant_rx_value[2] < OTA_SUB6_MIN) || 
            (diag_5g_swi_obj.ant_rx_value[2] > OTA_SUB6_MAX)){
            printf ("\nExpected value is between :%d and :%d", \
                                    OTA_SUB6_MIN, OTA_SUB6_MAX);
            cterr('f', 0, "MIMO2 Antenna test failed." \
                   "Check the Signal generator and UUT position.");
            rc = FAILED;
        }
    }
    if (rc != FAILED) {
        printf ("\nAntenna test passed.");
    }
    return rc;
}
/*******************************************************************************
 * Function   : ht_modem_sub6_tx_test
 * Description: To transmit radio signal from modem
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int ht_modem_sub6_tx_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = PASSED;
    float sgl_pwr = 0;

    testname("Modem main antenna TX");
    prpass(testpass, "SWI NR_5G  ");
    printf ("\nSet the test equipment to receive 4699.95 Mhz and power +30dbm");
    fflush(stdout);
    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    //Beta1 and  Beta2 modem firmware has different AT command
    //sequence and parameter. From beta3 there is additional
    //AT command and parameter change.
    //Added additional param to differentiate Legacy and new
    //1 - for legacy 0-latest

    rc = diag_5g_swi_obj_p->callin_fvt->modem_sub6_tx_test(
                          (dev_object_t *)&diag_5g_swi_obj, input, 0);

    printf ("\nCheck the test equipment and enter the signal power level : ");
    scanf ("%f", &sgl_pwr);
    printf ("You entered: %f\n", sgl_pwr);

    //expected reading is 23, Tolerance +/-5dbm
    if ((sgl_pwr < MODEM_TX_POWER_LOW ) || (sgl_pwr > MODEM_TX_POWER_HIGH)) {
        cterr('f', 0, "Expected power is between %d and %d, check the UUT is aligned properly",
                    MODEM_TX_POWER_LOW, MODEM_TX_POWER_HIGH);
        rc = FAILED;
    }

    rc |= diag_5g_swi_obj_p->callin_fvt->modem_stop_tx(
                          (dev_object_t *)&diag_5g_swi_obj, input, 1);

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "%s fails", testpass);
        return (FAILED);
    }
    fflush(stdout);
    return rc;

}

/*******************************************************************************
 * Function   : modem_sim_test
 * Description: To detect SIM card0
 * Inputs     : test_sim - SIM 0
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_sim_test (int test_sim)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("SIM slot %d ", test_sim);
    prpass(testpass, "SWI NR_5G  ");

    /* Check whether SIM card is detected by GPIO expander */
    if (diag_gpio_sim_card_detect(test_sim) != TRUE) {
        cterr('f', 0, "SIM card is not detected by GPIO");
        return (FAILED);
    }

    //Switch MUX to select appropriate SIM
    if (diag_swi_5g_sim_selection (test_sim) == FAILED) {
        cterr('f', 0, "SIM Mux selection failed");
        return (FAILED);
    }

    msleep(SIM_MUX_SWITCH_DELAY);


    /* Read SIM detect PIN status through AT command to check
     * whether modem is present or not */
    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    //Modem has only one sim. exterl mux is used to sel two sims
    rc = diag_5g_swi_obj_p->callin_fvt->sim_detect_pin_present(
                      (dev_object_t *)&diag_5g_swi_obj, SIM_PRESENT, 0);


    if (rc != PASSED) {
        cterr('f', 0, "Unexpected SIM%d_DETECT pin status.", test_sim);
        return (FAILED);
    }

    printf ("\nAccessing the SIM card...\n");
    fflush(stdout);
    rc = diag_5g_swi_obj_p->callin_fvt->sim_detect_test((dev_object_t *)
                                                           &diag_5g_swi_obj,
                                                           0); //modem has only one sim

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    if (rc != PASSED) {
        cterr('f', 0, "%s fails", testpass);
        return (FAILED);
    }

    fflush(stdout);
    return 0;
}

/*******************************************************************************
 * Function   : diag_gpio_sim_card_detect
 * Description: To detect SIM  card
 * Inputs     : which_sim - SIM 0/SIM1
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_gpio_sim_card_detect (int which_sim)
{
    int val = HIGH, rc;
    if (which_sim == SIM0) {
        /* CP_MPP[27] value 0 mean present*/
        rc = gpio_read(SIM0_DETECT_L, &val);
        if (rc == -1) {
            return (FAILED);
        }
    } else if (which_sim == SIM1) {
        /* CP_MPP[24] value 0 mean present*/
        rc = gpio_read(SIM1_DETECT_L, &val);
        if (rc == -1) {
            return (FAILED);
        }
    } else {
        printf("%s:Invalid SIM slot number(%d)\n", __func__, which_sim);
        return (FAILED);
    }

    if (val == LOW) {
        return (TRUE);
    }

    return (FALSE);
}

/*******************************************************************************
 * Function   : modem_bootup_msg
 * Description: Display modem information
 * Inputs     : 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_bootup_msg (char *modem_model, char *modem_sku)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("Modem access");
    prpass(testpass, "modem test");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    rc = diag_5g_swi_obj_p->callin_fvt->show_modem_info(
                                           (dev_object_t *)&diag_5g_swi_obj, SHOW_MODEM_INFO);

    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
        goto __bootup_msg_exit;
    }
    snprintf (modem_model, MODEM_MODEL_NUM_LEN, "%s", diag_5g_swi_obj_p->model);

    /* Query modem HWID */
    rc = diag_5g_swi_obj_p->callin_fvt->show_modem_info(
                                           (dev_object_t *)&diag_5g_swi_obj, SHOW_MODEM_HWID);

    if (rc != PASSED) {
        cterr('f', 0, "Modem HW ID read fails");
        goto __bootup_msg_exit;
    }


    /* Query modem SKU */
    rc = diag_5g_swi_obj_p->callin_fvt->show_modem_info(
                                           (dev_object_t *)&diag_5g_swi_obj, SHOW_MODEM_SKU);

    if (rc != PASSED) {
        cterr('f', 0, "Modem SKU read fails");
        goto __bootup_msg_exit;
    }
    snprintf (modem_sku, MODEM_SKU_NUM_LEN, "%s", diag_5g_swi_obj_p->sku);

__bootup_msg_exit:
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : modem_dpr_pin_test
 * Description: Function to check modem DPR pin status
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
static int modem_dpr_pin_test (void)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = PASSED;
    unsigned long  reg = 0;

    testname("Modem DPR pin test");
    prpass(testpass, "modem DPR pin test");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    //Modem DPR pin = 1
    rc |= hr_cpld_reg_read_32 (HR_CPLD_MODEM_CTRL, &reg);

    rc |= hr_cpld_reg_write_32 (HR_CPLD_MODEM_CTRL, (reg  | HR_CPLD_MODEM_CTRL_DPR));

    msleep(DPR_WAIT_DELAY);
    rc |= hr_cpld_reg_read_32 (HR_CPLD_MODEM_CTRL, &reg);

    if (rc == FAILED) {
        cterr('f', 0, "Could not set DPR pin value in CPLD");
        goto __exit;
    }
    //Read the sar status
    if (diag_5g_swi_obj_p->callin_fvt->modem_get_sarstate(
              (dev_object_t *)&diag_5g_swi_obj, MODEM_IS_SARSTATE_0) == FAILED){
        cterr('f', 0, "Could not detect the modem SAR backoff STATE");
        rc = FAILED;
        goto __exit;

    }

    //Modem DPR pin = 0
    rc |= hr_cpld_reg_read_32 (HR_CPLD_MODEM_CTRL, &reg);

    rc |= hr_cpld_reg_write_32 (HR_CPLD_MODEM_CTRL, (reg &  (~HR_CPLD_MODEM_CTRL_DPR)));

    msleep(DPR_WAIT_DELAY);
    rc |= hr_cpld_reg_read_32 (HR_CPLD_MODEM_CTRL, &reg);

    if (rc == FAILED) {
        cterr('f', 0, "Could not set DPR pin value in CPLD");
        goto __exit;
    }


    //Read the sar status
    if (diag_5g_swi_obj_p->callin_fvt->modem_get_sarstate(
              (dev_object_t *)&diag_5g_swi_obj, MODEM_IS_SARSTATE_1)) {
        cterr('f', 0, "Could not detect the modem SAR normal STATE");
        rc = FAILED;

    }
__exit:
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : set_custom_config
 * Description: Function to set custom config required for SIM and DPR
 * Inputs     : input 1 for set custom config, 0 to set default
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
static int set_custom_config (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("custom config");
    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    if (input == DIAG_CUSTOM_CFG) {
    //Make the Custom config to set GPIOSAR and SIM config
        rc = diag_5g_swi_obj_p->callin_fvt->modem_config_custom_param(
                                           (dev_object_t *)&diag_5g_swi_obj, MODEM_SET_DIAG_CUSTOM_CONFIG);
        rc |= diag_5g_swi_obj_p->callin_fvt->modem_reset_test(
                                           (dev_object_t *)&diag_5g_swi_obj);
    } else if (input == DEFAUT_CUSTOM_CFG) {
        rc = diag_5g_swi_obj_p->callin_fvt->modem_config_custom_param(
                                           (dev_object_t *)&diag_5g_swi_obj, MODEM_SET_DEFAULT_CUSTOM_CONFIG);

    }

    if (rc == FAILED) {
        cterr('f', 0, "Could not configure the custom configuration");
        rc = FAILED;
    }


    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : modem_ant_con_test
 * Description: To detect mmwave antenna connection 
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_ant_con_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("Modem mmwave Antenna connection");
    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }


    rc = diag_5g_swi_obj_p->callin_fvt->modem_mmwv_ant_status(
                                           (dev_object_t *)&diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "Modem Detection fails");
    }
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);

    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : test_not_supported_yet
 * Description: Function to check whether modem test available
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean test_not_supported_yet (void)
{
    /* Currently hardware not yet supportt */
    printf("\n!! Test not yet supported !!\n");

    return (FALSE);
}

/*******************************************************************************
 * Function   : check_pla_status
 * Description: Function to check the PLA status
 * Inputs     : pin_state :0 - check PLA for 0, 1: check for high
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int check_pla_status(int pin_state){
    int i = 0;
    unsigned long  reg = 0;
    unsigned long cmp_val = 0;

    if (pin_state) cmp_val = HR_CPLD_MODEM_STA_VREG_PWR_ON;

    while (i < MODEM_PWR_OFF_OR_ON_PLA_WAIT_TIME){
        hr_cpld_reg_read_32 (HR_CPLD_MODEM_STATUS, &reg);
        if ((reg & HR_CPLD_MODEM_STA_VREG_PWR_ON) == cmp_val)  {
            break;
        }
        i++;
        sleep (DELAY_ONE_SEC);
    }

    if (i >= MODEM_PWR_OFF_OR_ON_PLA_WAIT_TIME)  {
        cterr('f', 0, "PLA not detected. Modem status reg value : 0x%lx", reg);
        return FAILED;
    }

    return PASSED;
}

/*******************************************************************************
 * Function   : modem_power_on
 * Description: Function to power on the modem
 * Inputs     : dummy - None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */

int modem_power_on (int dummy)
{
    unsigned long  reg = 0;
    int modem_found = FALSE;
    char fname[64];
    int ret = PASSED;


    /* Check if the file exists */
    sprintf(fname, "%s/%s", PCI_SYS_DEV_PATH, MODEM_SWI_PCI_BUS_NUM);
    if (access(fname, F_OK) == -1) {
        modem_found = FAILED;
    }

    if (modem_found == PASSED) {
        printf ("\nDetach the pci device...");
        fflush(stdout);
        modem_power_off(0);
    }

    testname("Modem Power on");
    prpass(testpass, "SWI NR_5G ");
    printf ("\nAttaching the pci device...");
    fflush(stdout);


    hr_cpld_reg_read_32 (HR_CPLD_MODEM_STATUS, &reg);
    hr_cpld_reg_write_32 (HR_CPLD_MODEM_STATUS, (reg |  HR_CPLD_MODEM_STA_PWR_ON));
    sleep (DELAY_ONE_SEC);

    //Trigger the PCI script
    system("/root/attach_pci_device.sh");
    printf ("\nwait for %d sec to get modem enumurate...\n",DELAY_TEN_SEC);
    fflush(stdout);
    sleep(DELAY_TEN_SEC);

    ret = check_pla_status(MODEM_PLA_HI);

    //Turn on the Antenna and GPS.
    hr_cpld_reg_read_32 (HR_CPLD_MODEM_CTRL, &reg);
    hr_cpld_reg_write_32 (HR_CPLD_MODEM_CTRL, (reg | MODEM_RADIO_ON | \
                                          MODEM_GNSS_ON));
    return ret;
}


/*******************************************************************************
 * Function   : modem_power_off
 * Description: Function to power on the modem
 * Inputs     : dummy - None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
int modem_power_off (int dummy)
{
    int ret = PASSED;
    unsigned long  reg = 0;

    testname("Modem Power off");
    prpass(testpass, "SWI NR_5G ");
    printf ("\nModem shutdown... ");
    fflush(stdout);


    //Turn off the modem.
    hr_cpld_reg_read_32 (HR_CPLD_MODEM_STATUS, &reg);
    hr_cpld_reg_write_32 (HR_CPLD_MODEM_STATUS, (reg &  (~HR_CPLD_MODEM_STA_PWR_ON)));
    sleep (DELAY_ONE_SEC);

    //Remove the PCI device
    system("/root/remove_pci_device.sh");

    ret = check_pla_status(MODEM_PLA_LO);

    //Turn on the Antenna and GPS.
    hr_cpld_reg_read_32 (HR_CPLD_MODEM_CTRL, &reg);
    hr_cpld_reg_write_32 (HR_CPLD_MODEM_CTRL, (reg & ~(MODEM_RADIO_ON | \
                                          MODEM_GNSS_ON)));
    return ret;
}


/*******************************************************************************
 * Function   : modem_power_dnd
 * Description: Function to dnd modem power / make it defaulf
 * Inputs     : dummy - None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
int modem_power_dnd (int dummy) {

    testname("modem power DND");
    prpass(testpass, "SWI NR_5G  ");

    printf ("\nThis option is to perform modem default power on/off sequence or "
            "do not disturb the power when invoking the modem menu.");
    printf ("\nSet flag value 0 to make default test case or 1 to DND modem power");
    printf ("\nCurrent flag value is : %d", modem_power_do_not_disturb);

    modem_power_do_not_disturb =  getdec_answer("\n\nEnter 0 to make default or 1 to DND modem power",
                                    0, 0, 1);
    printf ("\nSet flag value is : %d", modem_power_do_not_disturb);

    return PASSED;
}
/*******************************************************************************
 * Function   : modem_power_cycle
 * Description: Function to physical power on/off the modem
 * Inputs     : is_modem_pwr_on : 1 - ON and 0 - OFF
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
void modem_power_cycle (int is_modem_pwr_on)
{
    int rc = PASSED;
    unsigned long int reg = 0 ;

    if (is_modem_pwr_on) {
        rc = hr_cpld_reg_read_32 (HR_CPLD_PWR_CYCLE, &reg);
        reg = reg & ~(0xf0);
        reg |= 0x20;
        rc |= hr_cpld_reg_write_32 (HR_CPLD_PWR_CYCLE, reg);
        sleep (DELAY_ONE_SEC);
        rc |=  modem_power_on(0); //Dummy param
        if (rc == FAILED){
            printf ("\nFailed to power on the modem");
        }
        fflush (stdout);
    } else {
        rc = modem_power_off(0); //Dummy param
        if (rc != FAILED) {
            rc = hr_cpld_reg_read_32 (HR_CPLD_PWR_CYCLE, &reg);
            reg = reg & ~(0xf0);
            reg |=0xd0;
            rc |= hr_cpld_reg_write_32 (HR_CPLD_PWR_CYCLE, reg);
            sleep(DELAY_ONE_SEC);
        }else {
            printf ("\nFailed to make the modem shutdown, modem power is ON");
        }
        fflush (stdout);
    }
}

/*******************************************************************************
 * Function   : modem_mmwv_rssi_test
 * Description: Modem mmwave RSSI test via QTM527 Antenna
 * Inputs     : input - Which QTM527 Antenna to use
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
static int modem_mmwv_rssi_test(int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;
    int band_to_test = BAND_N261;
    int power_level  = MMWAVE_RX_POWER_LEVEL;

    testname("Modem MMWAVE antenna (mask:%#x) RSSI", input);

    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    rc = diag_5g_swi_obj_p->callin_fvt->modem_mmwv_rssi_test(
            (dev_object_t *)&diag_5g_swi_obj, input, band_to_test, power_level);

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
            &diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "%s fails", testpass);
        return (FAILED);
    }

    fflush(stdout);
    return rc;

}
/*******************************************************************************
 * Function   : modem_mmwv_transmit_test
 * Description: Modem mmwave RSSI test via QTM527 Antenna
 * Inputs     : input - Which QTM527 Antenna to use
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
static int modem_mmwv_transmit_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;
    int band_to_test = BAND_N261;
    int power_level  = MMWAVE_TX_POWER_LEVEL;
    float sgl_pwr = 0;

    testname("Modem MMWAVE antenna (mask:%#x) Transmit", input);

    prpass(testpass, "SWI NR_5G  ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    /* Start Tx test */
    rc = diag_5g_swi_obj_p->callin_fvt->modem_mmwv_transmit_test(
            (dev_object_t *)&diag_5g_swi_obj, input, band_to_test, power_level);

    if (rc == PASSED) {
        /* Prompt user to measure the Tx signal power on CMP200 equipment */
        printf ("\nCheck the test equipment and enter the signal power level : ");
        scanf ("%f", &sgl_pwr);
    	printf ("You entered: %f\n", sgl_pwr);

        //expected reading is MMWAVE_TX_POWER_LEVEL, Tolerance +/-5dbm
        if ((sgl_pwr < MMWAVE_TX_POWER_LOW ) || (sgl_pwr > MMWAVE_TX_POWER_HIGH)) {
            printf ("Entered power is not between %d and %d",
                    MMWAVE_TX_POWER_LOW, MMWAVE_TX_POWER_HIGH);
            rc = FAILED;
        }
    }

    rc |= diag_5g_swi_obj_p->callin_fvt->modem_mmwv_transmit_stop(
            (dev_object_t *)&diag_5g_swi_obj, MMWAVE_FR2_TRANSMIT_STOP);

    rc |= diag_5g_swi_obj_p->callin_fvt->modem_mmwv_transmit_stop(
            (dev_object_t *)&diag_5g_swi_obj, RSSI_DROP_RADIO_CFG);

    rc |= diag_5g_swi_obj_p->callin_fvt->modem_mmwv_transmit_stop(
            (dev_object_t *)&diag_5g_swi_obj, MMWAVE_ANTENNA_PON_DISABLE);

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
            &diag_5g_swi_obj);

    if (rc != PASSED) {
        cterr('f', 0, "%s fails", testpass);
        return (FAILED);
    }
    fflush(stdout);
    return rc;

}

/*******************************************************************************
 * Function   : ht_modem_mmwave_ant_test
 * Description: Modem mmwave Antenna test menu
 * Inputs     : input - Not used
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */

int ht_modem_mmwave_ant_test (boolean modem_rssi_test_items_executed)
{
    int ret = PASSED;
	
    //Modem menu
    build_primary_submenu(modem_mmwave_ant_test_menu_table, MODEM_MMWAVE_ANT_TEST_MENU_TABLE_SZ, 
                          "mmWave Modem RSSI test", &modem_mmwave_ant_test_menup);

    build_secondary_submenu(modem_mmwave_ant_test_menu_table, MODEM_MMWAVE_ANT_TEST_MENU_TABLE_SZ,
                            modem_sec_mmwave_ant_test_items);

    menu(&modem_mmwave_ant_test_menu, modem_sec_mmwave_ant_test_items, '\0');

    return ret;
}

/*******************************************************************************
 * Function   : ht_modem_sub6_ant_test
 * Description: Modem sub6 Antenna test menu
 * Inputs     : input - Not used
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */

int ht_modem_sub6_ant_test (boolean modem_tx_test_items_executed)
{
    int ret = PASSED;
	
    //Modem menu
    build_primary_submenu(modem_sub6_ant_test_menu_table, MODEM_SUB6_ANT_TEST_MENU_TABLE_SZ, 
                          "mmWave Modem Antenna TX test", &modem_sub6_ant_test_menup);

    build_secondary_submenu(modem_sub6_ant_test_menu_table, MODEM_SUB6_ANT_TEST_MENU_TABLE_SZ,
                            modem_sec_sub6_ant_test_items);

    menu(&modem_sub6_ant_test_menu, modem_sec_sub6_ant_test_items, '\0');

    return ret;
}


/*******************************************************************************
 * Function   : ht_modem_sub6_rssi_ind_ant_test
 * Description: Modem sub6 individual Antenna test menu
 * Inputs     : input - Not used
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */

int ht_modem_sub6_rssi_ind_ant_test (int modem_test_items_executed)
{
    int ret = PASSED;
	
    //Modem menu
    build_primary_submenu(modem_sub6_ind_ant_test_menu_table, MODEM_SUB6_IND_ANT_TEST_MENU_TABLE_SZ, 
                          "Sub6 Rx antenna Test", &modem_sub6_ind_ant_test_menup);

    build_secondary_submenu(modem_sub6_ind_ant_test_menu_table, MODEM_SUB6_IND_ANT_TEST_MENU_TABLE_SZ,
                            modem_sec_sub6_ind_ant_test_items);

    menu(&modem_sub6_ind_ant_test_menu, modem_sec_sub6_ind_ant_test_items, '\0');

    return ret;
}


/*********************************************************************
 * $Log: hightower_5g_modem_test.c,v $
 * Revision 1.4  2021/06/30 20:04:56  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.3  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.10  2021/05/12 17:54:23  tshanmug
 * Chrysler infra for SIM1 test
 *
 * Revision 1.1.4.9  2021/03/18 10:12:29  alpeng
 * fixed compile error
 *
 * Revision 1.1.4.8  2021/03/17 09:34:12  alpeng
 * sync trunk to this branch
 *
 * Revision 1.1.4.7  2021/01/22 07:01:21  tshanmug
 * chrysler modem power OFF ON sequence and modem access through external usb access
 *
 * Revision 1.1.4.6  2020/12/31 07:21:51  tshanmug
 * chrysler mmwave antenna detetection test added
 *
 * Revision 1.1.4.5  2020/10/12 15:48:35  tshanmug
 * Chrysler menu change, mmwave ant test added and Empire modem code cleanup
 *
 * Revision 1.1.4.4  2020/09/18 17:53:53  ksabzwar
 * Add Empire & Chrysler 5G modem SKU check
 *
 * Revision 1.1.4.3  2020/08/27 22:51:05  ksabzwar
 * fix compile error
 *
 * Revision 1.1.4.2  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

