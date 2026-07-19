/* $Id: usb_dongle_lte_swi_lib.c,v 1.2 2019/06/14 09:59:36 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/usb_dongle/usb_dongle_lte_swi/usb_dongle_lte_swi_lib.c,v $
 *------------------------------------------------------------------
 *
 * usb_dongle_lte_swi_lib.c - USB Dongle LTE Library Functions 
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
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include "error.h"
#include "common.h"
#include "types.h"
#include "menu.h"
#include "proto.h"
#include "common_utils.h"
#include "dev_lte_swi.h"
#include "nvmonvars.h"
#include "usb_dongle_common_test.h"
#include "usb_dongle_lte_swi_test.h"
#include "usb_dongle_lte_swi_lib.h"

/* Prototype */
boolean usb_dongle_lte_swi_usb_detect(char *, int, int);
int usb_dongle_lte_swi_modem_pwr_ctrl(int);
int usb_dongle_lte_wi_nsmod(int);
int usb_dongle_lte_swi_dev_create(dev_lte_swi_object_t *);
void usb_dongle_lte_swi_set_devname(char *);
void usb_dongle_lte_swi_get_devname(char *);
void usb_dongle_lte_swi_set_at_devinfo(char *);
void usb_dongle_lte_swi_get_ttyusb_name(char *);
static int usb_dongle_lte_swi_get_vid_pid_speed(char *, int *, int *, int *);
static void usb_dongle_lte_swi_atcmd_assign_ttydev(void);
extern uint32 err_report(dev_object_t *, char *, uint32);

/* Global */
char udongle_lte_swi_devname[64] = {0,}; //Sierra Wireless WP7607
char udongle_lte_swi_at_devinfo[32] = {0,}; //1-1:1.3
char udongle_lte_swi_atcmd_tty_dev[32] = {0,}; //ttyUSB2

extern int modem_found;

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_dev_create
 * Description: Function to create LTE SWI Device Object
 * Inputs:      lte_swi_obj - Pointer of LTE SWI device driver object
 * Outputs:     PASSED / FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_lte_swi_dev_create (dev_lte_swi_object_t *ud_lte_swi_obj)
{
    dev_object_t *dev = (dev_object_t *)ud_lte_swi_obj;

    /* Create common device object */
    lte_swi_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }   

    /* Attach the device */
    ud_lte_swi_obj->base.dev_object_fvt->dev_attach(dev);

    memset(ud_lte_swi_obj->model, 0, sizeof(ud_lte_swi_obj->model));
    sprintf(ud_lte_swi_obj->model, "%s", udongle_lte_swi_devname);

    /* Assign this object to have WP modem */
    if(strstr(ud_lte_swi_obj->model, "WP7601") || 
       strstr(ud_lte_swi_obj->model, "WP7603") ||
       strstr(ud_lte_swi_obj->model, "WP7610")) {
        ud_lte_swi_obj->modem_type = SWI_WP7601_03;
    } else {
        ud_lte_swi_obj->modem_type = SWI_WP760X_B8;
    }

    ud_lte_swi_obj->callout_fvt->get_ttyusb_dev_name = 
                                 usb_dongle_lte_swi_get_ttyusb_name;

    return (PASSED);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_set_devname
 * Description: Store the LTE product name to global variable
 *              e.g. Sierra Wireless WP7607
 * Inputs:      devname - LTE device name
 * Outputs:     None
 *------------------------------------------------------------------------------
 */
