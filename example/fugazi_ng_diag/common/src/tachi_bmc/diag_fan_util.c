/* $Id: diag_fan_util.c,v 1.4 2016/07/29 07:35:47 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/diag_fan_util.c,v $
 *------------------------------------------------------------------
 *
 * diag_fan_util.c - Fan Tray Utility Functions
 * 
 * June 2015, Times Huang
 *
 * Copyright (c) 2015-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include "common.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "defs.h"
#include "diag_fan_util.h"
#include "diag_fpga_lib.h"
#include "platform_i2c.h"
#include "patriot_linux/apps/common_utils.h"
#include "diag_i2c_api.h"
#include "i2c_api.h"
#include "diag_temp_sensor_util.h"

int diag_fan_util(void);
static int fan_pwm_ctrl(void);
static void show_fan_rps(void);
static int change_fan_speed(void);
static int change_psu_fan_speed(void);
static int change_all_fan_speed(void);
static int diag_get_psu_fan_i2c_struct (n2g_i2c_if_t *);
static int diag_psu_fan_reg_write (uint32_t, uint16_t);

uint tach_rpm_read(int);

/* Sub Menu used for FAN utility.
 */
static submenu_xtable_t fan_util_submenu_table[] = {
    { "Fan PWM control", (type_t (*)()) fan_pwm_ctrl, 0,
    0, (type_t (*)()) 0, 0, (type_t (*)()) 0, 0 },
    { "Show single fan RPM", (type_t (*)()) show_fan_rps, 0,
    0, (type_t (*)()) 0, 0, (type_t (*)()) 0, 0 },
    { "Show all fan RPM", (type_t (*)()) show_all_fan_rpm, 0,
    0, (type_t (*)()) 0, 0, (type_t (*)()) 0, 0 },
    { "Change single fan speed", (type_t (*)()) change_fan_speed, 0,
    0, (type_t (*)()) 0, 0, (type_t (*)()) 0, 0 },
    { "Change all fan speed", (type_t (*)()) change_all_fan_speed, 0,
    0, (type_t (*)()) 0, 0, (type_t (*)()) 0, 0 },
    { "Change psu fan speed", (type_t (*)()) change_psu_fan_speed, 0,
    0, (type_t (*)()) 0, 0, (type_t (*)()) 0, 0 },
    { "Show Motherboard Temperature", (type_t(*)())diag_show_temperature_lib,   0,
	0, (type_t(*)())0, 0, (type_t(*)())0,   0},
};

#define FAN_UTIL_SUBMENU_TABLE_SIZE (sizeof(fan_util_submenu_table) / \
				       sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t fan_util_primary_items[FAN_UTIL_SUBMENU_TABLE_SIZE
        + MAX_BASE_ITEMS];
static mitem_t fan_util_secondary_items[FAN_UTIL_SUBMENU_TABLE_SIZE
        + MAX_BASE_ITEMS];

menuinfo_t fan_util_subtest_menu = { "%s Subtest Menu", 0, /* mtparam added by init_empty_menu */
(PFT) show_endnote, /* notes missing WICs in combos */
0, /* use generic prompt */
0, /* size (bumped by add_menu_item() */
fan_util_primary_items, };
menuinfo_t *fan_util_submenup = &fan_util_subtest_menu;

int diag_fan_util(void) 
{
    set_nios_mode(NIOS_DISABLE_MODE);
    build_primary_submenu(fan_util_submenu_table,
    FAN_UTIL_SUBMENU_TABLE_SIZE, "FAN Utilities", &fan_util_submenup);
    
    build_secondary_submenu(fan_util_submenu_table,
    FAN_UTIL_SUBMENU_TABLE_SIZE, fan_util_secondary_items);

    menu(fan_util_submenup, fan_util_secondary_items, '\0');
    
    set_nios_mode(NIOS_DIAG_MODE);
    return (PASSED);
}

/*********************************************************************
 *
 * Function:    fan_pwm_ctrl
 *
 * Description: Fan PWM Slope
 *              This register specifies how many steps the PWM can change
 *              each second.  A value of 0 is not allowed and will be
 *              automatically changed to a 1.
 *              The slope only applies to normal operation;
 *              in alert mode the PWM to the fan can change instantaneously.
 *              The default value of 0x14 corresponds to a 10%
 *              change in PWM/second.
 *
 * Inputs:  None.
 *
 * Outputs: PASSE/FAILED.
 *
 *********************************************************************
 */
