/* $Id: platform_sensor.c,v 1.2 2021/06/02 02:56:22 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_sensor.c,v $
 *------------------------------------------------------------------
 * platform_sensor.c
 *
 * Description: Digital Temperature Sensor Definitions.
 *              This file is based on TMP75 Datasheet.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <endian.h>
#include "nvmonvars.h"
#include "endians.h"
#include "common.h"
#include "platform_sensor.h"
#include "defs.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "common_utils.h"
#include "byteswap.h"
#include "hr_commn_util.h"
#include "highrise_cpld_lib.h"

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)
#define DELAY_2_SEC 2000000 

/******************************************************************************
 *                                   Externs
 ******************************************************************************/

uint32 highrise_display_temp(void);
int build_ts_utils_menu (boolean ts_util_items_executed);
extern int highrise_mem_read32 (uint offset, uint *buf);
extern int highrise_mem_write32 (uint offset, uint buf);
extern int ht_version_v2;   

/******************************************************************************
 *                               Function prototypes
 ******************************************************************************/
static uint32 show_temperature (int);
static uint32 ts_sanity_test (int);
static uint32 alter_ts_reg(int);
static uint32 show_ts_reg(int);
static uint32 ts_interrupt_test(int);
static int ht_has_intr_test(void);
static int ts_is_intr_active(int);
int ts_open_test(int); 

/******************************************************************************
 *                                 Global variables
 ******************************************************************************/

const static uint8_t cfg_shutdown_pos   = 0x0;
const static uint8_t cfg_shutdown_msk   = 0x1;
const static uint8_t cfg_mode_pos       = 0x1;
const static uint8_t cfg_mode_msk       = 0x1;
const static uint8_t cfg_polarity_pos   = 0x2;
const static uint8_t cfg_polarity_msk   = 0x1;
const static uint8_t cfg_fault_pos      = 0x3;
const static uint8_t cfg_fault_msk      = 0x3;
const static uint8_t cfg_resolution_pos = 0x5;
const static uint8_t cfg_resolution_msk = 0x3;
const static uint8_t cfg_oneshot_pos    = 0x7;
const static uint8_t cfg_oneshot_msk    = 0x1;

/* Temperature sensor registers table. This device has registers with different sizes.
 */