void usb_dongle_lte_swi_set_devname (char *devname)
{
    sprintf(udongle_lte_swi_devname, "%s", (char *)devname);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_get_devname
 * Description: Return the LTE product name to global variable
 *              e.g. Sierra Wireless WP7607
 * Inputs:      devname - LTE device name
 * Outputs:     None
 *------------------------------------------------------------------------------
 */
void usb_dongle_lte_swi_get_devname (char *devname)
{
    sprintf((char *)devname, "%s", udongle_lte_swi_devname);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_usb_detect
 * Description: Enumerates USB and detects USB device through host USB 
 *              by given vendor ID, product ID and speed
 * Inputs:      usb_devinfo - USB device info(e.g. 3-1, 4-1)
 *              vid - Vendor ID
 *              speed - 480 (USB2.0)
 * Outputs:     TRUE / FALSE 
 *------------------------------------------------------------------------------
 */
boolean usb_dongle_lte_swi_usb_detect (char *usb_devinfo, int vid, int speed)
{
    int dev_vid, dev_pid, dev_speed;

    if (usb_dongle_lte_swi_get_vid_pid_speed(usb_devinfo, &dev_vid, &dev_pid,
                                             &dev_speed) == FAILED) {
        return (FALSE);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Vendor ID = %#x, Device ID = %#x, Speed = %#x\n", dev_vid,
                dev_pid, dev_speed);
    }

    if ((vid == dev_vid) && (speed == dev_speed)) {
        return (TRUE);
    }

    return (FALSE);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_usb_get_vid_pid_speed
 * Description: This function reads from system USB file and return
 *              vendor ID, product ID and speed
 * Inputs:      usb_path - USB Path, e.g. 1-1, 3-1, 4-1
 *              *vid, *pid, *speed
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
static int usb_dongle_lte_swi_get_vid_pid_speed (char *usb_path, int *vid,
                                                 int *pid, int *speed)
{
    FILE *file;
    char fname[64];

    /* Check if the file exists */
    sprintf(fname, "%s/%s", LTE_USB_SYS_DRV_PATH, usb_path);
    if (access(fname, F_OK) == -1) {
        return (FAILED);
    }

    /* Get the Vendor ID */
    sprintf(fname, "%s/%s/%s", LTE_USB_SYS_DRV_PATH, usb_path,
            USB_SYS_VID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%x", vid);

    fclose(file);

    /* Get the Product ID */
    sprintf(fname, "%s/%s/%s", LTE_USB_SYS_DRV_PATH, usb_path,
            USB_SYS_PID_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%x", pid);

    fclose(file);

    /* Get the Speed */ 
    sprintf(fname, "%s/%s/%s", LTE_USB_SYS_DRV_PATH, usb_path,
            USB_SYS_SPEED_FILE);
    file = fopen(fname, "rb");
    if (file == NULL) {
        return (FAILED);
    }

    fscanf(file, "%d", speed);

    fclose(file);

    return (PASSED); 
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_set_at_devinfo
 * Description: Function to set the usb device info which is used for
 *              transmitting AT command
 * Inputs:      usb_devinfo - USB device info (e.g. 3-1, 4-1)
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
void usb_dongle_lte_swi_set_at_devinfo (char *usb_devinfo)
{
    sprintf(udongle_lte_swi_at_devinfo, "%s:%s", usb_devinfo,
            LTE_USB_AT_CMD_PORT);
}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_modem_pwr_ctrl
 * Description: Function to power on/off LTE modem
 * Inputs:      pwr_opt - 0 for power off, 1 for power on
 * Outputs:     PASSED/FAILED
 *------------------------------------------------------------------------------
 */
int usb_dongle_lte_swi_modem_pwr_ctrl (int pwr_opt)
{
    int rc = FAILED;

    if (pwr_opt == TRUE) {
        printf("Power on USB Dongle LTE modem.\n");
    } else {
        dev_lte_swi_object_t ud_lte_obj;
        dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
        if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
            printf("Create SWI Dev Object Fails\n");
            return (FAILED);
        }
        rc = ud_lte_obj_p->callin_fvt->modem_power_down
                                       ((dev_object_t *)&ud_lte_obj);
        /* Due to there's no way to monitor SAFE_PWR_REMOVE signal on USB dongle
         * LTE module, adding delay to make sure the power-off sequence of 
         * module is completed.
         * According to WP76xx product spec., max t_pwr_off_sqe is 6 seconds */
        msleep(WP_MAX_PWR_OFF_DELAY);

        if (rc != PASSED) {
            printf("Failed to power off LTE modem\n");
            return (FAILED);
        }
        usb_dongle_lte_swi_insmod(FALSE);
        modem_found = FALSE;
    }
    printf("Power off LTE modem successfully\n");    
    return (PASSED);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_insmod
 * Description: To insert LTE driver for the test
 * Inputs:      TRUE/FALSE
 * Outputs:     None 
 *------------------------------------------------------------------------------
 */
void usb_dongle_lte_swi_insmod (int input)
{
    DIR *dir = opendir (LTE_USB_SWI_DRV_PATH);
   
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        if (input == FALSE) {
            system(RMMOD_SWI_TEST_KO_1);
            system(RMMOD_SWI_TEST_KO_2);
        }
    } else if ((ENOENT == errno) && (input == TRUE)) {
        system(INSMOD_SWI_TEST_KO_1);
        system(INSMOD_SWI_TEST_KO_2);
    } else {
        printf ("Module file doesn't exist and cannot be removed...\n"); 
    }

}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_atcmd_assign_ttydev
 * Description: Assign TTY device name to global variable
 * Inputs:      None
 * Outputs:     None
 *------------------------------------------------------------------------------
 */
static void usb_dongle_lte_swi_atcmd_assign_ttydev (void)
{
    /* Assign to default value */
    sprintf(udongle_lte_swi_atcmd_tty_dev, "%s", UDONGLE_LTE_SWI_TTY_DEV2);

}


/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_get_ttyusb_name 
 * Description: Function to provide TTY USB device path for AT command
 * Inputs:      dev_name - Pointer to buffer
 * Outputs:     PASSED / FAILED
 *------------------------------------------------------------------------------
 */
void usb_dongle_lte_swi_get_ttyusb_name (char *dev_name) 
{
    if (dev_name == NULL) {
        printf("%s: NULL pointer\n", __func__);
        return ;
    }

    /* Get TTY Device number from system */
    if (udongle_lte_swi_atcmd_tty_dev[0] == 0) {
        usb_dongle_lte_swi_atcmd_assign_ttydev();
    }
    sprintf(dev_name, "/%s/%s", DEV_PATH,  udongle_lte_swi_atcmd_tty_dev);
}


/*******************************************************************************
 * Function   : usb_dongle_lte_swi_run_at_cmd
 * Description: To execute AT command for USB dongle LTE
 * Inputs     : input - Not used 
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int usb_dongle_lte_swi_run_at_cmd (int input)
{
    const int maxlen = 128;
    char cmd[maxlen];
    char usb_tty_dev[256];
    char usb_tty[15];

    printf("\n\n ### NOTE: Type CTRL-x "
                              "to switch back to host's console\n\n");
    fflush(stdout);
    fflush(stderr);
    msleep(AT_COMMAND_UTIL_DELAY); 

    /* Dynamically get the corresponding ttyUSB number in case usb device
     * attaches to different ttyUSB 
     */
    usb_dongle_lte_swi_get_ttyusb_name(usb_tty_dev); 
    
    sprintf(usb_tty, "%s", usb_tty_dev);
    snprintf(cmd, maxlen-1, "microcom %s", usb_tty);

    system(cmd);

    return (PASSED);
}

/*------------------------------------------------------------------------------
 * Function:    usb_dongle_lte_swi_poll_tty_symlink 
 * Description: To polling tty symbolic link by checking existance. 
 * Inputs:      None 
 * Outputs:     PASSED / FAILED 
 *------------------------------------------------------------------------------
 */
int usb_dongle_lte_swi_poll_tty_symlink (void)
{
    struct timeval t_curr;
    struct timeval swi_at_access_tty_start_t;
    int t_diff = 0;
    int ret = FAILED;
    char tty_usb_name[64];
    dev_lte_swi_object_t ud_lte_obj;
    dev_lte_swi_object_t *ud_lte_obj_p = &ud_lte_obj;
    
    if (usb_dongle_lte_swi_dev_create(ud_lte_obj_p) != PASSED) {
        printf("Create SWI Dev Object Fails\n");
        return (FAILED);
    }
    /* Get TTY USB Device Name/Path */
    ud_lte_obj_p->callout_fvt->get_ttyusb_dev_name(tty_usb_name);

    gettimeofday(&swi_at_access_tty_start_t, NULL);
    while (t_diff < AT_POLL_SEC) { 
        gettimeofday(&t_curr, NULL);
        t_diff = (t_curr.tv_sec - swi_at_access_tty_start_t.tv_sec);
        /* Check if the file exists  */
        if (access (tty_usb_name, F_OK) == -1) {
            ret = FAILED;
        } else {
            ret = PASSED;
            break;
        }
        sleep(DELAY_1_SEC);
    }
    
    if (ret == FAILED) {
        cterr ('f', 0, "Can't access tty device");
    }

    ud_lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)
                                                   &ud_lte_obj_p);
    return (ret);
}


/*-------------------------------------------------
 * $Log: usb_dongle_lte_swi_lib.c,v $
 * Revision 1.2  2019/06/14 09:59:36  steja
 * Supported Cooper usb dongle LTE
 *
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */
