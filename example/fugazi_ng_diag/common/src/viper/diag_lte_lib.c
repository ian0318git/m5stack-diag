 /* $Id: diag_lte_lib.c,v 1.3 2018/11/09 07:33:24 yungchen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/viper/diag_lte_lib.c,v $
 *------------------------------------------------------------------
 *
 * diag_lte_lib.c - LTE functions library
 *
 *
 * Copyright (c) 2008-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include "proto.h"
#include "common.h"
#include "types.h"
#include "nvmonvars.h"
#include "diag_lte_lib.h"
#include "dev_lte_swi.h"
#include "diag_fpga.h"
#include "diag_fpga_lib.h"
#include "diag_usb_lib.h"
#include "diag_common.h"

static void diag_lte_atcmd_assign_ttydev(void);

extern uint32 err_report(dev_object_t *, char *, uint32);

int diag_lte_swi_dev_create(dev_lte_swi_object_t *);
int diag_lte_pwr_on(int);
int diag_lte_is_usb_found(int, int);
int diag_lte_sim_detected_by_fpga(void);
void diag_lte_get_ttyusb_name(char *);
int diag_lte_dport_host_usb_detect(char *, int);
int diag_lte_modem_power_down (void);

static char lte_atcmd_tty_dev[16]={0,};

/*******************************************************************************
 *
 * Function    : diag_lte_swi_dev_create
 * Description : Function to create LTE SWI Device Object
 * Inputs      : lte_swi_obj - Pointer of LTE SWI device driver object
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_lte_swi_dev_create (dev_lte_swi_object_t *lte_swi_obj)
{
    dev_object_t *dev = (dev_object_t *)lte_swi_obj;

    /* Create common device object */
    lte_swi_dev_create(dev, (dev_error_report_t)err_report);

    if (dev == NULL) {
        return (FAILED);
    }   

    /* Attach the device */
    lte_swi_obj->base.dev_object_fvt->dev_attach(dev);

    memset(lte_swi_obj->model, 0, sizeof(lte_swi_obj->model));
    lte_get_model_name (LTE_MODEL_NAME_PATH, lte_swi_obj->model);

    /* Assign this object to have WP modem */
    if(strstr(lte_swi_obj->model, "WP7607") || strstr(lte_swi_obj->model, "WP7608")
              || strstr(lte_swi_obj->model, "WP7609")) {
        lte_swi_obj->modem_type = SWI_WP7607_08_09;
    } else {
        printf("Unknow Modem");
        return (FAILED);
    }

    lte_swi_obj->callout_fvt->get_ttyusb_dev_name = diag_lte_get_ttyusb_name;

    return (PASSED);
}


/*******************************************************************************
 *
 * Function    : diag_lte_sim_detected_by_fpga
 * Description : Return true if SIM presence is detected by FPGA
 * Inputs      : None
 * Outputs     : TRUE / FALSE 
 *
 *******************************************************************************
 */
int diag_lte_sim_detected_by_fpga (void)
{
    uint32 read_data;

    if (fpga_read_reg(FPGA_SIM_STATUS_CTL_REG, &read_data) != PASSED) {
        return (FALSE);
    }

    if (read_data & LTE_SIM_0_PRESENT_DECTECT) {
        return (TRUE);
    }

    return (FALSE);
}



/*******************************************************************************
 *
 * Function    : diag_lte_pwr_on
 * Description : Function to turn on/off LTE module
 * Inputs      : turn_on - TRUE/FALSE
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
int diag_lte_pwr_on (int turn_on)
{
    uint32 read_data;
    uint32 time_out = 0;
    int rc = PASSED;

    if (turn_on == TRUE) {
        if (fpga_read_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        read_data |= (EXT_PRI_LTE_WDIS_1_RESET);

        if (fpga_write_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        if (fpga_read_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        read_data |= LTE_MODEM_POWER_CONTROL;

        if (fpga_write_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        if (fpga_read_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {

            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        read_data |= LTE_PRI_MODEM_EN_CTL;

        if (fpga_write_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        msleep(LTE_PWR_ON_TIME);


    } else {

        /* 1. Power down modem */
        diag_lte_modem_power_down();

        /* 2. Wait safe power remove */
        read_data = 0;
        while (time_out < TIMEOUT_600) {
            if (fpga_read_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {
                printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
                return (FAILED);
            }

            if (read_data & LTE_SAFE_POWER_RM_SIG) {
                /* Write 1 to clean SAFE POWER REMOVE bit */
                read_data |= LTE_SAFE_POWER_RM_SIG;
                if (fpga_write_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
                    printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
                    return (FAILED);
                }    
                break;
            }
            time_out++;

            msleep(SLEEP_100);
        }

        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("time_out: %d\n", time_out);
        }

        if (time_out == TIMEOUT_600) {
            cterr('f', 0, "Wait safe Power Remove timeout");
            rc = FAILED;
        }


        /* 3. Clean 3.7V power control and modem power control */
        if (fpga_read_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {

            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        read_data &= ~(LTE_MODEM_POWER_CONTROL);

        if (fpga_write_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        if (fpga_read_reg(FPGA_LTE_CTL_REG, &read_data) != PASSED) {

            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }

        read_data &= ~(LTE_PRI_MODEM_EN_CTL);

        if (fpga_write_reg(FPGA_LTE_CTL_REG, read_data) != PASSED) {
            printf("FPGA read fail, reg: %x\n", FPGA_LTE_CTL_REG);
            return (FAILED);
        }


    }

    return (rc);
}


/*******************************************************************************
 * Function   : diag_lte_modem_power_down
 * Description: To power down LTE modem 
 * Inputs     : None
 * Outputs    : PASSED or FAILED
 *******************************************************************************
 */
int diag_lte_modem_power_down (void)
{
    dev_lte_swi_object_t lte_obj;
    dev_lte_swi_object_t *lte_obj_p = &lte_obj;
    int rc;

    if (diag_lte_swi_dev_create(lte_obj_p) != PASSED) {
        printf("Create SWI Dev Object Fails\n");
        return (FAILED);
    }

    rc = lte_obj_p->callin_fvt->modem_power_down((dev_object_t *)&lte_obj);

    if (rc != PASSED) {
        printf("Modem Detection fails\n");
    }

    lte_obj_p->base.dev_object_fvt->dev_destroy((dev_object_t **)&lte_obj_p);


    return (rc);
}


/*******************************************************************************
 *
 * Function    : diag_lte_is_usb_found
 * Description : Poll and check whether usb device is found in the system
 * Inputs      : poll - TRUE/FALSE
 *               timeout - timeout value in 10 msecs (poll = TRUE)
 *                         e.g. 100 for 1 sec timeout
 * Outputs     : TRUE - Found
 *               FALSE - Found
 *
 *******************************************************************************
 */
int diag_lte_is_usb_found (int poll, int timeout)
{
    char dirname[64];
    char fname[64];
    int ret;
    int directory_found = FALSE;

    /* Check if the directory exists */
    sprintf(dirname, "%s/%s", LTE_USB_SYS_DEV_PATH, LTE_USB_AT_DEV_PATH);
    do {
        ret = access(dirname, F_OK);

        if (ret != -1) {
            /* Directory is found */
            directory_found = TRUE;
            break;
        }

        if (poll == FALSE) {
            break;
        }

        msleep(10);
    } while (timeout--);

    if (directory_found == TRUE) {
        /* Assign TTY Dev to global variable */
        diag_lte_atcmd_assign_ttydev();

        /* Now check whether '/dev/ttyUSBx' is there */
        sprintf(fname, "/dev/%s", lte_atcmd_tty_dev);

        do {
            ret = access(fname, F_OK);

            if (ret != -1) {
                return (TRUE);
            }

            if (poll == FALSE) {
                break;
            }

            msleep(10);
        } while (timeout--);
    }
    
    return (FALSE);
}


/*******************************************************************************
 *
 * Function    : diag_lte_get_ttyusb_name 
 * Description : Function to provide TTY USB device path for AT command
 * Inputs      : dev_name - Pointer to buffer
 * Outputs     : PASSED / FAILED
 *
 *******************************************************************************
 */
void diag_lte_get_ttyusb_name (char *dev_name) 
{
    if (dev_name == NULL) {
        printf("%s: NULL pointer\n", __func__);
        return ;
    }

    /* Get TTY Device number from system */
    if (lte_atcmd_tty_dev[0] == 0) {
        diag_lte_atcmd_assign_ttydev();
    }
    sprintf(dev_name, "/dev/%s", lte_atcmd_tty_dev);
    
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("device name is %s\n", dev_name);
    }
}


/*******************************************************************************
 *
 * Function    : diag_lte_atcmd_assign_ttydev
 * Description : Assign TTY device name to global variable
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
static void diag_lte_atcmd_assign_ttydev (void)
{
    struct dirent *dir_ent;
    DIR *dir;
    char dirname[64];

    /* Assign to default value */
    sprintf(lte_atcmd_tty_dev, "%s", LTE_USB_TTY_DEV);

    /* Search for 'ttyUSBx' under /sys/bus/usb directory */
    sprintf(dirname, "%s/%s", LTE_USB_SYS_DEV_PATH, LTE_USB_AT_DEV_PATH);

    dir = opendir(dirname);

    if (dir) {
        while ((dir_ent = readdir(dir)) != NULL) {
            if (strstr(dir_ent->d_name, "ttyUSB")) {
                sprintf(lte_atcmd_tty_dev, "%s", dir_ent->d_name);
            }
        }
    }
    closedir(dir);
}


/*******************************************************************************
 *
 * Function    : diag_lte_dport_host_usb_detect
 * Description : This function read system usb file return Vendor ID,
 *               Device ID and Speed
 * Inputs      : None
 * Outputs     : None
 *
 *******************************************************************************
 */
