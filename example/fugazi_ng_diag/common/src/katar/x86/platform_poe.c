/* $Id: platform_poe.c,v 1.2 2019/06/14 05:24:51 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_poe.c,v $
 *------------------------------------------------------------------
 *
 * katar_poe.c - This file contains functions for katar POE 
 *                    controller.
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "nvmonvars.h"
#include "common_utils.h"
#include "proto.h"
#include "strings.h"
#include "queryflags.h"
#include "plat_defs.h"
#ifndef TACHI_INTEL 
#include "platform_poe_psu.h"
#else 
#include "nim_test_defs.h"
#endif 
#include "plat_defs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h> 

#include "dev_ilp.h"
#include "platform_i2c.h"
#include "platform_poe.h"
#include "i2c_dev.h"
#include "i2c_api.h"

#define POE_DEBUG 1

extern int phy_detect_phone(int port);
extern boolean has_poe_psu(uint32_t);
extern boolean is_juno(void);
extern uint32_t check_poe_psu_present(uint32_t, uint32_t);
int katar_get_poe_54V_present (void);

boolean poe_si_flg; 
static int peek_poe_reg();
static int poke_poe_reg();
static int display_poe_reg();
int poe_power_ports_util();
int poe_status_util (int bBrief);

static poe_reg_info_t poe_reg_tbl[] = 
{    
	{"INT",              POE_INT_REG,              RO_REG,   0x00, 0x80},
	{"INTMASK",          POE_INTMASK_REG,          RW_REG,   0xff, 0xe4},
	{"PWREVN",           POE_PWREVN_REG,           RO_REG,   0x00, 0x00},
	{"PWREVN_COR",       POE_PWREVN_COR_REG,       COR_REG,  0x00, 0x00},
	{"DETEVN",           POE_DETEVN_REG,           RO_REG,   0x00, 0x00},
	{"DETEVN_COR",       POE_DETEVN_COR_REG,       COR_REG,  0x00, 0x00},
	{"FLTEVN",           POE_FLTEVN_REG,           RO_REG,   0x00, 0x00},
	{"FLTEVN_COR",       POE_FLTEVN_COR_REG,       COR_REG,  0x00, 0x00},
	{"TSEVN",            POE_TSEVN_REG,            RO_REG,   0x00, 0x00},
	{"TSEVN_COR",        POE_TSEVN_COR_REG,        COR_REG,  0x00, 0x00},
	{"SUPEVN",           POE_SUPEVN_REG,           RO_REG,   0x00, 0x00},
	{"SUPEVN_COR",       POE_SUPEVN_COR_REG,       COR_REG,  0x00, 0x00},
	{"STATP1",           POE_STATP1_REG,           RO_REG,   0x00, 0x00}, 
	{"STATP2",           POE_STATP2_REG,           RO_REG,   0x00, 0x00}, 
	{"STATP3",           POE_STATP3_REG,           RO_REG,   0x00, 0x00}, 
	{"STATP4",           POE_STATP4_REG,           RO_REG,   0x00, 0x00}, 
	{"STATPWR",          POE_STATPWR_REG,          RO_REG,   0x00, 0x00}, 
	{"STATPIN",          POE_STATPIN_REG,          RO_REG,   0x00, 0x00}, 
	{"OPMD",             POE_OPMD_REG,             RW_REG,   0xff, 0xff}, 
	{"DISENA",           POE_DISENA_REG,           RW_REG,   0xff, 0xf0}, 
	{"DETENA",           POE_DETENA_REG,           RW_REG,   0xff, 0xff}, 
	{"MIDSPAN",          POE_MIDSPAN_REG,          RW_REG,   0xff, 0x0f}, 
	{"TCONF",            POE_TCONF_REG,            RSRV_REG, 0xff, 0x00}, 
	{"MCONF",            POE_MCONF_REG,            RW_REG,   0xff, 0xa0}, 
	{"DETPB",            POE_DETPB_REG,            SO_REG,   0xff, 0x00}, 
	{"PWRPB",            POE_PWRPB_REG,            SO_REG,   0xff, 0x00}, 
	{"RSTPB",            POE_RSTPB_REG,            SO_REG,   0xff, 0x00}, 
	{"ID",               POE_ID_REG,               RO_REG,   0x00, 0x2e}, 
	{"TLIM12",           POE_TLIM12_REG,           RW_REG,   0xff, 0x00}, 
	{"TLIM34",           POE_TLIM34_REG,           RW_REG,   0xff, 0x00}, 
	{"IP1LSB",           POE_IP1LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP1MSB",           POE_IP1MSB_REG,           RO_REG,   0x00, 0x00}, 
	{"VP1LSB",           POE_VP1LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"VP1MSB",           POE_VP1MSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP2LSB",           POE_IP2LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP2MSB",           POE_IP2MSB_REG,           RO_REG,   0x00, 0x00}, 
	{"VP2LSB",           POE_VP2LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"VP2MSB",           POE_VP2MSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP3LSB",           POE_IP3LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP3MSB",           POE_IP3MSB_REG,           RO_REG,   0x00, 0x00}, 
	{"VP3LSB",           POE_VP3LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"VP3MSB",           POE_VP3MSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP4LSB",           POE_IP4LSB_REG,           RO_REG,   0x00, 0x00}, 
	{"IP4MSB",           POE_IP4MSB_REG,           RO_REG,   0x00, 0x00},
	{"VP4LSB",           POE_VP4LSB_REG,           RO_REG,   0x00, 0x00},
	{"VP4MSB",           POE_VP4MSB_REG,           RO_REG,   0x00, 0x00},
	{"FIRMWARE",         POE_FIRMWARE_REG,         RO_REG,   0x00, 0x0c},
	{"WDOG",             POE_WDOG_REG,             RW_REG,   0x1f, 0x16},
	{"DEVID",            POE_DEVID_REG,            RO_REG,   0x00, 0x44},
	{"HPEN",             POE_HPEN_REG,             RW_REG,   0x0f, 0x0f},
	{"HPMD1",            POE_HPMD1_REG,            RW_REG,   0x03, 0x01},
	{"CUT1",             POE_CUT1_REG,             RW_REG,   0xff, 0xe4},
	{"LIM1",             POE_LIM1_REG,             RW_REG,   0xff, 0x80},
	{"HPSTAT1",          POE_HPSTAT1_REG,          RO_REG,   0x00, 0x00},
	{"HPMD2",            POE_HPMD2_REG,            RW_REG,   0x03, 0x01},
	{"CUT2",             POE_CUT2_REG,             RW_REG,   0xff, 0xe4},
	{"LIM2",             POE_LIM2_REG,             RW_REG,   0xff, 0x80},
	{"HPSTAT2",          POE_HPSTAT2_REG,          RO_REG,   0x00, 0x00},
	{"HPMD3",            POE_HPMD3_REG,            RW_REG,   0x03, 0x01},
	{"CUT3",             POE_CUT3_REG,             RW_REG,   0xff, 0xe4},
	{"LIM3",             POE_LIM3_REG,             RW_REG,   0xff, 0x80},
	{"HPSTAT3",          POE_HPSTAT3_REG,          RO_REG,   0x00, 0x00},
	{"HPMD4",            POE_HPMD4_REG,            RW_REG,   0x03, 0x01},
	{"CUT4",             POE_CUT4_REG,             RW_REG,   0xff, 0xe4},
	{"LIM4",             POE_LIM4_REG,             RW_REG,   0xff, 0x80},
	{"HPSTAT4",          POE_HPSTAT4_REG,          RO_REG,   0x00, 0x00},
	{"VTEMP",            POE_VTEMP_REG,            RO_REG,   0x00, 0x00},
	{"VMAIN_LSB",        POE_VMAIN_LSB_REG,        RO_REG,   0x00, 0x00},
	{"VMAIN_MSB",        POE_VMAIN_MSB_REG,        RO_REG,   0x00, 0x00},
	{"PORT_SR12",        POE_PORT_SR12_REG,        RO_REG,   0x00, 0x00},
	{"PORT_SR34",        POE_PORT_SR34_REG,        RO_REG,   0x00, 0x00},
	{"INVD_CNT",         POE_INVD_CNT_REG,         RO_REG,   0x00, 0x00},
	{"PWRD_CNT",         POE_PWRD_CNT_REG,         RO_REG,   0x00, 0x00},
	{"OVL_CNT",          POE_OVL_CNT_REG,          RO_REG,   0x00, 0x00},
	{"UDL_CNT",          POE_UDL_CNT_REG,          RO_REG,   0x00, 0x00},
	{"SC_CNT",           POE_SC_CNT_REG,           RO_REG,   0x00, 0x00},
	{"CLS_CNT",          POE_CLS_CNT_REG,          RO_REG,   0x00, 0x00},
	{"INTR_EN",          POE_INTR_EN_REG,          RW_REG,   0x01, 0x00},
	{"SYS_CFG",          POE_SYS_CFG_REG,          RW_REG,   0xff, 0x68},
	{"SW_CFG",           POE_SW_CFG_REG,           RW_REG,   0xff, 0x83},
	{"PRIO_CR",          POE_PRIO_CR_REG,          RW_REG,   0xff, 0xaa},
	{"PWR_CR1",          POE_PWR_CR1_REG,          RW_REG,   0x3f, 0x24},
	{"PWR_CR2",          POE_PWR_CR2_REG,          RW_REG,   0x3f, 0x24},
	{"PWR_CR3",          POE_PWR_CR3_REG,          RW_REG,   0x3f, 0x24},
	{"PWR_CR4",          POE_PWR_CR4_REG,          RW_REG,   0x3f, 0x24},
	{"TMP_PWR_CR1",      POE_TMP_PWR_CR1_REG,      RW_REG,   0x3f, 0x00},
	{"TMP_PWR_CR2",      POE_TMP_PWR_CR2_REG,      RW_REG,   0x3f, 0x00},
	{"TMP_PWR_CR3",      POE_TMP_PWR_CR3_REG,      RW_REG,   0x3f, 0x00},
	{"TMP_PWR_CR4",      POE_TMP_PWR_CR4_REG,      RW_REG,   0x3f, 0x00},
	{"PWR_BNK0",         POE_BNK0_REG,             RW_REG,   0xff, 0x90},
	{"PWR_BNK1",         POE_BNK1_REG,             RW_REG,   0xff, 0x8c},
	{"PWR_BNK2",         POE_BNK2_REG,             RW_REG,   0xff, 0x88},
	{"PWR_BNK3",         POE_BNK3_REG,             RW_REG,   0xff, 0x84},
	{"PWR_BNK4",         POE_BNK4_REG,             RW_REG,   0xff, 0x80},
	{"PWR_BNK5",         POE_BNK5_REG,             RW_REG,   0xff, 0x7c},
	{"PWR_BNK6",         POE_BNK6_REG,             RW_REG,   0xff, 0x78},
	{"PWR_BNK7",         POE_BNK7_REG,             RW_REG,   0xff, 0x74},
	{"PWRGD",            POE_PWRGD_REG,            RW_REG,   0x07, 0x40},
	{"PORT1_CONS",       POE_PORT1_CONS_REG,       RO_REG,   0x00, 0x00},
	{"PORT2_CONS",       POE_PORT2_CONS_REG,       RO_REG,   0x00, 0x00},
	{"PORT3_CONS",       POE_PORT3_CONS_REG,       RO_REG,   0x00, 0x00},
	{"PORT4_CONS",       POE_PORT4_CONS_REG,       RO_REG,   0x00, 0x00},
	{"TOTAL_PWR_CONS",   POE_TOTAL_PWR_CONS_REG,   RO_REG,   0x00, 0x00},
	{"TOTAL_PWR_CALC",   POE_TOTAL_PWR_CALC_REG,   RO_REG,   0x00, 0x00},
	{"CHIP_PWR_REQ",     POE_CHIP_PWR_REQ_REG,     RO_REG,   0x00, 0x00},
	{"ICUT_AT_MAX_LSB",  POE_ICUT_AT_MAX_LSB_REG,  RW_REG,   0xff, 0x92},
	{"ICUT_AT_MAX_MSB",  POE_ICUT_AT_MAX_MSB_REG,  RW_REG,   0xff, 0x14},
	{"POE_MAX_LED_GB",   POE_POE_MAX_LED_GB_REG,   RW_REG,   0x3f, 0x0f},
	{"VMAIN_LOW_TH_LSB", POE_VMAIN_LOW_TH_LSB_REG, RW_REG,   0xff, 0xd7},
	{"VMAIN_LOW_TH_MSB", POE_VMAIN_LOW_TH_MSB_REG, RW_REG,   0x07, 0x00},
};

#define KATAR_MAX_POE_PORT 2

/* submenu for POE utilities */
submenu_xtable_t poe_util_submenu_table[] = {
    {"Read POE register",  
     (PFT)peek_poe_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Write POE register",  
     (PFT)poke_poe_reg,           0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Dump POE registers",  
     (PFT)display_poe_reg,        0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"POE status",  
     (PFT)poe_status_util,            0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
    {"Power on/off ports",  
     (PFT)poe_power_ports_util,   0, 0, (type_t(*)())0, 0, 
     (type_t(*)())0, 0},
};

#define POE_UTIL_SUBMENU_TABLE_SIZE (sizeof(poe_util_submenu_table) / \
                                       sizeof(submenu_xtable_t))