static int fan_pwm_ctrl(void) {
    int reg_val;
    reg_val = gethex_answer("Enter PWM slope ", 0x14, 0x14, 0xFF);
    return (diag_fpga_reg_write(FAN_PWM_SLOPE, reg_val));
}
/*********************************************************************
 *
 * Function:    show_fan_rps
 *
 * Description: Display fan rps from tachometer
 *              Fan speed indicated as revolutions per second
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 *********************************************************************
 */
static void show_fan_rps(void) {
    int fan_val, speed_rpm, fan_num;
    fan_num = gethex_answer("Enter Fan number ", 0, 0x0, 0x2);
    
    switch (fan_num) {
        case 0:
            fan_val = FAN_1_TACH_RPM;
            break;
        case 1:
            fan_val = FAN_2_TACH_RPM;
            break;
        case 2:
            fan_val = FAN_3_TACH_RPM;
            break;
        default:
            printf("Unknow fan number %d\n", fan_val);
    }
    diag_fpga_reg_read(fan_val, &speed_rpm);
    speed_rpm *= RPS_TO_RPM;
    printf("fan %d rpm is %d\n", fan_num, speed_rpm);
}

static int change_all_fan_speed(void) {
    int fan_val, speed_val, ix;
    
    speed_val = getdec_answer("Enter Fan speed ", 35, 20, 100);
    speed_val *=20;
    
    for (ix=0; ix < 3; ix++) {
        switch (ix) {
            case 0:
                fan_val = FAN_1_TACH_SPEED;
                break;
            case 1:
                fan_val = FAN_2_TACH_SPEED;
                break;
            case 2:
                fan_val = FAN_3_TACH_SPEED;
                break;
            default:
                printf("Unknow fan number %d\n", fan_val);
        }
        diag_fpga_reg_write(fan_val, speed_val);
    }
    return (PASSED);
}

static int change_psu_fan_speed(void) {
    int speed_val;
    uint16_t d8[32];
    speed_val = getdec_answer("Enter psu fan speed ", 20, 20, 100);
  
    d8[0] = speed_val;
    d8[1] = 0;
    if (diag_psu_fan_reg_write(FAN_COMMAND_1, d8[0]) == FAILED) {
        return (FAILED);
    }

    return (PASSED);
}

void show_all_fan_rpm(void) {
    int ix, fan_val, speed_rpm, fan_rotation;

    diag_fpga_reg_read(ENV_FAN_STATUS, &fan_rotation);
   
    for (ix =0;ix<TACHI_FAN_NUM; ix++) {
        switch (ix) {
            case 0:
                fan_val = FAN_1_TACH_RPM;
                if ((fan_rotation & FAN1_ROTATION) != FAN1_ROTATION) {
                    printf("***********************\n");
                    printf("Alert : Fan 0 may broken\n");
                    printf("***********************\n");
                }
                break;
            case 1:
                fan_val = FAN_2_TACH_RPM;
                if ((fan_rotation & FAN2_ROTATION) != FAN2_ROTATION) {
                    printf("***********************\n");
                    printf("Alert : Fan 1 may broken\n");
                    printf("***********************\n");
                }
                break;
            case 2:
                fan_val = FAN_3_TACH_RPM;
                if ((fan_rotation & FAN3_ROTATION) != FAN3_ROTATION) {
                    printf("***********************\n");
                    printf("Alert : Fan 2 may broken\n");
                    printf("***********************\n");
                }
                break;
            default:
                printf("Unknow fan number %d\n", fan_val);
        }
        diag_fpga_reg_read(fan_val, &speed_rpm);
        speed_rpm *= RPS_TO_RPM;

        if (speed_rpm > 0){
            printf("Fan %d is enable. Running at %d RPM\n", ix, speed_rpm);
        } else {
            printf("Fan %d is disble. Running at %d RPM\n", ix, speed_rpm);
        }

        

    }
}
/*********************************************************************
 *
 * Function:    change_fan_speed
 *
 * Description: Fan speed (PWM) control utility.
 *
 * Inputs:  None.
 *
 * Outputs: PASSE/FAILED.
 *
 *********************************************************************
 */