static reg_info_t ts_reg_table[] =
{
/*  { name, offset, rw type,
 *    size, mask, default value},
 */
    {"Configuration register1 (WR)",     TMP432_WR_CFG_1,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Conversion rate register (WR)",     TMP432_WR_CONVER_RATE,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature high limit (high byte) (WR)",     TMP432_WR_LT_HL_H,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature low limit (high byte) (WR)",     TMP432_WR_LT_LL_H,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 high limit (high byte) (WR)",     TMP432_WR_RT1_HL_H,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 low limit (high byte) (WR)",     TMP432_WR_RT1_LL_H,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"One-shot start",     TMP432_WR_ONE_SHOT, WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature (high byte)",     TMP432_LT_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 (high byte)",     TMP432_RT1_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Status register",     TMP432_STATUS,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Configuration register1",     TMP432_CFG_1,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Conversion rate register",     TMP432_CONVER_RATE,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature high limit (high byte)",     TMP432_LT_HL_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature low limit (high byte)",     TMP432_LT_LL_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 high limit (high byte)",     TMP432_RT1_HL_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 low limit (high byte)",     TMP432_RT1_LL_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 (low byte)",     TMP432_RT1_L,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 high limit (low byte)",     TMP432_RT1_HL_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature1 low limit (low byte)",     TMP432_RT1_LL_L ,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature2 high limit (high byte)",     TMP432_RT2_HL_H,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature2 low limit (high byte)",     TMP432_RT2_LL_H,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature2 high limit (low byte)",     TMP432_RT2_HL_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature2 low limit (low byte)",     TMP432_RT2_LL_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote therm limit",     TMP432_R_THREM_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote2 therm limit",     TMP432_R2_THREM_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Open status",     TMP432_OPEN_STATUS,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Channel mask",     TMP432_CHANNEL_MSK,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local therm limit",     TMP432_L_THREM_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Therm limit hysteresis",     TMP432_THREM_HYST,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Consecutive alert register",     TMP432_CONSE_ALT,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature2 (high byte)",     TMP432_RT2_H,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Remote temperature2 (low byte)",    TMP432_RT2_L,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Ch. 1 beta range selection",     TMP432_CH1_BETA_RAN_SEL,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Ch. 2 beta range selection",     TMP432_CH2_BETA_RAN_SEL,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"N-factor correction remote1",     TMP432_NF_CORR_R1,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"N-factor correction remote2",     TMP432_NF_CORR_R2,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature (low byte)",     TMP432_LT_L,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"High limit status",     TMP432_HL_STATUS,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Low limit status",     TMP432_LL_STATUS,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Therm status",     TMP432_THEREM_STATUS,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature high limit (low byte)",     TMP432_LT_HL_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Local temperature low limit (low byte)",     TMP432_LT_LL_L,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Configuration register2",     TMP432_CFG_2,    READ_WRITE,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Software reset",     TMP432_SOFT_RESET,    WRITE_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"TMP432 device ID",    TMP432_DEVICE_ID,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {"Manufacturer ID",     TMP432_MFG_ID,    READ_ONLY,
        {TMP432_BUF_SIZE},        0x0000, 0x0000},
    {0, 0, 0, {0}, 0, 0},

    {"Temperature",		TS_PTR_TEMP,	READ_ONLY,
	{TS_PTR_TEMP_L},	0xFF80, 0x0000},

    {"Configuration",	TS_PTR_CFG,	READ_WRITE,
	{TS_PTR_CFG_L},	0x1F, 0x00},

    {"T_LOW",	TS_PTR_THYST,	READ_WRITE,
	{TS_PTR_THYST_L},	0xFF80, 0x0000},

    {"T_HIGH",	TS_PTR_TOS,	READ_WRITE,
	{TS_PTR_TOS_L},	0xFF80, 0x0000},

    {0, 0, 0, {0}, 0, 0},
};

/******************************************************************************
 *                                   Menus
 ******************************************************************************/
/*
 * Temperature sensor Menu
 */
static submenu_xtable_t ts_menu_table[] = {
    {"TS utilities",
        (PFT)build_ts_utils_menu,    0,
        MF_SHOW_ERRCOUNT,
        (type_t(*)())0, 0,
        (PFT)build_ts_utils_menu,    0},

    {"Show temperature",
        (PFT)show_temperature,       0,
        MM_3,
        (type_t(*)())0, 0,
        (PFT)show_temperature,       0},

    {"Register sanity test",
        (PFT)ts_sanity_test,         0,
        MM_3,
        (type_t(*)())0, 0,
        (PFT)ts_sanity_test,         0},

    {"Interrupt test",
        (PFT)ts_interrupt_test,      0,
        MM_3,
        (type_t(*)())ht_has_intr_test, 0, 
        (PFT)ts_interrupt_test,      0},

    {"Open test", 
        (PFT)ts_open_test,           0,
        MM_3,
        (type_t(*)())0, 0,
        (PFT)ts_open_test,           1},

};

#define TS_MENU_TABLE_SIZE (sizeof(ts_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_menu_primary_items[TS_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];
static mitem_t ts_menu_secondary_items[TS_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo tsdiag = {
    "Temperature Sensors Utility Menu",	    /* title */
    0,				    /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	    /* shows major flags */
    0,				    /* generic prompt */
    0,				    /* size -- bumped by add_menu_item() */
    ts_menu_primary_items,
};

static struct menuinfo *tsdiagp = &tsdiag;




/* TS utils menu */
static submenu_xtable_t ts_util_menu_table[] = {
    {"Show register",
        (PFT)show_ts_reg, 0,
        MM_3,
        (type_t(*)())0, 0,
        (type_t(*)())0, 0},

    {"Alter register",
        (PFT)alter_ts_reg, 0,
        MM_3,
        (type_t(*)())0, 0,
        (type_t(*)())0, 0},
};

#define TS_UTIL_MENU_TABLE_SIZE (sizeof(ts_util_menu_table)/sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_util_menu_primary_items[TS_UTIL_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];
static mitem_t ts_util_menu_secondary_items[TS_UTIL_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo tsutildiag = {
  "TS Utility Menu",      /* title */
  0,                            /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,        /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  ts_util_menu_primary_items,
};

static struct menuinfo *tsutildiagp = &tsutildiag;


static int ht_has_intr_test (void) 
{
    return (ht_version_v2); 
}


/******************************************************************************
 *
 * function   : build_ts_menu
 * Description:	Build menu for temperature sensor related utility.
 * Inputs     :
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int
build_ts_menu (boolean ts_items_executed)
{
    char t_name[ERR_BUF_SIZE];

    sprintf((char *)t_name, "Temperature Sensor");

    testname(t_name);

    build_primary_submenu(ts_menu_table, TS_MENU_TABLE_SIZE,
                          "Temperature Sensors Utility Menu", &tsdiagp);
    build_secondary_submenu(ts_menu_table, TS_MENU_TABLE_SIZE,
                            ts_menu_secondary_items);

    if (ts_items_executed) {
        menu(&tsdiag, ts_menu_secondary_items, 0);
    } else {
        do_all_menu_items(tsdiagp);     
    }

    return (PASSED);
}


/*************************************************************************
 * Function:    build_ts_utils_menu
 *
 * Description: Entry to TS chip utilities menu.
 *
 * Inputs:      None.
 *
 * Outputs:     PASSED.
 *
 *************************************************************************
 */
int build_ts_utils_menu (boolean ts_util_items_executed)
{

    build_primary_submenu(ts_util_menu_table, TS_UTIL_MENU_TABLE_SIZE,
                          "TS Utility Main Menu", &tsutildiagp);
    build_secondary_submenu(ts_util_menu_table, TS_UTIL_MENU_TABLE_SIZE,
                            ts_util_menu_secondary_items);

    menu(tsutildiagp, ts_util_menu_secondary_items, '\0' );

    return PASSED;
}

/******************************************************************************
 *
 * function   : read_ts_reg
 * Description: Wrapper to read temperature sensor's register.
 * Inputs     : ts_id - sensor ID (Highrise only has one TS)
 *              offset - pointer register ID.
 * 				data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int
read_ts_reg (int ts_id, int offset, char * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t *i2c_if;

    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
            MB_I2C_ADDR_TMP75);

    if (NULL == i2c_if) {
        printf("%s[%d]: failed to get i2c_if structure\n", __FILE__, __LINE__);
        return (FAILED);
    }
    i2c_if->offset = offset;
    i2c_if->buf = data_buf_p;
    i2c_if->size = size;

    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, offset, size, rc);
        return (FAILED);
    }
    if (size == sizeof(uint16_t)) {
        *((uint16_t *)data_buf_p) =  be16toh(*((uint16_t *)data_buf_p));
    }

    return (rc);
}


/******************************************************************************
 *
 * function   : write_ts_reg
 * Description: Wrapper to write temperature sensor's register.
 * Inputs     : ts_id - sensor ID (Highrise only has one TS)
 *              offset - pointer register ID.
 * 				data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int
write_ts_reg (int ts_id, int offset, char *data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t *i2c_if;

    /* Setup I2C API interface struct */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_ONE, I2C_MUX_ZERO,
            MB_I2C_ADDR_TMP75);

    if (NULL == i2c_if) {
        printf("%s[%d]: failed to get i2c_if structure\n", __FILE__, __LINE__);
        return (FAILED);
    }
    i2c_if->buf = data_buf_p;
    i2c_if->offset = offset;
    i2c_if->size = size;

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to write offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, offset, size, rc);
        return (FAILED);
    }
    usleep(100000); /* sleep 100 ms after writing */
    return(rc);
}


/******************************************************************************
 *
 * function   : ts_sanity_test
 * Description: A quick sanity read and write access test to device's R/W register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static uint32
ts_sanity_test (int ts_id)
{
    int offset = TMP432_RT2_HL_H;   
    ts_t backup_buf, new_buf, compare_buf;
    uint32_t rc = FAILED;
    int size = TMP432_BUF_SIZE;
    char *tname = "sanity";

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    backup_buf = new_buf = compare_buf = 0;

    /* read and backup reg data */
    rc = read_ts_reg (ts_id, offset, (char*) &backup_buf, size); 
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d backup old value: read_ts_reg failed\n",
                __FUNCTION__, __LINE__);
        return (FAILED);
    }
    /* write new data */
    new_buf = TS_TEST_PATTERN & 0xFF; 
    rc = write_ts_reg (ts_id, offset, (char*) &new_buf, size);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d write new value: write_ts_reg failed\n",
                __FUNCTION__, __LINE__);
        return (FAILED);
    }
    // read and compare reg data
    rc = read_ts_reg (ts_id, offset, (char*) &compare_buf, size);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d read new value: read_ts_reg failed\n",
                __FUNCTION__, __LINE__);
        return (FAILED);
    }
    if (compare_buf != new_buf) {
        cterr('f', 0, "%s:%d compare result: value not matched (%#x != %#x)\n",
                __FUNCTION__, __LINE__, new_buf, compare_buf);
        return (FAILED);
    }

    /* restore original data */
    rc = write_ts_reg (ts_id, offset, (char*) &backup_buf, size);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d restore old value: write_ts_reg failed\n",
                __FUNCTION__, __LINE__);
        return (FAILED);
    }
    
    return(rc);
}

static int ts_get_temperature(const ts_t *regv, float *temp)
{
    int size = TMP432_BUF_SIZE;
    uint32 rc;
    char buf1, buf2;
    int ts_id = 0; 

    rc = read_ts_reg (ts_id, TMP432_LT_H, &buf1, size);
    rc = read_ts_reg (ts_id, TMP432_LT_L, &buf2, size);

    *temp = buf1 + ((buf2 & 0xF0) >> 4 ) * TMP432_RESOL; 

    return 0; 
}

static int ts_get_threshold(float *low, float *high, float *local)
{
    int size = TMP432_BUF_SIZE, ts_id = 0;
    uint32 rc;
    char buf1, buf2;


    if (low) { 
        rc = read_ts_reg (ts_id, TMP432_LT_LL_H, &buf1, size);
        rc = read_ts_reg (ts_id, TMP432_LT_LL_L, &buf2, size);
        *low =  buf1 + ((buf2 & 0xF0) >> 4 ) * TMP432_RESOL;
    } 

    if (high) {
        rc = read_ts_reg (ts_id, TMP432_LT_HL_H, &buf1, size);
        rc = read_ts_reg (ts_id, TMP432_LT_HL_L, &buf2, size);
        *high =  buf1 + ((buf2 & 0xF0) >> 4 ) * TMP432_RESOL;
    }

    if (local) {
        rc = read_ts_reg (ts_id, TMP432_L_THREM_L, &buf1, size);
        *local =  buf1;
    }
    return 0;
}

static int ts_set_threshold(const float *low, const float *high, const float *local)
{
    char buf; 
    int rc, size = TMP432_BUF_SIZE, ts_id = 0;

    if (low) { 
         buf = (char)*low; 
         rc = write_ts_reg (ts_id, TMP432_LT_LL_H, &buf, size);
    } 
    if (high) {
         buf = (char)*high; 
         rc = write_ts_reg (ts_id, TMP432_LT_HL_H, &buf, size);
    }   
    if (local) {
         buf = (char)*local; 
         rc = write_ts_reg (ts_id, TMP432_L_THREM_L, &buf, size);
    }
    return 0;
}

/******************************************************************************
 *
 * Function   : show_temperature
 * Description: show temperature of the selected sensor
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32
show_temperature (int ts_id)
{
    int size = TMP432_BUF_SIZE, ix, jx;
    uint32 rc;
    char buf1; 
    float val = 0; 
    int regs[3][2]= {{TMP432_LT_H, TMP432_LT_L}, 
                     {TMP432_RT1_H,TMP432_RT1_L},
                     {TMP432_RT2_H, TMP432_RT2_L}}; 
    char ts_name[3][16] = {"Local", "Remote1", "Remote2"}; 

    for (ix = 0; ix < 3; ix++) {
        for (jx = 0; jx < 2; jx++) {
            rc = read_ts_reg (ts_id, regs[ix][jx], &buf1, size);
            if ((NVRAM)->diagflag & D_VERBOSE) {
                printf("(0x%02X), data: 0x%04X\n", regs[ix][jx], buf1);
            }
            if (jx == 0) {
                val += buf1; 
            } else { 
                val += (buf1 & 0xF0 >> 4) * TMP432_RESOL; 
            }
        }
        printf("%s = %f degree C \n", ts_name[ix], val); 
        val = 0; 
    }
    return (PASSED);
}

/********************************************************************
 *
 * Function:	show_ts_reg
 *
 * Description:	show temperature sensor register.
 *
 * Inputs:	-
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32
show_ts_reg(int ts_id)
{
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t buffer = 0;

    reg_p = &ts_reg_table[0];

    printf("ts_id %d\n", ts_id);
    while (reg_p->name) {
        if (reg_p->type == WRITE_ONLY) {
            reg_p++;
            continue; 
        }
        buf_p  = (char *)&buffer;
        offset = reg_p->offset;
        size   = reg_p->size.size;
        rc = read_ts_reg (ts_id, offset, buf_p, size);
        if (rc != PASSED) {
             cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                    __FUNCTION__, __LINE__, ts_id);
             return (FAILED);
        } else {
            if (size == sizeof(ts_t))
                printf("%-36s (0x%02X), data: 0x%04X\n", reg_p->name,
                        reg_p->offset, buffer);
            else
                printf("%-36s (0x%02X), data: 0x%02X\n", reg_p->name,
                        reg_p->offset, buffer & 0xff);
        }
        reg_p++;
    }
    return (PASSED);
}


/********************************************************************
 *
 * Function:	alter_ts_reg
 *
 * Description:	alter temperature sensor register.
 *
 * Inputs:	-
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32
alter_ts_reg(int ts_id)
{
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t temp_data, old_temp_data;
    ts_c cfg_data, old_cfg_data;

    reg_p = &ts_reg_table[0];

    printf("ts_id %d\n", ts_id);
    printf("\nRegister number:\n");
    /* Get the register to peek-n-poke */
    while (reg_p->name) {
        if (reg_p->type & READ_ONLY) {
            /* Read only register */
        } else {
            /* Write only or read/write register */
            printf("   %02x - %s\n", reg_p->offset, reg_p->name);
        }
        reg_p++;	/* update the register table pointer */
    } /* endof while */

    /* Get the register to peek-n-poke */
    offset = gethex_answer("Enter the register number:", 0, 0, TMP432_SOFT_RESET); 

    /* Check if the register is read/writeable */
    /* Find the register text in the register table */
    reg_p = &ts_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != offset) {
         /* Not requested register */
         reg_p++; /* update the register table pointer */
    }

    if (reg_p->name) {
         /* Got the register. */
         if ((reg_p->type == READ_WRITE) || (reg_p->type == WRITE_ONLY)) {

             /* Read/Writeable. Read the register first. */
             if (reg_p->size.size == sizeof(ts_c)) {
                   /* Configuration register */
                   size = sizeof(ts_c);
                   buf_p = (char *)&old_cfg_data;
             } else {
                   /* Temperature registers */
                   size = sizeof(ts_t);
                   buf_p = (char *)&old_temp_data;
             }

             offset = reg_p->offset;

             /* Configuration register */
             rc = read_ts_reg (ts_id, offset, buf_p, size);
             if (rc != PASSED) {
                  cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
							  __FUNCTION__, __LINE__, ts_id);
                  return (FAILED);
             }

             /* Get the new data */
             if (reg_p->size.size == sizeof(ts_c)) {
                  /* Configuration register */
                  cfg_data = gethex_answer("Enter the data:", old_cfg_data, 0, 0xFF);
                  buf_p = (char*)&cfg_data;
             } else {
                  /* Temperature registers */
                  temp_data = gethex_answer("Enter the data:",
                  old_temp_data, 0, TS_TEMP_MASK);
                  buf_p = (char *)&temp_data;
             }

             /* Write the new data */
             rc = write_ts_reg (ts_id, offset, buf_p, size);
             if (rc != PASSED) {
                   cterr('f', 0, "%s:%d restore old value: write_ts_reg failed\n", __FUNCTION__, __LINE__);
                   return (FAILED);
              }
          } else {
               /* Not read/writeable */
               printf("alter_ts_reg() %s is a read only register", reg_p->name);
               return (FAILED);
          }
    } else {
          /* Invalid register */
          printf("alter_ts_reg() %#x is not a valid register", offset);
           return (FAILED);
    }
    return (PASSED);
}


