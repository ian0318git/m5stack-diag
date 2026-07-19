/* $Id: skye_current.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_current.c,v $
 *------------------------------------------------------------------
 * skye_current.c: Skye Current Sense function and utility.
 *
 * Dec 16 2013, Ian Chang.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "common.h"
#include "common_utils.h"
#include "defs.h"
#include "types.h"
#include "error.h"
#include "menu.h"
#include "proto.h"
#include "string.h"
#include "skye_i2c.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
int dump_all_current_util(int);
static int dump_current_reg_util(int);
static int util_current_reg_rd(int);
static int util_current_reg_wr(int);

/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int skye_current_wr(uint16_t, uint16_t *);
extern int skye_current_rd(uint16_t, uint16_t *);

/******************************************************************************
 *                             Global Variables
 ******************************************************************************/

/* Skye Current Sensor Registers */
static reg_info_t sray_current_sensor_tbl[] = {
    {"Configuration",                               CURRENT_CONF_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Shunt Voltage",                               CURRENT_SHUNT_VOL_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Bus Voltage",                                 CURRENT_BUS_VOL_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Power",                                       CURRENT_POWER_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Current",                                     CURRENT_CURRENT_REG,
     (READ_ONLY),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
    {"Calibration",                                 CURRENT_CALIBRATION_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Mask/Enable",                                 CURRENT_MASK_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Alert Limit",                                 CURRENT_ALERT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0xFFFF, 0x0000},
    {"Die ID",                                      CURRENT_ID_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                             0x0000, 0x0000},
};

/******************************************************************************
 *                                 Menus
 ******************************************************************************/ 
/*
 * current utilities SubMenu Table
 */
static submenu_xtable_t current_util_table[] = {
    {"Dump Skye Current sensor ",(PFT)dump_all_current_util,          TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Dump Current Sensor Reg.",      (PFT)dump_current_reg_util,         TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Read Current Sensor Reg.",      (PFT)util_current_reg_rd,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Write Current Sensor Reg.",     (PFT)util_current_reg_wr,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
};

#define CURRENT_UTIL_TABLE_SZ \
        (sizeof(current_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t current_util_primary_items[CURRENT_UTIL_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t current_util_secondary_items[CURRENT_UTIL_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t current_util_menu = {
    "%s Utilities SubMenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    current_util_primary_items,
};

menuinfo_t *current_util_submenup = &current_util_menu;


/*******************************************************************************
 *
 * Function   : build_current_util_menu
 * Description: Function to build Skye Current utility submenu.
 * Inputs     : num - dummy
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
build_current_util_menu (int num)
{
    char menu_title[32];
    int dummy;
    dummy = num;
    snprintf(menu_title, sizeof(menu_title), "Current");

    build_primary_submenu(current_util_table, CURRENT_UTIL_TABLE_SZ,
                          menu_title, &current_util_submenup);
    build_secondary_submenu(current_util_table, CURRENT_UTIL_TABLE_SZ,
                            current_util_secondary_items);

    /* Display Utility Menu */
    menu(current_util_submenup, current_util_secondary_items, 0);

    return (PASSED);
}


/*******************************************************************************
 *
 * Function   : dump_all_current_util
 * Description: Wrapped uility to dump all Skye current.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
dump_all_current_util (int opt)
{
    int reserved = 0, wdata = 0, sign;
    reg_info_t *reg_p = 0;
    uint16_t   rdata = 0, off = 0;
    float voltage, power, current;

    /* Setup Configuration Reg. */
    wdata = 0x0727;
    if (skye_current_wr(CURRENT_CONF_REG, (uint16_t *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%04X to current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, wdata, off);
        return (FAILED);
    }
    /* Shunt Voltage */
    reg_p = &sray_current_sensor_tbl[CURRENT_SHUNT_VOL_REG];
    if (skye_current_rd(CURRENT_SHUNT_VOL_REG, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }
    voltage = (2.5 * (int16_t)rdata) / 1000;
    printf("%15s : %.2f V\n", reg_p->name, voltage );

    /* Bus Voltage */
    reg_p = &sray_current_sensor_tbl[CURRENT_BUS_VOL_REG];
    if (skye_current_rd(CURRENT_BUS_VOL_REG, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }
    voltage = (1.25 * (rdata & 0x7FFF)) / 1000; /* lower 15bit */
    printf("%15s : %.2f V\n", reg_p->name, voltage );

    /* Setup Calibration Reg. */
    wdata = 0x0D55;
    if (skye_current_wr(CURRENT_CALIBRATION_REG, (uint16_t *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%04X to current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, wdata, off);
        return (FAILED);
    }

    /* Power */
    reg_p = &sray_current_sensor_tbl[CURRENT_POWER_REG];
    if (skye_current_rd(CURRENT_POWER_REG, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }
    power = (2.5 * (int16_t)rdata) / 1000;
    printf("%15s : %.2f W\n", reg_p->name, power );

    /* Current */
    reg_p = &sray_current_sensor_tbl[CURRENT_CURRENT_REG];
    if (skye_current_rd(CURRENT_CURRENT_REG, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }
    sign = 0x8000 & rdata;  /* msb for sign */
    reserved = 0x7FFF & rdata;
    current = (0.1 * reserved) / 1000;
    if (sign) {
        current = -1 * current;
    }
    printf("%15s :  %.2f A\n", reg_p->name, current );

    return (PASSED);

}

/*******************************************************************************
 *
 * Function   : dump_current_reg_util
 * Description: Wrapped uility to dump all current registers.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
dump_current_reg_util (int opt)
{
    int         reserved = 0, ctr = 0, total_num = 0;
    reg_info_t  *reg_p = 0;
    uint16_t    rd_val[16], rd_data = 0;

    reserved = opt;

    memset(&rd_val, 0, sizeof(rd_val));

    reg_p = &sray_current_sensor_tbl[0];
    total_num = (sizeof(sray_current_sensor_tbl) / sizeof(reg_info_t));
    
    /* Skye DIMM0 Thermal Sensor */
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        rd_data = 0;

        if (skye_current_rd(ctr, (uint16_t *)&rd_data) != PASSED) {
            printf("%s: Failed to read Current Sensor register"
                   "(offset = 0x%02X).\n", __FUNCTION__,  ctr);
            return (FAILED);
        }

        rd_val[ctr] = rd_data;
        msleep(10);
    }

    reg_p = &sray_current_sensor_tbl[0];
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        printf("%-19s (0x%02X): 0x%04X.\n",
               reg_p->name, reg_p->offset, rd_val[ctr]);
    }
    printf("\n");
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_current_reg_rd
 * Description: Wrapped uility to read specific register of
 *              Skye current Sensor.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_current_reg_rd (int opt)
{
    int        reserved = 0;
    reg_info_t *reg_p = 0;
    uint16_t   rdata = 0, off = 0;

    reserved = opt;

    off = (uint16_t)gethex_answer("Enter offset you want to read ",
                                  0x00, 0x00, 0xFF);
    if (skye_current_rd(off, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    reg_p = &sray_current_sensor_tbl[off];
    printf("%s (0x%02X): 0x%04X.\n", reg_p->name, reg_p->offset, rdata);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_current_reg_wr
 * Description: Wrapped uility to write specific register of
 *              Skye current Sensor.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_current_reg_wr (int opt)
{
    int        reserved = 0;
    reg_info_t *reg_p = 0;
    uint16_t   rdata = 0, off = 0, wdata = 0, cdata = 0;

    reserved = opt;
    off = (uint16_t)gethex_answer("Enter offset you want to write ",
                                  0x00, 0x00, 0x0F);

    reg_p = &sray_current_sensor_tbl[off];
    if (reg_p->type == READ_ONLY) {
        printf("\n\nSorry, this register is READ ONLY !!!\n\n");
        return (PASSED);
    } 

    if (skye_current_rd(off, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    wdata = (uint16_t)gethex_answer("Enter Data you want to write-in ",
                                    rdata, 0x0000, 0xFFFF);
    wdata &= (uint16_t)(reg_p->mask);

    if (skye_current_wr(off, (uint16_t *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%04X to current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, wdata, off);
        return (FAILED);
    }

    if (skye_current_rd(off, (uint16_t *)&cdata) != PASSED) {
        printf("%s: Failed to read current Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    printf("\n\nData you want to write-in to 0x%02X(Mask 0x%04X) is 0x%04X.\n",
           off, reg_p->mask, wdata);
    printf("The original value of 0x%02X is 0x%04X.\n", off, rdata);
    printf("Now the value of 0x%02X is 0x%04X.\n\n", off, cdata);

    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: skye_current.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:36:34  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:54  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * skye_current.c:
 * Revision 1.2  2014/02/27 15:01:44  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.4  2014/02/07 03:57:44  steja
 * code clean up
 *
 * Revision 1.1.2.3  2014/01/08 04:03:07  iachang
 * Display power info. in Tilera CPU Stress Test
 *
 * Revision 1.1.2.2  2013/12/16 09:35:35  iachang
 * Correct the current and voltage unit.
 *
 * Revision 1.1.2.1  2013/12/16 08:34:35  iachang
 * Support current sensor
 * Modify on-board thermal sensor
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

