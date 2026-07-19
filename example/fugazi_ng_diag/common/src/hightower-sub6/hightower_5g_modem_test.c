/* $Id: hightower_5g_modem_test.c,v 1.4 2021/06/30 20:04:56 tshanmug Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/hightower_5g_modem_test.c,v $
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
#include "hightower_sub6.h"
#include "highrise_cpld_lib.h"
#include "gpio.h"
#include "proto.h"
static int modem_detect_test (int input);
static int modem_rssi_test (int input);
static int modem_sub6_tx_ant_test (int input);
static int modem_gps_antenna_test (int input);
static int modem_sim_test (int test_sim);
static int modem_dpr_pin_test (void);
static int set_custom_config (int input);
static int modem_bootup_msg (char *, char *);
static int modem_fsn_detect (void);
int diag_gpio_sim_card_detect (int which_sim);
int modem_power_on (int dummy);
int modem_power_off (int dummy);
int modem_power_dnd (int dummy);
void modem_power_cycle (int is_modem_pwr_on);


static int modem_power_do_not_disturb = 0;
char modem_fw_version[64];

extern int hightower_modem_util (void);
extern int quiet_launch;
boolean test_not_supported_yet (void);
extern unsigned int getdec_answer(char *msgstr, unsigned int currentval,
                                  unsigned int min, unsigned int max);


submenu_xtable_t modem_menu_table[] = {

    {"Modem Detection Test", (type_t(*)())modem_detect_test, TRUE,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
     (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Card 0 Test", (type_t(*)())modem_sim_test, 0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"SIM Card 1 Test", (type_t(*)())modem_sim_test, 1,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT, 
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem RX Main RSSI Test", (type_t(*)())modem_rssi_test, MAIN_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem RX AUX RSSI Test", (type_t(*)())modem_rssi_test, AUX_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem RX MIMO1 RSSI Antenna Test", (type_t(*)())modem_rssi_test, MIMO1_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem RX MIMO2 RSSI Antenna Test", (type_t(*)())modem_rssi_test, MIMO2_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem TX Main sgl strength Test", (type_t(*)())modem_sub6_tx_ant_test, MAIN_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem GNSS L1 Antenna Test", (type_t(*)())modem_gps_antenna_test, GNSS_L1_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem GNSS L5 Antenna Test", (type_t(*)())modem_gps_antenna_test, GNSS_L5_RSSI,
     MF_SHOW_ERRCOUNT,
      (type_t(*)())0, 0, (type_t(*)())0, 0},
    {"Modem DPR PIN test", (type_t(*)())modem_dpr_pin_test, 0,
     MF_CONTINUOUS | MF_DOALL | MF_SHOW_ERRCOUNT,
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
int ht_modem_diag_menu (boolean mb_test_items_executed)
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

    if (!modem_power_do_not_disturb) //Power DND
    {

        //Configure the modem for DPR pin test and SIM test
        //As per SWI, SIM switching needed SIM to turn off and ON.
        //To make DPR pin test and SIM test, need to perform custom
        //configure 
        ret = set_custom_config (DIAG_CUSTOM_CFG);
        if (ret != PASSED){
            cterr('f', 0,"Custom cfg error");
            return FAILED;
        }

        //Scan the modem for model number, modem FW ver etc.    
        ret = modem_bootup_msg(modem_model_num, modem_sku_num);
        if (ret == FAILED) {
            return FAILED; 
        }


#if 0  //Not going to display error because EM9190 is super set of EM9191
        if (strstr(modem_model_num, MODEM_MODEL_9191) == NULL) {
            printf("*** Fatal error: Supported modem model is %s "
                   "but the system has %s \n",
                   MODEM_MODEL_9191, modem_model_num);
        }
#endif

        if (strstr(modem_sku_num, MODEM_9190_SUB6_ONLY_SKU) == NULL) {
            printf ("*** Warning: Supported Modem SKU Number is %s "
                   "but the system has %s \n",
                   MODEM_9190_SUB6_ONLY_SKU, modem_sku_num);
        }

        //Check the modem hardware revision. 
        //DV3.x is only for development not for production. FSN ends with 'ae' is DV3.x
        ret = modem_fsn_detect(); 
        if (ret == FAILED) return FAILED;
	
    }
    
    //Modem menu
    build_primary_submenu(modem_menu_table, MODEM_MENU_TABLE_SZ, "5G Modem test",
                          &modem_test_menup);
    build_secondary_submenu(modem_menu_table, MODEM_MENU_TABLE_SZ,
                            modem_sec_test_items);

    if (mb_test_items_executed) {
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
    prpass(testpass, "SWI NR_5G ");

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
        printf("*** Fatal error: Modem hardware revision is DV3.X!!! " 
                      "Please use DV4.x or higher hardware revision\n");
    }
    
    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    fflush(stdout);
    return rc;
}

/*******************************************************************************
 * Function   : modem_rssi_test
 * Description: To get all 4 antenna RSSI for 5G modem
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_rssi_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;
    int band_to_test = BAND_N1;
    int exp_power = SUB6_CONDUCT_TEST_EXP_POWER;

    testname("Modem %s antenna RSSI",
              (input == MAIN_RSSI) ? "Main" :
              (input == AUX_RSSI)  ? "Aux" :
              (input == MIMO1_RSSI)? "mimo1" : "mimo2");

    prpass(testpass, "SWI NR_5G ");


    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    rc = diag_5g_swi_obj_p->callin_fvt->modem_sub6_rssi_test(
                  (dev_object_t *)&diag_5g_swi_obj, input, band_to_test,
                                                            exp_power);

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
 * Function   : modem_sub6_tx_ant_test
 * Description: To transmit radio signal from modem
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_sub6_tx_ant_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = PASSED;
    float sgl_pwr = 0;
    int legacy_rssi_test = 0;

    testname("Modem main antenna TX");
    prpass(testpass, "SWI NR_5G ");

    printf ("\nSet the test equipment to receive 4699.95 Mhz and power +30dbm");
    fflush(stdout);

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    //Beta1 and  Beta2 modem firmware has different AT command
    //sequence and parameter. From beta3 there is additional
    //AT command and parameter change.
    //Since test started in production with Beta1 firmware
    //Need to support legacy method as well
    if (strstr(modem_fw_version, MODEM_FW_BETA1) ||
                  strstr(modem_fw_version, MODEM_FW_BETA2)){
        legacy_rssi_test = 1;
    }


    rc = diag_5g_swi_obj_p->callin_fvt->modem_sub6_tx_test(
                 (dev_object_t *)&diag_5g_swi_obj, input, legacy_rssi_test);

    printf ("\nCheck the test equipment and enter the signal power level : ");
    scanf ("%f", &sgl_pwr);
    printf ("You entered: %f\n", sgl_pwr);

    //expected reading is 23, Tolerance +/-5dbm
    if ((sgl_pwr < MODEM_TX_POWER_LOW ) || (sgl_pwr > MODEM_TX_POWER_HIGH)) {
        printf ("Expected power is between %d and %d",
                    MODEM_TX_POWER_LOW, MODEM_TX_POWER_HIGH); 
        rc = FAILED;
    }

    rc |= diag_5g_swi_obj_p->callin_fvt->modem_stop_tx(
                          (dev_object_t *)&diag_5g_swi_obj, input, legacy_rssi_test);

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
 * Function   : modem_gps_antenna_test
 * Description: To get GPS antenna readings for 5G modem
 *              The function will do L1-Aux ant and L5-MIMO2 ant test
 * Inputs     : input - 1 for L1 and 5 for L5
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_gps_antenna_test (int input)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("Modem GPS %s antenna ", (input == GNSS_L1_RSSI)? "L1(AUX)" : "L5(MIMO2)"); 
    prpass(testpass, "SWI NR_5G ");

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    rc = diag_5g_swi_obj_p->callin_fvt->modem_gps_ant_test(
                                           (dev_object_t *)&diag_5g_swi_obj, input);

    diag_5g_swi_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_5g_swi_obj);
    if (rc != PASSED) {
        cterr('f', 0, "GPS antenna %s test fails", 
                               (input == GNSS_L1_RSSI)? "L1" : "L5");
        return (FAILED);
    }

    fflush(stdout);

    return rc;
}

/*******************************************************************************
 * Function   : modem_sim_test
 * Description: To detect SIM card0/1  
 * Inputs     : test_sim - SIM 0/1 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_sim_test (int test_sim)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("SIM slot %d ", test_sim);
    prpass(testpass, "SWI NR_5G ");

    /* Check whether SIM card is detected using GPIO expander */
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

    if (diag_5g_swi_dev_create(diag_5g_swi_obj_p) != PASSED) {
        cterr('f', 0, "Create SWI Dev Object Fails");
        return (FAILED);
    }

    /* Read SIM detect PIN status through AT command to check
     * whether modem is present or not */
    rc = diag_5g_swi_obj_p->callin_fvt->sim_detect_pin_present(
                      (dev_object_t *)&diag_5g_swi_obj, SIM_PRESENT, SIM_0);


    if (rc != PASSED) {
        cterr('f', 0, "Unexpected SIM%d_DETECT pin status.", test_sim);
		goto __exit_sim_test;
    }

    printf ("\nAccessing the SIM card...\n");
    fflush(stdout);
    rc = diag_5g_swi_obj_p->callin_fvt->sim_detect_test((dev_object_t *)
                                                           &diag_5g_swi_obj,
                                                           SIM_0); //modem has only one sim
