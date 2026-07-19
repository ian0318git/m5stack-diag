/* $Id: plug_lte_telit_lib.c,v 1.9 2019/10/16 02:30:58 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_telit/plug_lte_telit_lib.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_telit_lib.c - Pluggable LTE Telit Library Functions
 *
 * Copyright (c) 2018 - 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "proto.h"
#include "nvmonvars.h"
#include "dev_pca9555.h"
#include "plug_lte_telit_host.h"
#include "plug_gpio_exp_test.h"
#include "plug_gpio_exp_lib.h"
#include "plug_lte_telit_lib.h"
#include "plug_lte_telit_test.h"
#include "plug_lte_telit_util.h"
#include "plug_slot.h"

static int plug_lte_telit_set_shutdown_indicator(void);
static int plug_lte_telit_modem_power_down(void);
static int plug_lte_telit_gpio_exp_table_trav(int, int *);
static int plug_lte_telit_read_modem_product_info(char *);
static int plug_lte_telit_set_testmode(int);
static int plug_lte_telit_config_carrier_img(void);
static int plug_lte_telit_config_modem_to_usb3p0(void);
static boolean plug_lte_telit_soft_shutdown_indicator_is_set(void);

int plug_lte_telit_set_gpio_exp_test_reg(int);
int plug_lte_telit_gpio_exp_out_init(void);
int plug_lte_telit_gpio_exp_dir_init(void);
int plug_lte_telit_gpio_exp_sim_card_detect(int);
int plug_lte_telit_modem_pwr_ctrl(int);
int plug_lte_telit_modem_pwr_down_seq(struct plug_intf_t *);
int plug_lte_telit_insmod(int);
int plug_lte_telit_usb_deb_enable(int);
int plug_lte_telit_w_disable1_ctrl(int);
int plug_lte_telit_modem_pwron_pin_ctrl(int);
int plug_lte_telit_modem_reset_pin_ctrl(int);
int plug_lte_telit_usb_is_found(int, int, int);
int plug_lte_telit_get_tty_devname(char *);
int plug_lte_telit_toggle_led(int, int);
int plug_lte_telit_hard_reset(void);
int plug_lte_telit_soft_reboot(int);
int plug_lte_telit_select_modem_serdes(int);
int plug_lte_telit_wwan_led_output_enable_ctrl(int);
int plug_lte_telit_wwan_led_sim_select(int);
int plug_lte_telit_dev_create(dev_lte_telit_object_t *);
int plug_lte_telit_dump_modem_temp(void);
int plug_lte_telit_set_modem_sku(void);
int plug_lte_telit_set_modem_pwron_pin_test(int);
int plug_lte_telit_set_modem_default_feature(void);
void plug_lte_telit_modem_searching (int *, int *);
void plug_lte_telit_get_modem_sku(int *);
void plug_lte_telit_store_usb_devinfo(int);
void plug_lte_telit_set_current_usb_port(int);
void plug_lte_telit_get_current_usb_port(int *);
void plug_lte_telit_atcmd_assign_ttydev(void);
boolean plug_lte_telit_has_temp_sensor(void);
boolean plug_lte_telit_has_dedicated_gps_antenna(void);
boolean plug_lte_telit_has_2_rssi_antenna(void);
boolean plug_lte_telit_supports_pcie_intf(void);

/* LTE GPIO Expander Bit Map */
static lte_gpio_exp lte_telit_gpio_exp_table[] = {
    {MANDATORY , OUTPUT , PORT0 , 0, ENABLE_LED_GREEN, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 1, ENABLE_LED_YELLOW, LOW},
    {MANDATORY , OUTPUT , PORT0 , 2, HOST_SERDES_TYPE_0, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 3, HOST_SERDES_TYPE_1, LOW},
    {MANDATORY , INPUT , PORT0 , 4, PRIMARY_INTERFACE_READY, HIGH},
    {MANDATORY , INPUT , PORT1 , 0, DYING_GASP_OK, HIGH},
    {MANDATORY , OUTPUT , PORT1 , 1, USB_DEBUG_ENABLE, LOW},
    {MANDATORY , OUTPUT , PORT1 , 2, W_DISABLE_1, LOW},
    {MANDATORY , INPUT , PORT1 , 3, MODEM_TRIGGER_SEND_SMS, HIGH},
    {MANDATORY , INPUT , PORT1 , 4, USB3_SERDES_SELECTOR, LOW},
    {MANDATORY , INPUT , PORT1 , 5, SAFE_PWR_REMOVAL, LOW},
    {MANDATORY , OUTPUT , PORT1 , 6, RESET, LOW},
    {MANDATORY , OUTPUT , PORT1 , 7, MODEM_POWER_ON, HIGH},
    {OPTIONAL , OUTPUT , PORT0 , 0, LED_SIM0_OK, HIGH},  
    {OPTIONAL , OUTPUT , PORT0 , 1, LED_SIM0_NOT_OK, LOW},
    {OPTIONAL , OUTPUT , PORT0 , 2, LED_SIM1_OK, HIGH},
    {OPTIONAL , OUTPUT , PORT0 , 3, LED_SIM1_NOT_OK, LOW},
    {OPTIONAL , OUTPUT , PORT0 , 4, LED_GPS_OK, HIGH},
    {OPTIONAL , OUTPUT , PORT0 , 5, LED_GPS_NOT_OK, LOW},
    {OPTIONAL , OUTPUT , PORT0 , 6, LED_RSSI0, HIGH},
    {OPTIONAL , OUTPUT , PORT0 , 7, LED_RSSI1, HIGH},
    {OPTIONAL , OUTPUT , PORT1 , 0, LED_RSSI2, HIGH},
    {OPTIONAL , OUTPUT , PORT1 , 1, LED_RSSI3, HIGH},
    {OPTIONAL , OUTPUT , PORT1 , 2, LED_4G_3G, LOW},
    {OPTIONAL , OUTPUT , PORT1 , 3, WWAN_LED_ENABLE, HIGH},
    {OPTIONAL , OUTPUT , PORT1 , 4, WWAN_LED_SIM_SEL, LOW},
    {OPTIONAL , OUTPUT , PORT1 , 5, SIM_SELECT, LOW},
    {OPTIONAL , INPUT , PORT1 , 6, SIM0_DETECT, HIGH},
    {OPTIONAL , INPUT , PORT1 , 7, SIM1_DETECT, HIGH},
};

