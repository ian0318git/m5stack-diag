/* $Id: plug_lte_lib.c,v 1.15 2020/03/31 01:38:00 sherliu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_lte/plug_lte_lib.c,v $
 *------------------------------------------------------------------
 *
 * plug_lte_lib.c - PLUGGABLE LTE Library Functions 
 *
 * Copyright (c) 2015 - 2019 by Cisco Systems, Inc.
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
#include "common_utils.h"
#include "cookie_4.h"
#include "nvmonvars.h"
#include "dev_pca9555.h"
#include "plug_gpio_exp_lib.h"
#include "plug_gpio_exp_test.h"
#include "plug_lte_host.h"
#include "plug_lte_lib.h"
#include "plug_lte_at.h"
#include "plug_common_host.h"
#include "plug_host_fpga_lib.h"

int plug_lte_gpio_exp_dir_init(void);
int plug_lte_gpio_exp_out_init(void);
int plug_lte_toggle_led(int, int);
int plug_lte_pri_interface_rdy(void);
int plug_lte_usb_deb_enable(int);
int plug_lte_w_1_disable(int);
int plug_lte_w_2_disable(int);
int plug_lte_em_hard_reset(void);
int plug_lte_modem_pwr_ctrl(int);
int plug_lte_wp_modem_pwr_ctrl(int);
int plug_lte_em_modem_pwr_ctrl(int);
int plug_lte_wwan_led_enable(int);
int plug_lte_wwan_led_sim_sel(int);
int plug_lte_sim_sel(int);
int plug_lte_sim_detect(int);
int plug_lte_dying_gasp_is_ok(void);
int plug_lte_get_tty_devname(char *);
int plug_lte_check_modem_rdy(int);
int plug_lte_get_modem_status(int);
int plug_lte_modem_is_online(int);
int plug_lte_set_gpio_exp_test_reg(int);
int plug_lte_wp_clr_saf_pwr_remv(void);
int plug_lte_chk_modem_carrier_is_match(void);
int plug_lte_set_modem_carrier(void);
int plug_lte_en_auto_switch_img(void);
int plug_lte_usb_detect(char *, int, int);
int plug_lte_insmod(int);
int plug_lte_force_gps_pin_val(int, int);
int plug_lte_get_gps_pin_status(int, int *);
int plug_lte_modem_soft_reset(int);
int plug_lte_modem_shutdown(void);
void plug_lte_get_ctype(int *);
void plug_lte_set_ctype(int);
void plug_lte_store_usb_devinfo(char *, char *);
void plug_lte_set_at_usb_devinfo(int);
boolean is_plug_lte_em(void);
boolean is_plug_lte_wp(void);
boolean plug_lte_has_2_sim_slot(void);

static int gpio_exp_table_trav(int, int *);
static int plug_lte_usb_get_vid_did_speed(char *, int *, int *, int *);
static int plug_lte_atcmd_assign_ttydev(void);

/* LTE GPIO Expander Bit Map */
static lte_gpio_exp lte_gpio_exp_table[] = {
    {MANDATORY , OUTPUT , PORT0 , 0, ENABLE_LED_GREEN, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 1, ENABLE_LED_YELLOW, LOW},
    {MANDATORY , OUTPUT , PORT0 , 2, HOST_SERDES_TYPE_0, HIGH},
    {MANDATORY , OUTPUT , PORT0 , 3, HOST_SERDES_TYPE_1, LOW},
    {MANDATORY , INPUT , PORT0 , 4, PRIMARY_INTERFACE_READY, HIGH},
    {MANDATORY , INPUT , PORT1 , 0, DYING_GASP_OK, HIGH},
    {MANDATORY , OUTPUT , PORT1 , 1, USB_DEBUG_ENABLE, LOW},
    {MANDATORY , OUTPUT , PORT1 , 2, WDISABLE_1, LOW},
    {MANDATORY , OUTPUT , PORT1 , 3, WDISABLE_2, LOW},
    {MANDATORY , INPUT , PORT1 , 4, GPS_BIAS_OK, HIGH},
    {MANDATORY , INPUT , PORT1 , 5, SAFE_PWR_REMOVAL, LOW},
    {MANDATORY , OUTPUT , PORT1 , 6, RESET, LOW},
    {MANDATORY , OUTPUT , PORT1 , 7, MODEM_POWER_OFF, HIGH},
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
    {OPTIONAL , OUTPUT , PORT1 , 2, LED_4G_3G, HIGH},
    {OPTIONAL , OUTPUT , PORT1 , 3, WWAN_LED_ENABLE, HIGH},
    {OPTIONAL , OUTPUT , PORT1 , 4, WWAN_LED_SIM_SEL, LOW},
    {OPTIONAL , OUTPUT , PORT1 , 5, SIM_SELECT, LOW},
    {OPTIONAL , INPUT , PORT1 , 6, SIM0_DETECT, HIGH},
    {OPTIONAL , INPUT , PORT1 , 7, SIM1_DETECT, HIGH},
};

int lte_gpio_exp_siz = sizeof(lte_gpio_exp_table)/sizeof(lte_gpio_exp);

/* Set up test register for GPIO exp register test, simply test LEDs */
reg_info_t plug_lte_man_pca9555_reg_table[] =
{
    {"Output Port 0",       OUTPUT_PORT_0,  PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},  0x00, 0x02},
    {0, 0, 0, {0}, 0, 0},
};

reg_info_t plug_lte_opt_pca9555_reg_table[] =
{
    {"Output Port 0",       OUTPUT_PORT_0,  PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},  0x00, 0xff},
    {0, 0, 0, {0}, 0, 0},
    {"Output Port 0",       OUTPUT_PORT_1,  PCA9555_REG_RW_FLAG,
            {(ulong)&pca9555_reg_ext},  0x00, 0x1f},
    {0, 0, 0, {0}, 0, 0},
};

int plug_lte_ctype = 0xFFFF;
char plug_lte_usb_devinfo[64] = {0,};
char plug_lte_at_usb_devinfo[64] = {0,};
static char lte_atcmd_tty_dev[64]={0,};
static struct timeval plug_lte_em_pwr_on_t;


