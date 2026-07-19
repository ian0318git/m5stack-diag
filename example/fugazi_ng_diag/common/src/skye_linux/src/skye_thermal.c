/* $Id: skye_thermal.c,v 1.2 2015/05/25 03:59:17 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/skye_thermal.c,v $
 *------------------------------------------------------------------
 *
 * skye_thermal.c: Skye Thermal function and utility.
 *
 * Dec 06 2013, Ian Chang.
 *
 * Jul 2014 - Paul Lin(palin2) ported from Shrinkray.
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
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
#include "skye_dimm.h"


/******************************************************************************
 *                             Function protos
 ******************************************************************************/
static int dump_dimm_ts_reg_util(int);
static int util_dimm_ts_reg_rd(int);
static int util_dimm_ts_reg_wr(int);
static int util_on_board_ts_reg_rd(int);
static int util_on_board_ts_reg_wr(int);
int dump_on_board_ts_reg_util(int);
int skye_dump_temps(void);


/******************************************************************************
 *                                Externs
 ******************************************************************************/
extern int skye_on_board_thermal_wr(uint16_t, uchar *);
extern int skye_on_board_thermal_rd(uint16_t, uchar *);
boolean cpu_id;

/******************************************************************************
 *                             Global Variables
 ******************************************************************************/
static uint32_t dimm_no = UNKNOWN_DIMM_NO;

/* Skye DIMM Thermal Sensor Registers */
static reg_info_t sray_dimm_ts_reg_tbl[] = {
    {"Capabilities",                             DIMM_TS_CAP_REG_OFF,
     (READ_ONLY),
     {(uint)TWO_B_REG},                          0x0000, 0x0000},
    {"Configuration",                            DIMM_TS_CONF_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0x074F, 0x0000},
    {"High Limit",                               DIMM_TS_HIGH_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0x1FFC, 0x0000},
    {"Low Limit",                                DIMM_TS_LOW_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0x1FFC, 0x0000},
    {"TCRIT Limit",                              DIMM_TS_TCRIT_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0x1FFC, 0x0000},
    {"Ambient Temperature",                      DIMM_TS_TEMP_REG_OFF,
     (READ_ONLY),
     {(uint)TWO_B_REG},                          0x0000, 0x0000},
    {"Manufacturer ID",                          DIMM_TS_ID_REG_OFF,
     (READ_ONLY),
     {(uint)TWO_B_REG},                          0x0000, 0x0000},
    {"Device/Revision",                          DIMM_TS_REV_REG_OFF,
     (READ_ONLY),
     {(uint)TWO_B_REG},                          0x0000, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV1_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV2_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV3_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV4_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV5_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV6_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV7_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
    {"Vendor-defined",                           DIMM_TS_REV8_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS),
     {(uint)TWO_B_REG},                          0xFFFF, 0x0000},
};

/* CPU#0 On-board Thermal Thermal Sensor Registers */
static reg_info_t cpu0_ts_reg_tbl[] = {
    {"ambient temperature",                      TS_TEMP_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00,  0x00},
    {"ambient temperature (Ext.)",               TS_TEMP_EXT_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x01F, 0x00},
    {"CPU0 Tj",                                  TS_CPU0_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x01F, 0x00},
    {"CPU0 Tj (Ext.)",                           TS_CPU0_EXT_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x01F, 0x00},
    {"Status byte",                              TS_STATUS_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"Read Config Reg",                          TS_R_CONFIG_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"Write Config Reg",                         TS_W_CONFIG_REG,
     WRITE_ONLY, {(uint)ONE_B_REG},              0xFF, 0x00},
    {"Read Ambient high limit",                  TS_R_TEMP_HIGH_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"Read CPU0 Tj high limit",                  TS_R_CPU0_HIGH_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"Write Ambient high limit",                 TS_W_TEMP_HIGH_REG,
     WRITE_ONLY, {(uint)ONE_B_REG},              0xFF, 0x00},
    {"Write CPU0 Tj high limit",                 TS_W_CPU0_HIGH_REG,
     WRITE_ONLY, {(uint)ONE_B_REG},              0xFF, 0x00},
    {"CPU0 Tj Over-temperature limit",           TS_CPU0_OVER_REG,
     READ_WRITE, {(uint)ONE_B_REG},              0x00, 0x00},
    {"Manufacturer ID",                          TS_ID_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"Revision ID",                              TS_REV_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
};
/* CPU#1 On-board Thermal Thermal Sensor Registers */
static reg_info_t cpu1_ts_reg_tbl[] = {
    {"PCB hot-spot temperature",                 TS_TEMP_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"PCB hot-spot temperature (Ext.)",          TS_TEMP_EXT_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x01F, 0x00},
    {"CPU1 Tj",                                  TS_CPU1_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x01F, 0x00},
    {"CPU1 Tj (Ext.)",                           TS_CPU1_EXT_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x01F, 0x00},
    {"Status byte",                              TS_STATUS_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"Read Config Reg",                          TS_R_CONFIG_REG,
     READ_ONLY, {(uint)ONE_B_REG},               0x00, 0x00},
    {"Write Config Reg",                         TS_W_CONFIG_REG,
     WRITE_ONLY, {(uint)ONE_B_REG},              0xFF, 0x00},
    {"Read PCB hot-spot high limit",             TS_R_HOT_SPOT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"Read CPU1 Tj high limit",                  TS_R_CPU1_HIGH_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"Write PCB hot-spot high limit",            TS_W_HOT_SPOT_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"Write CPU1 Tj high limit",                 TS_W_CPU1_HIGH_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"CPU1 Tj Over-temperature limit",           TS_CPU1_OVER_REG,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"Manufacturer ID",                          TS_ID_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
    {"Revision ID",                              TS_REV_REG_OFF,
     (READ_WRITE | SAVE_RESTORE | REG_ACCESS), {(uint)ONE_B_REG},  0x00, 0x00},
};

/******************************************************************************
 *                                 Menus
 ******************************************************************************/ 
/*
 * Thermal utilities SubMenu Table
 */
static submenu_xtable_t thermal_util_table[] = {
    {"Dump Skye Temp. sensor ",       (PFT)skye_dump_temps,               TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Dump DIMM Temp. sensor Reg.",   (PFT)dump_dimm_ts_reg_util,         TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Read DIMM Temp. sensor Reg.",   (PFT)util_dimm_ts_reg_rd,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Write DIMM Temp. sensor Reg.",  (PFT)util_dimm_ts_reg_wr,           TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Dump On-board Temp. sensor Reg.", (PFT)dump_on_board_ts_reg_util,   TRUE,
     MF_CONTINUOUS,                   (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Read On-board Temp. sensor Reg.",  (PFT)util_on_board_ts_reg_rd,    TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
    {"Write On-board Temp. sensor Reg.", (PFT)util_on_board_ts_reg_wr,    TRUE,
     0,                               (type_t(*)())0,                     0,
     (type_t(*)())0,                  0},
};

#define THERMAL_UTIL_TABLE_SZ \
        (sizeof(thermal_util_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t thermal_util_primary_items[THERMAL_UTIL_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t thermal_util_secondary_items[THERMAL_UTIL_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t thermal_util_menu = {
    "%s Utilities SubMenu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)show_endnote,            /* notes missing WICs in combos */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    thermal_util_primary_items,
};

menuinfo_t *thermal_util_submenup = &thermal_util_menu;


/*******************************************************************************
 *
 * Function   : build_thermal_util_menu
 * Description: Function to build Skye THERMAL utility submenu.
 * Inputs     : num - number of thermal
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
build_thermal_util_menu (int num)
{
    char menu_title[32];

    memset(menu_title, 0 , sizeof(menu_title));
    snprintf(menu_title, sizeof(menu_title), "CPU %d THERMAL", num);

    build_primary_submenu(thermal_util_table, THERMAL_UTIL_TABLE_SZ,
                          menu_title, &thermal_util_submenup);
    build_secondary_submenu(thermal_util_table, THERMAL_UTIL_TABLE_SZ,
                            thermal_util_secondary_items);

    /* Display Utility Menu */
    menu(thermal_util_submenup, thermal_util_secondary_items, 0);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : skye_dump_temps
 * Description: Wrapped uility to dump all Skye Temperature.
 * Inputs     : None 
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_dump_temps (void)
{
    int        total_num = 0;
    uint16_t   rd_val[16], rd_data = 0;
    float      board_temp= 0, tmp = 0;

    if (cpu_id == MASTER_CPU) {
        total_num = (sizeof(cpu0_ts_reg_tbl) / sizeof(reg_info_t));
    } else {
        total_num = (sizeof(cpu1_ts_reg_tbl) / sizeof(reg_info_t));
    }

    memset(&rd_val, 0, sizeof(rd_val));
    /* DIMM0 temperature*/
    dimm_no = 0;
    if (skye_dimm_thermal_rd(dimm_no, DIMM_TS_TEMP_REG_OFF, 
        (uint16_t *)&rd_data) != PASSED) {
        cterr_db_print("%s: Failed to read DIMM%d Thermal Sensor register"
                       "(offset = 0x%02X).\n",
                       __FUNCTION__, dimm_no, DIMM_TS_TEMP_REG_OFF);
        return (FAILED);
    }
    cterr_db_print("Skye DIMM%d Temperature       : %.2f\n",
                   dimm_no, DIMM_TEMP(rd_data));

    /* DIMM1 temperature*/
    dimm_no = 1;
    if (skye_dimm_thermal_rd(dimm_no, DIMM_TS_TEMP_REG_OFF, 
        (uint16_t *)&rd_data) != PASSED) {
        cterr_db_print("%s: Failed to read DIMM%d Thermal Sensor register"
                       "(offset = 0x%02X).\n",
                       __FUNCTION__, dimm_no, DIMM_TS_TEMP_REG_OFF);
        return (FAILED);
    }
    cterr_db_print("Skye DIMM%d Temperature       : %.2f\n",
                   dimm_no, DIMM_TEMP(rd_data));

    /* CPU Ambient temperature*/
    rd_data = 0;

    if (skye_on_board_thermal_rd(TS_TEMP_REG, (uchar *)&rd_data) != PASSED) {
        cterr_db_print("%s: Failed to read on board Thermal Sensor register"
                       "(offset = 0x%02X).\n",
                       __FUNCTION__, TS_TEMP_REG);
        return (FAILED);
    }
    tmp = rd_data ;
    if (skye_on_board_thermal_rd(TS_TEMP_EXT_REG, (uchar *)&rd_data) != PASSED) {
        cterr_db_print("%s: Failed to read on board Thermal Sensor register"
                       "(offset = 0x%02X).\n",
                       __FUNCTION__, TS_TEMP_REG);
        return (FAILED);
    }
    tmp += ((rd_data & 0xE0 ) >> 5) * 0.125;
    
    if (cpu_id == MASTER_CPU) {
        cterr_db_print("Skye Ambient Temperature     : %.3f \n", tmp);
    } else {
        cterr_db_print("Skye PCB Hot-Spot Temperature: %.3f \n", tmp);
    }

    /* CPU Tj temperature*/
    if (skye_on_board_thermal_rd(TS_CPU0_REG, (uchar *)&rd_data) != PASSED) {
        cterr_db_print("%s: Failed to read on board Thermal Sensor register"
                       "(offset = 0x%02X).\n",
                       __FUNCTION__, TS_CPU0_REG);
        return (FAILED);
    }
    tmp = rd_data ;
    if (skye_on_board_thermal_rd(TS_CPU0_EXT_REG, (uchar *)&rd_data) != PASSED) {
        cterr_db_print("%s: Failed to read on board Thermal Sensor register"
                       "(offset = 0x%02X).\n",
                       __FUNCTION__, TS_TEMP_REG);
        return (FAILED);
    }
    tmp += ((rd_data & 0xE0 ) >> 5) * 0.125;
    board_temp = ACTURE_TEMP(tmp);
    cterr_db_print("Skye CPU%d Tj Temperature     : %.3f (Measured %.3f)\n",
                   cpu_id, board_temp, tmp);
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dump_dimm_ts_reg_util
 * Description: Wrapped uility to dump all Skye DIMM registers.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
dump_dimm_ts_reg_util (int opt)
{
    int         reserved = 0, ctr = 0, total_num = 0;
    reg_info_t  *reg_p = 0;
    uint16_t    rd_val[16], rd_data = 0;
    uint16_t    cpu0_temp, cpu1_temp;

    reserved = opt;

    memset(&rd_val, 0, sizeof(rd_val));

    reg_p = &sray_dimm_ts_reg_tbl[0];
    total_num = (sizeof(sray_dimm_ts_reg_tbl) / sizeof(reg_info_t));
    
    /* Skye DIMM0 Thermal Sensor */
    dimm_no = 0;
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        rd_data = 0;

        if (skye_dimm_thermal_rd(dimm_no, ctr, (uint16_t *)&rd_data) != PASSED) {
            printf("%s: Failed to read DIMM%d Thermal Sensor register"
                   "(offset = 0x%02X).\n", __FUNCTION__, dimm_no, ctr);
            return (FAILED);
        }

        rd_val[ctr] = rd_data;
        msleep(10);
    }

    printf("\nSkye DIMM%d Thermal Sensor registers Dump\n", dimm_no);
    reg_p = &sray_dimm_ts_reg_tbl[0];
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        printf("%-19s (0x%02X): 0x%04X.\n",
               reg_p->name, reg_p->offset, rd_val[ctr]);
    }
    cpu0_temp = rd_val[5] ;
    printf("\n");

    /* Skye DIMM1 Thermal Sensor */
    dimm_no = 1;
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        rd_data = 0;

        if (skye_dimm_thermal_rd(dimm_no, ctr, (uint16_t *)&rd_data) != PASSED) {
            printf("%s: Failed to read DIMM%d Thermal Sensor register"
                   "(offset = 0x%02X).\n", __FUNCTION__, dimm_no, ctr);
            return (FAILED);
        }

        rd_val[ctr] = rd_data;
        msleep(10);
    }

    printf("\nSkye DIMM%d Thermal Sensor registers Dump\n", dimm_no);
    reg_p = &sray_dimm_ts_reg_tbl[0];
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        printf("%-19s (0x%02X): 0x%04X.\n",
               reg_p->name, reg_p->offset, rd_val[ctr]);
    }
    cpu1_temp = rd_val[5];
    printf("\nSkye DIMM0 temperature : %.2f", DIMM_TEMP(cpu0_temp));
    printf("\nSkye DIMM1 temperature : %.2f\n", DIMM_TEMP(cpu1_temp));


    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_dimm_ts_reg_rd
 * Description: Wrapped uility to read specific register of
 *              Skye DIMM Thermal Sensor.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_dimm_ts_reg_rd (int opt)
{
    int        reserved = 0;
    reg_info_t *reg_p = 0;
    uint16_t   rdata = 0, off = 0;

    reserved = opt;

    off = (uint16_t)gethex_answer("Enter offset you want to read ",
                                  0x05, 0x00, 0x0F);
    if (skye_dimm_thermal_rd(dimm_no, off, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read DIMM%d Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, dimm_no, off);
        return (FAILED);
    }

    reg_p = &sray_dimm_ts_reg_tbl[off];
    printf("%s (0x%02X): 0x%04X.\n", reg_p->name, reg_p->offset, rdata);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_dimm_ts_reg_wr
 * Description: Wrapped uility to write specific register of
 *              Skye DIMM Thermal Sensor.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_dimm_ts_reg_wr (int opt)
{
    int        reserved = 0;
    reg_info_t *reg_p = 0;
    uint16_t   rdata = 0, off = 0, wdata = 0, cdata = 0;

    reserved = opt;
    dimm_no = 1;
    dimm_no = (uint16_t)gethex_answer("Enter DIMM you want to write ",
                                  0x00, 0x00, 0x01);
    off = (uint16_t)gethex_answer("Enter offset you want to write ",
                                  0x01, 0x00, 0x0F);

    reg_p = &sray_dimm_ts_reg_tbl[off];
    if (reg_p->type == READ_ONLY) {
        printf("\n\nSorry, this register is READ ONLY !!!\n\n");
        return (PASSED);
    } 

    if (skye_dimm_thermal_rd(dimm_no, off, (uint16_t *)&rdata) != PASSED) {
        printf("%s: Failed to read DIMM%d Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, dimm_no, off);
        return (FAILED);
    }

    wdata = (uint16_t)gethex_answer("Enter Data you want to write-in ",
                                    rdata, 0x0000, 0xFFFF);
    wdata &= (uint16_t)(reg_p->mask);

    if (skye_dimm_thermal_wr(dimm_no, off, (uint16_t *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%04X to DIMM%d Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, wdata, dimm_no, off);
        return (FAILED);
    }

    if (skye_dimm_thermal_rd(dimm_no, off, (uint16_t *)&cdata) != PASSED) {
        printf("%s: Failed to read DIMM%d Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, dimm_no, off);
        return (FAILED);
    }

    printf("\n\nData you want to write-in to 0x%02X(Mask 0x%04X) is 0x%04X.\n",
           off, reg_p->mask, wdata);
    printf("The original value of 0x%02X is 0x%04X.\n", off, rdata);
    printf("Now the value of 0x%02X is 0x%04X.\n\n", off, cdata);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : dump_on_board_ts_reg_util
 * Description: Wrapped uility to dump all Skye on board thermal registers.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
dump_on_board_ts_reg_util (int opt)
{
    int        reserved = 0, ctr = 0, total_num = 0;
    reg_info_t *reg_p = 0, *tmp_p;
    uint16_t   rd_val[16], rd_data = 0;

    reserved = opt;

    memset(&rd_val, 0, sizeof(rd_val));

    if (cpu_id == MASTER_CPU) {
        tmp_p = &cpu0_ts_reg_tbl[0];
        total_num = (sizeof(cpu0_ts_reg_tbl) / sizeof(reg_info_t));
    } else {
        tmp_p = &cpu1_ts_reg_tbl[0];
        total_num = (sizeof(cpu1_ts_reg_tbl) / sizeof(reg_info_t));
    }
    reg_p = tmp_p;
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        rd_data = 0;
        if (skye_on_board_thermal_rd(reg_p->offset, (uchar *)&rd_data) != PASSED) {
            printf("%s: Failed to read on board Thermal Sensor register"
                   "(offset = 0x%02X).\n", __FUNCTION__, ctr);
            return (FAILED);
        }
        rd_val[ctr] = rd_data;
        msleep(100);
    }

    reg_p = tmp_p;

    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        printf("\n%-33s (0x%02X): 0x%02X ",
               reg_p->name, reg_p->offset, rd_val[ctr]);
    }

    printf("\n");
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_on_board_ts_reg_rd
 * Description: Wrapped uility to read specific register of
 *              Skye On Board Thermal Sensor.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_on_board_ts_reg_rd (int opt)
{
    int        reserved = 0;
    uint16_t    off = 0;
    uchar       rdata = 0;
    reserved = opt;

    off = gethex_answer("Enter offset you want to read ",
                                  0x00, 0x00, 0xFF);
    if (skye_on_board_thermal_rd(off, (uchar *)&rdata) != PASSED) {
            printf("%s: Failed to read on board Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    printf("(0x%X): 0x%0X.\n", off, rdata);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : util_on_board_ts_reg_wr
 * Description: Wrapped uility to write specific register of
 *              Skye On Board Thermal Sensor.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int
util_on_board_ts_reg_wr (int opt)
{
    int        reserved = 0;
    uint16_t   rdata = 0, off = 0,rd_off = 0, cdata = 0;
    uchar      wdata = 0;
    reserved = opt;

    off = (uint16_t)gethex_answer("Enter offset you want to write ",
                                  0x00, 0x00, 0x21);

    wdata = gethex_answer("Enter Data you want to write-in ",
                                    rdata, 0x00, 0xFF);
    if (skye_on_board_thermal_wr(off, (uchar *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%0X to Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, wdata, off);
        return (FAILED);
    }

    /* The Read / Write register is different */
    switch (off) {
        case TS_W_TEMP_HIGH_REG:
            rd_off = TS_R_TEMP_HIGH_REG;
	    break;
        case TS_W_CPU0_HIGH_REG:
            rd_off = TS_R_CPU0_HIGH_REG;
	    break;
        default: /* Long uncache */
            rd_off = off;
	    break;
    }
    if (skye_on_board_thermal_rd(rd_off, (uchar *)&cdata) != PASSED) {
        printf("%s: Failed to read Thermal Sensor register"
               "(offset = 0x%02X).\n", __FUNCTION__, off);
        return (FAILED);
    }

    printf("\n\nData you want to write-in to 0x%02X is 0x%02X.\n",
           off, wdata);
    printf("The original value of 0x%02X is 0x%02X.\n", off, rdata);
    printf("Now the value of 0x%02X is 0x%02X.\n\n", rd_off, cdata);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : on_board_ts_reg_wr
 * Description: Wrapped uility to write specific register of
 *              Skye On Board Thermal Sensor.
 * Inputs     :reg - offset of register that want to write
 *             max - variable to determine that thermal threshold
 *                    will be set to high or low
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
on_board_ts_reg_wr (uint16_t reg, boolean max)
{
    uchar      wdata = 0;
    int        ctr = 0, total_num = 0;
    reg_info_t *reg_p = 0, *tmp_p;
    uint16_t   rd_data = 0;

    if (max == TRUE) {
        wdata = 0xF0;
    } else {
        wdata = 0x00;
    }    
    if (skye_on_board_thermal_wr(reg, (uchar *)&wdata) != PASSED) {
        printf("%s: Failed to write 0x%0X to Thermal Sensor register"
               "(reg = 0x%02X).\n", __FUNCTION__, wdata, reg);
        return (FAILED);
    }

    if (cpu_id == MASTER_CPU) {
        tmp_p = &cpu0_ts_reg_tbl[0];
        total_num = (sizeof(cpu0_ts_reg_tbl) / sizeof(reg_info_t));
    } else {
        tmp_p = &cpu1_ts_reg_tbl[0];
        total_num = (sizeof(cpu1_ts_reg_tbl) / sizeof(reg_info_t));
    }
    reg_p = tmp_p;
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        rd_data = 0;
        if (skye_on_board_thermal_rd(reg_p->offset, (uchar *)&rd_data) !=
                                          PASSED) {
            printf("%s: Failed to read on board Thermal Sensor register"
                   "(offset = 0x%02X).\n", __FUNCTION__, ctr);
            return (FAILED);
        }
        msleep(50);
    }
    reg_p = tmp_p;
    for (ctr = 0; ctr < total_num; ctr++, reg_p++) {
        rd_data = 0;
        if (skye_on_board_thermal_rd(reg_p->offset, (uchar *)&rd_data) != 
                                          PASSED) {
            printf("%s: Failed to read on board Thermal Sensor register"
                   "(offset = 0x%02X).\n", __FUNCTION__, ctr);
            return (FAILED);
        }
        msleep(50);
    }
    return (PASSED);
}


/*
 *------------------------------------------------------------------
 * $Log: skye_thermal.c,v $
 * Revision 1.2  2015/05/25 03:59:17  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.3  2015/05/11 13:45:46  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.2  2015/04/29 11:36:36  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------
 * Revision 1.1.2.3  2015/01/20 00:51:05  palin2
 * Updated Skye thermal value dump display.
 *
 * Revision 1.1.2.2  2014/08/28 08:03:24  palin2
 * Update Skye show all temp. and all voltage margin states utilities to
 * support enhanced error message.
 *
 * Revision 1.1.2.1  2014/07/21 01:56:56  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * skye_thermal.c:
 * Revision 1.2.8.1  2014/06/27 09:40:22  palin2
 * Fixed Shrinkray CPU ambient thermal sensor interrupt test Failed(CSCup56001).
 *
 * Revision 1.2  2014/02/27 15:01:44  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.5  2014/02/07 18:31:32  steja
 * code clean up
 *
 * Revision 1.1.2.4  2014/01/13 03:25:41  iachang
 * CSCum50313 : CPU0 thermal interrupt test
 *
 * Revision 1.1.2.3  2014/01/08 04:03:07  iachang
 * Display power info. in Tilera CPU Stress Test
 *
 * Revision 1.1.2.2  2014/01/03 08:31:35  iachang
 * Display temperature with CPU Stress Test
 *
 * Revision 1.1.2.1  2013/12/06 09:39:44  iachang
 * Move DIMM Thermal sensor to skye_thermal.c
 * Support on-board Thermal sensor
 * Convert the measure to actual temperature
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