uint32 highrise_display_temp(void)
{
    int rc = 0;
    char *tname = "Temperature sensor";
    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    rc = show_temperature(0);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d- Error to show temperature\n", __FUNCTION__, __LINE__);
        return FAILED;

    }
    return PASSED;
}

int highrise_config_ts_init(void)
{
    /* HW updated to new chip tmp432 
     * no need for init seq */
    return (PASSED);
}

/* GPIO init is done in highrise.c */
static const struct {
    char *name;
    uint32_t rego;
    uint32_t boff;
    uint32_t blen;
    char *tag;
} ts_intr_mpp[] = {
    /*0 */{"function        ", 0xf2440004, 16,  4, ""                      } ,
    /*1 */{"data-out        ", 0xf2440100, 12,  1, ""                      } ,
    /*2 */{"data-out-en     ", 0xf2440104, 12,  1, "Active Low"            } ,
    /*3 */{"blink-en        ", 0xf2440108, 12,  1, ""                      } ,
    /*4 */{"data-in-polarity", 0xf244010c, 12,  1, "If set, 'data-in' is inverted of PIN" },
    /*5 */{"data-in         ", 0xf2440110, 12,  1, ""                      } ,
    /*6 */{"intr-cause      ", 0xf2440114, 12,  1, ""                      } ,
    /*7 */{"intr-mask       ", 0xf2440118, 12,  1, ""                      } ,
    /*8 */{"intr-level-mask ", 0xf244011c, 12,  1, ""                      } ,
    /*9 */{"blink-cntr      ", 0xf2440120, 12,  1, ""                      } ,
    /*10*/{NULL              , ~0        , ~0, ~0, ""                      } ,
};

static int ts_intr_gpio_dump(const char *tag)
{
    uint32_t regv = 0;
    int      idx  = 0;

    for(idx = 0; ts_intr_mpp[idx].name; idx++) {
        ERR_RET_COND(PASSED != highrise_mem_read32(ts_intr_mpp[idx].rego, &regv),
            -(__LINE__), "Failed to read ts intr gpio reg-%s.\n", ts_intr_mpp[idx].name);

        printf("%-16s:@%08x:%08x:boff-%-2u:blen-%u:bval-%x %s\n",
            ts_intr_mpp[idx].name,
            ts_intr_mpp[idx].rego, regv,
            ts_intr_mpp[idx].boff,
            ts_intr_mpp[idx].blen,
            (regv >> ts_intr_mpp[idx].boff) & ((1 << ts_intr_mpp[idx].blen) - 1),
            ts_intr_mpp[idx].tag
        );
    }
    return 0;
}