/* 
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t poe_util_primary_items[POE_UTIL_SUBMENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];
static mitem_t poe_util_secondary_items[POE_UTIL_SUBMENU_TABLE_SIZE +
                                                MAX_BASE_ITEMS];

menuinfo_t poe_util_menu = {
    "POE Utility Menu",
    0,                                /* mtparam added by init_empty_menu */
    (PFT)show_endnote,                /* notes missing WICs in combos */
    0,                                /* use generic prompt */
    0,                                /* size (bumped by add_menu_item() */
    poe_util_primary_items,
};
menuinfo_t *poe_submenup = &poe_util_menu;

boolean poe_intr_happened;

int 
poe_utils (void)
{
    char t_name[ERR_BUF_SIZE];

    sprintf((char *)t_name, "POE Utility");

    testname(t_name);

    build_primary_submenu(poe_util_submenu_table, 
                          POE_UTIL_SUBMENU_TABLE_SIZE,
                          "POE Utility", &poe_submenup);
    build_secondary_submenu(poe_util_submenu_table,
                            POE_UTIL_SUBMENU_TABLE_SIZE,
                            poe_util_secondary_items);
    menu(&poe_util_menu, poe_util_secondary_items, '\0');

    return (PASSED);
}

#define KATAR_POE_TWSI_ADDR 0x21
#define KATAR_POE_REG_SIZE 1