__exit_sim_test:
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
 * Inputs     : modem_model - returns modem number EM9190/EM9191
                modem_sku - returns modem SKU number to 
 *                          check EM9190 with sub6/mmwave
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int modem_bootup_msg (char *modem_model, char *modem_sku)
{
    dev_5g_swi_object_t diag_5g_swi_obj;
    dev_5g_swi_object_t *diag_5g_swi_obj_p = &diag_5g_swi_obj;
    int rc = FAILED;

    testname("Modem access");
    prpass(testpass, "SWI NR_5G ");

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
    snprintf ((char *)modem_fw_version, 64, "%s", diag_5g_swi_obj_p->modem_firmware);

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

    testname("Modem DPR pin");
    prpass(testpass, "SWI NR_5G ");

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
 * Function   : test_not_supported_yet
 * Description: Function to check whether modem test available
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean test_not_supported_yet (void)
{
    /* Currently hardware not yet supportt */
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
        reg |= MODEM_CPLD_PWR_ON;
        rc |= hr_cpld_reg_write_32 (HR_CPLD_PWR_CYCLE, reg);
        sleep (DELAY_ONE_SEC);
        rc |=  modem_power_on(0);
        if (rc == FAILED){
            printf ("\nFailed to power on the modem");
        }
        fflush (stdout);
    } else {
        rc = modem_power_off(0);
        if (rc != FAILED) {
            rc = hr_cpld_reg_read_32 (HR_CPLD_PWR_CYCLE, &reg);
            reg = reg & ~(0xf0);
            reg |= MODEM_CPLD_PWR_OFF;
            rc |= hr_cpld_reg_write_32 (HR_CPLD_PWR_CYCLE, reg);
            sleep(DELAY_ONE_SEC);
        }else {
            printf ("\nFailed to make the modem shutdown, modem power is ON");
        }
        fflush (stdout);
    }
}