int plug_lte_telit_gpio_exp_siz = sizeof(lte_telit_gpio_exp_table)
                                  /sizeof(lte_gpio_exp);
/* Set up test register for GPIO exp register test, simply test LEDs */
reg_info_t plug_lte_telit_man_pca9555_reg_table[] = {
    {"Output Port 0", OUTPUT_PORT_0, PCA9555_REG_RW_FLAG,
     {(ulong)&pca9555_reg_ext}, 0x00, 0x02},
    {0, 0, 0, {0}, 0, 0},
};
reg_info_t plug_lte_telit_opt_pca9555_reg_table[] = {
    {"Output Port 0", OUTPUT_PORT_0, PCA9555_REG_RW_FLAG,
     {(ulong)&pca9555_reg_ext}, 0x00, 0xff},
    {0, 0, 0, {0}, 0, 0},
    {"Output Port 0", OUTPUT_PORT_1, PCA9555_REG_RW_FLAG,
     {(ulong)&pca9555_reg_ext}, 0x00, 0x1f},
    {0, 0, 0, {0}, 0, 0},
};

int plug_lte_telit_msku = -1;
int plug_lte_telit_current_usb_port = USB3P0;
static char lte_telit_atcmd_tty_dev[64] = {0,};
extern uint32 err_report(dev_object_t *, char *, uint32);

static lte_telit_usb_config_t plug_lte_telit_usb_cfg[MAX_USB_PORT];

/*******************************************************************************
 * Function   : plug_lte_telit_set_gpio_exp_test_reg
 * Description: Attach the test register map of PCA9555 device for use
 * Inputs     : which_dev_fun - target GPIO
 * Outputs    :PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_set_gpio_exp_test_reg (int which_dev_fun)
{
    if (which_dev_fun == MANDATORY) {
        pca9555_reg_map = plug_lte_telit_man_pca9555_reg_table;
    } else {
        pca9555_reg_map = plug_lte_telit_opt_pca9555_reg_table;
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_gpio_exp_table_trav
 * Description: Function to traverse GPIO expander table
 * Inputs     : which_dev_fun - target GPIO
 *              *index - Index of GPIO expander table
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_gpio_exp_table_trav (int which_dev_fun, int *index)
{
    int ix;

    for (ix = 0; ix < plug_lte_telit_gpio_exp_siz; ix++) {
        if (lte_telit_gpio_exp_table[ix].dev_fun == which_dev_fun) {
            *index = ix;
            return (PASSED);
        }
    }
    printf("%s: Invalid GPIO device.\n", __func__);

    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_gpio_exp_out_init
 * Description: Function to initialize the output value of each port 
 *              if this port is configured as output
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_gpio_exp_out_init (void)
{
    int ix;

    for (ix = 0; ix < plug_lte_telit_gpio_exp_siz; ix++) {
        if (lte_telit_gpio_exp_table[ix].dir == OUTPUT) {
            plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev, 
                                     lte_telit_gpio_exp_table[ix].port,
                                     lte_telit_gpio_exp_table[ix].bit,
                                     lte_telit_gpio_exp_table[ix].def_val);
        }
    }
    return (PASSED);
}

                             
/*******************************************************************************
 * Function   : plug_lte_telit_gpio_exp_dir_init
 * Description: Function to initialize port direction to Output/Input 
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_gpio_exp_dir_init (void)
{
    int ix;

    for (ix = 0; ix < plug_lte_telit_gpio_exp_siz; ix++) {
        plug_gpio_exp_config_port(lte_telit_gpio_exp_table[ix].dev, 
                                  lte_telit_gpio_exp_table[ix].port,
                                  lte_telit_gpio_exp_table[ix].bit,
                                  lte_telit_gpio_exp_table[ix].dir);
    }
    return (PASSED);
}
                             

/*******************************************************************************
 * Function   : plug_lte_telit_has_temp_sensor
 * Description: Function to check whether temperature sensor is stuffed on 
 *              Pluggable LTE
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean plug_lte_telit_has_temp_sensor (void)
{
    /* The Temp Sensor device will only be populated for proto type */
    return (TRUE);
}