static int ts_is_intr_active(int clr)
{
    uint32_t regv = 0;
    uint32_t msk  = 0;
    int ret = 0;

    ERR_RET_COND(PASSED != highrise_mem_read32(ts_intr_mpp[6].rego, &regv),
        -(__LINE__), "Failed to read ts intr cause reg.\n");

    msk  = (((1 << ts_intr_mpp[6].blen) - 1) << ts_intr_mpp[6].boff);
    ret = regv & msk ? 1 : 0;

    if (ret && clr) {
        /* clear gpio intr status */
        regv &= ~msk;
        ERR_RET_COND(PASSED != highrise_mem_write32(ts_intr_mpp[6].rego, regv),
            -(__LINE__), "Failed to write ts intr cause reg.\n");
    }

    return ret;
}

/********************************************************************
 *
 * Function:    ts_interrupt_test
 *
 * Description: tmp432 has 2 interrput to CPLD and CPU, 
 *              we need to adjust threshold to trigger both interrupts.
 *              THREM to CPU; ALERT to CPLD.
 *
 * Inputs:      None
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32 ts_interrupt_test(int opt)
{
    int ret  = 0, ts_id = 0, ix;
    const char *tname = "Temp Sensor Intr Test";
    float lthresh = 0;
    float hthresh = 0;
    float localthresh = 0.0;
    float curtemp = 0;
    unsigned long status = 0;
    unsigned long enable = 0;
    char buf1; 

    testname("%s", tname);

    ERR_RET_COND(0 > ts_get_temperature(NULL, &curtemp), -(__LINE__), "Failed to get current temp.\n");
    if (curtemp < -20.0 || curtemp > 100) {
        printf("Current temp is out of range [-20, 100], abort TS intr test.\n");
        cterr('f', 0, "%s:%d-Current temp is %d out of range [-20, 100], abort TS intr test", 
                      __FUNCTION__, __LINE__, curtemp); 
        return (FAILED); 
    }

    /* save orig thresholds */
    ERR_RET_COND(0 >  ts_get_threshold(&lthresh, &hthresh, &localthresh), FAILED, "%s failed.\n", tname);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("Low limit temp = %f \n", lthresh); 
        printf("High limit temp = %f \n", hthresh); 
        printf("Local limit temp = %f \n", localthresh); 
    }

    /* enable CPLD intr */  
    prpass(testpass, "enable temp sensor interrupt"); 
    hr_cpld_intr_enable(HT_CPLD_INT_TEMP_SENSOR, TRUE); 

    /* read back to check ?*/
    hr_cpld_intr_status(&status, &enable);
    if (enable & HT_CPLD_INT_TEMP_SENSOR) {
        cterr('f', 0, "%s:%d-Cannot enable 0x%d on CPLD interrupt register ", 
                      __FUNCTION__, __LINE__, HT_CPLD_INT_TEMP_SENSOR); 
        printf("CPLD intr status:0x%08x, enable:0x%08x\n",
               (uint32_t)status, (uint32_t)enable);
        return (FAILED);
    }

    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("CPLD intr status:0x%08x, enable:0x%08x\n",
               (uint32_t)status, (uint32_t)enable);
    } 

    /* write configuration register bit2 = 1 for enable extended temp */
    /* in EDVT, normal mode cannot generate interrupt while 
     * current temp is -5C; this case also covert room temp. */
    char buf = 0x4; 
    ret = write_ts_reg (0, 0x9, &buf, TMP432_BUF_SIZE);

    prpass(testpass, "trigger interrupt");
    float l_intr = 60.0;
    float h_intr = 4.0;  /* in extended mode, 4 = 0x4 is -60C */
    float local_intr = 4.0; /* in extended mode, 4 = 0x4 is -60C */
    ERR_RET_COND(0 > ts_set_threshold(&l_intr, &h_intr, &local_intr), -(__LINE__), "Failed to trigger intr.\n");

    /* wait for interrupt generate */
    usleep(DELAY_2_SEC);

    prpass(testpass, "check CPLD intr "); 
    hr_cpld_intr_status(&status, &enable);
    if (status & HT_CPLD_INT_TEMP_SENSOR) {
        printf("\nDetect CPLD Intr\n");
        hr_cpld_intr_enable(HT_CPLD_INT_TEMP_SENSOR, FALSE); 
    } else { 
        printf("CPLD intr status:0x%08x, enable:0x%08x\n",
               (uint32_t)status, (uint32_t)enable);
        cterr('f', 0, "%s:%d-No interrupt on 0x44 CPLD interrupt status reg", 
                      __FUNCTION__, __LINE__); 
        /* fail through to check CPU intr */
    }

    for (ix = 0; ix < 4; ix++) {
        ret = read_ts_reg (ts_id, TMP432_THEREM_STATUS, &buf1, TMP432_BUF_SIZE); 

        if (buf1 & 0x1) { 
            /* restore the value to trigger intr to cpu */
            prpass(testpass, "Threm intr occurred, restore the value for CPU intr cnt = %d\n", ix); 
            ts_set_threshold(&lthresh, &hthresh, &localthresh);

            /* restore to normal mode */
            buf = 0; 
            ret = write_ts_reg (0, 0x9, &buf, TMP432_BUF_SIZE);

            /* read for clean up intr */
            ret = read_ts_reg (ts_id, TMP432_THEREM_STATUS, &buf1, TMP432_BUF_SIZE); 
            ret = read_ts_reg (ts_id, TMP432_LL_STATUS, &buf1, TMP432_BUF_SIZE); 
            ret = read_ts_reg (ts_id, TMP432_HL_STATUS,  &buf1, TMP432_BUF_SIZE); 
            hr_cpld_intr_clear(HT_CPLD_INT_TEMP_SENSOR); 
            break; 
        }
        usleep(DELAY_2_SEC);
    }

    if (ix == 4) { 
        cterr('f', 0, "%s:%d-No interrupt on THREM status (0x37) of Temp sensor", 
                      __FUNCTION__, __LINE__); 
    }

    for (ix = 0; ix < 4; ix++) {
        if (ts_is_intr_active(0) == 1) {
            if ((NVRAM)->diagflag & D_VERBOSE) {
                ts_intr_gpio_dump(NULL);
            }
            prpass(testpass, "Detect CPU Intr ", tname);
            ts_is_intr_active(1);  /* clear cpu intr */
            return (PASSED); 
        } else {
            show_ts_reg(0);
            ts_intr_gpio_dump(NULL);
            usleep(DELAY_2_SEC);
        }
    }

    if (ix == 4) { 
        cterr('f', 0, "%s:%d-No interrupt on CPU intr-cause 0xf2440114 register ", 
                      __FUNCTION__, __LINE__); 
    }

    return (FAILED); 

}