static int poe_i2c_fd = -1;
extern int get_i2c_fd (int dummy);

/******************************************************************************
 *
 * Function   : init_poe_i2c_struct
 * Description: To init i2c_dev structure.
 * Inputs     : dev_object_t *i2c_dev;
 *              uint32_t dimm_no.
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
uint32_t init_poe_i2c_struct (n2g_i2c_dev_t *i2c_dev) {
    uint32_t rc = FAILED;

    i2c_dev->bus_no = CPU_I2C0;
    i2c_dev->rd_hd_size = 1;
    i2c_dev->wr_hd_size = 1;
    i2c_dev->dev_addr = KATAR_POE_TWSI_ADDR;

    poe_i2c_fd = get_i2c_fd(0);

    /* Set I2C device to SLAVE mode */
    if (poe_i2c_fd <= 0) {
         cterr('f', 0, "/dev/i2c-0 is not opened correctly.");
         return (FAILED);
    } else {
        if ((rc = ioctl(poe_i2c_fd, I2C_SLAVE, i2c_dev->dev_addr)) < 0) {
            cterr('f', 0, "%s at %s: unable to connect to device %#x. "
                          "rc = %#x", __FUNCTION__, __FILE__,
                          i2c_dev->dev_addr, rc);
            return (FAILED);
        } else {
            i2c_dev->fp = poe_i2c_fd;
        }
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : katar_poe_reg_rd
 * Description : Function to read katar PoE controller register.
 * Inputs      : reg_addr - register address that wants to read
 *               *buf     - buffer to put the read back register value
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int katar_poe_reg_rd (int reg_addr, uint8_t *buf)
{
    int      rd_result = -1;
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0;

	if(katar_get_poe_54V_present() == 0) {
		printf("---- Can't detect POE 54V power. ----\n");
        return (FAILED);
	}

    /* Init device structure */	
    if (init_poe_i2c_struct(&i2c_dev) != PASSED) {
        printf("Init read poe i2c_dev struct failed.");
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Read the bytes from PoE controller */
    i2c_if.size = sizeof(uint8_t);       /* Read 1 bytes at a time */
    i2c_if.offset = reg_addr;
    i2c_if.buf = (char *)&reg_val;
    rd_result = api_mb_i2c_read(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);

    if (rd_result != PASSED) {
        printf("%s: TWSI Read Failed !!!\n", __FUNCTION__);
        printf("(Bus%d, Dev 0x%01X, offset 0x%01X)\n",
                           i2c_dev.bus_no, i2c_if.i2c_dev, i2c_if.offset);
        return (FAILED);
    }
    *buf = (uint8_t)(reg_val & 0xff); 

    //printf("%s: reg_val = 0x%016X.\n", __FUNCTION__, reg_val);
    //printf("%s: buf = 0x%02X.\n", __FUNCTION__, *buf);

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : katar_poe_reg_wr
 * Description : Function to write Katar PoE controller register.
 * Inputs      : reg_addr - register address that wants to write
 *               w_data   - data that wants to write to register
 * Outputs     : PASSED/FAILED
 *
 *******************************************************************************
 */
int katar_poe_reg_wr (int reg_addr, uint8_t w_data)
{
    int wr_result = -1;
    n2g_i2c_dev_t i2c_dev;
    n2g_i2c_if_t  i2c_if;
    uint32_t      reg_val = 0;

    if(katar_get_poe_54V_present() == 0) {
        printf("---- Can't detect POE 54V power. ----\n");
        return (FAILED);
    }

    /* Init device structure */
    if (init_poe_i2c_struct(&i2c_dev) != PASSED) {
        printf("Init write poe i2c_dev struct failed.");
        return (FAILED);
    }

    /* Get Registers value */
    /* Setup the interface struct for I2C API read */
    i2c_if.i2c_bus_type = i2c_dev.bus_no;
    i2c_if.i2c_dev = i2c_dev.dev_addr;

    /* Write the bytes from PoE controller */
    i2c_if.size = sizeof(uint8_t);       /* Read 1 bytes at a time */
    i2c_if.offset = reg_addr;
    i2c_if.buf = (char *)&reg_val;
    reg_val = w_data;
    wr_result = api_mb_i2c_write(&i2c_dev, i2c_if.offset, i2c_if.size,
                         (char *)i2c_if.buf);

    if (wr_result  != PASSED) {
        printf("%s: TWSI Write Failed !!!n", __FUNCTION__);
        printf("(Bus%d, Dev 0x%01X, offset 0x%01X)\n",
                           i2c_dev.bus_no, i2c_if.i2c_dev, i2c_if.offset);
        return (FAILED);
    }
	if (diagflag_xram & D_DEBUG_OPTIONS)
	    printf("%s: Done writing 0x%02X to PoE reg.(0x%02X).\n",
	                       __FUNCTION__, w_data, (uint8_t)(i2c_if.offset & 0xff));

    return (PASSED);
}

/*******************************************************************************
 *
 * Function: display_poe_reg
 *
 * Input: 
 *
 * Output: PASSED/FAILED
 *
 *******************************************************************************
 */
static int 
display_poe_reg ()
{
    int            ctr = 0, dump_end = 0;
    poe_reg_info_t *reg_p = &poe_reg_tbl[0];
    uchar        reg_val = 0;

    if(katar_get_poe_54V_present() == 0) {
        printf("---- Can't detect POE 54V power. ----\n");
        return (FAILED);
    }

    dump_end = (int)(sizeof(poe_reg_tbl) / sizeof(poe_reg_info_t));
    printf("Katar PoE registers dump:\n");
    for (ctr = 0; ctr < dump_end; ctr++, reg_p++) 
    {
        if (katar_poe_reg_rd(reg_p->addr, &reg_val) != PASSED)
        {
            printf("Failed to read %s reg.(0x%02X)\n", reg_p->name, (uchar)reg_p->addr);
            return (FAILED);
        }
        printf("%-16s (0x%02X) = 0x%02X\n",reg_p->name, (uchar)reg_p->addr, reg_val);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function: peek_poe_reg
 *
 * Description: Peek PoE controller register.
 *
 * Input:   
 *
 * Outputs:  PASSED/FAILED
 *
 * Assumptions:
 *
 *******************************************************************************
 */
static int 
peek_poe_reg ()
{
    uchar reg, data = 0;
    int retval=FAILED;

    reg = gethex_answer("Enter the register offset: ", 0, 0, 0x58);
    retval = katar_poe_reg_rd(reg, &data);

    if (retval == PASSED)
        printf("Failed to read PoE controller Reg.(0x%02X)\n", (uchar)(reg));
    else
        printf("PoE controller Reg. 0x%02X = 0x%02X.\n",  (uchar)(reg), data);
    return (retval);
}

/*******************************************************************************
 *
 * Function: poke_poe_reg
 *
 * Description: Poke PoE controller register.
 *
 * Input:   
 *
 * Outputs:  PASSED/FAILED
 *
 * Assumptions:
 *
 *******************************************************************************
 */
static int 
poke_poe_reg ()
{
    uchar reg, data = 0;
    int retval=FAILED;

    reg = gethex_answer("Enter the register offset: ", 0, 0, 0xFF);
    data = gethex_answer("Enter the data: ", 0, 0, 0xFF);
    retval = katar_poe_reg_wr(reg, data);
    if (retval == PASSED)
        printf("Failed to read PoE controller Reg.(0x%02X)\n", (uchar)(reg));
    else
        printf("PoE controller Reg. 0x%02X = 0x%02X.\n",  (uchar)(reg), data);

    return (retval);
}

int katar_poe_set_force_intr(void)
{
	uchar data = 0;

	if(katar_get_poe_54V_present() == 0) {
        printf("---- Can't detect POE 54V power. ----\n");
        return (FAILED);
    }

	/* enable intr pin  */
    data = 0x1;
    if (katar_poe_reg_wr(POE_INTR_EN_REG, data) != PASSED) {
        printf("Failed to set port 1 in manual mode.\n");
        return (FAILED);
    }

	/* before the test, set to manual mode and force to turn on port 1 */
	data = 0x1;
	if (katar_poe_reg_wr(ILP_OPERATING_MODE, data) != PASSED) {
		printf("Failed to set port 1 in manual mode.\n");
        return (FAILED);
    }

	data = 0x1;
    if (katar_poe_reg_wr(ILP_POWER_ENABLE_PB, data) != PASSED) {
        printf("Failed to force turn on port 1.\n");
        return (FAILED);
    }
	msleep(200);
	/* enable POE power enable status change interrupt (register 1, bit 0) */
	data = POWER_ENABLE;
	if (katar_poe_reg_wr(ILP_INT_MASK, data) != PASSED) {
        printf("POE I2C write failed.\n");
        return (FAILED);
    }
	/* force to turn off port 1 to generate power good status change interrupt */
	data = 0x10;
	if (katar_poe_reg_wr(ILP_POWER_ENABLE_PB, data) != PASSED) {
        printf("Failed to force turn off port 1.\n");
        return (FAILED);
    }
	return PASSED;
}

int katar_poe_clear_force_intr(void)
{
	uchar data = 0;
	int status = PASSED;

	/* disable POE interrupts */
    data = 0x0;
    if (katar_poe_reg_wr(POE_INTR_EN_REG, data) != PASSED) {
        printf("Failed to set port 1 in manual mode.\n");
        return (FAILED);
    }
	data = 0;
    if (katar_poe_reg_wr(ILP_INT_MASK,data) != PASSED) {
        printf("POE I2C write failed.");
        status = FAILED;
    }

    /* set OFF mode for port 1 */
    data = 0;
    if (katar_poe_reg_wr(ILP_OPERATING_MODE,data) != PASSED) {
        cterr('f',0,"Failed to set port 1 in manual mode.\n");
        status = FAILED;
    }

    /* reset port 1 to clear all event registers */
    data = 0x1;
    if (katar_poe_reg_wr(ILP_GLOBAL_PB,data) != PASSED) {
        cterr('f',0, "POE I2C write failed.");
        status = FAILED;
    }
	return (status);
}

/*******************************************************************************
 *
 * Function    : poe_port_opmod
 * Description : Function to get PoE port state.
 * Inputs      : p_num  - number of PoE port(1/2)
 *               *opmod_str   - buffer to put port opmod string
 * Outputs     : STAT_OK/STAT_FAIL
 *
 *******************************************************************************
 */
static int poe_port_opmod (int p_num, char *opmod_str)
{
    int     opmod_reg = POE_OPMD_REG;
    uint8_t stat_val = 0;
	int 	opmask=0;

    switch (p_num) {
    case POE_PORT_1:
        opmask = 0x03;
        break;
    case POE_PORT_2:
        opmask = 0x0C;
        break;
    case POE_PORT_3:
        opmask = 0x30;
        break;
    case POE_PORT_4:
        opmask = 0xC0;
        break;
    default:
        printf("%s: Unsupported port number %d.\n",
                           __FUNCTION__, p_num);
        return (FAILED);
    }

    if (katar_poe_reg_rd(opmod_reg, &stat_val) != PASSED) {
        printf("%s: Failed to read PoE Reg.(0x%02X)\n",
                           __FUNCTION__, opmod_reg);
        return (FAILED);
    }

	stat_val &= opmask;

    switch (stat_val) {
    case 0:
        sprintf(opmod_str, "Shutdown");
        break;
    case POE_OPMD_P1_AUTO:
	case POE_OPMD_P2_AUTO:
	case POE_OPMD_P3_AUTO:
	case POE_OPMD_P4_AUTO:
        sprintf(opmod_str, "Auto");
        break;
    case POE_OPMD_P1_SEMI:
	case POE_OPMD_P2_SEMI:
	case POE_OPMD_P3_SEMI:
	case POE_OPMD_P4_SEMI:
        sprintf(opmod_str, "Semi-auto");
        break;
    case POE_OPMD_P1_MANU:
	case POE_OPMD_P2_MANU:
	case POE_OPMD_P3_MANU:
	case POE_OPMD_P4_MANU:
        sprintf(opmod_str, "Manual");
        break;
    default:
        printf("%s: Unsupported detection number %d.\n",
                           __FUNCTION__, stat_val);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function    : poe_port_stat
 * Description : Function to get PoE port state.
 * Inputs      : p_num  - number of PoE port(1/2)
 *               *det_str   - buffer to put port detection string
 *               *class_str - buffer to put port class string
 * Outputs     : STAT_OK/STAT_FAIL
 *
 *******************************************************************************
 */
static int poe_port_stat (int p_num, char *det_str, char *class_str)
{
    int     stat_reg = 0xc;
    uint8_t stat_val = 0;
    int     s_det = 0, s_class = 0;

    switch (p_num) {
    case POE_PORT_1:
        stat_reg = POE_STATP1_REG;
        break;
    case POE_PORT_2:
        stat_reg = POE_STATP2_REG;
        break;
    default:
        printf("%s: Unsupported port number %d.\n",
                           __FUNCTION__, p_num);
        return (FAILED);
    }

    if (katar_poe_reg_rd(stat_reg, &stat_val) != PASSED) {
        printf("%s: Failed to read PoE Reg.(0x%02X)\n",
                           __FUNCTION__, stat_reg);
        return (FAILED);
    }

    s_det = (int)(stat_val & (uint8_t)POE_STAT_DETECT);
    s_class = (int)((stat_val & (uint8_t)POE_STAT_CLASS) >> POE_STAT_CLASS_OFF);

    switch (s_det) {
    case POE_UNKNOWN:
        sprintf(det_str, "Unknown");
        break;
    case POE_SHORT:
        sprintf(det_str, "Short");
        break;
    case POE_CPD_TOO_HIGH:
        sprintf(det_str, "Cpd too high");
        break;
    case POE_RSIG_TOO_LOW:
        sprintf(det_str, "RSIG too low");
        break;
    case POE_GOOD:
        sprintf(det_str, "Good");
        break;
    case POE_RSIG_TOO_HIGH:
        sprintf(det_str, "RSIG too high");
        break;
    case POE_OPEN_CIRCUIT:
        sprintf(det_str, "Open circuit");
        break;
    case POE_RESERVED:
        sprintf(det_str, "Reserved");
        break;
    default:
        printf("%s: Unsupported detection number %d.\n",
                           __FUNCTION__, s_det);
        return (FAILED);
    }

    switch (s_class) {
    case POE_C_UNKNOWN:
        sprintf(class_str, "Unknown");
        break;
    case POE_C_CLASS_1:
        sprintf(class_str, "Class 1");
        break;
    case POE_C_CLASS_2:
        sprintf(class_str, "Class 2");
        break;
    case POE_C_CLASS_3:
        sprintf(class_str, "Class 3");
        break;
    case POE_C_CLASS_4:
        sprintf(class_str, "Class 4");
        break;
    case POE_C_RESERVED:
        sprintf(class_str, "Reserved");
        break;
    case POE_C_CLASS_0:
        sprintf(class_str, "Class 0");
        break;
    case POE_C_OVERCURRENT:
        sprintf(class_str, "Over-current");
        break;
    default:
        printf("%s: Unsupported detection number %d.\n",
                           __FUNCTION__, s_det);
        return (FAILED);
    }

    return (PASSED);
}

/*******************************************************************************
*
* Function    : poe_port_power
* Description : Function to get PoE port power consumption.
* Inputs      : p_num  - number of PoE port(1/2)
*               *p_pwr - buffer to put port power consumption
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
static int poe_port_power (int p_num, int *p_pwr){
	int     pwr_reg = 0x92;
	uchar pwr_val = 0;
	switch (p_num) {
		case POE_PORT_1:
			pwr_reg = POE_PORT1_CONS_REG;
			break;
		case POE_PORT_2:
			pwr_reg = POE_PORT2_CONS_REG;
			break;
		default:
			printf("%s: Unsupported port number %d.\n",  __FUNCTION__, p_num);
			return (FAILED);
	}
	/* Get port power consumption */
	if (katar_poe_reg_rd(pwr_reg, &pwr_val) != PASSED) {
		printf("%s: Failed to read PoE reg.(0x%02X).\n", __FUNCTION__, pwr_reg);
		return (FAILED);
	}
	*p_pwr = (int)pwr_val;
	return (PASSED);
}
/*******************************************************************************
*
* Function    : poe_port_current
* Description : Function to get PoE port current.
* Inputs      : p_num - number of PoE port(1/2)
*               *curr - buffer to put port current value
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
static int poe_port_current (int p_num, float *curr){
	int      curr_l_reg = 0x30, curr_m_reg = 0x31;
	uchar  curr_l_val = 0, curr_m_val = 0;
	uint16_t curr_val = 0;
	switch (p_num) {
		case POE_PORT_1:
			curr_l_reg = POE_IP1LSB_REG;
			curr_m_reg = POE_IP1MSB_REG;
			break;
	       case POE_PORT_2:
		   	curr_l_reg = POE_IP2LSB_REG;
			curr_m_reg = POE_IP2MSB_REG;
			break;
		default:
			printf("%s: Unsupported port number %d.\n", __FUNCTION__, p_num);
			return (FAILED);
	}
	/* Get port current */
	/* 1-1. Get port current LSB */
	if (katar_poe_reg_rd(curr_l_reg, &curr_l_val) != PASSED) {
		printf("%s: Failed to read PoE Reg.(0x%02X)\n", __FUNCTION__, curr_l_reg);
		return (FAILED);
	}
	/* 1-2. Get port current MSB */
	if (katar_poe_reg_rd(curr_m_reg, &curr_m_val) != PASSED) {
		printf("%s: Failed to read PoE Reg.(0x%02X)\n", __FUNCTION__, curr_m_reg);
		return (FAILED);
	}
	/* 1-3. Combine current LSB and MSB */
	curr_val = (uint16_t)((curr_m_val << 8) | curr_l_val);
	/* 1-4. Calculate to get real current value */
	*curr = (float)(curr_val * 122.07);
	return (PASSED);
}
/*******************************************************************************
*
* Function    : poe_port_voltage
* Description : Function to get PoE port voltage.
* Inputs      : p_num   - number of PoE port(1/2)
*               *p_volt - buffer to put port voltage value
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
static int poe_port_voltage (int p_num, float *p_volt){
	int      volt_l_reg = 0x32, volt_m_reg = 0x33;
	uchar  volt_l_val = 0, volt_m_val = 0;
	uint16_t volt_val = 0;
	switch (p_num) {
		case POE_PORT_1:
			volt_l_reg = POE_VP1LSB_REG;
			volt_m_reg = POE_VP1MSB_REG;
			break;
		case POE_PORT_2:
			volt_l_reg = POE_VP2LSB_REG;
			volt_m_reg = POE_VP2MSB_REG;
			break;
		default:
			printf("%s: Unsupported port number %d.\n", __FUNCTION__, p_num);
			return (FAILED);
	}
	/* Get port voltage */
	/* 1-1. Get port voltage LSB */
	if (katar_poe_reg_rd(volt_l_reg, &volt_l_val) != PASSED) {
		printf("%s: Failed to read PoE Reg.(0x%02X)\n", __FUNCTION__, volt_l_reg);
		return (FAILED);
		}
	/* 1-2. Get port voltage MSB */
	if (katar_poe_reg_rd(volt_m_reg, &volt_m_val) != PASSED) {
		printf("%s: Failed to read PoE Reg.(0x%02X)\n", __FUNCTION__, volt_m_reg);
		return (FAILED);
		}
	/* 1-3. Combine voltage LSB and MSB */
	volt_val = (uint16_t)((volt_m_val << 8) | volt_l_val);
	/* 1-4. Calculate to get real voltage value */
	*p_volt = (float)(volt_val * 5.835);
	return (PASSED);
}

/*******************************************************************************
 *
 * Function    : poe_get_ic_temp
 * Description : Function to read PoE controller IC temperature(deg C).
 * Inputs      : *ret_buf - buffer to put the read back temperature
 * Outputs     : PASSED - No errors encountered. 
 *                   FAILED - Errors encountered.
 * *******************************************************************************
 */
static int poe_get_ic_temp (float *ret_buf)
{    
    float   ctemp = 0;
    uchar reg_val = 0;
    /* Read Temperature sensor data from PoE VTEMP(0x70) Reg. */
    if (katar_poe_reg_rd((int)POE_VTEMP_REG, &reg_val) != PASSED)
    {
        printf("Failed to read VTEMP reg.(0x%02X)\n", (uchar)POE_VTEMP_REG);
        return (FAILED);
    }
    /* Transfer read back temp. data to deg Celsius */
    ctemp = (float)((reg_val * 0.96) - 27);
    //printf("%s: IC Temp. = %3.2f deg C.\n", __FUNCTION__, ctemp);
    *ret_buf = ctemp;
    return (PASSED);
}

/*******************************************************************************
*
* Function    : poe_get_port_info
* Description : Function to get PoE port current info.
* Inputs      : p_num - number of PoE port(1/2)
* Outputs     : PASSED/FAILED
* *******************************************************************************
*/
static int poe_get_port_info (int p_num)
{
	int      p_power = 0;
	float    p_curr = 0, p_volt = 0;
	//char     stat_det[MAX_CMD_LINE_LEN], stat_class[MAX_CMD_LINE_LEN];
	char     stat_det[128], stat_class[128], opmod[128];

    if(katar_get_poe_54V_present() == 0) {
        printf("---- Can't detect POE 54V power. ----\n");
        return (FAILED);
    }

	/* . Get port opmod */
    if (poe_port_opmod(p_num, opmod) != PASSED) {
        printf("%s Failed to get PoE port%d opmod.\n",__FUNCTION__, p_num);
        return (FAILED);
    }
	/* 1. Get port status */
	if (poe_port_stat(p_num, stat_det, stat_class) != PASSED) {
		printf("%s Failed to get PoE port%d state.\n",__FUNCTION__, p_num);
		return (FAILED);
	}
	/* 2. Get port power consumption */
	if (poe_port_power(p_num, &p_power) != PASSED) {
		printf("%s Failed to get PoE port%d power consumption.\n", __FUNCTION__, p_num);
		return (FAILED);
	}
	/* 3. Get port current */
	if (poe_port_current(p_num, &p_curr) != PASSED) {
		printf("%s Failed to get PoE port%d current.\n", __FUNCTION__, p_num);
		return (FAILED);
	}
	/* 4. Get port voltage */
	if (poe_port_voltage(p_num, &p_volt) != PASSED) {
		printf("%s Failed to get PoE port%d voltage.\n", __FUNCTION__, p_num);
		return (FAILED);
	}
	/* Print out PoE port info */
	printf("\nPoE port%d info\n", p_num);
    printf("-operating mode: %s\n", opmod);
	printf("-detection: %s\n", stat_det);
	printf("-class: %s\n", stat_class);
	printf("-power consumption: %d W\n", p_power);
	printf("-current: %f uA\n", p_curr);
	printf("-voltage: %f mV\n", p_volt);
	return (PASSED);
}


int 
poe_status_util (int bBrief)
{
    uchar poe_id = 0,poe_devid = 0;
    float   ic_temp = 0;
    int     ctr = 1;
	char    opmod[128];

    if(katar_get_poe_54V_present() == 0) {
        printf("---- Can't detect POE 54V power. ----\n\n");
        return (FAILED);
    }

	if(bBrief)
	{
		/* Get PoE controller IC Temp. */
        if (poe_get_ic_temp(&ic_temp) != PASSED) {
            printf("Failed to get PoE IC Temp.\n");
            return (FAILED);
        }
		printf("PoE controller temperature : %.4f Celcius\n",ic_temp);
		/* Show PoE port status */
        for (ctr = 1; ctr <= KATAR_MAX_POE_PORT; ctr++) {
		    if (poe_port_opmod(ctr, opmod) != PASSED) {
        		printf("%s Failed to get PoE port%d opmod.\n",__FUNCTION__, ctr);
		        return (FAILED);
		    }
			printf("PoE port %d operating mode: %s\n", ctr, opmod);
		}
		printf("\n");
		return (PASSED);
	}

    /* Get PoE controller IDs */
        if (katar_poe_reg_rd((int)POE_ID_REG, &poe_id) != PASSED) {
                printf("Failed to read ID reg.(0x%02X)\n", (uchar)POE_ID_REG);
                return (FAILED);
                }
        if (katar_poe_reg_rd((int)POE_DEVID_REG, &poe_devid) != PASSED) {
                printf("Failed to read DEVID reg.(0x%02X)\n", (uchar)POE_DEVID_REG);
                return (FAILED);
                }
        /* Get PoE controller IC Temp. */
        if (poe_get_ic_temp(&ic_temp) != PASSED) {
                printf("Failed to get PoE IC Temp.\n");
                return (FAILED);
        }
        /* Show PoE controller status */
        printf("Katar PoE controller status:\n");
        printf("-ID: 0x%02X, DEVID: 0x%02X.\n", poe_id, poe_devid);
        printf("-IC Temp: %3.2f deg C.\n", ic_temp);
        /* Show PoE port status */
        for (ctr = 1; ctr <= KATAR_MAX_POE_PORT; ctr++) {
                if (poe_get_port_info(ctr) != PASSED) {
                        printf("Failed to get PoE port %d info.\n", ctr);
                        if (ctr == KATAR_MAX_POE_PORT) {
                                return (FAILED);
                        }
                }
        }
        return (PASSED);
}

/*******************************************************************************
*
* Function    : katar_poe_pwrctrl
* Description : Function to control Katar PoE port power on/off.
* Inputs      : p_num - port 1/2 that wants to control its power
*               p_opt - port power on/off
* Outputs     : PASSED/FAILED
*
*******************************************************************************
*/
int katar_poe_pwrctrl (int p_num, int p_opt){
	uchar reg_val = 0;
	if (katar_poe_reg_rd((int)POE_OPMD_REG, &reg_val) != PASSED) {
		printf("%s: Failed to read PoE Reg.(0x%02X)\n",  __FUNCTION__, (uchar)POE_OPMD_REG);
		return (FAILED);
	}
	printf("%s: port %d, power option = %#X, reg_val = 0x%02X.\n", __FUNCTION__, p_num, p_opt, reg_val);
	switch (p_num) {
		case POE_PORT_1:
			if (p_opt == POE_PWR_OFF) {
				reg_val &= (uchar)(~POE_OPMD_P1_AUTO);
			} else {
				reg_val |= (uchar)(POE_OPMD_P1_AUTO);
			}
			break;
		case POE_PORT_2:
			if (p_opt == POE_PWR_OFF) {
				reg_val &= (uchar)(~POE_OPMD_P2_AUTO);
			} else {
				reg_val |= (uchar)(POE_OPMD_P2_AUTO);
			}
			break;
		default:
			printf("%s: Unsupported port number %d.\n",  __FUNCTION__, p_num);
			return (FAILED);
	}
	if (katar_poe_reg_wr((int)POE_OPMD_REG, reg_val) != PASSED) {
		printf("%s: Failed to write PoE Reg.(0x%02X)\n", __FUNCTION__, (uchar)POE_OPMD_REG);
		return (FAILED);
	}
	if (p_opt == POE_PWR_ON) {
		reg_val = 0;
		if (katar_poe_reg_rd((int)POE_DETENA_REG, &reg_val) != PASSED) {
			printf("%s: Failed to read PoE Reg.(0x%02X)\n",  __FUNCTION__, (uchar)POE_DETENA_REG);
			return (FAILED);
		}
		switch (p_num) {
			case POE_PORT_1:
				reg_val |= (uchar)(POE_DETENA_P1_DET | POE_DETENA_P1_CLA);
				break;
			case POE_PORT_2:
				reg_val |= (uchar)(POE_DETENA_P2_DET | POE_DETENA_P2_CLA);
				break;
			default:
				printf("%s: Unsupported port number %d.\n", __FUNCTION__, p_num);
				return (FAILED);
		}
		if (katar_poe_reg_wr((int)POE_DETENA_REG, reg_val) != PASSED) {
			printf("%s: Failed to write PoE Reg.(0x%02X)\n",  __FUNCTION__, (uchar)POE_DETENA_REG);
			return (FAILED);
		}
	}
	return (PASSED);
}

/* ******************************************************************
 *
 * Function: poe_power_ports_util
 *
 * Description: Power on or off all ports in Dreamliner.
 *
 * Input:   
 *
 * Outputs:  PASSED - No errors encountered.
 *           FAILED - Errors encountered.
 *
 * Assumptions:
 *
 * ******************************************************************
 */
int 
poe_power_ports_util ()
{
    int  port_num = 3, port_pwr = 0;
    uchar port, enable = 0;

    port = gethex_answer("Enter the port number(0 to exit): ", 0, 0x1, 0x2);
	if(port==0)
	{
		printf("EXIT by user selection\n");
		return (FAILED);
	}

    enable = gethex_answer("Enter the on(1)/off(0): ", 0x1, 0x0, 0x1);
    if (enable == 0) {
        port_pwr = POE_PWR_OFF;
    } 
    else if (enable == 1) 
    {
        port_pwr = POE_PWR_ON;
    }
    else 
    {
        printf("%s:%d Syntax Error.\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    port_num = port;
    if ((port_num < 1) || (port_num > 2)) {
        printf("%s:%d Syntax Error.\n", __FUNCTION__, __LINE__);
        return (FAILED);
    }
    if (katar_poe_pwrctrl(port_num, port_pwr) != PASSED) {
        printf("Failed to power %s PoE port %d.\n",  (port_pwr == POE_PWR_ON) ? "ON" : "OFF", port_num);
        return (FAILED);
    }
    printf("PoE port %d is powered %s.\n",  port_num, (port_pwr == POE_PWR_ON) ? "ON" : "OFF");
    if (poe_get_port_info(port_num) != PASSED) {
        printf("Failed to get PoE port %d info.\n", port_num);
        return (FAILED);
    }
    return (PASSED);
}

/*
 *------------------------------------------------------------------
 * $Log: platform_poe.c,v $
 * Revision 1.2  2019/06/14 05:24:51  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.6  2019/06/10 03:47:18  mikech2
 * Remove platform_fru.h base on PRRQ#4685780 Comment#6
 *
 * Revision 1.1.2.5  2019/04/30 06:06:59  mikech2
 * Code cleanup
 *
 * Revision 1.1.2.4  2019/03/13 06:46:50  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.3  2019/03/08 07:19:19  mikech2
 * Clean up codes
 *
 * Revision 1.1.2.2  2019/02/12 08:06:30  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.1  2019/01/29 01:54:21  mikech2
 * rename katar_* files
 *
 * Revision 1.1.2.5  2018/12/12 09:06:16  mikech2
 * Update FPGA utility according to SPEC2.2(FW ver:2018121214)
 *
 * Revision 1.1.2.4  2018/11/22 06:55:25  mikech2
 * Add POE 54V detect
 *
 * Revision 1.1.2.3  2018/11/13 02:53:07  mikech2
 * Add PoE operating mode info
 *
 * Revision 1.1.2.2  2018/11/08 06:00:15  mikech2
 * Add fan low and interrupt test in mb test and remove intr utility
 *
 * Revision 1.1.2.1  2018/10/22 08:02:30  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.2  2018/09/04 06:09:08  mikech2
 * Fix I2C util , realtek port & get_pcie_cap_struct_ptr return error issue
 *
 * Revision 1.1.2.1  2018/07/17 11:34:21  benlu
 * For poe diag test
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