int diag_lte_dport_host_usb_detect (char *vid, int speed)
{
    usb_get_info();
        
    if ((strcmp("Sierra Wireless, Incorporated", usb[2].mfg[0]) == 0) 
                && usb[2].spd == speed) {
        return (PASSED);
    } else if ((usb[3].lev == 1) && (usb[3].prnt == 1) && (usb[3].port == 1)) {
        /* LTE device still on CPU */
        return (-2);
    } else {
        return (FAILED);
    }

}

/***************************************************************************
* Name: lte_get_model_name
*
* Description: Get LTE Model Name from sysfs
*
* Input: path:sysfs LTE model name path
*
* Output: PASSED/FAILED
***************************************************************************/
int lte_get_model_name (char *path, char *model_name)
{
    int rc = 0, len=0;
    char buf[512], *p = buf, cmd[512];
    FILE *fp;
    char c;

    memset(cmd, 0, sizeof(cmd));    
    sprintf(cmd, "cat %s 2>/dev/null", path);
    /*opens a process by creating a pipe, forking, and invoking the shell.*/
    fp = popen(cmd, "r");
    if (NULL == fp) {
        printf("popen Fail! \n");
        return (FAILED);
    }

    while((c = fgetc(fp)) != EOF) {
        if(len >= 512) {
	    return (FAILED);
        }
		if(c == '\n')
			p[len++] = '\0';
		else
			p[len++] = (char)c;
    }

    /*waits for the associated process to terminate and returns the exit status of the command.*/
    rc = pclose(fp);
 
    if(-1 == rc)
    {
        printf("pclose Fail! \n");
        return (FAILED);
    }

    strcpy(model_name, buf);
    return (PASSED);
}

/**********************************************************************
 *
 * Function: diag_lte_mux_switch_util 
 *
 * Description: This function to provide utility for LTE debug port
 *
 * Input : r_opt (TRUE; Viper CPU --> LTE --> External Micro USB) 
 *                FALSE; External device --> External Micro USB --> LTE) 
 *
 * Output: PASSED - No errors encountered.
 *         FAILED - Errors encountered.
 *
 **********************************************************************
*/
int diag_lte_mux_switch_util (boolean r_opt) 
{
    uint       reg_val = 0;

    /* Read FPGA interface LTE control register. */
    if (fpga_read_reg(FPGA_LTE_CTL_REG, &reg_val) != PASSED) {
        printf("%s: Failed to read FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }

    if (r_opt == TRUE) {
        /* Set switch USB mux to Connector. */
        printf("\nPlease connect micro usb to External USB 0\n");
        reg_val |= LTE_USB_MUX_SEL_CTL;
    } else if (r_opt == FALSE) {
        /* Set switch USB mux to CPU */
        printf("\nRemove cable from External USB 0\n");
        reg_val &= (uint)(~LTE_USB_MUX_SEL_CTL);
    } else {
        printf("%s: Invalid  option(%#x).\n", __FUNCTION__, r_opt);
        return (FAILED);
    }
             
    /* USB Mux Output enable */ 
    reg_val &= (uint)(~LTE_USB_MUX_DISABLE);
    /* Write the LTE control register to the corresponding register bit. */
    if (fpga_write_reg(FPGA_LTE_CTL_REG, reg_val) != PASSED) {
        printf("%s: Failed to write FPGA reg.(0x%04X).\n",
               __FUNCTION__, FPGA_LTE_CTL_REG);
        return (FAILED);
    }
    
    return (PASSED);

}

/*-------------------------------------------------
 * $Log: diag_lte_lib.c,v $
 * Revision 1.3  2018/11/09 07:33:24  yungchen
 * Merge viper branch4 to the main trunk (CSCvn11857)
 *
 * Revision 1.2  2018/08/06 02:31:51  harrchan
 * Merge viper E2E to the main trunk (CSCvk28469)
 *
 * Revision 1.1.2.10  2018/07/03 05:38:55  harrchan
 * Follow the coding rule to clean up code
 *
 * Revision 1.1.2.9  2018/05/29 03:04:20  harrchan
 * Add LTE power on/off sequence
 *
 * Revision 1.1.2.8  2018/05/10 09:35:32  lucywang
 * Added LTE Mux Switch Utility
 *
 * Revision 1.1.2.7  2018/05/04 07:25:19  lucywang
 * Show message when FPGA MUX switch failed in LTE debug port test
 *
 * Revision 1.1.2.6  2018/05/03 09:23:25  lucywang
 * Modified LTE USB debug port test flow
 *
 * Revision 1.1.2.5  2018/04/20 02:10:50  lucywang
 * Added to support LTE WP7607/WP7608/WP7609
 *
 * Revision 1.1.2.4  2018/04/10 06:17:15  harrchan
 * Modify FPGA register address
 *
 * Revision 1.1.2.3  2018/03/31 01:55:39  harrchan
 * Fix bug of USB debug port detection test
 *
 * Revision 1.1.2.2  2018/03/26 09:21:22  harrchan
 * Support usb debug port detection test
 *
 * Revision 1.1.2.1  2018/02/27 08:06:45  harrchan
 * Initial viper application code base
 *
 *
 * $Endlog$
 *-------------------------------------------------
 */