static int change_fan_speed(void) {

    int fan_val, speed_val;
    fan_val = gethex_answer("Enter Fan number ", 0, 0x0, 0x2);

    switch (fan_val) {
        case 0:
            fan_val = FAN_1_TACH_SPEED;
            break;
        case 1:
            fan_val = FAN_2_TACH_SPEED;
            break;
        case 2:
            fan_val = FAN_3_TACH_SPEED;
            break;
        default:
            printf("Unknow fan number %d\n", fan_val);
    }

    speed_val = getdec_answer("Enter Fan speed ", 35, 20, 100);

    speed_val *=20;
    return (diag_fpga_reg_write(fan_val, speed_val));
}

uint tach_rpm_read(int fan_num) {
    int fan_tach;
    uint tach_rpm = 0xFFFF;
    switch (fan_num) {
    case 0:
        fan_tach = FAN_1_TACH_RPM;
    case 1:
        fan_tach = FAN_2_TACH_RPM;
    case 2:
        fan_tach = FAN_3_TACH_RPM;
    case 3:
        fan_tach = FAN_4_TACH_RPM;
    default:
        printf("Unknow fan number %d\n", fan_num);
    }
    diag_fpga_reg_read(fan_tach, &tach_rpm);
    return (tach_rpm);
}

#if 0
static int diag_psu_fan_reg_read (uint32_t, uint16_t *);
int diag_psu_fan_reg_read (uint32_t offset, uint16_t *data_in)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = diag_get_psu_fan_i2c_struct(&i2c_if);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)data_in;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_read(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        printf("%s: Unable to read. rc=0x%08x\n", __FUNCTION__, rc);
        return (FAILED);
    }

    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}
#endif

int diag_psu_fan_reg_write (uint32_t offset, uint16_t data_out)
{
    int rc;
    n2g_i2c_if_t i2c_if;

    rc = diag_get_psu_fan_i2c_struct(&i2c_if);

    if (rc != PASSED) {
        return (FAILED);
    }

    i2c_if.buf = (char *)&data_out;
    i2c_if.size = sizeof(uint16_t);
    i2c_if.offset = offset;

    rc = n2g_i2c_write(&i2c_if);

    if (rc != RC_I2C_OP_OK) {
        msleep(REN_I2C_PROC_TIME);
        return (FAILED);
    }

    /* I2C cycle time */
    msleep(REN_I2C_PROC_TIME);

    return (PASSED);
}


static int diag_get_psu_fan_i2c_struct (n2g_i2c_if_t *psu_fan_i2c)
{
    n2g_i2c_if_t *tmp;

    tmp = (n2g_i2c_if_t *)platform_fpga_get_n2g_i2c_if(I2C_CTRL_FOUR,
                                                       I2C_MUX_ZERO,
                                                       MB_I2C_PSU_FAN);

    if (tmp == NULL) {
        printf("%s: Failed to get PSU FAN I2C interface structure\n",
               __FUNCTION__);
        return (FAILED);
    }

    memcpy(psu_fan_i2c, tmp, sizeof(n2g_i2c_if_t));
    return (PASSED);
}
/*---------------------------------------------------------------
 $Log: diag_fan_util.c,v $
 Revision 1.4  2016/07/29 07:35:47  benchen2
 fix fan alert err

 Revision 1.3  2016/07/22 06:58:16  benchen2
 add fan alert feature

 Revision 1.2  2016/04/20 11:25:29  benchen2
 add tachi fru portion

 Revision 1.1.2.12  2016/04/01 08:01:59  benchen2
 add rtc/fan info

 Revision 1.1.2.11  2015/12/04 09:15:10  benchen2
 for fix cdets CSCux41949

 Revision 1.1.2.10  2015/11/16 07:52:32  benchen2
 add raid utility

 Revision 1.1.2.9  2015/10/15 06:23:22  benchen2
 add set_nios_mode

 Revision 1.1.2.8  2015/10/08 04:42:30  benchen2
 add fan util

 Revision 1.1.2.7  2015/10/01 12:39:29  benchen2
 correct fan util

 Revision 1.1.2.6  2015/09/30 06:49:16  benchen2
 add show rpm uti

 Revision 1.1.2.5  2015/09/24 03:01:18  benchen2
 add fan util

 Revision 1.1.2.4  2015/08/17 02:41:54  benchen2
 fix missing define

 Revision 1.1.2.3  2015/08/17 02:39:40  benchen2
 fix build err

 Revision 1.1.2.2  2015/08/16 04:45:10  benchen2
 add fan util function

 Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
 Add files for Tachi BMC project


 $Endlog$
 */

