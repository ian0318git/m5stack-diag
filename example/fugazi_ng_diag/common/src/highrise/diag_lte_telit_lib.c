/* 
 *------------------------------------------------------------------
 *
 * diag_lte_telit_lib.c - LTE Telit Library Functions
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
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "proto.h"
#include "nvmonvars.h"
#include "gpio.h"
#include "highrise.h"
#include "diag_lte_telit_lib.h"
#include "diag_lte_test.h"
#include "diag_lte_telit_util.h"
#include "diag_lte_host_impl.h"
#include "highrise_cpld_lib.h"

static int diag_lte_telit_atcmd_assign_ttydev(void);
static int diag_lte_telit_read_modem_product_info(char *);
extern uint32 err_report(dev_object_t *, char *, uint32);

int diag_lte_telit_msku = -1;
int diag_lte_telit_current_usb_port = USB3P0;
static char lte_telit_atcmd_tty_dev[64] = {0,};
static lte_telit_usb_config_t diag_lte_telit_usb_cfg[MAX_USB_PORT];

/*******************************************************************************
 * Function   : diag_lte_telit_has_temp_sensor
 * Description: Function to check whether temperature sensor is stuffed on 
 *              Telit LTE
 * Inputs     : none
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean diag_lte_telit_has_temp_sensor (void)
{
    /* The Temp Sensor device will only be populated for proto type */
    return (TRUE);
}

/*******************************************************************************
 * Function   : diag_lte_telit_has_2_rssi_antenna
 * Description: Function to check whether LTE has #0/#1 RSSI antenna
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean diag_lte_telit_has_2_rssi_antenna (void)
{
    int msku;

    diag_lte_telit_get_modem_sku(&msku);
    if (msku == LTE_TELIT_LM960) {
        return (TRUE);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : diag_lte_telit_has_dedicated_gps_antenna
 * Description: Function to check whether LTE has dedicated gps 
 *              antenna
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean diag_lte_telit_has_dedicated_gps_antenna (void)
{
    int msku;

    diag_lte_telit_get_modem_sku(&msku);
    if (msku == LTE_TELIT_LM940 || msku == LTE_TELIT_LM960) {
        return (TRUE);
    }

    return (FALSE);
}


/*******************************************************************************
 * Function   : diag_lte_telit_get_current_usb_port
 * Description: Get the modem current USB port
 * Inputs     : uport - which USB port modem is connected to 
 * Outputs    : None
 *******************************************************************************
 */
void diag_lte_telit_get_current_usb_port (int *uport)
{
    *uport = diag_lte_telit_current_usb_port;
}

    
/*******************************************************************************
 * Function   : diag_lte_telit_set_current_usb_port
 * Description: Set the modem current USB port
 * Inputs     : uport - which USB port modem is connected to 
 * Outputs    : None
 *******************************************************************************
 */