/******************************************************************************
 *
 * Name:	plug_lte_set_gpio_exp_test_reg()
 *
 * Description:	Attach the test register map of PCA9555 device for use. 
 *
 * Input:	which_dev_fun - target GPIO
 *
 * Returns:	PASSED/FAILED
 *
 *****************************************************************************/
int plug_lte_set_gpio_exp_test_reg (int which_dev_fun)
{
    if (which_dev_fun == MANDATORY) {
        pca9555_reg_map = plug_lte_man_pca9555_reg_table;
    } else {
        pca9555_reg_map = plug_lte_opt_pca9555_reg_table;
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_usb_detect
 * Description: Enumerates USB and detects USB device by given vendor and device
 *              ID and speed
 * Inputs     : usb_devinfo - USB device info(e.g. 3-1, 4-1)
 *              vid - Vendor ID
 *              speed - 480 (USB 2.0) or 5000 (USB 3.0)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_usb_detect (char *usb_devinfo, int vid, int speed)
{
    int dev_vid, dev_did, dev_speed;

    if (plug_lte_usb_get_vid_did_speed(usb_devinfo, &dev_vid, &dev_did, 
                                       &dev_speed) == FAILED) {
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) { 
        printf("Vendor ID=%#x, Device ID=%#x, Speed=%#x\n", dev_vid, dev_did, 
                                                            dev_speed);
    }

    if ((vid == dev_vid) && (speed == dev_speed)) {
        return (PASSED);
    }

    return (FAILED);
}


/*******************************************************************************
 * Function   : plug_lte_usb_get_vid_did_speed
 * Description: This function reads from system USB file and return Vendor ID, 
 *              Device ID and speed
 * Inputs     : usb_path - USB Path, e.g. 1-1, 3-1, 4-1
 *              *vid, *did, *speed
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
static int plug_lte_usb_get_vid_did_speed (char *usb_path, int *vid, 
                                           int *did, int *speed)
{
    FILE *file;
    char fname[64];


    /* Check if the file exists */
    sprintf(fname, "%s/%s", USB_SYS_DRV_PATH, usb_path);
    if (access(fname, F_OK) == -1) {
        return (FAILED);
    }

    /* Get the Vendor ID */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH, usb_path, USB_SYS_VID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%x", vid);

    fclose(file);

    /* Get the Product ID */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH, usb_path, USB_SYS_DID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%x", did);

    fclose(file);

    /* Get the Speed */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH, usb_path, USB_SYS_SPEED_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%d", speed);

    fclose(file);

    return (PASSED);
}


/********************************************************************
 * Function   : plug_lte_gpio_exp_out_init
 * Description: Function to initialize the output value of each port 
 *              if this port is configured as output
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_gpio_exp_out_init (void)
{
    int ix;

    for (ix = 0; ix < lte_gpio_exp_siz; ix++) {
        if (lte_gpio_exp_table[ix].dir == OUTPUT) {
            plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev, 
                                     lte_gpio_exp_table[ix].port,
                                     lte_gpio_exp_table[ix].bit,
                                     lte_gpio_exp_table[ix].def_val);
        }
    }
    return (PASSED);
}


/********************************************************************
 * Function   : plug_lte_gpio_exp_dir_init
 * Description: Function to initialize port direction to Output/Input
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_gpio_exp_dir_init (void)
{
    int ix;

    for (ix = 0; ix < lte_gpio_exp_siz; ix++) {
        plug_gpio_exp_config_port(lte_gpio_exp_table[ix].dev, 
                                  lte_gpio_exp_table[ix].port,
                                  lte_gpio_exp_table[ix].bit,
                                  lte_gpio_exp_table[ix].dir);
    }
    return (PASSED);
}


/********************************************************************
 *
 * Function   : gpio_exp_table_trav
 * Description: Function to traverse GPIO expander table
 * Inputs     : which_dev_fun - target GPIO
 *              *index - index of GPIO expander table
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
static int gpio_exp_table_trav (int which_dev_fun, int *index)
{
    int ix;

    for (ix = 0; ix < lte_gpio_exp_siz; ix++) {
        if (lte_gpio_exp_table[ix].dev_fun == which_dev_fun) {
            *index = ix;
            return (PASSED);
        } 
    }
    printf("%s:Invalid GPIO device.\n", __func__);

    return (FAILED);
}


/********************************************************************
 *
 * Function   : plug_lte_toggle_led 
 * Description: Function to toggle port dirve to High/Low
 * Inputs     : which_led - EN, SIM0/1, GPS, and RSSI LEDs 
 * Outputs    : none 
 *
 ********************************************************************/
int plug_lte_toggle_led (int which_led, int led_on_off)
{
    int ix;

    /* Traverse GPIO expander table */
    gpio_exp_table_trav(which_led, &ix); 

    /* Drive GPIO to High/Low */
    if (plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                                 lte_gpio_exp_table[ix].port,
                                 lte_gpio_exp_table[ix].bit,
                                 led_on_off) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/********************************************************************
 *
 * Function   : set_plug_lte_host_serdes 
 * Description: Function to set host serdes type 
 * Inputs     : type - 10G-KR
 *                     1000BASE-X
 *                     PCIe 
 * Outputs    : none 
 *
 ********************************************************************/
int set_plug_lte_host_serdes (int type)
{
    int ix;
    int bit_2, bit_3;
    
    gpio_exp_table_trav(HOST_SERDES_TYPE_0, &ix); 
    
    if (type == PCIe) {
        bit_2 = bit_3 = LOW;
    } else if (type == SERDES_1000BASEX) {
        bit_2 = HIGH;
        bit_3 = LOW;
    } else if (type == SERDES_10GKR) {
        bit_2 = LOW;
        bit_3 = HIGH;
    } else {
        printf("%s: Invalid Serdes Type %d\n", __func__, type);
        return (FAILED);
    }

    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             bit_2);
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             bit_3);

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_pri_interface_rdy 
 * Description: Function to check if primary interface is ready
 * Inputs     : none
 * Outputs    : TRUE OR FALSE
 *
 ********************************************************************/
int plug_lte_pri_interface_rdy (void)
{
    int val, ix;

    gpio_exp_table_trav(PRIMARY_INTERFACE_READY, &ix); 
    plug_gpio_exp_read_port(lte_gpio_exp_table[ix].dev,
                            lte_gpio_exp_table[ix].port,
                            lte_gpio_exp_table[ix].bit,
                            &val);
    if (val == LOW) {
        printf("%s:Pluggable primary interface is not ready.\n", __func__);
        return (FALSE);
    }
    return (TRUE);
}


/********************************************************************
 *
 * Function   : plug_lte_dying_gasp_is_ok 
 * Description: Function to check if primary interface is ready
 * Inputs     : none
 * Outputs    : TRUE or FALSE
 *
 ********************************************************************/
int plug_lte_dying_gasp_is_ok (void)
{
    int val, ix;
    
    gpio_exp_table_trav(DYING_GASP_OK, &ix); 
    plug_gpio_exp_read_port(lte_gpio_exp_table[ix].dev,
                            lte_gpio_exp_table[ix].port,
                            lte_gpio_exp_table[ix].bit,
                            &val);
    if (val == LOW) {
        return (FALSE);
    }
    return (TRUE);
}


/********************************************************************
 *
 * Function   : plug_lte_usb_deb_enable 
 * Description: Function to route USB2 signals from the EM/WP modem
 *              to the onboard USB connector
 * Inputs     : input - 0 for disable, 1 for enable 
 * Outputs    : TRUE or FALSE 
 * 
 ********************************************************************/
int plug_lte_usb_deb_enable (int input)
{
    int ix;
    
    gpio_exp_table_trav(USB_DEBUG_ENABLE, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             input);

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_w_1_disable 
 * Description: 
 * Inputs     : input - 0 for disable, 1 for enable
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_w_1_disable (int input)
{
    int ix;
    
    gpio_exp_table_trav(WDISABLE_1, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             input);

    return (PASSED);  
}


/********************************************************************
 *
 * Function   : plug_lte_w_2_disable 
 * Description: 
 * Inputs     : input - 0 for disable, 1 for enable
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_w_2_disable (int input)
{
    int ix;
    
    gpio_exp_table_trav(WDISABLE_2, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             input);

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_wp_clr_saf_pwr_remv 
 * Description: Function to clear SAFE_POWER_REMOVAL pin of WP module
 *              by assert GPIO exp. RESET# pin
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_wp_clr_saf_pwr_remv (void)
{
    int ix, iy, val = HIGH;
    
    /* Assert RESET pin to clear SAFE_PWR_REMOVAL pin */
    printf("Clearing SAFE_PWR_REMOVAL pin.\n");
    gpio_exp_table_trav(RESET, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             HIGH);
    /* Base on WP modem spec, RESET# should be toggle to high at least 32ms */
    msleep(WP_HD_RESET_H_DELAY);
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             LOW);

    /* Check if SAFE_PWR_REMOVAL is de-asserted */
    gpio_exp_table_trav(SAFE_PWR_REMOVAL, &ix); 
    for (iy = 0; iy < WP_CHK_PWR_TOUT; iy++) {
        val = HIGH;
        plug_gpio_exp_read_port(lte_gpio_exp_table[ix].dev,
                                lte_gpio_exp_table[ix].port,
                                lte_gpio_exp_table[ix].bit,
                                &val);
        if (val == LOW) {
            printf("OK\n");
            break;
        }
        msleep(PLUG_LTE_POLLING_DELAY);
        printf(".");
    }

    if (val == HIGH) {
        printf("Failed to clear SAFE_POWER_REMOVAL signal\n");
        return (FAILED);
    }
    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_em_hard_reset 
 * Description: Function to toggle GPIO exp. RESET# pin to hard
 *              reset EM modem
 * Inputs     : none
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_em_hard_reset (void)
{
    int ix, val = LOW;
    
    printf("Reset modem.\n");
    gpio_exp_table_trav(RESET, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             HIGH);
    /* Base on EM modem spec, RESET# should be toggle to high at least 3 secs */
    msleep(EM_HD_RESET_H_DELAY);
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             LOW);

    printf("Wait 25 seconds...\n");
    msleep(MODEM_HD_RESET_DELAY);

    /* Make sure the RESET# pin is released */
    plug_gpio_exp_read_port(lte_gpio_exp_table[ix].dev,
                            lte_gpio_exp_table[ix].port,
                            lte_gpio_exp_table[ix].bit,
                            &val);
    printf("RESET# pin value = %d\n", val);

    /* Get modem status to ensure modem is online */
    if (plug_lte_modem_is_online(0) != TRUE) {
        printf("Modem's not online after hard-reset\n");
        return (FAILED);
    } else {
        return (PASSED);
    }
}