/*******************************************************************************
 * Function   : plug_lte_telit_supports_pcie_intf
 * Description: Function to check whether LTE modem supports PCIe3.0 interface
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean plug_lte_telit_supports_pcie_intf (void)
{
    /* Currently hardware not yet support PCIe test */
    return (FALSE);
}


/*******************************************************************************
 * Function   : plug_lte_telit_has_2_rssi_antenna
 * Description: Function to check whether pluggable LTE has #0/#1 RSSI antenna
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean plug_lte_telit_has_2_rssi_antenna (void)
{
    int msku;

    plug_lte_telit_get_modem_sku(&msku);
    if (msku == PLUG_LTE_TELIT_LM960) {
        return (TRUE);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : plug_lte_telit_has_dedicated_gps_antenna
 * Description: Function to check whether pluggable LTE has dedicated gps 
 *              antenna
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean plug_lte_telit_has_dedicated_gps_antenna (void)
{
    int msku;

    plug_lte_telit_get_modem_sku(&msku);
    if (msku == PLUG_LTE_TELIT_LM940) {
        return (TRUE);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : plug_lte_telit_get_current_usb_port
 * Description: Get the modem current USB port
 * Inputs     : uport - which USB port modem is connected to 
 * Outputs    : None
 *******************************************************************************
 */
void plug_lte_telit_get_current_usb_port (int *uport)
{
    *uport = plug_lte_telit_current_usb_port;
}

    
/*******************************************************************************
 * Function   : plug_lte_telit_set_current_usb_port
 * Description: Set the modem current USB port
 * Inputs     : uport - which USB port modem is connected to 
 * Outputs    : None
 *******************************************************************************
 */
void plug_lte_telit_set_current_usb_port (int uport)
{
    plug_lte_telit_current_usb_port = uport;
}

    
/*******************************************************************************
 * Function   : plug_lte_telit_read_modem_product_info
 * Description: Return the modem USB device product info
 * Inputs     : product - pointer to store product info 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_read_modem_product_info (char *product)
{
    FILE *file;
    char fname[64];
    int uport = -1;

    /* Get the current modem USB port */
    plug_lte_telit_get_current_usb_port(&uport);

    /* Sanity check */
    sprintf(fname, "%s/%s", USB_SYS_DRV_PATH,
                            plug_lte_telit_usb_cfg[uport].usb_devinfo);
    if (access(fname, F_OK) == -1) {
        return (FAILED);
    }

    /* Get the product info */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH,
                               plug_lte_telit_usb_cfg[uport].usb_devinfo,
                               USB_SYS_PRODUCT_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%s", product);

    fclose(file);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_set_modem_sku
 * Description: Store the modem sku to global variable
 * Inputs     : None 
 * Outputs    : None
 *******************************************************************************
 */