void diag_lte_telit_set_current_usb_port (int uport)
{
    diag_lte_telit_current_usb_port =  uport;
}

    
/*******************************************************************************
 * Function   : diag_lte_telit_read_modem_product_info
 * Description: Return the modem USB device product info
 * Inputs     : product - pointer to store product info 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_read_modem_product_info (char *product)
{
    FILE *file;
    char fname[PATH_MAX];
    int uport = -1;

    /* Get the current modem USB port */
    diag_lte_telit_get_current_usb_port(&uport);

    /* Sanity check */
    sprintf(fname, "%s/%s", USB_SYS_DRV_PATH,
                            diag_lte_telit_usb_cfg[uport].usb_devinfo);
    if (access(fname, F_OK) == -1) {
        return (FAILED);
    }

    /* Get the product info */
    sprintf(fname, "%s/%s/%s", USB_SYS_DRV_PATH,
                               diag_lte_telit_usb_cfg[uport].usb_devinfo,
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
 * Function   : diag_lte_telit_set_modem_sku
 * Description: Store the modem sku to global variable
 * Inputs     : None 
 * Outputs    : None
 *******************************************************************************
 */
int diag_lte_telit_set_modem_sku (void)
{
    char product[64];

    if (diag_lte_telit_read_modem_product_info(product) != PASSED ) {
        return (FAILED);
    }

    if ((strstr(product, LTE_LM940_STR)) != NULL) {
        diag_lte_telit_msku = LTE_TELIT_LM940;
    } else if ((strstr(product,LTE_LM960_STR)) != NULL) {
        diag_lte_telit_msku = LTE_TELIT_LM960;
    } else {
        printf("Mis-matching modem sku\n");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_get_modem_sku
 * Description: Return the modem sku
 * Inputs     : msku - pointer to modem sku 
 * Outputs    : None
 *******************************************************************************
 */
void diag_lte_telit_get_modem_sku (int *msku)
{
    if (*msku == -1) {
        diag_lte_telit_set_modem_sku();
    }
    *msku = diag_lte_telit_msku;
}


/*******************************************************************************
 * Function   : diag_lte_telit_set_modem_default_feature
 * Description: Restore modem to default test setup
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_set_modem_default_feature (void)
{
    int modem_found = FALSE, modem_uport = -1;

    /* Set modem in default USB mode(super-speed mode) */
    printf("LTE modem searching...\n");
    diag_lte_telit_modem_searching(&modem_found, &modem_uport);
    
    if (modem_found != TRUE) {
        printf("Telit Modem is not detected\n");
        return (FAILED);
    } else {
        diag_lte_telit_set_current_usb_port(modem_uport);
    }

    if (modem_uport != USB3P0) {
        if (diag_lte_telit_config_modem_to_usb3p0() != PASSED) {
            printf("Telit modem is not detected\n");
            return (FAILED);
        }
        diag_lte_telit_set_current_usb_port(USB3P0);
    }

    diag_lte_telit_set_modem_sku();

    /* Set modem in default testmode(operation mode) */
    if (diag_lte_telit_set_testmode(OPERATION_MODE) != PASSED) {
        printf("Failed to set modem in operation mode\n");
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_config_modem_to_usb3p0
 * Description: Configure modem to usb3.0
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_config_modem_to_usb3p0 (void)
{
    int restore_rc = FAILED; 
    int modem_found_3p0 = FALSE;
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int waiting_time = 0;
   
    /* Create device object */
    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* Restore USB2.0 to USB3.0 */
    restore_rc = diag_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &diag_lte_telit_obj,
                                                   SUPER_SPD_USB);
    if (restore_rc != PASSED) {
        printf("Switching modem to USB mode(3.0) fails\n");
        goto __exit;
    } else {
        diag_lte_telit_set_current_usb_port(USB3P0);
    }
    
    /* Polling USB3.0 bus to see if modem is detected */
    modem_found_3p0 = diag_lte_telit_usb_is_found(USB3P0, TRUE,
                                                  PROBE_LTE_TELIT_USB_TOUT);
    if (modem_found_3p0 != TRUE) {
        /* 1. This exception happened intermittently after switching the usb-mode
         * and SW reset the modem by "AT#REBOOT". At this time, the modem device
         * cannot responding to set address during the enumeration negotiation.
         *
         * 2. The USB detection could be restored by unconditional hardware reset
         * the modem or by disable/enable USB HUB to trigger enumeration again.
         *
         * 3. There is a warning that LM960 may fail to enumerate SS_USB.
         *
         * 4. Choose to disable/enable USB HUB when modem enumeration failed at first
         * time of the mode switching as a work around to fix this issue,
         * as the Hardware Reset must be used only as an emergency according to Telit
         * HW spec. */
        printf("Re-enumberate LTE modem ...\n");
        if (diag_lte_telit_rescan(USB3P0) == FAILED) {
            printf("Re-enumberate LTE modem failed\n");
            goto __exit;
        }
        /* It will take 45 seconds for the modem to be enumerated in 55C or -5C.
         * So introduce re-polling per 5 seconds with timeout of 360 seconds.*/
        while (TRUE != diag_lte_telit_usb_is_found(USB3P0, TRUE, PROBE_LTE_TELIT_USB_TOUT)) {
            sleep (5);
            waiting_time += 5;
            printf("Modem is not found on USB3.0 bus, in %d seconds\n", waiting_time);
            if (waiting_time > 360) {
                printf("Timeout!\n");
                goto __exit;
            }
        }
    }


    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (PASSED);
    
__exit:
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (FAILED);
}

int diag_lte_telit_config_modem_to_usb2p0 (void)
{
    int restore_rc = FAILED; 
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int waiting_time = 0;
    int cnt = 0;
    int modem_found = 0;
   
    /* Create device object */
    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    /* Switch to USB2.0 */
    restore_rc = diag_lte_telit_obj_p->callin_fvt->modem_switch_usb_mode(
                                                   (dev_object_t *)
                                                   &diag_lte_telit_obj,
                                                   HIGH_SPD_USB);
    if (restore_rc != PASSED) {
        printf("Switching modem to USB mode(2.0) fails\n");
    } else {
        diag_lte_telit_set_current_usb_port(USB2P0);
    }
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    if (restore_rc != PASSED) {
        return (FAILED);
    }

    /* Polling USB2.0 bus to see if modem is detected */
    modem_found = diag_lte_telit_usb_is_found(USB2P0, TRUE, PROBE_LTE_TELIT_USB_TOUT);
    if (modem_found == TRUE) {
        return (PASSED);
    }
    /* Workaround for USB2.0 instable issue */
    while (cnt < 3) {
        /* 1. This exception happened intermittently after switching the usb-mode
         * and SW reset the modem by "AT#REBOOT". At this time, the modem device
         * cannot responding to set address during the enumeration negotiation.
         *
         * 2. The USB detection could be restored by unconditional hardware reset
         * the modem or by disable/enable USB HUB to trigger enumeration again.
         *
         * 3. There is a warning that LM960 may fail to enumerate SS_USB.
         *
         * 4. Choose to disable/enable USB HUB when modem enumeration failed at first
         * time of the mode switching as a work around to fix this issue,
         * as the Hardware Reset must be used only as an emergency according to Telit
         * HW spec. */
        printf("WARNNING: bad USB2.0 connection ???\n");
        printf("Trying FI process ...\n");
        printf("Re-enumberate LTE modem (%d) ...\n", cnt);
        if (diag_lte_telit_rescan(USB2P0) == FAILED) {
            printf("Re-enumberate LTE modem failed\n");
            return (FAILED);
        }
        /* It will take 45 seconds for the modem to be enumerated in 55C or -5C.
         * So introduce re-polling per 5 seconds with timeout of 360 seconds.*/
        while (1) {
            modem_found = diag_lte_telit_usb_is_found(USB2P0, TRUE, PROBE_LTE_TELIT_USB_TOUT);
            if (modem_found == TRUE) {
                return (PASSED);
            }
            sleep (5);
            printf("Modem is not found on USB2.0 bus, in %d seconds\n", waiting_time);
            waiting_time += 5;
            if (waiting_time > 360) {
                printf("Re-enumberation timeout(%d)!\n", cnt);
                break;
            }
        }
        /* Try another workaround: Switch to debug port and then switch back */
        printf("Switch to debug port and then switch back (%d) ...\n", cnt);
        diag_lte_telit_usb_deb_enable(OPT_ENABLE);
        sleep(10);
        diag_lte_telit_usb_deb_enable(OPT_DISABLE);
        sleep(45);
        modem_found = diag_lte_telit_usb_is_found(USB2P0, TRUE, PROBE_LTE_TELIT_USB_TOUT);
        if (modem_found == TRUE) {
            return (PASSED);
        }

        waiting_time = 0;
        cnt++;
    }
    return (FAILED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_set_testmode
 * Description: Function to set modem testmode
 * Inputs     : testmode - which mode want to set
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_set_testmode (int testmode)
{
    int rc = FAILED, ret = FALSE, cur_mode;
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    
    /* Create device object */
    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    ret = diag_lte_telit_obj_p->callin_fvt->modem_in_operation_mode(
                                           (dev_object_t *)&diag_lte_telit_obj);
    /* Current testmode */
    if (ret == TRUE) {
        cur_mode = OPERATION_MODE;
    } else {
        cur_mode = TEST_MODE;
    }
    
    /* Set testmode */
    if (testmode != cur_mode) {
        if (testmode == OPERATION_MODE) {
            rc = diag_lte_telit_obj_p->callin_fvt->modem_enable_op_mode_without_esc(
                                                   (dev_object_t *)&diag_lte_telit_obj);
            if (rc != PASSED) {
                rc = diag_lte_telit_obj_p->callin_fvt->modem_enable_op_mode(
                                                   (dev_object_t *)&diag_lte_telit_obj);
                if (rc != PASSED) {
                    printf("Failed to force modem in operation mode!\n");
                    return (FAILED);
                }
            }
        } else {
            /* testmode == TEST_MODE */
            rc = diag_lte_telit_obj_p->callin_fvt->modem_enable_test_mode(
                                                   (dev_object_t *)
                                                   &diag_lte_telit_obj);
            if (rc != PASSED) {
                printf("Failed to force modem in test mode!\n");
                return (FAILED);
            }
        }
    }

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_store_usb_devinfo
 * Description: This function stores the usb devices info to global variable
 * Inputs     : None
 * Outputs    : None
 *******************************************************************************
 */
void diag_lte_telit_store_usb_devinfo (void)
{
    int ix;

    /* usb_devinfo is the root USB device info of LTE module,
     * provided by host.
     * (e.g. 3-1 for USB2.0 mode, 4-1 for USB3.0 mode on Star C1101 platform)
     * at_usb_devinfo is one of the downstream port of modem, which is used
     * for AT command transition.(e.g. 3-1:1.4, 4-1:1.4, 3-1.1:1.4) */
    diag_lte_telit_host_get_usb_devinfo(
                            diag_lte_telit_usb_cfg[USB2P0].usb_devinfo,
                            diag_lte_telit_usb_cfg[USB3P0].usb_devinfo,
                            diag_lte_telit_usb_cfg[DEBUG_USB].usb_devinfo);

    for (ix = 0; ix <= MAX_USB_PORT - 1; ix++) {
        sprintf(diag_lte_telit_usb_cfg[ix].at_usb_devinfo, "%s:%s",
                diag_lte_telit_usb_cfg[ix].usb_devinfo, USB_AT_CMD_PORT);
    }
}


/*******************************************************************************
 * Function    : diag_lte_telit_atcmd_assign_ttydev
 * Description : Assign TTY device name to global variable
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
static int diag_lte_telit_atcmd_assign_ttydev ()
{
    /* Assign to default value */
    sprintf(lte_telit_atcmd_tty_dev, "%s", DEFAULT_LTE_USB_TTY_DEV);

    return (PASSED);
}


/*******************************************************************************
 * Name: diag_lte_telit_get_tty_devname 
 * Description: This function returns USB serial TTY device name which the
 *              specified usb device attaches to.
 * Input: *tty_dev - Pointer to store which TTY device name that the specified
 *                   usb device attaches to
 * Example: ttyUSB2, ttyUSB3
 * Output: PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_get_tty_devname (char *tty_dev)
{
    /* Sanity check */
    if (tty_dev == NULL) {
        printf("%s: NULL pointer\n", __func__);
    }

    /* Get TTY Device number from system */
    if (diag_lte_telit_atcmd_assign_ttydev() != PASSED) {
        return (FAILED);
    }
    sprintf(tty_dev, lte_telit_atcmd_tty_dev);
    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_insmod  
 * Description: To insert LTE Telit driver for the test 
 * Inputs     : input - TRUE/FALSE
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_insmod (int input)
{
    char drv_path[64];
    char fname[PATH_MAX];
    char cmd[PATH_MAX];

    diag_lte_telit_host_get_modem_drv_path(drv_path);
    
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

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_usb_is_found
 * Description: This function polls/detects modem as specified USB device
 * Inputs     : uport - which USB port modem is connected to
 *              poll - flag to introduce polling mechanism, TRUE/FALSE
 *              timeout - polling time out(*10 msec)
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
int diag_lte_telit_usb_is_found (int uport, int poll, int timeout)
{
    char dirname[PATH_MAX];
    char fname[PATH_MAX];
    char at_tty_dev[64];
    int ret = -1;
    int dir_found = FALSE;

    /* Check if the directory exists */
    sprintf(dirname, "%s/%s/%s", USB_SYS_DRV_PATH,
                                 diag_lte_telit_usb_cfg[uport].usb_devinfo,
                                 diag_lte_telit_usb_cfg[uport].at_usb_devinfo);

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
    msleep(LTE_TELIT_TTY_ATTACH_DELAY);

    if (diag_lte_telit_get_tty_devname(at_tty_dev) != PASSED) {
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
 * Function   : diag_lte_telit_modem_searching
 * Description: This function scans USB busses to seek for LTE modem
 * Inputs     : modem_found - flag to indicate LTE modem is found or not
 *              modem_uport - pointer to store which usb port that modem
 *                            connects to
 * Outputs    : None
 *******************************************************************************
 */
void diag_lte_telit_modem_searching (int *modem_found, int *modem_uport)
{
    int ix;
    int time = 3;

__retry:
    for (ix = USB2P0; ix < MAX_USB_PORT; ix++) {
        *modem_found = diag_lte_telit_usb_is_found(ix, TRUE,
                                                   PROBE_LTE_TELIT_USB_TOUT);
        if (*modem_found == TRUE) {
            *modem_uport = ix;
            break;
        }
    }

    if (TRUE != *modem_found) {
        if (time <= 0) {
            return;
        }
        time--;
        diag_lte_telit_rescan(USB3P0);
        diag_lte_telit_rescan(USB2P0);
        goto __retry;
    }
}
 

/*******************************************************************************
 * Function    : diag_lte_telit_dev_create
 * Description : Function to create LTE Telit Device Object
 * Inputs      : lte_telit_obj - Pointer of LTE Telit device driver object
 * Outputs     : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_dev_create (dev_lte_telit_object_t *diag_lte_telit_obj)
{
    int msku;
    dev_object_t *dev = (dev_object_t *)diag_lte_telit_obj;

    /* Create common device object */
    lte_telit_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }

    /* Attach the device */
    diag_lte_telit_obj->base.dev_object_fvt->dev_attach(dev);

    /* Assign modem type */
    memset(diag_lte_telit_obj->model, 0, sizeof(diag_lte_telit_obj->model));
    diag_lte_telit_get_modem_sku(&msku);
    if (msku == LTE_TELIT_LM940) {
       diag_lte_telit_obj->modem_type = TELIT_LM940;
    } else {
       diag_lte_telit_obj->modem_type = TELIT_LM960;
    } 

    diag_lte_telit_obj->callout_fvt->get_current_usb_port =
                                     diag_lte_telit_get_current_usb_port;
    diag_lte_telit_obj->callout_fvt->get_ttyusb_dev_name = 
                                     diag_lte_telit_get_tty_devname;
    return (PASSED); 
}


/*******************************************************************************
 * Function   : diag_lte_telit_usb_deb_enable 
 * Description: Function to route USB2 signals from the modem
 *              to the onboard USB connector
 * Inputs     : input - 1 for enable, 0 for disable 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int diag_lte_telit_usb_deb_enable (int input)
{
    int rc;
    if (input == OPT_ENABLE) {
        input = 1;
    } else {
        input = 0;
    }
    rc = gpio_write(USB_MUX_DEBUG_EN, input);
    if (rc == -1) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_lte_telit_w_disable1_ctrl 
 * Description: Function to control WDISABLE1# signal
 * Inputs     : value - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int diag_lte_telit_w_disable1_ctrl (int value)
{
    /* cpld reg 0x28 bit 0 modem radio on */
    unsigned long wr_buf, rd_buf;

    if(hr_cpld_reg_read_32(HR_CPLD_MODEM_CTRL, &rd_buf) == FAILED) {
        return (FAILED);
    }

    if (value == DISABLE) {
        /*disable w_disabe to write 1 */
        wr_buf = rd_buf | HR_CPLD_MODEM_CTRL_RADIO;
    } else {
        /*enable w_disabe to write 0 */
        wr_buf = rd_buf & ~(HR_CPLD_MODEM_CTRL_RADIO); 
    }

    if(hr_cpld_reg_write_32(HR_CPLD_MODEM_CTRL, wr_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_modem_pwron_pin_ctrl 
 * Description: Function to control MODEM_POWER_ON signal
 * Inputs     : input - 0 for power off, 1 for power on 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int diag_lte_telit_modem_pwron_pin_ctrl (int input)
{
    unsigned long val;

    if(hr_cpld_reg_read_32(HR_CPLD_MODEM_STATUS, &val) == FAILED) {
        return (FAILED);
    }

    /* Disable power control protection, bits[0,1] */
    val &= (~HR_CPLD_MODEM_STA_PWR_PROT_ENA);
    val |= HR_CPLD_MODEM_STA_PWR_PROT_DIS;
    if(hr_cpld_reg_write_32(HR_CPLD_MODEM_STATUS, val) == FAILED) {
        return (FAILED);
    }

    /* Set power control bit3*/
    if (input == HIGH) {
        val |= HR_CPLD_MODEM_STA_PWR_ON;
    } else {
        val &= (~HR_CPLD_MODEM_STA_PWR_ON); 
    }
    if(hr_cpld_reg_write_32(HR_CPLD_MODEM_STATUS, val) == FAILED) {
        return (FAILED);
    }

    /* Enable power control protection, bits[0,1] */
    val |= HR_CPLD_MODEM_STA_PWR_PROT_ENA;
    val &= (~HR_CPLD_MODEM_STA_PWR_PROT_DIS);
    if(hr_cpld_reg_write_32(HR_CPLD_MODEM_STATUS, val) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_modem_reset_pin_ctrl 
 * Description: Function to control WDISABLE1# signal
 * Inputs     : value - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED 
 *******************************************************************************
 */
int diag_lte_telit_modem_reset_pin_ctrl (uchar value)
{
    unsigned long wr_buf, rd_buf;

    /* check cpld reg 0x1c bit 5 modem reset protect*/
    if(hr_cpld_reg_read_32(HR_CPLD_RESET_PROTECT, &rd_buf) == FAILED) {
        return (FAILED);
    }
    /* Unlock the reset protect */
    if ((rd_buf & HR_CPLD_RESET_LOCK_MODEM) == HR_CPLD_RESET_LOCK_MODEM) {
        wr_buf = rd_buf & ~(HR_CPLD_RESET_LOCK_MODEM);

        if(hr_cpld_reg_write_32(HR_CPLD_RESET_PROTECT, wr_buf) == FAILED) {
            printf("fail to write cpld reg %x\n", HR_CPLD_RESET_PROTECT);
            return (FAILED);
        }
    }

    /* cpld reg 0x18 bit 5 modem reset*/
    if(hr_cpld_reg_read_32(HR_CPLD_RESET_CTRL, &rd_buf) == FAILED) {
        return (FAILED);
    }

    if (value == HIGH) {
        wr_buf = rd_buf | HR_CPLD_UNRESET_MODEM;
    } else {
        wr_buf = rd_buf & ~(HR_CPLD_UNRESET_MODEM); 
    }

    if(hr_cpld_reg_write_32(HR_CPLD_RESET_CTRL, wr_buf) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_toggle_led 
 * Description: Function to toggle port dirve to High/Low
 * Inputs     : which_led - EN, SIM0/1, GPS, and RSSI LEDs 
 * Outputs    : none 
 *******************************************************************************
 */
int diag_lte_telit_toggle_led (int which_led, int led_on_off)
{

    /* TBD  */

    return (PASSED);
}

  
/*******************************************************************************
 * Function   : diag_lte_telit_select_modem_serdes 
 * Description: Function to toggle GPIO exp. Host SerDes Type pin to select
 *              modem SerDes type
 * Inputs     : serdes_type
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_select_modem_serdes (int serdes_type)
{

    /* TBD  */

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_wwan_led_output_enable_ctrl 
 * Description: Function to toggle GPIO exp. WWAN_LED Enable pin to enable or
 *              disable WWAN_LED signal
 * Inputs     : value - 0 for low, 1 for high
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_wwan_led_output_enable_ctrl (int value)
{

    /* TBD  */

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_wwan_led_sim_select 
 * Description: Function to toggle GPIO exp. WWAN_LED_SIM_SEL pin to select 
 *              which SIM LED will the WWAN_LED signal be routed to.
 * Inputs     : value - 0 for SIM0, 1 for SIM1
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_wwan_led_sim_select (int value)
{

    /* TBD  */

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_gpio_sim_card_detect
 * Description: This function returns the staus of SIM detect bit on optional
 *              GPIO
 * Inputs     : which_sim - Testing SIM slot
 * Outputs    : TRUE - SIM card is detected
 *              FALSE - SIM card is not detected
 *******************************************************************************
 */
int diag_lte_telit_gpio_sim_card_detect (int which_sim)
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
 * Function   : diag_lte_telit_dump_modem_temp
 * Description: Function to return the current temperature of Telit LTE modem
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_dump_modem_temp (void)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = diag_lte_telit_obj_p->callin_fvt->modem_dump_temp((dev_object_t *)
                                                           &diag_lte_telit_obj);
    if (rc != PASSED) {
        printf("Fail to dump LTE modem temperature\n");
    }
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_telit_modem_pwr_ctrl 
 * Description: Function to power on/off LTE Telit modem 
 * Inputs     : pwr_opt - 0 for power off/1 for power on
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_modem_pwr_ctrl (int pwr_opt)
{
    int iy;
    int msku, timeout;
    unsigned long val;

    diag_lte_telit_get_modem_sku(&msku);

    if (pwr_opt == TRUE) {
        /* Power supply VBATT*/
        if (diag_lte_telit_modem_pwron_pin_ctrl(HIGH) != PASSED) {
            printf("Hard power on failed!\n");
            return (FAILED);
        }

        /* De-assert SYSTEM_RESET_N*/
        if (msku == LTE_TELIT_LM960) {
            if (diag_lte_telit_modem_reset_pin_ctrl(HIGH) != PASSED) {
                printf("De-assert SYSTEM_RESET_N failed!\n");
                return (FAILED);
            }
        }

        /* Based on LM940/960 product spec., enumeration starts within 30
         * seconds */
        printf("Wait %d seconds for modules to start enumeration ...\n",
                MODEM_LM9X0_PWR_ON_DELAY);
        sleep(MODEM_LM9X0_PWR_ON_DELAY);

    } else {

        /* 0. Check whether the shutdown indicator is set */
        if (diag_lte_telit_soft_shutdown_indicator_is_set() != TRUE) {
            /* If not, set the shutdown indicator */
            printf("Setting modem shutdown indicator ...\n");
            if (diag_lte_telit_set_shutdown_indicator() != PASSED) {
                printf("Failed to set shutdown indicator\n");
                return (FAILED);
            }
        }

        /* 1. Power down modem by using AT command*/
        if (diag_lte_telit_modem_power_down() != PASSED) {
            printf("Soft power down LTE Telit modem failed\n");
            return (FAILED);
        }

        /* 2. Monitor shutdown indicator, the maximum time to shutdown the device completely is 25 seconds*/
        for (iy = 0; iy < LTE_TELIT_SHDN_TIMEOUT; iy++) {
            timeout = TRUE;
            hr_cpld_reg_read_32(HR_CPLD_MODEM_STATUS, &val);
            if ((val & HR_CPLD_MODEM_STA_ACTIVE) == LOW) {
                timeout = FALSE;
                break;
            }
            msleep(1000);
        }

        if (timeout == TRUE) {
            printf("WARR: soft power down timeout!\n");
            /* power down should keep going even if no shutdown indicator as expected */
        }

        /* 3. If we're using soft power down, SYSTEM_RESET_N must be asserted
         *    low for LM960, W_DISABLE_N to low for LM940, to avoid modems turn
         *    on again based on product spec. 
         */
        if (msku == LTE_TELIT_LM960) {
            if (diag_lte_telit_modem_reset_pin_ctrl(LOW) != PASSED) {
                printf("ERR: Assert SYSTEM_RESET_N failed!\n");
                return (FAILED);
            }
        }
    
        if (msku == LTE_TELIT_LM940) {
            if (diag_lte_telit_w_disable1_ctrl(LOW) != PASSED) {
                printf("ERR: Assert W_DISABLE failed!\n");
                return (FAILED);
            }
        }

        /* Power off VBATT */
        if (diag_lte_telit_modem_pwron_pin_ctrl(LOW) != PASSED) {
            printf("ERR: Hard power off failed!\n");
            return (FAILED);
        }
    }

    return (PASSED);
}


/*******************************************************************************
 * Function   : diag_lte_telit_modem_power_down
 * Description: Function to power down modem
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_modem_power_down (void)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = diag_lte_telit_obj_p->callin_fvt->modem_power_down((dev_object_t *)
                                                           &diag_lte_telit_obj);

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    if (rc != PASSED) {
        printf("Modem Power Down fails\n");
    }

    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_telit_set_shutdown_indicator 
 * Description: Function to set the modem shutdown indicator 
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_set_shutdown_indicator (void)
{
    int rc = FAILED;
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = diag_lte_telit_obj_p->callin_fvt->modem_set_shdn_indicator(
                                           (dev_object_t *)&diag_lte_telit_obj);

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_telit_soft_shutdown_indicator_is_set 
 * Description: Function to check whether the modem soft shutdown indicator is
 *              set or not
 * Inputs     : None
 * Outputs    : TRUE/FALSE
 *******************************************************************************
 */
boolean diag_lte_telit_soft_shutdown_indicator_is_set (void)
{
    int rc = FALSE;
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FALSE);
    }

    rc = diag_lte_telit_obj_p->callin_fvt->modem_softshdn_indic_is_enable(
                                           (dev_object_t *)&diag_lte_telit_obj);

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    return (rc);
}


/*******************************************************************************
 * Function   :diag_lte_telit_set_modem_pwron_pin_test 
 * Description: Function to toggle GPIO exp. Modem_Power_ON pin, and to see the
 *              corresponding modem GPIO pin value is correct or not 
 * Inputs     : gpio_val - 0 for low, 1 for high 
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_set_modem_pwron_pin_test (int gpio_val)
{
    int rc = FAILED;
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;

    /* Toggle the Modem_Power_ON pin from CPLD register */
    if (diag_lte_telit_modem_pwron_pin_ctrl(gpio_val) != PASSED) {
        printf("Failed to set Modem_Power_ON pin to %s\n", gpio_val?
                                                           "HIGH":"LOW");
        return (FAILED);
    }

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = diag_lte_telit_obj_p->callin_fvt->modem_check_gpio5_stat(
                                           (dev_object_t *)&diag_lte_telit_obj,
                                           gpio_val);
    if (rc != PASSED) {
        printf("Unexpected modem GPIO pin value\n");
    }

    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);
    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_telit_hard_reset 
 * Description: Function to toggle GPIO exp. Modem Reset pin to hard reset modem
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_hard_reset (void)
{
    int rc = 0;
    rc = diag_lte_telit_modem_reset();
    printf("Waiting %d seconds for LTE modem to get into the active state ...\n",
           TELIT_LTE_HARD_RST_DELAY);
    sleep(TELIT_LTE_HARD_RST_DELAY);
    return (rc);
}

/*******************************************************************************
 * Function   : diag_lte_telit_modem_reset 
 * Description: Function to toggle CPLD register reset control to hard reset modem
 * Inputs     : none
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_modem_reset (void)
{
    int msku;

    printf("Reset LTE modem ...\n");
    /* Release RESET# */
    if (diag_lte_telit_modem_reset_pin_ctrl(LOW) != PASSED) {
        return (FAILED);
    }

    diag_lte_telit_get_modem_sku(&msku);
    /* Base on LM960 modem spec, RESET# should be toggle to high at least
     * 1 sec */
    if (msku == LTE_TELIT_LM960) {
        msleep(LM960_RST_ASSERTION);
    } else {
        /* Base on LM940 modem spec, RESET# should be toggle to high between
         * 100ms~150ms */
        msleep(LM940_RST_ASSERTION);
    }

    if (diag_lte_telit_modem_reset_pin_ctrl(HIGH) != PASSED) {
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 * Function   : diag_lte_telit_soft_reboot 
 * Description: Function to soft reboot LTE modem via AT command
 * Inputs     : reboot_uport - which USB port modem will be connected to
 *                             after re-boot
 * Outputs    : PASSED/FAILED
 *******************************************************************************
 */
int diag_lte_telit_soft_reboot (int reboot_uport)
{
    dev_lte_telit_object_t diag_lte_telit_obj;
    dev_lte_telit_object_t *diag_lte_telit_obj_p = &diag_lte_telit_obj;
    int rc = FAILED;

    if (diag_lte_telit_dev_create(diag_lte_telit_obj_p) != PASSED) {
        printf("Create Telit Dev Object Fails\n");
        return (FAILED);
    }

    rc = diag_lte_telit_obj_p->callin_fvt->modem_reboot((dev_object_t *)
                                                        &diag_lte_telit_obj);
    if (rc != PASSED) {
        printf("Modem soft reboot fails\n");
        return (FAILED);
    }
    diag_lte_telit_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                           &diag_lte_telit_obj);

    if (diag_lte_telit_usb_is_found(reboot_uport, TRUE, PROBE_LTE_TELIT_TOUT)
                                    != TRUE) {
       printf("Modem is not detected.\n");
       return (FAILED);
    }

    return (PASSED);
}

int diag_lte_telit_rescan (int modem_uport)
{
    int fd = 0;
    char dev_file[PATH_MAX];

    if (USB3P0 == modem_uport) {
        sprintf(dev_file, "/sys/bus/usb/devices/usb2/authorized");
    } else {
        sprintf(dev_file, "/sys/bus/usb/devices/usb1/authorized");
    }

    fd = open(dev_file, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open %s, err(%s, %d)!\n", dev_file, strerror(errno), errno);
        return FAILED;
    }

    /* Disable root hub */
    if (-1 == write(fd, "0", 1)) {
        fprintf(stderr, "Failed to disable usb hub, err(%s, %d)!\n", strerror(errno), errno);
        return (-1);
    }

    sleep(3);
    
    /* Enable root hub */
    if (-1 == write(fd, "1", 1)) {
        fprintf(stderr, "Failed to disable usb hub, err(%s, %d)!\n", strerror(errno), errno);
        return (-1);
    }

    close(fd);
    return PASSED;
}
/*------------------------------------------------------------------
$Log: diag_lte_telit_lib.c,v $
Revision 1.2  2021/01/25 09:20:59  markzha
Sync RDT issues fixing and optimize compiling for Highrise

Revision 1.1  2020/08/19 09:49:34  markzha
*** empty log message ***

$Endlog$
*/