/********************************************************************
 *
 * Function   : plug_lte_modem_soft_reset 
 * Description: Function to soft reset LTE modem
 * Inputs     : input - Not used
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_modem_soft_reset (int input)
{
    int stat = FAILED;

    if (plug_lte_at_run_cmd(RSSI_LTE_RESET_TEST) != PASSED) {
        printf("Pluggable LTE soft reset command transmission failed\n");
        return (FAILED);
    }

    /* Need some time to start reset, check if modem is in reset mode */
    stat = plug_lte_check_modem_rdy(FALSE);
    if (stat != PASSED) {
        printf("Modem failed to switch to reset mode.\n");
        return (FAILED);
    } else {
        printf("Modem is in reset mode.\n");
    }

    /* Check if modem is out of reset */
    if (plug_lte_check_modem_rdy(TRUE) != PASSED) {
        printf("Pluggable LTE is not ready");
        return (FAILED);
    }
    msleep(MODEM_USB_RESET_DELAY);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_chk_modem_carrier_is_match
 * Description: Function to check LTE modem carrier is matched 
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_chk_modem_carrier_is_match (void)
{
    int at_test_cmd;
    int ix;

    printf("\nCheck LTE modem carrier is matched or not ");

    /* Get modem carrier info (carrier is matched/mismatched) */
    at_test_cmd = PLUG_LTE_CHK_IMG_CARRIER_MATCH;
    /* Max time: 1 mins */
    for (ix = 0; ix < MAX_IMG_CARRIER_MATCH_TIME; ix++) {
        if (plug_lte_at_run_cmd(at_test_cmd) == PASSED) {
            break;
        }
        /* Delay 10s for check modem carrier is matching or not */
        printf("\nWait 10 seconds for modem to switch carrier.\n");
        sleep(CARRIER_MATCH_POLLING_DELAY);
    }

    if (ix == MAX_IMG_CARRIER_MATCH_TIME) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : plug_lte_set_modem_carrier
 * Description: Function to set LTE modem carrier 
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_set_modem_carrier (void)
{
    int at_test_cmd;
    int ctype;

    /* Set modem carrier has 2 steps:
       (1) Set prefer carrier.
           - AT!IMPREF="[carrier name]"
       (2) Reboot modem to set carrier type from prefer to current.
           - AT!REBOOT
     */

    printf("\nSetup LTE modem carrier...");
    /* Read controller type from cookie */
    plug_lte_get_ctype(&ctype);

    /* Step 1 */
    switch (ctype) {
    case PLUGGABLE_LTE_EM:
    case PLUGGABLE_LTE_WP7607:
    case PLUGGABLE_LTE_WP7608:
    case PLUGGABLE_LTE_WP7609:
    case PLUGGABLE_LTE_WP7610:
        at_test_cmd = PLUG_LTE_SET_IMG_GENC;
        break; 
    case PLUGGABLE_LTE_WP7601:
        at_test_cmd = PLUG_LTE_SET_IMG_VERZ;
        break;
    case PLUGGABLE_LTE_WP7603:
        at_test_cmd = PLUG_LTE_SET_IMG_ATT;
        break;
    /* CSCvs71179: WP7605 change modem carrier firmware from "Generic" 
       to "DOCOMO" in pilot build */
    case PLUGGABLE_LTE_WP7605:
        at_test_cmd = PLUG_LTE_SET_IMG_DOCOMO;
        break;
    default:
        printf("%s: Invaid controller type('%d')\n", __func__, ctype);
        return (FAILED);
    }
    if (plug_lte_at_run_cmd(at_test_cmd) != PASSED) {
        return (FAILED);
    }

    /* Step 2 */
    if (plug_lte_modem_soft_reset(0) != PASSED) {
        printf("%s: Failed to soft reboot modem\n", __func__);
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_en_auto_switch_img
 * Description: Function to enable LTE modem auto SIM-based image switching
 *              feature 
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int plug_lte_en_auto_switch_img (void)
{
    int at_test_cmd;

    printf("Restore LTE modem auto SIM-based image switching feature as default...\n");
    at_test_cmd = PLUG_LTE_EN_AUTO_SWITCH_IMG;

    if (plug_lte_at_run_cmd(at_test_cmd) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_modem_is_online
 * Description: Function to check if LTE modem is online 
 * Inputs     : input - Not used
 * Outputs    : TRUE/FLASE
 *******************************************************************************
 */
int plug_lte_modem_is_online (int input)
{
    int ret = FAILED;
    int ix;

    /* Polling modem status to see if modem is online */
    for (ix = 0; ix < MAX_RECHECK_LTE_STAT_TIME; ix++) {
        ret = plug_lte_get_modem_status(0);
        if (ret == PASSED) {
            printf("LTE modem is ready\n");
            return (TRUE);
        }
        msleep(PLUG_LTE_POLLING_DELAY);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : plug_lte_get_modem_status
 * Description: Send AT command to get LTE modem current status
 * Inputs     : input - not used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_get_modem_status (int input)
{
    int at_cmd_test;
    
    at_cmd_test = LTE_GET_MODEM_STATUS;
    
    /* Send AT command to get LTE modem status */
    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        printf("Failed to get LTE modem status\n");
        return (FAILED);
    }

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_modem_pwr_ctrl 
 * Description: Function to power on/off LTE modem
 * Inputs     : pwr_opt - 0 for power off, 1 for power on
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_modem_pwr_ctrl (int pwr_opt)
{
    int ctype;

    /* Read controller type from cookie */
    plug_lte_get_ctype(&ctype);

    if (ctype == PLUGGABLE_LTE_EM) {
        return (plug_lte_em_modem_pwr_ctrl(pwr_opt));
    } else {
        return (plug_lte_wp_modem_pwr_ctrl(pwr_opt));
    }
}

   
/********************************************************************
 *
 * Function   : plug_lte_wp_modem_pwr_ctrl 
 * Description: Function to power on/off LTE WP modem
 * Inputs     : pwr_opt - 0 for power off, 1 for power on
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_wp_modem_pwr_ctrl (int pwr_opt)
{
    int ix, iy, val;
    int at_test_cmd;
   
    if (pwr_opt == TRUE) { 
        printf("Power on Pluggable LTE modem.\n");
        gpio_exp_table_trav(MODEM_POWER_OFF, &ix); 
        plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                                 lte_gpio_exp_table[ix].port,
                                 lte_gpio_exp_table[ix].bit,
                                 pwr_opt);
        /* Based on WP76XX product spec., enumeration starts within 14.5-15.5
         * seconds */
        printf("Wait 15.5 seconds for WP modules to start enumeration.\n");
        msleep(WP_PWR_ON_DELAY);
    } else {
        /* 1. De-assert Modem Power ON pin to meet WP7605/7610 modem power off
         *    sequence */
        gpio_exp_table_trav(MODEM_POWER_OFF, &ix); 
        plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                                 lte_gpio_exp_table[ix].port,
                                 lte_gpio_exp_table[ix].bit,
                                 pwr_opt);

        /* 2. Power down modem */
        at_test_cmd = PLUG_LTE_PWR_DOWN;

        if(plug_lte_at_run_cmd(at_test_cmd) != PASSED) {
            cterr('f', 0, "Failed to power down Pluggable LTE modem");
            return (FAILED);
        }

        /* 3. Monitor SAFE_POWER_REMOVAL signal */
        printf("Monitoring SAFE_POWER_REMOVAL signal");
        gpio_exp_table_trav(SAFE_PWR_REMOVAL, &ix); 

        /* When SAFE_POWER_REMOVAL transitions from low to high,
         * return PASSED and remove module power.
         */
        for (iy = 0; iy < WP_CHK_PWR_TOUT; iy++) {
            val = LOW;
            plug_gpio_exp_read_port(lte_gpio_exp_table[ix].dev,
                                    lte_gpio_exp_table[ix].port,
                                    lte_gpio_exp_table[ix].bit,
                                    &val);
            if (val == HIGH) {
                printf("OK\n");
                break;
            }
            msleep(PLUG_LTE_POLLING_DELAY);
            printf(".");
        }

        if (val == LOW) {
            printf("The SAFE_POWER_REMOVAL signal didn't transition as"
                   " expected.\n");
            return (FAILED);
        }

        /* 4. Wait 13 ms as t_pwr_remove defined in WP76XX product spec. r5 */
        msleep(WP_PWR_REMOVE_DELAY);

        /* 5. Clear SAFE_PWR_REMOVAL */
        if (plug_lte_wp_clr_saf_pwr_remv() == FAILED) {
            return (FAILED);
        }
    }

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_em_modem_pwr_ctrl 
 * Description: Function to power on/off LTE EM modem
 * Inputs     : pwr_opt - 0 for power off, 1 for power on
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_em_modem_pwr_ctrl (int pwr_opt)
{
    int ix;
    struct timeval t_curr;
    int t_diff = 0;
   
    /* EM module needs at least 30 seconds between power-on and power-off 
     * 45 seconds is recommended by SWI(CDETS:CSCvg21284)
     */
    if ((pwr_opt == FALSE) && (is_plug_lte_em())) {
        gettimeofday(&t_curr, NULL);
        t_diff = (t_curr.tv_sec - plug_lte_em_pwr_on_t.tv_sec);
        if (t_diff < PLUG_LTE_MIN_ACTIVE_SEC) { 
            printf("Wait %d sec to power off...\n", 
                   (PLUG_LTE_MIN_ACTIVE_SEC - t_diff));
            sleep(PLUG_LTE_MIN_ACTIVE_SEC - t_diff);
        }
    }

    gpio_exp_table_trav(MODEM_POWER_OFF, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             pwr_opt);

    /* Set up delay time to meet the power on/off sequence */
    if (pwr_opt == TRUE) { 
        printf("Power on Pluggable LTE modem.\n");
        if (is_plug_lte_em()) {
            gettimeofday(&plug_lte_em_pwr_on_t, NULL);
        }
    } else {
        /* Based on LTE modem spec, the max of t_pwr_off_seq on
         * EM7455/7430 modem is 25 seconds */ 
        printf("Power off Pluggable LTE modem.\n");
        printf("Wait 25 seconds to complete power-off sequence...\n");
        msleep(MODEM_PWROFF_SEQ_DELAY);
    }
    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_wwan_led_enable 
 * Description: Function to enable WWAN_LED to be routed to SIM- and
 *              SIM1 LED 
 * Inputs     : input - 0 for disable, 1 for enable 
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_wwan_led_enable (int input)
{
    int ix;
    
    gpio_exp_table_trav(WWAN_LED_ENABLE, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             HIGH);

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_wwan_led_sim_sel 
 * Description: For WP LTE Pluggable, this function determines the
 *              mux signal to select which SIM green LED for WWAN_LED 
 * Inputs     : sim_sel - 0 for select SIM 0
 *                        1 for select SIM 1 
 * Outputs    : PASSED or FAILED
 *
 ********************************************************************/
int plug_lte_wwan_led_sim_sel (int sim_sel)
{
    int ix;
    
    gpio_exp_table_trav(WWAN_LED_SIM_SEL, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             sim_sel);
    printf("SIM card %d is routed to the modem.\n", sim_sel);

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_sim_sel 
 * Description: For WP LTE Plaggable, this function determines which
 *              SIM card to be routed to the modem 
 * Inputs     : none
 * Outputs    : none 
 *
 ********************************************************************/
int plug_lte_sim_sel (int sim_sel)
{
    int ix;
    
    gpio_exp_table_trav(SIM_SELECT, &ix); 
    plug_gpio_exp_drive_port(lte_gpio_exp_table[ix].dev,
                             lte_gpio_exp_table[ix].port,
                             lte_gpio_exp_table[ix].bit,
                             sim_sel);
    printf("SIM card %d is routed to the modem.\n", sim_sel);

    return (PASSED);
}


/********************************************************************
 *
 * Function   : plug_lte_sim_detect 
 * Description: Function to detect SIM card signal
 * Inputs     : sim_sel - 0 for SIM 0 detection
 *                        1 for SIM 1 detection
 * Outputs    : TRUE OR FALSE 
 *
 ********************************************************************/
int plug_lte_sim_detect (int sim_sel)
{
    int val = HIGH, ix;
    int gpio_func;

    if (sim_sel == SIM0) {
        gpio_func = SIM0_DETECT;
    } else {
        gpio_func = SIM1_DETECT;
    }
    gpio_exp_table_trav(gpio_func, &ix); 
    plug_gpio_exp_read_port(lte_gpio_exp_table[ix].dev,
                            lte_gpio_exp_table[ix].port,
                            lte_gpio_exp_table[ix].bit, 
                            &val);

    if (val == LOW) { 
        printf("SIM card %d is detected.\n", sim_sel);
        return (TRUE);
    } else {
        printf("SIM card %d is not detected.\n", sim_sel);
        return (FALSE);
    }
}


/*******************************************************************************
 * Function   : plug_lte_has_2_sim_slot
 * Description: Function to check whether pluggable LTE has two sim slot
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean plug_lte_has_2_sim_slot (void)
{
    int ctype;

    /* Read controller type from cookie */
    plug_lte_get_ctype(&ctype);

    /* WP7601 only has 1 sim slot since P2 build */
    if (ctype == PLUGGABLE_LTE_WP7601) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}


/*******************************************************************************
 * Function   : plug_lte_check_modem_rdy 
 * Description: Function to check whether modem is ready for opening ttyUSB to 
 *              transmit AT command
 * Inputs     : status - TRUE: expect modem is ready 
 *                       FALSE: expect modem is not ready(ex. in reset mode)
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_check_modem_rdy (int status)
{
    int ix, stat = FAILED;
    char usb_tty_dev[256];
    char usb_tty[15];

    /* Check if the usb device which is used for transmitting AT command is 
     * attached to tty device successfully */
    printf("\nCheck modem usb device status ");

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */
    for (ix = 0; ix < MAX_RETRY_TIME; ix++) {
        /* Delay 500 ms to avoid ttyUSB resource is always occupied */ 
        msleep(PLUG_LTE_CHK_TTY_STAT_DELAY);
        if (status != TRUE) {
            if (plug_lte_get_tty_devname(usb_tty_dev) != PASSED) {
                printf("'usb device is unavailable'\n");
                return (PASSED);
            }
        } else {
            if (plug_lte_get_tty_devname(usb_tty_dev) == PASSED) {
                break;
            }
        }

        printf(".");
        fflush(stdout);
    }

    if (ix == MAX_RETRY_TIME) {
        return (FAILED);
    }
    
    sprintf(usb_tty, "%s%s", USB_TTY_PATH, usb_tty_dev);
    /* Wait 500 ms before access tty device */
    msleep(TTY_ACCESS_DELAY);

    for (ix = 0; ix < USB_TTY_TOUT; ix++) {
        if (access(usb_tty, F_OK) != -1) {
            printf("'OK'\n");
            stat = PASSED;
            break;
        }
        msleep(PLUG_LTE_POLLING_DELAY);
    }

    return (stat);
}


/*******************************************************************************
 * Function   : plug_lte_has_temp_sensor
 * Description: Function to check whether temperature sensor is stuffed on pluggable LTE
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean plug_lte_has_temp_sensor (void)
{
    return (FALSE);
}


/*******************************************************************************
 * Function   : is_plug_lte_em
 * Description: Function to check whether this is pluggable LTE EM or WP
 * Inputs     : None
 * Outputs    : TRUE for EM, FALSE for otherwise
 *******************************************************************************
 */
boolean is_plug_lte_em (void)
{
    int ctype;

    /* Read controller type from cookie */
    plug_lte_get_ctype(&ctype);

    if (ctype == PLUGGABLE_LTE_EM) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}


/*******************************************************************************
 * Function   : is_plug_lte_wp
 * Description: Function to check whether this is pluggable LTE EM or WP
 * Inputs     : None
 * Outputs    : TRUE for WP, FALSE for otherwise
 *******************************************************************************
 */
boolean is_plug_lte_wp (void)
{
    int ctype, ix;
    int ctype_wp[] = {PLUGGABLE_LTE_WP7601, PLUGGABLE_LTE_WP7603, 
                      PLUGGABLE_LTE_WP7605, PLUGGABLE_LTE_WP7607,
                      PLUGGABLE_LTE_WP7608, PLUGGABLE_LTE_WP7609,
                      PLUGGABLE_LTE_WP7610};
    int ctype_wp_siz = sizeof(ctype_wp)/sizeof(int);

    /* Read controller type from cookie */
    plug_lte_get_ctype(&ctype);

    for (ix = 0; ix < ctype_wp_siz; ix++) {
        if (ctype == ctype_wp[ix]) {
            return (TRUE);
        }
    }
    return (FALSE);
}


/********************************************************************
 * Function   : plug_lte_set_ctype
 * Description: Store the controller type to global variable
 * Inputs     : ctype - Controller Type
 * Outputs    : None
 *
 ********************************************************************/
void plug_lte_set_ctype (int ctype)
{
    plug_lte_ctype = ctype;
}


/********************************************************************
 * Function   : plug_lte_get_ctype
 * Description: Return the controller type
 * Inputs     : ctype - Controller Type
 * Outputs    : None
 *
 ********************************************************************/
void plug_lte_get_ctype (int *ctype)
{
    *ctype = plug_lte_ctype;
}


/***************************************************************************
* Name: plug_lte_store_usb_devinfo
*
* Description: This function sotres the usb devices info to global variable
* 
* Input: *plug_usb2p0_devinfo - Root USB2.0 device info for pluggable module
*        *plug_usb3p0_devinfo - Root USB3.0 device info for pluggable module
*
* Example: usb device info(On Star platform): 
*          Modules support USB2.0 : usb3-1, usb3-1.1, usb3-1.2 
*          Modules support USB3.0 : usb4-1, usb4-1.1, usb4-1.2
*
* Output: None
***************************************************************************/
void plug_lte_store_usb_devinfo (char *plug_usb2p0_devinfo,
                                 char *plug_usb3p0_devinfo)
{
    if (is_plug_lte_em() == TRUE) {
        sprintf(plug_lte_usb_devinfo, plug_usb3p0_devinfo); 
    } else {
        sprintf(plug_lte_usb_devinfo, plug_usb2p0_devinfo); 
    }
}


/***************************************************************************
* Name: plug_lte_set_at_usb_devinfo
*
* Description: This function stores the usb device info which is used for
*              transmitting AT command
* 
* Input: input - unused
*
* Example: usb device info for transmitting AT cmd: 
*          Star c1101p - USB2.0 : usb3-1:1.3 / USB3.0 : usb4-1:1.3
*          Star c1109_4p -
*               (slot 1) USB2.0: usb3-1.1:1.3 / USB3.0: usb4-1.1:1.3
*               (slot 2) USB2.0: usb3-1.2:1.3 / USB3.0: usb4-1.2:1.3
*
* Output: None
***************************************************************************/
void plug_lte_set_at_usb_devinfo (input) 
{
    sprintf(plug_lte_at_usb_devinfo, "%s:%s", plug_lte_usb_devinfo, 
            USB_AT_CMD_PORT);
}


/***************************************************************************
* Name: plug_lte_get_tty_devname 
*
* Description: This function returns USB serial TTY device name which the
*              specified usb device attaches to.
* 
* Input: *tty_dev - Pointer to store which TTY device name that the specified
*                   usb device attaches to
*
* Example: ttyUSB2, ttyUSB3
*
* Output: PASSED/FAILED
***************************************************************************/
int plug_lte_get_tty_devname (char *tty_dev)
{
    /* Sanity check */
    if (tty_dev == NULL) {
        printf("%s: NULL pointer\n", __func__);
        return (FAILED);
    }

    /* Get TTY Device number from system */
    if (plug_lte_atcmd_assign_ttydev() != PASSED) {
        return (FAILED);
    }
    sprintf(tty_dev, lte_atcmd_tty_dev);
    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : plug_lte_atcmd_assign_ttydev
 * Description : Assign TTY device name to global variable
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
static int plug_lte_atcmd_assign_ttydev (void)
{
    struct dirent *dir_ent;
    DIR *dir;
    char dirname[64];
    int ix = 0;
    int ttydev_access_flag = FALSE;


    /* Search for 'ttyUSBx' under /sys/bus/usb directory */
    sprintf(dirname, "%s/%s/%s", USB_SYS_DRV_PATH, plug_lte_usb_devinfo,
                                 plug_lte_at_usb_devinfo);

    dir = opendir(dirname);

    if (dir) {
        /* CDETS# CSCvs60975. 
         * Fix timing issue - modem disconnect/reconnect sometimes will 
         * get wrong tty device (default setting tty device). */
        /* Polling 5 secs to access tty device */
        for (ix = 0; ix < MAX_GET_TTY_DEV_POLLING_TIME; ix++) {
            while ((dir_ent = readdir(dir)) != NULL) {
                if (strstr(dir_ent->d_name, "ttyUSB")) {
                    sprintf(lte_atcmd_tty_dev, "%s", dir_ent->d_name);
                    ttydev_access_flag = TRUE;
                } 
            }
            if (ttydev_access_flag == FALSE) {
                closedir(dir);
                dir = opendir(dirname);
            } else {
                break;
            }
            /* Polling delay 500ms */
            msleep(GET_TTY_DEV_POLLING_DELAY);
        }
    } else {
        closedir(dir);
        return (FAILED);
    }

    closedir(dir);
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_insmod
 * Description: To insert LTE driver for the test
 * Inputs     : input - TRUE or FALSE
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_insmod (int input)
{
    int ctype;
    char drv_path[64];
    char cmd_1[128], drv[128];

    plug_lte_host_get_modem_drv_path(drv_path);

    /* To fix CSCvp39617: host USB controller intermittently crash while
     * powering down WP7605 modem with sierra driver. Replace sierra.ko with 
     * GobiSerial.ko when testing WP7605/10 pluggable module */
    plug_lte_get_ctype(&ctype); 

    if ((ctype == PLUGGABLE_LTE_WP7605) ||
        (ctype == PLUGGABLE_LTE_WP7610)) {
        sprintf(drv, "%s", GOBISERIAL_KO);
    } else {
        sprintf(drv, "%s", SIERRA_KO);
    }

    if (input == TRUE) {
        sprintf(cmd_1, "%s %s/%s", INSMOD_CMD, drv_path, drv);
    } else {
        sprintf(cmd_1, "%s %s/%s", RMMOD_CMD, drv_path, drv);
    }

    system(cmd_1);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_force_gps_pin_val 
 * Description: Function to force GPS pin value to high or low 
 * Inputs     : slot - which slot 
 *              value - HIGH / LOW
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_force_gps_pin_val (int slot, int value)
{
    int ix, at_cmd_test;
    int gps_pin_status;
    int stat = FAILED;

    /* Set GPS DR_SYNC value */
    if (value == HIGH) {
        at_cmd_test = LTE_GPS_DR_SYNC_FORCE_HIGH;
        printf("Force GPS Dead Reckoning Synchronize signal to high.\n");
    } else {
        at_cmd_test = LTE_GPS_DR_SYNC_FORCE_LOW;
        printf("Force GPS Dead Reckoning Synchronize signal to low.\n");
    }

    if (plug_lte_at_run_cmd(at_cmd_test) != PASSED) {
        return (FAILED);
    }

    /* Polling Sirius FPGA for GPS DR_SYNC */
    printf("Polling for GPS DR_SYNC.");
    fflush(stdout);
    for (ix = 0; ix < MAX_POLLING_TIME; ix++) {
        if (plug_lte_get_gps_pin_status(slot, &gps_pin_status) != PASSED) {
            return (FAILED);
        }
        if (value == gps_pin_status) {
            printf("\nGPS dead reckoning synchronize signal is %s.\n",
                    value? "HIGH":"LOW");
            stat = PASSED;
            break;
        }
        printf(".");
        msleep(PLUG_LTE_POLLING_DELAY);
        fflush(stdout);
    }

    if (stat != PASSED) {
        printf("\nFailed to set GPS Pin value\n");
        return (FAILED);
    }
    
    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_get_gps_pin_status 
 * Description: Function to get GPS pin value 
 * Inputs     : slot - which slot 
 *              gps_pin_val - HIGH / LOW
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_get_gps_pin_status (int slot, int *gps_pin_val)
{
    int gps_sync_status_bit;
    uint reg_val;

    switch (slot) {
    case PLUG_SLOT_1:
        gps_sync_status_bit = PLUG_LTE_GPS_SYNC_STATUS_0;
        break;
    case PLUG_SLOT_2:
        gps_sync_status_bit = PLUG_LTE_GPS_SYNC_STATUS_1;
        break;
    default:
        printf("%s: Invalid slot.(slot %d)\n", __FUNCTION__, slot);
        return (FAILED);
    }

    if (plug_common_host_plug_fpga_reg_read(PLUG_FPGA_DBG_LED_ADDR_REG, 
                                            &reg_val) != PASSED) {
        printf("%s: Failed to read Pluggable FPGA Reg.\n", __FUNCTION__);
        return (FAILED);
    }

    *gps_pin_val = BIT_VAL(reg_val, gps_sync_status_bit);

    return (PASSED);
}


/*******************************************************************************
 * Function   : plug_lte_modem_shutdown 
 * Description: Function to shutdown modem. 
                Step 1. Enable "AUTO-SIM" feature to isolate Diag and IOS.
                        (In IOS, "AUTO-SIM" enable is default setting)
                Step 2. Power down modem. 
 * Inputs     : No used
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int plug_lte_modem_shutdown (void)
{
    printf("Enable AUTO-SIM feature and power down modem...\n");

    if (plug_lte_en_auto_switch_img() == FAILED) {
        cterr('f', 0, 
              "Failed to enable LTE modem auto SIM-based image switching");
    }
    if (plug_lte_modem_pwr_ctrl(FALSE) == FAILED) {
        cterr('f', 0, "Failed to soft power-off SWI modem");

        return (FAILED);
    }

    return (PASSED);
}


/*-------------------------------------------------
$Log: plug_lte_lib.c,v $
Revision 1.15  2020/03/31 01:38:00  sherliu2
Fix CSCvs60975: Timing issue - modem disconnect/reconnect sometimes will get wrong tty device

Revision 1.14  2020/01/18 07:02:07  sherliu2
Modify WP7605 test carrier firmware from Generic to DOCOMO.

Revision 1.13  2020/01/17 03:06:05  sherliu2
Add function to check pluggable modem carrier is matched before testing

Revision 1.12  2019/08/15 09:27:51  shjung
Supported WP7610 PIM

Revision 1.11  2019/06/14 05:48:11  shjung
Supported WP7605 modules

Revision 1.10  2018/11/23 09:15:07  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.9.30.2  2018/10/15 07:43:26  shjung
Re-struct for pluggable-LTE common codes

Revision 1.9.30.1  2018/10/15 06:51:05  hondwang
pluggable common code re-instruct modify code

Revision 1.9  2018/06/05 09:54:08  lucywang
Merge Star branch star-branch-c110x to main trunk

Revision 1.8  2018/05/21 08:11:29  shjung
Merged code from star-branch-c110x

Revision 1.7  2018/04/13 09:35:00  shjung

1. Fix CSCvh79986 and CSCvh79979: Added modem tty device file descriptor
   slef test to ensure communication between host and modem is good
2. Modified code based on Pluggable LTE WP7601/03 ER code review
3. Put all cterr functions to the outer file
4. Modified modem USB device enumeration timeout and GPS pin vaule polling
   timeout

Revision 1.6  2018/03/27 12:46:36  hondwang
Code modify for Star_C1101_4PLTEP_4PLTEPWX and Pluggable LTE EM7455, WP7601, WP7603 ER

Revision 1.5  2018/02/27 07:23:25  shjung
Add CDETS number in comment for LTE modem power sequence timing

Revision 1.4  2018/02/26 09:56:43  shjung
Code clean up

Revision 1.3.2.10  2018/05/21 07:40:24  shjung
Based on PRRQ code review(CSCvj53467) comment: Check return value of soft power-off function

Revision 1.3.2.9  2018/05/17 02:53:41  shjung

1. Added delay for SIM initialization while switching UIM interface
2. Clear SAFE_PWR_REMOVAL signal after powered down WP module
3. Added 15.5 seconds boot-up delay for WP module based on spec.

Revision 1.3.2.6  2018/03/23 06:53:39  shjung
Modified USB2.0 detetcion test reset and retry mechanism

Revision 1.3.2.5  2018/03/23 06:15:36  shjung
Slow down USB write speed from host to LTE modem

Revision 1.3.2.4  2018/03/22 11:32:15  shjung
Modified EM module power-off timing

Revision 1.3.2.3  2018/03/16 07:47:28  shjung
1. Correct the default value of WP SAFE_POWER_REMOVAL and implement WP modem power-off function 2. Check modem status after hard reset

Revision 1.3.2.2  2018/03/09 05:55:35  shjung
Supported WP7608/7609

Revision 1.3.2.1  2018/03/02 03:29:32  shjung
Remove debug port test from default test items and code clean up

Revision 1.3  2018/02/09 09:15:45  hondwang
Merge Star branch star-branch-c9xx to main trunk

Revision 1.2.2.3  2018/02/01 23:41:02  shjung

1. Added USB2.0 Detection Tset via AT command
2. Adjusted LTE modem power on/off timing as SWI recommanded
3. Added modem temperature reading utility and modem hard-reset utility
4. Hide SIM Slot 1 Detection Test for WP7601 due to HW changes
5. Extended delay time while checking modem usb device status to avoid tty resource is occupied
6. Added modem status check mechanism to ensure modem is ready after power-cycle
7. Added delay time in pluggable LTE modem power on/off function
8. Added WP7607 RSSI test configuration

Revision 1.2.2.2  2018/01/20 06:56:34  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.2  2018/01/20 05:01:08  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.9  2018/01/09 06:10:00  shjung
Added test criteria for the hold-up time of super caps, which are used for dying gasp feature

Revision 1.1.4.8  2017/12/13 15:14:51  shjung
Added dying gasp test for pluggable LTE-EM module

Revision 1.1.4.7  2017/12/12 13:02:57  shjung
Supported LTE-WP7607 module

Revision 1.1.4.6  2017/12/08 12:28:46  shjung
Check if usb device attaches to tty successfully before capture corresponding ttyUSB number

Revision 1.1.4.5  2017/12/06 13:23:13  shjung
Dynamically get the according ttyUSB number in case usb device attaches to different ttyUSB

Revision 1.1.4.4  2017/11/15 01:35:23  shjung
Corrected the GPIO expander bit direction

Revision 1.1.4.3  2017/10/25 04:40:50  shjung
Modified pluggable module USB interface power-on/off sequence and USB interface mode configuration

Revision 1.1.4.2  2017/08/08 07:42:13  hondwang
add pluggable LTE for star-branch-c9xx

Revision 1.1.2.2  2017/07/20 17:22:50  tirawan
Add USB 2.0 test and Debug port, and host implementation function prototype

Revision 1.1.2.1  2017/07/13 06:32:20  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.6  2017/07/07 23:53:25  shjung
Correct the SIM card detection test

Revision 1.1.2.5  2017/06/25 06:41:23  tirawan
Initialize GPIO Expander Output port before configuring its direction

Revision 1.1.2.4  2017/06/23 02:20:18  tirawan
Upload Star Second FPGA read parameter if this platform is Star with Pluggable module and correct LTE reset initialization

Revision 1.1.2.3  2017/06/22 19:27:11  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