/********************************************************************
 *
 * Function:    ts_open_test
 *
 * Description: The comparator output is continuously checked during 
 *              a conversion. If a fault is detected, the last valid 
 *              measured temperature is used for the temperature 
 *              measurement result, the OPEN bit (Status
 *              Register, bit 2) is set high.
 *
 * Inputs:      None
 *
 * Outputs:     PASSED - No errors encounterd.
 *              FAILED - Errors encounterd.
 *
 *********************************************************************
 */
int ts_open_test (int dummy) {

    int ret  = 0, ts_id = 0; 
    char *tname = "Temperature sensor Open";
    char buf1; 

    testname("%s", tname);
    prpass(testpass, "%s, ", tname);

    ret = read_ts_reg (ts_id, TMP432_STATUS, &buf1, TMP432_BUF_SIZE); 
    if (buf1 & TS_STATUS_OPEN) { 
        cterr('f', 0, "%s:%d- Error to show temperature\n", __FUNCTION__, __LINE__);
        return FAILED;
    } 
    
    return (PASSED);
}

/*********************************************************************
 * $Log: platform_sensor.c,v $
 * Revision 1.2  2021/06/02 02:56:22  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.7  2021/04/19 07:44:21  alpeng
 * update temp intr high threshold lower to -60C for EDVT
 *
 * Revision 1.1.4.6  2021/03/29 07:12:37  alpeng
 * support temp interrupt in -5C
 *
 * Revision 1.1.4.5  2020/09/28 09:39:41  alpeng
 * support temp open test
 *
 *
 * $Endlog$
 */