int plug_lte_telit_set_modem_sku (void)
{
    char product[64];

    if (plug_lte_telit_read_modem_product_info(product) != PASSED ) {
        return (FAILED);
    }

    if ((strstr(product, PLUG_LTE_LM940_STR)) != NULL) {
        plug_lte_telit_msku = PLUG_LTE_TELIT_LM940;
    } else if ((strstr(product, PLUG_LTE_LM960_STR)) != NULL) {
        plug_lte_telit_msku = PLUG_LTE_TELIT_LM960;
    } else {
        printf("Mis-matching modem sku\n");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_get_modem_sku
 * Description: Return the modem sku
 * Inputs     : msku - pointer to modem sku 
 * Outputs    : None
 *******************************************************************************
 */
void plug_lte_telit_get_modem_sku (int *msku)
{
    if (*msku == -1) {
        plug_lte_telit_set_modem_sku();
    }
    *msku = plug_lte_telit_msku;
}


/*******************************************************************************
 * Function   : plug_lte_telit_set_modem_default_feature
 * Description: Restore modem to default test setup
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_set_modem_default_feature (void)
{
    int modem_found = FALSE, modem_uport = -1;

    printf("Pluggable modem searching...\n");
    plug_lte_telit_modem_searching(&modem_found, &modem_uport);
    
    if (modem_found != TRUE) {
        /* Modem not detect, can refer CDETS:
         * 1. CSCvq98193: Found host USB3.0 port stucks in suspend mode(U3)  
         *    in failin gcondition. Can be recovered by disable auto-suspend 
         *    feature (set autosuspend value to -1).
         * 2. CSCvq73276: Found host USB3.0 port entered in compliance mode.
         *    When USB3.0 port entered in compliance mode, warm reset it. */
        printf("Telit Modem is not detected\n");
        printf("Please check PLS in PORTSC register for USB port status. "
               "If Link is in the U3/Compliance mode State, please "
               "refer to CSCvq98193 & CSCvq73276 for more information.\n");
        return (FAILED);
    } else {
        plug_lte_telit_set_current_usb_port(modem_uport);
    }

    /* Set modem carrier image */
    if (plug_lte_telit_config_carrier_img() != PASSED) {
        printf("Failed to set modem carrier image\n");
        return (FAILED);
    }

    /* Set modem in default USB mode(super-speed mode) */
    if (modem_uport != USB3P0) {
        if (plug_lte_telit_config_modem_to_usb3p0() != PASSED) {
            printf("Telit modem is not detected\n");
            return (FAILED);
        }
        plug_lte_telit_set_current_usb_port(USB3P0);
    }

    plug_lte_telit_set_modem_sku();

    /* Set modem in default testmode(operation mode) */
    if (plug_lte_telit_set_testmode(OPERATION_MODE) != PASSED) {
        printf("Failed to set modem in operation mode\n");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_config_modem_to_usb3p0
 * Description: Configure modem to usb3.0
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_config_modem_to_usb3p0 (void)
{
    int restore_rc = FAILED; 
    int modem_found_3p0 = FALSE;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
   
    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* Restore USB2.0 to USB3.0 */
    restore_rc = plug_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &plug_lte_telit_obj,
                                                   SUPER_SPD_USB);
    if (restore_rc != PASSED) {
        printf("Switching modem to USB mode(3.0) fails\n");
        goto __exit;
    }
    
    /* Polling USB3.0 bus to see if modem is detected */
    modem_found_3p0 = plug_lte_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);
    if (modem_found_3p0 != TRUE) {
        printf("Modem is not found on USB3.0 bus\n");
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
 * Function   : plug_lte_telit_set_testmode
 * Description: Function to set modem testmode
 * Inputs     : testmode - which mode want to set
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_set_testmode (int testmode)
{
    int rc = FAILED, ret = FALSE, cur_mode;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    
    /* Create device object */
    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    ret = plug_lte_telit_obj_p->callin_fvt->modem_in_operation_mode(
                                           (dev_object_t *)&plug_lte_telit_obj);
    /* Current testmode */
    if (ret == TRUE) {
        cur_mode = OPERATION_MODE;
    } else {
        cur_mode = TEST_MODE;
    }
    
    /* Set testmode */
    if (testmode != cur_mode) {
        if (testmode == OPERATION_MODE) {
            rc = plug_lte_telit_obj_p->callin_fvt->modem_enable_op_mode(
                                                   (dev_object_t *)
                                                   &plug_lte_telit_obj);
            if (rc != PASSED) {
                printf("Failed to set modem in operation mode.\n");
                return (FAILED);
            }
        } else {
            /* testmode == TEST_MODE */
            rc = plug_lte_telit_obj_p->callin_fvt->modem_enable_test_mode(
                                                   (dev_object_t *)
                                                   &plug_lte_telit_obj);
            if (rc != PASSED) {
                printf("Failed to set modem in test mode.\n");
                return (FAILED);
            }
        }
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_config_carrier_img
 * Description: Configure modem carrier image
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_config_carrier_img (void)
{
    int rc = FAILED;
    char carrier[16] = {0};
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    strncpy(carrier, GENERIC, sizeof(carrier));

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* Disable modem carrier image auto-switching feature */
    if (plug_lte_telit_obj_p->callin_fvt->modem_disable_img_switching(
                              (dev_object_t *)&plug_lte_telit_obj) != PASSED) {
        printf("Disabling modem carrier auto-switching fails\n");
        goto __exit;
    }
    
    /* Check whether the current image is as expected */
    if (plug_lte_telit_obj_p->callin_fvt->modem_check_lte_img(
                              (dev_object_t *)&plug_lte_telit_obj, carrier)
                              != TRUE) {
        /* If not, set modem image */
        printf("Set LTE carrier image\n");
        if (plug_lte_telit_obj_p->callin_fvt->modem_set_lte_img(
                                  (dev_object_t *)&plug_lte_telit_obj, carrier)
                                  !=PASSED ) {
            printf("Failed to configure modem carrier image\n");
            goto __exit;
        }
    }
    rc = PASSED;

 __exit:
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_store_usb_devinfo
 * Description: This function stores the usb devices info to global variable
 * Inputs     : plug_slot - which pluggable slot
 * Outputs    : None
 *******************************************************************************
 */
void plug_lte_telit_store_usb_devinfo (int plug_slot)
{
    int ix;

    /* usb_devinfo is the root USB device info of pluggable LTE module,
     * provided by host.
     * (e.g. 3-1 for USB2.0 mode, 4-1 for USB3.0 mode on Star C1101 platform)
     * at_usb_devinfo is one of the downstream port of modem, which is used
     * for AT command transition.(e.g. 3-1:1.4, 4-1:1.4, 3-1.1:1.4) */
    plug_lte_telit_host_get_plug_usb_devinfo(plug_slot,
                            plug_lte_telit_usb_cfg[USB2P0].usb_devinfo,
                            plug_lte_telit_usb_cfg[USB3P0].usb_devinfo,
                            plug_lte_telit_usb_cfg[DEBUG_USB].usb_devinfo);

    for (ix = 0; ix <= MAX_USB_PORT - 1; ix++) {
        sprintf(plug_lte_telit_usb_cfg[ix].at_usb_devinfo, "%s:%s",
                plug_lte_telit_usb_cfg[ix].usb_devinfo, USB_AT_CMD_PORT);
    }
}


/*******************************************************************************
 * Function    : plug_lte_telit_atcmd_assign_ttydev
 * Description : Assign TTY device name to "telit_at_cmd" via mdev 
 * Inputs      : none 
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
void plug_lte_telit_atcmd_assign_ttydev (void)
{
    /* Assign to default value */
    sprintf(lte_telit_atcmd_tty_dev, "%s", TELIT_AT_CMD_DEV_NAME);
}


/*******************************************************************************
 * Name: plug_lte_telit_get_tty_devname 
 * Description: This function returns USB serial TTY device name which the
 *              specified usb device attaches to.
 * Input: *tty_dev - Pointer to store which TTY device name that the specified
 *                   usb device attaches to
 * Example: ttyUSB2, ttyUSB3
 * Output: PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_get_tty_devname (char *tty_dev)
{
    /* Sanity check */
    if (tty_dev == NULL) {
        printf("%s: NULL pointer\n", __func__);
    }

    /* Get TTY Device number from system */
    plug_lte_telit_atcmd_assign_ttydev();
    sprintf(tty_dev, lte_telit_atcmd_tty_dev);
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_insmod  
 * Description: To insert LTE Telit driver for the test 
 * Inputs     : input - TRUE/FALSE
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_insmod (int input)
{
    char drv_path[64];
    char fname[64];
    char cmd[128];

    plug_lte_telit_host_get_modem_drv_path(drv_path);
    
    sprintf(fname, "%s/%s", drv_path, TELIT_USB_SERIAL_DRV);

    if (access(fname, F_OK) == -1) {
        printf("Can not find modem driver.\n");
        return (FAILED);
    }

    if (input == TRUE) {
        sprintf(cmd, "%s %s", INSMOD_CMD, fname);
    } else {
        sprintf(cmd, "%s %s", RMMOD_CMD, fname);
    }

    system(cmd);

    /* Based on LM960 product spec, modem is activated 5 seconds after
       the USB driver is loaded */
    if (input == TRUE) {
        printf("Wait 5 seconds for modem to be activated.\n");
        sleep(TELIT_LTE_ACTIVATED_DELAY);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_usb_is_found
 * Description: This function polls/detects modem as specified USB device
 * Inputs     : uport - which USB port modem is connected to
 *              poll - flag to introduce polling mechanism, TRUE/FALSE
 *              timeout - polling time out(*10 msec)
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
int plug_lte_telit_usb_is_found (int uport, int poll, int timeout)
{
    char dirname[128];
    char fname[64];
    char at_tty_dev[64];
    int ret = -1;
    int dir_found = FALSE;

    /* Check if the directory exists */
    sprintf(dirname, "%s/%s/%s", USB_SYS_DRV_PATH,
                                 plug_lte_telit_usb_cfg[uport].usb_devinfo,
                                 plug_lte_telit_usb_cfg[uport].at_usb_devinfo);

    while (timeout > 0) {
        ret = access(dirname, F_OK);
        /* directory is found */
        if (ret != -1) {
            dir_found = TRUE;
            break;
        }
        
        if (poll == FALSE) {
            break;
        }
        timeout--;
        msleep(10);
    }

    if (dir_found != TRUE) {
        return (FALSE);
    }

    /* Wait for USB serial driver to attach LTE modem with tty devices */
    msleep(PLUG_LTE_TELIT_TTY_ATTACH_DELAY);

    if (plug_lte_telit_get_tty_devname(at_tty_dev) != PASSED) {
        return (FALSE);
    }

    sprintf(fname, "%s/%s", USB_TTY_PATH, at_tty_dev);

    while (timeout > 0) {
        ret = access(fname, F_OK);
        if (ret != -1) {
            return (TRUE);
        }
        if (poll == FALSE) {
            break;
        }
        timeout--;
        msleep(10);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_searching
 * Description: This function scans USB busses to seek for LTE modem
 * Inputs     : modem_found - flag to indicate LTE modem is found or not
 *              modem_uport - pointer to store which usb port that modem
 *                            connects to
 * Outputs    : None
 *******************************************************************************
 */
void plug_lte_telit_modem_searching (int *modem_found, int *modem_uport)
{
    int ix;

    for (ix = 0; ix < MAX_USB_PORT; ix++) {
        *modem_found = plug_lte_telit_usb_is_found(ix, TRUE,
                                                   PROBE_LTE_TELIT_USB_TOUT);
        if (*modem_found == TRUE) {
            *modem_uport = ix;
            break;
        }
    }
}
 

/*******************************************************************************
 * Function    : plug_lte_telit_dev_create
 * Description : Function to create LTE Telit Device Object
 * Inputs      : lte_telit_obj - Pointer of LTE Telit device driver object
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_dev_create (dev_lte_telit_object_t *plug_lte_telit_obj)
{
    int msku;
    dev_object_t *dev = (dev_object_t *)plug_lte_telit_obj;

    /* Create common device object */
    lte_telit_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    plug_lte_telit_obj->base.dev_object_fvt->dev_attach(dev);

    /* Assign modem type */
    memset(plug_lte_telit_obj->model, 0, sizeof(plug_lte_telit_obj->model));
    plug_lte_telit_get_modem_sku(&msku);
    if (msku == PLUG_LTE_TELIT_LM940) {
       plug_lte_telit_obj->modem_type = TELIT_LM940;
    } else {
       plug_lte_telit_obj->modem_type = TELIT_LM960;
    } 

    plug_lte_telit_obj->callout_fvt->get_current_usb_port =
                                     plug_lte_telit_get_current_usb_port;
    plug_lte_telit_obj->callout_fvt->get_ttyusb_dev_name = 
                                     plug_lte_telit_get_tty_devname;
    return (PASSED); 
}


/*******************************************************************************
 * Function   : plug_lte_telit_usb_deb_enable 
 * Description: Function to route USB2 signals from the EM/WP modem
 *              to the onboard USB connector
 * Inputs     : input - 0 for disable, 1 for enable 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int plug_lte_telit_usb_deb_enable (int input)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(USB_DEBUG_ENABLE, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 input) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_w_disable1_ctrl 
 * Description: Function to control WDISABLE1# signal
 * Inputs     : value - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int plug_lte_telit_w_disable1_ctrl (int value)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(W_DISABLE_1, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 value) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_pwron_pin_ctrl 
 * Description: Function to control MODEM_POWER_ON signal
 * Inputs     : value - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int plug_lte_telit_modem_pwron_pin_ctrl (int value)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(MODEM_POWER_ON, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 value) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_reset_pin_ctrl 
 * Description: Function to control WDISABLE1# signal
 * Inputs     : value - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int plug_lte_telit_modem_reset_pin_ctrl (int value)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(RESET, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 value) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_toggle_led 
 * Description: Function to toggle port dirve to High/Low
 * Inputs     : which_led - EN, SIM0/1, GPS, and RSSI LEDs 
 * Outputs    : none 
 *******************************************************************************
 */
int plug_lte_telit_toggle_led (int which_led, int led_on_off)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(which_led, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 led_on_off) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}

  
/*******************************************************************************
 * Function   : plug_lte_telit_select_modem_serdes 
 * Description: Function to toggle GPIO exp. Host SerDes Type pin to select
 *              modem SerDes type
 * Inputs     : serdes_type
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_select_modem_serdes (int serdes_type)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(HOST_SERDES_TYPE_0, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 serdes_type) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_wwan_led_output_enable_ctrl 
 * Description: Function to toggle GPIO exp. WWAN_LED Enable pin to enable or
 *              disable WWAN_LED signal
 * Inputs     : value - 0 for low, 1 for high
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_wwan_led_output_enable_ctrl (int value)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(WWAN_LED_ENABLE, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 value) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_wwan_led_sim_select 
 * Description: Function to toggle GPIO exp. WWAN_LED_SIM_SEL pin to select 
 *              which SIM LED will the WWAN_LED signal be routed to.
 * Inputs     : value - 0 for SIM0, 1 for SIM1
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_wwan_led_sim_select (int value)
{
    int ix;

    plug_lte_telit_gpio_exp_table_trav(WWAN_LED_SIM_SEL, &ix);
    if (plug_gpio_exp_drive_port(lte_telit_gpio_exp_table[ix].dev,
                                 lte_telit_gpio_exp_table[ix].port,
                                 lte_telit_gpio_exp_table[ix].bit,
                                 value) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_gpio_exp_sim_card_detect
 * Description: This function returns the staus of SIM detect bit on optional
 *              GPIO expander
 * Inputs     : which_sim - Testing SIM slot
 * Outputs    : TRUE - SIM card is detected
 *              FALSE - SIM card is not detected
 *******************************************************************************
 */
int plug_lte_telit_gpio_exp_sim_card_detect (int which_sim)
{
    int ix, val = HIGH;
    int gpio_func;

    if (which_sim == SIM0) {
        gpio_func = SIM0_DETECT;
    } else if (which_sim == SIM1) {
        gpio_func = SIM1_DETECT;
    } else {
        printf("%s:Invalid SIM slot number(%d)\n", __func__, which_sim);
        return (FAILED);
    }

    plug_lte_telit_gpio_exp_table_trav(gpio_func, &ix);
    plug_gpio_exp_read_port(lte_telit_gpio_exp_table[ix].dev,
                            lte_telit_gpio_exp_table[ix].port,
                            lte_telit_gpio_exp_table[ix].bit,
                            &val);

    if (val == LOW) {
        return (TRUE);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : plug_lte_telit_dump_modem_temp
 * Description: Function to return the current temperature of Telit LTE modem
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_dump_modem_temp (void)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_dump_temp((dev_object_t *)
                                                           &plug_lte_telit_obj);
    if (rc != PASSED) {
        printf("Fail to dump LTE modem temperature\n");
    }
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_pwr_ctrl 
 * Description: Function to power on/off LTE Telit modem 
 * Inputs     : pwr_opt - 0 for power off/1 for power on
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_modem_pwr_ctrl (int pwr_opt)
{
    int ix, iy, val;
    int msku;

    plug_lte_telit_get_modem_sku(&msku);

    if (pwr_opt == TRUE) {
        printf("Power on Pluggable LTE Telit modem.\n");
        if (plug_lte_telit_modem_pwron_pin_ctrl(HIGH) != PASSED) {
            return (FAILED);
        }

        /* Based on LM940/960 product spec., enumeration starts within 30
         * seconds */
        printf("Wait %d seconds for modules to start enumeration.\n",
                MODEM_LM9X0_PWR_ON_DELAY);
        sleep(MODEM_LM9X0_PWR_ON_DELAY);
    } else {

        testname("Modem Power Down");
        
        /* 0. Check whether the shutdown indicator is set */
        prpass(testpass, "Check the configuration of modem shutdown "
               "indicator...");
        if (plug_lte_telit_soft_shutdown_indicator_is_set() != TRUE) {
            /* If not, set the shutdown indicator */
            prpass(testpass, "Setting modem shutdown indicator...");
            if (plug_lte_telit_set_shutdown_indicator() != PASSED) {
                printf("Failed to set shutdown indicator\n");
                return (FAILED);
            }
        }

        /* 1. Power down modem */
        if (plug_lte_telit_modem_power_down() != PASSED) {
            printf("Failed to power down pluggable LTE Telit modem\n");
            return (FAILED);
        }

        /* 2. Monitor SAFE_POWER_REMOVAL signal */
        prpass(testpass, "Monitoring SAFE_POWER_REMOVAL signal");
        plug_lte_telit_gpio_exp_table_trav(SAFE_PWR_REMOVAL, &ix);

        for (iy = 0; iy < MODEM_CHK_PWR_TOUT; iy++) {
            val = HIGH;
            plug_gpio_exp_read_port(lte_telit_gpio_exp_table[ix].dev,
                                    lte_telit_gpio_exp_table[ix].port,
                                    lte_telit_gpio_exp_table[ix].bit,
                                    &val);
            if (val == LOW) {
                break;
            }
            msleep(PLUG_LTE_TELIT_POLLING_DELAY);
        }

        if (val == HIGH) {
            printf("The SAVE_POWER_REMOVAL signal didn't transition "
                   "as expected.\n");
            return (FAILED);
        }

    prcomplete(testpass, errcount, (char *)0);

    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_power_down
 * Description: Function to power down modem
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_modem_power_down (void)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_power_down((dev_object_t *)
                                                           &plug_lte_telit_obj);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    if (rc != PASSED) {
        printf("Modem Power Down fails\n");
    }

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_modem_pwr_down_seq
 * Description: A weak function for Telit LTE modem power down
 * Inputs     : plug_ptr - pointr to pluggable structure
 * Outputs    : None
 *******************************************************************************
 */
int plug_lte_telit_modem_pwr_down_seq(struct plug_intf_t *plugp)
{
    int modem_ready = FALSE;
    int ret = PASSED;
   
    /* Suppress printk so kernel won't print out brunch of messages */
    system(TELIT_SYS_SUPPRESS_PRINTK);

    /* Store all USB port info that LTE modem might use from platform */
    plug_lte_telit_store_usb_devinfo(plugp->slot);
     
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
   
    msleep(1000);

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
 * Function   : plug_lte_telit_set_shutdown_indicator 
 * Description: Function to set the modem shutdown indicator 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int plug_lte_telit_set_shutdown_indicator (void)
{
    int rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_set_shdn_indicator(
                                           (dev_object_t *)&plug_lte_telit_obj);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_soft_shutdown_indicator_is_set 
 * Description: Function to check whether the modem soft shutdown indicator is
 *              set or not
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
static boolean plug_lte_telit_soft_shutdown_indicator_is_set (void)
{
    int rc = FALSE;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FALSE);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_softshdn_indic_is_enable(
                                           (dev_object_t *)&plug_lte_telit_obj);

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_set_modem_pwron_pin_test 
 * Description: Function to toggle GPIO exp. Modem_Power_ON pin, and to see the
 *              corresponding modem GPIO pin value is correct or not 
 * Inputs     : gpio_val - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_set_modem_pwron_pin_test (int gpio_val)
{
    int rc = FAILED;
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;

    /* Toggle the Modem_Power_ON pin from GPIO expander */
    if (plug_lte_telit_modem_pwron_pin_ctrl(gpio_val) != PASSED) {
        printf("Failed to set Modem_Power_ON pin to %s\n", gpio_val?
                                                           "HIGH":"LOW");
        return (FAILED);
    }

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_check_gpio5_stat(
                                           (dev_object_t *)&plug_lte_telit_obj,
                                           gpio_val);
    if (rc != PASSED) {
        printf("Unexpected modem GPIO pin value\n");
    }

    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : plug_lte_telit_hard_reset 
 * Description: Function to toggle GPIO exp. Modem Reset pin to hard reset modem
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_hard_reset (void)
{
    int msku;

    printf("[WARNING] This utility should only be used while host cannot"
           " communicate with modem.\n");
    printf("Reset modem.\n");
    if (plug_lte_telit_modem_reset_pin_ctrl(HIGH) != PASSED) {
        return (FAILED);
    }

    plug_lte_telit_get_modem_sku(&msku);
    /* Base on LM960 modem spec, RESET# should be toggle to high at least
     * 1 sec */
    if (msku == PLUG_LTE_TELIT_LM960) {
        msleep(LM960_RST_ASSERTION);
    } else {
        /* Base on LM940 modem spec, RESET# should be toggle to high between
         * 100ms~150ms */
        msleep(LM940_RST_ASSERTION);
    }

    /* Release RESET# */
    if (plug_lte_telit_modem_reset_pin_ctrl(LOW) != PASSED) {
        return (FAILED);
    }
    printf("Wait %d seconds...\n", TELIT_LTE_HARD_RST_DELAY);
    sleep(TELIT_LTE_HARD_RST_DELAY);

    if (plug_lte_telit_usb_is_found(USB3P0, TRUE, PROBE_LTE_TELIT_TOUT)
                                    != TRUE) {
       printf("Modem is not detected.\n");
       return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_telit_soft_reboot 
 * Description: Function to soft reboot LTE modem via AT command
 * Inputs     : reboot_uport - which USB port modem will be connected to
 *                             after re-boot
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_telit_soft_reboot (int reboot_uport)
{
    dev_lte_telit_object_t plug_lte_telit_obj;
    dev_lte_telit_object_t *plug_lte_telit_obj_p = &plug_lte_telit_obj;
    int rc = FAILED;

    if (plug_lte_telit_dev_create(plug_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = plug_lte_telit_obj_p->callin_fvt->modem_reboot((dev_object_t *)
                                                        &plug_lte_telit_obj);
    if (rc != PASSED) {
        printf("Modem soft reboot fails\n");
        return (FAILED);
    }
    plug_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &plug_lte_telit_obj);

    if (plug_lte_telit_usb_is_found(reboot_uport, TRUE, PROBE_LTE_TELIT_TOUT)
                                    != TRUE) {
       printf("Modem is not detected.\n");
       return (FAILED);
    }

    return (PASSED);
}


/*------------------------------------------------------------------
$Log: plug_lte_telit_lib.c,v $
Revision 1.9  2019/10/16 02:30:58  sherliu2
Modify power off sequence based on LTE modem spec.

Revision 1.8  2019/08/14 02:27:18  shjung

1. Disabling modem carrier auto-switching feature and configure CAT18 carrier as Generic
2. Followed the LTE spec to add 5 seconds pause after USB driver is loaded to ensure that modem becomes fully operational

Revision 1.7  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.6.6.3  2019/07/17 06:01:46  sherliu2
Added Hyperloop-PIM module in Poll Slot util

Revision 1.6.6.2  2019/07/10 08:57:35  sherliu2
Supported Hyperloop-PIM

Revision 1.6  2019/07/01 10:05:24  sherliu2
Supported mdev for Hyperloop

Revision 1.5  2019/06/26 03:52:55  shjung
1. Added dump modem basic info utility
2. Due to the default configuration of modem power saving modem is changed in B018 FW, modified W_DISABLE pin related functions
3. Added modem power savinf mode control utility
4. Modified pluggable slot init sequence, instead of powering down all pluggable modules before testing, simply power off non-testing pluggable modules

Revision 1.4  2019/05/22 03:20:40  shjung
Removed carrier configuration related functions

Revision 1.3  2019/05/20 07:28:06  shjung
1. Replace USB serial driver option.ko/usb_wwan.ko with GobiSerial.ko
2. Changes based on last code review comments.
3. Use poll mechanism to query modem functionality level in W_DISABLE pin test
4. Add 1 second delay after close tty device, which is following Telit's test script process.

Revision 1.2  2019/05/14 08:48:37  sherliu2
Support hyperloop

Revision 1.1.2.17  2019/05/09 07:50:18  sherliu2
1. Added Dump Modem USB Connection Info Utility \n 2. Based on review comments to clean up code

Revision 1.1.2.16  2019/05/02 06:13:34  sherliu2
1. Added enable modem fast shutdown utlity. 2. Added restore modem back to the default testing setup(super speed mode).

Revision 1.1.2.15  2019/04/17 10:09:11  sherliu2
remove mdev related

Revision 1.1.2.14  2019/04/10 11:24:44  shjung
Code clean up

Revision 1.1.2.13  2019/04/08 09:54:25  sherliu2
Modified tty device name to symbolic name generated by mdev

Revision 1.1.2.12  2019/04/08 06:41:22  shjung
Check the configuration of modem shutdown indicator while powering down the modem

Revision 1.1.2.11  2019/03/27 08:25:50  shjung
Added Modem_Power_ON pin test

Revision 1.1.2.10  2019/03/14 03:31:06  shjung
Added LTE modem carrier image select/check mechanism

Revision 1.1.2.9  2019/02/23 06:50:28  shjung
Followed Telit's suggestion to increase modem reboot pause time

Revision 1.1.2.8  2019/02/15 02:59:49  shjung
Added WWAN_LED control utility

Revision 1.1.2.7  2019/02/13 01:44:30  shjung
Corrected the MODEM_POWER_ON pin value

Revision 1.1.2.6  2019/01/18 13:40:28  shjung
Added utility to control all LEDs

Revision 1.1.2.5  2019/01/18 06:15:31  shjung

1. Added W_DISABLE pin test
2. Added modem USB mode switching utility
3. Added delay in modem reboot function based on spec
4. Removed USB mode resotre operation when debug port test failed
5. Code clean up

Revision 1.1.2.4  2019/01/15 10:22:19  shjung
Modified the mechanism to get modem USB device info

Revision 1.1.2.3  2018/12/19 19:37:51  shjung
Modified debug port test

Revision 1.1.2.2  2018/12/15 01:41:39  shjung
Corrected the return value of modem soft-reboot utility

Revision 1.1.2.1  2018/12/14 00:50:16  shjung
Initial check-in for Hyperloop



$Endlog$
*/