/*********************************************************************
 * $Log: hightower_5g_modem_test.c,v $
 * Revision 1.4  2021/06/30 20:04:56  tshanmug
 * Chrysler Sub6 OTA and SWI common layer changes, Dual SIM test support
 *
 * Revision 1.3  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.10  2021/03/18 10:12:29  alpeng
 * fixed compile error
 *
 * Revision 1.1.4.9  2021/03/17 09:34:12  alpeng
 * sync trunk to this branch
 *
 * Revision 1.1.4.8  2021/01/21 03:17:22  tshanmug
 * Empire modem power on sequence and PLA wait time increased
 *
 * Revision 1.1.4.7  2020/12/22 22:49:28  tshanmug
 * Empire prrq review comment fix
 *
 * Revision 1.1.4.6  2020/12/11 04:34:22  tshanmug
 * Hightower sub6 power OFF/ON using CPLD 0x60 reg when performing modem test
 *
 * Revision 1.1.4.5  2020/11/05 01:29:25  tshanmug
 * Empire Modem power ON/OFF utility updated in basic utility
 *
 * Revision 1.1.4.4  2020/10/12 15:48:35  tshanmug
 * Chrysler menu change, mmwave ant test added and Empire modem code cleanup
 *
 * Revision 1.1.4.3  2020/09/18 17:53:53  ksabzwar
 * Add Empire & Chrysler 5G modem SKU check
 *
 * Revision 1.1.4.2  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

